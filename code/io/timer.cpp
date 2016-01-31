/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifdef _WIN32
#include <windows.h>
#else
#include <SDL_timer.h>
#include <sys/time.h>
#endif

#include <limits.h>

#include "globalincs/pstypes.h"
#include "io/timer.h"
#include "graphics/2d.h"
#include "globalincs/alphacolors.h"

#define THREADED	// to use the proper set of macros
#include "osapi/osapi.h"	// for multi-thread macros


#ifdef _WIN32
static longlong Timer_last_value = 0, Timer_base = 0, Timer_freq = 0;
static const int precision = 1;
#endif

static longlong Timer_perf_counter_base = 0;	// perf counter start time
static longlong Timer_perf_counter_freq = 0;	// perf counter frequency - number of ticks per second

static int Timer_inited = 0;

static CRITICAL_SECTION Timer_lock;

#define MICROSECONDS_PER_SECOND 1000000

void timer_close()
{
	if ( Timer_inited )	{
		Timer_inited = 0;
#ifdef _WIN32
		timeEndPeriod(precision); 
#endif
		DELETE_CRITICAL_SECTION( Timer_lock );
	}
}

void timer_init()
{
	if ( !Timer_inited )	{
		INITIALIZE_CRITICAL_SECTION( Timer_lock );

#ifdef _WIN32
		timeBeginPeriod(precision);
		Timer_base = Timer_last_value = timeGetTime();

		// get the performance counter start time
		LARGE_INTEGER perf_start_time;
		QueryPerformanceCounter(&perf_start_time);
		Timer_perf_counter_base = perf_start_time.QuadPart;

		// get the performance counter's ticks per second frequency
		LARGE_INTEGER perf_frequency;
		QueryPerformanceFrequency(&perf_frequency);
		Timer_perf_counter_freq = perf_frequency.QuadPart;
#else
		timeval time_value;

		// get the performance counter start time
		gettimeofday(&time_value, NULL);

		Timer_perf_counter_base = time_value.tv_sec * MICROSECONDS_PER_SECOND + time_value.tv_usec;

		// get the performance counter's ticks per second frequency
		Timer_perf_counter_freq = 1;
#endif

		Timer_inited = 1;

		atexit(timer_close);
	}
}

static uint timer_get()
{
#ifdef _WIN32
	ENTER_CRITICAL_SECTION( Timer_lock );

	longlong time_now;

	time_now = timeGetTime();

	if ( time_now < Timer_last_value ) {
		// the clock has rolled!
		Timer_base = time_now;
		mprintf(("TIMER ROLLED!\n"));
	}

	Timer_last_value = time_now;

	LEAVE_CRITICAL_SECTION( Timer_lock );

	return (uint)(time_now - Timer_base);
#else
	return SDL_GetTicks();
#endif
}

fix timer_get_fixed_seconds()
{
	if (!Timer_inited) {
		Int3();					// Make sure you call timer_init before anything that uses timer functions!
		return 0;
	}

	longlong a = timer_get();

	a *= 65536;

	return (fix)(a / 1000);
}

fix timer_get_fixed_secondsX()
{
	return timer_get_fixed_seconds();
}

fix timer_get_approx_seconds()
{
	return timer_get_fixed_seconds();
}

int timer_get_seconds()
{
	if (!Timer_inited) {
		Int3();
		return 0;
	}

	return (timer_get() / 1000);
}

int timer_get_milliseconds()
{
	if (!Timer_inited) {
		Int3();					// Make sure you call timer_init before anything that uses timer functions!
		return 0;
	}

	return timer_get();
}

int timer_get_microseconds()
{
	if (!Timer_inited) {
		Int3();					// Make sure you call timer_init before anything that uses timer functions!
		return 0;
	}

	return timer_get() * 1000;
}

uint timer_get_high_res_microseconds()
{
	if ( !Timer_inited ) {
		Int3();
		return 0;
	}

#ifdef _WIN32
	ENTER_CRITICAL_SECTION( Timer_lock);

	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	longlong elapsed = time.QuadPart;// - Timer_perf_counter_base;

	LEAVE_CRITICAL_SECTION( Timer_lock);

	return (uint)(elapsed * MICROSECONDS_PER_SECOND / Timer_perf_counter_freq);
#else
	timeval time_value;

	gettimeofday(&time_value, NULL);

	return time_value.tv_sec * MICROSECONDS_PER_SECOND + time_value.tv_usec;// - Timer_perf_counter_base);
#endif
}

// 0 means invalid,
// 1 means always return true
// 2 and above actually check the time
int timestamp_ticker = 2;

void timestamp_reset()
{
	timestamp_ticker = 2;
}

// Restrict all time values between 0 and MAX_TIME
// so we don't have to use UINTs to calculate rollover.
// For debugging & testing, you could set this to 
// something like 1 minute (6000).
#define MAX_TIME (INT_MAX/2)

void timestamp_inc(int frametime_ms)
{
	timestamp_ticker += frametime_ms;

	if ( timestamp_ticker > MAX_TIME )	{
		timestamp_ticker = 2;		// Roll!
	}

	if (timestamp_ticker < 2 ) {
		mprintf(("Whoa!!!  timestamp_ticker < 2 -- resetting to 2!!!\n"));
		timestamp_ticker = 2;
	}
}

int timestamp(int delta_ms )
{
	int t2;
	if (delta_ms < 0 ) return 0;
	if (delta_ms == 0 ) return 1;
	t2 = timestamp_ticker + delta_ms;
	if ( t2 > MAX_TIME )	{
		// wrap!!!
		t2 = delta_ms - (MAX_TIME-timestamp_ticker);
	}
	if (t2 < 2 ) t2 = 2;	// hack??
	return t2;
}

//	Returns milliseconds until timestamp will elapse.
//	Negative value gives milliseconds ago that timestamp elapsed.
int timestamp_until(int stamp)
{
	// JAS: FIX
	// HACK!! This doesn't handle rollover!
	// (Will it ever happen?)
	
	return stamp - timestamp_ticker;

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
int timestamp()
{
	return timestamp_ticker;
}

int timestamp_has_time_elapsed(int stamp, int time)
{
	int t;

	if (time <= 0)
		return 1;

	t = stamp + time;
	if (t <= timestamp_ticker)
		return 1;  // if we are unlucky enough to have it wrap on us, this will assume time has elapsed.

	return 0;
}

