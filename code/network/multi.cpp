/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "playerman/player.h"
#include "mission/missionparse.h"
#include "mission/missioncampaign.h"
#include "gamesequence/gamesequence.h"
#include "freespace2/freespace.h"
#include "osapi/osapi.h"
#include "network/stand_gui.h"
#include "network/multi_xfer.h"
#include "network/multiui.h"
#include "network/multi_ingame.h"
#include "popup/popup.h"
#include "missionui/chatbox.h"
#include "network/multiteamselect.h"
#include "network/multi_data.h"
#include "network/multi_kick.h"
#include "network/multi_campaign.h"
#include "network/multi_voice.h"
#include "network/multi_team.h"
#include "network/multi_respawn.h"
#include "network/multi_pmsg.h"
#include "network/multi_endgame.h"
#include "missionui/missiondebrief.h"
#include "network/multi_pause.h"
#include "mission/missiongoals.h"
#include "network/multi_log.h"
#include "network/multi_rate.h"
#include "hud/hudescort.h"
#include "hud/hudmessage.h"
#include "globalincs/alphacolors.h"
#include "globalincs/pstypes.h"
#include "cfile/cfile.h"
#include "fs2netd/fs2netd_client.h"
#include "pilotfile/pilotfile.h"
#include "debugconsole/console.h"



// ----------------------------------------------------------------------------------------
// Basic module scope defines
//
//

// timestamp defines
#define NETGAME_SEND_TIME								2						// time between sending netgame update packets
#define STATE_SEND_TIME									2						// time between sending netplayer state packets
#define GAMEINFO_SEND_TIME								3						// time between sending game information packets
#define PING_SEND_TIME									2						// time between player pings
#define BYTES_SENT_TIME									5						// every five seconds

// local network buffer stuff
#define MAX_NET_BUFFER									(1024 * 16)			// define and variable declaration for our local tcp buffer
#define NUM_REENTRANT_LEVELS							3

// time (in fixed seconds) to put up dialog about no contect from server
#define MULTI_SERVER_MAX_TIMEOUT						(F1_0 * 4)					// after this number of milliseoncds, stop client simulation
#define MULTI_SERVER_MAX_TIMEOUT_LARGE				(F1_0 * 40)					// done anytime not in mission
#define MULTI_SERVER_WAIT_TIME						(F1_0 * 60)					// wait 60 seconds to reconnect with the server
#define MULTI_SERVER_GONE								1
#define MULTI_SERVER_ALIVE								2

// define for when to show "slow network" icon
#define MULTI_SERVER_SLOW_PING_TIME					700					// when average ping time to server reaches this -- display hud icon

// update times for clients ships based on object update level
#define MULTI_CLIENT_UPDATE_TIME						333

int Multi_display_netinfo = 1;

// ----------------------------------------------------------------------------------------
// Multiplayer vars
//
//

// net player vars		
net_player Net_players[MAX_PLAYERS];							// array of all netplayers in the game
net_player *Net_player;												// pointer to console's net_player entry

// netgame vars
netgame_info Netgame;												// netgame information
int Multi_mission_loaded = 0;										// flag, so that we don't load the mission more than once client side
int Ingame_join_net_signature = -1;								// signature for the player obj for use when joining ingame
int Multi_button_info_ok = 0;										// flag saying it is ok to apply critical button info on a client machine
int Multi_button_info_id = 0;										// identifier of the stored button info to be applying

// low level networking vars
int ADDRESS_LENGTH;													// will be 6 for IPX, 4 for IP
int PORT_LENGTH;														// will be 2 for IPX, 2 for IP
int HEADER_LENGTH;													// 1 byte (packet type)

// misc data
active_game* Active_game_head;									// linked list of active games displayed on Join screen
int Active_game_count;												// for interface screens as well
CFILE* Multi_chat_stream;											// for streaming multiplayer chat strings to a file
int Multi_has_cd = 0;												// if this machine has a cd or not (call multi_common_verify_cd() to set this)
int Multi_connection_speed;										// connection speed of this machine.
int Multi_num_players_at_start = 0;								// the # of players present (kept track of only on the server) at the very start of the mission
short Multi_id_num = 0;												// for assigning player id #'s

// permanent server list
server_item* Game_server_head;								// list of permanent game servers to be querying

// timestamp data
int Netgame_send_time = -1;							// timestamp used to send netgame info to players before misison starts
int State_send_time = -1;								// timestamp used to send state information to the host before a mission starts
int Gameinfo_send_time = -1;							// timestamp used by master to send game information to clients
int Next_ping_time = -1;								// when we should next ping all
int Multi_server_check_count = 0;					// var to keep track of reentrancy when checking server status
int Next_bytes_time = -1;								// bytes sent

// how often each player gets updated
int Multi_client_update_times[MAX_PLAYERS];	// client update packet timestamp

// local network buffer data
LOCAL ubyte net_buffer[NUM_REENTRANT_LEVELS][MAX_NET_BUFFER];
LOCAL ubyte Multi_read_count;

int Multi_restr_query_timestamp = -1;
join_request Multi_restr_join_request;
net_addr Multi_restr_addr;				
int Multi_join_restr_mode = -1;

LOCAL fix Multi_server_wait_start;				// variable to hold start time when waiting to reestablish with server

// non API master tracker vars
char Multi_tracker_login[MULTI_TRACKER_STRING_LEN+1] = "";
char Multi_tracker_passwd[MULTI_TRACKER_STRING_LEN+1] = "";
char Multi_tracker_squad_name[MULTI_TRACKER_STRING_LEN+1] = "";
int Multi_tracker_id = -1;
char Multi_tracker_id_string[255];

// current file checksum
ushort Multi_current_file_checksum = 0;
int Multi_current_file_length = -1;


// -------------------------------------------------------------------------------------------------
//	multi_init() is called only once, at game start-up.  Get player address + port, initialize the
// network players list.
//
//

void multi_init()
{
	int idx;

	// read in config file
	multi_options_read_config();

	Assert( Net_player == NULL );
	Multi_id_num = 0;

	// clear out all netplayers
	memset(Net_players, 0, sizeof(net_player) * MAX_PLAYERS);
	for(idx=0; idx<MAX_PLAYERS; idx++){
		Net_players[idx].reliable_socket = INVALID_SOCKET;
	}

	// initialize the local netplayer
	Net_player = &Net_players[0];	
	Net_player->tracker_player_id = Multi_tracker_id;
	Net_player->m_player = Player;
	Net_player->flags = 0;	
	Net_player->s_info.xfer_handle = -1;
	Net_player->player_id = multi_get_new_id();
	Net_player->client_cinfo_seq = 0;
	Net_player->client_server_seq = 0;		

	// get our connection speed
	Multi_connection_speed = multi_get_connection_speed();			
	
	// initialize other stuff
	multi_log_init();

	// load up common multiplayer icons
	if (!Is_standalone)
		multi_load_common_icons();	
}

