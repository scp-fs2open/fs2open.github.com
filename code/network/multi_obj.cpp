/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include <algorithm>

#include "network/multi_obj.h"
#include "globalincs/globals.h"
#include "freespace.h"
#include "io/timer.h"
#include "io/key.h"
#include "globalincs/linklist.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multi_interpolate.h"
#include "network/multi_options.h"
#include "network/multi_rate.h"
#include "network/multi.h"
#include "object/object.h"
#include "object/objcollide.h"		// for multi rollback collisions
#include "object/objectshield.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "math/spline.h"
#include "physics/physics.h"
#include "ship/afterburner.h"
#include "cfile/cfile.h"
#include "debugconsole/console.h"
#include "object/waypoint.h"
#include "weapon/weapon.h"

// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE STRUCTS
// 

extern const std::uint32_t MAX_TIME;
constexpr int OO_MAIN_HEADER_SIZE = 9;  // two ints and a ubyte (recall! fix is basically an int)


// One frame record per ship with each contained array holding one element for each frame.
struct rollback_ship_position_records {
	vec3d positions[MAX_FRAMES_RECORDED];							// The recorded ship positions, cur_frame_index is the index.
	matrix orientations[MAX_FRAMES_RECORDED];						// The recorded ship orientations, cur_frame_index is the index. 
	vec3d velocities[MAX_FRAMES_RECORDED];							// The recorded ship velocities (required for additive velocity shots and auto aim), cur_frame_index is the index.
	vec3d rotational_velocities[MAX_FRAMES_RECORDED];				// The recorded ship rotational velocities (required for auto aim if certain ), cur_frame_index is the index.

	vec3d first_pos; 												// The "last_pos" of the oldest recorded frame.
};

// keeps track of what has been sent to each player, helps cut down on bandwidth, allowing only new information to be sent instead of old.
struct oo_info_sent_to_players {	
	TIMESTAMP timestamp;					// The overall timestamp which is used to decide if a new packet should be sent to this player for this ship.

	vec3d position;					// If they are stationary, there's no need to update their position.
	float hull;						// no need to send hull if hull hasn't changed.

	// shields are a little special because they are constantly regenerating, so if they are *not* at full strength we need to send them.
	bool perfect_shields_sent;		

	int ai_mode;					// what ai mode was last sent.
	int ai_submode;					// what ai submode was last sent.
	int target_signature;			// what target_signature was last sent (used for AI portion of OO packet)

	SCP_vector<float> subsystem_health;	// We need vectors to keep track of all subsystem health and subsystem angles.
	SCP_vector<float> subsystem_1b;
	SCP_vector<float> subsystem_1h;
	SCP_vector<float> subsystem_1p;
	SCP_vector<float> subsystem_2b;
	SCP_vector<float> subsystem_2h;
	SCP_vector<float> subsystem_2p;
};

struct oo_netplayer_records{
	SCP_vector<oo_info_sent_to_players> last_sent;			// Subcategory of which player did I send this info to?  Corresponds to net_player index.
	// This is not yet implemented, but may be necessary for autoaim to work in more busy scenes.  Basically, if you're switching targets,
	// autoaim may succeed on the client but head to the wrong target on the server.
//	int player_target_record[MAX_FRAMES_RECORDED];			// For rollback, we need to keep track of the player's targets. Uses frame as its index.
};

// Keep track of the ships we'll need to restore later after rollback is done.
struct rollback_restore_record {
	int roll_objnum;		// object pointer to the ship we need to restore. 
	vec3d position;			// position to restore to
	vec3d old_position;		// old_pos to restore to
	matrix orientation;		// orientation to restore to
	vec3d velocity;			// velocity to restore to
	vec3d rotational_velocity; // rotational velocity to restore to
};

// Keeps track of how many shots that need to be fired in rollback mode.
struct rollback_unsimulated_shots {
	object* shooterp;		// pointer to the shooting object (ship)
	vec3d pos;				// the relative position calculated from the non-homing packet.
	matrix orient;			// the relative orientation from the non-homing packet.
	bool secondary_shot;	// is this a dumbfire missile shot?
};

// our main struct for keeping track of all interpolation and oo packet info.
struct oo_general_info {
	// info that helps us figure out what is the best reference object available when sending a rollback shot.
	// We go by what is the most recent object update packet received, and then by distance.
	int rollback_reference_timestamp;							// what time did we receive the reference object
	ushort most_recent_updated_net_signature;	// what is the net signature of the reference object.
	int most_recent_frame;						// what is the frame from the update of most recently updated object.

	// Frame tracking info, we can have up to INT_MAX frames, but to save on bandwidth and memory we have to "wrap"
	// the index.  Cur_frame_index goes up to MAX_FRAMES_RECORDED and makes info easy to access, wrap_count counts 
	// how many times that is reset, up to MAX_SERVER_TRACKER_SMALL_WRAPS, we multiply these two to get the seq_num
	// sent to oo_packet recipients. larger_wrap_count allows us to figure out if we are in an "odd" larger wrap
	int number_of_frames;									// how many frames have we gone through, total.
	ubyte cur_frame_index;									// the current frame index (to access the recorded info)

	TIMESTAMP timestamps[MAX_FRAMES_RECORDED];					// The timestamp for the recorded frame
	SCP_vector<rollback_ship_position_records> frame_info;		// Actually keeps track of ship physics info.  Uses net_signature as its index.
	SCP_vector<oo_netplayer_records> player_frame_info;		// keeps track of player targets and what has been sent to each player. Uses player as the index

	// rollback info
	bool rollback_mode;										// are we currently creating and moving weapons from the client primary fire packets
	SCP_vector<int> rollback_weapon_numbers_created_this_frame;	// the weapons created this rollback frame.
	SCP_vector<int> rollback_weapon_object_number;						// a list of the weapons that were created, so that we can roll them into the current simulation

	SCP_vector<int> rollback_ships;						// a list of ships that take part in roll back, no quick index, must be iterated through.
	SCP_vector<rollback_restore_record> restore_points;	// where to move ships back to when done with rollback. no quick index, must be iterated through.
	SCP_vector<rollback_unsimulated_shots> 
		rollback_shots_to_be_fired[MAX_FRAMES_RECORDED];				// the shots we will need to fire and simulate during rollback, organized into the frames they will be fired
	SCP_vector<int>rollback_collide_list;					// the list of ships and weapons that we need to pass to collision detection during rollback.
														
	SCP_vector<const ship_registry_entry*> rotation_list;	// subsystem rotation
};

oo_general_info Oo_info;

// flags
bool Afterburn_hack = false;			// HACK!!!

// returns the last frame's index.
int multi_find_prev_frame_idx();

// quickly lookup how much time has passed since the given frame.
int multi_ship_record_get_time_elapsed(int original_frame, int new_frame);

// fire the rollback weapons that are in the rollback struct
void multi_oo_fire_rollback_shots(int frame_idx);

// moves all rollbacked ships back to the original frame
void multi_oo_restore_frame(int frame_idx);

// pushes the rollback weapons forward for a single rollback frame.
bool multi_oo_simulate_rollback_shots(int frame_idx);

// restores ships to the positions they were in bedfore rollback.
void multi_record_restore_positions();

// See if a newly arrived packet is a good new option as a reference object
void multi_ship_record_rank_seq_num(object* objp, int seq_num);

// recalculate how much time is between position packets
float multi_oo_calc_pos_time_difference(int player_id, int net_sig_idx);


// tolerance for bashing position
#define OO_POS_UPDATE_TOLERANCE	150.0f

// new improved - more compacted info type
#define OO_POS_AND_ORIENT_NEW		(1<<0)		// To update position and orientation. Because getting accurate velocity requires orientation, and accurate orienation requires velocity
#define OO_FULL_PHYSICS				(1<<1)		// Since AI don't use all phys_info values, we need a flag to confirm when all have been transmitted.
#define OO_HULL_NEW					(1<<2)		// To Update Hull
#define OO_SHIELDS_NEW				(1<<3)		// To Update Shields.
#define OO_SUBSYSTEMS_NEW			(1<<4)		// Send Subsystem Info
#define OO_ANIMATION				(1<<5)		// Subsystem Animation info
#define OO_AI_NEW					(1<<6)		// Send updated AI Info
#define OO_AFTERBURNER_NEW			(1<<7)		// Flag for Afterburner hack
#define OO_PRIMARY_BANK				(1<<8)		// if this is set, fighter has selected bank one
#define OO_PRIMARY_LINKED			(1<<9)		// if this is set, banks are linked
#define OO_TRIGGER_DOWN				(1<<10)		// if this is set, trigger is DOWN
#define OO_SUPPORT_SHIP				(1<<11)		// Send extra info for the support ship.

#define OO_SBUSYS_ROTATION_CUTOFF	0.1f		// if the squared difference between the old and new angles is less than this, don't send.

#define OO_VIEW_CONE_DOT			(0.1f)
#define OO_VIEW_DIFF_TOL			(0.15f)			// if the dotproducts differ this far between frames, he's coming into view

// no timestamp should ever have sat for longer than this. 
#define OO_MAX_TIMESTAMP			2500

// distance class
#define OO_NEAR						0
#define OO_NEAR_DIST					(200.0f)
#define OO_MIDRANGE					1
#define OO_MIDRANGE_DIST			(600.0f)
#define OO_FAR							2
#define OO_FAR_DIST					(1400.0f)

// how often we should send full hull/shield updates
#define OO_HULL_SHIELD_TIME		600
#define OO_SUBSYS_TIME				1000

// timestamp values for object update times based on client's update level.
// Cyborg17 - This is the one update number I have adjusted, because it's the player's target.
int Multi_oo_target_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	50, 				// 20x a second 
	50, 				// 20x a second
	20,				// 50x a second
	20,				// 50x a second
};

// for near ships
int Multi_oo_front_near_update_times[MAX_OBJ_UPDATE_LEVELS] =
{
	150,				// low update
	100,				// medium update
	66,				// high update
	66,				// LAN update
};

// for medium ships
int Multi_oo_front_medium_update_times[MAX_OBJ_UPDATE_LEVELS] =
{
	250,				// low update
	180, 				// medium update
	120,				// high update
	66,					// LAN update
};

// for far ships
int Multi_oo_front_far_update_times[MAX_OBJ_UPDATE_LEVELS] =
{
	750,				// low update
	350, 				// medium update
	150, 				// high update
	66,					// LAN update
};

// for near ships
int Multi_oo_rear_near_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	300,				// low update
	200,				// medium update
	100,				// high update
	66,					// LAN update
};

// for medium ships
int Multi_oo_rear_medium_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	800,				// low update
	600,				// medium update
	300,				// high update
	66,					// LAN update
};

// for far ships
int Multi_oo_rear_far_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	2500, 				// low update
	1500,				// medium update
	400,				// high update
	66,					// LAN update
};

// ship index list for possibly sorting ships based upon distance, etc
short OO_ship_index[MAX_SHIPS];

// Cyborg17 - I'm leaving this system in place, just in case, although I never used it. 
// It needs cleanup in keycontrol.cpp before it can be used.
int OO_update_index = -1;							// The player index that allows us to look up multi rate through the debug
													//  console and display it on the hud.


// ---------------------------------------------------------------------------------------------------
// POSITION AND ORIENTATION RECORDING
// if it breaks, find Cyborg17

// We record positions and orientations in Oo_info.frame_info so that we can create a weapon in the same relative 
// circumstances as on the client.  I was directly in front, 600 meters away when I fired?  Well, now the client 
// will tell the server that and the server will rewind part of its simulation to recreate that shot and then 
// redo its simulation. There will still be some very small differences between server and client caused by the 
// fact that the flFrametimes will be different, but changing that would be impossible.
// ---------------------------------------------------------------------------------------------------

// Add a new ship to the tracking struct
void multi_rollback_ship_record_add_ship(int obj_num)
{
	object* objp = &Objects[obj_num];
	int net_sig_idx = objp->net_signature;

	// check for a ship that will enter the mission later on and has not yet had its net_signature set.
	// These ships will be added later when they are actually in the mission
	if (net_sig_idx == 0) {
		return;
	}

	// our target size is the number of ships in the vector plus one because net_signatures start at 1 and size gives the number of elements, and this should be a new element.
	int current_size = (int)Oo_info.frame_info.size();
	
	objp->interp_info.reset();

	// if we're right where we should be.
	if (net_sig_idx == current_size) {
		Oo_info.frame_info.push_back(Oo_info.frame_info[0]);

		for (int i = 0; i < MAX_PLAYERS; i++) {
			Oo_info.player_frame_info[i].last_sent.push_back( Oo_info.player_frame_info[i].last_sent[0] );
		}

	} // if not, create the storage for it.
	else if (net_sig_idx > current_size) {
		while (net_sig_idx >= current_size) {
			Oo_info.frame_info.push_back(Oo_info.frame_info[0]);

			for (int i = 0; i < MAX_PLAYERS; i++) {
				Oo_info.player_frame_info[i].last_sent.push_back( Oo_info.player_frame_info[i].last_sent[0] );
			}

			current_size++;
		}
	} 

	Assertion(net_sig_idx <= (current_size + 1), "New entry into the multi ship traker struct does not equal the index that should belong to it.\nNet_signature: %d and current_size %d\n", net_sig_idx, current_size);

	ship_info* sip = &Ship_info[Ships[objp->instance].ship_info_index];

	// To use vectors for the subsystems, we have to init the subsystem them here.
	uint subsystem_count = (uint)sip->n_subsystems;

	SCP_vector<int> comparison_frames_out;

	comparison_frames_out.reserve(subsystem_count);

	while (comparison_frames_out.size() < subsystem_count) {
		comparison_frames_out.push_back(-1);

		for (int i = 0; i < MAX_PLAYERS; i++) {
			Oo_info.player_frame_info[i].last_sent[net_sig_idx].subsystem_health.push_back(-1.0f);
			Oo_info.player_frame_info[i].last_sent[net_sig_idx].subsystem_1b.push_back(-1.0f);
			Oo_info.player_frame_info[i].last_sent[net_sig_idx].subsystem_1h.push_back(-1.0f);
			Oo_info.player_frame_info[i].last_sent[net_sig_idx].subsystem_1p.push_back(-1.0f);
			Oo_info.player_frame_info[i].last_sent[net_sig_idx].subsystem_2b.push_back(-1.0f);
			Oo_info.player_frame_info[i].last_sent[net_sig_idx].subsystem_2h.push_back(-1.0f);
			Oo_info.player_frame_info[i].last_sent[net_sig_idx].subsystem_2p.push_back(-1.0f);
		}
	}

	// store info from the obj struct if it's already in the mission, otherwise, let the physics update call take care of it when the first frame starts.
	if (Game_mode & GM_IN_MISSION) {

		// only add positional info if they are in the mission.
		Oo_info.frame_info[net_sig_idx].positions[Oo_info.cur_frame_index] = objp->pos;
		Oo_info.frame_info[net_sig_idx].first_pos = objp->last_pos;
		Oo_info.frame_info[net_sig_idx].orientations[Oo_info.cur_frame_index] = objp->orient;
		Oo_info.frame_info[net_sig_idx].velocities[Oo_info.cur_frame_index] = objp->phys_info.vel;
		Oo_info.frame_info[net_sig_idx].rotational_velocities[Oo_info.cur_frame_index] = objp->phys_info.rotvel;
	}
}

