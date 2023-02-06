/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#include "globalincs/linklist.h"
#include "io/timer.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "ship/ship.h"
#include "tracing/tracing.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"
#include "tracing/Monitor.h"


// the next 2 variables are used for pair statistics
// also in weapon.cpp there is Weapons_created.
int Num_pairs = 0;
int Num_pairs_checked = 0;

SCP_vector<int> Collision_sort_list;

class collider_pair
{
public:
	object *a;
	object *b;
	int signature_a;
	int signature_b;
	int next_check_time;
	bool initialized;

	// we need to define a constructor because the hash map can
	// implicitly insert an object when we use the [] operator
	collider_pair()
		: a(nullptr), b(nullptr), signature_a(-1), signature_b(-1), next_check_time(-1), initialized(false)
	{}
};

static SCP_unordered_map<uint, collider_pair> Collision_cached_pairs;

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

int reject_due_collision_groups(object *A, object *B)
{
	if (A->collision_group_id == 0 || B->collision_group_id == 0)
		return 0;

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
	mc_info mc;
	mc_info_init(&mc);

	Assert(goalobjp->type == OBJ_SHIP);

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
	for (auto& pair : Collision_cached_pairs) {
        collider_pair* pair_obj = &pair.second;

		if (!pair_obj->initialized) {
			continue;
		}

		if (pair_obj->a->type == OBJ_WEAPON && pair_obj->signature_a == pair_obj->a->signature) {
			crw_check_weapon(pair_obj->a->instance, pair_obj->next_check_time);

			if (crw_status[pair_obj->a->instance] == CRW_CAN_DELETE) {
				pair_obj->initialized = false;
			}
		}

		if (pair_obj->b->type == OBJ_WEAPON && pair_obj->signature_b == pair_obj->b->signature) {
			crw_check_weapon(pair_obj->b->instance, pair_obj->next_check_time);

			if (crw_status[pair_obj->b->instance] == CRW_CAN_DELETE) {
				pair_obj->initialized = false;
			}
		}
	}

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
	hit->submodel_num = mc->hit_submodel;

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

void obj_collide_retime_cached_pairs()
{
	for ( auto& pair : Collision_cached_pairs ) {
		pair.second.next_check_time = timestamp(0);
	}
}

//local helper functions only used in objcollide.cpp
namespace
{

float obj_get_collider_endpoint(int obj_num, int axis, bool min)
{
    if ( Objects[obj_num].type == OBJ_BEAM ) {
        beam *b = &Beams[Objects[obj_num].instance];

        // use the last start and last shot as endpoints
        float min_end, max_end;
        if ( b->last_start.a1d[axis] > b->last_shot.a1d[axis] ) {
            min_end = b->last_shot.a1d[axis];
            max_end = b->last_start.a1d[axis];
        } else {
            min_end = b->last_start.a1d[axis];
            max_end = b->last_shot.a1d[axis];
        }

        if ( min ) {
            return min_end;
        } else {
            return max_end;
        }
    } else if ( Objects[obj_num].type == OBJ_WEAPON ) {
        float min_end, max_end;

        if ( Objects[obj_num].pos.a1d[axis] > Objects[obj_num].last_pos.a1d[axis] ) {
            min_end = Objects[obj_num].last_pos.a1d[axis];
            max_end = Objects[obj_num].pos.a1d[axis];
        } else {
            min_end = Objects[obj_num].pos.a1d[axis];
            max_end = Objects[obj_num].last_pos.a1d[axis];
        }

        if ( min ) {
            return min_end - Objects[obj_num].radius;
        } else {
            return max_end + Objects[obj_num].radius;
        }
    } else {
        vec3d *pos = &Objects[obj_num].pos;

        if ( min ) {
            return pos->a1d[axis] - Objects[obj_num].radius;
        } else {
            return pos->a1d[axis] + Objects[obj_num].radius;
        }
    }
}

void obj_quicksort_colliders(SCP_vector<int> *list, int left, int right, int axis)
{
    Assert( axis >= 0 );
    Assert( axis <= 2 );

    if ( right > left ) {
        int pivot_index = left + (right - left) / 2;

        float pivot_value = obj_get_collider_endpoint((*list)[pivot_index], axis, true);

        // swap!
        int temp = (*list)[pivot_index];
        (*list)[pivot_index] = (*list)[right];
        (*list)[right] = temp;

        int store_index = left;

        for (int i = left; i < right; ++i ) {
            if ( obj_get_collider_endpoint((*list)[i], axis, true) <= pivot_value ) {
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

void obj_collide_pair(object *A, object *B)
{
    TRACE_SCOPE(tracing::CollidePair);

    int (*check_collision)( obj_pair *pair ) = nullptr;
    int swapped = 0;

    if ( A==B ) return;		// Don't check collisions with yourself

    if ( !(A->flags[Object::Object_Flags::Collides]) ) return;		// This object doesn't collide with anything
    if ( !(B->flags[Object::Object_Flags::Collides]) ) return;		// This object doesn't collide with anything

    if ((A->flags[Object::Object_Flags::Immobile]) && (B->flags[Object::Object_Flags::Immobile])) return;	// Two immobile objects will never collide with each other

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
            break;
        case COLLISION_OF(OBJ_SHIP, OBJ_WEAPON):
            check_collision = collide_ship_weapon;
            break;
        case COLLISION_OF(OBJ_DEBRIS, OBJ_WEAPON):
            check_collision = collide_debris_weapon;
            break;
        case COLLISION_OF(OBJ_WEAPON, OBJ_DEBRIS):
            swapped = 1;
            check_collision = collide_debris_weapon;
            break;
        case COLLISION_OF(OBJ_DEBRIS, OBJ_SHIP):
            check_collision = collide_debris_ship;
            break;
        case COLLISION_OF(OBJ_SHIP, OBJ_DEBRIS):
            check_collision = collide_debris_ship;
            swapped = 1;
            break;
        case COLLISION_OF(OBJ_ASTEROID, OBJ_WEAPON):
            check_collision = collide_asteroid_weapon;
            break;
        case COLLISION_OF(OBJ_WEAPON, OBJ_ASTEROID):
            swapped = 1;
            check_collision = collide_asteroid_weapon;
            break;
        case COLLISION_OF(OBJ_ASTEROID, OBJ_SHIP):
            check_collision = collide_asteroid_ship;
            break;
        case COLLISION_OF(OBJ_SHIP, OBJ_ASTEROID):
            check_collision = collide_asteroid_ship;
            swapped = 1;
            break;
        case COLLISION_OF(OBJ_SHIP,OBJ_SHIP):
            check_collision = collide_ship_ship;
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
            }

            break;
        }

        default:
            return;
    }

    if ( !check_collision ) return;

    // Swap them if needed
    if ( swapped ) {
        std::swap(A,B);
    }

    bool valid = false;
    uint key = (OBJ_INDEX(A) << 12) + OBJ_INDEX(B);

    collider_pair* collision_info = &Collision_cached_pairs[key];

    if ( collision_info->initialized ) {
        // make sure we're referring to the correct objects in case the original pair was deleted
        if ( collision_info->signature_a == collision_info->a->signature &&
             collision_info->signature_b == collision_info->b->signature ) {
            valid = true;
        } else {
            collision_info->a = A;
            collision_info->b = B;
            collision_info->signature_a = A->signature;
            collision_info->signature_b = B->signature;
            collision_info->next_check_time = timestamp(0);
        }
    } else {
        collision_info->a = A;
        collision_info->b = B;
        collision_info->signature_a = A->signature;
        collision_info->signature_b = B->signature;
        collision_info->initialized = true;
        collision_info->next_check_time = timestamp(0);
    }

    if ( valid ) {
        // if this signature is valid, make the necessary checks to see if we need to collide check
        if ( collision_info->next_check_time == -1 ) {
            return;
        } else {
            if ( !timestamp_elapsed(collision_info->next_check_time) ) {
                return;
            }
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
                if ( (B->parent >= 0) && !((Objects[B->parent].signature == B->parent_sig) && (Objects[B->parent].flags[Object::Object_Flags::Player_ship])) && (vm_vec_dist(&B->pos, &A->pos) < (4.0f*A->radius + 200.0f)) ) {
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

    if ( check_collision(&new_pair) ) {
        // don't have to check ever again
        collision_info->next_check_time = -1;
    } else {
        collision_info->next_check_time = new_pair.next_check_time;
    }
}

void obj_find_overlap_colliders(SCP_vector<int> &overlap_list_out, SCP_vector<int> &list, int axis, bool collide)
{
    TRACE_SCOPE(tracing::FindOverlapColliders);

    bool first_not_added = true;
    SCP_vector<int> overlappers;

    for (int in_index : list){
        bool overlapped = false;

        const float min = obj_get_collider_endpoint(in_index, axis, true);

        for (size_t j = 0; j < overlappers.size(); ) {
            const float overlap_max = obj_get_collider_endpoint(overlappers[j], axis, false);
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

// used only in obj_sort_and_collide()
static SCP_vector<int> sort_list_y;
static SCP_vector<int> sort_list_z;

void obj_sort_and_collide(SCP_vector<int>* Collision_list)
{
	if (Cmdline_dis_collisions)
		return;

	if ( !(Game_detail_flags & DETAIL_FLAG_COLLISION) )
		return;

	// the main use case is to go through the main Collision detection list, so use that if
	// nothing is defined.
	if (Collision_list == nullptr) {
		Collision_list = &Collision_sort_list;
	}

	sort_list_y.clear();
	{
		TRACE_SCOPE(tracing::SortColliders);
		obj_quicksort_colliders(Collision_list, 0, (int)(Collision_list->size() - 1), 0);
	}
	obj_find_overlap_colliders(sort_list_y, *Collision_list, 0, false);

	sort_list_z.clear();
	{
		TRACE_SCOPE(tracing::SortColliders);
		obj_quicksort_colliders(&sort_list_y, 0, (int)(sort_list_y.size() - 1), 1);
	}
	obj_find_overlap_colliders(sort_list_z, sort_list_y, 1, false);

	sort_list_y.clear();
	{
		TRACE_SCOPE(tracing::SortColliders);
		obj_quicksort_colliders(&sort_list_z, 0, (int)(sort_list_z.size() - 1), 2);
	}
	obj_find_overlap_colliders(sort_list_y, sort_list_z, 2, true);
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