/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionLoad.h $
 * $Revision: 2.4 $
 * $Date: 2005-07-13 03:25:59 $
 * $Author: Goober5000 $
 *
 * Mission load header file
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 20    2/23/98 6:55p Lawrance
 * Rip out obsolete code.
 * 
 * 19    6/26/97 5:53p Lawrance
 * save recently played missions, allow player to choose from list
 * 
 * 18    4/25/97 11:31a Allender
 * Campaign state now saved in campaign save file in player directory.
 * Made some global variables follow naming convention.  Solidified
 * continuing campaigns based on new structure
 *
 * $NoKeywords: $
 */

#ifndef _MISSIONLOAD_H
#define _MISSIONLOAD_H

#include "globalincs/pstypes.h"

// -----------------------------------------------
// For recording most recent missions played
// -----------------------------------------------
#define			MAX_RECENT_MISSIONS	10
extern	char	Recent_missions[MAX_RECENT_MISSIONS][MAX_FILENAME_LEN];
extern	int	Num_recent_missions;

// Mission_load takes no parameters.
// It expects the following variables to be set correctly:
// Game_current_mission_filename

int mission_load();
void mission_init();

// Functions for mission load menu
void mission_load_menu_init();
void mission_load_menu_close();
void mission_load_menu_do();

#endif
