/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_kick.cpp $
 * $Revision: 2.2 $
 * $Date: 2004-03-05 09:02:02 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 8     10/13/99 3:51p Jefff
 * fixed unnumbered XSTRs
 * 
 * 7     3/10/99 6:50p Dave
 * Changed the way we buffer packets for all clients. Optimized turret
 * fired packets. Did some weapon firing optimizations.
 * 
 * 6     3/09/99 6:24p Dave
 * More work on object update revamping. Identified several sources of
 * unnecessary bandwidth.
 * 
 * 5     11/19/98 8:03a Dave
 * Full support for D3-style reliable sockets. Revamped packet lag/loss
 * system, made it receiver side and at the lowest possible level.
 * 
 * 4     11/17/98 11:12a Dave
 * Removed player identification by address. Now assign explicit id #'s.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 16    9/15/98 7:24p Dave
 * Minor UI changes. Localized bunch of new text.
 * 
 * 15    9/11/98 5:53p Dave
 * Final revisions to kick system changes.
 * 
 * 14    9/11/98 5:08p Dave
 * More tweaks to kick notification system.
 * 
 * 13    9/11/98 4:14p Dave
 * Fixed file checksumming of < file_size. Put in more verbose kicking and
 * PXO stats store reporting.
 * 
 * 12    6/13/98 9:32p Mike
 * Kill last character in file which caused "Find in Files" to report the
 * file as "not a text file."
 * 
 * 11    6/13/98 6:01p Hoffoss
 * Externalized all new (or forgot to be added) strings to all the code.
 * 
 * 10    4/23/98 6:18p Dave
 * Store ETS values between respawns. Put kick feature in the text
 * messaging system. Fixed text messaging system so that it doesn't
 * process or trigger ship controls. Other UI fixes.
 * 
 * 9     4/04/98 4:22p Dave
 * First rev of UDP reliable sockets is done. Seems to work well if not
 * overly burdened.
 * 
 * 8     4/03/98 1:03a Dave
 * First pass at unreliable guaranteed delivery packets.
 * 
 * 7     4/01/98 11:19p Dave
 * Put in auto-loading of xferred pilot pic files. Grey out background
 * behind pinfo popup. Put a chatbox message in when players are kicked.
 * Moved mission title down in briefing. Other ui fixes.
 * 
 * 6     3/30/98 6:27p Dave
 * Put in a more official set of multiplayer options, including a system
 * for distributing netplayer and netgame settings.
 * 
 * 5     3/24/98 5:00p Dave
 * Fixed several ui bugs. Put in pre and post voice stream playback sound
 * fx. Put in error specific popups for clients getting dropped from games
 * through actions other than their own.
 * 
 * 4     3/18/98 5:52p Dave
 * Put in netgame password popup. Numerous ui changes. Laid groundwork for
 * streamed multi_voice data.
 * 
 * 3     3/17/98 5:29p Dave
 * Minor bug fixes in player select menu. Solidified mp joining process.
 * Made furball mode support ingame joiners and dropped players correctly.
 * 
 * 2     3/15/98 4:17p Dave
 * Fixed oberver hud problems. Put in handy netplayer macros. Reduced size
 * of network orientation matrices.
 * 
 * 1     2/12/98 4:38p Dave
 * Multiplayer kick functionality
 * 
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "network/multi.h"
#include "network/multi_kick.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "freespace2/freespace.h"
#include "playerman/player.h"
#include "io/timer.h"

// ----------------------------------------------------------------------------------
// KICK DEFINES/VARS
//

#define MULTI_KICK_RESPONSE_TIME						4000		// if someone who has been kicked has not responded in this time, disconnect him hard

#define MAX_BAN_SLOTS		30
net_addr Multi_kick_ban_slots[MAX_BAN_SLOTS];				// banned addresses
int Multi_kick_num_ban_slots;										// the # of banned addresses
							 
// ----------------------------------------------------------------------------------
// KICK FORWARD DECLARATIONS
//

// send a player kick packet
void send_player_kick_packet(int player_index, int ban = 1, int reason = KICK_REASON_NORM);

// process a player kick packet
void process_player_kick_packet(ubyte *data, header *hinfo);

// add a net address to the banned list
void multi_kick_add_ban(net_addr *addr);

// can the given player perform a kick
int multi_kick_can_kick(net_player *player);


// ----------------------------------------------------------------------------------
// KICK FUNCTIONS
//

// initialize all kicking details (ban lists, etc). it is safe to call this function at any time
void multi_kick_init()
{
	// blast all the ban slots
	memset(Multi_kick_ban_slots,0,sizeof(net_addr)*MAX_BAN_SLOTS);
	Multi_kick_num_ban_slots = 0;
}

// process all kick details (disconnecting players who have been kicked but haven't closed their socket)
void multi_kick_process()
{
	int idx;

	// if i'm not the server, don't do anything
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		return;
	}

	// disconnect any kicked players who have timed out on leaving
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].s_info.kick_timestamp != -1) && timestamp_elapsed(Net_players[idx].s_info.kick_timestamp) ){
			delete_player(idx, Net_players[idx].s_info.kick_reason);
		}
	}
}

