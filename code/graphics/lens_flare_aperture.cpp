#include "lens_flare.h"
#include "lens_flare_internal.h"

#include <algorithm>
#include <cmath>
#include <complex>

// Image synthesis for the iris: one aperture definition is rasterized into an R8
// transmission mask (blade shape plus the optional grating/scratch/dust layers,
// ported from realflare's kernels), and the starburst is that mask's Fraunhofer
// transform, i.e. |FFT(mask)|^2. Like the optics, none of this touches engine
// state -- it is a pure function of a lens_aperture.

namespace graphics {
namespace {

constexpr int APERTURE_TEXTURE_SIZE = 512;

// ---- texture generation ----

const float IRIS_RADIUS = 0.9f; // in normalized [-1,1] texture space; keeps the border black

// Hash used by realflare's aperture kernels to scatter scratches and dust.
// Kept bit-for-bit so a given density reproduces its layout.
float aperture_noise(float x, float y, float z)
{
	float ignored;
	return modff(sinf(x * 112.9898f + y * 179.233f + z * 237.212f) * 43758.5453f, &ignored);
}

// Signed distance to an axis-aligned rectangle of the given half-extents
float sdf_rectangle(float px, float py, float hx, float hy)
{
	float ex = fabsf(px) - hx;
	float ey = fabsf(py) - hy;
	float outside = sqrtf(MAX(ex, 0.0f) * MAX(ex, 0.0f) + MAX(ey, 0.0f) * MAX(ey, 0.0f));
	float inside = MIN(MAX(ex, ey), 0.0f);
	return outside + inside;
}

float smoothstep01(float edge0, float edge1, float x)
{
	if (edge0 == edge1) {
		return (x < edge0) ? 0.0f : 1.0f;
	}
	float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

// The iris opening (realflare's aperture_shape kernel). Returns transmission in
// [0,1] at a point in normalized texture space.
//
// The polygon is the intersection of `blades` half-planes; curvature bows each
// blade by adding a per-blade sine bulge to the distance field, exactly as
// realflare's "roundness" does, but scaled so that our curvature keeps its
// original meaning: 0 leaves the blades straight, 1 pushes each blade's midpoint
// out to the corner radius (a circular iris), and negative values bow the blades
// inward into a star.
float aperture_shape(const lens_aperture& ap, float px, float py)
{
	const int blades = ap.blades;
	const float curvature = std::clamp(ap.curvature, -1.0f, 1.0f);
	const float softness = MAX(ap.softness, 2.0f / APERTURE_TEXTURE_SIZE); // never sub-pixel

	if (blades < 3 || curvature >= 0.999f) {
		// exact circle; the polygon path only approaches one
		float r = sqrtf(px * px + py * py) / IRIS_RADIUS;
		return 1.0f - smoothstep01(1.0f - softness, 1.0f + softness, r);
	}

	const float rot = ap.rotation * (PI / 180.0f);
	const float c = cosf(rot), s = sinf(rot);
	const float rx = px * c + py * s;
	const float ry = py * c - px * s;

	// Half-plane intersection, normalized so the corners (not the blade
	// midpoints) sit at the iris radius, matching the pre-curvature look.
	// The half-sector phase keeps a corner on the +x axis at rotation 0, which
	// is where the polygon used to put one.
	const float sector = 2.0f * PI / blades;
	const float apothem = IRIS_RADIUS * cosf(PI / blades);
	float sdf = 0.0f;
	for (int i = 0; i < blades; i++) {
		float angle = (static_cast<float>(i) + 0.5f) * sector;
		sdf = MAX(sdf, (cosf(angle) * rx + sinf(angle) * ry) / apothem);
	}

	// Per-blade bulge: realflare's sine gradient, phrased in our own frame --
	// 0 at the corners, 1 at each blade's midpoint
	float t = atan2f(ry, rx) / sector;
	t -= floorf(t);
	float bulge = sinf(t * PI);

	// curvature 1 must lift the midpoints (sdf 1) to the corner radius
	sdf -= bulge * curvature * (1.0f / cosf(PI / blades) - 1.0f);

	return 1.0f - smoothstep01(1.0f - softness, 1.0f + softness, sdf);
}

// Multiply an occlusion primitive into the mask over its bounding box only.
// realflare evaluates min(sdf) over every primitive at every pixel, which is
// fine on a GPU but far too slow on the CPU; taking the max of the smoothstepped
// occlusion instead is equivalent (smoothstep is monotonically decreasing in
// sdf) and lets each primitive touch only the pixels it covers.
template <typename Sdf>
void aperture_stamp(SCP_vector<float>& occlusion, float cx, float cy, float reach, float softness, Sdf&& sdf)
{
	const int size = APERTURE_TEXTURE_SIZE;
	const float to_px = size * 0.5f;
	int x0 = static_cast<int>(floorf((cx - reach + 1.0f) * to_px));
	int x1 = static_cast<int>(ceilf((cx + reach + 1.0f) * to_px));
	int y0 = static_cast<int>(floorf((cy - reach + 1.0f) * to_px));
	int y1 = static_cast<int>(ceilf((cy + reach + 1.0f) * to_px));
	x0 = MAX(x0, 0);
	y0 = MAX(y0, 0);
	x1 = MIN(x1, size - 1);
	y1 = MIN(y1, size - 1);

	for (int y = y0; y <= y1; y++) {
		float py = (y + 0.5f) / size * 2.0f - 1.0f;
		for (int x = x0; x <= x1; x++) {
			float px = (x + 0.5f) / size * 2.0f - 1.0f;
			float cover = smoothstep01(-softness, softness, -sdf(px, py));
			float& dst = occlusion[static_cast<size_t>(y) * size + x];
			dst = MAX(dst, cover);
		}
	}
}

// Rim diffraction grating (realflare's aperture_grating): radial ridges evenly
// spaced around the iris. Because they are evenly spaced in angle, each pixel
// only has to test the few ridges nearest its own bearing.
//
// realflare anchors the ridges at a fixed distance that lines up with the rim of
// *its* aperture; ours sits at IRIS_RADIUS, so the ridges are anchored there and
// `length` is how far in they reach as a fraction of the iris. `width` is a duty
// cycle of the spacing between neighbouring ridges rather than an absolute size,
// so raising the density thins the ridges instead of merging them into a ring.
void aperture_apply_grating(const lens_aperture& ap, SCP_vector<float>& mask)
{
	const int size = APERTURE_TEXTURE_SIZE;
	const int count = static_cast<int>(MIN(ap.grating.density, 1.0f) * 360.0f);
	if (count <= 0 || ap.grating.length <= 0.0f) {
		return;
	}

	const float step = 2.0f * PI / count;
	const float hl = 0.5f * std::clamp(ap.grating.length, 0.0f, 1.0f) * IRIS_RADIUS;
	const float centre = IRIS_RADIUS - hl; // outer end of every ridge sits on the rim
	const float hw = 0.5f * std::clamp(ap.grating.width, 0.0f, 1.0f) * (step * IRIS_RADIUS);
	const float softness = MAX(ap.grating.softness, 1.0f / size);

	for (int y = 0; y < size; y++) {
		float py = (y + 0.5f) / size * 2.0f - 1.0f;
		for (int x = 0; x < size; x++) {
			float px = (x + 0.5f) / size * 2.0f - 1.0f;

			// nearest ridge to this pixel's bearing, plus neighbours: ridges
			// converge towards the centre, so +-2 avoids gaps between them
			int k = static_cast<int>(lroundf(atan2f(py, px) / step));
			float cover = 0.0f;
			for (int d = -2; d <= 2; d++) {
				float angle = (k + d) * step;
				float c = cosf(angle), s = sinf(angle);
				// into the ridge's own frame, where it lies along +x
				float rx = px * c + py * s;
				float ry = py * c - px * s;
				float sdf = sdf_rectangle(rx - centre, ry, hl, hw);
				cover = MAX(cover, smoothstep01(-softness, softness, -sdf));
			}
			mask[static_cast<size_t>(y) * size + x] *= 1.0f - ap.grating.strength * cover;
		}
	}
}

void aperture_apply_scratches(const lens_aperture& ap, SCP_vector<float>& mask)
{
	const int size = APERTURE_TEXTURE_SIZE;
	const int count = static_cast<int>(MIN(ap.scratches.density, 1.0f) * 1000.0f);
	if (count <= 0) {
		return;
	}

	const float hw = ap.scratches.width * 0.1f * 0.5f;
	const float hl = ap.scratches.length * 0.5f;
	const float softness = MAX(ap.scratches.softness, 1.0f / size);
	const float rot = ap.scratches.rotation * (PI / 180.0f);
	const float rot_var = ap.scratches.rotation_variation * PI;
	const float reach = sqrtf(hw * hw + hl * hl) + softness;

	SCP_vector<float> occlusion(static_cast<size_t>(size) * size, 0.0f);
	for (int i = 0; i < count; i++) {
		auto fi = static_cast<float>(i);
		auto fc = static_cast<float>(count);
		float cx = aperture_noise(fi, fc, 0.0f) * 2.0f - 1.0f;
		float cy = aperture_noise(fi, fc, 1.0f) * 2.0f - 1.0f;
		float angle = rot + (aperture_noise(fi, fc, 2.0f) - 0.5f) * rot_var;
		float c = cosf(angle), s = sinf(angle);

		aperture_stamp(occlusion, cx, cy, reach, softness, [=](float px, float py) {
			// rotate about the scratch centre, then measure against the sliver
			float dx = px - cx, dy = py - cy;
			return sdf_rectangle(dx * c + dy * s, dy * c - dx * s, hw, hl);
		});
	}

	for (size_t i = 0; i < mask.size(); i++) {
		mask[i] *= 1.0f - ap.scratches.strength * occlusion[i];
	}
}

void aperture_apply_dust(const lens_aperture& ap, SCP_vector<float>& mask)
{
	const int size = APERTURE_TEXTURE_SIZE;
	const int count = static_cast<int>(MIN(ap.dust.density, 1.0f) * 1000.0f);
	if (count <= 0) {
		return;
	}

	const float radius = ap.dust.radius * 0.1f;
	const float softness = MAX(ap.dust.softness, 1.0f / size);
	const float reach = radius + softness;

	SCP_vector<float> occlusion(static_cast<size_t>(size) * size, 0.0f);
	for (int i = 0; i < count; i++) {
		auto fi = static_cast<float>(i);
		auto fc = static_cast<float>(count);
		float cx = aperture_noise(fi, fc, 0.0f) * 2.0f - 1.0f;
		float cy = aperture_noise(fi, fc, 1.0f) * 2.0f - 1.0f;

		aperture_stamp(occlusion, cx, cy, reach, softness, [=](float px, float py) {
			return sqrtf((px - cx) * (px - cx) + (py - cy) * (py - cy)) - radius;
		});
	}

	for (size_t i = 0; i < mask.size(); i++) {
		mask[i] *= 1.0f - ap.dust.strength * occlusion[i];
	}
}

void generate_aperture(const lens_aperture& ap, lens_flare_textures* tex)
{
	const int size = APERTURE_TEXTURE_SIZE;

	tex->aperture_size = size;
	tex->aperture.resize(static_cast<size_t>(size) * size);

	SCP_vector<float> mask(static_cast<size_t>(size) * size);
	for (int y = 0; y < size; y++) {
		float py = (y + 0.5f) / size * 2.0f - 1.0f;
		for (int x = 0; x < size; x++) {
			float px = (x + 0.5f) / size * 2.0f - 1.0f;
			mask[static_cast<size_t>(y) * size + x] = aperture_shape(ap, px, py);
		}
	}

	// Imperfection layers, in realflare's order. All default to strength 0.
	if (ap.grating.strength > 0.0f) {
		aperture_apply_grating(ap, mask);
	}
	if (ap.scratches.strength > 0.0f) {
		aperture_apply_scratches(ap, mask);
	}
	if (ap.dust.strength > 0.0f) {
		aperture_apply_dust(ap, mask);
	}

	for (size_t i = 0; i < mask.size(); i++) {
		tex->aperture[i] = static_cast<ubyte>(std::clamp(mask[i], 0.0f, 1.0f) * 255.0f + 0.5f);
	}
}

void generate_starburst(const lens_flare_textures* apert, lens_flare_textures* tex)
{
	const int size = apert->aperture_size;
	tex->starburst_size = size;
	tex->starburst.resize(static_cast<size_t>(size) * size * 4);

	// Fraunhofer diffraction pattern: |FFT(aperture)|^2. The (-1)^(x+y)
	// modulation shifts the DC term to the texture center.
	SCP_vector<std::complex<float>> grid(static_cast<size_t>(size) * size);
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			float v = apert->aperture[static_cast<size_t>(y) * size + x] * (1.0f / 255.0f);
			if ((x + y) & 1) {
				v = -v;
			}
			grid[static_cast<size_t>(y) * size + x] = v;
		}
	}
	lens_flare_fft2d(grid, size, false);

