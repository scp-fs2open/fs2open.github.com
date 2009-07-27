/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "network/multi.h"
#include "object/object.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "popup/popup.h"
#include "popup/popupdead.h"
#include "network/multi_endgame.h"
#include "playerman/player.h"
#include "network/multimsgs.h"
#include "network/multiui.h"
#include "network/multiutil.h"
#include "network/multi_pmsg.h"
#include "fs2netd/fs2netd_client.h"


// ----------------------------------------------------------------------------------------------------------
// Put all functions/data related to leaving a netgame, handling players leaving, handling the server leaving,
// and notifying the user of all of these actions, here.
//


// ----------------------------------------------------------------------------------------------------------
// MULTI ENDGAME DEFINES/VARS
//


// set when the server/client has ended the game on some notification or error and is waiting for clients to leave
#define MULTI_ENDGAME_SERVER_WAIT				5.0f
int Multi_endgame_server_waiting = 0;
float Multi_endgame_server_wait_stamp = -1.0f;
int Multi_endgame_client_waiting = 0;

// error/notification codes (taken from parameters to multi_quit_game(...)
int Multi_endgame_notify_code;
int Multi_endgame_error_code;
int Multi_endgame_wsa_error;

// for reentrancy problems on standalone
int Multi_endgame_processing;

// ----------------------------------------------------------------------------------------------------------
// MULTI ENDGAME FORWARD DECLARATIONS
//

// called when a given netgame is about to end completely
void multi_endgame_cleanup();

// throw up a popup with the given notification code and optional winsock code
void multi_endgame_popup(int notify_code,int error_code,int wsa_error = -1);

// called when server is waiting for clients to disconnect
int multi_endgame_server_ok_to_leave();

// check to see if we need to be warping out (strange transition possibilities)
void multi_endgame_check_for_warpout();


// ----------------------------------------------------------------------------------------------------------
// MULTI ENDGAME FUNCTIONS
//

// initialize the endgame processor (call when joining/starting a new netgame)
void multi_endgame_init()
{
	// set this so that the server/client knows he hasn't tried to end the game
	Multi_endgame_server_waiting = 0;	
	Multi_endgame_client_waiting = 0;

	// reset the timestamp used when server is waiting for all other clients to leave.
	Multi_endgame_server_wait_stamp = -1.0f;

	// initialiaze all endgame notify and error codes
	Multi_endgame_notify_code = -1;
	Multi_endgame_error_code = -1;
	Multi_endgame_wsa_error = -1;

	// for reentrancy problems in to endgame_process
	Multi_endgame_processing = 0;
}

// process all endgame related events
void multi_endgame_process()
{
	if ( Multi_endgame_processing )
		return;

	Multi_endgame_processing = 1;

	// check to see if we need to be warping out (strange transition possibilities)
	multi_endgame_check_for_warpout();
	
	// if we're the server of the game
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		// if we're not waiting for clients to leave, do nothing
		if(!Multi_endgame_server_waiting){
			Multi_endgame_processing = 0;
			return;
		}

		// if a popup is already active, do nothing		
		if(popup_active()){
			Multi_endgame_processing = 0;
			return;
		}

		// otherwise popup until things are hunky-dory
		if(!multi_endgame_server_ok_to_leave()){
			if(Game_mode & GM_STANDALONE_SERVER){
				while(!multi_endgame_server_ok_to_leave()){
					// run networking, etc.
					game_set_frametime(-1);
					game_do_state_common(gameseq_get_state());
				}
			} else {
				popup_till_condition( multi_endgame_server_ok_to_leave , XSTR("&Cancel",645), XSTR("Waiting for clients to disconnect",646));		
			}
		}

		// mark myself as not waiting and get out
		multi_endgame_cleanup();	
	} else {
		// if we're not waiting to leave the game, do nothing
		if(!Multi_endgame_client_waiting){
			Multi_endgame_processing = 0;
			return;
		}

		// otherwise, check to see if there is a popup active
		if(popup_active()){
			Multi_endgame_processing = 0;
			return;
		}

		// if not, then we are good to leave		
		multi_endgame_cleanup();
	}

	Multi_endgame_processing = 0;
}

