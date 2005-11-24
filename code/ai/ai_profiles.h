/*
 * ai_profiles.h
 *
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/ai/ai_profiles.h $
 * $Revision: 1.2 $
 * $Date: 2005-11-24 08:46:11 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/11/21 02:43:30  Goober5000
 * change from "setting" to "profile"; this way makes more sense
 * --Goober5000
 *
 */

#ifndef _AI_PROFILES_H_
#define _AI_PROFILES_H_

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "globalincs/systemvars.h"


// flags
#define AIPF_SMART_SHIELD_MANAGEMENT								(1 << 0)
#define AIPF_BIG_SHIPS_CAN_ATTACK_BEAM_TURRETS_ON_UNTARGETED_SHIPS	(1 << 1)
#define AIPF_AI_CHASE_DISABLES_DYNAMIC_TARGETING_TEMPORARILY		(1 << 2)
#define AIPF_AI_CHASE_ALLOWS_REARM									(1 << 3)
#define AIPF_SMART_PRIMARY_WEAPON_SELECTION							(1 << 4)
#define AIPF_SMART_SECONDARY_WEAPON_SELECTION						(1 << 5)
#define AIPF_ALLOW_RAPID_SECONDARY_DUMBFIRE							(1 << 6)
#define AIPF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS						(1 << 7)
#define AIPF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY					(1 << 8)
#define AIPF_HACK_IMPROVE_NON_HOMING_SWARM_TURRET_FIRE_ACCURACY		(1 << 9)
#define AIPF_SHOCKWAVES_DAMAGE_SMALL_SHIP_SUBSYSTEMS				(1 << 10)
#define AIPF_NAVIGATION_SUBSYS_GOVERNS_WARP							(1 << 11)


#define MAX_AI_PROFILES	5

typedef struct ai_profile_t {

	char profile_name[NAME_LENGTH];

	int flags;

	// difficulty-related values
	int max_incoming_asteroids[NUM_SKILL_LEVELS];			// max number of asteroids thrown at friendlies
	int	max_allowed_player_homers[NUM_SKILL_LEVELS];		// max number of simultaneous homing weapons on player
	int	max_attackers[NUM_SKILL_LEVELS];					// max number of ships that can attack a ship
	fix predict_position_delay[NUM_SKILL_LEVELS];			// how long until AI predicts a ship position
	float in_range_time[NUM_SKILL_LEVELS];					// seconds to add to time it takes to get enemy in range (only for player's enemies)
	float shield_manage_delay[NUM_SKILL_LEVELS];			// how long before AI manages shields (note that the player's team always uses the average skill's delay)

	// AI ships link primaries if energy levels greater than these percents
	float link_energy_levels_always[NUM_SKILL_LEVELS];		// always link
	float link_energy_levels_maybe[NUM_SKILL_LEVELS];		// link if hull strength low

	// AI ships link primaries if ammunition levels greater than these percents
	float link_ammo_levels_always[NUM_SKILL_LEVELS];		//	always link
	float link_ammo_levels_maybe[NUM_SKILL_LEVELS];			//	link if hull strength low

	// difficulty-related scales
	float cmeasure_life_scale[NUM_SKILL_LEVELS];			// life of countermeasures
	float cmeasure_fire_chance[NUM_SKILL_LEVELS];			// chance a countermeasure will be fired based on skill level (also scaled by ai_class)
	float weapon_energy_scale[NUM_SKILL_LEVELS];			// weapon energy available
	float shield_energy_scale[NUM_SKILL_LEVELS];			// shield energy available
	float afterburner_recharge_scale[NUM_SKILL_LEVELS];		// speed of afterburner recharge
	float player_damage_scale[NUM_SKILL_LEVELS];			// damage applied to the player
	
	float subsys_damage_scale[NUM_SKILL_LEVELS];			// damage applied to a player subsystem
	float beam_friendly_damage_cap[NUM_SKILL_LEVELS];		// damage cap values for friendly beam fire
	float turn_time_scale[NUM_SKILL_LEVELS];				// speed at which enemy ships turn

	//	Multiplicative delay factors for increasing skill levels.
	float ship_fire_delay_scale_hostile[NUM_SKILL_LEVELS];
	float ship_fire_delay_scale_friendly[NUM_SKILL_LEVELS];

} ai_profile_t;


extern int Num_ai_profiles;
extern int Default_ai_profile;
extern ai_profile_t Ai_profiles[MAX_AI_PROFILES];


void ai_profiles_init();

int ai_profile_lookup(char *name);

#endif
