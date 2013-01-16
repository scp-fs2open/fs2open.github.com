/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "network/multi_respawn.h"
#include "network/multi.h"
#include "object/object.h"
#include "globalincs/linklist.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "missionui/missionweaponchoice.h"
#include "gamesequence/gamesequence.h"
#include "hud/hudconfig.h"
#include "hud/hudobserver.h"
#include "hud/hudmessage.h"
#include "network/multi_observer.h"
#include "network/multi_team.h"
#include "hud/hudwingmanstatus.h"
#include "mission/missionparse.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "missionui/missionscreencommon.h"
#include "network/multiteamselect.h"
#include "io/timer.h"
#include "iff_defs/iff_defs.h"


// ---------------------------------------------------------------------------------------
// MULTI RESPAWN DEFINES/VARS
//

// respawn notice codes
#define RESPAWN_BROADCAST			0x1		// server to clients - create this ship, and if you're the respawner, set it to be your object
#define RESPAWN_REQUEST				0x2		// client to server requesting a respawn (observer or normal)
#define AI_RESPAWN_NOTICE			0x3		// respawn an AI ship

// struct used to store AI objects which should get respawned
#define MAX_AI_RESPAWNS				MAX_PLAYERS
#define AI_RESPAWN_TIME				(7000)			// should respawn 2.0 seconds after they die

typedef struct ai_respawn
{
	p_object		*pobjp;				// parse object
	int			timestamp;			// timestamp when this object should get respawned
} ai_respawn;

ai_respawn Ai_respawns[MAX_AI_RESPAWNS];			// this is way too many, but I don't care

// respawn point
typedef struct respawn_point {
	char ship_name[NAME_LENGTH+1];					// for priority respawns
	vec3d pos;												// respawn location (non-priority respawns)
	int team;												// team it belongs to
} respawn_point;

// respawn points
#define MAX_MULTI_RESPAWN_POINTS					MAX_PLAYERS
respawn_point Multi_respawn_points[MAX_MULTI_RESPAWN_POINTS];
int Multi_respawn_point_count = 0;
int Multi_next_respawn_point = 0;

// priority ships for respawning
#define MAX_PRIORITY_POINTS						10
respawn_point Multi_respawn_priority_ships[MAX_PRIORITY_POINTS];
int Multi_respawn_priority_count = 0;

// ---------------------------------------------------------------------------------------
// MULTI RESPAWN FORWARD DECLARATIONS
//

// respawn the passed player with the passed ship object and weapon link settings
void multi_respawn_player(net_player *pl, char cur_primary_bank, char cur_secondary_bank, ubyte cur_link_status, ushort ship_ets, ushort net_sign, char *parse_name, vec3d *pos = NULL);

// respawn an AI ship
void multi_respawn_ai(p_object *pobjp);

// respawn myself as an observer
void multi_respawn_as_observer();

// send a request to the server saying I want to respawn (as an observer or not)
void multi_respawn_send_request(int as_observer);

// send a request to respawn an AI ship
void multi_respawn_send_ai_respawn(ushort net_signature);

// send a broadcast pack indicating a player has respawned
void multi_respawn_broadcast(net_player *player);

// <server> make the given player an observer
void multi_respawn_make_observer(net_player *pl);

// place a newly respawned object intelligently
void multi_respawn_place(object *new_obj, int team);

// respawn the server immediately
void multi_respawn_server();

void prevent_spawning_collision(object *new_obj);

// ---------------------------------------------------------------------------------------
// MULTI RESPAWN FUNCTIONS
//

