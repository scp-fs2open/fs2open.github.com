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
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "ship/ship.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"
#include "object/objectdock.h"



//#define MAX_PAIRS 10000	//	Bumped back to 10,000 by WMC
			//	Reduced from 10,000 to 6,000 by MK on 4/1/98.
			//	Most I saw was 3400 in sm1-06a, the asteriod mission.  No other mission came close.
#define MIN_PAIRS	2500	// start out with this many pairs
#define PAIRS_BUMP	1000		// increase by this many avialable pairs when more are needed

// the next 3 variables are used for pair statistics
// also in weapon.cpp there is Weapons_created.
int Pairs_created = 0;
int Num_pairs = 0;
int Num_pairs_allocated = 0;
int Num_pairs_checked = 0;
int pairs_not_created = 0;

int Num_pairs_hwm = 0;

obj_pair *Obj_pairs = NULL;

obj_pair pair_used_list;
obj_pair pair_free_list;

void obj_pairs_close()
{
	if (Obj_pairs != NULL) {
		vm_free(Obj_pairs);
		Obj_pairs = NULL;
	}

	Num_pairs_allocated = 0;
}

void obj_all_collisions_retime(int checkdly)
// sets all collisions to be checked (in 25ms by default)
// this is for when we warp objects
{
	obj_pair *parent, *tmp;	

	parent = &pair_used_list;
	tmp = parent->next;


	while (tmp != NULL) 
	{
		tmp->next_check_time = timestamp(checkdly);
		tmp = tmp->next;
	}
}


void obj_reset_pairs()
{
	int i;
	
//	mprintf(( "Resetting object pairs...\n" ));

	pair_used_list.a = pair_used_list.b = NULL;		
	pair_used_list.next = NULL;
	pair_free_list.a = pair_free_list.b = NULL;

	Num_pairs = 0;

	if (Obj_pairs != NULL) {
		vm_free(Obj_pairs);
		Obj_pairs = NULL;
	}

	Obj_pairs = (obj_pair*) vm_malloc_q( sizeof(obj_pair) * MIN_PAIRS );

	if ( Obj_pairs == NULL ) {
		mprintf(("Unable to create space for collision pairs!!\n"));
		return;
	}

	Num_pairs_allocated = MIN_PAIRS;

	memset( Obj_pairs, 0, sizeof(obj_pair) * MIN_PAIRS );

	for (i = 0; i < MIN_PAIRS; i++) {
		Obj_pairs[i].next = &Obj_pairs[i+1];
	}

	Obj_pairs[MIN_PAIRS-1].next = NULL;

	pair_free_list.next = &Obj_pairs[0];
}

