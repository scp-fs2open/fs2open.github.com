/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "ship/shiphit.h"
#include "object/object.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "render/3d.h"
#include "fireball/fireballs.h"
#include "debris/debris.h"
#include "hud/hud.h"
#include "io/timer.h"
#include "mission/missionlog.h"
#include "io/joy_ff.h"
#include "playerman/player.h"
#include "freespace2/freespace.h"
#include "globalincs/linklist.h"
#include "hud/hudtarget.h"
#include "gamesnd/gamesnd.h"
#include "gamesnd/eventmusic.h"
#include "ship/shipfx.h"
#include "gamesequence/gamesequence.h"
#include "weapon/shockwave.h"
#include "hud/hudmessage.h"
#include "popup/popup.h"
#include "weapon/emp.h"
#include "weapon/beam.h"
#include "object/objectdock.h"
#include "iff_defs/iff_defs.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multi_respawn.h"
#include "network/multi_pmsg.h"
#include "asteroid/asteroid.h"
#include "parse/scripting.h"
#include "parse/parselo.h"
#include "object/objectsnd.h"

//#pragma optimize("", off)
//#pragma auto_inline(off)

struct ssm_firing_info;
extern void ssm_create(object *target, vec3d *start, int ssm_index, ssm_firing_info *override, int team);

typedef struct spark_pair {
	int index1, index2;
	float dist;
} spark_pair;

#define MAX_SPARK_PAIRS		((MAX_SHIP_HITS * MAX_SHIP_HITS - MAX_SHIP_HITS) / 2)

#define	BIG_SHIP_MIN_RADIUS	80.0f	//	ship radius above which death rolls can't be shortened by excessive damage

vec3d	Dead_camera_pos;
vec3d	Original_vec_to_deader;

static bool global_damage = false;

//WMC - Camera rough draft stuff
/*
camid dead_get_camera()
{
	static camid dead_camera;
	if(!dead_camera.isValid())
		dead_camera = cam_create("Dead camera");

	return dead_camera;
}
*/

bool is_subsys_destroyed(ship *shipp, int submodel)
{
	ship_subsys *subsys;

	if (submodel == -1) {
		return false;
	}

	for ( subsys=GET_FIRST(&shipp->subsys_list); subsys != END_OF_LIST(&shipp->subsys_list); subsys = GET_NEXT(subsys) ) {
		if (subsys->system_info->subobj_num == submodel) {
			if (subsys->current_hits > 0) {
				return false;
			} else {
				return true;
			}
		}
	}

	return false;
}

// do_subobj_destroyed_stuff is called when a subobject for a ship is killed.  Separated out
// to separate function on 10/15/97 by MWA for easy multiplayer access.  It does all of the
// cool things like blowing off the model (if applicable, writing the logs, etc)
void do_subobj_destroyed_stuff( ship *ship_p, ship_subsys *subsys, vec3d* hitpos )
{
	ship_info *sip;
	object *ship_obj;
	model_subsystem *psub;
	vec3d	g_subobj_pos;
	int type, i, log_index;

	// get some local variables
	sip = &Ship_info[ship_p->ship_info_index];
	ship_obj = &Objects[ship_p->objnum];
	psub = subsys->system_info;
	type = psub->type;
	get_subsystem_world_pos(ship_obj, subsys, &g_subobj_pos);

	// create fireballs when subsys destroy for large ships.
	object* objp = &Objects[ship_p->objnum];

	if (!(subsys->flags & SSF_VANISHED)) {
		if (objp->radius > 100.0f) {
			// number of fireballs determined by radius of subsys
			int num_fireballs;
			if ( psub->radius < 3 ) {
				num_fireballs = 1;
			} else {
				 num_fireballs = 5;
			}

			vec3d temp_vec, center_to_subsys, rand_vec;
			vm_vec_sub(&center_to_subsys, &g_subobj_pos, &objp->pos);
			for (i=0; i<num_fireballs; i++) {
				if (i==0) {
					// make first fireball at hitpos
					if (hitpos) {
						temp_vec = *hitpos;
					} else {
						temp_vec = g_subobj_pos;
					}
				} else {
					// make other fireballs at random positions, but try to keep on the surface
					vm_vec_rand_vec_quick(&rand_vec);
					float dot = vm_vec_dotprod(&center_to_subsys, &rand_vec);
					vm_vec_scale_add2(&rand_vec, &center_to_subsys, -dot/vm_vec_mag_squared(&center_to_subsys));
					vm_vec_scale_add(&temp_vec, &g_subobj_pos, &rand_vec, 0.5f*psub->radius);
				}

				// scale fireball size according to size of subsystem, but not less than 10
				float fireball_rad = psub->radius * 0.2f;
				if (fireball_rad < 10) {
					fireball_rad = 10.0f;
				}

				vec3d fb_vel;
				vm_vec_crossprod(&fb_vel, &objp->phys_info.rotvel, &center_to_subsys);
				vm_vec_add2(&fb_vel, &objp->phys_info.vel);

				int fireball_type = fireball_ship_explosion_type(sip);
				if(fireball_type < 0) {
					fireball_type = FIREBALL_EXPLOSION_MEDIUM;
				}
				fireball_create( &temp_vec, fireball_type, FIREBALL_MEDIUM_EXPLOSION, OBJ_INDEX(objp), fireball_rad, 0, &fb_vel );
			}
		}
	}

	if ( MULTIPLAYER_MASTER ) {
		int index;

		index = ship_get_index_from_subsys(subsys, ship_p->objnum);
		
		vec3d hit;
		if (hitpos) {
			hit = *hitpos;
		} else {
			hit = g_subobj_pos;
		}
		send_subsystem_destroyed_packet( ship_p, index, hit );
	}

	// next do a quick sanity check on the current hits that we are keeping for the generic subsystems
	// I think that there might be rounding problems with the floats.  This code keeps us safe.
	if ( ship_p->subsys_info[type].type_count == 1 ) {
		ship_p->subsys_info[type].aggregate_current_hits = 0.0f;
	} else {
		float hits;
		ship_subsys *ssp;

		hits = 0.0f;
		for ( ssp=GET_FIRST(&ship_p->subsys_list); ssp != END_OF_LIST(&ship_p->subsys_list); ssp = GET_NEXT(ssp) ) {
			// type matches?
			if ( (ssp->system_info->type == type) && !(ssp->flags & SSF_NO_AGGREGATE) ) {
				hits += ssp->current_hits;
			}
		}
		ship_p->subsys_info[type].aggregate_current_hits = hits;
	}

	// store an event in the event log.  Also, determine if all turrets or all
	// engines have been destroyed (if the subsystem is a turret or engine).
	// put a disabled or disarmed entry in the log if this is the case
	// 
	// MWA -- 1/8/98  A problem was found when trying to determine (via sexpression) when some subsystems
	// were destroyed.  The bottom line is that is the psub->name and psub->subobj_name are different,
	// then direct detection doesn't work.  (This scenario happens mainly with turrets and probably with
	// engines).  So, my solution is to encode the ship_info index, and the subsystem index into one
	// integer, and pass that as the "index" parameter to add_entry.  We'll use that information to
	// print out the info in the mission log.
	Assert( ship_p->ship_info_index < 65535 );

	// get the "index" of this subsystem in the ship info structure.
	for ( i = 0; i < sip->n_subsystems; i++ ) {
		if ( &(sip->subsystems[i]) == psub )
			break;
	}
	Assert( i < sip->n_subsystems );
	Assert( i < 65535 );
	log_index = ((ship_p->ship_info_index << 16) & 0xffff0000) | (i & 0xffff);

	// Don't log, display info, or play sounds about the activation subsytem
	// FUBAR/Goober5000 - or about vanishing subsystems, per precedent with ship-vanish
	int notify = (psub->type != SUBSYSTEM_ACTIVATION) && !(subsys->flags & SSF_VANISHED);

	if (notify) 
	{
		mission_log_add_entry(LOG_SHIP_SUBSYS_DESTROYED, ship_p->ship_name, psub->subobj_name, log_index );
		if ( ship_obj == Player_obj )
		{
			snd_play( &Snds[SND_SUBSYS_DIE_1], 0.0f );
			if (strlen(psub->alt_dmg_sub_name))
				HUD_printf(XSTR( "Your %s subsystem has been destroyed", 499), psub->alt_dmg_sub_name);
			else {
				char r_name[NAME_LENGTH];
				strcpy_s(r_name, ship_subsys_get_name(subsys));
				for (i = 0; r_name[i] > 0; i++) {
					if (r_name[i] == '|')
						r_name[i] = ' ';
				}
				HUD_printf(XSTR( "Your %s subsystem has been destroyed", 499), r_name );
			}
		}
	}

	if ( psub->type == SUBSYSTEM_TURRET ) {
		if ( ship_p->subsys_info[type].aggregate_current_hits <= 0.0f ) {
			//	Don't create "disarmed" event for small ships.
			if (!(Ship_info[ship_p->ship_info_index].flags & SIF_SMALL_SHIP)) {
				mission_log_add_entry(LOG_SHIP_DISARMED, ship_p->ship_name, NULL );
				// ship_p->flags |= SF_DISARMED;
			}
		}
	} else if (psub->type == SUBSYSTEM_ENGINE ) {
		// when an engine is destroyed, we must change the max velocity of the ship
		// to be some fraction of its normal maximum value

		if ( ship_p->subsys_info[type].aggregate_current_hits <= 0.0f ) {
			mission_log_add_entry(LOG_SHIP_DISABLED, ship_p->ship_name, NULL );
			ship_p->flags |= SF_DISABLED;				// add the disabled flag
		}
	}

	if ( psub->subobj_num > -1 )	{
		shipfx_blow_off_subsystem(ship_obj,ship_p,subsys,&g_subobj_pos);
		subsys->submodel_info_1.blown_off = 1;
	}

	if ( (psub->subobj_num != psub->turret_gun_sobj) && (psub->turret_gun_sobj >= 0) )		{
		subsys->submodel_info_2.blown_off = 1;
	}

	if (notify) {
		// play sound effect when subsys gets blown up
		int sound_index=-1;
		if ( Ship_info[ship_p->ship_info_index].flags & SIF_HUGE_SHIP ) {
			sound_index=SND_CAPSHIP_SUBSYS_EXPLODE;
		} else if ( Ship_info[ship_p->ship_info_index].flags & SIF_BIG_SHIP ) {
			sound_index=SND_SUBSYS_EXPLODE;
		}
		if ( sound_index >= 0 ) {
			snd_play_3d( &Snds[sound_index], &g_subobj_pos, &View_position );
		}
	}

	// make the shipsounds work as they should...
	if(subsys->subsys_snd_flags & SSSF_ALIVE)
	{
		obj_snd_delete_type(ship_p->objnum, subsys->system_info->alive_snd, subsys);
		subsys->subsys_snd_flags &= ~SSSF_ALIVE;
	}
	if(subsys->subsys_snd_flags & SSSF_TURRET_ROTATION)
	{
		obj_snd_delete_type(ship_p->objnum, subsys->system_info->turret_base_rotation_snd, subsys);
		obj_snd_delete_type(ship_p->objnum, subsys->system_info->turret_gun_rotation_snd, subsys);
		subsys->subsys_snd_flags &= ~SSSF_TURRET_ROTATION;
	}
	if(subsys->subsys_snd_flags & SSSF_ROTATE)
	{
		obj_snd_delete_type(ship_p->objnum, subsys->system_info->rotation_snd, subsys);
		subsys->subsys_snd_flags &= ~SSSF_ROTATE;
	}
	if((subsys->system_info->dead_snd != -1) && !(subsys->subsys_snd_flags & SSSF_DEAD))
	{
		obj_snd_assign(ship_p->objnum, subsys->system_info->dead_snd, &subsys->system_info->pnt, 0, OS_SUBSYS_DEAD, subsys);
		subsys->subsys_snd_flags |= SSSF_DEAD;
	}
}

// Return weapon type that is associated with damaging_objp
// input:	damaging_objp		=>	object pointer responsible for damage
//	exit:		-1		=>	no weapon type is associated with damage object
//				>=0	=>	weapon type associated with damage object
int shiphit_get_damage_weapon(object *damaging_objp)
{
	int weapon_info_index = -1;

	if ( damaging_objp ) {
		switch(damaging_objp->type) {
		case OBJ_WEAPON:
			weapon_info_index = Weapons[damaging_objp->instance].weapon_info_index;
			break;
		case OBJ_SHOCKWAVE:
			weapon_info_index = shockwave_get_weapon_index(damaging_objp->instance);
			break;
		default:
			weapon_info_index = -1;
			break;
		}
	}

	return weapon_info_index;
}

