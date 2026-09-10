
#include <gtest/gtest.h>

#include <starfield/sun_disc.h>

#include <cmath>
#include <vector>

namespace {

// Builds a BGRA sun-like bitmap: a saturated disc of radius disc_radius, wrapped in a
// gaussian halo of the given peak brightness (0 for no halo at all). This is the shape the
// real art has -- a saturated plateau with a falloff -- and the halo is exactly what the
// measurement is supposed to ignore.
std::vector<ubyte> make_sun(int width, int height, float disc_radius, float halo_peak, float halo_sigma,
	ubyte alpha = 255)
{
	std::vector<ubyte> pixels(static_cast<size_t>(width) * height * 4, 0);

	const float cx = (width - 1) * 0.5f;
	const float cy = (height - 1) * 0.5f;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			const float dx = x - cx;
			const float dy = y - cy;
			const float r = std::sqrt(dx * dx + dy * dy);

			float value;
			if (r <= disc_radius) {
				value = 255.0f;
			} else {
				const float t = (r - disc_radius) / halo_sigma;
				value = halo_peak * std::exp(-t * t);
			}

			auto* px = &pixels[(static_cast<size_t>(y) * width + x) * 4];
			px[0] = px[1] = px[2] = static_cast<ubyte>(value);
			px[3] = alpha;
		}
	}

	return pixels;
}

float measure(const std::vector<ubyte>& pixels, int width, int height, bool has_alpha = false)
{
	SCP_vector<ubyte> weights;
	sun_disc_build_weights(pixels.data(), width, height, 4, has_alpha, weights);
	return sun_disc_fraction_from_weights(weights.data(), width, height);
}

} // namespace

// A saturated disc of known radius comes back as that radius, in units of the quad's
// half-width, regardless of the halo painted around it.
TEST(SunDiscTest, recovers_disc_radius)
{
	constexpr int size = 256;

	for (float radius : {16.0f, 32.0f, 64.0f}) {
		const auto pixels = make_sun(size, size, radius, 200.0f, 24.0f);

		const float expected = radius / (0.5f * size);
		EXPECT_NEAR(expected, measure(pixels, size, size), 0.02f) << "disc radius " << radius;
	}
}

// The halo must not move the answer -- a second-moment style estimator would be dominated
// by it, which is why the measurement is coverage based.
TEST(SunDiscTest, ignores_halo_size_and_brightness)
{
	constexpr int size = 256;
	constexpr float radius = 32.0f;

	const float bare = measure(make_sun(size, size, radius, 0.0f, 1.0f), size, size);
	const float small_halo = measure(make_sun(size, size, radius, 180.0f, 16.0f), size, size);
	const float big_halo = measure(make_sun(size, size, radius, 220.0f, 64.0f), size, size);

	EXPECT_NEAR(bare, small_halo, 0.01f);
	EXPECT_NEAR(bare, big_halo, 0.01f);
}

// Mods ship deliberately blank sun bitmaps (BtA's SunAntares*-BLANK, Blue Planet's
// SunSolDummy) to get a light source without a visible sun. Without the peak floor the
// threshold collapses to zero, every pixel matches, and a sun that isn't drawn at all would
// get the widest penumbra in the game.
TEST(SunDiscTest, blank_bitmap_has_no_disc)
{
	constexpr int size = 64;
	const std::vector<ubyte> black(static_cast<size_t>(size) * size * 4, 0);

	EXPECT_EQ(0.0f, measure(black, size, size));
}

TEST(SunDiscTest, near_black_bitmap_has_no_disc)
{
	constexpr int size = 64;
	std::vector<ubyte> dim(static_cast<size_t>(size) * size * 4, 0);
	for (size_t i = 0; i < dim.size(); i += 4) {
		dim[i] = dim[i + 1] = dim[i + 2] = SUN_DISC_MIN_PEAK_WEIGHT - 1;
		dim[i + 3] = 255;
	}

	EXPECT_EQ(0.0f, measure(dim, size, size));
}

// A bitmap lit into its corners measures slightly over 1 before clamping; the disc still
// can't be wider than the quad it's drawn on.
TEST(SunDiscTest, fully_lit_bitmap_clamps_to_one)
{
	constexpr int size = 64;
	std::vector<ubyte> white(static_cast<size_t>(size) * size * 4, 255);

	EXPECT_FLOAT_EQ(1.0f, measure(white, size, size));
}

