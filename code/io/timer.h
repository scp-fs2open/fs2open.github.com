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

#include "utils/id.h"
#include "utils/Random.h"

#include <cstdint>

using Random = util::Random;

// "strong typedef", based on similar usage in gamesnd
struct ui_timestamp_tag {};
class UI_TIMESTAMP : public util::ID<ui_timestamp_tag, int, -1>
{
public:
	static UI_TIMESTAMP invalid() { return {}; }
	static UI_TIMESTAMP never() { return UI_TIMESTAMP(0); }
	static UI_TIMESTAMP immediate() { return UI_TIMESTAMP(1); }

	UI_TIMESTAMP() = default;
	explicit UI_TIMESTAMP(int val) : ID(val) { }

	inline bool isNever() const { return m_val == 0; }
	inline bool isImmediate() const { return m_val == 1; }
};


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

//==========================================================================
// These functions return the time since the timer was initialized in
// some various units. The total length of reading time varies for each
// one.  They will roll around after they read 2^32.
// There are milliseconds, milliseconds times 10, milliseconds times 100,
// and microseconds.  They time out after 1000 hrs, 100 hrs, 10 hrs, and
// 1 hr, respectively.

extern fix timer_get_fixed_seconds();		// Rolls about every 9 hours...
extern fix timer_get_approx_seconds();		// Returns time since program started... accurate to 1/120th of a second
extern int timer_get_milliseconds();		//
extern std::uint64_t timer_get_microseconds();
extern std::uint64_t timer_get_nanoseconds();
extern int timer_get_seconds();				// seconds since program started... not accurate, but good for long
											//     runtimes with second-based timeouts


//=================================================================
//=================================================================
//               T I M E S T A M P   F U N C T I O N S
//=================================================================
//=================================================================

// You shouldn't use the output of timestamp() directly, 
// but if you have to, use the TIMESTAMP_FREQUENCY to 
// scale it correctly.
#define TIMESTAMP_FREQUENCY 1000

// use this call to get the current counter value (which represents the time at the time
// this function is called).  I.e. it doesn't return a count that would be in the future,
// but the count that is right now.
int timestamp();

// same, but for use in the UI, so not subject to time compression or pauses
UI_TIMESTAMP ui_timestamp();

inline bool timestamp_valid(int stamp) {
	return stamp != 0;
}

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
int timestamp(int delta_ms);
UI_TIMESTAMP ui_timestamp(int delta_ms);

// gets a timestamp randomly between a and b milliseconds in
// the future.
inline int timestamp_rand(int a, int b) {
	return timestamp(Random::next(a, b));
}

//	Returns milliseconds until timestamp will elapse.
int timestamp_until(int stamp);
int ui_timestamp_until(UI_TIMESTAMP stamp);

// checks if a specified time (in milliseconds) has elapsed past the given timestamp (which
// should be obtained from timestamp() or timestamp(x) with a positive x)
int timestamp_has_time_elapsed(int stamp, int time);

// Example that makes a ship fire in 1/2 second

// ...
// ship->next_fire = timestamp(500);
// ...
// if (fire && timestamp_elapsed(ship->next_fire))
//   fire_laser();

bool timestamp_elapsed( int stamp );
bool ui_timestamp_elapsed( UI_TIMESTAMP stamp );

// safer version of timestamp
bool timestamp_elapsed_safe(int a, int b);

//=================================================================
//               T I M E S T A M P   A D J U S T M E N T
//=================================================================

// This should be called when the timestamp should stop ticking, e.g when the game is paused.
// The "sudo" is for cases where we want the time to remain paused, e.g. during level loading,
//     even if the game loses focus (which would normally unpause when focus is regained)
void timestamp_pause(bool sudo);

// This should be called when the timestamp should resume ticking, e.g. when the player is in-mission.
// See above re: the sudo parameter
void timestamp_unpause(bool sudo);

// for timestamp handling during debugging
bool timestamp_is_paused();
void timestamp_adjust_pause_offset(int delta_milliseconds);

enum class TIMER_DIRECTION { FORWARD, BACKWARD };
// Alters the timestamp time forward or backward.  Use with caution!
void timestamp_adjust_seconds(float delta_seconds, TIMER_DIRECTION dir);
void timestamp_adjust_microseconds(uint64_t delta_microseconds, TIMER_DIRECTION dir);

// This should be called when the game time compression is changed in any way, so that
// the timestamp will be consistent with the faster or slower time.
void timestamp_update_time_compression();

//=================================================================
//               M I S S I O N   T I M E
//=================================================================

// Save the timestamp corresponding to the beginning of the mission
void timestamp_start_mission();

// Restore the timestamp corresponding to the beginning of the mission, since we essentially start time twice
void timestamp_revert_to_mission_start();

// Calculate the current mission time using the timestamps
fix timestamp_get_mission_time();

#endif
