/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#include "cmdline/cmdline.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "object/collideprofile.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "ship/ship.h"
#include "tracing/tracing.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"
#include "tracing/Monitor.h"
#include "utils/threading.h"

#include <condition_variable>
#include <limits>
#include <thread>


// the next 2 variables are used for pair statistics
// also in weapon.cpp there is Weapons_created.
int Num_pairs = 0;
int Num_pairs_checked = 0;

SCP_vector<int> Collision_sort_list;

static_assert(1 << collision_cache_bitshift > MAX_OBJECTS, "Collision pair caching currently relies on the highest possible objnum being less than 2^collision_cache_bitshift.");

constexpr uint collision_cache_key_mask = (1u << collision_cache_bitshift) - 1;

// The two objects of a cached pair are implied by the key -- it is built from their objnums --
// so the entry only has to remember which *incarnation* of those two slots it was built for.
struct collider_pair
{
	int signature_a;
	int signature_b;
	int next_check_time;
};

static int collision_pair_objnum_a(uint key) { return static_cast<int>(key >> collision_cache_bitshift); }
static int collision_pair_objnum_b(uint key) { return static_cast<int>(key & collision_cache_key_mask); }

/**
 * @brief Open-addressed hash table mapping a packed objnum pair to its cached collision state.
 *
 * This used to be an SCP_unordered_map, which is node based: every entry is its own heap
 * allocation, so a lookup costs a miss on the bucket array and then a second miss chasing the
 * node pointer.  A heavy mission keeps a quarter of a million pairs alive, which made this
 * lookup alone roughly 44% of all collision detection time.  Linear probing over a flat array
 * of 16-byte entries keeps the common case to a single cache line.
 *
 * Key 0 is used as the empty sentinel.  That is safe because key 0 means "objnum 0 against
 * objnum 0", and obj_collide_pair rejects A == B before it ever builds a key.
 */
class collider_pair_cache
{
	struct slot {
		uint key;
		collider_pair val;
	};

	SCP_vector<slot> _slots;
	size_t _mask = 0;
	size_t _count = 0;

	// Fibonacci hashing.  The low bits of the key are objnum b, which is far from uniform, so
	// mix the whole key and use the high bits of the product.
	static size_t hash_key(uint key)
	{
		return static_cast<size_t>((static_cast<std::uint64_t>(key) * 0x9E3779B97F4A7C15ull) >> 32);
	}

	void rehash(size_t new_capacity)
	{
		SCP_vector<slot> old;
		old.swap(_slots);

		_slots.assign(new_capacity, slot{0, collider_pair{}});
		_mask = new_capacity - 1;
		_count = 0;

		for (const auto& s : old) {
			if (s.key != 0) {
				insert_unchecked(s.key, s.val);
			}
		}
	}

	void insert_unchecked(uint key, const collider_pair& val)
	{
		size_t i = hash_key(key) & _mask;
		while (_slots[i].key != 0) {
			i = (i + 1) & _mask;
		}
		_slots[i].key = key;
		_slots[i].val = val;
		++_count;
	}

public:
	collider_pair_cache() { rehash(1024); }

	size_t size() const { return _count; }

	void clear()
	{
		_slots.assign(_slots.size(), slot{0, collider_pair{}});
		_count = 0;
	}

	/**
	 * @brief Find the entry for @a key, creating a default one if it is not present.
	 * @param[out] existed set to true if the entry was already in the table
	 *
	 * The returned pointer is invalidated by any subsequent insertion.
	 */
	collider_pair* get_or_create(uint key, bool& existed)
	{
		size_t i = hash_key(key) & _mask;
		while (_slots[i].key != 0) {
			if (_slots[i].key == key) {
				existed = true;
				return &_slots[i].val;
			}
			i = (i + 1) & _mask;
		}

		// Keep the load factor at or below 1/2; linear probing degrades badly past that.
		if ((_count + 1) * 2 > _slots.size()) {
			rehash(_slots.size() * 2);
			existed = false;
			// re-probe in the grown table
			size_t j = hash_key(key) & _mask;
			while (_slots[j].key != 0) {
				j = (j + 1) & _mask;
			}
			_slots[j].key = key;
			_slots[j].val = collider_pair{-1, -1, -1};
			++_count;
			return &_slots[j].val;
		}

		existed = false;
		_slots[i].key = key;
		_slots[i].val = collider_pair{-1, -1, -1};
		++_count;
		return &_slots[i].val;
	}

	/**
	 * @brief Visit every live entry, dropping the ones @a pred rejects.
	 *
	 * Implemented as a compacting rebuild rather than in-place erase: linear probing needs
	 * either tombstones or backward shifting to erase safely, and this is only ever called
	 * while already walking the whole table.
	 */
	template <typename Pred>
	void retain(Pred pred)
	{
		SCP_vector<slot> old;
		old.swap(_slots);

		_slots.assign(old.size(), slot{0, collider_pair{}});
		_mask = old.size() - 1;
		_count = 0;

		for (auto& s : old) {
			if (s.key != 0 && pred(s.key, s.val)) {
				insert_unchecked(s.key, s.val);
			}
		}
	}
};

static SCP_set<object*> Collision_cache_stale_objects;
static collider_pair_cache Collision_cached_pairs;

class checkobject;
extern checkobject CheckObjects[MAX_OBJECTS];

// returns true if we should reject object pair if one is child of other.
int reject_obj_pair_on_parent(object *A, object *B)
{
	if (A->flags[Object::Object_Flags::Collides_with_parent] || B->flags[Object::Object_Flags::Collides_with_parent])
		return 0;

	if (A->type == OBJ_SHIP) {
		if (B->type == OBJ_DEBRIS) {
			if (B->parent_sig == A->signature) {
				return 0;
			}
		}
	}

	if (B->type == OBJ_SHIP) {
		if (A->type == OBJ_DEBRIS) {
			if (A->parent_sig == B->signature) {
				return 0;
			}
		}
	}

	if (A->parent_sig == B->signature) {
		return 1;
	}

	if (B->parent_sig == A->signature) {
		return 1;
	}

	return 0;
}

bool reject_due_collision_groups(object *A, object *B)
{
	if (A->collision_group_id == 0 || B->collision_group_id == 0)
		return false;

	return (A->collision_group_id & B->collision_group_id);
}

MONITOR(NumPairs)
MONITOR(NumPairsChecked)

//	See if two lines intersect by doing recursive subdivision.
//	Bails out if larger distance traveled is less than sum of radii + 1.0f.
int collide_subdivide(vec3d *p0, vec3d *p1, float prad, vec3d *q0, vec3d *q1, float qrad)
{
    float a_dist = vm_vec_dist(p0, p1);
    float b_dist = vm_vec_dist(q0, q1);
    float ab_dist = vm_vec_dist(p1, q1);

	//	See if their spheres intersect
	if (ab_dist < a_dist + b_dist + prad + qrad) {
		if (ab_dist  < prad + qrad)
			return 1;
		else if (vm_vec_dist(p0, q0) < prad + qrad)
			return 1;
		else if (MAX(a_dist, b_dist) < prad + qrad + 1.0f)
			return 0;
		else {
			int	r1, r2 = 0;
			vec3d	pa, qa;

			vm_vec_avg(&pa, p0, p1);
			vm_vec_avg(&qa, q0, q1);
			r1 = collide_subdivide(p0, &pa, prad, q0, &qa, qrad);
			if (!r1)
				r2 = collide_subdivide(&pa, p1, prad, &qa, q1, qrad);

			return r1 | r2;
		}
	} else
		return 0;
}



