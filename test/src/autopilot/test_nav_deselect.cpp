#include <gtest/gtest.h>

#include <iterator>

#include "ai/ai.h"
#include "autopilot/autopilot.h"
#include "playerman/player.h"
#include "ship/ship.h"

// defined in autopilot.cpp, not in its header
extern bool CinematicStarted;
// defined in aicode.cpp, not in its header
extern object* Autopilot_flight_leader;

class NavDeselectTest : public ::testing::Test {
protected:
	int saved_current_nav = -1;
	bool saved_engaged = false;
	NavPoint saved_navs[MAX_NAVPOINTS];

	void SetUp() override {
		saved_current_nav = CurrentNav;
		saved_engaged = AutoPilotEngaged;
		AutoPilotEngaged = false;
		for (int i = 0; i < MAX_NAVPOINTS; ++i) {
			saved_navs[i] = Navs[i];
			Navs[i].clear();
		}
		CurrentNav = -1;

		strcpy_s(Navs[0].m_NavName, "First");
		Navs[0].flags = NP_SHIP;
		Navs[0].target_index = 0;
		strcpy_s(Navs[1].m_NavName, "Second");
		Navs[1].flags = NP_SHIP;
		Navs[1].target_index = 1;
	}

	void TearDown() override {
		CurrentNav = saved_current_nav;
		AutoPilotEngaged = saved_engaged;
		for (int i = 0; i < MAX_NAVPOINTS; ++i) {
			Navs[i] = saved_navs[i];
		}
	}
};

// EndAutoPilot scans every ship and wing.
class NavDeselectEngagedTest : public NavDeselectTest {
protected:
	bool saved_player_ai = false;
	bool saved_cinematic = false;
	object* saved_leader = nullptr;
	static constexpr int target_count = 3;
	int saved_target_instances[target_count];
	char saved_target_names[target_count][NAME_LENGTH];
	int saved_carrier_ai_index = -1;
	int saved_carrier_wingnum = -1;
	flagset<Ship::Ship_Flags> saved_carrier_flags;
	int saved_active_goal = AI_ACTIVE_GOAL_NONE;
	int saved_speed_cap = -1;
	int saved_ai_mode = AIM_NONE;
	ai_goal saved_goals[MAX_AI_GOALS];
	int saved_ship_objnums[MAX_SHIPS];
	flagset<Ship::Wing_Flags> saved_wing_flags[MAX_WINGS];

	void SetUp() override {
		NavDeselectTest::SetUp();
		saved_player_ai = Player_use_ai;
		saved_cinematic = CinematicStarted;
		saved_leader = Autopilot_flight_leader;
		for (int i = 0; i < target_count; ++i) {
			saved_target_instances[i] = Objects[i].instance;
			strcpy_s(saved_target_names[i], Ships[i].ship_name);
			Objects[i].instance = i;
		}
		strcpy_s(Ships[0].ship_name, "First target");
		strcpy_s(Ships[1].ship_name, "Second target");
		strcpy_s(Ships[2].ship_name, "Third target");
		for (int i = 0; i < MAX_SHIPS; ++i) {
			saved_ship_objnums[i] = Ships[i].objnum;
			Ships[i].objnum = -1;
		}
		for (int i = 0; i < MAX_WINGS; ++i) {
			saved_wing_flags[i] = Wings[i].flags;
			Wings[i].flags.remove(Ship::Wing_Flags::Nav_carry);
		}
		auto& carrier = Ships[2];
		saved_carrier_ai_index = carrier.ai_index;
		saved_carrier_wingnum = carrier.wingnum;
		saved_carrier_flags = carrier.flags;
		carrier.objnum = 2;
		carrier.ai_index = 0;
		carrier.wingnum = -1;
		carrier.flags.set(Ship::Ship_Flags::Navpoint_carry);

		auto& ai = Ai_info[0];
		saved_active_goal = ai.active_goal;
		saved_speed_cap = ai.waypoint_speed_cap;
		saved_ai_mode = ai.mode;
		ai.active_goal = AI_ACTIVE_GOAL_NONE;
		for (int i = 0; i < MAX_AI_GOALS; ++i) {
			saved_goals[i] = ai.goals[i];
			ai_goal_reset(&ai.goals[i]);
		}
		ai.goals[0].ai_mode = AI_GOAL_FLY_TO_SHIP;
		ai.goals[0].target_name = Ships[1].ship_name;
		ai.goals[1].ai_mode = AI_GOAL_FLY_TO_SHIP;
		ai.goals[1].target_name = "Control target";
		CinematicStarted = false;
	}

	static void EngageOnSecond() {
		SelectNav("Second");
		AutoPilotEngaged = true;
		Player_use_ai = true;
		Autopilot_flight_leader = &Objects[2];
	}

	static void ExpectOnlyNavGoalRemoved() {
		EXPECT_EQ(AI_GOAL_NONE, Ai_info[0].goals[0].ai_mode);
		EXPECT_EQ(AI_GOAL_FLY_TO_SHIP, Ai_info[0].goals[1].ai_mode);
		EXPECT_STREQ("Control target", Ai_info[0].goals[1].target_name);
	}

