/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_team.h $
 * $Revision: 2.3 $
 * $Date: 2006-01-13 03:31:09 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2005/07/13 03:35:33  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.1  2004/08/11 05:06:29  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 5     9/04/99 1:54p Dave
 * externed team scores.
 * 
 * 4     4/09/99 2:21p Dave
 * Multiplayer beta stuff. CD checking.
 * 
 * 3     2/17/99 2:11p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 8     5/22/98 9:35p Dave
 * Put in channel based support for PXO. Put in "shutdown" button for
 * standalone. UI tweaks for TvT
 * 
 * 7     5/15/98 5:16p Dave
 * Fix a standalone resetting bug.Tweaked PXO interface. Display captaincy
 * status for team vs. team. Put in asserts to check for invalid team vs.
 * team situations.
 * 
 * 6     4/08/98 2:51p Dave
 * Fixed pilot image xfer once again. Solidify team selection process in
 * pre-briefing multiplayer.
 * 
 * 5     3/31/98 4:51p Dave
 * Removed medals screen and multiplayer buttons from demo version. Put in
 * new pilot popup screen. Make ships in mp team vs. team have proper team
 * ids. Make mp respawns a permanent option saved in the player file.
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

#ifndef _MULTI_TEAMPLAY_HEADER_FILE
#define _MULTI_TEAMPLAY_HEADER_FILE

#include <globalincs/globals.h>

// ------------------------------------------------------------------------------------
// MULTIPLAYER TEAMPLAY DEFINES/VARS
//

// prototypes
struct header;
struct net_player;
struct ship;

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
