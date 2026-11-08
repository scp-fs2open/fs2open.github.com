#include "io/timer.h"
#include "osapi/dialogs.h"
#include "util/FSTestFixture.h"
#include "utils/finally.h"

#include <gtest/gtest.h>

#include <chrono>
#include <climits>
#include <thread>

extern fix Game_time_compression;

class TimerTest : public test::FSTestFixture {
  public:
	TimerTest() : test::FSTestFixture(INIT_NONE) {}
};

namespace {
constexpr std::int64_t MS_MAX_TIME = INT_MAX / 2;
constexpr std::int64_t MS_2_31 = 2'147'483'648LL;
constexpr std::int64_t MS_2_32 = 4'294'967'296LL;
constexpr uint64_t US_PER_DAY = 86'400ULL * MICROSECONDS_PER_SECOND;
constexpr uint64_t STEP_US = 600'000ULL;
constexpr uint64_t COMPRESSED_JUMP_US = 13 * US_PER_DAY;
constexpr uint64_t COMPRESSED_STEP_US = 300'000ULL;

struct ClockJump {
	uint64_t days;
	std::int64_t boundary_ms;
};
} // namespace

TEST_F(TimerTest, gameStampsKeepWorkingPastThirtyTwoBits)
{
	for (const ClockJump jump : {ClockJump{13, MS_MAX_TIME}, ClockJump{25, MS_2_31}, ClockJump{50, MS_2_32}}) {
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

TEST_F(TimerTest, typedArithmeticAcrossThirtyTwoBitBoundaries)
{
	for (const std::int64_t boundary : {MS_2_31, MS_2_32}) {
		SCOPED_TRACE(boundary);
		const TIMESTAMP before(boundary - 700);
		const TIMESTAMP after = timestamp_delta(before, 1500);
		EXPECT_EQ(after.value(), boundary + 800);
		EXPECT_EQ(timestamp_get_delta(before, after), 1500);
		EXPECT_LT(timestamp_compare(before, after), 0);
		EXPECT_TRUE(timestamp_in_between(TIMESTAMP(boundary + 100), before, after));
		EXPECT_FALSE(timestamp_in_between(TIMESTAMP(boundary + 900), before, after));

		const UI_TIMESTAMP ui_before(boundary - 700);
		const UI_TIMESTAMP ui_after = ui_timestamp_delta(ui_before, 1500);
		EXPECT_EQ(ui_after.value(), boundary + 800);
		EXPECT_EQ(ui_timestamp_get_delta(ui_before, ui_after), 1500);
		EXPECT_LT(ui_timestamp_compare(ui_before, ui_after), 0);
		EXPECT_TRUE(ui_timestamp_in_between(UI_TIMESTAMP(boundary + 100), ui_before, ui_after));
		EXPECT_FALSE(ui_timestamp_in_between(UI_TIMESTAMP(boundary + 900), ui_before, ui_after));
	}

	// a 32-bit delta between stamps less than 2^31 ms apart is right by wraparound, so only a longer span shows width
	const TIMESTAMP low(MS_2_31 - 700);
	const TIMESTAMP high(MS_2_32 + 800);
	EXPECT_LT(timestamp_compare(low, high), 0);
	EXPECT_TRUE(timestamp_in_between(TIMESTAMP(MS_2_32 + 100), low, high));
	EXPECT_EQ(timestamp_get_delta(low, high), INT_MAX);
	EXPECT_EQ(timestamp_get_delta(high, low), INT_MIN);

	const UI_TIMESTAMP ui_low(MS_2_31 - 700);
	const UI_TIMESTAMP ui_high(MS_2_32 + 800);
	EXPECT_LT(ui_timestamp_compare(ui_low, ui_high), 0);
	EXPECT_TRUE(ui_timestamp_in_between(UI_TIMESTAMP(MS_2_32 + 100), ui_low, ui_high));
	EXPECT_EQ(ui_timestamp_get_delta(ui_low, ui_high), INT_MAX);
	EXPECT_EQ(ui_timestamp_get_delta(ui_high, ui_low), INT_MIN);
}

TEST_F(TimerTest, gameDeltasSaturateOutsideIntRange)
{
	timer_start_frame();
	const TIMESTAMP far(_timestamp().value() + 5'000'000'000LL);

	EXPECT_EQ(timestamp_until(far), INT_MAX);
	EXPECT_EQ(timestamp_since(far), INT_MIN);
}

TEST_F(TimerTest, gameStampsPastThirtyTwoBitsUnderTimeCompression)
{
	timer_start_frame();
	// a raw clock below zero reads as about 1.8e13 ms and would be folded into the compression offset for good
	ASSERT_LT(_timestamp().value(), MS_2_32 * 1000);
	const fix old_compression = Game_time_compression;
	Game_time_compression = 2 * F1_0;
	timestamp_update_time_compression();
	timestamp_adjust_microseconds(COMPRESSED_JUMP_US, TIMER_DIRECTION::FORWARD);
	bool stepped = false;
	// the counter is moved back before the multiplier, so each restore step undoes exactly what it moved
	auto restore_clock = util::finally([&stepped, old_compression] {
		if (stepped) {
			timestamp_adjust_microseconds(COMPRESSED_STEP_US, TIMER_DIRECTION::BACKWARD);
		}
		timestamp_adjust_microseconds(COMPRESSED_JUMP_US, TIMER_DIRECTION::BACKWARD);
		Game_time_compression = F1_0;
		timestamp_update_time_compression();
		Game_time_compression = old_compression;
		// compression changes grew the counter offset, which timer_init keeps; the next test's clock would go negative
		timestamp_adjust_microseconds(2 * timer_get_microseconds(), TIMER_DIRECTION::FORWARD);
		timer_start_frame();
	});
	timer_start_frame();
	ASSERT_GT(_timestamp().value(), MS_2_31);

	const TIMESTAMP stamp = _timestamp(500);
	EXPECT_FALSE(timestamp_elapsed(stamp));
	EXPECT_GT(timestamp_until(stamp), 0);
	EXPECT_LE(timestamp_until(stamp), 500);

	timestamp_adjust_microseconds(COMPRESSED_STEP_US, TIMER_DIRECTION::FORWARD);
	stepped = true;
	timer_start_frame();

	EXPECT_TRUE(timestamp_elapsed(stamp));
	EXPECT_GE(timestamp_since(stamp), 50);
}

TEST_F(TimerTest, gameClockPausedAcrossThirtyTwoBits)
{
	timer_start_frame();
	constexpr std::int64_t TARGET_MS = MS_2_31 - 200;
	const std::int64_t start_ms = _timestamp().value();
	// other tests may leave the clock ahead of the target, so approach it from whichever side the clock is on
	const bool behind = start_ms < TARGET_MS;
	const auto jump_dir = behind ? TIMER_DIRECTION::FORWARD : TIMER_DIRECTION::BACKWARD;
	const auto back_dir = behind ? TIMER_DIRECTION::BACKWARD : TIMER_DIRECTION::FORWARD;
	const auto jump_us =
		static_cast<uint64_t>(behind ? TARGET_MS - start_ms : start_ms - TARGET_MS) * MICROSECONDS_PER_MILLISECOND;
	timestamp_adjust_microseconds(jump_us, jump_dir);
	bool stepped = false;
	uint64_t pause_started_us = 0;
	// unpause first: an adjustment made while paused is discarded by the unpause
	auto restore_clock = util::finally([&stepped, &pause_started_us, jump_us, back_dir] {
		if (timestamp_is_paused()) {
			timestamp_unpause(true);
		}
		if (stepped) {
			timestamp_adjust_microseconds(STEP_US, TIMER_DIRECTION::BACKWARD);
		}
		timestamp_adjust_microseconds(jump_us, back_dir);
		// the unpause added the paused time to the counter offset; give it back, as in the compression test
		timestamp_adjust_microseconds(timer_get_microseconds() - pause_started_us, TIMER_DIRECTION::FORWARD);
		timer_start_frame();
	});
	timer_start_frame();
	ASSERT_GT(_timestamp().value(), TARGET_MS - 1000);
	ASSERT_LT(_timestamp().value(), TARGET_MS + 100);
	const TIMESTAMP stamp = _timestamp(500);

	pause_started_us = timer_get_microseconds();
	timestamp_pause(true);
	timer_start_frame();
	const std::int64_t paused_ms = _timestamp().value();
	ASSERT_LT(paused_ms, MS_2_31);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	timer_start_frame();
	EXPECT_EQ(_timestamp().value(), paused_ms);
	EXPECT_FALSE(timestamp_elapsed(stamp));

	timestamp_unpause(true);
	timer_start_frame();
	EXPECT_LT(_timestamp().value() - paused_ms, 50);
	EXPECT_FALSE(timestamp_elapsed(stamp));

	timestamp_adjust_microseconds(STEP_US, TIMER_DIRECTION::FORWARD);
	stepped = true;
	timer_start_frame();

	EXPECT_GT(_timestamp().value(), MS_2_31);
	EXPECT_TRUE(timestamp_elapsed(stamp));
	EXPECT_GE(timestamp_since(stamp), 50);
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

TEST_F(TimerTest, uiClockReadsZeroWhileTheTimerIsClosed)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(5));
	timer_close();
	auto reopen = util::finally([] { timer_init(); });
#ifdef NDEBUG
	EXPECT_EQ(ui_timestamp().value(), 0);
#else
	EXPECT_THROW(ui_timestamp(), os::dialogs::AssertException);
#endif
}