// returns true if we should reject object pair if one is child of other.
int reject_obj_pair_on_parent(object *A, object *B)
{
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

// Adds the pair to the pair list
void obj_add_pair( object *A, object *B, int check_time, int add_to_end )
{
	uint ctype;
	int (*check_collision)( obj_pair *pair );
	int swapped = 0;	
	
	check_collision = NULL;

	if ( Num_pairs_allocated == 0 ) return;		// don't have anything to add the pair too

	if ( A==B ) return;		// Don't check collisions with yourself

	if ( !(A->flags&OF_COLLIDES) ) return;		// This object doesn't collide with anything
	if ( !(B->flags&OF_COLLIDES) ) return;		// This object doesn't collide with anything
	
	// Make sure you're not checking a parent with it's kid or vicy-versy
//	if ( A->parent_sig == B->signature && !(A->type == OBJ_SHIP && B->type == OBJ_DEBRIS) ) return;
//	if ( B->parent_sig == A->signature && !(A->type == OBJ_DEBRIS && B->type == OBJ_SHIP) ) return;
	if ( reject_obj_pair_on_parent(A,B) ) {
		return;
	}

	Assert( A->type < 127 );
	Assert( B->type < 127 );

	ctype = COLLISION_OF(A->type,B->type);
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
		// Only check collision's with player weapons
//		if ( Objects[B->parent].flags & OF_PLAYER_SHIP ) {
			check_collision = collide_asteroid_weapon;
//		}
		break;
	case COLLISION_OF(OBJ_WEAPON, OBJ_ASTEROID):
		swapped = 1;
		// Only check collision's with player weapons
//		if ( Objects[A->parent].flags & OF_PLAYER_SHIP ) {
			check_collision = collide_asteroid_weapon;
//		}
		break;
	case COLLISION_OF(OBJ_ASTEROID, OBJ_SHIP):
		// Only check collisions with player ships
//		if ( B->flags & OF_PLAYER_SHIP )	{
			check_collision = collide_asteroid_ship;
//		}
		break;
	case COLLISION_OF(OBJ_SHIP, OBJ_ASTEROID):
		// Only check collisions with player ships
//		if ( A->flags & OF_PLAYER_SHIP )	{
			check_collision = collide_asteroid_ship;
//		}
		swapped = 1;
		break;
	case COLLISION_OF(OBJ_SHIP,OBJ_SHIP):
		check_collision = collide_ship_ship;
		break;	
	
	case COLLISION_OF(OBJ_BEAM, OBJ_SHIP):
		if(beam_collide_early_out(A, B)){
			return;
		}
		check_collision = beam_collide_ship;
		break;

	case COLLISION_OF(OBJ_BEAM, OBJ_ASTEROID):
		if(beam_collide_early_out(A, B)){
			return;
		}
		check_collision = beam_collide_asteroid;
		break;

	case COLLISION_OF(OBJ_BEAM, OBJ_DEBRIS):
		if(beam_collide_early_out(A, B)){
			return;
		}
		check_collision = beam_collide_debris;
		break;

	case COLLISION_OF(OBJ_BEAM, OBJ_WEAPON):
		if(beam_collide_early_out(A, B)){
			return;
		}		
		check_collision = beam_collide_missile;
		break;

	case COLLISION_OF(OBJ_WEAPON, OBJ_WEAPON): {
		weapon_info *awip, *bwip;
		awip = &Weapon_info[Weapons[A->instance].weapon_info_index];
		bwip = &Weapon_info[Weapons[B->instance].weapon_info_index];

		if ((awip->weapon_hitpoints > 0) || (bwip->weapon_hitpoints > 0)) {
			if (bwip->weapon_hitpoints == 0) {
				check_collision = collide_weapon_weapon;
				swapped=1;
			} else {
				check_collision = collide_weapon_weapon;
			}
		}
/*

		if (awip->subtype != WP_LASER || bwip->subtype != WP_LASER) {
			if (awip->subtype == WP_LASER) {
				if ( bwip->wi_flags & WIF_BOMB ) {
					check_collision = collide_weapon_weapon;
				}
			} else if (bwip->subtype == WP_LASER) {
				if ( awip->wi_flags & WIF_BOMB ) {
					check_collision = collide_weapon_weapon;
					swapped=1;			
				}
			} else {
				if ( (awip->wi_flags&WIF_BOMB) || (bwip->wi_flags&WIF_BOMB) ) {
					check_collision = collide_weapon_weapon;
				}
			}
		}
*/
/*
		int	atype, btype;

		atype = Weapon_info[Weapons[A->instance].weapon_info_index].subtype;
		btype = Weapon_info[Weapons[B->instance].weapon_info_index].subtype;

		if ((atype == WP_LASER) && (btype == WP_MISSILE))
			check_collision = collide_weapon_weapon;
		else if ((atype == WP_MISSILE) && (btype == WP_LASER)) {
			check_collision = collide_weapon_weapon;
			swapped = 1;
		} else if ((atype == WP_MISSILE) && (btype == WP_MISSILE))
			check_collision = collide_weapon_weapon;
*/

		break;
	}

	default:
		return;
	}

	// Swap them if needed
	if ( swapped )	{
		object *tmp = A;
		A = B;
		B = tmp;
	}

	// if there are any more obj_pair checks
	// we should then add function int maybe_not_add_obj_pair()
	// MWA -- 4/1/98 -- I'd do it, but I don't want to bust anything, so I'm doing my stuff here instead :-)
	//if ( MULTIPLAYER_CLIENT && !(Netgame.debug_flags & NETD_FLAG_CLIENT_NODAMAGE)){
		// multiplayer clients will only do ship/ship collisions, and their own ship to boot
	//	if ( check_collision != collide_ship_ship ){
	//		return;
	//	}

	//	if ( (A != Player_obj) && (B != Player_obj) ){
	//		return;
	//	}
	//}	

	// only check debris:weapon collisions for player
	if (check_collision == collide_debris_weapon) {
		// weapon is B
		if ( !(Weapon_info[Weapons[B->instance].weapon_info_index].wi_flags & WIF_TURNS) ) {
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
					return;
				}
			}

			// check dist vs. dist moved during weapon lifetime
			vec3d delta_v;
			vm_vec_sub(&delta_v, &B->phys_info.vel, &A->phys_info.vel);
			if (vm_vec_dist_squared(&A->pos, &B->pos) > (vm_vec_mag_squared(&delta_v)*Weapons[B->instance].lifeleft*Weapons[B->instance].lifeleft)) {
				return;
			}

			// for nonplayer ships, only create collision pair if close enough
			if ( (B->parent >= 0) && !(Objects[B->parent].flags & OF_PLAYER_SHIP) && (vm_vec_dist(&B->pos, &A->pos) < (4.0f*A->radius + 200.0f)) )
				return;
		}
	}

	// don't check same team laser:ship collisions on small ships if not player
	if (check_collision == collide_ship_weapon) {
		// weapon is B
		if ( (B->parent >= 0)
			&& !(Objects[B->parent].flags & OF_PLAYER_SHIP)
			&& (Ships[Objects[B->parent].instance].team == Ships[A->instance].team) 
			&& (Ship_info[Ships[A->instance].ship_info_index].flags & SIF_SMALL_SHIP) 
			&& (Weapon_info[Weapons[B->instance].weapon_info_index].subtype == WP_LASER) ) {
			pairs_not_created++;
			return;
		}
	}

	if ( !check_collision ) return;
	Pairs_created++;

	// At this point, we have determined that collisions between
	// these two should be checked, so add the pair to the
	// collision pair list.

	if ( pair_free_list.next == NULL )	{
		nprintf(( "collision", "Out of object pairs!! Not all collisions will work!\n" ));
		return;
	}

	Num_pairs++;
