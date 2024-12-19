/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/



#include "globalincs/pstypes.h"
#include "io/timer.h"
#include "graphics/2d.h"
#include "globalincs/alphacolors.h"

#include "osapi/osapi.h"	// for multi-thread macros

#include <SDL_timer.h>
#include <climits>

static Uint64 Timer_perf_counter_freq = 0;	// perf counter frequency - number of ticks per second
static Uint64 Timer_base_value;

static bool Timer_inited = false;

static long double Timer_to_microseconds;
static long double Timer_to_nanoseconds;


static uint64_t Timestamp_offset_from_counter = 0;
static uint64_t Timestamp_paused_at_counter = 0;

static fix Timestamp_current_time_compression = F1_0;
static float Timestamp_time_compression_multiplier = 1.0f;
static uint64_t Timestamp_time_compression_microseconds_offset = 0;

static bool Timestamp_is_paused = false;
static bool Timestamp_sudo_paused = false;

static uint64_t Timestamp_microseconds_at_mission_start = 0;


static uint64_t timestamp_get_raw(bool start_frame = false);


static uint64_t get_performance_counter()
{
	Assertion(Timer_inited, "This function can only be used when the timer system is initialized!");

	auto counter = SDL_GetPerformanceCounter();

	return counter - Timer_base_value;
}

void timer_close()
{
	if ( Timer_inited )	{
		Timer_inited = false;
	}
}

void timer_init()
{
	if ( !Timer_inited )	{
		Timer_perf_counter_freq = SDL_GetPerformanceFrequency();
		Timer_base_value = SDL_GetPerformanceCounter();
		Timer_to_nanoseconds = (long double) NANOSECONDS_PER_SECOND / (long double) Timer_perf_counter_freq;
		Timer_to_microseconds = (long double) MICROSECONDS_PER_SECOND / (long double) Timer_perf_counter_freq;
		Timer_inited = true;

		// set up the config so that timestamps are usable
		// (timestamps are used in the UI and in some init functions, not just within the mission)
		timestamp_pause(true);
		timestamp_unpause(true);

		atexit(timer_close);
	}
}

void timer_start_frame()
{
	// take a snapshot of the raw timestamp at the beginning of the frame
	timestamp_get_raw(true);
}

// ======================================== getting time ========================================

fix timer_get_fixed_seconds()
{
	if (!Timer_inited) {
		UNREACHABLE("Make sure you call timer_init before anything that uses timer functions!");
		return 0;
	}

	auto time = timer_get_microseconds();
	time *= F1_0;

	return static_cast<fix>(time / MICROSECONDS_PER_SECOND);
}

fix timer_get_approx_seconds()
{
	return timer_get_fixed_seconds();
}

int timer_get_seconds()
{
	if (!Timer_inited) {
		UNREACHABLE("Make sure you call timer_init before anything that uses timer functions!");
		return 0;
	}

	return static_cast<int>(timer_get_microseconds() / MICROSECONDS_PER_SECOND);
}

int timer_get_milliseconds()
{
	if (!Timer_inited) {
		UNREACHABLE("Make sure you call timer_init before anything that uses timer functions!");
		return 0;
	}

	return static_cast<int>(timer_get_microseconds() / MICROSECONDS_PER_MILLISECOND);
}

std::uint64_t timer_get_microseconds()
{
	auto time = get_performance_counter();

	return static_cast<uint64_t>(time * Timer_to_microseconds);
}

std::uint64_t timer_get_nanoseconds()
{
	auto time = get_performance_counter();

    return static_cast<uint64_t>(time * Timer_to_nanoseconds);
}

static uint64_t timestamp_get_raw(bool start_frame)
{
	static uint64_t timestamp_raw = 0;

	// The simulation timestamp is only updated at the beginning of the frame
	// because we want all timestamps within a frame to be identical.
	if (start_frame)
	{
		if (Timestamp_is_paused)
			timestamp_raw = Timestamp_paused_at_counter;
		else
			timestamp_raw = get_performance_counter();

		timestamp_raw -= Timestamp_offset_from_counter;
	}

	return timestamp_raw;
}

static uint64_t timestamp_get_microseconds()
{
	return Timestamp_time_compression_microseconds_offset + static_cast<uint64_t>(timestamp_get_raw() * Timer_to_microseconds * Timestamp_time_compression_multiplier);
}

static int timestamp_ms() {
	return static_cast<int>(timestamp_get_microseconds() / MICROSECONDS_PER_MILLISECOND);
}

