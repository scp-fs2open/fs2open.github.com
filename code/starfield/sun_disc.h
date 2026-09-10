#pragma once

#include "globalincs/pstypes.h"

// Measures how much of a sun bitmap is the emitting disc, as opposed to the glow/corona
// painted around it. This is how a sun's apparent angular size is worked out when its table
// entry doesn't give one outright, which is the usual case -- see sun_angular_radius_tangent()
// in starfield.cpp for how the measurement becomes a shadow penumbra, and $SunAngularSize: in
// stars.tbl for how a table opts out of it.

// Pixels at or above this fraction of the brightest pixel count as part of the disc.
// The measurement is only weakly sensitive to it -- across the shipped sun art of retail,
// the MediaVPs and several mods, moving this to 0.5 rescales every bitmap by ~1.37x while
// preserving their order almost exactly (Spearman rho 0.96) -- so it behaves as a constant
// factor that the calibration in starfield.cpp absorbs, not as a per-bitmap judgement call.
constexpr float SUN_DISC_THRESHOLD = 0.9f;

// Sun bitmaps whose brightest pixel is dimmer than this are treated as having no disc at
// all. Mods ship deliberately blank sun bitmaps (BtA's SunAntares*-BLANK, Blue Planet's
// SunSolDummy) to get a light source without a visible sun, and without this floor the
// threshold above would collapse to zero, match every pixel, and hand a sun that isn't
// drawn at all the widest penumbra in the game.
constexpr ubyte SUN_DISC_MIN_PEAK_WEIGHT = 8;

/**
 * @brief Reduces BGR(A) pixels to the per-pixel weight the disc measurement works on
 *
 * The weight is how much the texel actually contributes where the sun quad is drawn, which
 * depends on how it is blended: sun bitmaps without an alpha channel are drawn additively
 * and bitmaps with one are alpha blended (see material_determine_blend_mode()).
 *
 * @param pixels           BGR or BGRA pixel data, @a width * @a height * @a bytes_per_pixel bytes
 * @param bytes_per_pixel  3 (BGR) or 4 (BGRA)
 * @param has_alpha        whether the alpha channel is meaningful; ignored unless BGRA
 * @param[out] out_weights resized to @a width * @a height
 */
void sun_disc_build_weights(const ubyte* pixels, int width, int height, int bytes_per_pixel, bool has_alpha,
	SCP_vector<ubyte>& out_weights);

/**
 * @brief sun_disc_build_weights() for 16-bit texture-format pixels
 *
 * Bitmaps that went through bm_page_in_texture() are locked at 16 bpp (see bm_page_in_stop()),
 * so anything but a compressed DDS is likely to be in the renderer's packed 16-bit texture
 * format by the time a sun is drawn -- retail's PCX sun art included.
 */
void sun_disc_build_weights_16(const ubyte* pixels, int width, int height, bool has_alpha,
	SCP_vector<ubyte>& out_weights);

/**
 * @brief Radius of the emitting disc, as a fraction of the drawn quad's half-width
 *
 * Coverage based: the disc's area is taken as the number of pixels at or above
 * SUN_DISC_THRESHOLD of the brightest one, converted to the radius of a circle of that
 * area. That survives off-centre discs, baked-in flare spikes and the halo tail, all of
 * which a luminance-weighted radius would be dominated by.
 *
 * Normalized against the shorter edge, because that is the edge
 * g3_render_rect_screen_aligned_2d() fits the quad to.
 *
 * @return the disc radius in [0, 1], or 0 if the bitmap has no measurable disc
 */
float sun_disc_fraction_from_weights(const ubyte* weights, int width, int height);

/**
 * @brief Tangent of the angular radius for an apparent diameter in degrees
 *
 * This is the form a directional light's source_radius takes -- see traceShadowRayCone() in
 * shadows.sdr, where it sizes the shadow penumbra. 0 in, 0 out, i.e. hard shadows.
 */
float sun_disc_tangent_from_diameter(float degrees);

/**
 * @brief Same tangent, for a sun whose size is measured from its bitmap rather than given
 *
 * Turns a disc fraction into an angular size using the geometry of the drawn sun quad, then
 * applies the calibration and ceiling that keep the result usable (see the constants in
 * sun_disc.cpp for why a measured sun can't be believed literally).
 *
 * @param scale_x the mission's +Scale: for this sun instance
 * @return the tangent, or 0 for a sun with no measurable disc
 */
float sun_disc_tangent_from_fraction(float disc_fraction, float scale_x);

/**
 * @brief Measures the disc fraction of a loaded sun bitmap
 *
 * Reads the bitmap's pixels on the CPU, so this is a one-shot operation -- callers must
 * cache the result rather than calling it per frame. For animated sun bitmaps this measures
 * the first frame, which is the handle bm_load_animation() returns.
 *
 * @return the disc fraction, or a negative value if the bitmap could not be read
 */
float sun_disc_measure_bitmap(int bitmap_handle);