	SCP_vector<float> power(static_cast<size_t>(size) * size);
	for (size_t i = 0; i < power.size(); i++) {
		power[i] = std::norm(grid[i]);
	}

	// Normalize against the brightest off-DC value so the streaks (not the
	// gigantic central spike) span the useful range
	const int c = size / 2;
	float pmax = 0.0f;
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			if (abs(x - c) <= 2 && abs(y - c) <= 2) {
				continue;
			}
			pmax = MAX(pmax, power[static_cast<size_t>(y) * size + x]);
		}
	}
	if (pmax <= 0.0f) {
		pmax = 1.0f;
	}

	auto sample_power = [&](float fx, float fy) -> float {
		fx = std::clamp(fx, 0.0f, size - 1.001f);
		fy = std::clamp(fy, 0.0f, size - 1.001f);
		int x0 = static_cast<int>(fx), y0 = static_cast<int>(fy);
		float tx = fx - x0, ty = fy - y0;
		float p00 = power[static_cast<size_t>(y0) * size + x0];
		float p10 = power[static_cast<size_t>(y0) * size + x0 + 1];
		float p01 = power[static_cast<size_t>(y0 + 1) * size + x0];
		float p11 = power[static_cast<size_t>(y0 + 1) * size + x0 + 1];
		return (p00 * (1 - tx) + p10 * tx) * (1 - ty) + (p01 * (1 - tx) + p11 * tx) * ty;
	};

	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			float* out = &tex->starburst[(static_cast<size_t>(y) * size + x) * 4];

			// Radial fade so the pattern reaches zero before the texture border
			float nx = (x - c) / static_cast<float>(c);
			float ny = (y - c) / static_cast<float>(c);
			float rn = sqrtf(nx * nx + ny * ny);
			float fade = std::clamp((1.0f - rn) / 0.15f, 0.0f, 1.0f);

			for (int k = 0; k < 3; k++) {
				// Diffraction angles scale with wavelength: resample the green
				// pattern per channel
				float scale = Wavelengths_um[1] / Wavelengths_um[k];
				float p = sample_power(c + (x - c) * scale, c + (y - c) * scale) * scale * scale;
				out[k] = sqrtf(MIN(p / pmax, 1.0f)) * fade;
			}
			out[3] = 1.0f;
		}
	}
}

} // namespace

