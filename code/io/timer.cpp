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

static int Timer_inited = 0;

static uint64_t Timer_timestamp_offset_from_counter = 0;
static uint64_t Timer_timestamp_paused_at_counter = 0;

static uint64_t Timestamp_microseconds_at_mission_start = 0;

// forward declarations
static uint64_t timestamp_get_microseconds();


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
		Timer_inited = 0;
	}
}

void timer_init()
{
	if ( !Timer_inited )	{
		Timer_perf_counter_freq = SDL_GetPerformanceFrequency();
		Timer_base_value = SDL_GetPerformanceCounter();
		Timer_to_nanoseconds = (long double) NANOSECONDS_PER_SECOND / (long double) Timer_perf_counter_freq;
		Timer_to_microseconds = (long double) MICROSECONDS_PER_SECOND / (long double) Timer_perf_counter_freq;
		Timer_inited = 1;

		// set up the config so that timestamps are usable
		// (timestamps are used in the UI and in some init functions, not just within the mission)
		timer_pause_timestamp();
		timer_unpause_timestamp();

		atexit(timer_close);
	}
}

fix timer_get_fixed_seconds()
{
	if (!Timer_inited) {
		UNREACHABLE("Make sure you call timer_init before anything that uses timer functions!");
		return 0;
	}

	auto time = timer_get_microseconds();
	time *= 65536;

	return (fix)(time / MICROSECONDS_PER_SECOND);
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

	return (int) (timer_get_microseconds() / MICROSECONDS_PER_SECOND);
}

int timer_get_milliseconds()
{
	if (!Timer_inited) {
		UNREACHABLE("Make sure you call timer_init before anything that uses timer functions!");
		return 0;
	}

	return (int) (timer_get_microseconds() / 1000);
}

std::uint64_t timer_get_microseconds()
{
	auto time = get_performance_counter();

	return (uint64_t) (time * Timer_to_microseconds);
}

std::uint64_t timer_get_nanoseconds()
{
	auto time = get_performance_counter();

    return (uint64_t) (time * Timer_to_nanoseconds);
}

static bool Timer_is_paused = false;
static bool Timer_sudo_paused = false;

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
	Timer_is_paused = false;

	auto counter = get_performance_counter();

	if (Timer_timestamp_offset_from_counter == 0) {
		Timer_timestamp_offset_from_counter = counter;
	} else {
		// update our offset to take into account the time we were paused
		Timer_timestamp_offset_from_counter += (counter - Timer_timestamp_paused_at_counter);
	}
}

void timer_start_mission()
{
	Timestamp_microseconds_at_mission_start = timestamp_get_microseconds();
}

fix timer_get_mission_time()
{
	// convert timestamp to mission time
	// c.f. timer_get_fixed_seconds

	auto time = (timestamp_get_microseconds() - Timestamp_microseconds_at_mission_start);
	time *= 65536;

	return (fix)(time / MICROSECONDS_PER_SECOND);
}


// 0 means invalid,
// 1 means always return true
// 2 and above actually check the time

// Restrict all time values between 0 and MAX_TIME
// so we don't have to use UINTs to calculate rollover.
// For debugging & testing, you could set this to
// something like 1 minute (60000).
extern const std::uint32_t MAX_TIME = INT_MAX / 2;

static uint64_t timestamp_get_microseconds() {

	// TODO - time compression needs to be handled here

	uint64_t timestamp_raw;
	if (Timer_is_paused) {
		timestamp_raw = Timer_timestamp_paused_at_counter;
	} else {
		timestamp_raw = get_performance_counter();
	}
	timestamp_raw -= Timer_timestamp_offset_from_counter;

	return (uint64_t)(timestamp_raw * Timer_to_microseconds);
}

static int timestamp_ms() {
	return (int)(timestamp_get_microseconds() / 1000);
}

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

// alternate timestamp functions.  The way these work is you call xtimestamp() to get the
// current counter value, and then call
int timestamp() {
	return timestamp_ms();
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
