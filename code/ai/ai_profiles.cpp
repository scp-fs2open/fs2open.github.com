/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */




#include "globalincs/pstypes.h"
#include "globalincs/def_files.h"
#include "ai/ai_profiles.h"
#include "parse/parselo.h"
#include "localization/localize.h"


// global stuff
int Num_ai_profiles;
int Default_ai_profile;
ai_profile_t Ai_profiles[MAX_AI_PROFILES];

// local to this file
static int Ai_profiles_initted = 0;
static char Default_profile_name[NAME_LENGTH];


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

void parse_ai_profiles_tbl(char *filename)
{
	int i, rval;
	bool no_create = false;
	char profile_name[NAME_LENGTH];
	ai_profile_t dummy_profile;
	char *saved_Mp = NULL;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", (filename) ? filename : "<default ai_profiles.tbl>", rval));
		lcl_ext_close();
		return;
	}

	if (filename == NULL)
		read_file_text_from_array(defaults_get_file("ai_profiles.tbl"));
	else
		read_file_text(filename, CF_TYPE_TABLES);

	reset_parse();		


	// start parsing
	required_string("#AI Profiles");

	// new default?
	if (optional_string("$Default Profile:"))
		stuff_string(Default_profile_name, F_NAME, NAME_LENGTH);

	// begin reading data
	while (required_string_either("#End","$Profile Name:"))
	{
		ai_profile_t *profile = &dummy_profile;
		ai_profile_t *previous_profile = NULL;
		
		// get the name
		required_string("$Profile Name:");
		stuff_string(profile_name, F_NAME, NAME_LENGTH);

		// see if it exists
		for (i = 0; i < Num_ai_profiles; i++)
		{
			if (!stricmp(Ai_profiles[i].profile_name, profile_name))
			{
				previous_profile = &Ai_profiles[i];
				break;
			}
		}

		// modular table stuff
		if (optional_string("+nocreate"))
		{
			no_create = true;

			// use the previous one if possible,
			// otherwise continue to use the dummy one
			if (previous_profile != NULL)
				profile = previous_profile;
		}
		else
		{
			// don't create multiple profiles with the same name
			if (previous_profile != NULL)
			{
				Warning(LOCATION, "An ai profile named '%s' already exists!  The new one will not be created.\n", profile_name);
			}
			else
			{
				// make sure we're under the limit
				if (Num_ai_profiles >= MAX_AI_PROFILES)
				{
					Warning(LOCATION, "Too many profiles in ai_profiles.tbl!  Max is %d.\n", MAX_AI_PROFILES-1);	// -1 because one is built-in
					skip_to_string("#End", NULL);
					break;
				}

				profile = &Ai_profiles[Num_ai_profiles];
				Num_ai_profiles++;
			}
		}

		// initialize profile if we're not building from a previously parsed one
		if (!no_create)
		{
			// base profile, so zero it out
			if (profile == &Ai_profiles[0])
			{
				memset(profile, 0, sizeof(ai_profile_t));
			}
			// brand new profile, so set it to the base defaults
			else
			{
				memcpy(profile, &Ai_profiles[0], sizeof(ai_profile_t));
			}
		}

		// set the name
		strcpy(profile->profile_name, profile_name);


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

			if (optional_string("$Player Subsys Damage Factor:") || optional_string("$AI Damage Reduction to Player Subsys:"))
				parse_float_list(profile->subsys_damage_scale, NUM_SKILL_LEVELS);

			// represented in fractions of F1_0
			if (optional_string("$Predict Position Delay:"))
			{
				int iLoop;
				float temp_list[NUM_SKILL_LEVELS];

				parse_float_list(temp_list, NUM_SKILL_LEVELS);

				for (iLoop = 0; iLoop < NUM_SKILL_LEVELS; iLoop++)
					profile->predict_position_delay[iLoop] = fl2f(temp_list[iLoop]);
			}

			if (optional_string("$Player Shield Recharge Scale:"))
				parse_float_list(profile->shield_energy_scale, NUM_SKILL_LEVELS);

			if (optional_string("$AI Shield Manage Delay:") || optional_string("$AI Shield Manage Delays:"))
				parse_float_list(profile->shield_manage_delay, NUM_SKILL_LEVELS);

			if (optional_string("$Friendly AI Fire Delay Scale:"))
				parse_float_list(profile->ship_fire_delay_scale_friendly, NUM_SKILL_LEVELS);

			if (optional_string("$Hostile AI Fire Delay Scale:"))
				parse_float_list(profile->ship_fire_delay_scale_hostile, NUM_SKILL_LEVELS);

			if (optional_string("$Friendly AI Secondary Fire Delay Scale:"))
				parse_float_list(profile->ship_fire_secondary_delay_scale_friendly, NUM_SKILL_LEVELS);

			if (optional_string("$Hostile AI Secondary Fire Delay Scale:"))
				parse_float_list(profile->ship_fire_secondary_delay_scale_hostile, NUM_SKILL_LEVELS);

			if (optional_string("$Player Subsys Damage Factor:") || optional_string("$AI Damage Reduction to Player Subsys:"))
				parse_float_list(profile->subsys_damage_scale, NUM_SKILL_LEVELS);

			if (optional_string("$AI Turn Time Scale:"))
				parse_float_list(profile->turn_time_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Player Weapon Recharge Scale:"))
				parse_float_list(profile->weapon_energy_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Max Turret Target Ownage:"))
				parse_int_list(profile->max_turret_ownage_target, NUM_SKILL_LEVELS);

			if (optional_string("$Max Turret Player Ownage:"))
				parse_int_list(profile->max_turret_ownage_player, NUM_SKILL_LEVELS);

			if (optional_string("$Percentage Required For Kill Scale:"))
				parse_float_list(profile->kill_percentage_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Percentage Required For Assist Scale:"))
				parse_float_list(profile->assist_percentage_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Percentage Awarded For Capship Assist:"))
				parse_float_list(profile->assist_award_percentage_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Repair Penalty:"))
				parse_int_list(profile->repair_penalty, NUM_SKILL_LEVELS);

			if (optional_string("$Delay Before Allowing Bombs to Be Shot Down:"))
				parse_float_list(profile->delay_bomb_arm_timer, NUM_SKILL_LEVELS);

			set_flag(profile, "$big ships can attack beam turrets on untargeted ships:", AIPF_BIG_SHIPS_CAN_ATTACK_BEAM_TURRETS_ON_UNTARGETED_SHIPS);

			set_flag(profile, "$smart primary weapon selection:", AIPF_SMART_PRIMARY_WEAPON_SELECTION);

			set_flag(profile, "$smart secondary weapon selection:", AIPF_SMART_SECONDARY_WEAPON_SELECTION);

			set_flag(profile, "$smart shield management:", AIPF_SMART_SHIELD_MANAGEMENT);

			set_flag(profile, "$smart afterburner management:", AIPF_SMART_AFTERBURNER_MANAGEMENT);

			set_flag(profile, "$allow rapid secondary dumbfire:", AIPF_ALLOW_RAPID_SECONDARY_DUMBFIRE);
			
			set_flag(profile, "$huge turret weapons ignore bombs:", AIPF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS);

			set_flag(profile, "$don't insert random turret fire delay:", AIPF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY);

			set_flag(profile, "$hack improve non-homing swarm turret fire accuracy:", AIPF_HACK_IMPROVE_NON_HOMING_SWARM_TURRET_FIRE_ACCURACY);

			set_flag(profile, "$shockwaves damage small ship subsystems:", AIPF_SHOCKWAVES_DAMAGE_SMALL_SHIP_SUBSYSTEMS);

			set_flag(profile, "$navigation subsystem governs warpout capability:", AIPF_NAVIGATION_SUBSYS_GOVERNS_WARP);

			set_flag(profile, "$ignore lower bound for minimum speed of docked ship:", AIPF_NO_MIN_DOCK_SPEED_CAP);

			set_flag(profile, "$disable linked fire penalty:", AIPF_DISABLE_LINKED_FIRE_PENALTY);

			set_flag(profile, "$disable weapon damage scaling:", AIPF_DISABLE_WEAPON_DAMAGE_SCALING);

			set_flag(profile, "$use additive weapon velocity:", AIPF_USE_ADDITIVE_WEAPON_VELOCITY);

			set_flag(profile, "$use newtonian dampening:", AIPF_USE_NEWTONIAN_DAMPENING);

			set_flag(profile, "$include beams for kills and assists:", AIPF_INCLUDE_BEAMS_IN_STAT_CALCS);

			set_flag(profile, "$score kills based on damage caused:", AIPF_KILL_SCORING_SCALES_WITH_DAMAGE);

			set_flag(profile, "$score assists based on damage caused:", AIPF_ASSIST_SCORING_SCALES_WITH_DAMAGE);

			set_flag(profile, "$allow event and goal scoring in multiplayer:", AIPF_ALLOW_MULTI_EVENT_SCORING);

			set_flag(profile, "$fix linked primary weapon decision bug:", AIPF_FIX_LINKED_PRIMARY_BUG);

			set_flag(profile, "$prevent turrets targeting too distant bombs:", AIPF_PREVENT_TARGETING_BOMBS_BEYOND_RANGE);

			set_flag(profile, "$smart subsystem targeting for turrets:", AIPF_SMART_SUBSYSTEM_TARGETING_FOR_TURRETS);

			set_flag(profile, "$fix heat seekers homing on stealth ships bug:", AIPF_FIX_HEAT_SEEKER_STEALTH_BUG);

			set_flag(profile, "$multi allow empty primaries:", AIPF_MULTI_ALLOW_EMPTY_PRIMARIES);

			set_flag(profile, "$multi allow empty secondaries:", AIPF_MULTI_ALLOW_EMPTY_SECONDARIES);

			set_flag(profile, "$allow turrets target weapons freely:", AIPF_ALLOW_TURRETS_TARGET_WEAPONS_FREELY);

			// if we've been through once already and are at the same place, force a move
			if ( saved_Mp && (saved_Mp == Mp) )
				Mp++;

			// find next valid option
			skip_to_start_of_string_either("$", "#");
			saved_Mp = Mp;
		}
	}
	
	required_string("#End");

	// add tbl/tbm to multiplayer validation list
	extern void fs2netd_add_table_validation(char *tblname);
	fs2netd_add_table_validation(filename);

	// close localization
	lcl_ext_close();
}

void ai_profiles_init()
{
	int temp;

	if (Ai_profiles_initted)
		return;

	Num_ai_profiles = 0;
	Default_ai_profile = 0;
	Default_profile_name[0] = '\0';

	// init retail entry first
	parse_ai_profiles_tbl(NULL);

	// now parse the supplied table (if any)
	if (cf_exists_full("ai_profiles.tbl", CF_TYPE_TABLES))
		parse_ai_profiles_tbl("ai_profiles.tbl");

	// parse any modular tables
	parse_modular_table("*-aip.tbm", parse_ai_profiles_tbl);

	// set default if specified
	temp = ai_profile_lookup(Default_profile_name);
	if (temp >= 0)
		Default_ai_profile = temp;

	Ai_profiles_initted = 1;
}

int ai_profile_lookup(char *name)
{
	for (int i = 0; i < Num_ai_profiles; i++)
		if (!stricmp(name, Ai_profiles[i].profile_name))
			return i;

	return -1;
}
