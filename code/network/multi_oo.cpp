/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "network/multi_oo.h"
#include "freespace2/freespace.h"
#include "io/timer.h"
#include "globalincs/linklist.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multi.h"
#include "network/multi_options.h"
#include "ship/ship.h"
#include "object/object.h"
#include "playerman/player.h"
#include "io/key.h"



#ifndef OO_NEW

// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE DEFINES/VARS
//

// how much data we're willing to put into a given oo packet
#define OO_MAX_SIZE					480

// new improved - more compacted info type
#define OO_POS_NEW					(1<<0)		// 
#define OO_ORIENT_NEW				(1<<1)		// 
#define OO_HULL_NEW					(1<<2)		// 
#define OO_SHIELD_NEW				(1<<3)		// 
#define OO_AI_MODE_NEW				(1<<4)		// 
#define OO_SUBSYSTEMS_NEW			(1<<5)		// 
#define OO_EXTRA_PHYSICS			(1<<6)		// means pos update will have desired velocity and orient update will have desired rotvel as well

#define OO_VIEW_CONE_DOT			(0.1f)
#define OO_VIEW_DIFF_TOL			(0.15f)			// if the dotproducts differ this far between frames, he's coming into view

// distance class
#define OO_NEAR						0
#define OO_NEAR_DIST					(200.0f)
#define OO_MIDRANGE					1
#define OO_MIDRANGE_DIST			(600.0f)
#define OO_FAR							2
#define OO_FAR_DIST					(1200.0f)

// how often we should send full hull/shield updates
#define OO_HULL_SHIELD_TIME		600
#define OO_SUBSYS_TIME				1000

// timestamp values for object update times based on client's update level.
int Multi_oo_target_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	150,				// low update -- do every 1 second
	125,				// medium update -- do every 1/2 second
	100,				// high update -- every frame
	30,
};

// for near ships
int Multi_oo_front_near_update_times[MAX_OBJ_UPDATE_LEVELS] =
{
	250,				// low update
	150,				// medium update
	100,				// high update
	30,
};

// for medium ships
int Multi_oo_front_medium_update_times[MAX_OBJ_UPDATE_LEVELS] =
{
	400,				// low update
	300,				// medium update
	150,				// high update
	30,
};

// for far ships
int Multi_oo_front_far_update_times[MAX_OBJ_UPDATE_LEVELS] =
{
	1100,				// low update
	500,				// medium update
	300,				// high update
	30,
};

// for near ships
int Multi_oo_rear_near_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	1500,				// low update
	1000,				// medium update
	300,				// high update
	30,
};

// for medium ships
int Multi_oo_rear_medium_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	1900,				// low update
	1200,				// medium update
	400,				// high update
	30,
};

// for far ships
int Multi_oo_rear_far_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	3000,				// low update
	1600,				// medium update
	500,				// high update
	30,
};

// let's keep track of the breakdown of individual object update elements
int OO_pos_total = 0;
int OO_vel_total = 0;
int OO_desired_vel_total = 0;
int OO_orient_total = 0;
int OO_rotvel_total = 0;
int OO_desired_rotvel_total = 0;
int OO_hull_total = 0;
int OO_shield_total = 0;
int OO_single_subsys_total = 0;
int OO_subsys_total = 0;
int OO_forward_thrust_total = 0;

// keys for selectively disabling interpolation - these keys can be used in combination with each other
#define LAG_OFF_KEY							KEY_7
#define LEVEL_1_OFF_KEY						KEY_8
#define LEVEL_2_OFF_KEY						KEY_9

int OO_global_time;

// ship index list for possibly sorting ships based upon distance, etc
short OO_ship_index[MAX_SHIPS];

int OO_debug_info = 0 ;
DCF(ood, "switch on and off targeted object update info")
{
	OO_debug_info = !OO_debug_info;
}

// object update stats storing stuff
#define OO_POS_NEW					(1<<0)		// 
#define OO_ORIENT_NEW				(1<<1)		// 
#define OO_HULL_NEW					(1<<2)		// 
#define OO_SHIELD_NEW				(1<<3)		// 
#define OO_AI_MODE_NEW				(1<<4)		// 
#define OO_SUBSYSTEMS_NEW			(1<<5)		// 
#define OO_EXTRA_PHYSICS			(1<<6)		// means pos update will have desired velocity and orient update will have desired rotvel as well

#define NUM_UPDATE_RECORDS			5						// last five second average
typedef struct np_update_record {	
	int pos_bytes;												// how many pos bytes we've sent
	int orient_bytes;											// how many orient bytes we've sent
	int hull_bytes;											// how many hull bytes we've sent
	int shield_bytes;											// how many shield bytes we've sent
	int ai_mode_bytes;										// how many ai_mode bytes we've sent
	int extra_physics_bytes;								// how many bytes of extra physics info we've sent	
	int fthrust_bytes;										// how many bytes of forward thrust we've sent	
	int subsys_bytes;											// how many bytes of subsys data we've sent
	
	int pos_stamp;
	int pos_bytes_frame;										// how many bytes we've sent in the last second
	int pos_records[NUM_UPDATE_RECORDS];				//	average of the last bunch of bytes we've sent
	int pos_record_count;
	float pos_avg;

	int orient_stamp;
	int orient_bytes_frame;									// how many bytes we've sent in the last second
	int orient_records[NUM_UPDATE_RECORDS];			//	average of the last bunch of bytes we've sent
	int orient_record_count;
	float orient_avg;

	int hull_stamp;
	int hull_bytes_frame;									// how many bytes we've sent in the last second
	int hull_records[NUM_UPDATE_RECORDS];				//	average of the last bunch of bytes we've sent
	int hull_record_count;
	float hull_avg;

	int shield_stamp;
	int shield_bytes_frame;									// how many bytes we've sent in the last second
	int shield_records[NUM_UPDATE_RECORDS];			//	average of the last bunch of bytes we've sent
	int shield_record_count;
	float shield_avg;

	int ai_mode_stamp;
	int ai_mode_bytes_frame;								// how many bytes we've sent in the last second
	int ai_mode_records[NUM_UPDATE_RECORDS];			//	average of the last bunch of bytes we've sent
	int ai_mode_record_count;
	float ai_mode_avg;

	int extra_physics_stamp;
	int extra_physics_bytes_frame;						// how many bytes we've sent in the last second
	int extra_physics_records[NUM_UPDATE_RECORDS];	//	average of the last bunch of bytes we've sent
	int extra_physics_record_count;
	float extra_physics_avg;

	int fthrust_stamp;
	int fthrust_bytes_frame;								//	how many bytes we've sent in the last second
	int fthrust_records[NUM_UPDATE_RECORDS];			//	average of the last bunch of bytes we've sent
	int fthrust_record_count;
	float fthrust_avg;

	int subsys_stamp;
	int subsys_bytes_frame;									// how many bytes we've sent in the last second
	int subsys_records[NUM_UPDATE_RECORDS];			//	average of the last bunch of bytes we've sent
	int subsys_record_count;
	float subsys_avg;
} np_update_record;

