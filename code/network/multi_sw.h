/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef __FREESPACE2_SQUAD_WAR_HEADER_FILE
#define __FREESPACE2_SQUAD_WAR_HEADER_FILE

#include "network/ptrack.h"

// ------------------------------------------------------------------------------------
// MULTIPLAYER SQUAD WAR DEFINES/VARS
//

// the min # of players required from each squad for the mission to be valid
#define MULTI_SW_MIN_PLAYERS					1

// set on the host in response to a standalone sw query, -1 == waiting, 0 == fail, 1 == success
extern int Multi_sw_std_query;

// match code
#define MATCH_CODE_LEN		34			// from ptrack.h
extern char Multi_sw_match_code[MATCH_CODE_LEN];

// reply from a standalone on a bad response
extern char Multi_sw_bad_reply[MAX_SQUAD_RESPONSE_LEN+1];

// ------------------------------------------------------------------------------------
// MULTIPLAYER SQUAD WAR FUNCTIONS
//

// call before loading level - mission sync phase. only the server need do this
void multi_sw_level_init();

// determine if everything is ok to move forward for a squad war match
int multi_sw_ok_to_commit();

// query PXO on the standalone
void multi_sw_std_query(char *match_code);

// call to update everything on the tracker
void multi_sw_report(int stats_saved);

#endif

