#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "io/timer.h"
#include "math/vecmat.h"
#include <climits>
#include <array>


// How far behind a source's newest packet the playback clock should sit, in milliseconds.
// This is the main tuning knob for interpolation.  It buys smoothness at the cost of
// showing remote ships slightly in the past, so:
//   - it must exceed a source's per-object update interval, or playback runs past the
//     newest packet before the next one lands and the object falls back to dead reckoning
//   - it must stay under (PACKET_INFO_LIMIT - 1) * that interval, or playback falls off
//     the back of the packet history and the object dead reckons for the opposite reason
// See the Multi_oo_*_update_times tables in multi_obj.cpp for the intervals in play.
constexpr int MULTI_INTERP_BUFFER_MS = 150;

// How long a window of packets to gather before re-aiming the servo, in milliseconds.
constexpr int MULTI_CLOCK_WINDOW_MS = 1000;

// A correction larger than this means the source's clock genuinely jumped (mission
// restart, in-game join, a huge stall) rather than drifted.  Snap instead of slewing;
// slewing across a gap this size would take painfully long and look worse.
constexpr int MULTI_CLOCK_SNAP_THRESHOLD_MS = 1000;


class multiplayer_timing_info {
private:
	TIMESTAMP _start_time;		// when did the multiplayer mission start
	int _current_time;		// time delta, how much time has passed since it started on the local instance?
	int _last_time;		// time delta, how much time passed, last frame? Useful when switching back from simulation mode to interpolation
	int _skipped_time;		// time delta, how much has time this instance has "skipped" because it is falling behind the server
									// getting behind the server like that *should* be exceedingly rare, should always be 0 on server

	int _proposed_skip_time; // until skip time is finalized, we need to

	bool _in_game_time_set;

	std::array<int, MAX_PLAYERS> _most_recent_frame;

	// Every machine starts its mission clock independently, at the end of its own
	// game_level_init(), and nothing ever reconciles them.  On top of that offset sits
	// the network delay of each individual packet.  So a timestamp that arrived from
	// another machine cannot be compared against _current_time directly.
	//
	// This tracks, per source, the offset that converts our local clock into one that
	// *is* comparable with that source's packet timestamps, deliberately biased to sit
	// MULTI_INTERP_BUFFER_MS behind that source's newest packet.  Interpolation reads
	// the result through get_playback_time().
	//
	// It has to be per-source: a client only ever hears from the server, but the server
	// hears from every client, and no two of those clocks agree.
	struct source_clock {
		bool acquired;			// have we heard from this source at all?
		int offset;				// applied, slewed offset. playback = _current_time + offset
		int target_offset;		// where the servo is heading

		int window_min_skew;	// smallest (local - remote) seen in the window so far
		bool window_has_sample;
		int window_end;			// local time at which we re-aim
	};

	std::array<source_clock, MAX_PLAYERS> _source_clocks;

	// for in-game joiners, adjust local timing and then reset proposed time.
	void finalize_skip_time() { _skipped_time += _proposed_skip_time;  _proposed_skip_time = 0; }

	void reset_source_clocks();
	void update_source_clocks();

	static bool valid_source(int player_index) { return (player_index >= 0) && (player_index < MAX_PLAYERS); }

public:
	multiplayer_timing_info();

	// aka reset the class. Needs to be called every time the mission starts.
	void set_mission_start_time();

	// this was not part of the original design, but is useful when matching up
	// timestamps to what is kept internally in this class.
	int get_mission_start_time() { return _start_time.value(); }

	void update_current_time();

	int get_current_time() { return _current_time; }

	int get_last_time() { return _last_time; }

	// Record that a packet stamped remote_time arrived from player_index.  Feeds the servo.
	void note_packet_time(int player_index, int remote_time);

	// Local time expressed on player_index's clock, biased to sit MULTI_INTERP_BUFFER_MS
	// behind that source's newest packet.  This -- not get_current_time() -- is what any
	// comparison against a received timestamp should use.
	int get_playback_time(int player_index) const;

	// Same, for the previous frame.
	int get_playback_last_time(int player_index) const;

	// Convert a timestamp received from player_index into the local _current_time that
	// corresponds to it.  Inverse of get_playback_time().
	int remote_time_to_local(int player_index, int remote_time) const;

	// Convert a local _current_time-relative value into player_index's clock, so it can
	// be compared against timestamps received from that source.
	int local_time_to_remote(int player_index, int local_time) const;

	bool source_clock_acquired(int player_index) const
	{
		return valid_source(player_index) && _source_clocks[player_index].acquired;
	}

	// push local time forward or back on clients based on received server times
	// this will likely only ever be used for in-game joining, which is not ready.
	//void set_proposed_skip_time(int candidate) { _proposed_skip_time = candidate; }

	bool is_most_recent_frame(int player_index, int frame);

	void in_game_set_skip_time(float mission_time);
};

extern multiplayer_timing_info Multi_Timing_Info;
