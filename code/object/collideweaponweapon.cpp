/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "object/objcollide.h"
#include "object/object.h"
#include "weapon/weapon.h"
#include "ship/ship.h"
#include "parse/lua.h"
#include "parse/scripting.h"
#include "freespace2/freespace.h"


// moved to ai_profiles.tbl
//#define	BOMB_ARM_TIME	1.5f

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

	//WMC - Here's a reason why...scripting now!

	if (wipA->weapon_hitpoints > 0) {
		if (!(wipA->wi_flags2 & WIF2_HARD_TARGET_BOMB)) {
			A_radius *= 2;		// Makes bombs easier to hit
		}
		if ( (wipA->lifetime - wpA->lifeleft) < The_mission.ai_profile->delay_bomb_arm_timer[Game_skill_level] )
			return 0;
	}

	if (wipB->weapon_hitpoints > 0) {
		if (!(wipB->wi_flags2 & WIF2_HARD_TARGET_BOMB)) {
			B_radius *= 2;		// Makes bombs easier to hit
		}
		if ( (wipB->lifetime - wpB->lifeleft) < The_mission.ai_profile->delay_bomb_arm_timer[Game_skill_level] )
			return 0;
	}

	//	Rats, do collision detection.
	if (collide_subdivide(&A->last_pos, &A->pos, A_radius, &B->last_pos, &B->pos, B_radius))
	{
		Script_system.SetHookObjects(4, "Weapon", A, "WeaponB", B, "Self",A, "Object", B);
		bool a_override = Script_system.IsConditionOverride(CHA_COLLIDEWEAPON, A);
		
		//Should be reversed
		Script_system.SetHookObjects(4, "Weapon", B, "WeaponB", A, "Self",B, "Object", A);
		bool b_override = Script_system.IsConditionOverride(CHA_COLLIDEWEAPON, B);

		if(!a_override && !b_override)
		{
			//Do the normal stuff if no override -C
			ship	*sap, *sbp;

			sap = &Ships[Objects[A->parent].instance];
			sbp = &Ships[Objects[B->parent].instance];

			// MWA -- commented out next line because it was too long for output window on occation.
			// Yes -- I should fix the output window, but I don't have time to do it now.
			//nprintf(("AI", "[%s] %s's missile %i shot down by [%s] %s's laser %i\n", Iff_info[sbp->team].iff_name, sbp->ship_name, B->instance, Iff_info[sap->team].iff_name, sap->ship_name, A->instance));
			if (wipA->weapon_hitpoints > 0) {
				if (wipB->weapon_hitpoints > 0) {		//	Two bombs collide, detonate both.
					if ((wipA->wi_flags & WIF_BOMB) && (wipB->wi_flags & WIF_BOMB)) {
						Weapons[A->instance].lifeleft = 0.01f;
						Weapons[B->instance].lifeleft = 0.01f;
						Weapons[A->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
						Weapons[B->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
					} else {
						A->hull_strength -= wipB->damage;
						B->hull_strength -= wipA->damage;

						// safety to make sure either of the weapons die - allow 'bulkier' to keep going
						if ((A->hull_strength > 0.0f) && (B->hull_strength > 0.0f)) {
							if (wipA->weapon_hitpoints > wipB->weapon_hitpoints) {
								B->hull_strength = -1.0f;
							} else {
								A->hull_strength = -1.0f;
							}
						}
						
						if (A->hull_strength < 0.0f) {
							Weapons[A->instance].lifeleft = 0.01f;
							Weapons[A->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
						}
						B->hull_strength -= wipA->damage;
						if (B->hull_strength < 0.0f) {
							Weapons[B->instance].lifeleft = 0.01f;
							Weapons[B->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
						}
					}
				} else {
					A->hull_strength -= wipB->damage;
					Weapons[B->instance].lifeleft = 0.01f;
					Weapons[B->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
					if (A->hull_strength < 0.0f) {
						Weapons[A->instance].lifeleft = 0.01f;
						Weapons[A->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
					}
				}
			} else if (wipB->weapon_hitpoints > 0) {
				B->hull_strength -= wipA->damage;
				Weapons[A->instance].lifeleft = 0.01f;
				Weapons[A->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
				if (B->hull_strength < 0.0f) {
					Weapons[B->instance].lifeleft = 0.01f;
					Weapons[B->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
				}
			}
	#ifndef NDEBUG
			float dist = 0.0f;

			if (Weapons[A->instance].lifeleft == 0.01f) {
				dist = vm_vec_dist_quick(&A->pos, &wpA->homing_pos);
				//nprintf(("AI", "Frame %i: Weapon %s shot down. Dist: %.1f, inner: %.0f, outer: %.0f\n", Framecount, wipA->name, dist, wipA->inner_radius, wipA->outer_radius));
			}
			if (Weapons[B->instance].lifeleft == 0.01f) {
				dist = vm_vec_dist_quick(&A->pos, &wpB->homing_pos);
				//nprintf(("AI", "Frame %i: Weapon %s shot down. Dist: %.1f, inner: %.0f, outer: %.0f\n", Framecount, wipB->name, dist, wipB->inner_radius, wipB->outer_radius));
			}
	#endif
		}

		if(!(b_override && !a_override))
		{
			Script_system.SetHookObjects(4, "Weapon", A, "WeaponB", B, "Self",A, "Object", B);
			Script_system.RunCondition(CHA_COLLIDEWEAPON, '\0', NULL, A);
		}
		if((b_override && !a_override) || (!b_override && !a_override))
		{
			//SHould be reversed
			Script_system.SetHookObjects(4, "Weapon", B, "WeaponB", A, "Self",B, "Object", A);
			Script_system.RunCondition(CHA_COLLIDEWEAPON, '\0', NULL, B);
		}

		Script_system.RemHookVars(4, "Weapon", "WeaponB", "Self","ObjectB");
		return 1;
	}

	return 0;
}