#ifndef NDEBUG
#define R_AVG(ct, ar, avg)							do { int av_idx; float av_sum = 0.0f; if(ct == 0){avg = 0;} else { for(av_idx=0; av_idx<ct; av_idx++){ av_sum += (float)ar[av_idx]; } avg = av_sum / (float)ct;} } while(0);
#define R_POS_ADD(net_plr, byte_count)			do { if((net_plr == NULL) || (!MULTIPLAYER_MASTER)){break;}	np_update_record *r = &OO_update_records[NET_PLAYER_INDEX(net_plr)]; r->pos_bytes += byte_count; r->pos_bytes_frame += byte_count; if(time(NULL) > r->pos_stamp){ if(r->pos_record_count >= NUM_UPDATE_RECORDS) {	memmove(r->pos_records, r->pos_records+1, sizeof(int) * (NUM_UPDATE_RECORDS - 1)); r->pos_records[NUM_UPDATE_RECORDS-1] = r->pos_bytes_frame; } else {r->pos_records[r->pos_record_count++] = r->pos_bytes_frame; } r->pos_stamp = time(NULL); r->pos_bytes_frame = 0; R_AVG(r->pos_record_count, r->pos_records, r->pos_avg); } } while(0);
#define R_ORIENT_ADD(net_plr, byte_count)		do { if((net_plr == NULL) || (!MULTIPLAYER_MASTER)){break;} np_update_record *r = &OO_update_records[NET_PLAYER_INDEX(net_plr)]; r->orient_bytes += byte_count; r->orient_bytes_frame += byte_count; if(time(NULL) > r->orient_stamp){ if(r->orient_record_count >= NUM_UPDATE_RECORDS) {	memmove(r->orient_records, r->orient_records+1, sizeof(int) * (NUM_UPDATE_RECORDS - 1)); r->orient_records[NUM_UPDATE_RECORDS-1] = r->orient_bytes_frame;} else { r->orient_records[r->orient_record_count++] = r->orient_bytes_frame; } r->orient_stamp = time(NULL); r->orient_bytes_frame = 0; R_AVG(r->orient_record_count, r->orient_records, r->orient_avg); } } while(0);
#define R_HULL_ADD(net_plr, byte_count)		do { if((net_plr == NULL) || (!MULTIPLAYER_MASTER)){break;} np_update_record *r = &OO_update_records[NET_PLAYER_INDEX(net_plr)]; r->hull_bytes += byte_count; r->hull_bytes_frame += byte_count; if(time(NULL) > r->hull_stamp){ if(r->hull_record_count >= NUM_UPDATE_RECORDS) {	memmove(r->hull_records, r->hull_records+1, sizeof(int) * (NUM_UPDATE_RECORDS - 1)); r->hull_records[NUM_UPDATE_RECORDS-1] = r->hull_bytes_frame;} else { r->hull_records[r->hull_record_count++] = r->hull_bytes_frame; } r->hull_stamp = time(NULL); r->hull_bytes_frame = 0; R_AVG(r->hull_record_count, r->hull_records, r->hull_avg); } } while(0);
#define R_SHIELD_ADD(net_plr, byte_count)		do { if((net_plr == NULL) || (!MULTIPLAYER_MASTER)){break;} np_update_record *r = &OO_update_records[NET_PLAYER_INDEX(net_plr)]; r->shield_bytes += byte_count; r->shield_bytes_frame += byte_count; if(time(NULL) > r->shield_stamp){ if(r->shield_record_count >= NUM_UPDATE_RECORDS) {	memmove(r->shield_records, r->shield_records+1, sizeof(int) * (NUM_UPDATE_RECORDS - 1)); r->shield_records[NUM_UPDATE_RECORDS-1] = r->shield_bytes_frame; } else { r->shield_records[r->shield_record_count++] = r->shield_bytes_frame; } r->shield_stamp = time(NULL); r->shield_bytes_frame = 0; R_AVG(r->shield_record_count, r->shield_records, r->shield_avg); } } while(0);
#define R_AI_MODE_ADD(net_plr, byte_count)		do { if((net_plr == NULL) || (!MULTIPLAYER_MASTER)){break;} np_update_record *r = &OO_update_records[NET_PLAYER_INDEX(net_plr)]; r->ai_mode_bytes += byte_count; r->ai_mode_bytes_frame += byte_count; if(time(NULL) > r->ai_mode_stamp){ if(r->ai_mode_record_count >= NUM_UPDATE_RECORDS) {	memmove(r->ai_mode_records, r->ai_mode_records+1, sizeof(int) * (NUM_UPDATE_RECORDS - 1)); r->ai_mode_records[NUM_UPDATE_RECORDS-1] = r->ai_mode_bytes_frame;} else { r->ai_mode_records[r->ai_mode_record_count++] = r->ai_mode_bytes_frame; } r->ai_mode_stamp = time(NULL); r->ai_mode_bytes_frame = 0; R_AVG(r->ai_mode_record_count, r->ai_mode_records, r->ai_mode_avg); } } while(0);
#define R_EXTRA_PHYSICS_ADD(net_plr, byte_count)		do { if((net_plr == NULL) || (!MULTIPLAYER_MASTER)){break;} np_update_record *r = &OO_update_records[NET_PLAYER_INDEX(net_plr)]; r->extra_physics_bytes += byte_count; r->extra_physics_bytes_frame += byte_count; if(time(NULL) > r->extra_physics_stamp){ if(r->extra_physics_record_count >= NUM_UPDATE_RECORDS) {	memmove(r->extra_physics_records, r->extra_physics_records+1, sizeof(int) * (NUM_UPDATE_RECORDS - 1)); r->extra_physics_records[NUM_UPDATE_RECORDS-1] = r->extra_physics_bytes_frame;} else { r->extra_physics_records[r->extra_physics_record_count++] = r->extra_physics_bytes_frame; } r->extra_physics_stamp = time(NULL); r->extra_physics_bytes_frame = 0; R_AVG(r->extra_physics_record_count, r->extra_physics_records, r->extra_physics_avg); } } while(0);
#define R_FTHRUST_ADD(net_plr, byte_count)		do { if((net_plr == NULL) || (!MULTIPLAYER_MASTER)){break;} np_update_record *r = &OO_update_records[NET_PLAYER_INDEX(net_plr)]; r->fthrust_bytes += byte_count; r->fthrust_bytes_frame += byte_count; if(time(NULL) > r->fthrust_stamp){ if(r->fthrust_record_count >= NUM_UPDATE_RECORDS) {	memmove(r->fthrust_records, r->fthrust_records+1, sizeof(int) * (NUM_UPDATE_RECORDS - 1)); r->fthrust_records[NUM_UPDATE_RECORDS-1] = r->fthrust_bytes_frame; } else { r->fthrust_records[r->fthrust_record_count++] = r->fthrust_bytes_frame; } r->fthrust_stamp = time(NULL); r->fthrust_bytes_frame = 0; R_AVG(r->fthrust_record_count, r->fthrust_records, r->fthrust_avg); } } while(0);
#define R_SUBSYS_ADD(net_plr, byte_count)		do { if((net_plr == NULL) || (!MULTIPLAYER_MASTER)){break;} np_update_record *r = &OO_update_records[NET_PLAYER_INDEX(net_plr)]; r->subsys_bytes += byte_count; r->subsys_bytes_frame += byte_count; if(time(NULL) > r->subsys_stamp){ if(r->subsys_record_count >= NUM_UPDATE_RECORDS) {	memmove(r->subsys_records, r->subsys_records+1, sizeof(int) * (NUM_UPDATE_RECORDS - 1)); r->subsys_records[NUM_UPDATE_RECORDS-1] = r->subsys_bytes_frame; } else { r->subsys_records[r->subsys_record_count++] = r->subsys_bytes_frame; } r->subsys_stamp = time(NULL); r->subsys_bytes_frame = 0; R_AVG(r->subsys_record_count, r->subsys_records, r->subsys_avg); } } while(0);
np_update_record OO_update_records[MAX_PLAYERS];
#else 
#define R_POS_ADD(net_plr, byte_count)		
#define R_ORIENT_ADD(net_plr, byte_count)		
#define R_HULL_ADD(net_plr, byte_count)		
#define R_SHIELD_ADD(net_plr, byte_count)		
#define R_AI_MODE_ADD(net_plr, byte_count)		
#define R_EXTRA_PHYSICS_ADD(net_plr, byte_count)
#define R_FTHRUST_ADD(net_plr, byte_count)
#define R_SUBSYS_ADD(net_plr, byte_count)
#endif
int OO_update_index = -1;						// index into OO_update_records for displaying update record info

// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE FUNCTIONS
//

object *OO_player_obj;
int OO_sort = 1;

