/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#if defined _WIN32
#include <winsock.h>
#elif defined SCP_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#endif
#include <ctype.h>

#include "globalincs/pstypes.h"
#include "network/multiutil.h"
#include "globalincs/linklist.h"
#include "gamesequence/gamesequence.h"
#include "hud/hudmessage.h"
#include "freespace2/freespace.h"
#include "io/key.h"
#include "io/timer.h"
#include "ship/ship.h"
#include "globalincs/alphacolors.h"
#include "graphics/font.h"
#include "gamesnd/gamesnd.h"
#include "playerman/player.h"
#include "mission/missionparse.h"
#include "missionui/missionshipchoice.h"
#include "network/stand_gui.h"
#include "ship/shipfx.h"
#include "object/object.h"
#include "playerman/managepilot.h"
#include "missionui/missiondebrief.h"
#include "observer/observer.h"
#include "mission/missionmessage.h"
#include "popup/popup.h"
#include "popup/popupdead.h"
#include "hud/hudconfig.h"
#include "menuui/optionsmenu.h"
#include "mission/missionhotkey.h"
#include "mission/missiongoals.h"
#include "ship/afterburner.h"
#include "missionui/chatbox.h"
#include "osapi/osregistry.h"
#include "hud/hudescort.h"
#include "network/multi.h"
#include "cmdline/cmdline.h"
#include "cfile/cfile.h"
#include "cfile/cfilesystem.h"
#include "network/multimsgs.h"
#include "network/multi_xfer.h"
#include "network/multiteamselect.h"
#include "network/multiui.h"
#include "network/multi_kick.h"
#include "network/multi_data.h"
#include "network/multi_voice.h"
#include "network/multi_team.h"
#include "network/multi_respawn.h"
#include "network/multi_ingame.h"
#include "network/multi_observer.h"
#include "network/multi_pinfo.h"
#include "network/multi_endgame.h"
#include "network/multi_pmsg.h"
#include "network/multi_pause.h"
#include "network/multi_log.h"
#include "network/multi_rate.h"
#include "fs2netd/fs2netd_client.h"
#include "parse/parselo.h"

extern int ascii_table[];
extern int shifted_ascii_table[];

// network object management
ushort Next_ship_signature;										// next permanent network signature to assign to an object
ushort Next_asteroid_signature;									// next signature for an asteroid
ushort Next_non_perm_signature;									// next non-permanent network signature to assign to an object
ushort Next_debris_signature;										// next debris signature


// if a client doesn't receive an update for an object after this many seconds, query server
// as to the objects status.
#define MULTI_CLIENT_OBJ_TIMEOUT		10
#define MAX_SHIPS_PER_QUERY			10


// this function assignes the given object with the given signature.  If the signature is 0, then we choose
// the next signature number from the correct pool.  I thought that it might be desireable
// to not always have to take the next signature on the list.  what_kind is used to assign either a
// permanent or non-permanent signature to an object.  permanent signatures are used for ships, non_permanent
// signatures are used for everything else.
ushort multi_assign_network_signature( int what_kind )
{
	ushort sig;	

	// do limit checking on the permanent and non_permanent signatures.  Ships are considered "permanent"
	// as are debris and asteroids since they don't die very often.  It would be vary rare for this
	// value (the permanent signature) to wrap.  For now, this condition is an error condition
	if ( what_kind == MULTI_SIG_SHIP ) {
		if ( Next_ship_signature < SHIP_SIG_MIN ){
			Next_ship_signature = SHIP_SIG_MIN;
		}

		sig = Next_ship_signature++;

		if ( Next_ship_signature == SHIP_SIG_MAX ) {
			Int3();			// get Allender -- signature stuff wrapped.
			Next_ship_signature = SHIP_SIG_MIN;
		}

		// signature stuff for asteroids.
	} else if ( what_kind == MULTI_SIG_ASTEROID ) {
		if ( Next_asteroid_signature < ASTEROID_SIG_MIN ){
			Next_asteroid_signature = ASTEROID_SIG_MIN;
		}

		sig = Next_asteroid_signature++;
		if ( Next_asteroid_signature == ASTEROID_SIG_MAX ) {
			Int3();			// get Allender -- signature stuff wrapped.
			Next_asteroid_signature = ASTEROID_SIG_MIN;
		}

		// signatures for debris
	} else if ( what_kind == MULTI_SIG_DEBRIS ) {
		if ( Next_debris_signature < DEBRIS_SIG_MIN ){
			Next_debris_signature = DEBRIS_SIG_MIN;
		}

		sig = Next_debris_signature++;
		if ( Next_debris_signature == DEBRIS_SIG_MAX ) {
			Int3();			// get Allender -- signature stuff wrapped.
			Next_debris_signature = DEBRIS_SIG_MIN;
		}

		// signature stuff for weapons and other expendable things.
	} else if ( what_kind == MULTI_SIG_NON_PERMANENT ) {
		if ( Next_non_perm_signature < NPERM_SIG_MIN ){
			Next_non_perm_signature = NPERM_SIG_MIN;
		}

		sig = Next_non_perm_signature++;
		if ( (Next_non_perm_signature < NPERM_SIG_MIN) || (Next_non_perm_signature == NPERM_SIG_MAX) ) {
			Next_non_perm_signature = NPERM_SIG_MIN;
		}
	} else {
		Int3();		// get allender - Illegal signature type requested
		sig = 0;
	}

	return sig;
}

// this function returns the next network signature that will be used for a newly created object
// what_kind parameter tells us what kind of signature to get -- permanent or non-permanent
ushort multi_get_next_network_signature( int what_kind )
{
	if ( what_kind == MULTI_SIG_SHIP ) {
		if ( Next_ship_signature < SHIP_SIG_MIN )
			Next_ship_signature = SHIP_SIG_MIN;
		return Next_ship_signature;

	} else if ( what_kind == MULTI_SIG_DEBRIS ) {
		if ( Next_debris_signature < DEBRIS_SIG_MIN)
			Next_debris_signature = DEBRIS_SIG_MIN;
		return Next_debris_signature;

	} else if ( what_kind == MULTI_SIG_ASTEROID ) {
		if ( Next_asteroid_signature < ASTEROID_SIG_MIN )
			Next_asteroid_signature = ASTEROID_SIG_MIN;
		return Next_asteroid_signature;

	} else if ( what_kind == MULTI_SIG_NON_PERMANENT ) {
		if ( Next_non_perm_signature < NPERM_SIG_MIN )
			Next_non_perm_signature = NPERM_SIG_MIN;
		return Next_non_perm_signature;

	} else {
		Int3();			// get allender
		return 0;
	}
}

// this routine sets the network signature to the given value.  Should be called from client only
// and is used mainly for firing weapons.  what_kind tells us permanent or non-permanent signature
void multi_set_network_signature( ushort signature, int what_kind )
{
	Assert( signature != 0 );

	if ( what_kind == MULTI_SIG_SHIP ) {
		Assert( (signature >= SHIP_SIG_MIN) && (signature <= SHIP_SIG_MAX) );
		Next_ship_signature = signature;
	} else if ( what_kind == MULTI_SIG_DEBRIS ) {
		Assert( (signature >= DEBRIS_SIG_MIN) && (signature <= DEBRIS_SIG_MAX) );
		Next_debris_signature = signature;
	} else if ( what_kind == MULTI_SIG_ASTEROID ) {
		Assert( (signature >= ASTEROID_SIG_MIN) && (signature <= ASTEROID_SIG_MAX) );
		Next_asteroid_signature = signature;
	} else if ( what_kind == MULTI_SIG_NON_PERMANENT ) {
		Assert( (signature >= NPERM_SIG_MIN) /*&& (signature <= NPERM_SIG_MAX)*/ );
		Next_non_perm_signature = signature;
	} else
		Int3();			// get Allender
}

// multi_get_network_object() takes a net_signature and tries to locate the object in the object list
// with that network signature.  Returns NULL if the object cannot be found
object *multi_get_network_object( ushort net_signature )
{
	object *objp;

	if ( net_signature == 0 )
		return NULL;

	if(GET_FIRST(&obj_used_list) == NULL)
		return NULL;

	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
		if ( objp->net_signature == net_signature )
			break;

	// if not found on used list, check create list
	if ( objp == END_OF_LIST(&obj_used_list) ) {
		for ( objp = GET_FIRST(&obj_create_list); objp != END_OF_LIST(&obj_create_list); objp = GET_NEXT(objp) )
			if ( objp->net_signature == net_signature )
				break;

		if ( objp == END_OF_LIST(&obj_create_list) )
			objp = NULL;
	}

	return objp;
}


// -------------------------------------------------------------------------------------------------
// netmisc_calc_checksum() calculates the checksum of a block of memory.
//
//
ushort netmisc_calc_checksum( void * vptr, int len )
{
	ubyte * ptr = (ubyte *)vptr;
	unsigned int sum1,sum2;

	sum1 = sum2 = 0;

	while(len--)	{
		sum1 += *ptr++;
		if (sum1 >= 255 ) sum1 -= 255;
		sum2 += sum1;
	}
	sum2 %= 255;
	
	return (unsigned short)((sum1<<8)+ sum2);
}


// -------------------------------------------------------------------------------------------------
//	multi_random_death_word() will return a word from the below list at random.
//
//	Note:  Keep words grouped into sections of 10

#define NUM_DEATH_WORDS	40

char *multi_random_death_word()
{
	int index;

	index = rand() % NUM_DEATH_WORDS;
	switch (index) {
		case 0:
			return XSTR("zapped",853);
		case 1:
			return XSTR("caulked",854);
		case 2:
			return XSTR("killed",855);
		case 3:
			return XSTR("waxed",856);
		case 4:
			return XSTR("popped",857);
		case 5:
			return XSTR("murdered",858);
		case 6:
			return XSTR("bludgeoned",859);
		case 7:
			return XSTR("destroyed",860);
		case 8:
			return XSTR("iced",861);
		case 9:
			return XSTR("obliterated",862);
		case 10:
			return XSTR("toasted",863);
		case 11:
			return XSTR("roasted",864);
		case 12:
			return XSTR("turned into anti-matter",865);
		case 13:
			return XSTR("killed like a pig",866);
		case 14:
			return XSTR("taught a lesson",867);
		case 15:
			return XSTR("slaughtered with impunity",868);
		case 16:
			return XSTR("spanked like a naughty boy",869);
		case 17:
			return XSTR("skunked",870);
		case 18:
			return XSTR("beat senseless",871);
		case 19:
			return XSTR("shot up",872);
		case 20:
			return XSTR("spaced",873);
		case 21:
			return XSTR("hosed",874);
		case 22:
			return XSTR("capped",875);
		case 23:
			return XSTR("beat down",876);
		case 24:
			return XSTR("hit wit da shizzo",877);
		case 25:
			return XSTR("sk00led",878);
		case 26:
			return XSTR("whooped up",879);
		case 27:
			return XSTR("brought to the all-you-can-take whoop ass buffet",880);
		case 28:
			return XSTR("served up a whoop ass sandwich...hold the mercy",881);
		case 29:
			return XSTR("gibbed by Kayser Sozay's rocket",882);
		case 30:
			return XSTR("shot down",883);
		case 31:
			return XSTR("given early retirement",884);
		case 32:
			return XSTR("instructed",885);
		case 33:
			return XSTR("eviscerated",886);
		case 34:
			return XSTR("pummelled",887);
		case 35:
			return XSTR("eradicated",888);
		case 36:
			return XSTR("cleansed",889);
		case 37:
			return XSTR("perforated",890);
		case 38:
			return XSTR("canned",891);
		case 39:
			return XSTR("decompressed",892);
	}

	return NOX("Error");
}

// -------------------------------------------------------------------------------------------------
//	multi_random_chat_start() will return a word from the below list at random.
//
//

#define NUM_CHAT_START_WORDS	8
#define MAX_CHAT_PHRASE_LEN	25		// be careful not to exceed (or update if exceeded)

char *multi_random_chat_start()
{
	int index;

	index = rand() % NUM_CHAT_START_WORDS;
	switch (index) {
		case 0:
			return XSTR("says",893);
		case 1:
			return XSTR("bleats",894);
		case 2:
			return XSTR("opines",895);
		case 3:
			return XSTR("postulates",896);
		case 4:
			return XSTR("declares",897);
		case 5:
			return XSTR("vomits out",898);
		case 6:
			return XSTR("whines",899);
		case 7:
			return XSTR("barks",900);
	}

	return NOX("Error");
}

// -------------------------------------------------------------------------------------------------
//	multi_ship_class_lookup() will return the Ship_info[] index for the ship specified as a 
// parameter.
//
//

int multi_ship_class_lookup(char* ship_name)
{
	int i, player_ship_class;

	// find the ship_info index for the ship_name

	player_ship_class = -1;
	for (i = 0; i < Num_ship_classes; i++) {
		if ( !stricmp(Ship_info[i].name, ship_name) ) {
			player_ship_class = i;
			break;
		}
	} // end for 

	if (i == Num_ship_classes){
		return -1;
	}

	return player_ship_class;
}

// -------------------------------------------------------------------------------------------------
//	find_player() is called when a packet arrives, and we need to know which net player to update.
// The matching is done based on the address and port.  Port checking is done in case multiple
// instances of FreeSpace are running on the same box.
//
//

int find_player( net_addr* addr )
{
	int i;

	for (i = 0; i < MAX_PLAYERS; i++ ) {
		if ( !MULTI_CONNECTED(Net_players[i])){
			continue;
		}
		if ( psnet_same( addr, &(Net_players[i].p_info.addr)) ){
			return i;
		}
	}

	return -1;
}

// so that we can lookup on the admin port transparently
int find_player_no_port(net_addr *addr)
{
	int i;
	int len;

	for (i = 0; i < MAX_PLAYERS; i++ ) {
		if ( !MULTI_CONNECTED(Net_players[i])){
			continue;
		}
		if(addr->type == NET_IPX){
			len = 6;
		} else { 
			len = 4;
		}
		if ( memcmp(&addr->addr,&Net_players[i].p_info.addr.addr,len)== 0){
			return i;
		}
	}

	return -1;
}

int find_player_id(short player_id)
{
	int i;
	for (i = 0; i < MAX_PLAYERS; i++ ) {
		if ( !MULTI_CONNECTED(Net_players[i])){
			continue;
		}
		if(player_id == Net_players[i].player_id){
			return i;
		}
	}	

	// couldn't find the player
	return -1;
}

// note this is only valid to do on a server!
int find_player_socket(PSNET_SOCKET_RELIABLE sock)
{
	int i;
	for (i = 0; i < MAX_PLAYERS; i++ ) {
		if ( !MULTI_CONNECTED(Net_players[i])){
			continue;
		}
		if(sock == Net_players[i].reliable_socket){
			return i;
		}
	}	

	// couldn't find the player
	return -1;
}

// multi_find_player_by_object returns a player num (reference by Net_players[x]) when given a object *.
// used to find netplayers in game when only the object is known
int multi_find_player_by_object( object *objp )
{
	int i, objnum;

	objnum = OBJ_INDEX(objp);
	for (i = 0; i < MAX_PLAYERS; i++ ) {
		if ( !MULTI_CONNECTED(Net_players[i])){
			continue;
		}
		if ( objnum == Net_players[i].m_player->objnum ){
			return i;
		}
	}

	// return -1 if the object is not found -- this is a bad situation, but we will handle it in higher
	// level code
	return -1;
}

// returns a player num based upon player object signature
int multi_find_player_by_signature( int signature )
{
	int idx;

	for(idx=0;idx<MAX_PLAYERS;idx++){
		// compare against each player's object signature
		if(MULTI_CONNECTED(Net_players[idx]) && (Objects[Net_players[idx].m_player->objnum].signature == signature)){
			// found the player
			return idx;
		}
	}

	// didn't find the player
	return -1;
}

// returns a player num based upon object net signature
int multi_find_player_by_net_signature(ushort net_signature)
{
	int idx;

	for(idx=0;idx<MAX_PLAYERS;idx++){
		// compare against each player's object signature
		if(MULTI_CONNECTED(Net_players[idx]) && (Objects[Net_players[idx].m_player->objnum].net_signature == net_signature)){
			// found the player
			return idx;
		}
	}

	// didn't find the player
	return -1;
}

// returns a player num based upon it's parse_object unlike the above functions it can be used on respawning players
int multi_find_player_by_parse_object( p_object *p_objp )
{
	int idx;

	for(idx=0;idx<MAX_PLAYERS;idx++){
		// compare against each player's object signature
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].p_info.p_objp == p_objp) ){
			// found the player
			return idx;
		}
	}

	// didn't find the player
	return -1;
}

int multi_find_player_by_ship_name(char *ship_name, bool inc_respawning)
{
	int idx;
	p_object *p_objp;

	// bogus
	if(ship_name == NULL){
		return -1;
	}

	for(idx=0; idx<MAX_PLAYERS; idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx]) && (Net_players[idx].m_player != NULL) && (Net_players[idx].m_player->objnum >= 0) && (Net_players[idx].m_player->objnum < MAX_OBJECTS) && (Objects[Net_players[idx].m_player->objnum].type == OBJ_SHIP) && 
			(Objects[Net_players[idx].m_player->objnum].instance >= 0) && (Objects[Net_players[idx].m_player->objnum].instance < MAX_SHIPS) && !stricmp(ship_name, Ships[Objects[Net_players[idx].m_player->objnum].instance].ship_name) ){
			return idx;
		}
	}

	if (inc_respawning) {
		p_objp = mission_parse_get_arrival_ship(ship_name);
		idx = multi_find_player_by_parse_object(p_objp);
		
		if((idx >= 0) && (idx < MAX_PLAYERS)){
			return idx;
		}
	}
	// didn't find the player
	return -1;
}

