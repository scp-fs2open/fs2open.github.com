/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/



#include <algorithm>
#include <cmath>

#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "def_files/def_files.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "graphics/material.h"
#include "graphics/matrix.h"
#include "math/vecmat.h"
#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "render/3d.h"
#include "starfield/nebula.h"

// distance the background nebula sphere sits at; large so it reads as a faraway backdrop
#define NEBULA_RADIUS 1000.0f

// ----------------------------------------------------------------------------------------------------
// data-driven pattern / color registries
// ----------------------------------------------------------------------------------------------------

SCP_vector<generated_nebula_pattern> Generated_nebula_patterns;
SCP_vector<generated_nebula_color>   Generated_nebula_colors;

int generated_nebula_pattern_lookup(const char *name)
{
	for (int i = 0; i < static_cast<int>(Generated_nebula_patterns.size()); i++) {
		if (!stricmp(Generated_nebula_patterns[i].name.c_str(), name))
			return i;
	}
	return -1;
}

int generated_nebula_color_lookup(const char *name)
{
	for (int i = 0; i < static_cast<int>(Generated_nebula_colors.size()); i++) {
		if (!stricmp(Generated_nebula_colors[i].name.c_str(), name))
			return i;
	}
	return -1;
}

const char *generated_nebula_pattern_name(int index)
{
	if (index < 0 || index >= static_cast<int>(Generated_nebula_patterns.size()))
		return "";
	return Generated_nebula_patterns[index].name.c_str();
}

int generated_nebula_default_color()
{
	const int blue = generated_nebula_color_lookup("Blue");
	if (blue >= 0)
		return blue;
	return Generated_nebula_colors.empty() ? -1 : 0;
}

const char *generated_nebula_color_name(int index)
{
	if (index < 0 || index >= static_cast<int>(Generated_nebula_colors.size()))
		return "";
	return Generated_nebula_colors[index].name.c_str();
}

