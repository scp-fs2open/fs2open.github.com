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
 * $Revision: 2.6 $
 * $Date: 2006-09-08 06:20:14 $
 * $Author: taylor $
 * 
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2006/03/26 08:23:06  taylor
 * split pause_*() and multi_pause_*() functions into individual single and multi versions (why it was hacked up like that I'll never know)
 * fix screen save in multi pause mode
 * address some bmpman issues from interface graphics getting released and then still used by something else
 *
 * Revision 2.4  2005/07/13 03:25:58  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.3  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2002/10/19 03:50:29  randomtiger
 * Added special pause mode for easier action screenshots.
 * Added new command line parameter for accessing all single missions in tech room. - RT
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
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
