/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Freespace2/LevelPaging.cpp $
 * $Revision: 2.4 $
 * $Date: 2004-07-01 01:53:00 $
 * $Author: phreak $
 *
 * Code to page in all the bitmaps at the beginning of a level.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/06/18 04:59:53  wmcoolmon
 * Only used weapons paged in instead of all, fixed music box in FRED, sound quality settable with SoundSampleRate and SoundSampleBits registry values
 *
 * Revision 2.2  2004/03/05 09:02:01  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     8/19/99 10:12a Alanl
 * preload mission-specific messages on machines greater than 48MB
 * 
 * 3     8/10/99 6:54p Dave
 * Mad optimizations. Added paging to the nebula effect.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 6     5/23/98 4:14p John
 * Added code to preload textures to video card for AGP.   Added in code
 * to page in some bitmaps that weren't getting paged in at level start.
 * 
 * 5     4/05/98 4:15p Dave
 * Fixed a weapons model paging problem with the standalone server.
 * 
 * 4     4/01/98 5:34p John
 * Made only the used POFs page in for a level.   Reduced some interp
 * arrays.    Made custom detail level work differently.
 * 
 * 3     3/29/98 4:05p John
 * New paging code that loads everything necessary at level startup.
 * 
 * 2     3/26/98 5:26p John
 * added new paging code. nonfunctional.
 * 
 * 1     3/26/98 5:14p John
 *
 * $NoKeywords: $
 */

#include "freespace2/freespace.h"
#include "freespace2/levelpaging.h"

// All the page in functions
extern void ship_page_in();
extern void debris_page_in();
extern void particle_page_in();
extern void stars_page_in();
extern void hud_page_in();
extern void (*radar_page_in)();
extern void weapons_page_in();
extern void fireballs_page_in();
extern void shockwave_page_in();
extern void shield_hit_page_in();
extern void asteroid_page_in();
extern void training_mission_page_in();
extern void neb2_page_in();
extern void message_pagein_mission_messages();

// Pages in all the texutures for the currently
// loaded mission.  Call game_busy() occasionally...
void level_page_in()
{

	mprintf(( "Beginning level bitmap paging...\n" ));

	if(!(Game_mode & GM_STANDALONE_SERVER)){		
		bm_page_in_start();
	}

	// Most important ones first
	ship_page_in();
	//Must be called after paging in ships
	weapons_page_in();
	fireballs_page_in();
	particle_page_in();
	debris_page_in();
	hud_page_in();
	radar_page_in();
	training_mission_page_in();
	stars_page_in();
	shockwave_page_in();
	shield_hit_page_in();
	asteroid_page_in();
	neb2_page_in();

	// preload mission messages if NOT running low-memory (greater than 48MB)
	if (game_using_low_mem() == false) {
		message_pagein_mission_messages();
	}

	if(!(Game_mode & GM_STANDALONE_SERVER)){
		bm_page_in_stop();
	}

	mprintf(( "Ending level bitmap paging...\n" ));

}
