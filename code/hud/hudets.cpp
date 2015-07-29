/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "hud/hudets.h"
#include "hud/hud.h"
#include "ship/ship.h"
#include "freespace2/freespace.h"
#include "io/timer.h"
#include "gamesnd/gamesnd.h"
#include "weapon/emp.h"
#include "localization/localize.h"
#include "weapon/weapon.h"
#include "globalincs/systemvars.h"
#include "object/object.h"
#include "object/objectshield.h"
#include "ship/subsysdamage.h"

float Energy_levels[NUM_ENERGY_LEVELS] = {0.0f,  0.0833f, 0.167f, 0.25f, 0.333f, 0.417f, 0.5f, 0.583f, 0.667f, 0.75f, 0.833f, 0.9167f, 1.0f};
int Weapon_energy_cheat = 0;

// -------------------------------------------------------------------------------------------------
// ets_init_ship() is called by a ship when it is created (effectively, for every ship at the start
// of a mission).  This will set the default charge rates for the different systems and initialize
// the weapon energy reserve.
//
void ets_init_ship(object* obj)
{
	ship* sp;

	// fred should bail here
	if(Fred_running){
		return;
	}

	Assert(obj->type == OBJ_SHIP);
	sp = &Ships[obj->instance];
	
	sp->weapon_energy = Ship_info[sp->ship_info_index].max_weapon_reserve;
	if ((sp->flags2 & SF2_NO_ETS) == 0) {
		sp->next_manage_ets = timestamp(AI_MODIFY_ETS_INTERVAL);
	} else {
		sp->next_manage_ets = -1;
	}
	set_default_recharge_rates(obj);
}

// -------------------------------------------------------------------------------------------------
// update_ets() is called once per frame for every OBJ_SHIP in the game.  The amount of energy
// to send to the weapons and shields is calculated, and the top ship speed is calculated.  The
// amount of time elapsed from the previous call is passed in as the parameter fl_frametime.
//
// parameters:   obj          ==> object that is updating their energy system
//               fl_frametime ==> game frametime (in seconds)
//
void update_ets(object* objp, float fl_frametime)
{
	float max_new_shield_energy, max_new_weapon_energy, _ss;

	if ( fl_frametime <= 0 ){
		return;
	}

	ship* ship_p = &Ships[objp->instance];
	ship_info* sinfo_p = &Ship_info[ship_p->ship_info_index];
	float max_g=sinfo_p->max_weapon_reserve,
		  max_s=ship_p->ship_max_shield_strength;

	if ( ship_p->flags & SF_DYING ){
		return;
	}

	if ( sinfo_p->power_output == 0 ){
		return;
	}

//	new_energy = fl_frametime * sinfo_p->power_output;

	// update weapon energy
	max_new_weapon_energy = fl_frametime * sinfo_p->max_weapon_regen_per_second * max_g;
	if ( objp->flags & OF_PLAYER_SHIP ) {
		ship_p->weapon_energy += Energy_levels[ship_p->weapon_recharge_index] * max_new_weapon_energy * The_mission.ai_profile->weapon_energy_scale[Game_skill_level];
	} else {
		ship_p->weapon_energy += Energy_levels[ship_p->weapon_recharge_index] * max_new_weapon_energy;
	}

	if ( ship_p->weapon_energy > sinfo_p->max_weapon_reserve ){
		ship_p->weapon_energy = sinfo_p->max_weapon_reserve;
	}

	float shield_delta;
	max_new_shield_energy = fl_frametime * sinfo_p->max_shield_regen_per_second * max_s;
	if ( objp->flags & OF_PLAYER_SHIP ) {
		shield_delta = Energy_levels[ship_p->shield_recharge_index] * max_new_shield_energy * The_mission.ai_profile->shield_energy_scale[Game_skill_level];
	} else {
		shield_delta = Energy_levels[ship_p->shield_recharge_index] * max_new_shield_energy;
	}

	shield_add_strength(objp, shield_delta);

	if ( (_ss = shield_get_strength(objp)) > ship_p->ship_max_shield_strength ){
		for (int i=0; i<objp->n_quadrants; i++){
			objp->shield_quadrant[i] *= ship_p->ship_max_shield_strength / _ss;
		}
	}

	// calculate the top speed of the ship based on the energy flow to engines
	float y = Energy_levels[ship_p->engine_recharge_index];

	ship_p->current_max_speed = ets_get_max_speed(objp, y);

	// AL 11-15-97: Rules for engine strength affecting max speed:
	//						1. if strength >= 0.5 no affect 
	//						2. if strength < 0.5 then max_speed = sqrt(strength)
	//					 
	//					 This will translate to 71% max speed at 50% engines, and 31% max speed at 10% engines
	//
	float strength = ship_get_subsystem_strength(ship_p, SUBSYSTEM_ENGINE);

	// don't let engine strength affect max speed when playing on lowest skill level
	if ( (objp != Player_obj) || (Game_skill_level > 0) ) {
		if ( strength < SHIP_MIN_ENGINES_FOR_FULL_SPEED ) {
			ship_p->current_max_speed *= fl_sqrt(strength);
		}
	}

	if ( timestamp_elapsed(ship_p->next_manage_ets) ) {
		if ( !(objp->flags & OF_PLAYER_SHIP) ) {
			ai_manage_ets(objp);
			ship_p->next_manage_ets = timestamp(AI_MODIFY_ETS_INTERVAL);
		}
		else {
			if ( Weapon_energy_cheat ){
				ship_p->weapon_energy = sinfo_p->max_weapon_reserve;
			}
		}
	}
}