// this is an important function which re-initializes any variables required in multiplayer games. 
// Always make sure globals you add are re-initialized here !!!!
void multi_vars_init()
{
	// initialize this variable right away.  Used in game_level_init for init'ing the player
	Next_ship_signature = SHIP_SIG_MIN;		
	Next_asteroid_signature = ASTEROID_SIG_MIN;
	Next_non_perm_signature = NPERM_SIG_MIN;   
	Next_debris_signature = DEBRIS_SIG_MIN;
	
	// server-client critical stuff
	Multi_button_info_ok = 0;
	Multi_button_info_id = 0;

	// Ingame join stuff
	Ingame_join_net_signature = -1;

	// Netgame stuff
	Netgame.game_state = NETGAME_STATE_FORMING;	

	// team select stuff
	Multi_ts_inited = 0;	

	// load send stuff
	Multi_mission_loaded = 0;   // client side		

	// restricted game stuff
	Multi_restr_query_timestamp = -1;	

	// respawn stuff	
	Multi_server_check_count = 0;

	// reentrant variable
	Multi_read_count = 0;

	// unset the "have cd" var
	// NOTE: we unset this here because we are going to be calling multi_common_verify_cd() 
	//       immediately after this (in multi_level_init() to re-check the status)
	Multi_has_cd = 0;

	// current file checksum
	Multi_current_file_checksum = 0;
	Multi_current_file_length = -1;

	Active_game_head = NULL;
	Game_server_head = NULL;

	// only the server should ever care about this
	Multi_id_num = 0;
}

// -------------------------------------------------------------------------------------------------
//	multi_level_init() is called whenever the player starts a multiplayer game
//
//

void multi_level_init() 
{
	int idx;

	// NETLOG
	ml_string(NOX("multi_level_init()"));

	// initialize the Net_players array
	for ( idx = 0; idx < MAX_PLAYERS; idx++) {
		// close all sockets down just for good measure
		psnet_rel_close_socket(&Net_players[idx].reliable_socket);

		memset(&Net_players[idx],0,sizeof(net_player));
		Net_players[idx].reliable_socket = INVALID_SOCKET;

		Net_players[idx].s_info.xfer_handle = -1;
		Net_players[idx].p_info.team = 0;
	}

	// initialize the Players array
	for (idx=0;idx<MAX_PLAYERS;idx++) {
		if (Player == &Players[idx]) {
			continue;
		}
		Players[idx].reset();
	}

	multi_vars_init();	

	// initialize the fake lag/loss system
#ifdef MULTI_USE_LAG
	multi_lag_init();
#endif

	// initialize the kick system
	multi_kick_init();

	// initialize all file xfer stuff
	multi_xfer_init(multi_file_xfer_notify);

	// close the chatbox (if one exists)
	chatbox_close();	

	// reset the data xfer system
	multi_data_reset();

	// initialize the voice system
	multi_voice_init();

	// intialize the pause system
	multi_pause_reset();

	// initialize endgame stuff
	multi_endgame_init();

	// initialize respawning
	multi_respawn_init();

	// initialize all netgame timestamps
	multi_reset_timestamps();

	// flush psnet sockets
	psnet_flush();
}

// multi_check_listen() calls low level psnet routine to see if we have a connection from a client we
// should accept.
void multi_check_listen()
{
	int i;
	net_addr addr;
	PSNET_SOCKET_RELIABLE sock = INVALID_SOCKET;

	// call psnet routine which calls select to see if we need to check for a connect from a client
	// by passing addr, we are telling check_for_listen to do the accept and return who it was from in
	// addr.  The
	sock = psnet_rel_check_for_listen(&addr);
	if ( sock != INVALID_SOCKET ) {
		// be sure that my address and the server address are set correctly.
		if ( !psnet_same(&Psnet_my_addr, &Net_player->p_info.addr) ){
			Net_player->p_info.addr = Psnet_my_addr;
		}

		if ( !psnet_same(&Psnet_my_addr, &(Netgame.server_addr)) ){
			Netgame.server_addr = Psnet_my_addr;
		}

		// the connection was accepted in check_for_listen.  Find the netplayer whose address we connected
		// with and assign the socket descriptor
		for (i = 0; i < MAX_PLAYERS; i++ ) {
			if ( (Net_players[i].flags & NETINFO_FLAG_CONNECTED) && (!memcmp(&(addr.addr), &(Net_players[i].p_info.addr.addr), 6)) ) {
				// mark this flag so we know he's "fully" connected
				Net_players[i].flags |= NETINFO_FLAG_RELIABLE_CONNECTED;
				Net_players[i].reliable_socket = sock;

				// send player information to the joiner
				send_accept_player_data( &Net_players[i], (Net_players[i].flags & NETINFO_FLAG_INGAME_JOIN)?1:0 );

				// send a netgame update so the new client has all the necessary settings
				send_netgame_update_packet();	

				// if this is a team vs. team game, send an update
				if(Netgame.type_flags & NG_TYPE_TEAM){
					multi_team_send_update();
				}

				// NETLOG
				ml_printf(NOX("Accepted TCP connection from %s"), Net_players[i].m_player == NULL ? NOX("Unknown") : Net_players[i].m_player->callsign);				
				break;
			}
		}

		// if we didn't find a player, close the socket
		if ( i == MAX_PLAYERS ) {
			nprintf(("Network", "Got accept on my listen socket, but unknown player.  Closing socket.\n"));
			psnet_rel_close_socket(&sock);
		}
	}
}

// returns true is server hasn't been heard from in N seconds. false otherwise
int multi_client_server_dead()
{
	fix this_time, last_time, max;

	// get the last time we have heard from the server.  If greater than some default, then maybe
	// display some icon on the HUD indicating slow network connection.  if greater than some higher
	// max, stop simulating on the client side until we hear from the server again.
	this_time = timer_get_fixed_seconds();
	last_time = Netgame.server->last_heard_time;
	// check for wrap!  must return 0
	if ( last_time > this_time )
		return 0;

	this_time -= last_time;

	// if in mission, use the smaller timeout value.  Outside of mission, use a large one.
	if ( MULTI_IN_MISSION ){
		max = MULTI_SERVER_MAX_TIMEOUT;
	} else {
		max = MULTI_SERVER_MAX_TIMEOUT_LARGE;
	}

	if ( this_time > max){
		return 1;
	} else {
		return 0;
	}
}

void multi_process_incoming();		// prototype for function later in this module

// function to process network data in hopes of getting info back from server
int multi_client_wait_on_server()
{
	int is_dead;

	is_dead = multi_client_server_dead();

	// if the server is back alive, tell our popup
	if ( !is_dead ){
		return MULTI_SERVER_ALIVE;
	}

	// on release version -- keep popup active for 60 seconds, then bail
#ifdef NDEBUG
	fix this_time = timer_get_fixed_seconds();
	// if the timer wrapped:
	if ( this_time < Multi_server_wait_start ) {
		Multi_server_wait_start = timer_get_fixed_seconds();
		return FALSE;
	}
	// check to see if timeout expired
	this_time -= Multi_server_wait_start;
	if ( this_time > MULTI_SERVER_WAIT_TIME ){
		return MULTI_SERVER_GONE;
	}
#endif

	return FALSE;
}