// Update the tracking struct whenever the object is updated in-game
void multi_ship_record_update_all() 
{
	Assertion(MULTIPLAYER_MASTER, "Non-server accessed a server only function multi_ship_record_update_all(). Please report!!");

	if (!MULTIPLAYER_MASTER) {
		return;
	}

	int net_sig_idx;
	object* objp;

	for (ship & cur_ship : Ships) {
		// apparently this occasionally happens.
		if (cur_ship.objnum == -1) {
			continue;
		}		
		
		objp = &Objects[cur_ship.objnum];

		if (objp == nullptr || objp->type != OBJ_SHIP) {
			continue;
		}
		 
		net_sig_idx = objp->net_signature;

		Assertion(net_sig_idx <= STANDALONE_SHIP_SIG, "Multi tracker got an invalid index of %d while updating it records. This is likely a coder error, please report!", net_sig_idx);
		// make sure it's a valid index.
		if (net_sig_idx < SHIP_SIG_MIN || net_sig_idx == STANDALONE_SHIP_SIG || net_sig_idx > SHIP_SIG_MAX) {
			 continue;
		}

		Oo_info.frame_info[net_sig_idx].positions[Oo_info.cur_frame_index] = objp->pos;
		Oo_info.frame_info[net_sig_idx].orientations[Oo_info.cur_frame_index] = objp->orient;
		Oo_info.frame_info[net_sig_idx].velocities[Oo_info.cur_frame_index] = objp->phys_info.vel;
		Oo_info.frame_info[net_sig_idx].rotational_velocities[Oo_info.cur_frame_index] = objp->phys_info.rotvel;

		// if we have enough frames, set last position to the prev frame
		if (Oo_info.number_of_frames > MAX_FRAMES_RECORDED){
			Oo_info.frame_info[net_sig_idx].first_pos = Oo_info.frame_info[net_sig_idx].positions[(Oo_info.cur_frame_index == MAX_FRAMES_RECORDED - 1) ? 0 : Oo_info.cur_frame_index + 1];
		} else {
			Oo_info.frame_info[net_sig_idx].first_pos = Oo_info.frame_info[net_sig_idx].positions[0];
		}

	}
}

// Increment the tracker per frame, before packets are processed
void multi_ship_record_increment_frame() 
{
	++Oo_info.number_of_frames;
	++Oo_info.cur_frame_index;

	// Because we are only tracking 30 frames (up to a quarter second on a 120 frame machine), we will have to wrap the index often
	if (Oo_info.cur_frame_index == MAX_FRAMES_RECORDED) {
		Oo_info.cur_frame_index = 0;
	}

	// update the rollback record for timestamps
	Oo_info.timestamps[Oo_info.cur_frame_index] = _timestamp();

	// Let's also now update the other timing system from multi_interpolate
	Multi_Timing_Info.update_current_time();
}

// returns the last frame's index.
int multi_find_prev_frame_idx() 
{
	if (Oo_info.cur_frame_index == 0) {
		return MAX_FRAMES_RECORDED - 1;
	} else {
		return Oo_info.cur_frame_index - 1;
	}
}

// Finds the first frame that is before the incoming timestamp.
int multi_ship_record_find_frame(int client_frame, int time_elapsed)
{	
	// frame coming in from the client is too old to be used. (The -1 prevents an edge case bug at very high pings)
	if (Oo_info.number_of_frames - client_frame >= MAX_FRAMES_RECORDED - 1) {
		return -1;
	}

	// figure out the frame index (the frame index wraps every MAX_FRAMES_RECORDED frames)
	int frame = client_frame % MAX_FRAMES_RECORDED;

	// Now that the wrap has been verified, if time_elapsed is zero return the frame it gave us.
	if(time_elapsed == 0){
		return frame;
	}

	// Now we look for the frame that the client is saying to look for. 
	// get the timestamp we are looking for.
	TIMESTAMP target_timestamp = timestamp_delta(Oo_info.timestamps[frame], time_elapsed);

	// need to try to make rollback shot make some kind of sense if we have invalid timestamps,
	// and print to debugif it is.
	if (!target_timestamp.isValid() || target_timestamp.isNever()) {
		mprintf(("Nonsense timestamp in multi_ship_record_find_frame of %s. Get ~~Allender~~ Cyborg!\n", (target_timestamp.isValid()) ? "isNever" : "NOT isValid"));
		return frame;
	};

	for (int i = Oo_info.cur_frame_index - 2; i > -1; i--) {

		// need to try to make rollback shot make some kind of sense if we have invalid timestamps,
		// and print to debug if it is.	  No need to trigger the Assert in timestamp_in_between, as it is minor here. (i + 1 should be check on previous iteration, most of the time)
		if (!Oo_info.timestamps[i].isValid() || Oo_info.timestamps[i].isNever()) {
			mprintf(("timestamps[i] is %s, get ~~Allender~~ Cyborg!\n", (Oo_info.timestamps[i].isValid()) ? "isNever" : "invalid"));
			return frame;
		}

		// Check to see if the client's timestamp matches the recorded frames.
		if (timestamp_in_between(target_timestamp, Oo_info.timestamps[i], Oo_info.timestamps[i + 1])) {		
			return i;
		}

		// there is no need to check for an invalid frame here because, logically, the search cannot reach frame until the next section
	}


	// need to try to make rollback shot make some kind of sense if we have invalid timestamps,
	// and print to debug if it is. No need to trigger the Assert in timestamp_in_between, as it is minor here.
	if (!Oo_info.timestamps[MAX_FRAMES_RECORDED - 1].isValid() || Oo_info.timestamps[MAX_FRAMES_RECORDED - 1].isNever()) {
			mprintf(("timestamps[MAX_FRAMES_FRAMES_RECORDED - 1] is %s, get ~~Allender~~ Cyborg!\n", (Oo_info.timestamps[MAX_FRAMES_RECORDED - 1].isValid()) ? "isNever" : "invalid"));
			return frame;
	}


	// Check for an end of the wrap condition.
	if (timestamp_in_between(target_timestamp, Oo_info.timestamps[MAX_FRAMES_RECORDED - 1], Oo_info.timestamps[0])) {
		return MAX_FRAMES_RECORDED - 1;
	}

	// Check for the received frame being passed
	if (frame == 0){
		return -1;
	}


	// Check the oldest frames.
	for (int i = MAX_FRAMES_RECORDED - 2; i > Oo_info.cur_frame_index; i--) {
		// need to try to make rollback shot make some kind of sense if we have invalid timestamps,
		// and print to debug if it is. No need to trigger the Assert in timestamp_in_between, as it is minor here.
		if (!Oo_info.timestamps[i].isValid() || Oo_info.timestamps[i].isNever()) {
			mprintf(("timestamps[i] is %s, get ~~Allender~~ Cyborg!\n", (Oo_info.timestamps[i].isValid()) ? "isNever" : "invalid"));
			return frame;
		}

		if (timestamp_in_between(target_timestamp, Oo_info.timestamps[i], Oo_info.timestamps[i + 1])) {
			return i;
		} else if (i < frame) {
			return -1;
		}
	}

	// shouldn't be reachable... somehow wasn't caught earlier.  Just let the old system handle this one.
	return -1;
}

// Quick lookup for the record of position.
vec3d multi_ship_record_lookup_position(object* objp, int frame) 
{
	Assertion(objp != nullptr, "nullptr given to multi_ship_record_lookup_position. \nThis should be handled earlier in the code, please report!");
	return Oo_info.frame_info[objp->net_signature].positions[frame];
}

// Quick lookup for the record of orientation.
matrix multi_ship_record_lookup_orientation(object* objp, int frame) 
{
	Assertion(objp != nullptr, "nullptr given to multi_ship_record_lookup_position. \nThis should be handled earlier in the code, please report!");
	if (objp == nullptr) {
		return vmd_identity_matrix;
	}
	return Oo_info.frame_info[objp->net_signature].orientations[frame];
}

// quickly lookup how much time has passed between two frames.
int multi_ship_record_get_time_elapsed(int original_frame, int new_frame) 
{
	// Bogus values
	Assertion(original_frame <= MAX_FRAMES_RECORDED, "Function multi_ship_record_get_time_elapsed() got passed an invalid original frame, this is a code error, please report. ");
	Assertion(new_frame <= MAX_FRAMES_RECORDED, "Function multi_ship_record_get_time_elapsed() got passed an invalid new frame, this is a code error, please report. ");
	if (original_frame >= MAX_FRAMES_RECORDED || new_frame >= MAX_FRAMES_RECORDED) {
		return 0;
	}

	return timestamp_get_delta(Oo_info.timestamps[original_frame], Oo_info.timestamps[new_frame]);
}

// figures out how much time has passed bwetween the two frames.
int multi_ship_record_find_time_after_frame(int starting_frame, int ending_frame, int time_elapsed) 
{
	starting_frame = starting_frame % MAX_FRAMES_RECORDED;

	int return_value = time_elapsed - (timestamp_get_delta(Oo_info.timestamps[ending_frame], Oo_info.timestamps[starting_frame]));
	return return_value;
}

// Returns whether weapons currently being created should be part of the rollback simulation.
bool multi_ship_record_get_rollback_wep_mode() 
{
	return Oo_info.rollback_mode;
}

// Adds an object pointer to the list of weapons that needs to be simulated as part of rollback.
void multi_ship_record_add_rollback_wep(int wep_objnum) 
{
	// check for valid weapon
	if (wep_objnum < 0 || wep_objnum >= MAX_OBJECTS){
		mprintf(("Invalid object number passed when trying to add weapons to the weapon rollback tracker.\n"));
		return;
	}
	
	// add it to the list of weapons we'll need to add to the simulation.
	Oo_info.rollback_weapon_numbers_created_this_frame.push_back(wep_objnum);
}

// simply remove the ability to collide for weapons that have already collided.
void multi_oo_remove_colliders()
{
	for (auto& weap_objnum : Oo_info.rollback_weapon_object_number) {
		if (Objects[weap_objnum].flags[Object::Object_Flags::Should_be_dead]) {
			Objects[weap_objnum].flags.remove(Object::Object_Flags::Collides);
		}
	}
}

// This stores the information we got from the client to create later.
void multi_ship_record_add_rollback_shot(object* pobjp, vec3d* pos, matrix* orient, int frame, bool secondary) 
{
	Oo_info.rollback_mode = true;

	rollback_unsimulated_shots new_shot;
	new_shot.shooterp = pobjp;
	new_shot.pos = *pos;
	new_shot.orient = *orient;
	new_shot.secondary_shot = secondary;

	Oo_info.rollback_shots_to_be_fired[frame].push_back(new_shot);	
}

// Manage rollback for a frame
void multi_ship_record_do_rollback() 
{	
	// only rollback if there are shots to simulate.
	if (!Oo_info.rollback_mode) {
		return;
	}

	int net_sig_idx;
	object* objp;

	// set up all restore points and ship portion of the collision list
	for (ship& cur_ship : Ships) {

		// once this happens, we've run out of ships.
		if (cur_ship.objnum < 0) {
			break;
		}

		objp = &Objects[cur_ship.objnum];

		if (objp == nullptr || objp->type != OBJ_SHIP) {
			continue;
		}

		net_sig_idx = objp->net_signature;

		// this should not happen, but it would not access correct info. 
		//It only means a less accurate simulation (and a mystery), not a crash. So, for now, write to the log. 
		if (net_sig_idx < 1) {
			mprintf(("Rollback ship does not have a net signature.  Someone should probably investigate this at some point.\n"));
			continue;
		}

		// also, we must *not* attempt to rollback the standalone ship 
		if (net_sig_idx == STANDALONE_SHIP_SIG) {
			continue;
		}

		Oo_info.rollback_ships.push_back(cur_ship.objnum);

		rollback_restore_record restore_point;

		restore_point.roll_objnum = cur_ship.objnum;
		restore_point.position = objp->pos;
		restore_point.old_position = objp->last_pos;
		restore_point.orientation = objp->orient;
		restore_point.velocity = objp->phys_info.vel;
		restore_point.rotational_velocity = objp->phys_info.rotvel;

		Oo_info.restore_points.push_back(restore_point);
		// Also take this opportunity to set up their collision 
		Oo_info.rollback_collide_list.push_back(cur_ship.objnum);
	}

	// now we need to figure out which frame will start the rollback simulation
	int frame_idx = Oo_info.cur_frame_index + 1;

	if (frame_idx >= MAX_FRAMES_RECORDED) {
		frame_idx = 0;
	}

	// loop through them
	while (frame_idx != Oo_info.cur_frame_index) {

		if (!Oo_info.rollback_shots_to_be_fired[frame_idx].empty()) {
			break;
		}

		frame_idx++;

		if (frame_idx >= MAX_FRAMES_RECORDED) {
			frame_idx = 0;
		}
	}

	// make sure we found one.
	Assertion(frame_idx != Oo_info.cur_frame_index, "Rollback was called without there being a rollback shot to simulate. This is a coder error. Please report!");

	if (frame_idx == Oo_info.cur_frame_index) {
		return;
	}

	nprintf(("Network","At least one multiplayer rollback shot is being simulated this frame.\n"));

	bool continue_rollback = true;

	do {
		// move all ships to their recorded positions
		multi_oo_restore_frame(frame_idx);

		// push weapons forward for the frame (weapons do not get pushed forward for the first frame of their existence)
		continue_rollback = multi_oo_simulate_rollback_shots(frame_idx);

		// then fire all shots for the frame, primary and secondary, if there are any
		multi_oo_fire_rollback_shots(frame_idx);

		// perform collision detection for that frame.
		obj_sort_and_collide(&Oo_info.rollback_collide_list);

		multi_oo_remove_colliders();

		//
		if (!continue_rollback){
			break;
		}

		//increment the frame
		frame_idx++;
		if (frame_idx >= MAX_FRAMES_RECORDED) {
			frame_idx = 0;
		}

	} while (frame_idx != Oo_info.cur_frame_index);

	// restore the old frame
	multi_record_restore_positions();

	// clean up the rollback info that has been taken care of.
	Oo_info.rollback_collide_list.clear();
	Oo_info.rollback_mode = false;
	Oo_info.rollback_ships.clear();
	for (auto & shots_to_be_fired : Oo_info.rollback_shots_to_be_fired) {
		shots_to_be_fired.clear();
	}
	Oo_info.rollback_weapon_object_number.clear();
}

