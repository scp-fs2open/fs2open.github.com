/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MULTI_TEAMPLAY_HEADER_FILE
#define _MULTI_TEAMPLAY_HEADER_FILE

#include <globalincs/globals.h>

// ------------------------------------------------------------------------------------
// MULTIPLAYER TEAMPLAY DEFINES/VARS
//

// prototypes
struct header;
struct net_player;
class ship;

// score for teams for this mission
extern int Multi_team_score[MAX_TVT_TEAMS];

// ------------------------------------------------------------------------------------
// MULTIPLAYER TEAMPLAY FUNCTIONS
//

// call before level load (pre-sync)
void multi_team_level_init();

// call to determine who won the sw match, -1 == tie, 0 == team 0, 1 == team 1
int multi_team_winner();

// call to add score to a team
void multi_team_maybe_add_score(int points, int team);

// reset all players and assign them to default teams
void multi_team_reset();

// set the captaincy status of this player
void multi_team_set_captain(net_player *pl,int set);

// set the team of this given player (if called by the host, the player becomes locked, cnad only the host can modify him from thereon)
void multi_team_set_team(net_player *pl,int team);

// is it ok for the host to hit commit
int multi_team_ok_to_commit();

// handle a player drop
void multi_team_handle_drop();

// handle a player join
void multi_team_handle_join(net_player *pl);

// send a full update on a player-per-player basis (should call this to update all players after other relevant function calls)
void multi_team_send_update();

// set all ships in the mission to be marked as the proper team (TEAM_HOSTILE, TEAM_FRIENLY)
void multi_team_mark_all_ships();

// set the proper team for the passed in ship
void multi_team_mark_ship(ship *sp);

// host locks all players into their teams
void multi_team_host_lock_all();

// verify that we have valid team stuff
void multi_team_verify();

// get the player counts for team 0 and team 1 (NULL values are valid)
void multi_team_get_player_counts(int *team0,int *team1);

// report on the winner/loser of the game via chat text
void multi_team_report();

// ------------------------------------------------------------------------------------
// MULTIPLAYER TEAMPLAY PACKET HANDLERS
//

// process an incoming team update packet
void multi_team_process_packet(unsigned char *data,header *hinfo);

#endif