int multi_oo_sort_func(const void *ship1, const void *ship2)
{
	object *obj1, *obj2;
	short index1, index2;
	float dist1, dist2;

	// get the 2 indices
	memcpy(&index1, ship1, sizeof(short));
	memcpy(&index2, ship2, sizeof(short));

	// if the indices are bogus, or the objnums are bogus, return ">"
	if((index1 < 0) || (index2 < 0) || (Ships[index1].objnum < 0) || (Ships[index2].objnum < 0)){
		return 1;
	}

	// get the 2 objects
	obj1 = &Objects[Ships[index1].objnum];
	obj2 = &Objects[Ships[index2].objnum];

	// get the distance to the player obj for both
	dist1 = vm_vec_dist_quick(&OO_player_obj->pos, &obj1->pos);
	dist2 = vm_vec_dist_quick(&OO_player_obj->pos, &obj2->pos);

	return (dist1 <= dist2) ? -1 : 1;
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
	if(pl->player->objnum < 0){
		return;
	}
	player_obj = &Objects[pl->player->objnum];
	
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
		if (Objects[moveup->objnum].flags & OF_SHOULD_BE_DEAD){
			continue;
		}

		// don't send info for dying ships
		if (Ships[Objects[moveup->objnum].instance].flags & SF_DYING){
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

	// maybe qsort the thing here
	OO_player_obj = player_obj;
	if(OO_sort){
		qsort(OO_ship_index, ship_index, sizeof(short), multi_oo_sort_func);
	}
}

// set global object update timestamp for this frame
void multi_oo_set_global_timestamp()
{
	OO_global_time = timer_get_milliseconds();
}


int Interpolated_orient_inited[MAX_SHIPS];
float Interpolate_dot[MAX_SHIPS];
matrix Interpolated_orient[MAX_SHIPS];

void multi_oo_interpolate_init()
{
	int i;

	for (i=0; i<MAX_SHIPS;i++ )	{
		Interpolated_orient_inited[i] = 0;
	}
}

/*
void matrix_to_quaternion( D3DRMQUATERNION *quat, matrix *mat )
{
	float theta;
	vec3d rot_axis;
	
	vm_matrix_to_rot_axis_and_angle(mat, &theta, &rot_axis);

	quat->v.x = rot_axis.x;
	quat->v.y = rot_axis.y;
	quat->v.z = rot_axis.z;
	quat->s = theta;
}

void quaternion_to_matrix( matrix *mat, D3DRMQUATERNION *quat )
{
	float theta;
	vec3d rot_axis;

	rot_axis.x = quat->v.x;
	rot_axis.y = quat->v.y;
	rot_axis.z = quat->v.z;
	theta = quat->s;

	vm_quaternion_rotate(mat, theta, &rot_axis);
	vm_orthogonalize_matrix(mat);
}
*/

// interpolate for this object
void multi_oo_interpolate(object *objp, interp_info *current, interp_info *last)
{
#ifdef OO_NEW	
#else 
	player *pp;
	int pnum;

	// save position and orientation
	objp->last_pos = objp->pos;
	objp->last_orient = objp->orient;	

	// move all pre ?
	obj_move_all_pre(objp, flFrametime);

	// level 1 interpolation
	// if the key disabling level 1 interpolation is pressed, skip
	if(!keyd_pressed[LEVEL_1_OFF_KEY]){
		float lag = fl2i(current->lowest_ping)/1000.0f;

		lag /= 2.0f;		// Our ping time is round trip, we only account for 1/2 the trip.

		// Correct bogus lags
		if ( lag > 0.5f )	{
			lag = 0.5f;
		}

		// test - should behave like the old way of doing stuff		
		if (current->vel_time == OO_global_time){
			objp->phys_info.vel = current->vel;
		}
		if (current->rotvel_time == OO_global_time){
			objp->phys_info.rotvel = current->rotvel;
		}

		objp->phys_info.desired_vel = current->desired_vel;
		objp->phys_info.desired_rotvel = current->desired_rotvel;		

//		if ( !stricmp( Ships[objp->instance].ship_name, "alpha 1"))	{
//			mprintf(( "Rotvel = %.3f, %.3f, %.3f\n", current->rotvel.x, current->rotvel.y, current->rotvel.z ));
//		}

		if ( !Interpolated_orient_inited[objp->instance] )	{
			Interpolated_orient[objp->instance] = objp->orient;
			Interpolated_orient_inited[objp->instance] = 1;
		}

		matrix *actual_orient = &Interpolated_orient[objp->instance];
		
		// if this is from a network update
		if(current->orient_time == OO_global_time){
			*actual_orient = current->orient;	
			if ( lag > 0.0f )	{
				//adjust for lag
				physics_sim_rot(actual_orient, &objp->phys_info, lag );
			} else {
				physics_sim_rot(actual_orient, &objp->phys_info, flFrametime );
			}
		}
		// otherwise, if between network updates
		else {
			physics_sim_rot(actual_orient, &objp->phys_info, flFrametime );
		}
		
		if(keyd_pressed[LAG_OFF_KEY]){
			// Make orient go quickly to actual_orient

			/*
			matrix tmp;
			vec3d w_out;
			vec3d angular_accel;

			angular_accel.x  = 1000.0f;
			angular_accel.y  = 1000.0f;
			angular_accel.z  = 1000.0f;
			vm_matrix_interpolate(actual_orient, &objp->orient, &objp->phys_info.rotvel, flFrametime, &tmp,
									&w_out, &objp->phys_info.max_rotvel, &angular_accel, 1 );

			objp->phys_info.rotvel = w_out;	
			objp->orient = tmp; 
			*/

			// 
			/*
			D3DRMQUATERNION in1, in2, out;

			matrix_to_quaternion( &in1, &objp->orient );
			matrix_to_quaternion( &in2, actual_orient );

			D3DRMQuaternionSlerp( &out, &in1, &in2, 0.50f );

			quaternion_to_matrix( &objp->orient, &out );
			*/
			objp->orient = *actual_orient;
		} else {
			objp->orient = *actual_orient;
		}		
		
		if(current->pos_time == OO_global_time){
			objp->pos = current->pos;
			if ( lag > 0.0f )	{
				//adjust for lag
				physics_sim_vel(&objp->pos, &objp->phys_info, lag, &objp->orient );
			} else {
				physics_sim_vel(&objp->pos, &objp->phys_info, flFrametime, &objp->orient );
			}
		} else {
			physics_sim_vel(&objp->pos, &objp->phys_info, flFrametime, &objp->orient );
		}


		objp->phys_info.speed = vm_vec_mag(&objp->phys_info.vel);							//	Note, cannot use quick version, causes cumulative error, increasing speed.
		objp->phys_info.fspeed = vm_vec_dot(&objp->orient.fvec, &objp->phys_info.vel);		// instead of vector magnitude -- use only forward vector since we are only interested in forward velocity
	}
	// no interpolation, so bash
	else {
		objp->orient = current->orient;	
		objp->pos = current->pos;
	}

	// copy the "current info" to the "last" info
	memcpy(last, current, sizeof(interp_info));

	// move all post ?
	obj_move_all_post(objp, flFrametime);

	// if I'm the server of the game, do firing stuff here
	if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (objp->flags & OF_PLAYER_SHIP)){
		pnum = multi_find_player_by_object( objp );
		if ( pnum != -1 ) {
			pp = Net_players[pnum].player;
			obj_player_fire_stuff( objp, pp->ci );
		}
	}
#endif
}

// do all interpolation for this frame
void multi_oo_interpolate_all()
{
	object *objp;
	ship *shipp;
	ship_obj *so;		
	
	// for the server, this means "interpolate all player ships"
	// for the client, this means "interpolate all ships"

	// iterate over all _ships_
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		// get the ships object
		objp = &Objects[so->objnum];
		shipp = &Ships[objp->instance];
	
		// ignore dead or dying objects
		if ( !(objp->flags&OF_SHOULD_BE_DEAD) )	{											
			// multiplayer servers should only do the interpolation for player ships			
			if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
				if((objp->type != OBJ_SHIP) || !(objp->flags & OF_PLAYER_SHIP) || (objp == Player_obj)){
					continue;
				}
			} else {
				if((objp->type != OBJ_SHIP) || (objp == Player_obj)){
					continue;
				}
			}						

			// INTERPOLATE HERE
			multi_oo_interpolate(objp, &shipp->int_cur, &shipp->int_last);									
		}					
	}
}

// handle incoming new data for the given object
void multi_oo_handle_new_data(ship *shipp, object *obj_data, ubyte oo_flags, net_player *pl)
{		
	// fill in the int_current value for the passed ship
	
	// position type update
	if(oo_flags & OO_POS_NEW){
		// copy in position for now
		shipp->int_cur.pos = obj_data->pos;
		shipp->int_cur.pos_time = OO_global_time;
		
		// copy in velocity
		shipp->int_cur.vel = obj_data->phys_info.vel;
		shipp->int_cur.vel_time = OO_global_time;

/*
		if(oo_flags & OO_EXTRA_PHYSICS){
			// copy in desired velocity
			shipp->int_cur.desired_vel = obj_data->phys_info.desired_vel;
			shipp->int_cur.desired_vel_time = OO_global_time;
		} else {
		*/
			shipp->int_cur.desired_vel = obj_data->phys_info.vel;
			shipp->int_cur.desired_vel_time = OO_global_time;
		// }
	}

	// orientation type update
	if(oo_flags & OO_ORIENT_NEW){
		// copy in orientation for now
		shipp->int_cur.orient = obj_data->orient;
		shipp->int_cur.orient_time = OO_global_time;

		// copy in velocity
		shipp->int_cur.rotvel = obj_data->phys_info.rotvel;
		shipp->int_cur.rotvel_time = OO_global_time;

		if(oo_flags & OO_EXTRA_PHYSICS){
			// copy in desired velocity
			shipp->int_cur.desired_rotvel = obj_data->phys_info.desired_rotvel;
			shipp->int_cur.desired_rotvel_time = OO_global_time;
		} else {
			shipp->int_cur.desired_rotvel = obj_data->phys_info.rotvel;
			shipp->int_cur.desired_rotvel_time = OO_global_time;
		}
	}

	// set the ping values
	shipp->int_cur.lowest_ping = multi_ping_get_lowest(&pl->s_info.ping);
	shipp->int_cur.lowest_ping_avg = multi_ping_lowest_avg(&pl->s_info.ping);
}

// increment the packet sequence # (used for object updates from the server, as well as "control info"
// update from clients
void multi_oo_increment_seq()
{
	// increment the sequencing reference count for this frame if necessary
	if(Netgame.server_update_frame_ref != Framecount){
		Netgame.server_update_frame_ref = Framecount;

		if(Netgame.server_update_seq == 0xffff){
			Netgame.server_update_seq = 0;
		} else {
			Netgame.server_update_seq++;
		}
	}
}

