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

namespace {
// undoes the global Ship_info entry and the stack-held subsystem links even when a check throws
struct TwoSubsystemShip {
	std::unique_ptr<ship> shipp = std::make_unique<ship>();
	ship_subsys subsystems[2];

	TwoSubsystemShip()
	{
		Ship_info.emplace_back();
		Ship_info.back().n_subsystems = 2;
		shipp->clear();
		shipp->ship_info_index = static_cast<int>(Ship_info.size()) - 1;
		list_init(&shipp->subsys_list);
		list_append(&shipp->subsys_list, &subsystems[0]);
		list_append(&shipp->subsys_list, &subsystems[1]);
	}
	~TwoSubsystemShip()
	{
		list_init(&shipp->subsys_list);
		shipp.reset();
		Ship_info.pop_back();
	}
	TwoSubsystemShip(const TwoSubsystemShip&) = delete;
	TwoSubsystemShip& operator=(const TwoSubsystemShip&) = delete;
};
} // namespace

TEST_F(ShipSubsysIndexTest, out_of_range_index_warns_once_and_is_null)
{
	TwoSubsystemShip s;
	auto sp = s.shipp.get();

	EXPECT_EQ(ship_get_indexed_subsys(sp, 0), &s.subsystems[0]);
	EXPECT_EQ(ship_get_indexed_subsys(sp, 1), &s.subsystems[1]);

	// a multiplayer packet can name a subsystem this ship does not have; warnings throw under the test runner
	EXPECT_THROW(ship_get_indexed_subsys(sp, 2), os::dialogs::WarningException);

	// the warning is given once per ship, and the answer stays nullptr
	EXPECT_EQ(ship_get_indexed_subsys(sp, 2), nullptr);
	EXPECT_EQ(ship_get_indexed_subsys(sp, 1000), nullptr);
#ifndef NDEBUG
	// a negative index cannot come from a class mismatch, so it stays a programming error
	EXPECT_THROW(ship_get_indexed_subsys(sp, -1), os::dialogs::AssertException);
#else
	EXPECT_EQ(ship_get_indexed_subsys(sp, -1), nullptr);
#endif
	EXPECT_EQ(ship_get_indexed_subsys(sp, 1), &s.subsystems[1]);
}

TEST_F(ShipSubsysIndexTest, copied_cache_flag_without_a_table_rebuilds_it)
{
	TwoSubsystemShip s;
	auto sp = s.shipp.get();

	// a flagset copied from another ship or machine can say the table is built when this ship has none
	sp->flags.set(Ship::Ship_Flags::Subsystem_cache_valid);
	ASSERT_EQ(sp->subsys_list_indexer, nullptr);

	EXPECT_EQ(ship_get_indexed_subsys(sp, 1), &s.subsystems[1]);
	EXPECT_EQ(s.subsystems[1].parent_subsys_index, 1);
}
