#include <gtest/gtest.h>

#include <cstdio>
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

	void EngageOnSecond() {
		SelectNav("Second");
		AutoPilotEngaged = true;
		Player_use_ai = true;
		Autopilot_flight_leader = &Objects[2];
	}

	void ExpectOnlyNavGoalRemoved() {
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
	DeselectNav();
	EXPECT_EQ(-1, CurrentNav);
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
	EngageOnSecond();
	ASSERT_EQ(1, CurrentNav);

	SelectNav("Second");

	EXPECT_EQ(1, CurrentNav);
	EXPECT_TRUE(AutoPilotEngaged);
	EXPECT_TRUE(Player_use_ai);
	EXPECT_EQ(AI_GOAL_FLY_TO_SHIP, Ai_info[0].goals[0].ai_mode);
	EXPECT_STREQ(Ships[1].ship_name, Ai_info[0].goals[0].target_name);
}

TEST_F(NavDeselectTest, ClearsEachSelectedNavAndRepeatedDeselect) {
	SelectNav("First");
	ASSERT_EQ(0, CurrentNav);
	DeselectNav();
	EXPECT_EQ(-1, CurrentNav);

	SelectNav("Second");
	ASSERT_EQ(1, CurrentNav);
	DeselectNav();
	EXPECT_EQ(-1, CurrentNav);
	DeselectNav();
	EXPECT_EQ(-1, CurrentNav);
}

TEST_F(NavDeselectTest, NoSelectionKeepsCanAutopilotDisabled) {
	DeselectNav();
	ASSERT_EQ(-1, CurrentNav);
	EXPECT_FALSE(CanAutopilot(&vmd_zero_vector, false));
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

static void ExpectNavPointEquals(const NavPoint& expected, const NavPoint& actual) {
	EXPECT_STREQ(expected.m_NavName, actual.m_NavName);
	EXPECT_EQ(expected.flags, actual.flags);
	EXPECT_EQ(expected.target_index, actual.target_index);
	EXPECT_EQ(expected.waypoint_num, actual.waypoint_num);
	for (size_t i = 0; i < std::size(expected.normal_color); ++i) {
		SCOPED_TRACE(i);
		EXPECT_EQ(expected.normal_color[i], actual.normal_color[i]);
		EXPECT_EQ(expected.visited_color[i], actual.visited_color[i]);
	}
}

TEST_F(NavDeleteTest, CompactsFirstMiddleAndLastWithoutDuplicates) {
	NavPoint original[] = {Navs[0], Navs[1], Navs[2]};
	const int count = static_cast<int>(std::size(original));
	for (int i = 0; i < count; ++i) {
		auto& nav = original[i];
		nav.flags = NP_WAYPOINT;
		nav.waypoint_num = i + 1;
		for (size_t channel = 0; channel < std::size(nav.normal_color); ++channel) {
			nav.normal_color[channel] = static_cast<ubyte>(10 * i + channel);
			nav.visited_color[channel] = static_cast<ubyte>(100 + 10 * i + channel);
		}
	}
	const NavPoint cleared;

	for (int removed = 0; removed < count; ++removed) {
		SCOPED_TRACE(removed);
		for (int i = 0; i < count; ++i) {
			Navs[i] = original[i];
		}

		ASSERT_TRUE(DelNavPoint(removed));

		for (int i = 0; i < count - 1; ++i) {
			SCOPED_TRACE(i);
			const auto& expected = original[i < removed ? i : i + 1];
			ExpectNavPointEquals(expected, Navs[i]);
		}
		for (int i = count - 1; i < MAX_NAVPOINTS; ++i) {
			SCOPED_TRACE(i);
			ExpectNavPointEquals(cleared, Navs[i]);
		}
	}
}

TEST_F(NavDeleteTest, ClearsOccupiedFinalSlotWithoutChangingOtherSlots) {
	NavPoint original[MAX_NAVPOINTS];
	for (int i = 0; i < MAX_NAVPOINTS; ++i) {
		std::snprintf(Navs[i].m_NavName, sizeof(Navs[i].m_NavName), "Nav %d", i);
		Navs[i].flags = NP_WAYPOINT;
		Navs[i].target_index = i;
		Navs[i].waypoint_num = i + 1;
		original[i] = Navs[i];
	}

	const int last = MAX_NAVPOINTS - 1;
	ASSERT_TRUE(DelNavPoint(last));

	for (int i = 0; i < last; ++i) {
		SCOPED_TRACE(i);
		ExpectNavPointEquals(original[i], Navs[i]);
	}
	ExpectNavPointEquals(NavPoint{}, Navs[last]);
}

TEST_F(NavDeleteTest, SelectionFollowsNavAfterEarlierDeletion) {
	SelectNav("Third");
	ASSERT_EQ(2, CurrentNav);

	ASSERT_TRUE(DelNavPoint("First"));

	EXPECT_EQ(1, CurrentNav);
	EXPECT_STREQ("Third", Navs[1].m_NavName);
}

TEST_F(NavDeleteTest, DeletingTheSelectedNavClearsSelection) {
	SelectNav("First");
	ASSERT_EQ(0, CurrentNav);

	ASSERT_TRUE(DelNavPoint("First"));

	EXPECT_EQ(-1, CurrentNav);
	EXPECT_STREQ("Second", Navs[0].m_NavName);
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