// pack information for a client (myself), return bytes added
int multi_oo_pack_client_data(ubyte *data)
{
	ubyte out_flags,ret;
	ushort tnet_signature;
	char t_subsys, l_subsys;
	int packet_size = 0;

	// get our firing stuff
	out_flags = Net_player->s_info.accum_buttons;	

	// zero these values for now
	Net_player->s_info.accum_buttons = 0;

	// add any necessary targeting flags
	if ( Player_ai->current_target_is_locked ){
		out_flags |= OOC_TARGET_LOCKED;
	}
	if ( Player_ai->ai_flags & AIF_SEEK_LOCK ){	
		out_flags |= OOC_TARGET_SEEK_LOCK;
	}
	if ( Player->locking_on_center ){
		out_flags |= OOC_LOCKING_ON_CENTER;
	}

	// copy the final flags in
	ADD_DATA( out_flags );
	
	// client eye information	
	ret = (ubyte)multi_pack_unpack_position( 1, data + packet_size, &Net_player->s_info.eye_pos );
	Assert(ret == OO_POS_RET_SIZE);
	packet_size += ret;

	ret = (ubyte)multi_pack_unpack_orient( 1, data + packet_size, &Net_player->s_info.eye_orient );
	Assert(ret == OO_ORIENT_RET_SIZE);
	packet_size += ret;	

	// client targeting information	
	t_subsys = -1;
	l_subsys = -1;

	// if nothing targeted
	if(Player_ai->target_objnum == -1){
		tnet_signature = 0;
	}
	// if something is targeted 
	else {
		// target net signature
		tnet_signature = Objects[Player_ai->target_objnum].net_signature;
			
		// targeted subsys index
		if(Player_ai->targeted_subsys != NULL){
			t_subsys = (char)ship_get_index_from_subsys( Player_ai->targeted_subsys, Player_ai->target_objnum );
		}

		// locked targeted subsys index
		if(Player->locking_subsys != NULL){
			l_subsys = (char)ship_get_index_from_subsys( Player->locking_subsys, Player_ai->target_objnum, 1 );
		}
	}

	// add them all
	ADD_USHORT( tnet_signature );
	ADD_DATA( t_subsys );
	ADD_DATA( l_subsys );	

	return packet_size;
}

// pack the appropriate info into the data
#define PACK_PERCENT(v) { ubyte upercent; if(v < 0.0f){v = 0.0f;} upercent = (v * 255.0f) <= 255.0f ? (ubyte)(v * 255.0f) : (ubyte)255; memcpy(data + packet_size + header_bytes, &upercent, sizeof(ubyte)); packet_size++; }
#define PACK_BYTE(v) { memcpy( data + packet_size + header_bytes, &v, 1 ); packet_size += 1; }
#define PACK_USHORT(v) { short swap = INTEL_SHORT(v); memcpy( data + packet_size + header_bytes, &swap, sizeof(short) ); packet_size += sizeof(short); }
#define PACK_SHORT(v) { ushort swap = INTEL_SHORT(v); memcpy( data + packet_size + header_bytes, &swap, sizeof(ushort) ); packet_size += sizeof(ushort); }
#define PACK_INT(v) { int swap = INTEL_INT(v); memcpy( data + packet_size + header_bytes, &swap, sizeof(int) ); packet_size += sizeof(int); }
int multi_oo_pack_data(net_player *pl, object *objp, ubyte oo_flags, ubyte *data_out)
{	
	ubyte data[255];
	ubyte data_size;	
	char percent;
	ship *shipp;	
	ubyte ret;
	float temp;	
	int packet_size = 0;

	int header_bytes;

	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		header_bytes = 4;
	} else {
		header_bytes = 2;
	}

	Assert(objp->type == OBJ_SHIP);
	if((objp->instance >= 0) && (Ships[objp->instance].ship_info_index >= 0)){
		shipp = &Ships[objp->instance];
	} else {
		return 0;
	}		

	// if we're a client (and therefore sending control info), send button info first
	if((Net_player != NULL) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		packet_size += multi_oo_pack_client_data(data + packet_size + header_bytes);		
	}

	// orientation	
	if(oo_flags & OO_ORIENT_NEW){
		ret = (ubyte)multi_pack_unpack_orient( 1, data + packet_size + header_bytes, &objp->orient );
		Assert(ret == OO_ORIENT_RET_SIZE);
		packet_size += ret;
		R_ORIENT_ADD(pl, ret);

		// global records
		OO_orient_total += ret;

		ret = (ubyte)multi_pack_unpack_rotvel( 1, data + packet_size + header_bytes, &objp->orient, &objp->pos, &objp->phys_info );
		packet_size += ret;	

		// global records
		OO_rotvel_total += ret;
		R_ORIENT_ADD(pl, ret);

		if(oo_flags & OO_EXTRA_PHYSICS){
			ret = (ubyte)multi_pack_unpack_desired_rotvel( 1, data + packet_size + header_bytes, &objp->orient, &objp->pos, &objp->phys_info, &Ship_info[shipp->ship_info_index]);
			packet_size += ret;

			// global records
			OO_desired_rotvel_total += ret;						
			R_EXTRA_PHYSICS_ADD(pl, ret);
		}
	}

	// position, velocity
	if ( oo_flags & OO_POS_NEW ) {		
		ret = (ubyte)multi_pack_unpack_position( 1, data + packet_size + header_bytes, &objp->pos );
		packet_size += ret;
		
		// global records
		OO_pos_total += ret;
		R_POS_ADD(pl, ret);
		
		ret = (ubyte)multi_pack_unpack_vel( 1, data + packet_size + header_bytes, &objp->orient, &objp->pos, &objp->phys_info );
		packet_size += ret;		
		
		// global records
		OO_vel_total += ret;		
		R_POS_ADD(pl, ret);

		/*
		if(oo_flags & OO_EXTRA_PHYSICS){
			ret = (ubyte)multi_pack_unpack_desired_vel( 1, data + packet_size + header_bytes, &objp->orient, &objp->pos, &objp->phys_info, &Ship_info[shipp->ship_info_index]);
			packet_size += ret;		
		
			// global records
			OO_vel_total += ret;	
		}
		*/
	}	
		
	// forward thrust	
	percent = (char)(objp->phys_info.forward_thrust * 100.0f);
	Assert( percent <= 100 );

	PACK_BYTE( percent );

	// global records
	OO_forward_thrust_total++;	
	R_FTHRUST_ADD(pl, 1);

	// hull info
	if ( oo_flags & OO_HULL_NEW ){
		// add the hull value for this guy		
		temp = get_hull_pct(objp);
		PACK_PERCENT(temp);		
		R_HULL_ADD(pl, 1);
				
		// global records
		OO_hull_total++;
	}

	// shield info
	if( oo_flags & OO_SHIELD_NEW ){
		// pack 2 shield values into each byte

		float quad = get_max_shield_quad(objp);

		// pack quadrant 1
		temp = (objp->shield_quadrant[0] / quad);
		PACK_PERCENT(temp);
				
		// pack quadrant 2
		temp = (objp->shield_quadrant[1] / quad);
		PACK_PERCENT(temp);				

		// pack quadrant 3
		temp = (objp->shield_quadrant[2] / quad);
		PACK_PERCENT(temp);
				
		// pack quadrant 2
		temp = (objp->shield_quadrant[3] / quad);
		PACK_PERCENT(temp);				

		OO_shield_total += 4;
		R_SHIELD_ADD(pl, 4);
	}	

	// subsystem info
	if( oo_flags & OO_SUBSYSTEMS_NEW ){
		ubyte ns;		
		ship_subsys *subsysp;
				
		// just in case we have some kind of invalid data (should've been taken care of earlier in this function)
		if(shipp->ship_info_index < 0){
			ns = 0;
			PACK_BYTE( ns );
			R_SUBSYS_ADD(pl, 1);
		}
		// add the # of subsystems, and their data
		else {
			ns = (ubyte)Ship_info[shipp->ship_info_index].n_subsystems;
			PACK_BYTE( ns );
			R_SUBSYS_ADD(pl, 1);

			// now the subsystems.
			for ( subsysp = GET_FIRST(&shipp->subsys_list); subsysp != END_OF_LIST(&shipp->subsys_list); subsysp = GET_NEXT(subsysp) ) {
				temp = (float)subsysp->current_hits / (float)subsysp->system_info->max_hits;
				PACK_PERCENT(temp);
				R_SUBSYS_ADD(pl, 1);
			}
		}
	}

	// ai mode info
	if( oo_flags & OO_AI_MODE_NEW){
		ubyte umode = (ubyte)(Ai_info[shipp->ai_index].mode);
		short submode = (short)(Ai_info[shipp->ai_index].submode);
		ushort target_signature;

		target_signature = 0;
		if ( Ai_info[shipp->ai_index].target_objnum != -1 ){
			target_signature = Objects[Ai_info[shipp->ai_index].target_objnum].net_signature;
		}

		PACK_BYTE( umode );
		PACK_SHORT( submode );
		PACK_USHORT( target_signature );	

		R_AI_MODE_ADD(pl, 5);
	}

	Assert(packet_size < 255);
	data_size = (ubyte)packet_size;

	// add the object's net signature, type and oo_flags
	packet_size = 0;
	// don't add for clients
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		ADD_USHORT( objp->net_signature );	
	}
	ADD_DATA( oo_flags );
	ADD_DATA( data_size );	
	packet_size += data_size;

	memcpy(data_out,data,packet_size);
	
	return packet_size;	
}

