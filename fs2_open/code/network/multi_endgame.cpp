/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_endgame.cpp $
 * $Revision: 2.11 $
 * $Date: 2007-04-24 13:13:04 $
 * $Author: karajorma $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.10  2005/10/10 17:21:07  taylor
 * remove NO_NETWORK
 *
 * Revision 2.9  2005/07/13 03:25:59  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.8  2005/03/02 21:18:19  taylor
 * better support for Inferno builds (in PreProcDefines.h now, no networking support)
 * make sure NO_NETWORK builds are as friendly on Windows as it is on Linux/OSX
 * revert a timeout in Client.h back to the original value before Linux merge
 *
 * Revision 2.7  2005/01/31 23:27:55  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 2.6  2004/07/26 20:47:42  Kazan
 * remove MCD complete
 *
 * Revision 2.5  2004/07/12 16:32:57  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.4  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2003/11/11 02:15:45  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.2  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/22 01:22:25  penguin
 * Linux port -- added NO_STANDALONE ifdefs
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 10    8/22/99 1:55p Dave
 * Cleaned up host/team-captain leaving code.
 * 
 * 9     8/22/99 1:19p Dave
 * Fixed up http proxy code. Cleaned up scoring code. Reverse the order in
 * which d3d cards are detected.
 * 
 * 8     4/08/99 1:05p Dave
 * Fixed some leave game packet problems. Updated builtin mission list for
 * multiplayer.
 * 
 * 7     3/24/99 4:05p Dave
 * Put in support for assigning the player to a specific squadron with a
 * specific logo. Preliminary work for doing pos/orient checksumming in
 * multiplayer to reduce bandwidth.
 * 
 * 6     11/19/98 4:57p Dave
 * Ignore PXO option if IPX is selected.
 * 
 * 5     11/19/98 4:19p Dave
 * Put IPX sockets back in psnet. Consolidated all multiplayer config
 * files into one.
 * 
 * 4     11/19/98 8:03a Dave
 * Full support for D3-style reliable sockets. Revamped packet lag/loss
 * system, made it receiver side and at the lowest possible level.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 36    9/17/98 3:08p Dave
 * PXO to non-pxo game warning popup. Player icon stuff in create and join
 * game screens. Upped server count refresh time in PXO to 35 secs (from
 * 20).
 * 
 * 35    9/15/98 7:24p Dave
 * Minor UI changes. Localized bunch of new text.
 * 
 * 34    9/14/98 3:40p Allender
 * better error checking for invalid number of waves for player wings in a
 * multiplayer game.  Better popup message in FreeSpace side.
 * 
 * 33    9/11/98 5:53p Dave
 * Final revisions to kick system changes.
 * 
 * 32    9/11/98 5:08p Dave
 * More tweaks to kick notification system.
 * 
 * 31    9/11/98 4:14p Dave
 * Fixed file checksumming of < file_size. Put in more verbose kicking and
 * PXO stats store reporting.
 * 
 * 30    8/07/98 10:15a Allender
 * changed the way the endgame sequencing timer works so that timer wrap
 * doesn't hurt.  Also use the obj_set_flags for the COULD_BE_PLAYER flag
 * 
 * 29    7/24/98 9:27a Dave
 * Tidied up endgame sequencing by removing several old flags and
 * standardizing _all_ endgame stuff with a single function call.
 * 
 * 28    7/13/98 5:34p Lawrance
 * index a localized string in multi_endgame.cpp
 * 
 * 27    7/13/98 5:19p Dave
 * 
 * 26    6/30/98 4:53p Allender
 * fixed endgame problems where standalone wasn't properly dropping
 * everyone out of a game.  Be sure that timestamp for endgame processing
 * gets reset
 * 
 * 25    6/13/98 9:32p Mike
 * Kill last character in file which caused "Find in Files" to report the
 * file as "not a text file."
 * 
 * 24    6/13/98 6:01p Hoffoss
 * Externalized all new (or forgot to be added) strings to all the code.
 * 
 * 23    6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 22    5/24/98 8:15p Dave
 * Tweaked pxo some more. 
 * 
 * 21    5/21/98 10:09a Dave
 * Enable DNS checking for PXO and both trackers.
 * Fixed problem with leaving the mission from the pause state. Fixed
 * build errors in multimsgs.cpp
 * 
 * 20    5/20/98 9:01p Allender
 * change RELEASE to NDEBUG.  Fix reentryancy problem in process_endgame
 * for multiplayer
 * 
 * 19    5/17/98 11:54p Allender
 * only clear flying controls when in mission
 * 
 * 18    5/17/98 11:34p Allender
 * deal with resetting player controls better
 * 
 * 17    5/17/98 6:32p Dave
 * Make sure clients/servers aren't kicked out of the debriefing when team
 * captains leave a game. Fixed chatbox off-by-one error. Fixed image
 * xfer/pilot info popup stuff.
 * 
 * 16    5/15/98 5:15p Dave
 * Fix a standalone resetting bug.Tweaked PXO interface. Display captaincy
 * status for team vs. team. Put in asserts to check for invalid team vs.
 * team situations.
 * 
 * 15    5/15/98 12:09a Dave
 * New tracker api code. New game tracker code. Finished up first run of
 * the PXO screen. Fixed a few game server list exceptions.
 * 
 * 14    5/14/98 12:40a Dave
 * Still more additions to the PXO screen. Updated tracker code.
 * 
 * 13    5/09/98 7:16p Dave
 * Put in CD checking. Put in standalone host password. Made pilot into
 * popup scrollable.
 * 
 * 12    5/08/98 5:05p Dave
 * Go to the join game screen when quitting multiplayer. Fixed mission
 * text chat bugs. Put mission type symbols on the create game list.
 * Started updating standalone gui controls.
 * 
 * 11    5/07/98 6:26p Dave
 * Fix strange boundary conditions which arise when players die/respawn
 * while the game is being ended. Spiff up the chatbox doskey thing a bit.
 * 
 * 10    5/07/98 3:29p Jim
 * Make sure standalone doesn't display a popup when ending a game.
 * 
 * 9     5/05/98 5:02p Dave
 * Fix end-of-campaign sequencing to work right. Make all individual
 * missions of a campaign replayable.
 * 
 * 8     5/04/98 10:39p Dave
 * Put in endgame sequencing.  Need to check campaign situations.
 * Realigned ship info on team select screen.
 * 
 * 7     5/04/98 1:43p Dave
 * Fixed up a standalone resetting problem. Fixed multiplayer stats
 * collection for clients. Make sure all multiplayer ui screens have the
 * correct palette at all times.
 * 
 * 6     5/03/98 7:04p Dave
 * Make team vs. team work mores smoothly with standalone. Change how host
 * interacts with standalone for picking missions. Put in a time limit for
 * ingame join ship select. Fix ingame join ship select screen for Vasudan
 * ship icons.
 * 
 * 5     4/30/98 5:12p Dave
 * Fixed game polling code for joining clients. Reworked some file xfer
 * stuff.
 * 
 * 4     4/30/98 12:13a Allender
 * reset control info and afterburner when entering the quit game code.
 * Prevents odd things from happening while the popup is up.
 * 
 * 3     4/28/98 5:10p Dave
 * Fixed multi_quit_game() client side sequencing problem. Turn off
 * afterburners when ending multiplayer mission. Begin integration of mt
 * API from Kevin Bentley.
 * 
 * 2     4/23/98 1:28a Dave
 * Seemingly nailed the current_primary_bank and current_secondary_bank -1
 * problem. Made sure non-critical button presses are _never_ sent to the
 * server.
 * 
 * 1     4/22/98 5:50p Dave
 *  
 * 
 * $NoKeywords: $
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
		
	// this is a semi-hack so that if we're the master and we're quitting, we don't get an assert
	if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Player_obj != NULL)){
		Player_obj->flags &= ~(OF_PLAYER_SHIP);
		obj_set_flags( Player_obj, Player_obj->flags | OF_COULD_BE_PLAYER );
	}
	
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

