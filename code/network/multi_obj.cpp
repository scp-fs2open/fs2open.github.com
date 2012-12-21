/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "network/multi_obj.h"
#include "globalincs/globals.h"
#include "freespace2/freespace.h"
#include "io/timer.h"
#include "io/key.h"
#include "globalincs/linklist.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multi_options.h"
#include "network/multi_rate.h"
#include "network/multi.h"
#include "object/object.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "math/spline.h"
#include "physics/physics.h"
#include "ship/afterburner.h"
#include "cfile/cfile.h"


// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE DEFINES/VARS
//

// test stuff
float oo_arrive_time[MAX_SHIPS][5];				// the last 5 arrival times for each ship
int oo_arrive_time_count[MAX_SHIPS];			// size of the arrival queue
float oo_arrive_time_avg_diff[MAX_SHIPS];		// the average time between arrivals
float oo_arrive_time_next[MAX_SHIPS];			// how many seconds have gone by. should be equal to oo_arrive_time_avg_diff[] the next time we get an update

// interp stuff
int oo_interp_count[MAX_SHIPS];
vec3d oo_interp_points[MAX_SHIPS][2];
bez_spline oo_interp_splines[MAX_SHIPS][2];
void multi_oo_calc_interp_splines(int ship_index, vec3d *cur_pos, matrix *cur_orient, physics_info *cur_phys_info, vec3d *new_pos, matrix *new_orient, physics_info *new_phys_info);

// HACK!!!
bool Multi_oo_afterburn_hack = false;

// how much data we're willing to put into a given oo packet
#define OO_MAX_SIZE					480

// tolerance for bashing position
#define OO_POS_UPDATE_TOLERANCE	100.0f

// new improved - more compacted info type
#define OO_POS_NEW					(1<<0)		// 
#define OO_ORIENT_NEW				(1<<1)		// 
#define OO_HULL_NEW					(1<<2)		// Hull AND shields
#define OO_AFTERBURNER_NEW			(1<<3)		// 
#define OO_SUBSYSTEMS_AND_AI_NEW	(1<<4)		// 
#define OO_PRIMARY_BANK				(1<<5)		// if this is set, fighter has selected bank one
#define OO_PRIMARY_LINKED			(1<<6)		// if this is set, banks are linked
#define OO_TRIGGER_DOWN				(1<<7)		// if this is set, trigger is DOWN

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
int Multi_oo_target_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	100, 				// 15x a second 
	100, 				// 15x a second
	66,				// 30x a second
	66,
};

// for near ships
int Multi_oo_front_near_update_times[MAX_OBJ_UPDATE_LEVELS] =
{
	150,				// low update
	100,				// medium update
	66,				// high update
	66,
};

// for medium ships
int Multi_oo_front_medium_update_times[MAX_OBJ_UPDATE_LEVELS] =
{
	250,				// low update
	180, 				// medium update
	120,				// high update
	66,
};

// for far ships
int Multi_oo_front_far_update_times[MAX_OBJ_UPDATE_LEVELS] =
{
	750,				// low update
	350, 				// medium update
	150, 				// high update
	66,
};

// for near ships
int Multi_oo_rear_near_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	300,				// low update
	200,				// medium update
	100,				// high update
	66,
};

// for medium ships
int Multi_oo_rear_medium_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	800,				// low update
	600,				// medium update
	300,				// high update
	66,
};

// for far ships
int Multi_oo_rear_far_update_times[MAX_OBJ_UPDATE_LEVELS] = 
{
	2500, 			// low update
	1500,				// medium update
	400,				// high update
	66,
};

// ship index list for possibly sorting ships based upon distance, etc
short OO_ship_index[MAX_SHIPS];

