/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_team.cpp $
 * $Revision: 2.5 $
 * $Date: 2004-12-14 14:46:13 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2004/07/26 20:47:42  Kazan
 * remove MCD complete
 *
 * Revision 2.3  2004/07/12 16:32:57  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.2  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:08  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 13    9/08/99 1:59p Dave
 * Nailed the problem with assigning teams improperly in TvT. Basic
 * problem was that clients were sending team update packets with bogus
 * info that the server was willingly accepting, thereby blowing away his
 * own info.
 * 
 * 12    8/22/99 1:55p Dave
 * Cleaned up host/team-captain leaving code.
 * 
 * 11    8/22/99 1:19p Dave
 * Fixed up http proxy code. Cleaned up scoring code. Reverse the order in
 * which d3d cards are detected.
 * 
 * 10    4/09/99 2:21p Dave
 * Multiplayer beta stuff. CD checking.
 * 
 * 9     3/10/99 6:50p Dave
 * Changed the way we buffer packets for all clients. Optimized turret
 * fired packets. Did some weapon firing optimizations.
 * 
 * 8     3/09/99 6:24p Dave
 * More work on object update revamping. Identified several sources of
 * unnecessary bandwidth.
 * 
 * 7     2/17/99 2:11p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 6     2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
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
 * 18    6/13/98 3:19p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 17    5/26/98 7:33p Dave
 * Once extra check for TvT oktocommit. Tested against all cases.
 * 
 * 16    5/22/98 9:35p Dave
 * Put in channel based support for PXO. Put in "shutdown" button for
 * standalone. UI tweaks for TvT
 * 
 * 15    5/15/98 5:16p Dave
 * Fix a standalone resetting bug.Tweaked PXO interface. Display captaincy
 * status for team vs. team. Put in asserts to check for invalid team vs.
 * team situations.
 * 
 * 14    5/03/98 7:04p Dave
 * Make team vs. team work mores smoothly with standalone. Change how host
 * interacts with standalone for picking missions. Put in a time limit for
 * ingame join ship select. Fix ingame join ship select screen for Vasudan
 * ship icons.
 * 
 * 13    4/22/98 5:53p Dave
 * Large reworking of endgame sequencing. Updated multi host options
 * screen for new artwork. Put in checks for host or team captains leaving
 * midgame.
 * 
 * 12    4/15/98 5:03p Dave
 * Put in a rough countdown to mission start on final sync screen. Fixed
 * several team vs. team bugs on the ship/team select screen.
 * 
 * 11    4/14/98 12:57a Dave
 * Made weapon select screen show netplayer names above ships. Fixed pilot
 * info popup to show the status of pilot images more correctly.
 * 
 * 10    4/08/98 2:51p Dave
 * Fixed pilot image xfer once again. Solidify team selection process in
 * pre-briefing multiplayer.
 * 
 * 9     4/04/98 4:22p Dave
 * First rev of UDP reliable sockets is done. Seems to work well if not
 * overly burdened.
 * 
 * 8     4/03/98 1:03a Dave
 * First pass at unreliable guaranteed delivery packets.
 * 
 * 7     3/31/98 4:51p Dave
 * Removed medals screen and multiplayer buttons from demo version. Put in
 * new pilot popup screen. Make ships in mp team vs. team have proper team
 * ids. Make mp respawns a permanent option saved in the player file.
 * 
 * 6     3/15/98 4:17p Dave
 * Fixed oberver hud problems. Put in handy netplayer macros. Reduced size
 * of network orientation matrices.
 * 
 * 5     3/12/98 5:45p Dave
 * Put in new observer HUD. Made it possible for observers to join at the
 * beginning of a game and follow it around as an observer full-time.
 * 
 * 4     3/10/98 4:26p Dave
 * Second pass at furball mode. Fixed several team vs. team bugs.
 * 
 * 3     3/09/98 5:54p Dave
 * Fixed stats to take asteroid hits into account. Polished up UI stuff in
 * team select. Finished up pilot info popup. Tracked down and fixed
 * double click bug.
 * 
 * 2     3/03/98 8:55p Dave
 * Finished pre-briefing team vs. team support.
 * 
 * 1     3/03/98 5:09p Dave
 *  
 * $NoKeywords: $
 */