// check to see if a net player needs to be respawned
void multi_respawn_check(object *objp)
{
	int player_index;
	net_player *pl = NULL;
	p_object *pobjp;

	// get the parse object since we are storing all data for the respawns in the parse object
	pobjp = mission_parse_get_arrival_ship( objp->net_signature );
	
	// the server should check against all players
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		player_index = multi_find_player_by_object(objp);
		if(player_index != -1){
			pl = &Net_players[player_index];
		}
	}
	// clients should just check against themselves
	else if(objp == Player_obj){
		pl = Net_player;		
	}	

	// if this ship isn't a player ship, then maybe it is an AI ship which should get respawed.  Only respawn
	// on the server, then send message to respawn on client.
	if( pl == NULL ) {

		// try and find the parse object with this net signature.  If we found it, and it's a player start
		// position, respawn it.
		if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {
			if ( !pobjp ){
				return;
			}

			// if we need to respawn this ai ship, add him to a list of ships to get respawned
			if ( (pobjp->flags & P_OF_PLAYER_START) && (pobjp->respawn_count < Netgame.respawn) && !(Netgame.type_flags & NG_TYPE_DOGFIGHT) ){
				int i;

				for (i = 0; i < MAX_AI_RESPAWNS; i++ ) {
					if ( Ai_respawns[i].pobjp == NULL ) {
						Ai_respawns[i].pobjp = pobjp;
						Ai_respawns[i].timestamp = timestamp(AI_RESPAWN_TIME);
						break;
					}
				}
				Assert( i < MAX_AI_RESPAWNS );
			}
		}

		return;
	} else {
		// reset his datarate timestamp
		extern int OO_gran;
		pl->s_info.rate_stamp = timestamp( (int)(1000.0f / (float)OO_gran) );
	}

	Assert( pl != NULL );
	Assert( pobjp );				// we have a player, and we should have a record of it.
	
	// mark the player as in the state of respawning
	if( (pobjp->respawn_count < Netgame.respawn) || (Netgame.type_flags & NG_TYPE_DOGFIGHT) ){
		pl->flags |= NETINFO_FLAG_RESPAWNING;
	}
	// otherwise mark the player as being in limbo
	else {
		pl->flags |= NETINFO_FLAG_LIMBO;
	}
}

// notify of a player leaving
void multi_respawn_player_leave(net_player *pl)
{
	// bogus
	if(pl == NULL){
		return;
	}
	if( MULTI_OBSERVER((*pl)) ){
		return;
	}
	if((Net_player == NULL) || !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		return;
	}
	if(pl->p_info.p_objp == NULL){
		return;
	}

	// dogfight mode
	if(MULTI_DOGFIGHT){
		return;
	}	

	// if we need to respawn this ai ship, add him to a list of ships to get respawned
	p_object *pobjp = pl->p_info.p_objp;
	if ( (pobjp->flags & P_OF_PLAYER_START) && (pobjp->respawn_count < Netgame.respawn) ){
		int i;

		for (i = 0; i < MAX_AI_RESPAWNS; i++ ) {
			if ( Ai_respawns[i].pobjp == NULL ) {
				Ai_respawns[i].pobjp = pobjp;
				Ai_respawns[i].timestamp = timestamp(AI_RESPAWN_TIME);
				break;
			}
		}
	}
}

// respawn normally
void multi_respawn_normal()
{
	// make sure we should be respawning and _not_ as an observer
	Assert((Net_player->flags & NETINFO_FLAG_RESPAWNING));
	Assert(!(Net_player->flags & NETINFO_FLAG_LIMBO));
	
	// server respawns immediately
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_respawn_server();
	} else {
		// client sends a respawn request (multiple ones are ok if he clicks over and over)
		multi_respawn_send_request(0);
	}
}

// respawn as an observer
void multi_respawn_observer()
{
	// make sure we should be respawning as an observer 
	Assert(!(Net_player->flags & NETINFO_FLAG_RESPAWNING) && (Net_player->flags & NETINFO_FLAG_LIMBO));

	// respawn as an observer
	multi_respawn_as_observer();

	// clients should notify the server that they are doing so
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		multi_respawn_send_request(1);
	}	

	// jump back into the game right away
	gameseq_post_event(GS_EVENT_ENTER_GAME);				
}	

// server should check to see if any respawned players have run out of their invulnerability
void multi_respawn_handle_invul_players()
{
	int idx;
	object *objp;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (Objects[Net_players[idx].m_player->objnum].flags & OF_INVULNERABLE)){	
			// make him normal (_non_ invulnerable) on either of 2 conditions :
			// 1.) More than 5 seconds have passed
			// 2.) He's fired either a primary or a secondary weapon
			if( ((Net_players[idx].s_info.invul_timestamp != -1) && (timestamp_elapsed(Net_players[idx].s_info.invul_timestamp))) ||
				 ((Net_players[idx].m_player->ci.fire_primary_count > 0) || (Net_players[idx].m_player->ci.fire_secondary_count > 0)) ) {
				objp = &Objects[Net_players[idx].m_player->objnum];
				obj_set_flags(objp,objp->flags & ~(OF_INVULNERABLE));
			}
		}
	}
}

