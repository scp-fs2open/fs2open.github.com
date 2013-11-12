/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FS2_AWACS_HEADER_FILE
#define __FS2_AWACS_HEADER_FILE

// ----------------------------------------------------------------------------------------------------
// AWACS DEFINES/VARS
//
#include "globalincs/globals.h"

class object;
class ship;

// DAVE'S OFFICIAL DEFINITION OF AWACS

// total awacs levels for all teams
extern float Awacs_team[MAX_IFFS];	// total AWACS capabilities for each team
extern float Awacs_level;				// Awacs_friendly - Awacs_hostile

// ----------------------------------------------------------------------------------------------------
// AWACS FUNCTIONS
//

// call when initializing level, before parsing mission
void awacs_level_init();

// call every frame to process AWACS details
void awacs_process();

// get the total AWACS level for target to viewer
// < 0.0f		: untargetable
// 0.0 - 1.0f	: marginally targetable
// 1.0f			: fully targetable as normal
float awacs_get_level(object *target, ship *viewer, int use_awacs=1);

// Determine if ship is visible by team
// return 1 if ship is fully visible
// return 0 if ship is only partly visible
int ship_is_visible_by_team(object *target, ship *viewer);

#endif
