/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include <limits.h>

#include "globalincs/pstypes.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multiui.h"
#include "network/multi.h"
#include "globalincs/linklist.h"
#include "gamesequence/gamesequence.h"
#include "hud/hudmessage.h"
#include "hud/hudsquadmsg.h"
#include "freespace2/freespace.h"
#include "io/timer.h"
#include "mission/missiongoals.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "network/stand_gui.h"
#include "gamesnd/gamesnd.h"
#include "ship/shiphit.h"
#include "render/3d.h"
#include "playerman/player.h"
#include "debris/debris.h"
#include "missionui/missionweaponchoice.h"
#include "missionui/missionshipchoice.h"
#include "ship/shipfx.h"
#include "popup/popup.h"
#include "network/multi_ingame.h"
#include "network/multiteamselect.h"
#include "ai/aigoals.h"
#include "network/multi_campaign.h"
#include "network/multi_team.h"
#include "network/multi_respawn.h"
#include "network/multi_observer.h"
#include "asteroid/asteroid.h"
#include "network/multi_pmsg.h"
#include "object/object.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "hud/hudreticle.h"
#include "network/multi_pause.h"
#include "network/multi_endgame.h"
#include "missionui/missiondebrief.h"
#include "network/multi_log.h"
#include "weapon/emp.h"
#include "network/multi_kick.h"
#include "cmdline/cmdline.h"
#include "weapon/flak.h"
#include "weapon/beam.h"
#include "network/multi_rate.h"
#include "nebula/neblightning.h"
#include "hud/hud.h"
#include "missionui/missionscreencommon.h"
#include "mission/missionbriefcommon.h"
#include "network/multi_log.h"
#include "object/objectdock.h"
#include "cmeasure/cmeasure.h"
#include "parse/sexp.h"
#include "fs2netd/fs2netd_client.h"
#include "network/multi_sexp.h"

// #define _MULTI_SUPER_WACKY_COMPRESSION

#ifdef _MULTI_SUPER_WACKY_COMPRESSION
#define BITS                       15
#define MAX_CODE                   ( ( 1 << BITS ) - 1 )
#define TABLE_SIZE                 35023L
#define END_OF_STREAM              256
#define BUMP_CODE                  257
#define FLUSH_CODE                 258
#define FIRST_CODE                 259
#define UNUSED                     -1

typedef struct {
    int code_value;
    int parent_code;
    char character;
} DICTIONARY;

static DICTIONARY dict[TABLE_SIZE];
static char decode_stack[TABLE_SIZE];
static uint next_code;
static int current_code_bits;
static uint next_bump_code;

typedef struct BitBuf {
	ubyte	mask;
   int 	rack;
	ubyte *data;
} BitBuf;

void output_bits( BitBuf *bitbuf, uint code, int count ) 
{
    uint mask;

    mask = 1L << ( count - 1 );
    while ( mask != 0) {
        if ( mask & code )
            bitbuf->rack |= bitbuf->mask;
        bitbuf->mask >>= 1;
        if ( bitbuf->mask == 0 ) {
				*bitbuf->data++=(ubyte)bitbuf->rack;
				bitbuf->rack = 0;
            bitbuf->mask = 0x80;
        }
        mask >>= 1;
    }
}

uint input_bits( BitBuf *bitbuf, int bit_count ) 
{
	uint mask;
	uint return_value;

	mask = 1L << ( bit_count - 1 );
	return_value = 0;
	while ( mask != 0)	{
		if ( bitbuf->mask == 0x80 ) {
			bitbuf->rack = *bitbuf->data++;
			if ( bitbuf->rack == EOF ) 
				return END_OF_STREAM;
    	}
		if ( bitbuf->rack & bitbuf->mask )
			return_value |= mask;
		mask >>= 1;
		bitbuf->mask >>= 1;
		if ( bitbuf->mask == 0 )
			bitbuf->mask = 0x80;
	}
	return( return_value );
}


static void InitializeDictionary()
{
	uint i;

	for ( i = 0 ; i < TABLE_SIZE ; i++ )
		dict[i].code_value = UNUSED;

	next_code = FIRST_CODE;
	current_code_bits = 9;
	next_bump_code = 511;

}

static uint find_child_node( int parent_code, int child_character ) 
{
    uint index;
    int offset;

    index = ( child_character << ( BITS - 8 ) ) ^ parent_code;
    if ( index == 0 )
        offset = 1;
    else
        offset = TABLE_SIZE - index;
    for ( ; ; ) {
		if ( dict[ index ].code_value == UNUSED )
            return( (uint) index );
		if ( dict[ index ].parent_code == parent_code &&
			 dict[ index ].character == (char) child_character )
            return( index );
        if ( (int) index >= offset )
            index -= offset;
        else
            index += TABLE_SIZE - offset;
    }
}


static uint decode_string( uint count, uint code ) 
{
    while ( code > 255 ) {
		decode_stack[ count++ ] = dict[ code ].character;
		code = dict[ code ].parent_code;
    }
    decode_stack[ count++ ] = (char) code;
    return( count );
}

int lzw_compress( ubyte *outputbuf, ubyte *inputbuf, int input_size ) 
{
	BitBuf output;
	int character;
	int string_code;
	uint index;
	int i;

	// Init output bit buffer
	output.rack = 0;
	output.mask = 0x80;
	output.data = outputbuf;

	InitializeDictionary();

	string_code = *inputbuf++;

	for ( i=1 ; i<input_size ; i++ ) {
		character = *inputbuf++;
		index = find_child_node( string_code, character );
		if ( dict[ index ].code_value != - 1 )
			string_code = dict[ index ].code_value;
      else {
			dict[ index ].code_value = next_code++;
			dict[ index ].parent_code = string_code;
			dict[ index ].character = (char) character;
			output_bits( &output, (unsigned long) string_code, current_code_bits );
			string_code = character;
         if ( next_code > MAX_CODE ) {
				output_bits( &output, (unsigned long) FLUSH_CODE, current_code_bits );
				InitializeDictionary();
			} else if ( next_code > next_bump_code ) {
         	output_bits( &output, (unsigned long) BUMP_CODE, current_code_bits );
				current_code_bits++;
				next_bump_code <<= 1;
				next_bump_code |= 1;
			}
		}
	}
	output_bits( &output, (unsigned long) string_code, current_code_bits );
	output_bits( &output, (unsigned long) END_OF_STREAM, current_code_bits);

	if ( output.mask != 0x80 )
   	*output.data++ = (ubyte)output.rack;

	return output.data-outputbuf;
}


int lzw_expand( ubyte *outputbuf, ubyte *inputbuf ) 
{
	BitBuf input;
	uint new_code;
	uint old_code;
	int character;
	uint count;
	uint counter;

	input.rack = 0;	
	input.mask = 0x80;
	input.data = inputbuf;
	
	counter = 0;
	for ( ; ; ) {
		InitializeDictionary();
		old_code = (uint) input_bits( &input, current_code_bits );
		if ( old_code == END_OF_STREAM ) 
			return counter;
		character = old_code;
		outputbuf[counter++] = ( ubyte )old_code;
		for ( ; ; ) {
			new_code = (uint) input_bits( &input, current_code_bits );
			if ( new_code == END_OF_STREAM ) 
				return counter;
			if ( new_code == FLUSH_CODE )
				break;
			if ( new_code == BUMP_CODE ) {
				current_code_bits++;
				continue;
			}
			if ( new_code >= next_code ) {
				decode_stack[ 0 ] = (char) character;
				count = decode_string( 1, old_code );
			} else {
				count = decode_string( 0, new_code );
			}
			character = decode_stack[ count - 1 ];
			while ( count > 0 )
				outputbuf[counter++] = ( ubyte )decode_stack[ --count ];
			dict[ next_code ].parent_code = old_code;
			dict[ next_code ].character = (char) character;
			next_code++;
			old_code = new_code;
		}
	}
}
#endif

// process a join request packet add
void add_join_request(ubyte *data, int *size, join_request *jr)
{
	int packet_size = *size;
	join_request jr_tmp;

	memcpy(&jr_tmp, jr, sizeof(join_request));

	jr_tmp.tracker_id = INTEL_INT(jr->tracker_id);
	jr_tmp.player_options.flags = INTEL_INT(jr->player_options.flags);
	jr_tmp.player_options.obj_update_level = INTEL_INT(jr->player_options.obj_update_level);

	ADD_DATA(jr_tmp);

	*size = packet_size;
}

// process a join request packet get
void get_join_request(ubyte *data, int *size, join_request *jr)
{
	int offset = *size;

	GET_DATA(*jr);

	jr->tracker_id = INTEL_INT(jr->tracker_id);
	jr->player_options.flags = INTEL_INT(jr->player_options.flags);
	jr->player_options.obj_update_level = INTEL_INT(jr->player_options.obj_update_level);

	*size = offset;
}

void add_net_addr(ubyte *data, int *size, net_addr *addr)
{
	int packet_size = *size;
	net_addr addr_tmp;

	memcpy(&addr_tmp, addr, sizeof(net_addr));

	addr_tmp.type = INTEL_INT(addr->type);
	addr_tmp.port = INTEL_SHORT(addr->port);

	ADD_DATA(addr_tmp);

	*size = packet_size;
}

void get_net_addr(ubyte *data, int *size, net_addr *addr)
{
	int offset = *size;

	GET_DATA(*addr);

	addr->type = INTEL_INT(addr->type);
	addr->port = INTEL_SHORT(addr->port);

	*size = offset;
}
/*
void add_vector_data(ubyte *data, int *size, vec3d vec)
{
	int packet_size = *size;

	ADD_FLOAT(vec.xyz.x);
	ADD_FLOAT(vec.xyz.y);
	ADD_FLOAT(vec.xyz.z);

	*size = packet_size;
}

void get_vector_data(ubyte *data, int *size, vec3d vec)
{
	int offset = *size;

	GET_FLOAT(vec.xyz.x);
	GET_FLOAT(vec.xyz.y);
	GET_FLOAT(vec.xyz.z);

	*size = offset;
}
*/
// send the specified data packet to all players
void multi_io_send(net_player *pl, ubyte *data, int len)
{		
	// invalid
	if((pl == NULL) || (NET_PLAYER_NUM(pl) >= MAX_PLAYERS)){
		return;
	}

	// don't do it for single player
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// sanity checks
	if(MULTIPLAYER_CLIENT){
		// Assert(pl == Net_player);
		if(pl != Net_player){
			return;
		}
	} else {
		// Assert(pl != Net_player);
		if(pl == Net_player){
			return;
		}
	}

	// If this packet will push the buffer over MAX_PACKET_SIZE, send the current send_buffer
	if ((pl->s_info.unreliable_buffer_size + len) > MAX_PACKET_SIZE) {		
		multi_io_send_force(pl);
		pl->s_info.unreliable_buffer_size = 0;
	}

	Assert((pl->s_info.unreliable_buffer_size + len) <= MAX_PACKET_SIZE);

	memcpy(pl->s_info.unreliable_buffer + pl->s_info.unreliable_buffer_size, data, len);
	pl->s_info.unreliable_buffer_size += len;
}

void multi_io_send_to_all(ubyte *data, int length, net_player *ignore)
{	
	int i;
	Assert(MULTIPLAYER_MASTER);

  	// need to check for i > 1, hmmm... and connected. I don't know.	
	for (i = 0; i < MAX_PLAYERS; i++ ) {
		if ( !MULTI_CONNECTED(Net_players[i]) || (Net_player == &Net_players[i])){
			continue;
		}

		// maybe ignore a player
		if((ignore != NULL) && (&Net_players[i] == ignore)){
			continue;
		}

		// ingame joiners not waiting to select a ship doesn't get any packets
		if ( (Net_players[i].flags & NETINFO_FLAG_INGAME_JOIN) && !(Net_players[i].flags & INGAME_JOIN_FLAG_PICK_SHIP) ){
			continue;
		}

		// send it
		multi_io_send(&Net_players[i], data, length);
   }
}

void multi_io_send_force(net_player *pl)
{	
	// invalid
	if((pl == NULL) || (NET_PLAYER_NUM(pl) >= MAX_PLAYERS)){
		return;
	}
	
	// don't do it for single player
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// send everything in 
	if (MULTIPLAYER_MASTER) {
		psnet_send(&pl->p_info.addr, pl->s_info.unreliable_buffer, pl->s_info.unreliable_buffer_size, NET_PLAYER_NUM(pl));

		// add the bytes sent to this player
		pl->sv_bytes_sent += pl->s_info.unreliable_buffer_size;
	} else {
		psnet_send(&Netgame.server_addr, pl->s_info.unreliable_buffer, pl->s_info.unreliable_buffer_size, NET_PLAYER_NUM(pl));		
	}		
	pl->s_info.unreliable_buffer_size = 0;
}

// send the data packet to all players via their reliable sockets
void multi_io_send_reliable(net_player *pl, ubyte *data, int len)
{	
	// invalid
	if((pl == NULL) || (NET_PLAYER_NUM(pl) >= MAX_PLAYERS)){
		return;
	}

	// don't do it for single player
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}
	
	// sanity checks
	if(MULTIPLAYER_CLIENT){
		// Assert(pl == Net_player);
		if(pl != Net_player){
			return;
		}
	} else {
		// Assert(pl != Net_player);
		if(pl == Net_player){
			return;
		}
	}

	// If this packet will push the buffer over MAX_PACKET_SIZE, send the current send_buffer
	if ((pl->s_info.reliable_buffer_size + len) > MAX_PACKET_SIZE) {		
		multi_io_send_reliable_force(pl);
		pl->s_info.reliable_buffer_size = 0;
	}

	Assert((pl->s_info.reliable_buffer_size + len) <= MAX_PACKET_SIZE);

	memcpy(pl->s_info.reliable_buffer + pl->s_info.reliable_buffer_size, data, len);
	pl->s_info.reliable_buffer_size += len;
}

void multi_io_send_to_all_reliable(ubyte* data, int length, net_player *ignore)
{	
	int i;
	Assert(MULTIPLAYER_MASTER);

  	// need to check for i > 1, hmmm... and connected. I don't know.	
	for (i = 0; i < MAX_PLAYERS; i++ ) {
		if ( !MULTI_CONNECTED(Net_players[i]) || (Net_player == &Net_players[i])){
			continue;
		}

		// maybe ignore a player
		if((ignore != NULL) && (&Net_players[i] == ignore)){
			continue;
		}

		// ingame joiners not waiting to select a ship doesn't get any packets
		if ( (Net_players[i].flags & NETINFO_FLAG_INGAME_JOIN) && !(Net_players[i].flags & INGAME_JOIN_FLAG_PICK_SHIP) ){
			continue;
		}

		// send it
		multi_io_send_reliable(&Net_players[i], data, length);
   }
}

void multi_io_send_reliable_force(net_player *pl)
{	
	// invalid
	if((pl == NULL) || (NET_PLAYER_NUM(pl) >= MAX_PLAYERS)){
		return;
	}		

	// don't do it for single player
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// send everything in 
	if(MULTIPLAYER_MASTER) {
		psnet_rel_send(pl->reliable_socket, pl->s_info.reliable_buffer, pl->s_info.reliable_buffer_size, NET_PLAYER_NUM(pl));
	} else if(Net_player != NULL){
		psnet_rel_send(Net_player->reliable_socket, pl->s_info.reliable_buffer, pl->s_info.reliable_buffer_size, NET_PLAYER_NUM(pl));
	}		
	pl->s_info.reliable_buffer_size = 0;
}

// send all buffered packets
void multi_io_send_buffered_packets()	
{
	int idx;

	// don't do it for single player
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// server
	if(MULTIPLAYER_MASTER){
		for(idx=0; idx<MAX_PLAYERS; idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx])){
				// force unreliable data
				if(Net_players[idx].s_info.unreliable_buffer_size > 0){
					multi_io_send_force(&Net_players[idx]);
					Net_players[idx].s_info.unreliable_buffer_size = 0;
				}

				// force reliable data
				if(Net_players[idx].s_info.reliable_buffer_size > 0){
					multi_io_send_reliable_force(&Net_players[idx]);
					Net_players[idx].s_info.reliable_buffer_size = 0;
				}
			}
		}
	} 
	// clients
	else if(Net_player != NULL){
		// force unreliable data
		if(Net_player->s_info.unreliable_buffer_size > 0){
			multi_io_send_force(Net_player);
			Net_player->s_info.unreliable_buffer_size = 0;
		}

		// force reliable data
		if(Net_player->s_info.reliable_buffer_size > 0){
			multi_io_send_reliable_force(Net_player);
			Net_player->s_info.reliable_buffer_size = 0;
		}
	}
}

//*********************************************************************************************************
// Game Chat Packet
//*********************************************************************************************************
/*
struct fs2_game_chat_packet
{
	char packet_signature; //0xC3
	short from_player_id;
	int server_msg;
	char mode;
	
	// variable record
	if (mode) 
		short to_player_id;
	else
	{
		int i;
		char expr[i];
	}
	int j;
	char message[j];
	    
}
*/

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// send a general game chat packet (if msg_mode == MULTI_MSG_TARGET, need to pass in "to", if == MULTI_MSG_EXPR, need to pass in expr)
void send_game_chat_packet(net_player *from, char *msg, int msg_mode, net_player *to, char *expr, int server_msg)
{
	ubyte data[MAX_PACKET_SIZE],mode;
	int packet_size,idx;
	bool undeliverable = true;
	
	BUILD_HEADER(GAME_CHAT);
	
	// add the id
	ADD_SHORT(from->player_id);

	// add the message mode and if in MSG_TARGET mode, add who the target is
	ADD_INT(server_msg);
	mode = (ubyte)msg_mode;	
	ADD_DATA(mode);
	switch(mode){
	case MULTI_MSG_TARGET:	
		Assert(to != NULL);
		ADD_SHORT(to->player_id);
		break;
	case MULTI_MSG_EXPR:
		Assert(expr != NULL);
		ADD_STRING(expr);
		break;
	}
	// add the message itself
	ADD_STRING( msg );
	
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		switch(mode){
		// message all players
		case MULTI_MSG_ALL:			
			for(idx=0;idx<MAX_PLAYERS;idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (&Net_players[idx] != from)){					
					multi_io_send_reliable(&Net_players[idx], data, packet_size);
				}
			}
			break;

		// message only friendly players
		case MULTI_MSG_FRIENDLY:
			for(idx=0;idx<MAX_PLAYERS;idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (&Net_players[idx] != from) && (Net_players[idx].p_info.team == from->p_info.team)){					
					multi_io_send_reliable(&Net_players[idx], data, packet_size);
				}
			}
			break;

		// message only hostile players
		case MULTI_MSG_HOSTILE:
			for(idx=0;idx<MAX_PLAYERS;idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (&Net_players[idx] != from) && (Net_players[idx].p_info.team != from->p_info.team)){					
					multi_io_send_reliable(&Net_players[idx], data, packet_size);
				}
			}
			break;
		
		// message the player's target
		case MULTI_MSG_TARGET:
			Assert(to != NULL);
			if(MULTI_CONNECTED((*to)) && !MULTI_STANDALONE((*to))){				
				multi_io_send_reliable(to, data, packet_size);
			}
			break;

		// message all players who match the expression string
		case MULTI_MSG_EXPR:
			Assert(expr != NULL);
			for(idx=0;idx<MAX_PLAYERS;idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (&Net_players[idx] != from) && multi_msg_matches_expr(&Net_players[idx],expr) ){					
					multi_io_send_reliable(&Net_players[idx], data, packet_size);
					undeliverable = false; 
				}
			}
			break;
		}	

		// if the message can't be delivered, notify the player
		if (undeliverable) {
			switch(mode){
				case MULTI_MSG_EXPR:
					// if the message came from the server
					if (from == Net_player) {
						multi_display_chat_msg ("Unable to send message, player does not exist", 0, 0);
					}
					// otherwise send a message back to the player
					else {
						send_game_chat_packet(Net_player, "Unable to send message, player does not exist", MULTI_MSG_TARGET, from, NULL, 1); 
					}
					break;
			}	
		}
	}
	// send to the server, who will take care of routing it
	else {		
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// process a general game chat packet, if we're the standalone we should rebroadcast
void process_game_chat_packet( ubyte *data, header *hinfo )
{
	int offset;
	ubyte mode;
	int color_index,player_index,to_player_index,should_display,server_msg;	
	char msg[MULTI_MSG_MAX_TEXT_LEN+CALLSIGN_LEN+2];
	char expr[255];
	short from, to;

	offset = HEADER_LENGTH;

	// get the id of the sender
	GET_SHORT(from);
	
	// determine if this is a server message
	GET_INT(server_msg);

	// get the mode
	GET_DATA(mode);
	
	// if targeting a specific player, get the address
	to = -1;
	switch(mode){
	case MULTI_MSG_TARGET:	
		GET_SHORT(to);
		break;
	case MULTI_MSG_EXPR:
		GET_STRING(expr);
		break;
	}
	// get the message itself
	GET_STRING(msg);
	PACKET_SET_SIZE();	

   // get the index of the sending player
	color_index = find_player_id(from);
	player_index = color_index;
	
	// if we couldn't find the player - bail
	if(player_index == -1){
		nprintf(("Network","Could not find player for processing game chat packet!\n"));
		return;
	}

	should_display = 0;

	// if we're the server, determine what to do with the packet here
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		// if he's targeting a specific player, find out who it is
		if(mode == MULTI_MSG_TARGET){
			to_player_index = find_player_id(to);
		} else {
			to_player_index = -1;
		}	

		// if we couldn't find who sent the message or who should be getting the message, the bail
		if(((to_player_index == -1) && (mode == MULTI_MSG_TARGET)) || (player_index == -1)){
			return;
		}

		// determine if _I_ should be seeing the text
		if(Game_mode & GM_STANDALONE_SERVER){
			should_display = 1;			
		} 
		// check against myself for several specific cases
		else {
			if((mode == MULTI_MSG_ALL) || 
				((mode == MULTI_MSG_FRIENDLY) && (Net_player->p_info.team == Net_players[player_index].p_info.team)) ||
				((mode == MULTI_MSG_HOSTILE) && (Net_player->p_info.team != Net_players[player_index].p_info.team)) ||
				((mode == MULTI_MSG_TARGET) && (MY_NET_PLAYER_NUM == to_player_index)) ||
				((mode == MULTI_MSG_EXPR) && multi_msg_matches_expr(Net_player,expr)) ){
				should_display = 1;			
			}
		}
	
		// if we're the server of a game, we need to rebroadcast to all other players			
		switch(mode){
		// individual target mission
		case MULTI_MSG_TARGET:		
			// if I was the inteneded target, or we couldn't find the intended target, don't rebroadcast
			if(to_player_index != MY_NET_PLAYER_NUM){
				send_game_chat_packet(&Net_players[player_index], msg, (int)mode, &Net_players[to_player_index], NULL, server_msg);
			}
			break;
		// expression mode
		case MULTI_MSG_EXPR:
			send_game_chat_packet(&Net_players[player_index], msg, (int)mode, NULL, expr, server_msg);
			break;
		// all other modes
		default :		
			send_game_chat_packet(&Net_players[player_index], msg, (int)mode, NULL, NULL, server_msg);
			break;
		}
	}
	// if a client receives this packet, its always ok for him to display it
	else {
		should_display = 1;
	}

	// if we're not on a standalone
	if(should_display){
		if(server_msg == 2){
			HUD_printf(msg);
		} else {
			multi_display_chat_msg(msg, player_index, !server_msg);	
		}
	}	
}


//*********************************************************************************************************
// Hud Message packet
//*********************************************************************************************************
/*
struct fs2_game_chat_packet
{
	char packet_signature; //0xC1 HUD_MSG
	int msg_size;
	char msg[msg_size];
}
*/
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// broadcast a hud message to all players
void send_hud_msg_to_all( char* msg )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;

	// only the server should be sending this packet	
	BUILD_HEADER(HUD_MSG);

	ADD_STRING(msg);

	multi_io_send_to_all( data, packet_size );
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// process an incoming hud message packet
void process_hud_message(ubyte* data, header* hinfo)
{
	int offset;
	char msg_buffer[255];
	
	offset = HEADER_LENGTH;

	GET_STRING(msg_buffer);
	PACKET_SET_SIZE();
	
	// this is the only safe place to do this since only in the mission is the HUD guaranteed to be inited
	if(Game_mode & GM_IN_MISSION){
		HUD_printf(msg_buffer);
	}	
}