// build a list of respawn points for the mission
void multi_respawn_build_points()
{
	ship_obj *moveup;
	respawn_point *r;

	// respawn points
	Multi_respawn_point_count = 0;
	Multi_next_respawn_point = 0;
	moveup = GET_FIRST(&Ship_obj_list);
	while(moveup != END_OF_LIST(&Ship_obj_list)){
		// player ships
		if(Objects[moveup->objnum].flags & (OF_PLAYER_SHIP | OF_COULD_BE_PLAYER)){			
			r = &Multi_respawn_points[Multi_respawn_point_count++];
			
			r->pos = Objects[moveup->objnum].pos;
			r->team = Ships[Objects[moveup->objnum].instance].team;			
		}
		moveup = GET_NEXT(moveup);
	}	

	// priority respawn points
	Multi_respawn_priority_count = 0;
	moveup = GET_FIRST(&Ship_obj_list);
	while(moveup != END_OF_LIST(&Ship_obj_list)){
		// stuff info
		if((Ships[Objects[moveup->objnum].instance].respawn_priority > 0) && (Multi_respawn_priority_count < MAX_PRIORITY_POINTS)){
			r = &Multi_respawn_priority_ships[Multi_respawn_priority_count++];

			strcpy_s(r->ship_name, Ships[Objects[moveup->objnum].instance].ship_name);
			r->team = Ships[Objects[moveup->objnum].instance].team;
		}
		moveup = GET_NEXT(moveup);
	}	
}


// ---------------------------------------------------------------------------------------
// MULTI RESPAWN FORWARD DECLARATIONS
//