// function called by multiplayer clients to stop simulating when they have not heard from the server
// in a while.
void multi_client_check_server()
{
	int rval;

	Assert( MULTIPLAYER_CLIENT );	

	// this function can get called while in the popup code below.  So we include this check as a
	// reentrancy check.
	if ( Multi_server_check_count )
		return;

	// make sure we have a valid server
	if(Netgame.server == NULL){
		return;
	}

	Multi_server_check_count++;
	if(multi_client_server_dead()){
		Netgame.flags |= NG_FLAG_SERVER_LOST;
	} else {
		Netgame.flags &= ~(NG_FLAG_SERVER_LOST);
	}

	if(Netgame.flags & NG_FLAG_SERVER_LOST) {
		if(!(Game_mode & GM_IN_MISSION) && !popup_active()){	
			// need to start a popup
			Multi_server_wait_start = timer_get_fixed_seconds();
			rval = popup_till_condition( multi_client_wait_on_server, XSTR("Cancel",641), XSTR("Contact lost with server.  Stopping simulation until contact reestablished.  Press Cancel to exit game.",642) );
			
			if ( !rval || (rval == MULTI_SERVER_GONE) ) {				
				multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_CONTACT_LOST);								
			}
			Netgame.flags &= ~(NG_FLAG_SERVER_LOST);
		}
	}

	Multi_server_check_count--;
}


// -------------------------------------------------------------------------------------------------
//	process_packet_normal() will determine what sort of packet it is, and send it to the appropriate spot.
//	Prelimiary verification of the magic number and checksum are done here.  
//

