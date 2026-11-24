#include "network/multi_interpolate.h"
#include "freespace.h"

SCP_unordered_map<int, interpolation_manager> Interp_info;

extern void multi_ship_record_signal_update(int objnum, TIMESTAMP lower_time_limit, TIMESTAMP upper_time_limit, int prev_packet_index, int current_packet_index);
///////////////////////////////////////////
// interpolation info management functions

// seeks through the packets to find the one that we need, starting from the end, notice we cannot use MULTIPLAYER_CLIENT macro here.  We cannot include multi.h
void interpolation_manager::reassess_packet_index(vec3d* pos, matrix* ori, physics_info* pip) 
{
	// Must be the playback clock, not the raw local clock.  _packets hold timestamps
	// stamped on the source machine's clock, and nothing ever synchronized the two.
	const double current_time = Multi_Timing_Info.get_playback_time_precise(_source_player_index);
	int current_index = static_cast<int>(_packets.size()) - 2;
	int prev_index = static_cast<int>(_packets.size()) - 1;

	const bool index_only = ( !pos || !ori || !pip );

	// iterate through the packets
	for (; current_index > -1; current_index--, prev_index--) {

		// did we find where we should interpolate?
		if ((_packets[current_index].remote_missiontime >= current_time) && (_packets[prev_index].remote_missiontime <= current_time)) {
			_upcoming_packet_index = current_index;
			_prev_packet_index = prev_index;

			// probably the "hackiest" thing about this.  If we were just straight simulating, 
			// and we now need to go back, pretend that the position we were in *was* our old packet
			// and we are now going towards our new packet's physics.
			if (_simulation_mode && !index_only) {
				replace_packet(prev_index, pos, ori, pip); // TODO, if simulation mode was forced by the collision code, this method regresses a bug where collisions instantly kill
				_simulation_mode = false;
			}

			return;
		}
	}

	if ( !index_only ) {
		// if we didn't find indexes then we are overwhelmingly likely to have passed the server somehow
		// and we need to make sure that we just straight simulate these ships
		_simulation_mode = true;
	}
}

void interpolate_main_helper(int objnum, vec3d* pos, matrix* ori, physics_info* pip, vec3d* last_pos, matrix* last_orient, vec3d* gravity, bool player_ship)
{
	Interp_info[objnum].interpolate_main(pos, ori, pip, last_pos, last_orient, gravity, player_ship);
}

