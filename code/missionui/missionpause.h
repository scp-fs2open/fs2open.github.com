/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MissionUI/MissionPause.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:59 $
 * $Author: penguin $
 * 
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     6/09/99 2:17p Dave
 * Fixed up pleasewait bitmap rendering.
 * 
 *
 * $NoKeywords: $
 */

#ifndef _MISSION_PAUSE_HEADER_FILE
#define _MISSION_PAUSE_HEADER_FILE

#include "2d.h"

// ----------------------------------------------------------------------------------------------------------------
// PAUSE DEFINES/VARS
//

// pause bitmap display stuff
extern int Please_wait_coords[GR_NUM_RESOLUTIONS][4];


// ----------------------------------------------------------------------------------------------------------------
// PAUSE FUNCTIONS
//

// initialize the pause screen
void pause_init(int multi);

// pause do frame - will handle running multiplayer operations if necessary
void pause_do(int multi);

// close the pause screen
void pause_close(int multi);

// debug pause init
void pause_debug_init();

// debug pause do frame
void pause_debug_do();

// debug pause close
void pause_debug_close();

#endif
