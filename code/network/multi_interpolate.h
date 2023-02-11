#pragma once

#include "network/multi_time_manager.h"

struct physics_info;

constexpr int PACKET_INFO_LIMIT = 4; // we should never need more than 4 packets to do interpolation.  Overwrite the oldest ones if we do.

typedef struct packet_info {

	int frame;							// this allows us to directly compare one packet to another.  
	int remote_missiontime;				// the remote timestamp that matches this packet.
	vec3d 	position;						// what it says on the tin
	vec3d 	velocity;						// what it says on the tin
	vec3d 	rotational_velocity;			// what it says on the tin
	vec3d 	desired_velocity;				// what it says on the tin
	vec3d 	desired_rotational_velocity;	// this one is only actually from the packet when we are dealing with a player ship.
	matrix	orientation;					// the orientation as transmitted by the other instance

	packet_info(int frame_in = 0, int time_in = 0, const vec3d* position_in = &vmd_zero_vector, const vec3d* velocity_in = &vmd_zero_vector, 
		const vec3d* rotational_velocity_in = &vmd_zero_vector, const vec3d* desired_velocity_in = &vmd_zero_vector, const vec3d* desired_rotational_velocity_in = &vmd_zero_vector,
		const angles* angles_in = &vmd_zero_angles) 
	{	
		frame = frame_in;
		remote_missiontime = time_in;
		position = *position_in;
		velocity = *velocity_in;
		rotational_velocity = *rotational_velocity_in;
		desired_velocity = *desired_velocity_in;
		desired_rotational_velocity = *desired_rotational_velocity_in;
		vm_angles_2_matrix(&orientation, angles_in);
	}

} packet_info;

// the real center of the new interpolation code.  When a packet is received, its frame is used as the key in the unordered_map _packets
// those keys are kept in _received_frames
class interpolation_manager {
private:
	int _upcoming_packet_index;			// what packet should we look at as the next packet.  -1 if we do not yet have enough packets.
	int _prev_packet_index;				// what was the last packet? This and Upcoming are both idices into the _packets vector. 
	int _local_skip_forward;			// TODO: Use this? Used on the server if a client is inexplicably ahead of us.
	bool _simulation_mode;				// are we currently simulating as if this were a normal ship?
	bool _packets_expended;				// have we used up all packets while interpolating?
	SCP_vector<packet_info> _packets;	// all the info from the position/orientation portion of packets that we care to keep
	int _source_player_index;

	void reassess_packet_index(vec3d* pos, matrix* ori, physics_info* pip);		// for finding which packets from within _packets we should use
	void replace_packet(int index, vec3d* pos, matrix* orient, physics_info* pip);	// a function that acts as a workaround, when coming out of simulation_mode

	// Frame numbers that helps us figure out if we should ignore new information coming from the server because
	// we already received a newer packet than this one for that type of info.
	int _hull_comparison_frame;						// what frame was the last hull information received?
	int _shields_comparison_frame;					// what frame was the last shield information received?
	SCP_vector<std::pair<int,int>> _subsystems_comparison_frame;	// what frame was the last subsystem information received? (for each subsystem) First is health, second is animation
	int _ai_comparison_frame;						// what frame was the last ai information received?

public:

	// adds a new packet, whilst also manually sorting the relevant entries
	void add_packet(int objnum, int frame, int time_delta, vec3d* position, vec3d* velocity, vec3d* rotational_velocity, vec3d* desired_velocity, vec3d* desired_rotational_velocity, angles* angles, int player_index);
	void interpolate_main(vec3d* pos, matrix* ori, physics_info* pip, vec3d* last_pos, matrix* last_orient, vec3d* gravity, bool player_ship);
	void reinterpolate_previous(TIMESTAMP stamp, int prev_packet_index, int next_packet_index,  vec3d* position, matrix* orientation, vec3d* velocity, vec3d* rotational_velocity);

	int get_hull_comparison_frame() { return _hull_comparison_frame; }
	int get_shields_comparison_frame() { return _shields_comparison_frame; }
	
	int get_subsystem_health_frame(int i) 
	{ 
		if (i < static_cast<int>(_subsystems_comparison_frame.size()) && i >= 0) {
			return _subsystems_comparison_frame[i].first; 
		} // if it somehow got passed nonsense, chances are what it is is trying to read is nonsense, and INT_MAX will keep it from doing anything crazy.
		else {
			return INT_MAX;
		}
	}

	int get_subsystem_animation_frame(int i) {
		if (i < static_cast<int>(_subsystems_comparison_frame.size()) && i >= 0) {
			return _subsystems_comparison_frame[i].second; 
		} // if it somehow got passed nonsense, chances are what it is is trying to read is nonsense, and INT_MAX will keep it from doing anything crazy.
		else {
			return INT_MAX;
		}
	}


	int get_ai_comparison_frame() { return _ai_comparison_frame; }

	void set_hull_comparison_frame(int frame) { _hull_comparison_frame = frame; }
	void set_shields_comparison_frame(int frame) { _shields_comparison_frame = frame; }

	void set_subsystem_health_frame(int i, int frame)
	{ 
		if (i < static_cast<int>(_subsystems_comparison_frame.size()) && i >= 0) {
			_subsystems_comparison_frame[i].first = frame;
		} 
	}

	void set_subsystem_animation_frame(int i, int frame)
	{
		if (i < static_cast<int>(_subsystems_comparison_frame.size()) && i >= 0) {
			_subsystems_comparison_frame[i].second = frame;
		} 
	}

	void set_ai_comparison_frame(int frame) { _ai_comparison_frame = frame; }

	void force_interpolation_mode() { _simulation_mode = true; }

	void reset(int subsystem_count) 
	{
		if (!(Game_mode & GM_MULTIPLAYER)){
			return;
		}

		_packets.clear();
		_packets.reserve(PACKET_INFO_LIMIT * 2);
		_upcoming_packet_index = -1;
		_prev_packet_index = -1;
		_simulation_mode = false;
		_packets_expended = false;
		_local_skip_forward = 0;
		_hull_comparison_frame = -1;
		_shields_comparison_frame = -1;
		_source_player_index = -1;


		_subsystems_comparison_frame.clear();

		for (int i = 0; i < subsystem_count; i++) {
			_subsystems_comparison_frame.emplace_back(-1, -1);
		}

		_ai_comparison_frame = -1;
	}

	void clean_up() 
	{
		_packets.clear();
		_packets.shrink_to_fit();
		_subsystems_comparison_frame.clear();
		_subsystems_comparison_frame.shrink_to_fit();
	}

	interpolation_manager()
	{
		_upcoming_packet_index = -1;
		_prev_packet_index = -1;
		_simulation_mode = false;
		_packets_expended = false;
		_local_skip_forward = 0;
		_hull_comparison_frame = -1;
		_shields_comparison_frame = -1;
		_ai_comparison_frame = -1;
		_source_player_index = -1;
	}
};