// the meat and potatoes.  Basically, this figures out if we should interpolate, and then interpolates or sims
void interpolation_manager::interpolate_main(vec3d* pos, matrix* ori, physics_info* pip, vec3d* last_pos, matrix* last_orient, vec3d * gravity, bool player_ship)
{
	// make sure its a valid ship and valid mode
	Assert(Game_mode & GM_MULTIPLAYER);

	// if we have not received enough packets, simulate then return, 
	// To optimize, we should not reassess_packet_index with a negative index.  
	// The index will be made positive by add_packet, once a second packet has been received.
	if (_upcoming_packet_index < 0 ) {
		*last_pos = *pos;
		*last_orient = *ori;

		physics_sim(pos, ori, pip, gravity, flFrametime);

		// duplicate the rest of the physics engine's calls here to make the simulation more exact.
		pip->speed = vm_vec_mag(&pip->vel);
		pip->fspeed = vm_vec_dot(&ori->vec.fvec, &pip->vel);

		return;
	}

	reassess_packet_index(pos, ori, pip);

	// if we are off the beaten path
	if(_simulation_mode) {

		float sim_time = flFrametime;

		// we need to push this ship up to the limit of where we were on the remote instance, if we haven't already.
		// then we need to adjust our timing since some of the sim time is used up getting to that last packet.
		if (!_packets_expended && !_packets.empty()) {
			// Timestamps descend with index, so the bracket search can only have failed two
			// ways: our playback clock ran past the newest packet, or it is behind even the
			// oldest one we hold.  Dead reckon from whichever end we actually fell off --
			// always taking the newest would teleport the ship a whole buffer forward in the
			// second case.  Both terms are on the source's clock, hence the playback clock.
			const int playback_last = Multi_Timing_Info.get_playback_last_time(_source_player_index);

			const packet_info& reference = (playback_last >= _packets.back().remote_missiontime)
				? _packets.front() : _packets.back();

			physics_apply_snapshot_manual(*pos, *ori, pip->vel, pip->desired_vel, pip->rotvel, pip->desired_rotvel, reference.snapshot);

			sim_time -= (static_cast<float>(reference.remote_missiontime) - static_cast<float>(playback_last)) / TIMESTAMP_FREQUENCY;
			_packets_expended = true;
		}

		// A negative sim_time means our playback clock has not converged on this source
		// yet.  physics_sim will happily integrate backwards through it, which corrupts
		// the ship's state rather than just rewinding it, so clamp both ends.
		CLAMP(sim_time, 0.0f, 0.25f);

		// Catching up in one big step is not the same as taking it a frame at a time --
		// physics_sim_rot and the velocity damping are both nonlinear in time, so a
		// turning ship lands somewhere slightly wrong.  Sub-step it.
		constexpr float MAX_SIM_SUBSTEP = 1.0f / 60.0f;

		while (sim_time > 0.0001f) {
			const float step = MIN(sim_time, MAX_SIM_SUBSTEP);
			physics_sim(pos, ori, pip, gravity, step);
			sim_time -= step;
		}

		// we can't trust what the last position was on the local instance, so figure out what it should have been
		// use flFrametime here because we need to know what the last position would have been if it was accurate in the last frame.
		vm_vec_scale_add(last_pos, pos, &pip->vel, -flFrametime);

		// Asteroth's method for last orient.
		if (!IS_VEC_NULL(&pip->rotvel)) {

			vec3d normalized_rotvel;
			float mag = vm_vec_copy_normalize(&normalized_rotvel, &pip->rotvel);

			matrix rotate_to_previous;

			vm_quaternion_rotate(&rotate_to_previous, -mag * flFrametime, &normalized_rotvel);
			vm_matrix_x_matrix(last_orient, &rotate_to_previous, ori);
		} else {
			*last_orient = *ori;
		}
		
		// duplicate the rest of the physics engine's calls here to make the simulation more exact.
		pip->speed = vm_vec_mag(&pip->vel);
		pip->fspeed = vm_vec_dot(&ori->vec.fvec, &pip->vel);

		return; // we should not try interpolating and siming on the same call, so return.
	}

	// calc what the current timing should be.  Playback clock, for the same reason as in reassess_packet_index.
	float numerator = static_cast<float>(Multi_Timing_Info.get_playback_time_precise(_source_player_index) - _packets[_prev_packet_index].remote_missiontime);
	float denominator = static_cast<float>(_packets[_upcoming_packet_index].remote_missiontime) - static_cast<float>(_packets[_prev_packet_index].remote_missiontime);
	
	// work around for weird situations that might cause NAN (you just never know with multi)
	denominator = (denominator > 0.05f) ? denominator : 0.05f;
	
	float scale = numerator / denominator;

	// protect against bad floating point arithmetic making orientation or position look off
	CLAMP(scale, 0.001f, 0.999f);

	// one by one interpolate the vectors to get the desired results.
	physics_snapshot temp_state;

	// Interpolation in just two lines!  Who'da thunk?
	physics_interpolate_snapshots(temp_state, _packets[_prev_packet_index].snapshot, _packets[_upcoming_packet_index].snapshot, scale);
	physics_apply_snapshot_manual(*pos, *ori, pip->vel, pip->desired_vel, pip->rotvel, pip->desired_rotvel, temp_state);

	// we can't trust what the last position was on the local instance, so figure out what it should have been
	// use flFrametime here because we need to know what the last position would have been if it was accurate in the last frame.
	vm_vec_scale_add(last_pos, pos, &pip->vel, -flFrametime);

	// AI ships do not really use desired velocity, so undo that calc for AI ships.
	if (!player_ship) {
		pip->desired_rotvel = pip->rotvel;
	}

	// finally, a quick calculation for the last orientation, courtesy Asteroth
	if (!IS_VEC_NULL(&pip->rotvel)) {

		vec3d normalized_rotvel;
		float mag = vm_vec_copy_normalize(&normalized_rotvel, &pip->rotvel);

		matrix rotate_to_previous;

		vm_quaternion_rotate(&rotate_to_previous, -mag * flFrametime, &normalized_rotvel);
		vm_matrix_x_matrix(last_orient, &rotate_to_previous, ori);
	} else {
		*last_orient = *ori;
	}

	// duplicate the rest of the physics engine's calls here to make the simulation more exact.
	pip->speed = vm_vec_mag(&pip->vel);
	pip->fspeed = vm_vec_dot(&ori->vec.fvec, &pip->vel);
}

