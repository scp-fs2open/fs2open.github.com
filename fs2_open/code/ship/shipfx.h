/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/ShipFX.h $
 * $Revision: 2.1 $
 * $Date: 2003-07-04 02:30:54 $
 * $Author: phreak $
 *
 * Routines for ship effects (as in special)
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 10    7/02/99 9:55p Dave
 * Player engine wash sound.
 * 
 * 9     5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 8     5/18/99 1:30p Dave
 * Added muzzle flash table stuff.
 * 
 * 7     5/09/99 6:00p Dave
 * Lots of cool new effects. E3 build tweaks.
 * 
 * 6     3/23/99 2:29p Andsager
 * Fix shockwaves for kamikazi and Fred defined.  Collect together
 * shockwave_create_info struct.
 * 
 * 5     2/26/99 4:14p Dave
 * Put in the ability to have multiple shockwaves for ships.
 * 
 * 4     1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 3     10/20/98 1:39p Andsager
 * Make so sparks follow animated ship submodels.  Modify
 * ship_weapon_do_hit_stuff() and ship_apply_local_damage() to add
 * submodel_num.  Add submodel_num to multiplayer hit packet.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 19    5/22/98 5:32p Andsager
 * Make big ship explosion sounds play all the way through.  remove
 * cur_snd from ship struct.
 * 
 * 18    5/19/98 8:43p Andsager
 * Modify sound management (of big ship explosions).  Turn on big ship
 * explosions for release build.
 * 
 * 17    5/12/98 10:54p Andsager
 * Add new sound manager for big ship sub-explosion sounds
 * 
 * 16    4/14/98 11:11p John
 * Made ships with < 50% hull left show electrical damage arcs.
 * 
 * 15    4/14/98 5:43p John
 * Made large ship blowup system reinit between levels.
 * 
 * 14    4/14/98 4:56p John
 * Hooked in Andsager's large ship exploding code, but it is temporarily
 * disabled.
 * 
 * 13    4/10/98 12:16p Allender
 * fix ship hit kill and debris packets
 * 
 * 12    4/05/98 2:37p John
 * Made sun on by default.  Fixed some other sun tweaks.
 * 
 * 11    1/02/98 5:04p John
 * Several explosion related changes.  Made fireballs not be used as
 * ani's.  Made ship spark system expell particles.  Took away impact
 * explosion for weapon hitting ship... this needs to get added to weapon
 * info and makes shield hit more obvious.  Only make sparks when hit
 * hull, not shields.
 * 
 * 10    12/17/97 7:53p John
 * Fixed a bug where gunpoint for flashes were in world coordinates,
 * should have been object.
 * 
 * 9     12/17/97 5:11p John
 * Added brightening back into fade table.  Added code for doing the fast
 * dynamic gun flashes and thruster flashes.
 * 
 * 8     12/12/97 3:02p John
 * First Rev of Ship Shadows
 * 
 * 7     9/12/97 4:06p John
 * put in ship warp out effect.
 * put in dynamic lighting for warp in/out
 * 
 * 6     9/09/97 4:52p John
 * Almost done ship warp in code
 * 
 * 5     2/28/97 11:07a John
 * more fx
 * 
 * 4     2/28/97 10:57a John
 * Made so you can blow off any subsystems, not just radars.
 * 
 * 
 * 3     2/10/97 12:38p John
 * made all ships blow up into debris pieces when exploded.
 * 
 * 2     2/07/97 11:49a John
 * Some not-final explosions for turrets.
 * 
 * 1     2/07/97 10:53a John
 *
 * $NoKeywords: $
 */


#ifndef _SHIPFX_H
#define _SHIPFX_H

struct object;
struct ship;
struct ship_subsys;

// Make sparks fly off of ship n
// sn = spark number to spark, corrosponding to element in
//      ship->hitpos array.  If this isn't -1, it is a just
//      got hit by weapon spark, otherwise pick one randomally.
void shipfx_emit_spark( int n, int sn );

// Does the special effects to blow a subsystem off a ship
extern void shipfx_blow_off_subsystem(object *ship_obj,ship *ship_p,ship_subsys *subsys, vector *exp_center);


// Creates "ndebris" pieces of debris on random verts of the the "submodel" in the 
// ship's model.
extern void shipfx_blow_up_model(object *obj,int model, int submodel, int ndebris, vector *exp_center);

// put here for multiplayer purposes
void shipfx_blow_up_hull(object *obj,int model, vector *exp_center );


// =================================================
//          SHIP WARP IN EFFECT STUFF
// =================================================

// When a ship warps in, this gets called to start the effect
extern void shipfx_warpin_start( object *objp );

// During a ship warp in, this gets called each frame to move the ship
extern void shipfx_warpin_frame( object *objp, float frametime );

// When a ship warps out, this gets called to start the effect
extern void shipfx_warpout_start( object *objp );

// During a ship warp out, this gets called each frame to move the ship
extern void shipfx_warpout_frame( object *objp, float frametime );

// =================================================
//          SHIP SHADOW EFFECT STUFF
// =================================================

// Given point p0, in object's frame of reference, find if 
// it can see the sun.
int shipfx_point_in_shadow( vector *p0, matrix *src_orient, vector *src_pos, float radius );

// Given an ship see if it is in a shadow.
int shipfx_in_shadow( object * src_obj );

// Given world point see if it is in a shadow.
int shipfx_eye_in_shadow( vector *eye_pos, object *src_obj, int sun_n);


// =================================================
//          SHIP GUN FLASH EFFECT STUFF
// =================================================

// Resets the ship flash stuff. Call before
// each level.
void shipfx_flash_init();

// Given that a ship fired a weapon, light up the model
// accordingly.
// Set is_primary to non-zero if this is a primary weapon.
// Gun_pos should be in object's frame of reference, not world!!!
void shipfx_flash_create(object *objp, ship * shipp, vector *gun_pos, vector *gun_dir, int is_primary, int weapon_info_index);

// Sets the flash lights in the model used by this
// ship to the appropriate values.  There might not
// be any flashes linked to this ship in which
// case this function does nothing.
void shipfx_flash_light_model(object *objp, ship * shipp );

// Does whatever processing needs to be done each frame.
void shipfx_flash_do_frame(float frametime);


// =================================================
//          LARGE SHIP EXPLOSION EFFECT STUFF
// =================================================

// Call between levels
void shipfx_large_blowup_level_init();

// Returns 0 if couldn't init
int shipfx_large_blowup_init(ship *shipp);

// Returns 1 when explosion is done
int shipfx_large_blowup_do_frame(ship *shipp, float frametime);

void shipfx_large_blowup_render(ship *shipp);

// sound manager fore big ship sub explosions sounds
void do_sub_expl_sound(float radius, vector* sound_pos, int* sound_handle);

// do all shockwaves for a ship blowing up
void shipfx_do_shockwave_stuff(ship *shipp, shockwave_create_info *sci);


// =================================================
//          ELECTRICAL SPARKS ON DAMAGED SHIPS EFFECT STUFF
// =================================================
void shipfx_do_damaged_arcs_frame( ship *shipp );


// =================================================
//				NEBULA LIGHTNING.
// =================================================
void shipfx_do_lightning_frame( ship *shipp );

// engine wash level init
void shipfx_engine_wash_level_init();

// pause engine wash sounds
void shipfx_stop_engine_wash_sound();

// =====================================================
// CLOAKING
// =====================================================

//translate the texture matrix some
void shipfx_cloak_frame(ship *shipp, float frametime);
void shipfx_start_cloak(ship *shipp);
void shipfx_stop_cloak(ship *shipp);


#endif