#include "network/multi_team.h"
#include "globalincs/linklist.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multi_endgame.h"
#include "network/multi_pmsg.h"
#include "network/multi.h"
#include "object/object.h"
#include "ship/ship.h"

#ifndef NDEBUG
#include "playerman/player.h"
#endif


// ------------------------------------------------------------------------------------
// MULTIPLAYER TEAMPLAY DEFINES/VARS
//

// packet codes
#define MT_CODE_TEAM_UPDATE			0							// send a full team update on a player-per-player basis
#define MT_CODE_TEAM_REQUEST			1							// a request sent to the host to be be on a given team

//XSTR:OFF

// score for teams 0 and 1 for this mission
int Multi_team0_score = 0;
int Multi_team1_score = 0;


// ------------------------------------------------------------------------------------
// MULTIPLAYER TEAMPLAY FORWARD DECLARATIONS
//

// process a request to change a team
void multi_team_process_team_change_request(net_player *pl,net_player *who_from,int team);

// send a packet to the host requesting to change my team 
void multi_team_send_team_request(net_player *pl,int team);

// have the netgame host assign default teams
void multi_team_host_assign_default_teams();

// check to make sure all teams have a proper captain.
void multi_team_sync_captains();

// process a team update packet, return bytes processed
int multi_team_process_team_update(ubyte *data);


// ------------------------------------------------------------------------------------
// MULTIPLAYER TEAMPLAY FUNCTIONS
//

// call before level load (pre-sync)
void multi_team_level_init()
{
	// score for teams 0 and 1 for this mission
	Multi_team0_score = 0;
	Multi_team1_score = 0;
}

// call to determine who won the sw match, -1 == tie, 0 == team 0, 1 == team 1
int multi_team_winner()
{
	// determine who won

	// tie situation
	if(Multi_team0_score == Multi_team1_score){
		return -1;
	}

	// team 0 won
	if(Multi_team0_score > Multi_team1_score){
		return 0;
	}

	// team 1 must have won
	return 1;
}

// call to add score to a team
void multi_team_maybe_add_score(int points, int team)
{
	// if we're not in multiplayer mode
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// if not squad war
	if(!(Netgame.type_flags & NG_TYPE_TEAM)){
		return;
	}

	// if i'm not the server of the game, bail here
	if(!MULTIPLAYER_MASTER){
		return;
	}

	// add team score
	switch(team){
	case 0:
		nprintf(("Network", "TVT : adding %d points to team 0 (total == %d)\n", points, points + Multi_team0_score));
		Multi_team0_score += points;
		break;

	case 1:
		nprintf(("Network", "TVT : adding %d points to team 1 (total == %d)\n", points, points + Multi_team1_score));
		Multi_team1_score += points;
		break;
	}
}

// reset all players and assign them to default teams
void multi_team_reset()
{
	int idx;

	nprintf(("Network","MULTI TEAM : resetting\n"));
	
	// unset everyone's captaincy and locked flags	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx])){
			Net_players[idx].p_info.team = 0;
			Net_players[idx].flags &= ~(NETINFO_FLAG_TEAM_LOCKED | NETINFO_FLAG_TEAM_CAPTAIN);
		}
	}

	// host should divvy up the teams, send a reset notice to all players and assign captains
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		// divvy up the teams here
		multi_team_host_assign_default_teams();		
	}	
}

// set the captaincy status of this player
void multi_team_set_captain(net_player *pl,int set)
{
	// only the host should ever get here!
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);

	// set the player flags as being a captain and notify everyone else of this
	if(set){
		nprintf(("Network","MULTI TEAM : Server setting player %s team %d captain\n",pl->player->callsign,pl->p_info.team));
		pl->flags |= NETINFO_FLAG_TEAM_CAPTAIN;
	} else {
		nprintf(("Network","MULTI TEAM : Server unsetting player %s as team %d captain\n",pl->player->callsign,pl->p_info.team));
		pl->flags &= ~(NETINFO_FLAG_TEAM_CAPTAIN);
	}	
}