/*	if (Num_pairs > Num_pairs_hwm) {
		Num_pairs_hwm = Num_pairs;
		//nprintf(("AI", "Num_pairs high water mark = %i\n", Num_pairs_hwm));
	}
*/

	if ( Num_pairs >= (Num_pairs_allocated - 20) ) {
		int i;

		Assert( Obj_pairs != NULL );

		int old_pair_count = Num_pairs_allocated;
		obj_pair *old_pairs_ptr = Obj_pairs;

		// determine where we need to update the "previous" ptrs to
		int prev_free_mark = (pair_free_list.next - old_pairs_ptr);
		int prev_used_mark = (pair_used_list.next - old_pairs_ptr);

		Obj_pairs = (obj_pair*) vm_realloc_q( Obj_pairs, sizeof(obj_pair) * (Num_pairs_allocated + PAIRS_BUMP) );

		// allow us to fail here and only if we don't do we setup the new pairs

		if (Obj_pairs == NULL) {
			// failed, just go back to the way we were and use only the pairs we have already
			Obj_pairs = old_pairs_ptr;
		} else {
			Num_pairs_allocated += PAIRS_BUMP;

			Assert( Obj_pairs != NULL );

			// have to reset all of the "next" ptrs for the old set and handle the new set
			for (i = 0; i < Num_pairs_allocated; i++) {
				if (i >= old_pair_count) {
					memset( &Obj_pairs[i], 0, sizeof(obj_pair) );
					Obj_pairs[i].next = &Obj_pairs[i+1];
				} else {
					if (Obj_pairs[i].next != NULL) {
						// the "next" ptr will end up going backwards for used pairs so we have
						// to allow for that with this craziness...
						int next_mark = (Obj_pairs[i].next - old_pairs_ptr);
						Obj_pairs[i].next = &Obj_pairs[next_mark];
					}

					// catch that last NULL from the previously allocated set
					if ( i == (old_pair_count-1) ) {
						Obj_pairs[i].next = &Obj_pairs[i+1];
					}
				}
			}

			Obj_pairs[Num_pairs_allocated-1].next = NULL;

			// reset the "previous" ptrs
			pair_free_list.next = &Obj_pairs[prev_free_mark];
			pair_used_list.next = &Obj_pairs[prev_used_mark];
		}
	}

	// get a new obj_pair from the free list
	obj_pair * new_pair = pair_free_list.next;
	pair_free_list.next = new_pair->next;

	if ( add_to_end ) {
		obj_pair *last, *tmp;

		last = tmp = pair_used_list.next;
		while( tmp != NULL )	{
			if ( tmp->next == NULL )
				last = tmp;

			tmp = tmp->next;
		}

		if ( last == NULL )
			last = &pair_used_list;
			
		last->next = new_pair;
		Assert(new_pair != NULL);
		new_pair->next = NULL;
	}
	else {
		new_pair->next = pair_used_list.next;
		pair_used_list.next = new_pair;
	}

	A->num_pairs++;
	B->num_pairs++;
	
	new_pair->a = A;
	new_pair->b = B;
	new_pair->check_collision = check_collision;

	if ( check_time == -1 ){
		new_pair->next_check_time = timestamp(0);	// 0 means instantly time out
	} else {
		new_pair->next_check_time = check_time;
	}

}