	void TearDown() override {
		Player_use_ai = saved_player_ai;
		CinematicStarted = saved_cinematic;
		Autopilot_flight_leader = saved_leader;
		for (int i = 0; i < target_count; ++i) {
			Objects[i].instance = saved_target_instances[i];
			strcpy_s(Ships[i].ship_name, saved_target_names[i]);
		}
		auto& carrier = Ships[2];
		carrier.ai_index = saved_carrier_ai_index;
		carrier.wingnum = saved_carrier_wingnum;
		carrier.flags = saved_carrier_flags;
		auto& ai = Ai_info[0];
		ai.active_goal = saved_active_goal;
		ai.waypoint_speed_cap = saved_speed_cap;
		ai.mode = saved_ai_mode;
		for (int i = 0; i < MAX_AI_GOALS; ++i)
			ai.goals[i] = saved_goals[i];
		for (int i = 0; i < MAX_SHIPS; ++i)
			Ships[i].objnum = saved_ship_objnums[i];
		for (int i = 0; i < MAX_WINGS; ++i)
			Wings[i].flags = saved_wing_flags[i];
		NavDeselectTest::TearDown();
	}
};

TEST_F(NavDeselectEngagedTest, EndsEngagedAutopilotAndClearsSelection) {
	EngageOnSecond();
	ASSERT_EQ(1, CurrentNav);

	DeselectNav();

	EXPECT_EQ(-1, CurrentNav);
	EXPECT_FALSE(AutoPilotEngaged);
	EXPECT_FALSE(Player_use_ai);
	EXPECT_EQ(nullptr, Autopilot_flight_leader);
	ExpectOnlyNavGoalRemoved();
}

TEST_F(NavDeselectEngagedTest, SelectingDifferentNavEndsEngagedAutopilot) {
	EngageOnSecond();
	ASSERT_EQ(1, CurrentNav);

	SelectNav("First");

	EXPECT_EQ(0, CurrentNav);
	EXPECT_FALSE(AutoPilotEngaged);
	EXPECT_FALSE(Player_use_ai);
	ExpectOnlyNavGoalRemoved();
}

TEST_F(NavDeselectEngagedTest, ReselectingSameNavKeepsAutopilotEngaged) {
	// An earlier nav with the same name must not end autopilot on the way to the last match.
	strcpy_s(Navs[0].m_NavName, "Second");
	EngageOnSecond();
	ASSERT_EQ(1, CurrentNav);

	SelectNav("Second");

	EXPECT_EQ(1, CurrentNav);
	EXPECT_TRUE(AutoPilotEngaged);
	EXPECT_TRUE(Player_use_ai);
	EXPECT_EQ(AI_GOAL_FLY_TO_SHIP, Ai_info[0].goals[0].ai_mode);
	EXPECT_STREQ(Ships[1].ship_name, Ai_info[0].goals[0].target_name);
}

class NavDeleteTest : public NavDeselectTest {
protected:
	void SetUp() override {
		NavDeselectTest::SetUp();
		strcpy_s(Navs[2].m_NavName, "Third");
		Navs[2].flags = NP_SHIP;
		Navs[2].target_index = 2;
	}
};

TEST_F(NavDeleteTest, CompactsFirstMiddleAndLastWithoutDuplicates) {
	const NavPoint original[] = {Navs[0], Navs[1], Navs[2]};
	const int count = static_cast<int>(std::size(original));

	for (int removed = 0; removed < count; ++removed) {
		SCOPED_TRACE(removed);
		for (int i = 0; i < count; ++i) {
			Navs[i] = original[i];
		}

		ASSERT_TRUE(DelNavPoint(removed));

		for (int i = 0; i < count - 1; ++i) {
			EXPECT_STREQ(original[i < removed ? i : i + 1].m_NavName, Navs[i].m_NavName) << "slot " << i;
		}
		EXPECT_STREQ("", Navs[count - 1].m_NavName);
	}
}

TEST_F(NavDeselectEngagedTest, DeletingEarlierNavKeepsAutopilotEngaged) {
	EngageOnSecond();
	ASSERT_EQ(1, CurrentNav);

	ASSERT_TRUE(DelNavPoint("First"));

	EXPECT_EQ(0, CurrentNav);
	EXPECT_STREQ("Second", Navs[0].m_NavName);
	EXPECT_TRUE(AutoPilotEngaged);
	EXPECT_TRUE(Player_use_ai);
	EXPECT_EQ(AI_GOAL_FLY_TO_SHIP, Ai_info[0].goals[0].ai_mode);
	EXPECT_STREQ(Ships[1].ship_name, Ai_info[0].goals[0].target_name);
}

TEST_F(NavDeselectEngagedTest, DeletingSelectedNavEndsEngagedAutopilot) {
	// Keep the shifted-in target valid so a late shutdown fails on goal cleanup.
	strcpy_s(Navs[2].m_NavName, "Third");
	Navs[2].flags = NP_SHIP;
	Navs[2].target_index = 2;
	EngageOnSecond();
	ASSERT_EQ(1, CurrentNav);

	ASSERT_TRUE(DelNavPoint("Second"));

	EXPECT_EQ(-1, CurrentNav);
	EXPECT_FALSE(AutoPilotEngaged);
	EXPECT_FALSE(Player_use_ai);
	EXPECT_EQ(nullptr, Autopilot_flight_leader);
	ExpectOnlyNavGoalRemoved();
}