// if the game has been flagged as ended (ie, its going to be reset)
int multi_endgame_ending()
{
	return (Multi_endgame_client_waiting || Multi_endgame_server_waiting);
}

// reentrancy check
int Multi_quit_game = 0;
// general quit function, with optional notification, error, and winsock error codes
int multi_quit_game(int prompt, int notify_code, int err_code, int wsa_error)
{
	int ret_val,quit_already;

	// check for reentrancy
	if(Multi_quit_game){
		return 0;
	}

	// if we're not connected or have not net-player
	if((Net_player == NULL) || !(Net_player->flags & NETINFO_FLAG_CONNECTED)){
		return 1;
	}

	// reentrancy
	Multi_quit_game = 1;

	quit_already = 0;

	// reset my control info so that I don't continually do whacky stuff.  This is ugly
	//player_control_reset_ci( &Player->ci );
	if ( Game_mode & GM_IN_MISSION ) {
		memset(&Player->ci, 0, sizeof(Player->ci) );
		Player->ci.afterburner_stop = 1;
		physics_read_flying_controls( &Player_obj->orient, &Player_obj->phys_info, &(Player->ci), flFrametime);
	}

	// CASE 1 - response to a user request
	// if there is no associated notification or error code, don't override the prompt argument
	if((err_code == -1) && (notify_code == -1)){
		// if we're the server and we're already waiting for clients to leave, don't do anything
		if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && Multi_endgame_server_waiting){
			Multi_quit_game = 0;
			return 0;
		}

		// if we're the client and we're already waiting to leave, don't do anythin
		if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER) && Multi_endgame_client_waiting){
			Multi_quit_game = 0;
			return 0;
		}

		// see if we should be prompting the host for confirmation
		if((prompt==PROMPT_HOST || prompt==PROMPT_ALL) && (Net_player->flags & NETINFO_FLAG_GAME_HOST)){
			int p_flags;

			p_flags = PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_BODY_BIG;
			if ( Game_mode & GM_IN_MISSION )
				p_flags |= PF_RUN_STATE;

			ret_val = popup(p_flags,2,POPUP_CANCEL,POPUP_OK,XSTR("Warning - quitting will end the game for all players!",647));

			// check for host cancel
			if((ret_val == 0) || (ret_val == -1)){
				Multi_quit_game = 0;
				return 0;
			}

			// set this so that under certain circumstances, we don't call the popup below us as well
			quit_already = 1;
		}

		// see if we should be prompting the client for confirmation
		if((prompt==PROMPT_CLIENT || prompt==PROMPT_ALL) && !quit_already){
			ret_val = popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_BODY_BIG,2,POPUP_NO,POPUP_YES,XSTR("Are you sure you want to quit?",648));

			// check for host cancel
			if((ret_val == 0) || (ret_val == -1)){
				Multi_quit_game = 0;
				return 0;
			}
			quit_already = 1;
		}

		// if i'm the server of the game, tell all clients that i'm leaving, then wait
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			send_netgame_end_error_packet(MULTI_END_NOTIFY_SERVER_LEFT,MULTI_END_ERROR_NONE);

			// set the waiting flag and the waiting timestamp
			Multi_endgame_server_waiting = 1;
			Multi_endgame_server_wait_stamp = MULTI_ENDGAME_SERVER_WAIT;
		}
		// if i'm the client, quit now
		else {
			multi_endgame_cleanup();
		}
	}
	// CASE 2 - response to an error code or packet from the server
	// this is the case where we're being forced to quit the game because of some error or other notification
	else {				
		// if i'm the server, send a packet to the clients telling them that I'm leaving and why
		if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && !Multi_endgame_server_waiting){
			// if we're in the debrief state, mark down that the server has left the game
			if(((gameseq_get_state() == GS_STATE_DEBRIEF) || (gameseq_get_state() == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)) && !(Game_mode & GM_STANDALONE_SERVER)){
				multi_debrief_server_left();

				// add a message to the chatbox
				multi_display_chat_msg(XSTR("<Team captains have left>",649),0,0);				
				
				// set ourselves to be "not quitting"
				Multi_quit_game = 0;

				// tell the users, the game has ended
				send_netgame_end_error_packet(notify_code,err_code);				
				return 0;
			}

			send_netgame_end_error_packet(notify_code,err_code);			

			// store the globals 
			Multi_endgame_notify_code = notify_code;
			Multi_endgame_error_code = err_code;
			Multi_endgame_wsa_error = wsa_error;

			// by setting this, multi_endgame_process() will know to check and see if it is ok for us to leave
			Multi_endgame_server_waiting = 1;				
			Multi_endgame_server_wait_stamp = MULTI_ENDGAME_SERVER_WAIT;
		}
		// if i'm the client, set the error codes and leave the game now
		else if(!Multi_endgame_client_waiting){
			// if we're in the debrief state, mark down that the server has left the game
			if((gameseq_get_state() == GS_STATE_DEBRIEF) || (gameseq_get_state() == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)){
				multi_debrief_server_left();

				// add a message to the chatbox
				multi_display_chat_msg(XSTR("<The server has ended the game>",650),0,0);

				// shut our reliable socket to the server down
				psnet_rel_close_socket(&Net_player->reliable_socket);
				Net_player->reliable_socket = INVALID_SOCKET;

				// remove our do-notworking flag
				Net_player->flags &= ~(NETINFO_FLAG_DO_NETWORKING);
				
				Multi_quit_game = 0;
				return 0;
			}

			Multi_endgame_notify_code = notify_code;
			Multi_endgame_error_code = err_code;
			Multi_endgame_wsa_error = wsa_error;

			// by setting this, multi_endgame_process() will know to check and see if it is ok for us to leave
			Multi_endgame_client_waiting = 1;			
		}
	}		

	// unset the reentrancy flag
	Multi_quit_game = 0;

	return 1;
}


