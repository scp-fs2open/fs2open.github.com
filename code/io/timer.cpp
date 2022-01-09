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

static uint64_t Timer_timestamp_offset_from_counter = 0;
static uint64_t Timer_timestamp_paused_at_counter = 0;

static fix Timestamp_current_time_compression = F1_0;
static float Timestamp_time_compression_multiplier = 1.0f;
static uint64_t Timestamp_time_compression_microseconds_offset = 0;

static bool Timer_is_paused = false;
static bool Timer_sudo_paused = false;

static uint64_t Timestamp_microseconds_at_mission_start = 0;


#define MICROSECONDS_PER_SECOND 1000000
#define NANOSECONDS_PER_SECOND 1000000000

static long double Timer_to_microseconds;
static long double Timer_to_nanoseconds;

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
		timer_pause_timestamp(true);
		timer_unpause_timestamp(true);

		atexit(timer_close);
	}
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

	return static_cast<int>(timer_get_microseconds() / 1000);
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

static uint64_t timestamp_get_raw()
{
	uint64_t timestamp_raw;
	if (Timer_is_paused) {
		timestamp_raw = Timer_timestamp_paused_at_counter;
	} else {
		timestamp_raw = get_performance_counter();
	}
	timestamp_raw -= Timer_timestamp_offset_from_counter;

	return timestamp_raw;
}

static uint64_t timestamp_get_microseconds()
{
	return Timestamp_time_compression_microseconds_offset + static_cast<uint64_t>(timestamp_get_raw() * Timer_to_microseconds * Timestamp_time_compression_multiplier);
}

static int timestamp_ms() {
	return static_cast<int>(timestamp_get_microseconds() / 1000);
}

int timestamp() {
	return timestamp_ms();
}

// ======================================== checking timestamps ========================================

// Restrict all time values between 0 and MAX_TIME
// so we don't have to use UINTs to calculate rollover.
// For debugging & testing, you could set this to
// something like 1 minute (60000).
extern const std::uint32_t MAX_TIME = INT_MAX / 2;

