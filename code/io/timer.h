/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _TIMER_H
#define _TIMER_H

#include "globalincs/pstypes.h"

//==========================================================================
// This installs the timer services and interrupts at the rate specified by
// count_val.  If 'function' isn't 0, the function pointed to by function will
// be called 'freq' times per second.  Should be > 19 and anything around
// 2-3000 is gonna start slowing down the system.  Count_val should be
// 1,193,180 divided by your target frequency. Use 0 for the normal 18.2 Hz
// interrupt rate.

#ifdef _WIN32
#define TIMER_FREQUENCY 1193180
#endif

extern void timer_init();
extern void timer_close();
extern void timer_set_rate(int count_val);
extern void timer_set_function( void * function );

//==========================================================================
// These functions return the time since the timer was initialized in
// some various units. The total length of reading time varies for each
// one.  They will roll around after they read 2^32.
// There are milliseconds, milliseconds times 10, milliseconds times 100,
// and microseconds.  They time out after 1000 hrs, 100 hrs, 10 hrs, and
// 1 hr, respectively.

extern fix timer_get_fixed_seconds();		// Rolls about every 9 hours...
extern fix timer_get_fixed_secondsX();		// Assume interrupts already disabled
extern fix timer_get_approx_seconds();		// Returns time since program started... accurate to 1/120th of a second
extern int timer_get_milliseconds();		//
extern int timer_get_microseconds();
extern uint timer_get_high_res_microseconds();
extern int timer_get_seconds();				// seconds since program started... not accurate, but good for long
											//     runtimes with second-based timeouts

//==========================================================================
// Use to access the BIOS ticker... ie...   i = TICKER
void timer_delay(fix seconds);


//=================================================================
//=================================================================
//               T I M E S T A M P   F U N C T I O N S
//=================================================================
//=================================================================

// NEVER USE THIS DIRECTLY!!! IF YOU REALLY NEED IT, THEN:
// call timestamp(0) and use TIMESTAMP_FREQUENCY to scale it.
extern int timestamp_ticker;	

// You shouldn't use the output of timestamp() directly, 
// but if you have to, use the TIMESTAMP_FREQUENCY to 
// scale it correctly.
#define TIMESTAMP_FREQUENCY 1000

// Call this at least every 600 hours, I would
// say at the start of each level should do it.
extern void timestamp_reset();

// Call this once every frame with the frametime.
extern void timestamp_inc(float frametime);

// To do timing, call this with the interval you
// want to check.  Then, pass this to timestamp_elapsed
// to see if delta_ms time has elapsed.   If delta_ms is
// zero, the next call to timestamp_elapsed will always
// return 1.  If delta_ms is less than zero, then this is
// considered an invalid timestamp and all calls to 
// timestamp_elapsed will return 0.
// In other words:
// pass -1 for an invalid timestamp that will never time out
// pass 0 for a timestamp that is instantly timed out
// pass n > 0 for timestamp n milliseconds in the future.
int timestamp(int delta_ms );

// use this call to get the current counter value (which represents the time at the time
// this function is called).  I.e. it doesn't return a count that would be in the future,
// but the count that is right now.
int timestamp();

// gets a timestamp randomly between a and b milliseconds in
// the future.
#define timestamp_rand(a,b) timestamp((myrand()%((b)-(a)+1))+(a))

// Example that makes a ship fire in 1/2 second

// ...
// ship->next_fire = timestamp(500);
// ...
// if (fire && timestamp_elapsed(ship->next_fire))
//   fire_laser();

#define timestamp_elapsed( stamp ) ( (stamp!=0) ? (timestamp_ticker >= (stamp) ? 1 : 0) : 0 )

#define timestamp_valid(stamp) ((stamp==0) ? 0 : 1 )

//	Returns millliseconds until timestamp will elapse.
int timestamp_until(int stamp);

// checks if a specified time (in milliseconds) has elapsed past the given timestamp (which
// should be obtained from timestamp() or timestamp(x) with a positive x)
int timestamp_has_time_elapsed(int stamp, int time);

// safer version of timestamp
#define timestamp_elapsed_safe(_a, _b)		( (_a != 0) ? (((timestamp_ticker >= (_a)) || (timestamp_ticker < (_a - (_b + 100)))) ? 1 : 0) : 1 )


// timing functions -------------------------------------------------------------------------------

// start timing frame stuff
void timing_frame_start();

// done timing the frame
void timing_frame_stop();

// get the total frame time in microseconds
int timing_frame_total();

// time an individual event
void timing_event_start(char *event_name);

// stop timing an event
void timing_event_stop(char *event_name);

// get the total time for an event in microseconds
int timing_event_total(char *event_name);

// get the percentage of total frametime for the event (0.0 to 1.0)
float timing_event_pct(char *event_name);

// display timing 
void timing_display(int x, int y);

#endif