// correct the ship record for player ships when an up to date packet comes in.
void interpolation_manager::reinterpolate_previous(TIMESTAMP stamp, int prev_packet_index, int next_packet_index,  vec3d& position, matrix& orientation, vec3d& velocity, vec3d& rotational_velocity)
{
	// calc what the timing was previously.  The caller hands us an absolute local timestamp,
	// but remote_missiontime is relative to the *source's* mission start, so drop our start
	// time and then move the result onto the source's clock before comparing.
	int local_time = stamp.value() - Multi_Timing_Info.get_mission_start_time();
	float source_time = static_cast<float>(Multi_Timing_Info.local_time_to_remote(_source_player_index, local_time));
	float numerator = source_time - static_cast<float>(_packets[prev_packet_index].remote_missiontime);
	float denominator = static_cast<float>(_packets[next_packet_index].remote_missiontime) - static_cast<float>(_packets[prev_packet_index].remote_missiontime);

	denominator = (denominator > 0.05f) ? denominator : 0.05f;
	
	float scale = numerator / denominator;

	// protect against bad floating point arithmetic making orientation or position look off
	CLAMP(scale, 0.001f, 0.999f);

	physics_snapshot temp_snapshot;

	physics_interpolate_snapshots(temp_snapshot, _packets[prev_packet_index].snapshot, _packets[next_packet_index].snapshot, scale);
	physics_apply_snapshot_manual(position, orientation, velocity, rotational_velocity, temp_snapshot);
}

// add a packet to the vector, remove the last one if necessary.
void interpolation_manager::add_packet(int objnum, int frame, int packet_timestamp, vec3d* position, vec3d* velocity, vec3d* rotational_velocity, vec3d* desired_velocity, vec3d* desired_rotational_velocity, angles* angles, int player_index) 
{
	// Every timestamp in _packets is on this source's clock, and every comparison against
	// them goes through it, so keep it current rather than only setting it on the first packet.
	_source_player_index = player_index;

	// feed the clock servo, so that get_playback_time() can put our local clock on this
	// source's timeline
	Multi_Timing_Info.note_packet_time(player_index, packet_timestamp);

	if (_packets.empty()) {
		_packets.push_back(packet_info(frame, packet_timestamp, position, velocity, rotational_velocity, desired_velocity, desired_rotational_velocity, angles));

		return;
	}

	// go through each packet and compare frames. If the frame is equal, dump it.
	for (auto packet = _packets.begin(); packet != _packets.end(); packet++) {
		 // this packet was already received somehow, no need to record anything.
		if (frame == packet->frame){
			return;
		}

		// once we find the right place for this packet, insert and then bail, all other cases are meaningless
		if (frame > packet->frame) {

			// if this is now the most recent packet, then make sure the next round of interpolation uses its position, even if the packet came in too late.
			if (packet == _packets.begin()) {
				_packets_expended = false;
			}

			// yes, insert is somewhat inefficient here, but it *greatly* simplifies the code
			_packets.insert(packet, packet_info(frame, packet_timestamp, position, velocity, rotational_velocity, desired_velocity, desired_rotational_velocity, angles));
			
			// when the size of vector is 2, then we have just received enough pakets to begin interpolating,
			// and we need to mark this object as ready to interpolate
			if (_packets.size() == 2) {
				_upcoming_packet_index = 0;
				_prev_packet_index = 1;
			}

			if (_packets.size() > PACKET_INFO_LIMIT) {
				_packets.pop_back();
			}

			if ((_upcoming_packet_index >= 0) && (_prev_packet_index >= 0)) {
				// update packet indexes (ignoring simulation mode)
				// NOTE: indexes must be valid before being reassesed!
				reassess_packet_index();

				// whenenver the server gets a player packet, we need to update the ship record, since the old info is now stale
				if (Objects[objnum].flags[Object::Object_Flags::Player_ship]){
					int start_time = Multi_Timing_Info.get_mission_start_time();

					// The ship record is keyed on *local* absolute timestamps, but these are the
					// source's clock, so pull them back onto ours before adding our mission start.
					auto to_local_stamp = [&](int remote_time) {
						return TIMESTAMP(start_time + Multi_Timing_Info.remote_time_to_local(_source_player_index, remote_time));
					};

					multi_ship_record_signal_update(objnum, to_local_stamp(_packets[_prev_packet_index].remote_missiontime), to_local_stamp(_packets[_upcoming_packet_index].remote_missiontime), _prev_packet_index, _upcoming_packet_index);

					// if it's not the front packet, we need to update more info past the current packet, as well.
					// Should be rare though as it is a contingency for out of order packets.
					if (_upcoming_packet_index > 0) {
						multi_ship_record_signal_update(objnum, to_local_stamp(_packets[_upcoming_packet_index].remote_missiontime), to_local_stamp(_packets[_upcoming_packet_index - 1].remote_missiontime), _upcoming_packet_index, _upcoming_packet_index - 1);
					}
				}
			}

			return;
		}
	}
}

