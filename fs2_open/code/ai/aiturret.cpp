/*
 * $Logfile: /Freespace2/code/ai/aiturret.cpp $
 * $Revision: 1.39.2.9 $
 * $Date: 2007-03-22 20:09:05 $
 * $Author: taylor $
 *
 * Functions for AI control of turrets
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.39.2.8  2007/02/20 04:19:09  Goober5000
 * the great big duplicate model removal commit
 *
 * Revision 1.39.2.7  2007/02/11 06:19:07  Goober5000
 * invert the do-collision flag into a don't-do-collision flag, plus fixed a wee lab bug
 *
 * Revision 1.39.2.6  2006/12/07 17:51:39  taylor
 * make fire-on-normal return value a little more obvious
 * fix the "infinite genius" move I made by breaking turret movements ;)
 *
 * Revision 1.39.2.5  2006/10/27 21:33:06  taylor
 * updated/fixed modelanim code
 * add ships.tbl subsystem flag ("+fire-down-normals") which will force a turret to fire down it's barrel line (Mantis bug #591)
 *
 * Revision 1.39.2.4  2006/09/13 03:06:46  taylor
 * restore MAX_AIFTT_TURRETS to retail level, and add comment as to what it means for the next person that tries to change it
 *
 * Revision 1.39.2.3  2006/09/11 09:42:53  taylor
 * revert to retail behavior for turret_should_pick_new_target()
 * fix out-of-bounds issue
 *
 * Revision 1.39.2.2  2006/09/08 06:06:34  taylor
 * fix for Mantis bug #687 (at least until better swarming code can be written, post-3.6.9)
 * a quick speed/sanity check for turrets to be sure that they actually have a weapon to fire before processing them
 *
 * Revision 1.39.2.1  2006/09/04 18:05:09  Goober5000
 * fix macros
 *
 * Revision 1.39  2006/06/01 04:40:41  taylor
 * be sure to to reset ok_to_fire between weapon checks to make sure we don't count something by mistake
 *
 * Revision 1.38  2006/04/14 21:13:31  taylor
 * Grrr!  That was still stupid.  Just going to revert it to retail and work out that bug it was supposed to fix at a later time.
 *
 * Revision 1.37  2006/04/14 18:36:11  taylor
 * I might have blamed this on sleep, if it wasn't a bold faced lie. ;)
 *   - another part of the turret untargetting target bug
 *
 * Revision 1.36  2006/03/25 10:38:44  taylor
 * minor cleanup
 * address numerous out-of-bounds issues
 * add some better error checking and Assert()'s around
 *
 * Revision 1.35  2006/03/24 07:38:35  wmcoolmon
 * New subobject animation stuff and Lua functions.
 *
 * Revision 1.34  2006/03/21 02:50:59  Goober5000
 * fix taylor's fix :p
 * --Goober5000
 *
 * Revision 1.33  2006/03/21 00:08:18  taylor
 * fix target eval check for weapons so that friendly ships will stop shooting at friendly bombs
 *
 * Revision 1.32  2006/02/19 07:20:43  Goober5000
 * rearrange some turret code to be more like retail
 * --Goober5000
 *
 * Revision 1.31  2006/01/13 03:30:59  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 1.30  2006/01/11 21:15:15  wmcoolmon
 * Somewhat better turret comments
 *
 * Revision 1.29  2006/01/09 04:50:18  phreak
 * fix compile warnings.
 *
 * Revision 1.28  2006/01/06 04:18:54  wmcoolmon
 * turret-target-order SEXPs, ship thrusters
 *
 * Revision 1.27  2005/12/29 08:08:33  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 1.26  2005/11/21 02:43:30  Goober5000
 * change from "setting" to "profile"; this way makes more sense
 * --Goober5000
 *
 * Revision 1.25  2005/11/21 00:46:05  Goober5000
 * add ai_settings.tbl
 * --Goober5000
 *
 * Revision 1.24  2005/10/30 06:44:56  wmcoolmon
 * Codebase commit - nebula.tbl, scripting, new dinky explosion/shockwave stuff, moving muzzle flashes
 *
 * Revision 1.23  2005/10/22 22:22:41  Goober5000
 * rolled back UnknownPlayer's commit
 * --Goober5000
 *
 * Revision 1.21  2005/10/14 07:22:23  Goober5000
 * removed an unneeded parameter and renamed some stuff
 * --Goober5000
 *
 * Revision 1.20  2005/10/10 17:21:03  taylor
 * remove NO_NETWORK
 *
 * Revision 1.19  2005/07/25 05:23:33  Goober5000
 * whoops
 * --Goober5000
 *
 * Revision 1.18  2005/07/25 03:13:24  Goober5000
 * various code cleanups, tweaks, and fixes; most notably the MISSION_FLAG_USE_NEW_AI
 * should now be added to all places where it is needed (except the turret code, which I still
 * have to to review)
 * --Goober5000
 *
 * Revision 1.17  2005/05/16 15:40:43  phreak
 * reverted a tagged only commit, it wasn't needed
 *
 * Revision 1.16  2005/05/14 21:48:22  phreak
 * another tagged only turret fix
 *
 * Revision 1.15  2005/05/14 21:35:04  phreak
 * stop turrets from targeting non-bombs.  also some tagged-only fixes.
 *
 * Revision 1.14  2005/05/13 02:50:47  phreak
 * fixed another minimum range bug that prevented the Colossus and the Beast from
 * properly engaging one another in the mission: Their Finest Hour (SM3-08)
 *
 * Revision 1.13  2005/05/10 15:49:04  phreak
 * fixed a minimum weapon range bug that was causing turrets to fire at ships beyond
 * the actual range of a weapon.
 *
 * Revision 1.12  2005/05/10 02:44:40  phreak
 * if you're using all_turret_weapons_have_flags() with a WIF2_** flag, use
 * all_turret_weapons_have_flags2().
 *
 * Corrected some logic that would dump a valid target for turrets if its objnum is 0, as
 * opposed to -1.
 *
 * Revision 1.11  2005/05/10 01:47:34  phreak
 * when finding valid weapons, set i to MAX_SHIP_PRIMARY_BANKS - 1 since i
 * is incremented at the top of the loop.  otherwise, we would pass over the first secondary
 * weapon in the bank.
 *
 * Revision 1.10  2005/05/08 20:35:26  wmcoolmon
 * Various turret code changes in a vain attempt to make things more readable/work properly
 *
 * Revision 1.9  2005/04/28 05:29:28  wmcoolmon
 * Removed FS2_DEMO defines that looked like they wouldn't cause the universe to collapse
 *
 * Revision 1.8  2005/04/22 00:34:54  wmcoolmon
 * Minor updates to the GUI, and added some code that will (hopefully) resize HUD images in nonstandard resolutions. I couldn't test it; got an out of memory error.
 *
 * Revision 1.7  2005/04/20 04:29:50  phreak
 * CVS file header. take 1
 *
 *
 * $NoKeywords: $
 */

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

