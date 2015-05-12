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
#include "weapon/weapon.h"
#include "ship/ship.h"


// global stuff
int Num_ai_profiles;
int Default_ai_profile;
ai_profile_t Ai_profiles[MAX_AI_PROFILES];

// local to this file
static int Ai_profiles_initted = 0;
static char Default_profile_name[NAME_LENGTH];

flag_def_list_new<AI::Profile_flags> AI_Profile_Flags[] = {
	{ "$big ships can attack beam turrets on untargeted ships:", AI::Profile_flags::Big_ships_can_attack_beam_turrets_on_untargeted_ships, true, false },
	{ "$smart primary weapon selection:", AI::Profile_flags::Smart_primary_weapon_selection, true, false },
	{ "$smart secondary weapon selection:", AI::Profile_flags::Smart_secondary_weapon_selection, true, false },
	{ "$smart shield management:", AI::Profile_flags::Smart_shield_management, true, false},
	{ "$smart afterburner management:", AI::Profile_flags::Smart_afterburner_management, true, false },
	{ "$allow rapid secondary dumbfire:", AI::Profile_flags::Allow_rapid_secondary_dumbfire, true, false },
	{ "$huge turret weapons ignore bombs:", AI::Profile_flags::Huge_turret_weapons_ignore_bombs, true, false },
	{ "$don't insert random turret fire delay:", AI::Profile_flags::Dont_insert_random_turret_fire_delay, true, false },
	{ "$hack improve non-homing swarm turret fire accuracy:", AI::Profile_flags::Hack_improve_non_homing_swarm_turret_fire_accuracy, true, false },
	{ "$shockwaves damage small ship subsystems:", AI::Profile_flags::Shockwaves_damage_small_ship_subsystems, true, false },
	{ "$navigation subsystem governs warpout capability:", AI::Profile_flags::Navigation_subsys_governs_warp, true, false },
	{ "$ignore lower bound for minimum speed of docked ship:", AI::Profile_flags::No_min_dock_speed_cap, true, false},
	{ "$disable linked fire penalty:", AI::Profile_flags::Disable_linked_fire_penalty, true, false },
	{ "$disable weapon damage scaling :", AI::Profile_flags::Disable_weapon_damage_scaling, true, false },
	{ "$use additive weapon velocity:", AI::Profile_flags::Use_additive_weapon_velocity, true, false },
	{ "$use newtonian dampening:", AI::Profile_flags::Use_newtonian_dampening, true, false },
	{ "$include beams for kills and assists:", AI::Profile_flags::Include_beams_in_stat_calcs, true, false },
	{ "$score kills based on damage caused:", AI::Profile_flags::Kill_scoring_scales_with_damage, true, false },
	{ "$score assists based on damage caused:", AI::Profile_flags::Assist_scoring_scales_with_damage, true, false },
	{ "$allow event and goal scoring in multiplayer:", AI::Profile_flags::Allow_multi_event_scoring, true, false },
	{ "$fix linked primary weapon decision bug:", AI::Profile_flags::Fix_linked_primary_bug, true, false },
	{ "$prevent turrets targeting too distant bombs:", AI::Profile_flags::Prevent_targeting_bombs_beyond_range, true, false },
	{ "$smart subsystem targeting for turrets:", AI::Profile_flags::Smart_subsystem_targeting_for_turrets, true, false },
	{ "$fix heat seekers homing on stealth ships bug:", AI::Profile_flags::Fix_heat_seeker_stealth_bug, true, false },
	{ "$multi allow empty primaries:", AI::Profile_flags::Multi_allow_empty_primaries, true, false },
	{ "$multi allow empty secondaries:", AI::Profile_flags::Multi_allow_empty_secondaries, true, false },
	{ "$allow turrets target weapons freely:", AI::Profile_flags::Allow_turrets_target_weapons_freely, true, false },
	{ "$use only single fov for turrets:", AI::Profile_flags::Use_only_single_fov_for_turrets, true, false },
	{ "$allow vertical dodge:", AI::Profile_flags::Allow_vertical_dodge, true, false },
	{ "$force beam turrets to use normal fov", AI::Profile_flags::Force_beam_turret_fov, true, false },
	{ "$fix ai class bug:", AI::Profile_flags::Fix_ai_class_bug, true, false },
	{ "$turrets ignore targets radius in range checks:", AI::Profile_flags::Turrets_ignore_target_radius, true, false },
	{ "$no extra collision avoidance vs player:", AI::Profile_flags::No_special_player_avoid, true, false },
	{ "$perform fewer checks for death screams:", AI::Profile_flags::Perform_fewer_scream_checks, true, false },
	{ "$advanced turret fov edge checks:", AI::Profile_flags::Advanced_turret_fov_edge_checks, true, false },
	{ "$require turrets to have target in fov:", AI::Profile_flags::Require_turret_to_have_target_in_fov, true, false },
	{ "$all ships manage shields:", AI::Profile_flags::All_ships_manage_shields, true, false },
	{ "$ai aims from ship center:", AI::Profile_flags::Ai_aims_from_ship_center, true, false },
	{ "$allow primary link at mission start:", AI::Profile_flags::Allow_primary_link_at_start, true, false },
	{ "$allow beams to damage bombs:", AI::Profile_flags::Beams_damage_weapons, true, false },
	{ "$disable weapon damage scaling for player:", AI::Profile_flags::Player_weapon_scale_fix, true, false },
	{ "$countermeasures affect aspect seekers:", AI::Profile_flags::Aspect_lock_countermeasure, true, false },
	{ "$ai guards specific ship in wing:", AI::Profile_flags::Ai_guards_specific_ship_in_wing, true, false }
};