int timestamp() {
	return timestamp_ms();
}

TIMESTAMP _timestamp() {
	return TIMESTAMP(timestamp_ms());
}

UI_TIMESTAMP ui_timestamp() {
	return UI_TIMESTAMP(timer_get_milliseconds());
}

TIMESTAMP timestamp_delta(TIMESTAMP stamp, int delta_ms)
{
	if (!stamp.isFinite())
		return stamp;

	if (stamp.isImmediate())
		return TIMESTAMP(timestamp_ms() + delta_ms);

	return TIMESTAMP(stamp.value() + delta_ms);
}

UI_TIMESTAMP ui_timestamp_delta(UI_TIMESTAMP stamp, int delta_ms)
{
	if (!stamp.isFinite())
		return stamp;

	if (stamp.isImmediate())
		return UI_TIMESTAMP(timer_get_milliseconds() + delta_ms);

	return UI_TIMESTAMP(stamp.value() + delta_ms);
}

// Returns the difference between two timestamps as an int, does not check that the result will be positive.
// Also only error check if a timestamp is invalid, but when either is immediate or never, it returns 0.
int timestamp_get_delta(TIMESTAMP before, TIMESTAMP after)
{
	Assertion(before.isValid(), "timestamp_get_delta called with an invalid before timestamp%s. This is a coder mistake, please report!", (after.isValid()) ? "" : " and an invalid after timestamp.");
	Assertion(after.isValid(), "timestamp_get_delta called with an invalid after timestamp. This is a coder mistake, please report!");


	if (!before.isValid() || !after.isValid()) {
		return 0;
	}

	// infinite difference in the future.
	if (before.isImmediate() && after.isNever()) {
		return INT_MAX;
	// infinite difference in the past
	} else if (after.isImmediate() && before.isNever()) {
		return INT_MIN;	
	// no difference
	} else if ((before.isImmediate() && after.isImmediate())
			|| (before.isNever() && after.isNever())) {
		return 0;
	}

	return (after.value() - before.value());
}

// Returns the difference between two timestamps, does not check that the result will be positive.
// Also only error check if a timestamp is invalid, but when either is immediate or never, it returns 0.
int ui_timestamp_get_delta(UI_TIMESTAMP before, UI_TIMESTAMP after)
{
	Assertion(before.isValid(), "ui_timestamp_get_delta called with an invalid before timestamp%s. This is a coder mistake, please report!", (after.isValid()) ? "" : " and an invalid after timestamp.");
	Assertion(after.isValid(), "ui_timestamp_get_delta called with an invalid after timestamp. This is a coder mistake, please report!");

	if (!before.isValid() || !after.isValid()) {
		return 0;
	}

	// infinite difference in the future.
	if (before.isImmediate() && after.isNever()) {
		return INT_MAX;
	// infinite difference in the past
	} else if (after.isImmediate() && before.isNever()) {
		return INT_MIN;	
	// no difference
	} else if ((before.isImmediate() && after.isImmediate())
			|| (before.isNever() && after.isNever())) {
		return 0;
	}
	
	return (after.value() - before.value());
}

// ======================================== checking timestamps ========================================

// Restrict all time values between 0 and MAX_TIME
// so we don't have to use UINTs to calculate rollover.
// For debugging & testing, you could set this to
// something like 1 minute (60000).
// Although this is around 12.4 days (1073741823 milliseconds).
extern const std::uint32_t MAX_TIME = INT_MAX / 2;

int timestamp(int delta_ms) {
	int t2;
	if (delta_ms < 0 ) return 0;
	if (delta_ms == 0 ) return 1;
	t2 = timestamp_ms() + delta_ms;
	if ( t2 > (int)MAX_TIME )	{
		// wrap!!!
		t2 = delta_ms - (MAX_TIME-timestamp_ms());
	}
	if (t2 < 2 ) t2 = 2;	// hack??
	return t2;
}

TIMESTAMP _timestamp(int delta_ms) {
	int t2;
	if (delta_ms < 0 ) return TIMESTAMP::never();
	if (delta_ms == 0 ) return TIMESTAMP::immediate();
	t2 = timestamp_ms() + delta_ms;
	if ( t2 > (int)MAX_TIME )	{
		// wrap!!!
		t2 = delta_ms - (MAX_TIME-timestamp_ms());
	}
	if (t2 < 2 ) t2 = 2;	// hack??
	return TIMESTAMP(t2);
}