typedef struct eval_enemy_obj_struct {
	int			turret_parent_objnum;			// parent of turret
	float			weapon_travel_dist;				// max targeting range of turret weapon
	int			enemy_team_mask;
	int			weapon_system_ok;					// is the weapon subsystem of turret ship ok
	bool		big_only_flag;						// turret fires only at big and huge ships
	bool		small_only_flag;					// turret fires only at small ships
	bool		tagged_only_flag;					// turret fires only at tagged ships
	bool		beam_flag;							// turret is a beam
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
int object_in_turret_fov(object *objp, model_subsystem *tp, vec3d *tvec, vec3d *tpos, float dist)
{
	vec3d	v2e;
	float		dot;
	vm_vec_normalized_dir(&v2e, &objp->pos, tpos);
	dot = vm_vec_dot(&v2e, tvec);

	dot += objp->radius / (dist + objp->radius);

	if ( dot >= tp->turret_fov ) {
		return 1;
	}

	return 0;
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
//				turret_parent	=>	object index for ship that turret sits on
//				turret			=>	turret pointer
int valid_turret_enemy(object *objp, object *turret_parent)
{
	if ( objp == turret_parent ) {
		return 0;
	}

	if ( objp->type == OBJ_ASTEROID ) {
		return 1;
	}

	if ( (objp->type == OBJ_SHIP) ) {
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

		if ( !(wip->wi_flags & WIF_BOMB) ) {
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
	ship		*shipp;
	model_subsystem *tp = eeo->turret_subsys->system_info;
	float dist;

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
		if (eeo->beam_flag) {
			if (objp->flags & OF_BEAM_PROTECTED) {
				return;
			}
		}

		// don't shoot at small ships if we shouldn't
		if (eeo->big_only_flag) {
			if (!(Ship_info[shipp->ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))) {
				return;
			}
		}

		// don't shoot at big ships if we shouldn't
		if (eeo->small_only_flag) {
			if ((Ship_info[shipp->ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))) {
				return;
			}
		}

		// check if	turret flagged to only target tagged ships
		if (eeo->tagged_only_flag) {
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
	dist = vm_vec_dist_quick(eeo->tpos, &objp->pos) - objp->radius;
	if (dist < 0.0f) {
		dist = 0.0f;
	}

	// check if object is a bomb attacking the turret parent
	// check if bomb is homing on the turret parent ship
	if (objp->type == OBJ_WEAPON) {
		if ( Weapons[objp->instance].homing_object == &Objects[eeo->turret_parent_objnum] ) {
			if ( dist < eeo->nearest_homing_bomb_dist ) {
				if ( (eeo->current_enemy == -1) || object_in_turret_fov(objp, tp, eeo->tvec, eeo->tpos, dist + objp->radius) ) {
					eeo->nearest_homing_bomb_dist = dist;
					eeo->nearest_homing_bomb_objnum = OBJ_INDEX(objp);
				}
			}
		// if not homing, check if bomb is flying towards ship
		} else if ( bomb_headed_towards_ship(objp, &Objects[eeo->turret_parent_objnum]) ) {
			if ( dist < eeo->nearest_bomb_dist ) {
				if ( (eeo->current_enemy == -1) || object_in_turret_fov(objp, tp, eeo->tvec, eeo->tpos, dist + objp->radius) ) {
					eeo->nearest_bomb_dist = dist;
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
		dist *= (1.0f + 0.1f*num_att_turrets);

		// return if we're over the cap
//		int max_turrets = 3 + Game_skill_level * Game_skill_level;
		int max_turrets = The_mission.ai_profile->max_turret_ownage_target[Game_skill_level];
		if (objp->flags & OF_PLAYER_SHIP) {
			max_turrets = The_mission.ai_profile->max_turret_ownage_player[Game_skill_level];
		}
		if (num_att_turrets > max_turrets) {
			return;
		}

		// modify distance based on lethality of objp to my ship
		float active_lethality = aip->lethality;
		if (objp->flags & OF_PLAYER_SHIP) {
			active_lethality += Player_lethality_bump[Game_skill_level];
		}

		dist /= (1.0f + 0.01f*Lethality_range_const*active_lethality);

		// Make level 2 tagged ships more likely to be targeted
		if (shipp->level2_tag_left > 0.0f) {
			dist *= 0.3f;
		}

		// check if objp is targeting the turret's ship, or if objp has just hit the turret's ship
		if ( aip->target_objnum == eeo->turret_parent_objnum || aip->last_objsig_hit == Objects[eeo->turret_parent_objnum].signature ) {
			// A turret will always target a ship that is attacking itself... self-preservation!
			if ( aip->targeted_subsys == eeo->turret_subsys ) {
				dist *= 0.5f;	// highest priority
			}
		}

		// maybe update nearest attacker
		if ( dist < eeo->nearest_attacker_dist ) {
			if ( (eeo->current_enemy == -1) || object_in_turret_fov(objp, tp, eeo->tvec, eeo->tpos, dist + objp->radius) ) {
				// nprintf(("AI", "Nearest enemy = %s, dist = %7.3f, dot = %6.3f, fov = %6.3f\n", Ships[objp->instance].ship_name, dist, vm_vec_dot(&v2e, tvec), tp->turret_fov));
				eeo->nearest_attacker_dist = dist;
				eeo->nearest_attacker_objnum = OBJ_INDEX(objp);
			}
		}
	} // end ship section

	// check if object is an asteroid attacking the turret parent - taylor
	if (objp->type == OBJ_ASTEROID) {
		if ( eeo->turret_parent_objnum == asteroid_collide_objnum(objp) ) {
			// give priority to the closest asteroid *impact* (ms intervals)
			dist *= 0.9f + (0.01f * asteroid_time_to_impact(objp));

			if (dist < eeo->nearest_dist ) {
				if ( (eeo->current_enemy == -1) || object_in_turret_fov(objp, tp, eeo->tvec, eeo->tpos, dist + objp->radius) ) {
					eeo->nearest_dist = dist;
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
int get_nearest_turret_objnum(int turret_parent_objnum, ship_subsys *turret_subsys, int enemy_team_mask, vec3d *tpos, vec3d *tvec, int current_enemy, bool big_only_flag, bool small_only_flag, bool tagged_only_flag, bool beam_flag)
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
	eeo.big_only_flag = big_only_flag;
	eeo.small_only_flag = small_only_flag;
	eeo.tagged_only_flag = tagged_only_flag;
	eeo.beam_flag = beam_flag;
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

	for(int i = 0; i < NUM_TURRET_ORDER_TYPES; i++)
	{
		switch(turret_subsys->turret_targeting_order[i])
		{
			case -1:
					//Empty priority slot
				break;

			case 0:
				//Return if a bomb is found
				//don't fire anti capital ship turrets at bombs.
				if ( !((The_mission.ai_profile->flags & AIPF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS) && big_only_flag) )
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
				}
				return eeo.nearest_objnum;										// lowest priority is the closest enemy objnum

			default:
				Int3(); //Means invalid number passed.
		}
	}

	//prevent warning of all control paths not returning a value
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
int find_turret_enemy(ship_subsys *turret_subsys, int objnum, vec3d *tpos, vec3d *tvec, int current_enemy, float fov)
{
	int					enemy_team_mask, enemy_objnum;
	model_subsystem	*tp;
	ship_info			*sip;

	tp = turret_subsys->system_info;
	enemy_team_mask = iff_get_attackee_mask(obj_team(&Objects[objnum]));
	bool big_only_flag = all_turret_weapons_have_flags(&turret_subsys->weapons, WIF_HUGE);
	bool small_only_flag = all_turret_weapons_have_flags2(&turret_subsys->weapons, WIF2_SMALL_ONLY);
	bool tagged_only_flag = all_turret_weapons_have_flags2(&turret_subsys->weapons, WIF2_TAGGED_ONLY) || (turret_subsys->weapons.flags & SW_FLAG_TAGGED_ONLY);
	bool beam_flag = all_turret_weapons_have_flags(&turret_subsys->weapons, WIF_BEAM);

	//	If a small ship and target_objnum != -1, use that as goal.
	ai_info	*aip = &Ai_info[Ships[Objects[objnum].instance].ai_index];
	sip = &Ship_info[Ships[Objects[objnum].instance].ship_info_index];

	if ((sip->flags & SIF_SMALL_SHIP) && (aip->target_objnum != -1)) {
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
			// check for beam weapon and beam protected
			if ( !((target_flags & OF_BEAM_PROTECTED) && (beam_flag)) ) {
				if ( Objects[aip->target_objnum].type == OBJ_SHIP ) {
					// check for huge weapon and huge ship
					if ( !big_only_flag || (Ship_info[Ships[Objects[aip->target_objnum].instance].ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP)) ) {
						// check for tagged only and tagged ship
						if ( tagged_only_flag && ship_is_tagged(&Objects[aip->target_objnum]) ) {
							// select new target if aip->target_objnum is out of field of view
							vec3d v2e;
							float dot, dist;
							dist = vm_vec_normalized_dir(&v2e, &Objects[aip->target_objnum].pos, tpos);
							dot = vm_vec_dot(&v2e, tvec);
							//	MODIFY FOR ATTACKING BIG SHIP
							// dot += (0.5f * Objects[aip->target_objnum].radius / dist);
							if (dot > fov) {
								return aip->target_objnum;
							}
						}
					}
				}
			}
		}
	}

	enemy_objnum = get_nearest_turret_objnum(objnum, turret_subsys, enemy_team_mask, tpos, tvec, current_enemy, big_only_flag, small_only_flag, tagged_only_flag, beam_flag);
	if ( enemy_objnum >= 0 ) {
		Assert( !((Objects[enemy_objnum].flags & OF_BEAM_PROTECTED) && beam_flag) );
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

	ship_model_start(objp);

	gun_pos = &tp->turret_firing_point[ssp->turret_next_fire_pos % tp->turret_num_firing_points];

	model_find_world_point(gpos, gun_pos, tp->model_num, tp->turret_gun_sobj, &objp->orient, &objp->pos );

	if (use_angles)
		model_find_world_dir(gvec, &tp->turret_norm, tp->model_num, tp->turret_gun_sobj, &objp->orient, &objp->pos );
	else {
		//vector	gun_pos2;
		//vm_vec_add(&gun_pos2, gpos, gun_pos);
		vm_vec_normalized_dir(gvec, targetp, gpos);
	}

	ship_model_stop(objp);	
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
		vec3d	gun_pos, gun_vec;
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

		//Figure out what point on the ship we want to point the gun at, and store the global location
		//in enemy_point.
		vec3d	enemy_point;
		if (ss->targeted_subsys != NULL) {
			if (ss->turret_enemy_objnum != -1) {
				vm_vec_unrotate(&enemy_point, &ss->targeted_subsys->system_info->pnt, &Objects[ss->turret_enemy_objnum].orient);
				vm_vec_add2(&enemy_point, &Objects[ss->turret_enemy_objnum].pos);
			}
		} else {
			if ((lep->type == OBJ_SHIP) && (Ship_info[Ships[lep->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))) {
				ai_big_pick_attack_point_turret(lep, ss, &gun_pos, &gun_vec, &enemy_point, tp->turret_fov, MIN(wip->max_speed * wip->lifetime, wip->weapon_range));
			} else {
				enemy_point = lep->pos;
			}
		}

		//Try to guess where the enemy will be, and store that spot in predicted_enemy_pos
		set_predicted_enemy_pos_turret(predicted_enemy_pos, &gun_pos, objp, &enemy_point, &lep->phys_info.vel, wip->max_speed, ss->turret_time_enemy_in_range * (weapon_system_strength + 1.0f)/2.0f);

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

		if (vm_vec_dot(&v2e, gvec) > tp->turret_fov) {
			ret_val = model_rotate_gun(Ship_info[shipp->ship_info_index].model_num,
										ss->system_info, &Objects[parent_objnum].orient, 
										&ss->submodel_info_1.angs, &ss->submodel_info_2.angs,
										&Objects[parent_objnum].pos, predicted_enemy_pos);
		}
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

		vm_vec_rotate(&turret_norm, &turret_subsysp->system_info->turret_norm, &objp->orient);
		return vm_vec_dot(&turret_norm, &vector_out);
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
	for(idx=offset; idx<aifft_list_size; idx+=stride){
		dot = aifft_compute_turret_dot(objp, enemy_objp, &abs_gun_pos, ssp, aifft_list[idx]);			

		if (dot* aifft_rank[idx] > best_dot) {
			best_dot = dot*aifft_rank[idx];
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
	float wait = wip->fire_wait * 1000.0f;
	int *fs_dest;
	if(weapon_num < MAX_SHIP_PRIMARY_BANKS)
		fs_dest = &turret->weapons.next_primary_fire_stamp[weapon_num];
	else
		fs_dest = &turret->weapons.next_secondary_fire_stamp[weapon_num - MAX_SHIP_PRIMARY_BANKS];

	//Check for the new cooldown flag
	if(!(wip->wi_flags2 & WIF2_SAME_TURRET_COOLDOWN))
	{

		// make side even for team vs. team
		if ((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)) {
			// flak guns need to fire more rapidly
			if (wip->wi_flags & WIF_FLAK) {
				wait *= The_mission.ai_profile->ship_fire_delay_scale_friendly[Game_skill_level] * 0.5f;
				wait += (Num_ai_classes - aip->ai_class - 1) * 40.0f;
			} else {
				wait *= The_mission.ai_profile->ship_fire_delay_scale_friendly[Game_skill_level];
				wait += (Num_ai_classes - aip->ai_class - 1) * 100.0f;
			}
		} else {
			// flak guns need to fire more rapidly
			if (wip->wi_flags & WIF_FLAK) {
				if (Ships[aip->shipnum].team == Player_ship->team) {
					wait *= The_mission.ai_profile->ship_fire_delay_scale_friendly[Game_skill_level] * 0.5f;
				} else {
					wait *= The_mission.ai_profile->ship_fire_delay_scale_hostile[Game_skill_level] * 0.5f;
				}	
				wait += (Num_ai_classes - aip->ai_class - 1) * 40.0f;

			} else if (wip->wi_flags & WIF_HUGE) {
				// make huge weapons fire independently of team
				wait *= The_mission.ai_profile->ship_fire_delay_scale_friendly[Game_skill_level];
				wait += (Num_ai_classes - aip->ai_class - 1) * 100.0f;
			} else {
				// give team friendly an advantage
				if (Ships[aip->shipnum].team == Player_ship->team) {
					wait *= The_mission.ai_profile->ship_fire_delay_scale_friendly[Game_skill_level];
				} else {
					wait *= The_mission.ai_profile->ship_fire_delay_scale_hostile[Game_skill_level];
				}	
				wait += (Num_ai_classes - aip->ai_class - 1) * 100.0f;
			}
		}
		// vary wait time +/- 10%
		wait *= frand_range(0.9f, 1.1f);
	}

	(*fs_dest) = timestamp((int)wait);
}

// Decide  if a turret should launch an aspect seeking missile
int turret_should_fire_aspect(ship_subsys *turret, float dot, weapon_info *wip)
{
	if ( (dot > AICODE_TURRET_DUMBFIRE_ANGLE) && (turret->turret_time_enemy_in_range >= MIN(wip->min_lock_time,AICODE_TURRET_MAX_TIME_IN_RANGE)) ) {
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

	//WMC - Limit firing to firestamp
	if(!timestamp_elapsed(get_turret_weapon_next_fire_stamp(&turret->weapons, weapon_num)))
		return false;

	parent_aip = &Ai_info[Ships[Objects[parent_objnum].instance].ai_index];
	parent_ship = &Ships[Objects[parent_objnum].instance];
	wip = get_turret_weapon_wip(&turret->weapons, weapon_num);
	int turret_weapon_class = WEAPON_INFO_INDEX(wip);

	if (check_ok_to_fire(parent_objnum, turret->turret_enemy_objnum, wip)) {
		vm_vector_2_matrix(&turret_orient, turret_fvec, NULL, NULL);
		turret->turret_last_fire_direction = *turret_fvec;

		// set next fire timestamp for the turret
		turret_set_next_fire_timestamp(weapon_num, wip, turret, parent_aip);

		// if this weapon is a beam weapon, handle it specially
		if (wip->wi_flags & WIF_BEAM) {
			// if this beam isn't free to fire
			if (!(turret->weapons.flags & SW_FLAG_BEAM_FREE)) {
				//WMC - remove this
				//Int3();	// should never get this far
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
			fire_info.fighter_beam = false;

			// fire a beam weapon
			beam_fire(&fire_info);

			return true;
		}
		// don't fire swam, but set up swarm info instead
		else if ((wip->wi_flags & WIF_SWARM) || (wip->wi_flags & WIF_CORKSCREW)) {
			turret_swarm_set_up_info(parent_objnum, turret, wip);

			return true;
		}
		// now do anything else
		else {
			for (int i=0; i < wip->shots; i++)
			{		
				weapon_objnum = weapon_create( turret_pos, &turret_orient, turret_weapon_class, parent_objnum, -1, 1);
				weapon_set_tracking_info(weapon_objnum, parent_objnum, turret->turret_enemy_objnum, 1, turret->targeted_subsys);		
			

				objp=&Objects[weapon_objnum];
				wp=&Weapons[objp->instance];

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
						if ( parent_objnum != OBJ_INDEX(Player_obj) ) {						
							snd_play_3d( &Snds[wip->launch_snd], turret_pos, &View_position );						
						}
					}
				}
			}

			// reset any animations if we need to
			// (I'm not sure how accurate this timestamp would be in practice - taylor)
			if (turret->turret_animation_position == MA_POS_READY)
				turret->turret_animation_done_time = timestamp(100);
		}
	}
	//Not useful -WMC
	else if (!(The_mission.ai_profile->flags & AIPF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY))
	{
		float wait = 1000.0f * frand_range(0.9f, 1.1f);
		turret->turret_next_fire_stamp = timestamp((int) wait);
	}

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

	// starting animation checks
	if (ss->turret_animation_position == MA_POS_NOT_SET) {
		if ( model_anim_start_type(shipp, TRIGGER_TYPE_TURRET_FIRING, ss->system_info->subobj_num, 1) ) {
			ss->turret_animation_done_time = model_anim_get_time_type(shipp, TRIGGER_TYPE_TURRET_FIRING, ss->system_info->subobj_num);
			ss->turret_animation_position = MA_POS_SET;
		}
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

	// Use the turret info for all guns, not one gun in particular.
	vec3d	 gvec, gpos;
	ship_get_global_turret_info(&Objects[parent_objnum], tp, &gpos, &gvec);

	// Rotate the turret even if time hasn't elapsed, since it needs to turn to face its target.
	int use_angles = aifft_rotate_turret(shipp, parent_objnum, ss, objp, lep, &predicted_enemy_pos, &gvec);

	if ( !timestamp_elapsed(ss->turret_next_fire_stamp) ) {
		return;
	}

	// Don't try to fire beyond weapon_limit_range
	//WMC - OTC
	//weapon_firing_range = MIN(Weapon_info[tp->turret_weapon_type].lifetime * Weapon_info[tp->turret_weapon_type].max_speed, Weapon_info[tp->turret_weapon_type].weapon_range);

	//WMC - build a list of valid weapons. Fire spawns if there are any.
	float dist_to_enemy = 0.0f;
	if(lep != NULL)
		dist_to_enemy = MAX(0,vm_vec_normalized_dir(&v2e, &predicted_enemy_pos, &gpos) - lep->radius);

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
		if ( wip->wi_flags & WIF_SPAWN )
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

	//	Maybe pick a new enemy.
	if ( turret_should_pick_new_target(ss) ) {
		Num_find_turret_enemy++;
		int objnum = find_turret_enemy(ss, parent_objnum, &gpos, &gvec, ss->turret_enemy_objnum, tp->turret_fov);
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
			if ( lep->type == OBJ_SHIP ) {
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

	float dot = vm_vec_dot(&v2e, &gvec);

	// Ok, the turret is lined up... now line up a particular gun.
	bool ok_to_fire = false;
	bool something_was_ok_to_fire=false;
	vec3d tv2e;	//so flak can get their jitter without screwing up other guns

	if (dot > tp->turret_fov ) {

		for(i = 0; i < num_valid; i++)
		{
			// We're ready to fire... now get down to specifics, like where is the
			// actual gun point and normal, not just the one for whole turret.
			// moved here as if there are two weapons with indentical fire stamps
			// they would have shared the fire point.
			ship_get_global_turret_gun_info(&Objects[parent_objnum], ss, &gpos, &gvec, use_angles, &predicted_enemy_pos);

			// Fire in the direction the turret is facing, not right at the target regardless of turret dir.
			vm_vec_sub(&v2e, &predicted_enemy_pos, &gpos);
			dist_to_enemy = vm_vec_normalize(&v2e);
			dot = vm_vec_dot(&v2e, &gvec);

			wip = get_turret_weapon_wip(&ss->weapons, valid_weapons[i]);
			tv2e = v2e;

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
			if ( !(wip->wi_flags & WIF_HOMING) )
			{
				if ((dist_to_enemy < 75.0f) || (dot > AICODE_TURRET_DUMBFIRE_ANGLE ))
				{
					turret_update_enemy_in_range(ss, 2*wip->fire_wait);
					ok_to_fire = true;
				}
			}
			else if ( wip->wi_flags & WIF_HOMING_HEAT )
			{	// if heat seekers
				if ((dist_to_enemy < 50.0f) || (dot > AICODE_TURRET_HEATSEEK_ANGLE ))
				{
					turret_update_enemy_in_range(ss, 2*wip->fire_wait);
					ok_to_fire = true;
				}
			}
			else if ( wip->wi_flags & WIF_HOMING_ASPECT )
			{	// if aspect seeker
				if ((dist_to_enemy < 50.0f) || (dot > AICODE_TURRET_DUMBFIRE_ANGLE ))
				{
					turret_update_enemy_in_range(ss, 2*wip->fire_wait);
				}
				if ( turret_should_fire_aspect(ss, dot, wip) )
				{
					ok_to_fire = true;
				}
			}
			else if ( wip->wi_flags & WIF_HOMING_JAVELIN )
			{	// if javelin heat seeker
				if ((dist_to_enemy < 50.0f) || (dot > AICODE_TURRET_DUMBFIRE_ANGLE ))
				{
					turret_update_enemy_in_range(ss, 2*wip->fire_wait);
				}
				// Check if turret should fire and enemy's engines are
				// in line of sight
				if (turret_should_fire_aspect(ss, dot, wip) &&
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
				something_was_ok_to_fire = true;
				Num_turrets_fired++;
				
				//Pass along which gun we are using
				turret_fire_weapon(valid_weapons[i], ss, parent_objnum, &gpos, &tv2e, &predicted_enemy_pos);
			}
			// moved this here so we increment the fire pos only after we have fired and not during it
			ss->turret_next_fire_pos++;
		}

		if(!something_was_ok_to_fire)
		{
			mprintf(("nothing ok to fire\n"));
			//Impose a penalty on turret accuracy for losing site of its goal, or just not being able to fire.
			turret_update_enemy_in_range(ss, -4*Weapon_info[ss->turret_best_weapon].fire_wait);
			ss->turret_next_fire_stamp = timestamp(500);
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
		}
	}
	else
	{
		// Lost him!
		ss->turret_enemy_objnum = -1;		//	Reset enemy objnum, find a new one next frame.
		ss->turret_time_enemy_in_range = 0.0f;
	}
}