//	Return range at which this object can apply damage.
//	Based on object type and subsystem type.
float subsys_get_range(object *other_obj, ship_subsys *subsys)
{
	float	range;

	Assert(subsys);	// Goober5000

	if ((other_obj) && (other_obj->type == OBJ_SHOCKWAVE)) {	// Goober5000 - check for NULL when via sexp
		range = shockwave_get_max_radius(other_obj->instance) * 0.75f;	//	Shockwaves were too lethal to subsystems.
	} else if ( subsys->system_info->type == SUBSYSTEM_TURRET ) {
		range = subsys->system_info->radius*3;
	} else {
		range = subsys->system_info->radius*2;
	}

	return range;
}

#define MAX_DEBRIS_SHARDS	16		// cap the amount of debris shards that fly off per hit

// Make some random debris particles.  Previous way was not very random.  Create debris 75% of the time.
// Don't worry about multiplayer since this debris is the small stuff that cannot collide
void create_subsys_debris(object *ship_obj, vec3d *hitpos)
{
	float show_debris = frand();
	
	if ( show_debris <= 0.75f ) {
		int ndebris;

		ndebris = (int)(show_debris * Detail.num_small_debris) + 1;			// number of pieces of debris to create

		if ( ndebris > MAX_DEBRIS_SHARDS )
			ndebris = MAX_DEBRIS_SHARDS;

		//mprintf(( "Damage = %.1f, ndebris=%d\n", show_debris, ndebris ));
		for (int i=0; i<ndebris; i++ )	{
			debris_create( ship_obj, -1, -1, hitpos, hitpos, 0, 1.0f );
		}
	}
}

void create_vaporize_debris(object *ship_obj, vec3d *hitpos)
{
	int ndebris;
	float show_debris = frand();

	ndebris = (int)(4.0f * ((0.5f + show_debris) * Detail.num_small_debris)) + 5;			// number of pieces of debris to create

	if ( ndebris > MAX_DEBRIS_SHARDS ) {
		ndebris = MAX_DEBRIS_SHARDS;
	}

	//mprintf(( "Damage = %.1f, ndebris=%d\n", show_debris, ndebris ));
	for (int i=0; i<ndebris; i++ )	{
		debris_create( ship_obj, -1, -1, hitpos, hitpos, 0, 1.4f );
	}
}

#define	MAX_SUBSYS_LIST	200 //DTP MAX SUBSYS LIST BUMPED FROM 32 to 200, ahmm 32???

typedef struct {
	float	dist;
	float	range;
	ship_subsys	*ptr;
} sublist;

// do_subobj_hit_stuff() is called when a collision is detected between a ship and something
// else.  This is where we see if any sub-objects on the ship should take damage.
//
//	Depending on where the collision occurs, the sub-system and surrounding hull will take 
// different amounts of damage.  The amount of damage a sub-object takes depending on how
// close the colliding object is to the center of the sub-object.  The remaining hull damage
// will be returned to the caller via the damage parameter.
//
//
// 0   -> 0.5 radius   : 100% subobject    0%  hull
// 0.5 -> 1.0 radius   :  50% subobject   50%  hull
// 1.0 -> 2.0 radius   :  25% subobject   75%  hull
//     >  2.0 radius   :   0% subobject  100%  hull
//
//
// The weapon damage is not neccesarily distributed evently between sub-systems when more than
// one sub-system is to take damage.  Whenever damage is to be assigned to a sub-system, the above
// percentages are used.  So, if more than one sub-object is taking damage, the second sub-system
// to be assigned damage will take less damage.  Eg. weapon hits in the 25% damage range of two
// subsytems, and the weapon damage is 12.  First subsystem takes 3 points damage.  Second subsystem
// will take 0.25*9 = 2.25 damage.  Should be close enough for most cases, and hull would receive 
// 0.75 * 9 = 6.75 damage.
//
//	Used to use the following constants, but now damage is linearly scaled up to 2x the subsystem
//	radius.  Same damage applied as defined by constants below.
//
//	Returns unapplied damage, which will probably be applied to the hull.
//
// Shockwave damage is handled here.  If other_obj->type == OBJ_SHOCKWAVE, it's a shockwave.
// apply the same damage to all subsystems.
//	Note: A negative damage number means to destroy the corresponding subsystem.  For example, call with -SUBSYSTEM_ENGINE to destroy engine.
//
//WMC - hull_should_apply armor means that the initial subsystem had no armor, so the hull should apply armor instead.

float do_subobj_hit_stuff(object *ship_obj, object *other_obj, vec3d *hitpos, float damage, bool *hull_should_apply_armor)
{
	vec3d			g_subobj_pos;
	float				damage_left, damage_if_hull;
	int				weapon_info_index;
	ship_subsys		*subsys;
	ship				*ship_p;
	sublist			subsys_list[MAX_SUBSYS_LIST];
	vec3d			hitpos2;

	//WMC - first, set this to damage if it isn't NULL, in case we want to return with no damage to subsystems
	if(hull_should_apply_armor != NULL) {
		*hull_should_apply_armor = true;
	}

	Assert(ship_obj);	// Goober5000 (but other_obj might be NULL via sexp)
	Assert(hitpos);		// Goober5000

	ship_p = &Ships[ship_obj->instance];

	//	Don't damage player subsystems in a training mission.
	if ( The_mission.game_type & MISSION_TYPE_TRAINING ) {
		if (ship_obj == Player_obj){
			return damage;
		}
	}

	//	Shockwave damage is applied like weapon damage.  It gets consumed.
	if ((other_obj != NULL) && (other_obj->type == OBJ_SHOCKWAVE))	// Goober5000 check for NULL
	{
		//	MK, 9/2/99.  Shockwaves do zero subsystem damage on small ships.
		// Goober5000 - added back in via flag
		if ((Ship_info[ship_p->ship_info_index].flags & (SIF_SMALL_SHIP)) && !(The_mission.ai_profile->flags & AIPF_SHOCKWAVES_DAMAGE_SMALL_SHIP_SUBSYSTEMS))
			return damage;
		else {
			damage_left = shockwave_get_damage(other_obj->instance) / 4.0f;
			damage_if_hull = damage_left;
		}
		hitpos2 = other_obj->pos;
	} else {
		damage_left = damage;
		damage_if_hull = damage;
		hitpos2 = *hitpos;
	}

	// scale subsystem damage if appropriate
	weapon_info_index = shiphit_get_damage_weapon(other_obj);	// Goober5000 - a NULL other_obj returns -1
	if ((weapon_info_index >= 0) && (other_obj->type == OBJ_WEAPON)) {
		if ( Weapon_info[weapon_info_index].wi_flags2 & WIF2_TRAINING ) {
			return damage_left;
		}
		damage_left *= Weapon_info[weapon_info_index].subsystem_factor;
		damage_if_hull *= Weapon_info[weapon_info_index].armor_factor;
	}


#ifndef NDEBUG
	float hitpos_dist = vm_vec_dist( hitpos, &ship_obj->pos );	
	if ( hitpos_dist > ship_obj->radius * 2.0f )	{
		mprintf(( "BOGUS HITPOS PASSED TO DO_SUBOBJ_HIT_STUFF (%.1f > %.1f)!\n", hitpos_dist, ship_obj->radius * 2.0f ));
		Error(LOCATION, "BOGUS HITPOS PASSED TO DO_SUBOBJ_HIT_STUFF (%.1f > %.1f)!\n", hitpos_dist, ship_obj->radius * 2.0f );
		// Int3();	// Get John ASAP!!!!  Someone passed a local coordinate instead of world for hitpos probably.
	}
#endif

	if (!global_damage) {
		create_subsys_debris(ship_obj, hitpos);
	}

	//	First, create a list of the N subsystems within range.
	//	Then, one at a time, process them in order.
	int	count = 0;
	for ( subsys=GET_FIRST(&ship_p->subsys_list); subsys != END_OF_LIST(&ship_p->subsys_list); subsys = GET_NEXT(subsys) )
	{
		//Deal with cheat correctly. If damage is the negative of the subsystem type, then we'll just kill the subsystem
		//See process_debug_keys() in keycontrol.cpp for details. 
		if (damage < 0.0f) {
			// single player or multiplayer
			Assert(Player_ai->targeted_subsys != NULL);
			if ( (subsys == Player_ai->targeted_subsys) && (subsys->current_hits > 0) ) {
				Assert(subsys->system_info->type == (int) -damage);
				if (!(subsys->flags & SSF_NO_AGGREGATE)) {
					ship_p->subsys_info[subsys->system_info->type].aggregate_current_hits -= subsys->current_hits;
					if (ship_p->subsys_info[subsys->system_info->type].aggregate_current_hits < 0.0f) {
						ship_p->subsys_info[subsys->system_info->type].aggregate_current_hits = 0.0f;
					}
				}
				subsys->current_hits = 0.0f;
				do_subobj_destroyed_stuff( ship_p, subsys, hitpos );
				continue;
			} else {
				continue;
			}
		}
		
		if (subsys->current_hits > 0.0f) {
			float	dist;

			// calculate the distance between the hit and the subsystem center
			get_subsystem_world_pos(ship_obj, subsys, &g_subobj_pos);
			dist = vm_vec_dist_quick(&hitpos2, &g_subobj_pos);

			float range = subsys_get_range(other_obj, subsys);

			if ( dist < range) {
				subsys_list[count].dist = dist;
				subsys_list[count].range = range;
				subsys_list[count].ptr = subsys;
				count++;

				if (count >= MAX_SUBSYS_LIST){
					break;
				}
			}
		}
	}

	int dmg_type_idx = -1;
	int parent_armor_flags = 0;

	if(ship_p->armor_type_idx > -1)
		parent_armor_flags = Armor_types[ship_p->armor_type_idx].flags;

	if (other_obj)
	{
		if(other_obj->type == OBJ_SHOCKWAVE) {
			dmg_type_idx = shockwave_get_damage_type_idx(other_obj->instance);
		} else if(other_obj->type == OBJ_WEAPON) {
			dmg_type_idx = Weapon_info[Weapons[other_obj->instance].weapon_info_index].damage_type_idx;
		} else if(other_obj->type == OBJ_BEAM) {
			dmg_type_idx = Weapon_info[beam_get_weapon_info_index(other_obj)].damage_type_idx;
		} else if(other_obj->type == OBJ_ASTEROID) {
			dmg_type_idx = Asteroid_info[Asteroids[other_obj->instance].asteroid_type].damage_type_idx;
		} else if(other_obj->type == OBJ_DEBRIS) {
			dmg_type_idx = Ships[Objects[Debris[other_obj->instance].source_objnum].instance].debris_damage_type_idx;
		} else if(other_obj->type == OBJ_SHIP) {
			dmg_type_idx = Ships[other_obj->instance].collision_damage_type_idx;
		}
	}

	//	Now scan the sorted list of subsystems in range.
	//	Apply damage to the nearest one first, subtracting off damage as we go.
	int	i, j;
	for (j=0; j<count; j++)
	{
		float	dist, range;
		ship_subsys	*subsys;

		int	min_index = -1;
		float	min_dist = 9999999.9f;

		for (i=0; i<count; i++) {
			if (subsys_list[i].dist < min_dist) {
				min_dist = subsys_list[i].dist;
				min_index = i;
			}
		}
		Assert(min_index != -1);

		float	damage_to_apply = 0.0f;
		subsys = subsys_list[min_index].ptr;
		range = subsys_list[min_index].range;
		dist = subsys_list[min_index].dist;
		subsys_list[min_index].dist = 9999999.9f;	//	Make sure we don't use this one again.

		Assert(range > 0.0f);	// Goober5000 - avoid div-0 below

		// only do this for the closest affected subsystem
		if ( (j == 0) && (!(parent_armor_flags & SAF_IGNORE_SS_ARMOR))) {
			if(subsys->armor_type_idx > -1)
			{
				damage = Armor_types[subsys->armor_type_idx].GetDamage(damage, dmg_type_idx);
				if(hull_should_apply_armor) {
					*hull_should_apply_armor = false;
				}
			}
		}
		//	HORRIBLE HACK!
		//	MK, 9/4/99
		//	When Helios bombs are dual fired against the Juggernaut in sm3-01 (FS2), they often
		//	miss their target.  There is code dating to FS1 in the collision code to detect that a bomb or
		//	missile has somehow missed its target.  It gets its lifeleft set to 0.1 and then it detonates.
		//	Unfortunately, the shockwave damage was cut by 4 above.  So boost it back up here.
		if ((dist < 10.0f) && ((other_obj) && (other_obj->type == OBJ_SHOCKWAVE))) {	// Goober5000 check for NULL
			damage_left *= 4.0f * Weapon_info[weapon_info_index].subsystem_factor;
			damage_if_hull *= 4.0f * Weapon_info[weapon_info_index].armor_factor;			
		}

//		if (damage_left > 100.0f)
//			nprintf(("AI", "Applying %7.3f damage to subsystem %7.3f units away.\n", damage_left, dist));

		if ( dist < range/2.0f ) {
			if (subsys->flags & SSF_DAMAGE_AS_HULL)
				damage_to_apply = damage_if_hull;
			else
				damage_to_apply = damage_left;
		} else if ( dist < range ) {
			if (subsys->flags & SSF_DAMAGE_AS_HULL)
				damage_to_apply = damage_if_hull * (1.0f - dist/range);
			else
				damage_to_apply = damage_left * (1.0f - dist/range);
		}

		// if we're not in CLIENT_NODAMAGE multiplayer mode (which is a the NEW way of doing things)
		if ( (damage_to_apply > 0.1f) && !(MULTIPLAYER_CLIENT) )
		{
			//	Decrease damage to subsystems to player ships.
			if (ship_obj->flags & OF_PLAYER_SHIP){
				damage_to_apply *= The_mission.ai_profile->subsys_damage_scale[Game_skill_level];
			}
		
			// Goober5000 - subsys guardian
			if (subsys->subsys_guardian_threshold > 0)
			{
				float min_subsys_strength = 0.01f * subsys->subsys_guardian_threshold * subsys->max_hits;
				if ( (subsys->current_hits - damage_to_apply) < min_subsys_strength ) {
					// find damage needed to take object to min subsys strength
					damage_to_apply = subsys->current_hits - min_subsys_strength;

					// make sure damage is positive
					damage_to_apply = MAX(0, damage_to_apply);
				}
			}

			// decrease the damage left to apply to the ship subsystems
			// WMC - since armor aborbs damage, subtract the amount of damage before we apply armor
			damage_left -= damage_to_apply;

			// if this subsystem doesn't carry damage then subtract it off of our total return
			if (subsys->system_info->flags & MSS_FLAG_CARRY_NO_DAMAGE) {
				if ((other_obj->type != OBJ_SHOCKWAVE) || (!(subsys->system_info->flags & MSS_FLAG_CARRY_SHOCKWAVE))) {
					float subsystem_factor = 0.0f;
					if ((weapon_info_index >= 0) && ((other_obj->type == OBJ_WEAPON) || (other_obj->type == OBJ_SHOCKWAVE))) {
						if (subsys->flags & SSF_DAMAGE_AS_HULL)
							subsystem_factor = Weapon_info[weapon_info_index].armor_factor;
						else
							subsystem_factor = Weapon_info[weapon_info_index].subsystem_factor;
					}
					if (subsystem_factor > 0.0f) 
						damage -= ((MIN(subsys->current_hits, damage_to_apply)) / subsystem_factor);
					else
						damage -= MIN(subsys->current_hits, damage_to_apply);
				}
			}

			//Apply armor to damage
			if (subsys->armor_type_idx >= 0) {
				damage_to_apply = Armor_types[subsys->armor_type_idx].GetDamage(damage_to_apply, dmg_type_idx);
			}

			subsys->current_hits -= damage_to_apply;
			if (!(subsys->flags & SSF_NO_AGGREGATE)) {
				ship_p->subsys_info[subsys->system_info->type].aggregate_current_hits -= damage_to_apply;
			}

			if (subsys->current_hits < 0.0f) {
				damage_left -= subsys->current_hits;
				if (!(subsys->flags & SSF_NO_AGGREGATE)) {
					ship_p->subsys_info[subsys->system_info->type].aggregate_current_hits -= subsys->current_hits;
				}
				subsys->current_hits = 0.0f;					// set to 0 so repair on subsystem takes immediate effect
			}

			if ( ship_p->subsys_info[subsys->system_info->type].aggregate_current_hits < 0.0f ){
				ship_p->subsys_info[subsys->system_info->type].aggregate_current_hits = 0.0f;
			}

			// multiplayer clients never blow up subobj stuff on their own
			if ( (subsys->current_hits <= 0.0f) && !MULTIPLAYER_CLIENT) {
				do_subobj_destroyed_stuff( ship_p, subsys, hitpos );
			}

			if (damage_left <= 0)	{ // no more damage to distribute, so stop checking
				damage_left = 0.0f;
				break;
			}
		}
//nprintf(("AI", "j=%i, sys = %s, dam = %6.1f, dam left = %6.1f, subhits = %5.0f\n", j, subsys->system_info->name, damage_to_apply, damage_left, subsys->current_hits));
	}

	if (damage < 0.0f) {
		damage = 0.0f;
	}

	//	Note: I changed this to return damage_left and it completely screwed up balance.
	//	It had taken a few MX-50s to destory an Anubis (with 40% hull), then it took maybe ten.
	//	So, I left it alone. -- MK, 4/15/98

	return damage;
}