// ----------------------------------------------------------------------------------------------------------
// MULTI ENDGAME FORWARD DEFINITIONS
//

// called when a given netgame is about to end completely
void multi_endgame_cleanup()
{
	int idx;

	send_leave_game_packet();			

	// flush all outgoing io, force all packets through
	multi_io_send_buffered_packets();
		
	// mark myself as disconnected
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		Net_player->flags &= ~(NETINFO_FLAG_CONNECTED|NETINFO_FLAG_DO_NETWORKING);
	}
	
	/*this is a semi-hack so that if we're the master and we're quitting, we don't get an assert

    Karajorma - From the looks of things this code actually CAUSES an Int3 and doesn't cause an assert anymore
	besides if the game is over why are we setting flags on a Player_obj anyway? 

	if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Player_obj != NULL)){
		Player_obj->flags &= ~(OF_PLAYER_SHIP);
		obj_set_flags( Player_obj, Player_obj->flags | OF_COULD_BE_PLAYER );
	}
	*/
	
	// shut my socket down (will also let the server know i've received any notifications/error from him)
	// psnet_rel_close_socket( &(Net_player->reliable_socket) );

	// 11/18/98 - DB, changed the above to kill all sockets. Its the safest thing to do
	for(idx=0; idx<MAX_PLAYERS; idx++){
		psnet_rel_close_socket(&Net_players[idx].reliable_socket);
		Net_players[idx].reliable_socket = INVALID_SOCKET;
	}

	// set the game quitting flag in our local netgame info - this will _insure_ that even if we miss a packet or
	// there is some sequencing error, the next time through the multi_do_frame() loop, the game will be ended
	// Netgame.flags |= (NG_FLAG_QUITTING | NG_FLAG_ENDED);

	// close all open SPX/TCP reliable sockets
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		// do it for all players, since we're leaving anyway.
		for(idx=0;idx<MAX_PLAYERS;idx++){
			// 6/25/98 -- MWA delete all players from the game

			if ( &Net_players[idx] != Net_player ) {
				delete_player( idx );
			}			
		}
	}	

	// if we're currently in the pause state, pop back into gameplay first
	if(gameseq_get_state() == GS_STATE_MULTI_PAUSED){
		gameseq_pop_state();
	}

	// handle game disconnect from FS2NetD (NOTE: must be done *before* standalone is reset!!)
	if ( MULTI_IS_TRACKER_GAME && (Net_player->flags & NETINFO_FLAG_AM_MASTER) ) {
		fs2netd_gameserver_disconnect();
	}

	if (Game_mode & GM_STANDALONE_SERVER) {
		// multi_standalone_quit_game();		
		multi_standalone_reset_all();
	} else {		
		Player->flags |= PLAYER_FLAGS_IS_MULTI;		

		// if we're in Parallax Online mode, log back in there	
		gameseq_post_event(GS_EVENT_MULTI_JOIN_GAME);		

		// if we have an error code, bring up the discon popup						
		if ( ((Multi_endgame_notify_code != -1) || (Multi_endgame_error_code != -1)) && !(Game_mode & GM_STANDALONE_SERVER) ) {
			multi_endgame_popup(Multi_endgame_notify_code,Multi_endgame_error_code,Multi_endgame_wsa_error);			
		}		
	}

	/*
	extern CFILE *obj_stream;
	if(obj_stream != NULL){
		cfclose(obj_stream);
		obj_stream = NULL;
	}
	*/	

	// unload the multiplayer common interface palette
	multi_common_unload_palette();
	
	// reinitialize endgame stuff
	// multi_endgame_init();
}

