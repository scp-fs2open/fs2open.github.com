
#include <gtest/gtest.h>

#include <util/FSTestFixture.h>

#include <io/timer.h>
#include <weapon/beam.h>
#include <weapon/weapon.h>

// beam_get_muzzle_glow() is what turns a beam's own muzzle light into a
// lens-flare source (graphics/lens_flare_beams.cpp), so its ramp is what the
// flare actually follows. These pin the exact shape of that ramp --
// including the jump at fire onset and the early cutoff near the end of
// warmdown -- because both come straight from beam_add_light_small()'s
// pre-existing curve (the *0.5f halving and the *1.3f warmdown factor), and a
// future edit that "smoothed" them out would make the flare disagree with the
// light it is supposed to be tracking.
class BeamMuzzleGlowTest : public test::FSTestFixture {
 public:
	BeamMuzzleGlowTest() { pushModDir("beam"); }

 protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();

		Weapon_info.emplace_back();
		weapon_info_index = static_cast<int>(Weapon_info.size()) - 1;
		weapon_info& wip = Weapon_info[weapon_info_index];
		wip.b_info.beam_warmup = 1000;
		wip.b_info.beam_warmdown = 1000;
		// Isolate the ramp from beam_current_light_radius()'s other multipliers,
		// neither of which this test is about.
		wip.b_info.beam_light_as_multiplier = false;
		wip.b_info.beam_light_flicker = false;
		wip.light_radius = 5.0f;
		wip.light_color_set = true;
		wip.light_color = hdr_color(1.0f, 1.0f, 1.0f, 1.0f, 4.0f);

		bm = beam();
		bm.weapon_info_index = weapon_info_index;
		bm.warmup_stamp = -1;
		bm.warmdown_stamp = -1;
		bm.last_start = vmd_zero_vector;
	}

	void TearDown() override
	{
		Weapon_info.pop_back();

		test::FSTestFixture::TearDown();
	}

	int weapon_info_index = -1;
	beam bm;
};

TEST_F(BeamMuzzleGlowTest, FiringHoldsAtFullIntensity)
{
	bm.warmup_stamp = -1;
	bm.warmdown_stamp = -1;

	beam_muzzle_glow glow;
	ASSERT_TRUE(beam_get_muzzle_glow(&bm, &glow));
	EXPECT_NEAR(glow.intensity, 4.0f, 1e-3f);
}

TEST_F(BeamMuzzleGlowTest, WarmupStartsAtZero)
{
	bm.warmup_stamp = timestamp(1000); // the full warmup still ahead
	bm.warmdown_stamp = -1;

	beam_muzzle_glow glow;
	EXPECT_FALSE(beam_get_muzzle_glow(&bm, &glow));
}

TEST_F(BeamMuzzleGlowTest, WarmupRisesToHalfAtMidpoint)
{
	bm.warmup_stamp = timestamp(500); // halfway through a 1000ms warmup
	bm.warmdown_stamp = -1;

	beam_muzzle_glow glow;
	ASSERT_TRUE(beam_get_muzzle_glow(&bm, &glow));
	// BEAM_WARMUP_PCT ~= 0.5, halved by the legacy muzzle-light curve
	EXPECT_NEAR(glow.intensity, 4.0f * 0.25f, 0.05f);
}

TEST_F(BeamMuzzleGlowTest, WarmdownJumpsDownAtIgnition)
{
	bm.warmup_stamp = -1;
	bm.warmdown_stamp = timestamp(1000); // the full warmdown still ahead

	beam_muzzle_glow glow;
	ASSERT_TRUE(beam_get_muzzle_glow(&bm, &glow));
	// Firing was 1.0; warmdown's first instant is already halved -- the pop is
	// inherited from beam_add_light_small(), not introduced by the flare.
	EXPECT_NEAR(glow.intensity, 4.0f * 0.5f, 0.05f);
}

TEST_F(BeamMuzzleGlowTest, WarmdownFadesBeforeItsNominalEnd)
{
	bm.warmup_stamp = -1;
	bm.warmdown_stamp = timestamp(10); // almost fully warmed down

	beam_muzzle_glow glow;
	// The *1.3f factor drives the ramp to zero before timestamp_elapsed()
	// actually deletes the beam, so no flare should draw here.
	EXPECT_FALSE(beam_get_muzzle_glow(&bm, &glow));
}