// unpack information for a client , return bytes processed
int multi_oo_unpack_client_data(net_player *pl, ubyte *data)
{
	ubyte in_flags;
	ship *shipp;
	int ret;
	int offset = 0;
	
	memcpy(&in_flags, data, sizeof(ubyte));	
	offset++;
		
	// if we have a valid netplayer pointer
	if(pl != NULL){
		// primary fired
		pl->player->ci.fire_primary_count = 0;
		// if ( in_flags & OOC_FIRE_PRIMARY && !(Netgame.debug_flags & NETD_FLAG_CLIENT_FIRING)){
		if ( in_flags & OOC_FIRE_PRIMARY){
			pl->player->ci.fire_primary_count = 1;
		}	

		// secondary fired
		pl->player->ci.fire_secondary_count = 0;
		if ( in_flags & OOC_FIRE_SECONDARY ){
			pl->player->ci.fire_secondary_count = 1;
		}

		// countermeasure fired
		pl->player->ci.fire_countermeasure_count = 0;
		// if ( in_flags & OOC_FIRE_COUNTERMEASURE && !(Netgame.debug_flags & NETD_FLAG_CLIENT_FIRING)){
		if ( in_flags & OOC_FIRE_COUNTERMEASURE ){
			pl->player->ci.fire_countermeasure_count = 1;
		}

		// set up aspect locking information
		pl->player->locking_on_center = 0;
		if ( in_flags & OOC_LOCKING_ON_CENTER ){
			pl->player->locking_on_center = 1;
		}

		// other locking information
		if((pl->player->objnum != -1) && (Objects[pl->player->objnum].type == OBJ_SHIP) && (Ships[Objects[pl->player->objnum].instance].ai_index != -1)){
			shipp = &Ships[Objects[pl->player->objnum].instance];

			Ai_info[shipp->ai_index].current_target_is_locked = ( in_flags & OOC_TARGET_LOCKED) ? 1 : 0;
			if	( in_flags & OOC_TARGET_SEEK_LOCK ) {
				Ai_info[shipp->ai_index].ai_flags |= AIF_SEEK_LOCK;
			} else {
				Ai_info[shipp->ai_index].ai_flags &= ~AIF_SEEK_LOCK;
			}
		}
	}

	// client eye information
	vec3d eye_pos;
	matrix eye_orient;
	physics_info pi;

	// unpack the stuff
	memset(&pi,0,sizeof(physics_info));

	ret = multi_pack_unpack_position( 0, data + offset, &eye_pos );
	Assert(ret == OO_POS_RET_SIZE);
	offset += ret;

	ret = multi_pack_unpack_orient( 0, data + offset, &eye_orient );
	Assert(ret == OO_ORIENT_RET_SIZE);
	offset += ret;

	// if we have a valid player, copy the info in
	if(pl != NULL){
		pl->s_info.eye_pos = eye_pos;
		pl->s_info.eye_orient = eye_orient;
	} 	

	// client targeting information	
	ushort tnet_sig;
	char t_subsys,l_subsys;
	object *tobj;

	// get the data
	GET_USHORT(tnet_sig);
	GET_DATA(t_subsys);
	GET_DATA(l_subsys);

	// try and find the targeted object
	tobj = NULL;
	if(tnet_sig != 0){
		tobj = multi_get_network_object( tnet_sig );
	}
	// maybe fill in targeted object values
	if((tobj != NULL) && (pl != NULL) && (pl->player->objnum != -1)){
		// assign the target object
		if(Objects[pl->player->objnum].type == OBJ_SHIP){
			Ai_info[Ships[Objects[pl->player->objnum].instance].ai_index].target_objnum = OBJ_INDEX(tobj);
		}
		pl->s_info.target_objnum = OBJ_INDEX(tobj);

		// assign subsystems if possible					
		if(Objects[pl->player->objnum].type == OBJ_SHIP){		
			Ai_info[Ships[Objects[pl->player->objnum].instance].ai_index].targeted_subsys = NULL;
			if((t_subsys != -1) && (tobj->type == OBJ_SHIP)){
				Ai_info[Ships[Objects[pl->player->objnum].instance].ai_index].targeted_subsys = ship_get_indexed_subsys( &Ships[tobj->instance], t_subsys);
			}
		}

		pl->player->locking_subsys = NULL;
		if(Objects[pl->player->objnum].type == OBJ_SHIP){		
			if((l_subsys != -1) && (tobj->type == OBJ_SHIP)){
				pl->player->locking_subsys = ship_get_indexed_subsys( &Ships[tobj->instance], l_subsys);
			}				
		}
	}				

	return offset;
}

