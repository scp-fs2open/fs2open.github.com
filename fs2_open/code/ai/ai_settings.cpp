/*
 * ai_settings.cpp
 *
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/ai/ai_settings.cpp $
 * $Revision: 1.2 $
 * $Date: 2005-11-21 00:52:19 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/11/21 00:50:02  Goober5000
 * add ai_settings.tbl
 * --Goober5000
 *
 */


#include "ai/ai_settings.h"
#include "cfile/cfile.h"
#include "parse/parselo.h"


int Num_ai_settings;
int Default_ai_setting;
ai_setting Ai_settings[MAX_AI_SETTINGS];


// utility
void skill_level_array_copy(int dest[], int src[])
{
	memcpy(dest, src, NUM_SKILL_LEVELS * sizeof(int));
}

// utility
void skill_level_array_copy(fix dest[], fix src[])
{
	memcpy(dest, src, NUM_SKILL_LEVELS * sizeof(fix));
}

// utility
void skill_level_array_copy(float dest[], float src[])
{
	memcpy(dest, src, NUM_SKILL_LEVELS * sizeof(float));
}

// utility
void set_flag(ai_setting *setting, char *name, int flag)
{
	if (optional_string(name))
	{
		bool val;
		stuff_boolean(&val);

		if (val)
			setting->flags |= flag;
		else
			setting->flags &= ~flag;
	}
}