void process_packet_normal(ubyte* data, header *header_info)
{
	switch ( data[0] ) {

		case JOIN:
			process_join_packet(data, header_info);
			break;

		case GAME_CHAT:
			process_game_chat_packet( data, header_info );
			break;

		case NOTIFY_NEW_PLAYER:
			process_new_player_packet(data, header_info);
			break;

		case HUD_MSG:
			process_hud_message(data, header_info);
			break;

		case MISSION_MESSAGE:
			process_mission_message_packet( data, header_info );
			break;

		case LEAVE_GAME:
			process_leave_game_packet(data, header_info);
			break;

		case GAME_QUERY:
			process_game_query(data, header_info);
			break;

		case GAME_ACTIVE:
			process_game_active_packet(data, header_info);
			break;

		case GAME_INFO:
			process_game_info_packet( data, header_info );
			break;		

		case SECONDARY_FIRED_AI:
			process_secondary_fired_packet(data, header_info, 0);
			break;		

		case SECONDARY_FIRED_PLR:
			process_secondary_fired_packet(data, header_info, 1);
			break;

		case COUNTERMEASURE_FIRED:
			process_countermeasure_fired_packet( data, header_info );
			break;		

		case FIRE_TURRET_WEAPON:
			process_turret_fired_packet( data, header_info );
			break;

		case GAME_UPDATE:
			process_netgame_update_packet( data, header_info );
			break;

		case UPDATE_DESCRIPT:
			process_netgame_descript_packet( data, header_info );
			break;

		case NETPLAYER_UPDATE:
			process_netplayer_update_packet( data, header_info );
			break;

		case ACCEPT :
			process_accept_packet(data, header_info);
			break;				

		case OBJECT_UPDATE:
			multi_oo_process_update(data, header_info);
			break;

		case SHIP_KILL:
			process_ship_kill_packet( data, header_info );
			break;

		case WING_CREATE:
			process_wing_create_packet( data, header_info );
			break;
			
		case SHIP_CREATE:
			process_ship_create_packet( data, header_info );
			break;

		case SHIP_DEPART:
			process_ship_depart_packet( data, header_info );
			break;

		case MISSION_LOG_ENTRY:
			process_mission_log_packet( data, header_info );
			break;		

		case PING:
			process_ping_packet(data, header_info);
			break;

		case PONG:
			process_pong_packet(data, header_info);
			break;		

		case XFER_PACKET:
			Assert(header_info->id >= 0);
			int np_index;
			PSNET_SOCKET_RELIABLE sock;
			sock = INVALID_SOCKET;

			// if I'm the server of the game, find out who this came from			
			if((Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				np_index = find_player_id(header_info->id);
				if(np_index >= 0){
					sock = Net_players[np_index].reliable_socket;
				}
			}
			// otherwise always use my own socket
			else if(Net_player != NULL){
				sock = Net_player->reliable_socket;
			}
			
			header_info->bytes_processed = multi_xfer_process_packet(data + HEADER_LENGTH, sock) + HEADER_LENGTH;
			break;

		case MISSION_REQUEST:
			process_mission_request_packet(data,header_info);
			break;

		case MISSION_ITEM:
			process_mission_item_packet(data,header_info);
			break;		

		case MULTI_PAUSE_REQUEST:
			process_multi_pause_packet(data, header_info);
			break;		

		case INGAME_NAK:
			process_ingame_nak(data, header_info);
			break;

		case SHIPS_INGAME_PACKET:
			process_ingame_ships_packet(data, header_info);
			break;

		case WINGS_INGAME_PACKET:
			process_ingame_wings_packet(data, header_info);
			break;

		case MISSION_END:
			process_endgame_packet(data, header_info);
			break;

		case FORCE_MISSION_END:
			process_force_end_mission_packet(data, header_info);
			break;
		
		case OBSERVER_UPDATE:
			process_observer_update_packet(data, header_info);
			break;

		case NETPLAYER_SLOTS_P:
			process_netplayer_slot_packet(data, header_info);
			break;

		case SHIP_STATUS_CHANGE:
			process_ship_status_packet(data, header_info);
			break;

		case PLAYER_ORDER_PACKET:
			process_player_order_packet(data, header_info);
			break;

		case INGAME_SHIP_UPDATE:
			process_ingame_ship_update_packet(data, header_info);
			break;

		case INGAME_SHIP_REQUEST:
			process_ingame_ship_request_packet(data, header_info);
			break;
		
		case FILE_SIG_INFO:
			process_file_sig_packet(data, header_info);
			break;

		case RESPAWN_NOTICE:
			multi_respawn_process_packet(data,header_info);			
			break;

		case SUBSYSTEM_DESTROYED:
			process_subsystem_destroyed_packet( data, header_info );
			break;

		case LOAD_MISSION_NOW :
			process_netplayer_load_packet(data, header_info);
			break;

		case FILE_SIG_REQUEST :
			process_file_sig_request(data, header_info);
			break;

		case JUMP_INTO_GAME:
			process_jump_into_mission_packet(data, header_info);
			break;		

		case CLIENT_REPAIR_INFO:
			process_repair_info_packet(data,header_info);
			break;

		case MISSION_SYNC_DATA:
			process_mission_sync_packet(data,header_info);
			break;

		case STORE_MISSION_STATS:
			process_store_stats_packet(data, header_info);
			break;

		case DEBRIS_UPDATE:
			process_debris_update_packet(data, header_info);
			break;		

		case SHIP_WSTATE_CHANGE:
			process_ship_weapon_change( data, header_info );
			break;

		case WSS_UPDATE_PACKET:
			process_wss_update_packet(data, header_info);
			break;

		case WSS_REQUEST_PACKET:
			process_wss_request_packet( data, header_info );
			break;	

		case FIRING_INFO:
			process_firing_info_packet( data, header_info );
			break;		

		case CARGO_REVEALED:
			process_cargo_revealed_packet( data, header_info);
			break;		

		case CARGO_HIDDEN:
			process_cargo_hidden_packet( data, header_info);
			break;

		case SUBSYS_CARGO_REVEALED:
			process_subsystem_cargo_revealed_packet( data, header_info);
			break;		

		case SUBSYS_CARGO_HIDDEN:
			process_subsystem_cargo_hidden_packet( data, header_info);
			break;		

		case MISSION_GOAL_INFO:
			process_mission_goal_info_packet(data, header_info);
			break;

		case KICK_PLAYER:
			process_player_kick_packet(data, header_info);
			break;

		case PLAYER_SETTINGS:
			process_player_settings_packet(data, header_info);
			break;

		case DENY:
			process_deny_packet(data, header_info);
			break;

		case POST_SYNC_DATA:
			process_post_sync_data_packet(data, header_info);
			break;

		case WSS_SLOTS_DATA:
			process_wss_slots_data_packet(data,header_info);
			break;

		case SHIELD_EXPLOSION:
			process_shield_explosion_packet( data, header_info );
			break;

		case PLAYER_STATS:
			process_player_stats_block_packet(data, header_info);
			break;

		case SLOT_UPDATE:
			process_pslot_update_packet(data,header_info);
			break;

		case AI_INFO_UPDATE:
			process_ai_info_update_packet( data, header_info );
			break;		

		case CAMPAIGN_UPDATE :
			multi_campaign_process_update(data,header_info);
			break;

		case CAMPAIGN_UPDATE_INGAME:
			multi_campaign_process_ingame_start(data,header_info);
			break;

		case VOICE_PACKET :
			multi_voice_process_packet(data,header_info);
			break;

		case TEAM_UPDATE :
			multi_team_process_packet(data,header_info);
			break;

		case ASTEROID_INFO:
			process_asteroid_info(data, header_info);
			break;		

		case HOST_RESTR_QUERY:
			process_host_restr_packet(data, header_info);
			break;

		case OPTIONS_UPDATE:
			multi_options_process_packet(data,header_info);
			break;

		case SQUADMSG_PLAYER:
			multi_msg_process_squadmsg_packet(data,header_info);
			break;

		case NETGAME_END_ERROR:
			process_netgame_end_error_packet(data,header_info);
			break;

		case COUNTERMEASURE_SUCCESS:
			process_countermeasure_success_packet( data, header_info );
			break;

		case CLIENT_UPDATE:
			process_client_update_packet(data, header_info);
			break;

		case COUNTDOWN:
			process_countdown_packet(data, header_info);
			break;

		case DEBRIEF_INFO:
			process_debrief_info( data, header_info );
			break;

		case ACCEPT_PLAYER_DATA:
			process_accept_player_data( data, header_info );
			break;				

		case HOMING_WEAPON_UPDATE:
			process_homing_weapon_info( data, header_info );
			break;		

		case EMP_EFFECT:
			process_emp_effect(data, header_info);
			break;

		case REINFORCEMENT_AVAIL:
			process_reinforcement_avail( data, header_info );
			break;

		case CHANGE_IFF:
			process_change_iff_packet(data, header_info);
			break;

		case CHANGE_IFF_COLOR:
			process_change_iff_color_packet(data, header_info);
			break;

		case CHANGE_AI_CLASS:
			process_change_ai_class_packet(data, header_info);
			break;

		case PRIMARY_FIRED_NEW:
			process_NEW_primary_fired_packet(data, header_info);
			break;

		case COUNTERMEASURE_NEW:
			process_NEW_countermeasure_fired_packet(data, header_info);
			break;

		case BEAM_FIRED:
			process_beam_fired_packet(data, header_info);
			break;		
			
		case SW_STD_QUERY:
			process_sw_query_packet(data, header_info);
			break;

		case EVENT_UPDATE:
			process_event_update_packet(data, header_info);
			break;

		case VARIABLE_UPDATE:
			process_variable_update_packet(data, header_info);
			break;

		case WEAPON_OR_AMMO_CHANGED:
			process_weapon_or_ammo_changed_packet(data, header_info);
			break;

		case OBJECT_UPDATE_NEW:			
			multi_oo_process_update(data, header_info);
			break;

		case WEAPON_DET:
			process_weapon_detonate_packet(data, header_info);
			break;

		case FLAK_FIRED:
			process_flak_fired_packet(data, header_info);
			break;

		case NETPLAYER_PAIN:
			process_player_pain_packet(data, header_info);
			break;

		case LIGHTNING_PACKET:
			process_lightning_packet(data, header_info);
			break;

		case BYTES_SENT:
			process_bytes_recvd_packet(data, header_info);
			break;

		case TRANSFER_HOST:
			process_host_captain_change_packet(data, header_info);
			break;

		case SELF_DESTRUCT:
			process_self_destruct_packet(data, header_info);
			break;

		case SEXP:
			process_sexp_packet(data, header_info);
			break; 

		default:
			nprintf(("Network", "Received packet with unknown type %d\n", data[0] ));
			header_info->bytes_processed = 10000;
			break;

	} // end switch
}



// Takes a bunch of messages, check them for validity,
// and pass them to multi_process_data. 
//  --------------------^
// this should be process_packet() I think, or with the new code
// process_tracker_packet() as defined in MultiTracker.[h,cpp]
void multi_process_bigdata(ubyte *data, int len, net_addr *from_addr, int reliable)
{
	int type, bytes_processed;
	int player_num;
	header header_info;
	ubyte *buf;	

	// the only packets we will process from an unknown player are GAME_QUERY, GAME_INFO, JOIN, PING, PONG, ACCEPT, and GAME_ACTIVE packets
	player_num = find_player(from_addr);		

	// find the player who sent the message and mark the last_heard time for this player
	// check to see if netplayer is null (it may be in cases such as getting lists of games from the tracker)
	if(player_num >= 0){
		Net_players[player_num].last_heard_time = timer_get_fixed_seconds();
	}

	// store fields that were passed along in the message
	// store header information that was captured from the network-layer header
	memcpy(header_info.addr, from_addr->addr, 6);
	memcpy(header_info.net_id, from_addr->net_id, 4);
	header_info.port = from_addr->port;	
	if(player_num >= 0){
		header_info.id = Net_players[player_num].player_id;
	} else {
		header_info.id = -1;
	}   

	bytes_processed = 0;
	while( (bytes_processed >= 0) && (bytes_processed < len) )  {

      buf = &(data[bytes_processed]);

      type = buf[0];

		// if its coming from an unknown source, there are only certain packets we will actually process
		if((player_num == -1) && !multi_is_valid_unknown_packet((ubyte)type)){
			return ;
		}		

		if ( (type<0) || (type > MAX_TYPE_ID )) {
			nprintf( ("Network", "multi_process_bigdata: Invalid packet type %d!\n", type ));
			return;
		}		

		// perform any special processing checks here		
		process_packet_normal(buf,&header_info);
		 
		// MWA -- magic number was removed from header on 8/4/97.  Replaced with bytes_processed
		// variable which gets stuffed whenever a packet is processed.
		bytes_processed += header_info.bytes_processed;
	}

	// if this is not reliable data and we have a valid player
	if(Net_player != NULL){
		if(!MULTIPLAYER_MASTER && !reliable && (Game_mode & GM_IN_MISSION)){
			Net_player->cl_bytes_recvd += len;
		}
	}
}

// process all reliable socket details
void multi_process_reliable_details()
{
	int idx;
	int sock_status;

	// run reliable sockets
	psnet_rel_work();

	
	// server operations
	if ( MULTIPLAYER_MASTER ){
		// listen for new reliable socket connections
		multi_check_listen();		

		// check for any broken sockets and delete any players
		for(idx=0; idx<MAX_PLAYERS; idx++){
			// players who _should_ be validly connected
			if((idx != MY_NET_PLAYER_NUM) && MULTI_CONNECTED(Net_players[idx])){				
				// if this guy's socket is broken or disconnected, kill him
				sock_status = psnet_rel_get_status(Net_players[idx].reliable_socket);
				if((sock_status == RNF_UNUSED) || (sock_status == RNF_BROKEN) || (sock_status == RNF_DISCONNECTED)){
					ml_string("Shutting down rel socket because of disconnect!");
					delete_player(idx);
				}

				// if we're still waiting for this guy to connect on his reliable socket and he's timed out, boot him
				if(Net_players[idx].s_info.reliable_connect_time != -1){
					// if he's connected
					if(Net_players[idx].reliable_socket != INVALID_SOCKET){
						Net_players[idx].s_info.reliable_connect_time = -1;
					} 
					// if he's timed out
					else if(((time(NULL) - Net_players[idx].s_info.reliable_connect_time) > MULTI_RELIABLE_CONNECT_WAIT) && (Net_players[idx].reliable_socket == INVALID_SOCKET)){
						ml_string("Player timed out while connecting on reliable socket!");
						delete_player(idx);
					}
				}			
			}
		}
	}	
	// clients should detect broken sockets
	else {
		extern unsigned int Serverconn;
		if(Serverconn != 0xffffffff){
			int status = psnet_rel_get_status(Serverconn);
			if(status == RNF_BROKEN){
				mprintf(("CLIENT SOCKET DISCONNECTED\n"));

				// quit the game
				if(!multi_endgame_ending()){
					multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_CONTACT_LOST);
				}
			}
		}
	}
}

