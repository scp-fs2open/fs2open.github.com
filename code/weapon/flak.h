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
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:03 $
 * $Author: penguin $
 *
 * flak functions
 *
 * $Log: not supported by cvs2svn $
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

// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK DEFINES/VARS
//

struct weapon;
struct object;


// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK FUNCTIONS
//

// initialize flak stuff for the level
void flak_level_init();

// close down flak stuff
void flak_level_close();

// given a newly created weapon, turn it into a flak weapon
void flak_create(weapon *wp);

// free up a flak object
void flak_delete(int flak_index);

// given a just fired flak shell, pick a detonating distance for it
void flak_pick_range(object *objp, vector *predicted_target_pos, float weapon_subsys_strength);

// add some jitter to a flak gun's aiming direction, take into account range to target so that we're never _too_ far off
// assumes dir is normalized
void flak_jitter_aim(vector *dir, float dist_to_target, float weapon_subsys_strength);

// create a muzzle flash from a flak gun based upon firing position and weapon type
void flak_muzzle_flash(vector *pos, vector *dir, int turret_weapon_class);

// maybe detonate a flak shell early/late (call from weapon_process_pre(...))
void flak_maybe_detonate(object *obj);

// set the range on a flak object
void flak_set_range(object *objp, vector *start_pos, float range);

// get the current range for the flak object
float flak_get_range(object *objp);

#endif