int multi_get_player_ship(int np_index)
{
	// bogus
	if((np_index < 0) || (np_index >= MAX_PLAYERS)){
		return -1;
	}

	// cool?
	if(MULTI_CONNECTED(Net_players[np_index]) && !MULTI_OBSERVER(Net_players[np_index]) && !MULTI_STANDALONE(Net_players[np_index]) && 
		(Net_players[np_index].m_player != NULL) && (Net_players[np_index].m_player->objnum >= 0) && (Net_players[np_index].m_player->objnum < MAX_OBJECTS) && (Objects[Net_players[np_index].m_player->objnum].type == OBJ_SHIP) && 
		(Objects[Net_players[np_index].m_player->objnum].instance >= 0) && (Objects[Net_players[np_index].m_player->objnum].instance < MAX_SHIPS) ){

		return Objects[Net_players[np_index].m_player->objnum].instance;
	}

	// nope
	return -1;
}

// find_open_netplayer_slot() attempts to find a free slot in the Net_players structure for a new player.
// it returns -1 if there is no new slot, and the slot number if one is found
// NOTE : this attempts to preserve a player object as long as possible for him to come back to
int multi_find_open_netplayer_slot()
{
	int i;
	int found_first;

	found_first = -1;
	for (i = 0; i < MAX_PLAYERS; i++)
		if ( !MULTI_CONNECTED(Net_players[i]) ){
		   if(found_first == -1) {
				found_first = i;
				break;
			}
		}

	if(i == MAX_PLAYERS){
		if(found_first == -1)
			return -1;
		else
			return found_first;
	}

	return i;
}

// find_open_player_slot() attempts to find an open player array slot.  The 0th element of the array
// should always be taken by the console player's pilot.  All other slots are up for grab though.
int multi_find_open_player_slot()
{
	int i;

	for (i = 0; i < MAX_PLAYERS; i++)
		if ( !(Players[i].flags & PLAYER_FLAGS_STRUCTURE_IN_USE) )
			break;
	
	if ( i == MAX_PLAYERS )
		return -1;

	return i;
}

// stuff_netplayer_info stuffs information into the given Net_player structure.  It is called when
// a new person is entering the game.  The state of the Net_player is set to it's basic starting
// state
void stuff_netplayer_info( net_player *nplayer, net_addr *addr, int ship_class, player *pplayer )
{
	nplayer->p_info.addr = *addr;
	nplayer->flags |= NETINFO_FLAG_CONNECTED;
	nplayer->state = NETPLAYER_STATE_JOINING;
	nplayer->p_info.ship_class = ship_class;
	nplayer->m_player = pplayer;
	nplayer->p_info.options.obj_update_level = Cmdline_objupd; // Set the update to the specified value

	// if setting up my net flags, then set the flag to say I can do networking.
	if ( nplayer == Net_player ){
		nplayer->flags |= NETINFO_FLAG_DO_NETWORKING;
	}
}

// multi_assign_player_ship takes a Net_player index and an object * and assigned that player to
// that object
void multi_assign_player_ship( int net_player, object *objp,int ship_class )
{
	ship *shipp;
	int idx;

	Assert ( MULTI_CONNECTED(Net_players[net_player]) );

	shipp = &Ships[objp->instance];

	Net_players[net_player].m_player->objnum = OBJ_INDEX(objp);
	Net_players[net_player].p_info.ship_class = ship_class;

	// check to see if we are assigning my player -- if so, then set Player_ship and Player_ai
	if ( Net_player == &Net_players[net_player] ) {
		Player_obj = objp;
		Player_ship = shipp;
		Player_ai = &Ai_info[Player_ship->ai_index];
	}

	// find the parse object for this ship.  Also, set the wingman status stuff so wingman status gauge
	// works properly.
	Net_players[net_player].p_info.p_objp = mission_parse_get_arrival_ship( shipp->ship_name );
	Assert( Net_players[net_player].p_info.p_objp != NULL );		// get allender -- ship should be on list
	Net_players[net_player].p_info.p_objp->ship_class = ship_class;		// be sure this gets set so respawns work

	// game server and this client need to initialize this information so object updating
	// works properly.
	if ( MULTIPLAYER_MASTER || (Net_player == &Net_players[net_player]) ) {
		Net_players[net_player].s_info.eye_pos = objp->pos;
		Net_players[net_player].s_info.eye_orient = objp->orient;
	}

	// zero update info	
	for(idx=0; idx<MAX_PLAYERS; idx++){
		shipp->np_updates[idx].orient_chksum = 0;
		shipp->np_updates[idx].pos_chksum = 0;
		shipp->np_updates[idx].seq = 0;
		shipp->np_updates[idx].status_update_stamp = -1;
		shipp->np_updates[idx].subsys_update_stamp = -1;
		shipp->np_updates[idx].update_stamp = -1;
	}
}

// -------------------------------------------------------------------------------------------------
//	create_player() is called when a net player needs to be instantiated.  The ship that is created
// depends on the parameter ship_class.  Note that if ship_class is invalid, the ship default_player_ship
// is used.  Returns 1 on success, 0 otherwise

int multi_create_player( int net_player_num, player *pl, char* name, net_addr* addr, int ship_class, short id)
{
	int player_ship_class = ship_class;
	int i,current_player_count;

	Assert ( net_player_num < MAX_PLAYERS );				// probably shoudln't be able to even get into this routine if no room	
	
	// blast _any_ old data
	memset(&Net_players[net_player_num],0,sizeof(net_player));

	// get the current # of players
	current_player_count = multi_num_players();

	// DOH!!! The lack of this caused many bugs. 
	Net_players[net_player_num].flags = (NETINFO_FLAG_DO_NETWORKING);
		
	if ( ship_class == -1 ) {
		nprintf(("Network","Network ==> ship class is -1, creating a default ship for multiplayer\n"));

		// find the ship that matches the string stored in default_player_ship

		for (i = 0; i < Num_ship_classes; i++) {
			if ( !stricmp(Ship_info[i].name, default_player_ship) ) {
				player_ship_class = i;
				break;
			}
		}

		if (i == Num_ship_classes)
		{
			if(Num_ship_classes)
			{
				player_ship_class = 0;
				Warning(LOCATION, "Invalid default player ship specified in ship tables. Setting to %s", Ship_info[player_ship_class].name);
			}
			else
			{
				Error(LOCATION, "No ships have been loaded, but we are attempting to set a ship!!");
			}
		}
	}
	
	if ( player_ship_class >= Num_ship_classes ) {
		player_ship_class = multi_ship_class_lookup(default_player_ship);
		nprintf(("Network","Network ==> Ship class was %d Creating a default ship for multiplayer\n", player_ship_class));
	}

	// blast the old player data
	memset(pl,0,sizeof(player));

	// set up the net_player structure
	stuff_netplayer_info( &Net_players[net_player_num], addr, player_ship_class, pl );
	Net_players[net_player_num].s_info.num_last_buttons = 0;
	// Net_players[net_player_num].respawn_count = 0;
	Net_players[net_player_num].last_heard_time = timer_get_fixed_seconds();
	Net_players[net_player_num].reliable_socket = INVALID_SOCKET;
	Net_players[net_player_num].s_info.kick_timestamp = -1;
	Net_players[net_player_num].s_info.voice_token_timestamp = -1;
	Net_players[net_player_num].s_info.tracker_security_last = -1;
	Net_players[net_player_num].s_info.target_objnum = -1;
	Net_players[net_player_num].s_info.accum_buttons = 0;

	// zero out this players ping times data
	multi_ping_reset(&Net_players[net_player_num].s_info.ping);

	// zero out his object update and control info sequencing data
	Net_players[net_player_num].client_cinfo_seq = 0;
	Net_players[net_player_num].client_server_seq = 0;		
	
	// timestamp his last_full_update_time
	Net_players[net_player_num].s_info.last_full_update_time = timestamp(0);

	// nil his file xfer handle
	Net_players[net_player_num].s_info.xfer_handle = -1;

	// nil his data rate timestamp stuff
	Net_players[net_player_num].s_info.rate_stamp = -1;
	Net_players[net_player_num].s_info.rate_bytes = 0;

	// nil packet buffer stuff
	Net_players[net_player_num].s_info.unreliable_buffer_size = 0;
	Net_players[net_player_num].s_info.reliable_buffer_size = 0;
	
	// various ack handles	
	strcpy_s(pl->callsign, name);
	pilot_set_short_callsign(pl, SHORT_CALLSIGN_PIXEL_W);   // calculate the short callsign 
	pl->flags |= PLAYER_FLAGS_STRUCTURE_IN_USE;
	pl->objnum = -1;
	pl->insignia_texture = -1;

	// if we're the standalone server and this is the first guy to join, mark him as the host
	// and give him all host priveleges
	if ( (Game_mode & GM_STANDALONE_SERVER) && (current_player_count == 0) ) {
		Net_players[net_player_num].flags |= NETINFO_FLAG_GAME_HOST;
	}
	
	Net_players[net_player_num].player_id = id;

	Net_player->sv_bytes_sent = 0;
	Net_player->sv_last_pl = -1;
	Net_player->cl_bytes_recvd = 0;
	Net_player->cl_last_pl = -1;

	// add to the escort list
	if(MULTI_IN_MISSION){
		hud_escort_add_player(id);
	}

	return 1;
}

// next function makes a player object an ai object (also decrementing the num_players in the
// netgame).  It appropiately sets the object flags and other interesting AI information which
// ought to get set in order to make this new ai object behave correctly.
void multi_make_player_ai( object *pobj )
{

	Assert ( pobj != NULL );

	if ( pobj->type != OBJ_SHIP )
		return;

	pobj->flags &= ~(OF_PLAYER_SHIP);
	obj_set_flags( pobj, pobj->flags | OF_COULD_BE_PLAYER );
	obj_set_flags( pobj, pobj->flags & ~(OF_INVULNERABLE));		// Newly respawned players will still be invulnerable

	// target_objnum must be -1 or else new AI ship will fire on whatever this player
	// had targeted.
	set_target_objnum( &(Ai_info[Ships[pobj->instance].ai_index]), -1 );

}

// delete_player takes an index into the Net_players array to delete.  Deletion of a player might happen
// because of the player leaving the game on his own, or no net activity from the player after X seconds
void delete_player(int player_num,int kicked_reason)
{			
	char notify_string[256] = "";
	int idx;
	
	if(!MULTI_CONNECTED(Net_players[player_num])){
		return;
	}

	// NETLOG
	ml_printf(NOX("Deleting player %s"), Net_players[player_num].m_player->callsign);
	
	psnet_rel_close_socket( &(Net_players[player_num].reliable_socket) );					// close out the reliable socket	

	// if this guy was ingame joining, the remove my netgame flag so others may join
	if ( Net_players[player_num].flags & NETINFO_FLAG_INGAME_JOIN ) {
		Netgame.flags &= ~NG_FLAG_INGAME_JOINING;
		Netgame.flags &= ~NG_FLAG_INGAME_JOINING_CRITICAL;
	}
	
	Net_players[player_num].flags &= ~NETINFO_FLAG_CONNECTED;							// person not connected anymore
	Net_players[player_num].m_player->flags &= ~(PLAYER_FLAGS_STRUCTURE_IN_USE);    // free up his player structure

	Net_players[player_num].s_info.reliable_connect_time = -1;

	// add to the escort list
	hud_escort_remove_player(Net_players[player_num].player_id);

	// is this guy the server
	if(Net_players[player_num].flags & NETINFO_FLAG_AM_MASTER){
		// if the server leaves in the debriefing state, we should still wait until the player selects accept before we quit
		if ((gameseq_get_state() != GS_STATE_DEBRIEF) && (gameseq_get_state() != GS_STATE_MULTI_DOGFIGHT_DEBRIEF)){
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_SERVER_LEFT);
		}		
	}
	// if he was just the host	
	else if(Net_players[player_num].flags & NETINFO_FLAG_GAME_HOST){
		Net_players[player_num].flags &= ~(NETINFO_FLAG_GAME_HOST);
	
		// am I the server
		if ( (Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) ) {
			// are we a standalone server and in a mission?
			if((Game_mode & GM_STANDALONE_SERVER) && MULTI_IN_MISSION){			
				// choose a new host			
				int found_new_host = 0;
				for(idx=0; idx<MAX_PLAYERS; idx++){
					if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx])){
						// make this guy the host
						Net_players[idx].flags |= NETINFO_FLAG_GAME_HOST;

						// send a packet
						send_host_captain_change_packet(Net_players[idx].player_id, 0);

						found_new_host = 1;
						break;
					}
				}
				// end the game		
				if(!found_new_host){
					multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_HOST_LEFT);
				}			 
			} else {
				multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_HOST_LEFT);
			}
		} 
	}

	// if we're the server of the game, notify everyone else that this guy has left
	// if he's marked as being kicked, then other players know about it already, so don't send again
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		if(Net_players[player_num].flags & NETINFO_FLAG_KICKED){
			char str[512];
			memset(str, 0, 512);
			multi_kick_get_text(&Net_players[player_num], Net_players[player_num].s_info.kick_reason, str);
			multi_display_chat_msg(str, player_num, 0);							 
		} else {
			send_leave_game_packet(Net_players[player_num].player_id, kicked_reason);
		}
	}
	
	// if this guy is an observer, we have to make sure we delete his observer object (only if we're the server however)
	if( (Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Net_players[player_num].flags & NETINFO_FLAG_OBSERVER) ) {
		if ( Net_players[player_num].m_player->objnum != -1 ){
			obj_delete(Net_players[player_num].m_player->objnum);			// maybe change this to set flag instead		
		}
	} else {
		// otherwise mark it so that he can return to it later if possible
		if ( (Net_players[player_num].m_player->objnum >= 0) && (Net_players[player_num].m_player->objnum < MAX_OBJECTS) && (Objects[Net_players[player_num].m_player->objnum].type == OBJ_SHIP) && (Objects[Net_players[player_num].m_player->objnum].instance >= 0) && (Objects[Net_players[player_num].m_player->objnum].instance < MAX_SHIPS)) {
			multi_make_player_ai( &Objects[Net_players[player_num].m_player->objnum] );
		} else {
			multi_respawn_player_leave(&Net_players[player_num]);
		}
	}

	// if we're in the team select, and we're the host, we have to make sure all team select data is correctly updated			
	// MWA 7/28/98.  Don't do this for a standalone server.  Since he doesn't go through the team selection
	// screens, his data structures are not 100% accurate.  Doing so on a standalone resulted in the wrong
	// ships getting marked as COULD_BE_PLAYER.  On the standalone, these flags will get properly set
	// in the multi_make_player_ai code above.
	if( (Netgame.game_state == NETGAME_STATE_BRIEFING) && !(Game_mode & GM_STANDALONE_SERVER) ) {
		multi_ts_handle_player_drop();			
	}								

	if (gameseq_get_state() == GS_STATE_DEBRIEF){
		debrief_handle_player_drop();
	}

	// handle any specific dropping conditions
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_team_handle_drop();
	}	
	multi_data_handle_drop(player_num);	

	// tell the pinfo popup that a player has left
	multi_pinfo_notify_drop(&Net_players[player_num]);

	// tell the datarate stuff that the player has dropped
	multi_rate_reset(player_num);

	// display a message that this guy has left
	if(Net_players[player_num].m_player->callsign){
		sprintf(notify_string,XSTR("<%s has left>",901),Net_players[player_num].m_player->callsign);
		multi_display_chat_msg(notify_string,0,0);
	}
	
	// standalone gui type stuff
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_remove_player(&Net_players[player_num]);
		std_connect_set_connect_count();
	}

	// blast this memory clean
	memset(&Net_players[player_num], 0, sizeof(net_player));
	Net_players[player_num].reliable_socket = INVALID_SOCKET;

	extern int Multi_client_update_times[MAX_PLAYERS];
	Multi_client_update_times[player_num] = -1;
}

#define INACTIVE_LIMIT_NORMAL (15 * F1_0)
#define INACTIVE_LIMIT_WAIT   (20 * F1_0)

// -------------------------------------------------------------------------------------------------
//	multi_cull_zombies() will check if there has been any net players or observers that have been inactive for
// INACTIVE_LIMIT milliseconds since last check.  If so, they are taken out of the net game.
//
//

void multi_cull_zombies()
{
#if 0
	fix current_time;
	int inactive_limit;
	int i;

	if(gameseq_get_state() == GS_STATE_MULTI_WAIT)
		inactive_limit = INACTIVE_LIMIT_WAIT;
	else
		inactive_limit = INACTIVE_LIMIT_NORMAL;

	current_time = timer_get_fixed_seconds();

	for (i = 0; i < MAX_PLAYERS; i++) {
		if ( !MULTI_CONNECTED(&Net_players[i])){
			continue;
		}

		// server will(should) cull out players based on their sockets dying.
		if ( Net_players[i].flags & NETINFO_FLAG_MASTER ){
			continue;
		}

		if ( (current_time - Net_players[i].last_heard_time) > inactive_limit) {
			HUD_printf(XSTR("Dumping %s after prolonged inactivity",902),Net_players[i].m_player->callsign);
			nprintf(("Network", "Assuming %s is a zombie, removing from game\n", Net_players[i].m_player->callsign));

			multi_kick_player(i,0);			
		}
	}
#endif
}