namespace {

constexpr const char *GENERATED_NEBULA_TABLE = "generated_nebula.tbl";
constexpr const char *GENERATED_NEBULA_MODULAR_GLOB = "*-gneb.tbm";

void parse_generated_nebula_colors()
{
	while (optional_string("$Name:")) {
		SCP_string nm;
		stuff_string(nm, F_NAME);

		// a name already defined (by the base table or an earlier .tbm) is overridden in place
		int idx = generated_nebula_color_lookup(nm.c_str());
		generated_nebula_color *c;
		if (idx < 0) {
			Generated_nebula_colors.emplace_back();
			c = &Generated_nebula_colors.back();
			c->name = nm;
		} else {
			c = &Generated_nebula_colors[idx];
		}

		if (optional_string("+RGB:")) {
			int rgb[3];
			parse_int_list(rgb, 3);
			CLAMP(rgb[0], 0, 255);
			CLAMP(rgb[1], 0, 255);
			CLAMP(rgb[2], 0, 255);
			c->r = static_cast<ubyte>(rgb[0]);
			c->g = static_cast<ubyte>(rgb[1]);
			c->b = static_cast<ubyte>(rgb[2]);
		}
	}
}

void parse_generated_nebula_patterns()
{
	while (optional_string("$Name:")) {
		SCP_string nm;
		stuff_string(nm, F_NAME);

		// a name already defined (by the base table or an earlier .tbm) is overridden in place
		int idx = generated_nebula_pattern_lookup(nm.c_str());
		generated_nebula_pattern *p;
		if (idx < 0) {
			Generated_nebula_patterns.emplace_back();
			p = &Generated_nebula_patterns.back();
			p->name = nm;
		} else {
			p = &Generated_nebula_patterns[idx];
		}

		if (optional_string("+Clouds Only:"))
			stuff_boolean(&p->clouds_only);
		if (optional_string("+Cloud Detail:")) {
			stuff_float(&p->cloud_detail);
			CLAMP(p->cloud_detail, 0.0f, 1.0f);
		}
		// any +Cloud entries replace the pattern's whole cloud list
		bool first_cloud = true;
		while (optional_string("+Cloud:")) {
			float c[6];
			parse_float_list(c, 6);
			if (first_cloud) {
				p->clouds.clear();
				first_cloud = false;
			}
			generated_nebula_cloud cloud;
			cloud.lon = c[0];
			cloud.lat = c[1];
			CLAMP(cloud.lat, -90.0f, 90.0f);
			cloud.width = std::max(c[2], 0.1f);
			cloud.height = std::max(c[3], 0.1f);
			cloud.angle = c[4];
			cloud.brightness = std::max(c[5], 0.0f);
			p->clouds.push_back(cloud);
		}
		if (optional_string("+Density:"))
			stuff_float(&p->density);
		if (optional_string("+Frequency:")) {
			float f[2];
			parse_float_list(f, 2);
			p->freq_u = f[0];
			p->freq_v = f[1];
			CLAMP(p->freq_u, 0.1f, 64.0f);
			CLAMP(p->freq_v, 0.1f, 64.0f);
		}
		if (optional_string("+Octaves:")) {
			stuff_int(&p->octaves);
			CLAMP(p->octaves, 1, 8);
		}
		if (optional_string("+Warp:")) {
			stuff_float(&p->warp);
			CLAMP(p->warp, 0.0f, 8.0f);
		}
		if (optional_string("+Contrast:")) {
			stuff_float(&p->contrast);
			CLAMP(p->contrast, 0.05f, 10.0f);
		}
		if (optional_string("+Intensity:")) {
			stuff_float(&p->intensity);
			p->intensity = std::max(p->intensity, 0.0f);
		}
		if (optional_string("+Seed:")) {
			stuff_int(&p->seed);
			CLAMP(p->seed, -1000000, 1000000);
		}
		if (optional_string("+Resolution:")) {
			int r[2];
			parse_int_list(r, 2);
			p->res_lon = r[0];
			p->res_lat = r[1];
		}
		if (optional_string("+Band:")) {
			float b[2];
			parse_float_list(b, 2);
			p->band_min = b[0];
			p->band_max = b[1];
		}
	}
}

// generated_nebula.tbl or a *-gneb.tbm (filename), or the built-in default (nullptr).  Both
// sections are optional, so a .tbm can carry just colors or just patterns.
void parse_generated_nebula_table(const char *filename)
{
	try {
		if (filename != nullptr)
			read_file_text(filename, CF_TYPE_TABLES);
		else
			read_file_text_from_default(defaults_get_file(GENERATED_NEBULA_TABLE));
		reset_parse();

		// either section may come first, and either may be left out
		while (true) {
			if (optional_string("#Generated Nebula Colors")) {
				parse_generated_nebula_colors();
				required_string("#End");
			} else if (optional_string("#Generated Nebula Patterns")) {
				parse_generated_nebula_patterns();
				required_string("#End");
			} else {
				break;
			}
		}
		if (!check_for_eof())
			error_display(0, "Unexpected text in %s; expected #Generated Nebula Colors or #Generated Nebula Patterns.",
				filename != nullptr ? filename : GENERATED_NEBULA_TABLE);
	} catch (const parse::ParseException &e) {
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n",
			filename != nullptr ? filename : GENERATED_NEBULA_TABLE, e.what()));
	}
}

} // namespace

void generated_nebula_init()
{
	Generated_nebula_patterns.clear();
	Generated_nebula_colors.clear();

	// The base table holds the built-in FS1 patterns and colors (a mod's own generated_nebula.tbl
	// replaces it), then *-gneb.tbm files add to it and override entries by name
	if (cf_exists_full(GENERATED_NEBULA_TABLE, CF_TYPE_TABLES))
		parse_generated_nebula_table(GENERATED_NEBULA_TABLE);
	else
		parse_generated_nebula_table(nullptr);

	parse_modular_table(GENERATED_NEBULA_MODULAR_GLOB, parse_generated_nebula_table);
}

// ----------------------------------------------------------------------------------------------------
// procedural mesh generation + rendering
// ----------------------------------------------------------------------------------------------------

static vertex *Nebula_verts = nullptr;
static int Nebula_n_verts = 0;