//	Return true if object A is expected to collide with object B within time duration
//	For purposes of this check, the first object moves from current location to predicted
//	location.  The second object is assumed to be where it will be at time duration, NOT
//	where it currently is.
//	radius_scale is used to control the precision of the check.
//		If 0.0, then use polygon models to perform check, slow and accurate
//		If !0.0, then use as a scale on the radius of the objects.  1.0 is Descent style
//			collisions.  Larger values can be used to be sloppy about the collisions which
//			is useful if a moving object wants to prevent a collision.
int objects_will_collide(object *A, object *B, float duration, float radius_scale)
{
	vec3d hitpos;
	int ret;


    vec3d prev_pos = A->pos;
	vm_vec_scale_add2(&A->pos, &A->phys_info.vel, duration);

	if (radius_scale == 0.0f) {
		ret = ship_check_collision_fast(B, A, &hitpos);
	} else {
		vec3d	nearest_point;

		const float size_A = A->radius * radius_scale;
		const float size_B = B->radius * radius_scale;

		//	If A is moving, check along vector.
		if (A->phys_info.speed != 0.0f) {
			const float r = find_nearest_point_on_line(&nearest_point, &prev_pos, &A->pos, &B->pos);
			if (r < 0) {
				nearest_point = prev_pos;
			} else if (r > 1) {
				nearest_point = A->pos;
			}
			const float dist = vm_vec_dist_quick(&B->pos, &nearest_point);
			ret = (dist < size_A + size_B);
		} else {
			ret = vm_vec_dist_quick(&B->pos, &prev_pos) < size_A + size_B;
		}
	}

	// Reset the position to the previous value
	A->pos = prev_pos;

	return ret;
}

//	Return true if the vector from *start_pos to *end_pos is within objp->radius*radius_scale of *objp
int vector_object_collision(vec3d *start_pos, vec3d *end_pos, object *objp, float radius_scale)
{
	vec3d	nearest_point;

	float r = find_nearest_point_on_line(&nearest_point, start_pos, end_pos, &objp->pos);
	if ((r >= 0.0f) && (r <= 1.0f)) {
		float dist = vm_vec_dist_quick(&objp->pos, &nearest_point);

		return (dist < objp->radius * radius_scale);
	} else
		return 0;
}

// Returns TRUE if the weapon will never hit the other object.
// If it can it predicts how long until these two objects need
// to be checked and fills the time in in current_pair.
int weapon_will_never_hit( object *obj_weapon, object *other, obj_pair * current_pair )
{

	Assert( obj_weapon->type == OBJ_WEAPON );
	weapon *wp = &Weapons[obj_weapon->instance];
	weapon_info *wip = &Weapon_info[wp->weapon_info_index];

	// Do some checks for weapons that don't turn
	// gotta treat anything being affected by gravity as turning too
	if ( !(wip->wi_flags[Weapon::Info_Flags::Turns] && (IS_VEC_NULL(&The_mission.gravity) || obj_weapon->phys_info.gravity_const == 0.0))) {

		// This first check is to see if a weapon is behind an object, and they
		// are heading in opposite directions.   If so, we don't need to ever check	
		// them again.   This is only valid for weapons that don't turn. 

		float vdot;
		if (wip->subtype == WP_LASER) {
			vec3d velocity_rel_weapon;
			vm_vec_sub(&velocity_rel_weapon, &obj_weapon->phys_info.vel, &other->phys_info.vel);
			vdot = -vm_vec_dot(&velocity_rel_weapon, &obj_weapon->orient.vec.fvec);
		} else {
			vdot = vm_vec_dot( &other->phys_info.vel, &obj_weapon->phys_info.vel);
		}
		if ( vdot <= 0.0f )	{
			// They're heading in opposite directions...
			// check their positions
			vec3d weapon2other;
			vm_vec_sub( &weapon2other, &other->pos, &obj_weapon->pos );
			float pdot = vm_vec_dot( &obj_weapon->orient.vec.fvec, &weapon2other );
			if ( pdot <= -other->radius )	{
				// The other object is behind the weapon by more than
				// its radius, so it will never hit...
				return 1;
			}
		}

		// FUTURE ENHANCEMENT IDEAS 

		// Given a laser does it hit a slow or not moving object
		// in its life or the next n seconds?  We'd actually need to check the 
		// model for this.
	}


	// This check doesn't care about orient, only looks at the maximum speed
	// of the two objects, so it knows that in the next n seconds, they can't
	// go further than some distance, so don't bother checking collisions for 
	// that time.   This is very rough, but is so general that it works for
	// everything and immidiately gets rid of a lot of cases.
	
	if ( current_pair )	{
		// Find the time it will take before these get within each others distances.
		// tmp->next_check_time = timestamp(500);
		//vector	max_vel;			//maximum foward velocity in x,y,z

		float max_vel_weapon;

		//SUSHI: Fix bug where additive weapon velocity screws up collisions
		//If the PF_CONST_VEL flag is set, we can safely assume it doesn't change speed.
		if (obj_weapon->phys_info.flags & PF_CONST_VEL)
			max_vel_weapon = obj_weapon->phys_info.speed;
		else if (wp->lssm_stage == 5)
			max_vel_weapon = wip->lssm_stage5_vel;
		else if (IS_VEC_NULL(&The_mission.gravity) || obj_weapon->phys_info.gravity_const == 0.0f)
			max_vel_weapon = wp->weapon_max_vel;
		else
			max_vel_weapon = obj_weapon->phys_info.speed + wp->lifeleft * vm_vec_mag(&The_mission.gravity);

		float max_vel_other = other->phys_info.max_vel.xyz.z;
		if (max_vel_other < 10.0f) {
			if ( vm_vec_mag_squared( &other->phys_info.vel ) > 100 ) {
				// bump up velocity from collision
				max_vel_other = vm_vec_mag( &other->phys_info.vel ) + 10.0f;
			} else {
				max_vel_other = 10.0f;		// object may move from collision
			}
		}

		// check weapon that does not turn against sphere expanding at ship maxvel
		// compare (weapon) ray with expanding sphere (ship) to find earliest possible collision time
		// look for two time solutions to Xw = Xs, where Xw = Xw0 + Vwt*t  Xs = Xs + Vs*(t+dt), where Vs*dt = radius of ship 
		// Since direction of Vs is unknown, solve for (Vs*t) and find norm of both sides
		if ( !(wip->wi_flags[Weapon::Info_Flags::Turns]) && (obj_weapon->phys_info.flags & PF_CONST_VEL) ) {
			vec3d delta_x;
			float a,b,c, delta_x_dot_vl, delta_t;
			float root1, root2, root, earliest_time;

			vm_vec_sub( &delta_x, &obj_weapon->pos, &other->pos );

			if (max_vel_weapon == max_vel_other) {
				// this will give us NAN using the below formula, so check every frame
				current_pair->next_check_time = timestamp(0);
				return 0;
			}

			// vm_vec_copy_scale( &laser_vel, &weapon->orient.vec.fvec, max_vel_weapon );
			delta_t = (other->radius + 10.0f) / max_vel_other;		// time to get from center to radius of other obj
			delta_x_dot_vl = vm_vec_dot( &delta_x, &obj_weapon->phys_info.vel);

			a = max_vel_weapon * max_vel_weapon - max_vel_other*max_vel_other;
			b = 2.0f * (delta_x_dot_vl - max_vel_other*max_vel_other*delta_t);
			c = vm_vec_mag_squared( &delta_x ) - max_vel_other*max_vel_other*delta_t*delta_t;

			float discriminant = b*b - 4.0f*a*c;
			if ( discriminant < 0) {
				// neither entity passes the other
				if (c < 0) { 
					// ship outpaces weapon
					current_pair->next_check_time = timestamp(0);	// check next time
					return 0;
				} else {
					// weapon outpaces ship; will never hit
					return 1;
				}
			} else {
				root = fl_sqrt( discriminant );
				root1 = (-b + root) / (2.0f * a) * 1000.0f;	// get time in ms
				root2 = (-b - root) / (2.0f * a) * 1000.0f;	// get time in ms
			}

			// standard algorithm
			if (max_vel_weapon > max_vel_other) {
				// find earliest positive time
				if ( root1 > root2 ) {
					float temp = root1;
					root1 = root2;
					root2 = temp;
				}

				if (root1 > 0) {
					earliest_time = root1;
				} else if (root2 > 0) {
					// root1 < 0 and root2 > 0, so we're inside sphere and next check should be next frame
					current_pair->next_check_time = timestamp(0);	// check next time
					return 0;
				} else {
					// both times negative, so never collides
					return 1;
				}
			}
			// need to modify it for weapons that are slower than ships
			else {
				if (root2 > 0) {
					earliest_time = root2;
				} else {
					current_pair->next_check_time = timestamp(0);
					return 0;
				}
			}

			// check if possible collision occurs after weapon expires
			if ( earliest_time > 1000*wp->lifeleft )
				return 1;

			// Allow one worst case frametime to elapse (~5 fps)
			earliest_time -= 200.0f;

			if (earliest_time > 100) {
				current_pair->next_check_time = timestamp( fl2i(earliest_time) );
				return 0;
			} else {
				current_pair->next_check_time = timestamp(0);	// check next time
				return 0;
			}

		} else {
			const float max_vel = max_vel_weapon + max_vel_other;

			// suggest that fudge factor for other radius be changed to other_radius + const (~10)
			const float dist = vm_vec_dist( &other->pos, &obj_weapon->pos ) - (other->radius + 10.0f);
			if ( dist > 0.0f )	{
				const float time = (dist*1000.0f) / max_vel;
				int time_ms = fl2i(time);

				// check if possible collision occurs after weapon expires
				if ( time_ms > 1000*wp->lifeleft )
					return 1;

				time_ms -= 200;	// Allow at least one worst case frametime to elapse (~5 fps)
						
				if ( time_ms > 100 )	{		// If it takes longer than 1/10th of a second, then delay it
					current_pair->next_check_time = timestamp(time_ms);
					//mprintf(( "Delaying %d ms\n", time_ms ));
					return 0;
				}
			}
			current_pair->next_check_time = timestamp(0);	// check next time

		}
	}

	return 0;
}

