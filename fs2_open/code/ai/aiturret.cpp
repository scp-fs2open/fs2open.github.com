

#include "globalincs/systemvars.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "freespace2/freespace.h"
#include "asteroid/asteroid.h"
#include "globalincs/linklist.h"
#include "weapon/beam.h"
#include "weapon/swarm.h"
#include "io/timer.h"
#include "render/3d.h"
#include "gamesnd/gamesnd.h"
#include "ship/shipfx.h"
#include "network/multimsgs.h"
#include "weapon/flak.h"
#include "math/staticrand.h"
#include "network/multi.h"
#include "ai/aibig.h"
#include "object/objectdock.h"
#include "ai/aiinternal.h"	//Included last, so less includes are needed
#include "iff_defs/iff_defs.h"
#include "weapon/muzzleflash.h"
#include "parse/scripting.h"

#include <limits.h>


// How close a turret has to be point at its target before it
// can fire.  If the dot of the gun normal and the vector from gun
// to target is greater than this, the turret fires.  The smaller
// the sloppier the shooting.
#define AICODE_TURRET_DUMBFIRE_ANGLE		(0.8f)	
#define AICODE_TURRET_HEATSEEK_ANGLE		(0.7f)	
#define AICODE_TURRET_MAX_TIME_IN_RANGE	(5.0f)
#define BEAM_NEBULA_RANGE_REDUCE_FACTOR		(0.8f)

float Lethality_range_const = 2.0f;
DCF(lethality_range, "N for modifying range: 1 / (1+N) at 100")
{
	dc_get_arg(ARG_FLOAT);
	Lethality_range_const = Dc_arg_float;
}