// multi_process_incoming reads incoming data off the unreliable and reliable ports and sends
// the data to process_big_data
void multi_process_incoming()
{
	int size;
	ubyte *data, *savep;
	net_addr from_addr;	

	Assert( Multi_read_count < NUM_REENTRANT_LEVELS );
	savep = net_buffer[Multi_read_count];

	Multi_read_count++;

	data = savep;

	// get the other net players data
	while( (size = psnet_get(data, &from_addr))>0 )	{
		// ingame joiners will ignore UDP packets until they are have picked a ship and are in the mission
		if( (Net_player->flags & NETINFO_FLAG_INGAME_JOIN) && (Net_player->state != NETPLAYER_STATE_INGAME_SHIP_SELECT) ){
			nprintf(("Network","Tossing UDP like a good little ingame joiner...\n"));
		} 
		// otherwise process incoming data normally
		else {
			multi_process_bigdata(data, size, &from_addr, 0);
		}
	} // end while

	// read reliable sockets for data
	data = savep;
	int idx;

	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		for (idx=0;idx<MAX_PLAYERS;idx++) {
			if((Net_players[idx].flags & NETINFO_FLAG_CONNECTED) && (Net_player != NULL) && (Net_player->player_id != Net_players[idx].player_id)){
				while( (size = psnet_rel_get(Net_players[idx].reliable_socket, data, MAX_NET_BUFFER)) > 0){
					multi_process_bigdata(data, size, &Net_players[idx].p_info.addr, 1);
				}
			}
		}
	} else {
		// if I'm not the master of the game, read reliable data from my connection with the server
		if((Net_player->reliable_socket != INVALID_SOCKET) && (Net_player->reliable_socket != 0)){
			while( (size = psnet_rel_get(Net_player->reliable_socket,data, MAX_NET_BUFFER)) > 0){				
				multi_process_bigdata(data, size, &Netgame.server_addr, 1);
			}
		}
	}
		
	Multi_read_count--;
}

// -------------------------------------------------------------------------------------------------
//	multi_do_frame() is called once per game loop do update all the multiplayer objects, and send
// the player data to all the other net players.
//
//

int eye_tog = 1;
DCF(eye_tog, "Toggles setting of the local player eyepoint on every frame (Multiplayer)")
{
	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("proper eye stuff is %s\n", eye_tog ? "ON" : "OFF");
		return;
	}

	eye_tog = !eye_tog;
	dc_printf("proper eye stuff is %s\n", eye_tog ? "ON" : "OFF");
}

