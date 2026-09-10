#include "starfield/sun_disc.h"

#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "ddsutils/ddsutils.h"
#include "math/floating.h"
#include "starfield/starfield.h"

#include <cmath>

// The sun quad is drawn with rad = 0.05f * scale_x (see stars_draw_sun()), and
// g3_render_rect_screen_aligned_2d() sizes it so that rad is exactly the tangent of the
// half-angle it subtends, independent of FOV, resolution and aspect ratio.
static const float Sun_quad_tan_half_angle = 0.05f;

// How much of that drawn size to believe. The drawn sun is an art decision rather than a
// physical one: measured across the sun art of retail, the MediaVPs and several mods, the
// emitting disc subtends a median of ~3.9x Sol's 0.53 degrees once the mission's +Scale: is
// applied, and up to 26x in the worst retail mission. Taken literally that gives shadow edges
// tens of metres wide. This scales the measurement so that median MediaVPs content -- what
// nearly all modern mods ship -- lands near Sol; installs running retail's chunkier sun art
// land about twice as soft.
static const float Sun_derived_size_calibration = 0.25f;

// Ceiling on a derived sun's apparent diameter, in degrees. Missions scale their suns freely
// (retail's own missions go up to +Scale: 5.0), and penumbra width also decides how many rays
// are needed to resolve it without grain -- shadows_rt_sample_count() caps at
// RT_SHADOW_MAX_SAMPLES.
static const float Sun_derived_max_diameter = 1.5f;

float sun_disc_tangent_from_diameter(float degrees)
{
	return tanf(fl_radians(degrees) * 0.5f);
}

float sun_disc_tangent_from_fraction(float disc_fraction, float scale_x)
{
	if (disc_fraction <= 0.0f) {
		return 0.0f;
	}

	return MIN(Sun_quad_tan_half_angle * scale_x * disc_fraction * Sun_derived_size_calibration,
		sun_disc_tangent_from_diameter(Sun_derived_max_diameter));
}

void sun_disc_build_weights(const ubyte* pixels, int width, int height, int bytes_per_pixel, bool has_alpha,
	SCP_vector<ubyte>& out_weights)
{
	Assertion(pixels != nullptr, "sun_disc_build_weights() called with no pixel data!");
	Assertion(bytes_per_pixel == 3 || bytes_per_pixel == 4, "sun_disc_build_weights() only handles BGR and BGRA, got %d bytes per pixel!", bytes_per_pixel);

	if (width <= 0 || height <= 0) {
		out_weights.clear();
		return;
	}

	const size_t num_pixels = static_cast<size_t>(width) * static_cast<size_t>(height);
	out_weights.resize(num_pixels);

	const bool use_alpha = has_alpha && (bytes_per_pixel == 4);

	for (size_t i = 0; i < num_pixels; i++) {
		const ubyte* px = pixels + (i * static_cast<size_t>(bytes_per_pixel));

		// channel order doesn't matter to the maximum, so this works for both BGR and RGB
		ubyte weight = MAX(px[0], MAX(px[1], px[2]));

		if (use_alpha) {
			weight = static_cast<ubyte>((static_cast<int>(weight) * static_cast<int>(px[3])) / 255);
		}

		out_weights[i] = weight;
	}
}

void sun_disc_build_weights_16(const ubyte* pixels, int width, int height, bool has_alpha,
	SCP_vector<ubyte>& out_weights)
{
	Assertion(pixels != nullptr, "sun_disc_build_weights_16() called with no pixel data!");

	if (width <= 0 || height <= 0) {
		out_weights.clear();
		return;
	}

	const size_t num_pixels = static_cast<size_t>(width) * static_cast<size_t>(height);
	out_weights.resize(num_pixels);

	// bm_get_components() reads through whichever format bmpman currently has selected, and
	// bm_load_image_data() leaves that on the screen format, so point it at the texture format
	// for the duration -- the same select/restore bmpman does around its own loaders
	BM_SELECT_TEX_FORMAT();

	for (size_t i = 0; i < num_pixels; i++) {
		ubyte r = 0;
		ubyte g = 0;
		ubyte b = 0;
		ubyte a = 1;

		bm_get_components(const_cast<ubyte*>(pixels + (i * 2)), &r, &g, &b, has_alpha ? &a : nullptr);

		// the texture format only carries one bit of alpha (Gr_t_alpha), and bm_get_components()
		// hands it back as 0 or 1 rather than 0-255, so it masks rather than scales. The colour
		// channels are 5 bits each, so weights off this path are quantized to 1/31 -- coarser
		// than the 8-bit paths, but far finer than the measurement needs.
		out_weights[i] = (has_alpha && (a == 0)) ? static_cast<ubyte>(0) : MAX(r, MAX(g, b));
	}

	BM_SELECT_SCREEN_FORMAT();
}