// set the team of this given player (if called by the host, the player becomes locked, cnad only the host can modify him from thereon)
void multi_team_set_team(net_player *pl,int team)
{	
	// if i'm the server of the game, do it now
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		nprintf(("Network","MULTI TEAM : Server/Host setting player %s to team %d and locking\n",pl->player->callsign,team));

		pl->p_info.team = team;

		if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
			pl->flags |= NETINFO_FLAG_TEAM_LOCKED;
		}
		pl->flags &= ~(NETINFO_FLAG_TEAM_CAPTAIN);		
						
		// check to see if the resulting team rosters need a captain
		multi_team_sync_captains();
	} else {
		nprintf(("Network","MULTI TEAM : Sending team change request to server\n"));
		multi_team_send_team_request(pl,team);
	}

	// verify that we have valid team stuff
	multi_team_verify();
}

// process a request to change a team
void multi_team_process_team_change_request(net_player *pl,net_player *who_from,int team)
{
	// if this player has already been locked, don't do anything
	if((pl->flags & NETINFO_FLAG_TEAM_LOCKED) && !(who_from->flags & NETINFO_FLAG_GAME_HOST)){
		nprintf(("Network","MULTI TEAM : Server ignoring team change request because player is locked\n"));
		return;
	} 

	// otherwise set the team for the player and send an update
	nprintf(("Network","MULTI TEAM : Server changing player %s to team %d from client request\n",pl->player->callsign,team));
	pl->p_info.team = team;
	pl->flags &= ~(NETINFO_FLAG_TEAM_CAPTAIN);					

	// if this came from the host, lock the player down
	if(who_from->flags & NETINFO_FLAG_GAME_HOST){
		pl->flags |= NETINFO_FLAG_TEAM_LOCKED;
	}

	// check to see if the resulting team rosters need a captain
	multi_team_sync_captains();	

	// send a team update
	multi_team_send_update();

	// verify that we have valid team stuff
	multi_team_verify();
}

// have the netgame host assign default teams
void multi_team_host_assign_default_teams()
{
	int player_count,idx;
	int team0_count;
	
	// first determine how many players there are in the game
	player_count = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// count anyone except the standalone server (if applicable)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx])){
			player_count++;
		}
	}

	// determine how many players should be stuck on team 0
	if(player_count < 2){
		team0_count = 1;
	} else {
		team0_count = player_count / 2;
	}

	// assign the players to team 0
	idx = 0;
	while(team0_count > 0){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx])){
			Net_players[idx].p_info.team = 0;
			Net_players[idx].flags &= ~(NETINFO_FLAG_TEAM_LOCKED);
			Net_players[idx].flags &= ~(NETINFO_FLAG_TEAM_CAPTAIN);
			team0_count--;
		}

		idx++;
	}

	// assign the remaining players to team 1
	while(idx < MAX_PLAYERS){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx])){
			Net_players[idx].p_info.team = 1;
			Net_players[idx].flags &= ~(NETINFO_FLAG_TEAM_LOCKED);
			Net_players[idx].flags &= ~(NETINFO_FLAG_TEAM_CAPTAIN);			
		}		
		idx++;	
	}		

	// sync captains up
	multi_team_sync_captains();

	// send a full update to all players
	multi_team_send_update();

	// verify that we have valid team stuff
	multi_team_verify();
}

// check to see if the team this player on needs a captain and assign him if so
void multi_team_sync_captains()
{
	int idx;
	int team0_cap,team1_cap;

	// if I'm not the server, bail
	if(!MULTIPLAYER_MASTER){		
		return;
	}
	
	// determine if any team now needs a captain	
	team0_cap = 0;
	team1_cap = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN)){
			switch(Net_players[idx].p_info.team){
			case 0 :
				team0_cap = 1;
				break;
			case 1 : 
				team1_cap = 1;
				break;
			}
		}
	}

	// if team 0 needs a captain, get one
	if(!team0_cap){
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].p_info.team == 0) && !(Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN)){
				multi_team_set_captain(&Net_players[idx],1);
				break;
			}
		}
	}

	// if team 1 needs a captain, get one
	if(!team1_cap){
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].p_info.team == 1) && !(Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN)){
				multi_team_set_captain(&Net_players[idx],1);
				break;
			}
		}
	}			
}