// fires the rollback weapons that are in the rollback struct
void multi_oo_fire_rollback_shots(int frame_idx)
{
	for (auto & rollback_shot : Oo_info.rollback_shots_to_be_fired[frame_idx]) {
		rollback_shot.shooterp->pos = rollback_shot.pos;
		rollback_shot.shooterp->orient = rollback_shot.orient;

		if (rollback_shot.secondary_shot) {
			ship_fire_secondary(rollback_shot.shooterp, 1, true);
		}
		else {
			ship_fire_primary(rollback_shot.shooterp, 1, true);
		}
	}

	// add the newly created shots to the collision list.
	for (auto& wep_obj_number : Oo_info.rollback_weapon_numbers_created_this_frame) {
		Oo_info.rollback_weapon_object_number.push_back(wep_obj_number);
		Oo_info.rollback_collide_list.push_back(wep_obj_number);
	}
	Oo_info.rollback_weapon_numbers_created_this_frame.clear();
}

// moves all rollbacked ships back to the original frame
void multi_oo_restore_frame(int frame_idx)
{
	// set the position, orientation, and velocity for each object
	for (auto& objnum : Oo_info.rollback_ships) {
		object* objp = &Objects[objnum];
		Assertion(objp != nullptr, "Nullptr somehow got into the rollback ship vector, please report!");
		objp->pos = Oo_info.frame_info[objp->net_signature].positions[frame_idx];
		objp->orient = Oo_info.frame_info[objp->net_signature].orientations[frame_idx];
		objp->phys_info.vel = Oo_info.frame_info[objp->net_signature].velocities[frame_idx];
		objp->phys_info.rotvel = Oo_info.frame_info[objp->net_signature].rotational_velocities[frame_idx];

		// get the prev frame
		int temp_prev_frame = (frame_idx == 0) ? MAX_FRAMES_RECORDED - 1 : frame_idx - 1;

		// because of the way collision code, we must set last_pos as well. But it gets complicated.
		// first figure out if the current frame is actually the oldest frame on record
		if (Oo_info.cur_frame_index == temp_prev_frame) {
			// if so, use the special first_pos value
			objp->last_pos = Oo_info.frame_info[objp->net_signature].first_pos;
		// if not (most likely), use the previous frame's position.
		} else {
			objp->last_pos = Oo_info.frame_info[objp->net_signature].positions[temp_prev_frame];
		}
	}
}

// pushes the rollback weapons forward for a single rollback frame.
bool multi_oo_simulate_rollback_shots(int frame_idx) 
{
	int prev_frame = frame_idx - 1;
	if (prev_frame < 0) {
		prev_frame = MAX_FRAMES_RECORDED - 1;
	}

	// calculate the float version of the frametime.
	float frametime = static_cast<float>(multi_ship_record_get_time_elapsed(prev_frame, frame_idx)) / static_cast<float>(TIMESTAMP_FREQUENCY);

	if (Oo_info.rollback_weapon_object_number.empty()) {
		return true;
	}

	bool result = false;

	// push the weapons forward.
	for (auto& weap_objnum : Oo_info.rollback_weapon_object_number) {
		object* objp = &Objects[weap_objnum];

		// if this object is "dead", then that means it has collided with something.
		// so remove its ability to collide.  This is the easiest way to deal with it.
		if (!objp->flags[Object::Object_Flags::Should_be_dead]) {
			// this means at least one weapon is still waiting to collide.
			result = true;
			vm_vec_scale_add2(&objp->pos, &objp->phys_info.vel, frametime);
			Weapons[objp->instance].lifeleft -= frametime;
		}
	}

	return result;
}

// restores ships to the positions they were in bedfore rollback.
void multi_record_restore_positions() 
{
	for (auto restore_point : Oo_info.restore_points) {

		object* objp = &Objects[restore_point.roll_objnum];
		// reset the position, orientation, and velocity for each object
		objp->pos = restore_point.position;
		objp->last_pos = restore_point.old_position;
		objp->orient = restore_point.orientation;
		objp->phys_info.vel = restore_point.velocity;
		objp->phys_info.rotvel = restore_point.rotational_velocity;
	}

	Oo_info.restore_points.clear();
}

// ---------------------------------------------------------------------------------------------------
// Client side frame tracking, for now used only for referenced in fire packets to improve client accuracy.
// 

// See if a newly arrived object update packet should be the new reference for the improved primary fire packet 
void multi_ship_record_rank_seq_num(object* objp, int seq_num) 
{
	// see if it's more recent.  Most recent is best.
	if (seq_num > Oo_info.most_recent_frame || Oo_info.most_recent_updated_net_signature == 0) {
		Oo_info.most_recent_updated_net_signature = objp->net_signature;
		Oo_info.most_recent_frame = seq_num;
		Oo_info.rollback_reference_timestamp = Multi_Timing_Info.get_current_time();

	} // if this packet is from the same frame, the closer ship makes for a slightly more accurate reference point
	else if (seq_num == Oo_info.most_recent_frame) {
		object* temp_reference_object = multi_get_network_object(Oo_info.most_recent_updated_net_signature);
		// check the distance
		if ( (temp_reference_object == nullptr) || vm_vec_dist_squared(&temp_reference_object->pos, &Objects[Player->objnum].pos) > vm_vec_dist_squared(&objp->pos, &Objects[Player->objnum].pos) ) {
			Oo_info.most_recent_updated_net_signature = objp->net_signature;
			Oo_info.most_recent_frame = seq_num;
			Oo_info.rollback_reference_timestamp = Multi_Timing_Info.get_current_time();
		}
	}
}

// Quick lookup for the most recently received ship
ushort multi_client_lookup_ref_obj_net_sig()
{	
	return Oo_info.most_recent_updated_net_signature;
}

// Quick lookup for the most recently received frame
int multi_client_lookup_frame_idx()
{
	return Oo_info.most_recent_frame;
}

// Quick lookup for the most recently received timestamp.
int multi_client_lookup_frame_timestamp()
{
	return Oo_info.rollback_reference_timestamp;
}

// Resets what info we have sent and interpolation info for a respawn
// To be safe, I believe that most info should be reset.
void multi_oo_respawn_reset_info(object* objp) 
{
	Assertion(objp != nullptr, "multi_oo_respawn_reset_info got passed a null pointer object. This is a coder error, please report.");
	Assertion(objp->net_signature > 0, "multi_oo_respawn_reset_info got passed an invalid net_signature. This is a coder error, please report.");
	if (objp == nullptr || objp->net_signature == 0) {
		return;
	}

	// When a player respawns, they keep their net signature, so clean up all the info that could mess things up in the future.
	for (auto & player_record : Oo_info.player_frame_info) {
		player_record.last_sent[objp->net_signature].timestamp = TIMESTAMP::immediate();
		player_record.last_sent[objp->net_signature].position = vmd_zero_vector;
		player_record.last_sent[objp->net_signature].hull = -1.0f;
		player_record.last_sent[objp->net_signature].ai_mode = -1;
		player_record.last_sent[objp->net_signature].ai_submode = -1;
		player_record.last_sent[objp->net_signature].target_signature = -1;
		player_record.last_sent[objp->net_signature].perfect_shields_sent = false;
		for (int i = 0; i < (int)player_record.last_sent[objp->net_signature].subsystem_health.size(); i++) {
			player_record.last_sent[objp->net_signature].subsystem_health[i] = -1.0f;
			player_record.last_sent[objp->net_signature].subsystem_1b[i] = -1.0f;
			player_record.last_sent[objp->net_signature].subsystem_1h[i] = -1.0f;
			player_record.last_sent[objp->net_signature].subsystem_1p[i] = -1.0f;
			player_record.last_sent[objp->net_signature].subsystem_2b[i] = -1.0f;
			player_record.last_sent[objp->net_signature].subsystem_2h[i] = -1.0f;
			player_record.last_sent[objp->net_signature].subsystem_2p[i] = -1.0f;

		}
	}

	// To ensure clean interpolation, we should probably just reset everything.
	objp->interp_info.reset();
}

// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE FUNCTIONS
//

object *OO_player_obj;
int OO_sort = 1;

bool multi_oo_sort_func(const short &index1, const short &index2)
{
	object *obj1, *obj2;
	float dist1, dist2;
	float dot1, dot2;
	vec3d v1, v2;

	// if the indices are bogus, or the objnums are bogus, return ">"
	if((index1 < 0) || (index2 < 0) || (Ships[index1].objnum < 0) || (Ships[index2].objnum < 0)){
		return false;
	}

	// get the 2 objects
	obj1 = &Objects[Ships[index1].objnum];
	obj2 = &Objects[Ships[index2].objnum];

	// get the distance and dot product to the player obj for both
	vm_vec_sub(&v1, &OO_player_obj->pos, &obj1->pos);
	dist1 = vm_vec_normalize_safe(&v1);
	vm_vec_sub(&v2, &OO_player_obj->pos, &obj2->pos);
	dist2 = vm_vec_normalize_safe(&v2);
	dot1 = vm_vec_dot(&OO_player_obj->orient.vec.fvec, &v1);
	dot2 = vm_vec_dot(&OO_player_obj->orient.vec.fvec, &v2);

	// objects in front take precedence
	if((dot1 < 0.0f) && (dot2 >= 0.0f)){
		return false;
	} else if((dot2 < 0.0f) && (dot1 >= 0.0f)){
		return true;
	}

	// otherwise go by distance
	return (dist1 < dist2);
}

// build the list of ship indices to use when updating for this player
void multi_oo_build_ship_list(net_player *pl)
{
	int ship_index;
	int idx;
	ship_obj *moveup;
	object *player_obj;

	// set all indices to be -1
	for(idx = 0;idx<MAX_SHIPS; idx++){
		OO_ship_index[idx] = -1;
	}

	// get the player object
	if(pl->m_player->objnum < 0){
		return;
	}
	player_obj = &Objects[pl->m_player->objnum];
	
	// go through all other relevant objects
	ship_index = 0;
	for ( moveup = GET_FIRST(&Ship_obj_list); moveup != END_OF_LIST(&Ship_obj_list); moveup = GET_NEXT(moveup) ) {
		// if it is an invalid ship object, skip it
		if((moveup->objnum < 0) || (Objects[moveup->objnum].instance < 0) || (Objects[moveup->objnum].type != OBJ_SHIP)){
			continue;
		}

		// if we're a standalone server, don't send any data regarding its pseudo-ship
		if((Game_mode & GM_STANDALONE_SERVER) && ((&Objects[moveup->objnum] == Player_obj) || (Objects[moveup->objnum].net_signature == STANDALONE_SHIP_SIG)) ){
			continue;
		}		
			
		// must be a ship, a weapon, and _not_ an observer
		if (Objects[moveup->objnum].flags[Object::Object_Flags::Should_be_dead]){
			continue;
		}

		// don't send info for dying ships -- Cyborg17 - Or dead ships that are going to respawn later.
		if (Ships[Objects[moveup->objnum].instance].flags[Ship::Ship_Flags::Dying] || Ships[Objects[moveup->objnum].instance].flags[Ship::Ship_Flags::Exploded]) {
			continue;
		}		

		// never update the knossos device
		if ((Ships[Objects[moveup->objnum].instance].ship_info_index >= 0) && (Ships[Objects[moveup->objnum].instance].ship_info_index < ship_info_size()) && (Ship_info[Ships[Objects[moveup->objnum].instance].ship_info_index].flags[Ship::Info_Flags::Knossos_device])){
			continue;
		}
				
		// don't send him info for himself
		if ( &Objects[moveup->objnum] == player_obj ){
			continue;
		}

		// don't send info for his targeted ship here, since its always done first
		if((pl->s_info.target_objnum != -1) && (moveup->objnum == pl->s_info.target_objnum)){
			continue;
		}

		// add the ship 
		if(ship_index < MAX_SHIPS){
			OO_ship_index[ship_index++] = (short)Objects[moveup->objnum].instance;
		}
	}

	// maybe sort the thing here
	OO_player_obj = player_obj;
	if (OO_sort) {
		std::sort(OO_ship_index, OO_ship_index + ship_index, multi_oo_sort_func);
	}
}


constexpr int OO_CLIENT_HEADER_SIZE = 4;	// flags and data_size ushorts
constexpr int OO_SERVER_HEADER_SIZE = 6; // flags, data_size, and net_signature ushorts
constexpr int OO_POSITION_UPDATE_SIZE = 28; // see the position section of pack_data() to know where this number is coming from.
constexpr int OO_MAX_CLIENT_DATA_SIZE = MAX_PACKET_SIZE - OO_MAIN_HEADER_SIZE - OO_CLIENT_HEADER_SIZE - OO_POSITION_UPDATE_SIZE;
constexpr int OO_MAX_DATA_SIZE = MAX_PACKET_SIZE - OO_MAIN_HEADER_SIZE - OO_SERVER_HEADER_SIZE;

// whatever crazy thing happens, keep the buffer from overflowing because we can just "erase" the part that overflowed it
constexpr int OO_SAFE_BUFFER_SIZE = 10000; 
constexpr int OO_LOCK_SIZE = 4; // from the 

