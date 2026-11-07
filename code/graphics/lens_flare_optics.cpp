#include "lens_flare.h"
#include "lens_flare_internal.h"

// for MAX_LENS_FLARE_INSTANCES: a ghost the shader has no instance slot for is
// not worth enumerating, so the budget bounds the precompute
#include "graphics/util/uniform_structs.h"

#include <algorithm>
#include <cmath>

// The paraxial optics behind the flare: a lens_system is reduced here to a set
// of two-reflection ghost paths, each with its own ray-transfer matrices and
// coated-Fresnel tint, once at table load. Everything in this file is a pure
// function of the prescription -- no engine state, no frame, no screen -- with
// one exception: the ghost count is capped at the shader's instance budget,
// since a ghost with no instance slot to draw it in is not worth enumerating.

namespace graphics {
namespace {

// ---- 2x2 ray-transfer matrix helpers ([A B; C D] acting on [height; angle]) ----

struct mat2 {
	float a, b, c, d;
};

mat2 m2_identity() { return {1.0f, 0.0f, 0.0f, 1.0f}; }

mat2 m2_mul(const mat2& m, const mat2& n)
{
	return {m.a * n.a + m.b * n.c, m.a * n.b + m.b * n.d, m.c * n.a + m.d * n.c, m.c * n.b + m.d * n.d};
}

mat2 m2_translate(float t) { return {1.0f, t, 0.0f, 1.0f}; }

// Refraction at a spherical interface with signed curvature 1/R, from index n1 into n2
mat2 m2_refract(float n1, float n2, float inv_r) { return {1.0f, 0.0f, (n1 - n2) * inv_r / n2, n1 / n2}; }

// Mirror reflection at a spherical surface with signed curvature 1/R
mat2 m2_reflect(float inv_r) { return {1.0f, 0.0f, 2.0f * inv_r, 1.0f}; }

mat2 m2_inverse(const mat2& m)
{
	float det = m.a * m.d - m.b * m.c;
	return {m.d / det, -m.b / det, -m.c / det, m.a / det};
}

float surface_inv_radius(const lens_surface& s)
{
	return (s.radius != 0.0f) ? 1.0f / s.radius : 0.0f;
}

// Refractive index behind a surface at one of the three design wavelengths,
// with Cauchy 2-term dispersion fitted through n_d and the Abbe number.
float surface_index(const lens_surface& s, int wl)
{
	if (s.n <= 1.0005f || s.abbe <= 0.0f) {
		return s.n;
	}
	constexpr float lF = 0.48613f, lC = 0.65627f, lD = 0.58756f;
	float B = (s.n - 1.0f) / (s.abbe * (1.0f / (lF * lF) - 1.0f / (lC * lC)));
	float A = s.n - B / (lD * lD);
	float l = Wavelengths_um[wl];
	return A + B / (l * l);
}

float index_after(const lens_system& lens, int surf, int wl)
{
	return surface_index(lens.surfaces[surf], wl);
}

float index_before(const lens_system& lens, int surf, int wl)
{
	return (surf == 0) ? 1.0f : surface_index(lens.surfaces[surf - 1], wl);
}

int find_stop_index(const lens_system& lens)
{
	for (int i = 0; i < static_cast<int>(lens.surfaces.size()); i++) {
		if (lens.surfaces[i].is_stop) {
			return i;
		}
	}
	return -1;
}

// Forward system matrix (no reflections) from the first surface to just after
// the last surface, at the given wavelength.
mat2 system_matrix(const lens_system& lens, int wl)
{
	mat2 m = m2_identity();
	int n = static_cast<int>(lens.surfaces.size());
	for (int s = 0; s < n; s++) {
		m = m2_mul(m2_refract(index_before(lens, s, wl), index_after(lens, s, wl), surface_inv_radius(lens.surfaces[s])), m);
		if (s < n - 1) {
			m = m2_mul(m2_translate(lens.surfaces[s].thickness), m);
		}
	}
	return m;
}

// Compose the ray-transfer matrices of one two-reflection ghost path
// (first reflection at surface hi going forward, second at surface lo going
// backward, lo < hi), splitting at the LAST aperture-stop crossing.
void trace_ghost_path(const lens_system& lens, int hi, int lo, int stop, int wl, float bfd,
	float out_ma[4], float out_ms[4])
{
	mat2 m = m2_identity();
	mat2 ma = m2_identity();
	bool have_ma = false;

	auto note_stop = [&]() {
		ma = m;
		have_ma = true;
	};

	const auto& surf = lens.surfaces;
	int n = static_cast<int>(surf.size());

	// Phase 1: forward from surface 0 up to surface hi
	for (int s = 0; s < hi; s++) {
		if (s == stop) {
			note_stop();
		}
		m = m2_mul(m2_refract(index_before(lens, s, wl), index_after(lens, s, wl), surface_inv_radius(surf[s])), m);
		m = m2_mul(m2_translate(surf[s].thickness), m);
	}

	// Reflect at surface hi (now travelling backward; radii of surfaces crossed
	// backward flip sign in the unfolded system)
	m = m2_mul(m2_reflect(surface_inv_radius(surf[hi])), m);

	// Phase 2: backward from surface hi down to surface lo
	for (int s = hi - 1; s > lo; s--) {
		m = m2_mul(m2_translate(surf[s].thickness), m);
		if (s == stop) {
			note_stop();
		}
		m = m2_mul(m2_refract(index_after(lens, s, wl), index_before(lens, s, wl), -surface_inv_radius(surf[s])), m);
	}
	m = m2_mul(m2_translate(surf[lo].thickness), m);

	// Reflect at surface lo (hit from behind; forward again)
	m = m2_mul(m2_reflect(-surface_inv_radius(surf[lo])), m);

	// Phase 3: forward from surface lo to the sensor
	for (int s = lo + 1; s < n; s++) {
		m = m2_mul(m2_translate(surf[s - 1].thickness), m);
		if (s == stop) {
			note_stop();
		}
		m = m2_mul(m2_refract(index_before(lens, s, wl), index_after(lens, s, wl), surface_inv_radius(surf[s])), m);
	}
	m = m2_mul(m2_translate(bfd), m);

	if (!have_ma) {
		// Degenerate prescription (no stop crossing); treat the whole path as Ms
		ma = m2_identity();
	}

	mat2 ms = m2_mul(m, m2_inverse(ma));

	out_ma[0] = ma.a;
	out_ma[1] = ma.b;
	out_ma[2] = ma.c;
	out_ma[3] = ma.d;
	out_ms[0] = ms.a;
	out_ms[1] = ms.b;
	out_ms[2] = ms.c;
	out_ms[3] = ms.d;
}

float ghost_coating_wavelength(const lens_system& lens, const lens_surface& s)
{
	return (s.coating_wavelength < 0.0f) ? lens.coating_wavelength : s.coating_wavelength;
}

} // namespace

float lens_flare_fresnel_reflectance(float n1, float n2, float lambda0_nm, float lambda_nm)
{
	if (lambda0_nm <= 0.0f) {
		float r = (n1 - n2) / (n1 + n2);
		return r * r;
	}

	// Single quarter-wave layer (tuned to lambda0) between n1 and n2, ideally
	// index sqrt(n1*n2) but no better than MgF2 (1.38), at normal incidence
	float nc = MAX(1.38f, sqrtf(n1 * n2));
	float r1 = (n1 - nc) / (n1 + nc);
	float r2 = (nc - n2) / (nc + n2);
	float cphi = cosf(PI * lambda0_nm / lambda_nm);
	float num = r1 * r1 + r2 * r2 + 2.0f * r1 * r2 * cphi;
	float den = 1.0f + r1 * r1 * r2 * r2 + 2.0f * r1 * r2 * cphi;
	return num / den;
}

bool lens_flare_precompute(lens_system& lens)
{
	lens.ghosts.clear();

	int n = static_cast<int>(lens.surfaces.size());
	if (n < 2) {
		return false;
	}

	// Normalize stop surfaces: flat, index-continuous with the preceding medium
	for (int i = 0; i < n; i++) {
		if (lens.surfaces[i].is_stop) {
			lens.surfaces[i].radius = 0.0f;
			lens.surfaces[i].n = (i == 0) ? 1.0f : lens.surfaces[i - 1].n;
			lens.surfaces[i].abbe = (i == 0) ? 0.0f : lens.surfaces[i - 1].abbe;
		}
	}

	int stop = find_stop_index(lens);
	if (stop < 0) {
		// No explicit stop: use the middle surface's plane for aperture clipping
		stop = n / 2;
	}

	// Effective focal length and back focal distance (green), sensor placed at
	// the infinity focus
	mat2 sys = system_matrix(lens, 1);
	if (fabsf(sys.c) < 1e-6f) {
		return false; // afocal; can't image onto a sensor
	}
	lens.efl = -1.0f / sys.c;
	lens.bfd = -sys.a / sys.c;
	if (lens.efl <= 0.0f || lens.bfd <= 0.0f) {
		return false;
	}

	// Enumerate all two-reflection ghost paths between refractive surfaces
	struct scored_ghost {
		lens_flare_ghost g;
		float key;
	};
	SCP_vector<scored_ghost> scored;

	for (int hi = 1; hi < n; hi++) {
		if (fabsf(index_before(lens, hi, 1) - index_after(lens, hi, 1)) < 1e-4f) {
			continue; // no index step -> no reflection (also skips the stop)
		}
		for (int lo = 0; lo < hi; lo++) {
			if (fabsf(index_before(lens, lo, 1) - index_after(lens, lo, 1)) < 1e-4f) {
				continue;
			}

			scored_ghost sg;
			sg.g.surf_first = hi;
			sg.g.surf_second = lo;

			for (int wl = 0; wl < 3; wl++) {
				trace_ghost_path(lens, hi, lo, stop, wl, lens.bfd, sg.g.ma[wl], sg.g.ms[wl]);

				float lambda_nm = Wavelengths_um[wl] * 1000.0f;
				float r_first = lens_flare_fresnel_reflectance(index_before(lens, hi, wl), index_after(lens, hi, wl),
					ghost_coating_wavelength(lens, lens.surfaces[hi]), lambda_nm);
				float r_second = lens_flare_fresnel_reflectance(index_after(lens, lo, wl), index_before(lens, lo, wl),
					ghost_coating_wavelength(lens, lens.surfaces[lo]), lambda_nm);
				sg.g.reflectance[wl] = r_first * r_second;
			}

			if (sg.g.reflectance[1] < 1e-6f) {
				continue;
			}

			// Brightness-ish sort key: reflectance, boosted for concentrated
			// (small-footprint) ghosts
			float a_g = sg.g.ms[1][0] * sg.g.ma[1][0] + sg.g.ms[1][1] * sg.g.ma[1][2];
			sg.key = sg.g.reflectance[1] * MIN(1.0f / (a_g * a_g + 1e-3f), 100.0f);
			scored.push_back(sg);
		}
	}

	std::sort(scored.begin(), scored.end(), [](const scored_ghost& x, const scored_ghost& y) { return x.key > y.key; });

	int cap = std::clamp(lens.max_ghosts, 1, generic_data::MAX_LENS_FLARE_INSTANCES - 1);
	for (const auto& sg : scored) {
		if (static_cast<int>(lens.ghosts.size()) >= cap) {
			break;
		}
		lens.ghosts.push_back(sg.g);
	}

	return !lens.ghosts.empty();
}

} // namespace graphics