// is it ok for the host to hit commit
int multi_team_ok_to_commit()
{
	int team0_captains,team1_captains;
	int team0_count,team1_count;
	int idx;		

	// verify that we have valid team stuff
	multi_team_verify();
	
	// check to see if both teams have a captain
	team0_captains = 0;
	team1_captains = 0;

	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx])){
			// if this is team 0's captain
			if((Net_players[idx].p_info.team == 0) && (Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN)){
				team0_captains++;
			} else if((Net_players[idx].p_info.team == 1) && (Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN)){
				team1_captains++;
			}
		}
	}

	// check to see if both teams has <= 4 players
	multi_team_get_player_counts(&team0_count,&team1_count);
	team0_count = ((team0_count <= 4) && (team0_count > 0)) ? 1 : 0;
	team1_count = ((team1_count <= 4) && (team1_count > 0)) ? 1 : 0;

	return ((team0_captains == 1) && (team1_captains == 1) && team0_count && team1_count) ? 1 : 0;
}

// handle a player drop
void multi_team_handle_drop()
{		
	int idx;
	int team0_cap, team1_cap;

	// if I'm not the server, bail
	if(!MULTIPLAYER_MASTER){
		return;
	}
	
	// if we're not in a team vs team situation, we don't care
	if(!(Netgame.type_flags & NG_TYPE_TEAM)){
		return;
	}

	// is either team at 0 players, ingame?
	if(Game_mode & GM_IN_MISSION){
		int team0_count, team1_count;
		multi_team_get_player_counts(&team0_count, &team1_count);
		if(team0_count <= 0){
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_TEAM0_EMPTY);
			return;
		}
		if(team1_count <= 0){
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_TEAM1_EMPTY);
			return;
		}
	}

	// check to see if a team captain has left the game		
	team0_cap = 0;
	team1_cap = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN)){
			switch(Net_players[idx].p_info.team){
			case 0 :
				team0_cap = 1;
				break;
			case 1 : 
				team1_cap = 1;
				break;
			}
		}
	}
	
	// if we have lost a team captain and we're not in the forming state, abort the game
	if((!team0_cap || !team1_cap) && (Netgame.game_state != NETGAME_STATE_FORMING)){
		// if we're in-mission, just sync up captains
		if(Game_mode & GM_IN_MISSION){
			// sync up captains
			multi_team_sync_captains();			

			// send a team update
			multi_team_send_update();

			// find the player who is the new captain
			int team_check = team0_cap ? 1 : 0;
			for(idx=0; idx<MAX_PLAYERS; idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN) && (Net_players[idx].p_info.team == team_check)){			
					send_host_captain_change_packet(Net_players[idx].player_id, 1);
				}
			}
			return;
		}
		
		// quit the game
		multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_CAPTAIN_LEFT);
	} 
	// otherwise sync things up and be done with it.
	else {
		// sync up team captains	
		multi_team_sync_captains();
	
		// send a team update
		multi_team_send_update();

		// verify that we have valid team stuff
		multi_team_verify();
	}	
}

// handle a player join
void multi_team_handle_join(net_player *pl)
{
	int team0_count,team1_count,idx,team_select;
	
	// if we're not in a team vs team situation, we don't care
	if(!(Netgame.type_flags & NG_TYPE_TEAM)){
		return;
	}	
	
	// if the joining player is joining as an observer, don't put him on a team
	if(pl->flags & NETINFO_FLAG_OBSERVER){
		return;
	}

	// only the host should ever do this
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		return;
	}

	// count the # of players on each time
	team0_count = 0;
	team1_count = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx])){
			// player is on team 0
			if(Net_players[idx].p_info.team == 0){
				team0_count++;
			}
			// player is on team 1
			else if(Net_players[idx].p_info.team == 1){
				team1_count++;
			} 
			// some other case - should never happen
			else {
				Int3();
			}
		}
	}

	// determine what team he should be on
	if((team0_count == team1_count) || (team0_count < team1_count)){
		team_select = 0;
	} else {
		team_select = 1;
	}

	// place him on the team, but don't lock him yet
	multi_team_set_team(pl,team_select);
	pl->flags &= ~(NETINFO_FLAG_TEAM_LOCKED);		

	// send a team update
	multi_team_send_update();

	// verify that we have valid team stuff
	multi_team_verify();
}

// set all ships in the mission to be marked as the proper team (TEAM_HOSTILE, TEAM_FRIENLY)
void multi_team_mark_all_ships()
{
	ship_obj *moveup;	
	
   // look through all ships in the mission
	moveup = GET_FIRST(&Ship_obj_list);
	while(moveup!=END_OF_LIST(&Ship_obj_list)){
		multi_team_mark_ship(&Ships[Objects[moveup->objnum].instance]);		

		moveup = GET_NEXT(moveup);
	}	

	// verify that we have valid team stuff
	multi_team_verify();
}