float ets_get_max_speed(object* objp, float engine_energy)
{
	Assertion(objp != NULL, "Invalid object pointer passed!");
	Assertion(objp->type == OBJ_SHIP, "Object needs to be a ship object!");
	Assertion(engine_energy >= 0.0f && engine_energy <= 1.0f, "Invalid float passed, needs to be in [0, 1], was %f!", engine_energy);

	ship* shipp = &Ships[objp->instance];

	ship_info* sip = &Ship_info[shipp->ship_info_index];

	// check for a shortcuts first before doing linear interpolation
	if ( engine_energy == Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX] ){
		return sip->max_speed;
	} else if ( engine_energy == 0.0f ){
		return 0.5f * sip->max_speed;
	} else if ( engine_energy == 1.0f ){
		return sip->max_overclocked_speed;
	} else {
		// do a linear interpolation to find the current max speed, using points (0,1/2 default_max_speed) (.333,default_max_speed)
		// x = x1 + (y-y1) * (x2-x1) / (y2-y1);
		if ( engine_energy < Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX] ){
			return 0.5f*sip->max_speed + (engine_energy  * (0.5f*sip->max_speed) ) / Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX];
		} else {
			// do a linear interpolation to find the current max speed, using points (.333,default_max_speed) (1,max_overclock_speed)
			return sip->max_speed + (engine_energy - Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX]) * (sip->max_overclocked_speed - sip->max_speed) / (1.0f - Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX]);
		}
	}
}


// -------------------------------------------------------------------------------------------------
// ai_manage_ets() will determine if a ship should modify it's energy transfer percentages, or 
// transfer energy from shields->weapons or from weapons->shields
//

// minimum level rule constants
#define SHIELDS_MIN_LEVEL_PERCENT	0.3f
#define WEAPONS_MIN_LEVEL_PERCENT	0.3f

// maximum level rule constants
#define SHIELDS_MAX_LEVEL_PERCENT	0.8f
#define WEAPONS_MAX_LEVEL_PERCENT	0.8f

// emergency rule constants
#define SHIELDS_EMERG_LEVEL_PERCENT	0.10f
#define WEAPONS_EMERG_LEVEL_PERCENT	0.05f

// need this, or ai's tend to totally eliminate engine power!
#define MIN_ENGINE_RECHARGE_INDEX	3

#define DEFAULT_CHARGE_INDEX			4
#define NORMAL_TOLERANCE_PERCENT		.10f

