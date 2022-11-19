/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "asteroid/asteroid.h"
#include "debugconsole/console.h"
#include "fireball/fireballs.h"
#include "freespace.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "globalincs/systemvars.h"
#include "hud/hud.h"
#include "hud/hudescort.h"
#include "hud/hudgauges.h"
#include "hud/hudtarget.h"
#include "iff_defs/iff_defs.h"
#include "io/timer.h"
#include "localization/localize.h"
#include "math/staticrand.h"
#include "math/vecmat.h"
#include "model/model.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "scripting/global_hooks.h"
#include "particle/particle.h"
#include "render/3d.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "stats/scoring.h"
#include "weapon/weapon.h"

#include <algorithm>

#define			ASTEROID_OBJ_USED	(1<<0)				// flag used in asteroid_obj struct
#define			MAX_ASTEROID_OBJS	MAX_ASTEROIDS		// max number of asteroids tracked in asteroid list
asteroid_obj	Asteroid_objs[MAX_ASTEROID_OBJS];	// array used to store asteroid object indexes
asteroid_obj	Asteroid_obj_list;						// head of linked list of asteroid_obj structs

// used for randomly generating debris type when there are multiple sizes.
const float SMALL_DEBRIS_WEIGHT = 8.0f;
const float MEDIUM_DEBRIS_WEIGHT = 4.0f;
const float LARGE_DEBRIS_WEIGHT = 1.0f;

int	Asteroids_enabled = 1;
int	Num_asteroids = 0;

SCP_vector< asteroid_info > Asteroid_info;
asteroid			Asteroids[MAX_ASTEROIDS];
asteroid_field	Asteroid_field;


static int		Asteroid_impact_explosion_ani;
static float	Asteroid_impact_explosion_radius;
char	Asteroid_icon_closeup_model[NAME_LENGTH];
vec3d	Asteroid_icon_closeup_position;
float	Asteroid_icon_closeup_zoom;	

#define	ASTEROID_CHECK_WRAP_TIMESTAMP			2000	// how often an asteroid gets checked for wrapping
#define	ASTEROID_UPDATE_COLLIDE_TIMESTAMP	2000	// how often asteroid is checked for impending collisions with escort ships
#define	ASTEROID_MIN_COLLIDE_TIME				24		// time in seconds to check for asteroid colliding

typedef struct asteroid_target {
	int objnum;
	int signature;
	TIMESTAMP throw_stamp;
	int incoming_asteroids;
} asteroid_target;

// if default throwing behavior is enabled, then this should always have at most 1 target or possibly none, determined the retail way
// if not, then this is whatever number of mission-specified ships (after they arrive, list is sanitized when they exit)
SCP_vector<asteroid_target> Asteroid_targets;


/**
 * Return number of asteroids expected to collide with a ship.
 */
static int count_incident_asteroids(int target_objnum)
{
	object* asteroid_objp;
	int		count;

	count = 0;

	for (asteroid_objp = GET_FIRST(&obj_used_list); asteroid_objp != END_OF_LIST(&obj_used_list); asteroid_objp = GET_NEXT(asteroid_objp)) {
		if (asteroid_objp->type == OBJ_ASTEROID) {
			asteroid* asp = &Asteroids[asteroid_objp->instance];

			if (asp->target_objnum == target_objnum) {
				count++;
			}
		}
	}

	return count;
}

// add a ship as a new asteroid target
void asteroid_add_target(object* objp) {
	asteroid_target new_target;
	new_target.objnum = OBJ_INDEX(objp);
	new_target.signature = objp->signature;
	new_target.throw_stamp = _timestamp(Random::next(500, 2000));
	new_target.incoming_asteroids = count_incident_asteroids(OBJ_INDEX(objp)); // this *should* normally be 0

	Asteroid_targets.push_back(new_target);
}

/**
 * Force updating of pair stuff for asteroid *objp.
 */
static void asteroid_update_collide(object *objp)
{
	// Asteroid has wrapped, update collide objnum and flags
	Asteroids[objp->instance].collide_objnum = -1;
	Asteroids[objp->instance].collide_objsig = -1;
	OBJ_RECALC_PAIRS(objp);	
}

/**
 * Clear out the ::Asteroid_obj_list
 */
static void asteroid_obj_list_init()
{
	int i;

	list_init(&Asteroid_obj_list);
	for ( i = 0; i < MAX_ASTEROID_OBJS; i++ ) {
		Asteroid_objs[i].flags = 0;
	}
}

/**
 * Add a node from the Asteroid_obj_list.  
 * Only called from ::weapon_create()
 */
static int asteroid_obj_list_add(int objnum)
{
	int index;

	asteroid *cur_asteroid = &Asteroids[Objects[objnum].instance];
	index = (int)(cur_asteroid - Asteroids);

	Assert(index >= 0 && index < MAX_ASTEROID_OBJS);
	Assert(!(Asteroid_objs[index].flags & ASTEROID_OBJ_USED));

	Asteroid_objs[index].flags = 0;
	Asteroid_objs[index].objnum = objnum;
	list_append(&Asteroid_obj_list, &Asteroid_objs[index]);
	Asteroid_objs[index].flags |= ASTEROID_OBJ_USED;

	return index;
}

/**
 * Remove a node from the Asteroid_obj_list.  
 * Only called from ::weapon_delete()
 */
static void asteroid_obj_list_remove(object * obj)
{
	int index = obj->instance;

	Assert(index >= 0 && index < MAX_ASTEROID_OBJS);
	Assert(Asteroid_objs[index].flags & ASTEROID_OBJ_USED);

	list_remove(&Asteroid_obj_list, &Asteroid_objs[index]);	
	Asteroid_objs[index].flags = 0;
}


/**
 * Prevent speed from getting too huge so it's hard to catch up to an asteroid.
 */
static float asteroid_cap_speed(int asteroid_info_index, float speed)
{
	float max, double_max;

	Assert( asteroid_info_index < (int)Asteroid_info.size() );

	max = Asteroid_info[asteroid_info_index].max_speed;
	double_max = max * 2;

	while (speed > double_max){
		speed *= 0.5f;
	}
	
	if (speed > max){
		speed *= 0.75f;
	}

	return speed;
}

/**
 * Check if asteroid is within inner bound
 * @return 0 if not inside or no inner bound, 1 if inside inner bound
 */
static bool asteroid_in_inner_bound(asteroid_field *asfieldp, vec3d *pos, float delta) {

	if (!asfieldp->has_inner_bound)
		return false;

	return (pos->xyz.x > asfieldp->inner_min_bound.xyz.x - delta) && (pos->xyz.x < asfieldp->inner_max_bound.xyz.x + delta) &&
		(pos->xyz.y > asfieldp->inner_min_bound.xyz.y - delta) && (pos->xyz.y < asfieldp->inner_max_bound.xyz.y + delta) &&
		(pos->xyz.z > asfieldp->inner_min_bound.xyz.z - delta) && (pos->xyz.z < asfieldp->inner_max_bound.xyz.z + delta);
}

static bool asteroid_is_ship_inside_field(asteroid_field* asfieldp, vec3d* pos, float radius) {

	radius *= 2.0f;

	return pos->xyz.x + radius > Asteroid_field.min_bound.xyz.x && pos->xyz.x - radius < Asteroid_field.max_bound.xyz.x &&
		   pos->xyz.y + radius > Asteroid_field.min_bound.xyz.y && pos->xyz.y - radius < Asteroid_field.max_bound.xyz.y &&
	       pos->xyz.z + radius > Asteroid_field.min_bound.xyz.z && pos->xyz.z - radius < Asteroid_field.max_bound.xyz.z &&
		   !asteroid_in_inner_bound(asfieldp, pos, radius);
}

/**
 * Repositions asteroid outside the inner box on all 3 axes
 *
 * Moves to the other side of the inner box a distance delta from edge of box
 */
static void inner_bound_pos_fixup(asteroid_field *asfieldp, vec3d *pos)
{
	if (!asteroid_in_inner_bound(asfieldp, pos, 0)) {
		return;
	}

	float dist1, dist2;
	int axis;

	for (axis=0; axis<3; axis++) {
		dist1 = pos->a1d[axis] - asfieldp->inner_min_bound.a1d[axis];
		dist2 = asfieldp->inner_max_bound.a1d[axis] - pos->a1d[axis];
		Assert(dist1 >= 0 && dist2 >= 0);

		if (dist1 < dist2) {
			pos->a1d[axis] = asfieldp->inner_max_bound.a1d[axis] + dist1;
		} else {
			pos->a1d[axis] = asfieldp->inner_min_bound.a1d[axis] - dist2;
		}
	}
}

/**
 * Create a single asteroid 
 */
object *asteroid_create(asteroid_field *asfieldp, int asteroid_type, int asteroid_subtype)
{
	int				n, objnum;
	matrix			orient;
	object			*objp;
	asteroid			*asp;
	asteroid_info	*asip;
	vec3d			pos, delta_bound;
	angles			angs;
	float				radius;
	ushort			signature;
	int				rand_base;

	// bogus
	if(asfieldp == NULL) {
		return NULL;
	}

	for (n=0; n<MAX_ASTEROIDS; n++) {
		if (!(Asteroids[n].flags & AF_USED)) {
			break;
		}
	}

	if (n >= MAX_ASTEROIDS) {
		nprintf(("Warning","Could not create asteroid, no more slots left\n"));
		return NULL;
	}

	if((asteroid_type < 0) || (asteroid_type >= (int)Asteroid_info.size())) {
		return NULL;
	}

	if((asteroid_subtype < 0) || (asteroid_subtype >= NUM_DEBRIS_POFS)) {
		return NULL;
	}

	// HACK: multiplayer asteroid subtype always 0 to keep subtype in sync
	if ( Game_mode & GM_MULTIPLAYER) {
		asteroid_subtype = 0;
	}	

	asip = &Asteroid_info[asteroid_type];

	// bogus
	if(asip->modelp[asteroid_subtype] == NULL) {
		return NULL;
	}	

	asp = &Asteroids[n];
	asp->asteroid_type = asteroid_type;
	asp->asteroid_subtype = asteroid_subtype;
	asp->flags = 0;
	asp->flags |= AF_USED;
	asp->check_for_wrap = _timestamp_rand(0, ASTEROID_CHECK_WRAP_TIMESTAMP);
	asp->check_for_collide = _timestamp_rand(0, ASTEROID_UPDATE_COLLIDE_TIMESTAMP);
	asp->final_death_time = TIMESTAMP::invalid();
	asp->collide_objnum = -1;
	asp->collide_objsig = -1;
	asp->target_objnum = -1;

	radius = model_get_radius(asip->model_num[asteroid_subtype]);

