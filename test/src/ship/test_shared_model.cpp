
#include <gtest/gtest.h>

#include "util/FSTestFixture.h"
#include "util/test_util.h"

#include "iff_defs/iff_defs.h"
#include "ship/ship.h"
#include "species_defs/species_defs.h"

class ShipSharedModelTest : public test::FSTestFixture
{
 public:
	ShipSharedModelTest() : test::FSTestFixture(INIT_CFILE)
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

TEST_F(ShipSharedModelTest, identical_subsystems)
{
	DEBUG_TEST();

	ASSERT_NO_THROW(ship_init());
	ASSERT_EQ(Ship_info.size(), 2u);
}

TEST_F(ShipSharedModelTest, reordered_subsystems)
{
	DEBUG_TEST();

	ASSERT_THROW(ship_init(), os::dialogs::WarningException);
}

TEST_F(ShipSharedModelTest, extra_subsystem)
{
	DEBUG_TEST();

	ASSERT_THROW(ship_init(), os::dialogs::WarningException);
}

TEST_F(ShipSharedModelTest, no_subsystems)
{
	DEBUG_TEST();

	ASSERT_THROW(ship_init(), os::dialogs::WarningException);
}