//*********************************************************************************************************
// Join Packet
//*********************************************************************************************************
/*
struct fs2_game_chat_packet
{
	char packet_signature; //0xC1 HUD_MSG
	int msg_size;
	char msg[msg_size];
}
*/
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// send a join packet request to the specified address (should be a server)
void send_join_packet(net_addr* addr,join_request *jr)
{
	ubyte data[MAX_PACKET_SIZE];	
	int packet_size;
	
	// build the header and add the request
	BUILD_HEADER(JOIN);	
	add_join_request(data, &packet_size, jr);
	
	psnet_send(addr, data, packet_size);	
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// process an incoming join request packet
void process_join_packet(ubyte* data, header* hinfo)
{
	join_request jr;
	int offset;
	int ret_code;
	int host_restr_mode;
	int team0_avail,team1_avail;
	char join_string[255];
	net_addr addr;	

	// only the server of the game should ever receive this packet
	if ( !(Net_player->flags & NETINFO_FLAG_AM_MASTER) )
		return;

	offset = HEADER_LENGTH;	
	
	// read in the request info
	memset(&jr,0,sizeof(join_request));	

	get_join_request(data, &offset, &jr);

	PACKET_SET_SIZE();	

	// fill in the address information of where this came from
	fill_net_addr(&addr, hinfo->addr, hinfo->net_id, hinfo->port);

	// determine if we should accept this guy, or return a reason we should reject him
	// see the DENY_* codes in multi.h
	ret_code = multi_eval_join_request(&jr,&addr);

	// evaluate the return code
	switch(ret_code)
	{
		// he should be accepted
		case -1 :
			break;
			
		// we have to query the host because this is a restricted game
		case JOIN_QUERY_RESTRICTED :		
			if(!(Game_mode & GM_STANDALONE_SERVER)){			
				// notify the host of the event
				snd_play(&Snds[SND_CUE_VOICE]);
			}

			// set the query timestamp
			Multi_restr_query_timestamp = timestamp(MULTI_QUERY_RESTR_STAMP);
			Netgame.flags |= NG_FLAG_INGAME_JOINING;

			// determine what mode we're in
			host_restr_mode = -1;
			memset(join_string,0,255);
			if((Netgame.type_flags & NG_TYPE_TEAM) && Netgame.mode == NG_MODE_RESTRICTED){
				multi_player_ships_available(&team0_avail,&team1_avail);

				if(team0_avail && team1_avail){
					host_restr_mode = MULTI_JOIN_RESTR_MODE_4;
					sprintf(join_string,"Player %s has tried to join. Accept on team 1 or 2 ?",jr.callsign);
				} else if(team0_avail && !team1_avail){
					host_restr_mode = MULTI_JOIN_RESTR_MODE_2;
					sprintf(join_string,"Player %s has tried to join team 0, accept y/n ? ?",jr.callsign);
				} else if(!team0_avail && team1_avail){
					host_restr_mode = MULTI_JOIN_RESTR_MODE_3;
					sprintf(join_string,"Player %s has tried to join team 1, accept y/n ?",jr.callsign);
				}
			} else if(Netgame.mode == NG_MODE_RESTRICTED){
				host_restr_mode = MULTI_JOIN_RESTR_MODE_1;
				sprintf(join_string,XSTR("Player %s has tried to join, accept y/n ?",715),jr.callsign);
			}
			Assert(host_restr_mode != -1);

			// store the request info
			memcpy(&Multi_restr_join_request,&jr,sizeof(join_request));
			memcpy(&Multi_restr_addr,&addr,sizeof(net_addr));
			Multi_join_restr_mode = host_restr_mode;

			// if i'm the standalone server, I need to send a query to the host
			if(Game_mode & GM_STANDALONE_SERVER){
				send_host_restr_packet(jr.callsign,0,Multi_join_restr_mode);
			} else {
				HUD_printf(join_string);
			}

			// NETLOG
			ml_printf(NOX("Receive restricted join request from %s"), jr.callsign);

			return;
		
		// he'e being denied for some reason
		default :	
			// send him the reason he is being denied
			send_deny_packet(&addr,ret_code);
			Netgame.flags &= ~(NG_FLAG_INGAME_JOINING);
			return;
	} 

	// process the rest of the request
	multi_process_valid_join_request(&jr,&addr);
}

//*********************************************************************************************************
// New Player Packet
//*********************************************************************************************************
/*
struct fs2_new_player_packet
{
	char packet_signature; // 0xB4 NOTIFY_NEW_PLAYER
	int new_player_num;
	net_addr player_addr;
	short player_id;
	int flags;
	
	int i;
	char callsign[i];

	int j;
	char plyr_image_filename[j];

	int k;
	char plyr_squad_filename[k];

	int l;
	char plyr_pxo_squad_name[l];
}
*/

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// send a notification that a new player has joined the game (if target != NULL, broadcast the packet)
void send_new_player_packet(int new_player_num,net_player *target)
{
	ubyte data[MAX_PACKET_SIZE], val;
	int packet_size = 0;	
	
	BUILD_HEADER( NOTIFY_NEW_PLAYER );

	// add the new player's info
	ADD_INT(new_player_num);
//	ADD_DATA(Net_players[new_player_num].p_info.addr);
	add_net_addr(data, &packet_size, &Net_players[new_player_num].p_info.addr);
	ADD_SHORT(Net_players[new_player_num].player_id);
	ADD_INT(Net_players[new_player_num].flags);
	ADD_STRING(Net_players[new_player_num].m_player->callsign);
	ADD_STRING(Net_players[new_player_num].m_player->image_filename);
	ADD_STRING(Net_players[new_player_num].m_player->squad_filename);
	ADD_STRING(Net_players[new_player_num].p_info.pxo_squad_name);

	val = (ubyte)Net_players[new_player_num].p_info.team;
	ADD_DATA(val);

	// broadcast the data
	if(target != NULL){
		multi_io_send_reliable(target, data, packet_size);
	} else {
		multi_io_send_to_all_reliable(data, packet_size);	
	}
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// process a notification for a new player who has joined the game
void process_new_player_packet(ubyte* data, header* hinfo)
{
	int already_in_game = 0;
	int offset, new_player_num,player_num,new_flags;
	net_addr new_addr;	
	char new_player_name[CALLSIGN_LEN+2] = "";
	char new_player_image[MAX_FILENAME_LEN+1] = "";
	char new_player_squad[MAX_FILENAME_LEN+1] = "";
	char new_player_pxo_squad[LOGIN_LEN+1] = "";
	char notify_string[256];
	ubyte team;
	short new_id;

	offset = HEADER_LENGTH;

	// get the new players information
	GET_INT(new_player_num);
//	GET_DATA(new_addr);
	get_net_addr(data, &offset, &new_addr);
	GET_SHORT(new_id);
	GET_INT(new_flags);
	GET_STRING(new_player_name);	
	GET_STRING(new_player_image);
	GET_STRING(new_player_squad);
	GET_STRING(new_player_pxo_squad);
	GET_DATA(team);
	PACKET_SET_SIZE();

	player_num = multi_find_open_player_slot();
	Assert(player_num != -1);
	
	// note that this new code does not check for duplicate IPs. It merely checks to see if
	// the slot referenced by new_player_num is already occupied by a connected player
	if(MULTI_CONNECTED(Net_players[new_player_num])){
		already_in_game=1;
	}

	// if he's not alreayd in the game for one reason or another
	if ( !already_in_game ) {
		if ( Game_mode & GM_IN_MISSION ){
			HUD_sourced_printf(HUD_SOURCE_COMPUTER, XSTR("%s has entered the game\n",716), new_player_name);
		}

		// create the player
		memcpy(new_addr.net_id, Psnet_my_addr.net_id, 4);

		if(new_flags & NETINFO_FLAG_OBSERVER){
			multi_obs_create_player(new_player_num,new_player_name,&new_addr,&Players[player_num]);
			Net_players[new_player_num].flags |= new_flags;
		} else {
			multi_create_player( new_player_num, &Players[player_num],new_player_name, &new_addr, -1, new_id );
			Net_players[new_player_num].flags |= new_flags;
		}

		// copy in the filename
		if(new_player_image[0] != '\0'){
			strcpy_s(Net_players[new_player_num].m_player->image_filename, new_player_image);
		} else {
			strcpy_s(Net_players[new_player_num].m_player->image_filename, "");
		}
		// copy his pilot squad filename
		Net_players[new_player_num].m_player->insignia_texture = -1;
		player_set_squad_bitmap(Net_players[new_player_num].m_player, new_player_squad);				

		// copy in his pxo squad name
		strcpy_s(Net_players[new_player_num].p_info.pxo_squad_name, new_player_pxo_squad);

		// since we just created the player, set the last_heard_time here.
		Net_players[new_player_num].last_heard_time = timer_get_fixed_seconds();

		Net_players[new_player_num].p_info.team = team;

		Net_players[new_player_num].player_id = new_id;

		// zero out this players ping
		multi_ping_reset(&Net_players[new_player_num].s_info.ping);		

		// add a chat message
		if(Net_players[new_player_num].m_player->callsign != NULL){
			sprintf(notify_string,XSTR("<%s has joined>",717),Net_players[new_player_num].m_player->callsign);
			multi_display_chat_msg(notify_string,0,0);
		}
	}		

	// NETLOG
	ml_printf(NOX("Received notification of new player %s"), Net_players[new_player_num].m_player->callsign);
	
	// let the current ui screen know someone joined
	switch(gameseq_get_state()){
	case GS_STATE_MULTI_HOST_SETUP :
		multi_create_handle_join(&Net_players[new_player_num]);
		break;
	case GS_STATE_MULTI_CLIENT_SETUP :
		multi_jw_handle_join(&Net_players[new_player_num]);
		break;
	}
}

//*********************************************************************************************************
// Accept Player Data Packet
//*********************************************************************************************************
/*
struct fs2_accept_player_data
{
	char packet_signature; //ACCEPT_PLAYER_DATA
	ubyte stop;
	int player_num;
	net_addr plyr_addr;
	int player_id;

	int i;
	char callsign[i];

    int j;
	char plr_image_filename[j];

	int k;
    char plr_squad_filename[k]
	
	int l
	char plr_pxo_squadname[l];

    int flags;

    if (is_ingame)
	  int net_signature;

    
};
*/

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define PLAYER_DATA_SLOP	100

void send_accept_player_data( net_player *npp, int is_ingame )
{
	int packet_size;	
	int i;
	ubyte data[MAX_PACKET_SIZE], stop;

	BUILD_HEADER(ACCEPT_PLAYER_DATA);

	// add in the netplayer data for all players	
	stop = APD_NEXT;
	for (i=0; i<MAX_PLAYERS; i++) {
		// skip non connected players
		if ( !MULTI_CONNECTED(Net_players[i]) ){
			continue;
		}
		
		// skip this new player's entry
		if ( npp->player_id == Net_players[i].player_id ){
			continue;
		}
		
		// add the stop byte
		ADD_DATA(stop);

		// add the player's number
		ADD_INT(i);		

		// add the player's address
	//	ADD_DATA(Net_players[i].p_info.addr);
		add_net_addr(data, &packet_size, &Net_players[i].p_info.addr);

		// add his id#
		ADD_SHORT(Net_players[i].player_id);

		// add his callsign
		ADD_STRING(Net_players[i].m_player->callsign);

		// add his image filename
		ADD_STRING(Net_players[i].m_player->image_filename);

		// add his squad filename
		ADD_STRING(Net_players[i].m_player->squad_filename);

		// add his PXO squad name
		ADD_STRING(Net_players[i].p_info.pxo_squad_name);
		
		// add his flags
		ADD_INT(Net_players[i].flags);		

		// add his object's net sig
		if ( is_ingame ) {
			ADD_USHORT( Objects[Net_players[i].m_player->objnum].net_signature );
		}

		if ( (packet_size + PLAYER_DATA_SLOP) > MAX_PACKET_SIZE ) {
			stop = APD_END_PACKET;
			ADD_DATA(stop);			
			multi_io_send_reliable( npp, data, packet_size );
			BUILD_HEADER(ACCEPT_PLAYER_DATA);
			stop = APD_NEXT;
		}

	}

	// add the stop byte
	stop = APD_END_DATA;
	ADD_DATA(stop);	
	multi_io_send_reliable(npp, data, packet_size);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// process the player data from the server
void process_accept_player_data( ubyte *data, header *hinfo )
{
	int offset, player_num, player_slot_num, new_flags;
	char name[CALLSIGN_LEN + 1] = "";
	char image_name[MAX_FILENAME_LEN + 1] = "";
	char squad_name[MAX_FILENAME_LEN + 1] = "";
	char pxo_squad_name[LOGIN_LEN+1] = "";
	short player_id;
	net_addr addr;
	ubyte stop;
	ushort ig_signature;

	offset = HEADER_LENGTH;

	GET_DATA(stop);
	while ( stop == APD_NEXT ) {
		player_slot_num = multi_find_open_player_slot();
		Assert(player_slot_num != -1);

		// get the player's number
		GET_INT(player_num);

		// add the player's address
	//	GET_DATA(addr);
		get_net_addr(data, &offset, &addr);

		// get the player's id#
		GET_SHORT(player_id);		

		// get his callsign
		GET_STRING(name);

		// add his image filename
		GET_STRING(image_name);

		// get his squad logo filename
		GET_STRING(squad_name);

		// get his PXO squad name
		GET_STRING(pxo_squad_name);
		
		// get his flags
		GET_INT(new_flags);
		
		if (Net_players[player_num].flags & NETINFO_FLAG_OBSERVER) {
			if (!multi_obs_create_player(player_num, name, &addr, &Players[player_slot_num])) {
				Int3();
			}

		} else {
			//  the error handling here is less than stellar.  We should probably put up a popup and go
			// back to the main menu.  But then again, this should never ever happen!
			if ( !multi_create_player(player_num, &Players[player_slot_num],name, &addr, -1, player_id) ) {
				Int3();
			}
		}

		// copy his image filename
		strcpy_s(Net_players[player_num].m_player->image_filename, image_name);
		
		// copy his pilot squad filename
		Net_players[player_num].m_player->insignia_texture = -1;
		player_set_squad_bitmap(Net_players[player_num].m_player, squad_name);

		// copy his pxo squad name
		strcpy_s(Net_players[player_num].p_info.pxo_squad_name, pxo_squad_name);

		// set his player id#
		Net_players[player_num].player_id = player_id;

		// mark him as being connected
		Net_players[player_num].flags |= NETINFO_FLAG_CONNECTED;
		Net_players[player_num].flags |= new_flags;

		// set the server pointer
		if ( Net_players[player_num].flags & NETINFO_FLAG_AM_MASTER ) {
			Netgame.server = &Net_players[player_num];
			Netgame.server->last_heard_time = timer_get_fixed_seconds();

			// also - always set the server address to be where this data came from, NOT from 
			// the data in the packet		
			fill_net_addr(&Net_players[player_num].p_info.addr, hinfo->addr, hinfo->net_id, hinfo->port);
		}

		// set the host pointer
		if ( Net_players[player_num].flags & NETINFO_FLAG_GAME_HOST ) {
			Netgame.host = &Net_players[player_num];
		}

		// read in the player's object net signature and store as his objnum for now
		if ( Net_player->flags & NETINFO_FLAG_ACCEPT_INGAME ) {
			GET_USHORT( ig_signature );
			Net_players[player_num].m_player->objnum = ig_signature;
		}

		// get the stop byte
		GET_DATA(stop);
	}
	PACKET_SET_SIZE();

	if ( stop == APD_END_DATA ) {
		// if joining a game automatically, set the connect address to NULl so we don't try and
		// do this next time we enter a game
		if (Cmdline_connect_addr != NULL) {
			Cmdline_connect_addr = NULL;
		}

		// send my stats to the server if I'm not in observer mode
		if (!(Net_player->flags & NETINFO_FLAG_ACCEPT_OBSERVER)) {
			send_player_stats_block_packet(Net_player, STATS_ALLTIME);
		}

		// if i'm being accepted as a host, then move into the host setup state
		if ( Net_player->flags & NETINFO_FLAG_ACCEPT_HOST) {
			// set my permission bits
			Net_player->flags |= NETINFO_FLAG_GAME_HOST;
			Net_player->state = NETPLAYER_STATE_STD_HOST_SETUP;

			gameseq_post_event(GS_EVENT_MULTI_START_GAME);
		}

		if ( Net_player->flags & NETINFO_FLAG_ACCEPT_OBSERVER) {
			Net_player->flags |= NETINFO_FLAG_OBSERVER;

			// since observers can join 1 of 2 ways, only do this if we're not doing an ingame observer join
			if ( !(Net_player->flags & NETINFO_FLAG_ACCEPT_INGAME) ) {
				gameseq_post_event(GS_EVENT_MULTI_CLIENT_SETUP);
			}
		}

		if ( Net_player->flags & NETINFO_FLAG_ACCEPT_CLIENT) {
			gameseq_post_event(GS_EVENT_MULTI_CLIENT_SETUP);	
		}

		if ( Net_player->flags & NETINFO_FLAG_ACCEPT_INGAME) {
			// flag myself as being an ingame joiner
			Net_player->flags |= NETINFO_FLAG_INGAME_JOIN;		

			// move myself into the ingame join mission sync state
			Multi_sync_mode = MULTI_SYNC_INGAME;
			gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);				
		}

		// update my options on the server
		multi_options_update_local();

		// if we're in PXO mode, mark it down in our player struct
		if(MULTI_IS_TRACKER_GAME){
			Player->flags |= PLAYER_FLAGS_HAS_PLAYED_PXO;
			Player->save_flags |= PLAYER_FLAGS_HAS_PLAYED_PXO;
		}
	}
}


//*********************************************************************************************************
// Accept Player Packet
//*********************************************************************************************************
/*
struct fs2_accept_packet
{
	char packet_signature;
	int code;

    if (code & ACCEPT_INGAME)
	{
		int gm_mis_fname_len;
		char mission_filename[gm_mis_fname_len];
		unsigned char ingame_joining_team;
		if (ingame_joining_team == 1)
			unsigned char ingame_join_team;

	}

	int skill_level;
	int player_num;
	short player_id;
	int netgame_type_flags;

}
*/
// send an accept packet to a client in response to a request to join the game
void send_accept_packet(int new_player_num, int code, int ingame_join_team)
{
	int packet_size, i;
	ubyte data[MAX_PACKET_SIZE],val;
	char notify_string[256];

	// sanity
	Assert(new_player_num >= 0);

	// setup his "reliable" socket
	Net_players[new_player_num].last_heard_time = timer_get_fixed_seconds();

	// build the packet header
	packet_size = 0;
	BUILD_HEADER(ACCEPT);	
	
	// add the accept code
	ADD_INT(code);
	
	// add code specific accept data
	if (code & ACCEPT_INGAME) {
		// the game filename
		ADD_STRING(Game_current_mission_filename);

		// if he is joining on a specific team, mark it here
		if(ingame_join_team != -1){
			val = 1;
			ADD_DATA(val);
			val = (ubyte)ingame_join_team;
			ADD_DATA(val);
		} else {
			val = 0;
			ADD_DATA(val);
		}
	} 

	if (code & ACCEPT_OBSERVER) {
		Assert(!(code & (ACCEPT_CLIENT | ACCEPT_HOST)));
	}

	if (code & ACCEPT_HOST) {
		Assert(!(code & (ACCEPT_CLIENT | ACCEPT_OBSERVER | ACCEPT_INGAME)));
	}

	if (code & ACCEPT_CLIENT) {
		Assert(!(code & (ACCEPT_HOST | ACCEPT_OBSERVER | ACCEPT_INGAME)));
	}

	// add the current skill level setting on the host
	ADD_INT(Game_skill_level);

	// add this guys player num 
	ADD_INT(new_player_num);

	// add his player id
	ADD_SHORT(Net_players[new_player_num].player_id);

	// add netgame type flags
	ADD_INT(Netgame.type_flags);
	
//#ifndef NDEBUG
	// char buffer[100];
	// nprintf(("Network", "About to send accept packet to %s on port %d\n", get_text_address(buffer, addr->addr), addr->port ));
//#endif

	// actually send the packet	
	psnet_send(&Net_players[new_player_num].p_info.addr, data, packet_size);

   // if he's not an observer, inform all the other players in the game about him	
	// inform the other players in the game about this new player
	for (i=0; i<MAX_PLAYERS; i++) {
		// skip unconnected players as well as this new guy himself
		if ( !MULTI_CONNECTED(Net_players[i]) || psnet_same(&Net_players[new_player_num].p_info.addr, &(Net_players[i].p_info.addr)) || (Net_player == &Net_players[i])) {
			continue;
		}

		// send the new packet
		send_new_player_packet(new_player_num,&Net_players[i]);
	}

	// add a chat message
	if(Net_players[new_player_num].m_player->callsign != NULL){
		sprintf(notify_string,XSTR("<%s has joined>",717), Net_players[new_player_num].m_player->callsign);
		multi_display_chat_msg(notify_string, 0, 0);
	}	

	// handle any team vs. team details
	if (!(code & ACCEPT_OBSERVER)) {		
		multi_team_handle_join(&Net_players[new_player_num]);		
	}		

	// NETLOG
	if(Net_players[new_player_num].tracker_player_id >= 0){
		ml_printf(NOX("Server accepted %s (tracker id %d) as new client"), Net_players[new_player_num].m_player->callsign, Net_players[new_player_num].tracker_player_id);
	} else {
		ml_printf(NOX("Server accepted %s as new client"), Net_players[new_player_num].m_player->callsign);
	}
}

// process an accept packet from the server
extern int Select_default_ship;

void process_accept_packet(ubyte* data, header* hinfo)
{
	int code, my_player_num, offset;
	ubyte val,team = 0;
	short player_id;
	
	// get the accept code
	offset = HEADER_LENGTH;	

	GET_INT(code);

	// read in the accept code specific data
	val = 0;
	if (code & ACCEPT_INGAME) {
		// the game filename
		GET_STRING(Game_current_mission_filename);
		mprintf(("Got mission filename %s\n", Game_current_mission_filename));
		Select_default_ship = 0;

		// determine if I'm being placed on a team
		GET_DATA(val);
		if(val){
			GET_DATA(team);
		}
	}

	if (code & ACCEPT_OBSERVER) {
		Assert(!(code & (ACCEPT_CLIENT | ACCEPT_HOST)));
	}

	if (code & ACCEPT_HOST) {
		Assert(!(code & (ACCEPT_CLIENT | ACCEPT_OBSERVER | ACCEPT_INGAME)));
	}

	if (code & ACCEPT_CLIENT) {
		Assert(!(code & (ACCEPT_HOST | ACCEPT_OBSERVER | ACCEPT_INGAME)));
	}

	// fill in the netgame server address
	fill_net_addr( &Netgame.server_addr, hinfo->addr, hinfo->net_id, hinfo->port );	

	// get the skill level setting
	GET_INT(Game_skill_level);

	// get my netplayer number
	GET_INT(my_player_num);

	// get my id #
	GET_SHORT(player_id);

	// get netgame type flags
	GET_INT(Netgame.type_flags);

	// setup the Net_players structure for myself first
	Net_player = &Net_players[my_player_num];
	Net_player->flags = 0;
	Net_player->tracker_player_id = Multi_tracker_id;
	Net_player->player_id = player_id;
	Net_player->s_info.xfer_handle = -1;
	// stuff_netplayer_info( Net_player, &Psnet_my_addr, Ships[Objects[Player->objnum].instance].ship_info_index, Player );	
	stuff_netplayer_info( Net_player, &Psnet_my_addr, 0, Player );	
	multi_options_local_load(&Net_player->p_info.options, Net_player);
	Net_player->p_info.team = team;	

	// determine if I have a CD
	if(Multi_has_cd){
		Net_player->flags |= NETINFO_FLAG_HAS_CD;
	}	

	// set accept code in netplayer for this guy
	if ( code & ACCEPT_INGAME ){
		Net_player->flags |= NETINFO_FLAG_ACCEPT_INGAME;
	}
	if ( code & ACCEPT_OBSERVER ){
		Net_player->flags |= NETINFO_FLAG_ACCEPT_OBSERVER;
	}
	if ( code & ACCEPT_HOST ){
		Net_player->flags |= NETINFO_FLAG_ACCEPT_HOST;
	}
	if ( code & ACCEPT_CLIENT ){
		Net_player->flags |= NETINFO_FLAG_ACCEPT_CLIENT;
	}

	// if I have hacked data
	if(game_hacked_data()){
		Net_player->flags |= NETINFO_FLAG_HAXOR;
	}

	// if we're supposed to flush our local data cache, do so now
	if(Net_player->p_info.options.flags & MLO_FLAG_FLUSH_CACHE){
		multi_flush_multidata_cache();
	}

	Net_player->sv_bytes_sent = 0;
	Net_player->sv_last_pl = -1;
	Net_player->cl_bytes_recvd = 0;
	Net_player->cl_last_pl = -1;

	// intiialize endgame stuff
	multi_endgame_init();

	PACKET_SET_SIZE();

	// make a call to psnet to initialize and try to connect with the server.
	psnet_rel_connect_to_server( &Net_player->reliable_socket, &Netgame.server_addr );
	if ( Net_player->reliable_socket == INVALID_SOCKET ) {
		multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_CONNECT_FAIL);
	}
}

//*********************************************************************************************************
// Player Leave Packet
//*********************************************************************************************************
/*
struct fs2_leave_game_packet
{
	char packet_signature;
	char kicked_reason;
	short player_id;
	
}
*/
// send a notice that the player at net_addr is leaving (if target is NULL, the broadcast the packet)
void send_leave_game_packet(short player_id, int kicked_reason, net_player *target)
{
	ubyte data[MAX_PACKET_SIZE];
	char val;
	int packet_size = 0;

	BUILD_HEADER(LEAVE_GAME);

	// add a flag indicating whether he was kicked or not
	val = (char)kicked_reason;
	ADD_DATA(val);

	if (player_id < 0) {
		ADD_SHORT(Net_player->player_id);

		// inform the host that we are leaving the game
		if (Net_player->flags & NETINFO_FLAG_AM_MASTER) {			
			multi_io_send_to_all_reliable(data, packet_size);
		} else {
			multi_io_send_reliable(Net_player, data, packet_size);
		}
	}
	// this is the case where to server is tossing a player (or indicating a respawned player has quit or become an observer)
	// so he has to tell everyone that this guy left
	else {
		nprintf(("Network","Sending a leave game packet to all players (server)\n"));

		// a couple of important checks
		Assert(player_id != Net_player->player_id);
		Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);

		// add the id of the guy to be kicked
		ADD_SHORT(player_id);

		// broadcast to everyone
		if (target == NULL) {			
			multi_io_send_to_all_reliable(data, packet_size);
		} else {
			multi_io_send_reliable(target, data, packet_size);
		}
	}
}

// process a notification that a player has left the game
void process_leave_game_packet(ubyte* data, header* hinfo)
{
	int offset;
	short deader_id;
	int player_num;
	char kicked_reason;	
	char str[512];

	offset = HEADER_LENGTH;

	// get whether he was kicked
	GET_DATA(kicked_reason);

	// get the address of the guy who is to leave
	GET_SHORT(deader_id);
	PACKET_SET_SIZE();

	// determine who is dropping and printf out a notification
	player_num = find_player_id(deader_id);
	if (player_num == -1) {
		nprintf(("Network", "Received leave game packet for unknown player, ignoring\n"));
		return;

	} else {
		nprintf(("Network", "Received a leave game notice for %s\n", Net_players[player_num].m_player->callsign));
	}

	// a hook to display that a player was kicked
	if (kicked_reason >= 0){
		// if it was me that was kicked, leave the game
		if((Net_player != NULL) && (Net_player->player_id == deader_id)){
			int notify_code;

			switch(kicked_reason){
			case KICK_REASON_BAD_XFER:
				notify_code = MULTI_END_NOTIFY_KICKED_BAD_XFER;
				break;
			case KICK_REASON_CANT_XFER:
				notify_code = MULTI_END_NOTIFY_KICKED_CANT_XFER;
				break;
			case KICK_REASON_INGAME_ENDED:
				notify_code = MULTI_END_NOTIFY_KICKED_INGAME_ENDED;
				break;
			default:
				notify_code = MULTI_END_NOTIFY_KICKED;
				break;
			}

			multi_quit_game(PROMPT_NONE, notify_code);
			return;

		// otherwise indicate someone was kicked
		} else {
			nprintf(("Network","%s was kicked\n",Net_players[player_num].m_player->callsign));			

			// display the result
			memset(str, 0, 512);
			multi_kick_get_text(&Net_players[player_num], kicked_reason, str);			
			multi_display_chat_msg(str, player_num, 0);
		}
	}

	// first of all, if we're the master, we should be rebroadcasting this packet
	if (Net_player->flags & NETINFO_FLAG_AM_MASTER) {
		char msg[255];

		sprintf(msg, XSTR("%s has left the game",719), Net_players[player_num].m_player->callsign );

		if (!(Game_mode & GM_STANDALONE_SERVER)){
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, msg);
		}

		send_hud_msg_to_all(msg);		
		multi_io_send_to_all_reliable(data, offset);
	}

	// leave the game if the host and/or master has dropped
	/*
	if (((Net_players[player_num].flags & NETINFO_FLAG_AM_MASTER) || (Net_players[player_num].flags & NETINFO_FLAG_GAME_HOST)) ) {		
		nprintf(("Network","Host and/or server has left the game - aborting...\n"));

		// NETLOG
		ml_string(NOX("Host and/or server has left the game"));

		// if the host leaves in the debriefing state, we should still wait until the player selects accept before we quit
		if (gameseq_get_state() != GS_STATE_DEBRIEF) {			
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_SERVER_LEFT);
		}		

		delete_player(player_num);
	} else {
		*/
	delete_player(player_num);	

	// OSAPI GUI stuff (if standalone)
	if (Game_mode & GM_STANDALONE_SERVER) {
      // returns true if we should reset the standalone
		if (std_remove_player(&Net_players[player_num])) {
			nprintf(("Network", "Should reset!!\n"));
			return;
		}

		// update these gui vals
		std_connect_set_host_connect_status();
		std_connect_set_connect_count();
	}
}

//*********************************************************************************************************
// Game active packet
//*********************************************************************************************************
/*
struct fs2_game_active_packet
{
	char packet_signature;
	ubyte server_version;
	ubyte compat_version;
	int len1;
	char netgame_name[len1];
	int len2;
	char netgame_mission_name[len2];
	int len3;
	char netgame_title[len3];
	ubyte num_players;
	unsigned short flags;
}
*/
// send information about this currently active game to the specified address
void send_game_active_packet(net_addr* addr)
{
	int packet_size;
	ushort flags;
	ubyte data[MAX_PACKET_SIZE],val;

	// build the header and add the data
	BUILD_HEADER(GAME_ACTIVE);
	
	// add the server version and compatible version #
	val = MULTI_FS_SERVER_VERSION;
	ADD_DATA(val);
	val = MULTI_FS_SERVER_COMPATIBLE_VERSION;
	ADD_DATA(val);

	ADD_STRING(Netgame.name);
	ADD_STRING(Netgame.mission_name);
	ADD_STRING(Netgame.title);	
	val = (ubyte)multi_num_players();
	ADD_DATA(val);
	
	// add the proper flags
	flags = 0;
	if ( (Netgame.mode == NG_MODE_PASSWORD) || ((Game_mode & GM_STANDALONE_SERVER) && (multi_num_players() == 0) && (std_is_host_passwd())) ) {
		flags |= AG_FLAG_PASSWD;
	}

	// proper netgame type flags
	if(Netgame.type_flags & NG_TYPE_TEAM){
		flags |= AG_FLAG_TEAMS;
	} else if(Netgame.type_flags & NG_TYPE_DOGFIGHT){
		flags |= AG_FLAG_DOGFIGHT;
	} else {
		flags |= AG_FLAG_COOP;
	}	

	// proper netgame state flags
	switch(Netgame.game_state){
	case NETGAME_STATE_FORMING:
		flags |= AG_FLAG_FORMING;
		break;
	
	case NETGAME_STATE_BRIEFING:
	case NETGAME_STATE_MISSION_SYNC:
	case NETGAME_STATE_HOST_SETUP:
		flags |= AG_FLAG_BRIEFING;
		break;
	
	case NETGAME_STATE_IN_MISSION:
		flags |= AG_FLAG_IN_MISSION;
		break;
	
	case NETGAME_STATE_PAUSED:
		flags |= AG_FLAG_PAUSE;
		break;
	
	case NETGAME_STATE_ENDGAME:
	case NETGAME_STATE_DEBRIEF:
		flags |= AG_FLAG_DEBRIEF;
		break;
	}

	// if this is a standalone
	if(Game_mode & GM_STANDALONE_SERVER){
		flags |= AG_FLAG_STANDALONE;
	}

	// if we're in campaign mode
	if(Netgame.campaign_mode == MP_CAMPAIGN){
		flags |= AG_FLAG_CAMPAIGN;
	}

	// add the data about the connection speed of the host machine
	Assert( (Multi_connection_speed >= 0) && (Multi_connection_speed <= 4) );
	flags |= (Multi_connection_speed << AG_FLAG_CONNECTION_BIT);

	ADD_USHORT(flags);
	
	// send the data	
	psnet_send(addr, data, packet_size);
}

// process information about an active game
void process_game_active_packet(ubyte* data, header* hinfo)
{
	int offset;	
	ubyte val;
	active_game ag;
	int modes_compatible = 1;
	
	fill_net_addr(&ag.server_addr, hinfo->addr, hinfo->net_id, hinfo->port);

	// read this game into a temporary structure
	offset = HEADER_LENGTH;

	// get the server version and compatible version
	GET_DATA(ag.version);
	GET_DATA(ag.comp_version);

	GET_STRING(ag.name);
	GET_STRING(ag.mission_name);
	GET_STRING(ag.title);	
	GET_DATA(val);
	ag.num_players = val;
	GET_USHORT(ag.flags);

	PACKET_SET_SIZE();	

	if ( (ag.flags & AG_FLAG_TRACKER) && !Multi_options_g.pxo )
		modes_compatible = 0;

	if ( !(ag.flags & AG_FLAG_TRACKER) && Multi_options_g.pxo )
		modes_compatible = 0;

	// if this is a compatible version, and our modes are compatible, register it
	if ( (ag.version == MULTI_FS_SERVER_VERSION) && modes_compatible ) {
		multi_update_active_games(&ag);
	}
}

//*********************************************************************************************************
// Game Update Packet
//*********************************************************************************************************
/*
struct fs2_game_update
{
	char packet_signature;
	int len1;
	char netgame_name[len1];
	int len2;
	char netgame_mission_name[len2];
	int len3;
	char netgame_title[len3];
	int len4;
	char netgame_campaign_name[len4];
	int campaign_mode;
	int max_players;
	int security;
	unsigned int respawn;
	int flags;
	int type_flags;
	int version_info;
	ubyte debug_flags;

	// !!!!!! this isn't relying on information earlier in the packet!
	// receiving seems to always assume it's there!
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		 int game_state;
	}

  }

*/

// send_game_update_packet sends an updated Netgame structure to all players currently connected.  The update
// is used to change the current mission, current state, etc.
void send_netgame_update_packet(net_player *pl)
{
	int packet_size;
	int idx;
	ubyte data[MAX_PACKET_SIZE];

	packet_size = 0;
	BUILD_HEADER(GAME_UPDATE);
	
	// with new mission description field, this becomes way to large
	// so we must add every element piece by piece except the 	
	ADD_STRING(Netgame.name);
	ADD_STRING(Netgame.mission_name);	
	ADD_STRING(Netgame.title);
	ADD_STRING(Netgame.campaign_name);
	ADD_INT(Netgame.campaign_mode);	
	ADD_INT(Netgame.max_players);			
	ADD_INT(Netgame.security);
	ADD_UINT(Netgame.respawn);
	ADD_INT(Netgame.flags);
	ADD_INT(Netgame.type_flags);
	ADD_INT(Netgame.version_info);
	ADD_DATA(Netgame.debug_flags);

	// only the server should ever send the netgame state (standalone situation)
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		ADD_INT(Netgame.game_state);
	}
	
	// if we're the host on a standalone, send to the standalone and let him rebroadcast
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		if ( pl == NULL ) {			
			multi_io_send_to_all_reliable(data, packet_size);

			for(idx=0; idx<MAX_PLAYERS; idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx])){
					send_netgame_descript_packet(&Net_players[idx].p_info.addr, 1);
				}
			}
		} else {			
			multi_io_send_reliable(pl, data, packet_size);
			send_netgame_descript_packet( &pl->p_info.addr , 1 );
		}
	} else {
		Assert( pl == NULL );			// I don't think that a host in a standalone game would get here.
		multi_io_send_reliable(Net_player, data, packet_size);
	}		

	// host should always send a netgame options update as well
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
		multi_options_update_netgame();
	}
}

// process information about the netgame sent from the server/host
void process_netgame_update_packet( ubyte *data, header *hinfo )
{
	int offset,old_flags;	
	int ng_state;
		
	Assert(!(Game_mode & GM_STANDALONE_SERVER));
	Assert(!(Net_player->flags & NETINFO_FLAG_AM_MASTER));

	// read in the netgame information
	offset = HEADER_LENGTH;	
	GET_STRING(Netgame.name);
	GET_STRING(Netgame.mission_name);	
	GET_STRING(Netgame.title);	
	GET_STRING(Netgame.campaign_name);
	GET_INT(Netgame.campaign_mode);	
	GET_INT(Netgame.max_players);					// ignore on the standalone, who keeps track of this himself			
	GET_INT(Netgame.security);
	GET_UINT(Netgame.respawn);		
	
	// be sure not to blast the quitting flag because of the "one frame extra" problem
	old_flags = Netgame.flags;	
	GET_INT(Netgame.flags);	
	GET_INT(Netgame.type_flags);
	GET_INT(Netgame.version_info);
	GET_DATA(Netgame.debug_flags);

	// netgame state	
	GET_INT(ng_state);	
	
	PACKET_SET_SIZE();
							
	// now compare the passed in game state to our current known state.  If it has changed, then maybe
	// do something interesting.	
	// move from the forming or debriefing state to the mission sync state
  if ( ng_state == NETGAME_STATE_MISSION_SYNC ){
	  // if coming from the forming state
		if( (Netgame.game_state == NETGAME_STATE_FORMING) ||
			 ((Netgame.game_state != NETGAME_STATE_FORMING) && ((gameseq_get_state() == GS_STATE_MULTI_HOST_SETUP) || (gameseq_get_state() == GS_STATE_MULTI_CLIENT_SETUP))) ){
 			// do any special processing for forced state transitions
			multi_handle_state_special();
						
			Multi_sync_mode = MULTI_SYNC_PRE_BRIEFING;
			strncpy( Game_current_mission_filename, Netgame.mission_name, MAX_FILENAME_LEN );					
			gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);
		} 
		// if coming from the debriefing state
		else if( (Netgame.game_state == NETGAME_STATE_DEBRIEF) ||
			 ((Netgame.game_state != NETGAME_STATE_DEBRIEF) && ((gameseq_get_state() == GS_STATE_DEBRIEF) || (gameseq_get_state() == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)) ) ){ 

			// do any special processing for forced state transitions
			multi_handle_state_special();

			multi_flush_mission_stuff();
						
			Multi_sync_mode = MULTI_SYNC_PRE_BRIEFING;
			strncpy( Game_current_mission_filename, Netgame.mission_name, MAX_FILENAME_LEN );					
			gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);
		}
	} 
	// move from mission sync to team select
	else if ( ng_state == NETGAME_STATE_BRIEFING ){
		if( (Netgame.game_state == NETGAME_STATE_MISSION_SYNC) ||
			 ((Netgame.game_state != NETGAME_STATE_MISSION_SYNC) && (gameseq_get_state() == GS_STATE_MULTI_MISSION_SYNC) && (Multi_sync_mode != MULTI_SYNC_POST_BRIEFING)) ){
			
			// do any special processing for forced state transitions
			multi_handle_state_special();

			strncpy( Game_current_mission_filename, Netgame.mission_name, MAX_FILENAME_LEN );					
			gameseq_post_event(GS_EVENT_START_BRIEFING);			
		}
	} 		
	// move from the debriefing to the create game screen
	else if ( ng_state == NETGAME_STATE_FORMING ){
		if( (Netgame.game_state == NETGAME_STATE_DEBRIEF) ||
			 ((Netgame.game_state != NETGAME_STATE_DEBRIEF) && ((gameseq_get_state() == GS_STATE_DEBRIEF) || (gameseq_get_state() == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)) ) ){ 
			// do any special processing for forced state transitions
			multi_handle_state_special();

			multi_flush_mission_stuff();
			
			// move to the proper screen
			if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
				gameseq_post_event(GS_EVENT_MULTI_HOST_SETUP);
			} else {
				gameseq_post_event(GS_EVENT_MULTI_CLIENT_SETUP);
			}
		}
	}		

	Netgame.game_state = ng_state;	
}

//*********************************************************************************************************
// Game Update Packet
//*********************************************************************************************************
/*

*/
// send a request or a reply for mission description, if code == 0, request, if code == 1, reply
void send_netgame_descript_packet(net_addr *addr, int code)
{
	ubyte data[MAX_PACKET_SIZE],val;
	int desc_len;
	int packet_size = 0;

	// build the header
	BUILD_HEADER(UPDATE_DESCRIPT);

	val = (ubyte)code;
	ADD_DATA(val);	

	if(code == 1){
		// add as much of the description as we dare
		desc_len = strlen(The_mission.mission_desc);
		if(desc_len > MAX_PACKET_SIZE - 10){
			desc_len = MAX_PACKET_SIZE - 10;
			ADD_INT(desc_len);
			memcpy(data+packet_size, The_mission.mission_desc, desc_len);
			packet_size += desc_len;
		} else {
			ADD_STRING(The_mission.mission_desc);
		}
	} 
	
	Assert(addr != NULL);
	if(addr != NULL){
		psnet_send(addr, data, packet_size);
	}
}

// process an incoming netgame description packet
void process_netgame_descript_packet( ubyte *data, header *hinfo )
{
	int offset,state;
	ubyte code;	
	char mission_desc[MISSION_DESC_LENGTH+2];
	net_addr addr;

	fill_net_addr(&addr, hinfo->addr, hinfo->net_id, hinfo->port);

	// read this game into a temporary structure
	offset = HEADER_LENGTH;
	GET_DATA(code);	
	
	// if this is a request for mission description
	if(code == 0){
		if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
			PACKET_SET_SIZE();
			return;
		}		

		// send an update to this guy
		send_netgame_descript_packet(&addr, 1);
	} else {	
		memset(mission_desc,0,MISSION_DESC_LENGTH+2);		
		GET_STRING(mission_desc);

		// only display if we're in the proper state
		state = gameseq_get_state();
		switch(state){
		case GS_STATE_MULTI_JOIN_GAME:
		case GS_STATE_MULTI_CLIENT_SETUP:			
		case GS_STATE_MULTI_HOST_SETUP:
			multi_common_set_text(mission_desc);
			break;
		}
	}

	PACKET_SET_SIZE();	
}