// pack information for a client (myself), return bytes added
int multi_oo_pack_client_data(ubyte *data, ship* shipp)
{
	ubyte out_flags;
	ushort tnet_signature;
	int packet_size = 0;

	// get our firing stuff. Cyborg17 - This line is only for secondary fire, not other controls.
	out_flags = Net_player->s_info.accum_buttons;	
	
	// zero these values for now
	Net_player->s_info.accum_buttons = 0;

	// add any necessary targeting flags
	if ( Player_ai->ai_flags[AI::AI_Flags::Seek_lock] ){	
		out_flags |= OOC_TARGET_SEEK_LOCK;
	}
	if ( (Player_ship != nullptr) && (Player_ship->flags[Ship::Ship_Flags::Trigger_down]) ){
		out_flags |= OOC_TRIGGER_DOWN;
	}

	if ( (Player_obj != nullptr) && Player_obj->phys_info.flags & PF_AFTERBURNER_ON){
		out_flags |= OOC_AFTERBURNER_ON;
	}

	// send my bank info
	if(Player_ship != nullptr){
		if(Player_ship->weapons.current_primary_bank > 0){
			out_flags |= OOC_PRIMARY_BANK;
		}

		// linked or not
		if(Player_ship->flags[Ship::Ship_Flags::Primary_linked]){
			out_flags |= OOC_PRIMARY_LINKED;
		}
	}

	// copy the final flags in
	ADD_DATA( out_flags );

	// client targeting information	
	ushort t_subsys = OOC_INDEX_NULLPTR_SUBSYSEM;
	ushort l_subsys = OOC_INDEX_NULLPTR_SUBSYSEM;

	// if nothing targeted
	if(Player_ai->target_objnum == -1){
		tnet_signature = 0;
	}
	// if something is targeted 
	else {
		// target net signature
		tnet_signature = Objects[Player_ai->target_objnum].net_signature;
			
		// targeted subsys index
		if(Player_ai->targeted_subsys != nullptr){
			t_subsys = (char)ship_get_subsys_index( Player_ai->targeted_subsys );
		}

		// locked targeted subsys index
		if(Player->locking_subsys != nullptr){
			l_subsys = (char)ship_get_subsys_index( Player->locking_subsys );
		}
	}

	// add them all
	ADD_USHORT( tnet_signature );
	ADD_USHORT( t_subsys );
	ADD_USHORT( l_subsys );
	
	// multilock object update patch
	ushort count = 0;
	SCP_vector<ushort> lock_list;
	SCP_vector<ushort> subsystems;

	// look for locked slots
	for (auto & lock : shipp->missile_locks) {
		if (lock.locked) {
			lock_list.push_back(lock.obj->net_signature);
			// if the subsystem is a nullptr within the lock, send nullptr to the server.
			if (lock.subsys == nullptr) {
				subsystems.push_back(OOC_INDEX_NULLPTR_SUBSYSEM);
			} // otherwise, just send the subsystem index.
			else {
				subsystems.push_back( (ubyte)std::distance( GET_FIRST(&Ships[lock.obj->instance].subsys_list), lock.subsys) );
			}
				
			count++;

			// Check to see if the *next* lock will force us over the max.
			if (((count + 1) * OO_LOCK_SIZE + 1) >= OO_MAX_CLIENT_DATA_SIZE) {
				break;
			}
		}
	}

	// add the data we just found, in the correct order. (so the simulation will be as exact as possible)
	ADD_DATA(count);

	for (int i = 0; i < (int)lock_list.size(); i++) {
		ADD_USHORT(lock_list[i]);
		ADD_USHORT(subsystems[i]);
	}

	return packet_size;
}

// pack the appropriate info into the data
#define PACK_PERCENT(v) { std::uint8_t upercent; if(v < 0.0f){v = 0.0f;} upercent = (v * 255.0f) <= 255.0f ? (std::uint8_t)(v * 255.0f) : (std::uint8_t)255; memcpy(data + packet_size + header_bytes, &upercent, sizeof(std::uint8_t)); packet_size++; }
#define PACK_BYTE(v) { memcpy( data + packet_size + header_bytes, &v, 1 ); packet_size += 1; }
#define PACK_SHORT(v) { std::int16_t swap = INTEL_SHORT(v); memcpy( data + packet_size + header_bytes, &swap, sizeof(std::int16_t) ); packet_size += sizeof(std::int16_t); }
#define PACK_USHORT(v) { std::uint16_t swap = INTEL_SHORT(v); memcpy( data + packet_size + header_bytes, &swap, sizeof(std::uint16_t) ); packet_size += sizeof(std::uint16_t); }
#define PACK_INT(v) { std::int32_t swap = INTEL_INT(v); memcpy( data + packet_size + header_bytes, &swap, sizeof(std::int32_t) ); packet_size += sizeof(std::int32_t); }
#define PACK_ULONG(v) { std::uint64_t swap = INTEL_LONG(v); memcpy( data + packet_size + header_bytes, &swap, sizeof(std::uint64_t) ); packet_size += sizeof(std::uint64_t); }
int multi_oo_pack_data(net_player *pl, object *objp, ushort oo_flags, ubyte *data_out)
{
	ubyte data[OO_SAFE_BUFFER_SIZE];
	ushort data_size = 0;	// now a ushort because of IPv6 size extensions
	ship *shipp;	
	ship_info *sip;
	float temp_float;	
	int header_bytes;
	int packet_size = 0, ret = 0;

	// make sure we have a valid ship
	Assert(objp->type == OBJ_SHIP);
	if((objp->instance >= 0) && (Ships[objp->instance].ship_info_index >= 0)){
		shipp = &Ships[objp->instance];
		sip = &Ship_info[shipp->ship_info_index];
	} else {
		return 0;
	}			

	// invalid player
	if(pl == nullptr || shipp == nullptr){
		return 0;
	}

	// if no flags we now send an "empty" packet that tells the client "Keep this ship where it belongs"

	// if i'm the client, make sure I only send certain things	
	if (MULTIPLAYER_CLIENT) {
		Assert(!(oo_flags & (OO_HULL_NEW | OO_SHIELDS_NEW | OO_SUBSYSTEMS_NEW)));
		oo_flags &= ~(OO_HULL_NEW | OO_SHIELDS_NEW | OO_SUBSYSTEMS_NEW);
	} 

	// header sizes -- Cyborg17 - Note this is in place because the size of the packet is 
	// determined at the end of this function, and so we have to keep track of how much
	// we are adding to the packet throughout.
	if(MULTIPLAYER_MASTER){
		header_bytes = OO_SERVER_HEADER_SIZE;
	} else {
		header_bytes = OO_CLIENT_HEADER_SIZE;
	}	

	// putting this in the position bucket because it's mainly to help with position interpolation
	multi_rate_add(NET_PLAYER_NUM(pl), "pos", 1);


	// if we're a client (and therefore sending control info), pack client-specific info
	if((Net_player != NULL) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		packet_size += multi_oo_pack_client_data(data + packet_size + header_bytes, shipp);		
	}		
		
	// position - Now includes, position, orientation, velocity, rotational velocity, desired velocity and desired rotational velocity.
	// this should always be sent when it is determined to be needed.
	if ( oo_flags & OO_POS_AND_ORIENT_NEW ) {	
		ret = multi_pack_unpack_position( 1, data + packet_size + header_bytes, &objp->pos ); // 10 bytes
		packet_size += ret;

		// datarate tracking.
		multi_rate_add(NET_PLAYER_NUM(pl), "pos", ret);

		// orientation (now done via angles)
		angles temp_angles;
		vm_extract_angles_matrix_alternate(&temp_angles, &objp->orient);	

		// actual packing function, 6 bytes
		ret = multi_pack_unpack_orient( 1, data + packet_size + header_bytes, &temp_angles); 

		packet_size += ret;
		// datarate tracking.
		multi_rate_add(NET_PLAYER_NUM(pl), "ori", ret);	

		// velocity, 4 bytes-- Tried to do this by calculation instead but kept running into issues. 
		ret = multi_pack_unpack_vel(1, data + packet_size + header_bytes, &objp->orient, &objp->phys_info);

		packet_size += ret;
		// datarate tracking.
		multi_rate_add(NET_PLAYER_NUM(pl), "pos", ret);	

		// Rotational Velocity, 4 bytes
		ret = multi_pack_unpack_rotvel( 1, data + packet_size + header_bytes, &objp->phys_info );

		packet_size += ret;	

		// datarate tracking.		
		multi_rate_add(NET_PLAYER_NUM(pl), "ori", ret);		
		ret = 0;

		// in order to send data by axis we must rotate the global velocity into local coordinates
		vec3d local_desired_vel;

		vm_vec_rotate(&local_desired_vel, &objp->phys_info.desired_vel, &objp->orient);

		// is this a ship with full phyiscs? (just player-controled for now)
		bool full_physics = false;
		if (objp->flags[Object::Object_Flags::Player_ship]) {
			full_physics = true;
			oo_flags |= OO_FULL_PHYSICS;
		}

		// actual packing function, 4 bytes
		ret = multi_pack_unpack_desired_vel_and_desired_rotvel(1, full_physics, data + packet_size + header_bytes, &objp->phys_info, &local_desired_vel);

		packet_size += ret;
	}

	// datarate records	
	multi_rate_add(NET_PLAYER_NUM(pl), "fth", ret);	

	// hull info -- also should be required, but can never be sent by client, so unless something's really messed up,
	// at this point it is impossible to overflow the buffer.
	if (oo_flags & OO_HULL_NEW) {
		// add the hull value for this guy		
		temp_float = get_hull_pct(objp);
		if ((temp_float < 0.004f) && (temp_float > 0.0f)) {
			temp_float = 0.004f;		// 0.004 is the lowest positive value we can have before we zero out when packing
		}
		PACK_PERCENT(temp_float);
		multi_rate_add(NET_PLAYER_NUM(pl), "hul", 1);
	}

	// add shields, which can have now have a dynamic number of quadrants, we need to start checking for buffer overflow here
	if (oo_flags & OO_SHIELDS_NEW) {
		int pre_section_size = packet_size;

		float quad = shield_get_max_quad(objp);

		for (int i = 0; i < objp->n_quadrants; i++) {
			temp_float = (objp->shield_quadrant[i] / quad);
			PACK_PERCENT(temp_float);
		}
				
		// Check that we are not sending too much data, if so, don't actually send.
		if (packet_size > OO_MAX_DATA_SIZE) {
			nprintf(("Network","Had to remove shields section from data packet for %s\n", shipp->ship_name));
			packet_size = pre_section_size;
			oo_flags &= ~OO_SHIELDS_NEW;
		}
		else {
			multi_rate_add(NET_PLAYER_NUM(pl), "shl", objp->n_quadrants);	
		}
	}	

	// Cyborg17 - add the subsystem data, now with packer function.
	if ((MULTIPLAYER_MASTER || objp->flags[Object::Object_Flags::Player_ship]) && shipp->ship_info_index >= 0) {
		SCP_vector<ubyte> flags;
		SCP_vector<float> subsys_data;
		ubyte i = 0;

		flags.reserve(MAX_MODEL_SUBSYSTEMS);
		subsys_data.reserve(MAX_MODEL_SUBSYSTEMS); // propbably won't exceed this, and even if it does, it will get cut off.

		for (ship_subsys* subsystem = GET_FIRST(&shipp->subsys_list); subsystem != END_OF_LIST(&shipp->subsys_list);
			subsystem = GET_NEXT(subsystem)) {
			flags.push_back(0);
			// Don't send destroyed subsystems, (another packet handles that), but check to see if the subsystem changed since the last update. 
			if (MULTIPLAYER_MASTER && (subsystem->current_hits != 0.0f) && (subsystem->current_hits != Oo_info.player_frame_info[pl->player_id].last_sent[objp->net_signature].subsystem_health[i])) {
				flags[i] |= OO_SUBSYS_HEALTH;
				subsys_data.push_back(subsystem->current_hits / subsystem->max_hits);
				// good thing this cheap because we have to calculate this twice to avoid iterating through the whole system list twice.
				Oo_info.player_frame_info[pl->player_id].last_sent[objp->net_signature].subsystem_health[i] = subsystem->current_hits;

				// this should be safe because we only work with subsystems that have health.
				// and also track the list of subsystems that we packed by index
			}
			

			// retrieve the submodel for rotation info.
			if (subsystem->system_info->flags[Model::Subsystem_Flags::Rotates]) {
				angles *angs_1 = nullptr;
				angles *angs_2 = nullptr;
				if (subsystem->submodel_instance_1) {
					angs_1 = new angles;
					vm_extract_angles_matrix_alternate(angs_1, &subsystem->submodel_instance_1->canonical_orient);
				}
				if (subsystem->submodel_instance_2) {
					angs_2 = new angles;
					vm_extract_angles_matrix_alternate(angs_2, &subsystem->submodel_instance_2->canonical_orient);
				}

				// here we're checking to see if the subsystems rotated enough to send.
				if (angs_1 != nullptr && angs_1->b != Oo_info.player_frame_info[pl->player_id].last_sent[objp->net_signature].subsystem_1b[i]) {
					flags[i] |= OO_SUBSYS_ROTATION_1b;
					subsys_data.push_back(angs_1->b / PI2);
				}

				if (angs_1 != nullptr && angs_1->h != Oo_info.player_frame_info[pl->player_id].last_sent[objp->net_signature].subsystem_1h[i]) {
					flags[i] |= OO_SUBSYS_ROTATION_1h;
					subsys_data.push_back(angs_1->h / PI2);
				}

				if (angs_1 != nullptr && angs_1->p != Oo_info.player_frame_info[pl->player_id].last_sent[objp->net_signature].subsystem_1p[i]) {
					flags[i] |= OO_SUBSYS_ROTATION_1p;
					subsys_data.push_back(angs_1->p / PI2);
				}

				if (angs_2 != nullptr && angs_2->b != Oo_info.player_frame_info[pl->player_id].last_sent[objp->net_signature].subsystem_2b[i]) {
					flags[i] |= OO_SUBSYS_ROTATION_2b;
					subsys_data.push_back(angs_2->b / PI2);
				}

				if (angs_2 != nullptr && angs_2->h != Oo_info.player_frame_info[pl->player_id].last_sent[objp->net_signature].subsystem_2h[i]) {
					flags[i] |= OO_SUBSYS_ROTATION_2h;
					subsys_data.push_back(angs_2->h / PI2);
				}

				if (angs_2 != nullptr && angs_2->p != Oo_info.player_frame_info[pl->player_id].last_sent[objp->net_signature].subsystem_2p[i]) {
					flags[i] |= OO_SUBSYS_ROTATION_2p;
					subsys_data.push_back(angs_2->p / PI2);
				}

				// clang says deleting null pointer has no effect
				delete angs_1;
				delete angs_2;
			}
			i++;
		}

		// Only send info if the count is greater than zero and if we're *not* on the very first frame when everything is already synced, anyway.
		if (!subsys_data.empty() && Oo_info.number_of_frames != 0){

			Assertion(i <= MAX_MODEL_SUBSYSTEMS, "Object Update packet exceeded limit for number of subsystems. This is a coder error, please report!\n");
			ret = multi_pack_unpack_subsystem_list(true, data + packet_size + header_bytes, &flags, &subsys_data);

			// Check that we are not sending too much data, if so, don't actually send.
			if (packet_size + ret <= OO_MAX_DATA_SIZE) {
				oo_flags |= OO_SUBSYSTEMS_NEW;		
				packet_size += ret;
			} else {
				nprintf(("Network","Had to remove subsystems section from data packet for %s\n", shipp->ship_name));
			}
		}
	}

	// Cyborg17 - only server should send this
	if (oo_flags & OO_AI_NEW){
		int pre_section_size = packet_size;
		ai_info *aip = &Ai_info[shipp->ai_index];

		// ai mode info
		auto umode = (ubyte)(aip->mode);
		auto submode = (short)(aip->submode);
		ushort target_signature = 0;

		// either send out the waypoint they are trying to get to *or* their current target
		if (umode == AIM_WAYPOINTS) {
			// if it's already started pointing to a waypoint, grab its net_signature and send that instead
			if ((aip->wp_list != nullptr) && (aip->wp_list->get_waypoints().size() > aip->wp_index)) {
				target_signature = Objects[aip->wp_list->get_waypoints().at(aip->wp_index).get_objnum()].net_signature;
			}
		} // send the target signature. 2021 Version!
		else if ((aip->goals[0].target_name != nullptr) && strlen(aip->goals[0].target_name) != 0) {
			
			int instance = ship_name_lookup(aip->goals[0].target_name);
			if (instance > -1) {
				target_signature = Objects[Ships[instance].objnum].net_signature;
			}
		}

		PACK_BYTE( umode );
		PACK_SHORT( submode );
		PACK_USHORT( target_signature );	

		// primary weapon energy
		temp_float = shipp->weapon_energy / sip->max_weapon_reserve;
		PACK_PERCENT(temp_float);

		// check for adding too much data, if so don't send what we just wrote.
		if (packet_size > OO_MAX_DATA_SIZE) {
			nprintf(("Network","Had to remove AI section from data packet for %s\n", shipp->ship_name));
			packet_size = pre_section_size;
			oo_flags &= ~OO_AI_NEW;
		} // otherwise, make sure it gets counted int the rate limiting system.
		else {
			multi_rate_add(NET_PLAYER_NUM(pl), "aim", 5);
		}
	}		

	// if this ship is a support ship, send some extra info
	if(MULTIPLAYER_MASTER && (sip->flags[Ship::Info_Flags::Support]) && (shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO)){
		int pre_section_size = packet_size;
		ushort dock_sig;

		PACK_ULONG( Ai_info[shipp->ai_index].ai_flags.to_u64() );
		PACK_INT( Ai_info[shipp->ai_index].mode );
		PACK_INT( Ai_info[shipp->ai_index].submode );

		if((Ai_info[shipp->ai_index].support_ship_objnum < 0) || (Ai_info[shipp->ai_index].support_ship_objnum >= MAX_OBJECTS)){
			dock_sig = 0;
		} else {
			dock_sig = Objects[Ai_info[shipp->ai_index].support_ship_objnum].net_signature;
		}		

		PACK_USHORT( dock_sig );

		// check for adding too much data, if so don't send what we just wrote.
		if (packet_size > OO_MAX_DATA_SIZE) {
			nprintf(("Network","Had to remove support ship section from data packet for %s\n", shipp->ship_name));
			packet_size = pre_section_size;
		}
		else {
			oo_flags |= OO_SUPPORT_SHIP;
		}
	}

	// afterburner info
	oo_flags &= ~OO_AFTERBURNER_NEW;
	if(objp->phys_info.flags & PF_AFTERBURNER_ON){
		oo_flags |= OO_AFTERBURNER_NEW;
	}

	// make sure we have a a packet that will fit.
	Assertion(packet_size <= OO_MAX_DATA_SIZE, "Somehow, the buffer overflow safeguards that are supposed to keep the OO_packet from overflowing failed for %s and came up to a size of %d. Please report!!", shipp->ship_name, packet_size);
	if(packet_size >= OO_MAX_DATA_SIZE){
		return 0;
	}
	data_size = (ushort)packet_size;

	// reset packet_size so that we add the header at the beginning of the packet where it belongs.
	packet_size = 0;
	// don't add for clients
	if(MULTIPLAYER_MASTER){		
		multi_rate_add(NET_PLAYER_NUM(pl), "sig", 2);
		ADD_USHORT( objp->net_signature );
	}	

	multi_rate_add(NET_PLAYER_NUM(pl), "flg", 1);
	ADD_USHORT( oo_flags );

	multi_rate_add(NET_PLAYER_NUM(pl), "siz", 1);
	ADD_USHORT( data_size );	
	
	packet_size += data_size;

	// copy to the outgoing data
	memcpy(data_out, data, packet_size);	
	
	return packet_size;	
}

