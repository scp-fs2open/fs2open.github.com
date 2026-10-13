#include "mission/missionlog.h"
#include "scripting/scripting.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

TEST(MissionLogTypeName, EveryTypeHasItsScriptName)
{
	const struct {
		LogType type;
		const char* name;
	} expected[] = {
		{LOG_SHIP_DESTROYED, "ShipDestroyed"},
		{LOG_WING_DESTROYED, "WingDestroyed"},
		{LOG_SHIP_ARRIVED, "ShipArrived"},
		{LOG_WING_ARRIVED, "WingArrived"},
		{LOG_SHIP_DEPARTED, "ShipDeparted"},
		{LOG_WING_DEPARTED, "WingDeparted"},
		{LOG_SHIP_DOCKED, "ShipDocked"},
		{LOG_SHIP_SUBSYS_DESTROYED, "SubsystemDestroyed"},
		{LOG_SHIP_UNDOCKED, "ShipUndocked"},
		{LOG_SHIP_DISABLED, "ShipDisabled"},
		{LOG_SHIP_DISARMED, "ShipDisarmed"},
		{LOG_PLAYER_CALLED_FOR_REARM, "PlayerCalledForRearm"},
		{LOG_PLAYER_CALLED_FOR_REINFORCEMENT, "PlayerCalledForReinforcement"},
		{LOG_GOAL_SATISFIED, "GoalSatisfied"},
		{LOG_GOAL_FAILED, "GoalFailed"},
		{LOG_PLAYER_ABORTED_REARM, "PlayerAbortedRearm"},
		{LOG_WAYPOINTS_DONE, "WaypointsDone"},
		{LOG_CARGO_REVEALED, "CargoRevealed"},
		{LOG_CAP_SUBSYS_CARGO_REVEALED, "CapSubsystemCargoRevealed"},
		{LOG_SELF_DESTRUCTED, "SelfDestructed"},
	};
	for (const auto& e : expected) {
		ASSERT_STREQ(mission_log_type_name(e.type), e.name);
	}
	ASSERT_STREQ(mission_log_type_name(static_cast<LogType>(0)), "Unknown");
	ASSERT_STREQ(mission_log_type_name(static_cast<LogType>(31)), "Unknown");
}

class MissionLogHookTest : public test::FSTestFixture {
  public:
	// running a Lua hook opens a graphics debug group in debug builds, so the stub renderer has to be up
	MissionLogHookTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}

  protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		// hooks only run once the engine has finished its game init
		Scripting_game_init_run = true;
		Script_system.CreateLuaState();
		mission_log_init();
		ASSERT_TRUE(Script_system.EvalString("LOGGED = {} engine.addHook('On Mission Log Entry', function() "
											 "LOGGED[#LOGGED + 1] = hv.Type .. ':' .. hv.Primary end)"));
		// the engine does this after parsing script tables; a hook added by a script only joins the active hooks then
		Script_system.ProcessAddedHooks();
	}

	void TearDown() override
	{
		mission_log_init();
		Script_system.Clear();
		Scripting_game_init_run = false;
		test::FSTestFixture::TearDown();
	}

	static int delivered()
	{
		int count = -1;
		EXPECT_TRUE(Script_system.EvalStringWithReturn("#LOGGED", "|i", &count));
		return count;
	}
};

TEST_F(MissionLogHookTest, EntriesWaitForTheDrainAndArriveOnceInOrder)
{
	mission_log_add_entry(LOG_GOAL_SATISFIED, "first", nullptr, 0);
	mission_log_add_entry(LOG_GOAL_FAILED, "second", nullptr, 1);
	EXPECT_EQ(delivered(), 0);

	mission_log_run_entry_hooks();
	ASSERT_EQ(delivered(), 2);
	bool in_order = false;
	ASSERT_TRUE(
		Script_system.EvalStringWithReturn("LOGGED[1] == 'GoalSatisfied:first' and LOGGED[2] == 'GoalFailed:second'",
			"|b",
			&in_order));
	EXPECT_TRUE(in_order);

	mission_log_run_entry_hooks();
	EXPECT_EQ(delivered(), 2);
}

TEST_F(MissionLogHookTest, NewMissionDropsUndeliveredEntries)
{
	mission_log_add_entry(LOG_GOAL_SATISFIED, "stale", nullptr, 0);
	mission_log_init();
	mission_log_run_entry_hooks();
	EXPECT_EQ(delivered(), 0);
}