// -------------------------------------------------------------------------------------------------
// fill_net_addr() calculates the checksum of a block of memory.
//
//

void fill_net_addr(net_addr* addr, ubyte* address, ubyte* net_id, ushort port)
{
	Assert(addr != NULL);
	Assert(address != NULL);
	Assert(net_id != NULL);

	addr->type = Multi_options_g.protocol;
	memset( addr->addr, 0x00, 6);
	memcpy( addr->addr, address, ADDRESS_LENGTH);
	memcpy( addr->net_id, net_id, 4);
	addr->port = port;
}



// -------------------------------------------------------------------------------------------------
// get_text_address()
//
//

char* get_text_address( char * text, ubyte * address )
{

	in_addr temp_addr;

	switch ( Multi_options_g.protocol ) {
		case NET_IPX:
			strcpy( text, XSTR("[ipx address here]",903) );	// TODO: find equiv to inet_ntoa() for IPX
			sprintf(text, "%x %x %x %x %x %x",	address[0],
															address[1],
															address[2],
															address[3],
															address[4],
															address[5]);

			break;

		case NET_TCP:
			memcpy(&temp_addr.s_addr, address, 4);
			strcpy( text, inet_ntoa(temp_addr) );
			break;

		default:
			Assert(0);
			break;

	} // end switch

	return text;
}

// non-16byte version of matrix packing
// return size of packed matrix
void multi_pack_orient_matrix(ubyte *data,matrix *m)
{	
	data[16] = 0;
	float x1, y1, x2, y2;

	if(m->vec.rvec.xyz.z < 0) data[16] |= (1<<0);	// X
	if(m->vec.uvec.xyz.z < 0) data[16] |= (1<<1);	// Y
	if(m->vec.fvec.xyz.z < 0) data[16] |= (1<<2);	// V
	if(m->vec.fvec.xyz.x < 0) data[16] |= (1<<3);	// Z
	if(m->vec.fvec.xyz.y < 0) data[16] |= (1<<4);	// W

	x1 = INTEL_FLOAT(&m->vec.rvec.xyz.x);
	y1 = INTEL_FLOAT(&m->vec.rvec.xyz.y);
	x2 = INTEL_FLOAT(&m->vec.uvec.xyz.x);
	y2 = INTEL_FLOAT(&m->vec.uvec.xyz.y);

	memcpy(&data[0], &x1, 4);		// a
	memcpy(&data[4], &y1, 4);		// b
	memcpy(&data[8], &x2, 4);		// c
	memcpy(&data[12], &y2, 4);		// d
}

// return bytes processed
// non-16 byte version of unpack matrix code
void multi_unpack_orient_matrix(ubyte *data,matrix *m)
{
	float x1, y1, x2, y2;

	memcpy(&x1, &data[0], 4);
	memcpy(&y1, &data[4], 4);
	memcpy(&x2, &data[8], 4);
 	memcpy(&y2, &data[12],4);

	m->vec.rvec.xyz.x = INTEL_FLOAT(&x1);
	m->vec.rvec.xyz.y = INTEL_FLOAT(&y1);
	m->vec.uvec.xyz.x = INTEL_FLOAT(&x2);
	m->vec.uvec.xyz.y = INTEL_FLOAT(&y2);

	m->vec.rvec.xyz.z = fl_sqrt(fl_abs(1 - (m->vec.rvec.xyz.x * m->vec.rvec.xyz.x) - (m->vec.rvec.xyz.y * m->vec.rvec.xyz.y))); // X
	m->vec.uvec.xyz.z = fl_sqrt(fl_abs(1 - (m->vec.uvec.xyz.x * m->vec.uvec.xyz.x) - (m->vec.uvec.xyz.y * m->vec.uvec.xyz.y))); // Y
	m->vec.fvec.xyz.z = fl_sqrt(fl_abs(1 - (m->vec.rvec.xyz.z * m->vec.rvec.xyz.z) - (m->vec.uvec.xyz.z * m->vec.uvec.xyz.z))); // V
	m->vec.fvec.xyz.x = fl_sqrt(fl_abs(1 - (m->vec.rvec.xyz.x * m->vec.rvec.xyz.x) - (m->vec.uvec.xyz.x * m->vec.uvec.xyz.x))); // Z
	m->vec.fvec.xyz.y = fl_sqrt(fl_abs(1 - (m->vec.rvec.xyz.y * m->vec.rvec.xyz.y) - (m->vec.uvec.xyz.y * m->vec.uvec.xyz.y))); // W

	m->vec.rvec.xyz.z *= (data[16] & (1<<0)) ? -1.0f : 1.0f;
	m->vec.uvec.xyz.z *= (data[16] & (1<<1)) ? -1.0f : 1.0f;
	m->vec.fvec.xyz.z *= (data[16] & (1<<2)) ? -1.0f : 1.0f;
	m->vec.fvec.xyz.x *= (data[16] & (1<<3)) ? -1.0f : 1.0f;
	m->vec.fvec.xyz.y *= (data[16] & (1<<4)) ? -1.0f : 1.0f;
}
	                      
void multi_do_client_warp(float frame_time)
{
   ship_obj *moveup;
	
   moveup = GET_FIRST(&Ship_obj_list);
	while(moveup!=END_OF_LIST(&Ship_obj_list)){
		// do all _necessary_ ship warp in (arrival) processing
		if ( Ships[Objects[moveup->objnum].instance].flags & SF_ARRIVING )	
			shipfx_warpin_frame( &Objects[moveup->objnum], frame_time );
		moveup = GET_NEXT(moveup);
	}	
}	

// ------------------------------------------------------------------------------------
// ship status change stuff

int lookup_ship_status(net_player *p,int unique_id,int remove)
{
	int idx;
	
	for(idx=0;idx<p->s_info.num_last_buttons;idx++){
		if(p->s_info.last_buttons_id[idx] == unique_id){
			if(remove){
				remove_ship_status_item(p,idx);
				p->s_info.num_last_buttons--;
			}
			return 1;
		}
	}
	return 0;
}

void remove_ship_status_item(net_player *p,int id)
{
	int idx;
	for(idx=id;idx<BUTTON_INFO_SAVE_COUNT-1;idx++){
		p->s_info.last_buttons[idx] = p->s_info.last_buttons[idx+1];
		p->s_info.last_buttons_id[idx] = p->s_info.last_buttons_id[idx+1];
		p->s_info.last_buttons_time[idx] = p->s_info.last_buttons_time[idx+1];
	}
}

// 
void add_net_button_info(net_player *p,button_info *bi,int unique_id)
{
   int lookup,idx;
	fix earliest;

	// if the list is full, put it in the oldest slot since it's probably a lost packet anyway
   if(p->s_info.num_last_buttons < BUTTON_INFO_SAVE_COUNT-1){
		p->s_info.last_buttons[p->s_info.num_last_buttons] = *bi;
		p->s_info.last_buttons_id[p->s_info.num_last_buttons] = unique_id;
		p->s_info.last_buttons_time[p->s_info.num_last_buttons] = timer_get_fixed_seconds();
		p->s_info.num_last_buttons++;
	} else {
		earliest = 0;
      lookup = -1;
		for(idx=0;idx<BUTTON_INFO_SAVE_COUNT;idx++){
			if((p->s_info.last_buttons_time[idx] < earliest) || (earliest == 0)){
				earliest = p->s_info.last_buttons_time[idx];
				lookup = idx;
			}
		}
		if(lookup != -1){
			p->s_info.last_buttons[lookup] = *bi;
			p->s_info.last_buttons_id[lookup] = unique_id;
			p->s_info.last_buttons_time[lookup] = timer_get_fixed_seconds();
		}
	}		
}

extern int button_function_critical(int n,net_player *p = NULL);
void multi_apply_ship_status(net_player *p,button_info *bi,int locally)
{
	int i, j;
	Multi_button_info_ok=1;
	for ( i = 0; i < NUM_BUTTON_FIELDS; i++ ) {
		if ( bi->status[i] == 0 )
			continue;
		// at least one bit is set in the status integer
		for ( j = 0; j < 32; j++ ) {

			// check if the bit is set. If button_function returns 1 (implying the action was taken), then unset the bit
			if ( bi->status[i] & (1<<j) ) {
            if(locally){
					if(button_function_critical(32*i + j,NULL))   // will apply to this console
						bi->status[i] &= ~(1<<j);
				} else {
					if(button_function_critical(32*i + j,p))      // will only apply to a net-player
						bi->status[i] &= ~(1<<j);
				}
			}
		}
	}
	Multi_button_info_ok=0;
}

// send 10x a second MAX
#define MULTI_SHIP_STATUS_TIME			350
int Multi_ship_status_stamp = -1;
button_info Multi_ship_status_bi;

void multi_maybe_send_ship_status()
{
	int idx;
	button_info *bi = &Player->bi;

	// strip out noncritical button presses
	button_strip_noncritical_keys(bi);

	// xor all fields into the accum button info
	for(idx=0; idx<NUM_BUTTON_FIELDS; idx++){		
		Multi_ship_status_bi.status[idx] |= bi->status[idx];		
	}

	// check timestamp
	if((Multi_ship_status_stamp < 0) || timestamp_elapsed_safe(Multi_ship_status_stamp, MULTI_SHIP_STATUS_TIME*2)){
		int should_send = 0;
		for(idx=0; idx<NUM_BUTTON_FIELDS; idx++){			
			// we have at least something to send
			if(Multi_ship_status_bi.status[idx] != 0){
				should_send = 1;			
			}		
		}

		// do we have something to send
		if(should_send){
			// add_net_button_info(Net_player, &Multi_ship_status_bi, Multi_button_info_id);
			send_ship_status_packet(Net_player, &Multi_ship_status_bi, Multi_button_info_id++);
		}

		// zero it out
		memset(&Multi_ship_status_bi, 0, sizeof(button_info));

		// reset timestamp
		Multi_ship_status_stamp = timestamp(MULTI_SHIP_STATUS_TIME);
	}
}

void multi_subsys_update_all()
{
	/*
	int idx;
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if((Net_players[idx].flags & NETINFO_FLAG_CONNECTED) && !psnet_same(&My_addr,&Net_players[idx].addr) && !(Net_players[idx].flags & NETINFO_FLAG_OBSERVER))
			send_subsys_update_packet(&Net_players[idx]);
	}
	*/
}

int multi_find_player_by_callsign(char *callsign)
{
	int idx;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (strcmp(callsign,Net_players[idx].m_player->callsign)==0)){
			return idx;
		}
	}

	return -1;
}

// if Game_current_mission_filename is a builtin multiplayer mission
int multi_is_builtin_mission()
{
	// int idx;
	char name[512];

	// get the full filename
	memset(name,0,512);
	strcpy_s(name,Game_current_mission_filename);
	cf_add_ext(name, FS_MISSION_FILE_EXT);

	// if this mission is builtin	
	if(game_find_builtin_mission(name) != NULL){
		return 1;
	}

	// not builtin
	return 0;
}

// verify that the player has a valid mission file and do 1 of 3 things
void server_verify_filesig(short player_id, ushort sum_sig, int length_sig)
{   	
#if defined(MULTIPLAYER_BETA_BUILD) || defined(FS2_DEMO)
	net_player *pl;
   int player;

	player = find_player_id(player_id);
	Assert(player >= 0);
	if(player < 0){
		return;
	}
	pl = &Net_players[player];

	// all missions should be builtin, so if we don't have a match, kick the player
	if((sum_sig != Multi_current_file_checksum) || (length_sig != Multi_current_file_length)){
		multi_kick_player(player, 0, KICK_REASON_CANT_XFER);
	} else {
		pl->flags |= NETINFO_FLAG_MISSION_OK;
	}
#else
	net_player *pl;
   int player;
	int ok;	
	int is_builtin;
   
	player = find_player_id(player_id);
	Assert(player >= 0);
	if(player < 0){
		return;
	}
	pl = &Net_players[player];

	// if the current mission is a builtin mission, check stuff out
	is_builtin = multi_is_builtin_mission();
		
	if(is_builtin){
		// if the player doesn't have it, kick him
		if((sum_sig == 0xffff) && (length_sig == -1)){
			multi_kick_player(player, 0, KICK_REASON_CANT_XFER);
		} else {
			pl->flags |= NETINFO_FLAG_MISSION_OK;
		}

		// done
		return;
	}

	if( (length_sig != Multi_current_file_length) || (sum_sig != Multi_current_file_checksum)){
		ok = 0;
	} else {
		ok = 1;	
	}

   // in an ingame join situation
	if(pl->flags & NETINFO_FLAG_INGAME_JOIN){		
		if(!ok){
			// if the netgame settings allow in-mission file xfers
			if(Netgame.options.flags & MSO_FLAG_INGAME_XFER){
				pl->s_info.ingame_join_flags |= INGAME_JOIN_FLAG_FILE_XFER;
				pl->s_info.xfer_handle = multi_xfer_send_file(pl->reliable_socket, Netgame.mission_name, CF_TYPE_MISSIONS);
			}
			// otherwise send him a nak and tell him to get away 
			else {
				send_ingame_nak(ACK_FILE_ACCEPTED,pl);
			}
		} else {
			pl->flags |= NETINFO_FLAG_MISSION_OK;
		}
	}
	// in a normal join situation
	else {
		// if the file does not check out, send it to him
		if(!ok){			
			pl->s_info.xfer_handle = multi_xfer_send_file(pl->reliable_socket, Netgame.mission_name, CF_TYPE_MISSIONS);
		}
		// otherwise mark him as having a valid mission
		else {
			pl->flags |= NETINFO_FLAG_MISSION_OK;			
		}
	}
#endif
}

// check to see if every client has NETINFO_FLAG_MISSION_OK
int server_all_filesigs_ok()
{
	int idx;
	int ok = 1;
	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !(Net_players[idx].flags & NETINFO_FLAG_MISSION_OK)){
			ok = 0;
			break;
		}
	}

	return ok;
}

void multi_untag_player_ships()
{
	ship_obj *moveup;

	moveup = GET_FIRST(&Ship_obj_list);
	while(moveup != END_OF_LIST(&Ship_obj_list)){
		if(Objects[moveup->objnum].flags & OF_PLAYER_SHIP){
			Objects[moveup->objnum].flags &= ~(OF_PLAYER_SHIP);
			obj_set_flags( &Objects[moveup->objnum], Objects[moveup->objnum].flags | OF_COULD_BE_PLAYER );
		}
		moveup = GET_NEXT(moveup);
	}
}

// broadcast alltime stats to everyone in the game
void multi_broadcast_stats(int stats_code)
{
	int idx;

	// broadcast everyone's stats to everyone else
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){			
			send_player_stats_block_packet(&Net_players[idx], stats_code);
		}
	}			
}

// check to see if all players other than the local player are in a given NETPLAYER_ state
// this is _EXTREMELY_ useful when doing network sequencing with the reliable sockets
int multi_netplayer_state_check(int state,int ignore_standalone)
{
	int idx;	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_player->player_id != Net_players[idx].player_id)){
			if(ignore_standalone && ((Net_players[idx].flags & NETINFO_FLAG_AM_MASTER) && !(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST)) ){
				continue;
			}

			if(Net_players[idx].state != state){
				return 0;
			}
		}
	}
	return 1;
}

int multi_netplayer_state_check2(int state, int state2, int ignore_standalone)
{
	int idx;	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_player->player_id != Net_players[idx].player_id)){
			if(ignore_standalone && ((Net_players[idx].flags & NETINFO_FLAG_AM_MASTER) && !(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST)) ){
				continue;
			}

			if((Net_players[idx].state != state) && (Net_players[idx].state != state2)){
				return 0;
			}
		}
	}
	return 1;
}

int multi_netplayer_state_check3(int state, int state2, int state3, int ignore_standalone)
{
	int idx;	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_player->player_id != Net_players[idx].player_id)){
			if(ignore_standalone && ((Net_players[idx].flags & NETINFO_FLAG_AM_MASTER) && !(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST)) ){
				continue;
			}

			if((Net_players[idx].state != state) && (Net_players[idx].state != state2) && (Net_players[idx].state != state3)){
				return 0;
			}
		}
	}
	return 1;
}

// check to see if all players other than the local player are in a given NETPLAYER_ state
// this is _EXTREMELY_ useful when doing network sequencing with the reliable sockets
int multi_netplayer_flag_check(int flags,int ignore_standalone)
{
	int idx;	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_player->player_id != Net_players[idx].player_id)){
			if(ignore_standalone && ((Net_players[idx].flags & NETINFO_FLAG_AM_MASTER) && !(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST)) ){
				continue;
			}

			if(!(Net_players[idx].flags & flags)){
				return 0;
			}
		}
	}
	return 1;
}