void multi_do_frame()
{	
	PSNET_TOP_LAYER_PROCESS();

	// always set the local player eye position/orientation here so we know its valid throughout all multiplayer
	// function calls
	if((Net_player != NULL) && eye_tog){
		camid cid = player_get_cam();
		if(cid.isValid())
		{
			camera *cam = cid.getCamera();
			cam->get_info(&Net_player->s_info.eye_pos, &Net_player->s_info.eye_orient);
		}
	}

	// send all buffered packets from the previous frame
	multi_io_send_buffered_packets();

	// datarate tracking
	multi_rate_process();

	// always process any pending endgame details
	multi_endgame_process();		

	// process all reliable socket details, including :
	// 1.) Listening for new pending reliable connections (server)
	// 2.) Checking for broken sockets (server/client)
	// 3.) Checking for clients who haven't fully connected
	multi_process_reliable_details();	

	// get the other net players data
	multi_process_incoming();		

	// process object update datarate stuff (for clients and server both)
	multi_oo_rate_process();

	// clients should check when last time they heard from sever was -- if too long, then
	// pause the simulation so wait for it to possibly come back
	if ( (MULTIPLAYER_CLIENT) && (Net_player->flags & NETINFO_FLAG_CONNECTED) ){
		multi_client_check_server();
	}

	// everybody pings all the time	
	if((Next_ping_time < 0) || ((time(NULL) - Next_ping_time) > PING_SEND_TIME) ){
		if( (Net_player->flags & NETINFO_FLAG_AM_MASTER) ){
			send_netplayer_update_packet();
		}
		
		// ping everyone
		multi_ping_send_all();
		Next_ping_time = (int) time(NULL);		
	}	
	
	// if I am the master, and we are not yet actually playing the mission, send off netgame
	// status to all other players in the game.  If I am not the master of the game, and we
	// are not in the game, then send out my netplayer status to the host
	if ( (Net_player->flags & NETINFO_FLAG_CONNECTED) && !(Game_mode & GM_IN_MISSION)){	
		if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {			
			if ( (Netgame_send_time < 0) || ((time(NULL) - Netgame_send_time) > NETGAME_SEND_TIME) ) {
				send_netgame_update_packet();				
				
				Netgame_send_time = (int) time(NULL);
			}		
		} else {
			if ( (State_send_time < 0) || ((time(NULL) - State_send_time) > STATE_SEND_TIME) ){
				// observers shouldn't send an update state packet
				if ( !(Net_player->flags & NETINFO_FLAG_OBSERVER) ){
					send_netplayer_update_packet();
				}				
				
				State_send_time = (int) time(NULL);
			}
		}
	}
	else if ( (Net_player->flags & NETINFO_FLAG_CONNECTED) && (Game_mode & GM_IN_MISSION) ) {	
		// if I am connected and am in the mission, do things that need to be done on a regular basis
		if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {
			if ( (Gameinfo_send_time < 0) || ((time(NULL) - Gameinfo_send_time) > GAMEINFO_SEND_TIME)){
				send_game_info_packet();
				
				Gameinfo_send_time = (int) time(NULL);
			}
			
			// for any potential respawns
			multi_respawn_handle_invul_players();
			multi_respawn_check_ai();

			// for any potential ingame joiners
			multi_handle_ingame_joiners();
		} else {
			// the clients need to do some processing of stuff as well			
		}
	}

	// check to see if we're waiting on confirmation for a restricted ingame join
	if(Multi_restr_query_timestamp != -1){
		// if it has elapsed, unset the ingame join flag
		if(timestamp_elapsed(Multi_restr_query_timestamp)){
			Multi_restr_query_timestamp = -1;
			Netgame.flags &= ~(NG_FLAG_INGAME_JOINING);		
		}	
	}

	// while in the mission, send my PlayerControls to the host so that he can process
	// my movement
	if ( Game_mode & GM_IN_MISSION ) {
		// tickers
		extern void oo_update_time();
		oo_update_time();


		if ( !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){					
			if(Net_player->flags & NETINFO_FLAG_OBSERVER){
				// if the rate limiting system says its ok
				if(multi_oo_cirate_can_send()){
					// send my observer position/object update
					send_observer_update_packet();
				}
			} else if ( !(Player_ship->flags & SF_DEPARTING ) ){				
				// if the rate limiting system says its ok
				if(multi_oo_cirate_can_send()){
					// use the new method
					multi_oo_send_control_info();
				}				
			}

			// bytes received info
			if( (Next_bytes_time < 0) || ((time(NULL) - Next_bytes_time) > BYTES_SENT_TIME) ){
				if(Net_player != NULL){
					send_bytes_recvd_packet(Net_player);

					// reset bytes recvd
					Net_player->cl_bytes_recvd = 0;
				}

				// reset timestamp
				Next_bytes_time = (int) time(NULL);				
			}
		} else {			
			// sending new objects from here is dependent on having objects only created after
			// the game is done moving the objects.  I think that I can enforce this.				
			multi_oo_process();			

			// evaluate whether the time limit has been reached or max kills has been reached
			// Commented out by Sandeep 4/12/98, was causing problems with testing.
			if( ((f2fl(Netgame.options.mission_time_limit) > 0.0f) && (Missiontime > Netgame.options.mission_time_limit)) ||
				 multi_kill_limit_reached() ) {

				// make sure we don't do this more than once
				if(Netgame.game_state == NETGAME_STATE_IN_MISSION){				
					multi_handle_end_mission_request();									
				}
			}			
		}			
	}

	// periodically send a client update packet to all clients
	if((Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		int idx;
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx])){
				if((Multi_client_update_times[idx] < 0) || timestamp_elapsed_safe(Multi_client_update_times[idx], 1000)){
					
					send_client_update_packet(&Net_players[idx]);
					
					Multi_client_update_times[idx] = timestamp(MULTI_CLIENT_UPDATE_TIME);
				}
			}
		}
	}	


	// process any kicked player details
	multi_kick_process();

	// do any file xfer details
	multi_xfer_do();

	// process any player data details (wav files, pilot pics, etc)
	multi_data_do();

	// do any voice details
	multi_voice_process();

	// process any player messaging details
	multi_msg_process();		
	
	// if on the standalone, do any gui stuff
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_do_gui_frame();
	}	

	// dogfight nonstandalone players should recalc the escort list every frame
	if(!(Game_mode & GM_STANDALONE_SERVER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT) && MULTI_IN_MISSION){
		hud_setup_escort_list(0);
	}

	// do fs2netd stuff
	fs2netd_do_frame();
}

// -------------------------------------------------------------------------------------------------
//	multi_pause_do_frame() is called once per game loop do update all the multiplayer objects, and send
// the player data to all the other net players when the multiplayer game is paused. It only will do 
// checking for a few specialized packets (MULTI_UNPAUSE, etc)
//

void multi_pause_do_frame()
{
	PSNET_TOP_LAYER_PROCESS();

	// always set the local player eye position/orientation here so we know its valid throughout all multiplayer
	// function calls
	// if((Net_player != NULL) && eye_tog){
		// player_get_eye(&Net_player->s_info.eye_pos, &Net_player->s_info.eye_orient);
	// }

	// send all buffered packets from the previous frame
	multi_io_send_buffered_packets();

	// always process any pending endgame details
	multi_endgame_process();		

	// process all reliable socket details, including :
	// 1.) Listening for new pending reliable connections (server)
	// 2.) Checking for broken sockets (server/client)
	// 3.) Checking for clients who haven't fully connected
	multi_process_reliable_details();	

	// these timestamps and handlers shoul be evaluated in the pause state
	if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {
		if ( (Gameinfo_send_time < 0) || ((time(NULL) - Gameinfo_send_time) > GAMEINFO_SEND_TIME) ){
			send_game_info_packet();
			
			Gameinfo_send_time = (int) time(NULL);
		}				
	}

	// everybody pings all the time
	if((Next_ping_time < 0) || ((time(NULL) - Next_ping_time) > PING_SEND_TIME) ){
		multi_ping_send_all();
		
		Next_ping_time = (int) time(NULL);
	}

	// periodically send a client update packet to all clients
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		int idx;

		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx])){			
				if((Multi_client_update_times[idx] < 0) || timestamp_elapsed_safe(Multi_client_update_times[idx], 1000)){
					
					send_client_update_packet(&Net_players[idx]);
					
					Multi_client_update_times[idx] = timestamp(MULTI_CLIENT_UPDATE_TIME);
				}
			}				
		}

		// for any potential ingame joiners
		multi_handle_ingame_joiners();
	}	

	// do any file xfer details
	multi_xfer_do();

	// process any player data details (wav files, pilot pics, etc)
	multi_data_do();

	// get the other net players data
	multi_process_incoming();	

	// do any voice details
	multi_voice_process();

	// process any player messaging details
	multi_msg_process();

	// process any kicked player details
	multi_kick_process();

	// process any pending endgame details
	multi_endgame_process();

	// process object update stuff (for clients and server both)
	if(MULTIPLAYER_MASTER){
		multi_oo_process();
	}

	// if on the standalone, do any gui stuff
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_do_gui_frame();
	}
}




