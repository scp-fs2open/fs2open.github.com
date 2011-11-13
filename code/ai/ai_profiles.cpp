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


// utility
void set_flag(ai_profile_t *profile, char *name, int flag, int type)
{
	if (optional_string(name))
	{
		bool val;
		stuff_boolean(&val);

		if (type == AIP_FLAG) {
			if (val)
				profile->flags |= flag;
			else
				profile->flags &= ~flag;
		} else {
			if (val)
				profile->flags2 |= flag;
			else
				profile->flags2 &= ~flag;
		}
	}
}

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

void parse_ai_profiles_tbl(char *filename)
{
	int i, rval;
	char profile_name[NAME_LENGTH];
	ai_profile_t dummy_profile;
	char *saved_Mp = NULL;
	char buf[NAME_LENGTH];

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

			set_flag(profile, "$big ships can attack beam turrets on untargeted ships:", AIPF_BIG_SHIPS_CAN_ATTACK_BEAM_TURRETS_ON_UNTARGETED_SHIPS, AIP_FLAG);

			set_flag(profile, "$smart primary weapon selection:", AIPF_SMART_PRIMARY_WEAPON_SELECTION, AIP_FLAG);

			set_flag(profile, "$smart secondary weapon selection:", AIPF_SMART_SECONDARY_WEAPON_SELECTION, AIP_FLAG);

			set_flag(profile, "$smart shield management:", AIPF_SMART_SHIELD_MANAGEMENT, AIP_FLAG);

			set_flag(profile, "$smart afterburner management:", AIPF_SMART_AFTERBURNER_MANAGEMENT, AIP_FLAG);

			set_flag(profile, "$allow rapid secondary dumbfire:", AIPF_ALLOW_RAPID_SECONDARY_DUMBFIRE, AIP_FLAG);
			
			set_flag(profile, "$huge turret weapons ignore bombs:", AIPF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS, AIP_FLAG);

			set_flag(profile, "$don't insert random turret fire delay:", AIPF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY, AIP_FLAG);

			set_flag(profile, "$hack improve non-homing swarm turret fire accuracy:", AIPF_HACK_IMPROVE_NON_HOMING_SWARM_TURRET_FIRE_ACCURACY, AIP_FLAG);

			set_flag(profile, "$shockwaves damage small ship subsystems:", AIPF_SHOCKWAVES_DAMAGE_SMALL_SHIP_SUBSYSTEMS, AIP_FLAG);

			set_flag(profile, "$navigation subsystem governs warpout capability:", AIPF_NAVIGATION_SUBSYS_GOVERNS_WARP, AIP_FLAG);

			set_flag(profile, "$ignore lower bound for minimum speed of docked ship:", AIPF_NO_MIN_DOCK_SPEED_CAP, AIP_FLAG);

			set_flag(profile, "$disable linked fire penalty:", AIPF_DISABLE_LINKED_FIRE_PENALTY, AIP_FLAG);

			set_flag(profile, "$disable weapon damage scaling:", AIPF_DISABLE_WEAPON_DAMAGE_SCALING, AIP_FLAG);

			set_flag(profile, "$use additive weapon velocity:", AIPF_USE_ADDITIVE_WEAPON_VELOCITY, AIP_FLAG);

			set_flag(profile, "$use newtonian dampening:", AIPF_USE_NEWTONIAN_DAMPENING, AIP_FLAG);

			set_flag(profile, "$include beams for kills and assists:", AIPF_INCLUDE_BEAMS_IN_STAT_CALCS, AIP_FLAG);

			set_flag(profile, "$score kills based on damage caused:", AIPF_KILL_SCORING_SCALES_WITH_DAMAGE, AIP_FLAG);

			set_flag(profile, "$score assists based on damage caused:", AIPF_ASSIST_SCORING_SCALES_WITH_DAMAGE, AIP_FLAG);

			set_flag(profile, "$allow event and goal scoring in multiplayer:", AIPF_ALLOW_MULTI_EVENT_SCORING, AIP_FLAG);

			set_flag(profile, "$fix linked primary weapon decision bug:", AIPF_FIX_LINKED_PRIMARY_BUG, AIP_FLAG);

			set_flag(profile, "$prevent turrets targeting too distant bombs:", AIPF_PREVENT_TARGETING_BOMBS_BEYOND_RANGE, AIP_FLAG);

			set_flag(profile, "$smart subsystem targeting for turrets:", AIPF_SMART_SUBSYSTEM_TARGETING_FOR_TURRETS, AIP_FLAG);

			set_flag(profile, "$fix heat seekers homing on stealth ships bug:", AIPF_FIX_HEAT_SEEKER_STEALTH_BUG, AIP_FLAG);

			set_flag(profile, "$multi allow empty primaries:", AIPF_MULTI_ALLOW_EMPTY_PRIMARIES, AIP_FLAG);

			set_flag(profile, "$multi allow empty secondaries:", AIPF_MULTI_ALLOW_EMPTY_SECONDARIES, AIP_FLAG);

			set_flag(profile, "$allow turrets target weapons freely:", AIPF_ALLOW_TURRETS_TARGET_WEAPONS_FREELY, AIP_FLAG);

			set_flag(profile, "$use only single fov for turrets:", AIPF_USE_ONLY_SINGLE_FOV_FOR_TURRETS, AIP_FLAG);

			set_flag(profile, "$allow vertical dodge:", AIPF_ALLOW_VERTICAL_DODGE, AIP_FLAG);

			set_flag(profile, "$force beam turrets to use normal fov:", AIPF_FORCE_BEAM_TURRET_FOV, AIP_FLAG);

			set_flag(profile, "$fix ai class bug:", AIPF_FIX_AI_CLASS_BUG, AIP_FLAG);

			set_flag(profile, "$turrets ignore targets radius in range checks:", AIPF2_TURRETS_IGNORE_TARGET_RADIUS, AIP_FLAG2);

			set_flag(profile, "$no extra collision avoidance vs player:", AIPF2_NO_SPECIAL_PLAYER_AVOID, AIP_FLAG2);

			set_flag(profile, "$perform less checks for death screams:", AIPF2_PERFORM_LESS_SCREAM_CHECKS, AIP_FLAG2);

			set_flag(profile, "$advanced turret fov edge checks:", AIPF2_ADVANCED_TURRET_FOV_EDGE_CHECKS, AIP_FLAG2);

			set_flag(profile, "$require turrets to have target in fov:", AIPF2_REQUIRE_TURRET_TO_HAVE_TARGET_IN_FOV, AIP_FLAG2);

			set_flag(profile, "$all ships manage shields:", AIPF2_ALL_SHIPS_MANAGE_SHIELDS, AIP_FLAG2);

			set_flag(profile, "$ai aims from ship center:", AIPF2_AI_AIMS_FROM_SHIP_CENTER, AIP_FLAG2);

			set_flag(profile, "$allow primary link delay:", AIPF2_ALLOW_PRIMARY_LINK_DELAY, AIP_FLAG2);

			set_flag(profile, "$allow beams to damage bombs:", AIPF2_BEAMS_DAMAGE_WEAPONS, AIP_FLAG2);

			set_flag(profile, "$disable weapon damage scaling for player:", AIPF2_PLAYER_WEAPON_SCALE_FIX, AIP_FLAG2);

			profile->ai_path_mode = AI_PATH_MODE_NORMAL;
			if(optional_string("$ai path mode:"))
			{
				stuff_string(buf, F_NAME, NAME_LENGTH);
				int j = ai_path_type_match(buf);
				if(j >= 0) {
					profile->ai_path_mode = j;
				} else {
					Warning(LOCATION, "Invalid ai path mode '%s' specified", buf);
				}
			}

			if (optional_string("$Default weapon select effect:")) {
				char effect[NAME_LENGTH];
				stuff_string(effect, F_NAME, NAME_LENGTH);
				if (!stricmp(effect, "FS1"))
					Default_weapon_select_effect = 1;
				if (!stricmp(effect, "off"))
					Default_weapon_select_effect = 0;
			}

			if (optional_string("$Default ship select effect:")) {
				char effect[NAME_LENGTH];
				stuff_string(effect, F_NAME, NAME_LENGTH);
				if (!stricmp(effect, "FS1"))
					Default_ship_select_effect = 1;
				if (!stricmp(effect, "off"))
					Default_ship_select_effect = 0;
			}

			set_flag(profile, "$no warp camera:", AIPF2_NO_WARP_CAMERA, AIP_FLAG2);

			// if we've been through once already and are at the same place, force a move
			if ( saved_Mp && (saved_Mp == Mp) )
			{
				char buf[60];
				memset(buf, 0, 60);
				strncpy(buf, Mp, 59);
				mprintf(("WARNING: Unrecognized parameter in ai_profiles: %s\n", buf));

				Mp++;
			}

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