float Player_lethality_bump[NUM_SKILL_LEVELS] = {
	// 0.0f, 5.0f, 10.0f, 25.0f, 40.0f
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

char *Turret_target_order_names[NUM_TURRET_ORDER_TYPES] = {
	"Bombs",
	"Ships",
	"Asteroids",
};

#define EEOF_BIG_ONLY		(1<<0)	// turret fires only at big and huge ships
#define EEOF_SMALL_ONLY		(1<<1)	// turret fires only at small ships
#define EEOF_TAGGED_ONLY	(1<<2)	// turret fires only at tagged ships
#define EEOF_BEAM			(1<<3)	// turret is a beam
#define EEOF_FLAK			(1<<4)	// turret is flak
#define EEOF_LASER			(1<<5)	// turret is a laser
#define EEOF_MISSILE		(1<<6)	// turret is a missile

typedef struct eval_enemy_obj_struct {
	int			turret_parent_objnum;			// parent of turret
	float			weapon_travel_dist;				// max targeting range of turret weapon
	int			enemy_team_mask;
	int			weapon_system_ok;					// is the weapon subsystem of turret ship ok
	int			eeo_flags;

	vec3d		*tpos;
	vec3d		*tvec;
	ship_subsys *turret_subsys;
	int			current_enemy;


	float			nearest_attacker_dist;			// nearest ship	
	int			nearest_attacker_objnum;

	float			nearest_homing_bomb_dist;		// nearest homing bomb
	int			nearest_homing_bomb_objnum;

	float			nearest_bomb_dist;				// nearest non-homing bomb
	int			nearest_bomb_objnum;

	float			nearest_dist;						// nearest ship attacking this turret
	int			nearest_objnum;
}	eval_enemy_obj_struct;

// return 1 if objp is in fov of the specified turret, tp.  Otherwise return 0.
//	dist = distance from turret to center point of object
int object_in_turret_fov(object *objp, ship_subsys *ss, vec3d *tvec, vec3d *tpos, float dist)
{
	vec3d	v2e;
	float size_mod;
	bool  in_fov;

	vm_vec_normalized_dir(&v2e, &objp->pos, tpos);
	size_mod = objp->radius / (dist + objp->radius);

	in_fov = turret_fov_test(ss, tvec, &v2e, size_mod);

	if ( in_fov ) {
 		return 1;
	}

	return 0;
}

bool is_object_radius_in_turret_fov(object *objp, ship_subsys *ss, vec3d *tvec, vec3d *tpos, vec3d *v2e, vec3d *predicted_pos, float distance)
{
	float target_dist = distance;
	if (distance == 0.0f)
		target_dist = vm_vec_dist(predicted_pos,tpos);

	if (object_in_turret_fov(objp, ss, tvec, tpos, target_dist + objp->radius)) {
		// so the targeted spot in not in fov but the enemy + radius is
		// lets align the darn gun and try shooting there
		vec3d temp_vec;
		float multiplier = 0;
		model_subsystem *tp = ss->system_info;

		// project v2e_from_turret to turret normal
		// substract resultant vector from the temp_vec (v2e_from_turret)
		// adjust z component as necessary
		// calculate multiplier for the resultant vector
		// use multiplier and the z component and compose a new vector
		float dot = vm_vec_dot(v2e, tvec);

		vm_vec_scale_add(&temp_vec, v2e, tvec, -dot);

		if (IS_VEC_NULL_SQ_SAFE(&temp_vec)) {
			// return false, target is perfectly aligned over or below the turret
			// safe bet is to allow turret to reacquire better target
			return false;
		}

		// normalize the vec, it needs to be done regardless
		vm_vec_normalize(&temp_vec);
		bool fix_elevation = false;
		bool fix_base_rot = false;

		if (dot < tp->turret_fov) {
			dot = tp->turret_fov;
			fix_elevation = true;
		}
		if (dot > tp->turret_max_fov) {
			dot = tp->turret_max_fov;
			fix_elevation = true;
		}

		if (tp->flags & MSS_FLAG_TURRET_ALT_MATH) {
			vec3d temp_vec2;
			vm_vec_rotate(&temp_vec2, &temp_vec, &ss->world_to_turret_matrix);

			// now in turrets frame of reference
			// check if math is actually possible
			if (!((temp_vec2.xyz.x == 0) && (temp_vec2.xyz.y == 0))) {
				float temp_z = temp_vec2.xyz.z;
				temp_vec2.xyz.z = 0.0f;
				// make sure null vecs wont happen
				if (!IS_VEC_NULL_SQ_SAFE(&temp_vec2)) {
					vm_vec_normalize(&temp_vec2);
					// only do this if it actually is required
					if (-temp_vec2.xyz.y < tp->turret_y_fov) {
						float check_pos = 1;

						fix_base_rot = true;
						temp_vec2.xyz.y = -tp->turret_y_fov;
						if (temp_vec2.xyz.x < 0)
						check_pos = -1;
						temp_vec2.xyz.x = check_pos * sqrtf(1 - (temp_vec2.xyz.y*temp_vec2.xyz.y));

						// restore z component
						float scaler = sqrtf(1 - (temp_z*temp_z));
						vm_vec_scale(&temp_vec2,scaler);
						temp_vec2.xyz.z = temp_z;
						// back to world frame
						vm_vec_unrotate(&temp_vec, &temp_vec2, &ss->world_to_turret_matrix);
					}
				}
			}
		}

		if (fix_elevation || fix_base_rot) {
			if (fix_elevation) {
				multiplier = sqrtf(1 - (dot*dot));
				// keep the temp_vec scaled with the tweaked vector
				vm_vec_scale(&temp_vec, multiplier);
			}
			vm_vec_scale_add(v2e, &temp_vec, tvec, dot);
			// and we are done with v2e...
			vm_vec_scale_add(predicted_pos, tpos, v2e, target_dist);
			// and we are done with predicted position
			return true;
		} else {
			mprintf(("Warning: Function 'is_object_radius_in_turret_fov' was called\nwithout need to fix turret alignments\n"));
			return false;
		}
	}
	// outside of the expanded radii, unable to align, return false
	return false;
}

// return 1 if bomb_objp is headed towards ship_objp
int bomb_headed_towards_ship(object *bomb_objp, object *ship_objp)
{
	float		dot;
	vec3d	bomb_to_ship_vector;

	vm_vec_normalized_dir(&bomb_to_ship_vector, &ship_objp->pos, &bomb_objp->pos);
	dot = vm_vec_dot(&bomb_objp->orient.vec.fvec, &bomb_to_ship_vector);

	if ( dot > 0 ) {
		return 1;
	}

	return 0;
}

// Set active weapon for turret
//This really isn't needed...but whatever -WMC

//Returns the best weapon on turret for target
//Note that all non-negative return values are expressed in
//what I like to call "widx"s.
//Returns -1 if unable to find a weapon for the target at all.
int turret_select_best_weapon(ship_subsys *turret, object *target)
{
	//TODO: Fill this out with extraodinary gun-picking algorithms
	if(turret->weapons.num_primary_banks > 0)
		return 0;
	else if(turret->weapons.num_secondary_banks > 0)
		return MAX_SHIP_PRIMARY_BANKS;
	else
		return -1;
}

//doesn't work for WIF2
//Returns true if all weapons in swp have the specified flag
bool all_turret_weapons_have_flags(ship_weapon *swp, int flags)
{
	int i;
	for(i = 0; i < swp->num_primary_banks; i++)
	{
		if(!(Weapon_info[swp->primary_bank_weapons[i]].wi_flags & flags))
			return false;
	}
	for(i = 0; i < swp->num_secondary_banks; i++)
	{
		if(!(Weapon_info[swp->secondary_bank_weapons[i]].wi_flags & flags))
			return false;
	}

	return true;
}

bool all_turret_weapons_have_flags2(ship_weapon *swp, int flags)
{
	int i;
	for(i = 0; i < swp->num_primary_banks; i++)
	{
		if(!(Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & flags))
			return false;
	}
	for(i = 0; i < swp->num_secondary_banks; i++)
	{
		if(!(Weapon_info[swp->secondary_bank_weapons[i]].wi_flags2 & flags))
			return false;
	}

	return true;
}

//Returns true if any of the weapons in swp have flags
bool turret_weapon_has_flags(ship_weapon *swp, int flags)
{
	int i = 0;
	for(i = 0; i < swp->num_primary_banks; i++)
	{
		if(Weapon_info[swp->primary_bank_weapons[i]].wi_flags & flags)
			return true;
	}
	for(i = 0; i < swp->num_secondary_banks; i++)
	{
		if(Weapon_info[swp->primary_bank_weapons[i]].wi_flags & flags)
			return true;
	}

	return false;
}

//does work for WIF2, but not WIF
//Returns true if any of the weapons in swp have flags
bool turret_weapon_has_flags2(ship_weapon *swp, int flags)
{
	int i = 0;
	for(i = 0; i < swp->num_primary_banks; i++)
	{
		if(Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & flags)
			return true;
	}
	for(i = 0; i < swp->num_secondary_banks; i++)
	{
		if(Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & flags)
			return true;
	}

	return false;
}

//It might be a little faster to optimize based on WP_LASER should only appear in primaries
//and WP_MISSILE in secondaries. but in the interest of future coding I leave it like this.
//Returns true if any of the weapons in swp have the subtype specified
bool turret_weapon_has_subtype(ship_weapon *swp, int subtype)
{
	int i = 0;
	for(i = 0; i < swp->num_primary_banks; i++)
	{
		if(Weapon_info[swp->primary_bank_weapons[i]].subtype == subtype)
			return true;
	}
	for(i = 0; i < swp->num_secondary_banks; i++)
	{
		if(Weapon_info[swp->secondary_bank_weapons[i]].subtype == subtype)
			return true;
	}

	return false;
}

//Use for getting a Weapon_info pointer, given a turret and a turret weapon indice
//Returns a pointer to the Weapon_info for weapon_num
weapon_info *get_turret_weapon_wip(ship_weapon *swp, int weapon_num)
{
	Assert(weapon_num < MAX_SHIP_WEAPONS);
	Assert(weapon_num >= 0);

	if(weapon_num >= MAX_SHIP_PRIMARY_BANKS)
		return &Weapon_info[swp->secondary_bank_weapons[weapon_num - MAX_SHIP_PRIMARY_BANKS]];
	else
		return &Weapon_info[swp->primary_bank_weapons[weapon_num]];
}

int get_turret_weapon_next_fire_stamp(ship_weapon *swp, int weapon_num)
{
	Assert(weapon_num < MAX_SHIP_WEAPONS);
	Assert(weapon_num >= 0);

	if(weapon_num >= MAX_SHIP_PRIMARY_BANKS)
		return swp->next_secondary_fire_stamp[weapon_num - MAX_SHIP_PRIMARY_BANKS];
	else
		return swp->next_primary_fire_stamp[weapon_num];
}

//This function is kinda slow
//Returns the longest-ranged weapon on a turret
float longest_turret_weapon_range(ship_weapon *swp)
{
	float longest_range_so_far = 0.0f;
	float weapon_range;
	weapon_info *wip;

	int i = 0;
	for(i = 0; i < swp->num_primary_banks; i++)
	{
		wip = &Weapon_info[swp->primary_bank_weapons[i]];
		if (wip->wi_flags2 & WIF2_LOCAL_SSM)
			weapon_range = wip->lssm_lock_range;
		else
			weapon_range = MIN(wip->lifetime * wip->max_speed, wip->weapon_range);

		if(weapon_range > longest_range_so_far)
			longest_range_so_far = weapon_range;
	}
	for(i = 0; i < swp->num_secondary_banks; i++)
	{
		wip = &Weapon_info[swp->secondary_bank_weapons[i]];
		if (wip->wi_flags2 & WIF2_LOCAL_SSM)
			weapon_range = wip->lssm_lock_range;
		else
			weapon_range = MIN(wip->lifetime * wip->max_speed, wip->weapon_range);

		if(weapon_range > longest_range_so_far)
			longest_range_so_far = weapon_range;
	}

	return longest_range_so_far;
}

// return !0 if objp can be considered for a turret target, 0 otherwise
// input:	objp				=>	object that turret is considering as an enemy
//				turret_parent	=>	object of ship that turret sits on
//				turret			=>	turret pointer
int valid_turret_enemy(object *objp, object *turret_parent)
{
	if ( objp == turret_parent ) {
		return 0;
	}

	if ( objp->type == OBJ_ASTEROID ) {
		return 1;
	}

	if ( objp->type == OBJ_SHIP ) {
		Assert( objp->instance >= 0 );
		ship *shipp;
		ship_info *sip;
		shipp = &Ships[objp->instance];
		sip = &Ship_info[shipp->ship_info_index];

		// don't fire at ships with protected bit set!!!
		if ( objp->flags & OF_PROTECTED ) {
			return 0;
		}

		// don't shoot at ships without collision check
		if (sip->flags & SIF_NO_COLLIDE) {
			return 0;
		}

		// don't shoot at arriving ships
		if (shipp->flags & SF_ARRIVING) {
			return 0;
		}

		// Goober5000 - don't fire at cargo containers (now specified in ship_types)
		if ( (sip->class_type >= 0) && !(Ship_types[sip->class_type].ai_bools & STI_AI_TURRETS_ATTACK) ) {
			return 0;
		}

		return 1;
	}

	if ( objp->type == OBJ_WEAPON ) {
		Assert( objp->instance >= 0 );
		weapon *wp = &Weapons[objp->instance];
		weapon_info *wip = &Weapon_info[wp->weapon_info_index];

		if ( (!(wip->wi_flags & WIF_BOMB) && !(Ai_info[Ships[turret_parent->instance].ai_index].ai_profile_flags & AIPF_ALLOW_TURRETS_TARGET_WEAPONS_FREELY) ) ) {
			return 0;
		}

		if ( (wip->wi_flags2 & WIF2_LOCAL_SSM) && (wp->lssm_stage == 3) ) {
			return 0;
		}

		if ( !iff_x_attacks_y(obj_team(turret_parent), wp->team) ) {
			return 0;
		}

		return 1;
	}

	return 0;
}

extern int Player_attacking_enabled;
void evaluate_obj_as_target(object *objp, eval_enemy_obj_struct *eeo)
{
	object	*turret_parent_obj = &Objects[eeo->turret_parent_objnum];
	ship *shipp;
	ship_subsys *ss = eeo->turret_subsys;
	float dist, dist_comp;
	bool turret_has_no_target = false;

	// Don't look for bombs when weapon system is not ok
	if (objp->type == OBJ_WEAPON && !eeo->weapon_system_ok) {
		return;
	}

	if ( !valid_turret_enemy(objp, turret_parent_obj) ) {
		return;
	}

#ifndef NDEBUG
	if (!Player_attacking_enabled && (objp == Player_obj)) {
		return;
	}
#endif

	if ( objp->type == OBJ_SHIP ) {
		shipp = &Ships[objp->instance];

		// check on enemy team
		if ( !iff_matches_mask(shipp->team, eeo->enemy_team_mask) ) {
			return;
		}

		// check if protected
		if (objp->flags & OF_PROTECTED) {
			return;
		}

		// check if beam protected
		if (eeo->eeo_flags & EEOF_BEAM) {
			if (objp->flags & OF_BEAM_PROTECTED) {
				return;
			}
		}

		// check if flak protected
		if (eeo->eeo_flags & EEOF_FLAK) {
			if (objp->flags & OF_FLAK_PROTECTED) {
				return;
			}
		}

		// check if laser protected
		if (eeo->eeo_flags & EEOF_LASER) {
			if (objp->flags & OF_LASER_PROTECTED) {
				return;
			}
		}

		// check if missile protected
		if (eeo->eeo_flags & EEOF_MISSILE) {
			if (objp->flags & OF_MISSILE_PROTECTED) {
				return;
			}
		}

		// don't shoot at small ships if we shouldn't
		if (eeo->eeo_flags & EEOF_BIG_ONLY) {
			if (!(Ship_info[shipp->ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))) {
				return;
			}
		}

		// don't shoot at big ships if we shouldn't
		if (eeo->eeo_flags & EEOF_SMALL_ONLY) {
			if ((Ship_info[shipp->ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))) {
				return;
			}
		}

		// check if	turret flagged to only target tagged ships
		if (eeo->eeo_flags & EEOF_TAGGED_ONLY) {
			if (!ship_is_tagged(objp)) {
				return;
			}
		}

		// check if valid target in nebula
		if ( !object_is_targetable(objp, &Ships[Objects[eeo->turret_parent_objnum].instance]) ) {
			// BYPASS ocassionally for stealth
			int try_anyway = FALSE;
			if ( is_object_stealth_ship(objp) ) {
				float turret_stealth_find_chance = 0.5f;
				float speed_mod = -0.1f + vm_vec_mag_quick(&objp->phys_info.vel) / 70.0f;
				if (frand() > (turret_stealth_find_chance + speed_mod)) {
					try_anyway = TRUE;
				}
			}

			if (!try_anyway) {
				return;
			}
		}

	} else {
		shipp = NULL;
	}

	// modify dist for BIG|HUGE, getting closest point on bbox, if not inside
	vec3d vec_to_target;
	vm_vec_sub(&vec_to_target, &objp->pos, eeo->tpos);
	dist = vm_vec_mag_quick(&vec_to_target) - objp->radius;
	
	if (dist < 0.0f) {
		dist = 0.0f;
	}
	
	dist_comp = dist;
	// if weapon has optimum range set then use it
	float optimum_range = ss->optimum_range;
	if (optimum_range > 0.0f) {
		if (dist < optimum_range) {
			dist_comp = (2*optimum_range) - dist;
		}
	}

	// if turret has been told to prefer targets from the current direction then do so
	float favor_one_side = ss->favor_current_facing;
	if (favor_one_side >= 1.0f) {
		vm_vec_normalize(&vec_to_target);
		float dot_to_target = vm_vec_dot(&ss->turret_last_fire_direction, &vec_to_target);
		dot_to_target = 1.0f - (dot_to_target / favor_one_side);
		dist_comp *= dot_to_target;
	}

	// check if object is a bomb attacking the turret parent
	// check if bomb is homing on the turret parent ship
	bool check_weapon = true;

	if ((Ai_info[Ships[turret_parent_obj->instance].ai_index].ai_profile_flags & AIPF_PREVENT_TARGETING_BOMBS_BEYOND_RANGE) && (dist > eeo->weapon_travel_dist)) {
		check_weapon = false;
	}

	if ((objp->type == OBJ_WEAPON) && check_weapon) {
		//Maybe restrict the number of turrets attacking this bomb
		if (ss->turret_max_bomb_ownage != -1) {	
			int num_att_turrets = num_turrets_attacking(turret_parent_obj, OBJ_INDEX(objp));
			if (num_att_turrets > ss->system_info->turret_max_bomb_ownage) {
				return;
			}
		}

		if ( Weapons[objp->instance].homing_object == &Objects[eeo->turret_parent_objnum] ) {
			if ( dist_comp < eeo->nearest_homing_bomb_dist ) {
				if (!(ss->flags & SSF_FOV_REQUIRED) && (eeo->current_enemy == -1)) {
					turret_has_no_target = true;
				}
				if ( (turret_has_no_target) || object_in_turret_fov(objp, ss, eeo->tvec, eeo->tpos, dist + objp->radius) ) {
					eeo->nearest_homing_bomb_dist = dist_comp;
					eeo->nearest_homing_bomb_objnum = OBJ_INDEX(objp);
				}
			}
		// if not homing, check if bomb is flying towards ship
		} else if ( bomb_headed_towards_ship(objp, &Objects[eeo->turret_parent_objnum]) ) {
			if ( dist_comp < eeo->nearest_bomb_dist ) {
				if (!(ss->flags & SSF_FOV_REQUIRED) && (eeo->current_enemy == -1)) {
					turret_has_no_target = true;
				}
				if ( (turret_has_no_target) || object_in_turret_fov(objp, ss, eeo->tvec, eeo->tpos, dist + objp->radius) ) {
					eeo->nearest_bomb_dist = dist_comp;
					eeo->nearest_bomb_objnum = OBJ_INDEX(objp);
				}
			}
		}
	} // end weapon section

	// maybe recalculate dist for big or huge ship
//	if (shipp && (Ship_info[shipp->ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))) {
//		fvi_ray_boundingbox(min, max, start, direction, hit);
//		dist = vm_vec_dist_quick(hit, tvec);
//	}

	// check for nearest attcker
	if ( (shipp) && (dist < eeo->weapon_travel_dist) ) {
		ai_info *aip = &Ai_info[shipp->ai_index];

		// modify distance based on number of turrets from my ship attacking enemy (add 10% per turret)
		// dist *= (num_enemies_attacking(OBJ_INDEX(objp))+2)/2;	//	prevents lots of ships from attacking same target
		int num_att_turrets = num_turrets_attacking(turret_parent_obj, OBJ_INDEX(objp));
		dist_comp *= (1.0f + 0.1f*num_att_turrets);

		// return if we're over the cap
//		int max_turrets = 3 + Game_skill_level * Game_skill_level;
		int max_turrets = The_mission.ai_profile->max_turret_ownage_target[Game_skill_level];
		if (objp->flags & OF_PLAYER_SHIP) {
			max_turrets = The_mission.ai_profile->max_turret_ownage_player[Game_skill_level];
		}
		// Apply the per-turret limit, if there is one
		if (ss->turret_max_target_ownage != -1) {
			max_turrets = MIN(max_turrets, ss->system_info->turret_max_target_ownage);
		}
		if (num_att_turrets > max_turrets) {
			return;
		}

		// modify distance based on lethality of objp to my ship
		float active_lethality = aip->lethality;
		if (objp->flags & OF_PLAYER_SHIP) {
			active_lethality += Player_lethality_bump[Game_skill_level];
		}

		dist_comp /= (1.0f + 0.01f*Lethality_range_const*active_lethality);

		// Make level 2 tagged ships more likely to be targeted
		if (shipp->level2_tag_left > 0.0f) {
			dist_comp *= 0.3f;
		}

		// check if objp is targeting the turret's ship, or if objp has just hit the turret's ship
		if ( aip->target_objnum == eeo->turret_parent_objnum || aip->last_objsig_hit == Objects[eeo->turret_parent_objnum].signature ) {
			// A turret will always target a ship that is attacking itself... self-preservation!
			if ( aip->targeted_subsys == eeo->turret_subsys ) {
				dist_comp *= 0.5f;	// highest priority
			}
		}

		// maybe update nearest attacker
		if ( dist_comp < eeo->nearest_attacker_dist ) {
			if (!(ss->flags & SSF_FOV_REQUIRED) && (eeo->current_enemy == -1)) {
				turret_has_no_target = true;
			}
			if ( (turret_has_no_target) || object_in_turret_fov(objp, ss, eeo->tvec, eeo->tpos, dist + objp->radius) ) {
				// nprintf(("AI", "Nearest enemy = %s, dist = %7.3f, dot = %6.3f, fov = %6.3f\n", Ships[objp->instance].ship_name, dist, vm_vec_dot(&v2e, tvec), tp->turret_fov));
				eeo->nearest_attacker_dist = dist_comp;
				eeo->nearest_attacker_objnum = OBJ_INDEX(objp);
			}
		}
	} // end ship section

	// check if object is an asteroid attacking the turret parent - taylor
	if (objp->type == OBJ_ASTEROID) {
		if ( eeo->turret_parent_objnum == asteroid_collide_objnum(objp) ) {
			// give priority to the closest asteroid *impact* (ms intervals)
			dist_comp *= 0.9f + (0.01f * asteroid_time_to_impact(objp));

			if (dist_comp < eeo->nearest_dist ) {
				if (!(ss->flags & SSF_FOV_REQUIRED) && (eeo->current_enemy == -1)) {
					turret_has_no_target = true;
				}
				if ( (turret_has_no_target) || object_in_turret_fov(objp, ss, eeo->tvec, eeo->tpos, dist + objp->radius) ) {
					eeo->nearest_dist = dist_comp;
					eeo->nearest_objnum = OBJ_INDEX(objp);
				}
			}
		}
	} // end asteroid selection
}

// return 0 only if objnum is beam protected and turret is beam turret
int is_target_beam_valid(ship_weapon *swp, object *objp)
{
	// check if turret has beam weapon
	if (all_turret_weapons_have_flags(swp, WIF_BEAM)) {
		if (objp->flags & OF_BEAM_PROTECTED) {
			return 0;
		}

		if (all_turret_weapons_have_flags(swp, WIF_HUGE)) {
			if (objp->type == OBJ_SHIP && !(Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP)) ) {
				return 0;
			}
		}
	}

	return 1;
}

