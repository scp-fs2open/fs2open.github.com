/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/MultiTeamSelect.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:26 $
 * $Author: penguin $
 *
 * Multiplayer Team Selection Code header
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 22    5/19/98 8:35p Dave
 * Revamp PXO channel listing system. Send campaign goals/events to
 * clients for evaluation. Made lock button pressable on all screens. 
 * 
 * 21    5/18/98 12:41a Allender
 * fixed subsystem problems on clients (i.e. not reporting properly on
 * damage indicator).  Fixed ingame join problem with respawns.  minor
 * comm menu stuff
 * 
 * 20    5/10/98 7:06p Dave
 * Fix endgame sequencing ESC key. Changed how host options warning popups
 * are done. Fixed pause/message scrollback/options screen problems in mp.
 * Make sure observer HUD doesn't try to lock weapons.
 * 
 * 19    5/06/98 8:06p Dave
 * Made standalone reset properly under weird conditions. Tweak
 * optionsmulti screen. Upped MAX_WEAPONS to 350. Put in new launch
 * countdown anim. Minro ui fixes/tweaks.
 * 
 * 18    4/22/98 7:24p Dave
 * Made sure the "player/ships" locked button for multiplayer appears on
 * all briefing screens.
 * 
 * 17    4/20/98 4:56p Allender
 * allow AI ships to respawn as many times as there are respawns in the
 * mission.  
 * 
 * 16    4/01/98 11:19p Dave
 * Put in auto-loading of xferred pilot pic files. Grey out background
 * behind pinfo popup. Put a chatbox message in when players are kicked.
 * Moved mission title down in briefing. Other ui fixes.
 * 
 * 15    3/26/98 6:01p Dave
 * Put in file checksumming routine in cfile. Made pilot pic xferring more
 * robust. Cut header size of voice data packets in half. Put in
 * restricted game host query system.
 * 
 * 14    3/10/98 10:56a Dave
 * Put support for deleting ships from starting wings back in.
 * 
 * 13    3/09/98 5:55p Dave
 * Fixed stats to take asteroid hits into account. Polished up UI stuff in
 * team select. Finished up pilot info popup. Tracked down and fixed
 * double click bug.
 * 
 * 12    3/05/98 5:03p Dave
 * More work on team vs. team support for multiplayer. Need to fix bugs in
 * weapon select.
 * 
 * 11    3/01/98 3:26p Dave
 * Fixed a few team select bugs. Put in multiplayer intertface sounds.
 * Corrected how ships are disabled/enabled in team select/weapon select
 * screens.
 * 
 * 10    2/23/98 11:09p Dave
 * Finished up multiplayer campaign support. Seems bug-free.
 * 
 * 9     2/19/98 6:26p Dave
 * Fixed a few file xfer bugs. Tweaked mp team select screen. Put in
 * initial support for player data uploading.
 * 
 * 8     2/18/98 3:56p Dave
 * Several bugs fixed for mp team select screen. Put in standalone packet
 * routing for team select.
 * 
 * 7     2/17/98 6:08p Dave
 * Tore out old multiplayer team select screen, installed new one.
 * 
 * 6     10/16/97 4:58p Dave
 * Finsihed up server side respawning issues. Knocked off a bunch of
 * sequencing issues.
 * 
 * 5     10/10/97 4:42p Dave
 * Fixed up server transfer bugs.
 * 
 * 4     10/09/97 4:58p Lawrance
 * get short_callsign from player struct
 * 
 * 3     10/03/97 4:58p Dave
 * Put in client-side mimicing of host team selection changes. Some more
 * stanalone hooks. 
 * 
 * 2     9/19/97 4:24p Allender
 * start of team selection screen.
 * 
 * 1     9/18/97 11:45a Allender
 * 
 */

#ifndef _MULTITEAMSELECT_H
#define _MULTITEAMSELECT_H

// ------------------------------------------------------------------------------------------------------
// TEAM SELECT DEFINES/VARS
//

struct header;

// should be initialize to 0 inside of multi_vars_init
extern int Multi_ts_inited;

#define MULTI_TS_MAX_TEAMS									2						// 2 teams max for now
#define MULTI_TS_NUM_SHIP_SLOTS							12						// # of ship slots in non team vs. team mode

// deleted ship objnums
extern int Multi_ts_deleted_objnums[MULTI_TS_MAX_TEAMS * MULTI_TS_NUM_SHIP_SLOTS];
extern int Multi_ts_num_deleted;

// ------------------------------------------------------------------------------------------------------
// TEAM SELECT FUNCTIONS
//

// initialize the team select screen (always call, even when switching between weapon select, etc)
void multi_ts_init();

// initialize all critical internal data structures
void multi_ts_common_init();

// do frame for team select
void multi_ts_do();

// close the team select screen (always call, even when switching between weapon select, etc)
void multi_ts_close();

// drop a carried icon 
void multi_ts_drop(int from_type,int from_index,int to_type,int to_index,int ship_class,int player_index = -1);

// assign all players to appropriate default wings/slots
void multi_ts_assign_players_all();

// is the given slot disabled for the specified player
int multi_ts_disabled_slot(int slot_index,int player_index = -1);

// is the given slot disabled for the specified player, _and_ it is his ship as well
int multi_ts_disabled_high_slot(int slot_index,int player_index = -1);

// delete ships which have been removed from the game, tidy things 
void multi_ts_create_wings();

// resynch all display/interface elements based upon all the ship/weapon pool values
void multi_ts_sync_interface();

// do any necessary processing for players who have left the game
void multi_ts_handle_player_drop();

// handle all details when the commit button is pressed (including possibly reporting errors/popups)
void multi_ts_commit_pressed();

// get the team # of the given ship
int multi_ts_get_team(char *ship_name);

// function to get the team and slot of a particular ship
void multi_ts_get_team_and_slot(char *ship_name,int *team_index,int *slot_index);

// function to return the shipname of the ship belonging in slot N
char *multi_ts_get_shipname( int team, int slot_index );

// blit the proper "locked" button - used for weapon select and briefing screens
void multi_ts_blit_locked_button();

// the "lock" button has been pressed
void multi_ts_lock_pressed();

// if i'm "locked"
int multi_ts_is_locked();

// show a popup saying "only host and team captains can modify, etc, etc"
void multi_ts_maybe_host_only_popup();

// ------------------------------------------------------------------------------------------------------
// TEAM SELECT PACKET HANDLERS
//

// send a player slot position update
void send_pslot_update_packet(int team,int code,int sound = -1);

// process a player slot position update
void process_pslot_update_packet(ubyte *data, header *hinfo);



// ------------------------------------------------------------------------------------------------------
// TEAM SELECT STUBBED FUNCTIONS
//

#endif