// attempt to kick a player. return success or fail
void multi_kick_player(int player_index, int ban, int reason)
{	
	// only the standalone should be able to kick the host of the game
	if(!(Game_mode & GM_STANDALONE_SERVER) && ((Net_players[player_index].flags & NETINFO_FLAG_GAME_HOST) || (Net_players[player_index].flags & NETINFO_FLAG_AM_MASTER))){
		nprintf(("Network","Cannot kick the host or server of a game!\n"));
	} else {
		// if we're the master, then delete the guy
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			// if we're supposed to ban him, add his address to the banned list
			if(ban){
				multi_kick_add_ban(&Net_players[player_index].p_info.addr);
			}

			// mark him as having been kicked
			Net_players[player_index].flags |= NETINFO_FLAG_KICKED;

			// set his kick timestamp and send him a leave game packet
			Net_players[player_index].s_info.kick_timestamp = timestamp(MULTI_KICK_RESPONSE_TIME);
			Net_players[player_index].s_info.kick_reason = reason;
			send_leave_game_packet(Net_players[player_index].player_id, reason, &Net_players[player_index]);							

			// tell everyone else that he was kicked
			send_leave_game_packet(Net_players[player_index].player_id, reason);
			
			// wait until he either shuts his connection down or he times out)
			// add the string to the chatbox and the hud (always safe - if it is not inited, nothing bad will happen)			
			char str[512];
			memset(str, 0, 512);
			sprintf(str, XSTR("<kicking %s ...>", 1501), Net_players[player_index].player->callsign);
			multi_display_chat_msg(str, player_index, 0);							 
		}
		// otherwise, we should send the packet indicating that this guy should be kicked
		else {
			send_player_kick_packet(player_index, ban, reason);
		}
	}
}

// is this net address currently kicked and banded
int multi_kick_is_banned(net_addr *addr)
{
	int idx;
	
	// traverse the banned list
	for(idx=0;idx<Multi_kick_num_ban_slots;idx++){
		// if we found a duplicate
		if(psnet_same(&Multi_kick_ban_slots[idx],addr)){
			return 1;
		}
	}

	// he's not banned
	return 0;
}

// debug console function called to determine which player to kick
void multi_dcf_kick()
{
	int player_num,idx;

	// get the callsign of the player to kick
	dc_get_arg(ARG_STRING);

	if(strlen(Dc_arg) == 0){
		dc_printf("Invalid player callsign!\n");
		return ;
	}

	player_num = -1;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (stricmp(Net_players[idx].player->callsign,Dc_arg)==0)){
			player_num = idx;
			break;
		}
	}

	// if we didn't find the player, notify of the results
	if(player_num == -1){
		dc_printf("Could not find player %s to kick!",Dc_arg);
	} 
	// if we found the guy, then try and kick him
	else {
		multi_kick_player(player_num);
	}
}

// fill in the passed string with the appropriate "kicked" string
void multi_kick_get_text(net_player *pl, int reason, char *str)
{
	// safety net
	if((pl == NULL) || (pl->player == NULL)){
		strcpy(str, NOX(""));
	}

	switch(reason){
	case KICK_REASON_BAD_XFER:
		sprintf(str, XSTR("<%s was kicked because of mission file xfer failure>", 1003), pl->player->callsign);
		break;
	case KICK_REASON_CANT_XFER:
		sprintf(str, XSTR("<%s was kicked for not having builtin mission %s>", 1004), pl->player->callsign, Game_current_mission_filename);
		break;
	case KICK_REASON_INGAME_ENDED:
		sprintf(str, XSTR("<%s was kicked for ingame joining an ended game>",1005), pl->player->callsign);
		break;
	default:
		sprintf(str, XSTR("<%s was kicked>",687), pl->player->callsign);
		break;
	}		
}


// ----------------------------------------------------------------------------------
// KICK FORWARD DEFINITIONS
//

// add a net address to the banned list
void multi_kick_add_ban(net_addr *addr)
{
	// if we still have any slots left
	if(Multi_kick_num_ban_slots < (MAX_BAN_SLOTS - 1)){
		memcpy(&Multi_kick_ban_slots[Multi_kick_num_ban_slots++],addr,sizeof(net_addr));
	}
}


// ----------------------------------------------------------------------------------
// KICK PACKET HANDLERS
//

// send a player kick packet
void send_player_kick_packet(int player_index, int ban, int reason)
{		
	ubyte data[MAX_PACKET_SIZE];
	int packet_size = 0;

	BUILD_HEADER(KICK_PLAYER);

	// add the address of the player to be kicked
	ADD_DATA(Net_players[player_index].player_id);
	
	// indicate if he should be banned
	ADD_DATA(ban);
	ADD_DATA(reason);

	// send the request to the server	
	multi_io_send_reliable(Net_player, data, packet_size);
}

// process a player kick packet
void process_player_kick_packet(ubyte *data, header *hinfo)
{
	int player_num,from_player,ban,reason;	
	short player_id;
	int offset = HEADER_LENGTH;
	
	// get the address of the guy who is to be kicked
	GET_DATA(player_id);
	GET_DATA(ban);
	GET_DATA(reason);
	player_num = find_player_id(player_id);
	PACKET_SET_SIZE();

	// only the server should ever receive a request to kick a guy
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);
	
	// determine who sent the packet	
	from_player = find_player_id(hinfo->id);

	// check to see if this guy is allowed to make such a request
	if((from_player == -1) || !multi_kick_can_kick(&Net_players[from_player]) ){
		nprintf(("Network","Received a kick request from an invalid player!!\n"));
	} 
	// otherwise, process the request fully
	else {
		// make sure we have a valid player to kick
		if(player_num == -1){
			nprintf(("Network","Received request to kick an unknown player!\n"));
		} else {
			// will handle all the rest of the details
			multi_kick_player(player_num,ban,reason);
		}
	}
}

// can the given player perform a kick
int multi_kick_can_kick(net_player *player)
{
	// only host or server can kick
	if((player->flags & NETINFO_FLAG_AM_MASTER) || (player->flags & NETINFO_FLAG_GAME_HOST)){
		return 1;
	}
	
	// this guy cannot kick
	return 0;
}