// basically, copy the state from the object that had been simulated up to that point and treat it as the "old packet".
// should never replace index 0
void interpolation_manager::replace_packet(int index, vec3d* pos, matrix* orient, physics_info* pip) 
{
	Assertion(index >= 1, "Invalid replace interpolation packet index! (%d)", index);
	if (index < 1) {
		return;
	}

	// the hackiest part of the hack? Setting its frame. Let FSO think that it was basically brand new.
	// it needs to handle it this way because otherwise another packet might get placed in front of it,
	// and we lose our intended effect of interpolating the simulation error away.
	_packets[index].frame = _packets[index - 1].frame - 1;

	// ...and that invented frame number must never reach the server as a rollback reference
	_packets[index].synthetic = true;

	// This slot is about to be compared against real packets, which are stamped on the
	// source's clock, so it has to be stamped on that clock too -- get_last_time() alone
	// is our raw local clock and would drag the whole clock offset into the interpolation
	// denominator.
	int replacement_time = Multi_Timing_Info.get_playback_last_time(_source_player_index);

	// _packets is ordered newest first, and the bracket search relies on the timestamps
	// descending with it.  Never let the replacement stamp fall below the next older packet.
	if ((index + 1) < static_cast<int>(_packets.size())) {
		replacement_time = MAX(replacement_time, _packets[index + 1].remote_missiontime);
	}

	_packets[index].remote_missiontime = replacement_time;

	physics_populate_snapshot_manual(_packets[index].snapshot, *pos, *orient, pip->vel, pip->desired_vel, pip->rotvel, pip->desired_rotvel);
}

// Name the moment we last drew this object at, in the terms the server's rollback record
// understands.  The naive answer -- newest packet received, plus time since it arrived -- is
// what the fire packets used to send, and it is MULTI_INTERP_BUFFER_MS newer than what was
// actually on screen, so every rolled-back shot was aimed at where the target had not got to yet.
bool interpolation_manager::get_render_reference(int& frame, int& time_after_frame) const
{
	if (_packets.empty()) {
		return false;
	}

	const int playback = Multi_Timing_Info.get_playback_time(_source_player_index);

	// _packets runs newest first, so the first entry at or before the playback clock is the
	// one the rendered position sits just after.  Synthetic entries have to be skipped --
	// replace_packet() invents their frame numbers, and the server would rewind to whatever
	// unrelated frame that number happens to name.
	for (const auto& packet : _packets) {
		if (packet.synthetic || (packet.remote_missiontime > playback)) {
			continue;
		}

		frame = packet.frame;
		time_after_frame = playback - packet.remote_missiontime;

		return true;
	}

	return false;
}

bool multi_interpolate_get_render_reference(int objnum, int& frame, int& time_after_frame)
{
	// deliberately find() rather than operator[], so a lookup for an object we never
	// tracked does not create an empty entry for it
	auto entry = Interp_info.find(objnum);

	if (entry == Interp_info.end()) {
		return false;
	}

	return entry->second.get_render_reference(frame, time_after_frame);
}

// the contained vectors have been cleared during object shut down.
void multi_interpolate_clear_all()
{
	// clear the main container.
	Interp_info.clear();
}

// helper functiont that helps avoid include issues.
void multi_interpolate_clear_helper(int objnum) {
	Interp_info[objnum].clean_up();
}