//	Return true if vector from *curpos to *goalpos intersects with object *goalobjp
//	Else, return false.
//	radius is radius of object moving from curpos to goalpos.
int pp_collide(vec3d *curpos, vec3d *goalpos, object *goalobjp, float radius)
{
	Assert(goalobjp->type == OBJ_SHIP);

	mc_info mc;
	mc.model_instance_num = Ships[goalobjp->instance].model_instance_num;
	mc.model_num = Ship_info[Ships[goalobjp->instance].ship_info_index].model_num;			// Fill in the model to check
	mc.orient = &goalobjp->orient;	// The object's orient
	mc.pos = &goalobjp->pos;			// The object's position
	mc.p0 = curpos;					// Point 1 of ray to check
	mc.p1 = goalpos;					// Point 2 of ray to check
	mc.flags = MC_CHECK_MODEL | MC_CHECK_SPHERELINE;
	mc.radius = radius;

	model_collide(&mc);

	return mc.num_hits;
}

//	Setup and call pp_collide for collide_predict_large_ship
//	Returns true if objp will collide with objp2 before it reaches goal_pos.
int cpls_aux(vec3d *goal_pos, object *objp2, object *objp)
{
	float radius = objp->radius;
	if (1.5f * radius < 70.0f)
		radius *= 1.5f;
	else
		radius = 70.0f;

	if (pp_collide(&objp->pos, goal_pos, objp2, radius))
		return 1;
	else
		return 0;
}

//	Return true if objp will collide with some large object.
//	Don't check for an object this ship is docked to.
int collide_predict_large_ship(object *objp, float distance)
{
	object	*objp2;
	vec3d	goal_pos;
	ship_info* sip = &Ship_info[Ships[objp->instance].ship_info_index];

	vec3d cur_pos = objp->pos;

	vm_vec_scale_add(&goal_pos, &cur_pos, &objp->orient.vec.fvec, distance);

	for ( objp2 = GET_FIRST(&obj_used_list); objp2 != END_OF_LIST(&obj_used_list); objp2 = GET_NEXT(objp2) ) {
		if (objp2->flags[Object::Object_Flags::Should_be_dead])
			continue;

		if ((objp != objp2) && (objp2->type == OBJ_SHIP)) {
			if (Ship_info[Ships[objp2->instance].ship_info_index].is_big_or_huge()) {
				if (dock_check_find_docked_object(objp, objp2))
					continue;

				if (cpls_aux(&goal_pos, objp2, objp))
					return 1;
			}
		} else if (!(sip->is_big_or_huge()) && (objp2->type == OBJ_ASTEROID)) {
			if (vm_vec_dist_quick(&objp2->pos, &objp->pos) < (distance + objp2->radius)*2.5f) {
                vec3d delvec;

				const float d1 = 2.5f * distance + objp2->radius;
				auto count = (int) (d1/(objp2->radius + objp->radius));	//	Scale up distance, else looks like there would be a collision.
				vec3d pos = cur_pos;
				vm_vec_normalized_dir(&delvec, &goal_pos, &cur_pos);
				vm_vec_scale(&delvec, d1/count);

				for (; count>0; count--) {
					if (vm_vec_dist_quick(&pos, &objp2->pos) < objp->radius + objp2->radius)
						return 1;
					vm_vec_add2(&pos, &delvec);
				}
			}
		}
	}

	return 0;
}

// function to iterate through all object collision pairs looking for weapons
// which could be deleted since they are not going to hit anything.  Passed into this
// function is a 'time' parameter used as watermark for which weapons to check.

#define CRW_NO_OBJECT		-1
#define CRW_NO_PAIR			0
#define CRW_IN_PAIR			1
#define CRW_CAN_DELETE		2

#define CRW_MAX_TO_DELETE	4

char crw_status[MAX_WEAPONS];

void crw_check_weapon( int weapon_num, int collide_next_check )
{
	weapon *wp = &Weapons[weapon_num];

	// if this weapons life left > time before next collision, then we cannot remove it
	crw_status[WEAPON_INDEX(wp)] = CRW_IN_PAIR;
	const float next_check_time = ((float)(timestamp_until(collide_next_check)) / 1000.0f);
	if ( wp->lifeleft < next_check_time )
		crw_status[WEAPON_INDEX(wp)] = CRW_CAN_DELETE;
}