// function which gets called from psnet_* code to evaluate an error received on a reliable
// socket.  In general, we only want to do drastic measures if the error indicates that the client
// is no longer there.
void multi_eval_socket_error(PSNET_SOCKET sock, int error)
{
	if ( error == WSAENOTSOCK ){
		nprintf(("Network","Socket connection terminated and/or nonexistent, bailing..\n"));

		// mwa -- don't go back to main menu.  You don't want host to do this.  Maybe we can ignore it
		// because of a leaving player.
		return;
		//gameseq_post_event(GS_EVENT_MAIN_MENU);
		// Int3();							// get allender -- something happened to socket connection!!!
	}

	if ( (error != WSAECONNRESET) && (error != WSAECONNABORTED) && (error != WSAESHUTDOWN) ) {
		nprintf(("Network", "Error %d received on reliable socket -- ignoring\n", error));
		return;
	}

	if(error == WSAESHUTDOWN){
		nprintf(("Network","Received WSAESHUTDOWN on client socket. Cool.\n"));
	}

	// mwa -- always return for now because debugging with the stuff below is a real pain.
	// in essence, you can't do it!
	//return;

	if( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {
		int idx;

		nprintf(("Network", "pitching player because drop on reliable socket\n"));
		// find the netplayer whose socket we have an error on.  Dump the player when we find him.
		// NOTE : make sure not to ban him
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(Net_players[idx].reliable_socket == sock){
				delete_player(idx);
				return;
			}
		}
	} else {
		nprintf(("Network", "Communications to server lost -- quitting game\n"));
		multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_CONTACT_LOST);
	}
}

// send a repair info packet with code == code to a player if his object is the one being acted upon
// dest_objp is the player object (or possible player object).  source_objp is the object pointer
// of the repair ship that is the source of the action.  code means what is happening -- queueing up
// for repair, getting repaired, aborted, complete, etc. 
void multi_maybe_send_repair_info(object *dest_objp, object *source_objp, int code )
{
	// only send information on player objects
	if ( !MULTIPLAYER_MASTER )
		return;
	
	Assert( dest_objp->type == OBJ_SHIP );
	Assert( dest_objp != source_objp );

	send_repair_info_packet( dest_objp, source_objp, code );
}

int multi_is_valid_unknown_packet(ubyte type){
	return (type == GAME_QUERY) || (type == JOIN) || (type == PING) || (type == PONG) || (type == GAME_INFO)
		|| (type == ACCEPT) || (type == GAME_ACTIVE) || (type == INGAME_NAK) || (type == DENY)
		|| (type == UPDATE_DESCRIPT) || (type == ACCEPT_PLAYER_DATA) ? 1 : 0;	
}

void multi_create_standalone_object()
{
	// now create a dummy ship for the standalone
	matrix m = IDENTITY_MATRIX;
	vec3d v;
	int objnum, pobj_num;

	vm_vec_zero(&v);
	objnum = observer_create(&m,&v);
	Player_obj = &Objects[objnum];
	obj_set_flags(Player_obj, Player_obj->flags & (~OF_COLLIDES) );
	//obj_set_flags(Player_obj, Player_obj->flags | OF_SHOULD_BE_DEAD);
	obj_set_flags(Player_obj, Player_obj->flags & (~OF_PLAYER_SHIP));
	Net_player->m_player->objnum = objnum;

	// create the default player ship object and use that as my default virtual "ship", and make it "invisible"
	pobj_num = parse_create_object(&Player_start_pobject);
	Assert(pobj_num != -1);
	obj_set_flags(&Objects[pobj_num],OF_PLAYER_SHIP);
	Objects[pobj_num].net_signature = STANDALONE_SHIP_SIG;
	Player_ship = &Ships[Objects[pobj_num].instance];

	// make ship hidden from sensors so that this observer cannot target it.  Observers really have two ships
	// one observer, and one "Player_ship".  Observer needs to ignore the Player_ship.
	Player_ship->flags |= SF_HIDDEN_FROM_SENSORS;
	strcpy_s(Player_ship->ship_name, XSTR("Standalone Ship",904));
	Player_ai = &Ai_info[Ships[Objects[pobj_num].instance].ai_index];		

}

int multi_message_should_broadcast(int type)
{
	switch (type) {
		case MESSAGE_ARRIVE_ENEMY:
		case MESSAGE_BETA_ARRIVED:
		case MESSAGE_GAMMA_ARRIVED:
		case MESSAGE_HELP:
		case MESSAGE_REINFORCEMENTS:
		case MESSAGE_SUPPORT_KILLED:
			return 1; 
		default:
			return 0;
	}
}

// active game list handling functions
active_game *multi_new_active_game( void )
{
	active_game *new_game;

	new_game = (active_game *)vm_malloc(sizeof(active_game));
	if ( new_game == NULL ) {
		nprintf(("Network", "Cannot allocate space for new active game structure\n"));
		return NULL;
	}	

	if ( Active_game_head != NULL ) {
		new_game->next = Active_game_head->next;
		new_game->next->prev = new_game;
		Active_game_head->next = new_game;
		new_game->prev = Active_game_head;
	} else {
		Active_game_head = new_game;
		Active_game_head->next = Active_game_head->prev = Active_game_head;
	}

	Active_game_count++;

	// notify the join game screen of this new item
	multi_join_notify_new_game();
		
	return new_game;
}

active_game *multi_update_active_games(active_game *ag)
{
	active_game *gp = NULL;
	active_game *stop = NULL;		

	// see if we have a game from this address already -- if not, create one.  In either case, get a pointer
	// to an active_game structure
	if ( Active_game_head != NULL ) {	// no games on list at all
		int on_list;

		gp = Active_game_head;
		stop = Active_game_head;

		on_list = 0;
		do {
			if ( psnet_same(&gp->server_addr, &ag->server_addr) /*&& (gp->game.security == game->security)*/ ) {
				on_list = 1;
				break;
			}
			gp = gp->next;
		} while (gp != stop);

		// insert in the list
		if (!on_list){
			gp = multi_new_active_game();
			// gp->ping_time = -1.0f;			

			// copy in the game information
			memcpy(&gp->server_addr,&ag->server_addr,sizeof(net_addr));
			strcpy_s(gp->name,ag->name);
			strcpy_s(gp->mission_name,ag->mission_name);
			strcpy_s(gp->title,ag->title);			
			gp->num_players = ag->num_players;
			gp->flags = ag->flags;
			
			// ping him
			/*
			gp->ping_start = timer_get_fixed_seconds();
			gp->ping_end = -1;
			send_ping(&gp->server_addr);			
			*/
			multi_ping_reset(&gp->ping);
			multi_ping_send(&gp->server_addr,&gp->ping);
		}
		// otherwise update the netgame info we have for this guy
		else {				
			memset(gp->name,0,MAX_GAMENAME_LEN+1);
			strcpy_s(gp->name,ag->name);
			memset(gp->mission_name,0,NAME_LENGTH+1);
			strcpy_s(gp->mission_name,ag->mission_name);
			memset(gp->title,0,NAME_LENGTH+1);
			strcpy_s(gp->title,ag->title);			
			gp->num_players = ag->num_players;
			gp->flags = ag->flags;			
		}
	} else {
		gp = multi_new_active_game();
		// gp->ping_time = -1.0f;		

		// copy in the game information	
		memcpy(&gp->server_addr,&ag->server_addr,sizeof(net_addr));
		strcpy_s(gp->name,ag->name);
		strcpy_s(gp->mission_name,ag->mission_name);
		strcpy_s(gp->title,ag->title);		
		gp->num_players = ag->num_players;
		gp->flags = ag->flags;
		
		// ping him
		// gp->ping_start = timer_get_fixed_seconds();
		// gp->ping_end = -1;
		// send_ping(&gp->server_addr);
		multi_ping_reset(&gp->ping);
		multi_ping_send(&gp->server_addr,&gp->ping);
	}		

	// don't do anything if we don't have a game entry
	if(gp == NULL){
		return NULL;
	}
	
	// update the last time we heard from him
	if((Multi_options_g.protocol == NET_TCP) && (Net_player->p_info.options.flags & MLO_FLAG_LOCAL_BROADCAST)){
		gp->heard_from_timer = timestamp(MULTI_JOIN_SERVER_TIMEOUT_LOCAL);
	} else {
		gp->heard_from_timer = timestamp(MULTI_JOIN_SERVER_TIMEOUT);
	}

	return gp;
}

void multi_free_active_games()
{
	active_game *moveup,*backup;	

	moveup = Active_game_head;
	backup = NULL;
	if(moveup != NULL){
		do {			
			backup = moveup;
			moveup = moveup->next;
			
			vm_free(backup);
			backup = NULL;
		} while(moveup != Active_game_head);
		Active_game_head = NULL;
	}	
	Active_game_count = 0;
}

server_item *multi_new_server_item( void )
{
	server_item *new_game;

	new_game = (server_item *)vm_malloc(sizeof(server_item));
	if ( new_game == NULL ) {
		nprintf(("Network", "Cannot allocate space for new server_item structure\n"));
		return NULL;
	}

	if ( Game_server_head != NULL ) {
		new_game->next = Game_server_head->next;
		new_game->next->prev = new_game;
		Game_server_head->next = new_game;
		new_game->prev = Game_server_head;
	} else {
		Game_server_head = new_game;
		Game_server_head->next = Game_server_head->prev = Game_server_head;
	}
		
	return new_game;
}

void multi_free_server_list()
{
	server_item *moveup,*backup;	

	moveup = Game_server_head;
	backup = NULL;
	if(moveup != NULL){
		do {			
			backup = moveup;
			moveup = moveup->next;
			
			vm_free(backup);
			backup = NULL;
		} while(moveup != Game_server_head);
		Game_server_head = NULL;
	}	
}

int multi_num_players()
{
	int idx,count;

	// count the players who are actively connected
	count = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// count all connected players except the standalone server (if any)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx])){			
			count++;
		}
	}

	return count;
}

int multi_num_observers()
{
	int idx,count;

	// count the players who are actively connected
	count = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// count all connected players except the standalone server (if any)
		if(MULTI_CONNECTED(Net_players[idx]) && MULTI_PERM_OBSERVER(Net_players[idx])){
			count++;
		}
	}

	return count;

}

int multi_num_connections()
{
	int idx,count;

	// count the players who are actively connected
	count = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// count all connected players except the standalone server (if any)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			count++;
		}
	}

	return count;
}

int multi_can_message(net_player *p)
{
	int max_rank;
	ship *sp;

	// if the player is an observer of any kind, he cannot message
	if(p->flags & NETINFO_FLAG_OBSERVER){
		return 0;
	}

	switch(Netgame.options.squad_set){
	// only those of the highest rank can message
	case MSO_SQUAD_RANK:
		max_rank = multi_get_highest_rank();
		if(p->m_player->stats.rank < max_rank){
			return 0;
		}
		break;

	// only wing/team leaders can message
	case MSO_SQUAD_LEADER:
		// if the player has an invalid object #
		if(p->m_player->objnum < 0){
			return 0;
		}

		// check to see if he's a wingleader
		sp = &Ships[Objects[p->m_player->objnum].instance];
		if (sp->ship_name[strlen(sp->ship_name)-1] == '1') 
		{
			return 0;
		}	
		break;

	// anyone can end message
	case MSO_SQUAD_ANY:
		break;

	// only the host can message
	case MSO_SQUAD_HOST:
		if(!(p->flags & NETINFO_FLAG_GAME_HOST)){
			return 0;
		}
		break;
	}	
	
	return 1;
}

int multi_can_end_mission(net_player *p)
{
	int max_rank;
	ship *sp;	

	// the host can _always_ unpause a game
	if(p->flags & NETINFO_FLAG_GAME_HOST){
		return 1;
	}

	switch(Netgame.options.endgame_set){
	// only those of the highest rank can end the mission
	case MSO_END_RANK:
		max_rank = multi_get_highest_rank();
		if(p->m_player->stats.rank < max_rank){
			return 0;
		}
		break;

	// only wing/team leaders can end the mission
	case MSO_END_LEADER:
		// if the player has an invalid object #
		if(p->m_player->objnum < 0){
			return 0;
		}

		// check to see if he's a wingleader
		sp = &Ships[Objects[p->m_player->objnum].instance];
		if (sp->ship_name[strlen(sp->ship_name)-1] == '1') 
		{
			return 0;
		}	
		break;

	// anyone can end the mission
	case MSO_END_ANY:
		break;

	// only the host can end the mission
	case MSO_END_HOST:
		if(!(p->flags & NETINFO_FLAG_GAME_HOST)){
			return 0;
		}
		break;
	}	
	
	return 1;
}

int multi_eval_join_request(join_request *jr,net_addr *addr)
{	
	int team0_avail,team1_avail;
	char knock_message[256], knock_callsign[CALLSIGN_LEN+1], jr_ip_string[16]; 

	// if the server versions are incompatible
	if(jr->version < MULTI_FS_SERVER_COMPATIBLE_VERSION){
		return JOIN_DENY_JR_BAD_VERSION;
	}

	// Grab the joiner's callsign as we may need this later if they aren't accepted
	strcpy (knock_callsign, jr->callsign);

	// check to make sure we are otherwise in a state to accept
	if((Netgame.game_state != NETGAME_STATE_FORMING) &&
		((Netgame.game_state != NETGAME_STATE_IN_MISSION) || !Cmdline_ingamejoin)){

		// If the game isn't password protected pass it on that someone wants to join
		if (Netgame.mode != NG_MODE_PASSWORD ) {
			sprintf(knock_message, "%s has tried to join!", knock_callsign);
			switch(Netgame.game_state) {
				case NETGAME_STATE_BRIEFING:
				case NETGAME_STATE_PAUSED:
				case NETGAME_STATE_DEBRIEF:
				case NETGAME_STATE_MISSION_SYNC:
					send_game_chat_packet(&Net_players[MY_NET_PLAYER_NUM],knock_message,MULTI_MSG_ALL, NULL, NULL, 1);
					multi_display_chat_msg(knock_message,0,0);
					break;

				// in game we only bother the host. 
				case NETGAME_STATE_IN_MISSION:
					if( MULTIPLAYER_STANDALONE ) {
						send_game_chat_packet(&Net_players[MY_NET_PLAYER_NUM],knock_message,MULTI_MSG_TARGET, Netgame.host, NULL, 1);
					} else {
						snd_play(&Snds[SND_CUE_VOICE]);
						HUD_sourced_printf(HUD_SOURCE_HIDDEN, knock_message);
					}
					break;
			}
		}

		return JOIN_DENY_JR_STATE;
	}

	// the standalone has some oddball situations which we must handle seperately
	if (Game_mode & GM_STANDALONE_SERVER) {		
		// if this is the first connection, he will be the host so we must always accept him
		if(multi_num_players() == 0){
			/*
			   TODO:  We can use this now, but it's not compatible with older builds,
			          so comment it out until the next release

			// check to see if this is a tracker game, and if so make sure this is a valid MT player	
			// we probably eventually want to make sure he's not passing us a fake tracker id#
			if (MULTI_IS_TRACKER_GAME) {
				if (jr->tracker_id < 0) {
					return JOIN_DENY_JR_TRACKER_INVAL; 
				}			
			}
			*/

			// if we're password protected		
			if(std_is_host_passwd() && strcmp(jr->passwd, Multi_options_g.std_passwd)){
				return JOIN_DENY_JR_PASSWD;
			}
				
			// don't allow the host to join as an observer
			if(jr->flags & JOIN_FLAG_AS_OBSERVER){
				return JOIN_DENY_JR_NOOBS;
			} else {
				return -1;
			}
		}		
	}

	// first off check to see if we're violating any of our max players/observers/connections boundaries	
		// if we've already got the full 16 (MAX_PLAYERS) connections - yow
	if( (multi_num_connections() >= MULTI_MAX_CONNECTIONS) ||			
		// if we're full of observers and this guy wants to be an observer
		((multi_num_observers() >= MAX_OBSERVERS) && (jr->flags & JOIN_FLAG_AS_OBSERVER)) ||
		// if we're up to MULTI_MAX_PLAYERS-1 and we're on the standalone
		((multi_num_players() >= (MULTI_MAX_PLAYERS - 1)) && (Game_mode & GM_STANDALONE_SERVER)) ||
		// if we're up to MULTI_MAX_PLAYERS
		(multi_num_players() >= MULTI_MAX_PLAYERS) ||
		// if the max players for a standalone was set
		((Multi_options_g.std_max_players != -1) && (multi_num_players() >= Multi_options_g.std_max_players)) ){

		// we're full buddy - sorry
		return JOIN_DENY_JR_FULL;
	}

	/*
	   TODO:  We can use this now, but it's not compatible with older builds,
			  so comment it out until the next release

	// check to see if this is a tracker game, and if so make sure this is a valid MT player	
	// we probably eventually want to make sure he's not passing us a fake tracker id#
	if (MULTI_IS_TRACKER_GAME) {
		if (jr->tracker_id < 0){
			return JOIN_DENY_JR_TRACKER_INVAL;
		}			
	}
	*/

	// check to see if the player is trying to ingame join in a closed game
	if(MULTI_IN_MISSION && (Netgame.mode == NG_MODE_CLOSED)){
		return JOIN_DENY_JR_CLOSED;
	}	

	// check to see if the player has passed a valid password in a password protected game
	if((Netgame.mode == NG_MODE_PASSWORD) && strcmp(Netgame.passwd,jr->passwd)){
		return JOIN_DENY_JR_PASSWD;
	}

	// check to see if the netgame is forming and is temporarily marked as closed
	if((Netgame.game_state == NETGAME_STATE_FORMING) && (Netgame.flags & NG_FLAG_TEMP_CLOSED)){
		return JOIN_DENY_JR_TEMP_CLOSED;
	}	

	// check to make sure he meets the rank requirement
	if((Netgame.mode == NG_MODE_RANK_ABOVE) && (jr->player_rank < Netgame.rank_base)){
		return JOIN_DENY_JR_RANK_LOW;
	}

	// check to make sure he meets the rank requirement
	if((Netgame.mode == NG_MODE_RANK_BELOW) && (jr->player_rank > Netgame.rank_base)){
		return JOIN_DENY_JR_RANK_HIGH;
	}	

	// can't ingame join a non-dogfight game
/*	if((Netgame.game_state != NETGAME_STATE_FORMING) && !(Netgame.type_flags & NG_TYPE_DOGFIGHT)){
		return JOIN_DENY_JR_TYPE;
	}	*/

	// if the player was banned by the standalone
	if ( (MULTI_IS_TRACKER_GAME && fs2netd_player_banned(addr)) || ((Game_mode & GM_STANDALONE_SERVER) && std_player_is_banned(jr->callsign)) ) {
		// maybe we should log this
		sprintf(knock_message, "Banned user %s with IP: %s attempted to join server", knock_callsign, psnet_addr_to_string(jr_ip_string, addr));
		ml_string(knock_message);
		return JOIN_DENY_JR_BANNED;
	}

	// if the game is in-mission, make sure there are ships available
	if(MULTI_IN_MISSION && !(jr->flags & JOIN_FLAG_AS_OBSERVER)){
		team0_avail = 0;
		team1_avail = 0;
		multi_player_ships_available(&team0_avail,&team1_avail);

		// if there are no ships available on either team
		if((team0_avail == 0) && (team1_avail == 0)){
			return JOIN_DENY_JR_FULL;
		}
	}

	// if my ingame joining flag is set, then deny since we only allow one ingame joiner at a time
	if ( Netgame.flags & NG_FLAG_INGAME_JOINING ){
		return JOIN_DENY_JR_INGAME_JOIN;
	}

	// check to make sure the game is not full (of observers, or players, as appropriate)
	if((jr->flags & JOIN_FLAG_AS_OBSERVER)){
		if((multi_num_observers() + 1) > Netgame.options.max_observers){
			return JOIN_DENY_JR_FULL;
		}
	} 

	// if the netgame is restricted or is team vs. team
	if(Netgame.mode == NG_MODE_RESTRICTED){
		// ingame, we must query the host to see if this guy is accepted
		if(MULTI_IN_MISSION){
			return JOIN_QUERY_RESTRICTED;
		}		
	}		
	
	// check to make sure this player hasn't been kick/banned
	if(multi_kick_is_banned(addr)){
		return JOIN_DENY_JR_BANNED;
	}

	// check to make sure this player doesn't already exist
	if ( find_player(addr) >= 0 ) {
		return JOIN_DENY_JR_DUP;
	}

	return -1;
}