	vm_vec_sub(&delta_bound, &asfieldp->max_bound, &asfieldp->min_bound);

	// for multiplayer, we want to do a static_rand so that everything behaves the same on all machines
	signature = 0;
	rand_base = 0;
	if ( Game_mode & GM_NORMAL ) {
		pos.xyz.x = asfieldp->min_bound.xyz.x + delta_bound.xyz.x * frand();
		pos.xyz.y = asfieldp->min_bound.xyz.y + delta_bound.xyz.y * frand();
		pos.xyz.z = asfieldp->min_bound.xyz.z + delta_bound.xyz.z * frand();

		inner_bound_pos_fixup(asfieldp, &pos);
		angs.p = frand() * PI2;
		angs.b = frand() * PI2;
		angs.h = frand() * PI2;
	} else {
		signature = multi_assign_network_signature( MULTI_SIG_ASTEROID );
		rand_base = signature;

		pos.xyz.x = asfieldp->min_bound.xyz.x + delta_bound.xyz.x * static_randf( rand_base++ );
		pos.xyz.y = asfieldp->min_bound.xyz.y + delta_bound.xyz.y * static_randf( rand_base++ );
		pos.xyz.z = asfieldp->min_bound.xyz.z + delta_bound.xyz.z * static_randf( rand_base++ );

		inner_bound_pos_fixup(asfieldp, &pos);
		angs.p = static_randf( rand_base++ ) * PI2;
		angs.b = static_randf( rand_base++ ) * PI2;
		angs.h = static_randf( rand_base++ ) * PI2;
	}

	vm_angles_2_matrix(&orient, &angs);
    flagset<Object::Object_Flags> asteroid_default_flagset;
    asteroid_default_flagset += Object::Object_Flags::Renders;
    asteroid_default_flagset += Object::Object_Flags::Physics;
    asteroid_default_flagset += Object::Object_Flags::Collides;
    
    objnum = obj_create(OBJ_ASTEROID, -1, n, &orient, &pos, radius, asteroid_default_flagset, false);
	
	if ( (objnum == -1) || (objnum >= MAX_OBJECTS) ) {
		mprintf(("Couldn't create asteroid -- out of object slots\n"));
		return NULL;
	}

	asp->objnum = objnum;
	asp->model_instance_num = -1;

	if (model_get(asip->model_num[asteroid_subtype])->flags & PM_FLAG_HAS_INTRINSIC_MOTION) {
		asp->model_instance_num = model_create_instance(objnum, asip->model_num[asteroid_subtype]);
	}

	// Add to Asteroid_used_list
	asteroid_obj_list_add(objnum);

	objp = &Objects[objnum];

	if ( Game_mode & GM_MULTIPLAYER ){
		objp->net_signature = signature;
	}

	Num_asteroids++;

	if (radius < 1.0) {
		radius = 1.0f;
	}

	vec3d rotvel;
	if ( Game_mode & GM_NORMAL ) {
		vm_vec_rand_vec_quick(&rotvel);
		vm_vec_scale(&rotvel, frand()/4.0f + 0.1f);
		objp->phys_info.rotvel = rotvel;
		vm_vec_rand_vec_quick(&objp->phys_info.vel);
	} else {
		static_randvec( rand_base++, &rotvel );
		vm_vec_scale(&rotvel, static_randf(rand_base++)/4.0f + 0.1f);
		objp->phys_info.rotvel = rotvel;
		static_randvec( rand_base++, &objp->phys_info.vel );
	}


	float speed;

	if ( Game_mode & GM_NORMAL ) {
		speed = asteroid_cap_speed(asteroid_type, asfieldp->speed*frand_range(0.5f + (float) Game_skill_level/NUM_SKILL_LEVELS, 2.0f + (float) (2*Game_skill_level)/NUM_SKILL_LEVELS));
	} else {
		speed = asteroid_cap_speed(asteroid_type, asfieldp->speed*static_randf_range(rand_base++, 0.5f + (float) Game_skill_level/NUM_SKILL_LEVELS, 2.0f + (float) (2*Game_skill_level)/NUM_SKILL_LEVELS));
	}
	
	vm_vec_scale(&objp->phys_info.vel, speed);
	objp->phys_info.desired_vel = objp->phys_info.vel;

	// blow out his reverse thrusters. Or drag, same thing.
	objp->phys_info.rotdamp = 10000.0f;
	objp->phys_info.flags |= PF_BALLISTIC;
	objp->phys_info.gravity_const = asip->gravity_const;

	// Fill in the max_vel field, so the collision pair stuff knows
	// how fast this can move maximum in order to throw out collisions.
	// This is in local coordinates, so Z is forward velocity.
	objp->phys_info.max_vel.xyz.x = 0.0f;
	objp->phys_info.max_vel.xyz.y = 0.0f;
	objp->phys_info.max_vel.xyz.z = vm_vec_mag(&objp->phys_info.desired_vel);
	
	objp->phys_info.mass = asip->modelp[asteroid_subtype]->rad * 700.0f;
	objp->phys_info.I_body_inv.vec.rvec.xyz.x = 1.0f / (objp->phys_info.mass*asip->modelp[asteroid_subtype]->rad);
	objp->phys_info.I_body_inv.vec.uvec.xyz.y = objp->phys_info.I_body_inv.vec.rvec.xyz.x;
	objp->phys_info.I_body_inv.vec.fvec.xyz.z = objp->phys_info.I_body_inv.vec.rvec.xyz.x;
	objp->hull_strength = asip->initial_asteroid_strength * (0.8f + (float)Game_skill_level/NUM_SKILL_LEVELS)/2.0f;

	// ensure vel is valid
	Assert( !vm_is_vec_nan(&objp->phys_info.vel) );	

	return objp;
}

/**
 * Create asteroids when parent_objp blows up.
 */
void asteroid_sub_create(object *parent_objp, int asteroid_type, vec3d *relvec)
{
	object	*new_objp;
	float speed;

	Assert(parent_objp->type == OBJ_ASTEROID);
	int subtype = Asteroids[parent_objp->instance].asteroid_subtype;
	new_objp = asteroid_create(&Asteroid_field, asteroid_type, subtype);

	if (new_objp == NULL)
		return;

	if ( MULTIPLAYER_MASTER ){
		send_asteroid_create( new_objp, parent_objp, asteroid_type, relvec );
	}

	//	Now, bash some values.
	vm_vec_scale_add(&new_objp->pos, &parent_objp->pos, relvec, 0.5f * parent_objp->radius);
	float parent_speed = vm_vec_mag_quick(&parent_objp->phys_info.vel);

	if ( parent_speed < 0.1f ) {
		parent_speed = vm_vec_mag_quick(&Asteroid_field.vel);
	}

	new_objp->phys_info.vel = parent_objp->phys_info.vel;
	if ( Game_mode & GM_NORMAL )
		speed = asteroid_cap_speed(asteroid_type, (frand() + 2.0f) * parent_speed);
	else
		speed = asteroid_cap_speed(asteroid_type, (static_randf(new_objp->net_signature)+2.0f) * parent_speed);

	vm_vec_scale_add2(&new_objp->phys_info.vel, relvec, speed);
	if (vm_vec_mag_quick(&new_objp->phys_info.vel) > 80.0f)
		vm_vec_scale(&new_objp->phys_info.vel, 0.5f);

	new_objp->phys_info.desired_vel = new_objp->phys_info.vel;
	vm_vec_scale_add(&new_objp->last_pos, &new_objp->pos, &new_objp->phys_info.vel, -flFrametime);
}

/**
 * Load in an asteroid model
 */
static void asteroid_load(int asteroid_info_index, int asteroid_subtype)
{
	int i;
	asteroid_info	*asip;

	Assert( asteroid_info_index < (int)Asteroid_info.size() );
	Assert( asteroid_subtype < NUM_DEBRIS_POFS );

	if ( (asteroid_info_index >= (int)Asteroid_info.size()) || (asteroid_subtype >= NUM_DEBRIS_POFS) ) {
		return;
	}

	asip = &Asteroid_info[asteroid_info_index];

	if ( !VALID_FNAME(asip->pof_files[asteroid_subtype]) )
		return;

	asip->model_num[asteroid_subtype] = model_load( asip->pof_files[asteroid_subtype], 0, NULL );

	if (asip->model_num[asteroid_subtype] >= 0)
	{
		polymodel *pm = asip->modelp[asteroid_subtype] = model_get(asip->model_num[asteroid_subtype]);
		
		if ( asip->num_detail_levels != pm->n_detail_levels )
		{
			if ( !Is_standalone )
			{
				// just log to file for standalone servers
				Warning(LOCATION, "For asteroid '%s', detail level\nmismatch (POF needs %d)", asip->name, pm->n_detail_levels );
			}
			else
			{
				nprintf(("Warning",  "For asteroid '%s', detail level mismatch (POF needs %d)\n", asip->name, pm->n_detail_levels));
			}
		}	
		// Stuff detail level distances.
		for ( i=0; i<pm->n_detail_levels; i++ )
			pm->detail_depth[i] = (i < asip->num_detail_levels) ? i2fl(asip->detail_distance[i]) : 0.0f;
	}
}

/**
 * Create all the asteroids for the mission
 */