int collide_remove_weapons( )
{
	// setup remove_weapon array.  assume we can remove it.
	for (int i = 0; i < MAX_WEAPONS; i++ ) {
		if ( Weapons[i].objnum == -1 )
			crw_status[i] = CRW_NO_OBJECT;
		else
			crw_status[i] = CRW_NO_PAIR;
	}

	// first pass is to see if any of the weapons don't have collision pairs.
	// Dropping the entry here replaces the old "initialized = false" reset: either way the next
	// lookup of this key starts from scratch.
	Collision_cached_pairs.retain([](uint key, collider_pair& pair) {
		object* a = &Objects[collision_pair_objnum_a(key)];
		object* b = &Objects[collision_pair_objnum_b(key)];
		bool keep = true;

		if (a->type == OBJ_WEAPON && pair.signature_a == a->signature) {
			crw_check_weapon(a->instance, pair.next_check_time);

			if (crw_status[a->instance] == CRW_CAN_DELETE) {
				keep = false;
			}
		}

		if (b->type == OBJ_WEAPON && pair.signature_b == b->signature) {
			crw_check_weapon(b->instance, pair.next_check_time);

			if (crw_status[b->instance] == CRW_CAN_DELETE) {
				keep = false;
			}
		}

		return keep;
	});

	// for each weapon which could be removed, delete the object
	int num_deleted = 0;
	for (int i = 0; i < MAX_WEAPONS; i++ ) {
		if ( crw_status[i] == CRW_CAN_DELETE ) {
			Assert( Weapons[i].objnum != -1 );
			obj_delete( Weapons[i].objnum );
			num_deleted++;
		}
	}

	// stop here because any other weapon could currently be involved in a collision and can cause crashes
	return num_deleted;

}

void set_hit_struct_info(collision_info_struct *hit, mc_info *mc, bool submodel_move_hit)
{
	hit->edge_hit = mc->edge_hit;
	hit->hit_pos = mc->hit_point_world;
	hit->hit_time = mc->hit_dist;
	hit->heavy_model_num = mc->model_num;
	hit->heavy_submodel_num = mc->hit_submodel;

	hit->submodel_move_hit = submodel_move_hit;
}

//Previously, this was done with 
//memset(&ship_ship_hit_info, -1, sizeof(collision_info_struct));
//All those -1s are to replicate that logic
void init_collision_info_struct(collision_info_struct *cis)
{
	memset(cis, -1, sizeof(collision_info_struct));
	cis->is_landing = false;
}

void obj_add_collider(int obj_index)
{
	object *objp = &Objects[obj_index];

#ifdef OBJECT_CHECK 
	CheckObjects[obj_index].type = objp->type;
	CheckObjects[obj_index].signature = objp->signature;
    CheckObjects[obj_index].flags = objp->flags - Object::Object_Flags::Not_in_coll;
	CheckObjects[obj_index].parent_sig = objp->parent_sig;
#endif

	if(!(objp->flags[Object::Object_Flags::Not_in_coll])){
		return;
	}

	Collision_sort_list.push_back(obj_index);

	objp->flags.remove(Object::Object_Flags::Not_in_coll);
}

void obj_remove_collider(int obj_index)
{
#ifdef OBJECT_CHECK 
    CheckObjects[obj_index].flags.set(Object::Object_Flags::Not_in_coll);
#endif	

	for (size_t i = 0; i < Collision_sort_list.size(); ++i ) {
		if ( Collision_sort_list[i] == obj_index ) {
			Collision_sort_list[i] = Collision_sort_list.back();
			Collision_sort_list.pop_back();
			break;
		}
	}

	Objects[obj_index].flags.set(Object::Object_Flags::Not_in_coll);
}

void obj_reset_colliders()
{
	Collision_sort_list.clear();
	Collision_cached_pairs.clear();
}

void obj_collide_retime_stale_pairs()
{
	TRACE_SCOPE(tracing::RetimeCollisionCache);

	Collision_cached_pairs.retain([](uint key, collider_pair& pair) {
		object* a = &Objects[collision_pair_objnum_a(key)];
		object* b = &Objects[collision_pair_objnum_b(key)];

		// either slot has been recycled since this entry was made, so it is dead weight
		if (pair.signature_a != a->signature || pair.signature_b != b->signature)
			return false;

		if (a->flags[Object::Object_Flags::Collision_cache_stale] || b->flags[Object::Object_Flags::Collision_cache_stale])
			pair.next_check_time = timestamp(0);

		return true;
	});

	for (auto objp : Collision_cache_stale_objects)
		objp->flags.remove(Object::Object_Flags::Collision_cache_stale);
	Collision_cache_stale_objects.clear();
}

void obj_collide_obj_cache_stale(object* objp)
{
	objp->flags.set(Object::Object_Flags::Collision_cache_stale);
	Collision_cache_stale_objects.insert(objp);
}

//local helper functions only used in objcollide.cpp
namespace
{

// Endpoints for the axis currently being swept, indexed by objnum.  The sort and overlap passes
// between them ask for an endpoint hundreds of thousands of times a frame, and recomputing it
// every time meant an Objects[] indirection plus a three-way type dispatch each time.  Computing
// both ends once per object per axis turns all of that into an array read.
static SCP_vector<float> Collider_endpoint_min;
static SCP_vector<float> Collider_endpoint_max;

void obj_compute_collider_endpoints(int obj_num, int axis, float *min_out, float *max_out)
{
    const object *objp = &Objects[obj_num];

    if ( objp->type == OBJ_BEAM ) {
        const beam *b = &Beams[objp->instance];

        // use the last start and last shot as endpoints
        if ( b->last_start.a1d[axis] > b->last_shot.a1d[axis] ) {
            *min_out = b->last_shot.a1d[axis];
            *max_out = b->last_start.a1d[axis];
        } else {
            *min_out = b->last_start.a1d[axis];
            *max_out = b->last_shot.a1d[axis];
        }
    } else if ( objp->type == OBJ_WEAPON ) {
        if ( objp->pos.a1d[axis] > objp->last_pos.a1d[axis] ) {
            *min_out = objp->last_pos.a1d[axis] - objp->radius;
            *max_out = objp->pos.a1d[axis] + objp->radius;
        } else {
            *min_out = objp->pos.a1d[axis] - objp->radius;
            *max_out = objp->last_pos.a1d[axis] + objp->radius;
        }
    } else {
        *min_out = objp->pos.a1d[axis] - objp->radius;
        *max_out = objp->pos.a1d[axis] + objp->radius;
    }
}

//! Refresh the endpoint cache for @a axis, for exactly the objects in @a list.
void obj_cache_collider_endpoints(const SCP_vector<int> &list, int axis)
{
    if ( Collider_endpoint_min.size() < static_cast<size_t>(MAX_OBJECTS) ) {
        Collider_endpoint_min.resize(MAX_OBJECTS);
        Collider_endpoint_max.resize(MAX_OBJECTS);
    }

    for (int obj_num : list) {
        obj_compute_collider_endpoints(obj_num, axis, &Collider_endpoint_min[obj_num], &Collider_endpoint_max[obj_num]);
    }
}

inline float obj_get_collider_endpoint(int obj_num, bool min)
{
    return min ? Collider_endpoint_min[obj_num] : Collider_endpoint_max[obj_num];
}

void obj_quicksort_colliders(SCP_vector<int> *list, int left, int right, int axis)
{
    Assert( axis >= 0 );
    Assert( axis <= 2 );

    if ( right > left ) {
        int pivot_index = left + (right - left) / 2;

        float pivot_value = obj_get_collider_endpoint((*list)[pivot_index], true);

        // swap!
        int temp = (*list)[pivot_index];
        (*list)[pivot_index] = (*list)[right];
        (*list)[right] = temp;

        int store_index = left;

        for (int i = left; i < right; ++i ) {
            if ( obj_get_collider_endpoint((*list)[i], true) <= pivot_value ) {
                temp = (*list)[i];
                (*list)[i] = (*list)[store_index];
                (*list)[store_index] = temp;
                store_index++;
            }
        }

        temp = (*list)[right];
        (*list)[right] = (*list)[store_index];
        (*list)[store_index] = temp;

        obj_quicksort_colliders(list, left, store_index - 1, axis);
        obj_quicksort_colliders(list, store_index + 1, right, axis);
    }
}

struct collision_thread_data {
	struct collision_queue_item {
		obj_pair objs;
		uint ctype;
	};
	struct collision_queue_result {
		obj_pair objs;
		bool never_recheck;
		std::any collision_data;
		void (*process_collision)( obj_pair *pair,  const std::any& collision_data );
	};