int OO_update_index = -1;							// index into OO_update_records for displaying update record info

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
	float dot1, dot2;
	vec3d v1, v2;
	vec3d vn1, vn2;

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

	// get the distance and dot product to the player obj for both
	vm_vec_sub(&v1, &OO_player_obj->pos, &obj1->pos);
	dist1 = vm_vec_copy_normalize(&vn1, &v1);
	vm_vec_sub(&v2, &OO_player_obj->pos, &obj2->pos);
	dist2 = vm_vec_copy_normalize(&vn2, &v2);
	dot1 = vm_vec_dotprod(&OO_player_obj->orient.vec.fvec, &vn1);
	dot2 = vm_vec_dotprod(&OO_player_obj->orient.vec.fvec, &vn2);

	// objects in front take precedence
	if((dot1 < 0.0f) && (dot2 >= 0.0f)){
		return 1;
	} else if((dot2 < 0.0f) && (dot1 >= 0.0f)){
		return -1;
	}

	// otherwise go by distance
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
		if (Objects[moveup->objnum].flags & OF_SHOULD_BE_DEAD){
			continue;
		}

		// don't send info for dying ships
		if (Ships[Objects[moveup->objnum].instance].flags & SF_DYING){
			continue;
		}		

		// never update the knossos device
		if ((Ships[Objects[moveup->objnum].instance].ship_info_index >= 0) && (Ships[Objects[moveup->objnum].instance].ship_info_index < Num_ship_classes) && (Ship_info[Ships[Objects[moveup->objnum].instance].ship_info_index].flags & SIF_KNOSSOS_DEVICE)){
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

// pack information for a client (myself), return bytes added
int multi_oo_pack_client_data(ubyte *data)
{
	ubyte out_flags;
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
	if ( (Player_ship != NULL) && (Player_ship->flags & SF_TRIGGER_DOWN) ){
		out_flags |= OOC_TRIGGER_DOWN;
	}

	if ( (Player_obj != NULL) && Player_obj->phys_info.flags & PF_AFTERBURNER_ON){
		out_flags |= OOC_AFTERBURNER_ON;
	}

	// send my bank info
	if(Player_ship != NULL){
		if(Player_ship->weapons.current_primary_bank > 0){
			out_flags |= OOC_PRIMARY_BANK;
		}

		// linked or not
		if(Player_ship->flags & SF_PRIMARY_LINKED){
			out_flags |= OOC_PRIMARY_LINKED;
		}
	}

	// copy the final flags in
	ADD_DATA( out_flags );

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
	ubyte data_size = 0;	
	char percent;
	ship *shipp;	
	ship_info *sip;
	ubyte ret;
	float temp;	
	int header_bytes;
	int packet_size = 0;

	// make sure we have a valid ship
	Assert(objp->type == OBJ_SHIP);
	if((objp->instance >= 0) && (Ships[objp->instance].ship_info_index >= 0)){
		shipp = &Ships[objp->instance];
		sip = &Ship_info[shipp->ship_info_index];
	} else {
		return 0;
	}			

	// invalid player
	if(pl == NULL){
		return 0;
	}

	// no flags -> do nothing
	if(oo_flags == 0){
		return 0;
	}

	// if i'm the client, make sure I only send certain things	
	if(!MULTIPLAYER_MASTER){
		Assert(oo_flags & (OO_POS_NEW | OO_ORIENT_NEW));
		Assert(!(oo_flags & (OO_HULL_NEW | OO_SUBSYSTEMS_AND_AI_NEW)));
	} 
	// server 
	else {
		// Assert(oo_flags & OO_POS_NEW);
	}

	// header sizes
	if(MULTIPLAYER_MASTER){
		header_bytes = 5;
	} else {
		header_bytes = 2;
	}	

	// if we're a client (and therefore sending control info), pack client-specific info
	if((Net_player != NULL) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		packet_size += multi_oo_pack_client_data(data + packet_size + header_bytes);		
	}		
		
	// position, velocity
	if ( oo_flags & OO_POS_NEW ) {		
		ret = (ubyte)multi_pack_unpack_position( 1, data + packet_size + header_bytes, &objp->pos );
		packet_size += ret;
		
		// global records
		multi_rate_add(NET_PLAYER_NUM(pl), "pos", ret);		
			
		ret = (ubyte)multi_pack_unpack_vel( 1, data + packet_size + header_bytes, &objp->orient, &objp->pos, &objp->phys_info );
		packet_size += ret;		
			
		// global records		
		multi_rate_add(NET_PLAYER_NUM(pl), "pos", ret);				
	}	

	// orientation	
	if(oo_flags & OO_ORIENT_NEW){
		ret = (ubyte)multi_pack_unpack_orient( 1, data + packet_size + header_bytes, &objp->orient );
		// Assert(ret == OO_ORIENT_RET_SIZE);
		packet_size += ret;
		multi_rate_add(NET_PLAYER_NUM(pl), "ori", ret);				

		ret = (ubyte)multi_pack_unpack_rotvel( 1, data + packet_size + header_bytes, &objp->orient, &objp->pos, &objp->phys_info );
		packet_size += ret;	

		// global records		
		multi_rate_add(NET_PLAYER_NUM(pl), "ori", ret);		
	}
			
	// forward thrust	
	percent = (char)(objp->phys_info.forward_thrust * 100.0f);
	Assert( percent <= 100 );

	PACK_BYTE( percent );

	// global records	
	multi_rate_add(NET_PLAYER_NUM(pl), "fth", 1);	

	// hull info
	if ( oo_flags & OO_HULL_NEW ){
		// add the hull value for this guy		
		temp = get_hull_pct(objp);
		if ( (temp < 0.004f) && (temp > 0.0f) ) {
			temp = 0.004f;		// 0.004 is the lowest positive value we can have before we zero out when packing
		}
		PACK_PERCENT(temp);				
		multi_rate_add(NET_PLAYER_NUM(pl), "hul", 1);	

		float quad = get_max_shield_quad(objp);

		// pack 2 shield values into each byte

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
				
		multi_rate_add(NET_PLAYER_NUM(pl), "shl", 4);	
	}	

	// subsystem info
	if( oo_flags & OO_SUBSYSTEMS_AND_AI_NEW ){
		ubyte ns;		
		ship_subsys *subsysp;
				
		// just in case we have some kind of invalid data (should've been taken care of earlier in this function)
		if(shipp->ship_info_index < 0){
			ns = 0;
			PACK_BYTE( ns );

			multi_rate_add(NET_PLAYER_NUM(pl), "sub", 1);	
		}
		// add the # of subsystems, and their data
		else {
			ns = (ubyte)Ship_info[shipp->ship_info_index].n_subsystems;
			PACK_BYTE( ns );

			multi_rate_add(NET_PLAYER_NUM(pl), "sub", 1);	

			// now the subsystems.
			for ( subsysp = GET_FIRST(&shipp->subsys_list); subsysp != END_OF_LIST(&shipp->subsys_list); subsysp = GET_NEXT(subsysp) ) {
				temp = (float)subsysp->current_hits / (float)subsysp->max_hits;
				PACK_PERCENT(temp);
				
				multi_rate_add(NET_PLAYER_NUM(pl), "sub", 1);
			}
		}

		// ai mode info
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

		multi_rate_add(NET_PLAYER_NUM(pl), "aim", 5);

		// primary weapon energy
		temp = shipp->weapon_energy / sip->max_weapon_reserve;
		PACK_PERCENT(temp);
	}		

	// afterburner info
	oo_flags &= ~PF_AFTERBURNER_ON;
	if(objp->phys_info.flags & PF_AFTERBURNER_ON){
		oo_flags |= OO_AFTERBURNER_NEW;
	}

	// if this ship is a support ship, send some extra info
	ubyte support_extra = 0;
	if(MULTIPLAYER_MASTER && (sip->flags & SIF_SUPPORT) && (shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO)){
		ushort dock_sig;

		// flag
		support_extra = 1;		
		PACK_BYTE( support_extra );
		PACK_INT( Ai_info[shipp->ai_index].ai_flags );
		PACK_INT( Ai_info[shipp->ai_index].mode );
		PACK_INT( Ai_info[shipp->ai_index].submode );

		if((Ai_info[shipp->ai_index].support_ship_objnum < 0) || (Ai_info[shipp->ai_index].support_ship_objnum >= MAX_OBJECTS)){
			dock_sig = 0;
		} else {
			dock_sig = Objects[Ai_info[shipp->ai_index].support_ship_objnum].net_signature;
		}		

		PACK_USHORT( dock_sig );
	} else {
		support_extra = 0;
		PACK_BYTE( support_extra );
	}			

	// make sure we have a valid chunk of data
	Assert(packet_size < 255);
	if(packet_size >= 255){
		return 0;
	}
	data_size = (ubyte)packet_size;

	// add the object's net signature, type and oo_flags
	packet_size = 0;
	// don't add for clients
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		multi_rate_add(NET_PLAYER_NUM(pl), "sig", 2);
		ADD_USHORT( objp->net_signature );		

		multi_rate_add(NET_PLAYER_NUM(pl), "flg", 1);
		ADD_DATA( oo_flags );
	}	

	multi_rate_add(NET_PLAYER_NUM(pl), "siz", 1);
	ADD_DATA( data_size );	
	
	multi_rate_add(NET_PLAYER_NUM(pl), "seq", 1);
	ADD_DATA( shipp->np_updates[NET_PLAYER_NUM(pl)].seq );

	packet_size += data_size;

	// copy to the outgoing data
	memcpy(data_out, data, packet_size);	
	
	return packet_size;	
}

