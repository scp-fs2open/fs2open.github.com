#include <gtest/gtest.h>

#include <util/FSTestFixture.h>

#include <graphics/lens_flare.h>
#include <graphics/lens_flare_internal.h>
#include <model/model.h>
#include <render/3d.h>
#include <species_defs/species_defs.h>
#include <starfield/starfield.h>

#include <cmath>
#include <functional>
#include <iterator>
#include <string>

using namespace graphics;

namespace {

// Thick biconvex singlet (R1=100, R2=-100, n=1.5, d=2) with the stop embedded
// mid-glass so no extra air gaps skew the analytic reference values.
lens_system make_singlet()
{
	lens_system ls;
	ls.name = "test_singlet";
	ls.coating_wavelength = 0.0f; // uncoated

	lens_surface front;
	front.radius = 100.0f;
	front.thickness = 1.0f;
	front.n = 1.5f;
	ls.surfaces.push_back(front);

	lens_surface stop;
	stop.thickness = 1.0f;
	stop.is_stop = true;
	ls.surfaces.push_back(stop);

	lens_surface back;
	back.radius = -100.0f;
	back.thickness = 10.0f; // ignored; sensor distance is computed
	back.n = 1.0f;
	ls.surfaces.push_back(back);

	return ls;
}

} // namespace

TEST(LensFlare, FresnelUncoated)
{
	// Air -> glass at normal incidence: R = ((n1-n2)/(n1+n2))^2 = 0.04
	EXPECT_NEAR(lens_flare_fresnel_reflectance(1.0f, 1.5f, 0.0f, 550.0f), 0.04f, 1e-5f);
	// Symmetric in the direction of travel
	EXPECT_NEAR(lens_flare_fresnel_reflectance(1.5f, 1.0f, 0.0f, 550.0f), 0.04f, 1e-5f);
}

TEST(LensFlare, FresnelIdealQuarterWaveCoating)
{
	// With nc = sqrt(n1*n2) exactly (here 1.38 = sqrt(1.9044)) a quarter-wave
	// coating cancels reflection completely at its design wavelength
	float r = lens_flare_fresnel_reflectance(1.0f, 1.9044f, 550.0f, 550.0f);
	EXPECT_NEAR(r, 0.0f, 1e-6f);

	// Away from the design wavelength some reflection returns, but still far
	// below the uncoated value
	float uncoated = lens_flare_fresnel_reflectance(1.0f, 1.9044f, 0.0f, 400.0f);
	float coated = lens_flare_fresnel_reflectance(1.0f, 1.9044f, 550.0f, 400.0f);
	EXPECT_GT(coated, 0.0f);
	EXPECT_LT(coated, uncoated);
}

TEST(LensFlare, FftDeltaAndRoundtrip)
{
	const int size = 16;

	// FFT of a delta at the origin is a constant
	SCP_vector<std::complex<float>> data(size * size, {0.0f, 0.0f});
	data[0] = {1.0f, 0.0f};
	lens_flare_fft2d(data, size, false);
	for (const auto& v : data) {
		EXPECT_NEAR(v.real(), 1.0f, 1e-4f);
		EXPECT_NEAR(v.imag(), 0.0f, 1e-4f);
	}

	// Forward + inverse returns the input
	SCP_vector<std::complex<float>> orig(size * size);
	for (int i = 0; i < size * size; i++) {
		orig[i] = {sinf(i * 0.37f), cosf(i * 0.11f)};
	}
	SCP_vector<std::complex<float>> work = orig;
	lens_flare_fft2d(work, size, false);
	lens_flare_fft2d(work, size, true);
	for (int i = 0; i < size * size; i++) {
		EXPECT_NEAR(work[i].real(), orig[i].real(), 1e-3f);
		EXPECT_NEAR(work[i].imag(), orig[i].imag(), 1e-3f);
	}
}

TEST(LensFlare, SingletFocalLengthAndGhostCount)
{
	lens_system ls = make_singlet();
	ASSERT_TRUE(lens_flare_precompute(ls));

	// Thick-lens references: 1/f = (n-1)*(1/R1 - 1/R2 + (n-1)*d/(n*R1*R2)),
	// BFD = f*(1 - (n-1)*d/(n*R1))
	EXPECT_NEAR(ls.efl, 100.3344f, 0.01f);
	EXPECT_NEAR(ls.bfd, 99.6656f, 0.01f);

	// Two refractive surfaces (the stop doesn't reflect) -> exactly one
	// two-reflection ghost
	ASSERT_EQ(ls.ghosts.size(), 1u);
	EXPECT_EQ(ls.ghosts[0].surf_first, 2);
	EXPECT_EQ(ls.ghosts[0].surf_second, 0);

	// Uncoated air/glass reflections: R = 0.04 each, product 1.6e-3
	EXPECT_NEAR(ls.ghosts[0].reflectance[1], 0.04f * 0.04f, 1e-5f);
}