void asteroid_create_all()
{
	int i, idx;

	// ship_debris_odds_table keeps track of debris type of the next debris piece
	// each different type (size) of debris piece has a diffenent weight, smaller weighted more heavily than larger.
	// choose next type from table ship_debris_odds_table by Random::next()%max_weighted_range,
	// the threshold *below* which the debris type is selected.
	struct {
		float random_threshold;
		int debris_type;
	} ship_debris_odds_table[MAX_ACTIVE_DEBRIS_TYPES];

	float max_weighted_range = 0.0f;

	if (!Asteroids_enabled)
		return;

	if (Asteroid_field.num_initial_asteroids <= 0 ) {
		return;
	}

	if (Asteroid_field.num_used_field_debris_types <= 0) {
		Warning(LOCATION, "An asteroid field is enabled, but no asteroid types were enabled.");
		return;
	}

	int max_asteroids = Asteroid_field.num_initial_asteroids; // * (1.0f - 0.1f*(MAX_DETAIL_LEVEL-Detail.asteroid_density)));

	int num_debris_types = 0;

	// get number of ship debris types
	if (Asteroid_field.debris_genre == DG_SHIP) {
		for (idx=0; idx<MAX_ACTIVE_DEBRIS_TYPES; idx++) {
			if (Asteroid_field.field_debris_type[idx] != -1) {
				num_debris_types++;
			}
		}

		// Calculate the odds table
		for (idx=0; idx<num_debris_types; idx++) {
			float debris_weight = Asteroid_info[Asteroid_field.field_debris_type[idx]].spawn_weight;
			ship_debris_odds_table[idx].random_threshold = max_weighted_range + debris_weight;
			ship_debris_odds_table[idx].debris_type = Asteroid_field.field_debris_type[idx];
			max_weighted_range += debris_weight;
		}
	}

	// Load Asteroid/ship models
	if (Asteroid_field.debris_genre == DG_SHIP) {
		for (idx=0; idx<num_debris_types; idx++) {
			asteroid_load(Asteroid_field.field_debris_type[idx], 0);
		}
	} else {
		if (Asteroid_field.field_debris_type[0] != -1) {
			asteroid_load(ASTEROID_TYPE_SMALL, 0);
			asteroid_load(ASTEROID_TYPE_MEDIUM, 0);
			asteroid_load(ASTEROID_TYPE_LARGE, 0);
		}

		if (Asteroid_field.field_debris_type[1] != -1) {
			asteroid_load(ASTEROID_TYPE_SMALL, 1);
			asteroid_load(ASTEROID_TYPE_MEDIUM, 1);
			asteroid_load(ASTEROID_TYPE_LARGE, 1);
		}

		if (Asteroid_field.field_debris_type[2] != -1) {
			asteroid_load(ASTEROID_TYPE_SMALL, 2);
			asteroid_load(ASTEROID_TYPE_MEDIUM, 2);
			asteroid_load(ASTEROID_TYPE_LARGE, 2);
		}
	}

	// load all the asteroid/debris pieces
	for (i=0; i<max_asteroids; i++) {
		if (Asteroid_field.debris_genre == DG_ASTEROID) {
			// For asteroid, load only large asteroids

			// get a valid subtype
			int counter = Random::next(Asteroid_field.num_used_field_debris_types);
			int subtype = -1;
			for (int j = 0; j < NUM_DEBRIS_POFS; j++) {
				if (Asteroid_field.field_debris_type[j] >= 0) {
					if (counter == 0) {
						subtype = j;
						break;
					} else
						counter--;
				}
			}

			if (subtype >= 0)
				asteroid_create(&Asteroid_field, ASTEROID_TYPE_LARGE, subtype);
		} else {
			Assert(num_debris_types > 0);

			float rand_choice = frand() * max_weighted_range;

			for (idx=0; idx<MAX_ACTIVE_DEBRIS_TYPES; idx++) {
				// for ship debris, choose type according to odds table
				if (rand_choice < ship_debris_odds_table[idx].random_threshold) {
					asteroid_create(&Asteroid_field, ship_debris_odds_table[idx].debris_type, 0);
					break;
				}
			}
		}
	}
}

/**
 * Init asteroid system for the level, called from ::game_level_init()
 */
void asteroid_level_init()
{
	Asteroid_field.num_initial_asteroids = 0;  // disable asteroid field by default.
	Asteroid_field.speed = 0.0f;
	vm_vec_make(&Asteroid_field.min_bound, -1000.0f, -1000.0f, -1000.0f);
	vm_vec_make(&Asteroid_field.max_bound, 1000.0f, 1000.0f, 1000.0f);
	vm_vec_make(&Asteroid_field.inner_min_bound, -500.0f, -500.0f, -500.0f);
	vm_vec_make(&Asteroid_field.inner_max_bound, 500.0f, 500.0f, 500.0f);
	Asteroid_field.has_inner_bound = false;
	Asteroid_field.field_type = FT_ACTIVE;
	Asteroid_field.debris_genre = DG_ASTEROID;
	Asteroid_field.field_debris_type[0] = -1;
	Asteroid_field.field_debris_type[1] = -1;
	Asteroid_field.field_debris_type[2] = -1;
	Asteroid_field.num_used_field_debris_types = 0;
	Asteroid_field.target_names.clear();

	if (!Fred_running)
	{
		for (auto& ast : Asteroid_info)
			ast.damage_type_idx = ast.damage_type_idx_sav;

		Num_asteroids = 0;
		asteroid_obj_list_init();
		Asteroid_targets.clear();
	}
}

/**
 * Should asteroid wrap from one end of the asteroid field to the other.
 * Multiplayer clients will always return 0 from this function.  We will force a wrap on the clients when server tells us
 *
 * @return !0 if asteroid should be wrapped, 0 otherwise.  
 */
static int asteroid_should_wrap(object *objp, asteroid_field *asfieldp)
{
	if ( MULTIPLAYER_CLIENT )
		return 0;

	if (objp->pos.xyz.x < asfieldp->min_bound.xyz.x) {
		return 1;
	}

	if (objp->pos.xyz.y < asfieldp->min_bound.xyz.y) {
		return 1;
	}

	if (objp->pos.xyz.z < asfieldp->min_bound.xyz.z) {
		return 1;
	}

	if (objp->pos.xyz.x > asfieldp->max_bound.xyz.x) {
		return 1;
	}

	if (objp->pos.xyz.y > asfieldp->max_bound.xyz.y) {
		return 1;
	}

	if (objp->pos.xyz.z > asfieldp->max_bound.xyz.z) {
		return 1;
	}

	// check against inner bound
	if (asfieldp->has_inner_bound) {
		if ( (objp->pos.xyz.x > asfieldp->inner_min_bound.xyz.x) && (objp->pos.xyz.x < asfieldp->inner_max_bound.xyz.x)
		  && (objp->pos.xyz.y > asfieldp->inner_min_bound.xyz.y) && (objp->pos.xyz.y < asfieldp->inner_max_bound.xyz.y)
		  && (objp->pos.xyz.z > asfieldp->inner_min_bound.xyz.z) && (objp->pos.xyz.z < asfieldp->inner_max_bound.xyz.z) ) {

			return 1;
		}
	}

	return 0;
}

/**
 * Wrap an asteroid from one end of the asteroid field to the other
 */
static void asteroid_wrap_pos(object *objp, asteroid_field *asfieldp)
{
	if (objp->pos.xyz.x < asfieldp->min_bound.xyz.x) {
		objp->pos.xyz.x = asfieldp->max_bound.xyz.x + (objp->pos.xyz.x - asfieldp->min_bound.xyz.x);
	}

	if (objp->pos.xyz.y < asfieldp->min_bound.xyz.y) {
		objp->pos.xyz.y = asfieldp->max_bound.xyz.y + (objp->pos.xyz.y - asfieldp->min_bound.xyz.y);
	}
	
	if (objp->pos.xyz.z < asfieldp->min_bound.xyz.z) {
		objp->pos.xyz.z = asfieldp->max_bound.xyz.z + (objp->pos.xyz.z - asfieldp->min_bound.xyz.z);
	}

	if (objp->pos.xyz.x > asfieldp->max_bound.xyz.x) {
		objp->pos.xyz.x = asfieldp->min_bound.xyz.x + (objp->pos.xyz.x - asfieldp->max_bound.xyz.x);
	}

	if (objp->pos.xyz.y > asfieldp->max_bound.xyz.y) {
		objp->pos.xyz.y = asfieldp->min_bound.xyz.y + (objp->pos.xyz.y - asfieldp->max_bound.xyz.y);
	}

	if (objp->pos.xyz.z > asfieldp->max_bound.xyz.z) {
		objp->pos.xyz.z = asfieldp->min_bound.xyz.z + (objp->pos.xyz.z - asfieldp->max_bound.xyz.z);
	}

	// wrap on inner bound, check all 3 axes as needed, use of rand ok for multiplayer with send_asteroid_throw()
	inner_bound_pos_fixup(asfieldp, &objp->pos);

}


/**
 * Is asteroid targeted? 
 *
 * @return !0 if this asteroid is a target for any ship, otherwise return 0
 */
static int asteroid_is_targeted(object *objp)
{
	ship_obj	*so;
	object	*ship_objp;
	int		asteroid_obj_index;

	asteroid_obj_index=OBJ_INDEX(objp);

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		ship_objp = &Objects[so->objnum];
		if ( Ai_info[Ships[ship_objp->instance].ai_index].target_objnum == asteroid_obj_index ) {
			return 1;
		}
	}
	
	return 0;
}

/**
 * Create an asteroid that will hit object *objp in delta_time seconds
 */
static void asteroid_aim_at_target(object *objp, object *asteroid_objp, float delta_time)
{
	vec3d	predicted_center_pos;
	vec3d	rand_vec;
	float		speed;

	vm_vec_scale_add(&predicted_center_pos, &objp->pos, &objp->phys_info.vel, delta_time);
	vm_vec_rand_vec_quick(&rand_vec);
	vm_vec_scale_add2(&predicted_center_pos, &rand_vec, objp->radius/2.0f);

	vm_vec_add2(&rand_vec, &objp->orient.vec.fvec);
	if (vm_vec_mag_quick(&rand_vec) < 0.1f)
		vm_vec_add2(&rand_vec, &objp->orient.vec.rvec);
	vm_vec_normalize(&rand_vec);

	speed = Asteroid_info[0].max_speed * (frand()/2.0f + 0.5f);
	
	vm_vec_copy_scale(&asteroid_objp->phys_info.vel, &rand_vec, -speed);
	asteroid_objp->phys_info.desired_vel = asteroid_objp->phys_info.vel;
	vm_vec_scale_add(&asteroid_objp->pos, &predicted_center_pos, &asteroid_objp->phys_info.vel, -delta_time);
	vm_vec_scale_add(&asteroid_objp->last_pos, &asteroid_objp->pos, &asteroid_objp->phys_info.vel, -flFrametime);
}

/**
* Sanitizes the Asteroid targets list removing ships that have died/become invalid etc
*/
static void sanitize_asteroid_targets_list() {
	for (size_t i = 0; i < Asteroid_targets.size();) {
		object* target_objp = &Objects[Asteroid_targets[i].objnum];
		if (target_objp->type != OBJ_SHIP || target_objp->signature != Asteroid_targets[i].signature) {
			Asteroid_targets[i] = Asteroid_targets.back();
			Asteroid_targets.pop_back();
		}
		else  // if we needed to cull we should not advance because we just moved a new asteroid target into this spot
			i++;
	}
}

/**
 * Call once per frame to maybe throw some asteroids at one or more ships.
 *
 */