// throw up a popup with the given notification code and optional winsock code
void multi_endgame_popup(int notify_code,int error_code,int wsa_error)
{
	char err_msg[255];
	int flags = PF_USE_AFFIRMATIVE_ICON;	

	// if there is a popup already active, just kill it
	if(popup_active()){
		// if there is already a popup active, kill it
		popup_kill_any_active();

		Int3();
	} else {
		// if there is a winsock error code, stick it on the end of the text
		if(wsa_error != -1){		
			sprintf(err_msg,NOX("WSAERROR : %d\n\n"),wsa_error);
			flags |= PF_TITLE_RED;
		} else {
			strcpy_s(err_msg,"");
		}

		// setup the error message string
		if(notify_code != MULTI_END_NOTIFY_NONE){
			switch(notify_code){
			case MULTI_END_NOTIFY_KICKED :
				strcat_s(err_msg,XSTR("You have been kicked",651));
				break;
			case MULTI_END_NOTIFY_SERVER_LEFT:
				strcat_s(err_msg,XSTR("The server has left the game",652));
				break;
			case MULTI_END_NOTIFY_FILE_REJECTED:
				strcat_s(err_msg,XSTR("Your mission file has been rejected by the server",653));
				break;
			case MULTI_END_NOTIFY_EARLY_END:
				strcat_s(err_msg,XSTR("The game has ended while you were ingame joining",654));
				break;
			case MULTI_END_NOTIFY_INGAME_TIMEOUT:
				strcat_s(err_msg,XSTR("You have waited too long to select a ship",655));
				break;
			case MULTI_END_NOTIFY_KICKED_BAD_XFER:
				strcat_s(err_msg,XSTR("You were kicked because mission file xfer failed",998));
				break;
			case MULTI_END_NOTIFY_KICKED_CANT_XFER:
				strcat_s(err_msg,XSTR("You were kicked because you do not have the builtin mission",999));
				strcat_s(err_msg, NOX(" "));
				strcat_s(err_msg, Game_current_mission_filename);
				break;
			case MULTI_END_NOTIFY_KICKED_INGAME_ENDED:
				strcat_s(err_msg,XSTR("You were kicked because you were ingame joining a game that has ended",1000));
				break;
			default : 
				Int3();
			}		
		} else {	
			switch(error_code){
			case MULTI_END_ERROR_CONTACT_LOST :
				strcat_s(err_msg,XSTR("Contact with server has been lost",656));
				break;
			case MULTI_END_ERROR_CONNECT_FAIL :
				strcat_s(err_msg,XSTR("Failed to connect to server on reliable socket",657));
				break;
			case MULTI_END_ERROR_LOAD_FAIL :
				strcat_s(err_msg,XSTR("Failed to load mission file properly",658));
				break;						
			case MULTI_END_ERROR_INGAME_SHIP :
				strcat_s(err_msg,XSTR("Unable to create ingame join player ship",659));
				break;
			case MULTI_END_ERROR_INGAME_BOGUS :
				strcat_s(err_msg,XSTR("Recevied bogus packet data while ingame joining",660));
				break;
			case MULTI_END_ERROR_STRANS_FAIL :
				strcat_s(err_msg,XSTR("Server transfer failed (obsolete)",661));
				break;
			case MULTI_END_ERROR_SHIP_ASSIGN:
				strcat_s(err_msg,XSTR("Server encountered errors trying to assign players to ships",662));
				break;
			case MULTI_END_ERROR_HOST_LEFT:
				strcat_s(err_msg,XSTR("Host has left the game, aborting...",663));
				break;			
			case MULTI_END_ERROR_XFER_FAIL:
				strcat_s(err_msg,XSTR("There was an error receiving the mission file!",665));
				break;
			case MULTI_END_ERROR_WAVE_COUNT:
				strcat_s(err_msg,XSTR("The player wings Alpha, Beta, Gamma, and Zeta must have only 1 wave.  One of these wings currently has more than 1 wave.", 987));
				break;
			// Karajorma - both of these should really be replaced with new strings in strings.tbl but for now this one has much the same meaning
			case MULTI_END_ERROR_TEAM0_EMPTY:
				strcat_s(err_msg,XSTR("All players from team 1 have left the game", 664));
				break;
			case MULTI_END_ERROR_TEAM1_EMPTY:
				strcat_s(err_msg,XSTR("All players from team 2 have left the game", 664));
				break;
			case MULTI_END_ERROR_CAPTAIN_LEFT:
				strcat_s(err_msg,XSTR("Team captain(s) have left the game, aborting...",664));
				break;
			default :
				Int3();
			}		
		}

		// show the popup
		popup(flags,1,POPUP_OK,err_msg);	
	}
}