void multi_respawn_wing_stuff(ship *shipp)
{
	wing *wingp;

	// deal with re-adding this ship to it's wing
	Assert( shipp->wingnum != -1 );
	wingp = &Wings[shipp->wingnum];
	wingp->ship_index[wingp->current_count] = SHIP_INDEX(shipp);
	wingp->current_count++;

	hud_set_wingman_status_alive(shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
}

int multi_respawn_common_stuff(p_object *pobjp)
{
	int objnum, team, slot_index;
	object *objp;
	ship *shipp;
	int idx;

	// create the object
	objnum = parse_create_object(pobjp);
	Assert(objnum != -1);
	objp = &Objects[objnum];

	// get the team and slot
	shipp = &Ships[objp->instance];
	multi_ts_get_team_and_slot(shipp->ship_name, &team, &slot_index);
	Assert( team != -1 );
	Assert( slot_index != -1 );

	// reset object update stuff
	for(idx=0; idx<MAX_PLAYERS; idx++){
		shipp->np_updates[idx].orient_chksum = 0;
		shipp->np_updates[idx].pos_chksum = 0;
		shipp->np_updates[idx].seq = 0;
		shipp->np_updates[idx].status_update_stamp = -1;
		shipp->np_updates[idx].subsys_update_stamp = -1;
		shipp->np_updates[idx].update_stamp = -1;
	}

	// change the ship type and the weapons
	if (team != -1 && slot_index != -1) {
		change_ship_type(objp->instance, Wss_slots_teams[team][slot_index].ship_class);
		wl_bash_ship_weapons(&shipp->weapons,&Wss_slots_teams[team][slot_index]);
	}
	multi_respawn_wing_stuff( shipp );

	if(Netgame.type_flags & NG_TYPE_TEAM){
		multi_team_mark_ship(&Ships[Objects[objnum].instance]);
	}

	pobjp->respawn_count++;

	return objnum;
}

// respawn the passed player with the passed ship object and weapon link settings
void multi_respawn_player(net_player *pl, char cur_primary_bank, char cur_secondary_bank, ubyte cur_link_status, ushort ship_ets, ushort net_sig, char *parse_name, vec3d *pos)
{
	int objnum;
	object *objp;
	ship *shipp;
	p_object *pobjp;	

	// try and find the parse object
	pobjp = mission_parse_get_arrival_ship(parse_name);		
	Assert(pobjp != NULL);
	if(pobjp == NULL){
		return;
	}
	objnum = multi_respawn_common_stuff(pobjp);

	Assert( objnum != -1 );
	objp = &Objects[objnum];
	shipp = &Ships[objp->instance];	

	// this is a player, so mark him as a player,
	objp->flags |= OF_PLAYER_SHIP;
	objp->flags &= ~OF_COULD_BE_PLAYER;

	// server should mark this player as invulerable for a short time
	if ( MULTIPLAYER_MASTER ) {
		objp->flags |= OF_INVULNERABLE;
		pl->s_info.invul_timestamp = timestamp(RESPAWN_INVUL_TIMESTAMP); 					
		multi_respawn_place( objp, shipp->team );
	}

	// reset his datarate timestamp
	extern int OO_gran;
	pl->s_info.rate_stamp = timestamp( (int)(1000.0f / (float)OO_gran) );

	// set some player information
	pl->m_player->objnum = objnum;
	if ( pl == Net_player ) {
		object *oldplr = Player_obj;

		Player_obj = objp;
		Player_ship = shipp;
		Player_ai = &Ai_info[Player_ship->ai_index];

		// this is a hack to ensure that old (dead) player ships are destroyed, since at this point he's actually an OBJ_GHOST
		oldplr->flags |= OF_SHOULD_BE_DEAD;						
		obj_delete(OBJ_INDEX(oldplr));	

		//	get rid of the annoying HUD dead message text.
		HUD_init_fixed_text();
	}	

	// clients bash net signature
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		objp->net_signature = net_sig;
	}
	
	// restore the correct weapon bank selections
	shipp->weapons.current_primary_bank = (int)cur_primary_bank;
	shipp->weapons.current_secondary_bank = (int)cur_secondary_bank;
	if(cur_link_status & (1<<0)){
		shipp->flags |= SF_PRIMARY_LINKED;
	} else {
		shipp->flags &= ~(SF_PRIMARY_LINKED);
	}			
	if(cur_link_status & (1<<1)){
		shipp->flags |= SF_SECONDARY_DUAL_FIRE;
	} else {
		shipp->flags &= ~(SF_SECONDARY_DUAL_FIRE);
	}

	Assert( ship_ets != 0 );		// find dave or allender

	// restore the correct ets settings
	shipp->shield_recharge_index = ((ship_ets & 0x0f00) >> 8);
	// weapon ets
	shipp->weapon_recharge_index = ((ship_ets & 0x00f0) >> 4);
	// engine ets
	shipp->engine_recharge_index = (ship_ets & 0x000f);

	// give the current bank a half-second timestamp so that we don't fire immediately unpon respawn
	shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank] = timestamp(500);

	// if this is a dogfight mission, make him TEAM_TRAITOR
	if(Netgame.type_flags & NG_TYPE_DOGFIGHT){
		shipp->team = Iff_traitor;
	}

	// maybe bash ship position
	if(pos != NULL){
		objp->pos = *pos;
	}

	// unset his respawning flag
	pl->flags &= ~(NETINFO_FLAG_RESPAWNING | NETINFO_FLAG_LIMBO);

	// blast his control and button info clear
	memset(&pl->m_player->bi, 0, sizeof(pl->m_player->bi));
	memset(&pl->m_player->ci, 0, sizeof(pl->m_player->ci));

	// set throttle based on initial velocity specified in mission (the vel gets calculated
	//   like a percentage of our max speed, so we can just use it as-is for the throttle)
	pl->m_player->ci.forward_cruise_percent = (float)pobjp->initial_velocity;
	CLAMP(pl->m_player->ci.forward_cruise_percent, 0.0f, 100.0f);

	// if this is me, clear accum button info
	if(pl == Net_player){
		// clear multiplayer button info			
		extern button_info Multi_ship_status_bi;
		memset(&Multi_ship_status_bi, 0, sizeof(button_info));
	}

	// notify other players of the respawn
	if ( MULTIPLAYER_MASTER ){
		multi_respawn_broadcast(pl);
	}
}

// respawns an AI ship.
void multi_respawn_ai( p_object *pobjp )
{
	int objnum;
	object *objp;

	// create the object and change the ship type
	objnum = multi_respawn_common_stuff( pobjp );
	objp = &Objects[objnum];

	// be sure the the OF_PLAYER_SHIP flag is unset, and the could be player flag is set
	obj_set_flags( objp, objp->flags | OF_COULD_BE_PLAYER );
	objp->flags &= ~OF_PLAYER_SHIP;
}

