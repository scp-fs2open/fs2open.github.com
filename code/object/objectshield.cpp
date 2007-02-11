/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 */ 

/*
 * $Logfile: /Freespace2/code/Object/ObjectShield.cpp $
 * $Revision: 2.1 $
 * $Date: 2007-02-11 21:26:35 $
 * $Author: Goober5000 $
 *
 * Shield-specific functions
 *
 * $Log: not supported by cvs2svn $
 *
 */

#include "object/object.h"
#include "ship/ship.h"
#include "network/multi.h"


float shield_get_strength(object *objp)
{
	// no shield system, no strength!
	if (objp->flags & OF_NO_SHIELDS)
		return 0.0f;

	int	i;
	float strength = 0.0f;

	for (i = 0; i < MAX_SHIELD_SECTIONS; i++)
		strength += shield_get_quad(objp, i);

	return strength;
}

void shield_set_strength(object *objp, float strength)
{
	int	i;

	for (i = 0; i < MAX_SHIELD_SECTIONS; i++)
		shield_set_quad(objp, i, strength / MAX_SHIELD_SECTIONS);
}

//	Recharge whole shield.
//	Apply delta/MAX_SHIELD_SECTIONS to each shield section.
void shield_add_strength(object *objp, float delta)
{
	// if we aren't going to change anything anyway then just bail
	if (delta == 0.0f)
		return;

	if (!(The_mission.ai_profile->flags & AIPF_SMART_SHIELD_MANAGEMENT))
	{
		for (int i = 0; i < MAX_SHIELD_SECTIONS; i++)
			shield_add_quad(objp, i, delta / MAX_SHIELD_SECTIONS);
	}
	else
	{
		float section_max = shield_get_max_quad(objp);

		// smart shield repair
		while (delta > 0.0f)
		{
			float weakest;
			int weakest_idx = -1;

			// find weakest shield quadrant
			for (int i = 0; i < MAX_SHIELD_SECTIONS; i++)
			{
				float quad = shield_get_quad(objp, i);
				if (weakest_idx < 0 || quad < weakest)
				{
					weakest = quad;
					weakest_idx = i;
				}
			}

			// all quads are at full strength
			if (weakest >= section_max)
				break;
		
			// throw all possible shield power at this quadrant
			// if there's any left over then apply it to the next weakest on the next pass
			float xfer_amount;
			if (weakest + delta > section_max)
				xfer_amount = section_max - weakest;
			else
				xfer_amount = delta;

			shield_add_quad(objp, weakest_idx, xfer_amount);
			delta -= xfer_amount;
		}
	}
}

// Goober5000
float shield_get_quad(object *objp, int quadrant_num)
{
	// no shield system, no strength!
	if (objp->flags & OF_NO_SHIELDS)
		return 0.0f;

	// check array bounds
	Assert(quadrant_num >= 0 && quadrant_num < MAX_SHIELD_SECTIONS);
	if (quadrant_num < 0 || quadrant_num >= MAX_SHIELD_SECTIONS)
		return 0.0f;

	return objp->shield_quadrant[quadrant_num];
}

// Goober5000
void shield_set_quad(object *objp, int quadrant_num, float strength)
{
	// check array bounds
	Assert(quadrant_num >= 0 && quadrant_num < MAX_SHIELD_SECTIONS);
	if (quadrant_num < 0 || quadrant_num >= MAX_SHIELD_SECTIONS)
		return;

	// check range
	if (strength < 0.0f)
		strength = 0.0f;
	float max_quad = shield_get_max_quad(objp);
	if (strength > max_quad)
		strength = max_quad;

	objp->shield_quadrant[quadrant_num] = strength;
}

// Goober5000
void shield_add_quad(object *objp, int quadrant_num, float delta)
{
	// if we aren't going to change anything anyway then just bail
	if (delta == 0.0f)
		return;

	// check array bounds
	Assert(quadrant_num >= 0 && quadrant_num < MAX_SHIELD_SECTIONS);
	if (quadrant_num < 0 || quadrant_num >= MAX_SHIELD_SECTIONS)
		return;

	// important: don't use shield_get_quad here
	float strength = objp->shield_quadrant[quadrant_num] + delta;

	// check range
	if (strength < 0.0f)
		strength = 0.0f;
	float max_quad = shield_get_max_quad(objp);
	if (strength > max_quad)
		strength = max_quad;

	objp->shield_quadrant[quadrant_num] = strength;
}

// Goober5000
float shield_get_max_strength(object *objp)
{
	if (objp->type != OBJ_SHIP)
		return 0.0f;

	return Ships[objp->instance].ship_max_shield_strength;
}

// Goober5000
float shield_get_max_quad(object *objp)
{
	return shield_get_max_strength(objp) / MAX_SHIELD_SECTIONS;
}

//	***** This is the version that works on a quadrant basis.
//	Return absolute amount of damage not applied.
float shield_apply_damage(object *objp, int quadrant_num, float damage)
{
	float remaining_damage;

	// multiplayer clients bail here if nodamage
	// if(MULTIPLAYER_CLIENT && (Netgame.debug_flags & NETD_FLAG_CLIENT_NODAMAGE)){
	if (MULTIPLAYER_CLIENT)
		return damage;

	// check array bounds
	Assert(quadrant_num >= 0 && quadrant_num < MAX_SHIELD_SECTIONS);
	if ((quadrant_num < 0) || (quadrant_num >= MAX_SHIELD_SECTIONS))
		return damage;	
	
	if (objp->type != OBJ_SHIP)
		return damage;

	Ai_info[Ships[objp->instance].ai_index].last_hit_quadrant = quadrant_num;

	remaining_damage = damage - shield_get_quad(objp, quadrant_num);
	if (remaining_damage > 0.0f)
	{
		shield_set_quad(objp, quadrant_num, 0.0f);
		return remaining_damage;
	}
	else
	{
		shield_add_quad(objp, quadrant_num, -damage);
		return 0.0f;
	}
}

// Returns true if the shield presents any opposition to something 
// trying to force through it.
// If quadrant is -1, looks at entire shield, otherwise
// just one quadrant
int shield_is_up(object *objp, int quadrant_num)
{
	if ((quadrant_num >= 0) && (quadrant_num < MAX_SHIELD_SECTIONS))
	{
		// Just check one quadrant
		float quad = shield_get_quad(objp, quadrant_num);

		if (quad > MAX(2.0f, 0.1f * shield_get_max_quad(objp)))
			return 1;
	}
	else
	{
		// Check all quadrants
		float strength = shield_get_strength(objp);

		if (strength > MAX(2.0f * MAX_SHIELD_SECTIONS, 0.1f * shield_get_max_strength(objp)))
			return 1;
	}

	return 0;	// no shield strength
}

//	return quadrant containing hit_pnt.
//	\  1  /.
//	3 \ / 0
//	  / \.
//	/  2  \.
//	Note: This is in the object's local reference frame.  Do _not_ pass a vector in the world frame.
int shield_get_quadrant(vec3d *hit_pnt)
{
	int	result = 0;

	if (hit_pnt->xyz.x < hit_pnt->xyz.z)
		result |= 1;

	if (hit_pnt->xyz.x < -hit_pnt->xyz.z)
		result |= 2;

	return result;
}

//	Given a global point and an object, get the quadrant number the point belongs to.
int shield_get_quadrant_global(object *objp, vec3d *global_pos)
{
	vec3d	tpos;
	vec3d	rotpos;

	vm_vec_sub(&tpos, global_pos, &objp->pos);
	vm_vec_rotate(&rotpos, &tpos, &objp->orient);

	return shield_get_quadrant(&rotpos);
}