void ai_manage_ets(object* obj)
{
	ship* ship_p = &Ships[obj->instance];
	ship_info* ship_info_p = &Ship_info[ship_p->ship_info_index];

	if ( ship_info_p->power_output == 0 )
		return;

	if (ship_p->flags & SF_DYING)
		return;

	// check if any of the three systems are not being used.  If so, don't allow energy management.
	if ( !ship_p->ship_max_shield_strength || !ship_info_p->max_speed || !ship_info_p->max_weapon_reserve)
		return;

	float shield_left_percent = get_shield_pct(obj);
	float weapon_left_percent = ship_p->weapon_energy/ship_info_p->max_weapon_reserve;

	// maximum level check
	//	MK, changed these, might as well let them go up to 100% if nothing else needs the recharge ability.
	if ( weapon_left_percent == 1.0f) {
		decrease_recharge_rate(obj, WEAPONS);
	}

	if (!(obj->flags & OF_NO_SHIELDS) && (shield_left_percent == 1.0f)) {
		decrease_recharge_rate(obj, SHIELDS);
	}

	// minimum check

	if (!(obj->flags & OF_NO_SHIELDS) && (shield_left_percent < SHIELDS_MIN_LEVEL_PERCENT)) {
		if ( weapon_left_percent > WEAPONS_MIN_LEVEL_PERCENT )
			increase_recharge_rate(obj, SHIELDS);
	}

	if ( weapon_left_percent < WEAPONS_MIN_LEVEL_PERCENT ) {
		increase_recharge_rate(obj, WEAPONS);
	}

	if ( ship_p->engine_recharge_index < MIN_ENGINE_RECHARGE_INDEX ) {
		increase_recharge_rate(obj, ENGINES);
	}

	// emergency check
	if (!(obj->flags & OF_NO_SHIELDS)) {
		if ( shield_left_percent < SHIELDS_EMERG_LEVEL_PERCENT ) {
			if (ship_p->target_shields_delta == 0.0f)
				transfer_energy_to_shields(obj);
		} else if ( weapon_left_percent < WEAPONS_EMERG_LEVEL_PERCENT ) {
			if ( shield_left_percent > SHIELDS_MIN_LEVEL_PERCENT || weapon_left_percent <= 0.01 )	// dampen ai enthusiasm for sucking energy to weapons
				transfer_energy_to_weapons(obj);
		}

	
		// check for return to normal values
		if ( fl_abs( shield_left_percent - 0.5f ) < NORMAL_TOLERANCE_PERCENT ) {
			if ( ship_p->shield_recharge_index > DEFAULT_CHARGE_INDEX )
				decrease_recharge_rate(obj, SHIELDS);
			else if ( ship_p->shield_recharge_index < DEFAULT_CHARGE_INDEX )
				increase_recharge_rate(obj, SHIELDS);
		}
	}


	if ( fl_abs( weapon_left_percent - 0.5f ) < NORMAL_TOLERANCE_PERCENT ) {
		if ( ship_p->weapon_recharge_index > DEFAULT_CHARGE_INDEX )
			decrease_recharge_rate(obj, WEAPONS);
		else if ( ship_p->weapon_recharge_index < DEFAULT_CHARGE_INDEX )
			increase_recharge_rate(obj, WEAPONS);
	}
}

// -------------------------------------------------------------------------------------------------
// set_default_recharge_rates() will set the charge levels for the weapons, shields and
// engines to their default levels
void set_default_recharge_rates(object* obj)
{
	int ship_properties;

	ship* ship_p = &Ships[obj->instance];
	ship_info* ship_info_p = &Ship_info[ship_p->ship_info_index];

	if ( ship_info_p->power_output == 0 )
		return;

	ship_properties = 0;	
	if (ship_has_energy_weapons(ship_p))
		ship_properties |= HAS_WEAPONS;
	
	if (!(obj->flags & OF_NO_SHIELDS))
		ship_properties |= HAS_SHIELDS;

	if (ship_has_engine_power(ship_p))
		ship_properties |= HAS_ENGINES;

	// the default charge rate depends on what systems are on each ship
	switch ( ship_properties ) {
		case HAS_ENGINES | HAS_WEAPONS | HAS_SHIELDS:
			ship_p->shield_recharge_index = INTIAL_SHIELD_RECHARGE_INDEX;
			ship_p->weapon_recharge_index = INTIAL_WEAPON_RECHARGE_INDEX;
			ship_p->engine_recharge_index = INTIAL_ENGINE_RECHARGE_INDEX;
			break;

		case HAS_ENGINES | HAS_SHIELDS:
			ship_p->shield_recharge_index = ONE_HALF_INDEX;
			ship_p->weapon_recharge_index = ZERO_INDEX;
			ship_p->engine_recharge_index = ONE_HALF_INDEX;
			break;

		case HAS_WEAPONS | HAS_SHIELDS:
			ship_p->shield_recharge_index = ONE_HALF_INDEX;
			ship_p->weapon_recharge_index = ONE_HALF_INDEX;
			ship_p->engine_recharge_index = ZERO_INDEX;
			break;

		case HAS_ENGINES | HAS_WEAPONS:
			ship_p->shield_recharge_index = ZERO_INDEX;
			ship_p->weapon_recharge_index = ONE_HALF_INDEX;
			ship_p->engine_recharge_index = ONE_HALF_INDEX;
			break;

		case HAS_SHIELDS:
			ship_p->shield_recharge_index = ALL_INDEX;
			ship_p->weapon_recharge_index = ZERO_INDEX;
			ship_p->engine_recharge_index = ZERO_INDEX;
			break;

		case HAS_ENGINES:
			ship_p->shield_recharge_index = ZERO_INDEX;
			ship_p->weapon_recharge_index = ZERO_INDEX;
			ship_p->engine_recharge_index = ALL_INDEX;
			break;

		case HAS_WEAPONS:
			ship_p->shield_recharge_index = ZERO_INDEX;
			ship_p->weapon_recharge_index = ALL_INDEX;
			ship_p->engine_recharge_index = ZERO_INDEX;
			break;

		default:
			Int3();	// if no systems, power output should be zero, and this funtion shouldn't be called
			break;
	} // end switch
}