	std::atomic_size_t queue_length, result_length;
	std::mutex queue_mutex, result_mutex;
	std::condition_variable work_available;
	std::unique_ptr<SCP_vector<collision_queue_item>> queue_load, queue_process;
	std::unique_ptr<SCP_vector<collision_queue_result>> queue_results, queue_send;

	collision_thread_data() :
		queue_length(0),
		result_length(0),
		queue_load(std::make_unique<SCP_vector<collision_queue_item>>()),
		queue_process(std::make_unique<SCP_vector<collision_queue_item>>()),
		queue_results(std::make_unique<SCP_vector<collision_queue_result>>()),
		queue_send(std::make_unique<SCP_vector<collision_queue_result>>()) {}
};

std::unique_ptr<collision_thread_data[]> collision_thread_data_buffer;
std::atomic_bool collision_processing_done = false;

// Results gathered from the workers during the drain, applied once they have all stopped.
// Kept at file scope so the allocation is reused frame to frame.
SCP_vector<collision_thread_data::collision_queue_result> collision_pending_results;

void spin_up_mp_collision() {
	collision_processing_done.store(false);
	threading::spin_up_threaded_task(threading::WorkerThreadTask::COLLISION);
}

void spin_down_mp_collision() {
	threading::spin_down_threaded_task();
	collision_processing_done.store(true, std::memory_order_release);

	// Wake every worker parked in its idle wait.  Taking the queue mutex before notifying is
	// what makes this race free: a worker only ever parks while holding that mutex, so we
	// cannot slip the notify into the gap between its "am I done?" check and the wait itself.
	for (size_t i = 0; i < threading::get_num_workers(); i++) {
		auto& thread = collision_thread_data_buffer[i];
		{
			std::scoped_lock lock(thread.queue_mutex);
		}
		thread.work_available.notify_all();
	}

	threading::spin_down_wait_complete();
}

// Pairs are staged on the main thread and handed to the workers in chunks.  Locking a worker
// queue for every individual pair cost ~0.8us per pair -- about as much as the narrowphase work
// being offloaded -- because the producer contends with seven workers that are constantly
// parking and waking on that same mutex.
SCP_vector<collision_thread_data::collision_queue_item> collision_stage;
constexpr size_t collision_stage_flush_size = 256;

void flush_mp_collisions() {
	if (collision_stage.empty())
		return;

	const size_t workers = threading::get_num_workers();
	size_t offset = 0;

	for (size_t i = 0; i < workers; i++) {
		// deal the staged pairs out as evenly as the remainder allows
		const size_t remaining = collision_stage.size() - offset;
		const size_t take = remaining / (workers - i);
		if (take == 0)
			continue;

		auto& thread = collision_thread_data_buffer[i];
		{
			std::scoped_lock lock(thread.queue_mutex);
			thread.queue_load->insert(thread.queue_load->end(),
				std::make_move_iterator(collision_stage.begin() + offset),
				std::make_move_iterator(collision_stage.begin() + offset + take));
			thread.queue_length.fetch_add(take, std::memory_order_release);
		}
		thread.work_available.notify_one();

		offset += take;
	}

	collision_stage.clear();
}

void queue_mp_collision(uint ctype, const obj_pair& colliding) {
	collision_stage.emplace_back(collision_thread_data::collision_queue_item{colliding, ctype});

	if (collision_stage.size() >= collision_stage_flush_size)
		flush_mp_collisions();
}

void post_process_threaded_collisions() {
	collision_pending_results.clear();

	SCP_map<size_t, size_t> workerThreads;
	for (size_t i = 0; i < threading::get_num_workers(); i++)
		workerThreads.emplace(i, 0);

	while (!workerThreads.empty()) {
		COLLISION_PROF_INC(drain_spins);

		bool progress = false;

		for(auto& [i, processed] : workerThreads) {
			auto& thread = collision_thread_data_buffer[i];

			size_t queue_length = thread.queue_length.load(std::memory_order_acquire);
			size_t result_length = thread.result_length.load(std::memory_order_acquire);

			if (result_length > processed) {
				progress = true;
				{
					std::scoped_lock lock(thread.result_mutex);
					thread.queue_results.swap(thread.queue_send);
				}
				processed += thread.queue_send->size();
				// Only collect here.  Applying a collision mutates ship physics and hull state, and
				// the other workers are still running checks that read those same objects, so the
				// actual processing waits until after spin down.
				for (auto& collision : *thread.queue_send) {
					collision_pending_results.emplace_back(std::move(collision));
				}
				thread.queue_send->clear();
			}
			else if (queue_length == 0) {
				thread.queue_results->clear();
				workerThreads.erase(i);
				progress = true;
				break;
			}
		}

		if (!progress) {
			// Every worker still has outstanding work but none of it has landed yet.  Yield
			// rather than sleeping on a condition variable: the main thread has nothing else to
			// do, results land in tens of microseconds, and a timed wait rounds every one of
			// those up to its own granularity (a 50us wait_for here cost ~2.3ms/frame).  This
			// loop was only ~700 iterations a frame even before the worker spin was fixed, so
			// it was never the thing burning cores.
			std::this_thread::yield();
		}
	}

	spin_down_mp_collision();

	// Every worker has now stopped, so it is safe to mutate the objects the checks were reading.
	for (auto& collision : collision_pending_results) {
		if (collision.collision_data.has_value())
			collision.process_collision(&collision.objs, collision.collision_data);

		// Look the entry up only after process_collision has run -- it executes game logic that
		// can compact the pair cache out from under us.
		uint key = (OBJ_INDEX(collision.objs.a) << collision_cache_bitshift) + OBJ_INDEX(collision.objs.b);
		bool existed = false;
		collider_pair *collision_info = Collision_cached_pairs.get_or_create(key, existed);
		if (!existed) {
			collision_info->signature_a = collision.objs.a->signature;
			collision_info->signature_b = collision.objs.b->signature;
		}

		if (collision.never_recheck) {
			collision_info->next_check_time = -1;
		} else {
			collision_info->next_check_time = collision.objs.next_check_time;
		}
	}
	collision_pending_results.clear();
}

void obj_collide_pair(object *A, object *B)
{
    TRACE_SCOPE(tracing::CollidePair);

    int (*check_collision)( obj_pair *pair ) = nullptr;
    int swapped = 0;
	bool support_mp = false;

    COLLISION_PROF_INC(pair_calls);

    if ( A==B ) return;		// Don't check collisions with yourself

    if ( !(A->flags[Object::Object_Flags::Collides]) ) return;		// This object doesn't collide with anything
    if ( !(B->flags[Object::Object_Flags::Collides]) ) return;		// This object doesn't collide with anything

	// Two immobile objects will never collide with each other
    if ( (A->flags[Object::Object_Flags::Immobile] || (A->flags[Object::Object_Flags::Dont_change_position] && A->flags[Object::Object_Flags::Dont_change_orientation]))
		&& (B->flags[Object::Object_Flags::Immobile] || (B->flags[Object::Object_Flags::Dont_change_position] && B->flags[Object::Object_Flags::Dont_change_orientation])) )
			return;

    // Make sure you're not checking a parent with it's kid or vicy-versy
    if ( reject_obj_pair_on_parent(A,B) ) {
        return;
    }

    Assert( A->type < 127 );
    Assert( B->type < 127 );

    uint ctype = COLLISION_OF(A->type,B->type);
    switch( ctype )	{
        case COLLISION_OF(OBJ_WEAPON,OBJ_SHIP):
            swapped = 1;
            check_collision = collide_ship_weapon;
			support_mp = true;
            break;
        case COLLISION_OF(OBJ_SHIP, OBJ_WEAPON):
            check_collision = collide_ship_weapon;
			support_mp = true;
            break;
        case COLLISION_OF(OBJ_DEBRIS, OBJ_WEAPON):
            check_collision = collide_debris_weapon;
			support_mp = true;
            break;
        case COLLISION_OF(OBJ_WEAPON, OBJ_DEBRIS):
            swapped = 1;
            check_collision = collide_debris_weapon;
			support_mp = true;
            break;
        case COLLISION_OF(OBJ_DEBRIS, OBJ_SHIP):
            check_collision = collide_debris_ship;
			support_mp = true;
            break;
        case COLLISION_OF(OBJ_SHIP, OBJ_DEBRIS):
            check_collision = collide_debris_ship;
            swapped = 1;
			support_mp = true;
            break;
		case COLLISION_OF(OBJ_DEBRIS, OBJ_PROP):
			check_collision = collide_debris_prop;
			break;
		case COLLISION_OF(OBJ_PROP, OBJ_DEBRIS):
			check_collision = collide_debris_prop;
			swapped = 1;
			break;
        case COLLISION_OF(OBJ_ASTEROID, OBJ_WEAPON):
            check_collision = collide_asteroid_weapon;
			support_mp = true;
            break;
        case COLLISION_OF(OBJ_WEAPON, OBJ_ASTEROID):
            swapped = 1;
            check_collision = collide_asteroid_weapon;
			support_mp = true;
            break;
        case COLLISION_OF(OBJ_ASTEROID, OBJ_SHIP):
            check_collision = collide_asteroid_ship;
			support_mp = true;
            break;
        case COLLISION_OF(OBJ_SHIP, OBJ_ASTEROID):
            check_collision = collide_asteroid_ship;
            swapped = 1;
			support_mp = true;
            break;
		case COLLISION_OF(OBJ_ASTEROID, OBJ_PROP):
			check_collision = collide_asteroid_prop;
			break;
		case COLLISION_OF(OBJ_PROP, OBJ_ASTEROID):
			check_collision = collide_asteroid_prop;
			swapped = 1;
			break;
        case COLLISION_OF(OBJ_SHIP,OBJ_SHIP):
            check_collision = collide_ship_ship;
#ifdef NDEBUG
			//This is, due to debug prints, unfortunately only safe in release builds...
			support_mp = true;
#endif
            break;
		case COLLISION_OF(OBJ_PROP, OBJ_SHIP):
			check_collision = collide_prop_ship;
			break;
		case COLLISION_OF(OBJ_SHIP, OBJ_PROP):
			check_collision = collide_prop_ship;
			swapped = 1;
			break;
		case COLLISION_OF(OBJ_PROP, OBJ_WEAPON):
			check_collision = collide_prop_weapon;
			break;
		case COLLISION_OF(OBJ_WEAPON, OBJ_PROP):
			check_collision = collide_prop_weapon;
			swapped = 1;
			break;
        case COLLISION_OF(OBJ_SHIP, OBJ_BEAM):
            if(beam_collide_early_out(B, A)){
                return;
            }
            swapped = 1;
            check_collision = beam_collide_ship;
            break;

        case COLLISION_OF(OBJ_BEAM, OBJ_SHIP):
            if(beam_collide_early_out(A, B)){
                return;
            }
            check_collision = beam_collide_ship;
            break;

        case COLLISION_OF(OBJ_ASTEROID, OBJ_BEAM):
            if(beam_collide_early_out(B, A)) {
                return;
            }
            swapped = 1;
            check_collision = beam_collide_asteroid;
            break;

        case COLLISION_OF(OBJ_BEAM, OBJ_ASTEROID):
            if(beam_collide_early_out(A, B)){
                return;
            }
            check_collision = beam_collide_asteroid;
            break;
        case COLLISION_OF(OBJ_DEBRIS, OBJ_BEAM):
            if(beam_collide_early_out(B, A)) {
                return;
            }
            swapped = 1;
            check_collision = beam_collide_debris;
            break;
        case COLLISION_OF(OBJ_BEAM, OBJ_DEBRIS):
            if(beam_collide_early_out(A, B)){
                return;
            }
            check_collision = beam_collide_debris;
            break;
        case COLLISION_OF(OBJ_WEAPON, OBJ_BEAM):
            if(beam_collide_early_out(B, A)) {
                return;
            }
            swapped = 1;
            check_collision = beam_collide_missile;
            break;

        case COLLISION_OF(OBJ_BEAM, OBJ_WEAPON):
            if(beam_collide_early_out(A, B)){
                return;
            }
            check_collision = beam_collide_missile;
            break;
		case COLLISION_OF(OBJ_PROP, OBJ_BEAM):
			if (beam_collide_early_out(B, A)) {
				return;
			}
			swapped = 1;
			check_collision = beam_collide_prop;
			break;

		case COLLISION_OF(OBJ_BEAM, OBJ_PROP):
			if (beam_collide_early_out(A, B)) {
				return;
			}
			check_collision = beam_collide_prop;
			break;

        case COLLISION_OF(OBJ_WEAPON, OBJ_WEAPON): {
            weapon_info* awip = &Weapon_info[Weapons[A->instance].weapon_info_index];
            weapon_info* bwip = &Weapon_info[Weapons[B->instance].weapon_info_index];

            if ((awip->weapon_hitpoints > 0) || (bwip->weapon_hitpoints > 0)) {
                if (bwip->weapon_hitpoints == 0) {
                    check_collision = collide_weapon_weapon;
                    swapped=1;
                } else {
                    check_collision = collide_weapon_weapon;
                }
                support_mp = true;
            }

            break;
        }

        default:
            return;
    }

    if ( !check_collision ) return;

    COLLISION_PROF_INC(pairs_considered);

    // Swap them if needed
    if ( swapped ) {
        std::swap(A,B);
    }

    bool valid = false;
    uint key = (OBJ_INDEX(A) << collision_cache_bitshift) + OBJ_INDEX(B);

    // NOTE: timing this single lookup costs ~1.5ms/frame in timer calls alone, which is more
    // than it measures.  It was instrumented once to establish that the lookup was ~2.3ms of a
    // 5.3ms collision budget; don't leave a timer here.
    bool existed = false;
    collider_pair* collision_info = Collision_cached_pairs.get_or_create(key, existed);

    // The key is built from both objnums, so the entry always describes these two slots. All we
    // have to confirm is that neither slot has been recycled since the entry was made.
    if ( existed && collision_info->signature_a == A->signature && collision_info->signature_b == B->signature ) {
        valid = true;
    } else {
        collision_info->signature_a = A->signature;
        collision_info->signature_b = B->signature;
        collision_info->next_check_time = timestamp(0);
    }

    if ( valid ) {
        // if this signature is valid, make the necessary checks to see if we need to collide check
        if ( collision_info->next_check_time == -1 ) {
            COLLISION_PROF_INC(pairs_cache_skipped);
            return;
        } else if ( !timestamp_elapsed(collision_info->next_check_time) ) {
            COLLISION_PROF_INC(pairs_cache_skipped);
			return;
        }
    } else {
        // only check debris:weapon collisions for player
        if (check_collision == collide_debris_weapon) {
            // weapon is B
            if ( !(Weapon_info[Weapons[B->instance].weapon_info_index].wi_flags[Weapon::Info_Flags::Turns]) ) {
                // check for dumbfire weapon
                // check if debris is behind laser
                float vdot;
                if (Weapon_info[Weapons[B->instance].weapon_info_index].subtype == WP_LASER) {
                    vec3d velocity_rel_weapon;
                    vm_vec_sub(&velocity_rel_weapon, &B->phys_info.vel, &A->phys_info.vel);
                    vdot = -vm_vec_dot(&velocity_rel_weapon, &B->orient.vec.fvec);
                } else {
                    vdot = vm_vec_dot( &A->phys_info.vel, &B->phys_info.vel);
                }
                if ( vdot <= 0.0f )	{
                    // They're heading in opposite directions...
                    // check their positions
                    vec3d weapon2other;
                    vm_vec_sub( &weapon2other, &A->pos, &B->pos );
                    float pdot = vm_vec_dot( &B->orient.vec.fvec, &weapon2other );
                    if ( pdot <= -A->radius )	{
                        // The other object is behind the weapon by more than
                        // its radius, so it will never hit...
                        collision_info->next_check_time = -1;
                        return;
                    }
                }

                // check dist vs. dist moved during weapon lifetime
                vec3d delta_v;
                vm_vec_sub(&delta_v, &B->phys_info.vel, &A->phys_info.vel);
                if (vm_vec_dist_squared(&A->pos, &B->pos) > (vm_vec_mag_squared(&delta_v)*Weapons[B->instance].lifeleft*Weapons[B->instance].lifeleft)) {
                    collision_info->next_check_time = -1;
                    return;
                }

                // for nonplayer ships, only create collision pair if close enough
                if ( (B->parent >= 0) && ((Objects[B->parent].signature != B->parent_sig) || !(Objects[B->parent].flags[Object::Object_Flags::Player_ship])) && (vm_vec_dist_squared(&B->pos, &A->pos) < (4.0f*A->radius + 200.0f) * (4.0f*A->radius + 200.0f)) ) {
                    collision_info->next_check_time = -1;
                    return;
                }
            }
        }

        // don't check same team laser:ship collisions on small ships if not player
        if (check_collision == collide_ship_weapon) {
            // weapon is B
            if ( (B->parent >= 0)
                 && (Objects[B->parent].signature == B->parent_sig)
                 && !(Objects[B->parent].flags[Object::Object_Flags::Player_ship])
                 && (Ships[Objects[B->parent].instance].team == Ships[A->instance].team)
                 && (Ship_info[Ships[A->instance].ship_info_index].is_small_ship())
                 && (Weapon_info[Weapons[B->instance].weapon_info_index].subtype == WP_LASER) ) {
                collision_info->next_check_time = -1;
                return;
            }
        }
    }

    obj_pair new_pair;

    new_pair.a = A;
    new_pair.b = B;
    new_pair.next_check_time = collision_info->next_check_time;

	if (threading::is_threading() && support_mp) {
		COLLISION_PROF_INC(pairs_enqueued);
		queue_mp_collision(ctype, new_pair);
	}
	else {
		COLLISION_PROF_INC(pairs_checked_inline);
#if COLLISION_PROFILING
		switch (A->type == OBJ_BEAM || B->type == OBJ_BEAM ? OBJ_BEAM : ctype) {
			case OBJ_BEAM:
				COLLISION_PROF_INC(inline_beam); break;
			case COLLISION_OF(OBJ_WEAPON, OBJ_WEAPON):
				COLLISION_PROF_INC(inline_weapon_weapon); break;
			case COLLISION_OF(OBJ_DEBRIS, OBJ_SHIP):
			case COLLISION_OF(OBJ_SHIP, OBJ_DEBRIS):
				COLLISION_PROF_INC(inline_debris_ship); break;
			case COLLISION_OF(OBJ_ASTEROID, OBJ_SHIP):
			case COLLISION_OF(OBJ_SHIP, OBJ_ASTEROID):
				COLLISION_PROF_INC(inline_asteroid_ship); break;
			case COLLISION_OF(OBJ_PROP, OBJ_SHIP):
			case COLLISION_OF(OBJ_SHIP, OBJ_PROP):
			case COLLISION_OF(OBJ_PROP, OBJ_WEAPON):
			case COLLISION_OF(OBJ_WEAPON, OBJ_PROP):
			case COLLISION_OF(OBJ_DEBRIS, OBJ_PROP):
			case COLLISION_OF(OBJ_PROP, OBJ_DEBRIS):
			case COLLISION_OF(OBJ_ASTEROID, OBJ_PROP):
			case COLLISION_OF(OBJ_PROP, OBJ_ASTEROID):
				COLLISION_PROF_INC(inline_prop); break;
			default:
				COLLISION_PROF_INC(inline_other); break;
		}
#endif
		const std::uint64_t narrow_start_ns = timer_get_nanoseconds();
		const int hit = check_collision(&new_pair);
		COLLISION_PROF_ADD(narrowphase_inline_ns, timer_get_nanoseconds() - narrow_start_ns);

		// Re-acquire rather than reusing collision_info: check_collision runs arbitrary game
		// logic, and creating an object can reach collide_remove_weapons(), which compacts the
		// pair cache and invalidates every pointer into it.  (The old SCP_unordered_map had
		// stable node addresses and did not need this.)
		bool still_present = false;
		collider_pair *updated = Collision_cached_pairs.get_or_create(key, still_present);
		if (!still_present) {
			updated->signature_a = A->signature;
			updated->signature_b = B->signature;
		}

		if (hit) {
			// don't have to check ever again
			updated->next_check_time = -1;
		} else {
			updated->next_check_time = new_pair.next_check_time;
		}
	}
}

void obj_find_overlap_colliders(SCP_vector<int> &overlap_list_out, SCP_vector<int> &list, bool collide)
{
    TRACE_SCOPE(tracing::FindOverlapColliders);

    bool first_not_added = true;
    SCP_vector<int> overlappers;

    for (int in_index : list){
        bool overlapped = false;

        const float min = obj_get_collider_endpoint(in_index, true);

        for (size_t j = 0; j < overlappers.size(); ) {
            const float overlap_max = obj_get_collider_endpoint(overlappers[j], false);
            if ( min <= overlap_max ) {
                overlapped = true;

                if ( overlappers.size() == 1 && first_not_added ) {
                    first_not_added = false;
                    overlap_list_out.push_back(overlappers[j]);
                }

                if ( collide ) {
                    obj_collide_pair(&Objects[in_index], &Objects[overlappers[j]]);
                }
            } else {
                overlappers[j] = overlappers.back();
                overlappers.pop_back();
                continue;
            }

            ++j;
        }

        if ( overlappers.empty() ) {
            first_not_added = true;
        }

        if ( overlapped ) {
            overlap_list_out.push_back(in_index);
        }

        overlappers.push_back(in_index);
    }
}

} //anon namespace

