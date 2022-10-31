#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "io/timer.h"
#include "math/vecmat.h"
#include <climits>
#include <array>


class multiplayer_timing_info {
private:
	TIMESTAMP _start_time;		// when did the multiplayer mission start
	int _current_time;		// time delta, how much time has passed since it started on the local instance?
	int _last_time;		// time delta, how much time passed, last frame? Useful when switching back from simulation mode to interpolation
	int _skipped_time;		// time delta, how much has time this instance has "skipped" because it is falling behind the server
									// getting behind the server like that *should* be exceedingly rare, should always be 0 on server
	
	int _proposed_skip_time; // until skip time is finalized, we need to 

	bool _in_game_time_set;

	std::array<int, 12> _most_recent_frame;

	// for in-game joiners, adjust local timing and then reset proposed time.
	void finalize_skip_time() { _skipped_time += _proposed_skip_time;  _proposed_skip_time = 0; }

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

	// push local time forward or back on clients based on received server times
	// this will likely only ever be used for in-game joining, which is not ready.
	//void set_proposed_skip_time(int candidate) { _proposed_skip_time = candidate; }

	bool is_most_recent_frame(int player_index, int frame);

	void in_game_set_skip_time();
};

extern multiplayer_timing_info Multi_Timing_Info;