// -------------------------------------------------------------------------------------------------
// increase_recharge_rate() will increase the energy flow to the specified system (one of
// WEAPONS, SHIELDS or ENGINES).  The increase in energy will result in a decrease to
// the other two systems.
void increase_recharge_rate(object* obj, SYSTEM_TYPE ship_system) 
{
	int	*gain_index=NULL, *lose_index1=NULL, *lose_index2=NULL, *tmp=NULL;
	int	count=0;
	ship	*ship_p = &Ships[obj->instance];

	if (ship_p->flags2 & SF2_NO_ETS)
		return;

	switch ( ship_system ) {
		case WEAPONS:
			if ( !ship_has_energy_weapons(ship_p) )
				return;

			gain_index = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_SHIELDS )
				lose_index1 = NULL;
			else
				lose_index1 = &ship_p->shield_recharge_index;

			if ( !ship_has_engine_power(ship_p) )
				lose_index2 = NULL;
			else
				lose_index2 = &ship_p->engine_recharge_index;

			break;

		case SHIELDS:
			if ( obj->flags & OF_NO_SHIELDS )
				return;

			gain_index = &ship_p->shield_recharge_index;

			if ( !ship_has_energy_weapons(ship_p) )
				lose_index1 = NULL;
			else
				lose_index1 = &ship_p->weapon_recharge_index;

			if ( !ship_has_engine_power(ship_p) )
				lose_index2 = NULL;
			else
				lose_index2 = &ship_p->engine_recharge_index;

			break;

		case ENGINES:
			if ( !ship_has_engine_power(ship_p) )
				return;

			gain_index = &ship_p->engine_recharge_index;

			if ( !ship_has_energy_weapons(ship_p) )
				lose_index1 = NULL;
			else
				lose_index1 = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_SHIELDS )
				lose_index2 = NULL;
			else
				lose_index2 = &ship_p->shield_recharge_index;

			break;

	} // end switch

	// return if we can't transfer energy
	if (!lose_index1 && !lose_index2)
		return;

	// already full, nothing to do 
	count = MAX_ENERGY_INDEX - *gain_index;
	if ( count > 2 ) 
		count = 2;

	if ( count <= 0 )
	{
		if ( obj == Player_obj )
		{
			snd_play( &Snds[SND_ENERGY_TRANS_FAIL], 0.0f );
		}
		return;
	}

	*gain_index += count;

	// ensure that the highest lose index takes the first decrease
	if ( lose_index1 && lose_index2 ) {
		if ( *lose_index1 < *lose_index2 ) {
			tmp = lose_index1;
			lose_index1 = lose_index2;
			lose_index2 = tmp;
		}
	}

	int sanity = 0;
	while(count > 0) {
		if ( lose_index1 && *lose_index1 > 0 ) {
			*lose_index1 -= 1;
			count--;
		}

		if ( count <= 0 ) 
			break;

		if ( lose_index2 && *lose_index2 > 0 ) {
			*lose_index2 -= 1;
			count--;
		}

		if ( sanity++ > 10 ) {
			Int3();	// get Alan
			break;
		}
	}

	if ( obj == Player_obj )
		snd_play( &Snds[SND_ENERGY_TRANS], 0.0f );
}