void collide_mp_worker_thread(size_t threadIdx) {
	auto& thread = collision_thread_data_buffer[threadIdx];
	thread.result_length.store(0);

	while (!thread.queue_process->empty() || thread.queue_length.load(std::memory_order_acquire) > 0 || !collision_processing_done.load(std::memory_order_acquire)) {
		if (!thread.queue_process->empty()) {

			for (auto& collision_check : *thread.queue_process) {
				collision_result (*check_collision)( obj_pair *pair ) = nullptr;

				switch( collision_check.ctype )	{
					case COLLISION_OF(OBJ_WEAPON, OBJ_SHIP):
					case COLLISION_OF(OBJ_SHIP, OBJ_WEAPON):
						check_collision = collide_ship_weapon_check;
						break;
					case COLLISION_OF(OBJ_SHIP, OBJ_SHIP):
						check_collision = collide_ship_ship_check;
						break;
					case COLLISION_OF(OBJ_DEBRIS, OBJ_WEAPON):
					case COLLISION_OF(OBJ_WEAPON, OBJ_DEBRIS):
						check_collision = collide_debris_weapon_check;
						break;
					case COLLISION_OF(OBJ_ASTEROID, OBJ_WEAPON):
					case COLLISION_OF(OBJ_WEAPON, OBJ_ASTEROID):
						check_collision = collide_asteroid_weapon_check;
						break;
					case COLLISION_OF(OBJ_DEBRIS, OBJ_SHIP):
					case COLLISION_OF(OBJ_SHIP, OBJ_DEBRIS):
						check_collision = collide_debris_ship_check;
						break;
					case COLLISION_OF(OBJ_ASTEROID, OBJ_SHIP):
					case COLLISION_OF(OBJ_SHIP, OBJ_ASTEROID):
						check_collision = collide_asteroid_ship_check;
						break;
					case COLLISION_OF(OBJ_WEAPON, OBJ_WEAPON):
						check_collision = collide_weapon_weapon_check;
						break;
					default:
						UNREACHABLE("Got non MP-compatible collision type %d!", collision_check.ctype);
						thread.queue_length.fetch_sub(1, std::memory_order_release); // keep the counter balanced
						continue;                                                    // skip the bad pair
				}

				auto&& [never_check_again, collision_data_maybe, collision_fnc] = check_collision(&collision_check.objs);

				{
					std::scoped_lock lock{thread.result_mutex};
					thread.queue_results->emplace_back(collision_thread_data::collision_queue_result{collision_check.objs, never_check_again, collision_data_maybe, collision_fnc});
				}
				thread.result_length.fetch_add(1, std::memory_order_release);
				thread.queue_length.fetch_sub(1, std::memory_order_release);
			}
			thread.queue_process->clear();
		}
		else if (thread.queue_length.load(std::memory_order_acquire) > 0) {
			//We must have data in the load queue then.
			std::scoped_lock lock(thread.queue_mutex);
			thread.queue_load.swap(thread.queue_process);
			thread.queue_load->clear();
		}
		else {
			// Nothing to do, and we cannot leave until the main thread says collision is done,
			// so park instead of spinning.  Re-checking both conditions under the queue mutex is
			// what closes the lost-wakeup window: queue_mp_collision bumps queue_length while
			// holding this mutex, and spin_down_mp_collision takes it before notifying.
			COLLISION_PROF_INC(worker_idle_spins);

			std::unique_lock<std::mutex> lock(thread.queue_mutex);
			if (thread.queue_length.load(std::memory_order_acquire) == 0
				&& !collision_processing_done.load(std::memory_order_acquire)) {
				thread.work_available.wait(lock);
			}
		}
	}

	collision_profiling::flush_local();
}