// unpack information for a client, return bytes processed
int multi_oo_unpack_client_data(net_player* pl, ubyte* data)
{
	ushort in_flags;
	ship* shipp = nullptr;
	object* objp = nullptr;
	int offset = 0;

	if (pl == nullptr)
		Error(LOCATION, "Invalid net_player pointer passed to multi_oo_unpack_client\n");

	memcpy(&in_flags, data, sizeof(ubyte));
	offset++;

	// get the player ship and object
	if ((pl->m_player->objnum >= 0) && (Objects[pl->m_player->objnum].type == OBJ_SHIP) && (Objects[pl->m_player->objnum].instance >= 0)) {
		objp = &Objects[pl->m_player->objnum];
		shipp = &Ships[objp->instance];
	}

	// if we have a valid netplayer pointer
	if ((pl != nullptr) && !(pl->flags & NETINFO_FLAG_RESPAWNING) && !(pl->flags & NETINFO_FLAG_LIMBO)) {
		// primary fired
		pl->m_player->ci.fire_primary_count = 0;

		// secondary fired
		pl->m_player->ci.fire_secondary_count = 0;
		if (in_flags & OOC_FIRE_CONTROL_PRESSED) {
			pl->m_player->ci.fire_secondary_count = 1;
		}

		// countermeasure fired		
		pl->m_player->ci.fire_countermeasure_count = 0;

		// trigger down, bank info
		if (shipp != nullptr) {
			if (in_flags & OOC_TRIGGER_DOWN) {
				shipp->flags.set(Ship::Ship_Flags::Trigger_down);
			}
			else {
				shipp->flags.remove(Ship::Ship_Flags::Trigger_down);
			}

			if (in_flags & OOC_PRIMARY_BANK) {
				shipp->weapons.current_primary_bank = 1;
			}
			else {
				shipp->weapons.current_primary_bank = 0;
			}

			// linked or not								
			shipp->flags.remove(Ship::Ship_Flags::Primary_linked);
			if (in_flags & OOC_PRIMARY_LINKED) {
				shipp->flags.set(Ship::Ship_Flags::Primary_linked);
			}
		}

		// other locking information
		if ((shipp != nullptr) && (shipp->ai_index != -1)) {
			Ai_info[shipp->ai_index].ai_flags.set(AI::AI_Flags::Seek_lock, (in_flags & OOC_TARGET_SEEK_LOCK) != 0);
		}

		// afterburner status
		if ( (objp != nullptr) && (in_flags & OOC_AFTERBURNER_ON) ) {
			Afterburn_hack = true;
		}
	}

	// client targeting information	
	ushort tnet_sig;
	ushort t_subsys, l_subsys;
	object* tobj;

	// get the data
	GET_USHORT(tnet_sig);
	GET_USHORT(t_subsys);
	GET_USHORT(l_subsys);

	// try and find the targeted object
	tobj = nullptr;
	if (tnet_sig != 0) {
		tobj = multi_get_network_object(tnet_sig);
	}
	// maybe fill in targeted object values
	if ((tobj != nullptr) && (pl != nullptr) && (pl->m_player->objnum != -1)) {
		// assign the target object
		if (Objects[pl->m_player->objnum].type == OBJ_SHIP) {
			Ai_info[Ships[Objects[pl->m_player->objnum].instance].ai_index].target_objnum = OBJ_INDEX(tobj);
		}
		pl->s_info.target_objnum = OBJ_INDEX(tobj);

		// assign subsystems if possible					
		if (Objects[pl->m_player->objnum].type == OBJ_SHIP) {
			Ai_info[Ships[Objects[pl->m_player->objnum].instance].ai_index].targeted_subsys = nullptr;
			if ((t_subsys != OOC_INDEX_NULLPTR_SUBSYSEM) && (tobj->type == OBJ_SHIP)) {
				Ai_info[Ships[Objects[pl->m_player->objnum].instance].ai_index].targeted_subsys = ship_get_indexed_subsys(&Ships[tobj->instance], t_subsys);
			}
		}

		pl->m_player->locking_subsys = nullptr;
		if (Objects[pl->m_player->objnum].type == OBJ_SHIP) {
			if ((l_subsys != OOC_INDEX_NULLPTR_SUBSYSEM) && (tobj->type == OBJ_SHIP)) {
				pl->m_player->locking_subsys = ship_get_indexed_subsys(&Ships[tobj->instance], l_subsys);
			}
		}
	}

	// Cyborg17 - this section allows multilock to work in multiplayer.
	ushort count;

	// Get how many locks were in the packet.
	GET_USHORT(count);


	lock_info temp_lock_info;
	ship_clear_lock(&temp_lock_info);
	temp_lock_info.locked = true;

	ushort multilock_target_net_signature;
	ushort subsystem_index;

	// clear whatever we had before, because we're getting brand new info straight from the client.
	if (shipp != nullptr) {
		shipp->missile_locks.clear();
	}

	// add each lock, one at a time.
	for (int i = 0; i < count; i++) {
		GET_USHORT(multilock_target_net_signature);
		GET_USHORT(subsystem_index);
		temp_lock_info.obj = multi_get_network_object(multilock_target_net_signature);

		if (temp_lock_info.obj != nullptr && shipp != nullptr) {
			// if the subsystem is the special null value.
			if (subsystem_index == OOC_INDEX_NULLPTR_SUBSYSEM) {
				temp_lock_info.subsys = nullptr;
			} // otherwise look it up to store the lock onto the subsystem
			else {
				ship_subsys* ml_target_subsysp = GET_FIRST(&Ships[temp_lock_info.obj->instance].subsys_list);
				for (int j = 0; j < subsystem_index; j++) {
					ml_target_subsysp = GET_NEXT(ml_target_subsysp);
				}
				temp_lock_info.subsys = ml_target_subsysp;
			}
			// store the lock.
			shipp->missile_locks.push_back(temp_lock_info);
		}
		else if (shipp != nullptr) {
			shipp->missile_locks.push_back(temp_lock_info);
		}
	}
	return offset;
}

