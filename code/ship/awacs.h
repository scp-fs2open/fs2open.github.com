/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/AWACS.h $
 * $Revision: 2.2 $
 * $Date: 2004-03-05 09:01:52 $
 * $Author: Goober5000 $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2003/01/03 21:58:07  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     6/02/99 12:52p Andsager
 * Added team-wide ship visibility.  Implemented for player.
 * 
 * 3     1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 2     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 
 * $NoKeywords: $
 */

#ifndef __FS2_AWACS_HEADER_FILE
#define __FS2_AWACS_HEADER_FILE

// ----------------------------------------------------------------------------------------------------
// AWACS DEFINES/VARS
//
#include "globalincs/globals.h"

struct object;
struct ship;

// DAVE'S OFFICIAL DEFINITION OF AWACS

// total awacs levels for all teams
extern float Awacs_team[MAX_TEAMS];	// total AWACS capabilities for each team
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
int ship_is_visible_by_team(int ship_num, int team);

// Determine if ship is visible by team - new; incorporates friendly stealth invisible
// and primitive sensors
// return 1 if ship is fully visible
// return 0 if ship is only partly visible
extern int ship_is_visible_by_team_new(object *target, ship *viewer);

#endif