// unpack the object data, return bytes processed
#define UNPACK_PERCENT(v)					{ ubyte temp_byte; memcpy(&temp_byte, data + offset, sizeof(ubyte)); v = (float)temp_byte / 255.0f; offset++;}
int multi_oo_unpack_data(net_player *pl, ubyte *data, ushort packet_sequence_num, ushort packet_sequence_ref, int time_born)
{	
	int offset = 0;		
	object *objp,obj_fill,*pobjp;
	ushort net_sig = 0;
	ubyte data_size, oo_flags;
	char percent;	
	float fpct;
	ship *shipp;

	// add the object's net signature, type and oo_flags
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		GET_USHORT( net_sig );	
	}
	GET_DATA( oo_flags );	
	GET_DATA( data_size );	

	// try and find the object
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		pobjp = multi_get_network_object(net_sig);	
	} else {	
		if((pl != NULL) && (pl->player->objnum != -1)){
			pobjp = &Objects[pl->player->objnum];
		} else {
			pobjp = NULL;
		}
	}
	
	// if we can't find the object, set pointer to bogus object to continue reading the data
	// ignore out of sequence packets here as well
	if ( (pobjp == NULL) || (pobjp->type != OBJ_SHIP) || (pobjp->instance < 0) || (Ships[pobjp->instance].ship_info_index < 0) || (packet_sequence_num < packet_sequence_ref)){
		if(packet_sequence_num < packet_sequence_ref){
			nprintf(("Network","Tossing out of order packet!!\n"));
		}

		offset += data_size;

		return offset;
	}		

	// ship pointer
	shipp = &Ships[pobjp->instance];

	// use the "fill" object
	objp = &obj_fill;
	memset(objp,0,sizeof(object));
	objp->type = OBJ_SHIP;
	objp->phys_info.max_vel = pobjp->phys_info.max_vel;
	objp->phys_info.max_rotvel = pobjp->phys_info.max_rotvel;
	objp->phys_info.afterburner_max_vel = pobjp->phys_info.afterburner_max_vel;
	objp->orient = pobjp->orient;
	objp->instance = pobjp->instance;

	// if this is from a player, read his button info
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		offset += multi_oo_unpack_client_data(pl, data + offset);		
	}

	// read in the data

	// orientation	
	if ( oo_flags & OO_ORIENT_NEW ) {		
		int r2 = multi_pack_unpack_orient( 0, data + offset, &objp->orient );				
		Assert(fl_abs(objp->orient.fvec.x) < 10000.0f);
		Assert(fl_abs(objp->orient.fvec.y) < 10000.0f);
		Assert(fl_abs(objp->orient.fvec.z) < 10000.0f);		
		Assert(fl_abs(objp->orient.uvec.x) < 10000.0f);
		Assert(fl_abs(objp->orient.uvec.y) < 10000.0f);
		Assert(fl_abs(objp->orient.uvec.z) < 10000.0f);		
		Assert(fl_abs(objp->orient.rvec.x) < 10000.0f);
		Assert(fl_abs(objp->orient.rvec.y) < 10000.0f);
		Assert(fl_abs(objp->orient.rvec.z) < 10000.0f);		
		offset += r2;		

		int r5 = multi_pack_unpack_rotvel( 0, data + offset, &objp->orient, &objp->pos, &objp->phys_info );		
		offset += r5;		

		if(oo_flags & OO_EXTRA_PHYSICS){
			int r6 = multi_pack_unpack_desired_rotvel( 0, data + offset, &objp->orient, &objp->pos, &objp->phys_info, &Ship_info[Ships[pobjp->instance].ship_info_index] );
			offset += r6;	
		}
	} 

	// position
	if ( oo_flags & OO_POS_NEW ) {		
		int r1 = multi_pack_unpack_position( 0, data + offset, &objp->pos );
		offset += r1;				

		int r3 = multi_pack_unpack_vel( 0, data + offset, &objp->orient, &objp->pos, &objp->phys_info );
		offset += r3;			

		/*
		if(oo_flags & OO_EXTRA_PHYSICS){
			int r4 = multi_pack_unpack_desired_vel( 0, data + offset, &objp->orient, &objp->pos, &objp->phys_info, &Ship_info[Ships[objp->instance].ship_info_index] );
			offset += r4;
		}
		*/
	}	
		
	// forward thrust	
	percent = (char)(objp->phys_info.forward_thrust * 100.0f);
	Assert( percent <= 100 );
	GET_DATA(percent);	
	
	// hull info
	if ( oo_flags & OO_HULL_NEW ){
		UNPACK_PERCENT(fpct);
		pobjp->hull_strength = fpct * Ships[pobjp->instance].ship_max_hull_strength;

		// TEST code
#ifndef NDEBUG		
		if(OO_debug_info && (Player_ai != NULL) && (pobjp != NULL) && (pobjp->instance >= 0) && (Player_ai->target_objnum == OBJ_INDEX(pobjp))){
			nprintf(("Network", "HULL UPDATE : %d, %s\n", pobjp->hull_strength, Ships[pobjp->instance].ship_name));
		}		
#endif
	}

	// shield info
	if ( oo_flags & OO_SHIELD_NEW ){
		float shield_0, shield_1, shield_2, shield_3;
		
		// unpack the 4 quadrants
		UNPACK_PERCENT(shield_0);
		UNPACK_PERCENT(shield_1);
		UNPACK_PERCENT(shield_2);
		UNPACK_PERCENT(shield_3);

		float quad = get_max_shield_quad(pobjp);

		pobjp->shield_quadrant[0] = (shield_0 * quad);
		pobjp->shield_quadrant[1] = (shield_1 * quad);
		pobjp->shield_quadrant[2] = (shield_2 * quad);
		pobjp->shield_quadrant[3] = (shield_3 * quad);

		// TEST code
#ifndef NDEBUG
		if(OO_debug_info && (Player_ai != NULL) && (pobjp != NULL) && (pobjp->instance >= 0) && (Player_ai->target_objnum == OBJ_INDEX(pobjp))){
			nprintf(("Network", "SHIELD UPDATE %f %f %f %f: %s\n", pobjp->shield_quadrant[0], pobjp->shield_quadrant[1], pobjp->shield_quadrant[2], pobjp->shield_quadrant[3], Ships[pobjp->instance].ship_name));
		}
#endif
	}	

	if ( oo_flags & OO_SUBSYSTEMS_NEW ) {
		ubyte n_subsystems, subsys_count;
		float subsystem_percent[MAX_MODEL_SUBSYSTEMS];		
		ship_subsys *subsysp;		
		float val;		
		int i;		

		// get the data for the subsystems
		GET_DATA( n_subsystems );
		for ( i = 0; i < n_subsystems; i++ ){
			UNPACK_PERCENT( subsystem_percent[i] );
		}		
		
		// fill in the subsystem data
		subsys_count = 0;
		for ( subsysp = GET_FIRST(&shipp->subsys_list); subsysp != END_OF_LIST(&shipp->subsys_list); subsysp = GET_NEXT(subsysp) ) {
			int subsys_type;

			val = subsystem_percent[subsys_count] * subsysp->system_info->max_hits;
			subsysp->current_hits = val;

			// add the value just generated (it was zero'ed above) into the array of generic system types
			subsys_type = subsysp->system_info->type;					// this is the generic type of subsystem
			Assert ( subsys_type < SUBSYSTEM_MAX );
			shipp->subsys_info[subsys_type].current_hits += val;
			subsys_count++;

			// if we've reached max subsystems for some reason, bail out
			if(subsys_count >= n_subsystems){
				break;
			}
		}
		
		// recalculate all ship subsystems
		ship_recalc_subsys_strength( shipp );	

		// TEST code
#ifndef NDEBUG
		if(OO_debug_info && (Player_ai != NULL) && (pobjp != NULL) && (pobjp->instance >= 0) && (Player_ai->target_objnum == OBJ_INDEX(pobjp))){
			nprintf(("Network", "SUBSYS UPDATE : %s\n", Ships[pobjp->instance].ship_name));
		}
#endif
	}

	if ( oo_flags & OO_AI_MODE_NEW ) {
		ubyte umode;
		short submode;
		ushort target_signature;
		object *target_objp;

		GET_DATA(umode);
		GET_SHORT(submode);
		GET_USHORT( target_signature );		

		if(shipp->ai_index > 0){
			Ai_info[shipp->ai_index].mode = umode;
			Ai_info[shipp->ai_index].submode = submode;		

			// set this guys target objnum
			target_objp = multi_get_network_object( target_signature );
			if ( target_objp == NULL ){
				Ai_info[shipp->ai_index].target_objnum = -1;
			} else {
				Ai_info[shipp->ai_index].target_objnum = OBJ_INDEX(target_objp);
			}
		}
	}
		
	// handle this new data	
	if(pl != NULL){
		multi_oo_handle_new_data(&Ships[pobjp->instance], objp, oo_flags, pl);	
	}
	
	return offset;
}

// reset the timestamp appropriately for the passed in object
void multi_oo_reset_timestamp(net_player *pl, object *objp, int range, int in_cone)
{
	int stamp = 0;
	int player_index = NET_PLAYER_INDEX(pl);

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
		Ships[objp->instance].pos_update_stamp[player_index] = timestamp(stamp);
	} 
}

// reset the timestamp appropriately for the passed in object
void multi_oo_reset_status_timestamp(object *objp, int player_index)
{
	Ships[objp->instance].status_update_stamp[player_index] = timestamp(OO_HULL_SHIELD_TIME);
}

// reset the timestamp appropriately for the passed in object
void multi_oo_reset_subsys_timestamp(object *objp, int player_index)
{
	Ships[objp->instance].subsys_update_stamp[player_index] = timestamp(OO_SUBSYS_TIME);
}

// determine what needs to get sent for this player regarding the passed object, and when
int multi_oo_maybe_update(net_player *pl,object *pobj,object *obj,ubyte *data)
{
	ubyte oo_flags;
	int stamp;
	int player_index;
	vec3d player_eye;
	vec3d obj_dot;
	float eye_dot, dist;
	int in_cone;
	int range;
	int ship_index;

	// if the timestamp has elapsed for this guy, send stuff
	player_index = NET_PLAYER_INDEX(pl);
	if(!(player_index >= 0) || !(player_index < MAX_PLAYERS)){
		return 0;
	}

	// determine what the timestamp is for this object
	if(obj->type == OBJ_SHIP){
		stamp = Ships[obj->instance].pos_update_stamp[player_index];
		ship_index = &Ships[obj->instance] - Ships;
	} else {
		return 0;
	}
	
	// if we're supposed to update this guy
	if((stamp == -1) || timestamp_elapsed(stamp)){
		// check dot products		
		player_eye = pl->s_info.eye_orient.fvec;
		vm_vec_sub(&obj_dot,&obj->pos,&pobj->pos);
		vm_vec_normalize(&obj_dot);
		eye_dot = vm_vec_dot(&obj_dot,&player_eye);		
		in_cone = (eye_dot >= OO_VIEW_CONE_DOT) ? 1 : 0;		
		
		// if he's in view or is "swinging into view", send info for him
		if(Interpolate_dot[ship_index] >= -1.0f){
			if((eye_dot - Interpolate_dot[ship_index]) >= OO_VIEW_DIFF_TOL){
				in_cone = 1;
			}
		}

		/*
		if(!stricmp(Ships[obj->instance].ship_name, "alpha 1")){
			if(in_cone){
				nprintf(("Network","In cone\n"));
			} else {
				nprintf(("Network","Not in cone\n"));
			}
		}
		*/
		
		// store the dot product for this frame
		Interpolate_dot[ship_index] = eye_dot;		

		// determine distance (near, medium, far)
		vm_vec_sub(&obj_dot,&obj->pos,&pobj->pos);
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

		// zero oo_flags
		oo_flags = 0;
		
		// if the player is on "high" updates, add in extra physics info
		if((pl->p_info.options.obj_update_level == OBJ_UPDATE_HIGH) || (pl->p_info.options.obj_update_level == OBJ_UPDATE_LAN)){
			oo_flags |= OO_EXTRA_PHYSICS;
		}

		// if the object's hull/shield timestamp has expired
		if((Ships[obj->instance].status_update_stamp[player_index] == -1) || timestamp_elapsed(Ships[obj->instance].status_update_stamp[player_index])){
			oo_flags |= (OO_HULL_NEW | OO_SHIELD_NEW);

			// reset the timestamp
			multi_oo_reset_status_timestamp(obj, player_index);			
		}

		// if the object's hull/shield timestamp has expired
		if((Ships[obj->instance].subsys_update_stamp[player_index] == -1) || timestamp_elapsed(Ships[obj->instance].subsys_update_stamp[player_index])){
			oo_flags |= OO_SUBSYSTEMS_NEW | OO_AI_MODE_NEW;

			// reset the timestamp
			multi_oo_reset_subsys_timestamp(obj, player_index);
		}

		// add info for a targeted object)
		if((pl->s_info.target_objnum != -1) && (OBJ_INDEX(obj) == pl->s_info.target_objnum)){
			oo_flags |= (OO_POS_NEW | OO_ORIENT_NEW | OO_HULL_NEW | OO_SHIELD_NEW);
		}
		// all other cases
		else {			
			// if the position didn't change, don't send anything			
			oo_flags |= OO_POS_NEW;			

			// add info which is contingent upon being "in front"			
			if(in_cone){
				oo_flags |= OO_ORIENT_NEW;
			}						
		}

		// if the ship is a cruiser or capital ship, always send extra physics
		if(Ship_info[Ships[obj->instance].ship_info_index].flags & (SIF_CRUISER | SIF_CAPITAL)){
			oo_flags |= OO_EXTRA_PHYSICS;
		}

		// pack stuff only if we have to 
		if(oo_flags){
			return multi_oo_pack_data(pl,obj,oo_flags,data);
		} 
		
		// don't do anything
		return 0;
	}
	
	// didn't do anything
	return 0;
}