//	Given an object and an enemy team, return the index of the nearest enemy object.
//
// input:
//				turret_parent_objnum	=> parent objnum for the turret
//				turret_subsys			=> pointer to system_info for the turret subsystem
//				enemy_team_mask		=> OR'ed TEAM_ flags for the enemy of the turret parent ship
//				tpos						=> position of turret (world coords)
//				tvec						=> forward vector of turret (world coords)
//				current_enemy			=>	objnum of current turret target
int get_nearest_turret_objnum(int turret_parent_objnum, ship_subsys *turret_subsys, int enemy_team_mask, vec3d *tpos, vec3d *tvec, int current_enemy, bool big_only_flag, bool small_only_flag, bool tagged_only_flag, bool beam_flag, bool flak_flag, bool laser_flag, bool missile_flag)
{
	//float					weapon_travel_dist;
	int					weapon_system_ok;
	object				*objp;
	eval_enemy_obj_struct eeo;
	ship_weapon *swp = &turret_subsys->weapons;

	// list of stuff to go thru
	ship_obj		*so;
	missile_obj *mo;

	//wip=&Weapon_info[tp->turret_weapon_type];
	//weapon_travel_dist = MIN(wip->lifetime * wip->max_speed, wip->weapon_range);

	//if (wip->wi_flags2 & WIF2_LOCAL_SSM)
	//	weapon_travel_dist=wip->lssm_lock_range;

	// Set flag based on strength of weapons subsystem.  If weapons subsystem is destroyed, don't let turrets fire at bombs
	weapon_system_ok = 0;
	if ( ship_get_subsystem_strength( &Ships[Objects[turret_parent_objnum].instance], SUBSYSTEM_WEAPONS ) > 0 ) {
		weapon_system_ok = 1;
	}

	// Initialize eeo struct.
	eeo.turret_parent_objnum = turret_parent_objnum;
	eeo.weapon_system_ok = weapon_system_ok;
	eeo.weapon_travel_dist = longest_turret_weapon_range(swp);

	// set flags
	eeo.eeo_flags = 0;
	if (big_only_flag)
		eeo.eeo_flags |= EEOF_BIG_ONLY;
	if (small_only_flag)
		eeo.eeo_flags |= EEOF_SMALL_ONLY;
	if (tagged_only_flag)
		eeo.eeo_flags |= EEOF_TAGGED_ONLY;

	// flags for weapon types
	if (beam_flag)
		eeo.eeo_flags |= EEOF_BEAM;
	else if (flak_flag)
		eeo.eeo_flags |= EEOF_FLAK;
	else if (laser_flag)
		eeo.eeo_flags |= EEOF_LASER;
	else if (missile_flag)
		eeo.eeo_flags |= EEOF_MISSILE;

	eeo.enemy_team_mask = enemy_team_mask;
	eeo.current_enemy = current_enemy;
	eeo.tpos = tpos;
	eeo.tvec = tvec;
	eeo.turret_subsys = turret_subsys;

	eeo.nearest_attacker_dist = 99999.0f;
	eeo.nearest_attacker_objnum = -1;

	eeo.nearest_homing_bomb_dist = 99999.0f;
	eeo.nearest_homing_bomb_objnum = -1;

	eeo.nearest_bomb_dist = 99999.0f;
	eeo.nearest_bomb_objnum = -1;

	eeo.nearest_dist = 99999.0f;
	eeo.nearest_objnum = -1;

	// here goes the new targeting priority setting
	int n_tgt_priorities;
	int priority_weapon_idx = -1;

	// check for turret itself first
	n_tgt_priorities = turret_subsys->num_target_priorities;

	// turret had no priorities set for it.. try weapons
	if (n_tgt_priorities <= 0) {
		if (swp->num_primary_banks > 0)
			// first try highest primary slot...
			priority_weapon_idx = swp->primary_bank_weapons[0];
		else
			// ...and then secondary slot
			priority_weapon_idx = swp->secondary_bank_weapons[0];

		if (priority_weapon_idx > -1)
			n_tgt_priorities = Weapon_info[priority_weapon_idx].num_targeting_priorities;
	}

	if (n_tgt_priorities > 0) {

		for(int i = 0; i < n_tgt_priorities; i++) {
			// courtesy of WMC...
			ai_target_priority *tt;
			if (priority_weapon_idx == -1)
				tt = &Ai_tp_list[turret_subsys->target_priority[i]];
			else
				tt = &Ai_tp_list[Weapon_info[priority_weapon_idx].targeting_priorities[i]];

			int n_types = (int)tt->ship_type.size();
			int n_s_classes = (int)tt->ship_class.size();
			int n_w_classes = (int)tt->weapon_class.size();
			
			bool found_something;
			object *ptr = GET_FIRST(&obj_used_list);
			
			while (ptr != END_OF_LIST(&obj_used_list)) {
				found_something = false;

				if(tt->obj_type > -1 && (ptr->type == tt->obj_type)) {
					found_something = true;
				}

				if( ( n_types > 0 ) && ( ptr->type == OBJ_SHIP ) ) {
					for (int j = 0; j < n_types; j++) {
						if ( Ship_info[Ships[ptr->instance].ship_info_index].class_type == tt->ship_type[j] ) {
							found_something = true;
						}
					}
				}

				if( ( n_s_classes > 0 ) && ( ptr->type == OBJ_SHIP ) ) {
					for (int j = 0; j < n_s_classes; j++) {
						if ( Ships[ptr->instance].ship_info_index == tt->ship_class[j] ) {
							found_something = true;
						}
					}
				}

				if( ( n_w_classes > 0 ) && ( ptr->type == OBJ_WEAPON ) ) {
					for (int j = 0; j < n_w_classes; j++) {
						if ( Weapons[ptr->instance].weapon_info_index == tt->weapon_class[j] ) {
							found_something = true;
						}
					}
				}

				if( ( (tt->wif2_flags != 0) || (tt->wif_flags != 0) ) && (ptr->type == OBJ_WEAPON) ) {
					if( ( (Weapon_info[Weapons[ptr->instance].weapon_info_index].wi_flags & tt->wif_flags ) == tt->wif_flags)
						&& ( (Weapon_info[Weapons[ptr->instance].weapon_info_index].wi_flags2 & tt->wif2_flags ) == tt->wif2_flags) ) {
							found_something = true;
					}
				}

				if( ( ( tt->sif_flags != 0 ) || ( tt->sif2_flags != 0 ) ) && (ptr->type == OBJ_SHIP) ) {
					if( ( (Ship_info[Ships[ptr->instance].ship_info_index].flags & tt->sif_flags) == tt->sif_flags)
						&& ( (Ship_info[Ships[ptr->instance].ship_info_index].flags2 & tt->sif2_flags) == tt->sif2_flags) ) {
							found_something = true;
					}
				}

				if((tt->obj_flags != 0) && !((ptr->flags & tt->obj_flags) == tt->obj_flags)) {
					found_something = true;
				}

				if(!(found_something)) {
					//we didnt find this object within this priority group
					//skip to next without evaluating the object as target
					ptr = GET_NEXT(ptr);

					continue;
				}


				evaluate_obj_as_target(ptr, &eeo);

				ptr = GET_NEXT(ptr);
			}

			//homing weapon entry...
			/*
			if ( eeo.nearest_homing_bomb_objnum != -1 ) {               // highest priority is an incoming homing bomb
				return eeo.nearest_homing_bomb_objnum;
				//weapon entry...
			} else if ( eeo.nearest_bomb_objnum != -1 ) {               // next highest priority is an incoming dumbfire bomb
				return eeo.nearest_bomb_objnum;
				//ship entry...
			} else if ( eeo.nearest_attacker_objnum != -1 ) {        // next highest priority is an attacking ship
				return eeo.nearest_attacker_objnum;
				//something else entry...
			} else if ( eeo.nearest_objnum != -1 ) {
				return eeo.nearest_objnum;
			}
			*/
			// if we got something...
			if ( ( eeo.nearest_homing_bomb_objnum != -1 ) || 
				( eeo.nearest_bomb_objnum != -1 ) || 
				( eeo.nearest_attacker_objnum != -1 ) ||
				( eeo.nearest_objnum != -1 ) )
			{
				// ...start with homing bombs...
				int return_objnum =	eeo.nearest_homing_bomb_objnum;
				float return_distance = eeo.nearest_homing_bomb_dist;

				// ...next test non-homing bombs...
				if ( eeo.nearest_bomb_dist < return_distance ) {
					return_objnum =  eeo.nearest_bomb_objnum;
					return_distance = eeo.nearest_bomb_dist;
				}

				// ...then attackers...
				if ( eeo.nearest_attacker_dist < return_distance ) {
					return_objnum =  eeo.nearest_attacker_objnum;
					return_distance = eeo.nearest_attacker_dist;
				}

				// ...and finally the rest of the lot...
				if ( eeo.nearest_dist < return_distance ) {
					return_objnum =  eeo.nearest_objnum;
				}

				// ...and return the objnum to the closest target regardless
				return return_objnum;
			}
		}
	} else {

		for(int i = 0; i < NUM_TURRET_ORDER_TYPES; i++)
		{
			ai_info *aip = &Ai_info[Ships[Objects[eeo.turret_parent_objnum].instance].ai_index];
			switch(turret_subsys->turret_targeting_order[i])
			{
				case -1:
						//Empty priority slot
					break;

				case 0:
					//Return if a bomb is found
					//don't fire anti capital ship turrets at bombs.
					if ( !((aip->ai_profile_flags & AIPF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS) && big_only_flag) )
					{
						// Missile_obj_list
						for( mo = GET_FIRST(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
							objp = &Objects[mo->objnum];
							
							Assert(objp->type == OBJ_WEAPON);
							if (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_BOMB)
							{
								evaluate_obj_as_target(objp, &eeo);
							}
						}
						// highest priority
						if ( eeo.nearest_homing_bomb_objnum != -1 ) {					// highest priority is an incoming homing bomb
							return eeo.nearest_homing_bomb_objnum;
						} else if ( eeo.nearest_bomb_objnum != -1 ) {					// next highest priority is an incoming dumbfire bomb
							return eeo.nearest_bomb_objnum;
						}
					}
					break;

				case 1:
					//Return if a ship is found
					// Ship_used_list
					for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
						objp = &Objects[so->objnum];
						evaluate_obj_as_target(objp, &eeo);
					}

					Assert(eeo.nearest_attacker_objnum < 0 || is_target_beam_valid(swp, &Objects[eeo.nearest_attacker_objnum]));
						// next highest priority is attacking ship
					if ( eeo.nearest_attacker_objnum != -1 ) {			// next highest priority is an attacking ship
						return eeo.nearest_attacker_objnum;
					}
					break;

				case 2:
					//Return if an asteroid is found
					// asteroid check - taylor
					asteroid_obj *ao;

					// don't use turrets that are better for other things:
					// - no cap ship beams
					// - no flak
					// - no heat or aspect missiles
					// - no spawn type missiles/bombs
					// do use for sure:
					// - lasers
					// - dumbfire type missiles
					// - AAA beams
					if ( !all_turret_weapons_have_flags(swp, WIF_HUGE | WIF_FLAK | WIF_HOMING | WIF_SPAWN) ) {
						// Asteroid_obj_list
						for ( ao = GET_FIRST(&Asteroid_obj_list); ao != END_OF_LIST(&Asteroid_obj_list); ao = GET_NEXT(ao) ) {
							objp = &Objects[ao->objnum];
							evaluate_obj_as_target(objp, &eeo);
						}

						if (eeo.nearest_objnum != -1) {
							return eeo.nearest_objnum;
						}
					}
					break;

				default:
					Int3(); //Means invalid number passed.
			}
		}
	}

	return -1;
}

int Use_parent_target = 0;
DCF_BOOL(use_parent_target, Use_parent_target)