static void maybe_throw_asteroid()
{

	for (asteroid_target& target : Asteroid_targets) {
		if (!timestamp_elapsed(target.throw_stamp))
			continue;

		object* target_objp = &Objects[target.objnum];
		if (!asteroid_is_ship_inside_field(&Asteroid_field, &target_objp->pos, target_objp->radius))
			continue;
		
		// This should've been greater equal, but alas...
		if (target.incoming_asteroids > The_mission.ai_profile->max_incoming_asteroids[Game_skill_level])
			continue;

		nprintf(("AI", "Incoming asteroids to %s: %i\n", Ships[target_objp->instance].ship_name, target.incoming_asteroids));

		target.throw_stamp = _timestamp(1000 + 1200 * target.incoming_asteroids / (Game_skill_level+1));

		int counter = Random::next(Asteroid_field.num_used_field_debris_types);
		int subtype = -1;
		for (int i = 0; i < NUM_DEBRIS_POFS; i++) {
			if (Asteroid_field.field_debris_type[i] >= 0) {
				if (counter == 0) {
					subtype = i;
					break;
				}
				else
					counter--;
			}
		}

		// this really shouldn't happen but just in case...
		if (subtype < 0)
			return;

		object *objp = asteroid_create(&Asteroid_field, ASTEROID_TYPE_LARGE, subtype);
		if (objp != nullptr) {
			asteroid_aim_at_target(target_objp, objp, ASTEROID_MIN_COLLIDE_TIME + frand() * 20.0f);

			// if asteroid is inside inner bound, kill it
			if (asteroid_in_inner_bound(&Asteroid_field, &objp->pos, 0.0f)) {
				objp->flags.set(Object::Object_Flags::Should_be_dead);
			} else {
				Asteroids[objp->instance].target_objnum = target.objnum;
				target.incoming_asteroids++;

				if ( MULTIPLAYER_MASTER ) {
					send_asteroid_throw( objp );
				}
			}
		}
	}

}

/**
 * Delete asteroid from Asteroid_used_list
 */
void asteroid_delete( object * obj )
{
	int		num;
	asteroid	*asp;

	num = obj->instance;
	Assert( Asteroids[num].objnum == OBJ_INDEX(obj));

	asp = &Asteroids[num];

	if (asp->model_instance_num >= 0)
		model_delete_instance(asp->model_instance_num);

	if (asp->target_objnum >= 0) {
		for (asteroid_target& target : Asteroid_targets) {
			if (asp->target_objnum == target.objnum)
				target.incoming_asteroids--;
		}
	}

	asp->flags = 0;
	Num_asteroids--;
	Assert(Num_asteroids >= 0);

	asteroid_obj_list_remove( obj );
}

/**
 * See if we should reposition the asteroid.  
 * Only reposition if oustide the bounding volume and the player isn't looking towards the asteroid.
 */
static void asteroid_maybe_reposition(object *objp, asteroid_field *asfieldp)
{
	// passive field does not wrap
	if (asfieldp->field_type == FT_PASSIVE) {
		return;
	}

	if ( asteroid_should_wrap(objp, asfieldp) ) {
		vec3d	vec_to_asteroid, old_asteroid_pos, old_vel;
		float		dot, dist;

		old_asteroid_pos = objp->pos;
		old_vel = objp->phys_info.vel;

		// don't wrap asteroid if it is a target of some ship
		if ( !asteroid_is_targeted(objp) ) {

			// only wrap if player won't see asteroid disappear/reverse direction
			dist = vm_vec_normalized_dir(&vec_to_asteroid, &objp->pos, &Eye_position);
			dot = vm_vec_dot(&Eye_matrix.vec.fvec, &vec_to_asteroid);
			
			if ( (dot < 0.7f) || (dist > asfieldp->bound_rad) ) {
				if (Num_asteroids > MAX_ASTEROIDS-10) {
					objp->flags.set(Object::Object_Flags::Should_be_dead);
				} else {
					// check to ensure player won't see asteroid appear either
					asteroid_wrap_pos(objp, asfieldp);
					asteroid* astp = &Asteroids[objp->instance];

					// this doesnt count as a thrown asteroid anymore
					for (asteroid_target& target : Asteroid_targets)
						if (target.objnum == astp->target_objnum)
							target.incoming_asteroids--;

					astp->target_objnum = -1;

					dist = vm_vec_normalized_dir(&vec_to_asteroid, &objp->pos, &Eye_position);
					dot = vm_vec_dot(&Eye_matrix.vec.fvec, &vec_to_asteroid);
					
					if ( (dot > 0.7f) && (dist < (asfieldp->bound_rad * 1.3f)) ) {
						// player would see asteroid pop out other side, so reverse velocity instead of wrapping						
						objp->pos = old_asteroid_pos;		
						vm_vec_copy_scale(&objp->phys_info.vel, &old_vel, -1.0f);
						objp->phys_info.desired_vel = objp->phys_info.vel;
						Asteroids[objp->instance].target_objnum = -1;
					}

					// update last pos (after vel is known)
					vm_vec_scale_add(&objp->last_pos, &objp->pos, &objp->phys_info.vel, -flFrametime);

					asteroid_update_collide(objp);

					if ( MULTIPLAYER_MASTER )
						send_asteroid_throw( objp );
				}
			}
		}
	}
}

static void lerp(float *goal, float f1, float f2, float scale)
{
	*goal = (f2 - f1) * scale + f1;
}

void asteroid_process_pre( object *objp )
{
	if (Asteroids_enabled) {
		//	Make vel chase desired_vel
		lerp(&objp->phys_info.vel.xyz.x, objp->phys_info.vel.xyz.x, objp->phys_info.desired_vel.xyz.x, flFrametime);
		lerp(&objp->phys_info.vel.xyz.y, objp->phys_info.vel.xyz.y, objp->phys_info.desired_vel.xyz.y, flFrametime);
		lerp(&objp->phys_info.vel.xyz.z, objp->phys_info.vel.xyz.z, objp->phys_info.desired_vel.xyz.z, flFrametime);
	}
}