// -------------------------------------------------------------------------------------------------
// decrease_recharge_rate() will decrease the energy flow to the specified system (one of
// WEAPONS, SHIELDS or ENGINES).  The decrease in energy will result in an increase to
// the other two systems.
void decrease_recharge_rate(object* obj, SYSTEM_TYPE ship_system) 
{
	int	*lose_index=NULL, *gain_index1=NULL, *gain_index2=NULL, *tmp=NULL;
	int	count;
	ship	*ship_p = &Ships[obj->instance];

	if (ship_p->flags2 & SF2_NO_ETS)
		return;

	switch ( ship_system ) {
		case WEAPONS:
			if ( !ship_has_energy_weapons(ship_p) )
				return;

			lose_index = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_SHIELDS )
				gain_index1 = NULL;
			else
				gain_index1 = &ship_p->shield_recharge_index;

			if ( !ship_has_engine_power(ship_p) )
				gain_index2 = NULL;
			else
				gain_index2 = &ship_p->engine_recharge_index;

			break;

		case SHIELDS:
			if ( obj->flags & OF_NO_SHIELDS )
				return;

			lose_index = &ship_p->shield_recharge_index;

			if ( !ship_has_energy_weapons(ship_p) )
				gain_index1 = NULL;
			else
				gain_index1 = &ship_p->weapon_recharge_index;

			if ( !ship_has_engine_power(ship_p) )
				gain_index2 = NULL;
			else
				gain_index2 = &ship_p->engine_recharge_index;

			break;

		case ENGINES:
			if ( !ship_has_engine_power(ship_p) )
				return;

			lose_index = &ship_p->engine_recharge_index;

			if ( !ship_has_energy_weapons(ship_p) )
				gain_index1 = NULL;
			else
				gain_index1 = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_SHIELDS )
				gain_index2 = NULL;
			else
				gain_index2 = &ship_p->shield_recharge_index;

			break;

	} // end switch

	// return if we can't transfer energy
	if (!gain_index1 && !gain_index2)
		return;

	// check how much there is to lose
	count = MIN(2, *lose_index);
	if ( count <= 0 ) {
		if ( obj == Player_obj ) {
			snd_play( &Snds[SND_ENERGY_TRANS_FAIL], 0.0f );
		}
		return;
	}

	*lose_index -= count;

	// make sure that the gain starts with the system which needs it most
	if ( gain_index1 && gain_index2 ) {
		if ( *gain_index1 > *gain_index2 ) {
			tmp = gain_index1;
			gain_index1 = gain_index2;
			gain_index2 = tmp;
		}
	}

	int sanity=0;
	while(count > 0) {
		if ( gain_index1 && *gain_index1 < MAX_ENERGY_INDEX ) {
			*gain_index1 += 1;
			count--;
		}

		if ( count <= 0 ) 
			break;

		if ( gain_index2 && *gain_index2 < MAX_ENERGY_INDEX ) {
			*gain_index2 += 1;
			count--;
		}

		if ( sanity++ > 10 ) {
			Int3();	// get Alan
			break;
		}
	}

	if ( obj == Player_obj )
		snd_play( &Snds[SND_ENERGY_TRANS], 0.0f );
}

void transfer_energy_weapon_common(object *objp, float from_field, float to_field, float *from_delta, float *to_delta, float max, float scale)
{
	float	delta;

	delta = from_field * ENERGY_DIVERT_DELTA * scale;

	if (to_field + *to_delta + delta > max)
		delta = max - to_field - *to_delta;

	if ( delta > 0 ) {
		if ( objp == Player_obj )
			snd_play( &Snds[SND_ENERGY_TRANS], 0.0f );

		if (delta > from_field)
			delta = from_field;

		*to_delta += delta;
		*from_delta -= delta;
	} else
		if ( objp == Player_obj )
			snd_play( &Snds[SND_ENERGY_TRANS_FAIL], 0.0f );
}

// -------------------------------------------------------------------------------------------------
// transfer_energy_to_shields() will transfer ENERGY_DIVERT_DELTA percent of weapon energy
// to shield energy.
void transfer_energy_to_shields(object* obj)
{
	ship*			ship_p = &Ships[obj->instance];

	if (ship_p->flags & SF_DYING)
		return;

	if ( !ship_has_energy_weapons(ship_p) || obj->flags & OF_NO_SHIELDS )
	{
		return;
	}

	transfer_energy_weapon_common(obj, ship_p->weapon_energy, shield_get_strength(obj), &ship_p->target_weapon_energy_delta, &ship_p->target_shields_delta, ship_p->ship_max_shield_strength, 0.5f);
}

// -------------------------------------------------------------------------------------------------
// transfer_energy_to_weapons() will transfer ENERGY_DIVERT_DELTA percent of shield energy
// to weapon energy.
void transfer_energy_to_weapons(object* obj)
{
	ship*			ship_p = &Ships[obj->instance];
	ship_info*	sinfo_p = &Ship_info[ship_p->ship_info_index];

	if (ship_p->flags & SF_DYING)
		return;

	if ( !ship_has_energy_weapons(ship_p) || obj->flags & OF_NO_SHIELDS )
	{
		return;
	}

	transfer_energy_weapon_common(obj, shield_get_strength(obj), ship_p->weapon_energy, &ship_p->target_shields_delta, &ship_p->target_weapon_energy_delta, sinfo_p->max_weapon_reserve, 1.0f);
}

/**
 * decrease one ets index to zero & adjust others up
 */
