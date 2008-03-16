/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Flak.h $
 * $Revision: 2.6 $
 * $Date: 2005-12-29 08:08:42 $
 * $Author: wmcoolmon $
 *
 * flak functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2005/10/30 06:44:59  wmcoolmon
 * Codebase commit - nebula.tbl, scripting, new dinky explosion/shockwave stuff, moving muzzle flashes
 *
 * Revision 2.4  2005/07/13 03:35:30  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.3  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.2  2004/08/11 05:06:36  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
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
 * 6     7/31/99 2:57p Dave
 * Scaled flak aim and jitter by weapon subsystem strength.
 * 
 * 5     5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * $NoKeywords: $
 */

#ifndef _FLAK_WEAPONS_HEADER_FILE
#define _FLAK_WEAPONS_HEADER_FILE

#include "physics/physics.h"

// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK DEFINES/VARS
//

struct weapon;
struct object;
struct vec3d;

// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK FUNCTIONS
//

// initialize flak stuff for the level
void flak_level_init();

// close down flak stuff
void flak_level_close();

// given a newly created weapon, turn it into a flak weapon
//void flak_create(weapon *wp);

// free up a flak object
//void flak_delete(int flak_index);

// given a just fired flak shell, pick a detonating distance for it
void flak_pick_range(object *objp, vec3d *firing_pos, vec3d *predicted_target_pos, float weapon_subsys_strength);

// add some jitter to a flak gun's aiming direction, take into account range to target so that we're never _too_ far off
// assumes dir is normalized
void flak_jitter_aim(vec3d *dir, float dist_to_target, float weapon_subsys_strength);

// create a muzzle flash from a flak gun based upon firing position and weapon type
void flak_muzzle_flash(vec3d *pos, vec3d *dir, physics_info *pip, int turret_weapon_class);

// maybe detonate a flak shell early/late (call from weapon_process_pre(...))
//void flak_maybe_detonate(object *obj);

// set the range on a flak object
void flak_set_range(object *objp, float range);

// get the current range for the flak object
float flak_get_range(object *objp);


#endif
