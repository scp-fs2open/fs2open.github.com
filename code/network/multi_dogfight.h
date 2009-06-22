/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FS2_MULTIPLAYER_DOGFIGHT_HEADER_FILE
#define __FS2_MULTIPLAYER_DOGFIGHT_HEADER_FILE

// ----------------------------------------------------------------------------------------------------
// MULTI DOGFIGHT DEFINES/VARS
//

struct net_player;
struct object;


// ----------------------------------------------------------------------------------------------------
// MULTI DOGFIGHT FUNCTIONS
//

// call once per level just before entering the mission
void multi_df_level_pre_enter();

// evaluate a kill in dogfight by a netplayer
void multi_df_eval_kill(net_player *killer, object *dead_obj);

// debrief
void multi_df_debrief_init();

// do frame
void multi_df_debrief_do();

// close
void multi_df_debrief_close();

#endif
