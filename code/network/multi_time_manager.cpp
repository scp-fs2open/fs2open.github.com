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
}


// checks to see if this is the most recent frame from the source player.
bool multiplayer_timing_info::is_most_recent_frame(int player_index, int frame) 
{
    // quick sanity check, but nothing to crash over
    if (player_index < 0) {
        mprintf(("MULTI INTERPOLATION most recent frame got a negative index."));
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
    _in_game_time_set = false;

    for (auto& frame : _most_recent_frame) {
        frame = NO_PACKET_RECEIVED;
    }
}

// aka reset the class. Needs to be called every time the mission starts.
void multiplayer_timing_info::set_mission_start_time() 
{
    _start_time = _timestamp();	// set it to the mission's starting time.

    _current_time = 0;
    _last_time = 0;
    _skipped_time = 0;
    _in_game_time_set = false;
}

void multiplayer_timing_info::in_game_set_skip_time()
{
    // This function is specifically for in-game joiners, and in-game joiners
    // receive Missiontime from the server as their are told to jump into the mission.

    if (!_in_game_time_set){
        extern fix Missiontime;

        // do a manual conversion, since we need milliseconds, not seconds. (Multiply first to avoid loss of data)
        auto time = (static_cast<std::int64_t>(Missiontime) * MILLISECONDS_PER_SECOND) / F1_0;
        // yeah, I know, a hack to subtract here, but if the client gets ahead of the server bad things happen
        _skipped_time = static_cast<int>(time) - 100;
        _in_game_time_set = true;
    }
}