void zero_one_ets (int *reduce, int *add1, int *add2)
{
	int *tmp;
	// add to the smallest index 1st
	if (*add1 > *add2) {
		tmp = add1;
		add1 = add2;
		add2 = tmp;
	}
	while (*reduce > ZERO_INDEX) {
		if (*add1 < ALL_INDEX) {
			++*add1;
			--*reduce;
		}

		if (*reduce <= ZERO_INDEX) {
			break;
		}

		if (*add2 < ALL_INDEX) {
			++*add2;
			--*reduce;
		}
	}
}

/**
 * ensure input ETS indexs are valid.
 * If not, "fix" them by moving outliers towards the middle index
 */
 void sanity_check_ets_inputs(int (&ets_indexes)[num_retail_ets_gauges])
 {
	int i;
	int ets_delta = MAX_ENERGY_INDEX - ets_indexes[ENGINES] - ets_indexes[SHIELDS] - ets_indexes[WEAPONS];
	if ( ets_delta != 0 ) {
		if ( ets_delta > 0) { // add to lowest indexes
			while ( ets_delta != 0 ) {
				int lowest_val = MAX_ENERGY_INDEX;
				int lowest_idx = 0;

				for (i = 0; i < num_retail_ets_gauges; ++i) {
					if (ets_indexes[i] <= lowest_val ) {
						lowest_val = ets_indexes[i];
						lowest_idx = i;
					}
				}
				++ets_indexes[lowest_idx];
				--ets_delta;
			}
		} else { // remove from highest indexes
			while ( ets_delta != 0 ) {
				int highest_val = 0;
				int highest_idx = 0;

				for (i = 0; i < num_retail_ets_gauges; ++i) {
					if (ets_indexes[i] >= highest_val ) {
						highest_val = ets_indexes[i];
						highest_idx = i;
					}
				}
				--ets_indexes[highest_idx];
				++ets_delta;
			}
		}
	}
 }

 /**
  * adjust input ETS indexes to handle missing systems on the target ship
  * return true if indexes are valid to be set
  */
bool validate_ship_ets_indxes(const int &ship_idx, int (&ets_indexes)[num_retail_ets_gauges])
{
	if (ship_idx < 0) {
		return false;
	}
	if (Ships[ship_idx].objnum < 0) {
		return false;
	}
	ship *ship_p = &Ships[ship_idx];

	if (ship_p->flags2 & SF2_NO_ETS)
		return false;

	// handle ships that are missing parts of the ETS
	int ship_properties = 0;
	if (ship_has_energy_weapons(ship_p)) {
		ship_properties |= HAS_WEAPONS;
	}

	if (!(Objects[ship_p->objnum].flags & OF_NO_SHIELDS)) {
		ship_properties |= HAS_SHIELDS;
	}

	if (ship_has_engine_power(ship_p)) {
		ship_properties |= HAS_ENGINES;
	}

	switch ( ship_properties ) {
		case HAS_ENGINES | HAS_WEAPONS | HAS_SHIELDS:
			// all present, don't change ets indexes
			break;

		case HAS_ENGINES | HAS_SHIELDS:
			zero_one_ets(&ets_indexes[WEAPONS], &ets_indexes[ENGINES], &ets_indexes[SHIELDS]);
			break;

		case HAS_WEAPONS | HAS_SHIELDS:
			zero_one_ets(&ets_indexes[ENGINES], &ets_indexes[SHIELDS], &ets_indexes[WEAPONS]);
			break;

		case HAS_ENGINES | HAS_WEAPONS:
			zero_one_ets(&ets_indexes[SHIELDS], &ets_indexes[ENGINES], &ets_indexes[WEAPONS]);
			break;

		case HAS_ENGINES:
		case HAS_SHIELDS:
		case HAS_WEAPONS:
			// can't change anything if only one is active on this ship
			return false;
			break;

		default:
			Error(LOCATION, "Encountered a ship (%s) with a broken ETS", ship_p->ship_name);
			break;
	}
	return true;
}

HudGaugeEts::HudGaugeEts():
HudGauge(HUD_OBJECT_ETS_ENGINES, HUD_ETS_GAUGE, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255),
System_type(0)
{
}

HudGaugeEts::HudGaugeEts(int _gauge_object, int _system_type):
HudGauge(_gauge_object, HUD_ETS_GAUGE, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255),
System_type(_system_type)
{
}

void HudGaugeEts::initBarHeight(int _ets_h)
{
	ETS_bar_h = _ets_h;
}

void HudGaugeEts::initLetterOffsets(int _x, int _y)
{
	Letter_offsets[0] = _x;
	Letter_offsets[1] = _y;
}

