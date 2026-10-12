#include <gtest/gtest.h>

#include "mission/missionparse.h"

TEST(MissionParseTest, parse_date) {
	ASSERT_EQ(mission_parse_date("12/17/19 at 14:00:00"), 20191217);
	ASSERT_EQ(mission_parse_date("06/02/99 at 13:21:55"), 19990602);
	ASSERT_EQ(mission_parse_date("01/31/27 at 23:59:59"), 20270131);
	ASSERT_EQ(mission_parse_date("02/01/27 at 00:00:00"), 20270201);

	ASSERT_EQ(mission_parse_date(""), -1);
	ASSERT_EQ(mission_parse_date("garbage"), -1);
	ASSERT_EQ(mission_parse_date("13/01/20 at 00:00:00"), -1);
}
