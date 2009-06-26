/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MISSION_PAUSE_HEADER_FILE
#define _MISSION_PAUSE_HEADER_FILE

#include "graphics/2d.h"

// ----------------------------------------------------------------------------------------------------------------
// PAUSE DEFINES/VARS
//

// pause bitmap display stuff
extern int Please_wait_coords[GR_NUM_RESOLUTIONS][4];


// ----------------------------------------------------------------------------------------------------------------
// PAUSE FUNCTIONS
//

// initialize the pause screen
void pause_init();

// pause do frame - will handle running multiplayer operations if necessary
void pause_do();

// close the pause screen
void pause_close();

// debug pause init
void pause_debug_init();

// debug pause do frame
void pause_debug_do();

// debug pause close
void pause_debug_close();

enum
{
	PAUSE_TYPE_NORMAL,
	PAUSE_TYPE_VIEWER
};

void pause_set_type(int type);
int pause_get_type();

#endif
