/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/MuzzleFlash.h $
 * $Revision: 2.2 $
 * $Date: 2004-08-11 05:06:36 $
 * $Author: Kazan $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     5/18/99 1:30p Dave
 * Added muzzle flash table stuff.
 * 
 * 3     3/19/99 9:52a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 2     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef __FS2_MUZZLEFLASH_HEADER_FILE
#define __FS2_MUZZLEFLASH_HEADER_FILE

// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH DEFINES/VARS
// 

// prototypes
struct object;
struct vector;

// muzzle flash types
#define MAX_MUZZLE_FLASH_TYPES				10
extern int Num_mflash_types;

// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH FUNCTIONS
// 

// initialize muzzle flash stuff for the whole game
void mflash_game_init();

// initialize muzzle flash stuff for the level
void mflash_level_init();

// shutdown stuff for the level
void mflash_level_close();

// create a muzzle flash on the guy
void mflash_create(vector *gun_pos, vector *gun_dir, int mflash_type);

// process muzzle flash stuff
void mflash_process_all();

// render all muzzle flashes
void mflash_render_all();

// lookup type by name
int mflash_lookup(char *name);

#endif