#ifndef NO_STANDALONE
	if(Game_mode & GM_STANDALONE_SERVER){
		// multi_standalone_quit_game();		
		multi_standalone_reset_all();
	} else 
#endif
        {		
		Player->flags |= PLAYER_FLAGS_IS_MULTI;		

		// if we're in Parallax Online mode, log back in there	
		gameseq_post_event(GS_EVENT_MULTI_JOIN_GAME);		

		// if we have an error code, bring up the discon popup						
		if((Multi_endgame_notify_code != -1) || (Multi_endgame_error_code != -1) && !(Game_mode & GM_STANDALONE_SERVER)){
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
			strcpy(err_msg,"");
		}

		// setup the error message string
		if(notify_code != MULTI_END_NOTIFY_NONE){
			switch(notify_code){
			case MULTI_END_NOTIFY_KICKED :
				strcat(err_msg,XSTR("You have been kicked",651));
				break;
			case MULTI_END_NOTIFY_SERVER_LEFT:
				strcat(err_msg,XSTR("The server has left the game",652));
				break;
			case MULTI_END_NOTIFY_FILE_REJECTED:
				strcat(err_msg,XSTR("Your mission file has been rejected by the server",653));
				break;
			case MULTI_END_NOTIFY_EARLY_END:
				strcat(err_msg,XSTR("The game has ended while you were ingame joining",654));
				break;
			case MULTI_END_NOTIFY_INGAME_TIMEOUT:
				strcat(err_msg,XSTR("You have waited too long to select a ship",655));
				break;
			case MULTI_END_NOTIFY_KICKED_BAD_XFER:
				strcat(err_msg,XSTR("You were kicked because mission file xfer failed",998));
				break;
			case MULTI_END_NOTIFY_KICKED_CANT_XFER:
				strcat(err_msg,XSTR("You were kicked because you do not have the builtin mission",999));
				strcat(err_msg, NOX(" "));
				strcat(err_msg, Game_current_mission_filename);
				break;
			case MULTI_END_NOTIFY_KICKED_INGAME_ENDED:
				strcat(err_msg,XSTR("You were kicked because you were ingame joining a game that has ended",1000));
				break;
			default : 
				Int3();
			}		
		} else {	
			switch(error_code){
			case MULTI_END_ERROR_CONTACT_LOST :
				strcat(err_msg,XSTR("Contact with server has been lost",656));
				break;
			case MULTI_END_ERROR_CONNECT_FAIL :
				strcat(err_msg,XSTR("Failed to connect to server on reliable socket",657));
				break;
			case MULTI_END_ERROR_LOAD_FAIL :
				strcat(err_msg,XSTR("Failed to load mission file properly",658));
				break;						
			case MULTI_END_ERROR_INGAME_SHIP :
				strcat(err_msg,XSTR("Unable to create ingame join player ship",659));
				break;
			case MULTI_END_ERROR_INGAME_BOGUS :
				strcat(err_msg,XSTR("Recevied bogus packet data while ingame joining",660));
				break;
			case MULTI_END_ERROR_STRANS_FAIL :
				strcat(err_msg,XSTR("Server transfer failed (obsolete)",661));
				break;
			case MULTI_END_ERROR_SHIP_ASSIGN:
				strcat(err_msg,XSTR("Server encountered errors trying to assign players to ships",662));
				break;
			case MULTI_END_ERROR_HOST_LEFT:
				strcat(err_msg,XSTR("Host has left the game, aborting...",663));
				break;			
			case MULTI_END_ERROR_XFER_FAIL:
				strcat(err_msg,XSTR("There was an error receiving the mission file!",665));
				break;
			case MULTI_END_ERROR_WAVE_COUNT:
				strcat(err_msg,XSTR("The player wings Alpha, Beta, Gamma, and Zeta must have only 1 wave.  One of these wings currently has more than 1 wave.", 987));
				break;
			case MULTI_END_ERROR_TEAM0_EMPTY:
				strcat(err_msg,XSTR("All players from team 1 have left the game", 1466));
				break;
			case MULTI_END_ERROR_TEAM1_EMPTY:
				strcat(err_msg,XSTR("All players from team 2 have left the game", 1467));
				break;
			case MULTI_END_ERROR_CAPTAIN_LEFT:
				strcat(err_msg,XSTR("Team captain(s) have left the game, aborting...",664));
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
