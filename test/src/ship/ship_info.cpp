#include <gtest/gtest.h>

#include "util/FSTestFixture.h"

#include "hud/hud.h"
#include "particle/ParticleManager.h"
#include "ship/ship.h"

class ShipInfoCloneTest : public test::FSTestFixture {
  public:
	ShipInfoCloneTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}

  protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		// ship classes create their default particle effects on construction
		particle::ParticleManager::init();
	}
	void TearDown() override
	{
		particle::ParticleManager::shutdown();
		test::FSTestFixture::TearDown();
	}
};

TEST_F(ShipInfoCloneTest, copies_members) {
	ship_info src;
	strcpy_s(src.name, "Template Fighter");
	src.display_name = "Shown Name";
	src.type_str = "Fighter";
	src.desc = "a description";
	src.ship_length = "20 Meters";
	src.max_vel.xyz.z = 75.0f;
	src.flags.set(Ship::Info_Flags::Fighter);
	src.custom_data["key"] = "value";

	src.subsystems.resize(2);
	src.n_subsystems = 2;
	strcpy_s(src.subsystems[1].subobj_name, "turret01");

	src.hud_gauges.push_back(SCP_polymorphic<HudGauge>::make<HudGauge>());

	ship_info dest;
	dest.clone(src);

	EXPECT_STREQ(dest.name, "Template Fighter");
	EXPECT_EQ(dest.display_name, "Shown Name");
	EXPECT_EQ(dest.type_str, "Fighter");
	EXPECT_EQ(dest.desc, "a description");
	EXPECT_EQ(dest.ship_length, "20 Meters");
	EXPECT_EQ(dest.max_vel.xyz.z, 75.0f);
	EXPECT_TRUE(dest.flags[Ship::Info_Flags::Fighter]);
	EXPECT_EQ(dest.custom_data, src.custom_data);

	// subsystems and HUD gauges are deep copies
	ASSERT_EQ(dest.n_subsystems, 2);
	ASSERT_EQ(dest.subsystems.size(), 2u);
	EXPECT_STREQ(dest.subsystems[1].subobj_name, "turret01");
	EXPECT_NE(dest.subsystems.data(), src.subsystems.data());

	ASSERT_EQ(dest.hud_gauges.size(), 1u);
	EXPECT_NE(dest.hud_gauges[0].get(), src.hud_gauges[0].get());
}
