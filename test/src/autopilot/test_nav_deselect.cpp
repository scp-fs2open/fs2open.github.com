#include <gtest/gtest.h>

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

// Exercise actual shutdown with a backed nav target, without importing unrelated
// live ships/wing goals from other tests into EndAutoPilot's global scans.
class NavDeselectEngagedTest : public NavDeselectTest {
protected:
	bool saved_player_ai = false;
	bool saved_cinematic = false;
	object* saved_leader = nullptr;
	int saved_target_instance = -1;
	char saved_target_name[NAME_LENGTH];
	int saved_ship_objnums[MAX_SHIPS];
	flagset<Ship::Wing_Flags> saved_wing_flags[MAX_WINGS];

	void SetUp() override {
		NavDeselectTest::SetUp();
		saved_player_ai = Player_use_ai;
		saved_cinematic = CinematicStarted;
		saved_leader = Autopilot_flight_leader;
		saved_target_instance = Objects[1].instance;
		strcpy_s(saved_target_name, Ships[0].ship_name);
		for (int i = 0; i < MAX_SHIPS; ++i) {
			saved_ship_objnums[i] = Ships[i].objnum;
			Ships[i].objnum = -1;
		}
		for (int i = 0; i < MAX_WINGS; ++i) {
			saved_wing_flags[i] = Wings[i].flags;
			Wings[i].flags.remove(Ship::Wing_Flags::Nav_carry);
		}
		Objects[1].instance = 0;
		strcpy_s(Ships[0].ship_name, "Deselect shutdown target");
		CinematicStarted = false;
	}

	void TearDown() override {
		Player_use_ai = saved_player_ai;
		CinematicStarted = saved_cinematic;
		Autopilot_flight_leader = saved_leader;
		Objects[1].instance = saved_target_instance;
		strcpy_s(Ships[0].ship_name, saved_target_name);
		for (int i = 0; i < MAX_SHIPS; ++i)
			Ships[i].objnum = saved_ship_objnums[i];
		for (int i = 0; i < MAX_WINGS; ++i)
			Wings[i].flags = saved_wing_flags[i];
		NavDeselectTest::TearDown();
	}
};

TEST_F(NavDeselectEngagedTest, EndsEngagedAutopilotAndClearsSelection) {
	// Goal removal is not exercised because no ship or wing carries a nav goal.
	SelectNav("Second");
	ASSERT_EQ(1, CurrentNav);
	AutoPilotEngaged = true;
	Player_use_ai = true;
	Autopilot_flight_leader = &Objects[1];

	DeselectNav();

	EXPECT_EQ(-1, CurrentNav);
	EXPECT_FALSE(AutoPilotEngaged);
	EXPECT_FALSE(Player_use_ai);
	EXPECT_EQ(nullptr, Autopilot_flight_leader);
	DeselectNav();
	EXPECT_EQ(-1, CurrentNav);
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
