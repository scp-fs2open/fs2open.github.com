/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#ifndef _AI_PROFILES_H_
#define _AI_PROFILES_H_

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "globalincs/systemvars.h"
#include "ai/ai_flags.h"

// AI Path types
#define	AI_PATH_MODE_NORMAL 0
#define	AI_PATH_MODE_ALT1	1
	
#define MAX_AI_PROFILES	5

typedef struct ai_profile_t {

	ai_profile_t() {
		init();
	}

	ai_profile_t(const ai_profile_t &other) {
		strcpy_s(profile_name, other.profile_name);
		flags = other.flags;

		for (int i = 0; i < NUM_SKILL_LEVELS; ++i) {
			max_incoming_asteroids[i] = other.max_incoming_asteroids[i];
			max_allowed_player_homers[i] = other.max_allowed_player_homers[i];
			max_attackers[i] = other.max_attackers[i];
			predict_position_delay[i] = other.predict_position_delay[i];
			in_range_time[i] = other.in_range_time[i];
			shield_manage_delay[i] = other.shield_manage_delay[i];

			link_energy_levels_always[i] = other.link_energy_levels_always[i];
			link_energy_levels_maybe[i] = other.link_energy_levels_maybe[i];

			link_ammo_levels_always[i] = other.link_ammo_levels_always[i];
			link_ammo_levels_maybe[i] = other.link_ammo_levels_maybe[i];
			primary_ammo_burst_mult[i] = other.primary_ammo_burst_mult[i];

			cmeasure_life_scale[i] = other.cmeasure_life_scale[i];
			cmeasure_fire_chance[i] = other.cmeasure_fire_chance[i];
			weapon_energy_scale[i] = other.weapon_energy_scale[i];
			shield_energy_scale[i] = other.shield_energy_scale[i];
			afterburner_recharge_scale[i] = other.afterburner_recharge_scale[i];
			player_damage_scale[i] = other.player_damage_scale[i];

			subsys_damage_scale[i] = other.subsys_damage_scale[i];
			beam_friendly_damage_cap[i] = other.beam_friendly_damage_cap[i];
			turn_time_scale[i] = other.turn_time_scale[i];
			glide_attack_percent[i] = other.glide_attack_percent[i];
			circle_strafe_percent[i] = other.circle_strafe_percent[i];
			glide_strafe_percent[i] = other.glide_strafe_percent[i];
			random_sidethrust_percent[i] = other.random_sidethrust_percent[i];
			stalemate_time_thresh[i] = other.stalemate_time_thresh[i];
			stalemate_dist_thresh[i] = other.stalemate_dist_thresh[i];
			max_aim_update_delay[i] = other.max_aim_update_delay[i];

			turret_max_aim_update_delay[i] = other.turret_max_aim_update_delay[i];
			ship_fire_delay_scale_hostile[i] = other.ship_fire_delay_scale_hostile[i];
			ship_fire_delay_scale_friendly[i] = other.ship_fire_delay_scale_friendly[i];

			ship_fire_secondary_delay_scale_hostile[i] = other.ship_fire_secondary_delay_scale_hostile[i];
			ship_fire_secondary_delay_scale_friendly[i] = other.ship_fire_secondary_delay_scale_friendly[i];

			max_turret_ownage_target[i] = other.max_turret_ownage_target[i];
			max_turret_ownage_player[i] = other.max_turret_ownage_player[i];

			kill_percentage_scale[i] = other.kill_percentage_scale[i];
			assist_percentage_scale[i] = other.assist_percentage_scale[i];
			assist_award_percentage_scale[i] = other.assist_award_percentage_scale[i];

			repair_penalty[i] = other.repair_penalty[i];

			delay_bomb_arm_timer[i] = other.delay_bomb_arm_timer[i];

			chance_to_use_missiles_on_plr[i] = other.chance_to_use_missiles_on_plr[i];

			player_autoaim_fov[i] = other.player_autoaim_fov[i];
		}

		for (int i = 0; i < MAX_DETAIL_LEVEL + 1; ++i)
			detail_distance_mult[i] = other.detail_distance_mult[i];

		ai_path_mode = other.ai_path_mode;
	}

	void init() {
		strcpy_s(profile_name, "\0");
		flags.reset();

		for (int i = 0; i < NUM_SKILL_LEVELS; ++i) {
			max_incoming_asteroids[i] = 0;
			max_allowed_player_homers[i] = 0;
			max_attackers[i] = 0;
			predict_position_delay[i] = 0;
			in_range_time[i] = 0.0f;
			shield_manage_delay[i] = 0.0f;

			link_energy_levels_always[i] = 0.0f;
			link_energy_levels_maybe[i] = 0.0f;

			link_ammo_levels_always[i] = 0.0f;
			link_ammo_levels_maybe[i] = 0.0f;
			primary_ammo_burst_mult[i] = 0.0f;

			cmeasure_life_scale[i] = 0.0f;
			cmeasure_fire_chance[i] = 0.0f;
			weapon_energy_scale[i] = 0.0f;
			shield_energy_scale[i] = 0.0f;
			afterburner_recharge_scale[i] = 0.0f;
			player_damage_scale[i] = 0.0f;

			subsys_damage_scale[i] = 0.0f;
			beam_friendly_damage_cap[i] = 0.0f;
			turn_time_scale[i] = 0.0f;
			glide_attack_percent[i] = 0.0f;
			circle_strafe_percent[i] = 0.0f;
			glide_strafe_percent[i] = 0.0f;
			random_sidethrust_percent[i] = 0.0f;
			stalemate_time_thresh[i] = 0.0f;
			stalemate_dist_thresh[i] = 0.0f;
			max_aim_update_delay[i] = 0.0f;

			turret_max_aim_update_delay[i] = 0.0f;
			ship_fire_delay_scale_hostile[i] = 0.0f;
			ship_fire_delay_scale_friendly[i] = 0.0f;

			ship_fire_secondary_delay_scale_hostile[i] = 0.0f;
			ship_fire_secondary_delay_scale_friendly[i] = 0.0f;

			max_turret_ownage_target[i] = 0;
			max_turret_ownage_player[i] = 0;

			kill_percentage_scale[i] = 0.0f;
			assist_percentage_scale[i] = 0.0f;
			assist_award_percentage_scale[i] = 0.0f;

			repair_penalty[i] = 0;

			delay_bomb_arm_timer[i] = 0.0f;

			chance_to_use_missiles_on_plr[i] = 0;

			player_autoaim_fov[i] = 0.0f;
		}

		for (int i = 0; i < MAX_DETAIL_LEVEL + 1; ++i)
			detail_distance_mult[i] = 0.0f;

		ai_path_mode = 0;
	}

	char profile_name[NAME_LENGTH];

	flagset<AI::Profile_flags> flags;

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

} ai_profile_t;


extern int Num_ai_profiles;
extern int Default_ai_profile;
extern ai_profile_t Ai_profiles[MAX_AI_PROFILES];


void ai_profiles_init();

int ai_profile_lookup(char *name);

#endif