// -------------------------------------------------------------------
//	Return objnum if enemy found, else return -1;
//
// input:
//				turret_subsys	=> pointer to turret subsystem
//				objnum			=> parent objnum for the turret
//				tpos				=> position of turret (world coords)
//				tvec				=> forward vector of turret (world coords)
//				current_enemy	=>	objnum of current turret target
int find_turret_enemy(ship_subsys *turret_subsys, int objnum, vec3d *tpos, vec3d *tvec, int current_enemy)
{
	int					enemy_team_mask, enemy_objnum;
	model_subsystem	*tp;
	ship_info			*sip;

	tp = turret_subsys->system_info;
	enemy_team_mask = iff_get_attackee_mask(obj_team(&Objects[objnum]));

	bool big_only_flag = all_turret_weapons_have_flags(&turret_subsys->weapons, WIF_HUGE);
	bool small_only_flag = all_turret_weapons_have_flags2(&turret_subsys->weapons, WIF2_SMALL_ONLY);
	bool tagged_only_flag = all_turret_weapons_have_flags2(&turret_subsys->weapons, WIF2_TAGGED_ONLY) || (turret_subsys->weapons.flags & SW_FLAG_TAGGED_ONLY);

	bool beam_flag = turret_weapon_has_flags(&turret_subsys->weapons, WIF_BEAM);
	bool flak_flag = turret_weapon_has_flags(&turret_subsys->weapons, WIF_FLAK);
	bool laser_flag = turret_weapon_has_subtype(&turret_subsys->weapons, WP_LASER);
	bool missile_flag = turret_weapon_has_subtype(&turret_subsys->weapons, WP_MISSILE);

	//	If a small ship and target_objnum != -1, use that as goal.
	ai_info	*aip = &Ai_info[Ships[Objects[objnum].instance].ai_index];
	sip = &Ship_info[Ships[Objects[objnum].instance].ship_info_index];

	if ((Ship_types[sip->class_type].ship_bools & STI_TURRET_TGT_SHIP_TGT) && (aip->target_objnum != -1)) {
		int target_objnum = aip->target_objnum;

		if (Objects[target_objnum].signature == aip->target_signature) {
			if (iff_matches_mask(Ships[Objects[target_objnum].instance].team, enemy_team_mask)) {
				if ( !(Objects[target_objnum].flags & OF_PROTECTED) ) {		// check this flag as well
					// nprintf(("AI", "Frame %i: Object %i resuming goal of object %i\n", AI_FrameCount, objnum, target_objnum));
					return target_objnum;
				}
			}
		} else {
			aip->target_objnum = -1;
			aip->target_signature = -1;
		}
	// Not small or small with target objnum
	} else {
		// maybe use aip->target_objnum as next target
		if ((frand() < 0.8f) && (aip->target_objnum != -1) && Use_parent_target) {

			//check if aip->target_objnum is valid target
			int target_flags = Objects[aip->target_objnum].flags;
			if ( target_flags & OF_PROTECTED ) {
				// AL 2-27-98: why is a protected ship being targeted?
				set_target_objnum(aip, -1);
				return -1;
			}

			// maybe use ship target_objnum if valid for turret
			// check for beam weapon and beam protected, etc.
			bool skip = false;
			     if ( (target_flags & OF_BEAM_PROTECTED) && beam_flag ) skip = true;
			else if ( (target_flags & OF_FLAK_PROTECTED) && flak_flag ) skip = true;
			else if ( (target_flags & OF_LASER_PROTECTED) && laser_flag ) skip = true;
			else if ( (target_flags & OF_MISSILE_PROTECTED) && missile_flag ) skip = true;

			if (!skip) {
				if ( Objects[aip->target_objnum].type == OBJ_SHIP ) {
					// check for huge weapon and huge ship
					if ( !big_only_flag || (Ship_info[Ships[Objects[aip->target_objnum].instance].ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP)) ) {
						// check for tagged only and tagged ship
						if ( tagged_only_flag && ship_is_tagged(&Objects[aip->target_objnum]) ) {
							// select new target if aip->target_objnum is out of field of view
							vec3d v2e;
							float dist;
							bool in_fov;
							dist = vm_vec_normalized_dir(&v2e, &Objects[aip->target_objnum].pos, tpos);

							in_fov = turret_fov_test(turret_subsys, tvec, &v2e);

							if (turret_subsys->flags & SSF_FOV_EDGE_CHECK) {
								if (in_fov == false)
									if (object_in_turret_fov(&Objects[aip->target_objnum], turret_subsys, tvec, tpos, dist + Objects[aip->target_objnum].radius))
										in_fov = true;
							}
							// MODIFY FOR ATTACKING BIG SHIP
							// dot += (0.5f * Objects[aip->target_objnum].radius / dist);
							if (in_fov) {
								return aip->target_objnum;
							}
						}
					}
				}
			}
		}
	}

	enemy_objnum = get_nearest_turret_objnum(objnum, turret_subsys, enemy_team_mask, tpos, tvec, current_enemy, big_only_flag, small_only_flag, tagged_only_flag, beam_flag, flak_flag, laser_flag, missile_flag);
	if ( enemy_objnum >= 0 ) {
		Assert( !((Objects[enemy_objnum].flags & OF_BEAM_PROTECTED) && beam_flag) );
		Assert( !((Objects[enemy_objnum].flags & OF_FLAK_PROTECTED) && flak_flag) );
		Assert( !((Objects[enemy_objnum].flags & OF_LASER_PROTECTED) && laser_flag) );
		Assert( !((Objects[enemy_objnum].flags & OF_MISSILE_PROTECTED) && missile_flag) );

		if ( Objects[enemy_objnum].flags & OF_PROTECTED ) {
			Int3();
			enemy_objnum = aip->target_objnum;
		}
	}

	return enemy_objnum;
}


//	Given an object and a turret on that object, return the global position and forward vector
//	of the turret.   The gun normal is the unrotated gun normal, (the center of the FOV cone), not
// the actual gun normal given using the current turret heading.  But it _is_ rotated into the model's orientation
//	in global space.
void ship_get_global_turret_info(object *objp, model_subsystem *tp, vec3d *gpos, vec3d *gvec)
{
//	vm_vec_unrotate(gpos, &tp->turret_avg_firing_point, &objp->orient);
	vm_vec_unrotate(gpos, &tp->pnt, &objp->orient);
	vm_vec_add2(gpos, &objp->pos);
	vm_vec_unrotate(gvec, &tp->turret_norm, &objp->orient);	
}

// Given an object and a turret on that object, return the actual firing point of the gun
// and its normal.   This uses the current turret angles.  We are keeping track of which
// gun to fire next in the ship specific info for this turret subobject.  Use this info
// to determine which position to fire from next.
//	Stuffs:
//		*gpos: absolute position of gun firing point
//		*gvec: vector fro *gpos to *targetp
void ship_get_global_turret_gun_info(object *objp, ship_subsys *ssp, vec3d *gpos, vec3d *gvec, int use_angles, vec3d *targetp)
{
	vec3d * gun_pos;
	model_subsystem *tp = ssp->system_info;

	//ship_model_start(objp);

	gun_pos = &tp->turret_firing_point[ssp->turret_next_fire_pos % tp->turret_num_firing_points];

	//model_find_world_point(gpos, gun_pos, tp->model_num, tp->turret_gun_sobj, &objp->orient, &objp->pos );
	model_instance_find_world_point(gpos, gun_pos, tp->model_num, Ships[objp->instance].model_instance_num, tp->turret_gun_sobj, &objp->orient, &objp->pos);

	if (use_angles)
		model_instance_find_world_dir(gvec, &tp->turret_norm, tp->model_num, Ships[objp->instance].model_instance_num, tp->turret_gun_sobj, &objp->orient, &objp->pos );
	else {
		//vector	gun_pos2;
		//vm_vec_add(&gun_pos2, gpos, gun_pos);
		vm_vec_normalized_dir(gvec, targetp, gpos);
	}

	//ship_model_stop(objp);

	// per firingpoint based changes go here for turrets
}

//Update turret aiming data based on max turret aim update delay
void turret_ai_update_aim(ai_info *aip, object *En_Objp, ship_subsys *ss)
{
	if (Missiontime >= ss->next_aim_pos_time)
	{
		ss->last_aim_enemy_pos = En_Objp->pos;
		ss->last_aim_enemy_vel = En_Objp->phys_info.vel;
		ss->next_aim_pos_time = Missiontime + fl2f(frand_range(0.0f, aip->ai_turret_max_aim_update_delay));
	}
	else
	{
		//Update the position based on the velocity (assume no velocity vector change)
		vm_vec_scale_add2(&ss->last_aim_enemy_pos, &ss->last_aim_enemy_vel, flFrametime);
	}
}