void lens_flare_generate_textures(const lens_aperture& ap, lens_flare_textures* out)
{
	generate_aperture(ap, out);
	generate_starburst(out, out);
}

void lens_flare_fft2d(SCP_vector<std::complex<float>>& data, int size, bool inverse)
{
	Assertion((size & (size - 1)) == 0, "FFT size must be a power of two, got %d", size);
	Assertion(static_cast<int>(data.size()) == size * size, "FFT data size mismatch");

	auto fft_1d = [&](std::complex<float>* base, int stride) {
		// bit-reversal permutation
		for (int i = 1, j = 0; i < size; i++) {
			int bit = size >> 1;
			for (; j & bit; bit >>= 1) {
				j ^= bit;
			}
			j ^= bit;
			if (i < j) {
				std::swap(base[static_cast<size_t>(i) * stride], base[static_cast<size_t>(j) * stride]);
			}
		}
		for (int len = 2; len <= size; len <<= 1) {
			float ang = 2.0f * PI / len * (inverse ? 1.0f : -1.0f);
			std::complex<float> wlen(cosf(ang), sinf(ang));
			for (int i = 0; i < size; i += len) {
				std::complex<float> w(1.0f, 0.0f);
				for (int k = 0; k < len / 2; k++) {
					auto& lhs = base[static_cast<size_t>(i + k) * stride];
					auto& rhs = base[static_cast<size_t>(i + k + len / 2) * stride];
					std::complex<float> u = lhs;
					std::complex<float> v = rhs * w;
					lhs = u + v;
					rhs = u - v;
					w *= wlen;
				}
			}
		}
		if (inverse) {
			for (int i = 0; i < size; i++) {
				base[static_cast<size_t>(i) * stride] /= static_cast<float>(size);
			}
		}
	};

	for (int row = 0; row < size; row++) {
		fft_1d(&data[static_cast<size_t>(row) * size], 1);
	}
	for (int col = 0; col < size; col++) {
		fft_1d(&data[col], size);
	}
}

void lens_flare_generate_aperture_mask(const lens_aperture& ap, lens_flare_textures* out)
{
	generate_aperture(ap, out);
}

} // namespace graphics