int asteroid_check_collision(object *pasteroid, object *other_obj, vec3d *hitpos, collision_info_struct *asteroid_hit_info, vec3d* hitnormal)
{
	if (!Asteroids_enabled) {
		return 0;
	}

	mc_info	mc;
	mc_info_init(&mc);
	int		num, asteroid_subtype;

	Assert( pasteroid->type == OBJ_ASTEROID );

	num = pasteroid->instance;
	Assert( num >= 0 );

	Assert( Asteroids[num].objnum == OBJ_INDEX(pasteroid));
	asteroid_subtype = Asteroids[num].asteroid_subtype;

	// asteroid_hit_info NULL  --  asteroid-weapon collision
	if ( asteroid_hit_info == NULL ) {
		// asteroid weapon collision
		Assert( other_obj->type == OBJ_WEAPON );
		mc.model_instance_num = Asteroids[num].model_instance_num;
		mc.model_num = Asteroid_info[Asteroids[num].asteroid_type].model_num[asteroid_subtype];	// Fill in the model to check
		mc.orient = &pasteroid->orient;					// The object's orient
		mc.pos = &pasteroid->pos;							// The object's position
		mc.p0 = &other_obj->last_pos;				// Point 1 of ray to check
		mc.p1 = &other_obj->pos;					// Point 2 of ray to check
		mc.flags = (MC_CHECK_MODEL);

		if (model_collide(&mc))
		{
			*hitpos = mc.hit_point_world;

			if (hitnormal)
			{
				vec3d normal;

				if (mc.model_instance_num >= 0)
					model_instance_local_to_global_dir(&normal, &mc.hit_normal, mc.model_instance_num, mc.hit_submodel, mc.orient);
				else
					model_local_to_global_dir(&normal, &mc.hit_normal, mc.model_num, mc.hit_submodel, mc.orient);

				*hitnormal = normal;
			}
		}

		return mc.num_hits;
	}

	// asteroid ship collision -- use asteroid_hit_info to calculate physics
	object *pship_obj = other_obj;
	Assert( pship_obj->type == OBJ_SHIP );

	object* heavy = asteroid_hit_info->heavy;
	object* lighter = asteroid_hit_info->light;
	object *heavy_obj = heavy;
	object *light_obj = lighter;

	vec3d zero, p0, p1;
	vm_vec_zero( &zero );
	vm_vec_sub( &p0, &lighter->last_pos, &heavy->last_pos );
	vm_vec_sub( &p1, &lighter->pos, &heavy->pos );

	mc.pos = &zero;								// The object's position
	mc.p0 = &p0;									// Point 1 of ray to check
	mc.p1 = &p1;									// Point 2 of ray to check

	// find the light object's position in the heavy object's reference frame at last frame and also in this frame.
	vec3d p0_temp, p0_rotated;
		
	// Collision detection from rotation enabled if at rotation is less than 30 degree in frame
	// This should account for all ships
	if ( (vm_vec_mag_squared( &heavy->phys_info.rotvel ) * flFrametime*flFrametime) < (PI*PI/36) ) {
		// collide_rotate calculate (1) start position and (2) relative velocity
		asteroid_hit_info->collide_rotate = true;
		vm_vec_rotate( &p0_temp, &p0, &heavy->last_orient );
		vm_vec_unrotate( &p0_rotated, &p0_temp, &heavy->orient );
		mc.p0 = &p0_rotated;				// Point 1 of ray to check
		vm_vec_sub( &asteroid_hit_info->light_rel_vel, &p1, &p0_rotated );
		vm_vec_scale( &asteroid_hit_info->light_rel_vel, 1/flFrametime );
		// HACK - this applies to big ships warping in/out of asteroid fields - not sure what it does
		if (vm_vec_mag(&asteroid_hit_info->light_rel_vel) > 300) {
			asteroid_hit_info->collide_rotate = false;
			vm_vec_sub( &asteroid_hit_info->light_rel_vel, &lighter->phys_info.vel, &heavy->phys_info.vel );
		}
	} else {
		asteroid_hit_info->collide_rotate = false;
		vm_vec_sub( &asteroid_hit_info->light_rel_vel, &lighter->phys_info.vel, &heavy->phys_info.vel );
	}

	int mc_ret_val = 0;

	if ( asteroid_hit_info->heavy == pship_obj ) {	// ship is heavier, so asteroid is sphere. Check sphere collision against ship poly model
		mc.model_instance_num = Ships[pship_obj->instance].model_instance_num;
		mc.model_num = Ship_info[Ships[pship_obj->instance].ship_info_index].model_num;		// Fill in the model to check
		mc.orient = &pship_obj->orient;								// The object's orient
		mc.radius = pasteroid->radius;
		mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);

		// copy important data
		int copy_flags = mc.flags;  // make a copy of start end positions of sphere in  big ship RF
		vec3d copy_p0, copy_p1;
		copy_p0 = *mc.p0;
		copy_p1 = *mc.p1;

		// first test against the sphere - if this fails then don't do any submodel tests
		mc.flags = MC_ONLY_SPHERE | MC_CHECK_SPHERELINE;

		if (model_collide(&mc)) {

			// Set earliest hit time
			asteroid_hit_info->hit_time = FLT_MAX;

			auto pmi = model_get_instance(Ships[heavy_obj->instance].model_instance_num);
			auto pm = model_get(pmi->model_num);

			// Do collision the cool new way
			if ( asteroid_hit_info->collide_rotate ) {
				// We collide with the sphere, find the list of moving submodels and test one at a time
				SCP_vector<int> submodel_vector;
				model_get_moving_submodel_list(submodel_vector, heavy_obj);

				// turn off all moving submodels, collide against only 1 at a time.
				// turn off collision detection for all moving submodels
				for (auto submodel: submodel_vector) {
					pmi->submodel[submodel].collision_checked = true;
				}

				// Only check single submodel now, since children of moving submodels are handled as moving as well
				mc.flags = copy_flags | MC_SUBMODEL;

				// check each submodel in turn
				for (auto submodel: submodel_vector) {
					auto smi = &pmi->submodel[submodel];

					// turn on just one submodel for collision test
					smi->collision_checked = false;

					// find the start and end positions of the sphere in submodel RF
					model_instance_global_to_local_point(&p0, &light_obj->last_pos, pm, pmi, submodel, &heavy_obj->last_orient, &heavy_obj->last_pos, true);
					model_instance_global_to_local_point(&p1, &light_obj->pos, pm, pmi, submodel, &heavy_obj->orient, &heavy_obj->pos);

					mc.p0 = &p0;
					mc.p1 = &p1;

					mc.orient = &vmd_identity_matrix;
					mc.submodel_num = submodel;

					if ( model_collide(&mc) ) {
						if ( mc.hit_dist < asteroid_hit_info->hit_time ) {
							mc_ret_val = 1;

							// set up asteroid_hit_info common
							set_hit_struct_info(asteroid_hit_info, &mc, true);

							// set up asteroid_hit_info for rotating submodel
							if (!asteroid_hit_info->edge_hit) {
								model_instance_local_to_global_dir(&asteroid_hit_info->collision_normal, &mc.hit_normal, pm, pmi, mc.hit_submodel, &heavy_obj->orient);
							}

							// find position in submodel RF of light object at collison
							vec3d int_light_pos, diff;
							vm_vec_sub(&diff, mc.p1, mc.p0);
							vm_vec_scale_add(&int_light_pos, mc.p0, &diff, mc.hit_dist);
							model_instance_local_to_global_point(&asteroid_hit_info->light_collision_cm_pos, &int_light_pos, pm, pmi, mc.hit_submodel, &heavy_obj->orient, &zero);
						}
					}
					// Don't look at this submodel again
					smi->collision_checked = true;
				}

			}

			// Now complete base model collision checks that do not take into account rotating submodels.
			mc.flags = copy_flags;
			*mc.p0 = copy_p0;
			*mc.p1 = copy_p1;
			mc.orient = &heavy_obj->orient;

			// usual ship_ship collision test
			if ( model_collide(&mc) )	{
				// check if this is the earliest hit
				if (mc.hit_dist < asteroid_hit_info->hit_time) {
					mc_ret_val = 1;

					set_hit_struct_info(asteroid_hit_info, &mc, false);

					// get collision normal if not edge hit
					if (!asteroid_hit_info->edge_hit) {
						model_instance_local_to_global_dir(&asteroid_hit_info->collision_normal, &mc.hit_normal, pm, pmi, mc.hit_submodel, &heavy_obj->orient);
					}

					// find position in submodel RF of light object at collison
					vec3d diff;
					vm_vec_sub(&diff, mc.p1, mc.p0);
					vm_vec_scale_add(&asteroid_hit_info->light_collision_cm_pos, mc.p0, &diff, mc.hit_dist);

				}
			}
		}

	} else {
		// Asteroid is heavier obj
		mc.model_instance_num = Asteroids[num].model_instance_num;
		mc.model_num = Asteroid_info[Asteroids[num].asteroid_type].model_num[asteroid_subtype];		// Fill in the model to check
		mc.orient = &pasteroid->orient;				// The object's orient
		mc.radius = model_get_core_radius(Ship_info[Ships[pship_obj->instance].ship_info_index].model_num);

		// check for collision between asteroid model and ship sphere
		mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);

		mc_ret_val = model_collide(&mc);

		if (mc_ret_val) {
			set_hit_struct_info(asteroid_hit_info, &mc, false);

			// set normal if not edge hit
			if ( !asteroid_hit_info->edge_hit ) {
				vm_vec_unrotate(&asteroid_hit_info->collision_normal, &mc.hit_normal, &heavy->orient);
			}

			// find position in submodel RF of light object at collison
			vec3d diff;
			vm_vec_sub(&diff, mc.p1, mc.p0);
			vm_vec_scale_add(&asteroid_hit_info->light_collision_cm_pos, mc.p0, &diff, mc.hit_dist);

		}
	}

	// check if the hit point is beyond the clip plane if the ship is warping.
	if (mc_ret_val) {
		WarpEffect* warp_effect = nullptr;
		ship* shipp = &Ships[pship_obj->instance];

		// this is extremely confusing but mc.hit_point_world isn't actually in world coords
		// everything above was calculated relative to the heavy's position
		vec3d actual_world_hit_pos = mc.hit_point_world + heavy_obj->pos;
		if ((shipp->is_arriving()) && (shipp->warpin_effect != nullptr))
			warp_effect = shipp->warpin_effect;
		else if ((shipp->flags[Ship::Ship_Flags::Depart_warp]) && (shipp->warpout_effect != nullptr))
			warp_effect = shipp->warpout_effect;

		if (warp_effect != nullptr && point_is_clipped_by_warp(&actual_world_hit_pos, warp_effect))
			mc_ret_val = 0;
	}
	

	if ( mc_ret_val )	{

		// SET PHYSICS PARAMETERS
		// already have (hitpos - heavy) and light_cm_pos

		// get r_heavy and r_light
		asteroid_hit_info->r_heavy = asteroid_hit_info->hit_pos;
		vm_vec_sub(&asteroid_hit_info->r_light, &asteroid_hit_info->hit_pos, &asteroid_hit_info->light_collision_cm_pos);

		// set normal for edge hit
		if ( asteroid_hit_info->edge_hit ) {
			vm_vec_copy_normalize(&asteroid_hit_info->collision_normal, &asteroid_hit_info->r_light);
			vm_vec_negate(&asteroid_hit_info->collision_normal);
		}

		// get world hitpos
		vm_vec_add(hitpos, &asteroid_hit_info->heavy->pos, &asteroid_hit_info->r_heavy);

		return 1;
	} else {
		// no hit
		return 0;
	}
}

void asteroid_render(object * obj, model_draw_list *scene)
{
	if (Asteroids_enabled) {
		int			num;
		asteroid		*asp;
		
		num = obj->instance;

		Assert((num >= 0) && (num < MAX_ASTEROIDS));
		asp = &Asteroids[num];

		Assert( asp->flags & AF_USED );

		model_clear_instance( Asteroid_info[asp->asteroid_type].model_num[asp->asteroid_subtype]);

		model_render_params render_info;

		render_info.set_object_number( OBJ_INDEX(obj) );
		render_info.set_flags(MR_IS_ASTEROID);

		model_render_queue(&render_info, scene, Asteroid_info[asp->asteroid_type].model_num[asp->asteroid_subtype], &obj->orient, &obj->pos);	//	Replace MR_NORMAL with 0x07 for big yellow blobs
	}
}

/**
 * Create a normalized vector generally in the direction from *hitpos to other_obj->pos
 */
static void asc_get_relvec(vec3d *relvec, object *other_obj, vec3d *hitpos)
{
	vec3d	tvec, rand_vec;
	int		count = 0;

	vm_vec_normalized_dir(&tvec, &other_obj->pos, hitpos);

	//	Try up to three times to get a good vector.
	while (count++ < 3) {
		vm_vec_rand_vec_quick(&rand_vec);
		vm_vec_add(relvec, &tvec, &rand_vec);
		float mag = vm_vec_mag_quick(relvec);
		if ((mag > 0.2f) && (mag < 1.7f))
			break;
	}

	vm_vec_normalize_quick(relvec);
}

/**
 * Return multiplier on asteroid radius for fireball
 */
static float asteroid_get_fireball_scale_multiplier(int num)
{
	if (Asteroids[num].flags & AF_USED) {
		asteroid_info *asip = &Asteroid_info[Asteroids[num].asteroid_type];
		
		if (asip->fireball_radius_multiplier >= 0) {
			return asip->fireball_radius_multiplier;
		} else {
			switch(Asteroids[num].asteroid_type) {
			case ASTEROID_TYPE_LARGE:
				return 1.5f;
				break;

			default:
				return 1.0f;
				break;
			}
		}
	}

	Int3();	// this should not happen.  asteroid should be used.
	return 1.0f;
}


/**
 * Create asteroid explosion
 * @return expected time for explosion anim to last, in seconds
 */
static float asteroid_create_explosion(object *objp)
{
	int	fireball_objnum;
	float	explosion_life, fireball_scale_multiplier;
	asteroid_info *asip = &Asteroid_info[Asteroids[objp->instance].asteroid_type];

	int fireball_type = fireball_asteroid_explosion_type(asip);
	if (fireball_type < 0) {
		fireball_type = FIREBALL_ASTEROID;
	}

	if (fireball_type >= Num_fireball_types) {
		Warning(LOCATION, "Invalid fireball type %i specified for an asteroid, only %i fireball types are defined.", fireball_type, Num_fireball_types);

		return 0;
	}

	fireball_scale_multiplier = asteroid_get_fireball_scale_multiplier(objp->instance);

	fireball_objnum = fireball_create( &objp->pos, fireball_type, FIREBALL_LARGE_EXPLOSION, OBJ_INDEX(objp), objp->radius*fireball_scale_multiplier, false, &objp->phys_info.vel );
	if ( fireball_objnum > -1 )	{
		explosion_life = fireball_lifeleft(&Objects[fireball_objnum]);
	} else {
		explosion_life = 0.0f;
	}

	return explosion_life;
}

/**
 * Play sound when asteroid explodes
 */
static void asteroid_explode_sound(object *objp, int type, int play_loud)
{
	gamesnd_id sound_index;
	float range_factor = 1.0f;		// how many times sound should traver farther than normal

	if (type % NUM_DEBRIS_SIZES <= 1)
	{
		sound_index = gamesnd_id(GameSounds::ASTEROID_EXPLODE_SMALL);
		range_factor = 5.0;
	}
	else
	{
		sound_index = gamesnd_id(GameSounds::ASTEROID_EXPLODE_LARGE);
		range_factor = 10.0f;
	}

	Assert(sound_index.isValid());

	if ( !play_loud ) {
		range_factor = 1.0f;
	}

	snd_play_3d( gamesnd_get_game_sound(sound_index), &objp->pos, &Eye_position, objp->radius, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY, NULL, range_factor );
}

