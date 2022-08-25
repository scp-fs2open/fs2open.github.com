#include "network/multi_interpolate.h"
#include "globalincs/pstypes.h"
#include "freespace.h"

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

// the meat and potatoes.  Basically, this figures out if we should interpolate, and then interpolates or sims
void interpolation_manager::interpolate(vec3d* pos, matrix* ori, physics_info* pip, vec3d* last_pos, matrix* last_orient, bool player_ship)
{
	// make sure its a valid ship and valid mode
	Assert(Game_mode & GM_MULTIPLAYER);

	// if we have not received enough packets, simulate then return, 
	// To optimize, we should not reassess_packet_index with a negative index.  
	// The index will be made positive by add_packet, once a second packet has been received.
	if (_upcoming_packet_index < 0 ) {
		*last_pos = *pos;
		*last_orient = *ori;

		physics_sim(pos, ori, pip, flFrametime);

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
			*pos = _packets.front().position;
			*ori = _packets.front().orientation;
			pip->vel = _packets.front().velocity;
			pip->desired_vel = _packets.front().desired_velocity;
			pip->rotvel = _packets.front().rotational_velocity;
			pip->desired_rotvel = _packets.front().desired_rotational_velocity;

			sim_time -= (static_cast<float>(_packets.front().remote_missiontime) - static_cast<float>(Multi_Timing_Info.get_last_time())) / TIMESTAMP_FREQUENCY;
			_packets_expended = true;
		}

		sim_time = (sim_time > 0.25f) ? 0.25f : sim_time;

		// at some point, we may want to do something fancier, but for now...
		*last_orient = *ori;

		physics_sim(pos, ori, pip, sim_time);

		// we can't trust what the last position was on the local instance, so figure out what it should have been
		// use flFrametime here because we need to know what the last position would have been if it was accurate in the last frame.
		vm_vec_scale_add(last_pos, pos, &pip->vel, -flFrametime);

		// duplicate the rest of the physics engine's calls here to make the simulation more exact.
		pip->speed = vm_vec_mag(&pip->vel);
		pip->fspeed = vm_vec_dot(&ori->vec.fvec, &pip->vel);

		return; // we should not try interpolating and siming, so return.
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
	vec3d temp_vector;

	// set new position.
	vm_vec_sub(&temp_vector, &_packets[_upcoming_packet_index].position, &_packets[_prev_packet_index].position);
	vm_vec_scale_add(pos, &_packets[_prev_packet_index].position, &temp_vector, scale);

	// set new velocity
	vm_vec_sub(&temp_vector, &_packets[_upcoming_packet_index].velocity, &_packets[_prev_packet_index].velocity);
	vm_vec_scale_add(&pip->vel, &_packets[_prev_packet_index].velocity, &temp_vector, scale);

	// we can't trust what the last position was on the local instance, so figure out what it should have been
	// use flFrametime here because we need to know what the last position would have been if it was accurate in the last frame.
	vm_vec_scale_add(last_pos, pos, &pip->vel, -flFrametime);

	// set new desired velocity
	vm_vec_sub(&temp_vector, &_packets[_upcoming_packet_index].desired_velocity, &_packets[_prev_packet_index].desired_velocity);
	vm_vec_scale_add(&pip->desired_vel, &_packets[_prev_packet_index].desired_velocity, &temp_vector, scale);

	// set new rotational velocity
	vm_vec_sub(&temp_vector, &_packets[_upcoming_packet_index].rotational_velocity, &_packets[_prev_packet_index].rotational_velocity);
	vm_vec_scale_add(&pip->rotvel, &_packets[_prev_packet_index].rotational_velocity, &temp_vector, scale);

	// we only do desired rotational velocity if this is a player ship.
	if (player_ship) {
		vm_vec_sub(&temp_vector, &_packets[_upcoming_packet_index].desired_rotational_velocity, &_packets[_prev_packet_index].desired_rotational_velocity);
		vm_vec_scale_add(&pip->desired_rotvel, &_packets[_prev_packet_index].desired_rotational_velocity, &temp_vector, scale);
	} // So if AI, just set them to the same value.
	else {
		pip->desired_rotvel = pip->rotvel;
	}

	// calculate the new orientation.
	// at some point, we may want to do something fancier, but for now...
	*last_orient = *ori;
	*ori = vm_interpolate_matrices(&_packets[_prev_packet_index].orientation, &_packets[_upcoming_packet_index].orientation, scale);



	// duplicate the rest of the physics engine's calls here to make the simulation more exact.
	pip->speed = vm_vec_mag(&pip->vel);
	pip->fspeed = vm_vec_dot(&ori->vec.fvec, &pip->vel);
}

// add a packet to the vector, remove the last one if necessary.
void interpolation_manager::add_packet(int frame, int packet_timestamp, vec3d* position, vec3d* velocity, vec3d* rotational_velocity, vec3d* desired_velocity, vec3d* desired_rotational_velocity, angles* angles, int player_index) 
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
	_packets[index].position = *pos;
	_packets[index].velocity = pip->vel;
	_packets[index].desired_velocity = pip->desired_vel;
	_packets[index].rotational_velocity = pip->rotvel;
	_packets[index].desired_rotational_velocity = pip->desired_rotvel; 
	_packets[index].orientation = *orient;
}