// This function initializes the given setting with the default data.
// We need to do it this way instead of through a default table because
// 1) the default settings always need to be available; and 2) the
// other settings inherit their unspecified options from this setting,
// which means it must be initialized before any others.
void init_setting_to_default(ai_setting *setting)
{
	strcpy(setting->setting_name, "FS2 RETAIL");

	setting->flags = 0;

	int	max_incoming_asteroids[NUM_SKILL_LEVELS] = {3, 4, 5, 7, 10};
	skill_level_array_copy(setting->max_incoming_asteroids, max_incoming_asteroids);

	int	max_allowed_player_homers[NUM_SKILL_LEVELS] = {2, 3, 4, 7, 99};
	skill_level_array_copy(setting->max_allowed_player_homers, max_allowed_player_homers);

	int	max_attackers[NUM_SKILL_LEVELS] = {2, 3, 4, 5, 99};
	skill_level_array_copy(setting->max_attackers, max_attackers);

	fix predict_position_delay[NUM_SKILL_LEVELS] = {2*F1_0, 3*F1_0/2, 4*F1_0/3, F1_0/2, 0};
	skill_level_array_copy(setting->predict_position_delay, predict_position_delay);

	float cmeasure_life_scale[NUM_SKILL_LEVELS] = {3.0f, 2.0f, 1.5f, 1.25f, 1.0f};
	skill_level_array_copy(setting->cmeasure_life_scale, cmeasure_life_scale);

	float weapon_energy_scale[NUM_SKILL_LEVELS] = {10.0f, 4.0f, 2.5f, 2.0f, 1.5f};
	skill_level_array_copy(setting->weapon_energy_scale, weapon_energy_scale);

	float shield_energy_scale[NUM_SKILL_LEVELS] = {4.0f, 2.0f, 1.5f, 1.25f, 1.0f};
	skill_level_array_copy(setting->shield_energy_scale, shield_energy_scale);

	float afterburner_recharge_scale[NUM_SKILL_LEVELS] = {5.0f, 3.0f, 2.0f, 1.5f, 1.0f};
	skill_level_array_copy(setting->afterburner_recharge_scale, afterburner_recharge_scale);

	float player_damage_scale[NUM_SKILL_LEVELS] = {0.25f, 0.5f, 0.65f, 0.85f, 1.0f};
	skill_level_array_copy(setting->player_damage_scale, player_damage_scale);

	float subsys_damage_scale[NUM_SKILL_LEVELS] = {0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
	skill_level_array_copy(setting->subsys_damage_scale, subsys_damage_scale);

	float beam_friendly_damage_cap[NUM_SKILL_LEVELS] = { 0.0f, 5.0f, 10.0f, 20.0f, 30.0f };
	skill_level_array_copy(setting->beam_friendly_damage_cap, beam_friendly_damage_cap);

	float turn_time_scale[NUM_SKILL_LEVELS] = {3.0f, 2.2f, 1.6f, 1.3f, 1.0f};
	skill_level_array_copy(setting->turn_time_scale, turn_time_scale);

	float link_energy_levels_always[NUM_SKILL_LEVELS] = {100.0f, 80.0f, 60.0f, 40.0f, 20.0f};
	skill_level_array_copy(setting->link_energy_levels_always, link_energy_levels_always);

	float link_energy_levels_maybe[NUM_SKILL_LEVELS] = {90.0f, 60.0f, 40.0f, 20.0f, 10.0f};
	skill_level_array_copy(setting->link_energy_levels_maybe, link_energy_levels_maybe);

	float link_ammo_levels_always[NUM_SKILL_LEVELS] = {95.0f, 80.0f, 60.0f, 40.0f, 20.0f};
	skill_level_array_copy(setting->link_ammo_levels_always, link_ammo_levels_always);

	float link_ammo_levels_maybe[NUM_SKILL_LEVELS] = {90.0f, 60.0f, 40.0f, 20.0f, 10.0f};
	skill_level_array_copy(setting->link_ammo_levels_maybe, link_ammo_levels_maybe);

	float in_range_time[NUM_SKILL_LEVELS] = {2.0f, 1.4f, 0.75f, 0.0f, -1.0f};
	skill_level_array_copy(setting->in_range_time, in_range_time);

	float cmeasure_fire_chance[NUM_SKILL_LEVELS] = {0.2f, 0.3f, 0.5f, 0.9f, 1.1f};
	skill_level_array_copy(setting->cmeasure_fire_chance, cmeasure_fire_chance);

	float shield_manage_delay[NUM_SKILL_LEVELS] = {5.0f, 4.0f, 2.5f, 1.2f, 0.1f};
	skill_level_array_copy(setting->shield_manage_delay, shield_manage_delay);

	float ship_fire_delay_scale_hostile[NUM_SKILL_LEVELS] = {4.0f, 2.5f, 1.75f, 1.25f, 1.0f};
	skill_level_array_copy(setting->ship_fire_delay_scale_hostile, ship_fire_delay_scale_hostile);

	float ship_fire_delay_scale_friendly[NUM_SKILL_LEVELS] = {2.0f, 1.4f, 1.25f, 1.1f, 1.0f};
	skill_level_array_copy(setting->ship_fire_delay_scale_friendly, ship_fire_delay_scale_friendly);
}

// This function parses the data from the ai_settings.tbl

void ai_settings_init()
{
	char default_setting_name[NAME_LENGTH];

	// init retail entry first
	init_setting_to_default(&Ai_settings[0]);
	Default_ai_setting = 0;
	Num_ai_settings = 1;

	// Goober5000 - check for table file
	CFILE *sdt = cfopen("ai_settings.tbl", "rb");
	int table_exists = (sdt != NULL);
	if (table_exists)
		cfclose(sdt);

	// Goober5000 - if table doesn't exist, just use the defaults
	if (table_exists)
	{
		read_file_text("ai_settings.tbl");
	}
	else
	{
		nprintf(("No ai_options.tbl found; using defaults."));
		return;
	}

	reset_parse();

	required_string("#AI Settings");

	// different default?
	if (optional_string("$Default Setting:"))
		stuff_string(default_setting_name, F_NAME, NULL, NAME_LENGTH);
	else
		default_setting_name[0] = '\0';

	// begin reading data
	while (required_string_either("#End","$Setting Name:"))
	{
		ai_setting *setting;
		
		// make sure we're under the limit
		if (Num_ai_settings >= MAX_AI_SETTINGS)
		{
			Warning(LOCATION, "Too many settings in ai_settings.tbl!  Max is %d.\n", MAX_AI_SETTINGS-1);
			skip_to_string("#End", NULL);
			break;
		}

		setting = &Ai_settings[Num_ai_settings];
		Num_ai_settings++;

		// initialize
		memcpy(setting, &Ai_settings[0], sizeof(ai_setting));

		// get the name
		required_string("$Setting Name:");
		stuff_string(setting->setting_name, F_NAME, NULL, NAME_LENGTH);

		// fill in any and all AI options; they're all optional and can be in any order
		while (!check_for_string("$Setting Name:") && !check_for_string("#End"))
		{
			if (optional_string("$Player Afterburner Recharge Scale:"))
				stuff_float_list(setting->afterburner_recharge_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Max Beam Friendly Fire Damage:"))
				stuff_float_list(setting->beam_friendly_damage_cap, NUM_SKILL_LEVELS);

			if (optional_string("$Player Countermeasure Life Scale:"))
				stuff_float_list(setting->cmeasure_life_scale, NUM_SKILL_LEVELS);

			if (optional_string("$AI Countermeasure Firing Chance:"))
				stuff_float_list(setting->cmeasure_fire_chance, NUM_SKILL_LEVELS);

			if (optional_string("$AI In Range Time:"))
				stuff_float_list(setting->in_range_time, NUM_SKILL_LEVELS);

			if (optional_string("$AI Always Links Ammo Weapons:"))
				stuff_float_list(setting->link_ammo_levels_always, NUM_SKILL_LEVELS);

			if (optional_string("$AI Maybe Links Ammo Weapons:"))
				stuff_float_list(setting->link_ammo_levels_maybe, NUM_SKILL_LEVELS);

			if (optional_string("$AI Always Links Energy Weapons:"))
				stuff_float_list(setting->link_energy_levels_always, NUM_SKILL_LEVELS);

			if (optional_string("$AI Maybe Links Energy Weapons:"))
				stuff_float_list(setting->link_energy_levels_maybe, NUM_SKILL_LEVELS);

			if (optional_string("$Max Missles Locked on Player:"))
				stuff_int_list(setting->max_allowed_player_homers, NUM_SKILL_LEVELS);

			if (optional_string("$Max Player Attackers:"))
				stuff_int_list(setting->max_attackers, NUM_SKILL_LEVELS);

			if (optional_string("$Max Incoming Asteroids:"))
				stuff_int_list(setting->max_incoming_asteroids, NUM_SKILL_LEVELS);

			if (optional_string("$AI Damage Reduction to Player Hull:"))
				stuff_float_list(setting->player_damage_scale, NUM_SKILL_LEVELS);

			// represented in fractions of F1_0
			if (optional_string("$Predict Position Delay:"))
			{
				int i;
				float temp_list[NUM_SKILL_LEVELS];

				stuff_float_list(temp_list, NUM_SKILL_LEVELS);

				for (i = 0; i < NUM_SKILL_LEVELS; i++)
					setting->predict_position_delay[i] = (long) (temp_list[i] * F1_0);
			}

			if (optional_string("$Player Shield Recharge Scale:"))
				stuff_float_list(setting->shield_energy_scale, NUM_SKILL_LEVELS);

			if (optional_string("$AI Shield Manage Delays:"))
				stuff_float_list(setting->shield_manage_delay, NUM_SKILL_LEVELS);

			if (optional_string("$Friendly AI Fire Delay Scale:"))
				stuff_float_list(setting->ship_fire_delay_scale_friendly, NUM_SKILL_LEVELS);

			if (optional_string("$Hostile AI Fire Delay Scale:"))
				stuff_float_list(setting->ship_fire_delay_scale_hostile, NUM_SKILL_LEVELS);

			if (optional_string("$AI Damage Reduction to Player Subsys:"))
				stuff_float_list(setting->subsys_damage_scale, NUM_SKILL_LEVELS);

			if (optional_string("$AI Turn Time Scale:"))
				stuff_float_list(setting->turn_time_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Player Weapon Recharge Scale:"))
				stuff_float_list(setting->weapon_energy_scale, NUM_SKILL_LEVELS);

			set_flag(setting, "$big ships can attack beam turrets on untargeted ships:", AIOF_BIG_SHIPS_CAN_ATTACK_BEAM_TURRETS_ON_UNTARGETED_SHIPS);

			set_flag(setting, "$ai-chase allows rearm:", AIOF_AI_CHASE_ALLOWS_REARM);

			set_flag(setting, "$ai-chase disables dynamic targeting temporarily:", AIOF_AI_CHASE_DISABLES_DYNAMIC_TARGETING_TEMPORARILY);

			set_flag(setting, "$smart primary weapon selection:", AIOF_SMART_PRIMARY_WEAPON_SELECTION);

			set_flag(setting, "$smart secondary weapon selection:", AIOF_SMART_SECONDARY_WEAPON_SELECTION);

			set_flag(setting, "$smart shield management:", AIOF_SMART_SHIELD_MANAGEMENT);

			set_flag(setting, "$allow rapid secondary dumbfire:", AIOF_ALLOW_RAPID_SECONDARY_DUMBFIRE);
			
			set_flag(setting, "$huge turret weapons ignore bombs:", AIOF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS);

			set_flag(setting, "$don't insert random turret fire delay:", AIOF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY);

			set_flag(setting, "$hack improve non-homing swarm turret fire accuracy:", AIOF_HACK_IMPROVE_NON_HOMING_SWARM_TURRET_FIRE_ACCURACY);

			// find next valid option
			skip_to_start_of_string_either("$", "#");
		}
	}
	
	required_string("#End");

	// set default if specified
	int temp = ai_setting_lookup(default_setting_name);
	if (temp >= 0)
		Default_ai_setting = temp;
}

int ai_setting_lookup(char *name)
{
	for (int i = 0; i < Num_ai_settings; i++)
		if (!stricmp(name, Ai_settings[i].setting_name))
			return i;

	return -1;
}