// <server> make the given player an observer
void multi_respawn_make_observer(net_player *pl)
{	
	pl->flags |= (NETINFO_FLAG_OBSERVER | NETINFO_FLAG_OBS_PLAYER);		
	pl->flags &= ~(NETINFO_FLAG_RESPAWNING | NETINFO_FLAG_LIMBO);

	// MWA 6/3/98 -- don't set to high -- let it default to whatever player chose 
	//pl->p_info.options.obj_update_level = OBJ_UPDATE_HIGH;
	pl->last_heard_time = timer_get_fixed_seconds();	

	// reset the ping time for this player
	multi_ping_reset(&pl->s_info.ping);
	
	// timestamp his last_full_update_time
	pl->s_info.last_full_update_time = timestamp(0);

	// create an observer object for him
	multi_obs_create_observer(pl);		
}

// respawn myself as an observer
void multi_respawn_as_observer()
{
	// configure the hud to be in "observer" mode
	hud_config_as_observer(Player_ship,Player_ai);	

	// blow away my old player object
	Player_obj->flags |= OF_SHOULD_BE_DEAD;
	obj_delete(OBJ_INDEX(Player_obj));

	// create a new shiny observer object for me
	multi_obs_create_observer(Net_player);
	
	// set my object to be the observer object
	Player_obj = &Objects[Net_player->m_player->objnum];
	Player_ship = &Hud_obs_ship;	
	Player_ai = &Hud_obs_ai;	
	
	// set some flags for myself
	Net_player->flags |= NETINFO_FLAG_OBSERVER;
	Net_player->flags |= NETINFO_FLAG_OBS_PLAYER;
	Net_player->flags &= ~(NETINFO_FLAG_LIMBO);

	// clear my auto-match speed flag
	Net_player->m_player->flags &= ~(PLAYER_FLAGS_AUTO_MATCH_SPEED | PLAYER_FLAGS_MATCH_TARGET);
	
	// reset the control info structure
	memset(&Player->ci,0,sizeof(control_info));	
}

// send a request to respawn an AI object
void multi_respawn_send_ai_respawn( ushort net_signature )
{
	ubyte data[50],val;
	int packet_size = 0;

	// build the header and add the opcode
	BUILD_HEADER(RESPAWN_NOTICE);
	val = AI_RESPAWN_NOTICE;
	ADD_DATA(val);
	ADD_USHORT( net_signature );

	// broadcast the packet to all players
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);
	multi_io_send_to_all_reliable(data, packet_size);	
}

// send a request to the server saying I want to respawn (as an observer or not)
void multi_respawn_send_request(int as_observer)
{
	ubyte data[10],val;
	int packet_size = 0;

	// build the header and add the opcode
	BUILD_HEADER(RESPAWN_NOTICE);
	val = RESPAWN_REQUEST;
	ADD_DATA(val);

	// add a byte indicating whether or not we want to respawn as an observer
	val = (ubyte)as_observer;
	ADD_DATA(val);

	// send the request to the server	
	multi_io_send_reliable(Net_player, data, packet_size);
}

// send a broadcast pack indicating a player has respawned
void multi_respawn_broadcast(net_player *np)
{
	ubyte data[50],val;
	int packet_size = 0;
	ushort signature;
	vec3d pos;

	// broadcast the packet to all players
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);

	signature = Objects[np->m_player->objnum].net_signature;
	pos = Objects[np->m_player->objnum].pos;

	// build the header and add the opcode
	BUILD_HEADER(RESPAWN_NOTICE);
	val = RESPAWN_BROADCAST;
	ADD_DATA(val);

	// add the data for the respawn
	ADD_USHORT(signature);
	ADD_VECTOR(pos);
	ADD_SHORT(np->player_id);
	ADD_DATA(np->s_info.cur_primary_bank);
	ADD_DATA(np->s_info.cur_secondary_bank);
	ADD_DATA(np->s_info.cur_link_status);
	ADD_USHORT(np->s_info.ship_ets);
	ADD_STRING(np->p_info.p_objp->name);

	Assert( np->s_info.ship_ets != 0 );		// find dave or allender

	multi_io_send_to_all_reliable(data, packet_size);
}