TEST(LensFlare, GhostMatricesAreUnimodular)
{
	// Ghost paths start and end in air, so det(Ms * Ma) == 1 must hold for
	// every ghost and wavelength (ray-transfer matrix invariant)
	lens_system ls = make_singlet();

	// Add a cemented doublet behind the stop for more surfaces/ghost paths;
	// the exit of the doublet flows into the original back surface (1.58 -> 1.0)
	// so all four glass interfaces stay refractive
	lens_surface extra1;
	extra1.radius = 50.0f;
	extra1.thickness = 3.0f;
	extra1.n = 1.62f;
	extra1.abbe = 36.0f;
	lens_surface extra2;
	extra2.radius = -75.0f;
	extra2.thickness = 10.0f;
	extra2.n = 1.58f;
	ls.surfaces.insert(ls.surfaces.end() - 1, extra1);
	ls.surfaces.insert(ls.surfaces.end() - 1, extra2);

	ASSERT_TRUE(lens_flare_precompute(ls));

	// Four refractive surfaces -> C(4,2) = 6 ghost paths, but the pairing of
	// the two faint cement interfaces (R ~ 1e-4 * 1e-3) falls below the
	// reflectance culling floor, leaving 5
	EXPECT_EQ(ls.ghosts.size(), 5u);

	for (const auto& ghost : ls.ghosts) {
		for (int wl = 0; wl < 3; wl++) {
			const float* ma = ghost.ma[wl];
			const float* ms = ghost.ms[wl];
			float a = ms[0] * ma[0] + ms[1] * ma[2];
			float b = ms[0] * ma[1] + ms[1] * ma[3];
			float c = ms[2] * ma[0] + ms[3] * ma[2];
			float d = ms[2] * ma[1] + ms[3] * ma[3];
			EXPECT_NEAR(a * d - b * c, 1.0f, 1e-3f);
		}
	}
}

class LensFlareTableTest : public test::FSTestFixture {
  public:
	LensFlareTableTest() : test::FSTestFixture(INIT_CFILE) {}

	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		lens_flare_init();
	}

	void TearDown() override
	{
		lens_flare_close();
		test::FSTestFixture::TearDown();
	}
};

// Guards the shipped prescriptions in def_files/data/tables/lens_flares.tbl: a
// lens whose surface stack doesn't image onto a sensor is dropped at load with
// only a warning, so a bad transcription would otherwise go unnoticed.
TEST_F(LensFlareTableTest, ShippedLensesParseAndPrecompute)
{
	// Named individually because an unusable prescription is only warned about
	// and then skipped -- a lens that stopped loading would still leave a
	// well-formed (just smaller) table behind
	static const char* const shipped[] = {"angenieux_100mm", "tessar_50mm", "canon_70_200mm", "kodak_100mm",
		"leica_35mm", "nikon_50_135mm", "color_heliar_105mm", "zeiss_master_prime_50mm"};
	for (const char* name : shipped) {
		EXPECT_GE(lens_flare_lookup(name), 0) << "lens '" << name << "' is missing from lens_flares.tbl";
	}

	const int count = lens_flare_num_systems();
	ASSERT_GE(count, static_cast<int>(std::size(shipped)));

	for (int i = 0; i < count; i++) {
		const lens_system* ls = lens_flare_get_system(i);
		ASSERT_NE(ls, nullptr);
		SCOPED_TRACE(ls->name);

		// precompute() already rejects these, but a rejected lens is simply
		// absent, so assert on what did load
		EXPECT_GT(ls->efl, 0.0f);
		EXPECT_GT(ls->bfd, 0.0f);
		EXPECT_FALSE(ls->ghosts.empty());
		EXPECT_LE(static_cast<int>(ls->ghosts.size()), ls->max_ghosts);
		EXPECT_GT(ls->entrance_radius, 0.0f);
		EXPECT_GT(ls->aperture_radius, 0.0f);
		EXPECT_GT(ls->sensor_width, 0.0f);
		// every shipped lens is spherical, so both anamorphic paths must be
		// exactly off -- this is what keeps existing content identical
		EXPECT_FLOAT_EQ(ls->anamorphic_squeeze, 1.0f);
		EXPECT_FLOAT_EQ(ls->streak.strength, 0.0f);

		for (const auto& ghost : ls->ghosts) {
			for (int wl = 0; wl < 3; wl++) {
				const float* ma = ghost.ma[wl];
				const float* ms = ghost.ms[wl];
				for (int k = 0; k < 4; k++) {
					ASSERT_TRUE(std::isfinite(ma[k]));
					ASSERT_TRUE(std::isfinite(ms[k]));
				}
				// ghost paths start and end in air (see GhostMatricesAreUnimodular)
				float a = ms[0] * ma[0] + ms[1] * ma[2];
				float b = ms[0] * ma[1] + ms[1] * ma[3];
				float c = ms[2] * ma[0] + ms[3] * ma[2];
				float d = ms[2] * ma[1] + ms[3] * ma[3];
				EXPECT_NEAR(a * d - b * c, 1.0f, 1e-2f);
				EXPECT_GT(ghost.reflectance[wl], 0.0f);
				EXPECT_LT(ghost.reflectance[wl], 1.0f);
			}
		}
	}
}