// Store who/what killed the player, so we can tell the player how he died
void shiphit_record_player_killer(object *killer_objp, player *p)
{
	switch (killer_objp->type) {

	case OBJ_WEAPON:
		p->killer_objtype=OBJ_WEAPON;
		p->killer_weapon_index=Weapons[killer_objp->instance].weapon_info_index;
		p->killer_species = Ship_info[Ships[Objects[killer_objp->parent].instance].ship_info_index].species;

		if ( &Objects[killer_objp->parent] == Player_obj ) {
			// killed by a missile?
			if(Weapon_info[p->killer_weapon_index].subtype == WP_MISSILE){
				p->flags |= PLAYER_FLAGS_KILLED_SELF_MISSILES;
			} else {
				p->flags |= PLAYER_FLAGS_KILLED_SELF_UNKNOWN;
			}
		}

		// in multiplayer, record callsign of killer if killed by another player
		if ( (Game_mode & GM_MULTIPLAYER) && ( Objects[killer_objp->parent].flags & OF_PLAYER_SHIP) ) {
			int pnum;

			pnum = multi_find_player_by_object( &Objects[killer_objp->parent] );
			if ( pnum != -1 ) {
				strcpy_s(p->killer_parent_name, Net_players[pnum].m_player->callsign);
			} else {
				nprintf(("Network", "Couldn't find player object of weapon for killer of %s\n", p->callsign));
			}
		} else {
			strcpy_s(p->killer_parent_name, Ships[Objects[killer_objp->parent].instance].ship_name);
			end_string_at_first_hash_symbol(p->killer_parent_name);
		}
		break;

	case OBJ_SHOCKWAVE:
		p->killer_objtype=OBJ_SHOCKWAVE;
		p->killer_weapon_index = shockwave_get_weapon_index(killer_objp->instance);
		p->killer_species = Ship_info[Ships[Objects[killer_objp->parent].instance].ship_info_index].species;

		if ( &Objects[killer_objp->parent] == Player_obj ) {
			p->flags |= PLAYER_FLAGS_KILLED_SELF_SHOCKWAVE;
		}

		if ( (Game_mode & GM_MULTIPLAYER) && ( Objects[killer_objp->parent].flags & OF_PLAYER_SHIP) ) {
			int pnum;

			pnum = multi_find_player_by_object( &Objects[killer_objp->parent] );
			if ( pnum != -1 ) {
				strcpy_s(p->killer_parent_name, Net_players[pnum].m_player->callsign);
			} else {
				nprintf(("Network", "Couldn't find player object of shockwave for killer of %s\n", p->callsign));
			}
		} else {
			strcpy_s(p->killer_parent_name, Ships[Objects[killer_objp->parent].instance].ship_name);
			end_string_at_first_hash_symbol(p->killer_parent_name);
		}
		break;

	case OBJ_SHIP:
		p->killer_objtype=OBJ_SHIP;
		p->killer_weapon_index=-1;
		p->killer_species = Ship_info[Ships[killer_objp->instance].ship_info_index].species;

		if ( Ships[killer_objp->instance].flags & SF_EXPLODED ) {
			p->flags |= PLAYER_FLAGS_KILLED_BY_EXPLOSION;
		}

		if ( Ships[Objects[p->objnum].instance].wash_killed ) {
			p->flags |= PLAYER_FLAGS_KILLED_BY_ENGINE_WASH;
		}

		// in multiplayer, record callsign of killer if killed by another player
		if ( (Game_mode & GM_MULTIPLAYER) && (killer_objp->flags & OF_PLAYER_SHIP) ) {
			int pnum;

			pnum = multi_find_player_by_object( killer_objp );
			if ( pnum != -1 ) {
				strcpy_s(p->killer_parent_name, Net_players[pnum].m_player->callsign);
			} else {
				nprintf(("Network", "Couldn't find player object for killer of %s\n", p->callsign));
			}
		} else {
			strcpy_s(p->killer_parent_name, Ships[killer_objp->instance].ship_name);
			end_string_at_first_hash_symbol(p->killer_parent_name);
		}
		break;
	
	case OBJ_DEBRIS:
	case OBJ_ASTEROID:
		if ( killer_objp->type == OBJ_DEBRIS ) {
			p->killer_objtype = OBJ_DEBRIS;
		} else {
			p->killer_objtype = OBJ_ASTEROID;
		}
		p->killer_weapon_index=-1;
		p->killer_species = -1;
		p->killer_parent_name[0] = '\0';
		break;

	case OBJ_BEAM:
		int beam_obj;
		beam_obj = beam_get_parent(killer_objp);
		p->killer_species = -1;		
		p->killer_objtype = OBJ_BEAM;
		if(beam_obj != -1){			
			if((Objects[beam_obj].type == OBJ_SHIP) && (Objects[beam_obj].instance >= 0)){
				strcpy_s(p->killer_parent_name, Ships[Objects[beam_obj].instance].ship_name);
				end_string_at_first_hash_symbol(p->killer_parent_name);
			}
			p->killer_species = Ship_info[Ships[Objects[beam_obj].instance].ship_info_index].species;
		} else {			
			strcpy_s(p->killer_parent_name, "");
		}
		break;
	
	case OBJ_NONE:
		if ( Game_mode & GM_MULTIPLAYER ) {
			Int3();
		}
		p->killer_objtype=-1;
		p->killer_weapon_index=-1;
		p->killer_parent_name[0]=0;
		p->killer_species = -1;
		break;

	default:
		Int3();
		break;
	}
}

//	Say dead stuff.
void show_dead_message(object *ship_obj, object *other_obj)
{
	player *player_p;

	// not doing anything when a non player dies.
	if ( !(ship_obj->flags & OF_PLAYER_SHIP) ){
		return;
	}

	if(other_obj == NULL){
		return;
	}

	// Get a pointer to the player (we are assured a player ship was killed)
	if ( Game_mode & GM_NORMAL ) {
		player_p = Player;
	} else {
		// in multiplayer, get a pointer to the player that died.
		int pnum = multi_find_player_by_object( ship_obj );
		if ( pnum == -1 ) {
			//Int3();				// this condition is bad bad bad -- get Allender
			return;
		}
		player_p = Net_players[pnum].m_player;
	}

	// multiplayer clients should already have this information.
	if ( !MULTIPLAYER_CLIENT ){
		shiphit_record_player_killer( other_obj, player_p );
	}

	// display a hud message is the guy killed isn't me (multiplayer only)
	/*
	if ( (Game_mode & GM_MULTIPLAYER) && (ship_obj != Player_obj) ) {
		char death_text[256];

		player_generate_death_text( player_p, death_text );
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, death_text);
	}
	*/
}

/* JAS: THIS DOESN'T SEEM TO BE USED, SO I COMMENTED IT OUT
//	Apply damage to a ship, destroying if necessary, etc.
//	Returns portion of damage that exceeds ship shields, ie the "unused" portion of the damage.
//	Note: This system does not use the mesh shield.  It applies damage to the overall ship shield.
float apply_damage_to_ship(object *objp, float damage)
{
	float	_ss;

	add_shield_strength(objp, -damage);

	// check if shields are below 0%, if so take leftover damage and apply to ship integrity
	if ((_ss = get_shield_strength(objp)) < 0.0f ) {
		damage = -_ss;
		set_shield_strength(objp, 0.0f);
	} else
		damage = 0.0f;

	return damage;
}
*/