// broadcast a query for active games. IPX will use net broadcast and TCP will either request from the MT or from the specified list
void broadcast_game_query()
{
	int packet_size;
	net_addr addr;	
	server_item *s_moveup;
	ubyte data[MAX_PACKET_SIZE];

	if ( MULTI_IS_TRACKER_GAME && (Multi_options_g.protocol == NET_TCP) ) {
		fs2netd_send_game_request();
		return;
	}

	BUILD_HEADER(GAME_QUERY);	
	
	// go through the server list and query each of those as well
	s_moveup = Game_server_head;
	if(s_moveup != NULL){
		do {				
			send_server_query(&s_moveup->server_addr);			
			s_moveup = s_moveup->next;					
		} while(s_moveup != Game_server_head);		
	}	

	fill_net_addr(&addr, Psnet_my_addr.addr, Psnet_my_addr.net_id, DEFAULT_GAME_PORT);

	// send out a broadcast if our options allow us
	if(Net_player->p_info.options.flags & MLO_FLAG_LOCAL_BROADCAST){
		psnet_broadcast( &addr, data, packet_size);
	}		
}

// send an individual query to an address to see if there is an active game
void send_server_query(net_addr *addr)
{
	int packet_size;	
	ubyte data[MAX_PACKET_SIZE];

	// build the header and send the data
	BUILD_HEADER(GAME_QUERY);				
	psnet_send(addr, data, packet_size);
}

// process a query from a client looking for active freespace games
void process_game_query(ubyte* data, header* hinfo)
{
	int offset;	
	net_addr addr;

	offset = HEADER_LENGTH;

	PACKET_SET_SIZE();

	// check to be sure that we don't capture our own broadcast message
	fill_net_addr(&addr, hinfo->addr, hinfo->net_id, hinfo->port);
	if ( psnet_same( &addr, &Psnet_my_addr) ){
		return;
	}

	// if I am not a server of a game, don't send a reply!!!
	if ( !(Net_player->flags & NETINFO_FLAG_AM_MASTER) ){
		return;
	}

	// if the game options are being selected, then ignore the request
	// also, if Netgame.max_players == -1, the host has not chosen a mission yet and we should wait
	if((Netgame.game_state == NETGAME_STATE_STD_HOST_SETUP) || (Netgame.game_state == NETGAME_STATE_HOST_SETUP) || (Netgame.game_state == 0) || (Netgame.max_players == -1)){
		return;
	}

	// send information about this active game
	send_game_active_packet(&addr);
}

// sends information about netplayers in the game. if called on the server, broadcasts information about _all_ players
void send_netplayer_update_packet( net_player *pl )
{
	int packet_size,idx;
	ubyte data[MAX_PACKET_SIZE],val;

	BUILD_HEADER(NETPLAYER_UPDATE);

	// if I'm the server of the game, I should send an update for _all_players in the game
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		for(idx=0;idx<MAX_PLAYERS;idx++){
			// only send info for connected players
			if(MULTI_CONNECTED(Net_players[idx])){
				// add a stop byte
				val = 0x0;
				ADD_DATA(val);

				// add the net player's information
				ADD_SHORT(Net_players[idx].player_id);
				ADD_INT(Net_players[idx].state);
				ADD_INT(Net_players[idx].p_info.ship_class);				
				ADD_INT(Net_players[idx].tracker_player_id);

				if(Net_players[idx].flags & NETINFO_FLAG_HAS_CD){
					val = 1;
				} else {
					val = 0;
				}
				ADD_DATA(val);				
			}
		}
		// add the final stop byte
		val = 0xff;
		ADD_DATA(val);

		// broadcast the packet
		if(!(Game_mode & GM_IN_MISSION)){
			if ( pl == NULL ) {
				multi_io_send_to_all_reliable(data, packet_size);				
			} else {
				multi_io_send_reliable(pl, data, packet_size);
			}
		} else {
			if ( pl == NULL ) {
				multi_io_send_to_all(data, packet_size);
			} else {				
				multi_io_send(pl, data, packet_size);
			}
		}
	} else {
		// add a stop byte
		val = 0x0;
		ADD_DATA(val);

		// add my current state in the netgame to this packet
		ADD_SHORT(Net_player->player_id);
		ADD_INT(Net_player->state);
		ADD_INT(Net_player->p_info.ship_class);		
		ADD_INT(Multi_tracker_id);

		// add if I have a CD or not
		if(Multi_has_cd){
			val = 1;
		} else {
			val = 0;
		}
		ADD_DATA(val);		

		// add a final stop byte
		val = 0xff;
		ADD_DATA(val);

		// send the packet to the server
		Assert( pl == NULL );						// shouldn't ever be the case that pl is non-null here.
		if(!(Game_mode & GM_IN_MISSION)){			
			multi_io_send_reliable(Net_player, data, packet_size);
		} else {			
			multi_io_send(Net_player, data, packet_size);
		}
	}	
}

// process an incoming netplayer state update. if we're the server, we should rebroadcast
void process_netplayer_update_packet( ubyte *data, header *hinfo )
{
	int offset, player_num;
	net_player bogus;
	ubyte stop, has_cd;
	short player_id;
	int new_state;
	
	offset = HEADER_LENGTH;

	// get the first stop byte
	GET_DATA(stop);
	player_num = -1;
	while(stop != 0xff){
		// look the player up
		GET_SHORT(player_id);
		player_num = find_player_id(player_id);
		// if we couldn't find him, read in the bogus data
		if((player_num == -1) || (Net_player == &Net_players[player_num])){
			GET_INT(bogus.state);
			GET_INT(bogus.p_info.ship_class);			
			GET_INT(bogus.tracker_player_id);

			GET_DATA(has_cd);			
		} 
		// otherwise read in the data correctly
		else {
			GET_INT(new_state);
			GET_INT(Net_players[player_num].p_info.ship_class);			
			GET_INT(Net_players[player_num].tracker_player_id);
			GET_DATA(has_cd);
			if(has_cd){
				Net_players[player_num].flags |= NETINFO_FLAG_HAS_CD;
			} else {
				Net_players[player_num].flags &= ~(NETINFO_FLAG_HAS_CD);
			}			

			// if he's changing state to joined, send a team update
			if((Net_players[player_num].state == NETPLAYER_STATE_JOINING) && (new_state == NETPLAYER_STATE_JOINED) && (Netgame.type_flags & NG_TYPE_TEAM)){
				multi_team_send_update();
			}

			// set state
			Net_players[player_num].state = new_state;
		}

		// get the next stop byte
		GET_DATA(stop);
	}

	PACKET_SET_SIZE();	

	// if I'm the host or the server of the game, update everyone else so things are synched up as tightly as possible
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		send_netplayer_update_packet(NULL);
	}

	// if i'm the standalone and this is an update from the host, maybe change some netgame settings
	if((Game_mode & GM_STANDALONE_SERVER) && (player_num != -1) && (Net_players[player_num].flags & NETINFO_FLAG_GAME_HOST)){
		switch(Net_players[player_num].state){
		case NETPLAYER_STATE_STD_HOST_SETUP:
			Netgame.game_state = NETGAME_STATE_STD_HOST_SETUP;
			break;
		
		case NETPLAYER_STATE_HOST_SETUP:
			// check for race conditions
			if(Netgame.game_state != NETGAME_STATE_MISSION_SYNC){
				Netgame.game_state = NETGAME_STATE_FORMING;
			}
			break;		
		}
	}
}

#define EXTRA_DEATH_VAPORIZED		(1<<0)
#define EXTRA_DEATH_WASHED			(1<<1)
// send a packet indicating a ship has been killed
void send_ship_kill_packet( object *objp, object *other_objp, float percent_killed, int self_destruct )
{
	int packet_size, model;
	ubyte data[MAX_PACKET_SIZE], was_player, extra_death_info, vaporized;
	ushort debris_signature;
	ubyte sd;
	polymodel * pm;

	// only sendable from the master
	Assert ( Net_player->flags & NETINFO_FLAG_AM_MASTER );

	// special deaths
	vaporized = ( (Ships[objp->instance].flags & SF_VAPORIZE) > 0 );

	extra_death_info = 0; 
	if ( vaporized ) {
		extra_death_info |= EXTRA_DEATH_VAPORIZED;
	}

	if ( Ships[objp->instance].wash_killed ) {
		extra_death_info |= EXTRA_DEATH_WASHED;
	}

	// find out the next network signature that will be used for the debris pieces.
	model = Ship_info[Ships[objp->instance].ship_info_index].model_num;
	pm = model_get(model);
	debris_signature = 0;
	if ( pm && !vaporized ) {
		debris_signature = multi_get_next_network_signature( MULTI_SIG_DEBRIS );
		multi_set_network_signature( (ushort)(debris_signature + pm->num_debris_objects), MULTI_SIG_DEBRIS );
		Ships[objp->instance].arrival_distance = debris_signature;
	}

	BUILD_HEADER(SHIP_KILL);
	ADD_USHORT(objp->net_signature);

	// ships which are initially killed get the rest of the data sent.  self destructed ships and
	if ( other_objp == NULL ) {
		ushort temp;

		temp = 0;
		ADD_USHORT(temp);
		nprintf(("Network","Don't know other_obj for ship kill packet, sending NULL\n"));
	} else {
		ADD_USHORT( other_objp->net_signature );
	}

	ADD_USHORT( debris_signature );
	ADD_FLOAT( percent_killed );
	sd = (ubyte)self_destruct;
	ADD_DATA(sd);
	ADD_DATA( extra_death_info );

	// if the ship who died is a player, then send some extra info, like who killed him, etc.
	was_player = 0;
	if ( objp->flags & OF_PLAYER_SHIP ) {
		int pnum;
		char temp;
		short temp2;

		pnum = multi_find_player_by_object( objp );
		if ( pnum != -1 ) {
			was_player = 1;
			ADD_DATA( was_player );

			Assert(Net_players[pnum].m_player->killer_objtype < CHAR_MAX); 
			temp = (char)Net_players[pnum].m_player->killer_objtype;
			ADD_DATA( temp );

			Assert(Net_players[pnum].m_player->killer_species < CHAR_MAX); 
			temp = (char)Net_players[pnum].m_player->killer_species;
			ADD_DATA( temp );

			Assert(Net_players[pnum].m_player->killer_weapon_index < SHRT_MAX); 
			temp2 = (short)Net_players[pnum].m_player->killer_weapon_index;
			ADD_SHORT( temp2 );

			ADD_STRING( Net_players[pnum].m_player->killer_parent_name );
		} else {
			ADD_DATA( was_player );
		}
	} else {
		ADD_DATA( was_player );
	}

	// send the packet reliably!!!
	multi_io_send_to_all_reliable(data, packet_size);	
}

// process a packet indicating that a ship has been killed
void process_ship_kill_packet( ubyte *data, header *hinfo )
{
	int offset;
	ushort ship_sig, other_sig, debris_sig;
	object *sobjp, *oobjp;
	float percent_killed;	
	ubyte was_player, extra_death_info, sd;
	char killer_name[NAME_LENGTH], killer_objtype = OBJ_NONE, killer_species = 0;
	short killer_weapon_index = -1;

	offset = HEADER_LENGTH;
	GET_USHORT(ship_sig);

	GET_USHORT( other_sig );
	GET_USHORT( debris_sig );
	GET_FLOAT( percent_killed );
	GET_DATA( sd );
	GET_DATA( extra_death_info );
	GET_DATA( was_player );


	// pnum is >=0 when the dying ship is a pleyer ship.  Get the info about how he died
	if ( was_player != 0 ) {
		GET_DATA( killer_objtype );
		GET_DATA( killer_species );
		GET_SHORT( killer_weapon_index );
		GET_STRING( killer_name );
	}

	PACKET_SET_SIZE();

	sobjp = multi_get_network_object( ship_sig );

	// if I am unable to find the ship object which was killed, I have to bail and rely on getting
	// another message from the server that this happened!
	if ( sobjp == NULL ) {
		nprintf(("Network", "Couldn't find net signature %d for kill packet\n", ship_sig));		
		return;
	}

	// set this ship's hull value to 0
	sobjp->hull_strength = 0.0f;

	// maybe set vaporized
	if (extra_death_info & EXTRA_DEATH_VAPORIZED) {
		Ships[sobjp->instance].flags |= SF_VAPORIZE;
	}

	// maybe set wash_killed
	if (extra_death_info & EXTRA_DEATH_VAPORIZED) {
		Ships[sobjp->instance].wash_killed = 1;
	}

	oobjp = multi_get_network_object( other_sig );

	if ( was_player != 0 ) {
		int pnum;

		pnum = multi_find_player_by_object( sobjp );
		if ( pnum != -1 ) {
			Net_players[pnum].m_player->killer_objtype = killer_objtype;
			Net_players[pnum].m_player->killer_species = killer_species;
			Net_players[pnum].m_player->killer_weapon_index = killer_weapon_index;
			strcpy_s( Net_players[pnum].m_player->killer_parent_name, killer_name );
		}
	}	   

	// check to see if I need to respawn myself
	multi_respawn_check(sobjp);

	// store the debris signature in the arrival distance which will never get used for player ships
	Ships[sobjp->instance].arrival_distance = debris_sig;

	// set this bit so that we don't accidentally start switching targets when we die
	if(sobjp == Player_obj){
		Game_mode |= GM_DEAD_DIED;
	}

	mprintf(("Network Killing off %s\n", Ships[sobjp->instance].ship_name));

	// do the normal thing when not ingame joining.  When ingame joining, simply kill off the ship.
	if ( !(Net_player->flags & NETINFO_FLAG_INGAME_JOIN) ) {
		ship_hit_kill( sobjp, oobjp, percent_killed, sd );
	} else {
		sobjp->flags |= OF_SHOULD_BE_DEAD;
		ship_cleanup(sobjp->instance, SHIP_DESTROYED);
		obj_delete( OBJ_INDEX(sobjp) );
	}
}

// send a packet indicating a ship should be created
void send_ship_create_packet( object *objp, int is_support )
{
	int packet_size;
	ubyte data[MAX_PACKET_SIZE];

	// We will pass the ship to create by name.
	BUILD_HEADER(SHIP_CREATE);
	ADD_USHORT(objp->net_signature);
	ADD_INT( is_support );
	if ( is_support ){
		ADD_VECTOR( objp->pos );
	}

	// broadcast the packet	
	multi_io_send_to_all_reliable(data, packet_size);
}

// process a packet indicating a ship should be created
void process_ship_create_packet( ubyte *data, header *hinfo )
{
	int offset, objnum, is_support;
	ushort signature;
	p_object *objp;
	vec3d pos = ZERO_VECTOR;

	Assert ( !(Net_player->flags & NETINFO_FLAG_AM_MASTER) );
	offset = HEADER_LENGTH;
	GET_USHORT(signature);
	GET_INT( is_support );
	if ( is_support ){
		GET_VECTOR( pos );
	}

	PACKET_SET_SIZE();

	// find the name of this ship on ship ship arrival list.  if found, pass it to parse_object_create
	if ( !is_support ) {
		objp = mission_parse_get_arrival_ship( signature );
		if ( objp != NULL ) {
			objnum = parse_create_object(objp);
		} else {
			nprintf(("Network", "Ship with sig %d not found on ship arrival list -- not creating!!\n", signature));
		}
	} else {
		Assert( Arriving_support_ship );
		if(Arriving_support_ship == NULL){
			return;
		}
		Arriving_support_ship->pos = pos;
		Arriving_support_ship->net_signature = signature;
		objnum = parse_create_object( Arriving_support_ship );
		Assert( objnum != -1 );
		if(objnum >= 0){
			mission_parse_support_arrived( objnum );
		}
	}
}

// send a packet indicating a wing of ships should be created
void send_wing_create_packet( wing *wingp, int num_to_create, int pre_create_count )
{
	int packet_size, index, ship_instance;
	ubyte data[MAX_PACKET_SIZE];
	ushort signature;
	int val;

	// for creating wing -- we just send the index into the wing array of this wing.
	// all players load the same mission, and so their array's should all match. We also
	// need to send the signature of the first ship that was created.  We can find this by
	// looking num_to_create places back in the ship_index field in the wing structure.

	index = WING_INDEX(wingp);
	ship_instance = wingp->ship_index[wingp->current_count - num_to_create];
	signature = Objects[Ships[ship_instance].objnum].net_signature;

	BUILD_HEADER( WING_CREATE );
	ADD_INT(index);
	ADD_INT(num_to_create);
	ADD_USHORT(signature);		
	ADD_INT(pre_create_count);
	val = wingp->current_wave - 1;
	ADD_INT(val);
	
	multi_io_send_to_all_reliable(data, packet_size);	
}

// process a packet saying that a wing should be created
void process_wing_create_packet( ubyte *data, header *hinfo )
{
	int offset, index, num_to_create;
	ushort signature;
	int total_arrived_count, current_wave;

	offset = HEADER_LENGTH;
	GET_INT(index);
	GET_INT(num_to_create);
	GET_USHORT(signature);	
	GET_INT(total_arrived_count);
	GET_INT(current_wave);

	PACKET_SET_SIZE();

	// do a sanity check on the wing to be sure that we are actually working on a valid wing
	if ( (index < 0) || (index >= Num_wings) || (Wings[index].num_waves == -1) ) {
		nprintf(("Network", "invalid index %d for wing create packet\n"));
		return;
	}
	if ( (num_to_create <= 0) || (num_to_create > Wings[index].wave_count) ) {
		nprintf(("Network", "Invalid number of ships to create (%d) for wing %s\n", num_to_create, Wings[index].name));
		return;
	}

	// bash some info
	Wings[index].current_count = 0;
	Wings[index].total_arrived_count = total_arrived_count;
	Wings[index].current_wave = current_wave;

	// set the network signature that was passed.  The client should create ships in the same order
	// as the server -- so all ships should get the same sigs as assigned by the server.  We also
	// need to set some timestamps and cues correctly to be sure that these things get created on
	// the clients correctly
	multi_set_network_signature( signature, MULTI_SIG_SHIP );
	parse_wing_create_ships( &Wings[index], num_to_create, 1 );
}

// packet indicating a ship is departing
void send_ship_depart_packet( object *objp, int method )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;
	ushort signature;

	signature = objp->net_signature;

	BUILD_HEADER(SHIP_DEPART);
	ADD_USHORT( signature );
	ADD_SHORT( (short) method); 
	
	multi_io_send_to_all_reliable(data, packet_size);
}

// process a packet indicating a ship is departing
void process_ship_depart_packet( ubyte *data, header *hinfo )
{
	int offset;
	object *objp;
	ushort signature;
	short s_method; 

	offset = HEADER_LENGTH;
	GET_USHORT( signature );
	GET_SHORT(s_method); 
	PACKET_SET_SIZE();

	// find the object which is departing
	objp = multi_get_network_object( signature );
	if ( objp == NULL ) {
		nprintf(("network", "Couldn't find object with net signature %d to depart\n", signature ));
		return;
	}

	switch (s_method) {
		case SHIP_DEPARTED_BAY:
		case SHIP_VANISHED:
			if (objp->type == OBJ_SHIP) {
				ship_actually_depart(objp->instance, s_method); 
			}
			else {
				nprintf(("network", "Can not proces ship depart packed. Object with net signature %d is not a ship!\n", signature ));	
				return;
			}
			break;

		// assume standard warp out
		default: 
		// start warping him out
		shipfx_warpout_start( objp );
	}
}

// packet to tell clients cargo of a ship was revealed to all
void send_cargo_revealed_packet( ship *shipp )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;

	// build the header and add the data
	BUILD_HEADER(CARGO_REVEALED);
	ADD_USHORT( Objects[shipp->objnum].net_signature );

	// server sends to all players
	if(MULTIPLAYER_MASTER){		
		multi_io_send_to_all_reliable(data, packet_size);
	} 
	// clients just send to the server
	else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// process a cargo revealed packet
void process_cargo_revealed_packet( ubyte *data, header *hinfo )
{
	int offset;
	ushort signature;
	object *objp;

	offset = HEADER_LENGTH;
	GET_USHORT(signature);
	PACKET_SET_SIZE();

	// get a ship pointer and call the ship function to reveal the cargo
	objp = multi_get_network_object( signature );
	if ( objp == NULL ) {
		nprintf(("Network", "Could not find object with net signature %d for cargo revealed\n", signature ));
		return;
	}

	// Assert( objp->type == OBJ_SHIP );
	if((objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return;
	}

	// this will take care of re-routing to all other clients
	ship_do_cargo_revealed( &Ships[objp->instance], 1);	

	// server should rebroadcast
	if(MULTIPLAYER_MASTER){
		send_cargo_revealed_packet(&Ships[objp->instance]);
	}
}

// packet to tell clients cargo of a ship was hidden to all
void send_cargo_hidden_packet( ship *shipp )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;

	// build the header and add the data
	BUILD_HEADER(CARGO_HIDDEN);
	ADD_USHORT( Objects[shipp->objnum].net_signature );

	// server sends to all players
	if(MULTIPLAYER_MASTER){		
		multi_io_send_to_all_reliable(data, packet_size);
	} 
	// clients just send to the server
	else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// process a cargo hidden packet
void process_cargo_hidden_packet( ubyte *data, header *hinfo )
{
	int offset;
	ushort signature;
	object *objp;

	offset = HEADER_LENGTH;
	GET_USHORT(signature);
	PACKET_SET_SIZE();

	// get a ship pointer and call the ship function to hide the cargo
	objp = multi_get_network_object( signature );
	if ( objp == NULL ) {
		nprintf(("Network", "Could not find object with net signature %d for cargo hidden\n", signature ));
		return;
	}

	// Assert( objp->type == OBJ_SHIP );
	if((objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return;
	}

	// this will take care of re-routing to all other clients
	ship_do_cargo_hidden( &Ships[objp->instance], 1);	

	// server should rebroadcast
	if(MULTIPLAYER_MASTER){
		send_cargo_hidden_packet(&Ships[objp->instance]);
	}
}

// defines used for secondary fire packet
#define SFPF_ALLOW_SWARM		(1<<7)
#define SFPF_DUAL_FIRE			(1<<6)
#define SFPF_TARGET_LOCKED		(1<<5)

// send a packet indicating a secondary weapon was fired
void send_secondary_fired_packet( ship *shipp, ushort starting_sig, int starting_count, int num_fired, int allow_swarm )
{
	int packet_size, net_player_num;
	ubyte data[MAX_PACKET_SIZE], sinfo, current_bank;
	object *objp;
	ushort target_signature;
	char t_subsys;
	ai_info *aip;

	// Assert ( starting_count < UCHAR_MAX );

	// get the object for this ship.  If it is an AI object, send all the info to all player.  Otherwise,
	// we might send the info to the other player different than the one who fired
	objp = &Objects[shipp->objnum];
	if ( !(objp->flags & OF_PLAYER_SHIP) ) {
		if ( num_fired == 0 ) {
			return;
		}
	}

	aip = &Ai_info[shipp->ai_index];

	current_bank = (ubyte)shipp->weapons.current_secondary_bank;
	Assert( (current_bank < MAX_SHIP_SECONDARY_BANKS) );

	// build up the header portion
	BUILD_HEADER( SECONDARY_FIRED_AI );

	ADD_USHORT( Objects[shipp->objnum].net_signature );
	ADD_USHORT( starting_sig );
	
	// add a couple of bits for swarm missiles and dual fire secondary weapons
	sinfo = current_bank;

	if ( allow_swarm ){
		sinfo |= SFPF_ALLOW_SWARM;
	}

	if ( shipp->flags & SF_SECONDARY_DUAL_FIRE ){
		sinfo |= SFPF_DUAL_FIRE;
	}

	if ( aip->current_target_is_locked ){
		sinfo |= SFPF_TARGET_LOCKED;
	}

	ADD_DATA( sinfo );

	// add the ship's target and any targeted subsystem
	target_signature = 0;
	t_subsys = -1;
	if ( aip->target_objnum != -1) {
		target_signature = Objects[aip->target_objnum].net_signature;
		if ( (Objects[aip->target_objnum].type == OBJ_SHIP) && (aip->targeted_subsys != NULL) ) {
			int s_index;

			s_index = ship_get_index_from_subsys( aip->targeted_subsys, aip->target_objnum );
			Assert( s_index < CHAR_MAX );			// better be less than this!!!!
			t_subsys = (char)s_index;
		}

		if ( Objects[aip->target_objnum].type == OBJ_WEAPON ) {
			Assert(Weapon_info[Weapons[Objects[aip->target_objnum].instance].weapon_info_index].wi_flags & WIF_BOMB);
		}

	}

	ADD_USHORT( target_signature );
	ADD_DATA( t_subsys );

	// just send this packet to everyone, then bail if an AI ship fired.
	if ( !(objp->flags & OF_PLAYER_SHIP) ) {		
		multi_io_send_to_all(data, packet_size);
		return;
	}

	net_player_num = multi_find_player_by_object( objp );

	// getting here means a player fired.  Send the current packet to all players except the player
	// who fired.  If nothing got fired, then don't send to the other players -- we will just send
	// a packet to the player who will find out that he didn't fire anything
	if ( num_fired > 0 ) {
		multi_io_send_to_all_reliable(data, packet_size, &Net_players[net_player_num]);		
	}

	// if I (the master) fired, then return
	if ( Net_players[net_player_num].flags & NETINFO_FLAG_AM_MASTER ){
		return;
	}

	// now build up the packet to send to the player who actually fired.
	BUILD_HEADER( SECONDARY_FIRED_PLR );
	ADD_USHORT(starting_sig);
	ADD_DATA( sinfo );

	// add the targeting information so that the player's weapons will always home on the correct
	// ship
	ADD_USHORT( target_signature );
	ADD_DATA( t_subsys );
	
	multi_io_send_reliable(&Net_players[net_player_num], data, packet_size);
}

/// process a packet indicating a secondary weapon was fired
void process_secondary_fired_packet(ubyte* data, header* hinfo, int from_player)
{
	int offset, allow_swarm, target_objnum_save;
	ushort net_signature, starting_sig, target_signature;
	ubyte sinfo, current_bank;
	object* objp, *target_objp;
	ship *shipp;
	char t_subsys;
	ai_info *aip;
	ship_subsys *targeted_subsys_save;

	offset = HEADER_LENGTH;	// size of the header

	// if from_player is false, it means that the secondary weapon info in this packet was
	// fired by an ai object (or another player).  from_player == 1 means tha me (the person
	// receiving this packet) fired the secondary weapon
	if ( !from_player ) {
		GET_USHORT( net_signature );
		GET_USHORT( starting_sig );
		GET_DATA( sinfo );			// are we firing swarm missiles

		GET_USHORT( target_signature );
		GET_DATA( t_subsys );

		PACKET_SET_SIZE();

		// find the object (based on network signatures) for the object that fired
		objp = multi_get_network_object( net_signature );
		if ( objp == NULL ) {
			nprintf(("Network", "Could not find ship for fire secondary packet!"));
			return;
		}

		// set up the ships current secondary bank and that bank's mode.  Below, we will set the timeout
		// of the next fire time of this bank to 0 so we can fire right away
		shipp = &Ships[objp->instance];

	} else {
		GET_USHORT( starting_sig );
		GET_DATA( sinfo );

		GET_USHORT( target_signature );
		GET_DATA( t_subsys );

		PACKET_SET_SIZE();

		// get the object and ship
		objp = Player_obj;
		shipp = Player_ship;
	}

	// check the allow swarm bit
	allow_swarm = 0;
	if ( sinfo & SFPF_ALLOW_SWARM ){
		allow_swarm = 1;
	}

	// set the dual fire properties of the ship
	if ( sinfo & SFPF_DUAL_FIRE ){
		shipp->flags |= SF_SECONDARY_DUAL_FIRE;
	} else {
		shipp->flags &= ~SF_SECONDARY_DUAL_FIRE;
	}

	// determine whether current target is locked
	Assert( shipp->ai_index != -1 );
	aip = &Ai_info[shipp->ai_index];
	if ( sinfo & SFPF_TARGET_LOCKED ) {
		aip->current_target_is_locked = 1;
	} else {
		aip->current_target_is_locked = 0;
	}

	// find out the current bank
	current_bank = (ubyte)(sinfo & 0x3);
	Assert( (current_bank < MAX_SHIP_SECONDARY_BANKS) );
	shipp->weapons.current_secondary_bank = current_bank;

	// make it so we can fire this ship's secondary bank immediately!!!
	shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank] = timestamp(0);
	shipp->weapons.detonate_weapon_time = timestamp(5000);		// be sure that we don't detonate a remote weapon before it is time.

	// set this ship's target and subsystem information.  We will save and restore target and
	// targeted subsystem so that we do not accidentally change targets for this player or
	// any AI ships on his system.
	target_objnum_save = aip->target_objnum;
	targeted_subsys_save = aip->targeted_subsys;

	// reset these variables for accuracy.  They will get reassigned at the end of this fuction
	aip->target_objnum = -1;
	aip->targeted_subsys = NULL;

	target_objp = multi_get_network_object( target_signature );
	if ( target_objp != NULL ) {
		aip->target_objnum = OBJ_INDEX(target_objp);

		if ( (t_subsys != -1) && (target_objp->type == OBJ_SHIP) ) {
			aip->targeted_subsys = ship_get_indexed_subsys( &Ships[target_objp->instance], t_subsys);
		}
	}

	if ( starting_sig != 0 ){
		multi_set_network_signature( starting_sig, MULTI_SIG_NON_PERMANENT );
	} else {
		shipp->weapons.detonate_weapon_time = timestamp(0);		// signature of -1 say detonate remote weapon
	}

	ship_fire_secondary( objp, allow_swarm );

	// restore targeted object and targeted subsystem
	aip->target_objnum = target_objnum_save;
	aip->targeted_subsys = targeted_subsys_save;
}

// send a packet indicating a countermeasure was fired
void send_countermeasure_fired_packet( object *objp, int cmeasure_count, int rand_val )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;

	Int3();

	Assert ( cmeasure_count < UCHAR_MAX );
	BUILD_HEADER(COUNTERMEASURE_FIRED);
	ADD_USHORT( objp->net_signature );
	ADD_INT( rand_val );
		
	multi_io_send_to_all(data, packet_size);
}

// process a packet indicating a countermeasure was fired
void process_countermeasure_fired_packet( ubyte *data, header *hinfo )
{
	int offset, rand_val;
	ushort signature;
	object *objp;

	Int3();

	offset = HEADER_LENGTH;	

	GET_USHORT( signature );
	GET_INT( rand_val );
	PACKET_SET_SIZE();

	objp = multi_get_network_object( signature );
	if ( objp == NULL ) {
		nprintf(("network", "Could find object whose countermeasures are being launched!!!\n"));
		return;
	}
	if(objp->type != OBJ_SHIP){
		return;
	}
	// Assert ( objp->type == OBJ_SHIP );

	// make it so ship can fire right away!
	Ships[objp->instance].cmeasure_fire_stamp = timestamp(0);
	if ( objp == Player_obj ){
		nprintf(("network", "firing countermeasure from my ship\n"));
	}

	ship_launch_countermeasure( objp, rand_val );		
}

// send a packet indicating that a turret has been fired
void send_turret_fired_packet( int ship_objnum, int subsys_index, int weapon_objnum )
{
	int packet_size;
	ushort pnet_signature;
	ubyte data[MAX_PACKET_SIZE], cindex;
	object *objp;
	ubyte has_sig = 0;
	ship_subsys *ssp;
	short val;

	// sanity
	if((weapon_objnum < 0) || (Objects[weapon_objnum].type != OBJ_WEAPON) || (Objects[weapon_objnum].instance < 0) || (Weapons[Objects[weapon_objnum].instance].weapon_info_index < 0)){
		return;
	}

	// local setup -- be sure we are actually passing a weapon!!!!
	objp = &Objects[weapon_objnum];
	Assert ( objp->type == OBJ_WEAPON );
	if(Weapon_info[Weapons[objp->instance].weapon_info_index].subtype == WP_MISSILE){
		has_sig = 1;
	}

	pnet_signature = Objects[ship_objnum].net_signature;

	Assert( subsys_index < UCHAR_MAX );
	cindex = (ubyte)subsys_index;

	ssp = ship_get_indexed_subsys( &Ships[Objects[ship_objnum].instance], subsys_index, NULL );
	if(ssp == NULL){
		return;
	}

	// build the fire turret packet.  
	BUILD_HEADER(FIRE_TURRET_WEAPON);	
	packet_size += multi_pack_unpack_position(1, data + packet_size, &objp->orient.vec.fvec);
	ADD_DATA( has_sig );
	ADD_USHORT( pnet_signature );	
	if(has_sig){		
		ADD_USHORT( objp->net_signature );
	}
	ADD_DATA( cindex );
	val = (short)ssp->submodel_info_1.angs.h;
	ADD_SHORT( val );
	val = (short)ssp->submodel_info_2.angs.p;
	ADD_SHORT( val );	
	
	multi_io_send_to_all(data, packet_size);

	multi_rate_add(1, "tur", packet_size);
}

// process a packet indicating a turret has been fired
void process_turret_fired_packet( ubyte *data, header *hinfo )
{
	int offset, weapon_objnum, wid = -1;
	ushort pnet_signature, wnet_signature;
	vec3d pos, temp;
	matrix orient;
	vec3d o_fvec;
	ubyte turret_index;
	object *objp;
	ship_subsys *ssp;
	ubyte has_sig = 0;
	ship *shipp;
	short pitch, heading;	

	// get the data for the turret fired packet
	offset = HEADER_LENGTH;	
	offset += multi_pack_unpack_position(0, data + offset, &o_fvec);	
	GET_DATA( has_sig );
	GET_USHORT( pnet_signature );
	if(has_sig){
		GET_USHORT( wnet_signature );
	} else {
		wnet_signature = 0;
	}
	GET_DATA( turret_index );
	GET_SHORT( heading );
	GET_SHORT( pitch );	
	PACKET_SET_SIZE();				// move our counter forward the number of bytes we have read

	// find the object
	objp = multi_get_network_object( pnet_signature );
	if ( objp == NULL ) {
		nprintf(("network", "could find parent object with net signature %d for turret firing\n", pnet_signature));
		return;
	}

	// if this isn't a ship, do nothing
	if ( objp->type != OBJ_SHIP ){
		return;
	}

	// make an orientation matrix from the o_fvec
	vm_vector_2_matrix(&orient, &o_fvec, NULL, NULL);

	// find this turret, and set the position of the turret that just fired to be where it fired.  Quite a
	// hack, but should be suitable.
	shipp = &Ships[objp->instance];
	ssp = ship_get_indexed_subsys( shipp, turret_index, NULL );
	if(ssp == NULL){
		return;
	}

	if (ssp->weapons.num_primary_banks > 0) {
		wid = ssp->weapons.primary_bank_weapons[0];
	} else if (ssp->weapons.num_secondary_banks > 0) {
		wid = ssp->weapons.secondary_bank_weapons[0];
	}

	if (wid < 0)
		return;

	// bash the position and orientation of the turret
	ssp->submodel_info_1.angs.h = (float)heading;
	ssp->submodel_info_2.angs.p = (float)pitch;

	// get the world position of the weapon
	ship_get_global_turret_info(objp, ssp->system_info, &pos, &temp);

	// create the weapon object
	if(wnet_signature != 0){		
		multi_set_network_signature( wnet_signature, MULTI_SIG_NON_PERMANENT );
	}

	weapon_objnum = weapon_create( &pos, &orient, wid, OBJ_INDEX(objp), -1, 1);
	if (weapon_objnum != -1) {
		if ( Weapon_info[wid].launch_snd != -1 ) {
			snd_play_3d( &Snds[Weapon_info[wid].launch_snd], &pos, &View_position );
		}		
	}
}

