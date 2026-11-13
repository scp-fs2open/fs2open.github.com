#include "mission/missionbriefcommon.h"

#include <gtest/gtest.h>

#include <cstring>

class BriefingMoveListTest : public ::testing::Test {
  protected:
	void SetUp() override
	{
		_savedBriefing = Briefing;

		for (int i = 0; i < 2; i++) {
			memset(&_icons[i], 0, sizeof(brief_icon));
			_icons[i].id = 1;
			_briefing.stages[i].icons = &_icons[i];
			_briefing.stages[i].num_icons = 1;
		}
		_icons[1].pos = vm_vec_new(100.0f, 0.0f, 0.0f);
		_briefing.num_stages = 2;

		Briefing = &_briefing;
		brief_move_icon_reset();
	}

	void TearDown() override
	{
		brief_move_icon_reset();
		Briefing = _savedBriefing;
	}

	briefing _briefing;
	brief_icon _icons[2];
	briefing* _savedBriefing = nullptr;
};

TEST_F(BriefingMoveListTest, timed_change_moves_the_icon)
{
	EXPECT_EQ(1, brief_set_move_list(1, 0, 0.5f));
}

TEST_F(BriefingMoveListTest, instant_change_builds_no_mover)
{
	EXPECT_EQ(0, brief_set_move_list(1, 0, 0.0f));
}

TEST_F(BriefingMoveListTest, negative_time_still_builds_a_mover)
{
	EXPECT_EQ(1, brief_set_move_list(1, 0, -1.0f));
}

TEST_F(BriefingMoveListTest, unmoved_icon_builds_no_mover)
{
	_icons[1].pos = _icons[0].pos;

	EXPECT_EQ(0, brief_set_move_list(1, 0, 0.5f));
}