// process all other objects for this player
void multi_oo_process_all(net_player *pl)
{
	ubyte data[MAX_PACKET_SIZE];
	ubyte data_add[MAX_PACKET_SIZE];
	ubyte stop;
	int add_size;	
	int packet_size = 0;
	int idx;
		
	object *moveup;
	object *pobj;	

	// if the player has an invalid objnum..
	if(pl->player->objnum < 0){
		return;
	}

	// get the player's object
	pobj = &Objects[pl->player->objnum];

	object *targ_obj;	

	// build the list of ships to check against
	multi_oo_build_ship_list(pl);

	// do nothing if he has no object targeted, or if he has a weapon targeted
	if((pl->s_info.target_objnum != -1) && (Objects[pl->s_info.target_objnum].type == OBJ_SHIP)){
		// build the header
		BUILD_HEADER(OBJECT_UPDATE);

		// add the sequencing #
		ADD_USHORT(Netgame.server_update_seq);
		ADD_INT(OO_global_time);
	
		// get a pointer to the object
		targ_obj = &Objects[pl->s_info.target_objnum];
	
		// run through the maybe_update function
		add_size = multi_oo_maybe_update(pl,pobj,targ_obj,data_add);

		// copy in any relevant data
		if(add_size){
			stop = 0xff;
			ADD_DATA(stop);

			memcpy(data + packet_size, data_add, add_size);
			packet_size += add_size;		
		}
	} else {
		// just build the header for the rest of the function
		BUILD_HEADER(OBJECT_UPDATE);

		// add the sequencing #
		ADD_USHORT(Netgame.server_update_seq);
		ADD_INT(OO_global_time);
	}
		
	idx = 0;
	while((OO_ship_index[idx] >= 0) && (idx < MAX_SHIPS)){
		// if this guy is over his datarate limit, do nothing
		if(multi_oo_rate_exceeded(pl)){
			nprintf(("Network","Capping client\n"));
			idx++;

			continue;
		}			

		// get the object
		moveup = &Objects[Ships[OO_ship_index[idx]].objnum];

		// maybe send some info		
		add_size = multi_oo_maybe_update(pl,pobj,moveup,data_add);

		// if this data is too much for the packet, send off what we currently have and start over
		if(packet_size + add_size > OO_MAX_SIZE){
			stop = 0x00;
			ADD_DATA(stop);
			
			multi_io_send(pl data, packet_size);
			pl->s_info.rate_bytes += packet_size;

			packet_size = 0;
			BUILD_HEADER(OBJECT_UPDATE);

			// add the sequencing #
			ADD_USHORT(Netgame.server_update_seq);
			ADD_INT(OO_global_time);
		}

		if(add_size){
			stop = 0xff;
			ADD_DATA(stop);

			// copy in the data
			memcpy(data + packet_size,data_add,add_size);
			packet_size += add_size;
		}

		// next ship
		idx++;
	}

	// if we have anything more than 3 byte in the packet, send the last one off
	if(packet_size > 3){
		stop = 0x00;
		ADD_DATA(stop);
		
		multi_io_send(pl, data, packet_size);
		pl->s_info.rate_bytes += packet_size;
	}
}

// process all object update details for this frame
void multi_oo_process()
{
	int idx;

	// increment sequencing #
	multi_oo_increment_seq();
	
	// process each player
	for(idx=0; idx<MAX_PLAYERS; idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_player != &Net_players[idx])){
			// now process the rest of the objects
			multi_oo_process_all(&Net_players[idx]);
		}
	}
}

// process incoming object update data
void multi_oo_process_update(ubyte *data, header *hinfo)
{
	ushort packet_seq, sequence_ref;
	ubyte stop;	
	int player_index;
	int server_stamp,time_born = 0;
	int offset = HEADER_LENGTH;
	net_player *pl = NULL;

	// process sequencing info here
	GET_USHORT(packet_seq);
	GET_INT(server_stamp);

	// if this is processed on the server, its a client object update packet
	player_index = -1;
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		// determine what player this came from and determine sequencing				
		player_index = find_player_id(hinfo->id);
		if(player_index != -1){
			// wrap around
			if(((packet_seq - Net_players[player_index].client_cinfo_seq) < 0) && ((Net_players[player_index].client_cinfo_seq - packet_seq) > 2000)){
				Net_players[player_index].client_cinfo_seq = packet_seq;
			} else if(packet_seq > Net_players[player_index].client_cinfo_seq){
				Net_players[player_index].client_cinfo_seq = packet_seq;
			}

			// set the sequence reference
			sequence_ref = Net_players[player_index].client_cinfo_seq;

			pl = &Net_players[player_index];
		} else {
			sequence_ref = packet_seq;
			pl = NULL;
		}
	}
	// otherwise its a "regular" object update packet
	else {
		// do time latency stuff
		//multi_tbuf_add_time(&OO_diff,server_stamp,timer_get_milliseconds());		

		// wrap around
		if(((packet_seq - Net_player->client_server_seq) < 0) && ((Net_player->client_server_seq - packet_seq) > 2000)){
			Net_player->client_server_seq = packet_seq;
		} else if(packet_seq > Net_player->client_server_seq){
			Net_player->client_server_seq = packet_seq;
		}

		// set the sequence reference
		sequence_ref = Net_player->client_server_seq;

		pl = Netgame.server;
	}

	GET_DATA(stop);
	
	while(stop == 0xff){
		// process the data
		offset += multi_oo_unpack_data(pl, data + offset, packet_seq, sequence_ref, time_born);

		GET_DATA(stop);
	}
	PACKET_SET_SIZE();
}

// initialize all object update timestamps (call whenever entering gameplay state)
void multi_oo_gameplay_init()
{
	int split,cur,idx;
	ship_obj *so;		
	object *objp;
	ship *shipp;
	split = 3000 / ship_get_num_ships();

#ifdef OO_NEW
	extern multi_oo_new_gameplay_init();
	multi_oo_new_gameplay_init();
#else	

	// server should setup initial update timestamps	
	// stagger initial updates over 3 seconds or so
	cur = 0;
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		if(Objects[so->objnum].type == OBJ_SHIP){
			shipp = &Ships[Objects[so->objnum].instance];
		
			// update the timestamps
			for(idx=0;idx<MAX_PLAYERS;idx++){
				shipp->pos_update_stamp[idx] = timestamp(cur);
				shipp->status_update_stamp[idx] = timestamp(cur + split);
				shipp->subsys_update_stamp[idx] = timestamp(cur + (split * 2));
			}

			// increment the time
			cur += split;

			objp = &Objects[so->objnum];
			shipp->int_cur.pos = objp->pos;
			shipp->int_cur.orient = objp->orient;
			shipp->int_cur.vel = objp->phys_info.vel;
			shipp->int_cur.desired_vel = objp->phys_info.desired_vel;
			shipp->int_cur.rotvel = objp->phys_info.rotvel;
			shipp->int_cur.desired_rotvel = objp->phys_info.desired_rotvel;
			memcpy(&shipp->int_last, &shipp->int_cur, sizeof(interp_info));

			// uninitialized
			Interpolate_dot[MAX_SHIPS] = -2.0f;
		}
	}		
	
	// reset our data #'s
	OO_pos_total = 0;
	OO_vel_total = 0;
	OO_desired_vel_total = 0;
	OO_orient_total = 0;
	OO_rotvel_total = 0;
	OO_desired_rotvel_total = 0;
	OO_hull_total = 0;
	OO_shield_total = 0;
	OO_single_subsys_total = 0;
	OO_subsys_total = 0;
	OO_forward_thrust_total = 0;
