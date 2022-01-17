/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */




#include "ai/ai_profiles.h"
#include "ai/aibig.h"
#include "def_files/def_files.h"
#include "globalincs/pstypes.h"
#include "localization/localize.h"
#include "mod_table/mod_table.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "weapon/weapon.h"


// global stuff
int Num_ai_profiles;
int Default_ai_profile;
ai_profile_t Ai_profiles[MAX_AI_PROFILES];

// local to this file
static int Ai_profiles_initted = 0;
static char Default_profile_name[NAME_LENGTH];


// utility
void set_flag(ai_profile_t *profile, const char *name, AI::Profile_Flags flag)
{
	if (optional_string(name))
	{
		bool val;
		stuff_boolean(&val);
        profile->flags.set(flag, val);
	}
}

const char *AI_path_types[] = {
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


const char* AI_secondary_range_aware_select_modes[] = {
	"retail",
	"aware",
};

int Num_ai_secondary_range_aware_select_modes = sizeof(AI_secondary_range_aware_select_modes) / sizeof(char*);

int ai_secondary_range_select_mode_match(char* p)
{
	int i;
	for (i = 0; i < Num_ai_secondary_range_aware_select_modes; i++)
	{
		if (!stricmp(AI_secondary_range_aware_select_modes[i], p))
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
			read_file_text_from_default(defaults_get_file("ai_profiles.tbl"));
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
                    profile->reset();
				}
				// brand new profile, so set it to the base defaults
				else
				{
                    *profile = Ai_profiles[0];
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
					parse_float_list(profile->detail_distance_mult, MAX_DETAIL_LEVEL + 1);

				set_flag(profile, "$big ships can attack beam turrets on untargeted ships:", AI::Profile_Flags::Big_ships_can_attack_beam_turrets_on_untargeted_ships);

				set_flag(profile, "$smart primary weapon selection:", AI::Profile_Flags::Smart_primary_weapon_selection);

				set_flag(profile, "$smart secondary weapon selection:", AI::Profile_Flags::Smart_secondary_weapon_selection);

				set_flag(profile, "$smart shield management:", AI::Profile_Flags::Smart_shield_management);

				set_flag(profile, "$smart afterburner management:", AI::Profile_Flags::Smart_afterburner_management);

				set_flag(profile, "$free afterburner use:", AI::Profile_Flags::Free_afterburner_use);

				set_flag(profile, "$allow rapid secondary dumbfire:", AI::Profile_Flags::Allow_rapid_secondary_dumbfire);

				set_flag(profile, "$huge turret weapons ignore bombs:", AI::Profile_Flags::Huge_turret_weapons_ignore_bombs);

				set_flag(profile, "$don't insert random turret fire delay:", AI::Profile_Flags::Dont_insert_random_turret_fire_delay);

				set_flag(profile, "$hack improve non-homing swarm turret fire accuracy:", AI::Profile_Flags::Hack_improve_non_homing_swarm_turret_fire_accuracy);

				set_flag(profile, "$shockwaves damage small ship subsystems:", AI::Profile_Flags::Shockwaves_damage_small_ship_subsystems);

				set_flag(profile, "$navigation subsystem governs warpout capability:", AI::Profile_Flags::Navigation_subsys_governs_warp);

				set_flag(profile, "$check communications for non-player ships:", AI::Profile_Flags::Check_comms_for_non_player_ships);

				set_flag(profile, "$ignore lower bound for minimum speed of docked ship:", AI::Profile_Flags::No_min_dock_speed_cap);

				set_flag(profile, "$disable linked fire penalty:", AI::Profile_Flags::Disable_linked_fire_penalty);

				set_flag(profile, "$disable player secondary doublefire:", AI::Profile_Flags::Disable_player_secondary_doublefire);

				set_flag(profile, "$disable ai secondary doublefire:", AI::Profile_Flags::Disable_ai_secondary_doublefire);

				set_flag(profile, "$disable weapon damage scaling:", AI::Profile_Flags::Disable_weapon_damage_scaling);

				set_flag(profile, "$use additive weapon velocity:", AI::Profile_Flags::Use_additive_weapon_velocity);

				set_flag(profile, "$use newtonian dampening:", AI::Profile_Flags::Use_newtonian_dampening);

				set_flag(profile, "$include beams for kills and assists:", AI::Profile_Flags::Include_beams_in_stat_calcs);

				set_flag(profile, "$score kills based on damage caused:", AI::Profile_Flags::Kill_scoring_scales_with_damage);

				set_flag(profile, "$score assists based on damage caused:", AI::Profile_Flags::Assist_scoring_scales_with_damage);

				set_flag(profile, "$allow event and goal scoring in multiplayer:", AI::Profile_Flags::Allow_multi_event_scoring);

				set_flag(profile, "$fix linked primary weapon decision bug:", AI::Profile_Flags::Fix_linked_primary_bug);

				set_flag(profile, "$fix ramming stationary targets bug:", AI::Profile_Flags::Fix_ramming_stationary_targets_bug);

				set_flag(profile, "$prevent turrets targeting too distant bombs:", AI::Profile_Flags::Prevent_targeting_bombs_beyond_range);

				set_flag(profile, "$smart subsystem targeting for turrets:", AI::Profile_Flags::Smart_subsystem_targeting_for_turrets);

				set_flag(profile, "$fix heat seekers homing on stealth ships bug:", AI::Profile_Flags::Fix_heat_seeker_stealth_bug);

				set_flag(profile, "$multi allow empty primaries:", AI::Profile_Flags::Multi_allow_empty_primaries);

				set_flag(profile, "$multi allow empty secondaries:", AI::Profile_Flags::Multi_allow_empty_secondaries);

				set_flag(profile, "$allow turrets target weapons freely:", AI::Profile_Flags::Allow_turrets_target_weapons_freely);

				set_flag(profile, "$use only single fov for turrets:", AI::Profile_Flags::Use_only_single_fov_for_turrets);

				set_flag(profile, "$allow vertical dodge:", AI::Profile_Flags::Allow_vertical_dodge);

				set_flag(profile, "$force beam turrets to use normal fov:", AI::Profile_Flags::Force_beam_turret_fov);

				set_flag(profile, "$fix ai class bug:", AI::Profile_Flags::Fix_ai_class_bug);

				set_flag(profile, "$turrets ignore targets radius in range checks:", AI::Profile_Flags::Turrets_ignore_target_radius);

				set_flag(profile, "$no extra collision avoidance vs player:", AI::Profile_Flags::No_special_player_avoid);

				set_flag(profile, "$perform fewer checks for death screams:", AI::Profile_Flags::Perform_fewer_scream_checks);

				set_flag(profile, "$advanced turret fov edge checks:", AI::Profile_Flags::Advanced_turret_fov_edge_checks);

				set_flag(profile, "$require turrets to have target in fov:", AI::Profile_Flags::Require_turret_to_have_target_in_fov);

				set_flag(profile, "$all ships manage shields:", AI::Profile_Flags::All_ships_manage_shields);

				set_flag(profile, "$ai aims from ship center:", AI::Profile_Flags::Ai_aims_from_ship_center);

				set_flag(profile, "$allow primary link at mission start:", AI::Profile_Flags::Allow_primary_link_at_start);

				set_flag(profile, "$allow beams to damage bombs:", AI::Profile_Flags::Beams_damage_weapons);

				set_flag(profile, "$disable weapon damage scaling for player:", AI::Profile_Flags::Player_weapon_scale_fix);

				set_flag(profile, "$countermeasures affect aspect seekers:", AI::Profile_Flags::Aspect_lock_countermeasure);

				set_flag(profile, "$ai guards specific ship in wing:", AI::Profile_Flags::Ai_guards_specific_ship_in_wing);

				set_flag(profile, "$support don't add primaries:", AI::Profile_Flags::Support_dont_add_primaries);

				set_flag(profile, "$firing requires exact los:", AI::Profile_Flags::Require_exact_los);

				set_flag(profile, "$fighterbay arrivals use carrier orientation:", AI::Profile_Flags::Fighterbay_arrivals_use_carrier_orient);

				set_flag(profile, "$fighterbay departures use carrier orientation:", AI::Profile_Flags::Fighterbay_departures_use_carrier_orient);

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

				set_flag(profile, "$no warp camera:", AI::Profile_Flags::No_warp_camera);

				set_flag(profile, "$fix ai path order bug:", AI::Profile_Flags::Fix_ai_path_order_bug);

				set_flag(profile, "$strict turret-tagged-only targeting:", AI::Profile_Flags::Strict_turret_tagged_only_targeting);

				set_flag(profile, "$aspect bomb invulnerability fix:", AI::Profile_Flags::Aspect_invulnerability_fix);

				set_flag(profile, "$glide decay requires thrust:", AI::Profile_Flags::Glide_decay_requires_thrust);

				set_flag(profile, "$ai can slow down when attacking big ships:", AI::Profile_Flags::Ai_can_slow_down_attacking_big_ships);

				set_flag(profile, "$use actual primary range:", AI::Profile_Flags::Use_actual_primary_range);

				if (optional_string("$override radius for subsystem path points:")) {
					int path_radii;
					stuff_int(&path_radii);
					if (path_radii >= Minimum_subsystem_path_pt_dist) {
						profile->subsystem_path_radii = path_radii;
					} else {
						mprintf(("Warning: \"$override radius for subsystem path points:\" should be >= %i (read %i). Value will not be used.\n", Minimum_subsystem_path_pt_dist, path_radii));
					}
				}

				set_flag(profile, "$use POF radius for subsystem path points:", AI::Profile_Flags::Use_subsystem_path_point_radii);

				if (optional_string("$bay arrive speed multiplier:")) {
					stuff_float(&profile->bay_arrive_speed_mult);
				}
				if (optional_string("$bay depart speed multiplier:")) {
					stuff_float(&profile->bay_depart_speed_mult);
				}

				// ----------

				// compatibility
				if (optional_string("$perform less checks for death screams:"))
				{
					mprintf(("Warning: \"$perform less checks for death screams\" flag is deprecated in favor of \"$perform fewer checks for death screams\"\n"));
					bool temp;
					stuff_boolean(&temp);
                    profile->flags.set(AI::Profile_Flags::Perform_fewer_scream_checks, temp);
				}
				if (optional_string("$allow primary link delay:"))
				{
					mprintf(("Warning: \"$allow primary link delay\" flag is deprecated in favor of \"$allow primary link at mission start\"\n"));
					bool temp;
					stuff_boolean(&temp);
                    profile->flags.set(AI::Profile_Flags::Allow_primary_link_at_start, !temp);
				}
				if (optional_string("$lead indicator second-order prediction factor:")) {
					stuff_float(&profile->second_order_lead_predict_factor);
					if (profile->second_order_lead_predict_factor > 1 || profile->second_order_lead_predict_factor < 0) {
						mprintf(("Warning: \"$lead indicator second-order prediction factor\" must be 0 - 1, resetting to 0.\"\n"));
						profile->second_order_lead_predict_factor = 0;
					}
				}
				if (optional_string("$ships with no shields can manage ETS:"))
				{
					mprintf(("Warning: \"$ships with no shields can manage ETS\" flag is deprecated in favor of \"$any ship with no shields can manage ETS\"\n"));
					bool temp;
					stuff_boolean(&temp);
                    profile->flags.set(AI::Profile_Flags::all_nonshielded_ships_can_manage_ets, temp);
				}

                set_flag(profile, "$no directional bias for missile and ship turning:", AI::Profile_Flags::No_turning_directional_bias);

				set_flag(profile, "$respect ship axial turnrate differences:", AI::Profile_Flags::Use_axial_turnrate_differences);

				set_flag(profile, "$any ship with no shields can manage ETS:", AI::Profile_Flags::all_nonshielded_ships_can_manage_ets);

				set_flag(profile, "$fighters/bombers with no shields can manage ETS:", AI::Profile_Flags::fightercraft_nonshielded_ships_can_manage_ets);

				set_flag(profile, "$better combat collision avoidance for fightercraft:", AI::Profile_Flags::Better_collision_avoidance);

				set_flag(profile, "$improved missile avoidance for fightercraft:", AI::Profile_Flags::Improved_missile_avoidance);

				set_flag(profile, "$friendly ships use AI profile countermeasure chance:", AI::Profile_Flags::Friendlies_use_countermeasure_firechance);

				set_flag(profile, "$improved subsystem attack pathing:", AI::Profile_Flags::Improved_subsystem_attack_pathing);

				set_flag(profile, "$fixed ship-weapon collisions:", AI::Profile_Flags::Fixed_ship_weapon_collision);

				//Intention is to expand this feature to include a preference for close or long range weapons
				//hence using something besides a simple flag.
				if (optional_string("$AI secondary range awareness:"))
				{
					stuff_string(buf, F_NAME, NAME_LENGTH);
					int j = ai_secondary_range_select_mode_match(buf);
					if (j >= 0) {
						profile->ai_range_aware_secondary_select_mode = j;
					}
					else {
						Warning(LOCATION, "Invalid ai secondary range awareness mode '%s' specified", buf);
					}
				}

				set_flag(profile, "$no shield damage from ship collisions:", AI::Profile_Flags::No_shield_damage_from_ship_collisions);

				set_flag(profile, "$reset last_hit_target_time for player hits:", AI::Profile_Flags::Reset_last_hit_target_time_for_player_hits);

				if (optional_string("$turret target recheck time:"))
				{
					stuff_float(&profile->turret_target_recheck_time);
					if (profile->turret_target_recheck_time < 0) {
						Warning(LOCATION, "Turret target recheck time must be positive.");
						profile->turret_target_recheck_time = 2000.0f;
					}
				}

				set_flag(profile, "$prevent negative turret ammo:", AI::Profile_Flags::Prevent_negative_turret_ammo);


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
				if (!skip_to_start_of_string_either("$", "#"))
					break;
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

void ai_profile_t::reset()
{
    memset(profile_name, 0, sizeof(profile_name));

    flags.reset();

    ai_path_mode = AI_PATH_MODE_NORMAL;
	subsystem_path_radii = 0;
    bay_arrive_speed_mult = 1.0f;
    bay_depart_speed_mult = 1.0f;
	second_order_lead_predict_factor = 0;
	ai_range_aware_secondary_select_mode = AI_RANGE_AWARE_SEC_SEL_MODE_RETAIL;
	turret_target_recheck_time = 2000.0f;

    for (int i = 0; i < NUM_SKILL_LEVELS; ++i) {
        max_incoming_asteroids[i] = 0;
        max_allowed_player_homers[i] = 0;
        max_attackers[i] = 0;
        predict_position_delay[i] = 0;
        in_range_time[i] = 0;
        shield_manage_delay[i] = 0;

        link_energy_levels_always[i] = 0;
        link_energy_levels_maybe[i] = 0;

        link_ammo_levels_always[i] = 0;
        link_ammo_levels_maybe[i] = 0;
        primary_ammo_burst_mult[i] = 0;

        cmeasure_life_scale[i] = 0;
        cmeasure_fire_chance[i] = 0;
        weapon_energy_scale[i] = 0;
        shield_energy_scale[i] = 0;
        afterburner_recharge_scale[i] = 0;
        player_damage_scale[i] = 0;

        subsys_damage_scale[i] = 0;
        beam_friendly_damage_cap[i] = 0;
        turn_time_scale[i] = 0;
        glide_attack_percent[i] = 0;
        circle_strafe_percent[i] = 0;
        glide_strafe_percent[i] = 0;
        random_sidethrust_percent[i] = 0;
        stalemate_time_thresh[i] = 0;
        stalemate_dist_thresh[i] = 0;
        max_aim_update_delay[i] = 0;
        turret_max_aim_update_delay[i] = 0;

        ship_fire_delay_scale_hostile[i] = 0;
        ship_fire_delay_scale_friendly[i] = 0;

        ship_fire_secondary_delay_scale_hostile[i] = 0;
        ship_fire_secondary_delay_scale_friendly[i] = 0;

        max_turret_ownage_target[i] = 0;
        max_turret_ownage_player[i] = 0;

        kill_percentage_scale[i] = 0;
        assist_percentage_scale[i] = 0;

        assist_award_percentage_scale[i] = 0;

        repair_penalty[i] = 0;
        delay_bomb_arm_timer[i] = 0;
        chance_to_use_missiles_on_plr[i] = 0;
        player_autoaim_fov[i] = 0;
    }

    for (int i = 0; i <= MAX_DETAIL_LEVEL; ++i) {
        detail_distance_mult[i] = 0;
    }

	// via Github #2332, enable bugfixes if we are targeting version 20 and up
	if (mod_supports_version(20, 0, 0)) {
		flags.set(AI::Profile_Flags::Huge_turret_weapons_ignore_bombs);
		flags.set(AI::Profile_Flags::Fix_linked_primary_bug);
		flags.set(AI::Profile_Flags::Prevent_targeting_bombs_beyond_range);
		flags.set(AI::Profile_Flags::Fix_heat_seeker_stealth_bug);
		flags.set(AI::Profile_Flags::Allow_vertical_dodge);
		flags.set(AI::Profile_Flags::Fix_ai_class_bug);
		flags.set(AI::Profile_Flags::Ai_guards_specific_ship_in_wing);
		flags.set(AI::Profile_Flags::Fix_ai_path_order_bug);
		flags.set(AI::Profile_Flags::Aspect_invulnerability_fix);
		flags.set(AI::Profile_Flags::Use_actual_primary_range);
		flags.set(AI::Profile_Flags::fightercraft_nonshielded_ships_can_manage_ets);
	}
	// this flag has been enabled ever since 3.7.2
	if (mod_supports_version(3, 7, 2)) {
		flags.set(AI::Profile_Flags::Fix_ramming_stationary_targets_bug);
	}
	// and this flag has been enabled ever since 3.6.10
	if (mod_supports_version(3, 6, 10)) {
		flags.set(AI::Profile_Flags::Reset_last_hit_target_time_for_player_hits);
	}
	if (mod_supports_version(21, 4, 0)) {
		flags.set(AI::Profile_Flags::Fixed_ship_weapon_collision);
	}
	if (mod_supports_version(22, 0, 0)) {
		flags.set(AI::Profile_Flags::Fighterbay_arrivals_use_carrier_orient);
		flags.set(AI::Profile_Flags::Prevent_negative_turret_ammo);
	}
}