// g3_render_rect_screen_aligned_2d() fits the quad to the bitmap's shorter edge, so that's
// what the fraction is relative to.
TEST(SunDiscTest, normalizes_against_the_shorter_edge)
{
	constexpr int width = 256;
	constexpr int height = 128;
	constexpr float radius = 32.0f;

	const auto pixels = make_sun(width, height, radius, 0.0f, 1.0f);

	EXPECT_NEAR(radius / (0.5f * height), measure(pixels, width, height), 0.02f);
}

// Sun bitmaps with an alpha channel are alpha blended rather than drawn additively (see
// material_determine_blend_mode()), so alpha has to scale the weight.
TEST(SunDiscTest, alpha_scales_the_weight)
{
	constexpr int size = 128;
	constexpr float radius = 24.0f;

	// fully transparent art contributes nothing, however bright its colour channels are
	const auto transparent = make_sun(size, size, radius, 200.0f, 16.0f, 0);
	EXPECT_EQ(0.0f, measure(transparent, size, size, true));

	// and ignoring a meaningless alpha channel must not change an opaque bitmap
	const auto opaque = make_sun(size, size, radius, 200.0f, 16.0f, 255);
	EXPECT_FLOAT_EQ(measure(opaque, size, size, false), measure(opaque, size, size, true));
}

// The disc edge is where the plateau ends, so how steeply the halo falls off around it
// doesn't move the answer -- which is what lets the threshold act as a constant factor
// rather than a per-bitmap judgement call.
TEST(SunDiscTest, insensitive_to_falloff_steepness)
{
	constexpr int size = 256;
	constexpr float radius = 40.0f;

	const float steep = measure(make_sun(size, size, radius, 200.0f, 4.0f), size, size);
	const float shallow = measure(make_sun(size, size, radius, 200.0f, 48.0f), size, size);

	EXPECT_NEAR(steep, shallow, 0.03f);
}

// The boundary that makes the above hold: a halo is only excluded because it is dimmer than
// SUN_DISC_THRESHOLD of the peak. Art whose surround stays within that of full brightness has
// no plateau edge to find, and counting it as part of the emitting disc is the right answer --
// it is just as bright as the disc. Real sun art doesn't do this, but pin the behaviour so a
// future threshold change is a deliberate one.
TEST(SunDiscTest, halo_brighter_than_the_threshold_counts_as_disc)
{
	constexpr int size = 256;
	constexpr float radius = 40.0f;

	// 250/255 is above the 0.9 threshold, so the "halo" reads as more disc
	const float bright_halo = measure(make_sun(size, size, radius, 250.0f, 48.0f), size, size);
	const float dim_halo = measure(make_sun(size, size, radius, 200.0f, 48.0f), size, size);

	EXPECT_GT(bright_halo, dim_halo);
	EXPECT_NEAR(radius / (0.5f * size), dim_halo, 0.02f);
}

TEST(SunDiscTest, build_weights_takes_the_brightest_channel)
{
	// one pixel each: blue-dominant, green-dominant, red-dominant (BGRA byte order)
	const std::vector<ubyte> pixels = {
		200, 10, 20, 255,
		10, 180, 20, 255,
		10, 20, 160, 255,
	};

	SCP_vector<ubyte> weights;
	sun_disc_build_weights(pixels.data(), 3, 1, 4, false, weights);

	ASSERT_EQ(3u, weights.size());
	EXPECT_EQ(200, weights[0]);
	EXPECT_EQ(180, weights[1]);
	EXPECT_EQ(160, weights[2]);
}

TEST(SunDiscTest, build_weights_handles_bgr)
{
	const std::vector<ubyte> pixels = {
		200, 10, 20,
		10, 180, 20,
	};

	SCP_vector<ubyte> weights;
	sun_disc_build_weights(pixels.data(), 2, 1, 3, false, weights);

	ASSERT_EQ(2u, weights.size());
	EXPECT_EQ(200, weights[0]);
	EXPECT_EQ(180, weights[1]);
}