#endif
}

// process an object update sync packet
void multi_oo_process_update_sync(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;	

	// we're ignoring this value for now	
	PACKET_SET_SIZE();		

	// initialize the time buffer
	//multi_tbuf_init(&OO_diff);
}

// send an update sync packet
void multi_oo_send_update_sync(net_player *pl)
{
	ubyte data[20];
	int packet_size = 0;	

	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);

	// build the header and add the data
	BUILD_HEADER(OBJ_UPDATE_SYNC);	

	// send to everyone
	if(pl == NULL){		
		multi_io_send_to_all_reliable(data, packet_size);
	} else {		
		multi_io_send_reliable(pl, data, packet_size);
	}
}

// initialize the server's time sync stuff
void multi_oo_sync_init()
{	
//	OO_sync_stamp = timestamp(OO_SYNC_TIME);
}

// send control info for a client (which is basically a "reverse" object update)
void multi_oo_send_control_info()
{
	ubyte data[MAX_PACKET_SIZE], stop;
	ubyte data_add[MAX_PACKET_SIZE];
	ubyte oo_flags;	
	int add_size;
	int packet_size = 0;

	// if I'm dying or my object type is not a ship, bail here
	if((Player_obj != NULL) && (Player_ship->flags & SF_DYING)){
		return;
	}

	// increment sequencing
	multi_oo_increment_seq();
	
	// build the header
	BUILD_HEADER(OBJECT_UPDATE);	

	// add the sequencing #
	ADD_USHORT(Netgame.server_update_seq);
	ADD_INT(OO_global_time);

	oo_flags = (OO_POS_NEW | OO_ORIENT_NEW | OO_EXTRA_PHYSICS);

	// pack the appropriate info into the data
	add_size = multi_oo_pack_data(Net_player, Player_obj, oo_flags, data_add);

	// copy in any relevant data
	if(add_size){
		stop = 0xff;
		ADD_DATA(stop);

		memcpy(data + packet_size, data_add, add_size);
		packet_size += add_size;		
	}

	// add the final stop byte
	stop = 0x0;
	ADD_DATA(stop);

	// send to the server
	if(Netgame.server != NULL){		
		multi_io_send(Net_player, data, packet_size);
	}
}

// reset all sequencing info
void multi_oo_reset_sequencing()
{	
	int idx;

	// reset outgoing sequence #'s
	Netgame.server_update_frame_ref = 0;
	Netgame.server_update_seq = 0;
	
	// reset all incoming seqeunce #'s
	for(idx=0;idx<MAX_PLAYERS;idx++){
		Net_players[idx].client_cinfo_seq = 0;
		Net_players[idx].client_server_seq = 0;
	}
}

// display any oo info on the hud
void multi_oo_display()
{
#ifndef NDEBUG
	np_update_record *r;
	int sy = 0;

	if(!MULTIPLAYER_MASTER || MULTIPLAYER_STANDALONE){
		return;
	}
	if(OO_update_index < 0){
		return;
	}
	if(Net_players[OO_update_index].player == NULL){
		return;
	}
	r = &OO_update_records[OO_update_index];

	// display info
	gr_string(gr_screen.max_w - 150, sy, Net_players[OO_update_index].player->callsign);
	sy += 10;
	gr_printf(gr_screen.max_w - 150, sy, "pos : %d (%d/s)", r->pos_bytes, (int)r->pos_avg);
	sy += 10;
	gr_printf(gr_screen.max_w - 150, sy, "ori : %d (%d/s)", r->orient_bytes, (int)r->orient_avg);
	sy += 10;
	gr_printf(gr_screen.max_w - 150, sy, "hul : %d (%d/s)", r->hull_bytes, (int)r->hull_avg);
	sy += 10;
	gr_printf(gr_screen.max_w - 150, sy, "shi : %d (%d/s)", r->shield_bytes, (int)r->shield_avg);
	sy += 10;
	gr_printf(gr_screen.max_w - 150, sy, "aim : %d (%d/s)", r->ai_mode_bytes, (int)r->ai_mode_avg);
	sy += 10;
	gr_printf(gr_screen.max_w - 150, sy, "ext : %d (%d/s)", r->extra_physics_bytes, (int)r->extra_physics_avg);
	sy += 10;
	gr_printf(gr_screen.max_w - 150, sy, "fth : %d (%d/s)", r->fthrust_bytes, (int)r->fthrust_avg);	
	sy += 10;
	gr_printf(gr_screen.max_w - 150, sy, "sub : %d (%d/s)", r->subsys_bytes, (int)r->subsys_avg);	
#endif
}


// ---------------------------------------------------------------------------------------------------
// DATARATE DEFINES/VARS
//

// low object update datarate limit
#define OO_LIMIT_LOW				2000
#define OO_LIMIT_MED				4000
#define OO_LIMIT_HIGH				100000000

// timestamp for sending control info (movement only - we'll send button info all the time)
#define OO_CIRATE					67					// 15x a second
int Multi_cirate_stamp			= -1;				// timestamp for waiting on control info time
int Multi_cirate_can_send		= 1;				// if we can send control info this frame

// global max rates
int OO_server_rate = -1;							// max _total_ bandwidth to send to all clients
int OO_client_rate = -1;							// max bandwidth to go to an individual client

// update timestamp for server datarate checking
#define RATE_UPDATE_TIME		1250				// in ms
int OO_server_rate_stamp = -1;

// process datarate limiting stuff for the server
void multi_oo_server_process();

// process datarate limiting stuff for the client
void multi_oo_client_process();

// update the server datarate
void multi_oo_update_server_rate();


// ---------------------------------------------------------------------------------------------------
// DATARATE FUNCTIONS
//

// process all object update datarate details
void multi_oo_rate_process()
{
	// if I have no valid player, drop out here
	if(Net_player == NULL){
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
			if((Net_players[idx].s_info.rate_stamp == -1) || timestamp_elapsed(Net_players[idx].s_info.rate_stamp)){
				Net_players[idx].s_info.rate_stamp = timestamp(1000);
				Net_players[idx].s_info.rate_bytes = 0;
			}
		}
	}

	// determine if we should be updating the server datarate
	if((OO_server_rate_stamp == -1) || timestamp_elapsed(OO_server_rate_stamp)){
		// reset the timestamp
		OO_server_rate_stamp = timestamp(RATE_UPDATE_TIME);

		// update the server datarate
		multi_oo_update_server_rate();

		// nprintf(("Network","UPDATING SERVER DATARATE\n"));
	}

	// see if we should be updating sync times
	/*
	if((OO_sync_stamp != -1) && timestamp_elapsed(OO_sync_stamp)){
		// send an update sync packet
		multi_oo_send_update_sync();

		// initialize the server's time sync stuff
		multi_oo_sync_init();
	}*/
	
}

// process datarate limiting stuff for the client
void multi_oo_client_process()
{
	// if the timestamp is -1 or has elapsed, reset it
	if((Multi_cirate_stamp == -1) || timestamp_elapsed(Multi_cirate_stamp)){
		Multi_cirate_can_send = 1;
		Multi_cirate_stamp = timestamp(OO_CIRATE);
	}

	// calculate what time difference should be added to all prediction calculations
	/*
	OO_time_diff = 
						// server time plus current ping relative to "original" time
						abs(	(OO_server_time + OO_my_orig_ping + (Netgame.server->s_info.ping.ping_avg - OO_my_orig_ping)) -

							// whatever my original time was at the "original" time
							OO_my_time
						);
						*/	
}


// datarate limiting system for server -------------------------------------

// initialize the rate limiting system for all players
void multi_oo_rate_init_all()
{
	int idx;

	// if I don't have a net_player, bail here
	if(Net_player == NULL){
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

	// high update level - super high datarate
	case OBJ_UPDATE_HIGH:
		rate_compare = OO_LIMIT_HIGH;
		break;

	// LAN - no rate max
	case OBJ_UPDATE_LAN:
		return 0;

	// default level
	default:
		Int3();
		rate_compare = OO_LIMIT_LOW;
		break;
	}

	// if the server global rate is actually lower
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
	if((Net_player == NULL) || !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
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
		// set to 0 so we don't limit anything
		OO_server_rate = Multi_options_g.datarate_cap;
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
		Int3();
		return;
	}	

	// set the individual client level
	OO_client_rate = OO_server_rate / num_connections;
	// nprintf(("Network","Setting client rate to %d\n",OO_client_rate));
}

	
#endif // #ifndef OO_NEW
