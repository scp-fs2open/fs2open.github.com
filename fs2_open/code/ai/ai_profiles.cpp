/*
 * ai_profiles.cpp
 *
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/ai/ai_profiles.cpp $
 * $Revision: 1.9 $
 * $Date: 2006-02-13 00:20:44 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2005/12/22 04:32:44  taylor
 * GCC knows that fix == int so it hates the utility functions here, for sanity sake just rename them to type specific to avoid rampant casting
 *
 * Revision 1.7  2005/12/06 03:17:48  taylor
 * cleanup some debug log messages:
 *   note that a nprintf() with "Warning" or "General" is basically the same thing as mprintf()
 *   make sure that OpenAL init failures always get to the debug log
 *
 * Revision 1.6  2005/11/24 09:08:44  Goober5000
 * bah, forgot a colon
 * --Goober5000
 *
 * Revision 1.5  2005/11/24 08:46:11  Goober5000
 * * cleaned up mission_do_departure
 *   * fixed a hidden crash (array index being -1; would only
 * be triggered for ships w/o subspace drives under certain conditions)
 *   * removed finding a new fighterbay target because it might screw up missions
 *   * improved clarity, code flow, and readability :)
 * * added custom AI flag for disabling warpouts if navigation subsystem fails
 * --Goober5000
 *
 * Revision 1.4  2005/11/24 07:27:14  taylor
 * fix fred2 startup crash (we probably need to Assert() this case so it doesn't easily happen again)
 *
 * Revision 1.3  2005/11/21 15:01:18  taylor
 * little cleaner, lot more friendly to 64-bit archs (long type is usually 64-bit there)
 *
 * Revision 1.2  2005/11/21 03:47:51  Goober5000
 * bah and double bah
 * --Goober5000
 *
 * Revision 1.1  2005/11/21 02:43:30  Goober5000
 * change from "setting" to "profile"; this way makes more sense
 * --Goober5000
 *
 */


#include "ai/ai_profiles.h"
#include "parse/parselo.h"


int Num_ai_profiles;
int Default_ai_profile;
ai_profile_t Ai_profiles[MAX_AI_PROFILES];


// utility
void skill_level_array_copy_int(int dest[], int src[])
{
	memcpy(dest, src, NUM_SKILL_LEVELS * sizeof(int));
}

// utility
void skill_level_array_copy_fix(fix dest[], fix src[])
{
	memcpy(dest, src, NUM_SKILL_LEVELS * sizeof(fix));
}

// utility
void skill_level_array_copy_float(float dest[], float src[])
{
	memcpy(dest, src, NUM_SKILL_LEVELS * sizeof(float));
}

// utility
void set_flag(ai_profile_t *profile, char *name, int flag)
{
	if (optional_string(name))
	{
		bool val;
		stuff_boolean(&val);

		if (val)
			profile->flags |= flag;
		else
			profile->flags &= ~flag;
	}
}

