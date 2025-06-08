/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "freespace.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/systemvars.h"
#include "hud/hudets.h"
#include "hud/hudmessage.h"
#include "io/timer.h"
#include "localization/localize.h"
#include "object/object.h"
#include "object/objectshield.h"
#include "ship/ship.h"
#include "ship/subsysdamage.h"
#include "weapon/emp.h"
#include "weapon/weapon.h"
#include "globalincs/alphacolors.h"

float Energy_levels[NUM_ENERGY_LEVELS] = {0.0f,  1.0f/12, 2.0f/12, 3.0f/12, 4.0f/12, 5.0f/12, 6.0f/12, 7.0f/12, 8.0f/12, 9.0f/12, 10.0f/12, 11.0f/12, 1.0f};
bool Weapon_energy_cheat = false;

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
	if ((sp->flags[Ship::Ship_Flags::No_ets]) == 0) {
		sp->next_manage_ets = timestamp(AI_MODIFY_ETS_INTERVAL);
	} else {
		sp->next_manage_ets = -1;
	}
	set_default_recharge_rates(obj);
}

int ets_properties(object* objp)
{
	int properties = 0;
	ship* ship_p = &Ships[objp->instance];
	ship_info* ship_info_p = &Ship_info[ship_p->ship_info_index];

	if (ship_has_energy_weapons(ship_p))
		properties |= HAS_WEAPONS;

	if (!(objp->flags[Object::Object_Flags::No_shields]) && !ship_info_p->flags[Ship::Info_Flags::Intrinsic_no_shields])
		properties |= HAS_SHIELDS;

	if (ship_has_engine_power(ship_p))
		properties |= HAS_ENGINES;

	return properties;
}

// returns the energy that should be dedicated towards a single ETS system
// in retail, this is always 1.0
float ets_power_factor(object *objp, bool include_power_output)
{
	auto shipp = &Ships[objp->instance];
	int properties = ets_properties(objp);

	if (The_mission.ai_profile->flags[AI::Profile_Flags::ETS_energy_same_regardless_of_system_presence] && (properties != (HAS_WEAPONS | HAS_SHIELDS | HAS_ENGINES)))
	{
		// in retail, the effect of having a missing system (e.g. an unshielded ship) is as if all that energy were redirected to other systems, so take the inverse of that
		constexpr float missing_single_factor = 2.0f/3;
		constexpr float missing_double_factor = 1.0f/3;

		// if the properties are *equal* to just one, then it's missing double; otherwise we ruled out the all-three case so it has two and it's missing single
		float missing_factor = (properties == HAS_WEAPONS || properties == HAS_SHIELDS || properties == HAS_ENGINES) ? missing_double_factor : missing_single_factor;

		if (The_mission.ai_profile->flags[AI::Profile_Flags::ETS_uses_power_output] && include_power_output)
			return Ship_info[shipp->ship_info_index].power_output * missing_factor;
		else
			return missing_factor;
	}
	else
	{
		if (The_mission.ai_profile->flags[AI::Profile_Flags::ETS_uses_power_output] && include_power_output)
			return Ship_info[shipp->ship_info_index].power_output;
		else
			return 1.0f;
	}
}