//	Rotate a turret towards an enemy.
//	Return TRUE if caller should use angles in subsequent rotations.
//	Some obscure model thing only John Slagel knows about.
//	Sets predicted enemy position.
//	If the turret (*ss) has a subsystem targeted, the subsystem is used as the predicted point.
int aifft_rotate_turret(ship *shipp, int parent_objnum, ship_subsys *ss, object *objp, object *lep, vec3d *predicted_enemy_pos, vec3d *gvec)
{
	int ret_val = 0;

	if (ss->turret_enemy_objnum != -1) {
		model_subsystem *tp = ss->system_info;
		vec3d	gun_pos, gun_vec, target_moving_direction;
		//float		weapon_speed;
		float		weapon_system_strength;
		//HACK HACK HACK -WMC
		//This should use the best weapon variable
		//It doesn't, because I haven't implemented code to set it yet -WMC
		int best_weapon_tidx = turret_select_best_weapon(ss, lep);

		//This turret doesn't have any good weapons
		if (best_weapon_tidx < 0)
			return 0;

		weapon_info *wip = get_turret_weapon_wip(&ss->weapons, best_weapon_tidx);

		//	weapon_system_strength scales time enemy in range in 0..1.  So, the lower this is, the worse the aiming will be.
		weapon_system_strength = ship_get_subsystem_strength(shipp, SUBSYSTEM_WEAPONS);

		ship_get_global_turret_info(&Objects[parent_objnum], tp, &gun_pos, &gun_vec);

		//Update "known" position and velocity of target. Only matters if max_aim_update_delay is set.
		turret_ai_update_aim(&Ai_info[shipp->ai_index], &Objects[ss->turret_enemy_objnum], ss);

		//Figure out what point on the ship we want to point the gun at, and store the global location
		//in enemy_point.
		vec3d	enemy_point;
		if ((ss->targeted_subsys != NULL) && !(ss->flags & SSF_NO_SS_TARGETING)) {
			if (ss->turret_enemy_objnum != -1) {
				vm_vec_unrotate(&enemy_point, &ss->targeted_subsys->system_info->pnt, &Objects[ss->turret_enemy_objnum].orient);
				vm_vec_add2(&enemy_point, &ss->last_aim_enemy_pos);
			}
		} else {
			if ((lep->type == OBJ_SHIP) && (Ship_info[Ships[lep->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))) {
				ai_big_pick_attack_point_turret(lep, ss, &gun_pos, &gun_vec, &enemy_point, tp->turret_fov, MIN(wip->max_speed * wip->lifetime, wip->weapon_range));
			} else {
				enemy_point = ss->last_aim_enemy_pos;
			}
		}

		target_moving_direction = ss->last_aim_enemy_vel;

		//Try to guess where the enemy will be, and store that spot in predicted_enemy_pos
		if (The_mission.ai_profile->flags & AIPF_USE_ADDITIVE_WEAPON_VELOCITY) {
			vm_vec_sub2(&target_moving_direction, &objp->phys_info.vel);
		}

		set_predicted_enemy_pos_turret(predicted_enemy_pos, &gun_pos, objp, &enemy_point, &target_moving_direction, wip->max_speed, ss->turret_time_enemy_in_range * (weapon_system_strength + 1.0f)/2.0f);

		//Mess with the turret's accuracy if the weapon system is damaged.
		if (weapon_system_strength < 0.7f) {
			vec3d	rand_vec;

			static_randvec(Missiontime >> 18, &rand_vec);	//	Return same random number for two seconds.
			//	Add to predicted_enemy_pos value in .45 to 1.5x radius of enemy ship, so will often miss, but not by a huge amount.
			vm_vec_scale_add2(predicted_enemy_pos, &rand_vec, (1.0f - weapon_system_strength)*1.5f * lep->radius);
		}

		//Get the normalized dir between the turret and the predicted enemy position.
		//If the dot product is smaller than or equal to the turret's FOV, try and point the gun at it.
		vec3d	v2e;
		vm_vec_normalized_dir(&v2e, predicted_enemy_pos, &gun_pos);

		bool in_fov;

		in_fov = turret_fov_test(ss, gvec, &v2e);

		if (ss->flags & SSF_FOV_EDGE_CHECK) {
			if (in_fov == false)
				in_fov = is_object_radius_in_turret_fov(&Objects[ss->turret_enemy_objnum], ss, gvec, &gun_pos, &v2e, predicted_enemy_pos, 0.0f);
		}

		if (in_fov) {
			ret_val = model_rotate_gun(Ship_info[shipp->ship_info_index].model_num,
										tp, &Objects[parent_objnum].orient, 
										&ss->submodel_info_1.angs, &ss->submodel_info_2.angs,
										&Objects[parent_objnum].pos, predicted_enemy_pos, shipp->objnum);
		} else if ((tp->flags & MSS_FLAG_TURRET_RESET_IDLE) &&(timestamp_elapsed(ss->rotation_timestamp))) {
			ret_val = model_rotate_gun(Ship_info[shipp->ship_info_index].model_num, tp, &Objects[parent_objnum].orient, &ss->submodel_info_1.angs, &ss->submodel_info_2.angs, &Objects[parent_objnum].pos, predicted_enemy_pos, shipp->objnum, true);
		}
	} else if ((ss->system_info->flags & MSS_FLAG_TURRET_RESET_IDLE) && (timestamp_elapsed(ss->rotation_timestamp))) {
		ret_val = model_rotate_gun(Ship_info[shipp->ship_info_index].model_num, ss->system_info, &Objects[parent_objnum].orient, &ss->submodel_info_1.angs, &ss->submodel_info_2.angs, &Objects[parent_objnum].pos, predicted_enemy_pos, shipp->objnum, true);
	}

	// by default "ret_val" should be set to 1 for multi-part turrets, and 0 for single-part turrets
	// but we need to keep retail behavior by default, which means always returning 0 unless a special
	// flag is used.  the "ret_val" stuff is here is needed/wanted at a later date however.
	//return ret_val;

	// return 0 by default (to preserve retail behavior) but allow for a per-subsystem option
	// for using the turret normals for firing
	if (ss->system_info->flags & MSS_FLAG_FIRE_ON_NORMAL)
		return 1;

	return 0;
}

//	Determine if subsystem *enemy_subsysp is hittable from objp.
//	If so, return dot product of vector from point abs_gunposp to *enemy_subsysp
float	aifft_compute_turret_dot(object *objp, object *enemy_objp, vec3d *abs_gunposp, ship_subsys *turret_subsysp, ship_subsys *enemy_subsysp)
{
	float	dot_out;
	vec3d	subobj_pos, vector_out;

	vm_vec_unrotate(&subobj_pos, &enemy_subsysp->system_info->pnt, &enemy_objp->orient);
	vm_vec_add2(&subobj_pos, &enemy_objp->pos);

	if (ship_subsystem_in_sight(enemy_objp, enemy_subsysp, abs_gunposp, &subobj_pos, 1, &dot_out, &vector_out)) {
		vec3d	turret_norm;

		vm_vec_unrotate(&turret_norm, &turret_subsysp->system_info->turret_norm, &objp->orient);
		float dot_return = vm_vec_dot(&turret_norm, &vector_out);

		if (Ai_info[Ships[objp->instance].ai_index].ai_profile_flags & AIPF_SMART_SUBSYSTEM_TARGETING_FOR_TURRETS) {
			if (dot_return > turret_subsysp->system_info->turret_fov) {
				// target is in sight and in fov
				return dot_return;
			} else {
				// target is in sight but is not in turret's fov
				return -1.0f;
			}
		} else {
			// target is in sight and we don't care if its in turret's fov or not
			return dot_return;
		}
	} else
		return -1.0f;

}

// NOTE:  Do not change this value unless you understand exactly what it means and what it does.
//        It refers to how many (non-destroyed) subsystems (and turrets) will be scanned for possible
//        targetting, per turret, per frame.  A higher value will process more systems at once,
//        but it will be much slower to scan though them.  It is not necessary to scan all
//        non-destroyed subsystem each frame for each turret.  Also, "aifft_max_checks" is balanced
//        against the original value, be sure to account for this discrepancy with any changes.
#define MAX_AIFFT_TURRETS			60

ship_subsys *aifft_list[MAX_AIFFT_TURRETS];
float aifft_rank[MAX_AIFFT_TURRETS];
int aifft_list_size = 0;
int aifft_max_checks = 5;
DCF(mf, "")
{
	dc_get_arg(ARG_INT);
	aifft_max_checks = Dc_arg_int;
}


//	Pick a subsystem to attack on enemy_objp.
//	Only pick one if enemy_objp is a big ship or a capital ship.
//	Returns dot product from turret to subsystem in *dot_out
ship_subsys *aifft_find_turret_subsys(object *objp, ship_subsys *ssp, object *enemy_objp, float *dot_out)
{
	ship	*eshipp, *shipp;
	ship_info	*esip;
	ship_subsys	*best_subsysp = NULL;
	float dot;

	Assert(enemy_objp->type == OBJ_SHIP);

	eshipp = &Ships[enemy_objp->instance];
	esip = &Ship_info[eshipp->ship_info_index];

	shipp = &Ships[objp->instance];

	float	best_dot = 0.0f;
	*dot_out = best_dot;

	//	Compute absolute gun position.
	vec3d	abs_gun_pos;
	vm_vec_unrotate(&abs_gun_pos, &ssp->system_info->pnt, &objp->orient);
	vm_vec_add2(&abs_gun_pos, &objp->pos);

	//	Only pick a turret to attack on large ships.
	if (!(esip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)))
		return best_subsysp;

	// Make sure big or huge ship *actually* has subsystems  (ie, knossos)
	if (esip->n_subsystems == 0) {
		return best_subsysp;
	}

	// first build up a list subsystems to traverse
	ship_subsys	*pss;
	aifft_list_size = 0;
	for ( pss = GET_FIRST(&eshipp->subsys_list); pss !=END_OF_LIST(&eshipp->subsys_list); pss = GET_NEXT(pss) ) {
		model_subsystem *psub = pss->system_info;

		// if we've reached max turrets bail
		if(aifft_list_size >= MAX_AIFFT_TURRETS){
			break;
		}

		// Don't process destroyed objects
		if ( pss->current_hits <= 0.0f ){
			continue;
		}
		
		switch (psub->type) {
		case SUBSYSTEM_WEAPONS:
			aifft_list[aifft_list_size] = pss;
			aifft_rank[aifft_list_size++] = 1.4f;
			break;

		case SUBSYSTEM_TURRET:
			aifft_list[aifft_list_size] = pss;
			aifft_rank[aifft_list_size++] = 1.2f;
			break;

		case SUBSYSTEM_SENSORS:
		case SUBSYSTEM_ENGINE:
			aifft_list[aifft_list_size] = pss;
			aifft_rank[aifft_list_size++] = 1.0f;
			break;
		}
	}

	// DKA:  6/28/99 all subsystems can be destroyed.
	//Assert(aifft_list_size > 0);
	if (aifft_list_size == 0) {
		return best_subsysp;
	}

	// determine a stride value so we're not checking too many turrets
	int stride = aifft_list_size > aifft_max_checks ? aifft_list_size / aifft_max_checks : 1;
	if(stride <= 0){
		stride = 1;
	}
	int offset = (int)frand_range(0.0f, (float)(aifft_list_size % stride));
	int idx;
	float dot_fov_modifier = 0.0f;

	if (Ai_info[shipp->ai_index].ai_profile_flags & AIPF_SMART_SUBSYSTEM_TARGETING_FOR_TURRETS) {
		if (ssp->system_info->turret_fov < 0)
			dot_fov_modifier = ssp->system_info->turret_fov;
	}

	for(idx=offset; idx<aifft_list_size; idx+=stride){
		dot = aifft_compute_turret_dot(objp, enemy_objp, &abs_gun_pos, ssp, aifft_list[idx]);			

		if ((dot - dot_fov_modifier)* aifft_rank[idx] > best_dot) {
			best_dot = (dot - dot_fov_modifier)*aifft_rank[idx];
			best_subsysp = aifft_list[idx];
		}
	}

	Assert(best_subsysp != &eshipp->subsys_list);

	*dot_out = best_dot;
	return best_subsysp;
}

// return !0 if the specified target should scan for a new target, otherwise return 0
int turret_should_pick_new_target(ship_subsys *turret)
{
//	int target_type;

	if ( timestamp_elapsed(turret->turret_next_enemy_check_stamp) ) {
		return 1;
	}

/*
	if ( turret->turret_enemy_objnum == -1 ) {
		return 1;
	}
	
	target_type = Objects[turret->turret_enemy_objnum].type;
	if ( (target_type != OBJ_SHIP) && (target_type != OBJ_ASTEROID) ) {
		return 1;
	}
*/
	return 0;

}

// Set the next fire timestamp for a turret, based on weapon type and ai class
void turret_set_next_fire_timestamp(int weapon_num, weapon_info *wip, ship_subsys *turret, ai_info *aip)
{
	Assert(weapon_num < MAX_SHIP_WEAPONS);
	float wait = 1000.0f;

	if (wip->burst_shots > turret->weapons.burst_counter[weapon_num]) {
		wait *= wip->burst_delay;
		turret->weapons.burst_counter[weapon_num]++;
	} else {
		wait *= wip->fire_wait;
		if ((wip->burst_shots > 0) && (wip->burst_flags & WBF_RANDOM_LENGTH)) {
			turret->weapons.burst_counter[weapon_num] = (myrand() % wip->burst_shots);
		} else {
			turret->weapons.burst_counter[weapon_num] = 0;
		}
	}

	int *fs_dest;
	if(weapon_num < MAX_SHIP_PRIMARY_BANKS)
		fs_dest = &turret->weapons.next_primary_fire_stamp[weapon_num];
	else
		fs_dest = &turret->weapons.next_secondary_fire_stamp[weapon_num - MAX_SHIP_PRIMARY_BANKS];

	//Check for the new cooldown flag
	if(!((wip->wi_flags2 & WIF2_SAME_TURRET_COOLDOWN) || ((wip->burst_shots > 0) && (wip->burst_flags & WBF_FAST_FIRING))))
	{

		// make side even for team vs. team
		if (MULTI_TEAM) {
			// flak guns need to fire more rapidly
			if (wip->wi_flags & WIF_FLAK) {
				wait *= aip->ai_ship_fire_delay_scale_friendly * 0.5f;
				if (aip->ai_class_autoscale)
					wait += (Num_ai_classes - aip->ai_class - 1) * 40.0f;
			} else {
				wait *= aip->ai_ship_fire_delay_scale_friendly;
				if (aip->ai_class_autoscale)
					wait += (Num_ai_classes - aip->ai_class - 1) * 100.0f;
			}
		} else {
			// flak guns need to fire more rapidly
			if (wip->wi_flags & WIF_FLAK) {
				if (Ships[aip->shipnum].team == Player_ship->team) {
					wait *= aip->ai_ship_fire_delay_scale_friendly * 0.5f;
				} else {
					wait *= aip->ai_ship_fire_delay_scale_hostile * 0.5f;
				}	
				if (aip->ai_class_autoscale)
					wait += (Num_ai_classes - aip->ai_class - 1) * 40.0f;

			} else if (wip->wi_flags & WIF_HUGE) {
				// make huge weapons fire independently of team
				wait *= aip->ai_ship_fire_delay_scale_friendly;
				if (aip->ai_class_autoscale)
					wait += (Num_ai_classes - aip->ai_class - 1) * 100.0f;
			} else {
				// give team friendly an advantage
				if (Ships[aip->shipnum].team == Player_ship->team) {
					wait *= aip->ai_ship_fire_delay_scale_friendly;
				} else {
					wait *= aip->ai_ship_fire_delay_scale_hostile;
				}	
				if (aip->ai_class_autoscale)
					wait += (Num_ai_classes - aip->ai_class - 1) * 100.0f;
			}
		}
		// vary wait time +/- 10%
		wait *= frand_range(0.9f, 1.1f);
	}

	if(turret->rof_scaler != 1.0f)
		wait /= get_adjusted_turret_rof(turret);

	(*fs_dest) = timestamp((int)wait);
}

// Decide  if a turret should launch an aspect seeking missile
int turret_should_fire_aspect(ship_subsys *turret, weapon_info *wip, bool in_sight)
{
	if ( in_sight && (turret->turret_time_enemy_in_range >= MIN(wip->min_lock_time,AICODE_TURRET_MAX_TIME_IN_RANGE)) ) {
		return 1;
	}

	return 0;
}