// unpack information for a client , return bytes processed
int multi_oo_unpack_client_data(net_player *pl, ubyte *data)
{
	ubyte in_flags;
	ship *shipp = NULL;
	object *objp = NULL;
	int offset = 0;

	if (pl == NULL)
		Error(LOCATION, "Invalid net_player pointer passed to multi_oo_unpack_client\n");
	
	memcpy(&in_flags, data, sizeof(ubyte));	
	offset++;

	// get the player ship and object
	if((pl->m_player->objnum >= 0) && (Objects[pl->m_player->objnum].type == OBJ_SHIP) && (Objects[pl->m_player->objnum].instance >= 0)){
		objp = &Objects[pl->m_player->objnum];
		shipp = &Ships[objp->instance];
	}
		
	// if we have a valid netplayer pointer
	if((pl != NULL) && !(pl->flags & NETINFO_FLAG_RESPAWNING) && !(pl->flags & NETINFO_FLAG_LIMBO)){
		// primary fired
		pl->m_player->ci.fire_primary_count = 0;		

		// secondary fired
		pl->m_player->ci.fire_secondary_count = 0;
		if ( in_flags & OOC_FIRE_SECONDARY ){
			pl->m_player->ci.fire_secondary_count = 1;
		}

		// countermeasure fired		
		pl->m_player->ci.fire_countermeasure_count = 0;		

		// set up aspect locking information
		pl->m_player->locking_on_center = 0;
		if ( in_flags & OOC_LOCKING_ON_CENTER ){
			pl->m_player->locking_on_center = 1;
		}		

		// trigger down, bank info
		if(shipp != NULL){
			if(in_flags & OOC_TRIGGER_DOWN){
				shipp->flags |= SF_TRIGGER_DOWN;
			} else {
				shipp->flags &= ~SF_TRIGGER_DOWN;
			}
			
			if(in_flags & OOC_PRIMARY_BANK){		
				shipp->weapons.current_primary_bank = 1;
			} else {
				shipp->weapons.current_primary_bank = 0;
			}

			// linked or not								
			shipp->flags &= ~SF_PRIMARY_LINKED;
			if(in_flags & OOC_PRIMARY_LINKED){				
				shipp->flags |= SF_PRIMARY_LINKED;
			}
		}

		// other locking information
		if((shipp != NULL) && (shipp->ai_index != -1)){			
			Ai_info[shipp->ai_index].current_target_is_locked = ( in_flags & OOC_TARGET_LOCKED) ? 1 : 0;
			if	( in_flags & OOC_TARGET_SEEK_LOCK ) {
				Ai_info[shipp->ai_index].ai_flags |= AIF_SEEK_LOCK;
			} else {
				Ai_info[shipp->ai_index].ai_flags &= ~AIF_SEEK_LOCK;
			}
		}

		// afterburner status
		if ( (objp != NULL) && (in_flags & OOC_AFTERBURNER_ON) ) {
			Multi_oo_afterburn_hack = true;
		}
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
	if((tobj != NULL) && (pl != NULL) && (pl->m_player->objnum != -1)){
		// assign the target object
		if(Objects[pl->m_player->objnum].type == OBJ_SHIP){
			Ai_info[Ships[Objects[pl->m_player->objnum].instance].ai_index].target_objnum = OBJ_INDEX(tobj);
		}
		pl->s_info.target_objnum = OBJ_INDEX(tobj);

		// assign subsystems if possible					
		if(Objects[pl->m_player->objnum].type == OBJ_SHIP){		
			Ai_info[Ships[Objects[pl->m_player->objnum].instance].ai_index].targeted_subsys = NULL;
			if((t_subsys != -1) && (tobj->type == OBJ_SHIP)){
				Ai_info[Ships[Objects[pl->m_player->objnum].instance].ai_index].targeted_subsys = ship_get_indexed_subsys( &Ships[tobj->instance], t_subsys);
			}
		}

		pl->m_player->locking_subsys = NULL;
		if(Objects[pl->m_player->objnum].type == OBJ_SHIP){		
			if((l_subsys != -1) && (tobj->type == OBJ_SHIP)){
				pl->m_player->locking_subsys = ship_get_indexed_subsys( &Ships[tobj->instance], l_subsys);
			}				
		}
	}				

	return offset;
}

// unpack the object data, return bytes processed
#define UNPACK_PERCENT(v)					{ ubyte temp_byte; memcpy(&temp_byte, data + offset, sizeof(ubyte)); v = (float)temp_byte / 255.0f; offset++;}
int multi_oo_unpack_data(net_player *pl, ubyte *data)
{	
	int offset = 0;		
	object *pobjp;
	ushort net_sig = 0;
	ubyte data_size, oo_flags;
	ubyte seq_num;
	char percent;	
	float fpct;
	ship *shipp;
	ship_info *sip;

	// add the object's net signature, type and oo_flags
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		GET_USHORT( net_sig );		
		GET_DATA( oo_flags );	
	}	
	// clients always pos and orient stuff only
	else {			
		oo_flags = (OO_POS_NEW | OO_ORIENT_NEW);
	}
	GET_DATA( data_size );	
	GET_DATA( seq_num );

	// try and find the object
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		pobjp = multi_get_network_object(net_sig);	
	} else {	
		if((pl != NULL) && (pl->m_player->objnum != -1)){
			pobjp = &Objects[pl->m_player->objnum];
		} else {
			pobjp = NULL;
		}
	}	
	
	// if we can't find the object, set pointer to bogus object to continue reading the data
	// ignore out of sequence packets here as well
	if ( (pobjp == NULL) || (pobjp->type != OBJ_SHIP) || (pobjp->instance < 0) || (pobjp->instance >= MAX_SHIPS) || (Ships[pobjp->instance].ship_info_index < 0) || (Ships[pobjp->instance].ship_info_index >= Num_ship_classes)){		
		offset += data_size;
		return offset;
	}
	
	// ship pointer
	shipp = &Ships[pobjp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	// ---------------------------------------------------------------------------------------------------------------
	// CRITICAL OBJECT UPDATE SHIZ
	// ---------------------------------------------------------------------------------------------------------------
	
	// if the packet is out of order
	if(seq_num < shipp->np_updates[NET_PLAYER_NUM(pl)].seq){
		// non-wraparound case
		if((shipp->np_updates[NET_PLAYER_NUM(pl)].seq - seq_num) <= 100){
			offset += data_size;
			return offset;
		}
	}	

	// make sure the ab hack is reset before we read in new info
	Multi_oo_afterburn_hack = false;

	// if this is from a player, read his button info
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		offset += multi_oo_unpack_client_data(pl, data + offset);		
	}	

	// new info
	vec3d new_pos = pobjp->pos;
	physics_info new_phys_info = pobjp->phys_info;
	matrix new_orient = pobjp->orient;
	
	// position
	if ( oo_flags & OO_POS_NEW ) {						
		// AVERAGE TIME BETWEEN PACKETS FOR THIS SHIP
		// store this latest time stamp
		if(oo_arrive_time_count[shipp - Ships] == 5){
			memmove(&oo_arrive_time[shipp - Ships][0], &oo_arrive_time[shipp - Ships][1], sizeof(float) * 4);
			oo_arrive_time[shipp - Ships][4] = f2fl(Missiontime);
		} else {
			oo_arrive_time[shipp - Ships][oo_arrive_time_count[shipp - Ships]++] = f2fl(Missiontime);
		}
		// if we've got 5 elements calculate the average
		if(oo_arrive_time_count[shipp - Ships] == 5){
			int idx;
			oo_arrive_time_avg_diff[shipp - Ships] = 0.0f;
			for(idx=0; idx<4; idx++){
				oo_arrive_time_avg_diff[shipp - Ships] += oo_arrive_time[shipp - Ships][idx + 1] - oo_arrive_time[shipp - Ships][idx];
			}
			oo_arrive_time_avg_diff[shipp - Ships] /= 5.0f;
		}
		// next expected arrival time
		oo_arrive_time_next[shipp - Ships] = 0.0f;

		// int r1 = multi_pack_unpack_position( 0, data + offset, &pobjp->pos );
		int r1 = multi_pack_unpack_position( 0, data + offset, &new_pos );
		offset += r1;				

		// int r3 = multi_pack_unpack_vel( 0, data + offset, &pobjp->orient, &pobjp->pos, &pobjp->phys_info );
		int r3 = multi_pack_unpack_vel( 0, data + offset, &pobjp->orient, &new_pos, &new_phys_info );
		offset += r3;
		
		// bash desired vel to be velocity
		// pobjp->phys_info.desired_vel = pobjp->phys_info.vel;		
	}	

	// orientation	
	if ( oo_flags & OO_ORIENT_NEW ) {		
		// int r2 = multi_pack_unpack_orient( 0, data + offset, &pobjp->orient );
		int r2 = multi_pack_unpack_orient( 0, data + offset, &new_orient );
		offset += r2;		

		// int r5 = multi_pack_unpack_rotvel( 0, data + offset, &pobjp->orient, &pobjp->pos, &pobjp->phys_info );
		int r5 = multi_pack_unpack_rotvel( 0, data + offset, &new_orient, &new_pos, &new_phys_info );
		offset += r5;

		// bash desired rotvel to be 0
		// pobjp->phys_info.desired_rotvel = vmd_zero_vector;
	}
	
	// forward thrust	
	percent = (char)(pobjp->phys_info.forward_thrust * 100.0f);
	Assert( percent <= 100 );
	GET_DATA(percent);		

	// now stuff all this new info
	if(oo_flags & OO_POS_NEW){
		// if we're past the position update tolerance, bash.
		// this should cause our 2 interpolation splines to be exactly the same. so we'll see a jump,
		// but it should be nice and smooth immediately afterwards
		if(vm_vec_dist(&new_pos, &pobjp->pos) > OO_POS_UPDATE_TOLERANCE){
			pobjp->pos = new_pos;
		}

		// recalc any interpolation info		
		if(oo_interp_count[shipp - Ships] < 2){
			oo_interp_points[shipp - Ships][oo_interp_count[shipp - Ships]++] = new_pos;			
		} else {
			oo_interp_points[shipp - Ships][0] = oo_interp_points[shipp - Ships][1];
			oo_interp_points[shipp - Ships][1] = new_pos;			

			multi_oo_calc_interp_splines(shipp - Ships, &pobjp->pos, &pobjp->orient, &pobjp->phys_info, &new_pos, &new_orient, &new_phys_info);
		}
		
		pobjp->phys_info.vel = new_phys_info.vel;		
		pobjp->phys_info.desired_vel = new_phys_info.vel;
	} 

	// we'll just sim rotation straight. it works fine.
	if(oo_flags & OO_ORIENT_NEW){
		pobjp->orient = new_orient;
		pobjp->phys_info.rotvel = new_phys_info.rotvel;
		// pobjp->phys_info.desired_rotvel = vmd_zero_vector;
		pobjp->phys_info.desired_rotvel = new_phys_info.rotvel;
	}	


	// ---------------------------------------------------------------------------------------------------------------
	// ANYTHING BELOW HERE WORKS FINE - nothing here which causes jumpiness or bandwidth problems :) WHEEEEE!
	// ---------------------------------------------------------------------------------------------------------------
	
	// hull info
	if ( oo_flags & OO_HULL_NEW ){
		UNPACK_PERCENT(fpct);
		pobjp->hull_strength = fpct * Ships[pobjp->instance].ship_max_hull_strength;		

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
	}	

	if ( oo_flags & OO_SUBSYSTEMS_AND_AI_NEW ) {
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

			val = subsystem_percent[subsys_count] * subsysp->max_hits;
			subsysp->current_hits = val;

			// add the value just generated (it was zero'ed above) into the array of generic system types
			subsys_type = subsysp->system_info->type;					// this is the generic type of subsystem
			Assert ( subsys_type < SUBSYSTEM_MAX );
			if (!(subsysp->flags & SSF_NO_AGGREGATE)) {
				shipp->subsys_info[subsys_type].aggregate_current_hits += val;
			}
			subsys_count++;

			// if we've reached max subsystems for some reason, bail out
			if(subsys_count >= n_subsystems){
				break;
			}
		}
		
		// recalculate all ship subsystems
		ship_recalc_subsys_strength( shipp );			

		// ai mode info
		ubyte umode;
		short submode;
		ushort target_signature;
		object *target_objp;

		GET_DATA( umode );
		GET_SHORT( submode );
		GET_USHORT( target_signature );		

		if(shipp->ai_index >= 0){
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

		// primary weapon energy		
		float weapon_energy_pct;
		UNPACK_PERCENT(weapon_energy_pct);
		shipp->weapon_energy = sip->max_weapon_reserve * weapon_energy_pct;		
	}	

	// support ship extra info
	ubyte support_extra;
	GET_DATA(support_extra);
	if(support_extra){
		ushort dock_sig;
		int ai_flags, ai_mode, ai_submode;

		// flag		
		GET_INT(ai_flags);
		GET_INT(ai_mode);
		GET_INT(ai_submode);
		GET_USHORT(dock_sig);		

		// valid ship?							
		if((shipp != NULL) && (shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO)){
			Ai_info[shipp->ai_index].ai_flags = ai_flags;
			Ai_info[shipp->ai_index].mode = ai_mode;
			Ai_info[shipp->ai_index].submode = ai_submode;

			object *objp = multi_get_network_object( dock_sig );
			if(objp != NULL){
				Ai_info[shipp->ai_index].support_ship_objnum = OBJ_INDEX(objp);
			}
		}			
	} 

	// afterburner info
	if ( (oo_flags & OO_AFTERBURNER_NEW) || Multi_oo_afterburn_hack ) {
		// maybe turn them on
		if(!(pobjp->phys_info.flags & PF_AFTERBURNER_ON)){
			afterburners_start(pobjp);
		}

		// make sure the ab hack is reset before we read in new info
		Multi_oo_afterburn_hack = false;
	} else {
		// maybe turn them off
		if(pobjp->phys_info.flags & PF_AFTERBURNER_ON){
			afterburners_stop(pobjp);
		}
	}

	// primary info (only clients care about this)
	if( !MULTIPLAYER_MASTER && (shipp != NULL) ){
		// what bank
		if(oo_flags & OO_PRIMARY_BANK){
			shipp->weapons.current_primary_bank = 1;
		} else {
			shipp->weapons.current_primary_bank = 0;
		}

		// linked or not
		shipp->flags &= ~SF_PRIMARY_LINKED;
		if(oo_flags & OO_PRIMARY_LINKED){
			shipp->flags |= SF_PRIMARY_LINKED;
		}

		// trigger down or not - server doesn't care about this. he'll get it from clients anyway		
		shipp->flags &= ~SF_TRIGGER_DOWN;
		if(oo_flags & OO_TRIGGER_DOWN){
			shipp->flags |= SF_TRIGGER_DOWN;
		}		
	}
	
	// if we're the multiplayer server, set eye position and orient
	if(MULTIPLAYER_MASTER && (pl != NULL) && (pobjp != NULL)){
		pl->s_info.eye_pos = pobjp->pos;
		pl->s_info.eye_orient = pobjp->orient;
	} 		

	// update the sequence #
	shipp->np_updates[NET_PLAYER_NUM(pl)].seq = seq_num;

	// flag the object as just updated
	// pobjp->flags |= OF_JUST_UPDATED;
	
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
		Ships[objp->instance].np_updates[NET_PLAYER_NUM(pl)].update_stamp = timestamp(stamp);
	} 
}

