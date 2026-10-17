
#include <gtest/gtest.h>

#include <controlconfig/controlsconfig.h>

namespace
{
const int THRESHOLD = abs_axis_state::TAKEOVER_THRESHOLD;
}

TEST(AbsAxisStateTests, starts_centered)
{
	abs_axis_state state;

	ASSERT_EQ(F1_0 / 2, state.value);
	ASSERT_EQ(-1, state.active);
}

TEST(AbsAxisStateTests, first_reading_takes_control)
{
	abs_axis_state state;

	state.update_position(0, 1000);

	ASSERT_EQ(1000, state.value);
	ASSERT_EQ(0, state.active);
}

TEST(AbsAxisStateTests, active_source_tracks_below_threshold)
{
	abs_axis_state state;

	state.update_position(0, 1000);
	state.update_position(0, 1001);

	ASSERT_EQ(1001, state.value);
	ASSERT_EQ(0, state.active);
}

TEST(AbsAxisStateTests, new_source_does_not_take_control)
{
	abs_axis_state state;

	state.update_position(0, 1000);
	state.update_position(1, 50000);

	ASSERT_EQ(1000, state.value);
	ASSERT_EQ(0, state.active);
}

TEST(AbsAxisStateTests, idle_source_does_not_take_control)
{
	abs_axis_state state;

	state.update_position(0, 1000);
	state.update_position(1, 50000);

	// jitter on the second source
	state.update_position(0, 1000);
	state.update_position(1, 50000 + THRESHOLD);

	ASSERT_EQ(1000, state.value);
	ASSERT_EQ(0, state.active);
}

TEST(AbsAxisStateTests, moved_source_takes_control)
{
	abs_axis_state state;

	state.update_position(0, 1000);
	state.update_position(1, 50000);

	state.update_position(0, 1000);
	state.update_position(1, 50000 + THRESHOLD + 1);

	ASSERT_EQ(50000 + THRESHOLD + 1, state.value);
	ASSERT_EQ(1, state.active);

	// and the first source must now move to take it back
	state.update_position(0, 1000 + THRESHOLD);

	ASSERT_EQ(50000 + THRESHOLD + 1, state.value);
	ASSERT_EQ(1, state.active);

	state.update_position(0, 1000 + THRESHOLD + 1);

	ASSERT_EQ(1000 + THRESHOLD + 1, state.value);
	ASSERT_EQ(0, state.active);
}

TEST(AbsAxisStateTests, repeated_reads_are_idempotent)
{
	abs_axis_state state;

	state.update_position(0, 1000);
	state.update_position(1, 50000);
	state.update_position(0, 1000);
	state.update_position(1, 50000);

	ASSERT_EQ(1000, state.value);
	ASSERT_EQ(0, state.active);
}

TEST(AbsAxisStateTests, delta_nudges_current_value)
{
	abs_axis_state state;

	state.update_position(0, 20000);
	state.update_delta(500.0f);

	ASSERT_EQ(20500, state.value);
	ASSERT_EQ(-1, state.active);

	state.update_delta(-1500.0f);

	ASSERT_EQ(19000, state.value);
}

TEST(AbsAxisStateTests, delta_clamps)
{
	abs_axis_state state;

	state.update_delta(10.0f * F1_0);
	ASSERT_EQ(F1_0, state.value);

	state.update_delta(-10.0f * F1_0);
	ASSERT_EQ(0, state.value);
}

TEST(AbsAxisStateTests, source_must_move_to_retake_control_from_delta)
{
	abs_axis_state state;

	state.update_position(0, 20000);
	state.update_delta(5000.0f);

	// the lever is still where it was
	state.update_position(0, 20000 + THRESHOLD);

	ASSERT_EQ(25000, state.value);
	ASSERT_EQ(-1, state.active);

	state.update_position(0, 20000 + THRESHOLD + 1);

	ASSERT_EQ(20000 + THRESHOLD + 1, state.value);
	ASSERT_EQ(0, state.active);
}

TEST(AbsAxisStateTests, reset_returns_to_center)
{
	abs_axis_state state;

	state.update_position(0, 1000);
	state.update_position(1, 50000);
	state.reset();

	ASSERT_EQ(F1_0 / 2, state.value);
	ASSERT_EQ(-1, state.active);

	// after a reset, the first reading takes control again, from either source
	state.update_position(1, 50000);

	ASSERT_EQ(50000, state.value);
	ASSERT_EQ(1, state.active);
}