// Update how long current target has been in this turrets range
void turret_update_enemy_in_range(ship_subsys *turret, float seconds)
{
	turret->turret_time_enemy_in_range += seconds;

	if ( turret->turret_time_enemy_in_range < 0.0f ) {
		turret->turret_time_enemy_in_range = 0.0f;
	}

	if ( turret->turret_time_enemy_in_range > AICODE_TURRET_MAX_TIME_IN_RANGE ) {
		turret->turret_time_enemy_in_range = AICODE_TURRET_MAX_TIME_IN_RANGE;
	}
}

// Fire a weapon from a turret
bool turret_fire_weapon(int weapon_num, ship_subsys *turret, int parent_objnum, vec3d *turret_pos, vec3d *turret_fvec, vec3d *predicted_pos = NULL, float flak_range_override = 100.0f)
{
	matrix	turret_orient;
	int weapon_objnum;
	ai_info	*parent_aip;
	ship		*parent_ship;
	float flak_range = 0.0f;
	weapon_info *wip;
	weapon *wp;
	object *objp;
	bool last_shot_in_salvo = true;

	if (turret->system_info->flags & MSS_FLAG_TURRET_SALVO)
	{
		if ((turret->turret_next_fire_pos + 1) == (turret->system_info->turret_num_firing_points))
		{
			last_shot_in_salvo = true;
		}
		else
		{
			last_shot_in_salvo = false;
		}
	}

	//WMC - Limit firing to firestamp
	if((!timestamp_elapsed(get_turret_weapon_next_fire_stamp(&turret->weapons, weapon_num))) && last_shot_in_salvo)
		return false;

	parent_aip = &Ai_info[Ships[Objects[parent_objnum].instance].ai_index];
	parent_ship = &Ships[Objects[parent_objnum].instance];
	wip = get_turret_weapon_wip(&turret->weapons, weapon_num);
	int turret_weapon_class = WEAPON_INFO_INDEX(wip);

#ifndef NDEBUG
	// moved here from check_ok_to_fire
	if (turret->turret_enemy_objnum >= 0) {
		object	*tobjp = &Objects[turret->turret_enemy_objnum];

		// should not get this far. check if ship is protected from beam and weapon is type beam
		if ( (wip->wi_flags & WIF_BEAM) && (tobjp->flags & OF_BEAM_PROTECTED) ) {
			Int3();
			return 0;
		}
		// should not get this far. check if ship is protected from flak and weapon is type flak
		else if ( (wip->wi_flags & WIF_FLAK) && (tobjp->flags & OF_FLAK_PROTECTED) ) {
			Int3();
			return 0;
		}
		// should not get this far. check if ship is protected from laser and weapon is type laser
		else if ( (wip->subtype == WP_LASER) && (tobjp->flags & OF_LASER_PROTECTED) ) {
			Int3();
			return 0;
		}
		// should not get this far. check if ship is protected from missile and weapon is type missile
		else if ( (wip->subtype == WP_MISSILE) && (tobjp->flags & OF_MISSILE_PROTECTED) ) {
			Int3();
			return 0;
		}
	}
#endif

	if (check_ok_to_fire(parent_objnum, turret->turret_enemy_objnum, wip)) {
		vm_vector_2_matrix(&turret_orient, turret_fvec, NULL, NULL);
		turret->turret_last_fire_direction = *turret_fvec;

		// set next fire timestamp for the turret
		if (last_shot_in_salvo)
			turret_set_next_fire_timestamp(weapon_num, wip, turret, parent_aip);

		// if this weapon is a beam weapon, handle it specially
		if (wip->wi_flags & WIF_BEAM) {
			// if this beam isn't free to fire
			if (!(turret->weapons.flags & SW_FLAG_BEAM_FREE)) {
				return false;
			}
			beam_fire_info fire_info;

			// stuff beam firing info
			memset(&fire_info, 0, sizeof(beam_fire_info));
			fire_info.accuracy = 1.0f;
			fire_info.beam_info_index = turret_weapon_class;
			fire_info.beam_info_override = NULL;
			fire_info.shooter = &Objects[parent_objnum];
			fire_info.target = &Objects[turret->turret_enemy_objnum];
			fire_info.target_subsys = NULL;
			fire_info.turret = turret;

			// fire a beam weapon
			beam_fire(&fire_info);

			turret->flags |= SSF_HAS_FIRED; //set fired flag for scripting -nike
			return true;
		}
		// don't fire swam, but set up swarm info instead
		else if ((wip->wi_flags & WIF_SWARM) || (wip->wi_flags & WIF_CORKSCREW)) {
			turret_swarm_set_up_info(parent_objnum, turret, wip, turret->turret_next_fire_pos);

			turret->flags |= SSF_HAS_FIRED;	//set fired flag for scripting -nike
			return true;
		}
		// now do anything else
		else {
			for (int i=0; i < wip->shots; i++)
			{
				// zookeeper - Firepoints should cycle normally between shots, 
				// so we need to get the position info separately for each shot
				ship_get_global_turret_gun_info(&Objects[parent_objnum], turret, turret_pos, turret_fvec, 1, NULL);

				weapon_objnum = weapon_create( turret_pos, &turret_orient, turret_weapon_class, parent_objnum, -1, 1);
				weapon_set_tracking_info(weapon_objnum, parent_objnum, turret->turret_enemy_objnum, 1, turret->targeted_subsys);		
			

				objp=&Objects[weapon_objnum];
				wp=&Weapons[objp->instance];

				parent_ship->last_fired_turret = turret;
				turret->last_fired_weapon_info_index = wp->weapon_info_index;

				Script_system.SetHookObjects(3, "Ship", &Objects[parent_objnum], "Weapon", objp, "Target", &Objects[turret->turret_enemy_objnum]);
				Script_system.RunCondition(CHA_ONTURRETFIRED, 0, NULL, &Objects[parent_objnum]);

				//nprintf(("AI", "Turret_time_enemy_in_range = %7.3f\n", ss->turret_time_enemy_in_range));		
				if (weapon_objnum != -1) {
					wp->target_num = turret->turret_enemy_objnum;
					// AL 1-6-97: Store pointer to turret subsystem
					wp->turret_subsys = turret;	

					// if the gun is a flak gun
					if(wip->wi_flags & WIF_FLAK){			
						// show a muzzle flash
						flak_muzzle_flash(turret_pos, turret_fvec, &Objects[parent_ship->objnum].phys_info, turret_weapon_class);

						if(predicted_pos != NULL)
						{
							// pick a firing range so that it detonates properly			
							flak_pick_range(objp, turret_pos, predicted_pos, ship_get_subsystem_strength(parent_ship, SUBSYSTEM_WEAPONS));

							// determine what that range was
							flak_range = flak_get_range(objp);
						}
						else
						{
							flak_set_range(objp, flak_range_override);
						}
					} else if(wip->muzzle_flash > -1) {	
						mflash_create(turret_pos, turret_fvec, &Objects[parent_ship->objnum].phys_info, Weapon_info[turret_weapon_class].muzzle_flash);		
					}

					// in multiplayer (and the master), then send a turret fired packet.
					if ( MULTIPLAYER_MASTER && (weapon_objnum != -1) ) {
						int subsys_index;

						subsys_index = ship_get_index_from_subsys(turret, parent_objnum );
						Assert( subsys_index != -1 );
						if(wip->wi_flags & WIF_FLAK){			
							send_flak_fired_packet( parent_objnum, subsys_index, weapon_objnum, flak_range );
						} else {
							send_turret_fired_packet( parent_objnum, subsys_index, weapon_objnum );
						}
					}

					if ( wip->launch_snd != -1 ) {
						// Don't play turret firing sound if turret sits on player ship... it gets annoying.
						if ( parent_objnum != OBJ_INDEX(Player_obj) || (turret->flags & SSF_PLAY_SOUND_FOR_PLAYER) ) {						
							snd_play_3d( &Snds[wip->launch_snd], turret_pos, &View_position );						
						}
					}
				}
				turret->turret_next_fire_pos++;
			}
			turret->turret_next_fire_pos--;
			// reset any animations if we need to
			// (I'm not sure how accurate this timestamp would be in practice - taylor)
			if (turret->turret_animation_position == MA_POS_READY)
				turret->turret_animation_done_time = timestamp(100);
		}
	}
	//Not useful -WMC
	else if (!(parent_aip->ai_profile_flags & AIPF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY) && last_shot_in_salvo)
	{
		float wait = 1000.0f * frand_range(0.9f, 1.1f);
		turret->turret_next_fire_stamp = timestamp((int) wait);
	}

	turret->flags |= SSF_HAS_FIRED; //set has fired flag for scriptng - nuke
	return true;
}

//void turret_swarm_fire_from_turret(ship_subsys *turret, int parent_objnum, int target_objnum, ship_subsys *target_subsys)
void turret_swarm_fire_from_turret(turret_swarm_info *tsi)
{
	int weapon_objnum;
	matrix turret_orient;
	vec3d turret_pos, turret_fvec;

	// parent not alive, quick out.
	if (Objects[tsi->parent_objnum].type != OBJ_SHIP) {
		return;
	}
	
	//	if fixed fp make sure to use constant fp
	if (tsi->turret->system_info->flags & MSS_FLAG_TURRET_FIXED_FP)
		tsi->turret->turret_next_fire_pos = tsi->weapon_num;

	//	change firing point
	ship_get_global_turret_gun_info(&Objects[tsi->parent_objnum], tsi->turret, &turret_pos, &turret_fvec, 1, NULL);
	tsi->turret->turret_next_fire_pos++;

	//check if this really is a swarm. If not, how the hell did it get here?
	Assert((Weapon_info[tsi->weapon_class].wi_flags & WIF_SWARM) || (Weapon_info[tsi->weapon_class].wi_flags & WIF_CORKSCREW));


    // *If it's a non-homer, then use the last fire direction instead of turret orientation to fix inaccuracy
    //  problems with non-homing swarm weapons -Et1
	if ( (Weapon_info[tsi->weapon_class].subtype == WP_LASER) || ((The_mission.ai_profile->flags & AIPF_HACK_IMPROVE_NON_HOMING_SWARM_TURRET_FIRE_ACCURACY) 
																	&& !(Weapon_info[tsi->weapon_class].wi_flags & WIF_HOMING)) )
	{
		turret_fvec = tsi->turret->turret_last_fire_direction;
	}

	// make turret_orient from turret_fvec -- turret->turret_last_fire_direction
	vm_vector_2_matrix(&turret_orient, &turret_fvec, NULL, NULL);

	// create weapon and homing info
	weapon_objnum = weapon_create(&turret_pos, &turret_orient, tsi->weapon_class, tsi->parent_objnum, -1, 1);
	weapon_set_tracking_info(weapon_objnum, tsi->parent_objnum, tsi->target_objnum, 1, tsi->target_subsys);

	// do other cool stuff if weapon is created.
	if (weapon_objnum > -1) {
		Weapons[Objects[weapon_objnum].instance].turret_subsys = tsi->turret;
		Weapons[Objects[weapon_objnum].instance].target_num = tsi->turret->turret_enemy_objnum;

		// maybe sound
		if ( Weapon_info[tsi->weapon_class].launch_snd != -1 ) {
			// Don't play turret firing sound if turret sits on player ship... it gets annoying.
			if ( tsi->parent_objnum != OBJ_INDEX(Player_obj) ) {
				snd_play_3d( &Snds[Weapon_info[tsi->weapon_class].launch_snd], &turret_pos, &View_position );
			}
		}
		if(Weapon_info[tsi->weapon_class].muzzle_flash > -1)
			mflash_create(&turret_pos, &turret_fvec, &Objects[tsi->parent_objnum].phys_info, Weapon_info[tsi->weapon_class].muzzle_flash);
		// in multiplayer (and the master), then send a turret fired packet.
		if ( MULTIPLAYER_MASTER && (weapon_objnum != -1) ) {
			int subsys_index;

			subsys_index = ship_get_index_from_subsys(tsi->turret, tsi->parent_objnum );
			Assert( subsys_index != -1 );
			send_turret_fired_packet( tsi->parent_objnum, subsys_index, weapon_objnum );
		}
	}
}

