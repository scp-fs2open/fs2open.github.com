#include <gtest/gtest.h>

#include "util/FSTestFixture.h"

#include "io/timer.h"
#include "utils/Random.h"

class CheckpointClockTest : public test::FSTestFixture {
  public:
	CheckpointClockTest() : test::FSTestFixture(INIT_NONE) {}

  protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		// make sure the timestamp clock is running, as it is during a mission, and starts from zero; the timer is
		// re-initialized between tests, which leaves the clock offsets of the previous test behind
		timestamp_pause(true);
		timestamp_unpause(true);
		timestamp_restore_checkpoint_state({});
		timer_start_frame();
		timestamp_start_mission();
	}
};

TEST_F(CheckpointClockTest, restore_moves_clock_backward)
{
	auto saved = timestamp_get_checkpoint_state();
	auto saved_now = timestamp();

	timestamp_adjust_seconds(30.0f, TIMER_DIRECTION::FORWARD);
	timer_start_frame();
	ASSERT_GE(timestamp(), saved_now + 30000);

	timestamp_restore_checkpoint_state(saved);

	// allow a little real time to pass between the calls
	EXPECT_GE(timestamp(), saved_now);
	EXPECT_LT(timestamp(), saved_now + 1000);
}

TEST_F(CheckpointClockTest, restore_moves_clock_forward)
{
	// a checkpoint saved later in a longer session than the current one
	auto saved = timestamp_get_checkpoint_state();
	saved.microseconds += 600 * MICROSECONDS_PER_SECOND;
	auto saved_now = static_cast<int>(saved.microseconds / MICROSECONDS_PER_MILLISECOND);

	timestamp_restore_checkpoint_state(saved);

	EXPECT_GE(timestamp(), saved_now);
	EXPECT_LT(timestamp(), saved_now + 1000);

	// the mission time follows: ten minutes into the mission
	EXPECT_GE(timestamp_get_mission_time_in_milliseconds(), 600 * 1000);
	EXPECT_LT(timestamp_get_mission_time_in_milliseconds(), 601 * 1000);
}

TEST_F(CheckpointClockTest, restore_while_paused)
{
	auto saved = timestamp_get_checkpoint_state();
	auto saved_now = timestamp();

	timestamp_adjust_seconds(30.0f, TIMER_DIRECTION::FORWARD);
	timestamp_pause(true);
	timestamp_restore_checkpoint_state(saved);

	EXPECT_GE(timestamp(), saved_now);
	EXPECT_LT(timestamp(), saved_now + 1000);

	timestamp_unpause(true);
	timer_start_frame();
	EXPECT_GE(timestamp(), saved_now);
	EXPECT_LT(timestamp(), saved_now + 1000);
}

TEST_F(CheckpointClockTest, saved_timestamps_keep_their_remaining_time)
{
	auto stamp = _timestamp(5000);
	auto saved = timestamp_get_checkpoint_state();

	timestamp_adjust_seconds(10.0f, TIMER_DIRECTION::FORWARD);
	timer_start_frame();
	ASSERT_TRUE(timestamp_elapsed(stamp));

	timestamp_restore_checkpoint_state(saved);

	EXPECT_FALSE(timestamp_elapsed(stamp));
	EXPECT_GT(timestamp_until(stamp), 4000);
}

TEST(CheckpointRandom, state_round_trip)
{
	util::Random::seed(12345);
	util::Random::next();
	auto state = util::Random::save_state();

	int expected[5];
	for (int& value : expected)
		value = util::Random::next();

	util::Random::seed(999);
	util::Random::load_state(state);

	for (int value : expected)
		EXPECT_EQ(util::Random::next(), value);
}