// send a mission log item packet
void send_mission_log_packet( int num )
{
	int packet_size;
	ubyte data[MAX_PACKET_SIZE];
	ubyte type;
	int sindex;
	log_entry *entry;

	Assert ( MULTIPLAYER_MASTER );

	// get the data from the log
	entry = &log_entries[num];
	type = (ubyte)entry->type;			// do the type casting thing to save on packet space
	sindex = entry->index;

	BUILD_HEADER(MISSION_LOG_ENTRY);
	ADD_DATA(type);
	ADD_INT(entry->flags);
	ADD_INT(sindex);
	ADD_INT(entry->timestamp); // NOTE: this is a long so careful with swapping in 64-bit platforms - taylor
	ADD_STRING(entry->pname);
	ADD_STRING(entry->sname);

	// broadcast the packet to all players	
	multi_io_send_to_all_reliable(data, packet_size);
}

// process a mission log item packet
void process_mission_log_packet( ubyte *data, header *hinfo )
{
	int offset, flags;
	int sindex;
	ubyte type;
	char pname[NAME_LENGTH], sname[NAME_LENGTH];
	fix timestamp;

	Assert ( MULTIPLAYER_CLIENT );

	offset = HEADER_LENGTH;
	GET_DATA(type);
	GET_INT(flags);
	GET_INT(sindex);
	GET_INT(timestamp); // NOTE: this is a long so careful with swapping in 64-bit platforms - taylor
	GET_STRING(pname);
	GET_STRING(sname);

	PACKET_SET_SIZE();

	mission_log_add_entry_multi( type, pname, sname, sindex, timestamp, flags );
}

// send a mission message packet
void send_mission_message_packet( int id, char *who_from, int priority, int timing, int source, int builtin_type, int multi_target, int multi_team_filter, int delay)
{
	int packet_size;
	ubyte data[MAX_PACKET_SIZE], up, us, utime;
	
	Assert ( Net_player->flags & NETINFO_FLAG_AM_MASTER );
	Assert ( (priority >= 0) && (priority < UCHAR_MAX) );
	Assert ( (timing >= 0) && (timing < UCHAR_MAX) );	
	
	up = (ubyte) priority;
	us = (ubyte) source;
	utime = (ubyte)timing;

	BUILD_HEADER(MISSION_MESSAGE);
	ADD_INT(id);
	ADD_STRING(who_from);
	ADD_DATA(up);
	ADD_DATA(utime);
	ADD_DATA(us);
	ADD_INT(builtin_type);
	ADD_INT(multi_team_filter);
	ADD_INT(delay);

	if (multi_target == -1){		
		multi_io_send_to_all_reliable(data, packet_size);
	} else {		
		multi_io_send_reliable(&Net_players[multi_target], data, packet_size);
	}
}

// process a mission message packet
void process_mission_message_packet( ubyte *data, header *hinfo )
{
	int offset, id, builtin_type, delay;
	ubyte priority, source, utiming;
	char who_from[NAME_LENGTH];
	int multi_team_filter;

	Assert( !(Net_player->flags & NETINFO_FLAG_AM_MASTER) );

	offset = HEADER_LENGTH;
	GET_INT(id);
	GET_STRING(who_from);
	GET_DATA(priority);
	GET_DATA(utiming);
	GET_DATA(source);
	GET_INT(builtin_type);
	GET_INT(multi_team_filter);
	GET_INT(delay);

	PACKET_SET_SIZE();

	// filter out some builtin ones in TvT
	if((builtin_type >= 0) && (Netgame.type_flags & NG_TYPE_TEAM) && (Net_player != NULL) && (Net_player->p_info.team != multi_team_filter)) {
		mprintf(("Builtin message of type %d filtered out in process_mission_message_packet()\n", id));
		return;
	}

	// maybe filter this out
	if(!message_filter_multi(id)){
		// send the message as if it came from an sexpression
		message_queue_message( id, priority, utiming, who_from, source, 0, delay, builtin_type );
	}
}

// just send them a pong back as fast as possible
void process_ping_packet(ubyte *data, header *hinfo)
{
   net_addr addr;
	int offset;

	offset = HEADER_LENGTH;
	PACKET_SET_SIZE();

	// get the address to return the pong to
	fill_net_addr(&addr, hinfo->addr, hinfo->net_id, hinfo->port);	
	            
	// send the pong
	send_pong(&addr);	
}

// right now it just routes the pong through to the standalone gui, which is the only
// system which uses ping and pong right now.
void process_pong_packet(ubyte *data, header *hinfo)
{
	net_player *p;
	net_addr addr;
	int offset,lookup;
	
	offset = HEADER_LENGTH;

	fill_net_addr(&addr, hinfo->addr, hinfo->net_id, hinfo->port);
		
	PACKET_SET_SIZE();	
		
	// if we're connected , see who sent us this pong
	if(Net_player->flags & NETINFO_FLAG_CONNECTED){
		lookup = find_player_id(hinfo->id);
		if(lookup == -1){
			return;
		}
		
		p = &Net_players[lookup]; 

		// evaluate the ping
		multi_ping_eval_pong(&Net_players[lookup].s_info.ping);
			
		// put in calls to any functions which may want to know about the ping times from 
		// this guy
		if (Game_mode & GM_STANDALONE_SERVER) {
		   std_update_player_ping(p);	
		}

		// mark his socket as still alive (extra precaution)
		psnet_mark_received(Net_players[lookup].reliable_socket);
	}
	// otherwise, do any special processing
	else {
		// if we're in the join game state, see if this pong came from a server on our
		// list
		if(gameseq_get_state() == GS_STATE_MULTI_JOIN_GAME){
			multi_join_eval_pong(&addr, timer_get_fixed_seconds());
		}
	}
}

// send a ping packet
void send_ping(net_addr *addr)
{
	unsigned char data[8];
	int packet_size;

	// build the header and send the packet
	BUILD_HEADER( PING );		
	psnet_send(addr, &data[0], packet_size);
}

// send a pong packet
void send_pong(net_addr *addr)
{
   unsigned char data[8];
	int packet_size;	

	// build the header and send the packet
	BUILD_HEADER(PONG);		
	psnet_send(addr, &data[0], packet_size);   
}

// sent from host to master. give me the list of missions you have.
// this will be used only in a standalone mode
void send_mission_list_request( int what )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;

	// build the header and ask for a list of missions or campaigns (depending
	// on the 'what' flag).
	BUILD_HEADER(MISSION_REQUEST);
		
	multi_io_send_reliable(Net_player, data, packet_size);
}

// maximum number of bytes that we can send in a mission items packet.
#define MAX_MISSION_ITEMS_BYTES	(MAX_PACKET_SIZE - (sizeof(multi_create_info) + 1) )

// defines used to tell what type of packets are being sent
#define MISSION_LIST_ITEMS			1
#define CAMPAIGN_LIST_ITEMS		2

// send an individual mission file item
void send_mission_items( net_player *pl )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size, i;
	ubyte stop, type;

	// build the header	
	BUILD_HEADER(MISSION_ITEM);	

	// send the list of missions and campaigns avilable on the server.  Stop when
	// reaching a certain maximum
	type = MISSION_LIST_ITEMS;
	ADD_DATA( type );
	for (i = 0; i < (int)Multi_create_mission_list.size(); i++ ) {		
		stop = 0;
		ADD_DATA( stop );

		ADD_STRING( Multi_create_mission_list[i].filename );
		ADD_STRING( Multi_create_mission_list[i].name );
		ADD_INT( Multi_create_mission_list[i].flags );
		ADD_DATA( Multi_create_mission_list[i].max_players );
		ADD_INT( Multi_create_mission_list[i].respawn );		

		// STANDALONE_ONLY		
		ADD_DATA( Multi_create_mission_list[i].valid_status );

		if ( packet_size > (int)MAX_MISSION_ITEMS_BYTES ) {
			stop = 1;
			ADD_DATA( stop );			
			multi_io_send_reliable(pl, data, packet_size);
			BUILD_HEADER( MISSION_ITEM );
			ADD_DATA( type );
		}		
	}
	stop = 1;
	ADD_DATA(stop);	
	multi_io_send_reliable(pl, data, packet_size);

	// send the campaign information
	type = CAMPAIGN_LIST_ITEMS;
	BUILD_HEADER(MISSION_ITEM);
	ADD_DATA( type );
	for (i = 0; i < (int)Multi_create_campaign_list.size(); i++ ) {		
		stop = 0;
		ADD_DATA( stop );

		ADD_STRING( Multi_create_campaign_list[i].filename );
		ADD_STRING( Multi_create_campaign_list[i].name );
		ADD_INT( Multi_create_campaign_list[i].flags );	
		ADD_DATA( Multi_create_campaign_list[i].max_players );		

		if ( packet_size > (int)MAX_MISSION_ITEMS_BYTES ) {
			stop = 1;
			ADD_DATA( stop );			
			multi_io_send_reliable(pl, data, packet_size);
			BUILD_HEADER( MISSION_ITEM );
			ADD_DATA( type );
		}		
	}
	stop = 1;
	ADD_DATA(stop);	
	multi_io_send_reliable(pl, data, packet_size);
}

// process a request for a list of missions
void process_mission_request_packet(ubyte *data, header *hinfo)
{   
	int player_num,offset;	
	
	offset = HEADER_LENGTH;
	PACKET_SET_SIZE();

	// fill in the address information of where this came from	
	player_num = find_player_id(hinfo->id);
	if(player_num == -1){
		nprintf(("Network","Could not find player to send mission list items to!\n"));
		return;
	}

	send_mission_items( &Net_players[player_num] );
}

// process an individual mission file item
void process_mission_item_packet(ubyte *data,header *hinfo)
{
   int offset, flags;		
	char filename[MAX_FILENAME_LEN], name[NAME_LENGTH], valid_status;
	ubyte stop, type,max_players;
	uint respawn;
	multi_create_info mcip;

	Assert(gameseq_get_state() == GS_STATE_MULTI_HOST_SETUP);
	offset = HEADER_LENGTH;

	GET_DATA( type );
	GET_DATA(stop);
	while( !stop ) {
		GET_STRING( filename );
		GET_STRING( name );
		GET_INT( flags );
		GET_DATA( max_players );

		// missions also have respawns and a crc32 associated with them
		if(type == MISSION_LIST_ITEMS){
			GET_UINT(respawn);

			// STANDALONE_ONLY			
			GET_DATA(valid_status);

			strcpy_s(mcip.filename, filename );
			strcpy_s(mcip.name, name );
			mcip.flags = flags;
			mcip.respawn = respawn;
			mcip.max_players = max_players;

			// STANDALONE_ONLY				
			mcip.valid_status = valid_status;

			Multi_create_mission_list.push_back( mcip );
		} else if ( type == CAMPAIGN_LIST_ITEMS ) {
			strcpy_s(mcip.filename, filename );
			strcpy_s(mcip.name, name );
			mcip.flags = flags;
			mcip.respawn = 0;
			mcip.max_players = max_players;

			Multi_create_campaign_list.push_back( mcip );
		}

		GET_DATA( stop );
	}
	
	PACKET_SET_SIZE();	

	// this will cause whatever list to get resorted (although they should be appearing in order)
	multi_create_setup_list_data(-1);		
}

// send a request to the server to pause or unpause the game
void send_multi_pause_packet(int pause)
{
	ubyte data[MAX_PACKET_SIZE];
	ubyte val;
	int packet_size = 0;
	
	Assert(!(Net_player->flags & NETINFO_FLAG_AM_MASTER));
	
	// build the header
	BUILD_HEADER(MULTI_PAUSE_REQUEST);
	val = (ubyte) pause;
	
	// add the pause info
	ADD_DATA(val);	

	// send the request to the server	
	multi_io_send_reliable(Net_player, data, packet_size);
}

// process a pause update packet (pause, unpause, etc)
void process_multi_pause_packet(ubyte *data, header *hinfo)
{
	int offset;
	ubyte val;		
	int player_index;

	offset = HEADER_LENGTH;

	// get the data
	GET_DATA(val);	
	PACKET_SET_SIZE();

	// get who sent the packet	
	player_index = find_player_id(hinfo->id);
	// if we don't know who sent the packet, don't do anything
	if(player_index == -1){
		return;
	}

	// if we're the server, we should evaluate whether this guy is allowed to send the packet
	multi_pause_server_eval_request(&Net_players[player_index],(int)val);	
}

// send a game information update
void send_game_info_packet()
{
	int packet_size;
	ubyte data[MAX_PACKET_SIZE], paused;

	// set the paused variable
	paused = (ubyte)((Netgame.game_state == NETGAME_STATE_PAUSED)?1:0);

	BUILD_HEADER(GAME_INFO);
	ADD_INT( Missiontime ); // NOTE: this is a long so careful with swapping in 64-bit platforms - taylor
	ADD_DATA( paused );
	
	multi_io_send_to_all(data, packet_size);
}

// process a game information update
void process_game_info_packet( ubyte *data, header *hinfo )
{
	int offset;
	fix mission_time;
	ubyte paused;

	offset = HEADER_LENGTH;

	// get the mission time -- we should examine our time and the time from the server.  If off by some delta
	// time, set our time to server time (should take ping time into account!!!)
	GET_INT( mission_time ); // NOTE: this is a long so careful with swapping in 64-bit platforms - taylor
	GET_DATA( paused );
	PACKET_SET_SIZE();	
}

// send an ingame nak packet
void send_ingame_nak(int state, net_player *p)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;
	packet_size = 0;
	BUILD_HEADER(INGAME_NAK);

	ADD_INT(state);
		
	multi_io_send_reliable(p, data, packet_size);
}

// process an ingame nak packet
void process_ingame_nak(ubyte *data, header *hinfo)
{
	int offset,state,pid;	
	net_player *pl;

	offset = HEADER_LENGTH;
	GET_INT(state);	
	PACKET_SET_SIZE();
	
	pid = find_player_id(hinfo->id);
	if(pid < 0){
		return;
	}
	pl = &Net_players[pid];
	
	switch(state){
	case ACK_FILE_ACCEPTED :
		Assert(Net_player->flags & NETINFO_FLAG_INGAME_JOIN);
		nprintf(("Network","Mission file rejected by server, aborting...\n"));
		multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_FILE_REJECTED);		
		break;
	}	
}

// If the end_mission SEXP has been used tell clients to skip straight to the debrief screen
void send_force_end_mission_packet()
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;
	
	packet_size = 0;
	BUILD_HEADER(FORCE_MISSION_END);	

	if (Net_player->flags & NETINFO_FLAG_AM_MASTER)
	{	
		// tell everyone to leave the game		
		multi_io_send_to_all_reliable(data, packet_size);
	}
}

// process a packet indicating that we should jump straight to the debrief screen
void process_force_end_mission_packet(ubyte *data, header *hinfo)
{
	int offset;	
			
	offset = HEADER_LENGTH;
 	
	PACKET_SET_SIZE();

	ml_string("Receiving force end mission packet");

	// Since only the server sends out these packets it should never receive one
	Assert (!(Net_player->flags & NETINFO_FLAG_AM_MASTER)); 
	
	multi_handle_sudden_mission_end();
	send_debrief_event();
}

// send a packet telling players to end the mission
void send_endgame_packet(net_player *pl)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;
	
	packet_size = 0;
	BUILD_HEADER(MISSION_END);	

	// sending to a specific player?
	if(pl != NULL){
		Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);
		multi_io_send_reliable(pl, data, packet_size);
		return;
	}

	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		// send all player stats here
		multi_broadcast_stats(STATS_MISSION);		

		// if in dogfight mode, send all dogfight stats as well
		ml_string("Before dogfight stats!");
		if(Netgame.type_flags & NG_TYPE_DOGFIGHT){
			ml_string("Sending dogfight stats!");

			multi_broadcast_stats(STATS_DOGFIGHT_KILLS);
		}
		ml_string("After dogfight stats!");

		// tell everyone to leave the game		
		multi_io_send_to_all_reliable(data, packet_size);
	} else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// process a packet indicating we should end the current mission
void process_endgame_packet(ubyte *data, header *hinfo)
{
	int offset;	
	int player_num;
			
	offset = HEADER_LENGTH;
 	
	PACKET_SET_SIZE();

	ml_string("Receiving endgame packet");
	
	// if I'm the server, I should evaluate whether the sender is authorized to end the game
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		// determine who this came from and make sure he is allowed to end the game		
		player_num = find_player_id(hinfo->id);
		Assert(player_num != -1);
		if(player_num < 0){
			return;
		}

		// if the player is allowed to end the mission
		if(!multi_can_end_mission(&Net_players[player_num])){
			return;
		}		

		// act as if we hit alt+j locally 
		multi_handle_end_mission_request();
	}
	// all clients process immediately
	else {
		// ingame joiners should quit when they receive an endgame packet since the game is over
		if(Net_player->flags & NETINFO_FLAG_INGAME_JOIN){
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_EARLY_END);
			return;
		}

		// do any special processing for being in a state other than the gameplay state
		multi_handle_state_special();

		// make sure we're not already in the debrief state
		if((gameseq_get_state() != GS_STATE_DEBRIEF) && (gameseq_get_state() != GS_STATE_MULTI_DOGFIGHT_DEBRIEF)){
			multi_warpout_all_players();			
		}
	}	
}

// send a position/orientation update for myself (if I'm an observer)
void send_observer_update_packet()
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;
	int ret;
	ushort target_sig;
	
	// its possible for the master to be an observer if has run out of respawns. In this case, he doesn't need
	// to send any update packets to anyone.
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		return;
	}

	if((Player_obj == NULL) || (Player_obj->type != OBJ_OBSERVER) || (Net_player == NULL) || !(Net_player->flags & NETINFO_FLAG_OBSERVER)){
		return;
	}

	packet_size = 0;
	
	BUILD_HEADER(OBSERVER_UPDATE);

	ret = multi_pack_unpack_position( 1, data + packet_size, &Player_obj->pos );
	packet_size += ret;
	ret = multi_pack_unpack_orient( 1, data + packet_size, &Player_obj->orient );
	packet_size += ret;

	// add targeting infomation
	if((Player_ai != NULL) && (Player_ai->target_objnum >= 0)){
		target_sig = Objects[Player_ai->target_objnum].net_signature;
	} else {
		target_sig = 0;
	}
	ADD_USHORT(target_sig);
	
	multi_io_send(Net_player, data, packet_size);
}

// process a position/orientation update from an observer
void process_observer_update_packet(ubyte *data, header *hinfo)
{
	int offset,ret;
	int obs_num;
	vec3d g_vec;
	matrix g_mat;
	physics_info bogus_pi;	
	ushort target_sig;
	object *target_obj;
	offset = HEADER_LENGTH;

	obs_num = find_player_id(hinfo->id);
	
	memset(&bogus_pi,0,sizeof(physics_info));
	ret = multi_pack_unpack_position( 0, data + offset, &g_vec );
	offset += ret;
	ret = multi_pack_unpack_orient( 0, data + offset, &g_mat );
	offset += ret;

	// targeting information
	GET_USHORT(target_sig);	
	PACKET_SET_SIZE();	

	if((obs_num < 0) || (Net_players[obs_num].m_player->objnum < 0)){
		return;
	}

	// set targeting info
	if(target_sig == 0){
		Net_players[obs_num].s_info.target_objnum = -1;
	} else {
		target_obj = multi_get_network_object(target_sig);
		Net_players[obs_num].s_info.target_objnum = (target_obj == NULL) ? -1 : OBJ_INDEX(target_obj);
	}

	Objects[Net_players[obs_num].m_player->objnum].pos = g_vec;
	Objects[Net_players[obs_num].m_player->objnum].orient = g_mat;
	Net_players[obs_num].s_info.eye_pos = g_vec;
	Net_players[obs_num].s_info.eye_orient = g_mat;
}

void send_netplayer_slot_packet()
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size,idx;
	ubyte stop;

	packet_size = 0;
	stop = 0xff;
   BUILD_HEADER(NETPLAYER_SLOTS_P);
   for(idx=0;idx<MAX_PLAYERS;idx++){
		if( MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx])){
			ADD_DATA(stop);
			ADD_SHORT(Net_players[idx].player_id);  
			ADD_USHORT(Objects[Net_players[idx].m_player->objnum].net_signature);
			ADD_INT(Net_players[idx].p_info.ship_class);
			ADD_INT(Net_players[idx].p_info.ship_index);			
		}
	}
	stop = 0x0;
	ADD_DATA(stop);
		
	// standalone case or not
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		multi_io_send_to_all_reliable(data, packet_size);
	} else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

void process_netplayer_slot_packet(ubyte *data, header *hinfo)
{
	int offset;
	int player_num,ship_class,ship_index;
	ushort net_sig;
	object *objp;	
	ubyte stop;
	short player_id;
	
	offset = HEADER_LENGTH;	

   // first untag all of the player ships and make them OF_COULD_BE_PLAYER
	multi_untag_player_ships();

	GET_DATA(stop);
	while(stop != 0x0){
		GET_SHORT(player_id);
		GET_USHORT(net_sig);
		GET_INT(ship_class);
		GET_INT(ship_index);
		player_num = find_player_id(player_id);
		if(player_num < 0){
			nprintf(("Network","Error looking up player for object/slot assignment!!\n"));
		} else {
			// call the function in multiutil.cpp to set up the player object stuff
			// being careful not to muck with the standalone object
			if(!((player_num == 0) && (Game_mode & GM_STANDALONE_SERVER))){
				objp = multi_get_network_object(net_sig);
				Assert(objp != NULL);
				multi_assign_player_ship( player_num, objp, ship_class );
				Net_players[player_num].p_info.ship_index = ship_index;
				objp->flags &= ~(OF_COULD_BE_PLAYER);
				objp->flags |= OF_PLAYER_SHIP;
			}
		}
		GET_DATA(stop);
	}
	PACKET_SET_SIZE();

	// standalone should forward the packet and wait for a response
	if(Game_mode & GM_STANDALONE_SERVER){
		send_netplayer_slot_packet();
	} 

	Net_player->state = NETPLAYER_STATE_SLOT_ACK;
	send_netplayer_update_packet();	
}

// two functions to deal with ships changing their primary/secondary weapon status.  'what' indicates
// if this change is a primary or secondary change.  new_bank is the new current primary/secondary
// bank, link_status is whether primaries are linked or not, or secondaries are dual fire or not
void send_ship_weapon_change( ship *shipp, int what, int new_bank, int link_status )
{
	ubyte data[MAX_PACKET_SIZE], utmp;
	int packet_size;

	BUILD_HEADER(SHIP_WSTATE_CHANGE);
	ADD_USHORT( Objects[shipp->objnum].net_signature );
	utmp = (ubyte)(what);
	ADD_DATA( utmp );
	utmp = (ubyte)(new_bank);
	ADD_DATA( utmp );
	utmp = (ubyte)(link_status);
	ADD_DATA( utmp );

	// Removed the above psnet_send() call - it didn't appear to do anything since it was called only from the server anyway - DB	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_ship_weapon_change( ubyte *data, header *hinfo )
{
	int offset;
	ushort signature;
	ubyte what, new_bank, link_status;
	object *objp;
	ship *shipp;

	offset = HEADER_LENGTH;
	GET_USHORT( signature );
	GET_DATA( what );
	GET_DATA( new_bank );
	GET_DATA( link_status );
	PACKET_SET_SIZE();

	objp = multi_get_network_object( signature );
	if ( objp == NULL ) {
		nprintf(("network", "Unable to locate ship with signature %d for weapon state change\n", signature));
		return;
	}
	// Assert( objp->type == OBJ_SHIP );
	if(objp->type != OBJ_SHIP){
		return;
	}

	// if this is my data, do nothing since I already have my own data
	if ( objp == Player_obj ){
		return;
	}

	// now, get the ship and set the new bank and link modes based on the 'what' value
	shipp = &Ships[objp->instance];
	if ( what == MULTI_PRIMARY_CHANGED ) {
		shipp->weapons.current_primary_bank = new_bank;
		if ( link_status ){
			shipp->flags |= SF_PRIMARY_LINKED;
		} else {
			shipp->flags &= ~SF_PRIMARY_LINKED;
		}
	} else {
		shipp->weapons.current_secondary_bank = new_bank;
		if ( link_status ){
			shipp->flags |= SF_SECONDARY_DUAL_FIRE;
		} else {
			shipp->flags &= ~SF_SECONDARY_DUAL_FIRE;
		}
	}
}
	
	// ship status change procedure
// 1.) <client> - Client runs through the normal button_function procedure. Any remaining control bits are implied as being
//                server critical.
// 2.) <client> - Client puts this button_info item into his last_buttons array and sends a bunch of SHIP_STATUS packets 
//                for added redundancy.
// 3.) <server> - Receives the packet. Checks to see if the net_player on his side already has this one defined. If so, it
//                ignores as a repeat packet. Otherwise it puts it in the last_buttons array for the net_player
// 4.) <server> - Server applies the command on his side (with multi_apply_ship_status(...) and sends the ack (also a SHIP_STATUS) 
//                back to the client. Also sends multiple times for redundancy
// 5.) <client> - Receives the packet back. Does a lookup into his last_buttons array. If he finds the match, apply the functions
//                and remove the item from the list. If no match is found it means that either he has received an ack, has acted
//                on it and removed it, or that it has been "timed out" and replaced by a newer button_info.

#define SHIP_STATUS_REPEAT 2
void send_ship_status_packet(net_player *pl, button_info *bi, int id)
{
	int idx, temp;
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	if(pl == NULL){
		return;
	}

	BUILD_HEADER(SHIP_STATUS_CHANGE);
	ADD_INT(id);
	for(idx=0;idx<NUM_BUTTON_FIELDS;idx++){
		temp = bi->status[idx];
		ADD_INT(temp);
	}
	
   // server should send reliably (response packet)
	if(MULTIPLAYER_MASTER){		
		multi_io_send_reliable(pl, data, packet_size);
	} else {
		multi_io_send(pl, data, packet_size);
	}
}

void process_ship_status_packet(ubyte *data, header *hinfo)
{
   int idx;
	int offset;
	int player_num,unique_id;
	button_info bi;
	int i_tmp;
	
	offset = HEADER_LENGTH;

	// zero out the button info structure for good measure
	memset(&bi,0,sizeof(button_info));
	
	// read the button-info
	GET_INT(unique_id);	
		
	for(idx=0;idx<NUM_BUTTON_FIELDS;idx++){
		GET_INT(i_tmp);
		bi.status[idx] = i_tmp;
	}

	PACKET_SET_SIZE();

   // this will be handled differently client and server side. Duh.
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){                  // SERVER SIDE
		// find which net-player has sent us butotn information		
		player_num = find_player_id(hinfo->id);
		Assert(player_num >= 0);
		if(player_num < 0){
			return;
		}

		// don't process critical button information for observers
		// its a new button_info for this guy. apply and ack
		if(!MULTI_OBSERVER(Net_players[player_num]) && !lookup_ship_status(&Net_players[player_num],unique_id)){        
			// mark that he's pressed this button
			// add_net_button_info(&Net_players[player_num], &bi, unique_id);

			// send a return packet
			send_ship_status_packet(&Net_players[player_num], &bi,unique_id);
			
			// apply the button presses to his ship as normal
			multi_apply_ship_status(&Net_players[player_num], &bi, 0);         
		} 
		// else ignore it as a repeat from the same guy
	} else {                                                         // CLIENT SIDE
		// this is the return from the server, so we should now apply them locally
	//	if(lookup_ship_status(Net_player,unique_id,1)){
		multi_apply_ship_status(Net_player, &bi, 1);
	//	}
	}	
}

// MWA 4/28/9 -- redid this function since message all fighers was really broken
// for clients.  Left all details to this function instead of higher level messaging
// code
void send_player_order_packet(int type, int index, int cmd)
{
	ubyte data[MAX_PACKET_SIZE];
	ubyte val;
	ushort target_signature;
	char t_subsys;
	int packet_size = 0;

	BUILD_HEADER(PLAYER_ORDER_PACKET);

	val = (ubyte)type;
	ADD_DATA(val);         // ship order or wing order, or message all fighters

	// if we are not messaging all ships or wings, add the index, which is the shipnum or wingnum
	if ( val != SQUAD_MSG_ALL ){
		ADD_INT(index);  // net signature of target ship
	}

	ADD_INT(cmd);         // the command itself

	// add target data.
	target_signature = 0;
	if ( Player_ai->target_objnum != -1 ){
		target_signature = Objects[Player_ai->target_objnum].net_signature;
	}

	ADD_USHORT( target_signature );

	t_subsys = -1;
	if ( (Player_ai->target_objnum != -1) && (Player_ai->targeted_subsys != NULL) ) {
		int s_index;

		s_index = ship_get_index_from_subsys( Player_ai->targeted_subsys, Player_ai->target_objnum );
		Assert( s_index < CHAR_MAX );			// better be less than this!!!!
		t_subsys = (char)s_index;
	}
	ADD_DATA(t_subsys);
   
	multi_io_send_reliable(Net_player, data, packet_size);
}

// brief explanation :
// in either case (wing or ship command), we need to send in a pseudo-ai object. Basically, both command handler
// functions "normally" (non multiplayer) use a couple of the Player_ai fields. So, we just fill in the ones necessary
// (which we can reconstruct from the packet data), and pass this as the default variable ai_info *local
// Its kind of a hack, but it eliminates the need to go in and screw around with quite a bit of code
void process_player_order_packet(ubyte *data, header *hinfo)
{
	int offset, player_num, command, index = 0, tobjnum_save;	
	ushort target_signature;
	char t_subsys, type;
	object *objp, *target_objp;
	ai_info *aip;
	ship *shipp;
	ship_subsys *tsubsys_save, *targeted_subsys;

	Assert(MULTIPLAYER_MASTER);

	// packet values - its easier to read all of these in first
		
	offset = HEADER_LENGTH;
	
	GET_DATA( type );
	if ( type != SQUAD_MSG_ALL ){
		GET_INT( index );
	}

	GET_INT( command );
	GET_USHORT( target_signature );
	GET_DATA( t_subsys );

	PACKET_SET_SIZE();	

	player_num = find_player_id(hinfo->id);
	if(player_num == -1){
		nprintf(("Network","Received player order packet from unknown player\n"));		
		return;
	}	

	objp = &Objects[Net_players[player_num].m_player->objnum];
	if ( objp->type != OBJ_SHIP ) {
		nprintf(("Network", "not doing player order because object requestting is not a ship\n"));
		return;
	}

	// HACK HACK HACK HACK HACK HACK
	// if the player has sent a rearm-repair me message, we should bail here after evaluating it, since most likely the rest of
	// the data is BOGUS.  All people should be able to to these things as well.
	if(command == REARM_REPAIR_ME_ITEM){ 
		hud_squadmsg_repair_rearm(0,&Objects[Net_players[player_num].m_player->objnum]);		
		return;
	} else if(command == ABORT_REARM_REPAIR_ITEM){
		hud_squadmsg_repair_rearm_abort(0,&Objects[Net_players[player_num].m_player->objnum]);		
		return;
	}

	// if this player is not allowed to do messaging, quit here
	if( !multi_can_message(&Net_players[player_num]) ){
		nprintf(("Network","Recieved player order packet from player not allowed to give orders!!\n"));
		return;
	}

	// check to see if the type of order is a reinforcement call.  If so, intercept it, and
	// then call them in.
	if ( type == SQUAD_MSG_REINFORCEMENT ) {
		Assert( (index >= 0) && (index < Num_reinforcements) );
		hud_squadmsg_call_reinforcement(index, player_num);
		return;
	}

	// set the player's ai information here
	shipp = &Ships[objp->instance];
	aip = &Ai_info[shipp->ai_index];

	// get the target objnum and targeted subsystem.  Quick out if we don't have an object to act on.
	target_objp = multi_get_network_object( target_signature );
	if ( target_objp == NULL ) {
		return;
	}

	targeted_subsys = NULL;
	if ( t_subsys != -1 ) {
		Assert( target_objp != NULL );
		targeted_subsys = ship_get_indexed_subsys( &Ships[target_objp->instance], t_subsys);
	}

	// save and restore the target objnum and targeted subsystem so that we don't mess up other things
	// here
	tobjnum_save = aip->target_objnum;
	tsubsys_save = aip->targeted_subsys;

	if ( target_objp ) {
		aip->target_objnum = OBJ_INDEX(target_objp);
	} else {
		aip->target_objnum = -1;
	}

	aip->targeted_subsys = targeted_subsys;

	if ( type == SQUAD_MSG_SHIP ) {
		hud_squadmsg_send_ship_command(index, command, 1, SQUADMSG_HISTORY_ADD_ENTRY, player_num);
	} else if ( type == SQUAD_MSG_WING ) {
		hud_squadmsg_send_wing_command(index, command, 1, SQUADMSG_HISTORY_ADD_ENTRY, player_num);
	} else if ( type == SQUAD_MSG_ALL ) {
		hud_squadmsg_send_to_all_fighters( command, player_num );
	}

	Assert(tobjnum_save != Ships[aip->shipnum].objnum);	//	make sure not targeting self
	aip->target_objnum = tobjnum_save;
	aip->targeted_subsys = tsubsys_save;
}