UI_TIMESTAMP ui_timestamp(int delta_ms) {
	int t2;
	if (delta_ms < 0 ) return UI_TIMESTAMP::never();
	if (delta_ms == 0 ) return UI_TIMESTAMP::immediate();
	t2 = timer_get_milliseconds() + delta_ms;
	if ( t2 > (int)MAX_TIME )	{
		// wrap!!!
		t2 = delta_ms - (MAX_TIME-timer_get_milliseconds());
	}
	if (t2 < 2 ) t2 = 2;	// hack??
	return UI_TIMESTAMP(t2);
}

//	Returns milliseconds until timestamp will elapse.
//	Negative value gives milliseconds ago that timestamp elapsed.
int timestamp_until(int stamp)
{
	// handle special values
	if (stamp <= 0)
		return INT_MAX;
	if (stamp == 1)
		return 0;

	// JAS: FIX
	// HACK!! This doesn't handle rollover!
	// (Will it ever happen?)

	return stamp - timestamp_ms();

/*
	uint	delta;

	delta = stamp - timestamp_ticker;


	if (delta > UINT_MAX/2)
		delta = UINT_MAX - delta + 1;
	else if (delta < - ( (int) (UINT_MAX/2)))
		delta = UINT_MAX + delta + 1;

	return delta;
*/
}

int timestamp_until(TIMESTAMP stamp)
{
	Assertion(stamp.isFinite(), "timestamp_until was called with a%s timestamp!", !stamp.isValid() ? "n invalid" : " Never");
	if (!stamp.isFinite())
		return INT_MAX;

	if (stamp.isImmediate())
		return 0;

	return stamp.value() - timestamp_ms();
}

int ui_timestamp_until(UI_TIMESTAMP stamp)
{
	Assertion(stamp.isFinite(), "timestamp_until was called with a%s timestamp!", !stamp.isValid() ? "n invalid" : " Never");
	if (!stamp.isFinite())
		return INT_MAX;

	if (stamp.isImmediate())
		return 0;

	return stamp.value() - timer_get_milliseconds();
}

int timestamp_since(int stamp)
{
	// handle special values
	if (stamp <= 0)
		return INT_MIN;
	if (stamp == 1)
		return 0;

	return timestamp_ms() - stamp;
}

int timestamp_since(TIMESTAMP stamp)
{
	Assertion(stamp.isFinite(), "timestamp_since was called with a%s timestamp!", !stamp.isValid() ? "n invalid" : " Never");
	if (!stamp.isFinite())
		return INT_MIN;

	if (stamp.isImmediate())
		return 0;

	return timestamp_ms() - stamp.value();
}

int ui_timestamp_since(UI_TIMESTAMP stamp)
{
	Assertion(stamp.isFinite(), "timestamp_since was called with a%s timestamp!", !stamp.isValid() ? "n invalid" : " Never");
	if (!stamp.isFinite())
		return INT_MIN;

	if (stamp.isImmediate())
		return 0;

	return timer_get_milliseconds() - stamp.value();
}

int timestamp_compare(TIMESTAMP t1, TIMESTAMP t2)
{
	Assertion(t1.isValid(), "timestamp_compare called with an invalid 't1' timestamp!");
	Assertion(t2.isValid(), "timestamp_compare called with an invalid 't2' timestamp!");
	if (!t1.isValid() || !t2.isValid())
		return 0;

	if (t1.isImmediate())
	{
		if (t2.isImmediate())
			return 0;
		else
			return -1;
	}
	else if (t2.isImmediate())
		return 1;

	if (t1.isNever())
	{
		if (t2.isNever())
			return 0;
		else
			return 1;
	}
	else if (t2.isNever())
		return -1;

	if (t1.value() == t2.value())
		return 0;
	else if (t1.value() > t2.value())
		return 1;
	else
		return -1;
}

int ui_timestamp_compare(UI_TIMESTAMP t1, UI_TIMESTAMP t2)
{
	Assertion(t1.isValid(), "ui_timestamp_compare called with an invalid 't1' timestamp!");
	Assertion(t2.isValid(), "ui_timestamp_compare called with an invalid 't2' timestamp!");
	if (!t1.isValid() || !t2.isValid())
		return 0;

	if (t1.isImmediate())
	{
		if (t2.isImmediate())
			return 0;
		else
			return -1;
	}
	else if (t2.isImmediate())
		return 1;

	if (t1.isNever())
	{
		if (t2.isNever())
			return 0;
		else
			return 1;
	}
	else if (t2.isNever())
		return -1;

	if (t1.value() == t2.value())
		return 0;
	else if (t1.value() > t2.value())
		return 1;
	else
		return -1;
}