// The iris shape is the one definition behind both the ghosts and the
// starburst, so its geometry is worth pinning down: the polygon's corners sit
// on the iris radius, its blade midpoints pull in to the apothem, and curvature
// interpolates the midpoints out to a circle. (Mask only -- no FFT needed.)
TEST(LensFlareAperture, ShapeGeometry)
{
	lens_flare_textures tex;

	// probe the mask along a ray and report where transmission crosses 0.5
	auto boundary = [&tex](float angle_deg) {
		const int size = tex.aperture_size;
		float dx = cosf(fl_radians(angle_deg));
		float dy = sinf(fl_radians(angle_deg));
		float prev = 1.0f;
		for (int i = 1; i < 2000; i++) {
			float r = i / 2000.0f * 1.4f;
			int x = static_cast<int>((r * dx + 1.0f) * 0.5f * size);
			int y = static_cast<int>((r * dy + 1.0f) * 0.5f * size);
			if (x < 0 || x >= size || y < 0 || y >= size) {
				return r;
			}
			float v = tex.aperture[static_cast<size_t>(y) * size + x] / 255.0f;
			if (prev >= 0.5f && v < 0.5f) {
				return r;
			}
			prev = v;
		}
		return -1.0f;
	};

	const float iris = 0.9f;
	const float apothem = iris * cosf(PI / 6.0f);

	lens_aperture ap;
	ap.blades = 6;
	ap.rotation = 0.0f;
	ap.curvature = 0.0f;

	// straight blades: corners on the +x axis (and every 60 deg from it),
	// midpoints pulled in to the apothem
	lens_flare_generate_aperture_mask(ap, &tex);
	EXPECT_NEAR(boundary(0.0f), iris, 0.01f);
	EXPECT_NEAR(boundary(60.0f), iris, 0.01f);
	EXPECT_NEAR(boundary(30.0f), apothem, 0.01f);

	// full curvature lifts the midpoints to the corner radius (a circle)
	ap.curvature = 1.0f;
	lens_flare_generate_aperture_mask(ap, &tex);
	EXPECT_NEAR(boundary(30.0f), iris, 0.01f);

	// negative curvature bows them inward instead, past the straight-blade case
	ap.curvature = -1.0f;
	lens_flare_generate_aperture_mask(ap, &tex);
	EXPECT_LT(boundary(30.0f), apothem - 0.05f);

	// rotation carries the corners with it
	ap.curvature = 0.0f;
	ap.rotation = 30.0f;
	lens_flare_generate_aperture_mask(ap, &tex);
	EXPECT_NEAR(boundary(30.0f), iris, 0.01f);
}

// Every imperfection layer must actually occlude, and must leave the mask
// usable rather than blacking it out.
TEST(LensFlareAperture, ImperfectionLayers)
{
	lens_flare_textures tex;
	auto transmission = [&tex](const lens_aperture& ap) {
		lens_flare_generate_aperture_mask(ap, &tex);
		double sum = 0.0;
		for (auto v : tex.aperture) {
			sum += v;
		}
		return sum / tex.aperture.size() / 255.0;
	};

	lens_aperture ap; // defaults: every layer off
	const double base = transmission(ap);
	EXPECT_GT(base, 0.1);
	EXPECT_LT(base, 1.0);

	ap.grating.strength = 1.0f;
	EXPECT_LT(transmission(ap), base);
	ap.grating.strength = 0.0f;

	ap.scratches.strength = 1.0f;
	EXPECT_LT(transmission(ap), base);
	ap.scratches.strength = 0.0f;

	ap.dust.strength = 1.0f;
	EXPECT_LT(transmission(ap), base);
	ap.dust.strength = 0.0f;

	// strength 0 is exactly "off", whatever the other knobs say
	ap.grating.density = 1.0f;
	ap.scratches.density = 1.0f;
	ap.dust.density = 1.0f;
	EXPECT_NEAR(transmission(ap), base, 1e-9);
}

// One aperture definition drives both outputs, so editing it has to rebuild the
// starburst as well as the ghost mask.
TEST_F(LensFlareTableTest, ApertureEditRebuildsStarburst)
{
	const int idx = lens_flare_lookup("angenieux_100mm");
	ASSERT_GE(idx, 0);
	auto* lens = lens_flare_get_system_mutable(idx);
	ASSERT_NE(lens, nullptr);

	const auto* before = lens_flare_get_textures(idx);
	ASSERT_NE(before, nullptr);
	SCP_vector<float> starburst_before = before->starburst;
	ASSERT_FALSE(starburst_before.empty());

	lens->aperture.blades = 3;
	lens->aperture.rotation = 40.0f;
	lens_flare_invalidate_textures(idx);

	const auto* after = lens_flare_get_textures(idx);
	ASSERT_NE(after, nullptr);
	ASSERT_EQ(after->starburst.size(), starburst_before.size());
	EXPECT_NE(after->starburst, starburst_before) << "starburst did not follow the aperture";
}

// The backends cache their uploaded copy per lens, so an edit has to change the
// generation counter or the new mask never reaches the GPU.
TEST_F(LensFlareTableTest, TextureInvalidationBumpsGeneration)
{
	const int idx = lens_flare_lookup("angenieux_100mm");
	ASSERT_GE(idx, 0);

	ASSERT_NE(lens_flare_get_textures(idx), nullptr);
	const unsigned int before = lens_flare_get_texture_generation();

	lens_flare_invalidate_textures(idx);
	EXPECT_NE(lens_flare_get_texture_generation(), before);

	// out-of-range invalidation is a no-op, not a bump
	const unsigned int after = lens_flare_get_texture_generation();
	lens_flare_invalidate_textures(lens_flare_num_systems() + 5);
	EXPECT_EQ(lens_flare_get_texture_generation(), after);
}

// Same engine, but reached through the table parser and the *-lens.tbm modular
// path rather than by setting the struct directly. Every aperture field is
// wired up by hand, so only running a table through it proves each one lands in
// the member it names.
// Uses a table that declares a $Default Lens:, so that "take the default" and
// "no flares" are distinguishable -- see the tbm's own comment.
class LensFlareDefaultLensTest : public test::FSTestFixture {
  public:
	LensFlareDefaultLensTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("graphics");
		pushModDir("lens_flare");
		pushModDir("default_lens");
	}

	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		lens_flare_init();
	}

	void TearDown() override
	{
		lens_flare_close();
		test::FSTestFixture::TearDown();
	}
};

