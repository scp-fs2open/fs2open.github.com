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

// Empty structs used as type-safe markers.
struct ui_timestamp_tag {};
struct timestamp_tag {};

// These classes use the "strong typedef" pattern, based on
// similar usage in gamesnd, based in turn on this article:
// https://www.ilikebigbits.com/2014_05_06_type_safe_handles.html
// 
// Do not add additional fields or member functions to these classes
// (such as the functionality in timestamp_delta).  These classes must
// remain lightweight so that they can be optimized away.

// Timestamp measuring real time.  Runs regardless of time compression or pausing.
// Useful for timing real-world events and UI features like cursor blinking.
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

// Timestamp measuring game time.  Sensitive to time compression and pausing.
// Useful for timing in-mission events.
class TIMESTAMP : public util::ID<timestamp_tag, int, -1>
{
public:
	static TIMESTAMP invalid() { return {}; }
	static TIMESTAMP never() { return TIMESTAMP(0); }
	static TIMESTAMP immediate() { return TIMESTAMP(1); }

	TIMESTAMP() = default;
	explicit TIMESTAMP(int val) : ID(val) { }

	inline bool isNever() const { return m_val == 0; }
	inline bool isImmediate() const { return m_val == 1; }
};

// For converting from old-style timestamps:
//   A timestamp of 0 is invalid
//   A timestamp of 1 elapses immediately
//   A timestamp of 2 or more corresponds to that value of the timestamp timer
// See also the implementation of timestamp(int).  Also be careful not to confuse these values
// with the deltas; timestamp(-1) returns 0 and timestamp(0) returns 1.  But timestamp(1)
// returns the current value of timestamp_ms() plus 1 millisecond.  To add further confusion,
// legacy code often used -1 rather than 0 to indicate an invalid timestamp.


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
// TODO: remove when nobody uses the output directly
#define TIMESTAMP_FREQUENCY 1000

// conversion factors
constexpr int MILLISECONDS_PER_SECOND = 1000;
constexpr uint64_t MICROSECONDS_PER_SECOND = 1000000;
constexpr uint64_t NANOSECONDS_PER_SECOND = 1000000000;
constexpr uint64_t NANOSECONDS_PER_MICROSECOND = 1000;

// use this call to get the current counter value (which represents the time at the time
// this function is called).  I.e. it doesn't return a count that would be in the future,
// but the count that is right now.
int timestamp();
TIMESTAMP _timestamp();					// use a leading underscore for now until all timestamps are converted over

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
TIMESTAMP _timestamp(int delta_ms);		// use a leading underscore for now until all timestamps are converted over
UI_TIMESTAMP ui_timestamp(int delta_ms);

TIMESTAMP timestamp_delta(TIMESTAMP stamp, int delta_ms);
UI_TIMESTAMP ui_timestamp_delta(UI_TIMESTAMP stamp, int delta_ms);

int timestamp_get_delta(TIMESTAMP before, TIMESTAMP after);
int ui_timestamp_get_delta(UI_TIMESTAMP before, UI_TIMESTAMP after);

// gets a timestamp randomly between a and b milliseconds in
// the future.
inline int timestamp_rand(int a, int b) {
	return timestamp(Random::next(a, b));
}
inline TIMESTAMP _timestamp_rand(int a, int b) {
	return _timestamp(Random::next(a, b));
}

//	Returns milliseconds until timestamp will elapse.  Invalid timestamps are assumed to occur at approximately T=0.
int timestamp_until(int stamp);
//	Returns milliseconds until timestamp will elapse.  This will Assert against Invalid or Never timestamps but fail gracefully by returning INT_MAX.
int timestamp_until(TIMESTAMP stamp);
//	Returns milliseconds until timestamp will elapse.  This will Assert against Invalid or Never timestamps but fail gracefully by returning INT_MAX.
int ui_timestamp_until(UI_TIMESTAMP stamp);

//	Returns milliseconds after timestamp has elapsed.  Invalid timestamps are assumed to occur at approximately T=0.
int timestamp_since(int stamp);
//	Returns milliseconds after timestamp has elapsed.  This will Assert against Invalid or Never timestamps but fail gracefully by returning INT_MIN.
int timestamp_since(TIMESTAMP stamp);
//	Returns milliseconds after timestamp has elapsed.  This will Assert against Invalid or Never timestamps but fail gracefully by returning INT_MIN.
int ui_timestamp_since(UI_TIMESTAMP stamp);

// A standard comparison function that returns <0, 0, or >0 as the left item is less than, equal to, or greater than the right item.
int timestamp_compare(TIMESTAMP t1, TIMESTAMP t2);
// A standard comparison function that returns <0, 0, or >0 as the left item is less than, equal to, or greater than the right item.
int ui_timestamp_compare(UI_TIMESTAMP t1, UI_TIMESTAMP t2);

// Checks that a timestamp occurs between the "before" and "after" timestamps.
bool timestamp_in_between(TIMESTAMP stamp, TIMESTAMP before, TIMESTAMP after);
// Checks that a timestamp occurs between the "before" and "after" timestamps.
bool ui_timestamp_in_between(UI_TIMESTAMP stamp, UI_TIMESTAMP before, UI_TIMESTAMP after);

// Example that makes a ship fire in 1/2 second

// ...
// ship->next_fire = timestamp(500);
// ...
// if (fire && timestamp_elapsed(ship->next_fire))
//   fire_laser();

bool timestamp_elapsed( int stamp );
bool timestamp_elapsed( TIMESTAMP stamp );
bool ui_timestamp_elapsed( UI_TIMESTAMP stamp );

// safer version of timestamp
bool timestamp_elapsed_safe(int a, int b);
bool timestamp_elapsed_safe(TIMESTAMP a, int b);
bool ui_timestamp_elapsed_safe(UI_TIMESTAMP a, int b);

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

// Calculate the current mission time using the timestamps
fix timestamp_get_mission_time();
uint64_t timestamp_get_mission_time_in_microseconds();

#endif