// FILE SIGNATURE stuff :
// there are 2 cases for file signature sending which are handled very differently
// 1.) Pregame. In this case, the host requires that all clients send a filesig packet (when process_file_sig() is called, it
//     posts an ACK_FILE_ACCEPTED packet to ack_evaluate, so he thinks they have acked). 
// 2.) Ingame join. In this case, the client sends his filesig packet automatically to the server and the _client_ waits for
//     the ack, before continuing to join. It would be way too messy to have the server wait on the clients ack, since he
//     would have to keep track of up to potentially 14 other ack handles  (ouch).
void send_file_sig_packet(ushort sum_sig,int length_sig)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	BUILD_HEADER(FILE_SIG_INFO);
	ADD_USHORT(sum_sig);
	ADD_INT(length_sig);
		
	multi_io_send_reliable(Net_player, data, packet_size);
}

void process_file_sig_packet(ubyte *data, header *hinfo)
{
	int offset;
	int length_sig;
	ushort sum_sig;	
	offset = HEADER_LENGTH;

	// should only be received on the server-side
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);	
	
	GET_USHORT(sum_sig);
	GET_INT(length_sig);
	PACKET_SET_SIZE();
	server_verify_filesig(hinfo->id, sum_sig, length_sig);	
}

void send_file_sig_request(char *file_name)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	BUILD_HEADER(FILE_SIG_REQUEST);
	ADD_STRING(file_name);

	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);
		
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_file_sig_request(ubyte *data, header *hinfo)
{		
	int offset = HEADER_LENGTH;	

	// get the mission name
	GET_STRING(Netgame.mission_name);	 
	PACKET_SET_SIZE();	

	// set the current mission filename
	strcpy_s(Game_current_mission_filename,Netgame.mission_name);

	// get the checksum
	multi_get_mission_checksum(Game_current_mission_filename);	

	if(!multi_endgame_ending()){	
		// reply to the server
		send_file_sig_packet(Multi_current_file_checksum,Multi_current_file_length);
	}
}

// functions to deal with subsystems getting whacked
void send_subsystem_destroyed_packet( ship *shipp, int index, vec3d world_hitpos )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;
	ubyte uindex;
	vec3d tmp, local_hitpos;
	object *objp;

	Assert ( index < UCHAR_MAX );
	uindex = (ubyte)(index);

	objp = &Objects[shipp->objnum];

	vm_vec_sub(&tmp, &world_hitpos, &objp->pos );
	vm_vec_rotate( &local_hitpos, &tmp, &objp->orient );

	BUILD_HEADER(SUBSYSTEM_DESTROYED);
	ADD_USHORT( Objects[shipp->objnum].net_signature );
	ADD_DATA( uindex );
	ADD_VECTOR( local_hitpos );
	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_subsystem_destroyed_packet( ubyte *data, header *hinfo )
{
	int offset;
	ushort signature;
	ubyte uindex;
	object *objp;
	vec3d local_hit_pos, world_hit_pos;

	offset = HEADER_LENGTH;

	GET_USHORT( signature );
	GET_DATA( uindex );
	GET_VECTOR( local_hit_pos );

	// get the network object.  process it if we find it.
	objp = multi_get_network_object( signature );
	if ( objp != NULL ) {
		ship *shipp;
		ship_subsys *subsysp;

		// be sure we have a ship!!!
		// Assert ( objp->type == OBJ_SHIP );
		if(objp->type != OBJ_SHIP){
			PACKET_SET_SIZE();
			return;
		}

		shipp = &Ships[objp->instance];

		// call to get the pointer to the subsystem we should be working on
		subsysp = ship_get_indexed_subsys( shipp, (int)uindex );
		vm_vec_unrotate( &world_hit_pos, &local_hit_pos, &objp->orient );
		vm_vec_add2( &world_hit_pos, &objp->pos );

		do_subobj_destroyed_stuff( shipp, subsysp, &world_hit_pos );
		if ( objp == Player_obj ) {
			hud_gauge_popup_start(HUD_DAMAGE_GAUGE, 5000);
		}
	}

	PACKET_SET_SIZE();
}


// packet to tell clients cargo of a ship was revealed to all
void send_subsystem_cargo_revealed_packet( ship *shipp, int index )
{
	ubyte data[MAX_PACKET_SIZE], uindex;
	int packet_size;

	Assert ( index < UCHAR_MAX );
	uindex = (ubyte)(index);

	// build the header and add the data
	BUILD_HEADER(SUBSYS_CARGO_REVEALED);
	ADD_USHORT( Objects[shipp->objnum].net_signature );
	ADD_DATA( uindex );

	// server sends to all players
	if(MULTIPLAYER_MASTER){		
		multi_io_send_to_all_reliable(data, packet_size);
	} 
	// clients just send to the server
	else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// process a subsystem cargo revealed packet
void process_subsystem_cargo_revealed_packet( ubyte *data, header *hinfo )
{
	int offset;
	ushort signature;
	ubyte uindex;
	object *objp;
	ship *shipp;
	ship_subsys *subsysp;

	offset = HEADER_LENGTH;
	GET_USHORT( signature );
	GET_DATA( uindex );
	PACKET_SET_SIZE();

	// get a ship pointer and call the ship function to reveal the cargo
	objp = multi_get_network_object( signature );
	if ( objp == NULL ) {
		nprintf(("Network", "Could not find object with net signature %d for cargo revealed\n", signature ));
		return;
	}

	// Assert( objp->type == OBJ_SHIP );
	if((objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return;
	}

	shipp = &Ships[objp->instance];

	// call to get the pointer to the subsystem we should be working on
	subsysp = ship_get_indexed_subsys( shipp, (int)uindex );
	if (subsysp == NULL) {
		nprintf(("Network", "Could not find subsys for ship %s for cargo revealed\n", Ships[objp->instance].ship_name ));
		return;
	}

	// this will take care of re-routing to all other clients
	ship_do_cap_subsys_cargo_revealed( shipp, subsysp, 1 );

	// server should rebroadcast
	if(MULTIPLAYER_MASTER){
		send_subsystem_cargo_revealed_packet(&Ships[objp->instance], (int)uindex);
	}
}

// packet to tell clients cargo of a ship was hidden to all
void send_subsystem_cargo_hidden_packet( ship *shipp, int index )
{
	ubyte data[MAX_PACKET_SIZE], uindex;
	int packet_size;

	Assert ( index < UCHAR_MAX );
	uindex = (ubyte)(index);

	// build the header and add the data
	BUILD_HEADER(SUBSYS_CARGO_HIDDEN);
	ADD_USHORT( Objects[shipp->objnum].net_signature );
	ADD_DATA( uindex );

	// server sends to all players
	if(MULTIPLAYER_MASTER){		
		multi_io_send_to_all_reliable(data, packet_size);
	} 
	// clients just send to the server
	else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// process a subsystem cargo hidden packet
void process_subsystem_cargo_hidden_packet( ubyte *data, header *hinfo )
{
	int offset;
	ushort signature;
	ubyte uindex;
	object *objp;
	ship *shipp;
	ship_subsys *subsysp;

	offset = HEADER_LENGTH;
	GET_USHORT( signature );
	GET_DATA( uindex );
	PACKET_SET_SIZE();

	// get a ship pointer and call the ship function to reveal the cargo
	objp = multi_get_network_object( signature );
	if ( objp == NULL ) {
		nprintf(("Network", "Could not find object with net signature %d for cargo hidden\n", signature ));
		return;
	}

	// Assert( objp->type == OBJ_SHIP );
	if((objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return;
	}

	shipp = &Ships[objp->instance];

	// call to get the pointer to the subsystem we should be working on
	subsysp = ship_get_indexed_subsys( shipp, (int)uindex );
	if (subsysp == NULL) {
		nprintf(("Network", "Could not find subsys for ship %s for cargo hidden\n", Ships[objp->instance].ship_name ));
		return;
	}

	// this will take care of re-routing to all other clients
	ship_do_cap_subsys_cargo_hidden( shipp, subsysp, 1 );

	// server should rebroadcast
	if(MULTIPLAYER_MASTER){
		send_subsystem_cargo_hidden_packet(&Ships[objp->instance], (int)uindex);
	}
}

void send_netplayer_load_packet(net_player *pl)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	BUILD_HEADER(LOAD_MISSION_NOW);
	ADD_STRING(Netgame.mission_name);

	if(pl == NULL){		
		multi_io_send_to_all_reliable(data, packet_size);
	} else {
		multi_io_send_reliable(pl, data, packet_size);
	}
}

void process_netplayer_load_packet(ubyte *data, header *hinfo)
{
	char str[100];
	int offset = HEADER_LENGTH;	

	GET_STRING(str);
	PACKET_SET_SIZE();

	strcpy_s(Netgame.mission_name,str);
	strcpy_s(Game_current_mission_filename,str);
	if(!Multi_mission_loaded){

		// MWA 2/3/98 -- ingame join changes!!!
		// everyone can go through the same mission loading path here!!!!
		nprintf(("Network","Loading mission..."));

		// notify everyone that I'm loading the mission
		Net_player->state = NETPLAYER_STATE_MISSION_LOADING;
		send_netplayer_update_packet();		

		// do the load itself
		game_start_mission();		

		// ingame joiners need to "untag" all player ships as could_be_players.  The ingame joining
		// code will remark the correct player ships
		if ( Net_player->flags & NETINFO_FLAG_INGAME_JOIN ) {
			multi_untag_player_ships();
		}

		Net_player->flags |= NETINFO_FLAG_MISSION_OK;		
		Net_player->state = NETPLAYER_STATE_MISSION_LOADED;
		send_netplayer_update_packet();

		Multi_mission_loaded = 1;
		nprintf(("Network","Finished loading mission\n"));		
	}	
}

void send_jump_into_mission_packet(net_player *pl)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);

	BUILD_HEADER(JUMP_INTO_GAME);

	// ingame joiners will get special data.  We need to tell them about the state of the mission, like paused,
	// and possible other things.
	if ( pl != NULL ) {
		if ( pl->flags & NETINFO_FLAG_INGAME_JOIN ) {
			ADD_INT(Netgame.game_state);
		}
	}
	
	// broadcast
	if(pl == NULL){		
		multi_io_send_to_all_reliable(data, packet_size);
	}
	// send to a specific player
	else {
		multi_io_send_reliable(pl, data, packet_size);
	}
}

void process_jump_into_mission_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	int state;	
	
	state = 0;	

	// if I am ingame joining, there should be extra data.  For now, this data is the netgame state.
	// the game could be paused, so ingame joiner needs to deal with it.
	if ( Net_player->flags & NETINFO_FLAG_INGAME_JOIN ) {
		GET_INT( state );
		Netgame.game_state = state;
	}

	PACKET_SET_SIZE();	

	// handle any special processing for being in a weird substate
	multi_handle_state_special();

	// if I'm an ingame joiner, go to the ship select screen, or if I'm an observer, jump right in!
	if(Net_player->flags & NETINFO_FLAG_INGAME_JOIN){
		if(Net_player->flags & NETINFO_FLAG_OBSERVER){						
			multi_ingame_observer_finish();			
		} else {
			gameseq_post_event(GS_EVENT_INGAME_PRE_JOIN);
			Net_player->state = NETPLAYER_STATE_INGAME_SHIP_SELECT;
			send_netplayer_update_packet();
		}
	} else {
		// start the mission!!
		if(!(Game_mode & GM_IN_MISSION) && !(Game_mode & GM_STANDALONE_SERVER)){
			Netgame.game_state = NETGAME_STATE_IN_MISSION;
			gameseq_post_event(GS_EVENT_ENTER_GAME);
			Net_player->state = NETPLAYER_STATE_IN_MISSION;
			send_netplayer_update_packet();
		}		
	}

	//extern int Player_multi_died_check;
	//Player_multi_died_check = -1;

	// recalc all object pairs now	
	extern void obj_reset_all_collisions();
	obj_reset_all_collisions();

	// display some cool text
	multi_common_add_text(XSTR("Received mission start\n",720),1);

	// NETLOG
	ml_string(NOX("Client received mission start from server - entering mission"));
}

//XSTR:OFF

char *repair_text[] = {
	"unknown",
	"REPAIR_INFO_BEGIN",
	"REPAIR_INFO_END",
	"REPAIR_INFO_UPDATE",
	"REPAIR_INFO_QUEUE",
	"REPAIR_INFO_ABORT",
	"REPAIR_INFO_BROKEN",
	"REPAIR_INFO_WARP_ADD",
	"REPAIR_INFO_WARP_REMOVE",
	"REPAIR_INFO_ONWAY",
	"REPAIR_INFO_KILLED",
	"REPAIR_INFO_COMPLETE",
};

//XSTR:ON

// the following two routines deal with updating and sending information regarding players
// rearming and repairing during the game.  The process function calls the routines to deal with
// setting flags and other interesting things.
void send_repair_info_packet(object *repaired_objp, object *repair_objp, int code )
{
	int packet_size = 0;
	ushort repaired_signature, repair_signature;
	ubyte data[MAX_PACKET_SIZE];
	ubyte cd;

	// use the network signature of the destination object if there is one, -1 otherwise.
	// client will piece it all together
	repaired_signature = repaired_objp->net_signature;

	// the repair ship may be NULL here since it might have been destroyed
	repair_signature = 0;
	if ( repair_objp ){
		repair_signature = repair_objp->net_signature;
	}

	BUILD_HEADER(CLIENT_REPAIR_INFO);
	cd = (ubyte)code;
	ADD_DATA(cd);
	ADD_USHORT( repaired_signature );
	ADD_USHORT( repair_signature );
	
	multi_io_send_to_all_reliable(data, packet_size);

	nprintf(("Network", "Repair: %s sent to all players (%s/%s)\n", repair_text[cd], Ships[repaired_objp->instance].ship_name, (repair_objp==NULL)?"<none>":Ships[repair_objp->instance].ship_name));
}

void process_repair_info_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	ushort repaired_signature, repair_signature;
	object *repaired_objp, *repair_objp;
	ubyte code;

	GET_DATA(code);
	GET_USHORT( repaired_signature );
	GET_USHORT( repair_signature );
	PACKET_SET_SIZE();

	repaired_objp = multi_get_network_object( repaired_signature );
	repair_objp = multi_get_network_object( repair_signature );

	nprintf(("Network", "Repair: %s received (%s/%s)\n", repair_text[code], (repaired_objp==NULL)?"<None>":Ships[repaired_objp->instance].ship_name, (repair_objp==NULL)?"<None>":Ships[repair_objp->instance].ship_name));

	if ( Net_player->flags & NETINFO_FLAG_WARPING_OUT ){
		return;
	}

	if ( repaired_objp == NULL ) {
		Int3();				// Sandeep says this is bad bad bad.  No ship to repair.
		return;
	}

	// the hope is to simply call the routine in the ai code to set/unset flags
	// based on the code value and everything else should happen..I hope....
	if ( (code != REPAIR_INFO_WARP_ADD) && (code != REPAIR_INFO_WARP_REMOVE ) ) {

		ai_do_objects_repairing_stuff( repaired_objp, repair_objp, (int)code );

		// set the dock flags when repair begins.  Prevents problem in lagging docking
		// packet.  Also set any other flags/modes which need to be set to prevent Asserts.
		// bleah.
		if ( (code == REPAIR_INFO_BEGIN) && (repair_objp != NULL) ) {
// Karajorma removed this in revision 4808 to fix bug 1088.  Problem is, if
// this was originally intended to prevent docking problems, will they return?
/*
			// find indexes from goal
			ai_info *aip = &Ai_info[Ships[repair_objp->instance].ai_index];
			Assert(aip->active_goal >= 0);
			ai_goal *aigp = &aip->goals[aip->active_goal];
			Assert(aigp->flags & AIGF_DOCK_INDEXES_VALID);

			int docker_index = aigp->docker.index;
			int dockee_index = aigp->dockee.index;

			ai_do_objects_docked_stuff( repair_objp, docker_index, repaired_objp, dockee_index );
*/
			Ai_info[Ships[repair_objp->instance].ai_index].mode = AIM_DOCK;
		}

		// if the repair is done (either by abort, or ending), mark the repair ship's goal
		// as being done.
		if ( ((code == REPAIR_INFO_ABORT) || (code == REPAIR_INFO_END)) && repair_objp ){
			ai_mission_goal_complete( &Ai_info[Ships[repair_objp->instance].ai_index] );
		}
	} else {
		if ( code == REPAIR_INFO_WARP_ADD ){
			mission_bring_in_support_ship( repaired_objp );
		} else {
			mission_remove_scheduled_repair( repaired_objp );
		}
	}
}

// sends information updating clients on certain AI information that clients will
// need to know about to keep HUD information up to date.  objp is the object that we
// are updating, and what is the type of stuff that we are updating.
void send_ai_info_update_packet( object *objp, char what, object * other_objp )
{
	int packet_size;
	ushort other_signature;
	ubyte data[MAX_PACKET_SIZE];
	ai_info *aip;
	ai_goal *aigp = NULL;
	ubyte docker_index, dockee_index;

	// Assert( objp->type == OBJ_SHIP );
	if(objp->type != OBJ_SHIP){
		return;
	}
	aip = &Ai_info[Ships[objp->instance].ai_index];

	// do an out here
	if ( Ships[objp->instance].flags & (SF_DEPARTING | SF_DYING) )
		return;

	switch( what ) {

	case AI_UPDATE_DOCK:
	case AI_UPDATE_UNDOCK:
		Assert (other_objp != NULL); 
		if (other_objp == NULL) {
			return; 
		}
		break; 

	default: 
		Assert (other_objp == NULL); 
		break;
	}

	BUILD_HEADER( AI_INFO_UPDATE );
	ADD_USHORT( objp->net_signature );
	ADD_DATA( what );

	// depending on the "what" value, we will send different information
	// to the clients
	switch( what ) {

	case AI_UPDATE_DOCK:
		// for docking ships, add the signature of the ship that we are docked with.
		Assert( other_objp != NULL );
		other_signature = other_objp->net_signature;

		// Goober5000 - this is sort of weird, but it's the best way to do it
		docker_index = (ubyte) dock_find_dockpoint_used_by_object(objp, other_objp);
		dockee_index = (ubyte) dock_find_dockpoint_used_by_object(other_objp, objp);

		ADD_USHORT( other_signature );
		ADD_DATA( docker_index );
		ADD_DATA( dockee_index );
		break;

	case AI_UPDATE_UNDOCK:
		// same for undocking ships
		Assert( other_objp != NULL );
		other_signature = other_objp->net_signature;
		ADD_USHORT( other_signature );

		break;

	case AI_UPDATE_ORDERS: {
		int shipnum;

		// for orders, we only need to send a little bit of information here.  Be sure that the
		// first order for this ship is active
		Assert( (aip->active_goal != AI_GOAL_NONE) && (aip->active_goal != AI_ACTIVE_GOAL_DYNAMIC) );
		aigp = &aip->goals[aip->active_goal];

		ADD_INT( aigp->ai_mode );
		ADD_INT( aigp->ai_submode );

		shipnum = -1;
		if ( aigp->target_name != NULL )
			shipnum = ship_name_lookup( aigp->target_name );

		// the ship_name member of the goals structure may or may not contain a real shipname.  If we don't
		// have a valid shipnum, then don't sweat it since it may not really be a ship.
		if ( shipnum != -1 ) {
			Assert( Ships[shipnum].objnum != -1 );
			other_signature = Objects[Ships[shipnum].objnum].net_signature;
		} else
			other_signature = 0;
		
		ADD_USHORT( other_signature );

		// for docking, add the dock and dockee index
		if ( aigp->ai_mode & (AI_GOAL_DOCK|AI_GOAL_REARM_REPAIR) ) {
			Assert(aigp->flags & AIGF_DOCK_INDEXES_VALID);
			Assert( (aigp->docker.index >= 0) && (aigp->docker.index < UCHAR_MAX) );
			Assert( (aigp->dockee.index >= 0) && (aigp->dockee.index < UCHAR_MAX) );
			docker_index = (ubyte) aigp->docker.index;
			dockee_index = (ubyte) aigp->dockee.index;
			ADD_DATA( docker_index );
			ADD_DATA( dockee_index );
		}
		break;
		}

	default:
		Int3();
	}
	
	multi_rate_add(1, "aiu", packet_size);
	multi_io_send_to_all_reliable(data, packet_size);
}

// process an ai_info update packet.  Docking/undocking, ai orders, etc. are taken care of here.  This
// information is mainly used to keep the clients HUD up to date with the appropriate information.
void process_ai_info_update_packet( ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	int mode, submode;
	ushort net_signature, other_net_signature;
	object *objp, *other_objp;
	ai_info *aip;
	ai_goal *aigp = NULL;
	char code;
	ubyte docker_index = 0, dockee_index = 0;

	GET_USHORT( net_signature );		// signature of the object that we are dealing with.
	GET_DATA( code );					// code of what we are doing.
	objp = multi_get_network_object( net_signature );
	if ( !objp )
		nprintf(("Network", "Couldn't find object for ai update\n"));

	switch( code ) {
	case AI_UPDATE_DOCK:
		GET_USHORT( other_net_signature );
		GET_DATA( docker_index );
		GET_DATA( dockee_index );
		other_objp = multi_get_network_object( other_net_signature );
		if ( !other_objp )
			nprintf(("Network", "Couldn't find other object for ai update on dock\n"));
		
		// if we don't have an object to work with, break out of loop
		if ( !objp || !other_objp || (objp->type != OBJ_SHIP) || (other_objp->type != OBJ_SHIP)){
			break;
		}

		// don't assign the dock indexes, because they're part of the docking ship's goal code... just dock them
		// (and besides, we might be docking initially docked ships and we wouldn't have an active goal)
		ai_do_objects_docked_stuff( objp, docker_index, other_objp, dockee_index );
		break;

	case AI_UPDATE_UNDOCK:
		GET_USHORT( other_net_signature );
		other_objp = multi_get_network_object( other_net_signature );
		
		// if we don't have an object to work with, break out of loop
		if ( !objp )
			break;

		ai_do_objects_undocked_stuff( objp, other_objp );
		break;

	case AI_UPDATE_ORDERS:
		GET_INT( mode );
		GET_INT( submode );
		GET_USHORT( other_net_signature );
		if ( mode & (AI_GOAL_DOCK|AI_GOAL_REARM_REPAIR) ) {
			GET_DATA(docker_index);
			GET_DATA(dockee_index);
		}

		// be sure that we have a ship object!!!
		if ( !objp || (objp->type != OBJ_SHIP) )
			break;

		// set up the information in the first goal element of the object in question
		aip = &Ai_info[Ships[objp->instance].ai_index];
		aip->active_goal = 0;
		aigp = &aip->goals[aip->active_goal];
		aigp->ai_mode = mode;
		aigp->ai_submode = submode;

		// for docking, add the docker and dockee index to the active goal
		if ( mode & (AI_GOAL_DOCK|AI_GOAL_REARM_REPAIR) ) {
			aigp->docker.index = docker_index;
			aigp->dockee.index = dockee_index;
			aigp->flags |= AIGF_DOCK_INDEXES_VALID;
		}

		// get a shipname if we can.
		other_objp = multi_get_network_object( other_net_signature );
		if ( other_objp && (other_objp->type == OBJ_SHIP) ) {
			// get a pointer to the shipname in question.  Use the ship_name value in the
			// ship.  We are only using this for HUD display, so I think that using this
			// method will be fine.
			aigp->target_name = ai_get_goal_target_name(Ships[other_objp->instance].ship_name, &aigp->target_name_index);

			// special case for destroy subsystem -- get the ai_info pointer to our target ship
			// so that we can properly set up what subsystem this ship is attacking.
			if ( (mode == AI_GOAL_DESTROY_SUBSYSTEM ) && (submode >= 0) )
				aip->targeted_subsys = ship_get_indexed_subsys( &Ships[other_objp->instance], submode);
		}

		break;

	default:
		Int3();		// this Int3() should be temporary
		nprintf(("Network", "Invalid code for ai update: %d\n", code));
		break;
	}
	PACKET_SET_SIZE();
}

// tell the standalone to move into the MISSION_SYNC_STATE
void send_mission_sync_packet(int mode,int start_campaign)
{
	ubyte data[MAX_PACKET_SIZE],is_campaign;
	int packet_size = 0;

	Assert((Net_player->flags & NETINFO_FLAG_GAME_HOST) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER));

	// build the header and add the sync mode (pre or post briefing)
	BUILD_HEADER(MISSION_SYNC_DATA);
	ADD_INT(mode);

	// if this is a campaign game
	if(mode == MULTI_SYNC_PRE_BRIEFING){
		if(Game_mode & GM_CAMPAIGN_MODE){
			// add a byte indicating campaign mode
			is_campaign = 1;
			ADD_DATA(is_campaign);

			// add a byte indicating if we should be starting a campaign or continuing it
			is_campaign = (ubyte)start_campaign;
			ADD_DATA(is_campaign);

			// add the campaign filename
			ADD_STRING(Netgame.campaign_name);
		}
		// otherwise if this is a single mission 
		else {
			// add a byte indicating single mission mode
			is_campaign = 0;
			ADD_DATA(is_campaign);

			// add the mission filename
			ADD_STRING(Game_current_mission_filename);
		}
	}	
	multi_io_send_reliable(Net_player, data, packet_size);
}

// move into the MISSION_SYNC state when this is received
// this packet is sent only from a game host to a standalone
void process_mission_sync_packet(ubyte *data, header *hinfo)
{
	int mode;
	ubyte campaign_flag;
	int offset = HEADER_LENGTH;

	Assert(Game_mode & GM_STANDALONE_SERVER);

	// if this is a team vs team situation, lock the players send a final team update
	if(Netgame.type_flags & NG_TYPE_TEAM){
		multi_team_host_lock_all();
		multi_team_send_update();
	}

	// get the sync mode (pre or post briefing)
	GET_INT(mode);

	if(mode == MULTI_SYNC_PRE_BRIEFING){
		// get the flag indicating if this is a single mission or a campaign mode
		GET_DATA(campaign_flag);
		if(campaign_flag){
			// get the flag indicating whether we should be starting a new campaign
			GET_DATA(campaign_flag);

			// get the campaign filename
			GET_STRING(Netgame.campaign_name);

			// either start a new campaign or continue on to the next mission in the current campaign
			if(campaign_flag){
				multi_campaign_start(Netgame.campaign_name);
			} else {
				multi_campaign_next_mission();
			}
		} else {
			// make sure we remove the campaign mode flag
			Game_mode &= ~(GM_CAMPAIGN_MODE);

			// get the single mission filename
			GET_STRING(Game_current_mission_filename);
			strcpy_s(Netgame.mission_name,Game_current_mission_filename);
		}
	}
	else if (mode == MULTI_SYNC_POST_BRIEFING) {
		// process the initial orders now (moved from post_process_mission()in missionparse) 
		mission_parse_fixup_players();
		ai_post_process_mission();
	}
	PACKET_SET_SIZE();

	// set the correct mode and m ove into the state
	Multi_sync_mode = mode;
	gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);
}

// tell a player to merge his mission stats into his alltime stats
void send_store_stats_packet(int accept)
{
	ubyte data[10],val;
	int packet_size = 0;

	BUILD_HEADER(STORE_MISSION_STATS);

	// add whether we're accepting or tossing
	val = (ubyte)accept;
	ADD_DATA(val);

	// if I'm the server, send to everyone, else send to the standalone to be rebroadcasted
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		multi_io_send_to_all_reliable(data, packet_size);
	} else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

void process_store_stats_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	ubyte accept;

	GET_DATA(accept);
	PACKET_SET_SIZE();

	// if I'm the standalone, rebroadcast. Otherwise, if I'm a client, merge my mission stats with my alltime stats
	if(Game_mode & GM_STANDALONE_SERVER){
		// rebroadcast the packet to all others in the game
		nprintf(("Network","Standalone received store stats packet - rebroadcasting..\n"));				
		multi_io_send_to_all_reliable(data, offset);
	} else {			
		if(accept){
			// all players should mark the stats as being accepted in the debriefing
			multi_debrief_stats_accept();		
			

		} else {
			// all players should mark the stats as being "tossed" in the debriefing
			multi_debrief_stats_toss();			
		}			
	}
}

void send_debris_update_packet(object *objp,int code)
{
	ubyte data[MAX_PACKET_SIZE];
	ubyte val;
	int packet_size = 0;

	BUILD_HEADER(DEBRIS_UPDATE);	
	ADD_USHORT(objp->net_signature);
	val = (ubyte) code;
	ADD_DATA(val);
	
	// add any extra relevant data
	switch(code){
	case DEBRIS_UPDATE_UPDATE:
		ADD_VECTOR(objp->pos);						// add position
		ADD_ORIENT(objp->orient);				// add orientation
		ADD_VECTOR(objp->phys_info.vel);		// add velocity
		ADD_VECTOR(objp->phys_info.rotvel);	// add rotational velocity
		break;
	}	
	multi_io_send_to_all(data, packet_size);
}

void process_debris_update_packet(ubyte *data, header *hinfo)
{
	ushort net_sig;
	ubyte code;
	object bogus_object;
	object *objp;
	int offset = HEADER_LENGTH;
	
	GET_USHORT(net_sig);
	GET_DATA(code);

	objp = NULL;
	objp = multi_get_network_object(net_sig);
	if(objp == NULL){
		objp = &bogus_object;
	}

	switch((int)code){
	// update the object
	case DEBRIS_UPDATE_UPDATE:
		GET_VECTOR(objp->pos);
		GET_ORIENT(objp->orient);
		GET_VECTOR(objp->phys_info.vel);
		GET_VECTOR(objp->phys_info.rotvel);
		break;
	// simply remove it (no explosion)
	case DEBRIS_UPDATE_REMOVE:
		if(objp != &bogus_object){
			Assert(objp->type == OBJ_DEBRIS);
			obj_delete(OBJ_INDEX(objp));
		}
		break;
	// blow it up
	case DEBRIS_UPDATE_NUKE:
		if(objp != &bogus_object)
			debris_hit(objp,NULL,&objp->pos,1000000.0f);
		break;
	}

	PACKET_SET_SIZE();
}

// ALAN begin
void send_wss_request_packet(short player_id, int from_slot, int from_index, int to_slot, int to_index, int wl_ship_slot, int ship_class, int mode, net_player *p)
{
	ubyte data[MAX_PACKET_SIZE];

	int packet_size = 0;

	BUILD_HEADER(WSS_REQUEST_PACKET);
	
	// add the request information
	ADD_SHORT(player_id);
	ADD_INT(from_slot);
	ADD_INT(from_index);
	ADD_INT(to_slot);
	ADD_INT(to_index);
	ADD_INT(wl_ship_slot);	// only used in weapons loadout
	ADD_INT(ship_class);
	ADD_INT(mode);

	// a standard request
	if(p == NULL){		
		multi_io_send_reliable(Net_player, data, packet_size);
	} 
	// being routed through the standalone to the host of the game
	else {
		Assert(Game_mode & GM_STANDALONE_SERVER);			
		multi_io_send_reliable(p, data, packet_size);
	}
}

void process_wss_request_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	int from_slot,from_index;
	int to_slot,to_index;
	int mode;
	int wl_ship_slot,ship_class;
	short player_id;	
	int player_num;

	// determine who this request is from	
	GET_SHORT(player_id);	
	player_num = find_player_id(player_id);	

	// read in the request data	
	GET_INT(from_slot);
	GET_INT(from_index);
	GET_INT(to_slot);
	GET_INT(to_index);
	GET_INT(wl_ship_slot); // only used in weapons loadout
	GET_INT(ship_class);	// only used in multi team select
	GET_INT(mode);
	PACKET_SET_SIZE();

	Assert(player_num != -1);	
	if(player_num == -1){
		return;
	}

	// if we're the standalone, we have to route this packet to the host of the game
	if(Game_mode & GM_STANDALONE_SERVER){
		send_wss_request_packet(player_id, from_slot, from_index, to_slot, to_index, wl_ship_slot, ship_class, mode, Netgame.host);
	} 
	// otherwise we're the host and should process the request
	else {
		switch(mode){
		case WSS_WEAPON_SELECT :
			wl_drop(from_slot,from_index,to_slot,to_index,wl_ship_slot,player_num);
			break;
		case WSS_SHIP_SELECT :
			multi_ts_drop(from_slot,from_index,to_slot,to_index,ship_class,player_num);			
			break;
		default:
			Int3();
		}
	}
}

void send_wss_update_packet(int team_num,ubyte *wss_data,int size)
{
	ubyte data[MAX_PACKET_SIZE],team;
	int packet_size = 0;

	Assert(size <= (MAX_PACKET_SIZE - 10));

	BUILD_HEADER(WSS_UPDATE_PACKET);

	// add the team/pool # this is for
	team = (ubyte)team_num;
	ADD_DATA(team);

	// add the data block size
	ADD_INT(size);
	
	// add the data itself
	memcpy(data + packet_size,wss_data,size);
	packet_size += size;

	// if we're also the master of the game (not on a standalone)
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		multi_io_send_to_all_reliable(data, packet_size);
	}
	// if we're only the host on the standalone, then send the packet to the standalone to be routed
	else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