// lens_flare_switch_to() is the one place that resolves a camera-lens name, for
// the mission field, the set-camera-lens sexp and both editors alike. The two
// cases that matter are the ones a declared default separates: an empty name is
// "this mission says nothing" and must take the default, while LENS_NAME_NONE is
// "no flares" and must not. Collapsing those two is what silently overrode a
// mission that had deliberately asked for no flares.
TEST_F(LensFlareDefaultLensTest, NameVocabularyDistinguishesDefaultFromNone)
{
	const int declared = lens_flare_lookup("default_test_lens");
	ASSERT_GE(declared, 0) << "the modular *-lens.tbm was not picked up at all";
	ASSERT_STREQ(lens_flare_default_name(), "default_test_lens");

	// no opinion -> the declared default
	lens_flare_switch_to("");
	EXPECT_EQ(lens_flare_active_lens(), declared);
	lens_flare_switch_to(nullptr);
	EXPECT_EQ(lens_flare_active_lens(), declared);

	// ...and the default asked for by name is the same thing
	lens_flare_switch_to(LENS_NAME_DEFAULT);
	EXPECT_EQ(lens_flare_active_lens(), declared);

	// <none> is the one answer that survives a declared default
	lens_flare_switch_to(LENS_NAME_NONE);
	EXPECT_EQ(lens_flare_active_lens(), -1) << "<none> must not fall back to $Default Lens:";

	// the sentinels are case-insensitive, like every other name here
	lens_flare_switch_to("<NONE>");
	EXPECT_EQ(lens_flare_active_lens(), -1);
	lens_flare_switch_to("<Default>");
	EXPECT_EQ(lens_flare_active_lens(), declared);

	// (An unknown name also falls back to the default, but it warns on the way and
	// the test harness turns Warning() into a thrown exception, so that path can't
	// be exercised from here.)

	// leaving the mission re-arms the default, not "no lens"
	lens_flare_switch_to(LENS_NAME_NONE);
	ASSERT_EQ(lens_flare_active_lens(), -1);
	lens_flare_reset_for_level();
	EXPECT_EQ(lens_flare_active_lens(), declared);
}

// stars.tbl decides *whether* a sun flares; the camera lens only decides *how*.
// Parses a table covering both ways a sun can say so, and the case where they
// disagree. Only parses -- stars_init() would also load bitmaps this test has
// none of.
class SunCameraLensFlareTest : public test::FSTestFixture {
  public:
	SunCameraLensFlareTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("graphics");
		pushModDir("lens_flare");
		pushModDir("sun_flare_opt_in");
	}

	// The tabled answer, by sun name. stars_find_sun() gives the bitmap index the
	// option was parsed into, which is knowable without placing an instance.
	static bool sun_flares(const char* name)
	{
		const int idx = stars_find_sun(name);
		EXPECT_GE(idx, 0) << "sun '" << name << "' did not parse out of the test stars.tbl";
		return stars_sun_bitmap_has_camera_lens_flare(idx);
	}
};

TEST_F(SunCameraLensFlareTest, CameraLensFlareOptInOverridesTheFlareFallback)
{
	parse_startbl("stars.tbl");

	// A sun that never asked to flare gets nothing from the camera lens. This is
	// the whole point: mounting a lens must not invent flares on existing content.
	EXPECT_FALSE(sun_flares("SunNeither"));

	// A legacy sprite $Flare: block still stands in for the opt-in, so tables
	// written before "+Camera Lens Flare:" existed keep working unchanged
	EXPECT_TRUE(sun_flares("SunLegacyFlareOnly"));

	// ...and the new option opts in on its own, with no sprite flare fields, which
	// is the reason it exists
	EXPECT_TRUE(sun_flares("SunLensOnly"));

	// Where the two disagree the explicit option wins -- otherwise a sun could not
	// keep its sprite flare while sitting out the physically-based one
	EXPECT_FALSE(sun_flares("SunFlareButNoLens"));

	// The option is parsed between the $Flare: block and $NoGlare:; if that
	// ordering were wrong the option would silently never match, so check a sun
	// that uses it alongside the option that follows it
	EXPECT_TRUE(sun_flares("SunLensAndNoGlare"));
}

class LensFlareApertureTbmTest : public test::FSTestFixture {
  public:
	LensFlareApertureTbmTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("graphics");
		pushModDir("lens_flare");
		pushModDir("aperture_fields");
	}

	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		lens_flare_init();
	}

	void TearDown() override
	{
		lens_flare_close();
		test::FSTestFixture::TearDown();
	}
};

TEST_F(LensFlareApertureTbmTest, ParsesEveryApertureField)
{
	const int idx = lens_flare_lookup("aperture_test_lens");
	ASSERT_GE(idx, 0) << "the modular *-lens.tbm was not picked up at all";

	const lens_system* ls = lens_flare_get_system(idx);
	ASSERT_NE(ls, nullptr);
	const lens_aperture& ap = ls->aperture;

	EXPECT_EQ(ap.blades, 11);
	EXPECT_FLOAT_EQ(ap.rotation, 21.0f);
	EXPECT_FLOAT_EQ(ap.curvature, 0.31f);
	EXPECT_FLOAT_EQ(ap.softness, 0.041f);

	EXPECT_FLOAT_EQ(ap.grating.strength, 0.51f);
	EXPECT_FLOAT_EQ(ap.grating.density, 0.52f);
	EXPECT_FLOAT_EQ(ap.grating.length, 0.53f);
	EXPECT_FLOAT_EQ(ap.grating.width, 0.54f);
	EXPECT_FLOAT_EQ(ap.grating.softness, 0.055f);

	EXPECT_FLOAT_EQ(ap.scratches.strength, 0.61f);
	EXPECT_FLOAT_EQ(ap.scratches.density, 0.62f);
	EXPECT_FLOAT_EQ(ap.scratches.length, 0.63f);
	EXPECT_FLOAT_EQ(ap.scratches.width, 0.64f);
	EXPECT_FLOAT_EQ(ap.scratches.rotation, 65.0f);
	EXPECT_FLOAT_EQ(ap.scratches.rotation_variation, 0.66f);
	EXPECT_FLOAT_EQ(ap.scratches.softness, 0.067f);

	EXPECT_FLOAT_EQ(ap.dust.strength, 0.71f);
	EXPECT_FLOAT_EQ(ap.dust.density, 0.72f);
	EXPECT_FLOAT_EQ(ap.dust.radius, 0.73f);
	EXPECT_FLOAT_EQ(ap.dust.softness, 0.074f);

	// the fields that follow the aperture block must still be reachable -- a
	// mis-ordered optional_string would swallow the rest of the entry
	EXPECT_FALSE(ls->starburst);
	EXPECT_FLOAT_EQ(ls->intensity, 0.25f);
	EXPECT_EQ(ls->max_ghosts, 7);
	EXPECT_FLOAT_EQ(ls->entrance_radius, 20.0f);
	EXPECT_FLOAT_EQ(ls->aperture_radius, 14.0f);
	EXPECT_FLOAT_EQ(ls->anamorphic_squeeze, 1.75f);
	EXPECT_FLOAT_EQ(ls->streak.strength, 0.81f);
	EXPECT_FLOAT_EQ(ls->streak.length, 0.82f);
	EXPECT_FLOAT_EQ(ls->streak.thickness, 0.083f);
	EXPECT_FLOAT_EQ(ls->streak.tint[0], 0.84f);
	EXPECT_FLOAT_EQ(ls->streak.tint[1], 0.85f);
	EXPECT_FLOAT_EQ(ls->streak.tint[2], 0.86f);

	// and the tbm must not have disturbed the built-in table
	EXPECT_GE(lens_flare_lookup("angenieux_100mm"), 0);
}