MONITOR(NumPairs)
MONITOR(NumPairsChecked)

//#define PAIR_STATS

extern int Cmdline_dis_collisions;
void obj_check_all_collisions()
{
	obj_pair *parent, *tmp;	

#ifdef PAIR_STATS
	// debug info
	float avg_time_to_next_check = 0.0f;
#endif

	if (Cmdline_dis_collisions)
		return;

	if ( !(Game_detail_flags & DETAIL_FLAG_COLLISION) )
		return;


	parent = &pair_used_list;
	tmp = parent->next;

	Num_pairs_checked = 0;

	while (tmp != NULL) {
		int removed = 0;

		if ( !timestamp_elapsed(tmp->next_check_time) )
			goto NextPair;

		if ( (tmp->a) && (tmp->b) ) {
			Num_pairs_checked++;

			if ( (*tmp->check_collision)(tmp) ) {
				// We never need to check this pair again.
				#if 0	//def DONT_REMOVE_PAIRS
					// Never check it again, but keep the pair around
					// (useful for debugging)
					tmp->next_check_time = timestamp(-1);		
				#else
					// Never check it again, so remove the pair
					removed = 1;
					tmp->a->num_pairs--;	
					Assert( tmp->a->num_pairs > -1 );
					tmp->b->num_pairs--;
					Assert( tmp->b->num_pairs > -1 );
					Num_pairs--;
					// Assert(Num_pairs >= 0);
					parent->next = tmp->next;
					tmp->a = tmp->b = NULL;
					tmp->next = pair_free_list.next;
					pair_free_list.next = tmp;
					tmp = parent->next;
				#endif
			}
		} 

NextPair:
		if ( !removed ) {
			parent = tmp;
			tmp = tmp->next;

#ifdef PAIR_STATS
			// debug info
			if (tmp) {
				int add_time = timestamp_until( tmp->next_check_time );
				if (add_time > 0)
					avg_time_to_next_check += (float) add_time;
			}
#endif
		}
	}

	MONITOR_INC(NumPairs, Num_pairs);
	MONITOR_INC(NumPairsChecked, Num_pairs_checked);

#ifdef PAIR_STATS
	avg_time_to_next_check = avg_time_to_next_check / Num_pairs;
	extern int Num_hull_pieces;
	extern int Weapons_created;
	// mprintf(( "[pairs checked: %d, start_pairs: %d, num obj: %d, avg next time: %f]\n", n, org_pairs, Num_objects, avg_time_to_next_check ));
	// mprintf(( "[Num_hull_pieces: %3d, Num_weapons_created: %3d, pairs_not_created: %3d, pairs_created: %3d, percent new saved: %9.5f]\n", Num_hull_pieces, Weapons_created, pairs_not_created, Pairs_created, 100.0f*(float)pairs_not_created/(float)(pairs_not_created + Pairs_created) ));
	 mprintf(( "[pairs_created: %3d, pairs_not_created: %3d, percent saved %6.3f]\n", Pairs_created, pairs_not_created, 100.0f*pairs_not_created/(Pairs_created+pairs_not_created) ));
	pairs_not_created = 0;
	Weapons_created = 0;
	Pairs_created = 0;
#endif

	// What percent of the pairs did we check?
	// FYI: (n*(n-1))/2 is the total number of checks required for comparing n objects.

//	if ( org_pairs > 1 )	{
//		Object_checked_percentage = (i2fl(n)*100.0f) / i2fl(org_pairs);
//	} else {
//		Object_checked_percentage = 0.0f;
//	}

}