// Karajorma - called by any machine (client, host, server, standalone, etc) if the mission ends by any means
// other than a warpout
void multi_handle_sudden_mission_end()
{
	if (!(MULTIPLAYER_STANDALONE)) {
		multi_msg_text_flush();
	}
}

// called by any machine (client, host, server, standalone, etc), to begin warping out all player objects
void multi_warpout_all_players()
{
	int idx;

	// i'f i'm already marked as warping out, don't do this again
	if(Net_player->flags & NETINFO_FLAG_WARPING_OUT){
		return;
	}

	// stop my afterburners
	if((Player_obj != NULL) && (Player_obj->type == OBJ_SHIP) && !(Game_mode & GM_STANDALONE_SERVER)){
		afterburners_stop( Player_obj, 1 );
	}

	// traverse through each player
	for(idx=0;idx<MAX_PLAYERS;idx++) {
		object *objp;

		// only warpout player _ships_ which are not mine
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx]) && (Objects[Net_players[idx].m_player->objnum].type == OBJ_SHIP)){

			objp = &Objects[Net_players[idx].m_player->objnum];

			obj_set_flags( objp, objp->flags & (~OF_COLLIDES) );
			shipfx_warpout_start( objp );
		}
	}

	// now, mark ourselves as needing to warp out
	Net_player->flags |= NETINFO_FLAG_WARPING_OUT;
	
	// if we're an observer, or we're respawning, or we can't warp out. so just jump into the debrief state
	if((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Net_player->flags & NETINFO_FLAG_RESPAWNING) ||
		(Net_player->flags & NETINFO_FLAG_OBSERVER) || ((Player_obj->type == OBJ_SHIP) && (Player_ship->flags & SF_CANNOT_WARP)) ){		

		multi_handle_sudden_mission_end(); 

		if(Netgame.type_flags & NG_TYPE_DOGFIGHT){
			gameseq_post_event(GS_EVENT_MULTI_DOGFIGHT_DEBRIEF);
		} else {
			gameseq_post_event(GS_EVENT_DEBRIEF);		
		}
	}
	// if we're a ship, then begin the warpout process
	else {
		// turn off collision detection for my ship
		obj_set_flags(Player_obj, Player_obj->flags & (~OF_COLLIDES) );

		// turn off gliding too or ships can't get upt to speed
		if(object_get_gliding(Player_obj)) {
			object_set_gliding(Player_obj, false);
		}

		gameseq_post_event(GS_EVENT_PLAYER_WARPOUT_START_FORCED);				
	}
}

// determine the highest rank of any of the players in the game
int multi_get_highest_rank()
{
	int idx;
	int max_rank = -1;
	
	// go through all the players
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].m_player->stats.rank > max_rank)){
			max_rank = Net_players[idx].m_player->stats.rank;
		}
	}

	// return what we found
	return max_rank;			
}

// called on the machine of the player who hit alt+j
void multi_handle_end_mission_request()
{
	int idx;
	
	// all clients should send the request to the server. no exceptions
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		send_endgame_packet();
	}
	// the server of the game does some further processing
	else {
		ml_string("Server received endgame request, proceeding...");			

		// first we should toss all ingame joiners
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].flags & NETINFO_FLAG_INGAME_JOIN)){
				multi_kick_player(idx,0,KICK_REASON_INGAME_ENDED);
			}
		}		
				
		// send the endgame packet to all clients, who will act on it immediately
		send_endgame_packet();			
		Netgame.game_state = NETGAME_STATE_ENDGAME;		
		send_netgame_update_packet();					
		
		if (Game_mode & GM_STANDALONE_SERVER) {					
			// move to the standalone postgame (which is where we'll handle stats packets, etc)
			gameseq_post_event(GS_EVENT_STANDALONE_POSTGAME);
		}

		// begin the warpout process for all players and myself
		multi_warpout_all_players();
	} 
}

// called to handle any special cases where a player is in some submemu when the game is ended 
void multi_handle_state_special()
{
	int stack_depth,current_depth;	
	
	// first off - kill any active popups
	popup_kill_any_active();

	// kill any popupdeads
	if(popupdead_is_active()){
		popupdead_close();
	}

	// kill off the pilot info popup if its active
	if(multi_pinfo_popup_active()){
		multi_pinfo_popup_kill();
	}

	// now do any special processing for being in states other then the gameplay states
	stack_depth = gameseq_get_depth();
	
	// if we're not pushed on top of any states, do any special state case handling here
	if(stack_depth == 0){		
		// currently there are no special cases, so return
		return;
	} 
	// if we are pushed on any states, post events to pop them off one by one
	else {
		current_depth = stack_depth;
		do {		
			switch(gameseq_get_state(stack_depth - current_depth)){				
			// the hotkey screen
			case GS_STATE_HOTKEY_SCREEN :		
				mission_hotkey_exit();
				Game_do_state_should_skip = 1;
				break;
			// the options menu
			case GS_STATE_OPTIONS_MENU:		
				options_cancel_exit();
				Game_do_state_should_skip = 1;
				break;
			// the hud config (1 deeper in the options menu)
			case GS_STATE_HUD_CONFIG:
				hud_config_cancel();			
				Game_do_state_should_skip = 1;
				break;
			// controls config (1 deeper than the options menu)
			case GS_STATE_CONTROL_CONFIG:
				control_config_cancel_exit();	
				Game_do_state_should_skip = 1;
				break;
			// mission goals screen
			case GS_STATE_SHOW_GOALS:
				mission_goal_exit();
				Game_do_state_should_skip = 1;
				break;			
			// mission log scrollback
			case GS_STATE_MISSION_LOG_SCROLLBACK:
				hud_scrollback_exit();
				Game_do_state_should_skip = 1;
				break;
			// pause screen
			case GS_STATE_MULTI_PAUSED:
				gameseq_pop_state();
				Game_do_state_should_skip = 1;
				break;
			}

			// next pushed state
			current_depth--;
		} while(current_depth > 0);	
	}
}

// called by the file xfer subsytem when we start receiving a file
void multi_file_xfer_notify(int handle)
{
	char *filename;
	int len,idx;
	int force_dir;
	int cf_type;
	int is_mission = 0;	

	// get the filename of the file we are receiving
	filename = NULL;
	filename = multi_xfer_get_filename(handle);

	// get the directory the file is forced into
	force_dir = multi_xfer_get_force_dir(handle);
		
	// something is messed up
	if(filename == NULL){
		return;
	}

	// convert the filename to all lowercase
	len = strlen(filename);
	for(idx=0;idx<len;idx++){
		filename[idx] = (char)tolower(filename[idx]);
	}		

	// if this is a mission file
	is_mission = (strstr(filename, FS_MISSION_FILE_EXT) != NULL);
	
	// determine where its going to go
	if(is_mission){
		cf_type = Net_player->p_info.options.flags & MLO_FLAG_XFER_MULTIDATA ? CF_TYPE_MULTI_CACHE : CF_TYPE_MISSIONS;
	} else {
		cf_type = CF_TYPE_MULTI_CACHE;
	}

	// QUICK FIX
	// check to see if the file is read-only			
	if((strlen(filename) > 0) && !cf_access(filename, cf_type, 00) && (cf_access(filename, cf_type, 02) == -1)){	
		multi_xfer_xor_flags(handle, MULTI_XFER_FLAG_REJECT);

		Net_player->flags &= ~(NETINFO_FLAG_DO_NETWORKING);
		popup(PF_USE_AFFIRMATIVE_ICON, 1, XSTR("&Ok", 713), XSTR("An outdated copy of this file exists, but it cannot be overwritten by the server because it is set to be read-only. Change the permissions on this file next time.", 714));		
		multi_quit_game(PROMPT_NONE);		
		return;
	}	

	// if the incoming filename is a freespace file, set my netplayer state to be "file xfer"
	if(is_mission){
		// we'd better not be xferring a file right now
		Assert(Net_player->s_info.xfer_handle == -1);

		// force into the multidata directory
		multi_xfer_handle_force_dir(handle, cf_type);		
	
		// set my xfer handle
		Net_player->s_info.xfer_handle = handle;
		
		Net_player->state = NETPLAYER_STATE_MISSION_XFER;
		send_netplayer_update_packet();
	}
	// otherwise always hand it off to the multi_data system
	else {
		multi_data_handle_incoming(handle);		
	}
}

// return the lag/disconnected status of the game
#define MULTI_LAG_VAL		400
int multi_query_lag_status()
{
	// -1 == not lagged, 0 == lagged, 1 == disconnected

	// if I'm the server of the game, I can't be lagged
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		return -1;
	}
	
	// if we've been disconnected for some reason or another
	if(multi_client_server_dead()){
		return 1;
	}

	// if our ping time to the server is over a certain time
	if(Netgame.server->s_info.ping.ping_avg >= MULTI_LAG_VAL){
		return 0;
	} 

	// not lagged
	return -1;
}

// process a valid join request
void multi_process_valid_join_request(join_request *jr, net_addr *who_from, int ingame_join_team)
{
	int net_player_num,player_num;
	short id_num;
	
	// create netplayer and player objects for this guy
	net_player_num = multi_find_open_netplayer_slot();
	player_num = multi_find_open_player_slot();
	id_num = multi_get_new_id();
	Assert((net_player_num != -1) && (player_num != -1));			

	// if he is requesting to join as an observer
	if(jr->flags & JOIN_FLAG_AS_OBSERVER){			
		// create the (permanently) observer player
		if(!multi_obs_create_player(net_player_num, jr->callsign, who_from, &Players[player_num])){
			Int3();
		}

		// copy his pilot image filename
		if(strlen(jr->image_filename) > 0){
			strcpy_s(Net_players[net_player_num].m_player->image_filename, jr->image_filename);
		} else {
			strcpy_s(Net_players[net_player_num].m_player->image_filename, "");
		}

		// copy his pilot squad filename
		Net_players[net_player_num].m_player->insignia_texture = -1;
		player_set_squad_bitmap(Net_players[net_player_num].m_player, jr->squad_filename);		

		// clear his multi_data info
		multi_data_handle_join(net_player_num);

		// set some extra flags for him as appropriate
		if(MULTI_IN_MISSION){
			Net_players[net_player_num].flags |= NETINFO_FLAG_INGAME_JOIN;			
		}

		Net_players[net_player_num].flags |= NETINFO_FLAG_CONNECTED;		
		Net_players[net_player_num].player_id = id_num;
		Net_players[net_player_num].tracker_player_id = jr->tracker_id;

		// store pxo info
		if(strlen(jr->pxo_squad_name) > 0){
			strcpy_s(Net_players[net_player_num].p_info.pxo_squad_name, jr->pxo_squad_name);
		} else {
			strcpy_s(Net_players[net_player_num].p_info.pxo_squad_name, "");
		}		

		// if he's using hacked data
		if(jr->flags & JOIN_FLAG_HAXOR){
			Net_players[net_player_num].flags |= NETINFO_FLAG_HAXOR;
		}

		// set his reliable connect time
		Net_players[net_player_num].s_info.reliable_connect_time = (int) time(NULL);

		// send the accept packet here
		send_accept_packet(net_player_num, (Net_players[net_player_num].flags & NETINFO_FLAG_INGAME_JOIN) ? ACCEPT_OBSERVER | ACCEPT_INGAME : ACCEPT_OBSERVER);
	} else {
		// create the player object	
		if(!multi_create_player( net_player_num, &Players[player_num], jr->callsign, who_from, -1, id_num )){
			Int3();
		}
		
		// copy his pilot image filename
		if(strlen(jr->image_filename) > 0){
			strcpy_s(Net_players[net_player_num].m_player->image_filename, jr->image_filename);
		} else {
			strcpy_s(Net_players[net_player_num].m_player->image_filename, "");
		}

		// copy his pilot squad filename		
		Net_players[net_player_num].m_player->insignia_texture = -1;
		player_set_squad_bitmap(Net_players[net_player_num].m_player, jr->squad_filename);				

		// clear his multi_data info
		multi_data_handle_join(net_player_num);

		// mark him as being connected
		Net_players[net_player_num].flags |= NETINFO_FLAG_CONNECTED;
						
		// set his tracker id correctly
		Net_players[net_player_num].tracker_player_id = jr->tracker_id;				

		// set his player id#
		Net_players[net_player_num].player_id = id_num;

		// store pxo info
		if(strlen(jr->pxo_squad_name) > 0){
			strcpy_s(Net_players[net_player_num].p_info.pxo_squad_name, jr->pxo_squad_name);
		} else {
			strcpy_s(Net_players[net_player_num].p_info.pxo_squad_name, "");
		}		

		// if he's using hacked data
		if(jr->flags & JOIN_FLAG_HAXOR){
			Net_players[net_player_num].flags |= NETINFO_FLAG_HAXOR;
		}
		
		// flag him appropriately if he's doing an ingame join
		if(MULTI_IN_MISSION){
	//		Int3(); //-C Ingame stuff
			Net_players[net_player_num].flags |= NETINFO_FLAG_INGAME_JOIN;
			Net_players[net_player_num].s_info.ingame_join_flags = 0;
		}		

		// set his reliable connect time
		Net_players[net_player_num].s_info.reliable_connect_time = (int) time(NULL);

		// if he's joining as a host (on the standalone)
		if(Net_players[net_player_num].flags & NETINFO_FLAG_GAME_HOST){
			send_accept_packet(net_player_num, ACCEPT_HOST);

			Netgame.host = &Net_players[net_player_num];

			// set the game and player states appropriately
			Netgame.game_state = NETGAME_STATE_STD_HOST_SETUP;			
		}
		// if he's joining ingame
		else if(Net_players[net_player_num].flags & NETINFO_FLAG_INGAME_JOIN){
			// if we're in team vs. team mode
			if(Netgame.type_flags & NG_TYPE_TEAM)
			{
			/*	int i, j;
				int team_nums[MULTI_TS_MAX_TVT_TEAMS] = {0, 0};\

				//First get the number of players on each team
				for(i = 0; i < MULTI_TS_MAX_TVT_TEAMS; i++)
				{
					for(j = 0; j < MULTI_TS_NUM_SHIP_SLOTS; j++)
					{
						if(Multi_ts_team[i].multi_ts_flag != MULTI_TS_FLAG_EMPTY && Multi_ts_team[i].multi_ts_flag != MULTI_TS_FLAG_NONE)
						{
							team_nums[i]++;
						}
					}
				}
				//Find the lowest team
				//Init this to the first team, so it works properly
				int lowest_team[2] = {team_nums[0], 0};
				for(i = 0; i < MULTI_TS_MAX_TVT_TEAMS;i++)
				{
					if(Multi_ts_team[i] < lowest_team[0])
					{
						lowest_team[0] Multi_ts_team[i];
						lowest_team[1] = i;
					}
				}
				
				ingame_join_team = lowest_team[1];*/
				//Assert(ingame_join_team != -1);

				Net_players[net_player_num].p_info.team = ingame_join_team;
			}

			send_accept_packet(net_player_num, ACCEPT_INGAME, ingame_join_team);

			// set his last full update time for updating him on ingame join ships
			Net_players[net_player_num].s_info.last_full_update_time = timestamp(INGAME_SHIP_UPDATE_TIME);
		} 
		// if he's joining as an otherwise ordinary client
		else {
			send_accept_packet(net_player_num, ACCEPT_CLIENT);
		}
	}

	// set my ingame joining flag if the new guy is joining ingame
	if ( Net_players[net_player_num].flags & NETINFO_FLAG_INGAME_JOIN ){
//		Int3(); //-C Ingame stuff
		Netgame.flags |= NG_FLAG_INGAME_JOINING;
	}	
	
	// copy in his options
	memcpy(&Net_players[net_player_num].p_info.options, &jr->player_options, sizeof(multi_local_options));

	// if on the standalone, then do any necessary gui updating
	if (Game_mode & GM_STANDALONE_SERVER) {		
		std_add_player(&Net_players[net_player_num]);
		std_connect_set_connect_count();
		std_connect_set_host_connect_status();
	} else {
		// let the create game screen know someone has joined
		if(gameseq_get_state() == GS_STATE_MULTI_HOST_SETUP){
			multi_create_handle_join(&Net_players[net_player_num]);
		}
	}

	extern int Multi_client_update_times[MAX_PLAYERS];
	Multi_client_update_times[net_player_num] = -1;

	// notify datarate
	multi_rate_reset(net_player_num);
}