// A mission's set-lens-* sexps edit the tabled lens in place, so the reset that
// stars_pre_level_init() performs is the only thing stopping one mission's lens
// from carrying into the next.
TEST_F(LensFlareTableTest, ResetForLevelRestoresTabledValues)
{
	const int idx = lens_flare_lookup("angenieux_100mm");
	ASSERT_GE(idx, 0);
	auto* lens = lens_flare_get_system_mutable(idx);
	ASSERT_NE(lens, nullptr);

	const lens_aperture tabled = lens->tabled_aperture;
	ASSERT_EQ(lens->aperture, tabled) << "a freshly parsed lens must match its own snapshot";

	// make sure the textures exist, so the reset has something to invalidate
	ASSERT_NE(lens_flare_get_textures(idx), nullptr);
	const unsigned int generation = lens_flare_get_texture_generation();

	// what a mission's sexps would do
	lens->aperture.blades = 3;
	lens->aperture.dust.strength = 0.8f;
	lens->aperture.scratches.strength = 0.4f;
	ASSERT_NE(lens->aperture, tabled);

	lens_flare_reset_for_level();

	EXPECT_EQ(lens->aperture, tabled);
	EXPECT_NE(lens_flare_get_texture_generation(), generation) << "backends would keep the edited textures";

	// a second reset has nothing to do, and must not churn the backends' caches
	const unsigned int settled = lens_flare_get_texture_generation();
	lens_flare_reset_for_level();
	EXPECT_EQ(lens_flare_get_texture_generation(), settled);
}

// The sexps apply their arguments to a copy and only rebuild when the result
// differs, so that an unguarded repeating mission event doesn't regenerate a
// 512^2 mask and its FFT every frame. That guard is only as good as this
// comparison.
TEST(LensFlareAperture, EqualityCoversEveryField)
{
	lens_aperture a;
	EXPECT_EQ(a, lens_aperture());

	// every field, including the ones nested in the imperfection layers
	SCP_vector<std::function<void(lens_aperture&)>> mutators = {
		[](lens_aperture& x) { x.blades += 1; },
		[](lens_aperture& x) { x.rotation += 1.0f; },
		[](lens_aperture& x) { x.curvature += 0.5f; },
		[](lens_aperture& x) { x.softness += 0.5f; },
		[](lens_aperture& x) { x.grating.strength += 0.5f; },
		[](lens_aperture& x) { x.grating.density += 0.25f; },
		[](lens_aperture& x) { x.grating.length += 0.25f; },
		[](lens_aperture& x) { x.grating.width += 0.25f; },
		[](lens_aperture& x) { x.grating.softness += 0.25f; },
		[](lens_aperture& x) { x.scratches.strength += 0.5f; },
		[](lens_aperture& x) { x.scratches.density += 0.25f; },
		[](lens_aperture& x) { x.scratches.length += 0.25f; },
		[](lens_aperture& x) { x.scratches.width += 0.25f; },
		[](lens_aperture& x) { x.scratches.rotation += 1.0f; },
		[](lens_aperture& x) { x.scratches.rotation_variation += 0.25f; },
		[](lens_aperture& x) { x.scratches.softness += 0.25f; },
		[](lens_aperture& x) { x.dust.strength += 0.5f; },
		[](lens_aperture& x) { x.dust.density += 0.25f; },
		[](lens_aperture& x) { x.dust.radius += 0.25f; },
		[](lens_aperture& x) { x.dust.softness += 0.25f; },
	};

	for (size_t i = 0; i < mutators.size(); i++) {
		lens_aperture changed;
		mutators[i](changed);
		SCOPED_TRACE("field index " + std::to_string(i));
		EXPECT_NE(changed, a) << "a changed field is not covered by operator==";
		EXPECT_FALSE(changed == a);
	}
}