void process_wss_update_packet(ubyte *data, header *hinfo)
{		
	ubyte team;
	int size,player_index,idx;
	int offset = HEADER_LENGTH;

	// get the team/pool #
	GET_DATA(team);

	// get the data size
	GET_INT(size);		

	// if we're the standalone, then we should be routing this data to all the other clients
	if(Game_mode & GM_STANDALONE_SERVER){
		// read in the data		
		offset += size;		
		PACKET_SET_SIZE();

		// determine where this came from		
		player_index = find_player_id(hinfo->id);		
		Assert(player_index != -1);		
		if(player_index < 0){
			return;
		}

		// route the packet (don't resend it to the host)
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (&Net_players[idx] != Net_player) && (&Net_players[idx] != &Net_players[player_index]) ){				
				multi_io_send_reliable(&Net_players[idx], data, offset);
			}
		}			
	} else {
		// set the proper pool pointers
		common_set_team_pointers((int)team);

		// read in the block of data, and apply it to the weapons/ship pools
		offset += restore_wss_data(data + offset);
		PACKET_SET_SIZE();

		// set the pool pointers back to my own team
		common_set_team_pointers(Net_player->p_info.team);

		// sync the interface if this was for my own team
		if((int)team == Net_player->p_info.team){
			multi_ts_sync_interface();
		}		
	}
}
// ALAN END


// function to send firing information from the client to the server once they reach
// the final sync screen.
void send_firing_info_packet()
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;
	ubyte plinked, sdual;

	Assert( !(Net_player->flags & NETINFO_FLAG_AM_MASTER) );

	BUILD_HEADER(FIRING_INFO);
	plinked = (ubyte)((Player_ship->flags & SF_PRIMARY_LINKED)?1:0);
	sdual = (ubyte)((Player_ship->flags & SF_SECONDARY_DUAL_FIRE)?1:0);
	ADD_DATA( plinked );
	ADD_DATA( sdual );
	
	multi_io_send_reliable(Net_player, data, packet_size);
}

void process_firing_info_packet( ubyte *data, header *hinfo )
{
	int offset, player_num;	
	ubyte plinked, sdual;
	ship *shipp;

	// only the master of the game should be dealing with these packets
	Assert( Net_player->flags & NETINFO_FLAG_AM_MASTER );

	offset = HEADER_LENGTH;
	GET_DATA( plinked );
	GET_DATA( sdual );
	PACKET_SET_SIZE();	

	player_num = find_player_id(hinfo->id);
	if(player_num < 0){
		nprintf(("Network","Received firing info packet from unknown player, ignoring\n"));
		return;
	}

	// get the ship pointer for this player and set the flags accordingly.
	shipp = &(Ships[Objects[Net_players[player_num].m_player->objnum].instance]);
	if ( plinked )
		shipp->flags |= SF_PRIMARY_LINKED;
	else
		shipp->flags &= ~SF_PRIMARY_LINKED;

	if ( sdual )
		shipp->flags |= SF_SECONDARY_DUAL_FIRE;
	else
		shipp->flags &= ~SF_SECONDARY_DUAL_FIRE;
}

// packet to deal with changing status of mission goals.  used to be sent every so often from server
// to clients, but with addition of reliable sockets, send when complete, invalid, etc.
// goal_num is the index into mission_goals.  new_status means failed, success, etc.  -1 if not used.
// valid means goal is changing to invalid(0) or valid(1).  only applies if new_status == -1
void send_mission_goal_info_packet( int goal_num, int new_status, int valid )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;

	BUILD_HEADER(MISSION_GOAL_INFO);

	ADD_INT(goal_num);
	ADD_INT(new_status);
	ADD_INT(valid);
	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_mission_goal_info_packet( ubyte *data, header *hinfo )
{
	int offset, goal_num, new_status, valid;

	offset = HEADER_LENGTH;
	GET_INT(goal_num);
	GET_INT(new_status);
	GET_INT(valid);
	PACKET_SET_SIZE();

	// if new_status != -1, then this is a change in goal status (i.e. goal failed, or is successful)
	if ( new_status != -1 ){
		mission_goal_status_change( goal_num, new_status );
	} else {
		mission_goal_validation_change( goal_num, valid );
	}
}

void send_player_settings_packet(net_player *p)
{
	ubyte data[MAX_PACKET_SIZE];
	ubyte stop;
	int idx;
	int packet_size = 0;

	// build the header
	BUILD_HEADER(PLAYER_SETTINGS);

	// add all the data for all the players
	stop = 0x0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx])){
			ADD_DATA(stop);
			ADD_SHORT(Net_players[idx].player_id);

			// break the p_info structure by member, so we don't overwrite any absolute pointers
			// ADD_DATA(Net_players[idx].p_info);
			ADD_INT(Net_players[idx].p_info.team);
			ADD_INT(Net_players[idx].p_info.ship_index);
			ADD_INT(Net_players[idx].p_info.ship_class);
		}
	}
	// add the stop byte
	stop = 0xff;
	ADD_DATA(stop);

	// either broadcast the data or send to a specific player
	if(p == NULL){		
		multi_io_send_to_all_reliable(data, packet_size);
	} else {
		multi_io_send_reliable(p, data, packet_size);
	}			
}

void process_player_settings_packet(ubyte *data, header *hinfo)
{
	int offset,player_num;
	net_player_info bogus,*ptr;
	short player_id;
	ubyte stop;

	offset = HEADER_LENGTH;

	// read in the data for all the players
	GET_DATA(stop);
	while(stop != 0xff){
		// lookup the player
		GET_SHORT(player_id);
		player_num = find_player_id(player_id);

		// make sure this is a valid player
		if(player_num == -1){
			ptr = &bogus;			
		} else {
			ptr = &Net_players[player_num].p_info;
		}
		
		GET_INT(ptr->team);
		GET_INT(ptr->ship_index);
		GET_INT(ptr->ship_class);
		
		// next stop byte
		GET_DATA(stop);
	}
	PACKET_SET_SIZE();

	// update the server with my new state
	// MWA -- 3/31/98 -- check for in mission instead of state.
	//if ( Netgame.game_state == NETGAME_STATE_MISSION_SYNC) {
	if( !(Game_mode & GM_IN_MISSION) ) {
		Net_player->state = NETPLAYER_STATE_SETTINGS_ACK;
		send_netplayer_update_packet();	
	}


	// display some cool text
	multi_common_add_text(XSTR("Received player settings packet\n",721),1);	
}

void send_deny_packet(net_addr *addr, int code)
{
	ubyte data[10];
	int packet_size = 0;

	// build the header and add the rejection code
	BUILD_HEADER(DENY);

	ADD_INT(code);
	
	// send the packet	
	psnet_send(addr, data, packet_size);
}

void process_deny_packet(ubyte *data, header *hinfo)
{
	int offset,code;

	// get the denial code
	offset = HEADER_LENGTH;
	GET_INT(code);
	PACKET_SET_SIZE();

	// if there is already a dialog active, do nothing - who cares at this point.
	if(popup_active()){
		return;
	}

	// display the appropriate dialog
	switch(code){
	case JOIN_DENY_JR_STATE :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because the game is not in an appropriate state to accept",722));
		break;
	case JOIN_DENY_JR_TRACKER_INVAL :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because this is a Parallax Online game and you are not a registered Parallax Online pilot",723));
		break;
	case JOIN_DENY_JR_PASSWD :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because this is a password protected game",724));
		break;	
	case JOIN_DENY_JR_CLOSED :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because this is a closed game and the mission is in progress",725));
		break;
	case JOIN_DENY_JR_TEMP_CLOSED :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because the netgame is forming and the host has temporarily closed it",726));
		break;
	case JOIN_DENY_JR_RANK_HIGH :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because this is a rank limited game and your rank is too high",727));
		break;
	case JOIN_DENY_JR_RANK_LOW :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because this is a rank limited game and your rank is too low",728));
		break;
	case JOIN_DENY_JR_DUP :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because there is an identical player already in the game",729));
		break;
	case JOIN_DENY_JR_FULL :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because the game is full",730));
		break;
	case JOIN_DENY_JR_BANNED :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because you are banned from this server",731));
		break;
	case JOIN_DENY_JR_NOOBS :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been rejected because this game does not allow observers",732));
		break;
	case JOIN_DENY_JR_INGAME_JOIN :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You cannot join at this time since someone else is currently joining.  Try again shortly.",733));
		break;
	case JOIN_DENY_JR_BAD_VERSION :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You cannot join this game because you are running an older version of FreeSpace than the server.  Exit FreeSpace, and choose the 'Update FreeSpace' button in the FreeSpace launcher to download the latest version of FreeSpace.",734));
		break;	
	case JOIN_DENY_JR_TYPE :
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You cannot join a game in progress unless it is a dogfight mission.",1433));
		break;			
	}	

	// call this so that the join request timestamp automatically expires when we hear back from the server
	multi_join_reset_join_stamp();
}

// this packet will consist of 
// 1.) netplayer ship classes			(85 bytes max)
// 2.) ship weapon state data			(277 bytes max)
// 3.) player settings et. al.		(133 bytes max)
// TOTAL                            495				NOTE : keep this in mind when/if adding new data to this packet
void send_post_sync_data_packet(net_player *p, int std_request)
{
	ubyte data[MAX_PACKET_SIZE], val;
	char bval;
	ship *shipp;		
	net_player *pl;
	ship_obj *so;
	ushort sval, ship_ets;
	int idx, player_index;
	int packet_size = 0;
	int ship_count;
	short val_short;

	BUILD_HEADER(POST_SYNC_DATA);

	// some header information for standalone packet routing purposes
	val = (ubyte)std_request;
	ADD_DATA(val);

	// the standalone has two situations
	// 1.) sending a request to the host to distribute this block of data
	// 2.) having recevied this block of data from the host, it redistributes it
	if((Game_mode & GM_STANDALONE_SERVER) && std_request && (Netgame.host != NULL)){
		// case 1, send the request					
		multi_io_send_reliable(Netgame.host, data, packet_size);
		return;
	}
	// case 2 for the standalone is below (as normal)

	// otherwise build the data now	
	
	// add all deleted ships
	val = (ubyte)Multi_ts_num_deleted;
	ADD_DATA(val);
	for(idx=0;idx<Multi_ts_num_deleted;idx++){
		sval = (ushort)Objects[Multi_ts_deleted_objnums[idx]].net_signature;
		ADD_USHORT(sval);
	}

	// ship count	
	ship_count = 0;
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {		
		shipp = &Ships[Objects[so->objnum].instance];

		// don't process non player wing ships
		if ( !(shipp->flags & SF_FROM_PLAYER_WING ) )
			continue;

		ship_count++;
	}

	// # of ships - used multiple times in the packet
	val = (ubyte)ship_count;
	ADD_DATA(val);

	// add ship class information (85 bytes max)	
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {		
		shipp = &Ships[Objects[so->objnum].instance];

		// don't process non player wing ships
		if ( !(shipp->flags & SF_FROM_PLAYER_WING ) )
			continue;		
		
		// add the net signature of the object for look up
		ADD_USHORT( Objects[so->objnum].net_signature );
		
		// add the ship info index 
		val = (ubyte)(shipp->ship_info_index);
		ADD_DATA(val);		

		// add the ships's team select index
		val = (ubyte)shipp->ts_index;
		ADD_DATA(val);
	}	

	// add weapon state information for all starting ships (277 bytes max)
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		shipp = &Ships[Objects[so->objnum].instance];

		// don't process non player wing ships
		if ( !(shipp->flags & SF_FROM_PLAYER_WING ) )
			continue;			

		// if this is a ship owned by a player, we should mark down his weapons bank/link settings now if we're the server
		pl = NULL;
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			player_index = multi_find_player_by_net_signature(Objects[so->objnum].net_signature);
			if(player_index == -1){
				pl = NULL;
			} else {
				pl = &Net_players[player_index];
			}
		}

		// add the net signature and other weapon information
		ADD_USHORT( Objects[so->objnum].net_signature );		

		// add number of primary and secondary banks
		bval = (char)(shipp->weapons.num_primary_banks);
		ADD_DATA(bval);
		bval = (char)(shipp->weapons.num_secondary_banks);
		ADD_DATA(bval);

		// add weapon bank status
		bval = (char)(shipp->weapons.current_primary_bank);
		if(pl != NULL){
			pl->s_info.cur_primary_bank = bval;
		}
		// Assert(bval != -1);
		ADD_DATA(bval);

		bval = (char)(shipp->weapons.current_secondary_bank);
		if(pl != NULL){
			pl->s_info.cur_secondary_bank = bval;
		}
		// Assert(bval != -1);
		ADD_DATA(bval);
						
		// primary weapon info
		val = (ubyte)(shipp->weapons.primary_bank_weapons[0]);
		ADD_DATA(val);
		val = (ubyte)(shipp->weapons.primary_bank_weapons[1]);
		ADD_DATA(val);

		// secondary weapon info
		val_short = (short)(shipp->weapons.secondary_bank_weapons[0]);
		ADD_SHORT(val_short);
		val_short = (short)(shipp->weapons.secondary_bank_ammo[0]);
		ADD_SHORT(val_short);
		val_short = (short)(shipp->weapons.secondary_bank_weapons[1]);
		ADD_SHORT(val_short);
		val_short = (short)(shipp->weapons.secondary_bank_ammo[1]);
		ADD_SHORT(val_short);
		val_short = (short)(shipp->weapons.secondary_bank_weapons[2]);
		ADD_SHORT(val_short);
		val_short = (short)(shipp->weapons.secondary_bank_ammo[2]);
		ADD_SHORT(val_short);		
		
		// send primary and secondary weapon link status
		val = 0x0;
		if(shipp->flags & SF_PRIMARY_LINKED){
			if(pl != NULL){
				pl->s_info.cur_link_status |= (1<<0);
			}
			val |= (1<<0);
		}				
		if(shipp->flags & SF_SECONDARY_DUAL_FIRE){
			if(pl != NULL){
				pl->s_info.cur_link_status |= (1<<1);
			}
			val |= (1<<1);
		}		
		// if this is a player ship add (1<<2)
		if(Objects[shipp->objnum].flags & OF_PLAYER_SHIP){
			val |= (1<<2);
		}
		ADD_DATA(val);

		// add a ship ets value
		ship_ets = 0x0000;
		// shield ets		
		ship_ets |= ((ushort)shipp->shield_recharge_index << 8);
		// weapon ets
		ship_ets |= ((ushort)shipp->weapon_recharge_index << 4);
		// engine ets
		ship_ets |= ((ushort)shipp->engine_recharge_index);
		ADD_USHORT(ship_ets);

	}

	// 2 cases, if I'm the host on a standalone, I should be sending this to the standalone only
	// or if I'm the server as well as the host, I should be sending this to all players
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
 		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			// broadcast
			if(p == NULL){				
				multi_io_send_to_all_reliable(data, packet_size);
			}
			// send to a specific player
			else {
				multi_io_send_reliable(p, data, packet_size);
			}
		} else {
			multi_io_send_reliable(Net_player, data, packet_size);
		}
	}
	// standalone mode
	else {
		// broadcast
		if(p == NULL){
			multi_io_send_to_all_reliable(data, packet_size);
		}
		// send to a specific player
		else {
			multi_io_send_reliable(p, data, packet_size);
		}
	}
}

void process_post_sync_data_packet(ubyte *data, header *hinfo)
{
	ubyte val, sinfo_index, ts_index;
	char b;
	ushort net_sig, ship_ets, sval;
	ship *shipp;
	object *objp;
	int idx;
	int offset = HEADER_LENGTH;
	int ship_count;
	short val_short;

	// packet routing information
	GET_DATA(val);

	// if I'm the host on a standalone, this is going to be a request to send the data to the standalone to be routed
	if((Net_player->flags & NETINFO_FLAG_GAME_HOST) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) && val){
		PACKET_SET_SIZE();

		// at this point we want to delete all necessary ships, change all necessary ship classes, and set all weapons up
		multi_ts_create_wings();
		
		// send to the standalone through my socket
		send_post_sync_data_packet(Net_player);
		return;
	}

	// process player

	// add all deleted ships
	GET_DATA(val);
	Multi_ts_num_deleted = (int)val;
	for(idx=0;idx<Multi_ts_num_deleted;idx++){
		// get the ship's objnum
		GET_USHORT(sval);
		objp = NULL;
		objp = multi_get_network_object(sval);
		if(objp != NULL){
			// delete the ship appropriately
			// mark the object as having been deleted
			Multi_ts_deleted_objnums[idx] = OBJ_INDEX(objp);

			// delete the ship
			ship_add_exited_ship(&Ships[objp->instance], SEF_PLAYER_DELETED);
			obj_delete(Multi_ts_deleted_objnums[idx]);			
			ship_wing_cleanup(objp->instance,&Wings[Ships[objp->instance].wingnum]);
		} else {
			Multi_ts_num_deleted--;
			nprintf(("Network","Couldn't find object by net signature for ship delete in post sync data packet\n"));
		}
	}

	// ship count
	GET_DATA(val);
	ship_count = val;

	// process ship class information
	for(idx=0; idx<ship_count; idx++){	
		// get the object's net signature
		GET_USHORT(net_sig);
		GET_DATA(sinfo_index);
		GET_DATA(ts_index);

		// attempt to get the object
		objp = multi_get_network_object(net_sig);

		// make sure we found a ship
		Assert((objp != NULL) && (objp->type == OBJ_SHIP));

		// set the ship to be the right class
		change_ship_type(objp->instance,(int)sinfo_index);

		// set the ship's team select index
		Ships[objp->instance].ts_index = (int)ts_index;		
	}

	// process ship weapon state info
	for(idx=0; idx<ship_count; idx++){	
		// get the object's net signature
		GET_USHORT(net_sig);

		// attempt to get the object
		objp = multi_get_network_object(net_sig);

		// make sure we found a ship
		Assert((objp != NULL) && (objp->type == OBJ_SHIP));

		// get a pointer to the ship
		shipp = &Ships[objp->instance];

		// get number of primary and secondary banks;
		GET_DATA(b);
		Assert( b != -1 );
		shipp->weapons.num_primary_banks = (int)b;

		GET_DATA(b);
		Assert( b != -1 );
		shipp->weapons.num_secondary_banks = (int)b;

		// get bank selection info
		GET_DATA(b);
		if ( b == -1 ){
			b = 0;
		}
		shipp->weapons.current_primary_bank = (int)b;

		GET_DATA(b);
		if ( b == -1 ){
			b = 0;
		}
		shipp->weapons.current_secondary_bank = (int)b;		

			// primary weapon info
		GET_DATA(val);
		shipp->weapons.primary_bank_weapons[0] = (int)val;

		GET_DATA(val);
		shipp->weapons.primary_bank_weapons[1] = (int)val;

		// secondary weapon info
		GET_SHORT(val_short);
		shipp->weapons.secondary_bank_weapons[0] = (int)val_short;
		GET_SHORT(val_short);
		shipp->weapons.secondary_bank_ammo[0] = (int)val_short;

		GET_SHORT(val_short);
		shipp->weapons.secondary_bank_weapons[1] = (int)val_short;
		GET_SHORT(val_short);
		shipp->weapons.secondary_bank_ammo[1] = (int)val_short;

		GET_SHORT(val_short);
		shipp->weapons.secondary_bank_weapons[2] = (int)val_short;
		GET_SHORT(val_short);
		shipp->weapons.secondary_bank_ammo[2] = (int)val_short;

		// other flags
		val = 0x0;
		GET_DATA(val);

		if(val & (1<<0)){
			shipp->flags |= SF_PRIMARY_LINKED;				
		}				
		if(val & (1<<1)){		
			shipp->flags |= SF_SECONDARY_DUAL_FIRE;			
		}
		Objects[shipp->objnum].flags &= ~(OF_PLAYER_SHIP);
		Objects[shipp->objnum].flags &= ~(OF_COULD_BE_PLAYER);
		if(val & (1<<2)){
			Objects[shipp->objnum].flags |= OF_PLAYER_SHIP;
		} else {
			obj_set_flags( &Objects[shipp->objnum], Objects[shipp->objnum].flags | OF_COULD_BE_PLAYER );
		}

		// get ship ets
		GET_USHORT(ship_ets);
		// shield ets
		shipp->shield_recharge_index = ((ship_ets & 0x0f00) >> 8);
		// weapon ets
		shipp->weapon_recharge_index = ((ship_ets & 0x00f0) >> 4);
		// engine ets
		shipp->engine_recharge_index = (ship_ets & 0x000f);	
	}
	PACKET_SET_SIZE();

	// ack the server
	Net_player->state = NETPLAYER_STATE_POST_DATA_ACK;
	send_netplayer_update_packet();

	// the standalone server will receive this packet from the host of the game, to be applied locally and
	// also to be rebroadcast. 
	if(Game_mode & GM_STANDALONE_SERVER){
		// update player ets settings
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].m_player->objnum != -1)){
				multi_server_update_player_weapons(&Net_players[idx],&Ships[Objects[Net_players[idx].m_player->objnum].instance]);
			}
		}			
		
		send_post_sync_data_packet(NULL,0);
	}
}

void send_wss_slots_data_packet(int team_num,int final,net_player *p,int std_request)
{
	ubyte data[MAX_PACKET_SIZE],val;
	short val_short;
	int idx,i;
	int packet_size = 0;

	// build the header
	BUILD_HEADER(WSS_SLOTS_DATA);

	// some header information for standalone packet routing purposes
	val = (ubyte)std_request;
	ADD_DATA(val);

	// add the team #
	val = (ubyte)team_num;
	ADD_DATA(val);
	
	// add whether this is the final packet or not
	val = (ubyte)final;
	ADD_DATA(val);

	// the standalone has two situations
	// 1.) sending a request to the host to distribute this block of data
	// 2.) having recevied this block of data from the host, it redistributes it
	if((Game_mode & GM_STANDALONE_SERVER) && std_request){
		// case 1, send the request					
		multi_io_send_reliable(Netgame.host, data, packet_size);
		return;
	}
	// case 2 for the standalone is below (as normal)

	// add all the slots
	for(idx=0;idx<MULTI_TS_NUM_SHIP_SLOTS;idx++){
		// add the ship class
		val = (ubyte)Wss_slots_teams[team_num][idx].ship_class;
		ADD_DATA(val);

		// add the weapons
		for(i = 0;i<MAX_SHIP_WEAPONS;i++){
			val_short = (short)Wss_slots_teams[team_num][idx].wep[i];
			ADD_SHORT(val_short);
		}

		// add the weapon counts
		for(i = 0;i<MAX_SHIP_WEAPONS;i++){
			val_short = (short)Wss_slots_teams[team_num][idx].wep_count[i];
			ADD_SHORT(val_short);
		}
	}

		// 2 cases, if I'm the host on a standalone, I should be sending this to the standalone only
	// or if I'm the server as well as the host, I should be sending this to all players
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
 		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			// broadcast
			if(p == NULL){				
				multi_io_send_to_all_reliable(data, packet_size);
			}
			// send to a specific player
			else {
				multi_io_send_reliable(p, data, packet_size);
			}
		} else {
			multi_io_send_reliable(Net_player, data, packet_size);
		}
	}
	// standalone mode
	else {
		// broadcast
		if(p == NULL){
			multi_io_send_to_all_reliable(data, packet_size);			
		}
		// send to a specific player
		else {
			multi_io_send_reliable(p, data, packet_size);
		}
	}	
}

void process_wss_slots_data_packet(ubyte *data, header *hinfo)
{
	ubyte val,team_num,final;
	int idx,i;
	int offset = HEADER_LENGTH;
	short val_short;

	// packet routing information
	GET_DATA(val);

	// get team data
	GET_DATA(team_num);

	// get whether this is the final packet or not
	GET_DATA(final);

	// if I'm the host on a standalone, this is going to be a request to send the data to the standalone to be routed
	if((Net_player->flags & NETINFO_FLAG_GAME_HOST) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) && val){
		PACKET_SET_SIZE();

		// send to the standalone through my socket
		send_wss_slots_data_packet((int)team_num,(int)final,Net_player);
		return;
	}	

	// read in all the slot data
	for(idx=0;idx<MULTI_TS_NUM_SHIP_SLOTS;idx++){
		memset(&Wss_slots_teams[team_num][idx],0,sizeof(wss_unit));

		// get the ship class
		GET_DATA(val);
		Wss_slots_teams[team_num][idx].ship_class = (int)val;

		// get the weapons
		for(i = 0;i<MAX_SHIP_WEAPONS;i++){
			GET_SHORT(val_short);
			Wss_slots_teams[team_num][idx].wep[i] = (int)val_short;
		} 

		// get the weapon counts
		for(i = 0;i<MAX_SHIP_WEAPONS;i++){
			GET_SHORT(val_short);
			Wss_slots_teams[team_num][idx].wep_count[i] = (int)val_short;
		}
	}
	PACKET_SET_SIZE();

	// update my netplayer state if this is the final packet
	if(final){
		Net_player->state = NETPLAYER_STATE_WSS_ACK;
		send_netplayer_update_packet();
	}

	// the standalone server will receive this packet from the host of the game, to be applied locally and
	// also to be rebroadcast. 
	if(Game_mode & GM_STANDALONE_SERVER){
		send_wss_slots_data_packet((int)team_num,(int)final,NULL,0);
	} else {
		// add some mission sync text
		multi_common_add_text(XSTR("Weapon slots packet\n",735),1);
	}
}

#define OBJ_VISIBILITY_DOT					0.6f	

// send and receive packets for shield explosion information
void send_shield_explosion_packet( int objnum, int tri_num, vec3d hit_pos )
{
	int packet_size, i;
	ubyte data[MAX_PACKET_SIZE], utri_num;

	Int3();
	// Assert(!(Netgame.debug_flags & NETD_FLAG_CLIENT_NODAMAGE));

	Assert( tri_num < UCHAR_MAX );
	utri_num = (ubyte)tri_num;

	// for each player, determine if this object is behind the player -- if so, don't
	// send the packet.
	for ( i = 0; i < MAX_PLAYERS; i++ ) {
		if ( MULTI_CONNECTED(Net_players[i]) && (&Net_players[i] != Net_player) ) {
			float		dot;
			vec3d	eye_to_obj_vec, diff, eye_pos;
			matrix	eye_orient;

			eye_pos = Net_players[i].s_info.eye_pos;
			eye_orient = Net_players[i].s_info.eye_orient;

			// check for same vectors
			vm_vec_sub(&diff, &Objects[objnum].pos, &eye_pos);
			if ( vm_vec_mag_quick(&diff) < 0.01 ){
				continue;
			}

			vm_vec_normalized_dir(&eye_to_obj_vec, &Objects[objnum].pos, &eye_pos);
			dot = vm_vec_dot(&eye_orient.vec.fvec, &eye_to_obj_vec);

			if ( dot < OBJ_VISIBILITY_DOT ){
				continue;
			}

			BUILD_HEADER(SHIELD_EXPLOSION);

			ADD_USHORT( Objects[objnum].net_signature );
			ADD_DATA(utri_num);			
			
			multi_io_send(&Net_players[i], data, packet_size);
		}
	}
}

void add_shield_point_multi(int objnum, int tri_num, vec3d *hit_pos);

void process_shield_explosion_packet( ubyte *data, header *hinfo)
{
	int offset;
	ushort signature;
	ubyte utri_num;
	object *objp;

	// get the shield hit data
	offset = HEADER_LENGTH;
	GET_USHORT(signature);
	GET_DATA(utri_num);
	//GET_DATA(hit_pos);
	PACKET_SET_SIZE();

	// find the object with this signature.  If found, then do a shield explosion
	objp = multi_get_network_object( signature );
	if ( objp ) {
		polymodel *pm;
		shield_info *shieldp;
		shield_tri stri;
		vec3d hit_pos;
		int i;

		// given the tri num, find the local position which is the average of the
		// three vertices of the triangle affected.  Use this average point as the hit
		// point
		// Assert( objp->type == OBJ_SHIP );
		if(objp->type != OBJ_SHIP){
			return;
		}

		pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);
		shieldp = &pm->shield;
		Assert( utri_num < shieldp->ntris );
		stri = shieldp->tris[utri_num];
		vm_vec_zero(&hit_pos);
		for ( i = 0; i < 3; i++ ) {
			vm_vec_add2( &hit_pos, &(shieldp->verts[stri.verts[i]].pos) );
		}
		vm_vec_scale( &hit_pos, 1.0f/3.0f );
		add_shield_point_multi( OBJ_INDEX(objp), utri_num, &hit_pos );
	}
}

void send_player_stats_block_packet(net_player *pl, int stats_code, net_player *target, short offset)
{
	scoring_struct *sc;
	ubyte data[MAX_PACKET_SIZE], val;
	int idx;		
	int packet_size = 0;

	ushort u_tmp;
	int i_tmp;

	sc = &pl->m_player->stats;

	// build the header
	BUILD_HEADER(PLAYER_STATS);	

	// add the player id
	ADD_SHORT(pl->player_id);

	// add the byte indicating whether these stats are all-time or not
	val = (ubyte)stats_code;
	ADD_DATA(val);	
	
	// kill information - alltime
	switch(stats_code){
	case STATS_ALLTIME:	
		// alltime kills
#ifdef INF_BUILD
		idx = 0; 
		while(idx<MAX_SHIP_CLASSES)
		{
			send_player_stats_block_packet(pl, STATS_ALLTIME_KILLS, target, (short)idx);
			idx += MAX_SHIPS_PER_PACKET; 
		}
#else
		for(idx=0;idx<MAX_SHIP_CLASSES;idx++){
			u_tmp = (ushort)sc->kills[idx];
			ADD_USHORT(u_tmp);
		}
#endif

		// medal information
		for(idx=0;idx<MAX_MEDALS;idx++){
			i_tmp = sc->medals[idx];
			ADD_INT(i_tmp);
		}

		ADD_INT(sc->score);
		ADD_INT(sc->rank);
		ADD_INT(sc->assists);
		ADD_INT(sc->kill_count);
		ADD_INT(sc->kill_count_ok);
		ADD_UINT(sc->p_shots_fired);
		ADD_UINT(sc->s_shots_fired);
		ADD_UINT(sc->p_shots_hit);
		ADD_UINT(sc->s_shots_hit);
		ADD_UINT(sc->p_bonehead_hits);
		ADD_UINT(sc->s_bonehead_hits);
		ADD_INT(sc->bonehead_kills);

		ADD_UINT(sc->missions_flown);
		ADD_UINT(sc->flight_time);
		ADD_INT(sc->last_flown);  // should be 32-bit value - taylor
		ADD_INT(sc->last_backup);  // should be 32-bit value - taylor
		break;

	case STATS_MISSION:	
		// mission OKkills	
#ifdef INF_BUILD
		idx = 0; 
		while(idx<MAX_SHIP_CLASSES)
		{
			send_player_stats_block_packet(pl, STATS_MISSION_CLASS_KILLS, target, (short)idx);
			idx += MAX_SHIPS_PER_PACKET; 
		}
#else		
		for(idx=0;idx<MAX_SHIP_CLASSES;idx++){
			u_tmp = (ushort)sc->m_okKills[idx];
			ADD_USHORT(u_tmp);			
		}
#endif
	
		ADD_INT(sc->m_score);
		ADD_INT(sc->m_assists);
		ADD_INT(sc->m_kill_count);
		ADD_INT(sc->m_kill_count_ok);
		ADD_UINT(sc->mp_shots_fired);
		ADD_UINT(sc->ms_shots_fired);
		ADD_UINT(sc->mp_shots_hit);
		ADD_UINT(sc->ms_shots_hit);
		ADD_UINT(sc->mp_bonehead_hits);
		ADD_UINT(sc->ms_bonehead_hits);
		ADD_INT(sc->m_bonehead_kills);
		ADD_INT(sc->m_player_deaths);
		ADD_INT(sc->m_medal_earned);
		break;

	case STATS_MISSION_KILLS:		
		ADD_INT(sc->m_kill_count);
		ADD_INT(sc->m_kill_count_ok);
		ADD_INT(sc->m_assists);
		break;		

	case STATS_DOGFIGHT_KILLS:
		for(idx=0; idx<MAX_PLAYERS; idx++){
			u_tmp = (ushort)sc->m_dogfight_kills[idx];
			ADD_USHORT(u_tmp);
		}
		ADD_INT(sc->m_kill_count);
		ADD_INT(sc->m_kill_count_ok);
		ADD_INT(sc->m_assists);
		break;
	
#ifdef INF_BUILD		
	case STATS_MISSION_CLASS_KILLS:
		ADD_SHORT(offset);
		for (idx=offset; idx<MAX_SHIP_CLASSES && idx<offset+MAX_SHIPS_PER_PACKET; idx++)
		{
			ADD_USHORT((ushort)sc->m_okKills[idx]);			
		}
		break;
		
	case STATS_ALLTIME_KILLS:
		ADD_SHORT(offset);
		for (idx=offset; idx<MAX_SHIP_CLASSES && idx<offset+MAX_SHIPS_PER_PACKET; idx++)
		{
			ADD_USHORT((ushort)sc->kills[idx]);			
		}
		break;
#endif
	}

	Assert(packet_size < MAX_PACKET_SIZE);

	// if we're a client, always send the data to the server
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		multi_io_send_reliable(Net_player, data, packet_size);		
	}
	// otherwise do server specific stuff
	else {
		// send to a specific target
		if(target != NULL){
			multi_io_send_reliable(target, data, packet_size);			
		}
		// otherwise, send to everyone
		else {			
			multi_io_send_to_all_reliable(data, packet_size);
		}
	}
}