// This function initializes the given profile with the default data.
// We need to do it this way instead of through a default table because
// 1) the default profiles always need to be available; and 2) the
// other profiles inherit their unspecified options from this profile,
// which means it must be initialized before any others.
void init_profile_to_default(ai_profile_t *profile)
{
	strcpy(profile->profile_name, "FS2 RETAIL");

	profile->flags = 0;

	int	max_incoming_asteroids[NUM_SKILL_LEVELS] = {3, 4, 5, 7, 10};
	skill_level_array_copy_int(profile->max_incoming_asteroids, max_incoming_asteroids);

	int	max_allowed_player_homers[NUM_SKILL_LEVELS] = {2, 3, 4, 7, 99};
	skill_level_array_copy_int(profile->max_allowed_player_homers, max_allowed_player_homers);

	int	max_attackers[NUM_SKILL_LEVELS] = {2, 3, 4, 5, 99};
	skill_level_array_copy_int(profile->max_attackers, max_attackers);

	fix predict_position_delay[NUM_SKILL_LEVELS] = {2*F1_0, 3*F1_0/2, 4*F1_0/3, F1_0/2, 0};
	skill_level_array_copy_fix(profile->predict_position_delay, predict_position_delay);

	float cmeasure_life_scale[NUM_SKILL_LEVELS] = {3.0f, 2.0f, 1.5f, 1.25f, 1.0f};
	skill_level_array_copy_float(profile->cmeasure_life_scale, cmeasure_life_scale);

	float weapon_energy_scale[NUM_SKILL_LEVELS] = {10.0f, 4.0f, 2.5f, 2.0f, 1.5f};
	skill_level_array_copy_float(profile->weapon_energy_scale, weapon_energy_scale);

	float shield_energy_scale[NUM_SKILL_LEVELS] = {4.0f, 2.0f, 1.5f, 1.25f, 1.0f};
	skill_level_array_copy_float(profile->shield_energy_scale, shield_energy_scale);

	float afterburner_recharge_scale[NUM_SKILL_LEVELS] = {5.0f, 3.0f, 2.0f, 1.5f, 1.0f};
	skill_level_array_copy_float(profile->afterburner_recharge_scale, afterburner_recharge_scale);

	float player_damage_scale[NUM_SKILL_LEVELS] = {0.25f, 0.5f, 0.65f, 0.85f, 1.0f};
	skill_level_array_copy_float(profile->player_damage_scale, player_damage_scale);

	float subsys_damage_scale[NUM_SKILL_LEVELS] = {0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
	skill_level_array_copy_float(profile->subsys_damage_scale, subsys_damage_scale);

	float beam_friendly_damage_cap[NUM_SKILL_LEVELS] = { 0.0f, 5.0f, 10.0f, 20.0f, 30.0f };
	skill_level_array_copy_float(profile->beam_friendly_damage_cap, beam_friendly_damage_cap);

	float turn_time_scale[NUM_SKILL_LEVELS] = {3.0f, 2.2f, 1.6f, 1.3f, 1.0f};
	skill_level_array_copy_float(profile->turn_time_scale, turn_time_scale);

	float link_energy_levels_always[NUM_SKILL_LEVELS] = {100.0f, 80.0f, 60.0f, 40.0f, 20.0f};
	skill_level_array_copy_float(profile->link_energy_levels_always, link_energy_levels_always);

	float link_energy_levels_maybe[NUM_SKILL_LEVELS] = {90.0f, 60.0f, 40.0f, 20.0f, 10.0f};
	skill_level_array_copy_float(profile->link_energy_levels_maybe, link_energy_levels_maybe);

	float link_ammo_levels_always[NUM_SKILL_LEVELS] = {95.0f, 80.0f, 60.0f, 40.0f, 20.0f};
	skill_level_array_copy_float(profile->link_ammo_levels_always, link_ammo_levels_always);

	float link_ammo_levels_maybe[NUM_SKILL_LEVELS] = {90.0f, 60.0f, 40.0f, 20.0f, 10.0f};
	skill_level_array_copy_float(profile->link_ammo_levels_maybe, link_ammo_levels_maybe);

	float in_range_time[NUM_SKILL_LEVELS] = {2.0f, 1.4f, 0.75f, 0.0f, -1.0f};
	skill_level_array_copy_float(profile->in_range_time, in_range_time);

	float cmeasure_fire_chance[NUM_SKILL_LEVELS] = {0.2f, 0.3f, 0.5f, 0.9f, 1.1f};
	skill_level_array_copy_float(profile->cmeasure_fire_chance, cmeasure_fire_chance);

	float shield_manage_delay[NUM_SKILL_LEVELS] = {5.0f, 4.0f, 2.5f, 1.2f, 0.1f};
	skill_level_array_copy_float(profile->shield_manage_delay, shield_manage_delay);

	float ship_fire_delay_scale_hostile[NUM_SKILL_LEVELS] = {4.0f, 2.5f, 1.75f, 1.25f, 1.0f};
	skill_level_array_copy_float(profile->ship_fire_delay_scale_hostile, ship_fire_delay_scale_hostile);

	float ship_fire_delay_scale_friendly[NUM_SKILL_LEVELS] = {2.0f, 1.4f, 1.25f, 1.1f, 1.0f};
	skill_level_array_copy_float(profile->ship_fire_delay_scale_friendly, ship_fire_delay_scale_friendly);
}

// This function parses the data from ai_profiles.tbl

void ai_profiles_init()
{
	char default_profile_name[NAME_LENGTH];

	// init retail entry first
	init_profile_to_default(&Ai_profiles[0]);
	Default_ai_profile = 0;
	Num_ai_profiles = 1;

	// load file
	if (cf_exists_full("ai_profiles.tbl", CF_TYPE_TABLES))
	{
		read_file_text("ai_profiles.tbl");
	}
	// if table doesn't exist, just use the defaults
	else
	{
		mprintf(("No ai_profiles.tbl found; using defaults.\n"));
		return;
	}

	reset_parse();

	required_string("#AI Profiles");

	// different default?
	if (optional_string("$Default Profile:"))
		stuff_string(default_profile_name, F_NAME, NULL, NAME_LENGTH);
	else
		default_profile_name[0] = '\0';

	// begin reading data
	while (required_string_either("#End","$Profile Name:"))
	{
		ai_profile_t *profile;
		
		// make sure we're under the limit
		if (Num_ai_profiles >= MAX_AI_PROFILES)
		{
			Warning(LOCATION, "Too many profiles in ai_profiles.tbl!  Max is %d.\n", MAX_AI_PROFILES-1);
			skip_to_string("#End", NULL);
			break;
		}

		profile = &Ai_profiles[Num_ai_profiles];
		Num_ai_profiles++;

		// initialize
		memcpy(profile, &Ai_profiles[0], sizeof(ai_profile_t));

		// get the name
		required_string("$Profile Name:");
		stuff_string(profile->profile_name, F_NAME, NULL, NAME_LENGTH);

		// fill in any and all settings; they're all optional and can be in any order
		while (!check_for_string("$Profile Name:") && !check_for_string("#End"))
		{
			if (optional_string("$Player Afterburner Recharge Scale:"))
				parse_float_list(profile->afterburner_recharge_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Max Beam Friendly Fire Damage:"))
				parse_float_list(profile->beam_friendly_damage_cap, NUM_SKILL_LEVELS);

			if (optional_string("$Player Countermeasure Life Scale:"))
				parse_float_list(profile->cmeasure_life_scale, NUM_SKILL_LEVELS);

			if (optional_string("$AI Countermeasure Firing Chance:"))
				parse_float_list(profile->cmeasure_fire_chance, NUM_SKILL_LEVELS);

			if (optional_string("$AI In Range Time:"))
				parse_float_list(profile->in_range_time, NUM_SKILL_LEVELS);

			if (optional_string("$AI Always Links Ammo Weapons:"))
				parse_float_list(profile->link_ammo_levels_always, NUM_SKILL_LEVELS);

			if (optional_string("$AI Maybe Links Ammo Weapons:"))
				parse_float_list(profile->link_ammo_levels_maybe, NUM_SKILL_LEVELS);

			if (optional_string("$AI Always Links Energy Weapons:"))
				parse_float_list(profile->link_energy_levels_always, NUM_SKILL_LEVELS);

			if (optional_string("$AI Maybe Links Energy Weapons:"))
				parse_float_list(profile->link_energy_levels_maybe, NUM_SKILL_LEVELS);

			if (optional_string("$Max Missles Locked on Player:"))
				parse_int_list(profile->max_allowed_player_homers, NUM_SKILL_LEVELS);

			if (optional_string("$Max Player Attackers:"))
				parse_int_list(profile->max_attackers, NUM_SKILL_LEVELS);

			if (optional_string("$Max Incoming Asteroids:"))
				parse_int_list(profile->max_incoming_asteroids, NUM_SKILL_LEVELS);

			if (optional_string("$Player Damage Factor:") || optional_string("$AI Damage Reduction to Player Hull:"))
				parse_float_list(profile->player_damage_scale, NUM_SKILL_LEVELS);

			// represented in fractions of F1_0
			if (optional_string("$Predict Position Delay:"))
			{
				int i;
				float temp_list[NUM_SKILL_LEVELS];

				parse_float_list(temp_list, NUM_SKILL_LEVELS);

				for (i = 0; i < NUM_SKILL_LEVELS; i++)
					profile->predict_position_delay[i] = fl2f(temp_list[i]);
			}

			if (optional_string("$Player Shield Recharge Scale:"))
				parse_float_list(profile->shield_energy_scale, NUM_SKILL_LEVELS);

			if (optional_string("$AI Shield Manage Delay:") || optional_string("$AI Shield Manage Delays:"))
				parse_float_list(profile->shield_manage_delay, NUM_SKILL_LEVELS);

			if (optional_string("$Friendly AI Fire Delay Scale:"))
				parse_float_list(profile->ship_fire_delay_scale_friendly, NUM_SKILL_LEVELS);

			if (optional_string("$Hostile AI Fire Delay Scale:"))
				parse_float_list(profile->ship_fire_delay_scale_hostile, NUM_SKILL_LEVELS);

			if (optional_string("$Player Subsys Damage Factor:") || optional_string("$AI Damage Reduction to Player Subsys:"))
				parse_float_list(profile->subsys_damage_scale, NUM_SKILL_LEVELS);

			if (optional_string("$AI Turn Time Scale:"))
				parse_float_list(profile->turn_time_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Player Weapon Recharge Scale:"))
				parse_float_list(profile->weapon_energy_scale, NUM_SKILL_LEVELS);

			set_flag(profile, "$big ships can attack beam turrets on untargeted ships:", AIPF_BIG_SHIPS_CAN_ATTACK_BEAM_TURRETS_ON_UNTARGETED_SHIPS);

			set_flag(profile, "$ai-chase allows rearm:", AIPF_AI_CHASE_ALLOWS_REARM);

			set_flag(profile, "$ai-chase disables dynamic targeting temporarily:", AIPF_AI_CHASE_DISABLES_DYNAMIC_TARGETING_TEMPORARILY);

			set_flag(profile, "$smart primary weapon selection:", AIPF_SMART_PRIMARY_WEAPON_SELECTION);

			set_flag(profile, "$smart secondary weapon selection:", AIPF_SMART_SECONDARY_WEAPON_SELECTION);

			set_flag(profile, "$smart shield management:", AIPF_SMART_SHIELD_MANAGEMENT);

			set_flag(profile, "$allow rapid secondary dumbfire:", AIPF_ALLOW_RAPID_SECONDARY_DUMBFIRE);
			
			set_flag(profile, "$huge turret weapons ignore bombs:", AIPF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS);

			set_flag(profile, "$don't insert random turret fire delay:", AIPF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY);

			set_flag(profile, "$hack improve non-homing swarm turret fire accuracy:", AIPF_HACK_IMPROVE_NON_HOMING_SWARM_TURRET_FIRE_ACCURACY);

			set_flag(profile, "$shockwaves damage small ship subsystems:", AIPF_SHOCKWAVES_DAMAGE_SMALL_SHIP_SUBSYSTEMS);

			set_flag(profile, "$navigation subsystem governs warpout capability:", AIPF_NAVIGATION_SUBSYS_GOVERNS_WARP);

			// find next valid option
			skip_to_start_of_string_either("$", "#");
		}
	}
	
	required_string("#End");

	// set default if specified
	int temp = ai_profile_lookup(default_profile_name);
	if (temp >= 0)
		Default_ai_profile = temp;
}

int ai_profile_lookup(char *name)
{
	for (int i = 0; i < Num_ai_profiles; i++)
		if (!stricmp(name, Ai_profiles[i].profile_name))
			return i;

	return -1;
}