// The mounted lens is the mission's, so it has to survive nothing but a level
// change: mounting is what parse_mission_info() does with "$Camera Lens:", and
// the reset before it is what stops the previous mission's camera from leaking.
TEST_F(LensFlareTableTest, MountingAndOverridingTheCameraLens)
{
	const int tessar = lens_flare_lookup("tessar_50mm");
	const int angenieux = lens_flare_lookup("angenieux_100mm");
	ASSERT_GE(tessar, 0);
	ASSERT_GE(angenieux, 0);

	// The shipped table declares no default, so here "take the default" and "no
	// flares" happen to coincide -- LensFlareDefaultLensTest pulls them apart with
	// a table that does declare one.
	EXPECT_STREQ(lens_flare_default_name(), "");
	lens_flare_switch_to("");
	EXPECT_EQ(lens_flare_active_lens(), -1);
	lens_flare_switch_to(LENS_NAME_NONE);
	EXPECT_EQ(lens_flare_active_lens(), -1);

	lens_flare_switch_to("tessar_50mm");
	EXPECT_EQ(lens_flare_active_lens(), tessar);
	EXPECT_STREQ(lens_flare_mission_lens_name(), "tessar_50mm");

	// the lab's override wins while it is set, and reveals the mission's lens
	// again once cleared -- that is what its "Mission default (...)" entry means
	lens_flare_set_lab_lens(angenieux);
	EXPECT_EQ(lens_flare_active_lens(), angenieux);
	EXPECT_STREQ(lens_flare_mission_lens_name(), "tessar_50mm");
	lens_flare_set_lab_lens(-1);
	EXPECT_EQ(lens_flare_active_lens(), -1) << "a -1 override means 'no flares', not 'no override'";
	lens_flare_clear_lab_lens();
	EXPECT_EQ(lens_flare_active_lens(), tessar);

	// leaving the mission unmounts the lens and drops the lab override
	lens_flare_set_lab_lens(angenieux);
	lens_flare_reset_for_level();
	EXPECT_EQ(lens_flare_active_lens(), -1);
	EXPECT_STREQ(lens_flare_mission_lens_name(), "");
}

// ---- thruster flares ----

namespace {

// One nozzle, in model space. `norm` is the direction it fires (and so shines)
// in; a zero normal is legal and means "every way".
glow_point make_nozzle(vec3d pnt, vec3d norm, float radius)
{
	glow_point gpt;
	gpt.pnt = pnt;
	gpt.norm = norm;
	gpt.radius = radius;
	return gpt;
}

// The scene every test below shares: a ship at the origin, unrotated, seen from
// far down -z -- so the eye looks along +z at the back of the ship, and a nozzle
// whose normal is (0, 0, -1) faces the camera squarely.
const matrix Unrotated = vmd_identity_matrix;
const vec3d Ship_pos = vmd_zero_vector;
const vec3d Eye = {{{0.0f, 0.0f, -100.0f}}};

// The apparent size of one nozzle in the shared scene, or -1 when it produces no
// source at all.
float nozzle_apparent(const glow_point& gpt, vec3d* world_pnt = nullptr)
{
	vec3d scratch;
	float apparent = 0.0f;
	if (!lens_flare_nozzle_apparent(gpt, Unrotated, Ship_pos, Eye, world_pnt ? world_pnt : &scratch, &apparent)) {
		return -1.0f;
	}
	return apparent;
}

const vec3d Toward_eye = {{{0.0f, 0.0f, -1.0f}}};
const vec3d Away = {{{0.0f, 0.0f, 1.0f}}};

} // namespace

// A nozzle pointed away from the camera produces no source, which is what keeps
// the far side of every ship out of the pass.
TEST(LensFlareThrusters, FacingAwayProducesNoSource)
{
	EXPECT_LT(nozzle_apparent(make_nozzle(vmd_zero_vector, Away, 1.0f)), 0.0f);
	EXPECT_GT(nozzle_apparent(make_nozzle(vmd_zero_vector, Toward_eye, 1.0f)), 0.0f);
}

// Apparent size is the solid angle the nozzle subtends: it grows with the square
// of its radius and falls with the square of its distance. The distance term is
// what keeps a battle's worth of distant engines from each drawing at full
// strength, so it is worth pinning exactly.
TEST(LensFlareThrusters, ApparentSizeIsSolidAngle)
{
	// squarely facing, 100 units away, radius 1 -> pi * 1^2 / 100^2
	EXPECT_NEAR(nozzle_apparent(make_nozzle(vmd_zero_vector, Toward_eye, 1.0f)), PI / 10000.0f, 1e-9f);

	// twice the radius is four times the apparent size
	EXPECT_NEAR(nozzle_apparent(make_nozzle(vmd_zero_vector, Toward_eye, 2.0f)), 4.0f * PI / 10000.0f, 1e-9f);

	// half the distance is four times the apparent size
	const vec3d halfway = {{{0.0f, 0.0f, -50.0f}}};
	EXPECT_NEAR(nozzle_apparent(make_nozzle(halfway, Toward_eye, 1.0f)), PI / 2500.0f, 1e-9f);
}

// Each nozzle images where it actually is. A ship's engines are set far enough
// apart to read as separate points, which is the whole reason they are no longer
// averaged into one source at their centroid.
TEST(LensFlareThrusters, EachNozzleImagesAtItsOwnPosition)
{
	const vec3d port = {{{-40.0f, 0.0f, 0.0f}}};
	const vec3d starboard = {{{40.0f, 0.0f, 0.0f}}};

	vec3d where;
	nozzle_apparent(make_nozzle(port, Toward_eye, 1.0f), &where);
	EXPECT_NEAR(where.xyz.x, -40.0f, 1e-3f);
	nozzle_apparent(make_nozzle(starboard, Toward_eye, 1.0f), &where);
	EXPECT_NEAR(where.xyz.x, 40.0f, 1e-3f);
}

