/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/ShipHit.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:28 $
 * $Author: penguin $
 *
 * Code to deal with a ship getting hit by something, be it a missile, dog, or ship.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 9     8/22/99 5:53p Dave
 * Scoring fixes. Added self destruct key. Put callsigns in the logfile
 * instead of ship designations for multiplayer players.
 * 
 * 8     6/21/99 7:25p Dave
 * netplayer pain packet. Added type E unmoving beams.
 * 
 * 7     5/11/99 10:16p Andsager
 * First pass on engine wash effect.  Rotation (control input), damage,
 * shake.  
 * 
 * 6     4/20/99 3:43p Andsager
 * Added normal parameter to ship_apply_local_damage for case of ship_ship
 * collision.
 * 
 * 5     2/04/99 1:23p Andsager
 * Apply max spark limit to ships created in mission parse
 * 
 * 4     10/20/98 1:39p Andsager
 * Make so sparks follow animated ship submodels.  Modify
 * ship_weapon_do_hit_stuff() and ship_apply_local_damage() to add
 * submodel_num.  Add submodel_num to multiplayer hit packet.
 * 
 * 3     10/16/98 1:22p Andsager
 * clean up header files
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 8     5/18/98 12:41a Allender
 * fixed subsystem problems on clients (i.e. not reporting properly on
 * damage indicator).  Fixed ingame join problem with respawns.  minor
 * comm menu stuff
 * 
 * 7     4/24/98 5:35p Andsager
 * Fix sparks sometimes drawing not on model.  If ship is sphere in
 * collision, don't draw sparks.  Modify ship_apply_local_damage() to take
 * parameter no_spark.
 * 
 * 6     4/09/98 5:44p Allender
 * multiplayer network object fixes.  debris and self destructed ships
 * should all sync up.  Fix problem where debris pieces (hull pieces) were
 * not getting a net signature
 * 
 * 5     2/13/98 2:22p Allender
 * make ships spark when entering mission with < 100% hull
 * 
 * 4     10/16/97 4:41p Allender
 * new packet to tell clients when subsystem has been destroyed
 * 
 * 3     9/18/97 4:08p John
 * Cleaned up & restructured ship damage stuff.
 * 
 * 2     9/17/97 5:12p John
 * Restructured collision routines.  Probably broke a lot of stuff.
 * 
 * 1     9/17/97 3:42p John
 *
 * $NoKeywords: $
 */

#ifndef _SHIPHIT_H
#define _SHIPHIT_H

struct ship;
struct ship_subsys;
struct object;

#define NO_SPARKS			0
#define CREATE_SPARKS	1

#define MISS_SHIELDS		-1

// =====================   NOTE!! =========================
// To apply damage to a ship, call either ship_apply_local_damage or ship_apply_global_damage.
// These replace the old calls to ship_hit and ship_do_damage...
// These functions do nothing to the ship's physics; that is the responsibility
// of whoever is calling these functions.  These functions are strictly
// for damaging ship's hulls, shields, and subsystems.  Nothing more.

// function to destroy a subsystem.  Called internally and from multiplayer messaging code
extern void do_subobj_destroyed_stuff( ship *ship_p, ship_subsys *subsys, vector *hitpos );


// This gets called to apply damage when something hits a particular point on a ship.
// This assumes that whoever called this knows if the shield got hit or not.
// hitpos is in world coordinates.
// if shield_quadrant is not -1, then that part of the shield takes damage properly.
void ship_apply_local_damage(object *ship_obj, object *other_obj, vector *hitpos, float damage, int shield_quadrant, bool create_spark=true, int submodel_num=-1, vector *hit_normal=NULL);

// This gets called to apply damage when a damaging force hits a ship, but at no 
// point in particular.  Like from a shockwave.   This routine will see if the
// shield got hit and if so, apply damage to it.
// You can pass force_center==NULL if you the damage doesn't come from anywhere,
// like for debug keys to damage an object or something.  It will 
// assume damage is non-directional and will apply it correctly.   
void ship_apply_global_damage(object *ship_obj, object *other_obj, vector *force_center, float damage );

// like above, but does not apply damage to shields
void ship_apply_wash_damage(object *ship_obj, object *other_obj, float damage);

// next routine needed for multiplayer
void ship_hit_kill( object *ship_obj, object *other_obj, float percent_killed, int self_destruct);

void ship_self_destruct( object *objp );

// Call this instead of physics_apply_whack directly to 
// deal with two ships docking properly.
void ship_apply_whack(vector *force, vector *new_pos, object *objp);

// externed for code in missionparse to create sparks on a ship with < 100% hull integrity.
void ship_hit_sparks_no_rotate(object *ship_obj, vector *hitpos);

// externed so that ships which self destruct have the proper things done to them in multiplayer
void ship_generic_kill_stuff( object *objp, float percent_killed );

// find the max number of sparks allowed for ship
// limited for fighter by hull % others by radius.
int get_max_sparks(object* ship_obj);

// player pain
void ship_hit_pain(float damage);


#endif //_SHIPHIT_H