//	Do music processing for a ship hit.
void ship_hit_music(object *ship_obj, object *other_obj)
{
	Assert(ship_obj);	// Goober5000
	Assert(other_obj);	// Goober5000

	ship* ship_p = &Ships[ship_obj->instance];

	// Switch to battle track when a ship is hit by fire 
	//
	// If the ship hit has an AI class of none, it is a Cargo, NavBuoy or other non-aggressive
	// ship, so don't start the battle music	
	if (!stricmp(Ai_class_names[Ai_info[ship_p->ai_index].ai_class], NOX("none")))
		return;

	// Only start if ship hit and firing ship are from different teams
	int attackee_team, attacker_team;

	attackee_team = Ships[ship_obj->instance].team;

	switch ( other_obj->type )
	{
		case OBJ_SHIP:
			attacker_team = Ships[other_obj->instance].team;

			// Nonthreatening ship collided with ship, no big deal
			if ( !stricmp(Ai_class_names[Ai_info[Ships[other_obj->instance].ai_index].ai_class], NOX("none")) )
				return;

			break;

		case OBJ_WEAPON:
			// parent of weapon is object num of ship that fired it
			attacker_team = Ships[Objects[other_obj->parent].instance].team;	
			break;

		default:
			// Unexpected object type collided with ship, no big deal.
			return;
	}

	// start the music if it was an attacking ship
	if (iff_x_attacks_y(attacker_team, attackee_team))
		event_music_battle_start();
}

//	Make sparks fly off a ship.
// Currently used in misison_parse to create partially damaged ships.
// NOTE: hitpos is in model coordinates on the detail[0] submodel (highest detail hull)
// WILL NOT WORK RIGHT IF ON A ROTATING SUBMODEL
void ship_hit_sparks_no_rotate(object *ship_obj, vec3d *hitpos)
{
	ship		*ship_p = &Ships[ship_obj->instance];

	int n = ship_p->num_hits;
	if (n >= MAX_SHIP_HITS)	{
		n = rand() % MAX_SHIP_HITS;
	} else {
		ship_p->num_hits++;
	}

	// No rotation.  Just make the spark
	ship_p->sparks[n].pos = *hitpos;
	ship_p->sparks[n].submodel_num = -1;

	shipfx_emit_spark(ship_obj->instance, n);		// Create the first wave of sparks

	if ( n == 0 )	{
		ship_p->next_hit_spark = timestamp(0);		// when a hit spot will spark
	}
}

// find the max number of sparks allowed for ship
// limited for fighter by hull % others by radius.
int get_max_sparks(object* ship_obj)
{
	Assert(ship_obj->type == OBJ_SHIP);
	Assert((ship_obj->instance >= 0) && (ship_obj->instance < MAX_SHIPS));
	if(ship_obj->type != OBJ_SHIP){
		return 1;
	}
	if((ship_obj->instance < 0) || (ship_obj->instance >= MAX_SHIPS)){
		return 1;
	}

	ship *ship_p = &Ships[ship_obj->instance];
	ship_info* si = &Ship_info[ship_p->ship_info_index];
	if (si->flags & SIF_FIGHTER) {
		float hull_percent = ship_obj->hull_strength / ship_p->ship_max_hull_strength;

		if (hull_percent > 0.8f) {
			return 1;
		} else if (hull_percent > 0.3f) {
			return 2;
		} else {
			return 3;
		}
	} else {
		int num_sparks = (int) (ship_obj->radius * 0.08f);
		if (num_sparks < 3) {
			return 3;
		} else if (num_sparks > MAX_SHIP_HITS) {
			return MAX_SHIP_HITS;
		} else {
			return num_sparks;
		}
	}
}


// helper function to qsort, sorting spark pairs by distance
int spark_compare( const void *elem1, const void *elem2 )
{
	spark_pair *pair1 = (spark_pair *) elem1;
	spark_pair *pair2 = (spark_pair *) elem2;

	Assert(pair1->dist >= 0);
	Assert(pair2->dist >= 0);

	if ( pair1->dist <  pair2->dist ) {
		return -1;
	} else {
		return 1;
	}
}

