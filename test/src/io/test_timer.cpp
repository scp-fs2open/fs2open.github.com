#include "io/timer.h"
#include "util/FSTestFixture.h"
#include "utils/finally.h"

#include <gtest/gtest.h>

#include <climits>

class TimerTest : public test::FSTestFixture {
  public:
	TimerTest() : test::FSTestFixture(INIT_NONE) {}
};

namespace {
constexpr std::int64_t MS_2_31 = 2'147'483'648LL;
constexpr std::int64_t MS_2_32 = 4'294'967'296LL;
constexpr uint64_t US_PER_DAY = 86'400ULL * MICROSECONDS_PER_SECOND;
constexpr uint64_t STEP_US = 600'000ULL;

struct ClockJump {
	uint64_t days;
	std::int64_t boundary_ms;
};
} // namespace

TEST_F(TimerTest, gameStampsKeepWorkingPastThirtyTwoBits)
{
	for (const ClockJump jump : {ClockJump{25, MS_2_31}, ClockJump{50, MS_2_32}}) {
		SCOPED_TRACE(jump.days);
		const uint64_t jump_us = jump.days * US_PER_DAY;
		timestamp_adjust_microseconds(jump_us, TIMER_DIRECTION::FORWARD);
		bool stepped = false;
		// undone in the same steps, since each adjustment rounds to whole counter ticks on its own
		auto restore_clock = util::finally([&stepped, jump_us] {
			if (stepped) {
				timestamp_adjust_microseconds(STEP_US, TIMER_DIRECTION::BACKWARD);
			}
			timestamp_adjust_microseconds(jump_us, TIMER_DIRECTION::BACKWARD);
			timer_start_frame();
		});
		timer_start_frame();
		ASSERT_GT(_timestamp().value(), jump.boundary_ms);

		const TIMESTAMP stamp = _timestamp(500);
		EXPECT_FALSE(timestamp_elapsed(stamp));
		EXPECT_FALSE(timestamp_elapsed_last_frame(stamp));
		EXPECT_FALSE(timestamp_elapsed_safe(stamp, 2500));
		EXPECT_GT(timestamp_until(stamp), 0);
		EXPECT_LE(timestamp_until(stamp), 500);

		timestamp_adjust_microseconds(STEP_US, TIMER_DIRECTION::FORWARD);
		stepped = true;
		timer_start_frame();

		EXPECT_TRUE(timestamp_elapsed(stamp));
		EXPECT_TRUE(timestamp_elapsed_last_frame(stamp));
		EXPECT_TRUE(timestamp_elapsed_safe(stamp, 2500));
		EXPECT_GE(timestamp_since(stamp), 100);
	}
}

TEST_F(TimerTest, typedArithmeticPastThirtyTwoBits)
{
	const TIMESTAMP before(5'000'000'000LL);
	const TIMESTAMP after = timestamp_delta(before, 1500);
	EXPECT_EQ(after.value(), 5'000'001'500LL);
	EXPECT_EQ(timestamp_get_delta(before, after), 1500);
	EXPECT_LT(timestamp_compare(before, after), 0);
	EXPECT_TRUE(timestamp_in_between(TIMESTAMP(5'000'000'700LL), before, after));
	EXPECT_EQ(timestamp_get_delta(TIMESTAMP(2), before), INT_MAX);
	EXPECT_EQ(timestamp_get_delta(before, TIMESTAMP(2)), INT_MIN);

	const UI_TIMESTAMP ui_before(5'000'000'000LL);
	const UI_TIMESTAMP ui_after = ui_timestamp_delta(ui_before, 1500);
	EXPECT_EQ(ui_after.value(), 5'000'001'500LL);
	EXPECT_EQ(ui_timestamp_get_delta(ui_before, ui_after), 1500);
	EXPECT_LT(ui_timestamp_compare(ui_before, ui_after), 0);
	EXPECT_TRUE(ui_timestamp_in_between(UI_TIMESTAMP(5'000'000'700LL), ui_before, ui_after));
}

TEST_F(TimerTest, uiStampsPastThirtyTwoBitsStayPending)
{
	const UI_TIMESTAMP far = ui_timestamp_delta(ui_timestamp(INT_MAX), INT_MAX);
	ASSERT_GE(far.value(), MS_2_32 - 2);

	EXPECT_FALSE(ui_timestamp_elapsed(far));
	EXPECT_FALSE(ui_timestamp_elapsed_last_frame(far));
	EXPECT_EQ(ui_timestamp_until(far), INT_MAX);
	EXPECT_EQ(ui_timestamp_since(far), INT_MIN);
}