/**
 * Do the area effect for an asteroid exploding
 *
 * @param asteroid_objp	object pointer to asteroid causing explosion
 */
static void asteroid_do_area_effect(object *asteroid_objp)
{
	object			*ship_objp;
	float				damage, blast;
	ship_obj			*so;
	asteroid			*asp;
	asteroid_info	*asip;

	asp = &Asteroids[asteroid_objp->instance];
	asip = &Asteroid_info[asp->asteroid_type];

	if ( asip->damage <= 0 ) {		// do a quick out if there is no damage to apply
		return;
	}

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		ship_objp = &Objects[so->objnum];
	
		// don't blast navbuoys
		if ( ship_get_SIF(ship_objp->instance)[Ship::Info_Flags::Navbuoy] ) {
			continue;
		}

		if ( weapon_area_calc_damage(ship_objp, &asteroid_objp->pos, asip->inner_rad, asip->outer_rad, asip->blast, asip->damage, &blast, &damage, asip->outer_rad) == -1 )
			continue;

		ship_apply_global_damage(ship_objp, asteroid_objp, &asteroid_objp->pos, damage, asip->damage_type_idx);
		weapon_area_apply_blast(NULL, ship_objp, &asteroid_objp->pos, blast, 0);
	}	// end for
}

/**
 * Upon asteroid asteroid_obj being hit. Apply damage and maybe make it break into smaller asteroids.
 *
 * @param pasteroid_obj		pointer to asteroid object getting hit
 * @param other_obj		object that hit asteroid, can be NULL if asteroid hit by area effect
 * @param hitpos		world position asteroid was hit, can be NULL if hit by area effect
 * @param damage		amount of damage to apply to asteroid
 */
void asteroid_hit( object * pasteroid_obj, object * other_obj, vec3d * hitpos, float damage )
{
	float		explosion_life;
	asteroid	*asp;

	asp = &Asteroids[pasteroid_obj->instance];

	if (pasteroid_obj->flags[Object::Object_Flags::Should_be_dead]){
		return;
	}

	if ( MULTIPLAYER_MASTER ){
		send_asteroid_hit( pasteroid_obj, other_obj, hitpos, damage );
	}

	pasteroid_obj->hull_strength -= damage;

	if (pasteroid_obj->hull_strength < 0.0f) {
		if ( !asp->final_death_time.isValid() ) {
			int play_loud_collision = 0;

			explosion_life = asteroid_create_explosion(pasteroid_obj);

			asteroid_explode_sound(pasteroid_obj, asp->asteroid_type, play_loud_collision);
			asteroid_do_area_effect(pasteroid_obj);

			asp->final_death_time = _timestamp( fl2i(explosion_life*MILLISECONDS_PER_SECOND)/5 );	// Wait till 30% of vclip time before breaking the asteroid up.
			if ( hitpos ) {
				asp->death_hit_pos = *hitpos;
			} else {
				asp->death_hit_pos = pasteroid_obj->pos;
				// randomize hit pos a bit, otherwise we will get a NULL vector when trying to find direction to toss child asteroids
				vec3d rand_vec;
				vm_vec_rand_vec_quick(&rand_vec);
				vm_vec_add2(&asp->death_hit_pos, &rand_vec);
			}
		}
	} else if ( other_obj ) {
		if ( other_obj->type == OBJ_WEAPON ) {
			weapon_info *wip;
			wip = &Weapon_info[Weapons[other_obj->instance].weapon_info_index];
			// If the weapon didn't play any impact animation, play custom asteroid impact animation
			if (!wip->impact_weapon_expl_effect.isValid()) {
				particle::create( hitpos, &vmd_zero_vector, 0.0f, Asteroid_impact_explosion_radius, particle::PARTICLE_BITMAP, Asteroid_impact_explosion_ani );
			}
		}
	}

	// evaluate any relevant player scoring implications
	scoring_eval_hit(pasteroid_obj,other_obj);
}

/**
 * De-init asteroids, called from ::game_level_close()
 */
void asteroid_level_close()
{
	int	i;

	for (i=0; i<MAX_ASTEROIDS; i++) {
		if (Asteroids[i].flags & AF_USED) {
			Asteroids[i].flags &= ~AF_USED;
			Assert(Asteroids[i].objnum >=0 && Asteroids[i].objnum < MAX_OBJECTS);
			Objects[Asteroids[i].objnum].flags.set(Object::Object_Flags::Should_be_dead);
		}
	}

	Asteroid_field.num_initial_asteroids=0;
}

DCF_BOOL2(asteroids, Asteroids_enabled, "enables or disables asteroids", "Usage: asteroids [bool]\nTurns asteroid system on/off.  If nothing passed, then toggles it.\n");


/**
 * Return the number of active asteroids
 */
int asteroid_count()
{
	return Num_asteroids;
}

/**
 * See if asteroid should split up.  
 * We delay splitting up to allow the explosion animation to play for a bit.
 */
static void asteroid_maybe_break_up(object *pasteroid_obj)
{
	asteroid *asp;
	asteroid_info *asip;

	asp = &Asteroids[pasteroid_obj->instance];
	asip = &Asteroid_info[asp->asteroid_type];

	if ( timestamp_elapsed(asp->final_death_time) ) {
		vec3d	relvec, vfh, tvec;
		bool skip = false;
		bool hooked = scripting::hooks::OnDeath->isActive();

		if (hooked) {
			skip = scripting::hooks::OnDeath->isOverride(
				scripting::hook_param_list(scripting::hook_param("Self", 'o', pasteroid_obj)),
				pasteroid_obj);
		}
		if (!skip)
		{
			pasteroid_obj->flags.set(Object::Object_Flags::Should_be_dead);

			// multiplayer clients won't go through the following code.  asteroid_sub_create will send
			// a create packet to the client in the above named function
			// if the second condition isn't true it's just debris, and debris doesn't break up
			if ( !MULTIPLAYER_CLIENT && (asp->asteroid_type <= ASTEROID_TYPE_LARGE)) {
				if (asip->split_info.empty()) {
					switch (asp->asteroid_type) {
						case ASTEROID_TYPE_SMALL:
							break;
						case ASTEROID_TYPE_MEDIUM:
							asc_get_relvec(&relvec, pasteroid_obj, &asp->death_hit_pos);
							asteroid_sub_create(pasteroid_obj, ASTEROID_TYPE_SMALL, &relvec);
						
							vm_vec_normalized_dir(&vfh, &pasteroid_obj->pos, &asp->death_hit_pos);
							vm_vec_copy_scale(&tvec, &vfh, 2.0f);
							vm_vec_sub2(&tvec, &relvec);
							asteroid_sub_create(pasteroid_obj, ASTEROID_TYPE_SMALL, &tvec);
							
							break;
						case ASTEROID_TYPE_LARGE:
							asc_get_relvec(&relvec, pasteroid_obj, &asp->death_hit_pos);
							asteroid_sub_create(pasteroid_obj, ASTEROID_TYPE_MEDIUM, &relvec);
						
							vm_vec_normalized_dir(&vfh, &pasteroid_obj->pos, &asp->death_hit_pos);
							vm_vec_copy_scale(&tvec, &vfh, 2.0f);
							vm_vec_sub2(&tvec, &relvec);
							asteroid_sub_create(pasteroid_obj, ASTEROID_TYPE_MEDIUM, &tvec);

							while (frand() > 0.6f) {
								vec3d	rvec, tvec2;
								vm_vec_rand_vec_quick(&rvec);
								vm_vec_scale_add(&tvec2, &vfh, &rvec, 0.75f);
								asteroid_sub_create(pasteroid_obj, ASTEROID_TYPE_SMALL, &tvec2);
							}
							break;

						default: // this isn't going to happen.. really
							break;
					}
				} else {
					SCP_vector<int> roids_to_create;
					SCP_vector<asteroid_split_info>::iterator split;
					for (split = asip->split_info.begin(); split != asip->split_info.end(); ++split) {
						int num_roids = split->min;
						int num_roids_var = split->max - split->min;

						if (num_roids_var > 0)
							num_roids += Random::next(num_roids_var);

						if (num_roids > 0)
							for (int i=0; i<num_roids; i++)
								roids_to_create.push_back(split->asteroid_type);
					}

					random_shuffle(roids_to_create.begin(), roids_to_create.end());

					size_t total_roids = roids_to_create.size();
					for (size_t i = 0; i < total_roids; i++) {
						vec3d dir_vec,final_vec;
						vec3d parent_vel,hit_rel_vec;

						// The roid directions are picked from the so-called
						// "golden section spiral" to prevent them from
						// clustering and thus clipping

						float inc = PI * (3.0f - sqrt(5.0f));
						float offset = 2.0f / total_roids;

						float y = i * offset - 1 + (offset / 2);
						float r = sqrt(1.0f - y*y);
						float phi = i * inc;

						dir_vec.xyz.x = cosf(phi)*r;
						dir_vec.xyz.y = sinf(phi)*r;
						dir_vec.xyz.z = y;

						// Randomize the direction a bit
						vec3d tempv = dir_vec;
						vm_vec_random_cone(&dir_vec, &tempv, (360.0f / total_roids / 2));

						// Make the roid inherit half of parent's directional movement
						if (!IS_VEC_NULL(&pasteroid_obj->phys_info.vel)) {
							vm_vec_copy_normalize(&parent_vel, &pasteroid_obj->phys_info.vel);
						} else {
							vm_vec_rand_vec_quick(&parent_vel);
						}
						vm_vec_scale(&parent_vel, 0.5f);

						// Make the hit position affect the direction, but only a little
						vm_vec_sub(&hit_rel_vec, &pasteroid_obj->pos, &asp->death_hit_pos);
						vm_vec_normalize(&hit_rel_vec);
						vm_vec_scale(&hit_rel_vec, 0.25f);

						vm_vec_avg3(&final_vec, &parent_vel, &hit_rel_vec, &dir_vec);
						vm_vec_normalize(&final_vec);

						asteroid_sub_create(pasteroid_obj, roids_to_create[i], &final_vec);
					}
				}
			}
			asp->final_death_time = TIMESTAMP::invalid();
		}
		if (hooked) {
			scripting::hooks::OnDeath->run(
				scripting::hook_param_list(scripting::hook_param("Self", 'o', pasteroid_obj)),
				pasteroid_obj);
		}
		if (scripting::hooks::OnAsteroidDeath->isActive()) {
			scripting::hooks::OnAsteroidDeath->run(
				scripting::hook_param_list(
					scripting::hook_param("Asteroid", 'o', pasteroid_obj),
					scripting::hook_param("Hitpos", 'o', asp->death_hit_pos)),
				pasteroid_obj);
		}
	}
}