static const size_t num_ai_profile_flags = sizeof(AI_Profile_Flags) / sizeof(flag_def_list_new<AI::Profile_flags>);

char *AI_path_types[] = {
	"normal",
	"alt1",
};

int Num_ai_path_types = sizeof(AI_path_types)/sizeof(char*);

int ai_path_type_match(char *p)
{
	int i;
	for(i = 0; i < Num_ai_path_types; i++)
	{
		if(!stricmp(AI_path_types[i], p))
			return i;
	}

	return -1;
}

void parse_ai_profiles_tbl(const char *filename)
{
	int i;
	char profile_name[NAME_LENGTH];
	ai_profile_t dummy_profile;
	char *saved_Mp = NULL;
	char buf[NAME_LENGTH];

	try
	{
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
		while (required_string_either("#End", "$Profile Name:"))
	{
		ai_profile_t *profile = &dummy_profile;
		ai_profile_t *previous_profile = NULL;
		bool no_create = false;
		
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
						Warning(LOCATION, "Too many profiles in ai_profiles.tbl!  Max is %d.\n", MAX_AI_PROFILES - 1);	// -1 because one is built-in
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
		strcpy_s(profile->profile_name, profile_name);


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

			if (optional_string("$Primary Ammo Burst Multiplier:"))
				parse_float_list(profile->primary_ammo_burst_mult, NUM_SKILL_LEVELS);

			if (optional_string("$AI Always Links Energy Weapons:"))
				parse_float_list(profile->link_energy_levels_always, NUM_SKILL_LEVELS);

			if (optional_string("$AI Maybe Links Energy Weapons:"))
				parse_float_list(profile->link_energy_levels_maybe, NUM_SKILL_LEVELS);

			if (optional_string("$Max Missles Locked on Player:") || optional_string("$Max Missiles Locked on Player:"))
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

			if (optional_string("$AI Turn Time Scale:"))
				parse_float_list(profile->turn_time_scale, NUM_SKILL_LEVELS);

			if (optional_string("$Glide Attack Percent:")) {
				parse_float_list(profile->glide_attack_percent, NUM_SKILL_LEVELS);
				//Percent is nice for modders, but here in the code we want it betwwen 0 and 1.0
				//While we're at it, verify the range
				for (i = 0; i < NUM_SKILL_LEVELS; i++) {
					if (profile->glide_attack_percent[i] < 0.0f || profile->glide_attack_percent[i] > 100.0f) {
						Warning(LOCATION, "$Glide Attack Percent should be between 0 and 100.0 (read %f). Setting to 0.", profile->glide_attack_percent[i]);
						profile->glide_attack_percent[i] = 0.0f;
					}
					profile->glide_attack_percent[i] /= 100.0;
				}
			}

			if (optional_string("$Circle Strafe Percent:")) {
				parse_float_list(profile->circle_strafe_percent, NUM_SKILL_LEVELS);
				//Percent is nice for modders, but here in the code we want it betwwen 0 and 1.0
				//While we're at it, verify the range
				for (i = 0; i < NUM_SKILL_LEVELS; i++) {
					if (profile->circle_strafe_percent[i] < 0.0f || profile->circle_strafe_percent[i] > 100.0f) {
						Warning(LOCATION, "$Circle Strafe Percent should be between 0 and 100.0 (read %f). Setting to 0.", profile->circle_strafe_percent[i]);
						profile->circle_strafe_percent[i] = 0.0f;
					}
					profile->circle_strafe_percent[i] /= 100.0;
				}
			}

			if (optional_string("$Glide Strafe Percent:")) {
				parse_float_list(profile->glide_strafe_percent, NUM_SKILL_LEVELS);
				//Percent is nice for modders, but here in the code we want it betwwen 0 and 1.0
				//While we're at it, verify the range
				for (i = 0; i < NUM_SKILL_LEVELS; i++) {
					if (profile->glide_strafe_percent[i] < 0.0f || profile->glide_strafe_percent[i] > 100.0f) {
						Warning(LOCATION, "$Glide Strafe Percent should be between 0 and 100.0 (read %f). Setting to 0.", profile->glide_strafe_percent[i]);
						profile->glide_strafe_percent[i] = 0.0f;
					}
					profile->glide_strafe_percent[i] /= 100.0;
				}
			}

			if (optional_string("$Random Sidethrust Percent:")) {
				parse_float_list(profile->random_sidethrust_percent, NUM_SKILL_LEVELS);
				//Percent is nice for modders, but here in the code we want it betwwen 0 and 1.0
				//While we're at it, verify the range
				for (i = 0; i < NUM_SKILL_LEVELS; i++) {
					if (profile->random_sidethrust_percent[i] < 0.0f || profile->random_sidethrust_percent[i] > 100.0f) {
						Warning(LOCATION, "$Random Sidethrust Percent should be between 0 and 100.0 (read %f). Setting to 0.", profile->random_sidethrust_percent[i]);
						profile->random_sidethrust_percent[i] = 0.0f;
					}
					profile->random_sidethrust_percent[i] /= 100.0;
				}
			}

			if (optional_string("$Stalemate Time Threshold:"))
				parse_float_list(profile->stalemate_time_thresh, NUM_SKILL_LEVELS);

			if (optional_string("$Stalemate Distance Threshold:"))
				parse_float_list(profile->stalemate_dist_thresh, NUM_SKILL_LEVELS);

			if (optional_string("$Player Shield Recharge Scale:"))
				parse_float_list(profile->shield_energy_scale, NUM_SKILL_LEVELS);

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

			if (optional_string("$Chance AI Has to Fire Missiles at Player:"))
				parse_int_list(profile->chance_to_use_missiles_on_plr, NUM_SKILL_LEVELS);

			if (optional_string("$Max Aim Update Delay:"))
				parse_float_list(profile->max_aim_update_delay, NUM_SKILL_LEVELS);

			if (optional_string("$Turret Max Aim Update Delay:"))
				parse_float_list(profile->turret_max_aim_update_delay, NUM_SKILL_LEVELS);

			if (optional_string("$Player Autoaim FOV:"))
			{
				float fov_list[NUM_SKILL_LEVELS];
				parse_float_list(fov_list, NUM_SKILL_LEVELS);
				for (i = 0; i < NUM_SKILL_LEVELS; i++)
				{
					//Enforce range
					if (fov_list[i] < 0.0f || fov_list[i] >= 360.0f)
					{
						Warning(LOCATION, "$Player Autoaim FOV should be >= 0 and < 360.0 (read %f). Setting to 0.", fov_list[i]);
						fov_list[i] = 0.0f;
					}

					//Convert units
					profile->player_autoaim_fov[i] = fov_list[i] * PI / 180.0f;
				}
			}

			if (optional_string("$Detail Distance Multiplier:"))
				parse_float_list(profile->detail_distance_mult, NUM_SKILL_LEVELS);

			for (int i = 0; i < num_ai_profile_flags; ++i) {
				if (optional_string(AI_Profile_Flags[i].name)) {
					bool val;
					stuff_boolean(&val);

					if (val) {
						profile->flags.set(AI_Profile_Flags[i].def);
					}
				}
			}

			profile->ai_path_mode = AI_PATH_MODE_NORMAL;
				if (optional_string("$ai path mode:"))
			{
				stuff_string(buf, F_NAME, NAME_LENGTH);
				int j = ai_path_type_match(buf);
					if (j >= 0) {
					profile->ai_path_mode = j;
					}
					else {
					Warning(LOCATION, "Invalid ai path mode '%s' specified", buf);
				}
			}

			if (optional_string("$no warp camera:")) 
			{
				bool val;
				stuff_boolean(&val);

				if (val)
					profile->flags.set(AI::Profile_flags::No_warp_camera);
			}

			if (optional_string("$fix ai path order bug:")) 
			{
				bool val;
				stuff_boolean(&val);

				if (val)
					profile->flags.set(AI::Profile_flags::Fix_ai_path_order_bug);
			}

			if (optional_string("$strict turret-tagged-only targeting:"))
			{
				bool val;
				stuff_boolean(&val);

				if (val)
					profile->flags.set(AI::Profile_flags::Strict_turred_tagged_only_targeting);
			}

			// ----------

			// compatibility
			if (optional_string("$perform less checks for death screams:"))
			{
				mprintf(("Warning: \"$perform less checks for death screams\" flag is deprecated in favor of \"$perform fewer checks for death screams\"\n"));
				bool temp;
				stuff_boolean(&temp);
				if (temp)
					profile->flags.set(AI::Profile_flags::Perform_fewer_scream_checks);
				else
					profile->flags.unset(AI::Profile_flags::Perform_fewer_scream_checks);
			}
			if (optional_string("$allow primary link delay:"))
			{
				mprintf(("Warning: \"$allow primary link delay\" flag is deprecated in favor of \"$allow primary link at mission start\"\n"));
				bool temp;
				stuff_boolean(&temp);
				if (temp)
					profile->flags.unset(AI::Profile_flags::Allow_primary_link_at_start);
				else
					profile->flags.set(AI::Profile_flags::Allow_primary_link_at_start);
			}


			// if we've been through once already and are at the same place, force a move
				if (saved_Mp && (saved_Mp == Mp))
			{
				char tmp[60];
				memset(tmp, 0, 60);
				strncpy(tmp, Mp, 59);
				mprintf(("WARNING: Unrecognized parameter in ai_profiles: %s\n", tmp));

				Mp++;
			}

			// find next valid option
			skip_to_start_of_string_either("$", "#");
			saved_Mp = Mp;
		}
	}
	
	required_string("#End");
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", (filename) ? filename : "<default ai_profiles.tbl>", e.what()));
		return;
	}

	// add tbl/tbm to multiplayer validation list
	extern void fs2netd_add_table_validation(const char *tblname);
	fs2netd_add_table_validation(filename);
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
