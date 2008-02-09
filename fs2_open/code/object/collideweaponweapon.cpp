/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Object/CollideWeaponWeapon.cpp $
 * $Revision: 2.6 $
 * $Date: 2005-03-25 06:57:36 $
 * $Author: wmcoolmon $
 *
 * Routines to detect collisions and do physics, damage, etc for weapons and weapons
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2004/07/26 20:47:45  Kazan
 * remove MCD complete
 *
 * Revision 2.4  2004/07/12 16:32:59  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.3  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/09/16 11:56:46  unknownplayer
 * Changed the ship selection window to load the 3D FS2 ship models instead
 * of the custom *.ani files. It just does a techroom rotation for now, but I'll add
 * more features later - tell me of any problems or weirdness caused by it, or if
 * you don't like it and want it as an option only.
 *
 * Revision 2.1  2002/08/01 01:41:08  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     8/27/99 9:32a Andsager
 * Debug info for shooting down bombs
 * 
 * 4     8/27/99 1:34a Andsager
 * Modify damage by shockwaves for BIG|HUGE ships.  Modify shockwave damge
 * when weapon blows up.
 * 
 * 3     10/16/98 1:22p Andsager
 * clean up header files
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 8     5/22/98 11:00a Mike
 * Balance sm3-09a.
 * 
 * 7     4/15/98 11:30p Mike
 * Make bombs not drop for first half second and increase arm time from
 * 1.0 to 1.5 seconds.
 * 
 * 6     3/05/98 5:48p Lawrance
 * Double radius of bombs when doing weapon-weapon collisions
 * 
 * 5     2/22/98 12:19p John
 * Externalized some strings
 * 
 * 4     2/14/98 3:38p Mike
 * Make bombs have arm time, drop for 1/2 second, have hull_strength.
 * Make them not fired until closer to target.
 * 
 * 3     10/24/97 2:14p Adam
 * removed nprintf() call whose string is too long
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
#include "weapon/weapon.h"
#include "ship/ship.h"



#ifndef NDEBUG
//XSTR:OFF
char *lTeamNames[3] = { "Hostile", "Friendly", "Unknown" };
//XSTR:ON
#endif

#define	BOMB_ARM_TIME	1.5f

// Checks weapon-weapon collisions.  pair->a and pair->b are weapons.
// Returns 1 if all future collisions between these can be ignored
int collide_weapon_weapon( obj_pair * pair )
{
	float A_radius, B_radius;
	object *A = pair->a;
	object *B = pair->b;

	Assert( A->type == OBJ_WEAPON );
	Assert( B->type == OBJ_WEAPON );
	
	//	Don't allow ship to shoot down its own missile.
	if (A->parent_sig == B->parent_sig)
		return 1;

	//	Only shoot down teammate's missile if not traveling in nearly same direction.
	if (Weapons[A->instance].team == Weapons[B->instance].team)
		if (vm_vec_dot(&A->orient.vec.fvec, &B->orient.vec.fvec) > 0.7f)
			return 1;

	//	Ignore collisions involving a bomb if the bomb is not yet armed.
	weapon	*wpA, *wpB;
	weapon_info	*wipA, *wipB;

	wpA = &Weapons[A->instance];
	wpB = &Weapons[B->instance];
	wipA = &Weapon_info[wpA->weapon_info_index];
	wipB = &Weapon_info[wpB->weapon_info_index];

	A_radius = A->radius;
	B_radius = B->radius;

	// UnknownPlayer : Should we even be bothering with collision detection is neither one of these is a bomb?

	if (wipA->wi_flags & WIF_BOMB) {
		A_radius *= 2;		// Makes bombs easier to hit
		if (wipA->lifetime - wpA->lifeleft < BOMB_ARM_TIME)
			return 0;
	}

	if (wipB->wi_flags & WIF_BOMB) {
		B_radius *= 2;		// Makes bombs easier to hit
		if (wipB->lifetime - wpB->lifeleft < BOMB_ARM_TIME)
			return 0;
	}

	//	Rats, do collision detection.
	if (collide_subdivide(&A->last_pos, &A->pos, A_radius, &B->last_pos, &B->pos, B_radius)) {
		ship	*sap, *sbp;

		sap = &Ships[Objects[A->parent].instance];
		sbp = &Ships[Objects[B->parent].instance];
		// MWA -- commented out next line because it was too long for output window on occation.
		// Yes -- I should fix the output window, but I don't have time to do it now.
		//nprintf(("AI", "[%s] %s's missile %i shot down by [%s] %s's laser %i\n", lTeamNames[sbp->team], sbp->ship_name, B->instance, lTeamNames[sap->team], sap->ship_name, A->instance));
		if (wipA->wi_flags & WIF_BOMB) {
			if (wipB->wi_flags & WIF_BOMB) {		//	Two bombs collide, detonate both.
				Weapons[A->instance].lifeleft = 0.01f;
				Weapons[B->instance].lifeleft = 0.01f;
				Weapons[A->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
				Weapons[B->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
			} else {
				A->hull_strength -= wipB->damage;
				if (A->hull_strength < 0.0f) {
					Weapons[A->instance].lifeleft = 0.01f;
					Weapons[A->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
				}
			}
		} else if (wipB->wi_flags & WIF_BOMB) {
			B->hull_strength -= wipA->damage;
			if (B->hull_strength < 0.0f) {
				Weapons[B->instance].lifeleft = 0.01f;
				Weapons[B->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
			}
		}
#ifndef NDEBUG
		float dist = 0.0f;

		if (Weapons[A->instance].lifeleft == 0.01f) {
			dist = vm_vec_dist_quick(&A->pos, &wpA->homing_pos);
			nprintf(("AI", "Frame %i: Weapon %s shot down. Dist: %.1f, inner: %.0f, outer: %.0f\n", Framecount, wipA->name, dist, wipA->inner_radius, wipA->outer_radius));
		}
		if (Weapons[B->instance].lifeleft == 0.01f) {
			dist = vm_vec_dist_quick(&A->pos, &wpB->homing_pos);
			nprintf(("AI", "Frame %i: Weapon %s shot down. Dist: %.1f, inner: %.0f, outer: %.0f\n", Framecount, wipB->name, dist, wipB->inner_radius, wipB->outer_radius));
		}
#endif
		return 1;
	}

	return 0;
}

