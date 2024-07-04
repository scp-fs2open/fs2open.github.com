#include "network/multi_interpolate.h"
#include "freespace.h"

SCP_unordered_map<int, interpolation_manager> Interp_info;

extern void multi_ship_record_signal_update(int objnum, TIMESTAMP lower_time_limit, TIMESTAMP upper_time_limit, int prev_packet_index, int current_packet_index);
///////////////////////////////////////////
// interpolation info management functions

// seeks through the packets to find the one that we need, starting from the end, notice we cannot use MULTIPLAYER_CLIENT macro here.  We cannot include multi.h
void interpolation_manager::reassess_packet_index(vec3d* pos, matrix* ori, physics_info* pip) 
{
	auto current_time = Multi_Timing_Info.get_current_time();	
	int current_index = static_cast<int>(_packets.size()) - 2;
	int prev_index = static_cast<int>(_packets.size()) - 1;

	// iterate through the packets
	for (; current_index > -1; current_index--, prev_index--) {

		// did we find where we should interpolate?
		if ((_packets[current_index].remote_missiontime >= current_time) && (_packets[prev_index].remote_missiontime <= current_time)) {
			_upcoming_packet_index = current_index;
			_prev_packet_index = prev_index;

			// probably the "hackiest" thing about this.  If we were just straight simulating, 
			// and we now need to go back, pretend that the position we were in *was* our old packet
			// and we are now going towards our new packet's physics.
			if (_simulation_mode) {
				replace_packet(prev_index, pos, ori, pip); // TODO, if simulation mode was forced by the collision code, this method regresses a bug where collisions instantly kill
				_simulation_mode = false;
			}

			return;
		}
	}

	// if we didn't find indexes then we are overwhelmingly likely to have passed the server somehow
	// and we need to make sure that we just straight simulate these ships
	_simulation_mode = true;
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
			physics_apply_snapshot_manual(*pos, *ori, pip->vel, pip->desired_vel, pip->rotvel, pip->desired_rotvel, _packets.front().snapshot);

			sim_time -= (static_cast<float>(_packets.front().remote_missiontime) - static_cast<float>(Multi_Timing_Info.get_last_time())) / TIMESTAMP_FREQUENCY;
			_packets_expended = true;
		}

		sim_time = (sim_time > 0.25f) ? 0.25f : sim_time;

		physics_sim(pos, ori, pip, gravity, sim_time);

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

	// calc what the current timing should be.
	float numerator = static_cast<float>(_packets[_upcoming_packet_index].remote_missiontime) - static_cast<float>(Multi_Timing_Info.get_current_time());
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
	// calc what the timing was previously.
	float numerator = static_cast<float>(_packets[next_packet_index].remote_missiontime) - static_cast<float>(stamp.value());
	float denominator = static_cast<float>(_packets[next_packet_index].remote_missiontime) - static_cast<float>(_packets[prev_packet_index].remote_missiontime);

	denominator = (denominator > 0.05f) ? denominator : 0.05f;
	
	float scale = numerator / denominator;

	// protect against bad floating point arithmetic making orientation or position look off
	CLAMP(scale, 0.001f, 0.999f);

	physics_snapshot temp_snapshot;

	physics_interpolate_snapshots(temp_snapshot, _packets[_prev_packet_index].snapshot, _packets[_upcoming_packet_index].snapshot, scale);
	physics_apply_snapshot_manual(position, orientation, velocity, rotational_velocity, temp_snapshot);
}

// add a packet to the vector, remove the last one if necessary.
void interpolation_manager::add_packet(int objnum, int frame, int packet_timestamp, vec3d* position, vec3d* velocity, vec3d* rotational_velocity, vec3d* desired_velocity, vec3d* desired_rotational_velocity, angles* angles, int player_index) 
{
	if (_packets.empty()) {
		_packets.push_back(packet_info(frame, packet_timestamp, position, velocity, rotational_velocity, desired_velocity, desired_rotational_velocity, angles));

		_source_player_index = player_index;
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

			if (static_cast<int>(_packets.size()) > PACKET_INFO_LIMIT) {
				_packets.pop_back();
			}

			// whenenver the server gets a player packet, we need to update the ship record, since the old info is now stale
			if (Objects[objnum].flags[Object::Object_Flags::Player_ship]){

				int start_time = Multi_Timing_Info.get_mission_start_time();

				multi_ship_record_signal_update(objnum, TIMESTAMP(start_time + _packets[_prev_packet_index].remote_missiontime), TIMESTAMP(start_time + _packets[_upcoming_packet_index].remote_missiontime), _prev_packet_index, _upcoming_packet_index);

				// if it's not the front packet, we need to update more info past the current packet, as well.  
				// Should be rare though as it is a contingency for out of order packets.
				if (_upcoming_packet_index != 0){
					multi_ship_record_signal_update(objnum, TIMESTAMP(start_time + _packets[_upcoming_packet_index].remote_missiontime), TIMESTAMP(start_time + _packets[_upcoming_packet_index - 1].remote_missiontime), _upcoming_packet_index, _upcoming_packet_index - 1);
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
	// the hackiest part of the hack? Setting its frame. Let FSO think that it was basically brand new.
	// it needs to handle it this way because otherwise another packet might get placed in front of it, 
	// and we lose our intended effect of interpolating the simulation error away.
	_packets[index].frame = _packets[index - 1].frame - 1;

	_packets[index].remote_missiontime = Multi_Timing_Info.get_last_time();

	physics_populate_snapshot_manual(_packets[index].snapshot, *pos, *orient, pip->vel, pip->desired_vel, pip->rotvel, pip->desired_rotvel);
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