void process_player_stats_block_packet(ubyte *data, header *hinfo)
{
	ubyte val;
	int player_num,idx;
	scoring_struct *sc,bogus;
	short player_id;
	int offset = HEADER_LENGTH;
	ushort u_tmp;
	int i_tmp;

	// nprintf(("Network","----------++++++++++********RECEIVED STATS***********+++++++++----------\n"));

	// get the player who these stats are for
	GET_SHORT(player_id);	
	player_num = find_player_id(player_id);
	if (player_num == -1) {
		nprintf(("Network", "Couldn't find player for stats update!\n"));
		ml_string("Couldn't find player for stats update!\n");

		sc = &bogus;
		Int3();
	} else {
		sc = &Net_players[player_num].m_player->stats;
	}

	// get the stats code
	GET_DATA(val);	
	switch(val){

#ifdef INF_BUILD
	short si_offset;

	case STATS_ALLTIME_KILLS:
		GET_SHORT(si_offset);
		for (idx = si_offset; idx<MAX_SHIP_CLASSES && idx<si_offset+MAX_SHIPS_PER_PACKET; idx++) 
		{
			GET_USHORT(u_tmp);
			sc->kills[idx] = u_tmp;
		}
		break;

	case STATS_MISSION_CLASS_KILLS:
		GET_SHORT(si_offset);
		for (idx = si_offset; idx<MAX_SHIP_CLASSES && idx<si_offset+MAX_SHIPS_PER_PACKET; idx++) 
		{
			GET_USHORT(u_tmp);
			sc->m_okKills[idx] = u_tmp;
		}
		break;
#endif

	case STATS_ALLTIME:
		ml_string("Received STATS_ALLTIME\n");

#ifndef INF_BUILD
		// kills - alltime
		for (idx=0; idx<MAX_SHIP_CLASSES; idx++) {
			GET_USHORT(u_tmp);
			sc->kills[idx] = u_tmp;
		}
#endif

		// read in the stats
		for (idx=0; idx<MAX_MEDALS; idx++) {
			GET_INT(i_tmp);
			sc->medals[idx] = i_tmp;
		}

		GET_INT(sc->score);
		GET_INT(sc->rank);
		GET_INT(sc->assists);
		GET_INT(sc->kill_count);
		GET_INT(sc->kill_count_ok);
		GET_UINT(sc->p_shots_fired);
		GET_UINT(sc->s_shots_fired);
		GET_UINT(sc->p_shots_hit);
		GET_UINT(sc->s_shots_hit);
		GET_UINT(sc->p_bonehead_hits);
		GET_UINT(sc->s_bonehead_hits);
		GET_INT(sc->bonehead_kills);

		GET_UINT(sc->missions_flown);
		GET_UINT(sc->flight_time);
		GET_INT(sc->last_flown);  // should be 32-bit value - taylor
		GET_INT(sc->last_backup);  // should be 32-bit value - taylor
		break;

	case STATS_MISSION:
		ml_string("Received STATS_MISSION\n");

#ifndef INF_BUILD
		// kills - mission OK			
		for (idx=0; idx<MAX_SHIP_CLASSES; idx++) {
			GET_USHORT(u_tmp);
			sc->m_okKills[idx] = u_tmp;			
		}
#endif
		
		GET_INT(sc->m_score);
		GET_INT(sc->m_assists);
		GET_INT(sc->m_kill_count);
		GET_INT(sc->m_kill_count_ok);
		GET_UINT(sc->mp_shots_fired);
		GET_UINT(sc->ms_shots_fired);
		GET_UINT(sc->mp_shots_hit);
		GET_UINT(sc->ms_shots_hit);
		GET_UINT(sc->mp_bonehead_hits);
		GET_UINT(sc->ms_bonehead_hits);
		GET_INT(sc->m_bonehead_kills);
		GET_INT(sc->m_player_deaths);
		GET_INT(sc->m_medal_earned);
		break;

	case STATS_MISSION_KILLS:		
		ml_string("Received STATS_MISSION_KILLS\n");

		GET_INT(sc->m_kill_count);
		GET_INT(sc->m_kill_count_ok);
		GET_INT(sc->m_assists);
		break;		

	case STATS_DOGFIGHT_KILLS:
		ml_string("Received STATS_DOGFIGHT_KILLS\n");
		if(player_num >= 0){
			ml_printf("Dogfight stats for %s", Net_players[player_num].m_player->callsign);
		}
		for(idx=0; idx<MAX_PLAYERS; idx++){
			GET_USHORT(u_tmp);
			sc->m_dogfight_kills[idx] = u_tmp;
			if(player_num >= 0){				
				ml_printf("%d", Net_players[player_num].m_player->stats.m_dogfight_kills[idx]);
			}
		}
		GET_INT(sc->m_kill_count);
		GET_INT(sc->m_kill_count_ok);
		GET_INT(sc->m_assists);		
		break;	
	}
	PACKET_SET_SIZE();

	// if I'm the server of the game, I should always rebroadcast these stats
	if ((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (sc != &bogus)) {
		// make sure these are alltime stats
		Assert(val == STATS_ALLTIME || val == STATS_ALLTIME_KILLS);

		multi_broadcast_stats(val);
	}
}

// called to create asteroid stuff
void send_asteroid_create( object *new_objp, object *parent_objp, int asteroid_type, vec3d *relvec )
{
	int packet_size;
	ubyte data[MAX_PACKET_SIZE];
	ubyte packet_type, atype;
	vec3d vec;

	vm_vec_zero(&vec);
	if (relvec != NULL )
		vec = *relvec;

	BUILD_HEADER( ASTEROID_INFO );
	packet_type = ASTEROID_CREATE;

	Assert( asteroid_type < UCHAR_MAX );
	atype = (ubyte)asteroid_type;

	ADD_DATA( packet_type );
	ADD_USHORT( parent_objp->net_signature );
	ADD_USHORT( new_objp->net_signature );
	ADD_DATA( atype );
	ADD_VECTOR( vec );

	multi_io_send_to_all(data, packet_size);
}

void send_asteroid_throw( object *objp )
{
	int packet_size;
	ubyte data[MAX_PACKET_SIZE], packet_type;

	BUILD_HEADER( ASTEROID_INFO );

	// this packet type is an asteroid throw
	packet_type = ASTEROID_THROW;
	ADD_DATA( packet_type );
	ADD_USHORT( objp->net_signature );
	ADD_VECTOR( objp->pos );
	ADD_VECTOR( objp->phys_info.vel );
	
	multi_io_send_to_all(data, packet_size);
}

void send_asteroid_hit( object *objp, object *other_objp, vec3d *hitpos, float damage )
{
	int packet_size;
	ubyte data[MAX_PACKET_SIZE], packet_type;
	vec3d vec;

	vm_vec_zero(&vec);
	if ( hitpos != NULL )
		vec = *hitpos;

	// build up an asteroid hit packet
	BUILD_HEADER( ASTEROID_INFO );
	packet_type = ASTEROID_HIT;
	ADD_DATA( packet_type );
	ADD_USHORT( objp->net_signature );

	if(other_objp == NULL){
		ushort invalid_sig = 0xffff;
		ADD_USHORT(invalid_sig);
	} else {
		ADD_USHORT( other_objp->net_signature );
	}
	ADD_VECTOR( vec );
	ADD_FLOAT( damage );
	
	multi_io_send_to_all(data, packet_size);
}

void process_asteroid_info( ubyte *data, header *hinfo )
{
	int offset;
	ubyte packet_type;

	offset = HEADER_LENGTH;
	GET_DATA( packet_type );

	// based on the packet type, do something interesting with an asteroid!
	switch( packet_type ) {

	case ASTEROID_CREATE: {
		ushort psignature, signature;
		ubyte atype;
		vec3d relvec;
		object *parent_objp;

		GET_USHORT( psignature );
		GET_USHORT( signature );
		GET_DATA( atype );
		GET_VECTOR( relvec );

		// after getting the values, set the next network signature, and call the create sub function
		multi_set_network_signature( signature, MULTI_SIG_ASTEROID );
		parent_objp = multi_get_network_object( psignature );
		if ( parent_objp ) {
			asteroid_sub_create( parent_objp, atype, &relvec );
		} else {
			nprintf(("Network", "Couldn't create asteroid because parent wasn't found!!!\n"));
		}


		break;
	}

		// asteroid throw packet -- asteroid has wrapped bounds
	case ASTEROID_THROW: {
		ushort signature;
		vec3d pos, vel;
		object *objp;

		GET_USHORT( signature );
		GET_VECTOR( pos );
		GET_VECTOR( vel );
		objp = multi_get_network_object( signature );
		if ( !objp ) {
			nprintf(("Network", "Couldn't throw asteroid because couldn't find it\n"));
			break;
		}
		objp->pos = pos;
		objp->phys_info.vel = vel;
		objp->phys_info.desired_vel = vel;
		break;
	}

	case ASTEROID_HIT: {
		ushort signature, osignature;
		object *objp, *other_objp;
		vec3d hitpos;
		float damage;

		GET_USHORT( signature );
		GET_USHORT( osignature );
		GET_VECTOR( hitpos );
		GET_FLOAT( damage );

		objp = multi_get_network_object( signature );
		if(osignature == 0xffff){
			other_objp = NULL; 
		} else {
			other_objp = multi_get_network_object( osignature );
		}
		if ( !objp ) {
			nprintf(("Network", "Cannot hit asteroid because signature isn't found\n"));
			break;
		}

		if ( IS_VEC_NULL(&hitpos) ){
			asteroid_hit( objp, other_objp, NULL, damage );
		} else {
			asteroid_hit( objp, other_objp, &hitpos, damage );
		}
		
		// if we know the other object is a weapon, then do a weapon hit to kill the weapon
		if ( other_objp && (other_objp->type == OBJ_WEAPON) ){
			weapon_hit( other_objp, objp, &hitpos );
		}
		break;
	}

	default:
		Int3();
		break;
	}

	PACKET_SET_SIZE();
}

void send_host_restr_packet(char *callsign,int code,int mode)
{
	ubyte data[MAX_PACKET_SIZE],val;
	int packet_size = 0;

	// build the header and add the opcode
	BUILD_HEADER(HOST_RESTR_QUERY);
	val = (ubyte)code;
	ADD_DATA(val);
	val = (ubyte)mode;
	ADD_DATA(val);

	// add the callsign
	ADD_STRING(callsign);

	// if I'm the standalone server, I should be sending this to the game host
	if((Game_mode & GM_STANDALONE_SERVER) && (Netgame.host != NULL)){		
		multi_io_send_reliable(Netgame.host, data, packet_size);
	} 
	// otherwise if I'm the host, I should be sending a reply back to the standalone server
	else {
		Assert(Net_player->flags & NETINFO_FLAG_GAME_HOST);		
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}	

void process_host_restr_packet(ubyte *data, header *hinfo)
{
	char callsign[255];
	ubyte code,mode;
	int offset = HEADER_LENGTH;

	// get the opcode and the callsign
	GET_DATA(code);
	GET_DATA(mode);
	GET_STRING(callsign);
	PACKET_SET_SIZE();

	// do code specific operations
	switch(code)
	{
		// query to the host from standalone
		case 0:		
			Assert((Net_player->flags & NETINFO_FLAG_GAME_HOST) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER));

			// set the join mode
			Multi_join_restr_mode = mode;

			// set the timestamp
			Multi_restr_query_timestamp = timestamp(MULTI_QUERY_RESTR_STAMP);

			// notify the host of the event
			gamesnd_play_iface(SND_BRIEF_STAGE_CHG_FAIL);
			HUD_printf(XSTR("Player %s has tried to join - allow (y/n) ?",736),callsign);
			break;
			
		// affirmative reply from the host to the standalone
		case 1:
			Assert(Game_mode & GM_STANDALONE_SERVER);		

			// let the player join if the timestamp has not already elapsed on the server
			if(Multi_restr_query_timestamp != -1){
				multi_process_valid_join_request(&Multi_restr_join_request,&Multi_restr_addr,(int)mode);
			}
			break;	

		// negative reply
		case 2 :
			Assert(Game_mode & GM_STANDALONE_SERVER);
			Netgame.flags &= ~(NG_FLAG_INGAME_JOINING);
			Multi_restr_query_timestamp = -1;
			break;
	}
}

void send_netgame_end_error_packet(int notify_code,int err_code)
{
	ubyte data[10];
	char code;
	int packet_size = 0;

	// only the server should ever be here - although this might change if for some reason the host wants to end the game
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);
	
	// build the header and add the notification and error codes
	BUILD_HEADER(NETGAME_END_ERROR);
	code = (char)notify_code;
	ADD_DATA(code);
	code = (char)err_code;
	ADD_DATA(code);

	// send the packet	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_netgame_end_error_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	char notify_code,error_code;

	// get the error and notification codes
	GET_DATA(notify_code);
	GET_DATA(error_code);
	PACKET_SET_SIZE();

	// quit the game
	multi_quit_game(PROMPT_NONE,notify_code,error_code);	
}

// sends info that a countermeasure succeeded.
void send_countermeasure_success_packet( int objnum )
{
	int pnum, packet_size;
	ubyte data[MAX_PACKET_SIZE];

	pnum = multi_find_player_by_object( &Objects[objnum] );
	if ( pnum == -1 ) {
		nprintf(("Network", "Coulnd't find player for countermeasure success packet\n"));
		return;
	}

	BUILD_HEADER(COUNTERMEASURE_SUCCESS);	
	multi_io_send(&Net_players[pnum], data, packet_size);
}

// start the flashing of my hud gauge
void process_countermeasure_success_packet( ubyte *data, header *hinfo )
{
	int offset;

	offset = HEADER_LENGTH;
	PACKET_SET_SIZE();

	//Do this instead so there's less repeat code
	//Player_obj is necessary...infinitely recursive function calls != FTW
	cmeasure_maybe_alert_success(Player_obj);
	/*hud_start_text_flash(XSTR("Evaded", 1430), 800);
	snd_play(&Snds[SND_MISSILE_EVADED_POPUP]);*/
}

#define UPDATE_IS_PAUSED		(1<<0)
#define UPDATE_HULL_INFO		(1<<1)

void send_client_update_packet(net_player *pl)
{
	ubyte data[MAX_PACKET_SIZE],val;
	int packet_size = 0;

	// build the header
	BUILD_HEADER(CLIENT_UPDATE);

	val = 0;	

	// add the pause status
	if ( Multi_pause_status ) {
		val |= UPDATE_IS_PAUSED;
	} else if ( (Game_mode & GM_IN_MISSION) && !(pl->flags & NETINFO_FLAG_INGAME_JOIN) && !(NETPLAYER_IS_OBSERVER(pl)) && !(NETPLAYER_IS_DEAD(pl)) && (Objects[pl->m_player->objnum].type == OBJ_SHIP) ) {
		val |= UPDATE_HULL_INFO;
		Assert( Player_ship );			// I"d better have one of these!!!!
	}

	ADD_DATA(val);

	// if paused, add the net address of the guy who paused
	if(val & UPDATE_IS_PAUSED){
		Assert(Multi_pause_pauser != NULL);
		ADD_DATA(Multi_pause_pauser->player_id);
	}

	// when not paused, send hull/shield/subsystem updates to all clients (except for ingame joiners)
	if ( val & UPDATE_HULL_INFO ) {
		object *objp;
		ubyte percent, ns, threats;
		ship_info *sip;
		ship *shipp;
		ship_subsys *subsysp;
		int i;

		// get the object for the player
		Assert( pl->m_player->objnum != -1 );

		objp = &Objects[pl->m_player->objnum];

		Assert ( objp->type == OBJ_SHIP );
		shipp = &Ships[objp->instance];
		sip = &Ship_info[shipp->ship_info_index];

		// hull strength and shield mesh information are floats (as a percentage).  Pass the integer
		// percentage value since that should be close enough
		percent = (ubyte) ((get_hull_pct(objp) * 100.0f) + 0.5f);
		if ( (percent == 0) && (get_hull_pct(objp) > 0.0f) ) {
			percent = 1;
		}
		ADD_DATA( percent );

		for (i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {
			percent = (ubyte)(objp->shield_quadrant[i] / get_max_shield_quad(objp) * 100.0f);
			ADD_DATA( percent );
		}

		// add the data for the subsystem hits.  We can assume that the lists will be the same side of
		// both machines. Added as percent since that number <= 100

		// also write out the number of subsystems.  We do this because the client might not know
		// about the object he is getting data for.  (i.e. he killed the object already).
		ns = (ubyte)sip->n_subsystems;
		ADD_DATA( ns );

		// now the subsystems.
		for ( subsysp = GET_FIRST(&shipp->subsys_list); subsysp != END_OF_LIST(&shipp->subsys_list); subsysp = GET_NEXT(subsysp) ) {
			percent = (ubyte)(subsysp->current_hits / subsysp->max_hits * 100.0f);
			ADD_DATA( percent );
		}

		// compute the threats for this player.  Only compute the threats if this player is actually
		// playing (i.e. he has a ship)
		hud_update_reticle( pl->m_player );
		threats = (ubyte)pl->m_player->threat_flags;
		ADD_DATA( threats );

		// add his energy level for guns
		ADD_FLOAT(shipp->weapon_energy);

		// add his secondary bank ammo
		ADD_INT(shipp->weapons.num_secondary_banks);
		for(i=0; i<shipp->weapons.num_secondary_banks; i++){
			ADD_INT(shipp->weapons.secondary_bank_ammo[i]);
		}
	}

	// add pl
	ADD_INT(pl->sv_last_pl);

	// send the packet reliably to the player	
	multi_io_send(pl, data, packet_size);
}

void process_client_update_packet(ubyte *data, header *hinfo)
{
	ubyte val;
	short pauser;
	int player_index;
	int is_paused, have_hull_info;
	int ammo_count;
	int ammo[10];
	float weapon_energy;
	int offset = HEADER_LENGTH;

	// get the header byte containing useful information
	GET_DATA(val);

	is_paused = (val & UPDATE_IS_PAUSED)?1:0;
	have_hull_info = (val & UPDATE_HULL_INFO)?1:0;

	// if we are paused, get who paused
	if(is_paused){		
		GET_SHORT(pauser);
		player_index = find_player_id(pauser);
		if(player_index != -1){
			Multi_pause_pauser = &Net_players[player_index];
		} else {
			Multi_pause_pauser = NULL;
		}	
	}

	// if we have hull information, then read it in.
	if ( have_hull_info ) {
		float fl_val;
		ship_info *sip;
		ship *shipp;
		ubyte hull_percent, shield_percent[MAX_SHIELD_SECTIONS], n_subsystems, subsystem_percent[MAX_MODEL_SUBSYSTEMS], threats;
		ubyte ub_tmp;
		ship_subsys *subsysp;
		object *objp;
		int i;

		// hull strength and shield mesh information are floats (as a percentage).  Pass the integer
		// percentage value since that should be close enough
		GET_DATA( hull_percent );

		for (i = 0; i < MAX_SHIELD_SECTIONS; i++ ){
			GET_DATA(ub_tmp);
			shield_percent[i] = ub_tmp;
		}

		// get the data for the subsystems
		GET_DATA( n_subsystems );
		for ( i = 0; i < n_subsystems; i++ ){
			GET_DATA(ub_tmp);
			subsystem_percent[i] = ub_tmp;
		}

		GET_DATA( threats );

		// add his energy level for guns
		GET_FLOAT(weapon_energy);
		
		// add his secondary bank ammo
		GET_INT(ammo_count);
		for(i=0; i<ammo_count; i++){
			GET_INT(ammo[i]);
		}

		// assign the above information to my ship, assuming that I can find it!  Ingame joiners might get this
		// packet because of delay between reliable packet acknowledging my ingame ship and the start of these
		// UDP client update packets.  Only read this info if I have a ship.
		if ( !(Net_player->flags & NETINFO_FLAG_INGAME_JOIN) && (Player_ship != NULL) && (Player_obj != NULL) && (Net_player != NULL)) {			
			shipp = Player_ship;
			objp = Player_obj;
			sip = &Ship_info[shipp->ship_info_index];

			fl_val = hull_percent * shipp->ship_max_hull_strength / 100.0f;
			objp->hull_strength = fl_val;

			for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {
				fl_val = (shield_percent[i] * get_max_shield_quad(objp) / 100.0f);
				objp->shield_quadrant[i] = fl_val;
			}

			// for sanity, be sure that the number of susbystems that I read in matches the player.  If not,
			// then don't read these in.
			if ( n_subsystems == sip->n_subsystems ) {

				n_subsystems = 0;		// reuse this variable
				for ( subsysp = GET_FIRST(&shipp->subsys_list); subsysp != END_OF_LIST(&shipp->subsys_list); subsysp = GET_NEXT(subsysp) ) {
					int subsys_type;

					fl_val = subsystem_percent[n_subsystems] * subsysp->max_hits / 100.0f;
					subsysp->current_hits = fl_val;

					// add the value just generated (it was zero'ed above) into the array of generic system types
					subsys_type = subsysp->system_info->type;					// this is the generic type of subsystem
					Assert ( subsys_type < SUBSYSTEM_MAX );
					if (!(subsysp->flags & SSF_NO_AGGREGATE)) {
						shipp->subsys_info[subsys_type].aggregate_current_hits += fl_val;
					}
					n_subsystems++;
				}
			}
			ship_recalc_subsys_strength( shipp );

			shipp->weapon_energy = weapon_energy;
			for(i=0; i<ammo_count; i++){
				shipp->weapons.secondary_bank_ammo[i] = ammo[i];
			}

			// update my threat flags.
			// temporarily commented out until tested.
			Net_player->m_player->threat_flags = threats;
		}
	}

	// get pl
	int pl;
	GET_INT(pl);
	if(Net_player != NULL){
		Net_player->cl_last_pl = pl;
	}

	PACKET_SET_SIZE();
	// note, if we're already paused or unpaused, calling these will have no effect, so it is safe to do so
	if(!popup_active() && !(Net_player->flags & NETINFO_FLAG_RESPAWNING) && !(Net_player->flags & NETINFO_FLAG_LIMBO)){
		if( is_paused ) {
			multi_pause_pause();
		} else {
			multi_pause_unpause();
		}
	}
}

void send_countdown_packet(int time)
{
	ubyte data[20];
	char val;
	int packet_size = 0;

	// build the header and add the time
	BUILD_HEADER(COUNTDOWN);
	val = (char)time;
	ADD_DATA(val);

	// if we're the server, we should broadcast to everyone
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		multi_io_send_to_all_reliable(data, packet_size);
	}
	// otherwise we'de better be a host sending to the standalone
	else {
		Assert(Net_player->flags & NETINFO_FLAG_GAME_HOST);
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

void process_countdown_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	char time;

	// get the time
	GET_DATA(time);
	PACKET_SET_SIZE();

	// if we're not in the post sync data screen, ignore it
	if(gameseq_get_state() != GS_STATE_MULTI_MISSION_SYNC){
		return;
	}

	// if we're the standalone, this should be a -1 telling us to start the countdown
	if(Game_mode & GM_STANDALONE_SERVER){
		Assert((int)time == -1);

		// start the countdown
		multi_sync_start_countdown();
	}
	// otherwise if we're clients, just bash the countdown
	else {		
		Multi_sync_countdown = (int)time;
	}
}

// packets for debriefing information
void send_debrief_info( int stage_count[], int *stages[] )
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size, i, j;
	int i_tmp;

	BUILD_HEADER(DEBRIEF_INFO);

	// add the data for the teams
	for ( i = 0; i < Num_teams; i++ ) {
		int count;

		count = stage_count[i];
		ADD_INT( count );
		for ( j = 0; j < count; j++ ) {
			i_tmp = stages[i][j];
			ADD_INT( i_tmp );
		}
	}
	
	multi_io_send_to_all_reliable(data, packet_size);
}

// process a debrief info packet from the server
void process_debrief_info( ubyte *data, header *hinfo )
{
	int offset, i, j;
	int stage_counts[MAX_TVT_TEAMS], active_stages[MAX_TVT_TEAMS][MAX_DEBRIEF_STAGES], *stages[MAX_TVT_TEAMS];
	int i_tmp;

	offset = HEADER_LENGTH;
	for ( i = 0; i < Num_teams; i++ ) {
		int count;

		GET_INT( count );
		stage_counts[i] = count;
		stages[i] = active_stages[i];
		for ( j = 0; j < count; j++ ) {
			GET_INT(i_tmp);
			active_stages[i][j] = i_tmp;
		}
	}
	PACKET_SET_SIZE();

	// now that we have the stage data for the debriefing stages, call debrief function with the
	// data so that clients can now see the debriefing stuff.  Do it only for my team.
	Assert( (Net_player->p_info.team >= 0) && (Net_player->p_info.team < Num_teams) );
	debrief_set_multi_clients( stage_counts[Net_player->p_info.team], stages[Net_player->p_info.team] );
}

// sends homing information to all clients.  We only need signature and num_missiles (because of hornets).
// sends homing_object and homing_subsystem to all clients.
void send_homing_weapon_info( int weapon_num )
{
	ubyte data[MAX_PACKET_SIZE];
	char t_subsys;
	int packet_size;
	object *homing_object;
	ushort homing_signature;
	weapon *wp;

	wp = &Weapons[weapon_num];

	// be sure that this weapon object is a homing object.
	if ( !(Weapon_info[wp->weapon_info_index].wi_flags & WIF_HOMING) )
		return;

	// default the subsystem
	t_subsys = -1;

	// get the homing signature.  If this weapon isn't homing on anything, then sent 0 as the
	// homing signature.
	homing_signature = 0;
	homing_object = wp->homing_object;
	if ( homing_object != NULL ) {
		homing_signature = homing_object->net_signature;

		// get the subsystem index.
		if ( (homing_object->type == OBJ_SHIP) && (wp->homing_subsys != NULL) ) {
			int s_index;

			s_index = ship_get_index_from_subsys( wp->homing_subsys, OBJ_INDEX(homing_object), 1 );
			Assert( s_index < CHAR_MAX );			// better be less than this!!!!
			t_subsys = (char)s_index;
		}
	}

	BUILD_HEADER(HOMING_WEAPON_UPDATE);
	ADD_USHORT( Objects[wp->objnum].net_signature );
	ADD_USHORT( homing_signature );
	ADD_DATA( t_subsys );
	
	multi_io_send_to_all(data, packet_size);
}

// process a homing weapon info change packet.  multiple_missiles parameter specifies is this
// packet contains information for multiple weapons (like hornets).
void process_homing_weapon_info( ubyte *data, header *hinfo )
{
	int offset;
	ushort weapon_signature, homing_signature;
	char h_subsys;
	object *homing_object, *weapon_objp;
	weapon *wp;

	offset = HEADER_LENGTH;

	// get the data for the packet
	GET_USHORT( weapon_signature );
	GET_USHORT( homing_signature );
	GET_DATA( h_subsys );
	PACKET_SET_SIZE();

	// deal with changing this weapons homing information
	weapon_objp = multi_get_network_object( weapon_signature );
	if ( weapon_objp == NULL ) {
		nprintf(("Network", "Couldn't find weapon object for homing update -- skipping update\n"));
		return;
	}
	Assert( weapon_objp->type == OBJ_WEAPON );
	wp = &Weapons[weapon_objp->instance];

	// be sure that we can find these weapons and 
	homing_object = multi_get_network_object( homing_signature );
	if ( homing_object == NULL ) {
		nprintf(("Network", "Couldn't find homing object for homing update\n"));
		return;
	}

	if ( homing_object->type == OBJ_WEAPON ) {
		int flags = Weapon_info[Weapons[homing_object->instance].weapon_info_index].wi_flags;

	//	Assert( (flags & WIF_BOMB) || (flags & WIF_CMEASURE) );

		if ( !((flags & WIF_BOMB) || (flags & WIF_CMEASURE)) ) {
			nprintf(("Network", "Homing object is invalid for homing update\n"));
			return;
		}
	}

	wp->homing_object = homing_object;
	wp->homing_subsys = NULL;
	wp->target_num = OBJ_INDEX(homing_object);
	wp->target_sig = homing_object->signature;
	if ( h_subsys != -1 ) {
		Assert( homing_object->type == OBJ_SHIP );
		wp->homing_subsys = ship_get_indexed_subsys( &Ships[homing_object->instance], h_subsys);
	}

	if ( homing_object->type == OBJ_SHIP ) {
		nprintf(("Network", "Updating homing information for weapon -- homing on %s\n", Ships[homing_object->instance].ship_name));
	}
}

void send_emp_effect(ushort net_sig, float intensity, float time)
{
	ubyte data[25];
	int packet_size;

	Assert(MULTIPLAYER_MASTER);

	// build the packet and add the opcode
	BUILD_HEADER(EMP_EFFECT);
	ADD_USHORT(net_sig);
	ADD_FLOAT(intensity);
	ADD_FLOAT(time);

	// send it to the player		
	multi_io_send_to_all(data, packet_size);
}

void process_emp_effect(ubyte *data, header *hinfo)
{
	float intensity, time;
	ushort net_sig;
	object *objp;
	int offset = HEADER_LENGTH;

	// read in the EMP effect data
	GET_USHORT(net_sig);
	GET_FLOAT(intensity);
	GET_FLOAT(time);
	PACKET_SET_SIZE();

	// try and find the object
	objp = multi_get_network_object(net_sig);
	if((objp != NULL) && (objp->type == OBJ_SHIP)){		
		// if i'm not an observer and I have a valid ship, play the EMP effect
		if(!(Net_player->flags & NETINFO_FLAG_OBSERVER) && (Player_obj != NULL) && (Player_obj->type == OBJ_SHIP) && (Player_obj == objp)){
			emp_start_local(intensity, time);
		}

		// start the effect for the ship itself
		emp_start_ship(objp, intensity, time);
	}
}

// tells whether or not reinforcements are available
void send_reinforcement_avail( int rnum )
{
	ubyte data[25];
	int packet_size;

	BUILD_HEADER(REINFORCEMENT_AVAIL);
	ADD_INT( rnum );	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_reinforcement_avail( ubyte *data, header *hinfo )
{
	int offset;
	int rnum;

	offset = HEADER_LENGTH;
	GET_INT( rnum );
	PACKET_SET_SIZE();

	// sanity check for a valid reinforcement number
	if ( (rnum >= 0) && (rnum < Num_reinforcements) ) {
		Reinforcements[rnum].flags |= RF_IS_AVAILABLE;
	}
}

void send_change_iff_packet(ushort net_signature, int new_team)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	if(Net_player == NULL){
		return;
	}
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		return;
	}

	// build the packet and add the data
	BUILD_HEADER(CHANGE_IFF);
	ADD_USHORT(net_signature);
	ADD_INT(new_team);

	// send to all players	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_change_iff_packet( ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	ushort net_signature;
	int new_team;	
	object *objp;

	// get the data
	GET_USHORT(net_signature);
	GET_INT(new_team);
	PACKET_SET_SIZE();

	// lookup the object
	objp = multi_get_network_object(net_signature);
	if((objp != NULL) && (objp->type == OBJ_SHIP) && (objp->instance >=0)){
		Ships[objp->instance].team = new_team;
	}	
}

void send_change_iff_color_packet(ushort net_signature, int observer_team, int observed_team, int alternate_iff_color)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	if(Net_player == NULL){
		return;
	}
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		return;
	}

	// build the packet and add the data
	BUILD_HEADER(CHANGE_IFF_COLOR);
	ADD_USHORT(net_signature);
	ADD_INT(observer_team);
	ADD_INT(observed_team);
	ADD_INT(alternate_iff_color);

	// send to all players	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_change_iff_color_packet( ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	ushort net_signature;
	int observer_team, observed_team, alternate_iff_color;	
	object *objp;

	// get the data
	GET_USHORT(net_signature);
	GET_INT(observer_team);
	GET_INT(observed_team);
	GET_INT(alternate_iff_color);
	PACKET_SET_SIZE();

	// lookup the object
	objp = multi_get_network_object(net_signature);
	if((objp != NULL) && (objp->type == OBJ_SHIP) && (objp->instance >=0))
	{
		Ships[objp->instance].ship_iff_color[observer_team][observed_team] = alternate_iff_color;
	}	
}

void send_change_ai_class_packet(ushort net_signature, char *subsystem, int new_ai_class)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	if(Net_player == NULL){
		return;
	}
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		return;
	}

	// build the packet and add the data
	BUILD_HEADER(CHANGE_AI_CLASS);
	ADD_USHORT(net_signature);
	if (subsystem)
		ADD_STRING(subsystem);
	else
		ADD_STRING(NO_SUBSYS_STRING);
	ADD_INT(new_ai_class);

	// send to all players	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_change_ai_class_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	ushort net_signature;
	int new_ai_class;
	char subsys_buf[255];
	object *objp;

	// get the data
	GET_USHORT(net_signature);
	GET_STRING(subsys_buf);
	GET_INT(new_ai_class);
	PACKET_SET_SIZE();

	// lookup the object
	objp = multi_get_network_object(net_signature);
	if((objp != NULL) && (objp->type == OBJ_SHIP) && (objp->instance >=0))
	{
		// no subsystem?
		if (!strcmp(subsys_buf, NO_SUBSYS_STRING))
		{
			ship_set_new_ai_class(objp->instance, new_ai_class);
		}
		// subsystem
		else
		{
			ship_subsystem_set_new_ai_class(objp->instance, subsys_buf, new_ai_class);
		}
	}	
}

void send_NEW_primary_fired_packet(ship *shipp, int banks_fired)
{
	int packet_size, objnum;
	ubyte data[MAX_PACKET_SIZE]; // ubanks_fired, current_bank;
	object *objp;	
	int np_index;
	net_player *ignore = NULL;

	// get an object pointer for this ship.
	objnum = shipp->objnum;
	objp = &Objects[objnum];

	// if i'm a multiplayer client, I should never send primary fired packets for anyone except me
	if(MULTIPLAYER_CLIENT && (Player_obj != objp)){
		return;
	}

	// just in case nothing got fired
	if(banks_fired <= 0){
		return;
	}

	// ubanks_fired = (ubyte)banks_fired;
	// current_bank = (ubyte)shipp->weapons.current_primary_bank;
	// Assert( current_bank <= 3 );

	// insert the current primary bank into this byte
	// ubanks_fired |= (current_bank << CURRENT_BANK_BIT);

	// append the SF_PRIMARY_LINKED flag on the top nibble of the banks_fired
	// if ( shipp->flags & SF_PRIMARY_LINKED ){
		// ubanks_fired |= (1<<7);
	// }	

	if(MULTIPLAYER_MASTER){
		np_index = multi_find_player_by_net_signature(objp->net_signature);
		if((np_index >= 0) && (np_index < MAX_PLAYERS)){
			ignore = &Net_players[np_index];
		}
	}

	// build up the standard weapon fired packet.  This packet will get sent to all players if an AI
	// ship fired the primary weapons.  If a player fired the weapon, then this packet will get sent
	// to every player but the guy who actullly fired the weapon.  This method is used to help keep client
	// and server in sync w.r.t. weapon energy for player ship
	BUILD_HEADER( PRIMARY_FIRED_NEW );
	ADD_USHORT(objp->net_signature);
	// ADD_DATA(ubanks_fired);

	// if I'm a server, broadcast to all players
	if(MULTIPLAYER_MASTER){		
		multi_io_send_to_all(data, packet_size, ignore);

		// TEST CODE
		multi_rate_add(1, "wfi", packet_size);
	}
	// otherwise just send to the server
	else {
		multi_io_send(Net_player, data, packet_size);		
	}
}