bool timestamp_in_between(TIMESTAMP stamp, TIMESTAMP before, TIMESTAMP after)
{
	Assertion(stamp.isFinite(), "timestamp_in_between was called with a%s 'stamp' timestamp!", !stamp.isValid() ? "n invalid" : " Never");
	Assertion(before.isFinite(), "timestamp_in_between was called with a%s 'before' timestamp!", !before.isValid() ? "n invalid" : " Never");
	Assertion(after.isFinite(), "timestamp_in_between was called with a%s 'after' timestamp!", !after.isValid() ? "n invalid" : " Never");

	if (!stamp.isValid() || !before.isValid() || !after.isValid())
		return false;

	return timestamp_compare(before, stamp) <= 0 && timestamp_compare(stamp, after) <= 0;
}

bool ui_timestamp_in_between(UI_TIMESTAMP stamp, UI_TIMESTAMP before, UI_TIMESTAMP after)
{
	Assertion(stamp.isFinite(), "ui_timestamp_in_between was called with a%s 'stamp' timestamp!", !stamp.isValid() ? "n invalid" : " Never");
	Assertion(before.isFinite(), "ui_timestamp_in_between was called with a%s 'before' timestamp!", !before.isValid() ? "n invalid" : " Never");
	Assertion(after.isFinite(), "ui_timestamp_in_between was called with a%s 'after' timestamp!", !after.isValid() ? "n invalid" : " Never");

	if (!stamp.isValid() || !before.isValid() || !after.isValid())
		return false;

	return ui_timestamp_compare(before, stamp) <= 0 && ui_timestamp_compare(stamp, after) <= 0;
}

bool timestamp_elapsed(int stamp) {
	if (stamp == 0) {
		return false;
	}

	return timestamp_ms() >= stamp;
}

bool timestamp_elapsed(TIMESTAMP stamp) {
	if (!stamp.isFinite()) {
		return false;
	}
	if (stamp.isImmediate()) {
		return true;
	}

	return timestamp_ms() >= stamp.value();
}

bool ui_timestamp_elapsed(UI_TIMESTAMP ui_stamp) {
	if (!ui_stamp.isFinite()) {
		return false;
	}
	if (ui_stamp.isImmediate()) {
		return true;
	}

	return timer_get_milliseconds() >= ui_stamp.value();
}

bool timestamp_elapsed_last_frame(TIMESTAMP stamp) {
	if (!stamp.isFinite()) {
		return false;
	}
	if (stamp.isImmediate()) {
		return true;
	}

	return timestamp_ms() > stamp.value();
}

bool ui_timestamp_elapsed_last_frame(UI_TIMESTAMP ui_stamp) {
	if (!ui_stamp.isFinite()) {
		return false;
	}
	if (ui_stamp.isImmediate()) {
		return true;
	}

	return timer_get_milliseconds() > ui_stamp.value();
}

bool timestamp_elapsed_safe(int a, int b) {
	if (a == 0) {
		return true;
	}

	return timestamp_ms() >= a || timestamp_ms() < (a - b + 100);
}

bool timestamp_elapsed_safe(TIMESTAMP a, int b) {
	if (!a.isFinite()) {
		return false;
	}
	if (a.isImmediate()) {
		return true;
	}

	return timestamp_ms() >= a.value() || timestamp_ms() < (a.value() - b + 100);
}

bool ui_timestamp_elapsed_safe(UI_TIMESTAMP a, int b) {
	if (!a.isFinite()) {
		return false;
	}
	if (a.isImmediate()) {
		return true;
	}

	return timer_get_milliseconds() >= a.value() || timer_get_milliseconds() < (a.value() - b + 100);
}

// ======================================== pausing/unpausing/adjusting ========================================

void timestamp_pause(bool sudo)
{
	if (sudo)
		Timestamp_sudo_paused = true;
	if (Timestamp_is_paused)
		return;
	Timestamp_is_paused = true;

	Timestamp_paused_at_counter = get_performance_counter();
}

void timestamp_unpause(bool sudo)
{
	if (Timestamp_sudo_paused && !sudo)
		return;
	if (sudo)
		Timestamp_sudo_paused = false;
	if (!Timestamp_is_paused)
		return;
	Timestamp_is_paused = false;

	auto counter = get_performance_counter();

	if (Timestamp_offset_from_counter == 0) {
		Timestamp_offset_from_counter = counter;
	} else {
		// update our offset to take into account the time we were paused
		Timestamp_offset_from_counter += (counter - Timestamp_paused_at_counter);
	}
}