//	See if two lines intersect by doing recursive subdivision.
//	Bails out if larger distance traveled is less than sum of radii + 1.0f.
int collide_subdivide(vec3d *p0, vec3d *p1, float prad, vec3d *q0, vec3d *q1, float qrad)
{
	float	a_dist, b_dist, ab_dist;

	a_dist = vm_vec_dist(p0, p1);
	b_dist = vm_vec_dist(q0, q1);

	ab_dist = vm_vec_dist(p1, q1);

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
	object	A_future;
	vec3d	hitpos;


	A_future = *A;
	vm_vec_scale_add2(&A_future.pos, &A->phys_info.vel, duration);

	if (radius_scale == 0.0f) {
		return ship_check_collision_fast(B, &A_future, &hitpos );
	} else {
		float		size_A, size_B, dist, r;
		vec3d	nearest_point;

		size_A = A->radius * radius_scale;
		size_B = B->radius * radius_scale;

		//	If A is moving, check along vector.
		if (A->phys_info.speed != 0.0f) {
			r = find_nearest_point_on_line(&nearest_point, &A->pos, &A_future.pos, &B->pos);
			if (r < 0) {
				nearest_point = A->pos;
			} else if (r > 1) {
				nearest_point = A_future.pos;
			}
			dist = vm_vec_dist_quick(&B->pos, &nearest_point);
			return (dist < size_A + size_B);
		} else {
			return vm_vec_dist_quick(&B->pos, &A->pos) < size_A + size_B;
		}
	}
}

