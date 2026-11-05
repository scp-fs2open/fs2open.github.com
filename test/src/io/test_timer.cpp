#include "io/timer.h"
#include "utils/finally.h"

#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

class TimerTest : public test::FSTestFixture {
  public:
	TimerTest() : test::FSTestFixture(INIT_NONE) {}
};

TEST_F(TimerTest, stampsSetAfterTwelveDaysStillWait)
{
	// 12.5 days: past INT_MAX / 2 ms, where new stamps wrapped, and still below INT_MAX ms
	constexpr uint64_t jump_us = 1'080'000'000'000ULL;
	constexpr uint64_t step_us = 600'000ULL;

	timestamp_adjust_microseconds(jump_us, TIMER_DIRECTION::FORWARD);
	bool stepped = false;
	// undone in the same steps, since each adjustment rounds to whole counter ticks on its own
	auto restore_clock = util::finally([&stepped] {
		if (stepped) {
			timestamp_adjust_microseconds(step_us, TIMER_DIRECTION::BACKWARD);
		}
		timestamp_adjust_microseconds(jump_us, TIMER_DIRECTION::BACKWARD);
		timer_start_frame();
	});
	timer_start_frame();
	ASSERT_GT(timestamp(), 1'073'741'823);

	const int old_style = timestamp(500);
	const TIMESTAMP new_style = _timestamp(500);
	EXPECT_FALSE(timestamp_elapsed(old_style));
	EXPECT_FALSE(timestamp_elapsed(new_style));
	EXPECT_FALSE(timestamp_elapsed_safe(old_style, 2500));
	EXPECT_FALSE(timestamp_elapsed_safe(new_style, 2500));
	EXPECT_GT(timestamp_until(new_style), 0);
	EXPECT_LE(timestamp_until(new_style), 500);

	timestamp_adjust_microseconds(step_us, TIMER_DIRECTION::FORWARD);
	stepped = true;
	timer_start_frame();

	EXPECT_TRUE(timestamp_elapsed(old_style));
	EXPECT_TRUE(timestamp_elapsed(new_style));
	EXPECT_TRUE(timestamp_elapsed_safe(old_style, 2500));
	EXPECT_TRUE(timestamp_elapsed_safe(new_style, 2500));
	EXPECT_GE(timestamp_since(new_style), 100);
}