int Num_ai_firing = 0;
int Num_find_turret_enemy = 0;
int Num_turrets_fired = 0;
//	Given a turret tp and its parent parent_objnum, fire from the turret at its enemy.
extern int Nebula_sec_range;
void ai_fire_from_turret(ship *shipp, ship_subsys *ss, int parent_objnum)
{
	float		weapon_firing_range;

    // *Weapon minimum firing range -Et1
    float WeaponMinRange;

	vec3d	v2e;
	object	*lep;		//	Last enemy pointer
	model_subsystem	*tp = ss->system_info;
	ship_weapon *swp = &ss->weapons;
	vec3d	predicted_enemy_pos = vmd_zero_vector;
	object	*objp;
	//ai_info	*aip;

	// Reset the points to target value
	ss->points_to_target = -1.0f;
	ss->base_rotation_rate_pct = 0.0f;
	ss->gun_rotation_rate_pct = 0.0f;

	if (!Ai_firing_enabled) {
		return;
	}

	if (ss->current_hits <= 0.0f) {
		return;
	}

	if ( ship_subsys_disrupted(ss) ){		// AL 1/19/98: Make sure turret isn't suffering disruption effects
		return;
	}

	// Check turret free
	if (ss->weapons.flags & SW_FLAG_TURRET_LOCK) {
		return;
	}

	// check if there is any available weapon to fire
	int weap_check;
	bool valid_weap = false;
	for (weap_check = 0; (weap_check < swp->num_primary_banks) && !valid_weap; weap_check++) {
		if (swp->primary_bank_weapons[weap_check] >= 0)
			valid_weap = true;
	}

	if (!valid_weap) {
		for (weap_check = 0; (weap_check < ss->weapons.num_secondary_banks) && !valid_weap; weap_check++) {
			if (ss->weapons.secondary_bank_weapons[weap_check] >= 0)
				valid_weap = true;
		}

		if (!valid_weap)
			return;
	}

	// If beam weapon, check beam free
	if ( all_turret_weapons_have_flags(swp, WIF_BEAM) && !(swp->flags & SW_FLAG_BEAM_FREE) ) {
		return;
	}

	if (ss->turret_animation_position == MA_POS_SET) {
		if ( timestamp_elapsed(ss->turret_animation_done_time) ) {
			ss->turret_animation_position = MA_POS_READY;
			// setup a reversal (closing) timestamp at 1 second
			// (NOTE: this will probably get changed by other parts of the code (swarming, beams, etc) to be
			// a more accurate time, but we need to give it a long enough time for the other parts of the code
			// to change the timestamp before it gets acted upon - taylor)
			ss->turret_animation_done_time = timestamp(1000);
		} else {
			return;
		}
	}

	Assert((parent_objnum >= 0) && (parent_objnum < MAX_OBJECTS));
	objp = &Objects[parent_objnum];
	Assert(objp->type == OBJ_SHIP);
	Assert( shipp->objnum == parent_objnum );

	// Monitor number of calls to ai_fire_from_turret
	Num_ai_firing++;

	if ( (ss->turret_enemy_objnum < 0 || ss->turret_enemy_objnum >= MAX_OBJECTS) || (ss->turret_enemy_sig != Objects[ss->turret_enemy_objnum].signature))
	{
		ss->turret_enemy_objnum = -1;
		lep = NULL;
	}
	else
	{
		lep = &Objects[ss->turret_enemy_objnum];
	}

	
	//aip = &Ai_info[Ships[objp->instance].ai_index];
	// Wanderer - make sure turrets already have all the data
	if ( !(tp->flags & MSS_FLAG_TURRET_MATRIX) )
	{
		if (!(tp->turret_gun_sobj == tp->subobj_num))
		{
			model_make_turret_matrix(Ship_info[shipp->ship_info_index].model_num, tp );
		}
	}

	// Use the turret info for all guns, not one gun in particular.
	vec3d	 gvec, gpos;
	ship_get_global_turret_info(&Objects[parent_objnum], tp, &gpos, &gvec);

	if (tp->flags & MSS_FLAG_TURRET_ALT_MATH) {
		vm_matrix_x_matrix( &ss->world_to_turret_matrix, &Objects[parent_objnum].orient, &tp->turret_matrix );
	}

	// Rotate the turret even if time hasn't elapsed, since it needs to turn to face its target.
	int use_angles = aifft_rotate_turret(shipp, parent_objnum, ss, objp, lep, &predicted_enemy_pos, &gvec);

	if ((tp->flags & MSS_FLAG_FIRE_ON_TARGET) && (ss->points_to_target >= 0.0f))
	{
		// value probably needs tweaking... could perhaps be made into table option?
		if (ss->points_to_target > 0.010f)
			return;
	}

	if ( !timestamp_elapsed(ss->turret_next_fire_stamp) ) {
		return;
	}

	// Don't try to fire beyond weapon_limit_range
	//WMC - OTC
	//weapon_firing_range = MIN(Weapon_info[tp->turret_weapon_type].lifetime * Weapon_info[tp->turret_weapon_type].max_speed, Weapon_info[tp->turret_weapon_type].weapon_range);

	//WMC - build a list of valid weapons. Fire spawns if there are any.
	float dist_to_enemy = 0.0f;
	if(lep != NULL) {
		if (The_mission.ai_profile->flags2 & AIPF2_TURRETS_IGNORE_TARGET_RADIUS)
			dist_to_enemy = MAX(0,vm_vec_normalized_dir(&v2e, &predicted_enemy_pos, &gpos));
		else
			dist_to_enemy = MAX(0,vm_vec_normalized_dir(&v2e, &predicted_enemy_pos, &gpos) - lep->radius);
	}

	int valid_weapons[MAX_SHIP_PRIMARY_BANKS + MAX_SHIP_SECONDARY_BANKS];
	int num_valid = 0;

	weapon_info *wip;
	int num_ships_nearby = num_nearby_fighters(iff_get_attackee_mask(obj_team(&Objects[parent_objnum])), &gpos, 1500.0f);
	int i;
	int secnum = 0;
	for(i = 0; i < (MAX_SHIP_WEAPONS); i++)
	{
		//WMC - Only fire more than once if we have multiple guns flag set.
		if(num_valid > 0 && !(tp->flags & MSS_FLAG_USE_MULTIPLE_GUNS))
			break;

		if(i < MAX_SHIP_PRIMARY_BANKS)
		{
			if(i >= swp->num_primary_banks)
			{
				//set to MAX_SHIP_PRIMARY_BANKS - 1 since i is incremented at the top of the loop
				//otherwise, we would pass over the first secondary weapon in the bank
				i = MAX_SHIP_PRIMARY_BANKS - 1;	//we are done with primaries
				continue;
			}
			if ( !timestamp_elapsed(swp->next_primary_fire_stamp[i]))
			{
					continue;
			}
			wip = &Weapon_info[swp->primary_bank_weapons[i]];
		}
		else
		{
			secnum = i - MAX_SHIP_PRIMARY_BANKS;
			if(secnum >= swp->num_secondary_banks)
				break;	//we are done.
			if ( !timestamp_elapsed(swp->next_secondary_fire_stamp[secnum]))
			{
				continue;
			}
			wip = &Weapon_info[swp->secondary_bank_weapons[secnum]];
		}

		//If this is a spawning type weapon, shoot it!
		//If 'smart spawn' just use it like normal weapons
		if ( (wip->wi_flags & WIF_SPAWN) && !(wip->wi_flags2 & WIF2_SMART_SPAWN) )
		{
			if (( num_ships_nearby >= 3 ) || ((num_ships_nearby >= 2) && (frand() < 0.1f))) {
				turret_fire_weapon(i, ss, parent_objnum, &gpos, &ss->turret_last_fire_direction);
			} else {
				ss->turret_next_fire_stamp = timestamp(1000);	//	Regardless of firing rate, don't check whether should fire for awhile.
			}

			//we're done with this weapon mount
			continue;
		}


		if(wip->wi_flags2 & WIF2_LOCAL_SSM)
		{
			weapon_firing_range=wip->lssm_lock_range;
			WeaponMinRange = 0.0f;
		}
		else
		{
			weapon_firing_range = MIN(wip->lifetime * wip->max_speed, wip->weapon_range);
			WeaponMinRange = wip->WeaponMinRange;
		}


		if (wip->wi_flags & WIF_BEAM) {
			if ( !((shipp->tag_left > 0) || (shipp->level2_tag_left > 0)) ) {
				if (Nebula_sec_range)
				{
					weapon_firing_range *= BEAM_NEBULA_RANGE_REDUCE_FACTOR;

					// *Scale minimum weapon range in nebula    -Et1
					//Why? - WMC
					//Commented this out, because it doesn't make sense - WMC
					//WeaponMinRange *= BEAM_NEBULA_RANGE_REDUCE_FACTOR;
				}
			}
		}

		if(lep != NULL)
		{
			if( (dist_to_enemy <= weapon_firing_range) && (dist_to_enemy >= WeaponMinRange) )
			{
				if ( wip->wi_flags & WIF_HUGE ) {
					if ( lep->type != OBJ_SHIP ) {
						continue;
					}
					if ( !(Ship_info[Ships[lep->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
						continue;
					}
				}

				if ( wip->wi_flags2 & WIF2_SMALL_ONLY ) {
					if ( (lep->type == OBJ_SHIP) && (Ship_info[Ships[lep->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
						continue;
					}
				}

				if (lep->type == OBJ_SHIP) {
					// Check if we're targeting a protected ship
					if (lep->flags & OF_PROTECTED) {
						ss->turret_enemy_objnum = -1;
						ss->turret_time_enemy_in_range = 0.0f;
						continue;
					}

					// Check if we're targeting a beam protected ship with a beam weapon
					if ( (lep->flags & OF_BEAM_PROTECTED) && (wip->wi_flags & WIF_BEAM) ) {
						ss->turret_enemy_objnum = -1;
						ss->turret_time_enemy_in_range = 0.0f;
						continue;
					}
					// Check if we're targeting a flak protected ship with a flak weapon
					else if ( (lep->flags & OF_FLAK_PROTECTED) && (wip->wi_flags & WIF_FLAK) ) {
						ss->turret_enemy_objnum = -1;
						ss->turret_time_enemy_in_range = 0.0f;
						continue;
					}
					// Check if we're targeting a laser protected ship with a laser weapon
					else if ( (lep->flags & OF_LASER_PROTECTED) && (wip->subtype == WP_LASER) ) {
						ss->turret_enemy_objnum = -1;
						ss->turret_time_enemy_in_range = 0.0f;
						continue;
					}
					// Check if we're targeting a missile protected ship with a missile weapon
					else if ( (lep->flags & OF_MISSILE_PROTECTED) && (wip->subtype == WP_MISSILE) ) {
						ss->turret_enemy_objnum = -1;
						ss->turret_time_enemy_in_range = 0.0f;
						continue;
					}
				}
				else
				{
					//can't tag anything else, other than asteroids
					//but we don't want to waste this type of
					//weaponary on asteroids now do we?
					if ((wip->wi_flags2 & WIF2_TAGGED_ONLY) || (ss->weapons.flags & SW_FLAG_TAGGED_ONLY))
					{
						continue;
					}
				}

				//Add it to the list
				valid_weapons[num_valid++] = i;
			}
		}
	}

	//none of our guns can hit the enemy, so find a new one
	if(num_valid < 1)
		ss->turret_enemy_objnum = -1;

	//	Maybe pick a new enemy, unless targeting has been taken over by scripting
	if ( turret_should_pick_new_target(ss) && !ss->scripting_target_override ) {
		Num_find_turret_enemy++;
		int objnum = find_turret_enemy(ss, parent_objnum, &gpos, &gvec, ss->turret_enemy_objnum);
		//Assert(objnum < 0 || is_target_beam_valid(tp, objnum));

		if (objnum >= 0 && is_target_beam_valid(&ss->weapons, &Objects[objnum])) {
			if (ss->turret_enemy_objnum == -1) {
				ss->turret_enemy_objnum = objnum;
				ss->turret_enemy_sig = Objects[objnum].signature;
				// why return?
				return;
			} else {
				ss->turret_enemy_objnum = objnum;
				ss->turret_enemy_sig = Objects[objnum].signature;
			}
		} else {
			ss->turret_enemy_objnum = -1;
		}

		if (ss->turret_enemy_objnum != -1) {
			float	dot = 1.0f;
			lep = &Objects[ss->turret_enemy_objnum];
			if (( lep->type == OBJ_SHIP ) && !(ss->flags & SSF_NO_SS_TARGETING)) {
				ss->targeted_subsys = aifft_find_turret_subsys(objp, ss, lep, &dot);				
			}
			ss->turret_next_enemy_check_stamp = timestamp((int) (MAX(dot, 0.5f)*2000.0f) + 1000);
		} else {
			ss->turret_next_enemy_check_stamp = timestamp((int) (2000.0f * frand_range(0.9f, 1.1f)));	//	Check every two seconds
		}
	}

	//	If still don't have an enemy, return.  Or, if enemy is protected, return.
	if (ss->turret_enemy_objnum != -1) {
		// Don't shoot at ship we're docked with.
		if (dock_check_find_docked_object(objp, &Objects[ss->turret_enemy_objnum]))
		{
			ss->turret_enemy_objnum = -1;
			return;
		}

		// Goober5000 - Also, don't shoot at a ship we're docking with.  Volition
		// had this in the code originally but messed it up so that it didn't work.
		ai_info *aip = &Ai_info[shipp->ai_index];
		if ((aip->mode == AIM_DOCK) && (aip->goal_objnum == ss->turret_enemy_objnum))
		{
			ss->turret_enemy_objnum = -1;
			return;
		}

		if (Objects[ss->turret_enemy_objnum].flags & OF_PROTECTED) {
			//	This can happen if the enemy was selected before it became protected.
			ss->turret_enemy_objnum = -1;
			return;
		}

		lep = &Objects[ss->turret_enemy_objnum];
	} else {
		if (timestamp_until(ss->turret_next_fire_stamp) < 500) {
			ss->turret_next_fire_stamp = timestamp(500);
		}
		return;
	}

	if ( lep == NULL ){
		mprintf(("last enemy is null\n"));
		return;
	}

	//This can't happen. See above code
	//Assert(ss->turret_enemy_objnum != -1);

	float dot;
	bool in_fov;
	in_fov = turret_fov_test(ss, &gvec, &v2e);

	// Ok, the turret is lined up... now line up a particular gun.
	bool ok_to_fire = false;
	bool something_was_ok_to_fire=false;
	vec3d tv2e;	//so flak can get their jitter without screwing up other guns

	if (in_fov ) {

		// Do salvo thing separately - to prevent messing up things
		int number_of_firings;
		if (tp->flags & MSS_FLAG_TURRET_SALVO)
		{
			number_of_firings = tp->turret_num_firing_points;
			ss->turret_next_fire_pos = 0;
		}
		else
		{
			number_of_firings = num_valid;
		}

		for(i = 0; i < number_of_firings; i++)
		{
			if (tp->flags & MSS_FLAG_TURRET_FIXED_FP)
			{
				int ffp_pos = 0;
				int ffp_bank;
				
				ffp_bank = valid_weapons[i];

				if (ffp_bank < MAX_SHIP_PRIMARY_BANKS)
				{
					ffp_pos = ffp_bank;
				}
				else
				{
					ffp_pos = ffp_bank - MAX_SHIP_PRIMARY_BANKS + ss->weapons.num_primary_banks;
				}

				ss->turret_next_fire_pos = ffp_pos;
			}

			ship_get_global_turret_gun_info(&Objects[parent_objnum], ss, &gpos, &gvec, use_angles, &predicted_enemy_pos);

			// Fire in the direction the turret is facing, not right at the target regardless of turret dir.
			vm_vec_sub(&v2e, &predicted_enemy_pos, &gpos);
			dist_to_enemy = vm_vec_normalize(&v2e);
			dot = vm_vec_dot(&v2e, &gvec);

			if (tp->flags & MSS_FLAG_TURRET_SALVO)
				wip = get_turret_weapon_wip(&ss->weapons, valid_weapons[0]);
			else
				wip = get_turret_weapon_wip(&ss->weapons, valid_weapons[i]);

			// We're ready to fire... now get down to specifics, like where is the
			// actual gun point and normal, not just the one for whole turret.
			// moved here as if there are two weapons with indentical fire stamps
			// they would have shared the fire point.
			tv2e = gvec;
	
			// make sure to reset this for current weapon
			ok_to_fire = false;

			// if the weapon is a flak gun, add some jitter to its aim so it fires in a "cone" 
			// to make a cool visual effect and make them less lethal
			if(wip->wi_flags & WIF_FLAK){
				flak_jitter_aim(&tv2e, dist_to_enemy, ship_get_subsystem_strength(shipp, SUBSYSTEM_WEAPONS));
			}
			
			// Fire if:
			//		dumbfire and nearly pointing at target.
			//		heat seeking and target in a fairly wide cone.
			//		aspect seeking and target is locked.
			//turret_weapon_class = tp->turret_weapon_type;
			bool in_sight = false;
			
			if (The_mission.ai_profile->flags & AIPF_USE_ONLY_SINGLE_FOV_FOR_TURRETS) {
				// we have already passed the FOV test of the turret so...
				in_sight = true;
			} else {
				if (wip->wi_flags & WIF_HOMING_HEAT) {
					if (dot > AICODE_TURRET_HEATSEEK_ANGLE) {
						in_sight = true;
					}
				} else {
					if (dot > AICODE_TURRET_DUMBFIRE_ANGLE) {
						in_sight = true;
					}
				}
			}

			if ( !(wip->wi_flags & WIF_HOMING) )
			{
				if ((dist_to_enemy < 75.0f) || in_sight)
				{
					turret_update_enemy_in_range(ss, 2*wip->fire_wait);
					ok_to_fire = true;
				}
			}
			else if ( wip->wi_flags & WIF_HOMING_HEAT )
			{	// if heat seekers
				if ((dist_to_enemy < 50.0f) || in_sight)
				{
					turret_update_enemy_in_range(ss, 2*wip->fire_wait);
					ok_to_fire = true;
				}
			}
			else if ( wip->wi_flags & WIF_HOMING_ASPECT )
			{	// if aspect seeker
				if ((dist_to_enemy < 50.0f) || in_sight)
				{
					turret_update_enemy_in_range(ss, 2*wip->fire_wait);
				}
				if ( turret_should_fire_aspect(ss, wip, in_sight) )
				{
					ok_to_fire = true;
				}
			}
			else if ( wip->wi_flags & WIF_HOMING_JAVELIN )
			{	// if javelin heat seeker
				if ((dist_to_enemy < 50.0f) || in_sight)
				{
					turret_update_enemy_in_range(ss, 2*wip->fire_wait);
				}
				// Check if turret should fire and enemy's engines are
				// in line of sight
				if (turret_should_fire_aspect(ss, wip, in_sight) &&
					ship_get_closest_subsys_in_sight(&Ships[lep->signature], SUBSYSTEM_ENGINE, &gpos))
				{
					ok_to_fire = true;
				}
			}

			if ( ok_to_fire && (tp->flags & MSS_FLAG_TURRET_HULL_CHECK) ) {
				int model_num = Ship_info[shipp->ship_info_index].model_num;
				mc_info hull_check;
				vec3d end;

				vm_vec_scale_add(&end, &gpos, &gvec, model_get_radius(model_num));

				hull_check.model_instance_num = shipp->model_instance_num;
				hull_check.model_num = model_num;
				hull_check.orient = &objp->orient;
				hull_check.pos = &objp->pos;
				hull_check.p0 = &gpos;
				hull_check.p1 = &end;
				hull_check.flags = MC_CHECK_MODEL | MC_CHECK_RAY;

				if ( model_collide(&hull_check) ) {
					ok_to_fire = false;
				}
			}

			if ( ok_to_fire )
			{
				// starting animation checks
				if (ss->turret_animation_position == MA_POS_NOT_SET) {
					if ( model_anim_start_type(shipp, TRIGGER_TYPE_TURRET_FIRING, ss->system_info->subobj_num, 1) ) {
						ss->turret_animation_done_time = model_anim_get_time_type(shipp, TRIGGER_TYPE_TURRET_FIRING, ss->system_info->subobj_num);
						ss->turret_animation_position = MA_POS_SET;
					}
				}

				//Wait until the animation is done to actually fire
				if (tp->flags & MSS_FLAG_TURRET_ANIM_WAIT && (ss->turret_animation_position != MA_POS_READY))
				{
					ok_to_fire = false;
				}
			}

			if ( ok_to_fire )
			{
				something_was_ok_to_fire = true;
				Num_turrets_fired++;

				//Pass along which gun we are using
				if (tp->flags & MSS_FLAG_TURRET_SALVO)
					turret_fire_weapon(valid_weapons[0], ss, parent_objnum, &gpos, &tv2e, &predicted_enemy_pos);
				else
					turret_fire_weapon(valid_weapons[i], ss, parent_objnum, &gpos, &tv2e, &predicted_enemy_pos);
			} else {
				// make sure salvo fire mode does not turn into autofire
				if ((tp->flags & MSS_FLAG_TURRET_SALVO) && ((i + 1) == number_of_firings)) {
					ai_info *parent_aip = &Ai_info[Ships[Objects[parent_objnum].instance].ai_index];
					turret_set_next_fire_timestamp(valid_weapons[0], wip, ss, parent_aip);
				}
			}
			ss->turret_next_fire_pos++;
		}

		if(!something_was_ok_to_fire)
		{
			mprintf(("nothing ok to fire\n"));
			//Impose a penalty on turret accuracy for losing site of its goal, or just not being able to fire.
			turret_update_enemy_in_range(ss, -4*Weapon_info[ss->turret_best_weapon].fire_wait);
			ss->turret_next_fire_stamp = timestamp(500);

			// If nothing is OK to fire (lost track of the target?) 
			// reset the target (so we don't continue to track what we can't hit)
			if (tp->flags2 & MSS_FLAG2_TURRET_ONLY_TARGET_IF_CAN_FIRE)
			{
				ss->turret_enemy_objnum = -1;		//	Reset enemy objnum, find a new one next frame.
				ss->turret_time_enemy_in_range = 0.0f;
			}
		}
		else
		{
			ss->turret_next_fire_stamp = INT_MAX;
			//something did fire, get the lowest valid timestamp 
			for (i = 0; i < MAX_SHIP_WEAPONS; i++)
			{
				if (i < MAX_SHIP_PRIMARY_BANKS)
				{
					//timestamp range is 2 to INT_MAX/2
					if (swp->next_primary_fire_stamp[i] < 2) continue;
					
					if (swp->next_primary_fire_stamp[i] < ss->turret_next_fire_stamp)
					{
						ss->turret_next_fire_stamp = swp->next_primary_fire_stamp[i];
					}
				}
				else
				{
					//timestamp range is 2 to INT_MAX/2
					if (swp->next_secondary_fire_stamp[i - MAX_SHIP_PRIMARY_BANKS] < 2) continue;

					if (swp->next_secondary_fire_stamp[i - MAX_SHIP_PRIMARY_BANKS] < ss->turret_next_fire_stamp)
					{
						ss->turret_next_fire_stamp = swp->next_secondary_fire_stamp[i - MAX_SHIP_PRIMARY_BANKS];
					}

				}

			}
			// Wanderer -- sanity check
			// if timestamp is still at int_max reset the wait to 100 ms instead of locking the turret for the rest of the game
			if (ss->turret_next_fire_stamp == INT_MAX)
				ss->turret_next_fire_stamp = timestamp(100);
		}
	}
	else
	{
		// Lost him!
		ss->turret_enemy_objnum = -1;		//	Reset enemy objnum, find a new one next frame.
		ss->turret_time_enemy_in_range = 0.0f;
	}
}

bool turret_std_fov_test(ship_subsys *ss, vec3d *gvec, vec3d *v2e, float size_mod)
{
	model_subsystem *tp = ss->system_info;
	float dot = vm_vec_dot(v2e, gvec);
	if (((dot + size_mod) >= tp->turret_fov) && ((dot - size_mod) <= tp->turret_max_fov))
		return true;
	
	return false;
}

bool turret_adv_fov_test(ship_subsys *ss, vec3d *gvec, vec3d *v2e, float size_mod)
{
	model_subsystem *tp = ss->system_info;
	float dot = vm_vec_dot(v2e, gvec);
	if (((dot + size_mod) >= tp->turret_fov) && ((dot - size_mod) <= tp->turret_max_fov)) {
		vec3d of_dst;
		vm_vec_rotate( &of_dst, v2e, &ss->world_to_turret_matrix );
		if ((of_dst.xyz.x == 0) && (of_dst.xyz.y == 0)) {
			return true;
		} else {
			of_dst.xyz.z = 0;
			if (!IS_VEC_NULL_SQ_SAFE(&of_dst)) {
				vm_vec_normalize(&of_dst);
				// now we have 2d vector with lenght of 1 that points at the targets direction after being rotated to turrets FOR
				if ((-of_dst.xyz.y + size_mod) >= tp->turret_y_fov)
					return true;
			}
		}
	}
	return false;
}

bool turret_fov_test(ship_subsys *ss, vec3d *gvec, vec3d *v2e, float size_mod)
{
	bool in_fov = false;
	if (ss->system_info->flags & MSS_FLAG_TURRET_ALT_MATH)
		in_fov = turret_adv_fov_test(ss, gvec, v2e, size_mod);
	else
		in_fov = turret_std_fov_test(ss, gvec, v2e, size_mod);

	return in_fov;
}

float get_adjusted_turret_rof(ship_subsys *ss)
{
	float tempf = ss->rof_scaler;

	// optional reset switch (negative value)
	if (tempf < 0) {
		ss->rof_scaler = 1.0f;
		return 1.0f;
	}

	if (tempf == 0) {
		// special case returning the number of firingpoints
		ss->rof_scaler = (float) ss->system_info->turret_num_firing_points;
		tempf = ss->rof_scaler;

		// safety check to avoid div/0 issues
		if (tempf == 0) {
			ss->rof_scaler = 1.0f;
			return 1.0f;
		}
	}

	return tempf;
}