// reset the timestamp appropriately for the passed in object
void multi_oo_reset_status_timestamp(object *objp, int player_index)
{
	Ships[objp->instance].np_updates[player_index].status_update_stamp = timestamp(OO_HULL_SHIELD_TIME);
}

// reset the timestamp appropriately for the passed in object
void multi_oo_reset_subsys_timestamp(object *objp, int player_index)
{
	Ships[objp->instance].np_updates[player_index].subsys_update_stamp = timestamp(OO_SUBSYS_TIME);
}

// determine what needs to get sent for this player regarding the passed object, and when
int multi_oo_maybe_update(net_player *pl, object *obj, ubyte *data)
{
	ubyte oo_flags;
	int stamp;
	int player_index;
	vec3d player_eye;
	vec3d obj_dot;
	float eye_dot, dist;
	int in_cone;
	int range;
	ship *shipp;
	ship_info *sip;
	ushort cur_pos_chksum = 0;
	ushort cur_orient_chksum = 0;

	// if the timestamp has elapsed for this guy, send stuff
	player_index = NET_PLAYER_INDEX(pl);
	if(!(player_index >= 0) || !(player_index < MAX_PLAYERS)){
		return 0;
	}

	// determine what the timestamp is for this object
	if(obj->type == OBJ_SHIP){
		stamp = Ships[obj->instance].np_updates[NET_PLAYER_NUM(pl)].update_stamp;
	} else {
		return 0;
	}

	// stamp hasn't popped yet
	if((stamp != -1) && !timestamp_elapsed_safe(stamp, OO_MAX_TIMESTAMP)){
		return 0;
	}
	
	// if we're supposed to update this guy	

	// get the ship pointer
	shipp = &Ships[obj->instance];

	// get ship info pointer
	sip = NULL;
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

	// base oo_flags
	oo_flags = OO_POS_NEW | OO_ORIENT_NEW;

	// if its a small ship, add weapon link info
	if((sip != NULL) && (sip->flags & (SIF_FIGHTER | SIF_BOMBER))){
		// primary bank 0 or 1
		if(shipp->weapons.current_primary_bank > 0){
			oo_flags |= OO_PRIMARY_BANK;
		}

		// linked or not
		if(shipp->flags & SF_PRIMARY_LINKED){
			oo_flags |= OO_PRIMARY_LINKED;
		}

		// trigger down or not
		if(shipp->flags & SF_TRIGGER_DOWN){
			oo_flags |= OO_TRIGGER_DOWN;
		}
	}	
		
	// if the object's hull/shield timestamp has expired
	if((Ships[obj->instance].np_updates[player_index].status_update_stamp == -1) || timestamp_elapsed_safe(Ships[obj->instance].np_updates[player_index].status_update_stamp, OO_MAX_TIMESTAMP)){
		oo_flags |= (OO_HULL_NEW);

		// reset the timestamp
		multi_oo_reset_status_timestamp(obj, player_index);			
	}

	// if the object's hull/shield timestamp has expired
	if((Ships[obj->instance].np_updates[player_index].subsys_update_stamp == -1) || timestamp_elapsed_safe(Ships[obj->instance].np_updates[player_index].subsys_update_stamp, OO_MAX_TIMESTAMP)){
		oo_flags |= OO_SUBSYSTEMS_AND_AI_NEW;

		// reset the timestamp
		multi_oo_reset_subsys_timestamp(obj, player_index);
	}

	// add info for a targeted object
	if((pl->s_info.target_objnum != -1) && (OBJ_INDEX(obj) == pl->s_info.target_objnum)){
		oo_flags |= (OO_POS_NEW | OO_ORIENT_NEW | OO_HULL_NEW);
	}
	// all other cases
	else {					
		// add info which is contingent upon being "in front"			
		if(in_cone){
			oo_flags |= OO_ORIENT_NEW;
		}						
	}		

	// get current position and orient checksums		
	cur_pos_chksum = cf_add_chksum_short(cur_pos_chksum, (ubyte*)(&obj->pos), sizeof(vec3d));
	cur_orient_chksum = cf_add_chksum_short(cur_orient_chksum, (ubyte*)(&obj->orient), sizeof(matrix));

	// if position or orientation haven't changed	
	if((shipp->np_updates[player_index].pos_chksum != 0) && (shipp->np_updates[player_index].pos_chksum == cur_pos_chksum)){
		// if we otherwise would have been sending it, keep track of it (debug only)
#ifndef NDEBUG
		if(oo_flags & OO_POS_NEW){
			multi_rate_add(player_index, "skp_p", OO_POS_RET_SIZE + OO_VEL_RET_SIZE);
		}		
#endif
		oo_flags &= ~(OO_POS_NEW);
	}
	if((shipp->np_updates[player_index].orient_chksum != 0) && (shipp->np_updates[player_index].orient_chksum == cur_orient_chksum)){
		// if we otherwise would have been sending it, keep track of it (debug only)
#ifndef NDEBUG
		if(oo_flags & OO_ORIENT_NEW){
			multi_rate_add(player_index, "skp_o", OO_ORIENT_RET_SIZE + OO_ROTVEL_RET_SIZE);
		}		
#endif
		oo_flags &= ~(OO_ORIENT_NEW);
	}
	shipp->np_updates[player_index].pos_chksum = cur_pos_chksum;
	shipp->np_updates[player_index].orient_chksum = cur_orient_chksum;

	// pack stuff only if we have to 	
	int packed = multi_oo_pack_data(pl, obj, oo_flags ,data);	

	// increment sequence #
	Ships[obj->instance].np_updates[NET_PLAYER_NUM(pl)].seq++;

	// bytes packed
	return packed;
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

	// if the player has an invalid objnum..
	if(pl->m_player->objnum < 0){
		return;
	}

	object *targ_obj;	

	// build the list of ships to check against
	multi_oo_build_ship_list(pl);

	// do nothing if he has no object targeted, or if he has a weapon targeted
	if((pl->s_info.target_objnum != -1) && (Objects[pl->s_info.target_objnum].type == OBJ_SHIP)){
		// build the header
		BUILD_HEADER(OBJECT_UPDATE);		
	
		// get a pointer to the object
		targ_obj = &Objects[pl->s_info.target_objnum];
	
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
	} else {
		// just build the header for the rest of the function
		BUILD_HEADER(OBJECT_UPDATE);		
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
		add_size = multi_oo_maybe_update(pl, moveup, data_add);

		// if this data is too much for the packet, send off what we currently have and start over
		if(packet_size + add_size > OO_MAX_SIZE){
			stop = 0x00;			
			multi_rate_add(NET_PLAYER_NUM(pl), "stp", 1);
			ADD_DATA(stop);
									
			multi_io_send(pl, data, packet_size);
			pl->s_info.rate_bytes += packet_size + UDP_HEADER_SIZE;

			packet_size = 0;
			BUILD_HEADER(OBJECT_UPDATE);			
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

	// if we have anything more than 3 byte in the packet, send the last one off
	if(packet_size > 3){
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
			if((Net_players[idx].m_player != NULL) && (Net_players[idx].m_player->objnum >= 0) && !(Net_players[idx].flags & NETINFO_FLAG_LIMBO) && !(Net_players[idx].flags & NETINFO_FLAG_RESPAWNING)){
				if((Objects[Net_players[idx].m_player->objnum].flags & OF_PLAYER_SHIP) && !(Objects[Net_players[idx].m_player->objnum].flags & OF_SHOULD_BE_DEAD)){
					obj_player_fire_stuff( &Objects[Net_players[idx].m_player->objnum], Net_players[idx].m_player->ci );
				}
			}
		}
	}
}

// process incoming object update data
void multi_oo_process_update(ubyte *data, header *hinfo)
{	
	ubyte stop;	
	int player_index;	
	int offset = HEADER_LENGTH;
	net_player *pl = NULL;	

	// determine what player this came from 
	player_index = find_player_id(hinfo->id);
	if(player_index != -1){						
		pl = &Net_players[player_index];
	}
	// otherwise its a "regular" object update packet on a client from the server. use "myself" as the reference player
	else {						
		pl = Net_player;
	}

	GET_DATA(stop);
	
	while(stop == 0xff){
		// process the data
		offset += multi_oo_unpack_data(pl, data + offset);

		GET_DATA(stop);
	}
	PACKET_SET_SIZE();
}

// initialize all object update timestamps (call whenever entering gameplay state)
void multi_oo_gameplay_init()
{
	int cur, s_idx, idx;
	//ship_obj *so;				
	ship *shipp;
	/*
	int num_ships = ship_get_num_ships();

	if(num_ships <= 0){
		return;
	}
	split = 3000 / num_ships;
	*/
	//split = 1;
	//split = 150;

	// server should setup initial update timestamps	
	// stagger initial updates over 3 seconds or so
	cur = 0;
	//for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		//if(Objects[so->objnum].type == OBJ_SHIP){
	for(s_idx=0; s_idx<MAX_SHIPS; s_idx++){
			shipp = &Ships[s_idx];
		
			// update the timestamps
			for(idx=0;idx<MAX_PLAYERS;idx++){
				shipp->np_updates[idx].update_stamp = timestamp(cur);
				shipp->np_updates[idx].status_update_stamp = timestamp(cur);
				shipp->np_updates[idx].subsys_update_stamp = timestamp(cur);
				shipp->np_updates[idx].seq = 0;		
				shipp->np_updates[idx].pos_chksum = 0;
				shipp->np_updates[idx].orient_chksum = 0;
			} 
			
			oo_arrive_time_count[shipp - Ships] = 0;			
			oo_interp_count[shipp - Ships] = 0;

			// increment the time
//			cur += split;			
		}
	//}			

	// reset datarate stamp now
	extern int OO_gran;
	for(idx=0; idx<MAX_PLAYERS; idx++){
		Net_players[idx].s_info.rate_stamp = timestamp( (int)(1000.0f / (float)OO_gran) );
	}
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
	
	// build the header
	BUILD_HEADER(OBJECT_UPDATE);		

	// pos and orient always
	oo_flags = (OO_POS_NEW | OO_ORIENT_NEW);		

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

	// increment sequence #
	Player_ship->np_updates[MY_NET_PLAYER_NUM].seq++;

	// send to the server
	if(Netgame.server != NULL){								
		multi_io_send(Net_player, data, packet_size);
	}
}

// Sends a packet from the server to the client, syncing the player's position/orientation to the
// Server's. Allows for use of certain SEXPs in multiplayer.
void multi_oo_send_changed_object(object *changedobj)
{
	ubyte data[MAX_PACKET_SIZE], stop;
	ubyte data_add[MAX_PACKET_SIZE];
	ubyte oo_flags;	
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

	// pos and orient always
	oo_flags = (OO_POS_NEW | OO_ORIENT_NEW);

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

	// increment sequence #
//	Player_ship->np_updates[idx].seq++;

	multi_io_send(&Net_players[idx], data, packet_size);
}


// display any oo info on the hud
void multi_oo_display()
{
#ifndef NDEBUG	
#endif
}


// ---------------------------------------------------------------------------------------------------
// DATARATE DEFINES/VARS
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
DCF(oog, "")
{
	dc_get_arg(ARG_INT);
	OO_gran = Dc_arg_int;
}

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
			if((Net_players[idx].s_info.rate_stamp == -1) || timestamp_elapsed_safe(Net_players[idx].s_info.rate_stamp, OO_MAX_TIMESTAMP) || (abs(timestamp_ticker - Net_players[idx].s_info.rate_stamp) >= (int)(1000.0f / (float)OO_gran)) ){
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

	// high update level - super high datarate (no capping, just intelligent updating)
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
		Int3();
		return;
	}	

	// set the individual client level
	OO_client_rate = (int)(((float)OO_server_rate / (float)OO_gran) / (float)num_connections);
}