float sun_disc_fraction_from_weights(const ubyte* weights, int width, int height)
{
	if ((weights == nullptr) || (width <= 0) || (height <= 0)) {
		return 0.0f;
	}

	const size_t num_pixels = static_cast<size_t>(width) * static_cast<size_t>(height);

	ubyte peak = 0;
	for (size_t i = 0; i < num_pixels; i++) {
		if (weights[i] > peak) {
			peak = weights[i];
		}
	}

	// blank or near-blank art has no disc to measure
	if (peak < SUN_DISC_MIN_PEAK_WEIGHT) {
		return 0.0f;
	}

	const auto cutoff = static_cast<ubyte>(std::ceil(SUN_DISC_THRESHOLD * static_cast<float>(peak)));

	size_t disc_pixels = 0;
	for (size_t i = 0; i < num_pixels; i++) {
		if (weights[i] >= cutoff) {
			disc_pixels++;
		}
	}

	if (disc_pixels == 0) {
		return 0.0f;
	}

	const float radius = fl_sqrt(static_cast<float>(disc_pixels) / PI);
	const float half_edge = 0.5f * static_cast<float>(MIN(width, height));

	// a bitmap lit all the way into its corners measures slightly over 1; the disc still
	// can't be bigger than the quad it is drawn on
	return MIN(radius / half_edge, 1.0f);
}

float sun_disc_measure_bitmap(int bitmap_handle)
{
	if (bitmap_handle < 0) {
		return -1.0f;
	}

	const bool has_alpha = bm_has_alpha_channel(bitmap_handle);

	int width = 0;
	int height = 0;
	SCP_vector<ubyte> weights;

	if (bm_is_compressed(bitmap_handle)) {
		// bm_lock() hands back block-compressed data as-is whenever the renderer supports
		// S3TC/BPTC (see bm_lock_dds()), so this path has to go around bmpman and decompress
		// the file itself. Sun art really does ship compressed, so it isn't an edge case.
		const char* filename = bm_get_filename(bitmap_handle);

		if ((filename == nullptr) || (*filename == '\0')) {
			return -1.0f;
		}

		SCP_vector<ubyte> pixels;
		if (dds_decompress_top_mip_bgra(filename, CF_TYPE_ANY, &width, &height, pixels) != DDS_ERROR_NONE) {
			return -1.0f;
		}

		sun_disc_build_weights(pixels.data(), width, height, 4, has_alpha, weights);
	} else {
		bitmap* bmp = bm_lock(bitmap_handle, 32, BMP_TEX_XPARENT);

		if (bmp == nullptr) {
			return -1.0f;
		}

		if (bmp->data == 0) {
			// bm_lock() took a reference before it failed to populate the data
			bm_unlock(bitmap_handle);
			return -1.0f;
		}

		// The requested bpp is a request, not a guarantee: bm_lock() ignores it for JPGs (always
		// 24-bit BGR) and uncompressed DDS (whatever the file uses), and for anything already
		// paged in it returns the data in the format page-in chose -- 16-bit for textures, see
		// bm_page_in_stop(). Handle each layout rather than assuming the 32 we asked for.
		width = bmp->w;
		height = bmp->h;

		const auto* pixels = reinterpret_cast<const ubyte*>(bmp->data);

		if (bmp->bpp == 32) {
			sun_disc_build_weights(pixels, width, height, 4, has_alpha, weights);
		} else if (bmp->bpp == 24) {
			sun_disc_build_weights(pixels, width, height, 3, false, weights);
		} else if (bmp->bpp == 16) {
			sun_disc_build_weights_16(pixels, width, height, has_alpha, weights);
		} else {
			bm_unlock(bitmap_handle);
			return -1.0f;
		}

		bm_unlock(bitmap_handle);
	}

	if (weights.empty()) {
		return -1.0f;
	}

	return sun_disc_fraction_from_weights(weights.data(), width, height);
}