// unpack the object data, return bytes processed
// Cyborg17 - This function has been revamped to ignore out of date information by type.  For example, if we got pos info
// more recently, but the packet has the newest AI info, we will still use the AI info, even though it's not the newest
// packet.
#define UNPACK_PERCENT(v)					{ ubyte temp_byte; memcpy(&temp_byte, data + offset, sizeof(ubyte)); v = (float)temp_byte / 255.0f; offset++;}
int multi_oo_unpack_data(net_player* pl, ubyte* data, int seq_num, int time_delta)
{
	int offset = 0;
	object* pobjp;
	ushort net_sig = 0;
	ushort data_size; // now a ushort because of IPv6
	ushort oo_flags;
	float fpct;
	ship* shipp;
	ship_info* sip;

	// ---------------------------------------------------------------------------------------------------------------
	// Header Processing
	// ---------------------------------------------------------------------------------------------------------------

	// add the object's net signature, type and oo_flags
	if (!(Net_player->flags & NETINFO_FLAG_AM_MASTER)) {
		GET_USHORT(net_sig);
	}

	// clients always pos and orient stuff only
	GET_USHORT(oo_flags);
	GET_USHORT(data_size);
	if (MULTIPLAYER_MASTER) {
		// client cannot send these types because the server is in charge of all of these things.
		Assertion(!(oo_flags & (OO_AI_NEW | OO_SHIELDS_NEW | OO_HULL_NEW | OO_SUPPORT_SHIP)), "Invalid flag from client, please report! oo_flags value: %d\n", oo_flags);
		if (oo_flags & (OO_AI_NEW | OO_SHIELDS_NEW | OO_HULL_NEW | OO_SUPPORT_SHIP)) {
			offset += data_size;
			return offset;
		}
	}
	// try and find the object
	if (MULTIPLAYER_CLIENT) {
		pobjp = multi_get_network_object(net_sig);
	}
	else {
		if ((pl != nullptr) && (pl->m_player->objnum != -1)) {
			pobjp = &Objects[pl->m_player->objnum];
			// Cyborg17 - We still need the net_signature if we're the Server because that's how we track interpolation info now.
			net_sig = pobjp->net_signature;
		}
		else {
			pobjp = nullptr;
		}
	}

	// if we can't find the object, skip the packet
	if ( (pobjp == nullptr) || (pobjp->type != OBJ_SHIP) || (pobjp->instance < 0) || (pobjp->instance >= MAX_SHIPS) || (Ships[pobjp->instance].ship_info_index < 0) || (Ships[pobjp->instance].ship_info_index >= ship_info_size())) {
		offset += data_size;
		return offset;
	}

	// ship pointer
	shipp = &Ships[pobjp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	// Cyborg17 - determine if this is the most recently updated ship.  If it is, it will become the ship that the
	// client will use as its reference when sending a primary shot packet.  Since Rollback uses pos and orient, only rank if it has that info.
	if (MULTIPLAYER_CLIENT && (oo_flags & OO_POS_AND_ORIENT_NEW)) {
		multi_ship_record_rank_seq_num(pobjp, seq_num);
	}

	// ---------------------------------------------------------------------------------------------------------------
	// SPECIAL CLIENT INFO
	// ---------------------------------------------------------------------------------------------------------------

	// if this is from a player, read his button info
	if(MULTIPLAYER_MASTER){
		int r0 = multi_oo_unpack_client_data(pl, data + offset);		
		offset += r0;
	}

	// ---------------------------------------------------------------------------------------------------------------
	// CRITICAL OBJECT UPDATE SHIZ
	// ---------------------------------------------------------------------------------------------------------------
	// (Positon, Orientation, velocities, and desired velocities)

	// Have "new info" default to the old info before reading
	vec3d new_pos = pobjp->pos;
	angles new_angles;
	matrix new_orient = pobjp->orient;
	physics_info new_phys_info = pobjp->phys_info;

	if ( oo_flags & OO_POS_AND_ORIENT_NEW) {

		// unpack position
		int r1 = multi_pack_unpack_position(0, data + offset, &new_pos);
		offset += r1;

		// unpack orientation
		int r2 = multi_pack_unpack_orient( 0, data + offset, &new_angles );
		offset += r2;

		// new version of the orient packer sends angles instead to save on bandwidth, so we'll need the orienation from that.
		vm_angles_2_matrix(&new_orient, &new_angles);

		int r3 = multi_pack_unpack_vel(0, data + offset, &new_orient, &new_phys_info);
		offset += r3;

		int r4 = multi_pack_unpack_rotvel( 0, data + offset, &new_phys_info );
		offset += r4;

		vec3d local_desired_vel = vmd_zero_vector;
		
		bool full_physics = false;

		if (oo_flags & OO_FULL_PHYSICS) {
			full_physics = true;
		}

		int r5 = multi_pack_unpack_desired_vel_and_desired_rotvel(0, full_physics, data + offset, &pobjp->phys_info, &local_desired_vel);
		offset += r5;
		// change it back to global coordinates.
		vm_vec_unrotate(&new_phys_info.desired_vel, &local_desired_vel, &new_orient);

		if (!full_physics) {
			new_phys_info.desired_rotvel = new_phys_info.rotvel;
		}

		pobjp->interp_info.add_packet(seq_num, time_delta, &new_pos, &new_phys_info.vel, &new_phys_info.rotvel, &new_phys_info.desired_vel, &new_phys_info.desired_rotvel, &new_angles, pl->player_id);
	}

	// Packet processing needs to stop here if the ship is still arriving, leaving, dead or dying to prevent bugs.
	if (shipp->is_arriving() || shipp->is_dying_or_departing() || shipp->flags[Ship::Ship_Flags::Exploded]) {
		int header_bytes = (MULTIPLAYER_MASTER) ? OO_CLIENT_HEADER_SIZE : OO_SERVER_HEADER_SIZE;		
		offset = header_bytes + data_size;
		return offset;
	}

	// ---------------------------------------------------------------------------------------------------------------
	// SHIP STATUS
	// ---------------------------------------------------------------------------------------------------------------
	
	// hull info
	if ( oo_flags & OO_HULL_NEW ){
		UNPACK_PERCENT(fpct);
		if (seq_num > pobjp->interp_info.get_hull_comparison_frame()) {
			pobjp->hull_strength = fpct * Ships[pobjp->instance].ship_max_hull_strength;
			pobjp->interp_info.set_hull_comparison_frame(seq_num);
		}
	}	

	// update shields
	if (oo_flags & OO_SHIELDS_NEW) {
		float quad = shield_get_max_quad(pobjp);

		// check before unpacking here so we don't have to recheck for each quadrant.
		if (seq_num > pobjp->interp_info.get_shields_comparison_frame()) {
			for (int i = 0; i < pobjp->n_quadrants; i++) {
				UNPACK_PERCENT(fpct);
				pobjp->shield_quadrant[i] = fpct * quad;
			}
			pobjp->interp_info.set_shields_comparison_frame(seq_num);
		}
		else {
			for (int i = 0; i < pobjp->n_quadrants; i++) {
				UNPACK_PERCENT(fpct);
			}
		}
	}

	// get the subsystem info
	if (oo_flags & OO_SUBSYSTEMS_NEW) {
		SCP_vector<ubyte> flags;  flags.reserve(MAX_MODEL_SUBSYSTEMS);
		SCP_vector<float> subsys_data;  subsys_data.reserve(MAX_MODEL_SUBSYSTEMS); // couldn't think of a better constant to put here
		
		int ret7 = multi_pack_unpack_subsystem_list(false, data + offset, &flags, &subsys_data);
		offset += ret7;

		if (NOT_EMPTY(&shipp->subsys_list)) {

			// Before we start the loop, we need to get the first subsystem, to make sure that it's set up to avoid issues.
			ship_subsys* subsysp = GET_FIRST(&shipp->subsys_list);

			// and the index to use in the subsys_data vector
			int data_idx = 0;

			// look for a match, in order to set values.
			for (int i = 0; i < (int)flags.size(); i++) {

				if (subsysp == END_OF_LIST(&shipp->subsys_list)) {
					break;
				}

				// the current subsystem had no info, so try the next subsystem.
				if (flags[i] == 0) {
					subsysp = GET_NEXT(subsysp);
					continue;
				}

				// update health
				if (flags[i] & OO_SUBSYS_HEALTH) {
					if (seq_num > pobjp->interp_info.get_subsystem_comparison_frame(i)) {
						pobjp->interp_info.set_subsystem_comparison_frames(i, seq_num);
						subsysp->current_hits = subsys_data[data_idx] * subsysp->max_hits;

						// Aggregate if necessary.
						if (!(subsysp->flags[Ship::Subsystem_Flags::No_aggregate])) {
							shipp->subsys_info[subsysp->system_info->type].aggregate_current_hits += subsysp->current_hits;
						}
					}
					data_idx++;
				}

				angles *prev_angs_1 = nullptr;
				angles *prev_angs_2 = nullptr;
				angles *angs_1 = nullptr;
				angles *angs_2 = nullptr;
				if (subsysp->submodel_instance_1) {
					prev_angs_1 = new angles;
					angs_1 = new angles;
					vm_extract_angles_matrix_alternate(prev_angs_1, &subsysp->submodel_instance_1->canonical_prev_orient);
					vm_extract_angles_matrix_alternate(angs_1, &subsysp->submodel_instance_1->canonical_orient);
				}
				if (subsysp->submodel_instance_2) {
					prev_angs_2 = new angles;
					angs_2 = new angles;
					vm_extract_angles_matrix_alternate(prev_angs_2, &subsysp->submodel_instance_2->canonical_prev_orient);
					vm_extract_angles_matrix_alternate(angs_2, &subsysp->submodel_instance_2->canonical_orient);
				}

				if (flags[i] & OO_SUBSYS_ROTATION_1b) {
					prev_angs_1->b = angs_1->b;
					angs_1->b = (subsys_data[data_idx] * PI2);
					data_idx++;
				}

				if (flags[i] & OO_SUBSYS_ROTATION_1h) {
					prev_angs_1->h = angs_1->h;
					angs_1->h = (subsys_data[data_idx] * PI2);
					data_idx++;
				}

				if (flags[i] & OO_SUBSYS_ROTATION_1p) {
					prev_angs_1->p = angs_1->p;
					angs_1->p = (subsys_data[data_idx] * PI2);
					data_idx++;
				}

				if (flags[i] & OO_SUBSYS_ROTATION_2b) {
					prev_angs_2->b = angs_2->b;
					angs_2->b = (subsys_data[data_idx] * PI2);
					data_idx++;
				}

				if (flags[i] & OO_SUBSYS_ROTATION_2h) {
					prev_angs_2->h = angs_2->h;
					angs_2->h = (subsys_data[data_idx] * PI2);
					data_idx++;
				}

				if (flags[i] & OO_SUBSYS_ROTATION_2p) {
					prev_angs_2->p = angs_2->p;
					angs_2->p = (subsys_data[data_idx] * PI2);
					data_idx++;
				}

				// fix up the matrixes
				if (flags[i] & OO_SUBSYS_ROTATION_1) {
					vm_angles_2_matrix(&subsysp->submodel_instance_1->canonical_prev_orient, prev_angs_1);
					vm_angles_2_matrix(&subsysp->submodel_instance_1->canonical_orient, angs_1);
					delete prev_angs_1;
					delete angs_1;
				}
				if (flags[i] & OO_SUBSYS_ROTATION_2) {
					vm_angles_2_matrix(&subsysp->submodel_instance_2->canonical_prev_orient, prev_angs_2);
					vm_angles_2_matrix(&subsysp->submodel_instance_2->canonical_orient, angs_2);
					delete prev_angs_2;
					delete angs_2;
				}

				subsysp = GET_NEXT(subsysp);

			}
			// recalculate all ship subsystems
			ship_recalc_subsys_strength(shipp);
		}

	}

	// ---------------------------------------------------------------------------------------------------------------
	// AI & SUPPORT SHIP INFO
	// ---------------------------------------------------------------------------------------------------------------

	if ( oo_flags & OO_AI_NEW ) {
		// ai mode info
		ubyte umode;
		short submode;
		ushort target_signature;
		object *target_objp;

		// AI info
		GET_DATA( umode );
		GET_SHORT( submode );
		GET_USHORT( target_signature );		

		// primary weapon energy		
		float weapon_energy_pct;
		UNPACK_PERCENT(weapon_energy_pct);

		if( seq_num > pobjp->interp_info.get_ai_comparison_frame() ){
			if ( shipp->ai_index >= 0 ){
				// make sure to undo the wrap if it occurred during compression for unset ai mode.
				if (umode == 255) {
					Ai_info[shipp->ai_index].mode = -1; 
				}
				else {
					Ai_info[shipp->ai_index].mode = umode;
				}
				Ai_info[shipp->ai_index].submode = submode;		

				// set this guy's target objnum, and other info
				target_objp = multi_get_network_object( target_signature );

				Ai_info[shipp->ai_index].mode = umode;
				Ai_info[shipp->ai_index].submode = submode;		

				// if the info was bogus, set the target to an invalid object, this is a general failure state
				if ( umode == 255 || target_objp == nullptr ){
					Ai_info[shipp->ai_index].mode = -1; 
					Ai_info[shipp->ai_index].submode = -1;
					Ai_info[shipp->ai_index].target_objnum = -1;
					Ai_info[shipp->ai_index].goals[0].target_name = nullptr;
				// set their waypoints if in waypoint mode.
				} else if (umode == AIM_WAYPOINTS) {
					waypoint* destination = find_waypoint_with_instance(target_objp->instance);
					if (destination != nullptr) {
						Ai_info[shipp->ai_index].wp_list = destination->get_parent_list();
						Ai_info[shipp->ai_index].wp_index = find_index_of_waypoint(Ai_info[shipp->ai_index].wp_list, destination);
					} else {
						Ai_info[shipp->ai_index].wp_list = nullptr;
					}
				} else {
					Ai_info[shipp->ai_index].target_objnum = OBJ_INDEX(target_objp);
					Ai_info[shipp->ai_index].goals[0].target_name = Ships[target_objp->instance].ship_name;
				}
			}

			shipp->weapon_energy = sip->max_weapon_reserve * weapon_energy_pct;

			pobjp->interp_info.set_ai_comparison_frame(seq_num);
		}		
	}	

	if(oo_flags & OO_SUPPORT_SHIP){
		ushort dock_sig;
		int ai_mode, ai_submode;
		std::uint64_t ai_flags;

		// flag		
		GET_ULONG(ai_flags);
		GET_INT(ai_mode);
		GET_INT(ai_submode);
		GET_USHORT(dock_sig);		

		// verify that it's a valid ship							
		if((shipp != nullptr) && (shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO)){
			// bash ai info, this info does not get rebashed, because it is not as vital.
			Ai_info[shipp->ai_index].ai_flags.from_u64(ai_flags);
			Ai_info[shipp->ai_index].mode = ai_mode;
			Ai_info[shipp->ai_index].submode = ai_submode;

			object *objp = multi_get_network_object( dock_sig );
			if(objp != nullptr){
				Ai_info[shipp->ai_index].support_ship_objnum = OBJ_INDEX(objp);
				if ((objp->instance > -1) && (objp->type == OBJ_SHIP)) {
					Ai_info[shipp->ai_index].goals[0].target_name = Ships[objp->instance].ship_name;
					Ai_info[shipp->ai_index].goals[0].target_signature = objp->signature;
				} else {
					Ai_info[shipp->ai_index].goals[0].target_name = nullptr;
					Ai_info[shipp->ai_index].goals[0].target_signature = 0;
				}
			}
		}			
	} 

	// make sure the ab hack is reset before we read in new info
	Afterburn_hack = false;

	// afterburner info
	if ( (oo_flags & OO_AFTERBURNER_NEW) || Afterburn_hack ) {
		// maybe turn them on
		if(!(pobjp->phys_info.flags & PF_AFTERBURNER_ON)){
			afterburners_start(pobjp);
		}

		// make sure the ab hack is reset before we read in new info
		Afterburn_hack = false;
	} else {
		// maybe turn them off
		if(pobjp->phys_info.flags & PF_AFTERBURNER_ON){
			afterburners_stop(pobjp);
		}
	}

	// primary info (only clients care about this)
	if( !MULTIPLAYER_MASTER && (shipp != nullptr) ){
		// what bank
		if(oo_flags & OO_PRIMARY_BANK){
			shipp->weapons.current_primary_bank = 1;
		} else {
			shipp->weapons.current_primary_bank = 0;
		}

		// linked or not
        shipp->flags.remove(Ship::Ship_Flags::Primary_linked);
		if(oo_flags & OO_PRIMARY_LINKED){
			shipp->flags.set(Ship::Ship_Flags::Primary_linked);
		}

		// trigger down or not - server doesn't care about this. he'll get it from clients anyway		
		shipp->flags.remove(Ship::Ship_Flags::Trigger_down);
		if(oo_flags & OO_TRIGGER_DOWN){
			shipp->flags.set(Ship::Ship_Flags::Trigger_down);
		}		
	}
	
	// if we're the multiplayer server, set eye position and orient
	if(MULTIPLAYER_MASTER && (pl != nullptr) && (pobjp != nullptr)){
		pl->s_info.eye_pos = pobjp->pos;
		pl->s_info.eye_orient = pobjp->orient;
	} 		
	return offset;
}

// reset the timestamp appropriately for the passed in object
void multi_oo_reset_timestamp(net_player *pl, object *objp, int range, int in_cone)
{
	int stamp = 0;	

	// if this is the guy's target, 
	if((pl->s_info.target_objnum != -1) && (pl->s_info.target_objnum == OBJ_INDEX(objp))){
		stamp = Multi_oo_target_update_times[pl->p_info.options.obj_update_level];
	} else {
		// reset the timestamp appropriately
		if(in_cone){
			// base it upon range
			switch(range){
			case OO_NEAR:
				stamp = Multi_oo_front_near_update_times[pl->p_info.options.obj_update_level];
				break;

			case OO_MIDRANGE:
				stamp = Multi_oo_front_medium_update_times[pl->p_info.options.obj_update_level];
				break;

			case OO_FAR:
				stamp = Multi_oo_front_far_update_times[pl->p_info.options.obj_update_level];
				break;
			}
		} else {
			// base it upon range
			switch(range){
			case OO_NEAR:
				stamp = Multi_oo_rear_near_update_times[pl->p_info.options.obj_update_level];
				break;

			case OO_MIDRANGE:
				stamp = Multi_oo_rear_medium_update_times[pl->p_info.options.obj_update_level];
				break;

			case OO_FAR:
				stamp = Multi_oo_rear_far_update_times[pl->p_info.options.obj_update_level];
				break;
			}
		}						
	}

	// reset the timestamp for this object
	if(objp->type == OBJ_SHIP){
		Oo_info.player_frame_info[pl->player_id].last_sent[objp->net_signature].timestamp = _timestamp(stamp);
	} 
}

// determine what needs to get sent for this player regarding the passed object, and when
int multi_oo_maybe_update(net_player *pl, object *obj, ubyte *data)
{
	ushort oo_flags = 0;
	TIMESTAMP stamp;
	int player_index;
	vec3d player_eye;
	vec3d obj_dot;
	float eye_dot, dist;
	int in_cone;
	int range;
	ship *shipp;
	ship_info *sip;

	player_index = NET_PLAYER_INDEX(pl);
	if(!(player_index >= 0) || !(player_index < MAX_PLAYERS)){
		return 0;
	}

	int net_sig_idx = obj->net_signature;

	// determine what the timestamp is for this object
	if(obj->type == OBJ_SHIP){
		stamp = Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].timestamp;
	} else {
		return 0;
	}

	// if the timestamp hasn't elapsed for this guy return - This timestamp_elapsed
	if(!(stamp.isNever()) && !(timestamp_elapsed_safe(stamp, OO_MAX_TIMESTAMP))){
		return 0;
	}
	
	// if we're supposed to update this guy	start figuring out what we should send.

	// get the ship pointer
	shipp = &Ships[obj->instance];

	// get ship info pointer
	sip = nullptr;
	if(shipp->ship_info_index >= 0){
		sip = &Ship_info[shipp->ship_info_index];
	}
	
	// check dot products		
	player_eye = pl->s_info.eye_orient.vec.fvec;
	vm_vec_sub(&obj_dot, &obj->pos, &pl->s_info.eye_pos);
	in_cone = 0;
	if (!(IS_VEC_NULL(&obj_dot))) {
		vm_vec_normalize(&obj_dot);
		eye_dot = vm_vec_dot(&obj_dot, &player_eye);		
		in_cone = (eye_dot >= OO_VIEW_CONE_DOT) ? 1 : 0;
	}
							
	// determine distance (near, medium, far)
	vm_vec_sub(&obj_dot, &obj->pos, &pl->s_info.eye_pos);
	dist = vm_vec_mag(&obj_dot);		
	if(dist < OO_NEAR_DIST){
		range = OO_NEAR;
	} else if(dist < OO_MIDRANGE_DIST){
		range = OO_MIDRANGE;
	} else {
		range = OO_FAR;
	}

	// reset the timestamp for the next update for this guy
	multi_oo_reset_timestamp(pl, obj, range, in_cone);
	
	// position should be almost constant, except for ships that aren't moving.
	if ( (Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].position != obj->pos) && (vm_vec_mag_quick(&obj->phys_info.vel) > 0.0f ) ) {
		oo_flags |= OO_POS_AND_ORIENT_NEW;
		// update the last position sent, will be done in each of the cases below.
		Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].position = obj->pos;
	}   // same with orientation
	else if (obj->phys_info.rotvel != vmd_zero_vector) {
		oo_flags |= OO_POS_AND_ORIENT_NEW;
	} 	// add info for a targeted object
	else if((pl->s_info.target_objnum != -1) && (OBJ_INDEX(obj) == pl->s_info.target_objnum)){
		oo_flags |= OO_POS_AND_ORIENT_NEW;
	}	// add info which is contingent upon being "in front"			
	else if(in_cone){
		oo_flags |= OO_POS_AND_ORIENT_NEW;
	}						
		


	// if its a small ship, add weapon link info
	// Cyborg17 - these don't take any extra space because they are part of the flags variable, so it's ok to send them with every packet.
	if((sip != nullptr) && (sip->is_fighter_bomber())){
		// primary bank 0 or 1
		if(shipp->weapons.current_primary_bank > 0){
			oo_flags |= OO_PRIMARY_BANK;
		}

		// linked or not
		if(shipp->flags[Ship::Ship_Flags::Primary_linked]){
			oo_flags |= OO_PRIMARY_LINKED;
		}

		// trigger down or not
		if(shipp->flags[Ship::Ship_Flags::Trigger_down]){
			oo_flags |= OO_TRIGGER_DOWN;
		}
	}	
		
	// maybe update hull
	if(Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].hull != obj->hull_strength){
		oo_flags |= (OO_HULL_NEW);
		Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].hull = obj->hull_strength;		
	}

	float temp_max = shield_get_max_quad(obj);
	bool all_max = true;
	// Client and server deal with ship death differently, so sending this info for dead ships can cause bugs
	if (!(shipp->is_dying_or_departing() || shipp->flags[Ship::Ship_Flags::Exploded])) {
		// maybe update shields, which are constantly repairing, and should be regularly updated, unless they are already spotless.
		for (auto quadrant : obj->shield_quadrant) {
			if (quadrant != temp_max) {
				all_max = false;
				break;
			}
		}
	}

	if (all_max) {
		// shields are currently perfect, were they perfect last time?
		if ( !Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].perfect_shields_sent){
			// send the newly perfected shields
			oo_flags |= OO_SHIELDS_NEW;
		}
		// make sure to mark it as perfect for next time.
		Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].perfect_shields_sent = true;
	} // if they're not perfect, make sure they're marked as not perfect.
	else {
		Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].perfect_shields_sent = false;
		oo_flags |= OO_SHIELDS_NEW;
	}


	ai_info *aip = &Ai_info[shipp->ai_index];

	// check to see if the AI mode updated
	if ((Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].ai_mode != aip->mode) 
		|| (Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].ai_submode != aip->submode) 
		|| (Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].target_signature != aip->target_signature)) {

		// send, if so.
		oo_flags |= OO_AI_NEW;

		// set new values to check against later.
		Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].ai_mode = aip->mode;
		Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].ai_submode = aip->submode;
		Oo_info.player_frame_info[pl->player_id].last_sent[net_sig_idx].target_signature = aip->target_signature;
	}

	// finally, pack stuff only if we have to 	
	int packed = multi_oo_pack_data(pl, obj, oo_flags, data);	

	// bytes packed
	return packed;
}


