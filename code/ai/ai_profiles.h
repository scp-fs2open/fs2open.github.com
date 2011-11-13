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

// flag int defines
#define AIP_FLAG		1
#define AIP_FLAG2		2

// flags
#define AIPF_SMART_SHIELD_MANAGEMENT								(1 << 0)
#define AIPF_BIG_SHIPS_CAN_ATTACK_BEAM_TURRETS_ON_UNTARGETED_SHIPS	(1 << 1)
#define AIPF_SMART_PRIMARY_WEAPON_SELECTION							(1 << 2)
#define AIPF_SMART_SECONDARY_WEAPON_SELECTION						(1 << 3)
#define AIPF_ALLOW_RAPID_SECONDARY_DUMBFIRE							(1 << 4)
#define AIPF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS						(1 << 5)
#define AIPF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY					(1 << 6)
#define AIPF_HACK_IMPROVE_NON_HOMING_SWARM_TURRET_FIRE_ACCURACY		(1 << 7)
#define AIPF_SHOCKWAVES_DAMAGE_SMALL_SHIP_SUBSYSTEMS				(1 << 8)
#define AIPF_NAVIGATION_SUBSYS_GOVERNS_WARP							(1 << 9)
#define AIPF_NO_MIN_DOCK_SPEED_CAP									(1 << 10)
#define AIPF_DISABLE_LINKED_FIRE_PENALTY							(1 << 11)
#define AIPF_DISABLE_WEAPON_DAMAGE_SCALING							(1 << 12)
#define AIPF_USE_ADDITIVE_WEAPON_VELOCITY							(1 << 13)
#define AIPF_USE_NEWTONIAN_DAMPENING								(1 << 14)
#define AIPF_INCLUDE_BEAMS_IN_STAT_CALCS							(1 << 15)
#define AIPF_KILL_SCORING_SCALES_WITH_DAMAGE						(1 << 16)
#define AIPF_ASSIST_SCORING_SCALES_WITH_DAMAGE						(1 << 17)
#define AIPF_ALLOW_MULTI_EVENT_SCORING								(1 << 18)
#define AIPF_SMART_AFTERBURNER_MANAGEMENT							(1 << 19)
#define AIPF_FIX_LINKED_PRIMARY_BUG									(1 << 20)
#define AIPF_PREVENT_TARGETING_BOMBS_BEYOND_RANGE					(1 << 21)
#define AIPF_SMART_SUBSYSTEM_TARGETING_FOR_TURRETS					(1 << 22)
#define AIPF_FIX_HEAT_SEEKER_STEALTH_BUG							(1 << 23)
#define AIPF_MULTI_ALLOW_EMPTY_PRIMARIES							(1 << 24)
#define AIPF_MULTI_ALLOW_EMPTY_SECONDARIES							(1 << 25)
#define AIPF_ALLOW_TURRETS_TARGET_WEAPONS_FREELY                    (1 << 26)
#define AIPF_USE_ONLY_SINGLE_FOV_FOR_TURRETS						(1 << 27)
#define AIPF_ALLOW_VERTICAL_DODGE									(1 << 28)	//Allows AI ships to evade weapons vertically as well as horizontally
#define AIPF_FORCE_BEAM_TURRET_FOV									(1 << 29)
#define AIPF_FIX_AI_CLASS_BUG										(1 << 30)

// flags2
#define AIPF2_TURRETS_IGNORE_TARGET_RADIUS							(1 << 0)
#define AIPF2_NO_SPECIAL_PLAYER_AVOID								(1 << 1)
#define AIPF2_PERFORM_LESS_SCREAM_CHECKS							(1 << 2)
#define AIPF2_ALL_SHIPS_MANAGE_SHIELDS								(1 << 3)
#define AIPF2_ADVANCED_TURRET_FOV_EDGE_CHECKS						(1 << 4)
#define AIPF2_REQUIRE_TURRET_TO_HAVE_TARGET_IN_FOV					(1 << 5)
#define AIPF2_AI_AIMS_FROM_SHIP_CENTER								(1 << 6)
#define AIPF2_ALLOW_PRIMARY_LINK_DELAY								(1 << 7)
#define	AIPF2_BEAMS_DAMAGE_WEAPONS									(1 << 8)
#define AIPF2_PLAYER_WEAPON_SCALE_FIX								(1 << 9)
#define AIPF2_NO_WARP_CAMERA										(1 << 10)

// AI Path types
#define	AI_PATH_MODE_NORMAL 0
#define	AI_PATH_MODE_ALT1	1
	
#define MAX_AI_PROFILES	5

typedef struct ai_profile_t {

	char profile_name[NAME_LENGTH];

	int flags;
	int flags2;

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