// -------------------------------------------------------------------------------------------------
// update_ets() is called once per frame for every OBJ_SHIP in the game.  
// The amount of energy to send to the weapons and shields is calculated.
// The max speed is also updated if there is a change in aggregate engine health.
// The amount of time elapsed from the previous call is passed in as the parameter fl_frametime.
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
	float max_g=sinfo_p->max_weapon_reserve;

	if ( ship_p->flags[Ship::Ship_Flags::Dying] ){
		return;
	}

	if ( sinfo_p->power_output == 0 ){
		return;
	}

	// See?  Volition did, at one point, intend for power output to affect ETS!
	//	new_energy = fl_frametime * sinfo_p->power_output;

	// update weapon energy
	max_new_weapon_energy = fl_frametime * ship_p->max_weapon_regen_per_second * max_g;
	if ( objp->flags[Object::Object_Flags::Player_ship] ) {
		ship_p->weapon_energy += ets_power_factor(objp) * Energy_levels[ship_p->weapon_recharge_index] * max_new_weapon_energy * The_mission.ai_profile->weapon_energy_scale[Game_skill_level];
	} else {
		ship_p->weapon_energy += ets_power_factor(objp) * Energy_levels[ship_p->weapon_recharge_index] * max_new_weapon_energy;
	}

	if ( ship_p->weapon_energy > sinfo_p->max_weapon_reserve ){
		ship_p->weapon_energy = sinfo_p->max_weapon_reserve;
	}

	float shield_delta;
	max_new_shield_energy = fl_frametime * ship_p->max_shield_regen_per_second * shield_get_max_strength(ship_p, true); // recharge rate is unaffected by $Max Shield Recharge
	if ( objp->flags[Object::Object_Flags::Player_ship] ) {
		shield_delta = ets_power_factor(objp) * Energy_levels[ship_p->shield_recharge_index] * max_new_shield_energy * The_mission.ai_profile->shield_energy_scale[Game_skill_level];
	} else {
		shield_delta = ets_power_factor(objp) * Energy_levels[ship_p->shield_recharge_index] * max_new_shield_energy;
	}

	if (Missiontime - Ai_info[ship_p->ai_index].last_hit_time < fl2f(sinfo_p->shield_regen_hit_delay))
		shield_delta = 0.0f;

	shield_add_strength(objp, shield_delta);

	// if strength now exceeds max, scale back segments proportionally
	float max_shield = shield_get_max_strength(ship_p);
	if ( (_ss = shield_get_strength(objp)) > max_shield ){
		for (auto &quad: objp->shield_quadrant) {
			quad *= max_shield / _ss;
		}
	}

	// AL 11-15-97: Rules for engine strength affecting max speed:
	//						1. if strength >= 0.5 no affect
	//						2. if strength < 0.5 then max_speed = sqrt(strength)
	//
	//					 This will translate to 71% max speed at 50% engines, and 31% max speed at 10% engines
	//

	float effective_engine_strength;
	float actual_engine_strength;
	if (ship_p->flags[Ship::Ship_Flags::Maneuver_despite_engines]) {
		// Pretend our strength is 100% when this flag is active
		effective_engine_strength = 1.0f;
		actual_engine_strength = 1.0f;
	} else {
		effective_engine_strength = ship_get_subsystem_strength(ship_p, SUBSYSTEM_ENGINE);

		// very annoying, but ship_get_subsystem_strength will typically cap at no lower than 15% strength reported
		// causing the condition below to possibly erroneously believe the engine strength isn't changing while being
		// repaired below that threshold. use the ACTUAL strength for this check
		actual_engine_strength = ship_get_subsystem_strength(ship_p, SUBSYSTEM_ENGINE, false, true);
	}

	// only update max speed if engine_aggregate_strength has changed
	// which helps minimize amount of overrides to max speed
	if (actual_engine_strength != ship_p->prev_engine_strength) {
		ets_update_max_speed(objp);
		ship_p->prev_engine_strength = actual_engine_strength;

		// check if newly updated max speed should be reduced due to engine damage
		// don't let engine strength affect max speed when playing on lowest skill level
		if ((objp != Player_obj) || (Game_skill_level > 0)) {
			if (effective_engine_strength < SHIP_MIN_ENGINES_FOR_FULL_SPEED) {
				objp->phys_info.max_vel.xyz.z *= fl_sqrt(effective_engine_strength);
			}
		}
	}

	if ( timestamp_elapsed(ship_p->next_manage_ets) ) {
		if ( !(objp->flags[Object::Object_Flags::Player_ship]) ) {
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
	// NOTE: ets_power_factor() doesn't need to be called in this function, because all the factors cancel out.  But
	// the system presence does need to be checked since it affects the recharge indexes.

	Assertion(objp != NULL, "Invalid object pointer passed!");
	Assertion(objp->type == OBJ_SHIP, "Object needs to be a ship object!");
	Assertion(engine_energy >= 0.0f && engine_energy <= 1.0f, "Invalid float passed, needs to be in [0, 1], was %f!", engine_energy);

	ship* shipp = &Ships[objp->instance];
	ship_info* sip = &Ship_info[shipp->ship_info_index];

	float initial_engine_recharge_energy_level;
	if (The_mission.ai_profile->flags[AI::Profile_Flags::ETS_energy_same_regardless_of_system_presence])
	{
		int properties = ets_properties(objp);
		if (properties == (HAS_WEAPONS | HAS_SHIELDS | HAS_ENGINES))
			initial_engine_recharge_energy_level = Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX];
		else if (properties == HAS_WEAPONS || properties == HAS_SHIELDS || properties == HAS_ENGINES)
			initial_engine_recharge_energy_level = Energy_levels[ALL_INDEX];
		else
			initial_engine_recharge_energy_level = Energy_levels[ONE_HALF_INDEX];
	}
	else
		initial_engine_recharge_energy_level = Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX];

	// check for a shortcuts first before doing linear interpolation
	if ( engine_energy == initial_engine_recharge_energy_level ){
		return sip->max_speed;
	} else if ( engine_energy == 0.0f ){
		return 0.5f * sip->max_speed;
	} else if ( engine_energy == 1.0f ){
		return sip->max_overclocked_speed;
	} else {
		// do a linear interpolation to find the current max speed, using points (0,1/2 default_max_speed) (.333,default_max_speed)
		// x = x1 + (y-y1) * (x2-x1) / (y2-y1);
		if ( engine_energy < initial_engine_recharge_energy_level ){
			return 0.5f*sip->max_speed + (engine_energy  * (0.5f*sip->max_speed) ) / initial_engine_recharge_energy_level;
		} else {
			// do a linear interpolation to find the current max speed, using points (.333,default_max_speed) (1,max_overclock_speed)
			return sip->max_speed + (engine_energy - initial_engine_recharge_energy_level) * (sip->max_overclocked_speed - sip->max_speed) / (1.0f - initial_engine_recharge_energy_level);
		}
	}
}