// process all other objects for this player
void multi_oo_process_all(net_player *pl)
{
	// if the player has an invalid objnum abort..
	if(pl->m_player->objnum < 0){
		return;
	}

	// these two variables are needed throughout the whole function because of the ADD and BUILD_HEADER macros
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;	

	// build the list of ships to check against
	multi_oo_build_ship_list(pl);

	// build the header
	BUILD_HEADER(OBJECT_UPDATE);		

	// Cyborg17 - And now header shared between ships, to help simplify the sequence and timing logic. This will save Server bandwidth
	ADD_INT(Oo_info.number_of_frames);

	int time_out = Multi_Timing_Info.get_current_time();

	ADD_INT(time_out);

	ubyte stop;
	int add_size;	
	ubyte data_add[MAX_PACKET_SIZE * 2]; // we could have up to two maximum sized packets in the array without it overflowing.

	// do nothing if he has no object targeted, or if he has a weapon targeted
	if((pl->s_info.target_objnum != -1) && (Objects[pl->s_info.target_objnum].type == OBJ_SHIP)){

		// get a pointer to the object
		object *targ_obj = &Objects[pl->s_info.target_objnum];
	
		// run through the maybe_update function
		add_size = multi_oo_maybe_update(pl, targ_obj, data_add);

		// copy in any relevant data
		if(add_size){
			stop = 0xff;			
			multi_rate_add(NET_PLAYER_NUM(pl), "stp", 1);
			ADD_DATA(stop);

			memcpy(data + packet_size, data_add, add_size);
			packet_size += add_size;		
		}
	}
	
	bool packet_sent = false;
	int idx = 0;

	// rely on logical-AND shortcut evaluation to prevent array out-of-bounds read of OO_ship_index[idx]
	while((idx < MAX_SHIPS) && (OO_ship_index[idx] >= 0)){
		// if this guy is over his datarate limit, do nothing
		if(multi_oo_rate_exceeded(pl)){
			nprintf(("Network","Capping client\n"));
			break;
		}			

		// get the object
		object *moveup = &Objects[Ships[OO_ship_index[idx]].objnum];

		// maybe send some info		
		add_size = multi_oo_maybe_update(pl, moveup, data_add);

		// if this data is too much for the packet, send off what we currently have and start over
		if(packet_size + add_size > MAX_PACKET_SIZE - 3){
			stop = 0x00;			
			multi_rate_add(NET_PLAYER_NUM(pl), "stp", 1);
			ADD_DATA(stop);
									
			multi_io_send(pl, data, packet_size);
			packet_sent = true;
			pl->s_info.rate_bytes += packet_size + UDP_HEADER_SIZE;

			packet_size = 0;
			BUILD_HEADER(OBJECT_UPDATE);
			// Cyborg17 - regurgitate shared header
			ADD_INT(Oo_info.number_of_frames);
			ADD_INT(time_out);
		}

		if(add_size){
			stop = 0xff;			
			multi_rate_add(NET_PLAYER_NUM(pl), "stp", 1);
			ADD_DATA(stop);

			// copy in the data
			memcpy(data + packet_size,data_add,add_size);
			packet_size += add_size;
		}

		// next ship
		idx++;
	}

	// Cyborg17 - Now that this is basically an object update and timing update packet, we always should send at least one.
	if (packet_size > OO_MAIN_HEADER_SIZE || !packet_sent) {
		stop = 0x00;		
		multi_rate_add(NET_PLAYER_NUM(pl), "stp", 1);
		ADD_DATA(stop);

		multi_io_send(pl, data, packet_size);
		pl->s_info.rate_bytes += packet_size + UDP_HEADER_SIZE;
	}
}

// process all object update details for this frame
void multi_oo_process()
{
	int idx;	
	
	// process each player
	for(idx=0; idx<MAX_PLAYERS; idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_player != &Net_players[idx]) /*&& !MULTI_OBSERVER(Net_players[idx])*/ ){
			// now process the rest of the objects
			multi_oo_process_all(&Net_players[idx]);

			// do firing stuff for this player
			if((Net_players[idx].m_player != nullptr) && (Net_players[idx].m_player->objnum >= 0) && !(Net_players[idx].flags & NETINFO_FLAG_LIMBO) && !(Net_players[idx].flags & NETINFO_FLAG_RESPAWNING)){
				if((Objects[Net_players[idx].m_player->objnum].flags[Object::Object_Flags::Player_ship]) && !(Objects[Net_players[idx].m_player->objnum].flags[Object::Object_Flags::Should_be_dead])){
					obj_player_fire_stuff( &Objects[Net_players[idx].m_player->objnum], Net_players[idx].m_player->ci );
				}
			}
		}
	}
}

// process incoming object update data
void multi_oo_process_update(ubyte *data, header *hinfo)
{	
	int player_index;	
	int offset = HEADER_LENGTH;
	net_player *pl = nullptr;	

	// determine what player this came from 
	player_index = find_player_index(hinfo->id);
	if(player_index != -1){						
		pl = &Net_players[player_index];
	}
	// otherwise its a "regular" object update packet on a client from the server. Use the server as the reference.
	else {						
		pl = Netgame.server;
	}

	int seq_num;
	int timestamp;
	ubyte stop;	

	// TODO: ADD COMPLICATED TIMESTAMP LOGIC HERE
	GET_INT(seq_num);
	GET_INT(timestamp);
	GET_DATA(stop);
	
	while(stop == 0xff){
		// process the data
		offset += multi_oo_unpack_data(pl, data + offset, seq_num, timestamp);

		GET_DATA(stop);
	}
	PACKET_SET_SIZE();
}

// initialize all object update info (call whenever entering gameplay state)
void multi_init_oo_and_ship_tracker()
{

	if (!(Game_mode & GM_MULTIPLAYER)) {
		return;
	}
	// setup initial object update info	
	// Part 1: Get the non-repeating parts of the struct set.

	Oo_info.rollback_reference_timestamp = 0;
	Oo_info.most_recent_updated_net_signature = 0;
	Oo_info.most_recent_frame = 0;
	Oo_info.number_of_frames = 0;
	Oo_info.cur_frame_index = 0;

	for (int i = 0; i < MAX_FRAMES_RECORDED; i++) { // NOLINT
		Oo_info.timestamps[i] = _timestamp(); // as of Interpolate overhaul 2, this can be something besides infinite
	}

	Oo_info.rollback_mode = false;
	Oo_info.rollback_weapon_object_number.clear();
	Oo_info.rollback_collide_list.clear();
	Oo_info.rollback_ships.clear();

	for (int i = 0; i < MAX_FRAMES_RECORDED; i++) { // NOLINT
		Oo_info.rollback_shots_to_be_fired[i].clear();
		Oo_info.rollback_shots_to_be_fired[i].reserve(20);
	}

	// Part 2: Init/Reset the repeating parts of the struct. 
	Oo_info.frame_info.clear();		
	Oo_info.player_frame_info.clear();

	Oo_info.frame_info.reserve(MAX_SHIPS); // Reserving up to a reasonable number of ships here should help optimize a little bit.
	Oo_info.player_frame_info.reserve(MAX_PLAYERS); // Reserve up to the max players

	rollback_ship_position_records temp_position_records;
	oo_netplayer_records temp_netplayer_records;

	for (int i = 0; i < MAX_FRAMES_RECORDED; i++) {
		temp_position_records.orientations[i] = vmd_identity_matrix;
		temp_position_records.positions[i] = vmd_zero_vector;
		temp_position_records.velocities[i] = vmd_zero_vector;
		temp_position_records.rotational_velocities[i] = vmd_zero_vector;
	}

	temp_position_records.first_pos = vmd_zero_vector;

	int cur = 0;
	oo_info_sent_to_players temp_sent_to_player;

	temp_sent_to_player.timestamp = _timestamp(cur);
	temp_sent_to_player.position = vmd_zero_vector;
	temp_sent_to_player.hull = 0.0f;
	temp_sent_to_player.ai_mode = 0;
	temp_sent_to_player.ai_submode = -1;
	temp_sent_to_player.target_signature = 0;
	temp_sent_to_player.perfect_shields_sent = false;

	// See if *any* of the subsystems changed, so we have to allow for a variable number of subsystems within a variable number of ships.
	temp_sent_to_player.subsystem_health.reserve(MAX_MODEL_SUBSYSTEMS);
	temp_sent_to_player.subsystem_health.push_back(0.0f);

	temp_sent_to_player.subsystem_1b.reserve(MAX_MODEL_SUBSYSTEMS);
	temp_sent_to_player.subsystem_1b.push_back(0.0f);
	
	temp_sent_to_player.subsystem_1h.reserve(MAX_MODEL_SUBSYSTEMS);
	temp_sent_to_player.subsystem_1h.push_back(0.0f);
	
	temp_sent_to_player.subsystem_1p.reserve(MAX_MODEL_SUBSYSTEMS);
	temp_sent_to_player.subsystem_1p.push_back(0.0f);
	
	temp_sent_to_player.subsystem_2b.reserve(MAX_MODEL_SUBSYSTEMS);
	temp_sent_to_player.subsystem_2b.push_back(0.0f);
	
	temp_sent_to_player.subsystem_2h.reserve(MAX_MODEL_SUBSYSTEMS);
	temp_sent_to_player.subsystem_2h.push_back(0.0f);
	
	temp_sent_to_player.subsystem_2p.reserve(MAX_MODEL_SUBSYSTEMS);
	temp_sent_to_player.subsystem_2p.push_back(0.0f);

	temp_netplayer_records.last_sent.push_back(temp_sent_to_player);
	Oo_info.frame_info.push_back(temp_position_records);
	
	for (int i = 0; i < MAX_PLAYERS; i++) {
		Oo_info.player_frame_info.push_back(temp_netplayer_records);
	}

	// Finally init the new timing system.
	Multi_Timing_Info.set_mission_start_time();

	// reset datarate stamp now
	extern int OO_gran;
	for(int i=0; i<MAX_PLAYERS; i++){ // NOLINT
		Net_players[i].s_info.rate_stamp = timestamp( (int)(1000.0f / (float)OO_gran) );
	}
}