//	Return true if the vector from *start_pos to *end_pos is within objp->radius*radius_scale of *objp
int vector_object_collision(vec3d *start_pos, vec3d *end_pos, object *objp, float radius_scale)
{
	float		dist, r;
	vec3d	nearest_point;

	r = find_nearest_point_on_line(&nearest_point, start_pos, end_pos, &objp->pos);
	if ((r >= 0.0f) && (r <= 1.0f)) {
		dist = vm_vec_dist_quick(&objp->pos, &nearest_point);

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

//	mprintf(( "Frame: %d,  Weapon=%d, Other=%d, pair=$%08x\n", G3_frame_count, OBJ_INDEX(weapon), OBJ_INDEX(other), current_pair ));
	

	// Do some checks for weapons that don't turn
	if ( !(wip->wi_flags & WIF_TURNS) )	{

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

		float max_vel_weapon, max_vel_other;

		//SUSHI: Fix bug where additive weapon velocity screws up collisions
		//Assumes that weapons which don't home don't change speed, which is currently the case.
		if (!(wip->wi_flags & WIF_TURNS))
			max_vel_weapon = obj_weapon->phys_info.speed;
		else if (wp->lssm_stage==5)
			max_vel_weapon = wip->lssm_stage5_vel;
		else
			max_vel_weapon = wp->weapon_max_vel;

		max_vel_other = other->phys_info.max_vel.xyz.z;
		if (max_vel_other < 10.0f) {
			if ( vm_vec_mag_squared( &other->phys_info.vel ) > 100 ) {
				// bump up velocity from collision
				max_vel_other = vm_vec_mag( &other->phys_info.vel ) + 10.0f;
			} else {
				max_vel_other = 10.0f;		// object may move from collision
			}
		}

		// check weapon that does not turn against sphere expanding at ship maxvel
		// compare (weeapon) ray with expanding sphere (ship) to find earliest possible collision time
		// look for two time solutions to Xw = Xs, where Xw = Xw0 + Vwt*t  Xs = Xs + Vs*(t+dt), where Vs*dt = radius of ship 
		// Since direction of Vs is unknown, solve for (Vs*t) and find norm of both sides
		if ( !(wip->wi_flags & WIF_TURNS) ) {
			vec3d delta_x, laser_vel;
			float a,b,c, delta_x_dot_vl, delta_t;
			float root1, root2, root, earliest_time;

			if (max_vel_weapon == max_vel_other) {
				// this will give us NAN using the below formula, so check every frame
				current_pair->next_check_time = timestamp(0);
				return 0;
			}

			vm_vec_sub( &delta_x, &obj_weapon->pos, &other->pos );
			laser_vel = obj_weapon->phys_info.vel;
			// vm_vec_copy_scale( &laser_vel, &weapon->orient.vec.fvec, max_vel_weapon );
			delta_t = (other->radius + 10.0f) / max_vel_other;		// time to get from center to radius of other obj
			delta_x_dot_vl = vm_vec_dotprod( &delta_x, &laser_vel );

			a = max_vel_weapon*max_vel_weapon - max_vel_other*max_vel_other;
			b = 2.0f * (delta_x_dot_vl - max_vel_other*max_vel_other*delta_t);
			c = vm_vec_mag_squared( &delta_x ) - max_vel_other*max_vel_other*delta_t*delta_t;

			float discriminant = b*b - 4.0f*a*c;
			if ( discriminant < 0) {
				// never hit
				return 1;
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

			float dist, max_vel, time;

			max_vel = max_vel_weapon + max_vel_other;

			// suggest that fudge factor for other radius be changed to other_radius + const (~10)
			dist = vm_vec_dist( &other->pos, &obj_weapon->pos ) - (other->radius + 10.0f);
			if ( dist > 0.0f )	{
				time = (dist*1000.0f) / max_vel;
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
	float	radius;
	
	radius = objp->radius;
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
	vec3d	cur_pos, goal_pos;
	ship_info	*sip;

	sip = &Ship_info[Ships[objp->instance].ship_info_index];

	cur_pos = objp->pos;

	vm_vec_scale_add(&goal_pos, &cur_pos, &objp->orient.vec.fvec, distance);

	for ( objp2 = GET_FIRST(&obj_used_list); objp2 != END_OF_LIST(&obj_used_list); objp2 = GET_NEXT(objp2) ) {
		if ((objp != objp2) && (objp2->type == OBJ_SHIP)) {
			if (Ship_info[Ships[objp2->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
				if (dock_check_find_docked_object(objp, objp2))
					continue;

				if (cpls_aux(&goal_pos, objp2, objp))
					return 1;
			}
		} else if (!(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) && (objp2->type == OBJ_ASTEROID)) {
			if (vm_vec_dist_quick(&objp2->pos, &objp->pos) < (distance + objp2->radius)*2.5f) {
				vec3d	pos, delvec;
				int		count;
				float		d1;

				d1 = 2.5f * distance + objp2->radius;
				count = (int) (d1/(objp2->radius + objp->radius));	//	Scale up distance, else looks like there would be a collision.
				pos = cur_pos;
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
	float next_check_time;
	weapon *wp;

	wp = &Weapons[weapon_num];

	// if this weapons life left > time before next collision, then we cannot remove it
	crw_status[WEAPON_INDEX(wp)] = CRW_IN_PAIR;
	next_check_time = ((float)(timestamp_until(collide_next_check)) / 1000.0f);
	if ( wp->lifeleft < next_check_time )
		crw_status[WEAPON_INDEX(wp)] = CRW_CAN_DELETE;
}

int collide_remove_weapons( )
{
	obj_pair *opp;
	int i, num_deleted, oldest_index, j, loop_count;
	float oldest_time;

	// setup remove_weapon array.  assume we can remove it.
	for (i = 0; i < MAX_WEAPONS; i++ ) {
		if ( Weapons[i].objnum == -1 )
			crw_status[i] = CRW_NO_OBJECT;
		else
			crw_status[i] = CRW_NO_PAIR;
	}

	// first pass is to see if any of the weapons don't have collision pairs.

	opp = &pair_used_list;
	opp = opp->next;
	while( opp != NULL )	{
		// for each collide pair, if the two objects can still collide, then set the remove_weapon
		// parameter for the weapon to 0.  need to check both parameters
		if ( opp->a->type == OBJ_WEAPON )
			crw_check_weapon( opp->a->instance, opp->next_check_time );

		if ( opp->b->type == OBJ_WEAPON )
			crw_check_weapon( opp->b->instance, opp->next_check_time );

		opp = opp->next;
	}

	// for each weapon which could be removed, delete the object
	num_deleted = 0;
	for ( i = 0; i < MAX_WEAPONS; i++ ) {
		if ( crw_status[i] == CRW_CAN_DELETE ) {
			Assert( Weapons[i].objnum != -1 );
			obj_delete( Weapons[i].objnum );
			num_deleted++;
		}
	}

	if ( num_deleted )
		return num_deleted;

	// if we didn't remove any weapons, try to the N oldest weapons.  first checking for pairs, then
	// checking for oldest weapons in general.  We will go through the loop a max of 2 times.  first time
	// through, we check oldest weapons with pairs, next time through, for oldest weapons.
	loop_count = 0;
	do {
		for ( j = 0; j < CRW_MAX_TO_DELETE; j++ ) {
			oldest_time = 1000.0f;
			oldest_index = -1;
			for (i = 0; i < MAX_WEAPONS; i++ ) {
				if ( Weapons[i].objnum == -1 )			// shouldn't happen, but this is the safe thing to do.
					continue;
				if ( ((loop_count || crw_status[i] == CRW_NO_PAIR)) && (Weapons[i].lifeleft < oldest_time) ) {
					oldest_time = Weapons[i].lifeleft;
					oldest_index = i;
				}
			}
			if ( oldest_index != -1 ) {
				obj_delete(Weapons[oldest_index].objnum);
				num_deleted++;
			}
		}

		// if we deleted some weapons, then we can break
		if ( num_deleted )
			break;

		loop_count++;
	} while ( loop_count < 2);

	return num_deleted;

}

void set_hit_struct_info(collision_info_struct *hit, mc_info *mc, int submodel_rot_hit)
{
	hit->edge_hit = mc->edge_hit;
	hit->hit_pos = mc->hit_point_world;
	hit->hit_time = mc->hit_dist;
	hit->submodel_num = mc->hit_submodel;

	hit->submodel_rot_hit = submodel_rot_hit;
}

//Previously, this was done with 
//memset(&ship_ship_hit_info, -1, sizeof(collision_info_struct));
//All those -1s are to replicate that logic
void init_collision_info_struct(collision_info_struct *cis)
{
	memset(cis, -1, sizeof(collision_info_struct));
	cis->is_landing = false;
}
