/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HudArtillery.h $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:05 $
 * $Author: penguin $
 * 
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 2     4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 1     4/20/99 12:00a Dave
 * 
 * 
 * $NoKeywords: $
 */

#ifndef _FS2_HUD_ARTILLERY_HEADER_FILE
#define _FS2_HUD_ARTILLERY_HEADER_FILE

#include "globalincs/pstypes.h"

// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY DEFINES/VARS
//


// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY FUNCTIONS
//

// level init
void hud_init_artillery();

// update all hud artillery related stuff
void hud_artillery_update();

// render all hud artillery related stuff
void hud_artillery_render();


#define MAX_SSM_STRIKES			10
#define MAX_SSM_COUNT			10

// creation info for the strike (useful for multiplayer)
typedef struct ssm_firing_info {
	int			delay_stamp[MAX_SSM_COUNT];	// timestamps
	vector		start_pos[MAX_SSM_COUNT];		// start positions
	
	int			ssm_index;							// index info ssm_info array
	vector		target;								// target for the strike	
} ssm_firing_info;

// start a subspace missile effect
void ssm_create(vector *target, vector *start, int ssm_index, ssm_firing_info *override);

#endif