bool timestamp_is_paused()
{
	return Timestamp_is_paused;
}

void timestamp_adjust_pause_offset(int delta_milliseconds)
{
	Assertion(!Timestamp_is_paused, "This function is not needed if the game is actually paused.");
	Assertion(delta_milliseconds > 0, "Pausing for a negative amount of time doesn't make sense.  Also, negative numbers won't work with uint64_t.");

	// act like we were paused for a certain period of time, even though we weren't
	if (Timestamp_offset_from_counter == 0) {
		Timestamp_offset_from_counter = get_performance_counter();
	} else {
		Timestamp_offset_from_counter += static_cast<uint64_t>(static_cast<uint64_t>(delta_milliseconds) * MICROSECONDS_PER_MILLISECOND / Timer_to_microseconds);
	}
}

void timestamp_adjust_seconds(float delta_seconds, TIMER_DIRECTION dir)
{
	timestamp_adjust_microseconds(static_cast<uint64_t>(static_cast<long double>(delta_seconds) * MICROSECONDS_PER_SECOND), dir);
}

void timestamp_adjust_microseconds(uint64_t delta_microseconds, TIMER_DIRECTION dir)
{
	Assertion(Timer_inited, "Timer should be initialized at this point!");
	Assertion(Timestamp_offset_from_counter != 0 && Timestamp_paused_at_counter != 0, "Warranty void if these variables have not been set!");

	auto delta_timer = static_cast<uint64_t>(delta_microseconds / Timer_to_microseconds);

	// adjust the internal variables so it is as if the timer jumped forward or backward
	if (dir == TIMER_DIRECTION::FORWARD)
	{
		Timestamp_offset_from_counter -= delta_timer;
		Timestamp_paused_at_counter -= delta_timer;
	}
	else
	{
		Timestamp_offset_from_counter += delta_timer;
		Timestamp_paused_at_counter += delta_timer;
	}
}

extern fix Game_time_compression;
void timestamp_update_time_compression()
{
	// if no change, no need to update
	if (Game_time_compression == Timestamp_current_time_compression) {
		return;
	}

	// grab the independent variable of the equation before we change anything
	// (we need to get the live value to be accurate, which takes a new snapshot,
	// but this is ok since time compression is only updated at the start of the frame)
	auto timestamp_raw = timestamp_get_raw(true);

	// we need to move the counter offset to make the raw timestamp zero (so that it can start ticking with a new multiplier)
	Timestamp_offset_from_counter += timestamp_raw;
	Timestamp_paused_at_counter += timestamp_raw;

	// add the accumulated time so we don't lose any of it
	auto delta_microseconds = static_cast<uint64_t>(timestamp_raw * Timer_to_microseconds * Timestamp_time_compression_multiplier);
	Timestamp_time_compression_microseconds_offset += delta_microseconds;

	// now we can set the new info
	Timestamp_current_time_compression = Game_time_compression;
	Timestamp_time_compression_multiplier = static_cast<float>(Game_time_compression) / F1_0;

	// and now take a new snapshot so that the raw timestamp is correct for this frame
	timestamp_get_raw(true);
}

// ======================================== mission-specific stuff ========================================

void timestamp_start_mission()
{
	Timestamp_microseconds_at_mission_start = timestamp_get_microseconds();
}

fix timestamp_get_mission_time()
{
	// convert timestamp to mission time
	// c.f. timer_get_fixed_seconds

	auto time = (timestamp_get_microseconds() - Timestamp_microseconds_at_mission_start);
	time *= F1_0;

	return static_cast<fix>(time / MICROSECONDS_PER_SECOND);
}

uint64_t timestamp_get_mission_time_in_microseconds()
{
	return timestamp_get_microseconds() - Timestamp_microseconds_at_mission_start;
}

int timestamp_get_mission_time_in_milliseconds()
{
	return static_cast<int>(timestamp_get_mission_time_in_microseconds() / MICROSECONDS_PER_MILLISECOND);
}

void timestamp_offset_mission_time(float offset)
{
	auto time = static_cast<uint64_t>(static_cast<long double>(offset) * MICROSECONDS_PER_SECOND);

	Timestamp_microseconds_at_mission_start -= time;
}