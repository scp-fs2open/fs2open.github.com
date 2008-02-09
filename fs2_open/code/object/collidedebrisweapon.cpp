/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Object/CollideDebrisWeapon.cpp $
 * $Revision: 2.9 $
 * $Date: 2007-11-23 23:49:33 $
 * $Author: wmcoolmon $
 *
 * Routines to detect collisions and do physics, damage, etc for weapons and debris
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.8  2007/02/19 07:24:51  wmcoolmon
 * WMCoolmon experiences a duh moment. Move scripting collision variable declarations in front of overrides, to give
 * them access to these (somewhat useful) variables
 *
 * Revision 2.7  2007/01/14 10:26:38  wmcoolmon
 * Attempt to remove various warnings under MSVC 2003, mostly related to casting, but also some instances of inaccessible code.
 *
 * Revision 2.6  2006/12/28 00:59:39  wmcoolmon
 * WMC codebase commit. See pre-commit build thread for details on changes.
 *
 * Revision 2.5  2005/04/05 05:53:21  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.4  2004/07/26 20:47:45  Kazan
 * remove MCD complete
 *
 * Revision 2.3  2004/07/12 16:32:59  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.2  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:08  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 3     10/16/98 1:22p Andsager
 * clean up header files
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 7     4/02/98 6:29p Lawrance
 * compile out asteroid references for demo
 * 
 * 6     3/02/98 2:58p Mike
 * Make "asteroids" in debug console turn asteroids on/off.
 * 
 * 5     2/19/98 12:46a Lawrance
 * Further work on asteroids.
 * 
 * 4     2/05/98 12:51a Mike
 * Early asteroid stuff.
 * 
 * 3     1/13/98 8:09p John
 * Removed the old collision system that checked all pairs.   Added code
 * to disable collisions and particles.
 * 
 * 2     9/17/97 5:12p John
 * Restructured collision routines.  Probably broke a lot of stuff.
 * 
 * 1     9/17/97 2:14p John
 * Initial revision
 *
 * $NoKeywords: $
 */

#include "object/objcollide.h"
#include "object/object.h"
#include "asteroid/asteroid.h"
#include "debris/debris.h"
#include "weapon/weapon.h"
#include "math/fvi.h"
#include "parse/scripting.h"



// placeholder struct for ship_debris collisions
typedef struct ship_weapon_debris_struct {
	object	*ship_object;
	object	*debris_object;
	vec3d	ship_collision_cm_pos;
	vec3d	r_ship;
	vec3d	collision_normal;
	int		shield_hit_tri;
	vec3d	shield_hit_tri_point;
	float		impulse;
} ship_weapon_debris_struct;


// Checks debris-weapon collisions.  pair->a is debris and pair->b is weapon.
// Returns 1 if all future collisions between these can be ignored
int collide_debris_weapon( obj_pair * pair )
{
	vec3d	hitpos;
	int		hit;
	object *pdebris = pair->a;
	object *weapon = pair->b;

	Assert( pdebris->type == OBJ_DEBRIS );
	Assert( weapon->type == OBJ_WEAPON );

	// first check the bounding spheres of the two objects.
	hit = fvi_segment_sphere(&hitpos, &weapon->last_pos, &weapon->pos, &pdebris->pos, pdebris->radius);
	if (hit) {
		hit = debris_check_collision(pdebris, weapon, &hitpos );
		if ( !hit )
			return 0;

		Script_system.SetHookObjects(4, "Weapon", weapon, "Debris", pdebris, "Self",weapon, "Object", pdebris);
		bool weapon_override = Script_system.IsConditionOverride(CHA_COLLIDEDEBRIS, weapon);

		Script_system.SetHookObjects(2, "Self",pdebris, "Object", weapon);
		bool debris_override = Script_system.IsConditionOverride(CHA_COLLIDEWEAPON, pdebris);

		if(!weapon_override && !debris_override)
		{
			weapon_hit( weapon, pdebris, &hitpos );
			debris_hit( pdebris, weapon, &hitpos, Weapon_info[Weapons[weapon->instance].weapon_info_index].damage );
		}

		Script_system.SetHookObjects(2, "Self",weapon, "Object", pdebris);
		if(!(debris_override && !weapon_override))
			Script_system.RunCondition(CHA_COLLIDEDEBRIS, '\0', NULL, weapon);

		Script_system.SetHookObjects(2, "Self",pdebris, "Object", weapon);
		if((debris_override && !weapon_override) || (!debris_override && !weapon_override))
			Script_system.RunCondition(CHA_COLLIDEWEAPON, '\0', NULL, pdebris);

		Script_system.RemHookVars(4, "Weapon", "Debris", "Self","ObjectB");
		return 0;

	} else {
		return weapon_will_never_hit( weapon, pdebris, pair );
	}
}				



// Checks debris-weapon collisions.  pair->a is debris and pair->b is weapon.
// Returns 1 if all future collisions between these can be ignored
int collide_asteroid_weapon( obj_pair * pair )
{
	if (!Asteroids_enabled)
		return 0;

	vec3d	hitpos;
	int		hit;
	object	*pasteroid = pair->a;
	object	*weapon = pair->b;

	Assert( pasteroid->type == OBJ_ASTEROID);
	Assert( weapon->type == OBJ_WEAPON );

	// first check the bounding spheres of the two objects.
	hit = fvi_segment_sphere(&hitpos, &weapon->last_pos, &weapon->pos, &pasteroid->pos, pasteroid->radius);
	if (hit) {
		hit = asteroid_check_collision(pasteroid, weapon, &hitpos );
		if ( !hit )
			return 0;
			
		Script_system.SetHookObjects(4, "Weapon", weapon, "Asteroid", pasteroid, "Self",weapon, "Object", pasteroid);

		bool weapon_override = Script_system.IsConditionOverride(CHA_COLLIDEASTEROID, weapon);
		Script_system.SetHookObjects(2, "Self",pasteroid, "Object", weapon);
		bool asteroid_override = Script_system.IsConditionOverride(CHA_COLLIDEWEAPON, pasteroid);

		if(!weapon_override && !asteroid_override)
		{
			weapon_hit( weapon, pasteroid, &hitpos );
			asteroid_hit( pasteroid, weapon, &hitpos, Weapon_info[Weapons[weapon->instance].weapon_info_index].damage );
		}

		Script_system.SetHookObjects(2, "Self",weapon, "Object", pasteroid);
		if(!(asteroid_override && !weapon_override))
			Script_system.RunCondition(CHA_COLLIDEASTEROID, '\0', NULL, weapon);

		Script_system.SetHookObjects(2, "Self",pasteroid, "Object", weapon);
		if((asteroid_override && !weapon_override) || (!asteroid_override && !weapon_override))
			Script_system.RunCondition(CHA_COLLIDEWEAPON, '\0', NULL, pasteroid);

		Script_system.RemHookVars(4, "Weapon", "Asteroid", "Self","ObjectB");
		return 0;

	}

	return weapon_will_never_hit( weapon, pasteroid, pair );
}				