// release and reset object update info
void multi_close_oo_and_ship_tracker()
{
	if ( !(Game_mode & GM_MULTIPLAYER) ) {
		return;
	}

	// Part 1: Get the non-repeating parts of the struct set.
	Oo_info.rollback_reference_timestamp = 0;
	Oo_info.most_recent_updated_net_signature = 0;
	Oo_info.most_recent_frame = 0;

	Oo_info.number_of_frames = 0;
	Oo_info.cur_frame_index = 0;

	for (int i = 0; i < MAX_FRAMES_RECORDED; i++) { // NOLINT
		Oo_info.timestamps[i] = _timestamp(); // // as of Interpolate overhaul 2, this can be something besides infinite
	}

	Oo_info.rollback_mode = false;
	Oo_info.rollback_weapon_object_number.clear();
	Oo_info.rollback_weapon_object_number.shrink_to_fit();
	Oo_info.rollback_collide_list.clear();
	Oo_info.rollback_collide_list.shrink_to_fit();
	Oo_info.rollback_ships.clear();
	Oo_info.rollback_ships.shrink_to_fit();

	for (int i = 0; i < MAX_FRAMES_RECORDED; i++) { // NOLINT
		Oo_info.rollback_shots_to_be_fired[i].clear();
		Oo_info.rollback_shots_to_be_fired[i].shrink_to_fit();
	}

	// Part 2: Init/Reset the repeating parts of the struct.
	Oo_info.frame_info.clear();
	Oo_info.frame_info.shrink_to_fit();
	Oo_info.player_frame_info.clear();
	Oo_info.player_frame_info.shrink_to_fit();
}


// send control info for a client (which is basically a "reverse" object update)
void multi_oo_send_control_info()
{
	ubyte data[MAX_PACKET_SIZE], stop;
	ubyte data_add[MAX_PACKET_SIZE];
	ushort oo_flags;	
	int add_size;
	int packet_size = 0;

	// if I'm dying or my object type is not a ship, bail here
	if((Player_obj != nullptr) && (Player_ship->flags[Ship::Ship_Flags::Dying])){
		return;
	}	
	
	// build the header
	BUILD_HEADER(OBJECT_UPDATE);		

	// Cyborg17 - And now the shared header, to help simplify the logic. Will save Server bandwidth
	ADD_INT(Oo_info.number_of_frames);

	// also the timestamp.
	int time_out = Multi_Timing_Info.get_current_time();

	ADD_INT(time_out);

	// pos and orient always
	oo_flags = OO_POS_AND_ORIENT_NEW;		

	// pack the appropriate info into the data
	add_size = multi_oo_pack_data(Net_player, Player_obj, oo_flags, data_add);

	// copy in any relevant data
	if(add_size){
		stop = 0xff;		
		multi_rate_add(NET_PLAYER_NUM(Net_player), "stp", 1);
		
		ADD_DATA(stop);

		memcpy(data + packet_size, data_add, add_size);
		packet_size += add_size;		
	}

	// add the final stop byte
	stop = 0x0;	
	multi_rate_add(NET_PLAYER_NUM(Net_player), "stp", 1);
	ADD_DATA(stop);

	// send to the server
	if(Netgame.server != nullptr){								
		multi_io_send(Net_player, data, packet_size);
	}
}

// Sends a packet from the server to the client, syncing the player's position/orientation to the
// Server's. Allows for use of certain SEXPs in multiplayer.
void multi_oo_send_changed_object(object *changedobj)
{
	ubyte data[MAX_PACKET_SIZE], stop;
	ubyte data_add[MAX_PACKET_SIZE];
	ushort oo_flags;	
	int add_size;
	int packet_size = 0;
	int idx = 0;
#ifndef NDEBUG
	nprintf(("Network","Attempting to affect player object.\n"));
#endif
	for (; idx < MAX_PLAYERS; idx++)
	{
		if( changedobj == &(Objects[Net_players[idx].m_player->objnum]) ) {
			break;
		}
	}
#ifndef NDEBUG
	nprintf(("Network","Index for changed object found: [%d].\n",idx));
#endif
	if( idx >= MAX_PLAYERS ) {
		return;
	}
	// build the header
	BUILD_HEADER(OBJECT_UPDATE);

	// Cyborg17 - And now the shared header, to help simplify the logic. Will save Server bandwidth
	ADD_INT(Oo_info.number_of_frames);

	int time_out = Multi_Timing_Info.get_current_time();

	ADD_INT(time_out);

	// pos and orient always
	oo_flags = (OO_POS_AND_ORIENT_NEW);

	// pack the appropriate info into the data
	add_size = multi_oo_pack_data(&Net_players[idx], changedobj, oo_flags, data_add);

	// copy in any relevant data
	if(add_size){
		stop = 0xff;		
		multi_rate_add(idx, "stp", 1);
		
		ADD_DATA(stop);

		memcpy(data + packet_size, data_add, add_size);
		packet_size += add_size;		
	}

	// add the final stop byte
	stop = 0x0;	
	multi_rate_add(idx, "stp", 1);
	ADD_DATA(stop);

	multi_io_send(&Net_players[idx], data, packet_size);
}


// ---------------------------------------------------------------------------------------------------
// DATARATE Defines and Functions
//

// low object update datarate limit
#define OO_LIMIT_LOW				1800
#define OO_LIMIT_MED				3400
#define OO_LIMIT_HIGH				100000000

// timestamp for sending control info (movement only - we'll send button info all the time)
#define OO_CIRATE					85					// 15x a second
int Multi_cirate_stamp			= -1;				// timestamp for waiting on control info time
int Multi_cirate_can_send		= 1;				// if we can send control info this frame

// global max rates
int OO_server_rate = -1;							// max _total_ bandwidth to send to all clients
int OO_client_rate = -1;							// max bandwidth to go to an individual client

// update timestamp for server datarate checking
#define RATE_UPDATE_TIME		1250				// in ms
int OO_server_rate_stamp = -1;

// bandwidth granularity
int OO_gran = 1;
DCF(oog, "Sets bandwidth granularity (Multiplayer)")
{
	if (dc_optional_string_either("help", "--help")) {
		dc_printf("Usage: oog <OO_gran>\n");
		dc_printf("Sets bandwidth granularity\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Current Granularity is '%i' (default is 1)", OO_gran);
		return;
	}

	dc_stuff_int(&OO_gran);
	dc_printf("Ganularity set to %i", OO_gran);
}

// process datarate limiting stuff for the server
void multi_oo_server_process();

// process datarate limiting stuff for the client
void multi_oo_client_process();

// update the server datarate
void multi_oo_update_server_rate();

// process all object update datarate details
void multi_oo_rate_process()
{
	// if I have no valid player, drop out here
	if(Net_player == nullptr){
		return;
	}

	// if we're not in mission, don't do anything
	if(!(Game_mode & GM_IN_MISSION)){
		return;
	}

	// if I'm the server of a game, process server stuff
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_oo_server_process();
	}
	// otherwise process client-side stuff
	else {
		multi_oo_client_process();
	}
}

// process datarate limiting stuff for the server
void multi_oo_server_process()
{
	int idx;
	
	// go through all players
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_SERVER(Net_players[idx])){
			// if his timestamp is -1 or has expired, reset it and zero his rate byte count
			if((Net_players[idx].s_info.rate_stamp == -1) || timestamp_elapsed_safe(Net_players[idx].s_info.rate_stamp, OO_MAX_TIMESTAMP) || (abs(timestamp() - Net_players[idx].s_info.rate_stamp) >= (int)(1000.0f / (float)OO_gran)) ){
				Net_players[idx].s_info.rate_stamp = timestamp( (int)(1000.0f / (float)OO_gran) );
				Net_players[idx].s_info.rate_bytes = 0;
			}
		}
	}

	// determine if we should be updating the server datarate
	if((OO_server_rate_stamp == -1) || timestamp_elapsed_safe(OO_server_rate_stamp, OO_MAX_TIMESTAMP)){
		// reset the timestamp
		OO_server_rate_stamp = timestamp(RATE_UPDATE_TIME);

		// update the server datarate
		multi_oo_update_server_rate();

		// nprintf(("Network","UPDATING SERVER DATARATE\n"));
	}	
}

// process datarate limiting stuff for the client
void multi_oo_client_process()
{
	// if the timestamp is -1 or has elapsed, reset it
	if((Multi_cirate_stamp == -1) || timestamp_elapsed_safe(Multi_cirate_stamp, OO_CIRATE)){
		Multi_cirate_can_send = 1;
		Multi_cirate_stamp = timestamp(OO_CIRATE);
	}	
}


// datarate limiting system for server -------------------------------------

// initialize the rate limiting system for all players
void multi_oo_rate_init_all()
{
	int idx;

	// if I don't have a net_player, bail here
	if(Net_player == nullptr){
		return;
	}

	// if I'm the server of the game
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){	
		// go through all players
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx])){
				multi_oo_rate_init(&Net_players[idx]);
			}
		}

		OO_server_rate_stamp = -1;
	}
	// if i'm the client, initialize my control info datarate stuff
	else {
		Multi_cirate_stamp = -1;
		Multi_cirate_can_send = 1;
	}
}

// initialize the rate limiting for the passed in player
void multi_oo_rate_init(net_player *pl)
{
	// reinitialize his datarate timestamp
	pl->s_info.rate_stamp = -1;
	pl->s_info.rate_bytes = 0;
}

// if the given net-player has exceeded his datarate limit
int multi_oo_rate_exceeded(net_player *pl)
{
	int rate_compare;
		
	// check against the guy's object update level
	switch(pl->p_info.options.obj_update_level){
	// low update level
	case OBJ_UPDATE_LOW:
		// the low object update limit
		rate_compare = OO_LIMIT_LOW;
		break;

	// medium update level
	case OBJ_UPDATE_MEDIUM:		
		// the low object update limit
		rate_compare = OO_LIMIT_MED;
		break;

	// high update level - super high datarate (no capping, just intelligent updating)
	case OBJ_UPDATE_HIGH:
		rate_compare = OO_LIMIT_HIGH;
		break;

	// LAN - no rate max
	case OBJ_UPDATE_LAN:
		return 0;

	// default level
	default:
		UNREACHABLE("Unknown Object Update level in multi_oo_rate_exceeded of %d", pl->p_info.options.obj_update_level);
		rate_compare = OO_LIMIT_LOW;
		break;
	}

	// if the server global rate PER CLIENT (OO_client_rate) is actually lower
	if(OO_client_rate < rate_compare){
		rate_compare = OO_client_rate;
	}

	// compare his bytes sent against the allowable amount
	if(pl->s_info.rate_bytes >= rate_compare){
		return 1;
	}

	// we're allowed to send
	return 0;
}

// if it is ok for me to send a control info (will be ~N times a second)
int multi_oo_cirate_can_send()
{
	// if we're allowed to send
	if(Multi_cirate_can_send){
		Multi_cirate_can_send = 0;
		return 1;
	} 
	
	return 0;		
}

// dynamically update the server capped bandwidth rate
void multi_oo_update_server_rate()
{	
	int num_connections;	
	
	// bail conditions
	if((Net_player == nullptr) || !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		return;
	}

	// get the # of connections
	num_connections = multi_num_connections();
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		num_connections--;
	}
	// make sure we always pretend there's at least one guy available
	if(num_connections <= 0){
		num_connections = 1;
	}
		
	// set the data rate	
	switch(Net_player->p_info.options.obj_update_level){
	// LAN update level
	case OBJ_UPDATE_LAN:
		// set to something super big so we don't limit anything
		OO_server_rate = 500000000;
		break;

	// high update level
	case OBJ_UPDATE_HIGH:
		// set to 0 so we don't limit anything
		OO_server_rate = Multi_options_g.datarate_cap;
		break;

	// medium update level
	case OBJ_UPDATE_MEDIUM:
		// set the rate to be "medium" update level
		OO_server_rate = OO_LIMIT_MED;
		break;

	// low update level 
	case OBJ_UPDATE_LOW:
		// set the rate to be the "low" update level
		OO_server_rate = OO_LIMIT_LOW;
		break;

	default:
		UNREACHABLE("Unknown Object Update level in multi_oo_update_server_rate of %d", Net_player->p_info.options.obj_update_level);
		return;
	}	

	// set the individual client level
	OO_client_rate = (int)(((float)OO_server_rate / (float)OO_gran) / (float)num_connections);
}

// is this object one which needs to go through the interpolation
bool multi_oo_is_interp_object(object *objp)
{	
	// if not multiplayer, skip it
	if(!(Game_mode & GM_MULTIPLAYER)){
		return false;
	}

	// if its not a ship, skip it
	if(objp->type != OBJ_SHIP){
		return false;
	}

	// other bogus cases
	if((objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return false;
	}

	// if I'm a client and this is not me, I need to interp it
	if(!MULTIPLAYER_MASTER){
		if(objp != Player_obj){
			return true;
		} else {
			return false;
		}
	}

	// servers only interpolate other player ships
	if(!(objp->flags[Object::Object_Flags::Player_ship])){
		return false;
	}

	// here we know its a player ship - is it mine?
	if(objp == Player_obj){
		return false;
	}

	// interp it
	return true;
}

