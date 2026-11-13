#include <gtest/gtest.h>

#include "mission/missionlog.h"
#include "object/object.h"
#include "ship/ship.h"

// defined in missionlog.cpp but not declared in its header
extern SCP_vector<log_entry> Log_entries;

class MissionLogTest : public ::testing::Test {
  protected:
	void SetUp() override
	{
		obj_init();
		mission_log_init();

		add_ship(0, "Alpha 1", 2);
		add_ship(1, "Virgo", 5);
		Ships[1].display_name = "Virgo Display";
		Ships[1].flags.set(Ship::Ship_Flags::Has_display_name);
	}

	void TearDown() override
	{
		mission_log_init();
		Ships[0].clear();
		Ships[1].clear();
		obj_init();
	}

	static void add_ship(int shipnum, const char* name, int team)
	{
		Ships[shipnum].clear();
		strcpy_s(Ships[shipnum].ship_name, name);
		Ships[shipnum].team = team;
		Ships[shipnum].objnum = shipnum;
		Objects[shipnum].type = OBJ_SHIP;
		Objects[shipnum].instance = shipnum;
	}
};

TEST_F(MissionLogTest, names_without_ships_are_looked_up)
{
	mission_log_add_entry(LOG_SHIP_DESTROYED, "Alpha 1", "Virgo", 50);

	const auto& entry = Log_entries.back();
	EXPECT_EQ(entry.primary_team, 2);
	EXPECT_EQ(entry.pname_display, "Alpha 1");
	EXPECT_EQ(entry.secondary_team, 5);
	EXPECT_EQ(entry.sname_display, "Virgo Display");
}

// multiplayer logs callsigns, and a callsign can match some other ship's name
TEST_F(MissionLogTest, passed_ships_win_over_names)
{
	mission_log_add_entry(LOG_SHIP_DESTROYED, "Virgo", "Alpha 1", 50, 0, 0, 1);

	const auto& entry = Log_entries.back();
	EXPECT_STREQ(entry.pname, "Virgo");
	EXPECT_STREQ(entry.sname, "Alpha 1");
	EXPECT_EQ(entry.index, 50);
	EXPECT_EQ(entry.primary_team, 2);
	EXPECT_EQ(entry.pname_display, "Alpha 1");
	EXPECT_EQ(entry.secondary_team, 5);
	EXPECT_EQ(entry.sname_display, "Virgo Display");
}

TEST_F(MissionLogTest, self_destruct_uses_passed_ship)
{
	mission_log_add_entry(LOG_SELF_DESTRUCTED, "Virgo", nullptr, -1, 0, 0);

	const auto& entry = Log_entries.back();
	EXPECT_STREQ(entry.pname, "Virgo");
	EXPECT_EQ(entry.primary_team, 2);
	EXPECT_EQ(entry.pname_display, "Alpha 1");
	EXPECT_EQ(entry.secondary_team, -1);
}
