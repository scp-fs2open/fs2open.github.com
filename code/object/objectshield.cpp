/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 */ 



#include "math/staticrand.h"
#include "network/multi.h"
#include "object/object.h"
#include "object/objectshield.h"
#include "ship/ship.h"
#include "ship/subsysdamage.h"

#include <limits.h>


float shield_get_strength(object *objp)
{
	// no shield system, no strength!
	if (objp->flags & OF_NO_SHIELDS)
		return 0.0f;

	int	i;
	float strength = 0.0f;

	for (i = 0; i < objp->n_quadrants; i++)
		strength += shield_get_quad(objp, i);

	return strength;
}

void shield_set_strength(object *objp, float strength)
{
	int	i;

	for (i = 0; i < objp->n_quadrants; i++)
		shield_set_quad(objp, i, strength / objp->n_quadrants);
}

//	Recharge whole shield.
//	Apply delta/n_quadrants to each shield section.
void shield_add_strength(object *objp, float delta)
{
	// if we aren't going to change anything anyway then just bail
	if (delta == 0.0f)
		return;

	
	if (!(Ai_info[Ships[objp->instance].ai_index].ai_profile_flags & AIPF_SMART_SHIELD_MANAGEMENT)
		|| delta <= 0.0f) //SUSHI: We don't want smart shield management for negative delta
	{
		for (int i = 0; i < objp->n_quadrants; i++)
			shield_add_quad(objp, i, delta / objp->n_quadrants);
	}
	else
	{
		float section_max = shield_get_max_quad(objp);

		// smart shield repair
		while (delta > 0.0f)
		{
			//WMC - Set to INT_MAX so that this is set to something
			float weakest = i2fl(INT_MAX);
			int weakest_idx = -1;

			// find weakest shield quadrant
			for (int i = 0; i < objp->n_quadrants; i++)
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

static double factor = 1.0 / (log(50.0) - log(1.0));

// Goober5000
float scale_quad(float generator_fraction, float quad_strength)
{
	// the following formula makes a nice logarithmic curve between 1 and 50,
	// when x goes from 0 to 100:
	//
	// ln(x) * (100 - 0)
	// -----------------
	//  ln(50) - ln(1)
	//
	float effective_strength = quad_strength * ((float)log(generator_fraction) * (float)factor);

	// ensure not negative, which may happen if the shield gets below 1 percent
	// (since we're dealing with logs)
	if (effective_strength < 0.0f)
		return 0.0f;
	else
		return effective_strength;
}

// Goober5000
float shield_get_quad(object *objp, int quadrant_num)
{
	// no shield system, no strength!
	if (objp->flags & OF_NO_SHIELDS)
		return 0.0f;

	// check array bounds
	Assert(quadrant_num >= 0 && quadrant_num < objp->n_quadrants);
	if (quadrant_num < 0 || quadrant_num >= objp->n_quadrants)
		return 0.0f;

	if (objp->type != OBJ_SHIP && objp->type != OBJ_START)
		return 0.0f;

	//WMC -	I removed SUBSYSTEM_SHIELD_GENERATOR to prevent pilot file
	//		corruption, so comment all this out...
	/*
	// yarr!
	ship_subsys_info *ssip = &Ships[objp->instance].subsys_info[SUBSYSTEM_SHIELD_GENERATOR];

	// do we have a shield generator?
	if (ssip->num > 0 && !Fred_running)
	{
		// rules for shield generator affecting coverage:
		//	1. if generator above 50%, effective strength = actual strength
		//	2. if generator below 50%, effective strength uses the scale_quad formula
		//	3. if generator below 30%, shields only have a sqrt(generator strength)
		//		chance of working, in addition to #2
		float generator_fraction = ssip->current_hits / ssip->total_hits;

		if (generator_fraction > MIN_SHIELDS_FOR_FULL_STRENGTH)
		{
			return objp->shield_quadrant[quadrant_num];
		}
		else if (generator_fraction > MIN_SHIELDS_FOR_FULL_COVERAGE)
		{
			return scale_quad(generator_fraction, objp->shield_quadrant[quadrant_num]);
		}
		else
		{
			// randomize according to this object and the current time
			// (Missiontime >> 13 is eighths of a second) 
			float rand_num = static_randf(OBJ_INDEX(objp) ^ (Missiontime >> 13));

			// maybe flicker the shield
			if (rand_num < sqrt(generator_fraction))
				return scale_quad(generator_fraction, objp->shield_quadrant[quadrant_num]);
			else
				return 0.0f;
		}
	}
	// no shield generator, so behave as normal
	else
	*/
		return objp->shield_quadrant[quadrant_num];
}

// Goober5000
void shield_set_quad(object *objp, int quadrant_num, float strength)
{
	// check array bounds
	Assert(quadrant_num >= 0 && quadrant_num < objp->n_quadrants);
	if (quadrant_num < 0 || quadrant_num >= objp->n_quadrants)
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
	Assert(quadrant_num >= 0 && quadrant_num < objp->n_quadrants);
	if (quadrant_num < 0 || quadrant_num >= objp->n_quadrants)
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
	if (objp->type != OBJ_SHIP && objp->type != OBJ_START)
		return 0.0f;

	return Ships[objp->instance].ship_max_shield_strength;
}

void shield_set_max_strength(object *objp, float newmax)
{
	if(objp->type != OBJ_SHIP)
		return;

	Ships[objp->instance].ship_max_shield_strength = newmax;
}

// Goober5000
float shield_get_max_quad(object *objp)
{
	return shield_get_max_strength(objp) / objp->n_quadrants;
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
	Assert(quadrant_num >= 0 && quadrant_num < objp->n_quadrants);
	if ((quadrant_num < 0) || (quadrant_num >= objp->n_quadrants))
		return damage;	
	
	if (objp->type != OBJ_SHIP && objp->type != OBJ_START)
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
	if ((quadrant_num >= 0) && (quadrant_num < objp->n_quadrants))
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

		if (strength > MAX(2.0f * objp->n_quadrants, 0.1f * shield_get_max_strength(objp)))
			return 1;
	}

	return 0;	// no shield strength
}

