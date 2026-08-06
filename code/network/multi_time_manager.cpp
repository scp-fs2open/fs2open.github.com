#include "network/multi_time_manager.h"

// the conatiner for our timing info
multiplayer_timing_info Multi_Timing_Info;

/////////////////////////////////
// Time Records Manager functions

// make sure that we recalculate the current local time
void multiplayer_timing_info::update_current_time()
{
	// finalize the time skip from last frame.
	finalize_skip_time();

	// set our _last time.
	_last_time = _current_time;

	_current_time = timestamp_since(_start_time);

	_current_time += _skipped_time;

	// now that the local clock has moved, walk each source's playback clock toward its target
	update_source_clocks();
}


// checks to see if this is the most recent frame from the source player.
bool multiplayer_timing_info::is_most_recent_frame(int player_index, int frame)
{
    // quick sanity check, but nothing to crash over
    if (!valid_source(player_index)) {
        mprintf(("MULTI INTERPOLATION most recent frame got an out of range index (%d).\n", player_index));
        return false;
    }

    // use < and not <= here so that we don't duplicate adjustments to the current time
    // from the same frame
    if (_most_recent_frame[player_index] < frame) {
        _most_recent_frame[player_index] = frame;
        return true;
    }

    return false;
}

multiplayer_timing_info::multiplayer_timing_info()
{
    constexpr int NO_PACKET_RECEIVED = -1;

    _start_time = TIMESTAMP::invalid();
    _current_time = 0;
    _last_time = 0;
    _skipped_time = 0;
    _proposed_skip_time = 0;
    _in_game_time_set = false;

    for (auto& frame : _most_recent_frame) {
        frame = NO_PACKET_RECEIVED;
    }

    reset_source_clocks();
}

// aka reset the class. Needs to be called every time the mission starts.
void multiplayer_timing_info::set_mission_start_time()
{
    _start_time = _timestamp();	// set it to the mission's starting time.

    _current_time = 0;
    _last_time = 0;
    _skipped_time = 0;
    _proposed_skip_time = 0;
    _in_game_time_set = false;

    // the old mission's offsets say nothing about the new one
    reset_source_clocks();
}

void multiplayer_timing_info::in_game_set_skip_time(float mission_time)
{
    // This function is specifically for in-game joiners, and in-game joiners
    // receive Missiontime from the server as their are told to jump into the mission.

    if (!_in_game_time_set){
        _skipped_time = static_cast<int>(mission_time * MILLISECONDS_PER_SECOND);
        _in_game_time_set = true;

        // _current_time is about to move by a large step that has nothing to do with
        // elapsed time, so every offset measured against the old one is meaningless.
        reset_source_clocks();
    }
}

/////////////////////////////////
// Per-source playback clocks

void multiplayer_timing_info::reset_source_clocks()
{
    for (auto& sc : _source_clocks) {
        sc.acquired = false;
        sc.converged = false;
        sc.offset = 0;
        sc.target_offset = 0;
        sc.window_min_skew = INT_MAX;
        sc.window_has_sample = false;
        sc.window_end = 0;
    }
}

// Called for every position packet that arrives, from add_packet().
void multiplayer_timing_info::note_packet_time(int player_index, int remote_time)
{
    if (!valid_source(player_index)) {
        return;
    }

    auto& sc = _source_clocks[player_index];

    // How far our clock reads ahead of the timestamp this packet was stamped with.
    // This is (clock offset + that packet's network delay), and the delay is always
    // positive, so the *minimum* skew over a window is the cleanest estimate of the
    // offset alone -- it is the packet that got here fastest.
    const int skew = _current_time - remote_time;

    if (!sc.acquired) {
        // Nothing is drawing off this clock yet, so take the estimate immediately
        // instead of slewing in from a meaningless zero.  It is only a rough estimate
        // though -- one packet, measured while the mission is still starting up -- so
        // the acquisition window that follows is short and ends in another snap.
        sc.acquired = true;
        sc.converged = false;
        sc.offset = -skew - MULTI_INTERP_BUFFER_MS;
        sc.target_offset = sc.offset;
        sc.window_min_skew = skew;
        sc.window_has_sample = true;
        sc.window_end = _current_time + MULTI_CLOCK_ACQUIRE_WINDOW_MS;
        return;
    }

    if (!sc.window_has_sample || (skew < sc.window_min_skew)) {
        sc.window_min_skew = skew;
        sc.window_has_sample = true;
    }
}

// Once per frame, from update_current_time().
void multiplayer_timing_info::update_source_clocks()
{
    const int frame_delta = _current_time - _last_time;

    for (auto& sc : _source_clocks) {
        if (!sc.acquired) {
            continue;
        }

        // window closed, so re-aim at whatever the best packet in it told us
        if (_current_time >= sc.window_end) {
            if (sc.window_has_sample) {
                sc.target_offset = -sc.window_min_skew - MULTI_INTERP_BUFFER_MS;
            }

            // First real estimate.  Take it outright rather than slewing, which would cost
            // a couple of seconds of dead reckoning.  At mission start nothing is on screen
            // to be spoiled by the jump; for a source acquired later (an in-game joiner)
            // this is a single one-off correction on that player's ship alone.
            if (!sc.converged) {
                sc.converged = true;
                sc.offset = sc.target_offset;
            }

            sc.window_has_sample = false;
            sc.window_min_skew = INT_MAX;
            sc.window_end = _current_time + MULTI_CLOCK_WINDOW_MS;
        }

        const int diff = sc.target_offset - sc.offset;

        if (diff == 0) {
            continue;
        }

        // A correction this large is a real discontinuity in the source's clock, not
        // drift.  Slewing across it would take many seconds of visibly wrong motion.
        if ((diff >= MULTI_CLOCK_SNAP_THRESHOLD_MS) || (diff <= -MULTI_CLOCK_SNAP_THRESHOLD_MS)) {
            sc.offset = sc.target_offset;
            continue;
        }

        if (frame_delta <= 0) {
            continue;
        }

        // Cap the correction at a quarter of the frame.  Playback time is
        // (_current_time + offset), so an offset moving faster than the frame itself
        // would run playback backwards and drag every interpolated ship back with it.
        // At a quarter, playback always advances, between 0.75x and 1.25x real time.
        const int max_step = MAX(1, frame_delta / 4);

        if (diff > max_step) {
            sc.offset += max_step;
        } else if (diff < -max_step) {
            sc.offset -= max_step;
        } else {
            sc.offset = sc.target_offset;
        }
    }
}

int multiplayer_timing_info::get_playback_time(int player_index) const
{
    if (!valid_source(player_index) || !_source_clocks[player_index].acquired) {
        return _current_time;
    }

    return _current_time + _source_clocks[player_index].offset;
}

int multiplayer_timing_info::get_playback_last_time(int player_index) const
{
    if (!valid_source(player_index) || !_source_clocks[player_index].acquired) {
        return _last_time;
    }

    return _last_time + _source_clocks[player_index].offset;
}

int multiplayer_timing_info::remote_time_to_local(int player_index, int remote_time) const
{
    if (!valid_source(player_index) || !_source_clocks[player_index].acquired) {
        return remote_time;
    }

    // playback == _current_time + offset, so the local time matching remote_time
    // is remote_time - offset.
    return remote_time - _source_clocks[player_index].offset;
}

int multiplayer_timing_info::local_time_to_remote(int player_index, int local_time) const
{
    if (!valid_source(player_index) || !_source_clocks[player_index].acquired) {
        return local_time;
    }

    return local_time + _source_clocks[player_index].offset;
}