void collide_init() {
	if (threading::is_threading())
		collision_thread_data_buffer = std::make_unique<collision_thread_data[]>(threading::get_num_workers());
}

// used only in obj_sort_and_collide()
static SCP_vector<int> sort_list_y;
static SCP_vector<int> sort_list_z;

void obj_sort_and_collide(SCP_vector<int>* Collision_list)
{
	if (Cmdline_dis_collisions)
		return;

	if ( !(Game_detail_flags & DETAIL_FLAG_COLLISION) )
		return;

	const std::uint64_t collide_start_ns = timer_get_nanoseconds();

	if (!Collision_cache_stale_objects.empty()) {
		obj_collide_retime_stale_pairs();
	}

	// the main use case is to go through the main Collision detection list, so use that if
	// nothing is defined.
	if (Collision_list == nullptr) {
		Collision_list = &Collision_sort_list;
	}

	std::uint64_t phase_ns = timer_get_nanoseconds();

	sort_list_y.clear();
	{
		TRACE_SCOPE(tracing::SortColliders);
		obj_cache_collider_endpoints(*Collision_list, 0);
		obj_quicksort_colliders(Collision_list, 0, (int)(Collision_list->size() - 1), 0);
	}
	COLLISION_PROF_FRAME_ADD(sort_ns, timer_get_nanoseconds() - phase_ns);
	phase_ns = timer_get_nanoseconds();

	obj_find_overlap_colliders(sort_list_y, *Collision_list, false);

	COLLISION_PROF_FRAME_ADD(overlap_ns, timer_get_nanoseconds() - phase_ns);
	phase_ns = timer_get_nanoseconds();

	sort_list_z.clear();
	{
		TRACE_SCOPE(tracing::SortColliders);
		obj_cache_collider_endpoints(sort_list_y, 1);
		obj_quicksort_colliders(&sort_list_y, 0, (int)(sort_list_y.size() - 1), 1);
	}
	COLLISION_PROF_FRAME_ADD(sort_ns, timer_get_nanoseconds() - phase_ns);
	phase_ns = timer_get_nanoseconds();

	obj_find_overlap_colliders(sort_list_z, sort_list_y, false);

	COLLISION_PROF_FRAME_ADD(overlap_ns, timer_get_nanoseconds() - phase_ns);
	phase_ns = timer_get_nanoseconds();

	sort_list_y.clear();
	{
		TRACE_SCOPE(tracing::SortColliders);
		obj_cache_collider_endpoints(sort_list_z, 2);
		obj_quicksort_colliders(&sort_list_z, 0, (int)(sort_list_z.size() - 1), 2);
	}
	COLLISION_PROF_FRAME_ADD(sort_ns, timer_get_nanoseconds() - phase_ns);
	phase_ns = timer_get_nanoseconds();

	// Only this last pass generates pairs, so the workers have nothing to do before it.  Spinning
	// them up any earlier just parks every one of them in an idle wait for the whole broadphase.
	if (threading::is_threading())
		spin_up_mp_collision();

	obj_find_overlap_colliders(sort_list_y, sort_list_z, true);

	COLLISION_PROF_FRAME_ADD(overlap_ns, timer_get_nanoseconds() - phase_ns);
	phase_ns = timer_get_nanoseconds();

	if (threading::is_threading()) {
		flush_mp_collisions();
		post_process_threaded_collisions();
	}

	COLLISION_PROF_FRAME_ADD(drain_ns, timer_get_nanoseconds() - phase_ns);

	COLLISION_PROF_FRAME_ADD(collision_ns, timer_get_nanoseconds() - collide_start_ns);
	COLLISION_PROF_FRAME_ADD(cache_size, Collision_cached_pairs.size());
	collision_profiling::flush_local();
}

void collide_apply_gravity_flags_weapons() {
	for (object* obj = GET_FIRST(&obj_used_list); obj != END_OF_LIST(&obj_used_list); obj = GET_NEXT(obj)) {
		if (obj->type != OBJ_WEAPON || obj->flags[Object::Object_Flags::Should_be_dead])
			continue;

		weapon* wp = &Weapons[obj->instance];
		weapon_info* wip = &Weapon_info[wp->weapon_info_index];

		if (!wip->is_homing() || (wp->weapon_flags[Weapon::Weapon_Flags::No_homing])) {
			// homing weapons dont get any gravity stuff
			if (wip->acceleration_time <= 0.0f || Missiontime - wp->creation_time >= fl2f(wip->acceleration_time)) {
				// if the weapon doesn't accelerate, or has finished accelerating...
				if (The_mission.gravity == vmd_zero_vector || obj->phys_info.gravity_const == 0.0f) {
					obj->phys_info.flags |= PF_CONST_VEL;
					obj->phys_info.flags &= ~PF_BALLISTIC;
				}
				else {
					obj->phys_info.flags |= PF_BALLISTIC;
					obj->phys_info.flags &= ~PF_CONST_VEL;
				}
			}
		}
	}
}