static void asteroid_test_collide(object *pasteroid_obj, object *pship_obj, mc_info *mc, bool lazy = false)
{
	float		asteroid_ray_dist;
	vec3d	asteroid_fvec, terminus;

	// See if ray from asteroid intersects bounding box of escort ship
	asteroid_ray_dist = vm_vec_mag_quick(&pasteroid_obj->phys_info.desired_vel) * ASTEROID_MIN_COLLIDE_TIME;
	asteroid_fvec = pasteroid_obj->phys_info.desired_vel;

	if(IS_VEC_NULL_SQ_SAFE(&asteroid_fvec)){
		terminus = pasteroid_obj->pos;
	} else {
		vm_vec_normalize(&asteroid_fvec);
		vm_vec_scale_add(&terminus, &pasteroid_obj->pos, &asteroid_fvec, asteroid_ray_dist);
	}

	Assert(pship_obj->type == OBJ_SHIP);

	mc->model_instance_num = Ships[pship_obj->instance].model_instance_num;
	mc->model_num = Ship_info[Ships[pship_obj->instance].ship_info_index].model_num;			// Fill in the model to check
	mc->orient = &pship_obj->orient;										// The object's orientation
	mc->pos = &pship_obj->pos;												// The object's position
	mc->p0 = &pasteroid_obj->pos;											// Point 1 of ray to check
	mc->p1 = &terminus;														// Point 2 of ray to check
	if (lazy) {
		mc->flags = MC_CHECK_MODEL | MC_ONLY_BOUND_BOX;
	} else {
		mc->flags = MC_CHECK_MODEL | MC_CHECK_SPHERELINE;
	}
	mc->radius = pasteroid_obj->radius;

	model_collide(mc);

	// PVS-Studio says that mc->p1 will become invalid... on the one hand, it will; on the other hand, we won't use it again; on the *other* other hand, we should keep things proper in case of future changes
	mc->p1 = NULL;
}

/**
 * Test if asteroid will collide with escort ship within ASTEROID_MIN_COLLIDE_TIME seconds
 *
 * @return !0 if the asteroid will collide with the escort
 */
static int asteroid_will_collide(object *pasteroid_obj, object *escort_objp)
{
	mc_info	mc;
	mc_info_init(&mc);

	asteroid_test_collide(pasteroid_obj, escort_objp, &mc);

	if ( !mc.num_hits ) {
		return 0;
	}	

	return 1;
}

/**
 * Warn if asteroid on collision path with ship
 * @return !0 if we should warn about asteroid hitting ship, otherwise return 0
 */
static int asteroid_valid_ship_to_warn_collide(ship *shipp)
{
	if ( !(Ship_info[shipp->ship_info_index].is_big_or_huge()) ) {
		return 0;
	}

	if ( shipp->flags[Ship::Ship_Flags::Dying] || shipp->flags[Ship::Ship_Flags::Depart_warp] ) {
		return 0;
	}

	// Goober5000 used to be if teams were unequal and player was not traitor, but this works for allies not on your team
	if ( iff_x_attacks_y(Player_ship->team, shipp->team) ) {
		return 0;
	}

	return 1;
}

/**
 * See if asteroid will collide with a large ship on the escort list in the next
 * ASTEROID_MIN_COLLIDE_TIME seconds.
 */
static void asteroid_update_collide_flag(object *asteroid_objp)
{
	int		i, num_escorts, escort_objnum, will_collide=0;
	ship		*escort_shipp;
	asteroid	*asp;

	asp = &Asteroids[asteroid_objp->instance];
	asp->collide_objnum = -1;
	asp->collide_objsig = -1;

	// multiplayer dogfight
	if(MULTI_DOGFIGHT){
		return;
	}

	num_escorts = hud_escort_num_ships_on_list();

	if ( num_escorts <= 0 ) {
		return;
	}

	for ( i = 0; i < num_escorts; i++ ) {
		escort_objnum = hud_escort_return_objnum(i);
		if ( escort_objnum >= 0 ) {
			escort_shipp = &Ships[Objects[escort_objnum].instance];
			if ( asteroid_valid_ship_to_warn_collide(escort_shipp) ) {
				will_collide = asteroid_will_collide(asteroid_objp, &Objects[escort_objnum]);
				if ( will_collide ) {
					asp->collide_objnum = escort_objnum;
					asp->collide_objsig = Objects[escort_objnum].signature;
				}
			}
		}
	}
}

/**
 * Ensure that the collide objnum for the asteroid is still valid
 */
static void asteroid_verify_collide_objnum(asteroid *asp)
{
	if ( asp->collide_objnum >= 0 ) {
		if ( Objects[asp->collide_objnum].signature != asp->collide_objsig ) {
			asp->collide_objnum = -1;
			asp->collide_objsig = -1;
		}
	}
}

void asteroid_process_post(object * obj)
{
	if (Asteroids_enabled) {
		int num;
		num = obj->instance;
		
		asteroid	*asp = &Asteroids[num];

		// Only wrap if active field
		if (Asteroid_field.field_type == FT_ACTIVE) {
			if ( timestamp_elapsed(asp->check_for_wrap) ) {
				asteroid_maybe_reposition(obj, &Asteroid_field);
				asp->check_for_wrap = _timestamp(ASTEROID_CHECK_WRAP_TIMESTAMP);
			}
		}

		asteroid_verify_collide_objnum(asp);

		if ( timestamp_elapsed(asp->check_for_collide) ) {
			asteroid_update_collide_flag(obj);
			asp->check_for_collide = _timestamp(ASTEROID_UPDATE_COLLIDE_TIMESTAMP);
		}

		asteroid_maybe_break_up(obj);
	}
}

/**
 * Find the object number of the object the asteroid is about to impact
 * @return the object number that the asteroid is about to impact
 */
int asteroid_collide_objnum(object *asteroid_objp)
{
	return Asteroids[asteroid_objp->instance].collide_objnum;
}

/**
 * Find the time until asteroid will collide with object
 * @return the time until the asteroid will impact its collide_objnum
 */
float asteroid_time_to_impact(object *asteroid_objp)
{
	float		time=-1.0f, total_dist, speed;
	asteroid	*asp;
	mc_info	mc;
	mc_info_init(&mc);

	asp = &Asteroids[asteroid_objp->instance];

	if ( asp->collide_objnum < 0 ) {
		return time;
	}
	
	asteroid_test_collide(asteroid_objp, &Objects[asp->collide_objnum], &mc, true);

	if ( mc.num_hits ) {
		total_dist = vm_vec_dist(&mc.hit_point_world, &asteroid_objp->pos) - asteroid_objp->radius;
		if ( total_dist < 0 ) {
			total_dist = 0.0f;
		}
		speed = vm_vec_mag(&asteroid_objp->phys_info.vel);
		time = total_dist/speed;
	}	

	return time;
}

/**
 * Read in a single asteroid section from asteroid.tbl
 */
static void asteroid_parse_section(asteroid_info *asip)
{
	required_string("$Name:");
	stuff_string(asip->name, F_NAME, NAME_LENGTH);

	required_string( "$POF file1:" );
	stuff_string(asip->pof_files[0], F_NAME, MAX_FILENAME_LEN);

	required_string( "$POF file2:" );
	stuff_string(asip->pof_files[1], F_NAME, MAX_FILENAME_LEN);

	if ( (stristr(asip->name, "Asteroid") != NULL) ) {
		required_string( "$POF file3:" );
		stuff_string(asip->pof_files[2], F_NAME, MAX_FILENAME_LEN);
	}

	required_string("$Detail distance:");
	asip->num_detail_levels = (int)stuff_int_list(asip->detail_distance, MAX_ASTEROID_DETAIL_LEVELS, RAW_INTEGER_TYPE);

	required_string("$Max Speed:");
	stuff_float(&asip->max_speed);

	if(optional_string("$Damage Type:")) {
		char buf[NAME_LENGTH];
		stuff_string(buf, F_NAME, NAME_LENGTH);
		asip->damage_type_idx_sav = damage_type_add(buf);
		asip->damage_type_idx = asip->damage_type_idx_sav;
	}
	
	if(optional_string("$Explosion Animations:")){
		int temp[MAX_FIREBALL_TYPES];
		auto parsed_ints = stuff_int_list(temp, MAX_FIREBALL_TYPES, RAW_INTEGER_TYPE);
		asip->explosion_bitmap_anims.clear();
		asip->explosion_bitmap_anims.insert(asip->explosion_bitmap_anims.begin(), temp, temp+parsed_ints);
	}

	if(optional_string("$Explosion Radius Mult:"))
		stuff_float(&asip->fireball_radius_multiplier);

	required_string("$Expl inner rad:");
	stuff_float(&asip->inner_rad);

	required_string("$Expl outer rad:");
	stuff_float(&asip->outer_rad);

	required_string("$Expl damage:");
	stuff_float(&asip->damage);

	required_string("$Expl blast:");
	stuff_float(&asip->blast);

	required_string("$Hitpoints:");
	stuff_float(&asip->initial_asteroid_strength);

	while(optional_string("$Split:")) {
		int split_type;

		stuff_int(&split_type);

		if (split_type>=0 && split_type<NUM_DEBRIS_SIZES) {
			asteroid_split_info new_split;

			new_split.asteroid_type = split_type;

			if(optional_string("+Min:"))
				stuff_int(&new_split.min);
			else
				new_split.min = 0;

			if(optional_string("+Max:"))
				stuff_int(&new_split.max);
			else
				new_split.max = 0;

			asip->split_info.push_back( new_split );
		} else
			Warning(LOCATION, "Invalid asteroid reference %i used for $Split in asteroids table, ignoring.", split_type);
	}

	if (optional_string("$Spawn Weight:")) {
		stuff_float(&asip->spawn_weight);
		if (asip->spawn_weight <= 0.0f) {
			Warning(LOCATION, "Spawn weight for asteroid '%s' must be greater than 0", asip->name);
			asip->spawn_weight = 1.0f;
		}
	} else {
		switch (Asteroid_info.size() % NUM_DEBRIS_SIZES)
		{
			case ASTEROID_TYPE_SMALL:
				asip->spawn_weight = SMALL_DEBRIS_WEIGHT;
				break;
			case ASTEROID_TYPE_MEDIUM:
				asip->spawn_weight = MEDIUM_DEBRIS_WEIGHT;
				break;
			case ASTEROID_TYPE_LARGE:
				asip->spawn_weight = LARGE_DEBRIS_WEIGHT;
				break;
			default:
				UNREACHABLE("Here be dragons");
		}
	}

	if (optional_string("$Gravity Const:")) {
		stuff_float(&asip->gravity_const);
	}
}