void process_NEW_primary_fired_packet(ubyte *data, header *hinfo)
{
	int offset; // linked;	
	// ubyte banks_fired, current_bank;
	object* objp;	
	ship *shipp;
	ushort shooter_sig;	

	// read all packet info
	offset = HEADER_LENGTH;
	GET_USHORT(shooter_sig);
	// GET_DATA(banks_fired);
	PACKET_SET_SIZE();

	// find the object this fired packet is operating on
	objp = multi_get_network_object( shooter_sig );
	if ( objp == NULL ) {
		nprintf(("Network", "Could not find ship for fire primary packet NEW!"));
		return;
	}
	// if this object is not actually a valid ship, don't do anything
	if(objp->type != OBJ_SHIP){
		return;
	}
	// Juke - also check (objp->instance >= MAX_SHIPS)
	if(objp->instance < 0 || objp->instance >= MAX_SHIPS){
		return;
	}
	shipp = &Ships[objp->instance];
	
	// get the link status of the primary banks
	// linked = 0;
	// if ( banks_fired & (1<<7) ) {
		// linked = 1;
		// banks_fired ^= (1<<7);
	// }

	// get the current primary bank
	// current_bank = (ubyte)(banks_fired >> CURRENT_BANK_BIT);
	// current_bank &= 0x3;
	// Assert( (current_bank >= 0) && (current_bank < MAX_SHIP_PRIMARY_BANKS) );
	// shipp->weapons.current_primary_bank = current_bank;

	// strip off all remaining bits and just keep which banks were actually fired.
	// banks_fired &= 0x3;
	
	// set the link status of the ship if not the player.  If it is the player, we will do sanity checking
	// only (for now).	
	// if ( !linked ){
// 		shipp->flags &= ~SF_PRIMARY_LINKED;
	// } else {
		// shipp->flags |= SF_PRIMARY_LINKED;
	// }

	// if we're in client firing mode, ignore ones for myself	
	if((Player_obj != NULL) && (Player_obj == objp)){		
		return;
	}
		
	ship_fire_primary( objp, 0, 1 );

	// Karajorma - It's still a hack but at least this way it only affects AI ships
	if (!(objp->flags & OF_PLAYER_SHIP))
	{
		// Juke - this is the hackiest hack, but hopefully it will fix stream weapon 
		// notifications generated by AI ships.
		uint flags = shipp->flags & SF_TRIGGER_DOWN;

		shipp->flags |= SF_TRIGGER_DOWN;
		ship_fire_primary( objp, 1, 1 );
		shipp->flags &= flags | ~SF_TRIGGER_DOWN;
	}
}

void send_NEW_countermeasure_fired_packet(object *objp, int cmeasure_count, int rand_val)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size;
	int np_index;	
	net_player *ignore = NULL;

	// if i'm a multiplayer client, I should never send primary fired packets for anyone except me
	if(MULTIPLAYER_CLIENT && (Player_obj != objp)){
		return;
	}

	Assert ( cmeasure_count < UCHAR_MAX );
	BUILD_HEADER(COUNTERMEASURE_NEW);
	ADD_USHORT( objp->net_signature );
	ADD_INT( rand_val );

	nprintf(("Network","Sending NEW countermeasure packet!\n"));

	// determine if its a player
	if(MULTIPLAYER_MASTER){
		np_index = multi_find_player_by_net_signature(objp->net_signature);
		if((np_index >= 0) && (np_index < MAX_PLAYERS)){
			ignore = &Net_players[np_index];
		}
	}
	
	// if I'm the server, send to all players
	if(MULTIPLAYER_MASTER){			
		multi_io_send_to_all(data, packet_size, ignore);
	} 
	// otherwise send to the server
	else {
		multi_io_send(Net_player, data, packet_size);
	}
}

void process_NEW_countermeasure_fired_packet(ubyte *data, header *hinfo)
{
	int offset;
	ushort signature;
	int rand_val;
	object *objp;	

	offset = HEADER_LENGTH;
	GET_USHORT( signature );
	GET_INT( rand_val );
	PACKET_SET_SIZE();

	objp = multi_get_network_object( signature );
	if ( objp == NULL ) {
		nprintf(("network", "Could find object whose countermeasures are being launched!!!\n"));
		return;
	}
	if(objp->type != OBJ_SHIP){
		return;
	}	

	// if we're in client firing mode, ignore ones for myself	
	if((Player_obj != NULL) && (Player_obj == objp)){		
		return;
	}

	if ( (rand_val >= NPERM_SIG_MIN) && (rand_val <= NPERM_SIG_MAX) ) {
		multi_set_network_signature((ushort)rand_val, MULTI_SIG_NON_PERMANENT);
	}

	// make it so ship can fire right away!
	Ships[objp->instance].cmeasure_fire_stamp = timestamp(0);
	if ( objp == Player_obj ){		
		nprintf(("network", "firing countermeasure from my ship\n"));
	}
	ship_launch_countermeasure( objp, rand_val );			
}

void send_beam_fired_packet(object *shooter, ship_subsys *turret, object *target, int beam_info_index, beam_info *override, int bfi_flags, int bank_point)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;	
	short u_beam_info;
	char subsys_index;
	beam_info b_info;
	ushort target_sig;

	// only the server should ever be doing this
	Assert(MULTIPLAYER_MASTER);

	// setup outgoing data
	Assert(shooter != NULL);
	Assert(turret != NULL);
	Assert(override != NULL);
	if((shooter == NULL) || (turret == NULL) || (override == NULL)){
		return;
	}

	if (!(bfi_flags & BFIF_IS_FIGHTER_BEAM)) {
		Assert(target != NULL);
		if (target == NULL) {
			return;
		}
	}

	target_sig = (target) ? target->net_signature : 0;

	u_beam_info = (short)beam_info_index;

	if (bfi_flags & BFIF_IS_FIGHTER_BEAM) {
		Assert( (bank_point >= 0) && (bank_point < UCHAR_MAX) );
		subsys_index = (char)bank_point;
	} else {
		subsys_index = (char)ship_get_index_from_subsys(turret, OBJ_INDEX(shooter), 1);
	}

	Assert(subsys_index >= 0);
	if (subsys_index < 0) {
		return;
	}

	// swap the beam_info override info into little endian byte order
	b_info.dir_a.xyz.x = INTEL_FLOAT(&override->dir_a.xyz.x);
	b_info.dir_a.xyz.y = INTEL_FLOAT(&override->dir_a.xyz.y);
	b_info.dir_a.xyz.z = INTEL_FLOAT(&override->dir_a.xyz.z);

	b_info.dir_b.xyz.x = INTEL_FLOAT(&override->dir_b.xyz.x);
	b_info.dir_b.xyz.y = INTEL_FLOAT(&override->dir_b.xyz.y);
	b_info.dir_b.xyz.z = INTEL_FLOAT(&override->dir_b.xyz.z);

	b_info.delta_ang = INTEL_FLOAT(&override->delta_ang);
	b_info.shot_count = override->shot_count;

	for (int i = 0; i < b_info.shot_count; i++) {
		b_info.shot_aim[i] = INTEL_FLOAT(&override->shot_aim[i]);
	}

	// build the header
	BUILD_HEADER(BEAM_FIRED);
	ADD_USHORT(shooter->net_signature);
	ADD_DATA(subsys_index);
	ADD_USHORT(target_sig);
	ADD_SHORT(u_beam_info);
	ADD_DATA(b_info);  // FIXME: This is still wrong, we shouldn't be sending an entire struct over the wire - taylor
//	ADD_DATA(bfi_flags);	// this breaks the protocol but is here in case we decided to do that in the future - taylor
//	ADD_DATA(target_pos);	// ditto - Goober5000

	// send to all clients	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_beam_fired_packet(ubyte *data, header *hinfo)
{
	int i, offset;
	ushort shooter_sig, target_sig;
	char subsys_index;
	short u_beam_info;
	beam_info b_info;
	beam_fire_info fire_info;
//	ubyte fighter_beam = 0;

	// only clients should ever get this
	Assert(MULTIPLAYER_CLIENT);

	// read in packet data
	offset = HEADER_LENGTH;
	GET_USHORT(shooter_sig);
	GET_DATA(subsys_index);
	GET_USHORT(target_sig);
	GET_SHORT(u_beam_info);
	GET_DATA(b_info);  // FIXME: This is still wrong, we shouldn't be sending an entire struct over the wire - taylor
//	GET_DATA(fighter_beam);  // this breaks the protocol but is here in case we decided to do that in the future - taylor
	PACKET_SET_SIZE();

	// swap the beam_info override info into native byte order
	b_info.dir_a.xyz.x = INTEL_FLOAT(&b_info.dir_a.xyz.x);
	b_info.dir_a.xyz.y = INTEL_FLOAT(&b_info.dir_a.xyz.y);
	b_info.dir_a.xyz.z = INTEL_FLOAT(&b_info.dir_a.xyz.z);
	
	b_info.dir_b.xyz.x = INTEL_FLOAT(&b_info.dir_b.xyz.x);
	b_info.dir_b.xyz.y = INTEL_FLOAT(&b_info.dir_b.xyz.y);
	b_info.dir_b.xyz.z = INTEL_FLOAT(&b_info.dir_b.xyz.z);
	
	b_info.delta_ang = INTEL_FLOAT(&b_info.delta_ang);
	
	for (i = 0; i < b_info.shot_count; i++) {
		b_info.shot_aim[i] = INTEL_FLOAT(&b_info.shot_aim[i]);
	}

	memset(&fire_info, 0, sizeof(beam_fire_info));

	// lookup all relevant data
	fire_info.beam_info_index = (int)u_beam_info;
	fire_info.shooter = NULL;
	fire_info.target = NULL;
	fire_info.turret = NULL;
	fire_info.target_subsys = NULL;
	fire_info.beam_info_override = NULL;		
	fire_info.shooter = multi_get_network_object(shooter_sig);
	fire_info.target = multi_get_network_object(target_sig);
	fire_info.beam_info_override = &b_info;
	fire_info.accuracy = 1.0f;

	if((fire_info.shooter == NULL) || (fire_info.shooter->type != OBJ_SHIP) || (fire_info.shooter->instance < 0) || (fire_info.shooter->instance > MAX_SHIPS)){
		nprintf(("Network", "Couldn't get shooter info for BEAM weapon!\n"));
		return;
	}

	ship *shipp = &Ships[fire_info.shooter->instance];

	// this check is a little convoluted but should cover all bases until we decide to just break the protocol
	if ( Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER) ) {
		// make sure the beam is a primary weapon and not attached to a turret or something
		for (i = 0; i < shipp->weapons.num_primary_banks; i++) {
			if ( shipp->weapons.primary_bank_weapons[i] == fire_info.beam_info_index ) {
				fire_info.bfi_flags |= BFIF_IS_FIGHTER_BEAM;
			}
		}
	}

	if ( !(fire_info.bfi_flags & BFIF_IS_FIGHTER_BEAM) && (fire_info.target == NULL) ) {
		nprintf(("Network", "Couldn't get target info for BEAM weapon!\n"));
		return;
	}

	if (fire_info.bfi_flags & BFIF_IS_FIGHTER_BEAM) {
		polymodel *pm = model_get( Ship_info[shipp->ship_info_index].model_num );
		float field_of_fire = Weapon_info[fire_info.beam_info_index].field_of_fire;

		int bank = (ubyte)subsys_index % 10;
		int point = (ubyte)subsys_index / 10;

		fire_info.targeting_laser_offset = pm->gun_banks[bank].pnt[point];

		shipp->beam_sys_info.turret_norm.xyz.x = 0.0f;
		shipp->beam_sys_info.turret_norm.xyz.y = 0.0f;
		shipp->beam_sys_info.turret_norm.xyz.z = 1.0f;
		shipp->beam_sys_info.model_num = Ship_info[shipp->ship_info_index].model_num;
		shipp->beam_sys_info.turret_gun_sobj = pm->detail[0];
		shipp->beam_sys_info.turret_num_firing_points = 1;
		shipp->beam_sys_info.turret_fov = (float)cos((field_of_fire != 0.0f) ? field_of_fire : 180);
		shipp->beam_sys_info.pnt = fire_info.targeting_laser_offset;
		shipp->beam_sys_info.turret_firing_point[0] = fire_info.targeting_laser_offset;

		shipp->fighter_beam_turret_data.disruption_timestamp = timestamp(0);
		shipp->fighter_beam_turret_data.turret_next_fire_pos = 0;
		shipp->fighter_beam_turret_data.current_hits = 1.0;
		shipp->fighter_beam_turret_data.system_info = &shipp->beam_sys_info;

		fire_info.turret = &shipp->fighter_beam_turret_data;
		fire_info.bank = bank;
	} else {
		fire_info.turret = ship_get_indexed_subsys(shipp, (int)subsys_index);

		if (fire_info.turret == NULL) {
			nprintf(("Network", "Couldn't get turret for BEAM weapon!\n"));
			return;
		}
	}

	// fire da beam
	beam_fire(&fire_info);
}

void send_sw_query_packet(ubyte code, char *txt)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	// build the packet and add the code
	BUILD_HEADER(SW_STD_QUERY);
	ADD_DATA(code);
	if((code == SW_STD_START) || (code == SW_STD_BAD)){		
		ADD_STRING(txt);
	}

	// if I'm the host, send to standalone
	if(MULTIPLAYER_HOST){
		Assert(!MULTIPLAYER_MASTER);
		Assert(code == SW_STD_START);		
		multi_io_send_reliable(Net_player, data, packet_size);
	}
	// otherwise standalone sends back to host
	else {
		Assert(Game_mode & GM_STANDALONE_SERVER);
		Assert(code != SW_STD_START);
		Assert(Netgame.host != NULL);
		if(Netgame.host != NULL){			
			multi_io_send_reliable(Netgame.host, data, packet_size);
		}
	}
}

void process_sw_query_packet(ubyte *data, header *hinfo)
{	
}

void send_event_update_packet(int event)
{
	ubyte data[MAX_PACKET_SIZE];
	ushort u_event = (ushort)event;
	int packet_size = 0;

	// build the header and add the event
	BUILD_HEADER(EVENT_UPDATE);
	ADD_USHORT(u_event);
	ADD_INT(Mission_events[event].flags);
	ADD_INT(Mission_events[event].formula);
	ADD_INT(Mission_events[event].result);
	ADD_INT(Mission_events[event].count);

	// send to all players	
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_event_update_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	int store_flags;
	ushort u_event;
	
	// get the data
	GET_USHORT(u_event);
	store_flags = Mission_events[u_event].flags;
	GET_INT(Mission_events[u_event].flags);
	GET_INT(Mission_events[u_event].formula);
	GET_INT(Mission_events[u_event].result);
	GET_INT(Mission_events[u_event].count);
	PACKET_SET_SIZE();

	// went from non directive special to directive special
	if(!(store_flags & MEF_DIRECTIVE_SPECIAL) && (Mission_events[u_event].flags & MEF_DIRECTIVE_SPECIAL)){
		mission_event_set_directive_special(u_event);
	}
	// if we went directive special to non directive special
	else if((store_flags & MEF_DIRECTIVE_SPECIAL) && !(Mission_events[u_event].flags & MEF_DIRECTIVE_SPECIAL)){
		mission_event_unset_directive_special(u_event);
	}	
}

void send_weapon_or_ammo_changed_packet (int ship_index, int bank_type, int bank_number, int ammo_left, int rearm_limit, int new_weapon_index)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	if(Net_player == NULL){
		return;
	}
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		return;
	}

	// build the packet and add the data
	BUILD_HEADER(WEAPON_OR_AMMO_CHANGED);
	ADD_INT(ship_index);
	ADD_INT(bank_type);
	ADD_INT(bank_number);
	ADD_INT(ammo_left);
	ADD_INT(rearm_limit);
	ADD_INT(new_weapon_index);

	//Send it to the player whose weapons have changes
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_weapon_or_ammo_changed_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	int ship_index, bank_type, bank_number, ammo_left, rearm_limit, new_weapon_index;
	ship *shipp;

	// get the data
	GET_INT(ship_index);
	GET_INT(bank_type);
	GET_INT(bank_number);
	GET_INT(ammo_left);
	GET_INT(rearm_limit);
	GET_INT(new_weapon_index);
	PACKET_SET_SIZE();

	// Now set the ships values up. 
	
	//Primary weapons
	if (bank_type == 0) 
	{
		// don't swap weapons
		if (new_weapon_index < 0)
		{
			set_primary_ammo(ship_index, bank_number, ammo_left, rearm_limit);
		}
		else 
		{
			Assert (new_weapon_index < MAX_WEAPON_TYPES);
			shipp = &Ships[ship_index];
			shipp->weapons.primary_bank_weapons[bank_number] = new_weapon_index;
			set_primary_ammo(Player_obj->instance, bank_number, ammo_left, rearm_limit);
		}
	}
	// Secondary weapons
	else if (bank_type == 1)
	{
		// don't swap weapons
		if (new_weapon_index < 0)
		{
			set_secondary_ammo(ship_index, bank_number, ammo_left, rearm_limit);
		}
		else 
		{
			Assert (new_weapon_index < MAX_WEAPON_TYPES);
			shipp = &Ships[ship_index];
			shipp->weapons.secondary_bank_weapons[bank_number] = new_weapon_index;
			set_secondary_ammo(Player_obj->instance, bank_number, ammo_left, rearm_limit);
		}
	}
	else
	{
		nprintf(("network", "weapon_or_ammo_changed_packet recived for tertiary or other unsupported type\n"));
		return;
	}
}

// Karajorma - Sends a packet to all clients telling them that a SEXP variable has changed its value
void send_variable_update_packet(int variable_index, char *value)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	if(Net_player == NULL){
		return;
	}
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		return;
	}

	// build the packet and add the data
	BUILD_HEADER(VARIABLE_UPDATE);
	ADD_INT(variable_index);
	ADD_STRING(value);

	if (MULTIPLAYER_MASTER) { 
		// send to all players	
		multi_io_send_to_all_reliable(data, packet_size);
	}
	else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

void process_variable_update_packet( ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	char value[TOKEN_LENGTH];
	int variable_index;

	// get the data
	GET_INT(variable_index);
	GET_STRING(value);
	PACKET_SET_SIZE();

	// set the sexp_variable
	if ( (variable_index >= 0) && (variable_index < sexp_variable_count()) )
	{
		strcpy_s(Sexp_variables[variable_index].text, value); 
	}	

	// send the packet on to all clients. 
	if (MULTIPLAYER_MASTER) {
		send_variable_update_packet(variable_index, Sexp_variables[variable_index].text);
	}
}

// weapon detonate packet
void send_weapon_detonate_packet(object *objp)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	// sanity checks
	Assert(MULTIPLAYER_MASTER);
	if(!MULTIPLAYER_MASTER){
		return;
	}
	Assert(objp != NULL);
	if(objp == NULL){
		return;
	}

	// build the header and add the data
	BUILD_HEADER(WEAPON_DET);
	ADD_USHORT(objp->net_signature);

	// send to all players
	multi_io_send_to_all(data, packet_size);
}

void process_weapon_detonate_packet(ubyte *data, header *hinfo)
{
	ushort net_sig;
	int offset = HEADER_LENGTH;
	object *objp = NULL;

	// get the weapon signature
	GET_USHORT(net_sig);
	PACKET_SET_SIZE();

	// lookup the weapon
	objp = multi_get_network_object(net_sig);
	if((objp != NULL) && (objp->type == OBJ_WEAPON) && (objp->instance >= 0)){
		weapon_detonate(objp);
	}
}	

// flak fired packet
void send_flak_fired_packet(int ship_objnum, int subsys_index, int weapon_objnum, float flak_range)
{
	int packet_size;
	ushort pnet_signature;
	ubyte data[MAX_PACKET_SIZE], cindex;
	object *objp;	
	ship_subsys *ssp;
	short val;

	// sanity
	if((weapon_objnum < 0) || (Objects[weapon_objnum].type != OBJ_WEAPON) || (Objects[weapon_objnum].instance < 0) || (Weapons[Objects[weapon_objnum].instance].weapon_info_index < 0)){
		return;
	}

	// local setup -- be sure we are actually passing a weapon!!!!
	objp = &Objects[weapon_objnum];
	Assert ( objp->type == OBJ_WEAPON );	
	pnet_signature = Objects[ship_objnum].net_signature;

	Assert( subsys_index < UCHAR_MAX );
	cindex = (ubyte)subsys_index;

	ssp = ship_get_indexed_subsys( &Ships[Objects[ship_objnum].instance], subsys_index, NULL );
	if(ssp == NULL){
		return;
	}

	// build the fire turret packet.  
	BUILD_HEADER(FLAK_FIRED);	
	packet_size += multi_pack_unpack_position(1, data + packet_size, &objp->orient.vec.fvec);	
	ADD_USHORT( pnet_signature );		
	ADD_DATA( cindex );
	val = (short)ssp->submodel_info_1.angs.h;
	ADD_SHORT( val );
	val = (short)ssp->submodel_info_2.angs.p;
	ADD_SHORT( val );	
	ADD_FLOAT( flak_range );
	
	multi_io_send_to_all(data, packet_size);

	multi_rate_add(1, "flk", packet_size);
}

void process_flak_fired_packet(ubyte *data, header *hinfo)
{
	int offset, weapon_objnum, wid = -1;
	ushort pnet_signature;
	vec3d pos, dir;
	matrix orient;
	vec3d o_fvec;
	ubyte turret_index;
	object *objp;
	ship_subsys *ssp;	
	ship *shipp;
	short pitch, heading;
	float flak_range;

	// get the data for the turret fired packet
	offset = HEADER_LENGTH;		
	offset += multi_pack_unpack_position(0, data + offset, &o_fvec);
	GET_USHORT( pnet_signature );
	GET_DATA( turret_index );
	GET_SHORT( heading );
	GET_SHORT( pitch );	
	GET_FLOAT( flak_range );
	PACKET_SET_SIZE();				// move our counter forward the number of bytes we have read

	// find the object
	objp = multi_get_network_object( pnet_signature );
	if ( objp == NULL ) {
		nprintf(("network", "could find parent object with net signature %d for flak firing\n", pnet_signature));
		return;
	}

	// if this isn't a ship, do nothing
	if ( objp->type != OBJ_SHIP ){
		return;
	}

	// make an orientation matrix from the o_fvec
	vm_vector_2_matrix(&orient, &o_fvec, NULL, NULL);

	// find this turret, and set the position of the turret that just fired to be where it fired.  Quite a
	// hack, but should be suitable.
	shipp = &Ships[objp->instance];
	ssp = ship_get_indexed_subsys( shipp, turret_index, NULL );
	if(ssp == NULL){
		return;
	}

	if (ssp->weapons.num_primary_banks > 0) {
		wid = ssp->weapons.primary_bank_weapons[0];
	} else if (ssp->weapons.num_secondary_banks > 0) {
		wid = ssp->weapons.secondary_bank_weapons[0];
	}

	if((wid < 0) || !(Weapon_info[wid].wi_flags & WIF_FLAK)){
		return;
	}

	// bash the position and orientation of the turret
	ssp->submodel_info_1.angs.h = (float)heading;
	ssp->submodel_info_2.angs.p = (float)pitch;

	// get the world position of the weapon
	ship_get_global_turret_info(objp, ssp->system_info, &pos, &dir);

	// create the weapon object	
	weapon_objnum = weapon_create( &pos, &orient, wid, OBJ_INDEX(objp), -1, 1);
	if (weapon_objnum != -1) {
		if ( Weapon_info[wid].launch_snd != -1 ) {
			snd_play_3d( &Snds[Weapon_info[wid].launch_snd], &pos, &View_position );
		}

		// create a muzzle flash from a flak gun based upon firing position and weapon type
		flak_muzzle_flash(&pos, &dir, &objp->phys_info, wid);

		// set its range explicitly - make it long enough so that it's guaranteed to still exist when the server tells us it blew up
		flak_set_range(&Objects[weapon_objnum], (float)flak_range);
	}
}

#define ADD_NORM_VEC(d) do { Assert((packet_size + 3) < MAX_PACKET_SIZE); char vnorm[3] = { (char)(d.x * 127.0f), (char)(d.y * 127.0f), (char)(d.z * 127.0f) }; memcpy(data + packet_size, vnorm, 3); packet_size += 3; } while(0);
#define GET_NORM_VEC(d) do { char vnorm[3]; memcpy(vnorm, data+offset, 3); d.x = (float)vnorm[0] / 127.0f; d.y = (float)vnorm[1] / 127.0f; d.z = (float)vnorm[2] / 127.0f; } while(0);

// player pain packet
void send_player_pain_packet(net_player *pl, int weapon_info_index, float damage, vec3d *force, vec3d *hitpos)
{
	ubyte data[MAX_PACKET_SIZE];
	short windex;
	ushort udamage;
	int packet_size = 0;

	Assert(MULTIPLAYER_MASTER);
	if(!MULTIPLAYER_MASTER){
		return;
	}
	Assert(pl != NULL);
	if(pl == NULL){
		return;
	}

	// build the packet and add the code
	BUILD_HEADER(NETPLAYER_PAIN);
	windex = (short)weapon_info_index;
	ADD_SHORT(windex);
	udamage = (ushort)damage;
	ADD_USHORT(udamage);
	ADD_VECTOR((*force));
	ADD_VECTOR((*hitpos));

	// send to the player
	multi_io_send(pl, data, packet_size);

	multi_rate_add(1, "pai", packet_size);
}	

void process_player_pain_packet(ubyte *data, header *hinfo)
{
	int offset;
	short windex = 0;
	ushort udamage;
	vec3d force;
	vec3d local_hit_pos;
	weapon_info *wip;

	// get the data for the pain packet
	offset = HEADER_LENGTH;		
	GET_SHORT(windex);
	GET_USHORT(udamage);
	GET_VECTOR(force);
	GET_VECTOR(local_hit_pos);
	PACKET_SET_SIZE();

	// mprintf(("PAIN!\n"));

	// get weapon info pointer
	Assert((windex < Num_weapon_types) && (Weapon_info[windex].subtype == WP_LASER));
	if(! ((windex < Num_weapon_types) && (Weapon_info[windex].subtype == WP_LASER)) ){
		return;
	}
	wip = &Weapon_info[windex];

	// play the weapon hit sound
	Assert(Player_obj != NULL);
	if(Player_obj == NULL){
		return;
	}
	
	//Assume the weapon is armed -WMC
	weapon_hit_do_sound(Player_obj, wip, &Player_obj->pos, true);

	// we need to do 3 things here. player pain (game flash), weapon hit sound, ship_apply_whack()
	ship_hit_pain((float)udamage);

	// apply the whack	
	ship_apply_whack(&force, &local_hit_pos, Player_obj);	
}

// lightning packet
void send_lightning_packet(int bolt_type, vec3d *start, vec3d *strike)
{
	ubyte data[MAX_PACKET_SIZE];
	char val;
	int packet_size = 0;

	// build the header and add the data
	BUILD_HEADER(LIGHTNING_PACKET);
	val = (char)bolt_type;
	ADD_DATA(val);
	ADD_VECTOR((*start));
	ADD_VECTOR((*strike));

	// send to everyone unreliable for now
	multi_io_send_to_all(data, packet_size);
}

void process_lightning_packet(ubyte *data, header *hinfo)
{
	int offset;
	char bolt_type;
	vec3d start, strike;

	// read the data
	offset = HEADER_LENGTH;
	GET_DATA(bolt_type);
	GET_VECTOR(start);
	GET_VECTOR(strike);
	PACKET_SET_SIZE();

	// invalid bolt?
	if(bolt_type < 0){
		return;
	}

	// fire it up
	nebl_bolt(bolt_type, &start, &strike);
}

void send_bytes_recvd_packet(net_player *pl)
{
	// only clients should ever be doing this
	if(pl == NULL){
		return;
	}	

	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;
	BUILD_HEADER(BYTES_SENT);
	ADD_INT(pl->cl_bytes_recvd);

	// send to the server
	multi_io_send_reliable(pl, data, packet_size);
}

void process_bytes_recvd_packet(ubyte *data, header *hinfo)
{
	int bytes;
	int pid;
	net_player *pl = NULL;
	int offset = HEADER_LENGTH;
	
	GET_INT(bytes);
	PACKET_SET_SIZE();

	// not server?
	if(Net_player == NULL){
		return;
	}
	if(!MULTIPLAYER_MASTER){
		return;
	}

	// make sure we know what player sent this
	pid = find_player_id(hinfo->id);
	if((pid < 0) || (pid >= MAX_PLAYERS)){
		return;
	}
	pl = &Net_players[pid];

	// compute his pl
	pl->cl_bytes_recvd = bytes;
	if(bytes < 0){
		return;
	}
	pl->sv_last_pl = (int)(100.0f * (1.0f - ((float)pl->cl_bytes_recvd / (float)pl->sv_bytes_sent)));

	// reset bytes sent
	pl->sv_bytes_sent = 0;
}

// host transfer
void send_host_captain_change_packet(short player_id, int captain_change)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	// build the packet
	BUILD_HEADER(TRANSFER_HOST);
	ADD_SHORT(player_id);
	ADD_INT(captain_change);

	// send to all
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_host_captain_change_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	int idx, found_player, captain_change;
	short player_id;

	// get the player id
	GET_SHORT(player_id);
	GET_INT(captain_change);
	PACKET_SET_SIZE();

	// captain change
	if(captain_change){
		// flag the new guy		
		for(idx=0; idx<MAX_PLAYERS; idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].player_id == player_id)){
				HUD_printf("%s is the new captain of team %d", Net_players[idx].m_player->callsign, Net_players[idx].p_info.team + 1);
				break;
			}
		}
	} else {
		// unflag all old players
		for(idx=0; idx<MAX_PLAYERS; idx++){
			Net_players[idx].flags &= ~NETINFO_FLAG_GAME_HOST;
		}

		// flag the new guy
		found_player = 0;
		for(idx=0; idx<MAX_PLAYERS; idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].player_id == player_id)){
				Net_players[idx].flags |= NETINFO_FLAG_GAME_HOST;

				// spew to the HUD config
				if(Net_players[idx].m_player != NULL){
					HUD_printf("%s is the new game host", Net_players[idx].m_player->callsign);
				}

				found_player = 1;
				break;
			}
		}

		// doh
		if(!found_player){
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_HOST_LEFT);
		}
	} 	
}

void send_self_destruct_packet()
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	// bogus
	if(Net_player == NULL){
		return;
	}

	// if i'm the server, I shouldn't be here
	Assert(!(Net_player->flags & NETINFO_FLAG_AM_MASTER));
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		return;
	}
	
	// only if this is valid
	if(MULTI_OBSERVER(Net_players[MY_NET_PLAYER_NUM])){
		return;
	}

	// bogus object?
	if((Player_ship == NULL) || (Player_obj == NULL)){
		return;
	}

	// self destruct
	BUILD_HEADER(SELF_DESTRUCT);
	ADD_USHORT(Player_obj->net_signature);

	// send to the server
	multi_io_send_reliable(Net_player, data, packet_size);
}

void process_self_destruct_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	ushort net_sig;
	int np_index;

	// get the net signature
	GET_USHORT(net_sig);
	PACKET_SET_SIZE();

	// get the player
	np_index = find_player_id(hinfo->id);
	if(np_index < 0){
		return;
	}
	if(MULTI_OBSERVER(Net_players[np_index])){
		return;
	}
	if(Net_players[np_index].m_player == NULL){
		return;
	}
	if((Net_players[np_index].m_player->objnum < 0) || (Net_players[np_index].m_player->objnum >= MAX_OBJECTS)){
		return;
	}
	if(Objects[Net_players[np_index].m_player->objnum].net_signature != net_sig){
		return;
	}
	if(Objects[Net_players[np_index].m_player->objnum].type != OBJ_SHIP){
		return;
	}
	if((Objects[Net_players[np_index].m_player->objnum].instance < 0) || (Objects[Net_players[np_index].m_player->objnum].instance >= MAX_SHIPS)){
		return;
	}

	// do eet
	ship_self_destruct(&Objects[Net_players[np_index].m_player->objnum]);
}

void send_sexp_packet(ubyte *sexp_packet, int num_ubytes)
{
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;
	int i;
	ushort val; 

	Assert (MULTIPLAYER_MASTER);
	// must have a bare minimum of OP, COUNT and TERMINATOR
	if (num_ubytes < 9) {
		Warning(LOCATION, "Invalid call to send_sexp_packet. Not enough data included!"); 
		return; 
	}
	
	BUILD_HEADER(SEXP);

	val = (ushort)num_ubytes;
	ADD_USHORT(val);

	for (i =0; i < num_ubytes; i++) {
		Assert (packet_size < MAX_PACKET_SIZE); 
		data[packet_size] = sexp_packet[i]; 
		packet_size++; 
	}

	// send to all
	multi_io_send_to_all_reliable(data, packet_size);
}

void process_sexp_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	int i;
	ushort num_ubytes;
	ubyte received_packet[MAX_PACKET_SIZE]; 

	// get the number of bytes of data in the packet
	GET_USHORT(num_ubytes);

	for (i=0; i < num_ubytes; i++) {
		GET_DATA(received_packet[i]); 
	}

	PACKET_SET_SIZE();

	sexp_packet_received(received_packet, num_ubytes);
}