// baked equirectangular brightness texture (procedural path).  Nebula_tex_data is our own buffer;
// bm_create() keeps the pointer without copying, so we hold it until bm_release + delete[].
static ubyte *Nebula_tex_data = nullptr;
static int Nebula_bitmap = -1;
// the pattern and color index the texture was baked from, so an orientation change can keep it
static int Nebula_baked_pattern = -1;
static int Nebula_baked_color = -1;

static matrix Nebula_orient;

int Nebula_pitch;
int Nebula_bank;
int Nebula_heading;

bool generated_nebula_enabled = false;
SCP_string Nebula_unknown_pattern;
SCP_string Nebula_unknown_color;

void nebula_close()
{
	delete[] Nebula_verts;
	Nebula_verts = nullptr;
	Nebula_n_verts = 0;

	if (Nebula_bitmap >= 0) {
		bm_release(Nebula_bitmap);
		Nebula_bitmap = -1;
	}
	delete[] Nebula_tex_data;
	Nebula_tex_data = nullptr;
	Nebula_baked_pattern = -1;
	Nebula_baked_color = -1;
}

// classic 2d -> sphere projection.  u,v in range 0..1.
static void project_2d_onto_sphere(vec3d *pnt, float u, float v)
{
	float a, x, y, z, s;

	a = PI * (2.0f * u - 1.0f);
	z = 2.0f * v - 1.0f;
	s = fl_sqrt(1.0f - z * z);
	x = s * cosf(a);
	y = s * sinf(a);
	pnt->xyz.x = x;
	pnt->xyz.y = y;
	pnt->xyz.z = z;
}

// --- small, deterministic value-noise field (no general noise utility exists in the engine) ---

static uint32_t neb_hash(uint32_t x)
{
	x ^= x >> 16;
	x *= 0x7feb352dU;
	x ^= x >> 15;
	x *= 0x846ca68bU;
	x ^= x >> 16;
	return x;
}

static float neb_hash2(int ix, int iy, int seed)
{
	uint32_t h = neb_hash((static_cast<uint32_t>(ix) * 73856093U) ^ (static_cast<uint32_t>(iy) * 19349663U) ^
						   (static_cast<uint32_t>(seed) * 83492791U));
	return (h & 0xffffffU) / static_cast<float>(0x1000000);
}