// for big ships, when all spark slots are filled, make intelligent choice of one to be recycled
int choose_next_spark(object *ship_obj, vec3d *hitpos)
{
	int i, j, count, num_sparks, num_spark_pairs, spark_num;
	vec3d world_hitpos[MAX_SHIP_HITS];
	spark_pair spark_pairs[MAX_SPARK_PAIRS];
	ship *shipp = &Ships[ship_obj->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	// only choose next spark when all slots are full
	Assert(get_max_sparks(ship_obj) == Ships[ship_obj->instance].num_hits);

	// get num_sparks
	num_sparks = shipp->num_hits;
	Assert(num_sparks <= MAX_SHIP_HITS);

	// get num_spark_paris -- only sort these
	num_spark_pairs = (num_sparks * num_sparks - num_sparks) / 2;

	// get the world hitpos for all sparks
	bool model_started = false;
	for (spark_num=0; spark_num<num_sparks; spark_num++) {
		if (shipp->sparks[spark_num].submodel_num != -1) {
			if ( !model_started) {
				model_started = true;
				ship_model_start(ship_obj);
			}
			model_find_world_point(&world_hitpos[spark_num], &shipp->sparks[spark_num].pos, sip->model_num, shipp->sparks[spark_num].submodel_num, &ship_obj->orient, &ship_obj->pos);
		} else {
			// rotate sparks correctly with current ship orient
			vm_vec_unrotate(&world_hitpos[spark_num], &shipp->sparks[spark_num].pos, &ship_obj->orient);
			vm_vec_add2(&world_hitpos[spark_num], &ship_obj->pos);
		}
	}

	if (model_started) {
		ship_model_stop(ship_obj);
	}

	// check we're not making a spark in the same location as a current one
	for (i=0; i<num_sparks; i++) {
		float dist = vm_vec_dist_squared(&world_hitpos[i], hitpos);
		if (dist < 1) {
			return i;
		}
	}

	// not same location, so maybe do random recyling
	if (frand() > 0.5f) {
		return (rand() % num_sparks);
	}

	// initialize spark pairs
	for (i=0; i<num_spark_pairs; i++) {
		spark_pairs[i].index1 = 0;
		spark_pairs[i].index2 = 0;
		spark_pairs[i].dist = FLT_MAX;
	}

	// set spark pairs
	count = 0;
	for (i=1; i<num_sparks; i++) {
		for (j=0; j<i; j++) {
			spark_pairs[count].index1 = i;
			spark_pairs[count].index2 = j;
			spark_pairs[count++].dist = vm_vec_dist_squared(&world_hitpos[i], &world_hitpos[j]);
		}
	}
	Assert(count == num_spark_pairs);

	// sort pairs
	qsort(spark_pairs, count, sizeof(spark_pair), spark_compare);
	//mprintf(("Min spark pair dist %.1f\n", spark_pairs[0].dist));

	// look through the first few sorted pairs, counting number of indices of closest pair
	int index1 = spark_pairs[0].index1;
	int index2 = spark_pairs[0].index2;
	int count1 = 0;
	int count2 = 0;

	for (i=1; i<6; i++) {
		if (spark_pairs[i].index1 == index1) {
			count1++;
		}
		if (spark_pairs[i].index2 == index1) {
			count1++;
		}
		if (spark_pairs[i].index1 == index2) {
			count2++;
		}
		if (spark_pairs[i].index2 == index2) {
			count2++;
		}
	}

	// recycle spark which has most indices in sorted list of pairs
	if (count1 > count2) {
		return index1;
	} else {
		return index2;
	}
}


//	Make sparks fly off a ship.
void ship_hit_create_sparks(object *ship_obj, vec3d *hitpos, int submodel_num)
{
	vec3d	tempv;
	ship	*shipp = &Ships[ship_obj->instance];
	ship_info	*sip = &Ship_info[shipp->ship_info_index];

	int n, max_sparks;

	n = shipp->num_hits;
	max_sparks = get_max_sparks(ship_obj);

	if (n >= max_sparks)	{
		if (sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
			// large ship, choose intelligently
			n = choose_next_spark(ship_obj, hitpos);
		} else {
			// otherwise, normal choice
			n = rand() % max_sparks;
		}
	} else {
		shipp->num_hits++;
	}

	bool instancing = false;
	// decide whether to do instancing
	if (submodel_num != -1) {
		polymodel *pm = model_get(sip->model_num);
		if (pm->detail[0] != submodel_num) {
			// submodel is not hull
			// OPTIMIZE ... check if submodel can not rotate
			instancing = true;
		}
	}

	if (instancing) {
		// get the hit position in the subobject RF
		ship_model_start(ship_obj);
		vec3d temp_zero, temp_x, temp_y, temp_z;
		model_find_world_point(&temp_zero, &vmd_zero_vector, sip->model_num, submodel_num, &ship_obj->orient, &ship_obj->pos);
		model_find_world_point(&temp_x, &vmd_x_vector, sip->model_num, submodel_num, &ship_obj->orient, &ship_obj->pos);
		model_find_world_point(&temp_y, &vmd_y_vector, sip->model_num, submodel_num, &ship_obj->orient, &ship_obj->pos);
		model_find_world_point(&temp_z, &vmd_z_vector, sip->model_num, submodel_num, &ship_obj->orient, &ship_obj->pos);
		ship_model_stop(ship_obj);

		// find submodel x,y,z axes
		vm_vec_sub2(&temp_x, &temp_zero);
		vm_vec_sub2(&temp_y, &temp_zero);
		vm_vec_sub2(&temp_z, &temp_zero);

		// find displacement from submodel origin
		vec3d diff;
		vm_vec_sub(&diff, hitpos, &temp_zero);

		// find displacement from submodel origin in submodel RF
		shipp->sparks[n].pos.xyz.x = vm_vec_dotprod(&diff, &temp_x);
		shipp->sparks[n].pos.xyz.y = vm_vec_dotprod(&diff, &temp_y);
		shipp->sparks[n].pos.xyz.z = vm_vec_dotprod(&diff, &temp_z);
		shipp->sparks[n].submodel_num = submodel_num;
		shipp->sparks[n].end_time = timestamp(-1);
	} else {
		// Rotate hitpos into ship_obj's frame of reference.
		vm_vec_sub(&tempv, hitpos, &ship_obj->pos);
		vm_vec_rotate(&shipp->sparks[n].pos, &tempv, &ship_obj->orient);
		shipp->sparks[n].submodel_num = -1;
		shipp->sparks[n].end_time = timestamp(-1);
	}

	// Create the first wave of sparks
	shipfx_emit_spark(ship_obj->instance, n);

	if ( n == 0 )	{
		shipp->next_hit_spark = timestamp(0);		// when a hit spot will spark
	}
}

//	Called from ship_hit_kill() when we detect the player has been killed.
void player_died_start(object *killer_objp)
{
	nprintf(("Network", "starting my player death\n"));
	gameseq_post_event(GS_EVENT_DEATH_DIED);	
	
/*	vm_vec_scale_add(&Dead_camera_pos, &Player_obj->pos, &Player_obj->orient.fvec, -10.0f);
	vm_vec_scale_add2(&Dead_camera_pos, &Player_obj->orient.uvec, 3.0f);
	vm_vec_scale_add2(&Dead_camera_pos, &Player_obj->orient.rvec, 5.0f);
*/

	//	Create a good vector for the camera to move along during death sequence.
	object	*other_objp = NULL;

	// on multiplayer clients, there have been occasions where we haven't been able to determine
	// the killer of a ship (due to bogus/mismatched/timed-out signatures on the client side).  If
	// we don't know the killer, use the Player_obj as the other_objp for camera position.
	if ( killer_objp ) {
		switch (killer_objp->type) {
		case OBJ_WEAPON:
		case OBJ_SHOCKWAVE:
			other_objp = &Objects[killer_objp->parent];
			break;
		case OBJ_SHIP:
		case OBJ_DEBRIS:
		case OBJ_ASTEROID:
		case OBJ_NONE:	//	Something that just got deleted due to also dying -- it happened to me! --MK.		
			other_objp = killer_objp;
			break;

		case OBJ_BEAM:
			int beam_obj_parent;
			beam_obj_parent = beam_get_parent(killer_objp);
			if(beam_obj_parent == -1){
				other_objp = killer_objp;
			} else {
				other_objp = &Objects[beam_obj_parent];
			}
			break;

		default:
			Int3();		//	Killed by an object of a peculiar type.  What is it?
			other_objp = killer_objp;	//	Enable to continue, just in case we shipped it with this bug...
		}
	} else {
		other_objp = Player_obj;
	}

	vm_vec_add(&Original_vec_to_deader, &Player_obj->orient.vec.fvec, &Player_obj->orient.vec.rvec);
	vm_vec_scale(&Original_vec_to_deader, 2.0f);
	vm_vec_add2(&Original_vec_to_deader, &Player_obj->orient.vec.uvec);
	vm_vec_normalize(&Original_vec_to_deader);

	vec3d	vec_from_killer;
	vec3d	*side_vec;
	float		dist;

	Assert(other_objp != NULL);

	if (Player_obj == other_objp) {
		dist = 50.0f;
		vec_from_killer = Player_obj->orient.vec.fvec;
	} else {
		dist = vm_vec_normalized_dir(&vec_from_killer, &Player_obj->pos, &other_objp->pos);
	}

	if (dist > 100.0f)
		dist = 100.0f;
	vm_vec_scale_add(&Dead_camera_pos, &Player_obj->pos, &vec_from_killer, dist);

	float	dot = vm_vec_dot(&Player_obj->orient.vec.rvec, &vec_from_killer);
	if (fl_abs(dot) > 0.8f)
		side_vec = &Player_obj->orient.vec.fvec;
	else
		side_vec = &Player_obj->orient.vec.rvec;
	
	vm_vec_scale_add2(&Dead_camera_pos, side_vec, 10.0f);

	Player_ai->target_objnum = -1;		//	Clear targeting.  Otherwise, camera pulls away from player as soon as he blows up.

	// stop any playing emp effect
	emp_stop_local();
}


//#define	DEATHROLL_TIME						3000			//	generic deathroll is 3 seconds (3 * 1000 milliseconds) - Moved to ships.tbl
#define	MIN_PLAYER_DEATHROLL_TIME		1000			// at least one second deathroll for a player
#define	DEATHROLL_ROTVEL_CAP				6.3f			// maximum added deathroll rotvel in rad/sec (about 1 rev / sec)
#define	DEATHROLL_ROTVEL_MIN				0.8f			// minimum added deathroll rotvel in rad/sec (about 1 rev / 12 sec)
#define	DEATHROLL_MASS_STANDARD			50				// approximate mass of lightest ship
#define	DEATHROLL_VELOCITY_STANDARD	70				// deathroll rotvel is scaled according to ship velocity
#define	DEATHROLL_ROTVEL_SCALE			4				// constant determines how quickly deathroll rotvel is ramped up  (smaller is faster)

void saturate_fabs(float *f, float max)
{
	if ( fl_abs(*f) > max) {
		if (*f > 0)
			*f = max;
		else
			*f = -max;
	}
}

// function to do generic things when a ship explodes
void ship_generic_kill_stuff( object *objp, float percent_killed )
{
	float rotvel_mag;
	int	delta_time;
	ship	*sp;

	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHIPS );
	if((objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return;
	}
	sp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[sp->ship_info_index];

	ai_announce_ship_dying(objp);

	ship_stop_fire_primary(objp);	//mostly for stopping fighter beam looping sounds -Bobboau

	sp->flags |= SF_DYING;
	objp->phys_info.flags |= (PF_DEAD_DAMP | PF_REDUCED_DAMP);
	delta_time = (int) (sip->death_roll_base_time);

	//	For smaller ships, subtract off time proportional to excess damage delivered.
	if (objp->radius < BIG_SHIP_MIN_RADIUS)
		delta_time -=  (int) (1.01f - 4*percent_killed);

	//	Cut down cargo death rolls.  Looks a little silly. -- MK, 3/30/98.
	if (sip->flags & SIF_CARGO) {
		delta_time /= 4;
	}
	
	//	Prevent bogus timestamps.
	if (delta_time < 2)
		delta_time = 2;
	
	if (objp->flags & OF_PLAYER_SHIP) {
		//	Note: Kamikaze ships have no minimum death time.
		if (!(Ai_info[Ships[objp->instance].ai_index].ai_flags & AIF_KAMIKAZE) && (delta_time < MIN_PLAYER_DEATHROLL_TIME))
			delta_time = MIN_PLAYER_DEATHROLL_TIME;
	}

	//nprintf(("AI", "ShipHit.cpp: Frame %i, Gametime = %7.3f, Ship %s will die in %7.3f seconds.\n", Framecount, f2fl(Missiontime), Ships[objp->instance].ship_name, (float) delta_time/1000.0f));

	//	Make big ships have longer deathrolls.
	//	This is debug code by MK to increase the deathroll time so ships have time to evade the shockwave.
	//	Perhaps deathroll time should be specified in ships.tbl.
	float damage = ship_get_exp_damage(objp);

	if (damage >= 250.0f)
		delta_time += 3000 + (int)(damage*4.0f + 4.0f*objp->radius);

	if (Ai_info[sp->ai_index].ai_flags & AIF_KAMIKAZE)
		delta_time = 2;

	// Knossos gets 7-10 sec to die, time for "little" explosions
	if (Ship_info[sp->ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
		delta_time = 7000 + (int)(frand() * 3000.0f);
		Ship_info[sp->ship_info_index].explosion_propagates = 0;
	}

	// Goober5000 - ship-specific deathroll time, woot
	if (sp->special_exp_deathroll_time > 0)
	{
		delta_time = sp->special_exp_deathroll_time;

		// prevent bogus timestamps, per comment several lines up
		if (delta_time < 2)
			delta_time = 2;
	}

	sp->death_time = sp->final_death_time = timestamp(delta_time);	// Give him 3 secs to explode

	//SUSHI: What are the chances of an instant vaporization? Check the ship type first (objecttypes.tbl), then the ship (ships.tbl)
	ship_type_info *stp = &Ship_types[sip->class_type];
	float vapChance = stp->vaporize_chance;
	if (sip->vaporize_chance > 0)
		vapChance = sip->vaporize_chance;

	if (sp->flags & SF_VAPORIZE || frand() < vapChance) {
		// Assert(Ship_info[sp->ship_info_index].flags & SIF_SMALL_SHIP);

		// LIVE FOR 100 MS
		sp->final_death_time = timestamp(100);
	}

	//nprintf(("AI", "Time = %7.3f: final_death_time set to %7.3f\n", (float) timestamp_ticker/1000.0f, (float) sp->final_death_time/1000.0f));

	sp->pre_death_explosion_happened = 0;				// The little fireballs haven't came in yet.

	sp->next_fireball = timestamp(0);	//start one right away

	ai_deathroll_start(objp);

	// play death roll begin sound
	sp->death_roll_snd = snd_play_3d( &Snds[SND_DEATH_ROLL], &objp->pos, &View_position, objp->radius );
	if (objp == Player_obj)
		joy_ff_deathroll();

	// apply a whack
	//	rotational velocity proportional to original translational velocity, with a bit added in.
	//	Also, preserve half of original rotational velocity.

	// At standard speed (70) and standard mass (50), deathroll rotvel should be capped at DEATHROLL_ROTVEL_CAP
	// Minimum deathroll velocity is set	DEATHROLL_ROTVEL_MIN
	// At lower speed, lower death rotvel (scaled linearly)
	// At higher mass, lower death rotvel (scaled logarithmically)
	// variable scale calculates the deathroll rotational velocity magnitude
	float logval = (float) log10(objp->phys_info.mass / (0.05f*DEATHROLL_MASS_STANDARD));
	float velval = ((vm_vec_mag_quick(&objp->phys_info.vel) + 3.0f) / DEATHROLL_VELOCITY_STANDARD);
	float	p1 = (float) (DEATHROLL_ROTVEL_CAP - DEATHROLL_ROTVEL_MIN);

	rotvel_mag = (float) DEATHROLL_ROTVEL_MIN * 2.0f/(logval + 2.0f);
	rotvel_mag += (float) (p1 * velval/logval) * 0.75f;

	// set so maximum velocity from rotation is less than 200
	if (rotvel_mag*objp->radius > 150) {
		rotvel_mag = 150.0f / objp->radius;
	}

	if (object_is_dead_docked(objp)) {
		// don't change current rotvel
		sp->deathroll_rotvel = objp->phys_info.rotvel;
	} else {
		// if added rotvel is too random, we should decrease the random component, putting a const in front of the rotvel.
		sp->deathroll_rotvel = objp->phys_info.rotvel;
		sp->deathroll_rotvel.xyz.x += (frand() - 0.5f) * 2.0f * rotvel_mag;
		saturate_fabs(&sp->deathroll_rotvel.xyz.x, 0.75f*DEATHROLL_ROTVEL_CAP);
		sp->deathroll_rotvel.xyz.y += (frand() - 0.5f) * 3.0f * rotvel_mag;
		saturate_fabs(&sp->deathroll_rotvel.xyz.y, 0.75f*DEATHROLL_ROTVEL_CAP);
		sp->deathroll_rotvel.xyz.z += (frand() - 0.5f) * 6.0f * rotvel_mag;
		// make z component  2x larger than larger of x,y
		float largest_mag = MAX(fl_abs(sp->deathroll_rotvel.xyz.x), fl_abs(sp->deathroll_rotvel.xyz.y));
		if (fl_abs(sp->deathroll_rotvel.xyz.z) < 2.0f*largest_mag) {
			sp->deathroll_rotvel.xyz.z *= (2.0f * largest_mag / fl_abs(sp->deathroll_rotvel.xyz.z));
		}
		saturate_fabs(&sp->deathroll_rotvel.xyz.z, 0.75f*DEATHROLL_ROTVEL_CAP);
		// nprintf(("Physics", "Frame: %i rotvel_mag: %5.2f, rotvel: (%4.2f, %4.2f, %4.2f)\n", Framecount, rotvel_mag, sp->deathroll_rotvel.x, sp->deathroll_rotvel.y, sp->deathroll_rotvel.z));
	}

	
	// blow out his reverse thrusters. Or drag, same thing.
	objp->phys_info.rotdamp = (float) DEATHROLL_ROTVEL_SCALE / rotvel_mag;
	objp->phys_info.side_slip_time_const = 10000.0f;

	vm_vec_zero(&objp->phys_info.max_vel);		// make so he can't turn on his own VOLITION anymore.

	vm_vec_zero(&objp->phys_info.max_rotvel);	// make so he can't change speed on his own VOLITION anymore.

}

// called from ship_hit_kill if the ship is vaporized
void ship_vaporize(ship *shipp)
{
	object *ship_obj;

	// sanity
	Assert(shipp != NULL);
	if(shipp == NULL){
		return;
	}
	Assert((shipp->objnum >= 0) && (shipp->objnum < MAX_OBJECTS));
	if((shipp->objnum < 0) || (shipp->objnum >= MAX_OBJECTS)){
		return;
	}
	ship_obj = &Objects[shipp->objnum];

	// create debris shards
	create_vaporize_debris(ship_obj, &ship_obj->pos);
}

//	*ship_obj was hit and we've determined he's been killed!  By *other_obj!
void ship_hit_kill(object *ship_obj, object *other_obj, float percent_killed, int self_destruct)
{
	Assert(ship_obj);	// Goober5000 - but not other_obj, not only for sexp but also for self-destruct

	Script_system.SetHookObject("Self", ship_obj);
	if(other_obj != NULL) Script_system.SetHookObject("Killer", other_obj);

	if(Script_system.IsConditionOverride(CHA_DEATH, ship_obj))
	{
		//WMC - Do scripting stuff
		Script_system.RunCondition(CHA_DEATH, 0, NULL, ship_obj);
		Script_system.RemHookVar("Self");
		Script_system.RemHookVar("Killer");
		return;
	}

	ship *sp;
	char *killer_ship_name;
	int killer_damage_percent = 0;
	int killer_index = -1;
	object *killer_objp = NULL;

	sp = &Ships[ship_obj->instance];
	show_dead_message(ship_obj, other_obj);

	if (ship_obj == Player_obj) {
		player_died_start(other_obj);
	}

	// maybe vaporize him
	if(sp->flags & SF_VAPORIZE){
		ship_vaporize(sp);
	}

	// hehe
	extern void game_tst_mark(object *objp, ship *shipp);
	game_tst_mark(ship_obj, sp);

	// single player and multiplayer masters evaluate the scoring and kill stuff
	if ( !MULTIPLAYER_CLIENT ) {
		killer_index = scoring_eval_kill( ship_obj );

		// ship is destroyed -- send this event to the mission log stuff to record this event.  Try to find who
		// killed this ship.  scoring_eval_kill above should leave the obj signature of the ship who killed
		// this guy (or a -1 if no one got the kill).
		killer_ship_name = NULL;
		killer_damage_percent = -1;
		if ( killer_index >= 0 ) {
			object *objp;
			int sig;

			sig = sp->damage_ship_id[killer_index];
			for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
				if ( objp->signature == sig ){
					break;
				}
			}
			// if the object isn't around, the try to find the object in the list of ships which has exited			
			if ( objp != END_OF_LIST(&obj_used_list) ) {
				Assert ( (objp->type == OBJ_SHIP ) || (objp->type == OBJ_GHOST) );					// I suppose that this should be true
				killer_ship_name = Ships[objp->instance].ship_name;

				killer_objp = objp;
			} else {
				int ei;

				ei = ship_find_exited_ship_by_signature( sig );
				if ( ei != -1 ){
					killer_ship_name = Ships_exited[ei].ship_name;
				}
			}
			killer_damage_percent = (int)(sp->damage_ship[killer_index]/sp->total_damage_received * 100.0f);
		}		

		if(!self_destruct){
			// multiplayer
			if(Game_mode & GM_MULTIPLAYER){
				char name1[256] = "";
				char name2[256] = "";
				int np_index;

				// get first name				
				np_index = multi_find_player_by_object(ship_obj);				
				if((np_index >= 0) && (np_index < MAX_PLAYERS) && (Net_players[np_index].m_player != NULL)){
					strcpy_s(name1, Net_players[np_index].m_player->callsign);
				} else {
					strcpy_s(name1, sp->ship_name);
				}

				// argh
				if((killer_objp != NULL) || (killer_ship_name != NULL)){

					// second name
					if(killer_objp == NULL){
						strcpy_s(name2, killer_ship_name);
					} else {
						np_index = multi_find_player_by_object(killer_objp);
						if((np_index >= 0) && (np_index < MAX_PLAYERS) && (Net_players[np_index].m_player != NULL)){
							strcpy_s(name2, Net_players[np_index].m_player->callsign);
						} else {
							strcpy_s(name2, killer_ship_name);
						}
					}					
				}

				mission_log_add_entry(LOG_SHIP_DESTROYED, name1, name2, killer_damage_percent);
			} else {
				// DKA: 8/23/99 allow message log in single player with no killer name
				//if(killer_ship_name != NULL){
				mission_log_add_entry(LOG_SHIP_DESTROYED, sp->ship_name, killer_ship_name, killer_damage_percent);
				//}
			}
		}

		// maybe praise the player for this kill
		if ( (killer_damage_percent > 10) && (other_obj != NULL) ) {
			if (other_obj->parent_sig == Player_obj->signature) {
				ship_maybe_praise_player(sp);
			}
			else if ((other_obj->parent_type == OBJ_SHIP) || (other_obj->parent_type == OBJ_START))  {
				ship_maybe_praise_self(sp, &Ships[Objects[other_obj->parent].instance]);
			}
		}
	}

	ship_generic_kill_stuff( ship_obj, percent_killed );

	// mwa -- removed 2/25/98 -- why is this here?  ship_obj->flags &= ~(OF_PLAYER_SHIP);
	// if it is for observers, must deal with it a separate way!!!!
	if ( MULTIPLAYER_MASTER ) {
		// check to see if this ship needs to be respawned
		multi_respawn_check(ship_obj);		
			
		// send the kill packet to all players
		// maybe send vaporize packet to all players
		send_ship_kill_packet( ship_obj, other_obj, percent_killed, self_destruct );
	}

	// if a non-player is dying, play a scream
	if ( !(ship_obj->flags & OF_PLAYER_SHIP) ) {
		ship_maybe_scream(sp);
	}

	// if the player is dying, have wingman lament
	if ( (ship_obj == Player_obj) ) {
		ship_maybe_lament();
	}

	Script_system.RunCondition(CHA_DEATH, 0, NULL, ship_obj);
	Script_system.RemHookVars(2, "Self", "Killer");
}

// function to simply explode a ship where it is currently at
void ship_self_destruct( object *objp )
{	
	Assert ( objp->type == OBJ_SHIP );

	// try and find a player
	if((Game_mode & GM_MULTIPLAYER) && (multi_find_player_by_object(objp) >= 0)){
		int np_index = multi_find_player_by_object(objp);
		if((np_index >= 0) && (np_index < MAX_PLAYERS) && (Net_players[np_index].m_player != NULL)){
			mission_log_add_entry(LOG_SELF_DESTRUCTED, Net_players[np_index].m_player->callsign, NULL );
		} else {
			mission_log_add_entry(LOG_SELF_DESTRUCTED, Ships[objp->instance].ship_name, NULL );
		}
	} else {
		mission_log_add_entry(LOG_SELF_DESTRUCTED, Ships[objp->instance].ship_name, NULL );
	}
	
	// check to see if this ship needs to be respawned
	if(MULTIPLAYER_MASTER){
		// player ship?
		int np_index = multi_find_player_by_object(objp);
		if((np_index >= 0) && (np_index < MAX_PLAYERS) && MULTI_CONNECTED(Net_players[np_index]) && (Net_players[np_index].m_player != NULL)){
			char msg[512] = "";
			sprintf(msg, "%s %s", Net_players[np_index].m_player->callsign, XSTR("Self destructed", 1476));

			// send a message
			send_game_chat_packet(Net_player, msg, MULTI_MSG_ALL, NULL, NULL, 2);

			// printf
			if(!(Game_mode & GM_STANDALONE_SERVER)){
				HUD_printf(msg);
			}
		}
	}

	// self destruct
	ship_hit_kill(objp, NULL, 1.0f, 1);	
}

extern int Homing_hits, Homing_misses;

// Call this instead of physics_apply_whack directly to 
// deal with two docked ships properly.
// Goober5000 - note... hit_pos is in *local* coordinates
void ship_apply_whack(vec3d *force, vec3d *hit_pos, object *objp)
{
	if (objp == Player_obj) {
		nprintf(("Sandeep", "Playing stupid joystick effect\n"));
		vec3d test;
		vm_vec_unrotate(&test, force, &objp->orient);

		game_whack_apply( -test.xyz.x, -test.xyz.y );
	}

	if (object_is_docked(objp))
	{
		float overall_mass = dock_calc_total_docked_mass(objp);

		// Goober5000 - this code attempts to account properly for whacking a docked object as one mass.
		// It isn't perfect, because physics doesn't completely account for it (particularly because it
		// still uses the moment of inertia for the whacked object, not for all objects).  Commenting
		// the bracketed code restores Volition's code, but it doesn't calculate the correct torque.
		{
			vec3d world_hit_pos, world_center_pos;

			// calc world hit pos of the hit ship
			vm_vec_unrotate(&world_hit_pos, hit_pos, &objp->orient);
			vm_vec_add2(&world_hit_pos, &objp->pos);

			// calc overall world center of ships
			dock_calc_docked_center(&world_center_pos, objp);

			// the new hitpos is the vector from world center to world hitpos
			vm_vec_sub(hit_pos, &world_hit_pos, &world_center_pos);
		}

		// whack it
		physics_apply_whack(force, hit_pos, &objp->phys_info, &objp->orient, overall_mass);
	}
	else
	{
		physics_apply_whack(force, hit_pos, &objp->phys_info, &objp->orient, objp->phys_info.mass);
	}					
}

// If a ship is dying and it gets hit, shorten its deathroll.
//	But, if it's a player, don't decrease below MIN_PLAYER_DEATHROLL_TIME
void shiphit_hit_after_death(object *ship_obj, float damage)
{
	float	percent_killed;
	int	delta_time, time_remaining;
	ship	*shipp = &Ships[ship_obj->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	// Since the explosion has two phases (final_death_time and really_final_death_time)
	// we should only shorten the deathroll time if that is the phase we're in.
	// And you can tell by seeing if the timestamp is valid, since it gets set to
	// invalid after it does the first large explosion.
	if ( !timestamp_valid(shipp->final_death_time) )	{
		return;
	}

	// Don't adjust vaporized ship
	if (shipp->flags & SF_VAPORIZE) {
		return;
	}

	//	Don't shorten deathroll on very large ships.
	if (ship_obj->radius > BIG_SHIP_MIN_RADIUS)
		return;

	percent_killed = damage/shipp->ship_max_hull_strength;
	if (percent_killed > 1.0f)
		percent_killed = 1.0f;

	delta_time = (int) (4 * sip->death_roll_base_time * percent_killed);
	time_remaining = timestamp_until(shipp->final_death_time);

	//nprintf(("AI", "Gametime = %7.3f, Time until %s dies = %7.3f, delta = %7.3f\n", f2fl(Missiontime), Ships[ship_obj->instance].ship_name, (float)time_remaining/1000.0f, delta_time));
	if (ship_obj->flags & OF_PLAYER_SHIP)
		if (time_remaining < MIN_PLAYER_DEATHROLL_TIME)
			return;

	// nprintf(("AI", "Subtracting off %7.3f seconds from deathroll, reducing to %7.3f\n", (float) delta_time/1000.0f, (float) (time_remaining - delta_time)/1000.0f));

	delta_time = time_remaining - delta_time;
	if (ship_obj->flags & OF_PLAYER_SHIP)
		if (delta_time < MIN_PLAYER_DEATHROLL_TIME)
			delta_time = MIN_PLAYER_DEATHROLL_TIME;

	//	Prevent bogus timestamp.
	if (delta_time < 2)
		delta_time = 2;

	shipp->final_death_time = timestamp(delta_time);	// Adjust time until explosion.
}

MONITOR( ShipHits )
MONITOR( ShipNumDied )

int maybe_shockwave_damage_adjust(object *ship_obj, object *other_obj, float *damage)
{
	ship_subsys *subsys;
	ship *shipp;
	float dist, nearest_dist = FLT_MAX;
	vec3d g_subobj_pos;
	float max_damage;
	float inner_radius, outer_radius;

	Assert(ship_obj);	// Goober5000 (but not other_obj in case of sexp)
	Assert(damage);		// Goober5000

	Assert(ship_obj->type == OBJ_SHIP);

	if (!other_obj) {
		return 0;
	}

	if (other_obj->type != OBJ_SHOCKWAVE) {
		return 0;
	}

	if (!(Ship_info[Ships[ship_obj->instance].ship_info_index].flags & SIF_HUGE_SHIP)) {
		return 0;
	}

	shipp = &Ships[ship_obj->instance];

	// find closest subsystem distance to shockwave origin
	for (subsys=GET_FIRST(&shipp->subsys_list); subsys != END_OF_LIST(&shipp->subsys_list); subsys = GET_NEXT(subsys) ) {
		get_subsystem_world_pos(ship_obj, subsys, &g_subobj_pos);
		dist = vm_vec_dist_quick(&g_subobj_pos, &other_obj->pos);

		if (dist < nearest_dist) {
			nearest_dist = dist;
		}
	}

	// get max damage and adjust if needed to account for shockwave created from destroyed weapon
	max_damage = shockwave_get_damage(other_obj->instance);
	if (shockwave_get_flags(other_obj->instance) & SW_WEAPON_KILL) {
		max_damage *= 4.0f;
	}

	outer_radius = shockwave_get_max_radius(other_obj->instance);
	inner_radius = shockwave_get_min_radius(other_obj->instance);

	// scale damage
	// floor of 25%, max if within inner_radius, linear between
	if (nearest_dist > outer_radius) {
		*damage = max_damage / 4.0f;
	} else if (nearest_dist < inner_radius) {
		*damage = max_damage;
	} else {
		*damage = max_damage * (1.0f - 0.75f * (nearest_dist - inner_radius) / (outer_radius - inner_radius));
	}

	return 1;
}

// ------------------------------------------------------------------------
// ship_do_damage()
//
// Do damage assessment on a ship.    This should only be called
// internally by ship_apply_global_damage and ship_apply_local_damage
//
// 
//	input:	ship_obj		=>		object pointer for ship receiving damage
//				other_obj	=>		object pointer to object causing damage
//				hitpos		=>		impact world pos on the ship 
//				TODO:	get a better value for hitpos
//				damage		=>		damage to apply to the ship
//				quadrant	=> which part of shield takes damage, -1 if not shield hit
//				wash_damage	=>		1 if damage is done by engine wash
// Goober5000 - sanity checked this whole function in the case that other_obj is NULL, which
// will happen with the explosion-effect sexp
void ai_update_lethality(object *ship_obj, object *weapon_obj, float damage);
static void ship_do_damage(object *ship_obj, object *other_obj, vec3d *hitpos, float damage, int quadrant, int wash_damage=0)
{
//	mprintf(("doing damage\n"));

	ship *shipp;	
	float subsystem_damage = damage;			// damage to be applied to subsystems
	int other_obj_is_weapon;
	int other_obj_is_beam;
	int other_obj_is_shockwave;
	int other_obj_is_asteroid;
	int other_obj_is_debris;
	int other_obj_is_ship;

	Assert(ship_obj);	// Goober5000
	Assert(hitpos);		// Goober5000

	Assert(ship_obj->instance >= 0);
	Assert(ship_obj->type == OBJ_SHIP);
	shipp = &Ships[ship_obj->instance];

	// maybe adjust damage done by shockwave for BIG|HUGE
	maybe_shockwave_damage_adjust(ship_obj, other_obj, &damage);

	// Goober5000 - check to see what other_obj is
	if (other_obj)
	{
		other_obj_is_weapon = ((other_obj->type == OBJ_WEAPON) && (other_obj->instance >= 0) && (other_obj->instance < MAX_WEAPONS));
		other_obj_is_beam = ((other_obj->type == OBJ_BEAM) && (other_obj->instance >= 0) && (other_obj->instance < MAX_BEAMS));
		other_obj_is_shockwave = ((other_obj->type == OBJ_SHOCKWAVE) && (other_obj->instance >= 0) && (other_obj->instance < MAX_SHOCKWAVES));
		other_obj_is_asteroid = ((other_obj->type == OBJ_ASTEROID) && (other_obj->instance >= 0) && (other_obj->instance < MAX_ASTEROIDS));
		other_obj_is_debris = ((other_obj->type == OBJ_DEBRIS) && (other_obj->instance >= 0) && (other_obj->instance < MAX_DEBRIS_PIECES));
		other_obj_is_ship = ((other_obj->type == OBJ_SHIP) && (other_obj->instance >= 0) && (other_obj->instance < MAX_SHIPS));
	}
	else
	{
		other_obj_is_weapon = 0;
		other_obj_is_beam = 0;
		other_obj_is_shockwave = 0;
		other_obj_is_asteroid = 0;
		other_obj_is_debris = 0;
		other_obj_is_ship = 0;
	}

	// update lethality of ship doing damage - modified by Goober5000
	if (other_obj_is_weapon || other_obj_is_shockwave) {
		ai_update_lethality(ship_obj, other_obj, damage);
	}

	// if this is a weapon
	if (other_obj_is_weapon)
		damage *= weapon_get_damage_scale(&Weapon_info[Weapons[other_obj->instance].weapon_info_index], other_obj, ship_obj);

	MONITOR_INC( ShipHits, 1 );

	//	Don't damage player ship in the process of warping out.
	if ( Player->control_mode >= PCM_WARPOUT_STAGE2 )	{
		if ( ship_obj == Player_obj ){
			return;
		}
	}

	if ( other_obj_is_weapon ) {		
		// for tvt and dogfight missions, don't scale damage
		if( (Game_mode & GM_MULTIPLAYER) && ((Netgame.type_flags & NG_TYPE_TEAM) || (Netgame.type_flags & NG_TYPE_DOGFIGHT)) ){
		} 
		else {
			// Do a little "skill" balancing for the player in single player and coop multiplayer
			if (ship_obj->flags & OF_PLAYER_SHIP)	{
				damage *= The_mission.ai_profile->player_damage_scale[Game_skill_level];
				subsystem_damage *= The_mission.ai_profile->player_damage_scale[Game_skill_level];
			}		
		}
	}

	// if this is not a laser, or i'm not a multiplayer client
	// apply pain to me

	// Goober5000: make sure other_obj doesn't cause a read violation!
	if (other_obj && !(Ship_info[Ships[Player_obj->instance].ship_info_index].flags2 & SIF2_NO_PAIN_FLASH))
	{
		// For the record, ship_hit_pain seems to simply be the red flash that appears
		// on the screen when you're hit.
		int special_check = !MULTIPLAYER_CLIENT;

		// now the actual checks
		if (other_obj->type == OBJ_BEAM)
		{
			Assert((beam_get_weapon_info_index(other_obj) >= 0) && (beam_get_weapon_info_index(other_obj) < Num_weapon_types));
			if (((Weapon_info[beam_get_weapon_info_index(other_obj)].subtype != WP_LASER) || special_check) && (Player_obj != NULL) && (ship_obj == Player_obj))
			{
				ship_hit_pain(damage);
			}	
		}
		if (other_obj->type == OBJ_WEAPON)
		{
			Assert((Weapons[other_obj->instance].weapon_info_index > -1) && (Weapons[other_obj->instance].weapon_info_index < Num_weapon_types));
			if (((Weapon_info[Weapons[other_obj->instance].weapon_info_index].subtype != WP_LASER) || special_check) && (Player_obj != NULL) && (ship_obj == Player_obj))
			{
				ship_hit_pain(damage);
			}
		}
	}	// read violation sanity check


	// If the ship is invulnerable, do nothing
	if (ship_obj->flags & OF_INVULNERABLE)	{
		return;
	}

	//	if ship is already dying, shorten deathroll.
	if (shipp->flags & SF_DYING) {
		shiphit_hit_after_death(ship_obj, damage);
		return;
	}
	
	//	If we hit the shield, reduce it's strength and found
	// out how much damage is left over.
	if ( quadrant >= 0 && !(ship_obj->flags & OF_NO_SHIELDS) )	{
//		mprintf(("applying damage ge to shield\n"));
		float shield_factor = -1.0f;
		int	weapon_info_index;

		weapon_info_index = shiphit_get_damage_weapon(other_obj);
		if ( weapon_info_index >= 0 ) {
			shield_factor = Weapon_info[weapon_info_index].shield_factor;
		}

		if ( shield_factor >= 0 ) {
			damage *= shield_factor;
			subsystem_damage *= shield_factor;
		}

		if ( damage > 0 ) {

			float piercing_pct = 0.0f;
			int dmg_type_idx = -1;

			//Do armor stuff
			if(other_obj_is_weapon) {
				dmg_type_idx = Weapon_info[Weapons[other_obj->instance].weapon_info_index].damage_type_idx;
			} else if(other_obj_is_beam) {
				dmg_type_idx = Weapon_info[beam_get_weapon_info_index(other_obj)].damage_type_idx;
			} else if(other_obj_is_shockwave) {
				dmg_type_idx = shockwave_get_damage_type_idx(other_obj->instance);
			} else if(other_obj_is_asteroid) {
				dmg_type_idx = Asteroid_info[Asteroids[other_obj->instance].asteroid_type].damage_type_idx;
			} else if(other_obj_is_debris) {
				dmg_type_idx = Ships[Objects[Debris[other_obj->instance].source_objnum].instance].debris_damage_type_idx;
			} else if(other_obj_is_ship) {
				dmg_type_idx = Ships[other_obj->instance].collision_damage_type_idx;
			}
				
			if(shipp->shield_armor_type_idx != -1)
			{
				piercing_pct = Armor_types[shipp->shield_armor_type_idx].GetShieldPiercePCT(dmg_type_idx);
			}
			
			float pre_shield = damage;
			float pre_shield_ss = subsystem_damage;

			if (piercing_pct > 0.0f) {
				damage = pre_shield * (1.0f - piercing_pct);
			}

			if(shipp->shield_armor_type_idx != -1)
			{
				damage = Armor_types[shipp->shield_armor_type_idx].GetDamage(damage, dmg_type_idx);
			}

			damage = apply_damage_to_shield(ship_obj, quadrant, damage);

			if(damage > 0.0f){
				subsystem_damage *= (damage / pre_shield);
			} else {
				subsystem_damage = 0.0f;
			}

			if (piercing_pct > 0.0f) {
				damage += (piercing_pct * pre_shield);
				subsystem_damage += (piercing_pct * pre_shield_ss);
			}
		}

		// if shield damage was increased, don't carry over leftover damage at scaled level
		if ( shield_factor > 1 ) {
			damage /= shield_factor;

			subsystem_damage /= shield_factor;
		}
	}
			
	// Apply leftover damage to the ship's subsystem and hull.
	if ( (damage > 0.0f) || (subsystem_damage > 0.0f) )	{
		int	weapon_info_index;
		int armor_flags = 0;		
		float pre_subsys = subsystem_damage;
		bool apply_hull_armor = true;

		subsystem_damage = do_subobj_hit_stuff(ship_obj, other_obj, hitpos, subsystem_damage, &apply_hull_armor);

		if(shipp->armor_type_idx != -1)
		{
			armor_flags = Armor_types[shipp->armor_type_idx].flags;
		}

		if(subsystem_damage > 0.0f){
			damage *= (subsystem_damage / pre_subsys);
		} else {
			damage = 0.0f;
		}

		//Do armor stuff
		if (apply_hull_armor)
		{
			int dmg_type_idx = -1;
			if(other_obj_is_weapon) {
				dmg_type_idx = Weapon_info[Weapons[other_obj->instance].weapon_info_index].damage_type_idx;
			} else if(other_obj_is_beam) {
				dmg_type_idx = Weapon_info[beam_get_weapon_info_index(other_obj)].damage_type_idx;
			} else if(other_obj_is_shockwave) {
				dmg_type_idx = shockwave_get_damage_type_idx(other_obj->instance);
			} else if(other_obj_is_asteroid) {
				dmg_type_idx = Asteroid_info[Asteroids[other_obj->instance].asteroid_type].damage_type_idx;
			} else if(other_obj_is_debris) {
				dmg_type_idx = Ships[Objects[Debris[other_obj->instance].source_objnum].instance].debris_damage_type_idx;
			} else if(other_obj_is_ship) {
				dmg_type_idx = Ships[other_obj->instance].collision_damage_type_idx;
			}
			
			if(shipp->armor_type_idx != -1)
			{
				damage = Armor_types[shipp->armor_type_idx].GetDamage(damage, dmg_type_idx);
			}
		}

		// continue with damage?
		if(damage > 0.0){
			weapon_info_index = shiphit_get_damage_weapon(other_obj);	// Goober5000 - a NULL other_obj returns -1
			if ( weapon_info_index >= 0 ) {
				if (Weapon_info[weapon_info_index].wi_flags & WIF_PUNCTURE) {
					damage /= 4;
				}

				damage *= Weapon_info[weapon_info_index].armor_factor;
			}

			// if ship is flagged as can not die, don't let it die
			if (shipp->ship_guardian_threshold > 0) {
				float min_hull_strength = 0.01f * shipp->ship_guardian_threshold * shipp->ship_max_hull_strength;
				if ( (ship_obj->hull_strength - damage) < min_hull_strength ) {
					// find damage needed to take object to min hull strength
					damage = ship_obj->hull_strength - min_hull_strength;

					// make sure damage is positive
					damage = MAX(0, damage);
				}
			}

			// multiplayer clients don't do damage
			if (((Game_mode & GM_MULTIPLAYER) && MULTIPLAYER_CLIENT)) {
			} else {
				// Check if this is simulated damage.
				weapon_info_index = shiphit_get_damage_weapon(other_obj);
				if ( weapon_info_index >= 0 ) {
					if (Weapon_info[weapon_info_index].wi_flags2 & WIF2_TRAINING) {
//						diag_printf2("Simulated Hull for Ship %s hit, dropping from %.32f to %d.\n", shipp->ship_name, (int) ( ship_obj->sim_hull_strength * 100 ), (int) ( ( ship_obj->sim_hull_strength - damage ) * 100 ) );
						ship_obj->sim_hull_strength -= damage;
						ship_obj->sim_hull_strength = MAX( 0, ship_obj->sim_hull_strength );
						return;
					}
				}
				ship_obj->hull_strength -= damage;		
			}

			// let damage gauge know that player ship just took damage
			if ( Player_obj == ship_obj ) {
				hud_gauge_popup_start(HUD_DAMAGE_GAUGE, 5000);
			}
		
			// DB - removed 1/12/99 - scoring code properly bails if MULTIPLAYER_CLIENT
			// in multiplayer, if I am not the host, get out of this function here!!
			//if ( MULTIPLAYER_CLIENT ) {
				//return;
			//}		

			if (other_obj)
			{
				switch (other_obj->type)
				{
					case OBJ_SHOCKWAVE:
						scoring_add_damage(ship_obj,other_obj,damage);
						break;
					case OBJ_ASTEROID:
						// don't call scoring for asteroids
						break;
					case OBJ_WEAPON:
						if((other_obj->parent < 0) || (other_obj->parent >= MAX_OBJECTS)){
							scoring_add_damage(ship_obj, NULL, damage);
						} else {
							scoring_add_damage(ship_obj, &Objects[other_obj->parent], damage);
						}
						break;
					case OBJ_BEAM://give kills for fighter beams-Bobboau
					{
						int bobjn = beam_get_parent(other_obj);

						// Goober5000 - only count beams fired by fighters or bombers unless the ai profile says different
						if (bobjn >= 0)
						{
							if ( !(The_mission.ai_profile->flags & AIPF_INCLUDE_BEAMS_IN_STAT_CALCS) && 
								 !(Ship_info[Ships[Objects[bobjn].instance].ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER)) && 
								 !(Objects[bobjn].flags & OF_PLAYER_SHIP) ) {
								bobjn = -1;
							}
						}

						if(bobjn == -1){
							scoring_add_damage(ship_obj, NULL, damage);
						} else {
							scoring_add_damage(ship_obj, &Objects[bobjn], damage);
						}
						break;
					  }
					default:
						break;
				}
			}	// other_obj

			if (ship_obj->hull_strength <= 0.0f) {
				MONITOR_INC( ShipNumDied, 1 );

				ship_info	*sip = &Ship_info[shipp->ship_info_index];

				// If massive beam hitting small ship, vaporize  otherwise normal damage pipeline
				// Only vaporize once
				// multiplayer clients should skip this
				if(!MULTIPLAYER_CLIENT) {
					if ( !(shipp->flags & SF_VAPORIZE) ) {
						// Only small ships can be vaporized
						if (sip->flags & (SIF_SMALL_SHIP)) {
							if (other_obj) {	// Goober5000 check for NULL
								if (other_obj->type == OBJ_BEAM)
								{
									int beam_weapon_info_index = beam_get_weapon_info_index(other_obj);
									if ( (beam_weapon_info_index > -1) && (Weapon_info[beam_weapon_info_index].wi_flags & (WIF_HUGE)) ) {
										// Flag as vaporized
										shipp->flags |= SF_VAPORIZE;
									}
								}
							}
						}
					}
				}
				
				// maybe engine wash death
				if (wash_damage) {
					shipp->wash_killed = 1;
				}

				float percent_killed = -get_hull_pct(ship_obj);
				if (percent_killed > 1.0f){
					percent_killed = 1.0f;
				}

				if ( !(shipp->flags & SF_DYING) && !MULTIPLAYER_CLIENT) {  // if not killed, then kill
					ship_hit_kill(ship_obj, other_obj, percent_killed, 0);
				}
			}
		}
	}

	// if the hitting object is a weapon, maybe do some fun stuff here
	if(other_obj_is_weapon)
	{
		weapon_info *wip;
		Assert(other_obj->instance >= 0);
		if(other_obj->instance < 0){
			return;
		}
		Assert(Weapons[other_obj->instance].weapon_info_index >= 0);
		if(Weapons[other_obj->instance].weapon_info_index < 0){
			return;
		}
		wip = &Weapon_info[Weapons[other_obj->instance].weapon_info_index];

		// if its a leech weapon - NOTE - unknownplayer: Perhaps we should do something interesting like direct the leeched energy into the attacker ?
		if(wip->wi_flags & WIF_ENERGY_SUCK){
			// reduce afterburner fuel
			shipp->afterburner_fuel -= wip->afterburner_reduce;
			shipp->afterburner_fuel = (shipp->afterburner_fuel < 0.0f) ? 0.0f : shipp->afterburner_fuel;

			// reduce weapon energy
			shipp->weapon_energy -= wip->weapon_reduce;
			shipp->weapon_energy = (shipp->weapon_energy < 0.0f) ? 0.0f : shipp->weapon_energy;
		}
	}
}

// Goober5000
void ship_apply_tag(int ship_num, int tag_level, float tag_time, object *target, vec3d *start, int ssm_index, int ssm_team)
{
	// set time first tagged
	if (Ships[ship_num].time_first_tagged == 0)
		Ships[ship_num].time_first_tagged = Missiontime;

	// do tag effect
	if (tag_level == 1)
	{
//		mprintf(("TAGGED %s for %f seconds\n", Ships[ship_num].ship_name, tag_time));
		Ships[ship_num].tag_left = tag_time;
		Ships[ship_num].tag_total = tag_time;
	}
	else if (tag_level == 2)
	{
//		mprintf(("Level 2 TAGGED %s for %f seconds\n", Ships[ship_num].ship_name, tag_time));
		Ships[ship_num].level2_tag_left = tag_time;
		Ships[ship_num].level2_tag_total = tag_time;
	}
	else if (tag_level == 3)
	{
		// tag C creates an SSM strike, yay -Bobboau
		Assert(target);
		Assert(start);

		struct ssm_firing_info;

		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR("Firing artillery", 1570));

		ssm_create(target, start, ssm_index, NULL, ssm_team);
	}
}

// This gets called to apply damage when something hits a particular point on a ship.
// This assumes that whoever called this knows if the shield got hit or not.
// hitpos is in world coordinates.
// if quadrant is not -1, then that part of the shield takes damage properly.
void ship_apply_local_damage(object *ship_obj, object *other_obj, vec3d *hitpos, float damage, int quadrant, bool create_spark, int submodel_num, vec3d *hit_normal)
{
	Assert(ship_obj);	// Goober5000
	Assert(other_obj);	// Goober5000

	ship *ship_p = &Ships[ship_obj->instance];	
    weapon *wp = &Weapons[other_obj->instance];

	//	If got hit by a weapon, tell the AI so it can react.  Only do this line in single player,
	// or if I am the master in a multiplayer game
	if ((other_obj->type == OBJ_WEAPON) && ( !(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER )) {
		//	If weapon hits ship on same team and that ship not targeted and parent of weapon not player,
		//	don't do damage.
		//	Ie, player can always do damage.  AI can only damage team if that ship is targeted.
		if (wp->target_num != OBJ_INDEX(ship_obj)) {
			if ((ship_p->team == wp->team) && !(Objects[other_obj->parent].flags & OF_PLAYER_SHIP) ) {
				return;
			}
		}
	}

	// only want to check the following in single player or if I am the multiplayer game server
	// Added OBJ_BEAM for traitor detection - FUBAR
	if ( !MULTIPLAYER_CLIENT && ((other_obj->type == OBJ_SHIP) || (other_obj->type == OBJ_WEAPON) || (other_obj->type == OBJ_BEAM)) ) {
		ai_ship_hit(ship_obj, other_obj, hitpos, quadrant, hit_normal);
	}

	//	Cut damage done on the player by 4x in training missions, but do full accredidation
	if ( The_mission.game_type & MISSION_TYPE_TRAINING ){
		if (ship_obj == Player_obj){
			damage /= 4.0f;
		}
	}

	// maybe tag the ship
	if(!MULTIPLAYER_CLIENT && (other_obj->type == OBJ_WEAPON || other_obj->type == OBJ_BEAM)) {
		weapon_info *wip = NULL;

		if (other_obj->type == OBJ_WEAPON)
			wip = &Weapon_info[Weapons[other_obj->instance].weapon_info_index];
		else if (other_obj->type == OBJ_BEAM)
			wip = &Weapon_info[Beams[other_obj->instance].weapon_info_index];

		Assert(wip != NULL);

		if (wip->wi_flags & WIF_TAG) {
			// ssm stuff
			vec3d *start = hitpos;
			int ssm_index = wip->SSM_index;

			ship_apply_tag(ship_obj->instance, wip->tag_level, wip->tag_time, ship_obj, start, ssm_index, wp->team);
		}
	}

#ifndef NDEBUG
	if (other_obj->type == OBJ_WEAPON) {
		weapon_info	*wip = &Weapon_info[Weapons[other_obj->instance].weapon_info_index];
		if (wip->wi_flags & WIF_HOMING) {
			Homing_hits++;
			// nprintf(("AI", " Hit!  Hits = %i/%i\n", Homing_hits, (Homing_hits + Homing_misses)));
		}
	}
#endif

	if ( Event_Music_battle_started == 0 )	{
		ship_hit_music(ship_obj, other_obj);
	}
	

	if (damage < 0.0f){
		damage = 0.0f;
	}

	// evaluate any possible player stats implications
	scoring_eval_hit(ship_obj,other_obj);

	global_damage = false;
	ship_do_damage(ship_obj, other_obj, hitpos, damage, quadrant );

	// DA 5/5/98: move ship_hit_create_sparks() after do_damage() since number of sparks depends on hull strength
	// doesn't hit shield and we want sparks
	if ((quadrant == MISS_SHIELDS) && create_spark)	{
		// check if subsys destroyed
		if ( !is_subsys_destroyed(ship_p, submodel_num) ) {
			ship_hit_create_sparks(ship_obj, hitpos, submodel_num);
		}
		//fireball_create( hitpos, FIREBALL_SHIP_EXPLODE1, OBJ_INDEX(ship_obj), 0.25f );
	}
}

extern int Cmdline_nohtl;

// This gets called to apply damage when a damaging force hits a ship, but at no 
// point in particular.  Like from a shockwave.   This routine will see if the
// shield got hit and if so, apply damage to it.
// You can pass force_center==NULL if you the damage doesn't come from anywhere,
// like for debug keys to damage an object or something.  It will 
// assume damage is non-directional and will apply it correctly.   
void ship_apply_global_damage(object *ship_obj, object *other_obj, vec3d *force_center, float damage )
{
	Assert(ship_obj);	// Goober5000 (but not other_obj in case of sexp)

	vec3d tmp, world_hitpos;
	global_damage = true;

	if ( force_center )	{
		int shield_quad;
		vec3d local_hitpos;

		// find world hitpos
		vm_vec_sub( &tmp, force_center, &ship_obj->pos );
		vm_vec_normalize_safe( &tmp );
		vm_vec_scale_add( &world_hitpos, &ship_obj->pos, &tmp, ship_obj->radius );

		// Rotate world_hitpos into local coordinates (local_hitpos)
		vm_vec_sub(&tmp, &world_hitpos, &ship_obj->pos );
		vm_vec_rotate( &local_hitpos, &tmp, &ship_obj->orient );

		// shield_quad = quadrant facing the force_center
		shield_quad = get_quadrant(&local_hitpos);

		// world_hitpos use force_center for shockwave
		// Goober5000 check for NULL
		if (Cmdline_nohtl && (other_obj != NULL) && (other_obj->type == OBJ_SHOCKWAVE) && (Ship_info[Ships[ship_obj->instance].ship_info_index].flags & SIF_HUGE_SHIP))
		{
			world_hitpos = *force_center;
		}

		// Do damage on local point		
		ship_do_damage(ship_obj, other_obj, &world_hitpos, damage, shield_quad );
	} else {
		// Since an force_center wasn't specified, this is probably just a debug key
		// to kill an object.   So pick a shield quadrant and a point on the
		// radius of the object.   
		vm_vec_scale_add( &world_hitpos, &ship_obj->pos, &ship_obj->orient.vec.fvec, ship_obj->radius );

		for (int i=0; i<MAX_SHIELD_SECTIONS; i++){
			ship_do_damage(ship_obj, other_obj, &world_hitpos, damage/MAX_SHIELD_SECTIONS, i);
		}
	}

	// AL 3-30-98: Show flashing blast icon if player ship has taken blast damage
	if ( ship_obj == Player_obj ) {
		// only show blast icon if playing on medium skill or lower -> unknownplayer: why? I think this should be changed.
		// Goober5000 - agreed; commented out
		//if ( Game_skill_level <= 2 ) {
			hud_start_text_flash(XSTR("Blast", 1428), 2000);
		//}
	}

	// evaluate any player stats scoring conditions (specifically, blasts from remotely detonated secondary weapons)
	scoring_eval_hit(ship_obj,other_obj,1);	
}

void ship_apply_wash_damage(object *ship_obj, object *other_obj, float damage)
{
	vec3d world_hitpos, direction_vec, rand_vec;

	// Since an force_center wasn't specified, this is probably just a debug key
	// to kill an object.   So pick a shield quadrant and a point on the
	// radius of the object
	vm_vec_rand_vec_quick(&rand_vec);
	vm_vec_scale_add(&direction_vec, &ship_obj->orient.vec.fvec, &rand_vec, 0.5f);
	vm_vec_normalize_quick(&direction_vec);
	vm_vec_scale_add( &world_hitpos, &ship_obj->pos, &direction_vec, ship_obj->radius );

	// Do damage to hull and not to shields
	global_damage = true;
	ship_do_damage(ship_obj, other_obj, &world_hitpos, damage, -1, 1);

	// AL 3-30-98: Show flashing blast icon if player ship has taken blast damage
	if ( ship_obj == Player_obj ) {
		// only show blast icon if playing on medium skill or lower
		// Goober5000 - commented out
		//if ( Game_skill_level <= 2 ) {
			hud_start_text_flash(XSTR("Engine Wash", 1429), 2000);
		//}
	}

	// evaluate any player stats scoring conditions (specifically, blasts from remotely detonated secondary weapons)
	scoring_eval_hit(ship_obj,other_obj,1);
}

// player pain
void ship_hit_pain(float damage)
{
    if (!(Player_obj->flags & OF_INVULNERABLE))
    {
    	game_flash( damage/15.0f, -damage/30.0f, -damage/30.0f );
    }

	// kill any active popups when you get hit.
	if ( Game_mode & GM_MULTIPLAYER ){
		popup_kill_any_active();
	}
}
