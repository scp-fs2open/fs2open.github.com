/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MULTITEAMSELECT_H
#define _MULTITEAMSELECT_H

// ------------------------------------------------------------------------------------------------------
// TEAM SELECT DEFINES/VARS
//
#include "globalincs/pstypes.h"

struct header;

// should be initialize to 0 inside of multi_vars_init
extern int Multi_ts_inited;

#define MULTI_TS_MAX_TVT_TEAMS							2						// 2 teams max for now
#define MULTI_TS_NUM_SHIP_SLOTS							12						// # of ship slots in non team vs. team mode

// deleted ship objnums
extern int Multi_ts_deleted_objnums[MULTI_TS_MAX_TVT_TEAMS * MULTI_TS_NUM_SHIP_SLOTS];
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
void multi_ts_get_team_and_slot(char *ship_name,int *team_index,int *slot_index, bool mantis2757switch = false);

// function to return the shipname of the ship belonging in slot N
void multi_ts_get_shipname( char *ship_name, int team, int slot_index );

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
