/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 */ 



#include "math/staticrand.h"
#include "network/multi.h"
#include "object/objectshield.h"
#include "ship/ship.h"
#include "ship/subsysdamage.h"

#include <climits>

// Private variables
static const float shield_scale_factor = static_cast<float>(1.0 / (log(50.0) - log(1.0)));	// Factor used in Goober5000's scale_quad

// Private Function Declarations
/**
 * @brief Logarithmically scales a quadrant's strength according to the shield generator's health %
 *
 * @param[in] generator_fraction The shield generator's HP in %
 * @param[in] quad_strength      The quadrant's strength, before scaling
 *
 * @returns The quadrant's effective strength.
 *
 * @author Goober5000
 */
float scale_quad(float generator_fraction, float quad_strength);

// Function definitions
float scale_quad(float generator_fraction, float quad_strength) {
	// the following formula makes a nice logarithmic curve between 1 and 50,
	// when x goes from 0 to 100:
	//
	// ln(x) * (100 - 0)
	// -----------------
	//  ln(50) - ln(1)
	//
	float effective_strength = quad_strength * (static_cast<float>(log(generator_fraction)) * shield_scale_factor);

	// ensure not negative, which may happen if the shield gets below 1 percent
	// (since we're dealing with logs)
	if (effective_strength < 0.0f)
		return 0.0f;
	else
		return effective_strength;
}