// changes the name to "[species] Debris" if it had a name like "[species] debris #"
void maybe_change_asteroid_name(asteroid_info* asip) {

	SCP_string name = asip->name;
	size_t split = std::string::npos;

	for (species_info species : Species_info) {
		if (name.compare(0, strlen(species.species_name), species.species_name) == 0) {
			split = strlen(species.species_name);
			break;
		}
	}

	if (split == std::string::npos)
		return;

	SCP_string remaining_name = name.substr(split + 1, name.length());

	split = remaining_name.find(' ');

	if (split == std::string::npos)
		return;

	SCP_string debris = remaining_name.substr(0, split);
	SCP_string num = remaining_name.substr(split + 1, remaining_name.length());

	if (stricmp(debris.c_str(), "Debris") != 0)
		return;

	if (num.empty() || std::find_if(num.begin(), num.end(), [](char c) {
	                       return !std::isdigit(c, SCP_default_locale);
	                   }) != num.end())
		return;

	// make sure this asteroid would correspond the 'species section' of the old style retail asteroids
	if (Asteroid_info.size() < NUM_DEBRIS_SIZES + Species_info.size() * NUM_DEBRIS_SIZES && Asteroid_info.size() >= NUM_DEBRIS_SIZES) {
		int idx = (int)(Asteroid_info.size()) / NUM_DEBRIS_SIZES - 1;
		strcpy_s(asip->name, Species_info[idx].species_name);
		strcat(asip->name, " ");
		strcat(asip->name, XSTR("debris", 348));
	}
}

/**
Read in data from asteroid.tbl into Asteroid_info[] array.

After this function is complete, the Asteroid_info[] array 
will have at least 3 generic asteroids plus whatever else 
special ones were added in the table.  The generic asteroids are 
a set of small, medium, and large asteroids in that exact order.

Note that by saying "asteroid" this code is talking about
the asteroids and the debris that make up asteroid fields
and debris fields. Which means that these debris have nothing
to do with the debris of ships that explode, however these
are the same debris and asteroids that get flung at a ship
that is being protected.
*/
static void asteroid_parse_tbl()
{
	char impact_ani_file[MAX_FILENAME_LEN];
	
	try
	{
		read_file_text("asteroid.tbl", CF_TYPE_TABLES);
		reset_parse();

		required_string("#Asteroid Types");

		size_t tally = 0;

#ifndef NDEBUG
		SCP_vector<SCP_string> parsed_asteroids;
#endif

		// parse and tally each asteroid
		while (required_string_either("#End", "$Name:"))
		{
			asteroid_info new_asteroid;

			asteroid_parse_section(&new_asteroid);

			maybe_change_asteroid_name(&new_asteroid);

#ifndef NDEBUG
			SCP_string msg;
			msg.append("Parsing asteroid: '");
			msg.append(new_asteroid.name);
			msg.append("' as a '");
			msg.append((tally >= NUM_DEBRIS_SIZES) ? "generic" : "special");
			msg.append("'");
			switch (tally % NUM_DEBRIS_SIZES) {
			case ASTEROID_TYPE_SMALL:
				msg.append(" small\n");
				break;
			case ASTEROID_TYPE_MEDIUM:
				msg.append(" medium\n");
				break;
			case ASTEROID_TYPE_LARGE:
				msg.append(" large\n");
				break;
			default:
				Error(LOCATION, "Get a coder! Math has broken!\n"
					"Important numbers:\n"
					"\ttally: " SIZE_T_ARG "\n"
					"\tNUM_DEBRIS_SIZES: %d\n",
					tally, NUM_DEBRIS_SIZES
					);
				msg.append(" unknown\n");
			}
			parsed_asteroids.push_back(msg);
#endif
			Asteroid_info.push_back(new_asteroid);
			tally++;
		}
		required_string("#End");

		Asteroid_impact_explosion_ani = -1;
		required_string("$Impact Explosion:");
		stuff_string(impact_ani_file, F_NAME, MAX_FILENAME_LEN);

		if (VALID_FNAME(impact_ani_file)) {
			int num_frames;
			Asteroid_impact_explosion_ani = bm_load_animation(impact_ani_file, &num_frames, nullptr, nullptr, nullptr, true);
		}

		required_string("$Impact Explosion Radius:");
		stuff_float(&Asteroid_impact_explosion_radius);

		if (optional_string("$Briefing Icon Closeup Model:")) {
			stuff_string(Asteroid_icon_closeup_model, F_NAME, NAME_LENGTH);
		}
		else {
			strcpy_s(Asteroid_icon_closeup_model, Asteroid_info[ASTEROID_TYPE_LARGE].pof_files[0]);	// magic file from retail
		}

		if (optional_string("$Briefing Icon Closeup Position:")) {
			stuff_vec3d(&Asteroid_icon_closeup_position);
		}
		else {
			vm_vec_make(&Asteroid_icon_closeup_position, 0.0f, 0.0f, -334.0f);  // magic numbers from retail
		}

		if (optional_string("$Briefing Icon Closeup Zoom:")) {
			stuff_float(&Asteroid_icon_closeup_zoom);
		}
		else {
			Asteroid_icon_closeup_zoom = 0.5f;	// magic number from retail
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", "asteroid.tbl", e.what()));
		return;
	}
}

/**
 * Pick object to throw asteroids at.
 * Pick any capital or big ship inside the bounds of the asteroid field.
 */
static int set_asteroid_throw_objnum()
{
	if (Asteroid_field.num_initial_asteroids < 1)
		return -1;

	ship_obj	*so;
	object	*ship_objp;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		ship_objp = &Objects[so->objnum];

		if (Ship_info[Ships[ship_objp->instance].ship_info_index].is_big_or_huge()) {
			if (asteroid_is_ship_inside_field(&Asteroid_field, &ship_objp->pos, ship_objp->radius))
				return so->objnum;
		}
	}
	return -1;

}

void asteroid_frame()
{
	if (Num_asteroids < 1)
		return;

	// Only throw if active field
	if (Asteroid_field.field_type == FT_PASSIVE) {
		return;
	}

	// If there are no explicit targets, fall back to default retail targeting behavior
	if (Asteroid_field.target_names.empty()) {
		int objnum = set_asteroid_throw_objnum();
		if (Asteroid_targets.empty() || Asteroid_targets[0].objnum != objnum) {
			Asteroid_targets.clear();
			if (objnum >= 0)
				asteroid_add_target(&Objects[objnum]);
		}
	} 

	sanitize_asteroid_targets_list();

	maybe_throw_asteroid();
}

/**
 * Called once, at game start.  Do any one-time initializations here
 */
void asteroid_init()
{
	asteroid_parse_tbl();
}

extern bool Extra_target_info;

/**
 * Draw brackets around on-screen asteroids that are about to collide, otherwise draw an offscreen indicator
 */
void asteroid_show_brackets()
{
	vertex	asteroid_vertex;
	object	*asteroid_objp, *player_target;
	asteroid	*asp;

	// get pointer to player target, so we don't need to take OBJ_INDEX() of asteroid_objp to compare to target_objnum
	if ( Player_ai->target_objnum >= 0 ) {
		player_target = &Objects[Player_ai->target_objnum];
	} else {
		player_target = NULL;
	}

	for ( asteroid_objp = GET_FIRST(&obj_used_list); asteroid_objp !=END_OF_LIST(&obj_used_list); asteroid_objp = GET_NEXT(asteroid_objp) ) {
		if (asteroid_objp->type != OBJ_ASTEROID ) {
			continue;
		}

		asp = &Asteroids[asteroid_objp->instance];

		if ( asp->collide_objnum < 0 ) {
			continue;
		}

		if ( asteroid_objp == player_target ) {
			continue;
		}

		g3_rotate_vertex(&asteroid_vertex,&asteroid_objp->pos);
		g3_project_vertex(&asteroid_vertex);

		if ( Extra_target_info ) {
			hud_target_add_display_list(asteroid_objp, &asteroid_vertex, &asteroid_objp->pos, 0, NULL, NULL, TARGET_DISPLAY_DIST | TARGET_DISPLAY_LEAD);
		} else {
			hud_target_add_display_list(asteroid_objp, &asteroid_vertex, &asteroid_objp->pos, 0, NULL, NULL, TARGET_DISPLAY_DIST);
		}
	}
}

/**
 * Target the closest danger asteroid to the player
 */
void asteroid_target_closest_danger()
{
	object	*asteroid_objp, *closest_asteroid_objp = NULL;
	asteroid	*asp;
	float		dist, closest_dist = 999999.0f;

	for ( asteroid_objp = GET_FIRST(&obj_used_list); asteroid_objp !=END_OF_LIST(&obj_used_list); asteroid_objp = GET_NEXT(asteroid_objp) ) {
		if (asteroid_objp->type != OBJ_ASTEROID ) {
			continue;
		}

		asp = &Asteroids[asteroid_objp->instance];

		if ( asp->collide_objnum < 0 ) {
			continue;
		}

		dist = vm_vec_dist_quick(&Player_obj->pos, &asteroid_objp->pos);

		if ( dist < closest_dist ) {
			closest_dist = dist;
			closest_asteroid_objp = asteroid_objp;
		}
	}

	if ( closest_asteroid_objp ) {
		set_target_objnum( Player_ai, OBJ_INDEX(closest_asteroid_objp) );
	}
}

void asteroid_page_in()
{
	if (Asteroid_field.num_initial_asteroids > 0 ) {
		int i, j, k;

		nprintf(( "Paging", "Paging in asteroids\n" ));


		// max of MAX_ACTIVE_DEBRIS_TYPES possible debris field models
		for (i=0; i<MAX_ACTIVE_DEBRIS_TYPES; i++) {
			asteroid_info	*asip;

			if (Asteroid_field.debris_genre == DG_ASTEROID) {
				// asteroid
				Assert(i < NUM_DEBRIS_SIZES);
				asip = &Asteroid_info[i];
			} else {
				// ship debris - always full until empty
				if (Asteroid_field.field_debris_type[i] != -1) {
					asip = &Asteroid_info[Asteroid_field.field_debris_type[i]];
				} else {
					break;
				}
			}


			for (k=0; k<NUM_DEBRIS_POFS; k++) {

				// SHIP DEBRIS - use subtype 0
				if (Asteroid_field.debris_genre == DG_SHIP) {
					if (k > 0) {
						break;
					}
				} else {
					// ASTEROID DEBRIS - use subtype (Asteroid_field.field_debris_type[] != -1)
					if (Asteroid_field.field_debris_type[k] == -1) {
						continue;
					}
				}

				if (asip->model_num[k] < 0)
					continue;

				asip->modelp[k] = model_get(asip->model_num[k]);

				// Page in textures
				for (j=0; j<asip->modelp[k]->n_textures; j++ )	{
					asip->modelp[k]->maps[j].PageIn();			
				}

			}
		} 
	}
}