// process an incoming respawn info packet
void multi_respawn_process_packet(ubyte *data, header *hinfo)
{
	ubyte code,cur_link_status;
	char cur_primary_bank,cur_secondary_bank;
	ushort net_sig,ship_ets;
	short player_id;
	int player_index;
	vec3d v;	
	char parse_name[1024] = "";
	int offset = HEADER_LENGTH;

	// determine who send the packet	
	player_index = find_player_id(hinfo->id);
	if(player_index == -1){
		nprintf(("Network","Couldn't find player for processing respawn packet!\n"));
	}

	// get the opcode
	GET_DATA(code);

	// do something based upon the opcode
	switch((int)code){

	case AI_RESPAWN_NOTICE: 
		p_object *pobjp;

		GET_USHORT( net_sig );
		pobjp = mission_parse_get_arrival_ship( net_sig );
		Assert( pobjp != NULL );
		multi_respawn_ai( pobjp );
		break;		

	case RESPAWN_BROADCAST:
		// get the respawn data
		GET_USHORT(net_sig);
		GET_VECTOR(v);
		GET_SHORT(player_id);
		GET_DATA(cur_primary_bank);
		GET_DATA(cur_secondary_bank);
		GET_DATA(cur_link_status);
		GET_USHORT(ship_ets);
		GET_STRING(parse_name);
		player_index = find_player_id(player_id);
		if(player_index == -1){
			nprintf(("Network","Couldn't find player to respawn!\n"));
			break;
		}

		// create the ship and assign its position, net_signature, and class
		// respawn the player
		multi_respawn_player(&Net_players[player_index], cur_primary_bank, cur_secondary_bank, cur_link_status, ship_ets, net_sig, parse_name, &v);

		// if this is for me, I should jump back into gameplay
		if(&Net_players[player_index] == Net_player){
			gameseq_post_event(GS_EVENT_ENTER_GAME);
		}
		break;
	
	case RESPAWN_REQUEST:
		// determine whether he wants to respawn as an observer or not
		GET_DATA(code);

		if(player_index == -1){
			nprintf(("Network","Received respawn request from unknown player!\n"));
			break;
		} 		     		
		nprintf(("Network","Received respawn request for player %s\n", Net_players[player_index].m_player->callsign));

		// make sure he's not making an invalid request
		if((code == 0) && !(Net_players[player_index].flags & NETINFO_FLAG_RESPAWNING)){
			nprintf(("Network","This player shouldn't be respawning!\n"));
			Int3();
			break;
		} else if((code == 1) && !(Net_players[player_index].flags & NETINFO_FLAG_LIMBO)){
			nprintf(("Network","This is a respawn observer request from a player who shouldn't be respawning as an observer!\n"));
			Int3();
			break;
		}

		// otherwise perform the operation
		// respawn the guy as an observer
		if(code){
			multi_respawn_make_observer(&Net_players[player_index]);			
		}
		// respawn him as normal
		else {						
			// create his new ship, and change him from respawning to respawned
			Assert(Net_players[player_index].p_info.p_objp != NULL);
			if(Net_players[player_index].p_info.p_objp != NULL){
				multi_respawn_player(&Net_players[player_index], Net_players[player_index].s_info.cur_primary_bank, Net_players[player_index].s_info.cur_secondary_bank,Net_players[player_index].s_info.cur_link_status, Net_players[player_index].s_info.ship_ets, 0, Net_players[player_index].p_info.p_objp->name);
			}			
		}	
		break;
	}

	PACKET_SET_SIZE();
}

// respawn the server immediately
void multi_respawn_server()
{	
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);

	// respawn me
	multi_respawn_player(Net_player, Net_player->s_info.cur_primary_bank, Net_player->s_info.cur_secondary_bank, Net_player->s_info.cur_link_status, Net_player->s_info.ship_ets, 0, Net_player->p_info.p_objp->name);

	// jump back into the game
	gameseq_post_event(GS_EVENT_ENTER_GAME);	
}