int timestamp(int delta_ms ) {
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

//	Returns milliseconds until timestamp will elapse.
//	Negative value gives milliseconds ago that timestamp elapsed.
int timestamp_until(int stamp) {
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

int timestamp_has_time_elapsed(int stamp, int time) {
	int t;

	if (time <= 0)
		return 1;

	t = stamp + time;
	if (t <= timestamp_ms())
		return 1;  // if we are unlucky enough to have it wrap on us, this will assume time has elapsed.

	return 0;
}

bool timestamp_elapsed(int stamp) {
	if (stamp == 0) {
		return false;
	}

	return timestamp_ms() >= stamp;
}

bool timestamp_elapsed_safe(int a, int b) {
	if (a == 0) {
		return true;
	}

	return timestamp_ms() >= a || timestamp_ms() < (a - b + 100);
}

// ======================================== pausing/unpausing/adjusting ========================================

void timer_pause_timestamp(bool sudo)
{
	if (sudo)
		Timer_sudo_paused = true;
	if (Timer_is_paused)
		return;
	Timer_is_paused = true;

	Timer_timestamp_paused_at_counter = get_performance_counter();
}

void timer_unpause_timestamp(bool sudo)
{
	if (Timer_sudo_paused && !sudo)
		return;
	if (sudo)
		Timer_sudo_paused = false;
	if (!Timer_is_paused)
		return;
	Timer_is_paused = false;

	auto counter = get_performance_counter();

	if (Timer_timestamp_offset_from_counter == 0) {
		Timer_timestamp_offset_from_counter = counter;
	} else {
		// update our offset to take into account the time we were paused
		Timer_timestamp_offset_from_counter += (counter - Timer_timestamp_paused_at_counter);
	}
}

void timer_adjust(float delta_seconds, TIMER_DIRECTION dir)
{
	Assertion(Timer_inited, "Timer should be initialized at this point!");
	Assertion(Timer_timestamp_offset_from_counter != 0 && Timer_timestamp_paused_at_counter != 0 && Timestamp_microseconds_at_mission_start != 0,
		"Warranty void if these variables have not been set!");
	Assertion(delta_seconds > 0.0f, "The delta should be positive!");

	auto delta_microseconds = static_cast<long double>(delta_seconds) * MICROSECONDS_PER_SECOND;
	auto delta_timer = static_cast<uint64_t>(delta_microseconds / Timer_to_microseconds);

	// adjust all the internal variables so it is as if the timer jumped forward or backward
	if (dir == TIMER_DIRECTION::FORWARD)
	{
		Timestamp_microseconds_at_mission_start -= static_cast<uint64_t>(delta_microseconds);
		Timer_timestamp_offset_from_counter -= delta_timer;
		Timer_timestamp_paused_at_counter -= delta_timer;
	}
	else
	{
		Timestamp_microseconds_at_mission_start += static_cast<uint64_t>(delta_microseconds);
		Timer_timestamp_offset_from_counter += delta_timer;
		Timer_timestamp_paused_at_counter += delta_timer;
	}
}

void timer_adjust_microseconds(uint64_t delta_microseconds, TIMER_DIRECTION dir)
{
	Assertion(Timer_inited, "Timer should be initialized at this point!");
	Assertion(Timer_timestamp_offset_from_counter != 0 && Timer_timestamp_paused_at_counter != 0 && Timestamp_microseconds_at_mission_start != 0,
		"Warranty void if these variables have not been set!");

	auto delta_timer = static_cast<uint64_t>(static_cast<long double>(delta_microseconds) / Timer_to_microseconds);

	// adjust all the internal variables so it is as if the timer jumped forward or backward
	if (dir == TIMER_DIRECTION::FORWARD)
	{
		Timestamp_microseconds_at_mission_start -= delta_microseconds;
		Timer_timestamp_offset_from_counter -= delta_timer;
		Timer_timestamp_paused_at_counter -= delta_timer;
	}
	else
	{
		Timestamp_microseconds_at_mission_start += delta_microseconds;
		Timer_timestamp_offset_from_counter += delta_timer;
		Timer_timestamp_paused_at_counter += delta_timer;
	}
}

extern fix Game_time_compression;
void timer_update_time_compression()
{
	// if no change, no need to update
	if (Game_time_compression == Timestamp_current_time_compression) {
		return;
	}

	// grab the independent variable of the equation before we change anything
	auto timestamp_raw = timestamp_get_raw();

	// we need to move the counter offset to make the raw timestamp zero (so that it can start ticking with a new multiplier)
	Timer_timestamp_offset_from_counter += timestamp_raw;
	Timer_timestamp_paused_at_counter += timestamp_raw;

	// add the accumulated time so we don't lose any of it
	auto delta_microseconds = static_cast<uint64_t>(timestamp_raw * Timer_to_microseconds * Timestamp_time_compression_multiplier);
	Timestamp_time_compression_microseconds_offset += delta_microseconds;

	// now we can set the new info
	Timestamp_current_time_compression = Game_time_compression;
	Timestamp_time_compression_multiplier = static_cast<float>(Game_time_compression) / F1_0;
}

// ======================================== mission-specific stuff ========================================

void timer_start_mission()
{
	Timestamp_microseconds_at_mission_start = timestamp_get_microseconds();
}

void timer_revert_to_mission_start()
{
	auto delta_microseconds = (timestamp_get_microseconds() - Timestamp_microseconds_at_mission_start);
	timer_adjust_microseconds(delta_microseconds, TIMER_DIRECTION::BACKWARD);
}

fix timer_get_mission_time()
{
	// convert timestamp to mission time
	// c.f. timer_get_fixed_seconds

	auto time = (timestamp_get_microseconds() - Timestamp_microseconds_at_mission_start);
	time *= F1_0;

	return static_cast<fix>(time / MICROSECONDS_PER_SECOND);
}