static float neb_smooth(float t)
{
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

// value noise that is periodic in x with period xperiod (so the longitude seam matches)
static float neb_vnoise(float x, float y, int seed, int xperiod)
{
	int ix = static_cast<int>(floorf(x));
	int iy = static_cast<int>(floorf(y));
	float fx = x - ix;
	float fy = y - iy;

	auto wrap = [xperiod](int i) {
		if (xperiod > 0) {
			i %= xperiod;
			if (i < 0)
				i += xperiod;
		}
		return i;
	};

	float h00 = neb_hash2(wrap(ix), iy, seed);
	float h10 = neb_hash2(wrap(ix + 1), iy, seed);
	float h01 = neb_hash2(wrap(ix), iy + 1, seed);
	float h11 = neb_hash2(wrap(ix + 1), iy + 1, seed);

	float u = neb_smooth(fx);
	float v = neb_smooth(fy);

	return (((h00 * (1.0f - u)) + (h10 * u)) * (1.0f - v)) + (((h01 * (1.0f - u)) + (h11 * u)) * v);
}

// multi-octave, anisotropic, longitude-seamless fbm in 0..1
static float neb_fbm(float lon, float lat, const generated_nebula_pattern &p)
{
	const int base = std::max(static_cast<int>(std::lround(p.freq_u)), 1);

	// low-frequency directional warp to stretch the wisps
	float w = (neb_vnoise(lon * base, lat * p.freq_v, p.seed + 777, base) - 0.5f) * p.warp;

	int octaves = p.octaves;
	CLAMP(octaves, 1, 8);

	float sum = 0.0f, amp = 0.5f, norm = 0.0f;
	for (int o = 0; o < octaves; o++) {
		int per = base << o;
		float n = neb_vnoise((lon + w) * per, lat * p.freq_v * static_cast<float>(1 << o), p.seed + o, per);
		sum += amp * n;
		norm += amp;
		amp *= 0.5f;
	}

	return (norm > 0.0f) ? (sum / norm) : 0.0f;
}

// fraction of latitude near each pole over which the pattern fades to black.  The (lon,lat) noise
// is singular at the poles (every longitude collapses to one point), so we fade the brightness out
// before it gets there -- that hides the UV-sphere convergence "starburst" and keeps the closed
// pole caps black-on-black against space, instead of trying to render coherent detail at the seam.
#define NEBULA_POLE_FADE 0.15f

// brightness 0..1 of the procedural background for a texel: mostly black, a soft scatter of
// bright cloud masses
static float nebula_background_brightness(float n, float lat, const generated_nebula_pattern &p)
{
	float density = p.density;
	CLAMP(density, 0.0f, 1.0f);
	if (density <= 0.0f)
		return 0.0f;

	float thr = 1.0f - density;

	// soft (feathered) ramp up from the threshold instead of a hard cutoff -- this, sampled
	// per-texel into the baked texture, is what gives the smooth FS1 clouds instead of hard edges
	float t = (n - thr) / (1.0f - thr);
	CLAMP(t, 0.0f, 1.0f);
	float b = neb_smooth(t);
	b = powf(b, p.contrast);

	// smoothly fade to black approaching either pole
	float edge = std::min(lat, 1.0f - lat);
	if (edge < NEBULA_POLE_FADE)
		b *= neb_smooth(edge / NEBULA_POLE_FADE);

	return b;
}

// A cloud ready to evaluate on the flat map: x is longitude in degrees (0..360, wrapping), y runs
// 0..180 along the map's other axis (y = v * 180, where v is the texture's 0..1 coordinate)
struct nebula_cloud_eval {
	float x, y;
	float cos_a, sin_a;
	float inv_wx2, inv_wy2;   // Gaussian falloff factors along the cloud's own axes
	float brightness;
	float reach;              // beyond this distance on either axis the cloud adds nothing visible
};

static SCP_vector<nebula_cloud_eval> nebula_prepare_clouds(const generated_nebula_pattern &p)
{
	SCP_vector<nebula_cloud_eval> out;
	out.reserve(p.clouds.size());
	// width/height are full widths at half brightness: sigma = width / (2 * sqrt(2 * ln 2))
	constexpr float fwhm_to_sigma = 1.0f / 2.3548f;
	for (const auto &c : p.clouds) {
		nebula_cloud_eval e;
		e.x = fmodf(c.lon, 360.0f);
		if (e.x < 0.0f)
			e.x += 360.0f;
		// the map's other axis is linear in sin(latitude), like the FS1 meshes
		e.y = (sinf(fl_radians(c.lat)) + 1.0f) * 90.0f;
		const float a = fl_radians(c.angle);
		e.cos_a = cosf(a);
		e.sin_a = sinf(a);
		const float sx = c.width * fwhm_to_sigma;
		const float sy = c.height * fwhm_to_sigma;
		e.inv_wx2 = 0.5f / (sx * sx);
		e.inv_wy2 = 0.5f / (sy * sy);
		e.brightness = c.brightness;
		e.reach = 3.5f * std::max(sx, sy);
		out.push_back(e);
	}
	return out;
}

// brightness 0..1 of the whole pattern for a texel: the procedural background (unless clouds
// only) screened with the placed clouds, which the noise breaks up per +Cloud Detail
static float nebula_brightness(float lon, float lat, const generated_nebula_pattern &p,
	const SCP_vector<nebula_cloud_eval> &clouds)
{
	// the noise is shared by the background and the cloud breakup, so compute it at most once
	float noise = -1.0f;
	auto get_noise = [&]() {
		if (noise < 0.0f)
			noise = neb_fbm(lon, lat, p);
		return noise;
	};

	const float bg = p.clouds_only ? 0.0f : nebula_background_brightness(get_noise(), lat, p);
	if (clouds.empty())
		return bg;

	const float x = lon * 360.0f;
	const float y = lat * 180.0f;
	float c = 0.0f;
	for (const auto &e : clouds) {
		const float dy = y - e.y;
		if (fabsf(dy) > e.reach)
			continue;
		float dx = x - e.x;
		if (dx > 180.0f)
			dx -= 360.0f;
		else if (dx < -180.0f)
			dx += 360.0f;
		if (fabsf(dx) > e.reach)
			continue;
		const float xr = dx * e.cos_a + dy * e.sin_a;
		const float yr = -dx * e.sin_a + dy * e.cos_a;
		c += e.brightness * expf(-(xr * xr * e.inv_wx2 + yr * yr * e.inv_wy2));
	}

	if (c > 0.0f && p.cloud_detail > 0.0f) {
		// the noise averages about 0.5, so this keeps the clouds' overall brightness while
		// carving them into brighter and darker wisps
		c *= std::max(0.0f, 1.0f + (p.cloud_detail * ((2.0f * get_noise()) - 1.0f) * 2.0f));
	}
	c = std::min(c, 1.0f);

	return 1.0f - (1.0f - bg) * (1.0f - c);
}

// dimensions of the baked equirectangular brightness texture
constexpr int NEBULA_TEX_W = 1024;
constexpr int NEBULA_TEX_H = 512;

static ubyte nebula_chan(float c, float b)
{
	int v = static_cast<int>(c * b);
	CLAMP(v, 0, 255);
	return static_cast<ubyte>(v);
}

// bake the procedural pattern into an equirectangular RGB texture, tinted by the mission color.
// sampling per-texel here (rather than per-vertex on a coarse mesh) is what removes the triangle
// facets; the smooth brightness field + bilinear filtering give the soft, feathered FS1 look.
static void nebula_bake_texture(const generated_nebula_pattern &p, const generated_nebula_color &col)
{
	// nebula_init() closed the previous texture before baking, so these start fresh
	Nebula_tex_data = new ubyte[NEBULA_TEX_W * NEBULA_TEX_H * 3];

	const float intensity = std::max(p.intensity, 0.0f);
	const auto clouds = nebula_prepare_clouds(p);

	ubyte *px = Nebula_tex_data;
	for (int j = 0; j < NEBULA_TEX_H; j++) {
		float v = (static_cast<float>(j) + 0.5f) / static_cast<float>(NEBULA_TEX_H); // latitude 0..1
		for (int i = 0; i < NEBULA_TEX_W; i++) {
			float u = (static_cast<float>(i) + 0.5f) / static_cast<float>(NEBULA_TEX_W); // longitude 0..1
			float b = nebula_brightness(u, v, p, clouds) * intensity;
			// 24-bit user bitmaps upload as GL_BGR, so store blue-green-red
			*px++ = nebula_chan(col.b, b);
			*px++ = nebula_chan(col.g, b);
			*px++ = nebula_chan(col.r, b);
		}
	}

	// 24-bit (no alpha) so material_set_unlit() picks additive blending
	Nebula_bitmap = bm_create(24, NEBULA_TEX_W, NEBULA_TEX_H, Nebula_tex_data, 0);
}

// build the background sphere (positions + equirectangular UVs) into Nebula_verts.  brightness and
// color live in the baked texture now, so the geometry only needs to be a smooth-enough sphere.
static void nebula_generate_sphere(const generated_nebula_pattern &p, const matrix *orient)
{
	delete[] Nebula_verts;
	Nebula_verts = nullptr;
	Nebula_n_verts = 0;

	int rlon = p.res_lon;
	int rlat = p.res_lat;
	CLAMP(rlon, 2, 200);
	CLAMP(rlat, 1, 100);

	float band_min = p.band_min;
	float band_max = p.band_max;
	CLAMP(band_min, 0.0f, 1.0f);
	CLAMP(band_max, 0.0f, 1.0f);
	if (band_max < band_min)
		std::swap(band_max, band_min);

	int n_grid = (rlon + 1) * (rlat + 1);
	SCP_vector<vec3d> gpt(n_grid);
	SCP_vector<uv_pair> guv(n_grid);

	for (int i = 0; i <= rlon; i++) {
		float u = static_cast<float>(i) / static_cast<float>(rlon); // 0..1 longitude
		for (int j = 0; j <= rlat; j++) {
			float t = static_cast<float>(j) / static_cast<float>(rlat);
			float v = band_min + (band_max - band_min) * t; // sphere v inside the band

			vec3d pt;
			project_2d_onto_sphere(&pt, u, v);
			vm_vec_scale(&pt, NEBULA_RADIUS);
			vm_vec_unrotate(&pt, &pt, orient);

			int gi = i * (rlat + 1) + j;
			gpt[gi] = pt;
			guv[gi].u = u;
			guv[gi].v = v;
		}
	}

	Nebula_n_verts = rlon * rlat * 6;
	Nebula_verts = new vertex[Nebula_n_verts];

	auto set_vert = [&](vertex *vt, int gi) {
		g3_transfer_vertex(vt, &gpt[gi]);
		vt->texture_position = guv[gi];
		vt->r = vt->g = vt->b = vt->a = 255;
	};

	int k = 0;
	for (int i = 0; i < rlon; i++) {
		for (int j = 0; j < rlat; j++) {
			int g00 = i * (rlat + 1) + j;
			int g10 = (i + 1) * (rlat + 1) + j;
			int g11 = (i + 1) * (rlat + 1) + (j + 1);
			int g01 = i * (rlat + 1) + (j + 1);

			set_vert(&Nebula_verts[k++], g00);
			set_vert(&Nebula_verts[k++], g10);
			set_vert(&Nebula_verts[k++], g11);

			set_vert(&Nebula_verts[k++], g00);
			set_vert(&Nebula_verts[k++], g11);
			set_vert(&Nebula_verts[k++], g01);
		}
	}

	Assertion(k == Nebula_n_verts, "generated nebula sphere vertex count mismatch (%d != %d)", k, Nebula_n_verts);
}

void nebula_init(int index, int pitch, int bank, int heading)
{
	const bool valid = generated_nebula_enabled && index >= 0 && index < static_cast<int>(Generated_nebula_patterns.size()) &&
		!Is_standalone;

	// Only the sphere depends on the orientation, so a change of pitch, bank or heading alone (the
	// editor's spinboxes, or undoing one) keeps the baked texture instead of baking it again.
	const bool same_texture = valid && Nebula_bitmap >= 0 && Nebula_baked_pattern == index &&
		Nebula_baked_color == Mission_palette;
	if (same_texture) {
		delete[] Nebula_verts;
		Nebula_verts = nullptr;
		Nebula_n_verts = 0;
	} else {
		nebula_close();
	}

	angles pbh;
	pbh.p = fl_radians(pitch);
	pbh.b = fl_radians(bank);
	pbh.h = fl_radians(heading);
	vm_angles_2_matrix(&Nebula_orient, &pbh);

	if (!valid)
		return;

	if (!same_texture) {
		// pick the tint color (fall back to white if the palette index is out of range)
		generated_nebula_color col;
		if (Mission_palette >= 0 && Mission_palette < static_cast<int>(Generated_nebula_colors.size()))
			col = Generated_nebula_colors[Mission_palette];

		nebula_bake_texture(Generated_nebula_patterns[index], col);
		Nebula_baked_pattern = index;
		Nebula_baked_color = Mission_palette;
	}

	nebula_generate_sphere(Generated_nebula_patterns[index], &Nebula_orient);
}

void nebula_render()
{
	if (Nebula_verts == nullptr || Nebula_n_verts <= 0 || Nebula_bitmap < 0)
		return;

	// additive textured sphere (no depth), instanced around the eye
	material mat;
	material_set_unlit(&mat, Nebula_bitmap, 1.0f, true, false);

	gr_start_instance_matrix(&Eye_position, &vmd_identity_matrix);
	g3_render_primitives_textured(&mat, Nebula_verts, Nebula_n_verts, PRIM_TYPE_TRIS, false);
	gr_end_instance_matrix();
}