// --------------------------------------------------------------------------------
// standalone_main_init()  the standalone equivalent of the main menu
//

extern int sock_inited;
float frame_time = (float)1.0/(float)30.0;

void standalone_main_init()
{
   std_debug_set_standalone_state_string("Main Init");   

	Game_mode = (GM_STANDALONE_SERVER | GM_MULTIPLAYER);	

	// NETLOG
	ml_string(NOX("Standalone server initializing"));

	// read in config file
	// multi_options_read_config();   
#ifdef _WIN32
	// if we failed to startup on our desired protocol, fail
	if((Multi_options_g.protocol == NET_IPX) && !Ipx_active){
		MessageBox((HWND)os_get_window(), XSTR( "You have selected IPX for multiplayer FreeSpace, but the IPX protocol was not detected on your machine.", 1402), "Error", MB_OK);
		exit(1);
	}
	if((Multi_options_g.protocol == NET_TCP) && !Tcp_active){
		if (Tcp_failure_code == WSAEADDRINUSE) {
			MessageBox((HWND)os_get_window(), XSTR("You have selected TCP/IP for multiplayer FreeSpace, but the TCP socket is already in use.  Check for another instance and/or use the \"-port <port_num>\" command line option to select an available port.", 1620), "Error", MB_OK);
		} else {
			MessageBox((HWND)os_get_window(), XSTR("You have selected TCP/IP for multiplayer FreeSpace, but the TCP/IP protocol was not detected on your machine.", 362), "Error", MB_OK);
		}

		exit(1);
	}
#endif // ifdef _WIN32

	// set the protocol
	psnet_use_protocol(Multi_options_g.protocol);
	switch (Multi_options_g.protocol) {
	case NET_IPX:
		ADDRESS_LENGTH = IPX_ADDRESS_LENGTH;
		PORT_LENGTH = IPX_PORT_LENGTH;
		break;

	case NET_TCP:
		ADDRESS_LENGTH = IP_ADDRESS_LENGTH;		
		PORT_LENGTH = IP_PORT_LENGTH;			
		break;

	default:
		Int3();
	} // end switch

	HEADER_LENGTH = 1;		
	
	// clear out the Netgame structure and start filling in the values
	// NOTE : these values are not incredibly important since they will be overwritten by the host when he joins
	memset( &Netgame, 0, sizeof(Netgame) );	
	Netgame.game_state = NETGAME_STATE_FORMING;		// game is currently starting up
	Netgame.security = 0;
	Netgame.server_addr = Psnet_my_addr;

	The_mission.Reset( );
		
	// reinitialize all systems	
	multi_level_init();	

	// intialize endgame stuff
	multi_endgame_init();

	// clear the file xfer system
	multi_xfer_reset();
	multi_xfer_force_dir(CF_TYPE_MULTI_CACHE);

	// reset timer
	timestamp_reset();

	// setup a blank pilot (this is a standalone usage only!)
	Pilot.load_player(NULL);

	// setup the netplayer for the standalone
	Net_player = &Net_players[0];	
	Net_player->tracker_player_id = -1;
	Net_player->flags |= (NETINFO_FLAG_AM_MASTER | NETINFO_FLAG_CONNECTED | NETINFO_FLAG_DO_NETWORKING | NETINFO_FLAG_MISSION_OK);
	Net_player->state = NETPLAYER_STATE_WAITING;
	Net_player->m_player = Player;
	strcpy_s(Player->callsign, "server");
	Net_player->p_info.addr = Psnet_my_addr;
	Net_player->s_info.xfer_handle = -1;	
	Net_player->player_id = multi_get_new_id();	
	Netgame.server = Net_player; 

	// maybe flag the game as having a hacked ships.tbl
	/*if(!Game_ships_tbl_valid){
		Netgame.flags |= NG_FLAG_HACKED_SHIPS_TBL;
	}
	// maybe flag the game as having a hacked weapons.tbl
	if(!Game_weapons_tbl_valid){
		Netgame.flags |= NG_FLAG_HACKED_WEAPONS_TBL;
	}*/

	// hacked data
	if(game_hacked_data()){
		Net_player->flags |= NETINFO_FLAG_HAXOR;
	}

	// setup debug flags
	Netgame.debug_flags = 0;

	// setup the default game name for the standalone
	std_connect_set_gamename(NULL);

	// set netgame default options
	multi_options_set_netgame_defaults(&Netgame.options);

	// set local netplayer default options
	multi_options_set_local_defaults(&Net_player->p_info.options);

	// set our object update level from the standalone default	
	Net_player->p_info.options.obj_update_level = Multi_options_g.std_datarate;
	switch(Net_player->p_info.options.obj_update_level){
	case OBJ_UPDATE_LOW:
		nprintf(("Network","STANDALONE USING LOW UPDATES\n"));
		break;
	case OBJ_UPDATE_MEDIUM:
		nprintf(("Network","STANDALONE USING MEDIUM UPDATES\n"));
		break;
	case OBJ_UPDATE_HIGH:
		nprintf(("Network","STANDALONE USING HIGH UPDATE\n"));
		break;
	case OBJ_UPDATE_LAN:
		nprintf(("Network","STANDALONE USING LAN UPDATE\n"));
		break;
	}

	// clear out various things
	psnet_flush();
	game_flush();
	ship_init();

	std_debug_set_standalone_state_string("Main Do");
	std_set_standalone_fps((float)0);
	std_multi_set_standalone_missiontime((float)0);

	// load my missions and campaigns
	multi_create_list_load_missions();
	multi_create_list_load_campaigns();

	// if this is a tracker game then we have some extra tasks to perform
	if (MULTI_IS_TRACKER_GAME) {
		// disconnect and prepare for reset if we are already connected
		fs2netd_disconnect();

		// login (duh!)
		if ( fs2netd_login() ) {
			// validate missions
			multi_update_valid_missions();

			// advertise our game to the server
			fs2netd_gameserver_start();

			// set tracker id
			Net_player->tracker_player_id = Multi_tracker_id;
		}
	}
}


// --------------------------------------------------------------------------------
// standalone_main_do()
//

// DESCRIPTION : the standalone server will wait in this state until the host of the game 
//               is "Waiting". That is, his state==NETPLAYER_STATE_WAITING, and he has finished
//               doing everything and wants to play the game. Once this happens, we will jump
//               into GS_STATE_MULTI_SERVER_WAIT

void standalone_main_do()
{
 
   Sleep(10);  // since nothing will really be going on here, we can afford to give some time
               // back to the operating system.

	// kind of a do-nothing spin state.
	// The standalone will eventually move into the GS_STATE_MULTI_MISSION_SYNC state when a host connects and
	// attempts to start a game
}

