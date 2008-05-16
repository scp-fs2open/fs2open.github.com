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
 * $Revision: 2.6.2.1 $
 * $Date: 2006-09-11 01:15:04 $
 * $Author: taylor $
 * 
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.6  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.5  2005/04/05 05:53:17  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.4  2004/09/17 07:11:02  Goober5000
 * moved ssm stuff to header file so it would work in FRED
 * --Goober5000
 *
 * Revision 2.3  2004/08/23 04:00:15  Goober5000
 * ship-tag and ship-untag
 * --Goober5000
 *
 * Revision 2.2  2004/08/11 05:06:25  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
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
#include "globalincs/globals.h"

// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY DEFINES/VARS
//
#define MAX_SSM_TYPES			10
#define MAX_SSM_STRIKES			10
#define MAX_SSM_COUNT			10

// global ssm types
typedef struct ssm_info {
	char			name[NAME_LENGTH];				// strike name
	int			count;								// # of missiles in this type of strike
	int			weapon_info_index;				// missile type
	float			warp_radius;						// radius of associated warp effect	
	float			warp_time;							// how long the warp effect lasts
	float			radius;								// radius around the shooting ship	
	float			offset;								// offset in front of the shooting ship
} ssm_info;

// creation info for the strike (useful for multiplayer)
typedef struct ssm_firing_info {
	int     delay_stamp[MAX_SSM_COUNT];	    // timestamps
	vec3d   start_pos[MAX_SSM_COUNT];       // start positions
	
	int             ssm_index;							// index info ssm_info array
	struct object*  target;								// target for the strike
    int             ssm_team;                           // team that fired the ssm.
} ssm_firing_info;

// the strike itself
typedef struct ssm_strike {
	int			fireballs[MAX_SSM_COUNT];		// warpin effect fireballs
	int			done_flags[MAX_SSM_COUNT];		// when we've fired off the individual missiles
	
	// this is the info that controls how the strike behaves (just like for beam weapons)
	ssm_firing_info		sinfo;

	ssm_strike	*next, *prev;						// for list
} ssm_strike;


extern int Ssm_info_count;
extern ssm_info Ssm_info[MAX_SSM_TYPES];


// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY FUNCTIONS
//

// level init
void hud_init_artillery();

// update all hud artillery related stuff
void hud_artillery_update();

// render all hud artillery related stuff
void hud_artillery_render();

// start a subspace missile effect
void ssm_create(object *target, vec3d *start, int ssm_index, ssm_firing_info *override, int team);

// Goober5000
extern int ssm_info_lookup(char *name);

#endif
