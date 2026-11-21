#include <gtest/gtest.h>

#include "mission/missionparse.h"
#include "missioneditor/common.h"
#include "object/object.h"
#include "ship/ship.h"

// The selection groups of FRED2 and QtFRED. Each group is one bit, and -1 is "in no group".
TEST(EditorGroups, no_group_matches_no_group) {
	ship shipp;
	shipp.group = -1;

	for (int bit = 1; bit <= (1 << 8); bit <<= 1)
		EXPECT_FALSE(ship_in_editor_group(shipp, bit));
}

TEST(EditorGroups, groups_do_not_overlap) {
	ship shipp;
	shipp.group = -1;

	// Group 3 is bit 4. It must not match groups 1 and 2.
	ship_set_editor_group(shipp, 4, true);
	EXPECT_EQ(shipp.group, 4);
	EXPECT_TRUE(ship_in_editor_group(shipp, 4));
	EXPECT_FALSE(ship_in_editor_group(shipp, 1));
	EXPECT_FALSE(ship_in_editor_group(shipp, 2));

	ship_set_editor_group(shipp, 1, true);
	EXPECT_EQ(shipp.group, 5);
	EXPECT_TRUE(ship_in_editor_group(shipp, 1));
	EXPECT_TRUE(ship_in_editor_group(shipp, 4));
}

TEST(EditorGroups, last_group_removed_gives_no_group) {
	ship shipp;
	shipp.group = -1;
	ship_set_editor_group(shipp, 2, true);
	ship_set_editor_group(shipp, 8, true);

	ship_set_editor_group(shipp, 2, false);
	EXPECT_EQ(shipp.group, 8);

	// The mission save writes +Group for 0, so the ship must go back to -1.
	ship_set_editor_group(shipp, 8, false);
	EXPECT_EQ(shipp.group, -1);
}

TEST(EditorGroups, remove_from_no_group_keeps_no_group) {
	ship shipp;
	shipp.group = -1;

	ship_set_editor_group(shipp, 2, false);
	EXPECT_EQ(shipp.group, -1);
}

namespace {

// The minimum arrival distance that FRED2 and QtFRED share.
class ArrivalMinDistanceTest : public ::testing::Test {
  protected:
	static constexpr int SHIPNUM = 3;
	static constexpr int OBJNUM = 5;

	void SetUp() override {
		Ship_registry.clear();
		Ship_registry_map.clear();

		ship_registry_entry entry("Target");
		entry.shipnum = SHIPNUM;
		entry.objnum = OBJNUM;
		Ship_registry.push_back(entry);
		Ship_registry_map[entry.name] = 0;

		_oldObjnum = Ships[SHIPNUM].objnum;
		_oldRadius = Objects[OBJNUM].radius;
		Ships[SHIPNUM].objnum = OBJNUM;
	}

	void TearDown() override {
		Ships[SHIPNUM].objnum = _oldObjnum;
		Objects[OBJNUM].radius = _oldRadius;

		Ship_registry.clear();
		Ship_registry_map.clear();
	}

	static anchor_t targetAnchor() { return anchor_t(0); }

	int _oldObjnum = -1;
	float _oldRadius = 0.0f;
};

} // namespace

TEST_F(ArrivalMinDistanceTest, location_without_distance) {
	Objects[OBJNUM].radius = 100.0f;

	EXPECT_EQ(arrival_min_distance(ArrivalLocation::AT_LOCATION, targetAnchor()), 0);
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::FROM_DOCK_BAY, targetAnchor()), 0);
}

TEST_F(ArrivalMinDistanceTest, special_or_invalid_anchor) {
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::NEAR_SHIP, anchor_t(ANCHOR_SPECIAL_ARRIVAL)), 1);
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::NEAR_SHIP, anchor_t(ANCHOR_SPECIAL_ARRIVAL | ANCHOR_SPECIAL_ARRIVAL_PLAYER)), 1);
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::IN_FRONT_OF_SHIP, anchor_t::invalid()), 1);
}

TEST_F(ArrivalMinDistanceTest, twice_the_target_radius) {
	Objects[OBJNUM].radius = 100.0f;
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::NEAR_SHIP, targetAnchor()), 200);

	// The value is rounded, not truncated.
	Objects[OBJNUM].radius = 100.3f;
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::ABOVE_SHIP, targetAnchor()), 201);

	// The value is rounded down when the fraction is small. The ceiling gives 201.
	Objects[OBJNUM].radius = 100.2f;
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::ABOVE_SHIP, targetAnchor()), 200);
}

TEST_F(ArrivalMinDistanceTest, limit_of_500) {
	Objects[OBJNUM].radius = 1000.0f;
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::BELOW_SHIP, targetAnchor()), 500);
}

TEST_F(ArrivalMinDistanceTest, at_least_1) {
	Objects[OBJNUM].radius = 0.1f;
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::TO_LEFT_OF_SHIP, targetAnchor()), 1);
}

TEST_F(ArrivalMinDistanceTest, target_without_object) {
	Ships[SHIPNUM].objnum = -1;
	EXPECT_EQ(arrival_min_distance(ArrivalLocation::NEAR_SHIP, targetAnchor()), 1);
}