// --------------------------------------------------------------------------------
// standalone_main_close()
//

void standalone_main_close()
{
   std_debug_set_standalone_state_string("Main Close");	
}

void multi_standalone_reset_all()
{	
	int idx;

	// NETLOG
	ml_string(NOX("Standalone resetting"));
	
	// shut all game stuff down
	game_level_close();

	// reinitialize the gui
	std_reset_standalone_gui();	

	// close down all sockets
	for(idx=0;idx<MAX_PLAYERS;idx++){

		// 6/25/98 -- MWA -- call delete_player here to remove the player.  This closes down the socket
		// and marks the player as not connected anymore.  It is probably cleaner to do this.
		if ( &Net_players[idx] != Net_player ) {
			delete_player( idx );
		}		
	}

	// make sure we go to the proper state.	
	if(gameseq_get_state() == GS_STATE_STANDALONE_MAIN){
		standalone_main_init();
	}
	gameseq_post_event(GS_EVENT_STANDALONE_MAIN);	
}

// --------------------------------------------------------------------------------
// multi_server_wait_init()  do stuff like setting the status bits correctly
//

void multi_standalone_wait_init()
{	
	std_debug_set_standalone_state_string("Wait Do");
	std_multi_add_goals();   // fill in the goals for the mission into the tree view
	multi_reset_timestamps();

	// create the bogus standalone object
	multi_create_standalone_object();
}


// --------------------------------------------------------------------------------
// multi_server_wait_do_frame() wait for everyone to log in or the host to send commands
// 

// DESCRIPTION : we will be in this state once the host of the game is waiting for everyone
//               to be finished and ready to go, at which point, we will will tell everyone
//               to enter the game, and we will start simulating ourselves. Note that most of
//               this code is lifted from multi_wait_do_frame()
void multi_standalone_wait_do()
{
}

// --------------------------------------------------------------------------------
// multi_server_wait_close() cleanup
//

void multi_standalone_wait_close()
{
	std_debug_set_standalone_state_string("Wait Close / Game Play");
	
	// all players should reset sequencing
	int idx;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(Net_player->flags & NETINFO_FLAG_CONNECTED){
			Net_players[idx].client_cinfo_seq = 0;
			Net_players[idx].client_server_seq = 0;	
		}
	}
}


// this is an artificial state which will push the standalone into the main state without it having to go through 
// the init function (which would disconnect everyone and generally just screw things up)
// it will also eventually do tracker stats update
extern int Multi_debrief_server_framecount;
void multi_standalone_postgame_init()	
{
	std_debug_set_standalone_state_string("Postgame / Send Stats");

	// NETLOG
	ml_string(NOX("Standlone entering postgame"));

	mission_goal_fail_incomplete();

	// handle campaign stuff
	if ( Game_mode & GM_CAMPAIGN_MODE ) {
		// MUST store goals and events first - may be used to evaluate next mission
		// store goals and events
		mission_campaign_store_goals_and_events_and_variables();

		// evaluate next mission
		mission_campaign_eval_next_mission();
	}	

	// always set my state to be "DEBRIEF_ACCEPT"
	Net_player->state = NETPLAYER_STATE_DEBRIEF_ACCEPT;	

	// mark stats as not being store yet
	Netgame.flags &= ~(NG_FLAG_STORED_MT_STATS);

	Multi_debrief_server_framecount = 0;

	// reset network timestamps
	multi_reset_timestamps();
}

void multi_standalone_postgame_do()
{
	// wait until everyone is in the debriefing
	if((Netgame.game_state != NETGAME_STATE_DEBRIEF) && multi_netplayer_state_check(NETPLAYER_STATE_DEBRIEF, 1)){		
		Netgame.game_state = NETGAME_STATE_DEBRIEF;
		send_netgame_update_packet();
		debrief_multi_server_stuff();
	}
	
	// process server debriefing details
	if(Netgame.game_state == NETGAME_STATE_DEBRIEF){
		multi_debrief_server_process();
	}
}

void multi_standalone_postgame_close()
{
}


void multi_reset_timestamps()
{
	int i;

	for ( i = 0 ; i < MAX_PLAYERS; i++ ){
		Multi_client_update_times[i] = -1;
	}
	Netgame_send_time = -1;
	Gameinfo_send_time = -1;	
	Next_ping_time = -1;
	State_send_time = -1;
	Next_bytes_time = -1;

	chatbox_reset_timestamps();

	// do for all players so that ingame joiners work properly.
	for (i = 0; i < MAX_PLAYERS; i++ ) {
		Players[i].update_dumbfire_time = timestamp(0);
		Players[i].update_lock_time = timestamp(0);

		Net_players[i].s_info.voice_token_timestamp = -1;
	}

	// reset standalone gui timestamps (these are not game critical, so there is not much danger)
	std_reset_timestamps();

	// initialize all object update timestamps
	multi_oo_gameplay_init();
}

// netgame debug flags for debug console stuff
DCF(netd, "change netgame debug flags (Mulitplayer)")
{
	int value;
	dc_stuff_int(&value);
	
	// if we're the server, change flags
	if ((Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) && (value <= 7)) {
		Netgame.debug_flags ^= (1 << value);
	}

	// display network flags
	dc_printf("BITS\n");
}

// display any multiplayer/networking information here
void multi_display_netinfo()
{
	int sx = gr_screen.max_w - 200;
	int sy = 20;
	int dy = gr_get_font_height() + 1;
	int idx;

	// not multiplayer
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// HUD is turned off
	if (!HUD_draw) {
		return;
	}

	// message window is open
	if (Player->flags & PLAYER_FLAGS_MSG_MODE) {
		return;
	}

	gr_set_color_fast(&Color_normal);

	// server or client
	if(MULTIPLAYER_MASTER){
		gr_string(sx, sy, "SERVER", GR_RESIZE_NONE); sy += dy;

		for(idx=0; idx<MAX_PLAYERS; idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx]) && (Net_players[idx].m_player != NULL)){
				if(Net_players[idx].sv_last_pl < 0){
					gr_printf_no_resize(sx, sy, "%s: %d, %d pl", Net_players[idx].m_player->callsign, Net_players[idx].sv_bytes_sent, 0);
					sy += dy;
				} else {
					gr_printf_no_resize(sx, sy, "%s: %d, %d pl", Net_players[idx].m_player->callsign, Net_players[idx].sv_bytes_sent, Net_players[idx].sv_last_pl);
					sy += dy;
				}
			}
		}
	} else {
		gr_string(sx, sy, "CLIENT", GR_RESIZE_NONE); sy += dy;

		// display PL
		if(Net_player != NULL){
			if(Net_player->cl_last_pl < 0){
				gr_printf_no_resize(sx, sy, "PL: %d %d pl\n", Net_player->cl_bytes_recvd, 0);
				sy += dy;
			} else {
				gr_printf_no_resize(sx, sy, "PL: %d %d pl\n", Net_player->cl_bytes_recvd, Net_player->cl_last_pl);
				sy += dy;
			}
		}
	}
}
