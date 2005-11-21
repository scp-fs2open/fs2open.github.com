/*
 * ai_settings.h
 *
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/ai/ai_settings.h $
 * $Revision: 1.2 $
 * $Date: 2005-11-21 01:53:57 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/11/21 00:50:02  Goober5000
 * add ai_settings.tbl
 * --Goober5000
 *
 */

#ifndef _AI_SETTINGS_H_
#define _AI_SETTINGS_H_

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "globalincs/systemvars.h"


// flags
#define AIOF_SMART_SHIELD_MANAGEMENT								(1 << 0)
#define AIOF_BIG_SHIPS_CAN_ATTACK_BEAM_TURRETS_ON_UNTARGETED_SHIPS	(1 << 1)
#define AIOF_AI_CHASE_DISABLES_DYNAMIC_TARGETING_TEMPORARILY		(1 << 2)
#define AIOF_AI_CHASE_ALLOWS_REARM									(1 << 3)
#define AIOF_SMART_PRIMARY_WEAPON_SELECTION							(1 << 4)
#define AIOF_SMART_SECONDARY_WEAPON_SELECTION						(1 << 5)
#define AIOF_ALLOW_RAPID_SECONDARY_DUMBFIRE							(1 << 6)
#define AIOF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS						(1 << 7)
#define AIOF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY					(1 << 8)
#define AIOF_HACK_IMPROVE_NON_HOMING_SWARM_TURRET_FIRE_ACCURACY		(1 << 9)
#define AIOF_SHOCKWAVES_DAMAGE_SMALL_SHIP_SUBSYSTEMS				(1 << 10)


#define MAX_AI_SETTINGS	5

typedef struct ai_setting {

	char setting_name[NAME_LENGTH];

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

} ai_setting;


extern int Num_ai_settings;
extern int Default_ai_setting;
extern ai_setting Ai_settings[MAX_AI_SETTINGS];


void ai_settings_init();

int ai_setting_lookup(char *name);

#endif
