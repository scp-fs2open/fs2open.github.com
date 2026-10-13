#include "iff_defs/iff_defs.h"
#include "libs/jansson.h"
#include "mission/missionjournal.h"
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

namespace {
log_entry journal_test_entry(const char* primary)
{
	log_entry entry{};
	entry.type = LOG_SHIP_DESTROYED;
	strcpy_s(entry.pname, primary);
	strcpy_s(entry.sname, "Alpha 1");
	entry.index = -1;
	entry.timestamp = fl2f(81.25f);
	entry.primary_team = -1;
	entry.secondary_team = -1;
	return entry;
}
} // namespace

TEST(MissionJournal, EntryIsOneJsonLine)
{
	ASSERT_EQ(mission_journal_format_entry(journal_test_entry("Leo 1"), "SM1-01", "2026-10-13T15:00:00Z"),
		"{\"timestamp\":\"2026-10-13T15:00:00Z\",\"event\":\"ShipDestroyed\",\"Mission\":\"SM1-01\","
		"\"MissionTime\":81.25,\"Primary\":\"Leo 1\",\"Secondary\":\"Alpha 1\",\"Index\":-1,\"Hidden\":false}");
}

TEST(MissionJournal, NonUtf8NamesBecomeUtf8)
{
	const auto line = mission_journal_format_entry(journal_test_entry("Z\xfcrich"), "SM1-01", "2026-10-13T15:00:00Z");
	json_error_t error;
	std::unique_ptr<json_t> parsed(json_loads(line.c_str(), 0, &error));
	ASSERT_NE(parsed, nullptr) << error.text;
	ASSERT_STREQ(json_string_value(json_object_get(parsed.get(), "Primary")), "Z\xc3\xbcrich");
}

TEST(MissionJournal, TeamsHiddenAndNamesAreWritten)
{
	Iff_info.emplace_back();
	strcpy_s(Iff_info.back().iff_name, "Friendly");
	const int team = static_cast<int>(Iff_info.size()) - 1;

	auto entry = journal_test_entry("Leo 1");
	strcpy_s(entry.sname, "Z\xfcrich");
	entry.primary_team = team;
	entry.secondary_team = team;
	entry.flags = MLF_HIDDEN;
	entry.timestamp = fl2f(68.8364f);
	// "Entee" with a Latin-1 e-acute, and the same word in UTF-8
	const char latin1_mission[] = {'E', 'n', 't', static_cast<char>(0xe9), 'e', '\0'};
	const char utf8_mission[] = {'E', 'n', 't', static_cast<char>(0xc3), static_cast<char>(0xa9), 'e', '\0'};
	const auto line = mission_journal_format_entry(entry, latin1_mission, "2026-10-13T15:00:00Z");
	Iff_info.pop_back();

	json_error_t error;
	std::unique_ptr<json_t> parsed(json_loads(line.c_str(), 0, &error));
	ASSERT_NE(parsed, nullptr) << error.text;
	EXPECT_STREQ(json_string_value(json_object_get(parsed.get(), "PrimaryTeam")), "Friendly");
	EXPECT_STREQ(json_string_value(json_object_get(parsed.get(), "SecondaryTeam")), "Friendly");
	EXPECT_TRUE(json_is_true(json_object_get(parsed.get(), "Hidden")));
	EXPECT_STREQ(json_string_value(json_object_get(parsed.get(), "Secondary")), "Z\xc3\xbcrich");
	EXPECT_STREQ(json_string_value(json_object_get(parsed.get(), "Mission")), utf8_mission);
	EXPECT_NE(line.find("\"MissionTime\":68.8364,"), SCP_string::npos) << line;
}
