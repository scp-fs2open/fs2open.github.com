/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#ifndef _AI_PROFILES_H_
#define _AI_PROFILES_H_

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "ai/ai_flags.h"

// AI Path types
#define	AI_PATH_MODE_NORMAL 0
#define	AI_PATH_MODE_ALT1	1

#define	AI_RANGE_AWARE_SEC_SEL_MODE_RETAIL 0
#define	AI_RANGE_AWARE_SEC_SEL_MODE_AWARE 1
	
#define MAX_AI_PROFILES	5

class ai_profile_t {
public:
	char profile_name[NAME_LENGTH];

    flagset<AI::Profile_Flags> flags;

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
	float link_ammo_levels_always[NUM_SKILL_LEVELS];		// always link
	float link_ammo_levels_maybe[NUM_SKILL_LEVELS];			// link if hull strength low
	float primary_ammo_burst_mult[NUM_SKILL_LEVELS];		// SUSHI: Multiplier adjusting burst frequency for ballistic primary weapons

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
	float glide_attack_percent[NUM_SKILL_LEVELS];			// SUSHI: The likelihood (0.0-1.0) of the AI to use the "glide attack" move
	float circle_strafe_percent[NUM_SKILL_LEVELS];			// SUSHI: The likelihood (0.0-1.0) of the AI to use the "circle strafe" move
	float glide_strafe_percent[NUM_SKILL_LEVELS];			// SUSHI: The likelihood (0.0-1.0) of the AI to use glide when strafing capships
	float random_sidethrust_percent[NUM_SKILL_LEVELS];		// SUSHI: The likelihood (0.0-1.0) of the AI to randomly use sidethrust while dogfihthing
	float stalemate_time_thresh[NUM_SKILL_LEVELS];			// SUSHI: The amount of time required for the AI to detect (and try to break) dogfight stalemate
	float stalemate_dist_thresh[NUM_SKILL_LEVELS];			// SUSHI: The maximum distance the AI and target must be within for a stalemate
	float max_aim_update_delay[NUM_SKILL_LEVELS];			// SUSHI: The maximum delay before the AI updates their aim against small ships
	float turret_max_aim_update_delay[NUM_SKILL_LEVELS];	// SUSHI: As above, but for turrets updating their aim

	//	Multiplicative delay factors for increasing skill levels.
	float ship_fire_delay_scale_hostile[NUM_SKILL_LEVELS];
	float ship_fire_delay_scale_friendly[NUM_SKILL_LEVELS];

	//	Multiplicative secondary delay factors for increasing skill levels.
	float ship_fire_secondary_delay_scale_hostile[NUM_SKILL_LEVELS];
	float ship_fire_secondary_delay_scale_friendly[NUM_SKILL_LEVELS];

	//	Maximum turrets of one ship allowed to shoot the same target
	int max_turret_ownage_target[NUM_SKILL_LEVELS];
	int max_turret_ownage_player[NUM_SKILL_LEVELS];

	// percentage of damage caused required for a kill/assist
	float kill_percentage_scale[NUM_SKILL_LEVELS];
	float assist_percentage_scale[NUM_SKILL_LEVELS];

	// percentage of the capships score given to other team mates on a kill
	float assist_award_percentage_scale[NUM_SKILL_LEVELS];

	// the number of points subtracted from score for a rearm started on a player
	int repair_penalty[NUM_SKILL_LEVELS];

	float delay_bomb_arm_timer[NUM_SKILL_LEVELS];

	// the chance (x/7) that ship is allowed to fire missiles at player ship.
	int chance_to_use_missiles_on_plr[NUM_SKILL_LEVELS];

	// Player-specific autoaim FOV override
	float player_autoaim_fov[NUM_SKILL_LEVELS];

	float detail_distance_mult[MAX_DETAIL_LEVEL + 1];	//MAX_DETAIL_LEVEL really needs to be 4

	int ai_path_mode;

	// radii to use for the radius for subsystem path points and default value --wookieejedi
	int subsystem_path_radii;

	// Ships flying bay paths will gradually accelerate/decelerate instead of
	// flying the whole path at max speed
	float bay_arrive_speed_mult;
	float bay_depart_speed_mult;

	// How much 0-1 of a second-order lead prediction factor to add to lead indicators. Affects only the HUD indicator, and autoaim.
	float second_order_lead_predict_factor;
	
	//Controls if the AI is dumb enough to keep trying to use out-of-range secondaries. 
	int ai_range_aware_secondary_select_mode;

	// how often turrets shoulds check for new targets, milliseconds
	float turret_target_recheck_time;

    void reset();
};


extern int Num_ai_profiles;
extern int Default_ai_profile;
extern ai_profile_t Ai_profiles[MAX_AI_PROFILES];

#define AI_PROFILES_INDEX(ai_p) ((int)((ai_p) - Ai_profiles))

void ai_profiles_init();

int ai_profile_lookup(char *name);

#endif