// reset all sequencing info (obsolete for new object update stuff)
void multi_oo_reset_sequencing()
{		
}

// is this object one which needs to go through the interpolation
int multi_oo_is_interp_object(object *objp)
{	
	// if not multiplayer, skip it
	if(!(Game_mode & GM_MULTIPLAYER)){
		return 0;
	}

	// if its not a ship, skip it
	if(objp->type != OBJ_SHIP){
		return 0;
	}

	// other bogus cases
	if((objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return 0;
	}

	// if I'm a client and this is not me, I need to interp it
	if(!MULTIPLAYER_MASTER){
		if(objp != Player_obj){
			return 1;
		} else {
			return 0;
		}
	}

	// servers only interpolate other player ships
	if(!(objp->flags & OF_PLAYER_SHIP)){
		return 0;
	}

	// here we know its a player ship - is it mine?
	if(objp == Player_obj){
		return 0;
	}

	// interp it
	return 1;
}

// interp
void multi_oo_interp(object *objp)
{		
	// make sure its a valid ship
	Assert(Game_mode & GM_MULTIPLAYER);
	if(objp->type != OBJ_SHIP){
		return;
	}
	if((objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return;
	}	

	// increment his approx "next" time
	oo_arrive_time_next[objp->instance] += flFrametime;

	// do stream weapon firing for this ship
	Assert(objp != Player_obj);
	if(objp != Player_obj){
		ship_fire_primary(objp, 1, 0);
	}

	// if this ship doesn't have enough data points yet, skip it
	if((oo_interp_count[objp->instance] < 2) || (oo_arrive_time_count[objp->instance] < 5)){
		return;
	}

	// store the magnitude of his velocity
	// float vel_mag = vm_vec_mag(&objp->phys_info.vel);

	// determine how far along we are (0.0 to 1.0) until we should be getting the next packet
	float t = oo_arrive_time_next[objp->instance] / oo_arrive_time_avg_diff[objp->instance];

	// gr_set_color_fast(&Color_bright);
	// gr_printf(100, 10, "%f\n", t);
	
	// we've overshot. hmm. just keep the sim running I guess	
	if(t > 1.0f){
		physics_sim(&objp->pos, &objp->orient, &objp->phys_info, flFrametime);
		return;
	}	

	// otherwise, blend the two curves together to get the new point
	float u = 0.5f + (t * 0.5f);
	vec3d p_bad, p_good;
	oo_interp_splines[objp->instance][0].bez_get_point(&p_bad, u);
	oo_interp_splines[objp->instance][1].bez_get_point(&p_good, u);		
	vm_vec_scale(&p_good, t);
	vm_vec_scale(&p_bad, 1.0f - t);
	vm_vec_add(&objp->pos, &p_bad, &p_good);	

	// set new velocity
	// vm_vec_sub(&objp->phys_info.vel, &objp->pos, &objp->last_pos);
	
	// run the sim for rotation	
	physics_sim_rot(&objp->orient, &objp->phys_info, flFrametime);

	// blend velocity vectors together with an average weight
	/*
	vec3d v_bad, v_good;
	oo_interp_splines[objp->instance][0].herm_get_deriv(&v_bad, u, 0);
	oo_interp_splines[objp->instance][1].herm_get_deriv(&v_good, u, 0);	

	// t -= 1.0f;
	vm_vec_scale(&v_good, t);
	vm_vec_scale(&v_bad, 1.0f - t);
	vm_vec_avg(&objp->phys_info.vel, &v_bad, &v_good);
	
	// run the sim
	physics_sim(&objp->pos, &objp->orient, &objp->phys_info, flFrametime);
	*/

	/*
	vec3d v_bad, v_good;
	oo_interp_splines[objp->instance][0].herm_get_point(&v_bad, u, 0);
	oo_interp_splines[objp->instance][1].herm_get_point(&v_good, u, 0);	

	// t -= 1.0f;
	vm_vec_scale(&v_good, t);
	vm_vec_scale(&v_bad, 1.0f - t);
	vm_vec_avg(&objp->pos, &v_bad, &v_good);	
	
	// run the sim
	// physics_sim(&objp->pos, &objp->orient, &objp->phys_info, flFrametime);
	physics_sim_rot(&objp->orient, &objp->phys_info, flFrametime);
	*/
}

float oo_error = 0.8f;
DCF(oo_error, "")
{
	dc_get_arg(ARG_FLOAT);
	oo_error = Dc_arg_float;
}

void multi_oo_calc_interp_splines(int ship_index, vec3d *cur_pos, matrix *cur_orient, physics_info *cur_phys_info, vec3d *new_pos, matrix *new_orient, physics_info *new_phys_info)
{
	vec3d a, b, c;
	matrix m_copy;
	physics_info p_copy;
	vec3d *pts[3] = {&a, &b, &c};	
	
	// average time between packets
	float avg_diff = oo_arrive_time_avg_diff[ship_index];	

	// would this cause us to rubber-band?
	vec3d v_norm = cur_phys_info->vel;	
	vec3d v_dir;
	vm_vec_sub(&v_dir, new_pos, cur_pos);	
	if(!IS_VEC_NULL_SQ_SAFE(&v_norm) && !IS_VEC_NULL_SQ_SAFE(&v_dir)){
		vm_vec_normalize(&v_dir);
		vm_vec_normalize(&v_norm);	
		if(vm_vec_dotprod(&v_dir, &v_norm) < 0.0f){
			*new_pos = *cur_pos;
		}
	}
	
	// get the spline representing our "bad" movement. its better to be little bit off than to overshoot altogether
	a = oo_interp_points[ship_index][0];
	b = *cur_pos;
	c = *cur_pos;
	m_copy = *cur_orient;
	p_copy = *cur_phys_info;
	physics_sim(&c, &m_copy, &p_copy, avg_diff * oo_error);			// next point, assuming we followed our current path
	oo_interp_splines[ship_index][0].bez_set_points(3, pts);

	// get the spline representing where this new point tells us we'd be heading
	a = oo_interp_points[ship_index][0]; //-V519
	b = oo_interp_points[ship_index][1]; //-V519
	c = oo_interp_points[ship_index][1];
	m_copy = *new_orient;
	p_copy = *new_phys_info;
	physics_sim(&c, &m_copy, &p_copy, avg_diff);			// next point, given this new info
	oo_interp_splines[ship_index][1].bez_set_points(3, pts);	

	// now we've got a spline representing our "new" path and where we would've gone had we been perfect before
	// we'll modify our velocity to move along a blend of these splines.
}

void oo_update_time()
{	
}

int display_oo_bez = 0;
DCF(bez, "")
{
	display_oo_bez = !display_oo_bez;
	if(display_oo_bez){
		dc_printf("Showing interp splines");
	} else {
		dc_printf("Not showing interp splines");
	}
}

void oo_display()
{
/*	int idx;	


	gr_set_color_fast(&Color_bright);

	for(idx=0; idx<MAX_SHIPS; idx++){		
		// invalid ship
		if(Ships[idx].objnum < 0){
			continue;
		}

		// time between updates
		if( (oo_arrive_time_count[idx] == 5) && (idx != (Player_ship - Ships)) ){
			gr_printf(20, 40, "avg time between updates : %f", oo_arrive_time_avg_diff[idx]);			
		}			
		
		// interpolation splines
		if( (oo_interp_count[idx] == 2) && (display_oo_bez) ){
			oo_interp_splines[idx][0].bez_render(10, &Color_bright_red);			// bad path
			oo_interp_splines[idx][1].bez_render(10, &Color_bright_green);		// good path
		}
	}
	*/
}