void HudGaugeEts::initTopOffsets(int _x, int _y)
{
	Top_offsets[0] = _x;
	Top_offsets[1] = _y;
}

void HudGaugeEts::initBottomOffsets(int _x, int _y)
{
	Bottom_offsets[0] = _x;
	Bottom_offsets[1] = _y;
}

void HudGaugeEts::initLetter(char _letter)
{
	Letter = _letter;
}

void HudGaugeEts::initBitmaps(char *fname)
{
	Ets_bar.first_frame = bm_load_animation(fname, &Ets_bar.num_frames);
	if ( Ets_bar.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeEts::render(float frametime)
{
}

void HudGaugeEts::pageIn()
{
	bm_page_in_aabitmap( Ets_bar.first_frame, Ets_bar.num_frames );
}

/**
 * Draw one ETS bar to screen
 */
void HudGaugeEts::blitGauge(int index)
{
	int y_start, y_end, clip_h, w, h, x, y;

	clip_h = fl2i( (1 - Energy_levels[index]) * ETS_bar_h );

	bm_get_info(Ets_bar.first_frame,&w,&h);

	if ( index < NUM_ENERGY_LEVELS-1 ) {
		// some portion of dark needs to be drawn

		setGaugeColor();

		// draw the top portion
		x = position[0] + Top_offsets[0];
		y = position[1] + Top_offsets[1];
		
		renderBitmapEx(Ets_bar.first_frame,x,y,w,clip_h,0,0);			

		// draw the bottom portion
		x = position[0] + Bottom_offsets[0];
		y = position[1] + Bottom_offsets[1];

		y_start = y + (ETS_bar_h - clip_h);
		y_end = y + ETS_bar_h;
		
		renderBitmapEx(Ets_bar.first_frame, x, y_start, w, y_end-y_start, 0, ETS_bar_h-clip_h);			
	}

	if ( index > 0 ) {
		if ( maybeFlashSexp() == 1 ) {
			setGaugeColor(HUD_C_DIM);
			// hud_set_dim_color();
		} else {
			setGaugeColor(HUD_C_BRIGHT);
			// hud_set_bright_color();
		}
		// some portion of recharge needs to be drawn

		// draw the top portion
		x = position[0] + Top_offsets[0];
		y = position[1] + Top_offsets[1];

		y_start = y + clip_h;
		y_end = y + ETS_bar_h;
		
		renderBitmapEx(Ets_bar.first_frame+1, x, y_start, w, y_end-y_start, 0, clip_h);			

		// draw the bottom portion
		x = position[0] + Bottom_offsets[0];
		y = position[1] + Bottom_offsets[1];
		
		renderBitmapEx(Ets_bar.first_frame+2, x,y,w,ETS_bar_h-clip_h,0,0);			
	}
}

/**
 * Default ctor for retail ETS gauge
 * 2nd arg (0) is not used
 */
HudGaugeEtsRetail::HudGaugeEtsRetail():
HudGaugeEts(HUD_OBJECT_ETS_RETAIL, 0)
{
}

/**
 * Render the ETS retail gauge to the screen (weapon+shield+engine)
 */
void HudGaugeEtsRetail::render(float frametime)
{
	int i;
	int initial_position;

	ship* ship_p = &Ships[Player_obj->instance];

	if ( Ets_bar.first_frame < 0 ) {
		return;
	}

	// if at least two gauges are not shown, don't show any
	i = 0;
	if (!ship_has_energy_weapons(ship_p)) i++;
	if (Player_obj->flags & OF_NO_SHIELDS) i++;
	if (!ship_has_engine_power(ship_p)) i++;
	if (i >= 2) return;

	setGaugeColor();

	// draw the letters for the gauges first, before any clipping occurs
	// skip letter for any missing gauges (max one, see check above)
	initial_position = 0;
	if (ship_has_energy_weapons(ship_p)) {
		Letter = Letters[0];
		position[0] = Gauge_positions[initial_position++];
		renderPrintf(position[0] + Letter_offsets[0], position[1] + Letter_offsets[1], NOX("%c"), Letter);
	}
	if (!(Player_obj->flags & OF_NO_SHIELDS)) {
		Letter = Letters[1];
		position[0] = Gauge_positions[initial_position++];
		renderPrintf(position[0] + Letter_offsets[0], position[1] + Letter_offsets[1], NOX("%c"), Letter);
	}
	if (ship_has_engine_power(ship_p)) {
		Letter = Letters[2];
		position[0] = Gauge_positions[initial_position++];
		renderPrintf(position[0] + Letter_offsets[0], position[1] + Letter_offsets[1], NOX("%c"), Letter);
	}

	// draw gauges, skipping any gauge that is missing
	initial_position = 0;
	if (ship_has_energy_weapons(ship_p)) {
		Letter = Letters[0];
		position[0] = Gauge_positions[initial_position++];
		blitGauge(ship_p->weapon_recharge_index);
	}
	if (!(Player_obj->flags & OF_NO_SHIELDS)) {
		Letter = Letters[1];
		position[0] = Gauge_positions[initial_position++];
		blitGauge(ship_p->shield_recharge_index);
	}
	if (ship_has_engine_power(ship_p)) {
		Letter = Letters[2];
		position[0] = Gauge_positions[initial_position++];
		blitGauge(ship_p->engine_recharge_index);
	}
}

/**
 * Set ETS letters for retail ETS gauge
 * Allows for different languages to be used in the hud
 */
void HudGaugeEtsRetail::initLetters(char *_letters)
{
	int i;
	for ( i = 0; i < num_retail_ets_gauges; ++i)
		Letters[i] = _letters[i];
}

/**
 * Set the three possible positions for ETS bars
 */
void HudGaugeEtsRetail::initGaugePositions(int *_gauge_positions)
{
	int i;
	for ( i = 0; i < num_retail_ets_gauges; ++i)
		Gauge_positions[i] = _gauge_positions[i];
}

HudGaugeEtsWeapons::HudGaugeEtsWeapons():
HudGaugeEts(HUD_OBJECT_ETS_WEAPONS, (int)WEAPONS)
{
}

void HudGaugeEtsWeapons::render(float frametime)
{
	int i;

	ship* ship_p = &Ships[Player_obj->instance];	

	if ( Ets_bar.first_frame < 0 ) {
		return;
	}

	// if at least two gauges are not shown, don't show any
	i = 0;
	if (!ship_has_energy_weapons(ship_p)) i++;
	if (Player_obj->flags & OF_NO_SHIELDS) i++;
	if (!ship_has_engine_power(ship_p)) i++;
	if (i >= 2) return;

	// no weapon energy, no weapon gauge
	if (!ship_has_energy_weapons(ship_p))
	{
		return;
	}

	setGaugeColor();

	// draw the letters for the gauge first, before any clipping occurs
	renderPrintf(position[0] + Letter_offsets[0], position[1] + Letter_offsets[1], NOX("%c"), Letter);

	// draw the gauges for the weapon system
	blitGauge(ship_p->weapon_recharge_index);
}

HudGaugeEtsShields::HudGaugeEtsShields():
HudGaugeEts(HUD_OBJECT_ETS_SHIELDS, (int)SHIELDS)
{
}

void HudGaugeEtsShields::render(float frametime)
{
	int i;

	ship* ship_p = &Ships[Player_obj->instance];	

	if ( Ets_bar.first_frame < 0 ) {
		return;
	}

	// if at least two gauges are not shown, don't show any
	i = 0;
	if (!ship_has_energy_weapons(ship_p)) i++;
	if (Player_obj->flags & OF_NO_SHIELDS) i++;
	if (!ship_has_engine_power(ship_p)) i++;
	if (i >= 2) return;

	// no shields, no shields gauge
	if (Player_obj->flags & OF_NO_SHIELDS) {
		return;
	}

	setGaugeColor();

	// draw the letters for the gauge first, before any clipping occurs
	renderPrintf(position[0] + Letter_offsets[0], position[1] + Letter_offsets[1], NOX("%c"), Letter);

	// draw the gauge for the shield system
	blitGauge(ship_p->shield_recharge_index);
}

HudGaugeEtsEngines::HudGaugeEtsEngines():
HudGaugeEts(HUD_OBJECT_ETS_ENGINES, (int)ENGINES)
{
}

void HudGaugeEtsEngines::render(float frametime)
{
	int i;

	ship* ship_p = &Ships[Player_obj->instance];	

	if ( Ets_bar.first_frame < 0 ) {
		return;
	}

	// if at least two gauges are not shown, don't show any
	i = 0;
	if (!ship_has_energy_weapons(ship_p)) i++;
	if (Player_obj->flags & OF_NO_SHIELDS) i++;
	if (!ship_has_engine_power(ship_p)) i++;
	if (i >= 2) return;

	// no engines, no engine gauge
	if (!ship_has_engine_power(ship_p)) {
		return;
	}

	setGaugeColor();

	// draw the letters for the gauge first, before any clipping occurs
	renderPrintf(position[0] + Letter_offsets[0], position[1] + Letter_offsets[1], NOX("%c"), Letter);

	// draw the gauge for the engine system
	blitGauge(ship_p->engine_recharge_index);
}