// The flare fades in over the same first third of the hemisphere the thruster
// glow itself does (modelrender.cpp's `d *= 3`), so a flare can never appear on
// an engine whose glow is still invisible.
TEST(LensFlareThrusters, FacingFalloffMatchesTheGlow)
{
	const float square = nozzle_apparent(make_nozzle(vmd_zero_vector, Toward_eye, 1.0f));

	// a normal tipped so that its cosine against the view is exactly 0.2
	const float cos_view = 0.2f;
	const vec3d tipped = {{{sqrtf(1.0f - (cos_view * cos_view)), 0.0f, -cos_view}}};
	EXPECT_NEAR(nozzle_apparent(make_nozzle(vmd_zero_vector, tipped, 1.0f)), 0.6f * square, 1e-6f);

	// and anything past a third of the hemisphere is simply full brightness
	const float past_third = 0.5f;
	const vec3d wide = {{{sqrtf(1.0f - (past_third * past_third)), 0.0f, -past_third}}};
	EXPECT_NEAR(nozzle_apparent(make_nozzle(vmd_zero_vector, wide, 1.0f)), square, 1e-6f);
}

// Glowpoints are allowed to carry a zero normal, and the thruster renderer reads
// that as "shines every way". The flare has to agree, or such a nozzle would
// silently never flare.
TEST(LensFlareThrusters, NullNormalShinesEveryWay)
{
	EXPECT_NEAR(nozzle_apparent(make_nozzle(vmd_zero_vector, vmd_zero_vector, 1.0f)), PI / 10000.0f, 1e-9f);
}

// lens_flare_point_visible() is the occlusion gate both the thruster and beam
// gathers run against a source's position before ever queuing it for a draw.
// A real occluding hit needs a scene with real ship geometry in it, which this
// test suite has no scaffolding for (the AI line-of-sight test it is built on,
// test_line_of_sight(), has none either) -- but the degenerate "eye sitting
// exactly on the source" case is pure vector math and worth pinning: a source
// coincident with the eye has no direction to test along, and must not be
// mistaken for occluded.
TEST(LensFlareVisibility, EyeExactlyOnSourceIsVisible)
{
	const vec3d saved_eye = Eye_position;
	Eye_position = vmd_zero_vector;

	EXPECT_TRUE(graphics::lens_flare_point_visible(vmd_zero_vector));

	Eye_position = saved_eye;
}

// The lab overrides every species at once and leaves the tabled values alone, so
// switching it off restores them without anything having to be backed up -- and
// leaving a mission drops it, the same way a lab lens is dropped.
TEST(LensFlareThrusters, LabOverrideBeatsTheSpeciesTableAndIsDroppedOnLevelReset)
{
	const auto saved_species = Species_info;
	Species_info.clear();

	species_info tabled;
	strcpy_s(tabled.species_name, "TestSpecies");
	tabled.thruster_flare.enabled = true;
	tabled.thruster_flare.intensity = 3.0f;
	Species_info.push_back(tabled);

	EXPECT_TRUE(lens_flare_thruster_settings(0).enabled);
	EXPECT_FLOAT_EQ(lens_flare_thruster_settings(0).intensity, 3.0f);
	EXPECT_FALSE(lens_flare_thruster_settings(-1).enabled) << "an unknown species must not flare";

	thruster_flare_info override_settings;
	override_settings.enabled = true;
	override_settings.intensity = 7.0f;
	lens_flare_lab_thruster_flare() = override_settings;

	EXPECT_FLOAT_EQ(lens_flare_thruster_settings(0).intensity, 7.0f);
	EXPECT_FLOAT_EQ(Species_info[0].thruster_flare.intensity, 3.0f) << "the override must not write to the table";
	// it applies to every species, including ones with no table entry of their own
	EXPECT_TRUE(lens_flare_thruster_settings(-1).enabled);

	lens_flare_reset_for_level();
	EXPECT_FLOAT_EQ(lens_flare_thruster_settings(0).intensity, 3.0f);

	Species_info = saved_species;
}

// The species-table half of the feature. Parsed here rather than through
// species_init(), which would drag in iff_defs for the shipped table's
// "$Default IFF:"; every entry a mod actually writes is a +nocreate that needs
// none of that.
extern void parse_species_tbl(const char* filename);

class ThrusterFlareTbmTest : public test::FSTestFixture {
  public:
	ThrusterFlareTbmTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("graphics");
		pushModDir("lens_flare");
		pushModDir("thruster_flare");
	}

	void SetUp() override
	{
		test::FSTestFixture::SetUp();

		// Stand in for whatever table the mod underneath us defined
		saved_species = Species_info;
		Species_info.clear();

		species_info existing;
		strcpy_s(existing.species_name, "ThrusterFlareTestSpecies");
		Species_info.push_back(existing);

		species_info bare;
		strcpy_s(bare.species_name, "ThrusterFlareDefaultsSpecies");
		Species_info.push_back(bare);

		parse_species_tbl("test-sdf.tbm");
	}

	void TearDown() override
	{
		Species_info = saved_species;
		test::FSTestFixture::TearDown();
	}

  private:
	SCP_vector<species_info> saved_species;
};

// Guards the exact syntax a mod writes, "$Thruster Flare:" in a +nocreate entry
// with nothing else in it -- which only parses because every option ahead of it
// is optional under +nocreate. Reordering the parse would break real tables
// silently, since an option read out of order is skipped rather than diagnosed.
TEST_F(ThrusterFlareTbmTest, NocreateEntryAddsAFlareToAnExistingSpecies)
{
	const int idx = species_info_lookup("ThrusterFlareTestSpecies");
	ASSERT_GE(idx, 0);

	const auto& flare = Species_info[idx].thruster_flare;
	EXPECT_TRUE(flare.enabled) << "the block was skipped entirely";
	EXPECT_FLOAT_EQ(flare.intensity, 6.0f);
	EXPECT_FLOAT_EQ(flare.afterburner_intensity, 15.0f);
	EXPECT_NEAR(flare.color.xyz.x, 170.0f / 255.0f, 1e-4f);
	EXPECT_NEAR(flare.color.xyz.y, 205.0f / 255.0f, 1e-4f);
	EXPECT_NEAR(flare.color.xyz.z, 255.0f / 255.0f, 1e-4f);
}

