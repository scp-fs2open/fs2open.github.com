/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "freespace.h"
#include "network/multi.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "scripting/scripting.h"
#include "scripting/api/objs/vecmath.h"
#include "ship/ship.h"
#include "stats/scoring.h"
#include "weapon/weapon.h"


/**
 * Checks weapon-weapon collisions.  
 * @param pair obj_pair pointer to the two objects. pair->a and pair->b are weapons.
 * @return 1 if all future collisions between these can be ignored
 */
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

	if (wipA->weapon_hitpoints > 0) {
		if (!(wipA->wi_flags[Weapon::Info_Flags::No_radius_doubling])) {
			A_radius *= 2;		// Makes bombs easier to hit
		}
		
		if ((The_mission.ai_profile->flags[AI::Profile_Flags::Aspect_invulnerability_fix]) && (wipA->is_locked_homing()) && (wpA->homing_object != &obj_used_list)) {
			if ( (wipA->max_lifetime - wpA->lifeleft) < The_mission.ai_profile->delay_bomb_arm_timer[Game_skill_level] )
				return 0;
		}
		else if ( (wipA->lifetime - wpA->lifeleft) < The_mission.ai_profile->delay_bomb_arm_timer[Game_skill_level] )
			return 0;
	}

	if (wipB->weapon_hitpoints > 0) {
		if (!(wipB->wi_flags[Weapon::Info_Flags::No_radius_doubling])) {
			B_radius *= 2;		// Makes bombs easier to hit
		}
		if ((The_mission.ai_profile->flags[AI::Profile_Flags::Aspect_invulnerability_fix]) && (wipB->is_locked_homing()) && (wpB->homing_object != &obj_used_list)) {
			if ( (wipB->max_lifetime - wpB->lifeleft) < The_mission.ai_profile->delay_bomb_arm_timer[Game_skill_level] )
				return 0;
		}
		else if ( (wipB->lifetime - wpB->lifeleft) < The_mission.ai_profile->delay_bomb_arm_timer[Game_skill_level] )
			return 0;
	}

	//	Rats, do collision detection.
	if (collide_subdivide(&A->last_pos, &A->pos, A_radius, &B->last_pos, &B->pos, B_radius))
	{
		bool a_override = false, b_override = false;

		if (scripting::hooks::OnWeaponCollision->isActive()) {
			a_override = scripting::hooks::OnWeaponCollision->isOverride(scripting::hooks::CollisionConditions{ {A, B} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', A),
					scripting::hook_param("Object", 'o', B),
					scripting::hook_param("Weapon", 'o', A),
					scripting::hook_param("WeaponB", 'o', B),
					scripting::hook_param("Hitpos", 'o', B->pos)));
			//Yes, this should be reversed
			b_override = scripting::hooks::OnWeaponCollision->isOverride(scripting::hooks::CollisionConditions{ {A, B} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', B),
					scripting::hook_param("Object", 'o', A),
					scripting::hook_param("Weapon", 'o', B),
					scripting::hook_param("WeaponB", 'o', A),
					scripting::hook_param("Hitpos", 'o', A->pos)));
		}

		// damage calculation should not be done on clients, the server will tell the client version of the bomb when to die
		if(!a_override && !b_override && !MULTIPLAYER_CLIENT)
		{
			float aDamage = wipA->damage;
			if (wipB->armor_type_idx >= 0)
				aDamage = Armor_types[wipB->armor_type_idx].GetDamage(aDamage, wipA->damage_type_idx, 1.0f);

			float bDamage = wipB->damage;
			if (wipA->armor_type_idx >= 0)
				bDamage = Armor_types[wipA->armor_type_idx].GetDamage(bDamage, wipB->damage_type_idx, 1.0f);

			if (wipA->weapon_hitpoints > 0) {
				if (wipB->weapon_hitpoints > 0) {		//	Two bombs collide, detonate both.
					if ((wipA->wi_flags[Weapon::Info_Flags::Bomb]) && (wipB->wi_flags[Weapon::Info_Flags::Bomb])) {
						wpA->lifeleft = 0.001f;
						wpA->weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);
						wpA->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
						wpB->lifeleft = 0.001f;
						wpB->weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);
						wpB->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
					} else {
						A->hull_strength -= bDamage;
						B->hull_strength -= aDamage;

						// safety to make sure either of the weapons die - allow 'bulkier' to keep going
						if ((A->hull_strength > 0.0f) && (B->hull_strength > 0.0f)) {
							if (wipA->weapon_hitpoints > wipB->weapon_hitpoints) {
								B->hull_strength = -1.0f;
							} else {
								A->hull_strength = -1.0f;
							}
						}
						
						if (A->hull_strength < 0.0f) {
							wpA->lifeleft = 0.001f;
							wpA->weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);
							wpA->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
						}
						if (B->hull_strength < 0.0f) {
							wpB->lifeleft = 0.001f;
							wpB->weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);
							wpB->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
						}
					}
				} else {
					A->hull_strength -= bDamage;
					wpB->lifeleft = 0.001f;
					wpB->weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);
					wpB->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
					if (A->hull_strength < 0.0f) {
						wpA->lifeleft = 0.001f;
						wpA->weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);
						wpA->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
					}
				}
			} else if (wipB->weapon_hitpoints > 0) {
				B->hull_strength -= aDamage;
				wpA->lifeleft = 0.001f;
				wpA->weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);
				wpA->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
				if (B->hull_strength < 0.0f) {
					wpB->lifeleft = 0.001f;
					wpB->weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);
					wpB->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
				}
			}

			// single player and multiplayer masters evaluate the scoring and kill stuff
			if (!MULTIPLAYER_CLIENT) {

				// If bomb was destroyed, do scoring
				if (wipA->wi_flags[Weapon::Info_Flags::Bomb]) {
					//Update stats. -Halleck
					scoring_eval_hit(A, B, 0);
					if (wpA->weapon_flags[Weapon::Weapon_Flags::Destroyed_by_weapon]) {
						scoring_eval_kill_on_weapon(A, B);
					}
				}
				if (wipB->wi_flags[Weapon::Info_Flags::Bomb]) {
					//Update stats. -Halleck
					scoring_eval_hit(B, A, 0);
					if (wpB->weapon_flags[Weapon::Weapon_Flags::Destroyed_by_weapon]) {
						scoring_eval_kill_on_weapon(B, A);
					}
				}
			}
		}

		if (!scripting::hooks::OnWeaponCollision->isActive()) {
			return 1;
		}

		if(!(b_override && !a_override))
		{
			scripting::hooks::OnWeaponCollision->run(scripting::hooks::CollisionConditions{ {A, B} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', A),
					scripting::hook_param("Object", 'o', B),
					scripting::hook_param("Weapon", 'o', A),
					scripting::hook_param("WeaponB", 'o', B),
					scripting::hook_param("Hitpos", 'o', B->pos)));
		}
		else
		{
			// Yes, this should be reversed.
			scripting::hooks::OnWeaponCollision->run(scripting::hooks::CollisionConditions{ {A, B} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', B),
					scripting::hook_param("Object", 'o', A),
					scripting::hook_param("Weapon", 'o', B),
					scripting::hook_param("WeaponB", 'o', A),
					scripting::hook_param("Hitpos", 'o', A->pos)));
		}

		return 1;
	}

	return 0;
}
