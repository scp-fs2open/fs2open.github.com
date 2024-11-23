/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "playerman/player.h"



#ifndef __FS2_MULTIPLAYER_DOGFIGHT_HEADER_FILE
#define __FS2_MULTIPLAYER_DOGFIGHT_HEADER_FILE

// ----------------------------------------------------------------------------------------------------
// MULTI DOGFIGHT DEFINES/VARS
//

struct net_player;
class object;

// players when the screen started - we need to store this explicity so that even after players leave, we can display
// the full kill matrix
typedef struct multi_df_score {
	char callsign[CALLSIGN_LEN + 1]; // callsign for this guy
	scoring_struct stats;            // stats for the guy
	int np_index;                    // absolute index into the netplayers array
} multi_df_score;

extern multi_df_score Multi_df_score[MAX_PLAYERS];
extern int Multi_df_score_count;

// ----------------------------------------------------------------------------------------------------
// MULTI DOGFIGHT FUNCTIONS
//

// call once per level just before entering the mission
void multi_df_level_pre_enter();

// evaluate a kill in dogfight by a netplayer
void multi_df_eval_kill(const net_player *killer, const object *dead_obj);

// debrief
void multi_df_debrief_init(bool API_Access = false);

// do frame
void multi_df_debrief_do(bool API_Access = false);

// close
void multi_df_debrief_close(bool API_Access = false);

#endif
