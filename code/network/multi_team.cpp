/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
#include "iff_defs/iff_defs.h"
#include "stats/scoring.h"

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

// score for teams for this mission
int Multi_team_score[MAX_TVT_TEAMS];

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
	for (int i = 0; i < MAX_TVT_TEAMS; i++)
		Multi_team_score[i] = 0;
}

// call to determine who won the sw match, -1 == tie, 0 == team 0, 1 == team 1
int multi_team_winner()
{
	int i, num_equal = 0, winner=-1, highest = -1;

	// determine highest score
	for (i = 0; i < Num_teams; i++)
	{
		if (Multi_team_score[i] > highest)
		{
			highest = Multi_team_score[i];
			winner = i;
		}
	}

	// see if there are any ties
	for (i = 0; i < Num_teams; i++)
	{
		if (Multi_team_score[i] == highest)
			num_equal++;
	}

	// tie situation?
	if (num_equal > 1)
		return -1;

	return winner;
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
	Multi_team_score[team] += (int)(points * scoring_get_scale_factor());
	nprintf(("Network", "TVT : adding %d points to team %d (total == %d)\n", points, team, Multi_team_score[team]));
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
		nprintf(("Network","MULTI TEAM : Server setting player %s team %d captain\n",pl->m_player->callsign,pl->p_info.team));
		pl->flags |= NETINFO_FLAG_TEAM_CAPTAIN;
	} else {
		nprintf(("Network","MULTI TEAM : Server unsetting player %s as team %d captain\n",pl->m_player->callsign,pl->p_info.team));
		pl->flags &= ~(NETINFO_FLAG_TEAM_CAPTAIN);
	}	
}

// set the team of this given player (if called by the host, the player becomes locked, cnad only the host can modify him from thereon)
void multi_team_set_team(net_player *pl,int team)
{	
	// if i'm the server of the game, do it now
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		nprintf(("Network","MULTI TEAM : Server/Host setting player %s to team %d and locking\n",pl->m_player->callsign,team));

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
	nprintf(("Network","MULTI TEAM : Server changing player %s to team %d from client request\n",pl->m_player->callsign,team));
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
	int i;
	
	// look through TVT wings... each wing corresponds to a team
	for (i = 0; i < MAX_TVT_WINGS; i++)
	{
		if (sp->wingnum == TVT_wings[i])
			sp->team = i;
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
	int i;
	char report[400] = "";	

	// a little header
	SEND_AND_DISPLAY("----****");	

	// display scores
	for (i = 0; i < Num_teams && i < MAX_TVT_TEAMS; i++)
	{
		// Retail FS2 teams (i.e red or green)
		if (i < 2)
		{
			sprintf(report, XSTR("<Team %d had %d points>", (1275+i)), Multi_team_score[i]);
		}
		// Karajorma - If the SCP has added more teams we won't have a XSTR to handle it. So just output in english for now
		else 
		{
			sprintf(report, "Team %d had %d points>", (i+1), Multi_team_score[i]);
		}

		SEND_AND_DISPLAY(report);
	}

	// display winner
	switch(multi_team_winner())
	{
		case -1:
			SEND_AND_DISPLAY(XSTR("<Match was a tie>", 1277));
			break;

		case 0:		
			SEND_AND_DISPLAY(XSTR("<Team 1 (green) is the winner>", 1278));
			break;

		case 1:				
			SEND_AND_DISPLAY(XSTR("<Team 2 (red) is the winner>", 1279));
			break;

		default:
			// need to come up with a new message here
			Int3();
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
		GET_USHORT(player_id);
		GET_INT(req_team);

		// if i'm the host of the game, process here		
		req_index = find_player_id(player_id);
		if( (req_index == -1) || (player_index == -1) ){
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
	ADD_SHORT(pl->player_id);

	// add the team I want to be on
	ADD_INT(team);

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
			ADD_SHORT(Net_players[idx].player_id);

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
		GET_SHORT(player_id);
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