// if a player is trying to join a restricted game, evaluate the keypress (accept or not, etc)
int multi_process_restricted_keys(int k)
{	
	int key1=-1,key2=-1;	//JAS: Get rid of optimized warning
	int team_val;
	
	// if the query timestamp is not set, don't do anything
	if(Multi_restr_query_timestamp == -1){
		return 0;
	}

	// determine what keys to look for based upon the mode we're in
	switch(Multi_join_restr_mode){
	// normal restricted join, Y or N
	case MULTI_JOIN_RESTR_MODE_1:
		key1 = KEY_Y;
		key2 = KEY_N;
		break;

	// team vs team, team 0 only has ships
	case MULTI_JOIN_RESTR_MODE_2:
		key1 = KEY_Y;
		key2 = KEY_N;
		break;

	// team vs team, team 1 only has ships		
	case MULTI_JOIN_RESTR_MODE_3:
		key1 = KEY_Y;
		key2 = KEY_N;
		break;

	// team vs team, both teams have ships
	case MULTI_JOIN_RESTR_MODE_4:
		key1 = KEY_1;
		key2 = KEY_2;
		break;
	
	// illegal mode
	default :
		Int3();
	}	

	// check the keypress
	if((k == key1) || (k == key2)){
		// unset the timestamp
		Multi_restr_query_timestamp = -1;			

		// MWA -- 5/26/98.  Next line commented out.  It should be cleared when the ingame joiner
		// actually gets into the mission
		//Netgame.flags &= ~(NG_FLAG_INGAME_JOINING);

		// determine which team to put him on (if any)
		switch(Multi_join_restr_mode){
		// normal restricted join, Y or N
		case MULTI_JOIN_RESTR_MODE_1:
			team_val = (k == key1) ? 0 : -1;
			break;

		// team vs team, team 0 only has ships
		case MULTI_JOIN_RESTR_MODE_2:
			team_val = (k == key1) ? 0 : -1;			
			break;

		// team vs team, team 1 only has ships		
		case MULTI_JOIN_RESTR_MODE_3:
			team_val = (k == key1) ? 1 : -1;			
			break;

		// team vs team, both teams have ships
		case MULTI_JOIN_RESTR_MODE_4:
			team_val = (k == key1) ? 0 : 1;			
			break;
	
		// illegal mode
		default :
			team_val = -1;	// JAS: Get rid of optimized warning
			Int3();
		}				

		// perform the proper response	
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			if(team_val >= 0){			
				multi_process_valid_join_request(&Multi_restr_join_request,&Multi_restr_addr,team_val);			
			}
		}
		// otherwise tell the standalone to accept him
		else {
			if(team_val >= 0){
				send_host_restr_packet("null",1,team_val);
			} else {
				send_host_restr_packet("null",2,-1);
			}
		}

		// processed a key
		return 1;
	}
	
	// didn't process any keys
	return 0;
}

// determine the status of available player ships (use team_0 for non team vs. team situations)
void multi_player_ships_available(int *team_0,int *team_1)
{
	ship_obj *moveup;
	int mp_team_num;
	
	*team_0 = 0;
	*team_1 = 0;
	
	moveup = GET_FIRST(&Ship_obj_list);
	while(moveup!=END_OF_LIST(&Ship_obj_list)){
		// if this ship is flagged as OF_COULD_BE_PLAYER
		if(Objects[moveup->objnum].flags & OF_COULD_BE_PLAYER){
			// get the team # for this ship
			mp_team_num = multi_ts_get_team(Ships[Objects[moveup->objnum].instance].ship_name);
			if(mp_team_num == 0){
				(*team_0)++;
			} else if(mp_team_num == 1){
				(*team_1)++;
			}
		}
		
		moveup = GET_NEXT(moveup);
	}	
}

// server should update the player's bank/link status with the data in the passed ship
void multi_server_update_player_weapons(net_player *pl,ship *shipp)
{
	// don't process when the ship is dying.
	if ( (shipp->flags & SF_DYING) || NETPLAYER_IS_DEAD(pl) )
		return;

	// primary bank status
	pl->s_info.cur_primary_bank = (char)shipp->weapons.current_primary_bank;

	// primary link status
	pl->s_info.cur_link_status &= ~(1<<0);
	if(shipp->flags & SF_PRIMARY_LINKED){
		pl->s_info.cur_link_status |= (1<<0);
	}

	// secondary bank status
	if ( shipp->weapons.current_secondary_bank < 0 ) {
		nprintf(("Network", "bashing %s's current sbank to 0\n", shipp->ship_name));
		shipp->weapons.current_secondary_bank = 0;
	}
	pl->s_info.cur_secondary_bank = (char)shipp->weapons.current_secondary_bank;

	// secondary link status
	pl->s_info.cur_link_status &= ~(1<<1);
	if(shipp->flags & SF_SECONDARY_DUAL_FIRE){
		pl->s_info.cur_link_status |= (1<<1);
	}

	// ets values
	pl->s_info.ship_ets = 0x0000;
	// shield ets		
	pl->s_info.ship_ets |= ((ushort)shipp->shield_recharge_index << 8);
	// weapon ets
	pl->s_info.ship_ets |= ((ushort)shipp->weapon_recharge_index << 4);
	// engine ets
	pl->s_info.ship_ets |= ((ushort)shipp->engine_recharge_index);

	Assert( pl->s_info.ship_ets != 0 );
}

// flush the multidata cache directory
void multi_flush_multidata_cache()
{
	nprintf(("Network","FLUSHING MULTIDATA CACHE\n"));
	
	// call the cfile function to flush the directory
	cfile_flush_dir(CF_TYPE_MULTI_CACHE);
}

// flush all data from a previous mission before starting the next
void multi_flush_mission_stuff()
{
	int idx;
	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx])){			
			// unset all unneeded status bits
			Net_players[idx].flags &= ~(NETINFO_FLAG_MISSION_OK | NETINFO_FLAG_RESPAWNING | NETINFO_FLAG_WARPING_OUT);
			
			// server is always "mission ok"
			if(Net_players[idx].flags & NETINFO_FLAG_AM_MASTER){
				Net_players[idx].flags |= NETINFO_FLAG_MISSION_OK;
			}

			// if this guy is a non-permanent observer, unset the appropriate flags
			if(MULTI_TEMP_OBSERVER(Net_players[idx])){
				Net_players[idx].flags &= ~(NETINFO_FLAG_OBSERVER | NETINFO_FLAG_OBS_PLAYER);
			}

			// misc
			multi_ping_reset(&Net_players[idx].s_info.ping);				
			Net_players[idx].s_info.num_last_buttons = 0;
			Net_players[idx].s_info.wing_index_backup = 0;
			Net_players[idx].s_info.wing_index = 0;
			Net_players[idx].p_info.ship_class = -1;
			Net_players[idx].p_info.ship_index = -1;
			Net_players[idx].s_info.xfer_handle = -1;

			// ack handles
			Net_players[idx].s_info.wing_index_backup = 0;
				
			// objnum 
			Players[idx].objnum = -1;
		}
	}	

	// reset netgame stuff
	Netgame.flags &= ~(NG_FLAG_TEMP_CLOSED);		

	multi_xfer_reset();

	// standalone servers should clear their goal trees now
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_multi_setup_goal_tree();
	}
	
	// object signatures
	// this will eventually get reset to Netgame.security the next time an object gets its signature assigned.
	// We do this to resynchronize the host/server and all clients
	Next_ship_signature = SHIP_SIG_MIN;
	Next_asteroid_signature = ASTEROID_SIG_MIN;
	Next_non_perm_signature = NPERM_SIG_MIN;

	// everyone will need to either reload the current mission, leave, or load the next mission, so in any case
	Multi_mission_loaded = 0;
}

// should we ignore all controls and keypresses because of some multiplayer 
int multi_ignore_controls(int key)
{		
	// if the multiplayer text messaging system is active, don't return any keys	
	if((key > 0) && multi_msg_text_process(key)){
		return 1;
	}

	// if the host of the game is being prompted to accept or deny a player in a restricted game
	if((key > 0) && multi_process_restricted_keys(key)){
		return 1;
	}
	
	// if we're in text messaging mode, ignore controls
	if(multi_msg_text_mode()){
		return 1;
	}	

	// if the pause system wants to eat keys for a while
	if(multi_pause_eat_keys()){
		return 1;
	}

	// multiplayer didn't eat the key
	return 0;
}

// if the kill limit has been reached by any given player
int multi_kill_limit_reached()
{
	int idx;

	// is the kill limit <= 0 ?
	// if so, consider it as _no_ kill limit
	if(Netgame.options.kill_limit <= 0){
		return 0;
	}
	
	// look through all active, non-observer players
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx]) && (Net_players[idx].m_player->stats.m_kill_count_ok >= Netgame.options.kill_limit)){
			// someone reached the limit
			return 1;
		}
	}

	// limit has not been reached yet
	return 0;
}

// display a chat message (write to the correct spot - hud, standalone gui, chatbox, etc)
void multi_display_chat_msg(char *msg, int player_index, int add_id)
{
	// if i'm a standalone, always add to the gui
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_add_chat_text(msg,player_index,add_id);
		return;
	}
	
	// in gameplay
	if(Game_mode & GM_IN_MISSION){					
		// if we're paused, send it to the chatbox
		if(Multi_pause_status){
			chatbox_add_line(msg, player_index, add_id);
		}
		// otherwise print to the HUD
		else {
			multi_msg_display_mission_text(msg, player_index);
		}
	}
	// otherwise add it to the chatbox
	else {
		chatbox_add_line(msg, player_index, add_id);
	}		
}

// fill in Current_file_checksum and Current_file_length
void multi_get_mission_checksum(char *filename)
{
	CFILE *in;

	Multi_current_file_checksum = 0xffff;
	Multi_current_file_length = -1;

	// get the filename
	in = cfopen(filename,"rb");
	if(in != NULL){
		// get the length of the file
		Multi_current_file_length = cfilelength(in);
		cfclose(in);

		in = cfopen(filename,"rb");
		if(in != NULL){
			// get the checksum of the file
			cf_chksum_short(in,&Multi_current_file_checksum);

			// close the file
			cfclose(in);
			in = NULL;
		}
		// if the file doesn't exist, setup some special values, so the server recognizes this
		else {
			Multi_current_file_checksum = 0xffff;
			Multi_current_file_length = -1;
		}
	} else {
		// don't transfew builtin missions
		if(multi_is_builtin_mission()){	
			multi_quit_game(PROMPT_ALL, MULTI_END_NOTIFY_KICKED_CANT_XFER);
		}
	}
	nprintf(("Network","NET FILE CHECKSUM : %d %d\n",Multi_current_file_checksum,Multi_current_file_length));
}

char multi_unit_to_char(float unit)
{
	char ret;
	
	if(unit > 1.0f){
		Int3();
		unit = 1.0f;
	}
	if(unit < -1.0f){
		Int3();
		unit = -1.0f;
	}

	ret = (char)(unit * 127.0f);
	return ret;
}

float multi_char_to_unit(float val)
{
	float ret;

	ret = (float)val / 127.0f;
	if(ret > 1.0f){
		Int3();
		ret = 1.0f;
	}
	if(ret < -1.0f){
		Int3();
		ret = -1.0f;
	}

	return ret;
}

// if we should render our ping time to the server in a multiplayer game
int multi_show_ingame_ping()
{
	// always show it for now
	return 1;
}

int multi_get_connection_speed()
{
	int cspeed;
	char *connection_speed;

#ifdef _WIN32	
	connection_speed = os_config_read_string(NULL, "ConnectionSpeed", "");	
#else
	connection_speed = os_config_read_string(NULL, "ConnectionSpeed", "Fast");
#endif

	if ( !stricmp(connection_speed, NOX("Slow")) ) {
		cspeed = CONNECTION_SPEED_288;
	} else if ( !stricmp(connection_speed, NOX("56K")) ) {
		cspeed = CONNECTION_SPEED_56K;
	} else if ( !stricmp(connection_speed, NOX("ISDN")) ) {
		cspeed = CONNECTION_SPEED_SISDN;
	} else if ( !stricmp(connection_speed, NOX("Cable")) ) {
		cspeed = CONNECTION_SPEED_CABLE;
	} else if ( !stricmp(connection_speed, NOX("Fast")) ) {
		cspeed = CONNECTION_SPEED_T1;
	} else {
		cspeed = CONNECTION_SPEED_NONE;
	}

	return cspeed;
}

// return a MVALID_STATUS_* define based upon the passed string
int multi_string_to_status(char *valid_string)
{
	if (strstr(valid_string, "invalid"))
		return MVALID_STATUS_INVALID;

	if (strstr(valid_string, "valid"))
		return MVALID_STATUS_VALID;


	return MVALID_STATUS_UNKNOWN;	
}

// if we're in tracker mode, do a validation update on all known missions
void multi_update_valid_missions()
{
	char next_filename[MAX_FILENAME_LEN+1];	
	char next_line[512];
	char status_string[50];
	char temp[256];
	char *tok;
	CFILE *in;
	int file_index;
	uint idx;	
	bool was_cancelled = false;

	// if we're a standalone, show a dialog saying "validating missions"
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_create_gen_dialog("Validating missions");
		std_gen_set_text("Querying:", 1);
	}

	Assert( MULTI_IS_TRACKER_GAME );

	// mark all missions on our list as being MVALID_STATUS_UNKNOWN
	for (idx = 0; idx < Multi_create_mission_list.size(); idx++) {
		Multi_create_mission_list[idx].valid_status = MVALID_STATUS_UNKNOWN;
	}
	
	// attempt to open the valid mission config file
	in = cfopen(MULTI_VALID_MISSION_FILE, "rt", CFILE_NORMAL, CF_TYPE_DATA);

	if (in != NULL) {		
		// read in all listed missions
		while ( !cfeof(in) ) {
			// read in a line
			memset(next_line, 0, 512);
			cfgets(next_line, 512, in);
			drop_trailing_white_space(next_line);
			drop_leading_white_space(next_line);

			// read in a filename
			memset(next_filename, 0, MAX_FILENAME_LEN+1);
			memset(temp, 0, 256);
			tok = strtok(next_line, " ");

			if (tok == NULL)
				continue;

			strcpy_s(temp, tok);
			drop_trailing_white_space(temp);
			drop_leading_white_space(temp);
			strcpy_s(next_filename, temp);
			
			// read in the status string
			memset(status_string, 0, 50);
			memset(temp, 0, 256);
			tok = strtok(NULL," \n");

			if (tok == NULL)
				continue;

			strcpy_s(temp, tok);
			drop_trailing_white_space(temp);
			drop_leading_white_space(temp);
			strcpy_s(status_string, temp);

			// try and find the file
			file_index = multi_create_lookup_mission(next_filename);
	
			if (file_index >= 0)
				Multi_create_mission_list[file_index].valid_status = (char)multi_string_to_status(status_string);
		}

		// close the infile
		cfclose(in);
		in = NULL;	
	}

	// now poll for all unknown missions
	was_cancelled = !(fs2netd_get_valid_missions());

	// if the operation was cancelled, don't write anything new
	if (was_cancelled) {
		// if we're a standalone, kill the validate dialog
		if(Game_mode & GM_STANDALONE_SERVER){
			std_destroy_gen_dialog();
		}

		return;
	}

	// now rewrite the outfile with the new mission info
	in = cfopen(MULTI_VALID_MISSION_FILE, "wt", CFILE_NORMAL, CF_TYPE_DATA);
	if(in == NULL){
		// if we're a standalone, kill the validate dialog
		if(Game_mode & GM_STANDALONE_SERVER){
			std_destroy_gen_dialog();
		}

		return;
	}

	for (idx = 0; idx < Multi_create_mission_list.size(); idx++) {
		switch(Multi_create_mission_list[idx].valid_status){
		case MVALID_STATUS_VALID:
			cfputs(Multi_create_mission_list[idx].filename, in);
			cfputs(NOX("   valid"), in);
			cfputs(NOX("\n"), in);
			break;

		case MVALID_STATUS_INVALID:
			cfputs(Multi_create_mission_list[idx].filename, in);
			cfputs(NOX("   invalid"), in);
			cfputs(NOX("\n"), in);
			break;
		}
	}

	// close the outfile
	cfclose(in);
	in = NULL;

	// if we're a standalone, kill the validate dialog
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_destroy_gen_dialog();
	}
}