// set the proper team for the passed in ship
void multi_team_mark_ship(ship *sp)
{	
	int team_num;

	// no team found yet
	team_num = -1;
		
	// look through team 0
	if (sp->wingnum == TVT_wings[0])
		team_num = 0;

	// look through team 1 if necessary
	if (sp->wingnum == TVT_wings[1])
		team_num = 1;

	// if we found a team
	switch(team_num){
	case 0 :
		sp->team = TEAM_FRIENDLY;
		break;
	case 1 :
		sp->team = TEAM_HOSTILE;
		break;
	}

	// verify that we have valid team stuff
	multi_team_verify();
}

// host locks all players into their teams
void multi_team_host_lock_all()
{
	int idx;

	// lock all players down
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx])){
			Net_players[idx].flags |= NETINFO_FLAG_TEAM_LOCKED;
		}
	}

	// verify that we have valid team stuff
	multi_team_verify();
}

// get the player counts for team 0 and team 1 (NULL values are valid)
void multi_team_get_player_counts(int *team0, int *team1)
{
	int idx;

	// initialize the values
	if(team0 != NULL){
		(*team0) = 0;
	}
	if(team1 != NULL){
		(*team1) = 0;
	}

	// check all players
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx])){
			if((Net_players[idx].p_info.team == 0) && (team0 != NULL)){
				(*team0)++;
			} else if((Net_players[idx].p_info.team == 1) && (team1 != NULL)){
				(*team1)++;
			}
		}
	}
}

// report on the winner/loser of the game via chat text
#define SEND_AND_DISPLAY(mesg)		do { send_game_chat_packet(Net_player, mesg, MULTI_MSG_ALL, NULL, NULL, 1); multi_display_chat_msg(mesg, 0, 0); } while(0);
void multi_team_report()
{
	char report[400] = "";	

	// a little header
	SEND_AND_DISPLAY("----****");	

	// display scores
	sprintf(report, XSTR("<Team 1 had %d points>", 1275), Multi_team0_score);
	SEND_AND_DISPLAY(report);
	sprintf(report, XSTR("<Team 2 had %d points>", 1276), Multi_team1_score);
	SEND_AND_DISPLAY(report);

	// display winner
	switch(multi_team_winner()){
	case -1:
		SEND_AND_DISPLAY(XSTR("<Match was a tie>", 1277));
		break;

	case 0:		
		SEND_AND_DISPLAY(XSTR("<Team 1 (green) is the winner>", 1278));
		break;

	case 1:				
		SEND_AND_DISPLAY(XSTR("<Team 2 (red) is the winner>", 1279));
		break;
	}
	// a little header
	SEND_AND_DISPLAY("----****");	
}


// ------------------------------------------------------------------------------------
// MULTIPLAYER TEAMPLAY PACKET HANDLERS
//

// process an incoming team update packet
void multi_team_process_packet(unsigned char *data, header *hinfo)
{
	ubyte code;	
	int player_index;
	int offset = HEADER_LENGTH;		

	// find out who is sending this data	
	player_index = find_player_id(hinfo->id);	

	// get the packet opcode
	GET_DATA(code);

	// take action absed upon the opcode	
	switch((int)code){
	// a request to set the team for a player
	case MT_CODE_TEAM_REQUEST:
		ushort player_id;
		int req_index,req_team;

		Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);

		// get the packet data
		GET_DATA(player_id);
		GET_DATA(req_team);

		// if i'm the host of the game, process here		
		req_index = find_player_id(player_id);
		if(req_index == -1){
			nprintf(("Network","Could not find player to process team change request !\n"));
		} else {
			multi_team_process_team_change_request(&Net_players[req_index],&Net_players[player_index],req_team);
		}		
		break;
	
	// a full team update
	case MT_CODE_TEAM_UPDATE:
		offset += multi_team_process_team_update(data+offset);
		break;	
	}	
	
	PACKET_SET_SIZE();

	// verify that we have valid team stuff
	multi_team_verify();
}