// level init for respawn stuff
void multi_respawn_init()
{
	int i;

	for (i = 0; i < MAX_AI_RESPAWNS; i++ ) {
		Ai_respawns[i].pobjp = NULL;
		Ai_respawns[i].timestamp = timestamp(-1);
	}
}

// function to detect whether or not we have AI ships to respawn this frame
void multi_respawn_check_ai()
{
	int i;

	for (i = 0; i < MAX_AI_RESPAWNS; i++ ) {
		if ( Ai_respawns[i].pobjp != NULL ) {
			if ( timestamp_elapsed(Ai_respawns[i].timestamp) ) {

				// be sure that ship is actually gone before respawning it.
				if ( ship_name_lookup(Ai_respawns[i].pobjp->name) != -1 ) {
					Ai_respawns[i].timestamp = timestamp(1000);
				} else {
					multi_respawn_ai( Ai_respawns[i].pobjp );
					multi_respawn_send_ai_respawn( Ai_respawns[i].pobjp->net_signature );
					Ai_respawns[i].pobjp = NULL;
					Ai_respawns[i].timestamp = timestamp(-1);
				}
			}
		}
	}
}



// this is a completely off the cuff way of doing things. Feel free to find a better way.
// Currently :
// 1. Take the average vector position of all the ships in the game
// 2. Check to make sure we aren't within the radius of any of the ships in the game
//    a.) If we are, move away along the vector between the two ships (by the radius of the ship it collided with)
//    b.) repeat step 2

void multi_respawn_place(object *new_obj, int team)
{
	ship *pri = NULL;
	object *pri_obj = NULL;
	int idx, lookup;

	// first determine if there are any appropriate priority ships to use
	pri = NULL;
	pri_obj = NULL;
	for(idx=0; idx<Multi_respawn_priority_count; idx++){
		// all relevant ships
		if((Multi_respawn_priority_ships[idx].team == team) || !(Netgame.type_flags & NG_TYPE_TEAM)){

			lookup = ship_name_lookup(Multi_respawn_priority_ships[idx].ship_name);
			if( (lookup >= 0) && ((pri == NULL) || (Ships[lookup].respawn_priority > pri->respawn_priority)) && (Ships[lookup].objnum >= 0) && (Ships[lookup].objnum < MAX_OBJECTS)){
				pri = &Ships[lookup];
				pri_obj = &Objects[Ships[lookup].objnum];
			}
		}
	}
	
	// if we have a relevant respawn ship
	if((pri != NULL) && (pri_obj != NULL)){
		// pick a point just outside his bounding box
		polymodel *pm = model_get(Ship_info[pri->ship_info_index].model_num); 

		// hmm, ugly. Pick a point 2000 meters to the y direction
		if(pm == NULL){			
			vm_vec_scale_add(&new_obj->pos, &pri_obj->pos, &pri_obj->orient.vec.rvec, 2000.0f);
		} else {
			// pick a random direction
			int d = (int)frand_range(0.0f, 5.9f);
			switch(d){
			case 0:
				vm_vec_scale_add(&new_obj->pos, &pri_obj->pos, &pri_obj->orient.vec.rvec, (pm->maxs.xyz.x - pm->mins.xyz.x)); 
				break;

			case 1:
				vm_vec_scale_add(&new_obj->pos, &pri_obj->pos, &pri_obj->orient.vec.rvec, -(pm->maxs.xyz.x - pm->mins.xyz.x)); 
				break;

			case 2:
				vm_vec_scale_add(&new_obj->pos, &pri_obj->pos, &pri_obj->orient.vec.uvec, (pm->maxs.xyz.y - pm->mins.xyz.y)); 
				break;

			case 3:
				vm_vec_scale_add(&new_obj->pos, &pri_obj->pos, &pri_obj->orient.vec.uvec, -(pm->maxs.xyz.y - pm->mins.xyz.y)); 
				break;

			case 4:
				vm_vec_scale_add(&new_obj->pos, &pri_obj->pos, &pri_obj->orient.vec.fvec, (pm->maxs.xyz.z - pm->mins.xyz.z)); 
				break;

			case 5:
				vm_vec_scale_add(&new_obj->pos, &pri_obj->pos, &pri_obj->orient.vec.fvec, -(pm->maxs.xyz.z - pm->mins.xyz.z)); 
				break;

			default:
				vm_vec_scale_add(&new_obj->pos, &pri_obj->pos, &pri_obj->orient.vec.uvec, -(pm->maxs.xyz.y - pm->mins.xyz.y)); 
				break;
			}
		}
	}
	// otherwise, resort to plain respawn points
	else {
		Assert(Multi_respawn_point_count > 0);
		
		// get the next appropriate respawn point by team
		lookup = 0;		
		int count = 0;
		while(!lookup && (count < 13)){
			if((team == Iff_traitor) || (team == Multi_respawn_points[Multi_next_respawn_point].team)){
				lookup = 1;
			}			

			// next item
			if(!lookup){
				if(Multi_next_respawn_point >= (Multi_respawn_point_count-1)){
					Multi_next_respawn_point = 0;
				} else {
					Multi_next_respawn_point++;
				}				
			}

			count++;
		}

		// set respawn info
		new_obj->pos = Multi_respawn_points[Multi_next_respawn_point].pos;		
	}

	// now make sure we're not colliding with anyone
	prevent_spawning_collision(new_obj);
}