// The block on its own is the opt-in; its fields are all optional and must fall
// back to the struct's defaults, not to zero.
TEST_F(ThrusterFlareTbmTest, EmptyBlockIsTheOptInAndKeepsTheDefaults)
{
	const int idx = species_info_lookup("ThrusterFlareDefaultsSpecies");
	ASSERT_GE(idx, 0);

	const auto& flare = Species_info[idx].thruster_flare;
	const thruster_flare_info defaults;
	EXPECT_TRUE(flare.enabled);
	EXPECT_FLOAT_EQ(flare.intensity, defaults.intensity);
	EXPECT_FLOAT_EQ(flare.afterburner_intensity, defaults.afterburner_intensity);
}

// A table written for one mod has to be harmless in another: a +nocreate naming
// a species nothing defined is discarded rather than creating a stub. This is
// what lets one -sdf.tbm carry entries for several mods' species at once.
TEST_F(ThrusterFlareTbmTest, NocreateForAnUnknownSpeciesCreatesNothing)
{
	EXPECT_LT(species_info_lookup("NoSuchSpecies"), 0);
	EXPECT_EQ(Species_info.size(), 2u);
}

// ---- +override in a *-lens.tbm ----

class LensFlareOverrideTbmTest : public test::FSTestFixture {
  public:
	LensFlareOverrideTbmTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("graphics");
		pushModDir("lens_flare");
		pushModDir("override");
	}

	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		lens_flare_init();
	}

	void TearDown() override
	{
		lens_flare_close();
		test::FSTestFixture::TearDown();
	}
};

// The point of +override: restyle a shipped lens without transcribing its
// prescription. Everything the entry does not mention has to survive, which is
// the part a hand-written copy of lens_system would quietly get wrong every time
// a field was added.
TEST_F(LensFlareOverrideTbmTest, OverrideKeepsEverythingItDoesNotMention)
{
	const int idx = lens_flare_lookup("angenieux_100mm");
	ASSERT_GE(idx, 0);
	const lens_system* ls = lens_flare_get_system(idx);
	ASSERT_NE(ls, nullptr);

	// what the tbm set
	EXPECT_FLOAT_EQ(ls->anamorphic_squeeze, 2.0f);
	EXPECT_FLOAT_EQ(ls->intensity, 0.75f);

	// what it did not: these are the shipped table's values, and the twelve-surface
	// prescription in particular must be intact
	EXPECT_FLOAT_EQ(ls->entrance_radius, 22.0f);
	EXPECT_FLOAT_EQ(ls->aperture_radius, 16.0f);
	EXPECT_FLOAT_EQ(ls->sensor_width, 36.0f);
	EXPECT_EQ(ls->aperture.blades, 6);
	EXPECT_GT(ls->surfaces.size(), 8u) << "the override dropped the prescription it never touched";
	EXPECT_FALSE(ls->ghosts.empty());

	// an override is still a tabled value, so it is what a mission's sexps and the
	// lab get reset back to
	EXPECT_TRUE(ls->tabled_aperture == ls->aperture);
}

// Opening a stack replaces the whole of it. A prescription is an ordered run
// whose focal length, ghost set and iris position all follow from the run as a
// whole, so a merged stack would be neither lens.
TEST_F(LensFlareOverrideTbmTest, OverridingTheStackReplacesItEntirely)
{
	const int idx = lens_flare_lookup("tessar_50mm");
	ASSERT_GE(idx, 0);
	const lens_system* ls = lens_flare_get_system(idx);
	ASSERT_NE(ls, nullptr);

	ASSERT_EQ(ls->surfaces.size(), 3u) << "the tbm's stack was merged into the shipped one instead of replacing it";
	EXPECT_FLOAT_EQ(ls->surfaces[0].radius, 60.0f);
	EXPECT_FLOAT_EQ(ls->surfaces[0].abbe, 55.0f);
	EXPECT_TRUE(ls->surfaces[1].is_stop);
	EXPECT_FLOAT_EQ(ls->surfaces[2].radius, -60.0f);

	// the entry named no other option, so everything else is still the shipped
	// lens -- and the new prescription was re-solved rather than left stale
	EXPECT_FLOAT_EQ(ls->entrance_radius, 7.0f);
	EXPECT_GT(ls->efl, 0.0f);
	EXPECT_FALSE(ls->ghosts.empty());
}

// Without +override an entry is a complete definition, and replaces a lens of
// the same name outright -- the behaviour that predates +override, and the
// reason +override had to be opt-in rather than the default for a tbm.
TEST_F(LensFlareOverrideTbmTest, WithoutOverrideTheLensIsReplacedOutright)
{
	const int idx = lens_flare_lookup("kodak_100mm");
	ASSERT_GE(idx, 0);
	const lens_system* ls = lens_flare_get_system(idx);
	ASSERT_NE(ls, nullptr);

	EXPECT_EQ(ls->surfaces.size(), 3u);
	EXPECT_FLOAT_EQ(ls->entrance_radius, 11.0f);
	// the shipped kodak_100mm sets these; a replacement must be back on the
	// struct's defaults rather than inheriting them
	EXPECT_FLOAT_EQ(ls->sensor_width, 36.0f);
	EXPECT_EQ(ls->aperture.blades, 6);
}
