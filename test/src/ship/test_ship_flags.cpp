
#include <gtest/gtest.h>

#include "util/FSTestFixture.h"
#include "util/test_util.h"

#include "iff_defs/iff_defs.h"
#include "ship/ship.h"
#include "species_defs/species_defs.h"

class ShipFlagsTest : public test::FSTestFixture
{
 public:
	ShipFlagsTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("ship");
	}

 protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();

		// ship parsing reads species data, which needs the IFFs
		iff_init();
		species_init();
	}
	void TearDown() override
	{
		ship_close();
		iff_close();

		test::FSTestFixture::TearDown();
	}
};

// a modular table that replaces $Flags without +noreplace must keep the flags that $Flags itself cannot set
TEST_F(ShipFlagsTest, replaced_flags)
{
	DEBUG_TEST();

	ASSERT_NO_THROW(ship_init());
	ASSERT_EQ(Ship_info.size(), 1u);

	const auto &si = Ship_info[0];
	ASSERT_EQ(si.n_subsystems, 3);

	// the replacement itself took effect
	EXPECT_FALSE(si.flags[Ship::Info_Flags::In_tech_database]);
	EXPECT_TRUE(si.flags[Ship::Info_Flags::Stealth]);
	EXPECT_TRUE(si.subsystems[0].flags[Model::Subsystem_Flags::Carry_no_damage]);

	EXPECT_TRUE(si.flags[Ship::Info_Flags::Has_display_name]);
	EXPECT_TRUE(si.flags[Ship::Info_Flags::Afterburner]);
	EXPECT_TRUE(si.flags[Ship::Info_Flags::Has_awacs]);
	EXPECT_TRUE(si.flags[Ship::Info_Flags::Allow_landings]);

	EXPECT_TRUE(si.subsystems[0].flags[Model::Subsystem_Flags::Turret_barrel_fov_overridden]);
	EXPECT_TRUE(si.subsystems[0].flags[Model::Subsystem_Flags::Turret_base_fov_overridden]);
	EXPECT_TRUE(si.subsystems[0].flags[Model::Subsystem_Flags::Turret_max_fov_overridden]);
}