// get a new id# for a player
short multi_get_new_id()
{
	if(Multi_id_num > 20000){
		Multi_id_num = 0;
	} 

	return Multi_id_num++;
}


// ------------------------------------

//XSTR:OFF
DCF(multi,"changes multiplayer settings")
{
	if(Dc_command){
		dc_get_arg(ARG_STRING);
		
		if(strcmp(Dc_arg, "kick")==0){				// kick a player
			multi_dcf_kick();
#ifndef NDEBUG
		} else if(strcmp(Dc_arg, "stats")==0) {
			// multi_toggle_stats();
		} else if(strcmp(Dc_arg, "show_stats")==0) {
			// multi_show_basic_stats(0);
		} else if(strcmp(Dc_arg, "dump_stats")==0) {
			// multi_show_basic_stats(1);
#endif
		} else if(strcmp(Dc_arg, "voice")==0){				// settings for multiplayer voice
			multi_voice_dcf();
		} else if(strcmp(Dc_arg, "respawn_chump")==0){	// set a really large # of respawns
			if((Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_GAME_HOST)){			
				Netgame.respawn = 9999;
				Netgame.options.respawn = 9999;				

				// if i'm the server, send a netgame update
				if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
					send_netgame_update_packet();
				} 
			}
		} else if(strcmp(Dc_arg, "ss_leaders")==0){		// only host or team captains can modify ships
			if((Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_GAME_HOST)){			
				Netgame.options.flags |= MSO_FLAG_SS_LEADERS;
				multi_options_update_netgame();
			}
		} else if(strcmp(Dc_arg, "make_players")==0){
#ifndef NDEBUG
			multi_make_fake_players(MAX_PLAYERS);
#endif
		} else if(strcmp(Dc_arg, "givecd")==0){
			extern int Multi_has_cd;
			Multi_has_cd = 1;
		} else if(strcmp(Dc_arg, "oo")==0){						
			int new_flags = -1;

			dc_get_arg(ARG_INT);
			if(Dc_arg_type & ARG_INT){
				new_flags = Dc_arg_int;
			}

			dc_printf("Interesting flags\nPos : %d\nVelocity : %d\nDesired vel : %d\nOrient : %d\nRotvel : %d\nDesired rotvel %d\n",
						 1<<0, 1<<7, 1<<8, 1<<1, 1<<9, 1<<10);						
		} else if(strcmp(Dc_arg, "oo_sort")==0){			
			extern int OO_sort;

			OO_sort = !OO_sort;
			if(OO_sort){
				dc_printf("Network object sorting ENABLED\n");
			} else {
				dc_printf("Network object sorting DISABLED\n");
			}
		}
	}
}

//XSTR:ON

// PXO crc checking stuff


void multi_spew_pxo_checksums(int max_files, char *outfile)
{
	char **file_names;
	char full_name[MAX_PATH_LEN];
	char wild_card[10];
	int count = 0, idx;
	uint checksum;
	FILE *out;
	char description[512] = { 0 };
	char filename[65] = { 0 };
	char gametype[32] = { 0 };
	size_t offset = 0;
	char *p = NULL;

	// allocate filename space	
	file_names = (char**)vm_malloc(sizeof(char*) * max_files);

	if (file_names != NULL) {
		memset(wild_card, 0, 10);
		strcpy_s(wild_card, NOX("*"));
		strcat_s(wild_card, FS_MISSION_FILE_EXT);
		count = cf_get_file_list(max_files, file_names, CF_TYPE_MISSIONS, wild_card);	

		if (count <= 0)
			goto Done;

		cf_create_default_path_string(full_name, sizeof(full_name) - 1, CF_TYPE_ROOT, outfile);

		// open the outfile
		out = fopen(full_name, "wt");

		if (out == NULL)
			goto Done;

		p = Cmdline_spew_mission_crcs;

		while (*p && (offset < sizeof(description))) {
			if (*p == '"') {
				description[offset++] = '"';
				description[offset++] = '"';
			} else {
				description[offset++] = *p;
			}

			p++;
		}

		// header
		fprintf(out, "filename,CRC32,mission type,max players,description\r\n");

		// do all the checksums
		for (idx = 0; idx < count; idx++) {
			memset( full_name, 0, sizeof(full_name) );			
			strcpy_s( full_name, cf_add_ext(file_names[idx], FS_MISSION_FILE_EXT) );

			if ( !cf_chksum_long(full_name, &checksum) ) {
				continue;
			}

			if (get_mission_info(full_name)) {
				continue;
			}

			if ( !(The_mission.game_type & MISSION_TYPE_MULTI) ) {
				continue;
			}

			offset = 0;
			p = full_name;

			while (*p && (offset < sizeof(filename))) {
				if (*p == '"') {
					filename[offset++] = '"';
					filename[offset++] = '"';
				} else {
					filename[offset++] = *p;
				}

				p++;
			}

			filename[offset] = '\0';

			if (IS_MISSION_MULTI_DOGFIGHT) {
				strcpy_s(gametype, "dogfight");
			} else if (IS_MISSION_MULTI_COOP) {
				strcpy_s(gametype, "coop");
			} else if (IS_MISSION_MULTI_TEAMS) {
				strcpy_s(gametype, "TvT");
			}

			fprintf(out, "\"%s\",%u,\"%s\",%d,\"%s\"\r\n", filename, checksum, gametype, The_mission.num_players, description);
		}

		fflush(out);
		fclose(out);

Done:
		if (file_names != NULL) {
			for (idx = 0; idx < count; idx++) {
				if (file_names[idx] != NULL) {
					vm_free(file_names[idx]);
					file_names[idx] = NULL;
				}
			}

			vm_free(file_names);
			file_names = NULL;
		}
	}
}

/*
void multi_spew_table_checksums(int max_files, char *outfile)
{
	char **file_names;
	char full_name[MAX_PATH_LEN];
	int count, idx;
	uint checksum;
	FILE *out = NULL;
	char modname[128];
	time_t my_time = 0;

	// allocate filename space	
	file_names = (char**)vm_malloc(sizeof(char*) * max_files);

	if (file_names != NULL) {
		count = cf_get_file_list(max_files, file_names, CF_TYPE_TABLES, NOX("*.tbl"));	

		if (count <= 0)
			goto Done;

		cf_create_default_path_string(full_name, sizeof(full_name) - 1, CF_TYPE_ROOT, outfile);

		// open the outfile
		out = fopen(full_name, "wt");

		if (out == NULL)
			goto Done;

		memset( modname, 0, sizeof(modname) );
		strcpy_s( modname, Cmdline_spew_table_crcs );

		my_time = time(NULL);
	
		fprintf(out, "--  Table CRCs generated on %s \n", ctime(&my_time));

		fprintf(out, "LOCK TABLES `fstables` WRITE;\n");
		fprintf(out, "INSERT INTO `fstables` VALUES ");

		// do all the checksums
		for (idx = 0; idx < count; idx++) {
			memset( full_name, 0, sizeof(full_name) );			
			strcpy_s( full_name, cf_add_ext(file_names[idx], ".tbl") );

			if ( cf_chksum_long(full_name, &checksum) ) {
				if (idx == 0)
					fprintf(out, "('%s',%u,'%s')", full_name, checksum, modname);
				else
					fprintf(out, ",('%s',%u,'%s')", full_name, checksum, modname);
			}
		}

		fprintf(out, ";\n");
		fprintf(out, "UNLOCK TABLES;\n");

		fclose(out);

Done:
		for (idx = 0; idx < count; idx++) {
			if (file_names[idx] != NULL) {
				vm_free(file_names[idx]);
				file_names[idx] = NULL;
			}
		}

		vm_free(file_names);
		file_names = NULL;
	}
}
*/

DCF(pxospew,"spew PXO 32 bit checksums for all visible mission files")
{
	int max_files;

	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){
		max_files = Dc_arg_int;	

		dc_get_arg(ARG_STRING);
		if(Dc_arg_type & ARG_STRING){
			multi_spew_pxo_checksums(max_files, Dc_arg);
		}
	}
}



// make a bunch of fake players - don't rely on this to be very safe - its mostly used for interface testing
#ifndef NDEBUG
void multi_make_fake_players(int count)
{
	int idx;
	
	for(idx=0;idx<count;idx++){
		if(!MULTI_CONNECTED(Net_players[idx])){
			Net_players[idx].m_player = &Players[idx];
			sprintf(Net_players[idx].m_player->callsign,"Player %d",idx);
			Net_players[idx].flags |= NETINFO_FLAG_CONNECTED;
		}
	}
}
#endif

// ---------------------------------------------------------------------------------------------------------------------
// PACK UNPACK STUFF
//

#pragma optimize("", off)

typedef struct bitbuffer {
	ubyte		mask;
   int		rack;
	ubyte		*data;
	ubyte		*org_data;
} bitbuffer;

void bitbuffer_init( bitbuffer *bitbuf, ubyte *data )
{
	bitbuf->rack = 0;	
	bitbuf->mask = 0x80;
	bitbuf->data = data;
	bitbuf->org_data = data;
}

int bitbuffer_write_flush( bitbuffer *bitbuf )
{
	// Flush to next byte
	if ( bitbuf->mask != 0x80 )	{
   	*bitbuf->data++ = (ubyte)bitbuf->rack;
	}
	return bitbuf->data-bitbuf->org_data;
}

int bitbuffer_read_flush( bitbuffer *bitbuf )
{
	return bitbuf->data-bitbuf->org_data;
}

void bitbuffer_put( bitbuffer *bitbuf, uint data, int bit_count ) 
{
	uint mask;

	mask = 1L << ( bit_count - 1 );
	while ( mask != 0) {
		if ( mask & data )	{
			bitbuf->rack |= bitbuf->mask;
		}
		bitbuf->mask >>= 1;
		if ( bitbuf->mask == 0 ) {
			*bitbuf->data++=(ubyte)bitbuf->rack;
			bitbuf->rack = 0;
			bitbuf->mask = 0x80;
		}
		mask >>= 1;
	}
}

uint bitbuffer_get_unsigned( bitbuffer *bitbuf, int bit_count ) 
{
	uint mask;
	uint return_value;

	mask = 1L << ( bit_count - 1 );
	return_value = 0;

	while ( mask != 0)	{
		if ( bitbuf->mask == 0x80 ) {
			bitbuf->rack = *bitbuf->data++;
		}
		if ( bitbuf->rack & bitbuf->mask )	{
			return_value |= mask;
		}
		mask >>= 1;
		bitbuf->mask >>= 1;
		if ( bitbuf->mask == 0 )	{
			bitbuf->mask = 0x80;
		}
	}

	return return_value;
}

int bitbuffer_get_signed( bitbuffer *bitbuf, int bit_count ) 
{
	uint mask;
	uint return_value;

	mask = 1L << ( bit_count - 1 );
	return_value = 0;

	while ( mask != 0)	{
		if ( bitbuf->mask == 0x80 ) {
			bitbuf->rack = *bitbuf->data++;
		}
		if ( bitbuf->rack & bitbuf->mask )	{
			return_value |= mask;
		}
		mask >>= 1;
		bitbuf->mask >>= 1;
		if ( bitbuf->mask == 0 )	{
			bitbuf->mask = 0x80;
		}
	}

	// sign extend return value
	return_value <<= (32-bit_count);
	
	return ((int)return_value)>>(32-bit_count);
}
	


// Packs/unpacks an object position.
// Returns number of bytes read or written.
// #define OO_POS_RET_SIZE							9
int multi_pack_unpack_position( int write, ubyte *data, vec3d *pos)
{
	bitbuffer buf;

	bitbuffer_init(&buf,data);

	int a, b, c;

	if ( write )	{
		// Output pos

		a = fl2i(pos->xyz.x*105.0f+0.5f); 
		b = fl2i(pos->xyz.y*105.0f+0.5f);
		c = fl2i(pos->xyz.z*105.0f+0.5f);
		CAP(a,-8388608,8388607);
		CAP(b,-8388608,8388607);
		CAP(c,-8388608,8388607);
		
		bitbuffer_put( &buf, (uint)a, 24 );
		bitbuffer_put( &buf, (uint)b, 24 );
		bitbuffer_put( &buf, (uint)c, 24 );


		return bitbuffer_write_flush(&buf);

	} else {

		// unpack pos
		a = bitbuffer_get_signed(&buf,24);
		b = bitbuffer_get_signed(&buf,24);
		c = bitbuffer_get_signed(&buf,24);

		pos->xyz.x = i2fl(a)/105.0f;
		pos->xyz.y = i2fl(b)/105.0f;
		pos->xyz.z = i2fl(c)/105.0f;

		return bitbuffer_read_flush(&buf);
	}
}

int degenerate_count = 0;
int non_degenerate_count = 0;

/*
hack = ((ushort)orient->vec.fvec.x * 32767);
			memcpy(&hack, &orient->vec.fvec.x, 4);
			bitbuffer_put( &buf, hack, 32  );
			memcpy(&hack, &orient->vec.fvec.y, 4);
			bitbuffer_put( &buf, hack, 32  );
			memcpy(&hack, &orient->vec.fvec.z, 4);
			bitbuffer_put( &buf, hack, 32  );

			memcpy(&hack, &orient->vec.uvec.x, 4);
			bitbuffer_put( &buf, hack, 32  );
			memcpy(&hack, &orient->vec.uvec.y, 4);
			bitbuffer_put( &buf, hack, 32  );
			memcpy(&hack, &orient->vec.uvec.z, 4);
			bitbuffer_put( &buf, hack, 32  );

			memcpy(&hack, &orient->vec.rvec.x, 4);
			bitbuffer_put( &buf, hack, 32  );
			memcpy(&hack, &orient->vec.rvec.y, 4);
			bitbuffer_put( &buf, hack, 32  );
			memcpy(&hack, &orient->vec.rvec.z, 4);
			bitbuffer_put( &buf, hack, 32  );*/

/*
hack = bitbuffer_get_unsigned(&buf, 32);
			memcpy(&orient->vec.fvec.x, &hack, 4);
			hack = bitbuffer_get_unsigned(&buf, 32);
			memcpy(&orient->vec.fvec.y, &hack, 4);
			hack = bitbuffer_get_unsigned(&buf, 32);
			memcpy(&orient->vec.fvec.z, &hack, 4);

			hack = bitbuffer_get_unsigned(&buf, 32);
			memcpy(&orient->vec.uvec.x, &hack, 4);
			hack = bitbuffer_get_unsigned(&buf, 32);
			memcpy(&orient->vec.uvec.y, &hack, 4);
			hack = bitbuffer_get_unsigned(&buf, 32);
			memcpy(&orient->vec.uvec.z, &hack, 4);

			hack = bitbuffer_get_unsigned(&buf, 32);
			memcpy(&orient->vec.rvec.x, &hack, 4);
			hack = bitbuffer_get_unsigned(&buf, 32);
			memcpy(&orient->vec.rvec.y, &hack, 4);
			hack = bitbuffer_get_unsigned(&buf, 32);
			memcpy(&orient->vec.rvec.z, &hack, 4);*/