void shield_add_quad(object *objp, int quadrant_num, float delta) {
	Assert(objp);

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

void shield_add_strength(object *objp, float delta) {
	Assert(objp);

	// if we aren't going to change anything anyway then just bail
	if (delta == 0.0f)
		return;

	float shield_str = shield_get_strength(objp);
	float shield_recharge_limit = shield_get_max_strength(objp);

	if ((delta > 0.0f) && (shield_str >= shield_recharge_limit))
		return;

	if (!(Ai_info[Ships[objp->instance].ai_index].ai_profile_flags[AI::Profile_Flags::Smart_shield_management])
		|| delta <= 0.0f) //SUSHI: We don't want smart shield management for negative delta
	{
		// set the limit for the shield recharge
		if ((delta > 0.0f) && ((shield_str + delta) > shield_recharge_limit))
			delta = shield_recharge_limit - shield_str;

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

			// set the limit for the shield recharge
			if ((delta > 0.0f) && ((shield_str + delta) > shield_recharge_limit))
				delta = shield_recharge_limit - shield_str;

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

// strengthens the weakest quadrant first, then spreads it out equally
void shield_apply_healing(object* objp, float healing) {
	Assert(objp);
	if (objp == nullptr)
		return;

	if (MULTIPLAYER_CLIENT)
		return;

	if (objp->type != OBJ_SHIP && objp->type != OBJ_START)
		return;

	// find the current strongest and weakest shield quads
	float min_shield = objp->shield_quadrant[0];
	int min_shield_index = 0;
	float max_shield = objp->shield_quadrant[0];
	for (int i = 0; i < objp->n_quadrants; i++) {
		if (objp->shield_quadrant[i] < min_shield) {
			min_shield = objp->shield_quadrant[i];
			min_shield_index = i;
		}
		if (objp->shield_quadrant[i] > max_shield)
			max_shield = objp->shield_quadrant[i];
	}

	// if the shields are approximately equal give to all quads equally
	if (max_shield - min_shield < shield_get_max_strength(objp) * 0.1f) {
		for (int i = 0; i < objp->n_quadrants; i++)
			shield_add_quad(objp, i, healing / objp->n_quadrants);
	} else { // else give to weakest
		shield_add_quad(objp, min_shield_index, healing);
	}
}

float shield_apply_damage(object *objp, int quadrant_num, float damage) {
	float remaining_damage;

	Assert(objp);

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
	if (remaining_damage > 0.0f) {
		shield_set_quad(objp, quadrant_num, 0.0f);
		return remaining_damage;
	} else {
		shield_add_quad(objp, quadrant_num, -damage);
		return 0.0f;
	}
}

void shield_balance(object *objp, float rate, float penalty) {
	Assert(objp);
	if (objp->flags[Object::Object_Flags::No_shields]) {
		// No shields, bail
		return;
	}

	float shield_hp;
	shield_hp = shield_get_strength(objp);
	if (shield_hp == 0.0f) {
		// Shields are down, bail
		return;

	} else if (shield_hp == Ships[objp->instance].ship_max_shield_strength) {
		// Shields are maxed, bail
		return;
	}

	// Are all quadrants equal?
	bool all_equal = true;
	for (int idx = 0; idx < objp->n_quadrants - 1; idx++) {
		if (objp->shield_quadrant[idx] != objp->shield_quadrant[idx + 1]) {
			all_equal = false;
			break;
		}
	}

	if (all_equal) {
		// Quadrants are equal, bail
		return;
	}

	Assert((rate > 0.0f) && (rate <= 1.0f));
	Assert((penalty >= 0.0f) && (penalty <= 1.0f));

	float shield_hp_avg = shield_hp / objp->n_quadrants;
	shield_hp_avg *= 1 - penalty;

	for (int i = 0; i < objp->n_quadrants; ++i) {
		if (fabsf(objp->shield_quadrant[i] - shield_hp_avg) < 0.01f) {
			// Very close, so clamp
			objp->shield_quadrant[i] = shield_hp_avg;

		} else {
			// Else, smoothly balance towards target
			objp->shield_quadrant[i] += rate * (shield_hp_avg - objp->shield_quadrant[i]);
		}
	}
}

float shield_get_max_quad(object *objp) {
	Assert(objp);

	if (objp->type != OBJ_SHIP) {
		return 0.0f;
	}

	return shield_get_max_strength(objp, true) / objp->n_quadrants;
}

float shield_get_max_strength(object *objp, bool no_msr) {
	Assert(objp);

	if (objp->type != OBJ_SHIP && objp->type != OBJ_START)
		return 0.0f;

	if (no_msr == true)
		return Ships[objp->instance].ship_max_shield_strength;
	else
		return Ships[objp->instance].ship_max_shield_strength * Ships[objp->instance].max_shield_recharge;
}

float shield_get_quad(object *objp, int quadrant_num) {
	Assert(objp);

	// no shield system, no strength!
	if (objp->flags[Object::Object_Flags::No_shields])
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
	}
	// no shield generator, so behave as normal
	else
	*/
		return objp->shield_quadrant[quadrant_num];
}

float shield_get_strength(object *objp)
{
	Assert(objp);

	// no shield system, no strength!
	if (objp->flags[Object::Object_Flags::No_shields])
		return 0.0f;

	int	i;
	float strength = 0.0f;

	for (i = 0; i < objp->n_quadrants; i++)
		strength += shield_get_quad(objp, i);

	return strength;
}

void shield_set_max_strength(object *objp, float newmax) {
	Assert(objp);

	if (objp->type != OBJ_SHIP)
		return;

	Ships[objp->instance].ship_max_shield_strength = newmax;
}

void shield_set_quad(object *objp, int quadrant_num, float strength) {
	Assert(objp);

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

void shield_set_strength(object *objp, float strength)
{
	int i;

	Assert(objp);

	for (i = 0; i < objp->n_quadrants; i++)
		shield_set_quad(objp, i, strength / objp->n_quadrants);
}

void shield_transfer(object *objp, int quadrant, float rate) {
	Assert(objp);
	Assert(objp->type == OBJ_SHIP);

	Assert(quadrant >= 0 && quadrant < objp->n_quadrants);
	Assert((0.0f < rate) && (rate <= 1.0f));

	// The energy to Xfer to the quadrant
	float xfer_amount = shield_get_max_strength(objp) * rate;

	// The max amount of energy a quad can have
	float max_quadrant_val = shield_get_max_quad(objp);

	if ((objp->shield_quadrant[quadrant] + xfer_amount) > max_quadrant_val) {
		xfer_amount = max_quadrant_val - objp->shield_quadrant[quadrant];
	}

	Assert(xfer_amount >= 0);
	if (xfer_amount == 0) {
		// TODO: provide a feedback sound
		return;
	
	} else if (objp == Player_obj) {
		snd_play(gamesnd_get_game_sound(GameSounds::SHIELD_XFER_OK));
	}

	float energy_avail = 0.0f;	// Energy available from the other quadrants that we can transfer

	for (int i = 0; i < objp->n_quadrants; i++) {
		if (i == quadrant)
			continue;
		energy_avail += objp->shield_quadrant[i];
	}

	// Percent energy to take from each quadrant
	float percent_to_take = xfer_amount / energy_avail;

	if (percent_to_take > 1.0f) {
		percent_to_take = 1.0f;
	}

	for (int i = 0; i < objp->n_quadrants; i++) {
		float delta;

		if (i == quadrant) {
			// Don't take energy from our target
			continue;
		}

		delta = percent_to_take * objp->shield_quadrant[i];
		objp->shield_quadrant[i] -= delta;

		Assert(objp->shield_quadrant[i] >= 0);

		objp->shield_quadrant[quadrant] += delta;

		if (objp->shield_quadrant[quadrant] > max_quadrant_val) {
			// Already reached the max quadrant value. Clamp and bail before losing more any energy.
			objp->shield_quadrant[quadrant] = max_quadrant_val;
			break;
		}
	}
}