// send a packet to the host requesting to change my team 
void multi_team_send_team_request(net_player *pl, int team)
{
	ubyte data[MAX_PACKET_SIZE],code;
	int packet_size = 0;

	// build the header and add the opcode
	BUILD_HEADER(TEAM_UPDATE);
	code = MT_CODE_TEAM_REQUEST;
	ADD_DATA(code);

	// add the address of the guy we want to change
	ADD_DATA(pl->player_id);

	// add the team I want to be on
	ADD_DATA(team);

	// send to the server of the game (will be routed to host if in a standalone situation)	
	multi_io_send_reliable(Net_player, data, packet_size);
}

// send a full update on a player-per-player basis (should call this to update all players after other relevant function calls)
void multi_team_send_update()
{
	ubyte data[MAX_PACKET_SIZE],val,stop;
	int idx;
	int packet_size = 0;

	// if I'm not the server, bail
	if(!MULTIPLAYER_MASTER){
		return;
	}

	// first, verify that we have valid settings
	multi_team_verify();

	// build the header and add the opcode
	BUILD_HEADER(TEAM_UPDATE);
	val = MT_CODE_TEAM_UPDATE;
	ADD_DATA(val);

	// add the info for all players;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			// add a stop byte
			stop = 0x0;
			ADD_DATA(stop);

			// add this guy's id
			ADD_DATA(Net_players[idx].player_id);

			// pack all his data into a byte
			val = 0x0;
			
			// set bit 0 if he's on team 1
			if(Net_players[idx].p_info.team == 1){
				val |= (1<<0);
			}

			// set bit 1 if he's a team captain
			if(Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN){
				val |= (1<<1);
			}
			ADD_DATA(val);
		}
	}

	// add the final stop byte
	stop = 0xff;
	ADD_DATA(stop);

	// if i'm the server, I should broadcast to this to all players, otherwise I should send it to the standalone
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		multi_io_send_to_all_reliable(data, packet_size);
	} else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}	

// process a team update packet, return bytes processed
int multi_team_process_team_update(ubyte *data)
{
	ubyte stop,flags;
	short player_id;
	int player_index;
	int offset = 0;	

	// if I'm the server, bail
	Assert(!MULTIPLAYER_MASTER);
	
	// process all players
	GET_DATA(stop);
	while(stop != 0xff){
		// get the net address and flags for the guy
		GET_DATA(player_id);
		GET_DATA(flags);

		// do a player lookup
		if(!MULTIPLAYER_MASTER){
			player_index = find_player_id(player_id);
			if(player_index != -1){
				// set his team correctly
				if(flags & (1<<0)){
					Net_players[player_index].p_info.team = 1;
				} else {
					Net_players[player_index].p_info.team = 0;
				}

				// set his captaincy flag correctly
				Net_players[player_index].flags &= ~(NETINFO_FLAG_TEAM_CAPTAIN);
				if(flags & (1<<1)){
					Net_players[player_index].flags |= NETINFO_FLAG_TEAM_CAPTAIN;
				}
			}
		}

		// get the next stop byte
		GET_DATA(stop);
	}

	// verify that we have valid team stuff
	multi_team_verify();
	
	// return bytes processed
	return offset;
}

// verify that we have valid team stuff
void multi_team_verify()
{
#ifndef NDEBUG
	int team0_count,team0_cap;
	int team1_count,team1_cap;
	int idx;

	// determine how many players we have on team 0 and if they have a captain
	team0_count = 0;
	team0_cap = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			// if he's on team 0
			if(Net_players[idx].p_info.team == 0){
				team0_count++;

				// if he's a captain
				if(Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN){
					team0_cap++;
				}
			}
		}
	}
	// if the team has members
	if(team0_count > 0){
		// make sure it also has a captain
		Assert(team0_cap > 0);

		// make sure it only has 1 captain
		Assert(team0_cap == 1);
	}

	// determine how many players we have on team 1 and if they have a captain
	team1_count = 0;
	team1_cap = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			// if he's on team 1
			if(Net_players[idx].p_info.team == 1){
				team1_count++;

				// if he's a captain
				if(Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN){
					team1_cap++;
				}
			}
		}
	}
	// if the team has members
	if(team1_count > 0){
		// make sure it also has a captain
		Assert(team1_cap > 0);

		// make sure it only has 1 captain
		Assert(team1_cap == 1);
	}
#endif
}
