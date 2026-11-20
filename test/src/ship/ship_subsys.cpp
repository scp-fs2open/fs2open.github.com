#include <gtest/gtest.h>

#include "util/FSTestFixture.h"

#include <memory>

#include "globalincs/linklist.h"
#include "osapi/dialogs.h"
#include "particle/ParticleManager.h"
#include "ship/ship.h"

class ShipSubsysIndexTest : public test::FSTestFixture {
  public:
	ShipSubsysIndexTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}

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

TEST_F(ShipSubsysIndexTest, out_of_range_index_warns_once_and_is_null)
{
	Ship_info.emplace_back();
	Ship_info.back().n_subsystems = 2;

	auto shipp = std::make_unique<ship>();
	shipp->clear();
	shipp->ship_info_index = static_cast<int>(Ship_info.size()) - 1;

	ship_subsys subsystems[2];
	list_init(&shipp->subsys_list);
	list_append(&shipp->subsys_list, &subsystems[0]);
	list_append(&shipp->subsys_list, &subsystems[1]);

	EXPECT_EQ(ship_get_indexed_subsys(shipp.get(), 0), &subsystems[0]);
	EXPECT_EQ(ship_get_indexed_subsys(shipp.get(), 1), &subsystems[1]);

	// a multiplayer packet can name a subsystem this ship does not have; warnings throw under the test runner
	EXPECT_THROW(ship_get_indexed_subsys(shipp.get(), 2), os::dialogs::WarningException);

	// the warning is given once per ship, and the answer stays nullptr
	EXPECT_EQ(ship_get_indexed_subsys(shipp.get(), 2), nullptr);
	EXPECT_EQ(ship_get_indexed_subsys(shipp.get(), 1000), nullptr);
	EXPECT_EQ(ship_get_indexed_subsys(shipp.get(), -1), nullptr);
	EXPECT_EQ(ship_get_indexed_subsys(shipp.get(), 1), &subsystems[1]);

	list_init(&shipp->subsys_list);
	shipp.reset();
	Ship_info.pop_back();
}