// Packs/unpacks an orientation matrix.
// Returns number of bytes read or written.
// #define OO_ORIENT_RET_SIZE						6
int multi_pack_unpack_orient( int write, ubyte *data, matrix *orient)
{
	bitbuffer buf;

	bitbuffer_init(&buf, data + 1);

	vec3d rot_axis;	
	float theta;
	int a, b, c, d;
	angles ang;	
	ubyte flag = 0x00;	

	#define D_SCALE 32768.0f
	#define D_MAX_RANGE 32767
	#define D_MIN_RANGE -32768

	#define N_SCALE 2048.0f
	#define N_MAX_RANGE 2047
	#define N_MIN_RANGE -2048

	if ( write )	{			
		// degenerate case - send the whole orient matrix
		vm_extract_angles_matrix(&ang, orient);	
		if((ang.h > 3.130) && (ang.h < 3.150)){
			degenerate_count++;

			flag = 0xff;
			
			// stuff it	
			a = fl2i(orient->vec.fvec.xyz.x * D_SCALE);
			CAP(a, D_MIN_RANGE, D_MAX_RANGE);			
			bitbuffer_put( &buf, a, 16  );
			a = fl2i(orient->vec.fvec.xyz.y * D_SCALE);
			CAP(a, D_MIN_RANGE, D_MAX_RANGE);			
			bitbuffer_put( &buf, a, 16  );
			a = fl2i(orient->vec.fvec.xyz.z * D_SCALE);
			CAP(a, D_MIN_RANGE, D_MAX_RANGE);			
			bitbuffer_put( &buf, a, 16  );

			a = fl2i(orient->vec.uvec.xyz.x * D_SCALE);
			CAP(a, D_MIN_RANGE, D_MAX_RANGE);			
			bitbuffer_put( &buf, a, 16  );
			a = fl2i(orient->vec.uvec.xyz.y * D_SCALE);
			CAP(a, D_MIN_RANGE, D_MAX_RANGE);			
			bitbuffer_put( &buf, a, 16  );
			a = fl2i(orient->vec.uvec.xyz.z * D_SCALE);
			CAP(a, D_MIN_RANGE, D_MAX_RANGE);			
			bitbuffer_put( &buf, a, 16  );

			a = fl2i(orient->vec.rvec.xyz.x * D_SCALE);
			CAP(a, D_MIN_RANGE, D_MAX_RANGE);			
			bitbuffer_put( &buf, a, 16  );
			a = fl2i(orient->vec.rvec.xyz.y * D_SCALE);
			CAP(a, D_MIN_RANGE, D_MAX_RANGE);			
			bitbuffer_put( &buf, a, 16  );
			a = fl2i(orient->vec.rvec.xyz.z * D_SCALE);
			CAP(a, D_MIN_RANGE, D_MAX_RANGE);			
			bitbuffer_put( &buf, a, 16  );
		} else {
			non_degenerate_count++;
				
			vm_matrix_to_rot_axis_and_angle(orient, &theta, &rot_axis);		
			// Have theta, which is an angle between 0 and PI.
			// Convert it to be between -1.0f and 1.0f
			theta = theta*2.0f/PI-1.0f;			

			// -1 to 1
			a = fl2i(rot_axis.xyz.x*N_SCALE); 
			b = fl2i(rot_axis.xyz.y*N_SCALE);
			c = fl2i(rot_axis.xyz.z*N_SCALE);
			d = fl2i(theta*N_SCALE);

			CAP(a, N_MIN_RANGE, N_MAX_RANGE);
			CAP(b, N_MIN_RANGE, N_MAX_RANGE);
			CAP(c, N_MIN_RANGE, N_MAX_RANGE);
			CAP(d, N_MIN_RANGE, N_MAX_RANGE);
					
			bitbuffer_put( &buf, (uint)a, 12 );
			bitbuffer_put( &buf, (uint)b, 12 );
			bitbuffer_put( &buf, (uint)c, 12 );
			bitbuffer_put( &buf, (uint)d, 12 );
		}

		// flag for degenerate case
		data[0] = flag;

		return bitbuffer_write_flush(&buf) + 1;
	} else {
		flag = data[0];

		// degenerate
		if(flag){
			a = bitbuffer_get_signed(&buf, 16);
			orient->vec.fvec.xyz.x = i2fl(a) / D_SCALE;			
			a = bitbuffer_get_signed(&buf, 16);
			orient->vec.fvec.xyz.y = i2fl(a) / D_SCALE;			
			a = bitbuffer_get_signed(&buf, 16);
			orient->vec.fvec.xyz.z = i2fl(a) / D_SCALE;			

			a = bitbuffer_get_signed(&buf, 16);
			orient->vec.uvec.xyz.x = i2fl(a) / D_SCALE;			
			a = bitbuffer_get_signed(&buf, 16);
			orient->vec.uvec.xyz.y = i2fl(a) / D_SCALE;			
			a = bitbuffer_get_signed(&buf, 16);
			orient->vec.uvec.xyz.z = i2fl(a) / D_SCALE;			

			a = bitbuffer_get_signed(&buf, 16);
			orient->vec.rvec.xyz.x = i2fl(a) / D_SCALE;			
			a = bitbuffer_get_signed(&buf, 16);
			orient->vec.rvec.xyz.y = i2fl(a) / D_SCALE;			
			a = bitbuffer_get_signed(&buf, 16);
			orient->vec.rvec.xyz.z = i2fl(a) / D_SCALE;			
		} else {
			a = bitbuffer_get_signed(&buf,12);
			b = bitbuffer_get_signed(&buf,12);
			c = bitbuffer_get_signed(&buf,12);
			d = bitbuffer_get_signed(&buf,12);

			// special case		
			rot_axis.xyz.x = i2fl(a)/N_SCALE;
			rot_axis.xyz.y = i2fl(b)/N_SCALE;
			rot_axis.xyz.z = i2fl(c)/N_SCALE;
			theta = i2fl(d)/N_SCALE;
				
			// Convert theta back to range 0-PI
			theta = (theta+1.0f)*PI_2;
				
			vm_quaternion_rotate(orient, theta, &rot_axis);		

			vm_orthogonalize_matrix(orient);		
		}

		return bitbuffer_read_flush(&buf) + 1;
	}
}


// Packs/unpacks an orientation matrix.
// Returns number of bytes read or written.
// #define OO_ORIENT_RET_SIZE						6
/*
int multi_pack_unpack_orient( int write, ubyte *data, matrix *orient)
{
	bitbuffer buf;

	bitbuffer_init(&buf,data);

	vec3d rot_axis;	
	float theta;
	int a, b, c, d;

	if ( write )	{

		// if our heading is 3.14 radians
		//angles ang;
		//vm_extract_angles_matrix(&a, orient);
		//if((ang.h > 3.1300) && (ang.h < 3.1500)){			
		//} else {

			util_matrix_to_rot_axis_and_angle(orient, &theta, &rot_axis);		
			// Have theta, which is an angle between 0 and PI.
			// Convert it to be between -1.0f and 1.0f
			theta = theta*2.0f/PI-1.0f;

			#define SCALE 2048.0f

			#define MAX_RANGE 2047
			#define MIN_RANGE -2048

			// -1 to 1
			a = fl2i(rot_axis.x*SCALE); 
			b = fl2i(rot_axis.y*SCALE);
			c = fl2i(rot_axis.z*SCALE);
			d = fl2i(theta*SCALE);

			CAP(a,MIN_RANGE,MAX_RANGE);
			CAP(b,MIN_RANGE,MAX_RANGE);
			CAP(c,MIN_RANGE,MAX_RANGE);
			CAP(d,MIN_RANGE,MAX_RANGE);
		//}
		
		bitbuffer_put( &buf, (uint)a, 12 );
		bitbuffer_put( &buf, (uint)b, 12 );
		bitbuffer_put( &buf, (uint)c, 12 );
		bitbuffer_put( &buf, (uint)d, 12 );

		return bitbuffer_write_flush(&buf);

	} else {

		a = bitbuffer_get_signed(&buf,12);
		b = bitbuffer_get_signed(&buf,12);
		c = bitbuffer_get_signed(&buf,12);
		d = bitbuffer_get_signed(&buf,12);

		// special case		
		rot_axis.x = i2fl(a)/SCALE;
		rot_axis.y = i2fl(b)/SCALE;
		rot_axis.z = i2fl(c)/SCALE;
		theta = i2fl(d)/SCALE;
			
		// Convert theta back to range 0-PI
		theta = (theta+1.0f)*PI_2;
			
		vm_quaternion_rotate(orient, theta, &rot_axis);		

		vm_orthogonalize_matrix(orient);		

		return bitbuffer_read_flush(&buf);
	}
}
*/

// Packs/unpacks velocity
// Returns number of bytes read or written.
// #define OO_VEL_RET_SIZE							4
int multi_pack_unpack_vel( int write, ubyte *data, matrix *orient, vec3d *pos, physics_info *pi)
{
	bitbuffer buf;

	bitbuffer_init(&buf,data);

	int a, b, c;
	float r, u, f;

	if ( write )	{
		// output velocity
		r = vm_vec_dot( &orient->vec.rvec, &pi->vel );
		u = vm_vec_dot( &orient->vec.uvec, &pi->vel );
		f = vm_vec_dot( &orient->vec.fvec, &pi->vel );

		a = fl2i(r * 0.5f); 
		b = fl2i(u * 0.5f);
		c = fl2i(f * 0.5f);
		CAP(a,-512,511);
		CAP(b,-512,511);
		CAP(c,-512,511);
		bitbuffer_put( &buf, (uint)a, 10 );
		bitbuffer_put( &buf, (uint)b, 10 );
		bitbuffer_put( &buf, (uint)c, 10 );

		return bitbuffer_write_flush(&buf);
	} else {
		// unpack velocity
		a = bitbuffer_get_signed(&buf,10);
		b = bitbuffer_get_signed(&buf,10);
		c = bitbuffer_get_signed(&buf,10);
		r = i2fl(a)/0.5f;
		u = i2fl(b)/0.5f;
		f = i2fl(c)/0.5f;

		// Convert into world coordinates
		vm_vec_zero(&pi->vel);
		vm_vec_scale_add2( &pi->vel, &orient->vec.rvec, r );
		vm_vec_scale_add2( &pi->vel, &orient->vec.uvec, u );
		vm_vec_scale_add2( &pi->vel, &orient->vec.fvec, f );

		return bitbuffer_read_flush(&buf);
	}
}

// Packs/unpacks desired_velocity
// Returns number of bytes read or written.
// #define OO_DESIRED_VEL_RET_SIZE				3
int multi_pack_unpack_desired_vel( int write, ubyte *data, matrix *orient, vec3d *pos, physics_info *pi, ship_info *sip)
{
	bitbuffer buf;

	bitbuffer_init(&buf,data);

	int a;
	vec3d	max_vel;
	float r,u,f;
	int fields = 0;

	max_vel.xyz.x = MAX( sip->max_vel.xyz.x, sip->afterburner_max_vel.xyz.x );
	max_vel.xyz.y = MAX( sip->max_vel.xyz.y, sip->afterburner_max_vel.xyz.y );
	max_vel.xyz.z = MAX( sip->max_vel.xyz.z, sip->afterburner_max_vel.xyz.z );	

	if ( write )	{
		// Find desired vel in local coordinates
		// Velocity can be from -1024 to 1024

		// bitfields for each value		
		if(max_vel.xyz.x > 0.0f){
			fields |= (1<<0);
		}
		if(max_vel.xyz.y > 0.0f){
			fields |= (1<<1);
		}
		if(max_vel.xyz.z > 0.0f){
			fields |= (1<<2);
		}		
		// fields = sip - Ship_info;
		bitbuffer_put(&buf, (uint)fields, 8);

		r = vm_vec_dot( &orient->vec.rvec, &pi->desired_vel );
		u = vm_vec_dot( &orient->vec.uvec, &pi->desired_vel );
		f = vm_vec_dot( &orient->vec.fvec, &pi->desired_vel );

		if ( max_vel.xyz.x > 0.0f )	{
			r = r / max_vel.xyz.x;
			a = fl2i( r * 128.0f );
			CAP(a,-128, 127 );			
			bitbuffer_put( &buf, (uint)a, 8 );			
		} 

		if ( max_vel.xyz.y > 0.0f )	{
			u = u / max_vel.xyz.y;
			a = fl2i( u * 128.0f );
			CAP(a,-128, 127 );
			bitbuffer_put( &buf, (uint)a, 8 );
		} 

		if ( max_vel.xyz.z > 0.0f )	{
			f = f / max_vel.xyz.z;
			a = fl2i( f * 128.0f );
			CAP(a,-128, 127 );
			bitbuffer_put( &buf, (uint)a, 8 );
		}

		return bitbuffer_write_flush(&buf);
	} else {

		// Find desired vel in local coordinates
		// Velocity can be from -1024 to 1024

		// get the fields bitbuffer
		fields = bitbuffer_get_signed(&buf, 8);
		
		if ( fields & (1<<0) )	{
			a = bitbuffer_get_signed(&buf,8);
			r = i2fl(a)/128.0f;
		} else {
			r = 0.0f;
		}
		
		if ( fields & (1<<1) )	{
			a = bitbuffer_get_signed(&buf,8);
			u = i2fl(a)/128.0f;
		} else {
			u = 0.0f;
		}
		
		if ( fields & (1<<2) )	{
			a = bitbuffer_get_signed(&buf,8);
			f = i2fl(a)/128.0f;
		} else {
			f = 0.0f;
		}		
		
		// Convert into world coordinates
		vm_vec_zero(&pi->vel);
		vm_vec_scale_add2( &pi->desired_vel, &orient->vec.rvec, r*max_vel.xyz.x );
		vm_vec_scale_add2( &pi->desired_vel, &orient->vec.uvec, u*max_vel.xyz.y );
		vm_vec_scale_add2( &pi->desired_vel, &orient->vec.fvec, f*max_vel.xyz.z );

		return bitbuffer_read_flush(&buf);
	}
}

// Packs/unpacks rotational velocity
// Returns number of bytes read or written.
// #define OO_ROTVEL_RET_SIZE						4
int multi_pack_unpack_rotvel( int write, ubyte *data, matrix *orient, vec3d *pos, physics_info *pi)
{
	bitbuffer buf;

	bitbuffer_init(&buf,data);

	int a, b, c;

	if ( write )	{
		// output rotational velocity
		a = fl2i(pi->rotvel.xyz.x*32.0f); 
		b = fl2i(pi->rotvel.xyz.y*32.0f);
		c = fl2i(pi->rotvel.xyz.z*32.0f);
		CAP(a,-512,511);
		CAP(b,-512,511);
		CAP(c,-512,511);
		bitbuffer_put( &buf, (uint)a, 10 );
		bitbuffer_put( &buf, (uint)b, 10 );
		bitbuffer_put( &buf, (uint)c, 10 );


		return bitbuffer_write_flush(&buf);

	} else {

		// unpack rotational velocity
		a = bitbuffer_get_signed(&buf,10);
		b = bitbuffer_get_signed(&buf,10);
		c = bitbuffer_get_signed(&buf,10);
		pi->rotvel.xyz.x = i2fl(a)/32.0f;
		pi->rotvel.xyz.y = i2fl(b)/32.0f;
		pi->rotvel.xyz.z = i2fl(c)/32.0f;

		return bitbuffer_read_flush(&buf);
	}
}

// Packs/unpacks desired rotvel
// Returns number of bytes read or written.
// #define OO_DESIRED_ROTVEL_RET_SIZE			3
int multi_pack_unpack_desired_rotvel( int write, ubyte *data, matrix *orient, vec3d *pos, physics_info *pi, ship_info *sip)
{
	bitbuffer buf;
	int fields = 0;

	bitbuffer_init(&buf,data);

	int a;
	float r,u,f;

	if ( write )	{
		// use ship_info values for max_rotvel instead of taking it from physics info

		// bitfields for each value
		if(sip->max_rotvel.xyz.x > 0.0f){
			fields |= (1<<0);
		}
		if(sip->max_rotvel.xyz.y > 0.0f){
			fields |= (1<<1);
		}
		if(sip->max_rotvel.xyz.z > 0.0f){
			fields |= (1<<2);

		}
		bitbuffer_put(&buf, (uint)fields, 8);

		// output desired rotational velocity as a percent of max		
		if ( sip->max_rotvel.xyz.x > 0.0f )	{		
			a = fl2i( pi->desired_rotvel.xyz.x*128.0f / sip->max_rotvel.xyz.x );
			CAP(a,-128, 127 );
			bitbuffer_put( &buf, (uint)a, 8 );
		} 

		if ( sip->max_rotvel.xyz.y > 0.0f )	{		
			a = fl2i( pi->desired_rotvel.xyz.y*128.0f / sip->max_rotvel.xyz.y );
			CAP(a,-128, 127 );
			bitbuffer_put( &buf, (uint)a, 8 );
		} 

		if ( sip->max_rotvel.xyz.z > 0.0f )	{		
			a = fl2i( pi->desired_rotvel.xyz.z*128.0f / sip->max_rotvel.xyz.z );
			CAP(a,-128, 127 );
			bitbuffer_put( &buf, (uint)a, 8 );
		} 

		return bitbuffer_write_flush(&buf);
	} else {
		fields = bitbuffer_get_signed(&buf, 8);

		// unpack desired rotational velocity
		if ( fields & (1<<0) )	{		
			a = bitbuffer_get_signed(&buf,8);
			r = i2fl(a)/128.0f;
		} else {
			r = 0.0f;
		}
		if ( fields & (1<<1) )	{		
			a = bitbuffer_get_signed(&buf,8);
			u = i2fl(a)/128.0f;
		} else {
			u = 0.0f;
		}
		if ( fields & (1<<2) )	{		
			a = bitbuffer_get_signed(&buf,8);
			f = i2fl(a)/128.0f;
		} else {
			f = 0.0f;
		}
		pi->desired_rotvel.xyz.x = r*sip->max_rotvel.xyz.x;
		pi->desired_rotvel.xyz.y = u*sip->max_rotvel.xyz.y;
		pi->desired_rotvel.xyz.z = f*sip->max_rotvel.xyz.z;

		return bitbuffer_read_flush(&buf);
	}
}

// Karajorma - sends the player to the correct debrief for this game type
// Currently supports the dogfight kill matrix and normal debriefing stages but if new types are created they should be added here
void send_debrief_event() {	
	// we have a special debriefing screen multiplayer furballs use by default
	if((Game_mode & GM_MULTIPLAYER) && IS_MISSION_MULTI_DOGFIGHT && !(The_mission.flags & MISSION_FLAG_TOGGLE_DEBRIEFING)) {
		gameseq_post_event( GS_EVENT_MULTI_DOGFIGHT_DEBRIEF);
	}
	// do the normal debriefing for all other situations
	else {
		gameseq_post_event(GS_EVENT_DEBRIEF);
	}
}

#pragma optimize("", on)