// called when server is waiting for clients to disconnect
int multi_endgame_server_ok_to_leave()
{
	int idx,clients_gone;
	
	// check to see if our client disconnect timestamp has elapsed
	if ( Multi_endgame_server_wait_stamp > 0.0f ) {
		Multi_endgame_server_wait_stamp -= flFrametime;
		if ( Multi_endgame_server_wait_stamp <= 0.0f ) {
			return 1;
		}
	}
		
	// check to see if all clients have disconnected
	clients_gone = 1;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx])){
			clients_gone = 0;
			return 0;
		}
	}

	// all conditions passed
	return 1;
}

// check to see if we need to be warping out (strange transition possibilities)
void multi_endgame_check_for_warpout()
{
	int need_to_warpout = 0;

	// if we're not in the process of warping out - do nothing
	if(!(Net_player->flags & NETINFO_FLAG_WARPING_OUT)){
		return;
	}

	// determine if sufficient warping-out conditions exist
	if((Game_mode & GM_IN_MISSION) &&										// if i'm still in the mission
		((Netgame.game_state == NETGAME_STATE_ENDGAME)	||				// if the netgame ended
		 (Netgame.game_state == NETGAME_STATE_DEBRIEF))					// if the netgame is now in the debriefing state
	  ) {
		need_to_warpout = 1;
	}

	// if we need to be warping out but are stuck in a dead popup, cancel it
	if(need_to_warpout && (popupdead_is_active() || (Net_player->flags & NETINFO_FLAG_RESPAWNING) || (Net_player->flags & NETINFO_FLAG_OBSERVER)) ){		
		// flush all active pushed state
		multi_handle_state_special();

		// begin the warpout process		
		send_debrief_event();

		// if text input mode is active, clear it
		multi_msg_text_flush();
	}	
}