void ets_update_max_speed(object* ship_objp)
{
	Assertion(ship_objp != nullptr, "Invalid object pointer passed!");
	Assertion(ship_objp->type == OBJ_SHIP, "Object needs to be a ship object!");

	// calculate the top speed of the ship based on the energy flow to engines
	// (note: this doesn't need the power factor; see comments in ets_get_max_speed())
	float x = Energy_levels[Ships[ship_objp->instance].engine_recharge_index];
	ship_objp->phys_info.max_vel.xyz.z = ets_get_max_speed(ship_objp, x);
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
	ai_info* aip = &Ai_info[Ships[obj->instance].ai_index];

	if ( ship_info_p->power_output == 0 )
		return;

	if (ship_p->flags[Ship::Ship_Flags::Dying])
		return;

	// check if weapons or engines are not being used. If so, don't allow energy management.
	if (!ship_info_p->max_speed || !ship_info_p->max_weapon_reserve) {
		return;
	}

	// also check if the ship has no shields and if the AI is allowed to manage weapons and engines --wookieejedi
	if ( !(ship_p->ship_max_shield_strength) && !( (aip->ai_profile_flags[AI::Profile_Flags::All_nonshielded_ships_can_manage_ets]) || 
		( (ship_info_p->is_fighter_bomber()) && (aip->ai_profile_flags[AI::Profile_Flags::Fightercraft_nonshielded_ships_can_manage_ets])) ) ) {
		return;
	}

	// also check if the ship is playing dead --Goober5000
	if (aip->mode == AIM_PLAY_DEAD && aip->ai_profile_flags[AI::Profile_Flags::Ships_playing_dead_dont_manage_ets])
		return;

	float weapon_left_percent = ship_p->weapon_energy/ship_info_p->max_weapon_reserve;

	// maximum level check for weapons
	//	MK, changed these, might as well let them go up to 100% if nothing else needs the recharge ability.
	if ( weapon_left_percent == 1.0f) {
		decrease_recharge_rate(obj, WEAPONS);
	}

	if (!(obj->flags[Object::Object_Flags::No_shields])) {
		float shield_left_percent = get_shield_pct(obj);
		// maximum level check for shields
		if (shield_left_percent == 1.0f) {
			decrease_recharge_rate(obj, SHIELDS);
		}
		// minimum check for shields
		if (shield_left_percent < SHIELDS_MIN_LEVEL_PERCENT) {
			if (weapon_left_percent > WEAPONS_MIN_LEVEL_PERCENT)
				increase_recharge_rate(obj, SHIELDS);
		}
	}

	// minimum check for weapons and engines
	if ( weapon_left_percent < WEAPONS_MIN_LEVEL_PERCENT ) {
		increase_recharge_rate(obj, WEAPONS);
	}

	if ( ship_p->engine_recharge_index < MIN_ENGINE_RECHARGE_INDEX ) {
		increase_recharge_rate(obj, ENGINES);
	}

	// emergency check for ships with shields
	if (!(obj->flags[Object::Object_Flags::No_shields])) {
		float shield_left_percent = get_shield_pct(obj);
		if (!(The_mission.ai_profile->flags[AI::Profile_Flags::Disable_ai_transferring_energy])) {
			if ( shield_left_percent < SHIELDS_EMERG_LEVEL_PERCENT ) {
				if (ship_p->target_shields_delta == 0.0f)
					transfer_energy_to_shields(obj);
			} else if ( weapon_left_percent < WEAPONS_EMERG_LEVEL_PERCENT ) {
				if ( shield_left_percent > SHIELDS_MIN_LEVEL_PERCENT || weapon_left_percent <= 0.01 )	// dampen ai enthusiasm for sucking energy to weapons
					transfer_energy_to_weapons(obj);
			}
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

void set_recharge_rates(object* obj, int shields, int weapons, int engines) {
	Assertion(obj->type == OBJ_SHIP, "Can't set ets values on a non-ship");
	if (obj->type != OBJ_SHIP)
		return;

	Ships[obj->instance].shield_recharge_index = shields;
	Ships[obj->instance].weapon_recharge_index = weapons;
	Ships[obj->instance].engine_recharge_index = engines;

	ets_update_max_speed(obj);
}

// -------------------------------------------------------------------------------------------------
// set_default_recharge_rates() will set the charge levels for the weapons, shields and
// engines to their default levels
void set_default_recharge_rates(object* obj)
{
	ship* ship_p = &Ships[obj->instance];
	ship_info* ship_info_p = &Ship_info[ship_p->ship_info_index];

	if ( ship_info_p->power_output == 0 )
		return;

	int ship_properties = ets_properties(obj);

	// the default charge rate depends on what systems are on each ship
	switch ( ship_properties ) {
		case HAS_ENGINES | HAS_WEAPONS | HAS_SHIELDS:
			set_recharge_rates(obj, INTIAL_SHIELD_RECHARGE_INDEX, INTIAL_WEAPON_RECHARGE_INDEX, INTIAL_ENGINE_RECHARGE_INDEX);
			break;

		case HAS_ENGINES | HAS_SHIELDS:
			set_recharge_rates(obj, ONE_HALF_INDEX, ZERO_INDEX, ONE_HALF_INDEX);
			break;

		case HAS_WEAPONS | HAS_SHIELDS:
			set_recharge_rates(obj, ONE_HALF_INDEX, ONE_HALF_INDEX, ZERO_INDEX);
			break;

		case HAS_ENGINES | HAS_WEAPONS:
			set_recharge_rates(obj, ZERO_INDEX, ONE_HALF_INDEX, ONE_HALF_INDEX);
			break;

		case HAS_SHIELDS:
			set_recharge_rates(obj, ALL_INDEX, ZERO_INDEX, ZERO_INDEX);
			break;

		case HAS_ENGINES:
			set_recharge_rates(obj, ZERO_INDEX, ZERO_INDEX, ALL_INDEX);
			break;

		case HAS_WEAPONS:
			set_recharge_rates(obj, ZERO_INDEX, ALL_INDEX, ZERO_INDEX);
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

	if (ship_p->flags[Ship::Ship_Flags::No_ets])
		return;

	switch ( ship_system ) {
		case WEAPONS:
			if ( !ship_has_energy_weapons(ship_p) )
				return;

			gain_index = &ship_p->weapon_recharge_index;

			if ( obj->flags[Object::Object_Flags::No_shields] )
				lose_index1 = NULL;
			else
				lose_index1 = &ship_p->shield_recharge_index;

			if ( !ship_has_engine_power(ship_p) )
				lose_index2 = NULL;
			else
				lose_index2 = &ship_p->engine_recharge_index;

			break;

		case SHIELDS:
			if ( obj->flags[Object::Object_Flags::No_shields] )
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

			if ( obj->flags[Object::Object_Flags::No_shields] )
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
			snd_play( gamesnd_get_game_sound(GameSounds::ENERGY_TRANS_FAIL), 0.0f );
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
		snd_play( gamesnd_get_game_sound(GameSounds::ENERGY_TRANS), 0.0f );

	ets_update_max_speed(obj);
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

	if (ship_p->flags[Ship::Ship_Flags::No_ets])
		return;

	switch ( ship_system ) {
		case WEAPONS:
			if ( !ship_has_energy_weapons(ship_p) )
				return;

			lose_index = &ship_p->weapon_recharge_index;

			if ( obj->flags[Object::Object_Flags::No_shields] )
				gain_index1 = NULL;
			else
				gain_index1 = &ship_p->shield_recharge_index;

			if ( !ship_has_engine_power(ship_p) )
				gain_index2 = NULL;
			else
				gain_index2 = &ship_p->engine_recharge_index;

			break;

		case SHIELDS:
			if ( obj->flags[Object::Object_Flags::No_shields] )
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

			if ( obj->flags[Object::Object_Flags::No_shields] )
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
			snd_play( gamesnd_get_game_sound(GameSounds::ENERGY_TRANS_FAIL), 0.0f );
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
		snd_play( gamesnd_get_game_sound(GameSounds::ENERGY_TRANS), 0.0f );

	ets_update_max_speed(obj);
}

void transfer_energy_weapon_common(object *objp, float from_field, float to_field, float *from_delta, float *to_delta, float from_max, float to_max, float scale, float eff)
{
	float	delta;

	delta = from_max * scale;

	if (to_field + *to_delta + eff * delta > to_max && eff > 0)
		delta = (to_max - to_field - *to_delta) / eff;

	if ( delta > 0 ) {
		if ( objp == Player_obj )
			snd_play( gamesnd_get_game_sound(GameSounds::ENERGY_TRANS), 0.0f );

		if (delta > from_field)
			delta = from_field;

		*to_delta += eff * delta;
		*from_delta -= delta;
	} else
		if ( objp == Player_obj )
			snd_play( gamesnd_get_game_sound(GameSounds::ENERGY_TRANS_FAIL), 0.0f );
}

// -------------------------------------------------------------------------------------------------
// transfer_energy_to_shields() will transfer a tabled percentage of max weapon energy
// to shield energy.
void transfer_energy_to_shields(object* obj)
{
	ship*		ship_p = &Ships[obj->instance];
	ship_info*	sinfo_p = &Ship_info[ship_p->ship_info_index];

	if (ship_p->flags[Ship::Ship_Flags::Dying])
		return;

	if ( !ship_has_energy_weapons(ship_p) || obj->flags[Object::Object_Flags::No_shields] )
	{
		return;
	}

	if (sinfo_p->weap_shield_amount == 0 && obj == Player_obj) {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, "%s", XSTR("Ship does not support weapon->shield transfer.", -1));
		snd_play( gamesnd_get_game_sound(GameSounds::ENERGY_TRANS_FAIL), 0.0f );
		return;
	}

	transfer_energy_weapon_common(obj, ship_p->weapon_energy, shield_get_strength(obj), &ship_p->target_weapon_energy_delta, &ship_p->target_shields_delta, sinfo_p->max_weapon_reserve, shield_get_max_strength(ship_p), sinfo_p->weap_shield_amount, sinfo_p->weap_shield_efficiency);
}

// -------------------------------------------------------------------------------------------------
// transfer_energy_to_weapons() will transfer a tabled percentage of max shield energy
// to weapon energy.
void transfer_energy_to_weapons(object* obj)
{
	ship*		ship_p = &Ships[obj->instance];
	ship_info*	sinfo_p = &Ship_info[ship_p->ship_info_index];

	if (ship_p->flags[Ship::Ship_Flags::Dying])
		return;

	if ( !ship_has_energy_weapons(ship_p) || obj->flags[Object::Object_Flags::No_shields] )
	{
		return;
	}

	if (sinfo_p->shield_weap_amount == 0 && obj == Player_obj) {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, "%s", XSTR("Ship does not support shield->weapon transfer.", -1));
		snd_play( gamesnd_get_game_sound(GameSounds::ENERGY_TRANS_FAIL), 0.0f );
		return;
	}

	transfer_energy_weapon_common(obj, shield_get_strength(obj), ship_p->weapon_energy, &ship_p->target_shields_delta, &ship_p->target_weapon_energy_delta, shield_get_max_strength(ship_p), sinfo_p->max_weapon_reserve, sinfo_p->shield_weap_amount, sinfo_p->shield_weap_efficiency);
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

	if (ship_p->flags[Ship::Ship_Flags::No_ets])
		return false;

	// handle ships that are missing parts of the ETS
	int ship_properties = 0;
	if (ship_has_energy_weapons(ship_p)) {
		ship_properties |= HAS_WEAPONS;
	}

	if (!(Objects[ship_p->objnum].flags[Object::Object_Flags::No_shields])) {
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

void HudGaugeEts::render(float  /*frametime*/, bool /*config*/)
{
}

void HudGaugeEts::pageIn()
{
	bm_page_in_aabitmap( Ets_bar.first_frame, Ets_bar.num_frames );
}

/**
 * Draw one ETS bar to screen
 */
void HudGaugeEts::blitGauge(int index, int ix, int iy, float scale, bool config)
{
	if (Ets_bar.first_frame < 0) {
		return;
	}

	int clip_h = fl2i( (1 - Energy_levels[index]) * ETS_bar_h );

	int w, h;
	bm_get_info(Ets_bar.first_frame,&w,&h);

	int x, y;
	if (HUD_shadows) {
		color cur = gr_screen.current_color;
		// These act more as a backing black layer.

		gr_set_color_fast(&Color_black);
		// draw the top portion
		x = ix + fl2i(Top_offsets[0] * scale);
		y = iy + fl2i(Top_offsets[1] * scale);
		
		renderBitmapEx(Ets_bar.first_frame,x,y,w,ETS_bar_h,0,0, scale, config);

		// draw the bottom portion
		x = ix + fl2i(Bottom_offsets[0] * scale);
		y = iy + fl2i(Bottom_offsets[1] * scale);

		renderBitmapEx(Ets_bar.first_frame, x, y, w, y + ETS_bar_h, 0, 0, scale, config);

		if (!config) {
			gr_set_color_fast(&cur);
		} else {
			setGaugeColor(HUD_C_NONE, config);
		}
	}

	int y_start, y_end;
	if ( index < NUM_ENERGY_LEVELS-1 ) {
		// some portion of dark needs to be drawn

		setGaugeColor(HUD_C_NONE, config);

		// draw the top portion
		x = ix + fl2i(Top_offsets[0] * scale);
		y = iy + fl2i(Top_offsets[1] * scale);
		
		renderBitmapEx(Ets_bar.first_frame,x,y,w,clip_h,0,0, scale, config);			

		// draw the bottom portion
		x = ix + fl2i(Bottom_offsets[0] * scale);
		y = iy + fl2i(Bottom_offsets[1] * scale);

		y_start = y + fl2i((ETS_bar_h - clip_h) * scale);
		y_end = y + ETS_bar_h;
		
		renderBitmapEx(Ets_bar.first_frame, x, y_start, w, y_end-y_start, 0, ETS_bar_h-clip_h, scale, config);			
	}

	if ( index > 0 ) {
		if (!config && maybeFlashSexp() == 1 ) {
			setGaugeColor(HUD_C_DIM, config);
			// hud_set_dim_color();
		} else {
			setGaugeColor(HUD_C_BRIGHT, config);
			// hud_set_bright_color();
		}
		// some portion of recharge needs to be drawn

		// draw the top portion
		x = ix + fl2i(Top_offsets[0] * scale);
		y = iy + fl2i(Top_offsets[1] * scale);

		y_start = y + fl2i(clip_h * scale);
		y_end = y + ETS_bar_h;
		
		renderBitmapEx(Ets_bar.first_frame+1, x, y_start, w, y_end-y_start, 0, clip_h, scale, config);			

		// draw the bottom portion
		x = ix + fl2i(Bottom_offsets[0] * scale);
		y = iy + fl2i(Bottom_offsets[1] * scale);
		
		renderBitmapEx(Ets_bar.first_frame+2, x,y,w,ETS_bar_h-clip_h,0,0, scale, config);			
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
void HudGaugeEtsRetail::render(float  /*frametime*/, bool config)
{
	ship* ship_p = nullptr;
	if (!config) {
		ship_p = &Ships[Player_obj->instance];
	}

	if ( Ets_bar.first_frame < 0 ) {
		return;
	}
	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}


	// if at least two gauges are not shown, don't show any
	if (!config) {
		int i = 0;
		if (!ship_has_energy_weapons(ship_p))
			i++;
		if (Player_obj->flags[Object::Object_Flags::No_shields])
			i++;
		if (!ship_has_engine_power(ship_p))
			i++;
		if (i >= 2)
			return;
	}

	setGaugeColor(HUD_C_NONE, config);

	// draw the letters for the gauges first, before any clipping occurs
	// skip letter for any missing gauges (max one, see check above)
	int initial_position = 0;
	if (config || ship_has_energy_weapons(ship_p)) {
		Letter = Letters[0];
		int rx = Gauge_positions[initial_position++] + Letter_offsets[0];
		int ry = position[1] + Letter_offsets[1]; // Explicitely use unconverted y here
		if (config) {
			std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
		}
		renderPrintf(rx, ry, scale, config, NOX("%c"), Letter);
	}
	if (config || !(Player_obj->flags[Object::Object_Flags::No_shields])) {
		Letter = Letters[1];
		int rx = Gauge_positions[initial_position++] + Letter_offsets[0];
		int ry = position[1] + Letter_offsets[1]; // Explicitely use unconverted y here
		if (config) {
			std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
		}
		renderPrintf(rx, ry, scale, config, NOX("%c"), Letter);
	}
	if (config || ship_has_engine_power(ship_p)) {
		Letter = Letters[2];
		int rx = Gauge_positions[initial_position++] + Letter_offsets[0];
		int ry = position[1] + Letter_offsets[1]; // Explicitely use unconverted y here
		if (config) {
			std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
		}
		renderPrintf(rx, ry, scale, config, NOX("%c"), Letter);
	}

	// draw gauges, skipping any gauge that is missing
	initial_position = 0;
	if (config || ship_has_energy_weapons(ship_p)) {
		Letter = Letters[0];
		int level = config ? 4 : ship_p->weapon_recharge_index;
		int rx = Gauge_positions[initial_position++];
		int ry = position[1];
		if (config) {
			std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
		}
		blitGauge(level, rx, ry, scale, config);
	}
	if (config || !(Player_obj->flags[Object::Object_Flags::No_shields])) {
		Letter = Letters[1];
		int level = config ? 4 : ship_p->shield_recharge_index;
		int rx = Gauge_positions[initial_position++];
		int ry = position[1];
		if (config) {
			std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
		}
		blitGauge(level, rx, ry, scale, config);
	}
	if (config || ship_has_engine_power(ship_p)) {
		Letter = Letters[2];
		int level = config ? 4 : ship_p->engine_recharge_index;
		int rx = Gauge_positions[initial_position++];
		int ry = position[1];
		if (config) {
			std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
		}
		blitGauge(level, rx, ry, scale, config);
	}

	if (config) {
		int bmw, bmh;
		bm_get_info(Ets_bar.first_frame, &bmw, &bmh);

		auto coords1 = hud_config_convert_coords(Gauge_positions[0], position[1], scale);
		auto coords2 = hud_config_convert_coords(Gauge_positions[initial_position - 1] + bmw, position[1] + (bmh * 2), scale);

		hud_config_set_mouse_coords(gauge_config_id, coords1.first, coords2.first, coords1.second, coords2.second);
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

void HudGaugeEtsWeapons::render(float  /*frametime*/, bool config)
{
	ship* ship_p = nullptr;
	if (!config) {
		ship_p = &Ships[Player_obj->instance];
	}	

	if ( Ets_bar.first_frame < 0 ) {
		return;
	}

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}


	// if at least two gauges are not shown, don't show any
	if (!config) {
		int i = 0;
		if (!ship_has_energy_weapons(ship_p))
			i++;
		if (Player_obj->flags[Object::Object_Flags::No_shields])
			i++;
		if (!ship_has_engine_power(ship_p))
			i++;
		if (i >= 2)
			return;

		// no weapon energy, no weapon gauge
		if (!ship_has_energy_weapons(ship_p)) {
			return;
		}
	}

	setGaugeColor(HUD_C_NONE, config);

	// draw the letters for the gauge first, before any clipping occurs
	int rx = position[0] + Letter_offsets[0];
	int ry = position[1] + Letter_offsets[1];
	if (config) {
		std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
	}
	renderPrintf(rx, ry, scale, config, NOX("%c"), Letter);

	// draw the gauges for the weapon system
	int level = config ? 4 : ship_p->weapon_recharge_index;
	rx = position[0];
	ry = position[1];
	if (config) {
		std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
	}
	blitGauge(level, rx, ry, scale, config);

	if (config) {
		int bmw, bmh;
		bm_get_info(Ets_bar.first_frame, &bmw, &bmh);

		hud_config_set_mouse_coords(gauge_config_id, rx, rx + bmw, ry, ry + (bmh * 2));
	}
}

HudGaugeEtsShields::HudGaugeEtsShields():
HudGaugeEts(HUD_OBJECT_ETS_SHIELDS, (int)SHIELDS)
{
}

void HudGaugeEtsShields::render(float  /*frametime*/, bool config)
{
	ship* ship_p = nullptr;
	if (!config) {
		ship_p = &Ships[Player_obj->instance];
	}

	if ( Ets_bar.first_frame < 0 ) {
		return;
	}

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}


	// if at least two gauges are not shown, don't show any
	if (!config) {
		int i = 0;
		if (!ship_has_energy_weapons(ship_p))
			i++;
		if (Player_obj->flags[Object::Object_Flags::No_shields])
			i++;
		if (!ship_has_engine_power(ship_p))
			i++;
		if (i >= 2)
			return;

		// no shields, no shields gauge
		if (Player_obj->flags[Object::Object_Flags::No_shields]) {
			return;
		}
	}

	setGaugeColor(HUD_C_NONE, config);

	// draw the letters for the gauge first, before any clipping occurs
	int rx = position[0] + Letter_offsets[0];
	int ry = position[1] + Letter_offsets[1];
	if (config) {
		std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
	}
	renderPrintf(rx, ry, scale, config, NOX("%c"), Letter);

	// draw the gauge for the shield system
	int level = config ? 4 : ship_p->shield_recharge_index;
	rx = position[0];
	ry = position[1];
	if (config) {
		std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
	}
	blitGauge(level, rx, ry, scale, config);

	if (config) {
		int bmw, bmh;
		bm_get_info(Ets_bar.first_frame, &bmw, &bmh);

		hud_config_set_mouse_coords(gauge_config_id, rx, rx + bmw, ry, ry + (bmh * 2));
	}
}

HudGaugeEtsEngines::HudGaugeEtsEngines():
HudGaugeEts(HUD_OBJECT_ETS_ENGINES, (int)ENGINES)
{
}

void HudGaugeEtsEngines::render(float  /*frametime*/, bool config)
{
	ship* ship_p = nullptr;
	if (!config) {
		ship_p = &Ships[Player_obj->instance];
	}	

	if ( Ets_bar.first_frame < 0 ) {
		return;
	}

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}


	// if at least two gauges are not shown, don't show any
	if (!config) {
		int i = 0;
		if (!ship_has_energy_weapons(ship_p))
			i++;
		if (Player_obj->flags[Object::Object_Flags::No_shields])
			i++;
		if (!ship_has_engine_power(ship_p))
			i++;
		if (i >= 2)
			return;

		// no engines, no engine gauge
		if (!ship_has_engine_power(ship_p)) {
			return;
		}
	}

	setGaugeColor(HUD_C_NONE, config);

	// draw the letters for the gauge first, before any clipping occurs
	int rx = position[0] + Letter_offsets[0];
	int ry = position[1] + Letter_offsets[1];
	if (config) {
		std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
	}
	renderPrintf(rx, ry, scale, config, NOX("%c"), Letter);

	// draw the gauge for the engine system
	int level = config ? 4 : ship_p->engine_recharge_index;
	rx = position[0];
	ry = position[1];
	if (config) {
		std::tie(rx, ry) = hud_config_convert_coords(rx, ry, scale);
	}
	blitGauge(level, rx, ry, scale, config);

	if (config) {
		int bmw, bmh;
		bm_get_info(Ets_bar.first_frame, &bmw, &bmh);

		hud_config_set_mouse_coords(gauge_config_id, rx, rx + bmw, ry, ry + (bmh * 2));
	}
}