/*
#define MOVE_AWAY() { vec3d away; vm_vec_sub(&away,&new_obj->pos,&hit_check->pos); \
	                   vm_vec_normalize_quick(&away); vm_vec_scale(&away,hit_check->radius+hit_check->radius); \
							 vm_vec_add2(&new_obj->pos,&away); }

#define WITHIN_RADIUS() { float dist; dist=vm_vec_dist(&new_obj->pos,&hit_check->pos); \
	                       if(dist <= hit_check->radius) collided = 1; }
*/

#define WITHIN_BBOX()	do { \
	if (pm != NULL) { \
		float scale = 2.0f; \
		collided = 0; \
		vec3d temp = new_obj->pos; \
		vec3d gpos; \
		vm_vec_sub2(&temp, &hit_check->pos); \
		vm_vec_rotate(&gpos, &temp, &hit_check->orient); \
		if((gpos.xyz.x >= pm->mins.xyz.x * scale) && (gpos.xyz.y >= pm->mins.xyz.y * scale) && (gpos.xyz.z >= pm->mins.xyz.z * scale) && (gpos.xyz.x <= pm->maxs.xyz.x * scale) && (gpos.xyz.y <= pm->maxs.xyz.y * scale) && (gpos.xyz.z <= pm->maxs.xyz.z * scale)) { \
			collided = 1; \
		} \
	} \
} while(0)

#define MOVE_AWAY_BBOX() do { \
	if (pm != NULL) { \
		switch((int)frand_range(0.0f, 3.9f)){ \
		case 0: \
			new_obj->pos.xyz.x += 200.0f; \
			break; \
		case 1: \
			new_obj->pos.xyz.x -= 200.0f; \
			break; \
		case 2: \
			new_obj->pos.xyz.y += 200.0f; \
			break; \
		case 3: \
			new_obj->pos.xyz.y -= 200.0f; \
			break; \
		default : \
			new_obj->pos.xyz.z -= 200.0f; \
			break; \
		} \
	} \
} while(0)


void prevent_spawning_collision(object *new_obj)
{
	int collided;
	ship_obj *moveup;
	object *hit_check;
	ship *s_check;

	do {
		collided = 0;

		for (moveup = GET_FIRST(&Ship_obj_list); moveup != END_OF_LIST(&Ship_obj_list); moveup = GET_NEXT(moveup))
		{
			// don't check the new object itself!!
			if (moveup->objnum == OBJ_INDEX(new_obj))
				continue;

			hit_check = &Objects[moveup->objnum];

			Assert(hit_check->type == OBJ_SHIP);
			Assert(hit_check->instance >= 0);
			if ((hit_check->type != OBJ_SHIP) || (hit_check->instance < 0))
				continue;

			s_check = &Ships[hit_check->instance];
							
			// just to make sure we don't get any strange magnitude errors
			if (vm_vec_same(&hit_check->pos, &new_obj->pos))
				new_obj->pos.xyz.x += 1.0f;
							
			polymodel *pm = model_get(Ship_info[s_check->ship_info_index].model_num);
			WITHIN_BBOX();				
			if (collided)
			{
				MOVE_AWAY_BBOX();
				break;
			}
		}
	} while (collided);
}
