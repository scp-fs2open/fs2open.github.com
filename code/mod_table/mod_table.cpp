/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 * This file is in charge of the "game_settings.tbl", colloquially referred to
 * as the "mod table", and contains many misc FSO specific settings.
 */

#include "gamesnd/eventmusic.h"
#include "def_files/def_files.h"
#include "globalincs/version.h"
#include "graphics/shadows.h"
#include "localization/localize.h"
#include "libs/discord/discord.h"
#include "mission/missioncampaign.h"
#include "mission/missionload.h"
#include "mission/missionmessage.h"
#include "missionui/fictionviewer.h"
#include "nebula/neb.h"
#include "mod_table/mod_table.h"
#include "options/Option.h"
#include "parse/parselo.h"
#include "sound/sound.h"
#include "starfield/supernova.h"
#include "playerman/player.h"

int Directive_wait_time;
bool True_loop_argument_sexps;
bool Fixed_turret_collisions;
bool Fixed_missile_detonation;
bool Damage_impacted_subsystem_first;
bool Cutscene_camera_displays_hud;
bool Alternate_chaining_behavior;
bool Fixed_chaining_to_repeat;
bool Use_host_orientation_for_set_camera_facing;
bool Use_3d_ship_select;
bool Use_3d_ship_icons;
bool Use_3d_weapon_select;
bool Use_3d_weapon_icons;
bool Use_3d_overhead_ship;
overhead_style Default_overhead_ship_style;
int Default_ship_select_effect;
int Default_weapon_select_effect;
int Default_fiction_viewer_ui;
bool Enable_external_shaders;
bool Enable_external_default_scripts;
int Default_detail_level;
bool Full_color_head_anis;
bool Dont_automatically_select_turret_when_targeting_ship;
bool Weapons_inherit_parent_collision_group;
bool Flight_controls_follow_eyepoint_orientation;
int FS2NetD_port;
int Default_multi_object_update_level;
float Briefing_window_FOV;
int Briefing_window_resolution[2];
bool Disable_hc_message_ani;
SCP_vector<SCP_string> Custom_head_anis;
SCP_vector<SCP_string> Ignored_music_player_files;
bool Red_alert_applies_to_delayed_ships;
bool Beams_use_damage_factors;
float Generic_pain_flash_factor;
float Shield_pain_flash_factor;
float Emp_pain_flash_factor;
std::tuple<float, float, float> Emp_pain_flash_color;
gameversion::version Targeted_version; // Defaults to retail
SCP_string Window_title;
SCP_string Mod_title;
SCP_string Mod_version;
bool Unicode_text_mode;
SCP_vector<SCP_string> Splash_screens;
bool Use_tabled_strings_for_default_language;
bool Dont_preempt_training_voice;
SCP_string Movie_subtitle_font;
bool Enable_scripts_in_fred; // By default FRED does not initialize the scripting system
SCP_string Window_icon_path;
bool Disable_built_in_translations;
bool Weapon_shockwaves_respect_huge;
bool Using_in_game_options;
float Dinky_shockwave_default_multiplier;
bool Shockwaves_always_damage_bombs;
bool Shockwaves_damage_all_obj_types_once;
bool Shockwaves_inherit_parent_damage_type;
SCP_string Inherited_shockwave_damage_type_suffix;
SCP_string Inherited_dinky_shockwave_damage_type_suffix;
SCP_string Default_shockwave_damage_type;
SCP_string Default_dinky_shockwave_damage_type;
color Arc_color_damage_p1;
color Arc_color_damage_p2;
color Arc_color_damage_s1;
float Arc_width_default_damage;
float Arc_width_radius_multiplier_damage;
float Arc_width_no_multiply_over_radius_damage;
float Arc_width_minimum_damage;
color Arc_color_emp_p1;
color Arc_color_emp_p2;
color Arc_color_emp_s1;
float Arc_width_default_emp;
float Arc_width_radius_multiplier_emp;
float Arc_width_no_multiply_over_radius_emp;
float Arc_width_minimum_emp;
bool Use_engine_wash_intensity;
bool Apply_shudder_to_chase_view;
bool Framerate_independent_turning; // an in-depth explanation how this flag is supposed to work can be found in #2740 PR description
bool Ai_respect_tabled_turntime_rotdamp;
bool Swarmers_lead_targets;
bool Default_start_chase_view;
SCP_vector<gr_capability> Required_render_ext;
float Weapon_SS_Threshold_Turret_Inaccuracy;
bool Render_player_mflash;
bool Neb_affects_beams;
bool Neb_affects_weapons;
bool Neb_affects_particles;
bool Neb_affects_fireballs;
std::tuple<float, float, float, float> Shadow_distances;
std::tuple<float, float, float, float> Shadow_distances_cockpit;
bool Show_ship_casts_shadow;
bool Cockpit_shares_coordinate_space;
bool Custom_briefing_icons_always_override_standard_icons;
float Min_pixel_size_thruster;
float Min_pixel_size_beam;
float Min_pizel_size_muzzleflash;
float Min_pixel_size_trail;
float Min_pixel_size_laser;
bool Supernova_hits_at_zero;
bool Show_subtitle_uses_pixels;
int Show_subtitle_screen_base_res[2];
int Show_subtitle_screen_adjusted_res[2];
bool Always_warn_player_about_unbound_keys;
leadIndicatorBehavior Lead_indicator_behavior;
shadow_disable_overrides Shadow_disable_overrides {false, false, false, false};
float Thruster_easing;
bool Always_use_distant_firepoints;
bool Discord_presence;
bool Hotkey_always_hide_hidden_ships;
bool Use_weapon_class_sounds_for_hits_to_player;
bool SCPUI_loads_hi_res_animations;
bool Auto_assign_personas;
bool Countermeasures_use_capacity;
bool Play_thruster_sounds_for_player;
std::array<std::tuple<float, float>, 6> Fred_spacemouse_nonlinearity;

static auto DiscordOption = options::OptionBuilder<bool>("Other.Discord", "Discord Presence", "Toggle Discord Rich Presence")
							 .category("Other")
							 .default_val(Discord_presence)
							 .level(options::ExpertLevel::Advanced)
							 .importance(55)
		                     .change_listener([](bool val, bool) {
									if(Discord_presence){
										if (!val) {
											Discord_presence = false;
											libs::discord::shutdown();
											return true;
										}
									} else {
										if (val) {
											Discord_presence = true;
											libs::discord::init();
											return true;
										}
									}
									return false;
								})
							 .finish();

void mod_table_set_version_flags();

SCP_vector<std::pair<SCP_string, gr_capability>> req_render_ext_pairs = {
	std::make_pair("BPTC Texture Compression", CAPABILITY_BPTC)
};

void parse_mod_table(const char *filename)
{
	// SCP_vector<SCP_string> lines;

	try
	{
		if (filename == NULL)
			read_file_text_from_default(defaults_get_file("game_settings.tbl"));
		else
			read_file_text(filename, CF_TYPE_TABLES);

		reset_parse();

		// start parsing
		optional_string("#GAME SETTINGS");

		if (optional_string("$Minimum version:") || optional_string("$Target Version:")) {
			Targeted_version = gameversion::parse_version();

			mprintf(("Game Settings Table: Parsed target version of %s\n", gameversion::format_version(Targeted_version).c_str()));

			if (!gameversion::check_at_least(Targeted_version)) {
				Error(LOCATION, "This modification needs at least version %s of FreeSpace Open. However, the current is only %s!",
					gameversion::format_version(Targeted_version).c_str(),
					gameversion::format_version(gameversion::get_executable_version()).c_str());
			}

			// whenever we specify a version, set the flags for that version
			mod_table_set_version_flags();
		}

		if (optional_string("$Window title:")) {
			stuff_string(Window_title, F_NAME);
		}

		if (optional_string("$Window icon:")) {
			stuff_string(Window_icon_path, F_NAME);
		}

		if (optional_string("$Mod title:")) {
			stuff_string(Mod_title, F_NAME);
		}

		if (optional_string("$Mod version:")) {
			stuff_string(Mod_version, F_NAME);
		}
		
		if (optional_string("$Unicode mode:")) {
			stuff_boolean(&Unicode_text_mode);

			mprintf(("Game Settings Table: Unicode mode: %s\n", Unicode_text_mode ? "yes" : "no"));
		}

		if (optional_string("$Splash screens:")) {
			SCP_string splash_bitmap;
			while (optional_string("+Bitmap:")) {
				stuff_string(splash_bitmap, F_NAME);

				// remove extension?
				if (drop_extension(splash_bitmap)) {
					mprintf(("Game Settings Table: Removed extension on splash screen file name %s\n", splash_bitmap.c_str()));
				}

				Splash_screens.push_back(splash_bitmap);
			}
		}

		optional_string("#LOCALIZATION SETTINGS");

		if (optional_string("$Use tabled strings for the default language:")) {
			stuff_boolean(&Use_tabled_strings_for_default_language);

			mprintf(("Game Settings Table: Use tabled strings (translations) for the default language: %s\n", Use_tabled_strings_for_default_language ? "yes" : "no"));
		}

		if (optional_string("$Don't pre-empt training message voice:")) {
			stuff_boolean(&Dont_preempt_training_voice);

			mprintf(("Game Settings Table: %sre-empting training message voice\n", Dont_preempt_training_voice ? "Not p" : "P"));
		}

		optional_string("#CAMPAIGN SETTINGS");

		if (optional_string("$Default Campaign File Name:")) {
			char temp[MAX_FILENAME_LEN];
			stuff_string(temp, F_NAME, MAX_FILENAME_LEN);

			// remove extension?
			if (drop_extension(temp)) {
				mprintf(("Game Settings Table: Removed extension on default campaign file name %s\n", temp));
			}

			// check length
			size_t maxlen = (MAX_FILENAME_LEN - 4);
			auto len = strlen(temp);
			if (len > maxlen) {
				error_display(0, "Token too long: [%s].  Length = " SIZE_T_ARG ".  Max is " SIZE_T_ARG ".", temp, len, maxlen);
				temp[maxlen] = 0;
			}

			strcpy_s(Default_campaign_file_name, temp);
		}

		if (optional_string("#Ignored Campaign File Names")) {
			SCP_string campaign_name;

			while (optional_string("$Campaign File Name:")) {
				stuff_string(campaign_name, F_NAME);

				// remove extension?
				if (drop_extension(campaign_name)) {
					mprintf(("Game Settings Table: Removed extension on ignored campaign file name %s\n", campaign_name.c_str()));
				}

				// we want case-insensitive matching, so make this lowercase
				SCP_tolower(campaign_name);

				Ignored_campaigns.push_back(campaign_name);
			}
		}

		// Note: this feature does not ignore missions that are contained in campaigns
		if (optional_string("#Ignored Mission File Names")) {
			SCP_string mission_name;

			while (optional_string("$Mission File Name:")) {
				stuff_string(mission_name, F_NAME);

				// remove extension?
				if (drop_extension(mission_name)) {
					mprintf(("Game Settings Table: Removed extension on ignored mission file name %s\n", mission_name.c_str()));
				}

				// we want case-insensitive matching, so make this lowercase
				SCP_tolower(mission_name);

				Ignored_missions.push_back(mission_name);
			}
		}

		if (optional_string("$Red-alert applies to delayed ships:")) {
			stuff_boolean(&Red_alert_applies_to_delayed_ships);
			if (Red_alert_applies_to_delayed_ships) {
				mprintf(("Game Settings Table: Red-alert stats will be loaded for ships that arrive later in missions\n"));
			}
			else {
				mprintf(("Game Settings Table: Red-alert stats will NOT be loaded for ships that arrive later in missions (this is retail behavior)\n"));
			}
		}

		optional_string("#HUD SETTINGS");

		// how long should the game wait before displaying a directive?
		if (optional_string("$Directive Wait Time:")) {
			stuff_int(&Directive_wait_time);
		}

		if (optional_string("$Cutscene camera displays HUD:")) {
			stuff_boolean(&Cutscene_camera_displays_hud);
		}
		// compatibility
		if (optional_string("$Cutscene camera disables HUD:")) {
			mprintf(("Game Settings Table: \"$$Cutscene camera disables HUD\" is deprecated in favor of \"$Cutscene camera displays HUD\"\n"));
			bool temp;
			stuff_boolean(&temp);
			Cutscene_camera_displays_hud = !temp;
		}

		if (optional_string("$Full color head animations:")) {
			stuff_boolean(&Full_color_head_anis);
		}
		// compatibility
		if (optional_string("$Color head animations with hud colors:")) {
			mprintf(("Game Settings Table: \"$Color head animations with hud colors\" is deprecated in favor of \"$Full color head animations\"\n"));
			bool temp;
			stuff_boolean(&temp);
			Full_color_head_anis = !temp;
		}

		if (optional_string("$Don't automatically select a turret when targeting a ship:")) {
			stuff_boolean(&Dont_automatically_select_turret_when_targeting_ship);
		}

		if (optional_string("$Supernova hits at zero:")) {
			stuff_boolean(&Supernova_hits_at_zero);
			if (Supernova_hits_at_zero) {
				mprintf(("Game Settings Table: HUD timer will reach 0 when the supernova shockwave hits the player\n"));
			} else {
				mprintf(("Game Settings Table: HUD timer will reach %.2f when the supernova shockwave hits the player\n", SUPERNOVA_HIT_TIME));
			}
		}

		if (optional_string("$Always warn player about unbound keys used in Directives Gauge:")) {
			stuff_boolean(&Always_warn_player_about_unbound_keys);
		}

		optional_string("#SEXP SETTINGS");

		if (optional_string("$Loop SEXPs Then Arguments:")) {
			stuff_boolean(&True_loop_argument_sexps);
			if (True_loop_argument_sexps) {
				mprintf(("Game Settings Table: Using Reversed Loops For SEXP Arguments\n"));
			}
			else {
				mprintf(("Game Settings Table: Using Standard Loops For SEXP Arguments\n"));
			}
		}

		if (optional_string("$Use Alternate Chaining Behavior:")) {
			stuff_boolean(&Alternate_chaining_behavior);
			if (Alternate_chaining_behavior) {
				mprintf(("Game Settings Table: Using alternate event chaining behavior\n"));
			}
			else {
				mprintf(("Game Settings Table: Using standard event chaining behavior\n"));
			}
		}

		if (optional_string("$Fixed Chaining To Repeating Events:")) {
			stuff_boolean(&Fixed_chaining_to_repeat);
			if (Fixed_chaining_to_repeat) {
				mprintf(("Game Settings Table: Using fixed chaining to repeating events\n"));
			}
			else {
				mprintf(("Game Settings Table: Using retail chaining to repeating events\n"));
			}
		}

		if (optional_string("$Use host orientation for set-camera-facing:")) {
			stuff_boolean(&Use_host_orientation_for_set_camera_facing);
			if (Use_host_orientation_for_set_camera_facing) {
				mprintf(("Game Settings Table: Using host orientation for set-camera-facing\n"));
			}
			else {
				mprintf(("Game Settings Table: Using identity orientation for set-camera-facing\n"));
			}
		}

		if (optional_string("$Show-subtitle uses pixels:")) {
			stuff_boolean(&Show_subtitle_uses_pixels);
			if (Show_subtitle_uses_pixels) {
				mprintf(("Game Settings Table: Show-subtitle uses pixels\n"));
			} else {
				mprintf(("Game Settings Table: Show-subtitle uses percentages\n"));
			}
		}

		if (optional_string("$Show-subtitle base resolution:")) {
			int base_res[2];
			if (stuff_int_list(base_res, 2) == 2) {
				if (base_res[0] >= 640 && base_res[1] >= 480) {
					Show_subtitle_screen_base_res[0] = base_res[0];
					Show_subtitle_screen_base_res[1] = base_res[1];
					mprintf(("Game Settings Table: Show-subtitle base resolution is (%d, %d)\n", base_res[0], base_res[1]));
				} else {
					Warning(LOCATION, "$Show-subtitle base resolution: arguments must be at least 640x480!");
				}
			} else {
				Warning(LOCATION, "$Show-subtitle base resolution: must specify two arguments");
			}
		}

		optional_string("#GRAPHICS SETTINGS");

		if (optional_string("$Enable External Shaders:")) {
			stuff_boolean(&Enable_external_shaders);
			if (Enable_external_shaders) {
				mprintf(("Game Settings Table: External shaders are enabled\n"));
			} else {
				mprintf(("Game Settings Table: External shaders are DISABLED\n"));
			}
		}

		if (optional_string("$Default Detail Level:")) {
			int detail_level;

			stuff_int(&detail_level);

			mprintf(("Game Settings Table: Setting default detail level to %i of %i-%i\n", detail_level, 0, NUM_DEFAULT_DETAIL_LEVELS - 1));

			if (detail_level < 0 || detail_level > NUM_DEFAULT_DETAIL_LEVELS - 1) {
				error_display(0, "Invalid detail level: %i, setting to %i", detail_level, Default_detail_level);
			}
			else {
				Default_detail_level = detail_level;
			}
		}

		if (optional_string("$Briefing Window FOV:")) {
			float fov;

			stuff_float(&fov);

			mprintf(("Game Settings Table: Setting briefing window FOV from %f to %f\n", Briefing_window_FOV, fov));

			Briefing_window_FOV = fov;
		}

		if (optional_string("$Generic Pain Flash Factor:")) {
			float temp;
			stuff_float(&temp);
			if (temp >= 0.0f) {
				mprintf(("Game Settings Table: Setting generic pain flash factor to %.2f\n", temp));
				Generic_pain_flash_factor = temp;
			}
		}

		if (optional_string("$Shield Pain Flash Factor:")) {
			float temp;
			stuff_float(&temp);
			if (temp >= 0.0f) {
				mprintf(("Game Settings Table: Setting shield pain flash factor to %.2f\n", temp));
				Shield_pain_flash_factor = temp;
			}
		}

		if (optional_string("$EMP Pain Flash Factor:")) {
			float temp;
			stuff_float(&temp);
			if (temp >= 0.0f) {
				mprintf(("Game Settings Table: Setting EMP pain flash factor to %.2f\n", temp));
				Emp_pain_flash_factor = temp;
			}
		}

		if (optional_string("$EMP Pain Flash Color:")) {
			int rgb[3];
			stuff_int_list(rgb, 3);
			if ((rgb[0] >= 0 && rgb[0] <= 255) && (rgb[1] >= 0 && rgb[1] <= 255) && (rgb[2] >= 0 && rgb[2] <= 255)) {
				Emp_pain_flash_color = std::make_tuple(static_cast<float>(rgb[0])/255, static_cast<float>(rgb[1])/255, static_cast<float>(rgb[2])/255);
			} else {
				error_display(0, "$EMP Pain Flash Color is %i, %i, %i. "
					"One or more of these values is not within the range of 0-255. Assuming default color.", rgb[0], rgb[1], rgb[2]);
			}
		}

		if (optional_string("$BMPMAN Slot Limit:")) {
			int tmp;
			stuff_int(&tmp);

			mprintf(("Game Settings Table: $BMPMAN Slot Limit is deprecated and should be removed. It is not needed anymore.\n"));
		}

		if (optional_string("$EMP Arc Color:")) {
			if (optional_string("+Primary Color Option 1:")) {
				int rgb[3];
				stuff_int_list(rgb, 3);
				if ((rgb[0] >= 0 && rgb[0] <= 255) && (rgb[1] >= 0 && rgb[1] <= 255) && (rgb[2] >= 0 && rgb[2] <= 255)) {
					gr_init_color(&Arc_color_emp_p1, rgb[0], rgb[1], rgb[2]);
				} else {
					error_display(0, "$EMP Arc Color: +Primary Color Option 1 is %i, %i, %i. "
						"One or more of these values is not within the range of 0-255. Assuming default color.", rgb[0], rgb[1], rgb[2]);
				}
			}
			if (optional_string("+Primary Color Option 2:")) {
				int rgb[3];
				stuff_int_list(rgb, 3);
				if ((rgb[0] >= 0 && rgb[0] <= 255) && (rgb[1] >= 0 && rgb[1] <= 255) && (rgb[2] >= 0 && rgb[2] <= 255)) {
					gr_init_color(&Arc_color_emp_p2, rgb[0], rgb[1], rgb[2]);
				} else {
					error_display(0, "$EMP Arc Color: +Primary Color Option 2 is %i, %i, %i. "
					    "One or more of these values is not within the range of 0-255. Assuming default color.", rgb[0], rgb[1], rgb[2]);
				}
			}
			if (optional_string("+Secondary Color Option 1:")) {
				int rgb[3];
				stuff_int_list(rgb, 3);
				if ((rgb[0] >= 0 && rgb[0] <= 255) && (rgb[1] >= 0 && rgb[1] <= 255) && (rgb[2] >= 0 && rgb[2] <= 255)) {
					gr_init_color(&Arc_color_emp_s1, rgb[0], rgb[1], rgb[2]);
			    } else {
				    error_display(0,"$EMP Arc Color: +Secondary Color Option 1 is %i, %i, %i. "
					    "One or more of these values is not within the range of 0-255. Assuming default color.", rgb[0], rgb[1], rgb[2]);
			    }
		    }
		}

		if (optional_string("$EMP Arc Width:")) {
			if (optional_string("+Width Default:")) {
				stuff_float(&Arc_width_default_emp);
			}
			if (optional_string("+Width Radius Multiplier:")) {
				stuff_float(&Arc_width_radius_multiplier_emp);
			}
			if (optional_string("+Do Not Use Width Multiplier Over Radius:")) {
				stuff_float(&Arc_width_no_multiply_over_radius_emp);
			}
			if (optional_string("+Width Radius Minimum:")) {
				stuff_float(&Arc_width_minimum_emp);
			}
		}

		if (optional_string("$Damage Arc Color:")) {
			if (optional_string("+Primary Color Option 1:")) {
				int rgb[3];
				stuff_int_list(rgb, 3);
				if ((rgb[0] >= 0 && rgb[0] <= 255) && (rgb[1] >= 0 && rgb[1] <= 255) && (rgb[2] >= 0 && rgb[2] <= 255)) {
					gr_init_color(&Arc_color_damage_p1, rgb[0], rgb[1], rgb[2]);
		        } else {
			        error_display(0, "Damage Arc Color: +Primary Color Option 1 is %i, %i, %i. "
					    "One or more of these values is not within the range of 0-255. Assuming default color.", rgb[0], rgb[1], rgb[2]);
		        }
	        }
			if (optional_string("+Primary Color Option 2:")) {
				int rgb[3];
				stuff_int_list(rgb, 3);
				if ((rgb[0] >= 0 && rgb[0] <= 255) && (rgb[1] >= 0 && rgb[1] <= 255) && (rgb[2] >= 0 && rgb[2] <= 255)) {
					gr_init_color(&Arc_color_damage_p2, rgb[0], rgb[1], rgb[2]);
	            } else {
		            error_display(0, "$Damage Arc Color: +Primary Color Option 2 is %i, %i, %i. "
					    "One or more of these values is not within the range of 0-255. Assuming default color.", rgb[0], rgb[1], rgb[2]);
	            }
			}
			if (optional_string("+Secondary Color Option 1:")) {
				int rgb[3];
				stuff_int_list(rgb, 3);
				if ((rgb[0] >= 0 && rgb[0] <= 255) && (rgb[1] >= 0 && rgb[1] <= 255) && (rgb[2] >= 0 && rgb[2] <= 255)) {
					gr_init_color(&Arc_color_damage_s1, rgb[0], rgb[1], rgb[2]);
	            } else {
		            error_display(0, "$Damage Arc Color: +Secondary Color Option 1 is %i, %i, %i. "
					    "One or more of these values is not within the range of 0-255. Assuming default color.", rgb[0], rgb[1], rgb[2]);
	            }
			}
		}

		if (optional_string("$Damage Arc Width:")) {
			if (optional_string("+Width Default:")) {
				stuff_float(&Arc_width_default_damage);
			}
			if (optional_string("+Width Radius Multiplier:")) {
				stuff_float(&Arc_width_radius_multiplier_damage);
			}
			if (optional_string("+Do Not Use Width Multiplier Over Radius:")) {
				stuff_float(&Arc_width_no_multiply_over_radius_damage);
			}
			if (optional_string("+Width Radius Minimum:")) {
				stuff_float(&Arc_width_minimum_damage);
			}
		}

		if (optional_string("$Requires Rendering Feature:")) {
			SCP_vector<SCP_string> ext_strings;
			stuff_string_list(ext_strings);

			for (auto& ext_str : ext_strings) {
				auto ext = std::find_if(req_render_ext_pairs.begin(), req_render_ext_pairs.end(), 
								[ext_str](const std::pair<SCP_string, gr_capability> &ext_pair) { return !stricmp(ext_pair.first.c_str(), ext_str.c_str()); });
				if (ext != req_render_ext_pairs.end()) {
					Required_render_ext.push_back(ext->second);
				}
			}
		}

		if (optional_string("$Render player muzzle flashes in cockpit:")) {
			stuff_boolean(&Render_player_mflash);
		}

		if (optional_string("$Glowpoint nebula visibility factor:")) {
			stuff_float(&Neb2_fog_visibility_glowpoint);
		}

		if (optional_string("$Shield nebula visibility factor:")) {
			stuff_float(&Neb2_fog_visibility_shield);
		}

		if (optional_string("$Thruster nebula visibility factor:")) {
			stuff_float(&Neb2_fog_visibility_thruster);
		}

		if (optional_string("$Beams affected by nebula visibility:")) {
			stuff_boolean(&Neb_affects_beams);

			if (optional_string("+Constant visibility factor:")) {
				stuff_float(&Neb2_fog_visibility_beam_const);
			}

			if (optional_string("+Scaled visibility factor:")) {
				stuff_float(&Neb2_fog_visibility_beam_scaled_factor);
			}
		}

		if (optional_string("$Weapons affected by nebula visibility:")) {
			stuff_boolean(&Neb_affects_weapons);

			if (optional_string("+Weapon visibility factor:")) {
				stuff_float(&Neb2_fog_visibility_weapon);
			}

			if (optional_string("+Trail visibility factor:")) {
				stuff_float(&Neb2_fog_visibility_trail);
			}

			if (optional_string("+Shockwave visibility factor:")) {
				stuff_float(&Neb2_fog_visibility_shockwave);
			}
		}

		if (optional_string("$Particles affected by nebula visibility:")) {
			stuff_boolean(&Neb_affects_particles);

			if (optional_string("+Constant visibility factor:")) {
				stuff_float(&Neb2_fog_visibility_particle_const);
			}

			if (optional_string("+Scaled visibility factor:")) {
				stuff_float(&Neb2_fog_visibility_particle_scaled_factor);
			}
		}
		
		if (optional_string("$Fireballs affected by nebula visibility:")) {
			stuff_boolean(&Neb_affects_fireballs);

			if (optional_string("+Constant visibility factor:")) {
				stuff_float(&Neb2_fog_visibility_fireball_const);
			}

			if (optional_string("+Scaled visibility factor:")) {
				stuff_float(&Neb2_fog_visibility_fireball_scaled_factor);
			}
		}

		if (optional_string("$Shadow Quality Default:")) {
			int quality;
			stuff_int(&quality);
			// only set values if shadows are enabled and using default quality --wookieejedi
			if (Shadow_quality_uses_mod_option) {
				switch (quality) {
				case 0:
					Shadow_quality = ShadowQuality::Disabled;
					break;
				case 1:
					Shadow_quality = ShadowQuality::Low;
					break;
				case 2:
					Shadow_quality = ShadowQuality::Medium;
					break;
				case 3:
					Shadow_quality = ShadowQuality::High;
					break;
				case 4:
					Shadow_quality = ShadowQuality::Ultra;
					break;
				default:
					// Shadow_quality was already set in cmdline.cpp, so just keep that default --wookieejedi
					mprintf(("Game Settings Table: '$Shadow Quality Default:' value for default shadow quality %d is invalid. Using default quality of %d...\n", quality, static_cast<int>(Shadow_quality)));
					break;
				}
			}
		}

		if (optional_string("$Shadow Cascade Distances:")) {
			float dis[4];
			stuff_float_list(dis, 4);
			if ((dis[0] >= 0) && (dis[1] > dis[0]) && (dis[2] > dis[1]) && (dis[3] > dis[2])) {
				Shadow_distances = std::make_tuple((dis[0]), (dis[1]), (dis[2]), (dis[3]));
			} else {
				error_display(0, "$Shadow Cascade Distances are %f, %f, %f, %f. One or more are < 0, and/or values are not increasing. Assuming default distances.", dis[0], dis[1], dis[2], dis[3]);
			}
		}

		if (optional_string("$Shadow Cascade Distances Cockpit:")) {
			float dis[4];
			stuff_float_list(dis, 4);
			if ((dis[0] >= 0) && (dis[1] > dis[0]) && (dis[2] > dis[1]) && (dis[3] > dis[2])) {
				Shadow_distances_cockpit = std::make_tuple((dis[0]), (dis[1]), (dis[2]), (dis[3]));
			}
			else {
				error_display(0, "$Shadow Cascade Distances Cockpit are %f, %f, %f, %f. One or more are < 0, and/or values are not increasing. Assuming default distances.", dis[0], dis[1], dis[2], dis[3]);
			}
		}

		if (optional_string("$Shadow Disable Techroom:")) {
			stuff_boolean(&Shadow_disable_overrides.disable_techroom);
		}

		if (optional_string("$Shadow Disable Cockpit:")) {
			stuff_boolean(&Shadow_disable_overrides.disable_cockpit);
		}

		if (optional_string("$Shadow Disable Mission Brief Weapons:")) {
			stuff_boolean(&Shadow_disable_overrides.disable_mission_select_weapons);
		}

		if (optional_string("$Shadow Disable Mission Brief Ships:")) {
			stuff_boolean(&Shadow_disable_overrides.disable_mission_select_ships);
		}

		if (optional_string("$Show Ship Casts Shadow:")) {
			stuff_boolean(&Show_ship_casts_shadow);
		}

		if (optional_string("$Ship Model And Cockpit Share Coordinate Space:")) {
			stuff_boolean(&Cockpit_shares_coordinate_space);
		}

		if (optional_string("$Minimum Pixel Size Thrusters:")) {
			stuff_float(&Min_pixel_size_thruster);
		}
		if (optional_string("$Minimum Pixel Size Beams:")) {
			stuff_float(&Min_pixel_size_beam);
		}
		if (optional_string("$Minimum Pixel Size Muzzle Flashes:")) {
			stuff_float(&Min_pizel_size_muzzleflash);
		}
		if (optional_string("$Minimum Pixel Size Trails:")) {
			stuff_float(&Min_pixel_size_trail);
		}
		if (optional_string("$Minimum Pixel Size Lasers:")) {
			stuff_float(&Min_pixel_size_laser);
		}

		if (optional_string("$Thruster easing value:")) {
			stuff_float(&Thruster_easing);
			if (Thruster_easing <= 0.0f) {
				Warning(LOCATION, "A \'Thruster easing value\' less than or equal to 0 will not be used.\n");
			}

		}

		if (optional_string("$SCPUI attempts to load hires animations:")) {
			stuff_boolean(&SCPUI_loads_hi_res_animations);
		}

		if (optional_string("$Max draw distance:")) {
			stuff_float(&Max_draw_distance);

			if (fl_near_zero(Max_draw_distance) || Max_draw_distance < 0.0f) {
				Warning(LOCATION, "The $Max draw distance must be above 0. Using default value.\n");
				Max_draw_distance = Default_max_draw_distance;
			} 
		}

		if (optional_string("$Min draw distance:")) {
			stuff_float(&Min_draw_distance);

			if (fl_near_zero(Min_draw_distance) || Min_draw_distance < 0.0f) {
				Warning(LOCATION, "The $Min draw distance must be above 0. Using default value.\n");
				Min_draw_distance = Default_min_draw_distance;
			}
		}

		if (Min_draw_distance >= Max_draw_distance) {
			Warning(LOCATION, "The $Min draw distance must be strictly less than the $Max draw distance. Using default values for both.\n");
			Min_draw_distance = Default_min_draw_distance;
			Max_draw_distance = Default_max_draw_distance;
		}

		optional_string("#NETWORK SETTINGS");

		if (optional_string("$FS2NetD port:")) {
			stuff_int(&FS2NetD_port);
			if (FS2NetD_port)
				mprintf(("Game Settings Table: FS2NetD connecting to port %i\n", FS2NetD_port));
		}

		if (optional_string("$Default object update level for multiplayer:")) {
			int object_update;
			stuff_int(&object_update);
			if ((object_update >= OBJ_UPDATE_LOW) && (object_update <= OBJ_UPDATE_LAN)) {
				Default_multi_object_update_level = object_update;
			} else {
				mprintf(("Game Settings Table: '$Default object update level for multiplayer:' value of %d is not between %d and %d. Using default value of %d.\n", object_update, OBJ_UPDATE_LOW, OBJ_UPDATE_LAN, OBJ_UPDATE_HIGH));
			}
		}

		optional_string("#SOUND SETTINGS");

		if (optional_string("$Default Sound Volume:")) {
			float snd_default;
			stuff_float(&snd_default);
			if ((snd_default >= 0) && (snd_default <= 1.0)) {
				Default_sound_volume = snd_default;
			} else {
				error_display(0, "$Default Sound Volume is %f. It is not within 0-1.0 and will not be used. ", snd_default);
			}
		}

		if (optional_string("$Default Music Volume:")) {
			float music_default;
			stuff_float(&music_default);
			if ((music_default >= 0) && (music_default <= 1.0)) {
				Default_music_volume = music_default;
			} else {
				error_display(0, "$Default Music Volume is %f. It is not within 0-1.0 and will not be used. ", music_default);
			}
		}

		if (optional_string("$Default Voice Volume:")) {
			float voice_default;
			stuff_float(&voice_default);
			if ((voice_default >= 0) && (voice_default <= 1.0)) {
				Default_voice_volume = voice_default;
			} else {
				error_display(0, "$Default Voice Volume is %f. It is not within 0-1.0 and will not be used. ", voice_default);
			}
		}

		if (optional_string("$Use weapon class impact sounds for hits to player:")) {
			stuff_boolean(&Use_weapon_class_sounds_for_hits_to_player);
		}

		if (optional_string("$Play thruster sounds for the player:")) {
			stuff_boolean(&Play_thruster_sounds_for_player);
		}

		optional_string("#FRED SETTINGS");

		if (optional_string("$Disable Hard Coded Message Head Ani Files:")) {
			stuff_boolean(&Disable_hc_message_ani);
			if (Disable_hc_message_ani) {
				mprintf(("Game Settings Table: FRED - Disabling Hard Coded Message Ani Files\n"));
			}
			else {
				mprintf(("Game Settings Table: FRED - Using Hard Coded Message Ani Files\n"));

			}
		}

		if (optional_string("$Add Message Head Ani Files:")) {
			SCP_string head_name;
			while (optional_string("+Head:")) {
				stuff_string(head_name, F_NAME);

				// remove extension?
				if (drop_extension(head_name)) {
					mprintf(("Game Settings Table: Removed extension on head ani file name %s\n", head_name.c_str()));
				}

				Custom_head_anis.push_back(head_name);
			}
		}

		if (optional_string("$Enable scripting in FRED:")) {
			stuff_boolean(&Enable_scripts_in_fred);
			if (Enable_scripts_in_fred) {
				mprintf(("Game Settings Table: FRED - Scripts will be executed when running FRED.\n"));
			}
			else {
				mprintf(("Game Settings Table: FRED - Scripts will not be executed when running FRED.\n"));
			}
		}

		if (optional_string("$FRED Briefing window resolution:")) {
			int res[2];
			if (stuff_int_list(res, 2) == 2) {
				mprintf(("Game Settings Table: Setting FRED briefing window resolution from (%ix%i) to (%ix%i)\n",
					Briefing_window_resolution[0],
					Briefing_window_resolution[1],
					res[0],
					res[1]));

				Briefing_window_resolution[0] = res[0];
				Briefing_window_resolution[1] = res[1];
			} else {
				Warning(LOCATION, "$FRED Briefing window resolution: must specify two arguments");
			}
		}


		if (optional_string("$Ignore Music Files In Music Player:")) {
			SCP_string music_name;
			while (optional_string("+File:")) {
				stuff_string(music_name, F_NAME);

				drop_extension(music_name);

				Ignored_music_player_files.push_back(music_name);
			}
		}

		if (optional_string("$FRED spacemouse nonlinearities:")) {
#define SPACEMOUSE_NONLINEARITY(name, id) \
			if (optional_string("+" name " exponent:")) \
				stuff_float(&std::get<0>(Fred_spacemouse_nonlinearity[id])); \
			if (optional_string("+" name " scale:")) \
				stuff_float(&std::get<1>(Fred_spacemouse_nonlinearity[id]));
			SPACEMOUSE_NONLINEARITY("Sideways", 0);
			SPACEMOUSE_NONLINEARITY("Forwards", 1);
			SPACEMOUSE_NONLINEARITY("Upwards", 2);
			SPACEMOUSE_NONLINEARITY("Pitch", 3);
			SPACEMOUSE_NONLINEARITY("Bank", 4);
			SPACEMOUSE_NONLINEARITY("Heading", 5);
#undef SPACEMOUSE_NONLINEARITY

		}

		optional_string("#OTHER SETTINGS");

		if (optional_string("$Fixed Turret Collisions:")) {
			stuff_boolean(&Fixed_turret_collisions);
		}

		if (optional_string("$Fixed Missile Detonation:")) {
			stuff_boolean(&Fixed_missile_detonation);
		}

		if (optional_string("$Damage Impacted Subsystem First:")) {
			stuff_boolean(&Damage_impacted_subsystem_first);
		}

		if (optional_string("$Use 3d ship select:")) {
			stuff_boolean(&Use_3d_ship_select);
		}

		if (optional_string("$Default ship select effect:")) {
			char effect[NAME_LENGTH];
			stuff_string(effect, F_NAME, NAME_LENGTH);
			if (!stricmp(effect, "FS2"))
				Default_ship_select_effect = 2;
			else if (!stricmp(effect, "FS1"))
				Default_ship_select_effect = 1;
			else if (!stricmp(effect, "off"))
				Default_ship_select_effect = 0;
		}

		if (optional_string("$Use 3d ship icons:")) {
			stuff_boolean(&Use_3d_ship_icons);
		}

		if (optional_string("$Use 3d weapon select:")) {
			stuff_boolean(&Use_3d_weapon_select);
		}

		if (optional_string("$Default weapon select effect:")) {
			char effect[NAME_LENGTH];
			stuff_string(effect, F_NAME, NAME_LENGTH);
			if (!stricmp(effect, "FS2"))
				Default_weapon_select_effect = 2;
			else if (!stricmp(effect, "FS1"))
				Default_weapon_select_effect = 1;
			else if (!stricmp(effect, "off"))
				Default_weapon_select_effect = 0;
		}

		if (optional_string("$Use 3d weapon icons:")) {
			stuff_boolean(&Use_3d_weapon_icons);
		}

		if (optional_string("$Use 3d overhead ship:")) {
			stuff_boolean(&Use_3d_overhead_ship);
		}

		if (optional_string("$Default overhead ship style:")) {
			char effect[NAME_LENGTH];
			stuff_string(effect, F_NAME, NAME_LENGTH);
			if (!stricmp(effect, "ROTATE")) {
				Default_overhead_ship_style = OH_ROTATING;
			} else if (!stricmp(effect, "TOPVIEW")) {
				Default_overhead_ship_style = OH_TOP_VIEW;
			} else {
				error_display(0, "Unknown Select Overhead Ship Style %s, using TOPVIEW instead.", effect);
			}
		}

		if (optional_string("$Weapons inherit parent collision group:")) {
			stuff_boolean(&Weapons_inherit_parent_collision_group);
			if (Weapons_inherit_parent_collision_group)
				mprintf(("Game Settings Table: Weapons inherit parent collision group\n"));
		}

		if (optional_string("$Flight controls follow eyepoint orientation:")) {
			stuff_boolean(&Flight_controls_follow_eyepoint_orientation);
			if (Flight_controls_follow_eyepoint_orientation)
				mprintf(("Game Settings Table: Flight controls follow eyepoint orientation\n"));
		}

		if (optional_string("$Beams Use Damage Factors:")) {
			stuff_boolean(&Beams_use_damage_factors);
			if (Beams_use_damage_factors) {
				mprintf(("Game Settings Table: Beams will use Damage Factors\n"));
			}
			else {
				mprintf(("Game Settings Table: Beams will ignore Damage Factors (retail behavior)\n"));
			}
		}

		if (optional_string("$Default fiction viewer UI:")) {
			char ui_name[NAME_LENGTH];
			stuff_string(ui_name, F_NAME, NAME_LENGTH);
			if (!stricmp(ui_name, "auto"))
				Default_fiction_viewer_ui = -1;
			else {
				int ui_index = fiction_viewer_ui_name_to_index(ui_name);
				if (ui_index >= 0)
					Default_fiction_viewer_ui = ui_index;
				else
					error_display(0, "Unrecognized fiction viewer UI: %s", ui_name);
			}
		}

		if (optional_string("$Movie subtitle font:")) {
			// Fonts have not been parsed at this point so we can't validate the font name here
			stuff_string(Movie_subtitle_font, F_NAME);
		}

		if (optional_string("$Disable built-in translations:")) {
			stuff_boolean(&Disable_built_in_translations);
		}

		if (optional_string("$Weapon shockwave damage respects huge ship flags:")) {
			stuff_boolean(&Weapon_shockwaves_respect_huge);
		}

		if (optional_string("$Enable external default scripts:")) {
			stuff_boolean(&Enable_external_default_scripts);

			if (Enable_external_default_scripts) {
				mprintf(("Game Settings Table: Enabled external default scripts.\n"));
			} else {
				mprintf(("Game Settings Table: Disabled external default scripts.\n"));
			}
		}

		if (optional_string("$Player warpout speed:")) {
			stuff_float(&Player_warpout_speed);
		}
		
		if (optional_string("$Target warpout match percent:")) {
			stuff_float(&Target_warpout_match_percent);
		}

		if (optional_string("$Minimum player warpout time:")) {
			stuff_float(&Minimum_player_warpout_time);
		}

		if (optional_string("$Enable in-game options:")) {
			stuff_boolean(&Using_in_game_options);

			if (Using_in_game_options) {
				mprintf(("Game Settings Table: Using in-game options system.\n"));
			} else {
				mprintf(("Game Settings Table: Not using in-game options system.\n"));
			}
		}

		if (optional_string("$Dinky Shockwave Default Multiplier:")) {
			stuff_float(&Dinky_shockwave_default_multiplier);
			if (Dinky_shockwave_default_multiplier != 1.0f) {
				mprintf(("Game Settings Table: Setting default dinky shockwave multiplier to %.2f.\n", Dinky_shockwave_default_multiplier));
			}
		}

		if (optional_string("$Shockwaves Always Damage Bombs:")) {
			stuff_boolean(&Shockwaves_always_damage_bombs);
		}

		if (optional_string("$Shockwaves Damage All Object Types Once:")) {
			stuff_boolean(&Shockwaves_damage_all_obj_types_once);
		}

		if (optional_string("$Shockwaves Inherit Parent Weapon Damage Type:")) {
			stuff_boolean(&Shockwaves_inherit_parent_damage_type);
		}

		if (optional_string("$Inherited Shockwave Damage Type Added Suffix:")) {
			stuff_string(Inherited_shockwave_damage_type_suffix, F_NAME);
			Inherited_dinky_shockwave_damage_type_suffix = Inherited_shockwave_damage_type_suffix;
		}

		if (optional_string("$Inherited Dinky Shockwave Damage Type Added Suffix:")) {
			stuff_string(Inherited_dinky_shockwave_damage_type_suffix, F_NAME);
		}

		if (optional_string("$Default Shockwave Damage Type:")) {
			stuff_string(Default_shockwave_damage_type, F_NAME);
			// Should this automatically be copied to dinky shockwaves?
			Default_dinky_shockwave_damage_type = Default_shockwave_damage_type;
		}

		if (optional_string("$Default Dinky Shockwave Damage Type:")) {
			stuff_string(Default_dinky_shockwave_damage_type, F_NAME);
		}

		if (optional_string("$Use Engine Wash Intensity:")) {
			stuff_boolean(&Use_engine_wash_intensity);
		}

		if (optional_string("$Apply HUD shudder to chase view:")) {
			stuff_boolean(&Apply_shudder_to_chase_view);
		}

		if (optional_string("$Swarmers Lead Targets:")) {
			stuff_boolean(&Swarmers_lead_targets);
		}

		if (optional_string("$Damage Threshold for Weapons Subsystems to Trigger Turret Inaccuracy:")) {
			float weapon_ss_threshold;
			stuff_float(&weapon_ss_threshold);
			if ( (weapon_ss_threshold >= 0.0f) && (weapon_ss_threshold <= 1.0f) ) {
				Weapon_SS_Threshold_Turret_Inaccuracy = weapon_ss_threshold;
			} else {
				mprintf(("Game Settings Table: '$Damage Threshold for Weapons Subsystems to Trigger Turret Inaccuracy:' value of %.2f is not between 0 and 1. Using default value of 0.70.\n", weapon_ss_threshold));
			}
		}

		if (optional_string("$AI use framerate independent turning:")) {
			stuff_boolean(&Framerate_independent_turning);
		}
		
		if (optional_string("+AI respect tabled turn time and rotdamp:")) {
			stuff_boolean(&Ai_respect_tabled_turntime_rotdamp);
			if (!Framerate_independent_turning) {
				Warning(LOCATION, "\'AI respect tabled turn time and rotdamp\' requires \'AI use framerate independent turning\' in order to function.\n");
			}
		}
		
		if (optional_string("$Player starts in third person/chase view by default:")) {
			stuff_boolean(&Default_start_chase_view);
		}
		
		if (optional_string("$Custom briefing icons always override standard icons:")) {
			stuff_boolean(&Custom_briefing_icons_always_override_standard_icons);
		}

		if (optional_string("$Lead indicator behavior:")){
			SCP_string temp;
			stuff_string(temp, F_RAW);
			SCP_tolower(temp);

			if (temp == "default")
			{
				Lead_indicator_behavior = leadIndicatorBehavior::DEFAULT;
			}
			else if (temp == "multiple")
			{
				Lead_indicator_behavior = leadIndicatorBehavior::MULTIPLE;
			}
			else if (temp == "average")
			{
				Lead_indicator_behavior = leadIndicatorBehavior::AVERAGE;
			}
			else
			{
				Warning(LOCATION, "$Lead indicator behavior: Invalid selection. Must be default, multiple or average. Reverting to default.");
				Lead_indicator_behavior = leadIndicatorBehavior::DEFAULT;
			}
		}

		if (optional_string("$Use distant firepoint for all turrets:")){
			stuff_boolean(&Always_use_distant_firepoints);
		}
		if (optional_string("$Enable Discord rich presence:")) {
			stuff_boolean(&Discord_presence);
		}

		if (optional_string("$Always hide hidden ships in hotkey list:")) {
			stuff_boolean(&Hotkey_always_hide_hidden_ships);
		}

		if (optional_string("$Allow automatically assigning personas:")) {
			stuff_boolean(&Auto_assign_personas);
		}

		if (optional_string("$Countermeasures use capacity:")) {
			stuff_boolean(&Countermeasures_use_capacity);
		}

		required_string("#END");
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", (filename) ? filename : "<default game_settings.tbl>", e.what()));
		return;
	}
}

void mod_table_init()
{
	mod_table_reset();

	// first parse the default table
	parse_mod_table(NULL);

	// if a mod.tbl exists read it
	if (cf_exists_full("game_settings.tbl", CF_TYPE_TABLES)) {
		parse_mod_table("game_settings.tbl");
	}

	// parse any modular tables
	parse_modular_table("*-mod.tbm", parse_mod_table);
}

// game_settings.tbl is parsed before graphics are actually initialized, so we can't calculate the resolution at that time
void mod_table_post_process()
{
	// use the same widescreen code as in adjust_base_res()
	// This calculates an adjusted resolution if the aspect ratio of the base resolution doesn't exactly match that of the current resolution.
	// The base resolution specified in game_settings.tbl does not need to be 1024x768 or even 4:3.
	float aspect_quotient = ((float)gr_screen.center_w / (float)gr_screen.center_h) / ((float)Show_subtitle_screen_base_res[0] / (float)Show_subtitle_screen_base_res[1]);
	if (aspect_quotient >= 1.0) {
		Show_subtitle_screen_adjusted_res[0] = (int)(Show_subtitle_screen_base_res[0] * aspect_quotient);
		Show_subtitle_screen_adjusted_res[1] = Show_subtitle_screen_base_res[1];
	} else {
		Show_subtitle_screen_adjusted_res[0] = Show_subtitle_screen_base_res[0];
		Show_subtitle_screen_adjusted_res[1] = (int)(Show_subtitle_screen_base_res[1] / aspect_quotient);
	}
	mprintf(("Game Settings Table: Show-subtitle adjusted resolution is (%d, %d)\n", Show_subtitle_screen_adjusted_res[0], Show_subtitle_screen_adjusted_res[1]));
}

bool mod_supports_version(int major, int minor, int build)
{
	return Targeted_version >= gameversion::version(major, minor, build, 0);
}

bool mod_supports_version(const gameversion::version& version) {
	return Targeted_version >= version;
}

void mod_table_reset()
{
	Directive_wait_time = 3000;
	True_loop_argument_sexps = false;
	Fixed_turret_collisions = false;
	Fixed_missile_detonation = false;
	Damage_impacted_subsystem_first = false;
	Cutscene_camera_displays_hud = false;
	Alternate_chaining_behavior = false;
	Fixed_chaining_to_repeat = false;
	Use_host_orientation_for_set_camera_facing = false;
	Default_ship_select_effect = 2;
	Default_weapon_select_effect = 2;
	Default_overhead_ship_style = OH_TOP_VIEW;
	Default_fiction_viewer_ui = -1;
	Enable_external_shaders = false;
	Enable_external_default_scripts = false;
	Default_detail_level = 3; // "very high" seems a reasonable default in 2012 -zookeeper
	Full_color_head_anis = false;
	Dont_automatically_select_turret_when_targeting_ship = false;
	Weapons_inherit_parent_collision_group = false;
	Flight_controls_follow_eyepoint_orientation = false;
	FS2NetD_port = 0;
	Default_multi_object_update_level = OBJ_UPDATE_HIGH;
	Briefing_window_FOV = 0.29375f;
	Briefing_window_resolution[0] = 888;
	Briefing_window_resolution[1] = 371;
	Disable_hc_message_ani = false;
	Red_alert_applies_to_delayed_ships = false;
	Beams_use_damage_factors = false;
	Generic_pain_flash_factor = 1.0f;
	Shield_pain_flash_factor = 0.0f;
	Emp_pain_flash_factor = 1.0f;
	Emp_pain_flash_color = std::make_tuple(1.0f, 1.0f, 0.5f);
	Targeted_version = gameversion::version(2, 0, 0, 0); // Defaults to retail
	Window_title = "";
	Mod_title = "";
	Mod_version = "";
	Unicode_text_mode = false;
	Use_tabled_strings_for_default_language = false;
	Dont_preempt_training_voice = false;
	Movie_subtitle_font = "font01.vf";
	Enable_scripts_in_fred = false;
	Window_icon_path = "app_icon_sse";
	Disable_built_in_translations = false;
	Weapon_shockwaves_respect_huge = false;
	Using_in_game_options = false;
	Dinky_shockwave_default_multiplier = 1.0f;
	Shockwaves_always_damage_bombs = false;
	Shockwaves_damage_all_obj_types_once = false;
	Shockwaves_inherit_parent_damage_type = false;
	Inherited_shockwave_damage_type_suffix = "";
	Inherited_dinky_shockwave_damage_type_suffix = "";
	Default_shockwave_damage_type = "";
	Default_dinky_shockwave_damage_type = "";
	gr_init_color(&Arc_color_damage_p1, 64, 64, 255);
	gr_init_color(&Arc_color_damage_p2, 128, 128, 255);
	gr_init_color(&Arc_color_damage_s1, 200, 200, 255);
	Arc_width_default_damage = 0.9f;
	Arc_width_radius_multiplier_damage = 0.01f;
	Arc_width_no_multiply_over_radius_damage = 500.0f;
	Arc_width_minimum_damage = 0.2f;
	gr_init_color(&Arc_color_emp_p1, 64, 64, 5);
	gr_init_color(&Arc_color_emp_p2, 128, 128, 10);
	gr_init_color(&Arc_color_emp_s1, 255, 255, 10);
	Arc_width_default_emp = 0.9f;
	Arc_width_radius_multiplier_emp = 0.01f;
	Arc_width_no_multiply_over_radius_emp = 500.0f;
	Arc_width_minimum_emp = 0.2f;
	Use_engine_wash_intensity = false;
	Apply_shudder_to_chase_view = false;
	Framerate_independent_turning = true;
	Ai_respect_tabled_turntime_rotdamp = false;
	Default_start_chase_view = false;
	Swarmers_lead_targets = false;
	Required_render_ext.clear();
	Weapon_SS_Threshold_Turret_Inaccuracy = 0.7f; // Defaults to retail value of 0.7 --wookieejedi
	Render_player_mflash = false;
	Neb_affects_beams = false;
	Neb_affects_weapons = false;
	Neb_affects_particles = false;
	Neb_affects_fireballs = false;
	Shadow_distances = std::make_tuple(200.0f, 600.0f, 2500.0f, 8000.0f); // Default values tuned by Swifty and added here by wookieejedi
	Shadow_distances_cockpit = std::make_tuple(0.25f, 0.75f, 1.5f, 3.0f); // Default values tuned by wookieejedi and added here by Lafiel
	Show_ship_casts_shadow = false;
	Cockpit_shares_coordinate_space = false;
	Custom_briefing_icons_always_override_standard_icons = false;
	Min_pixel_size_thruster = 0.0f;
	Min_pixel_size_beam = 0.0f;
	Min_pizel_size_muzzleflash = 0.0f;
	Min_pixel_size_trail = 0.0f;
	Min_pixel_size_laser = 0.0f;
	Supernova_hits_at_zero = false;
	Show_subtitle_uses_pixels = false;
	Show_subtitle_screen_base_res[0] = -1;
	Show_subtitle_screen_base_res[1] = -1;
	Show_subtitle_screen_adjusted_res[0] = -1;
	Show_subtitle_screen_adjusted_res[1] = -1;
	Always_warn_player_about_unbound_keys = false;
	Lead_indicator_behavior = leadIndicatorBehavior::DEFAULT;
	Thruster_easing = 0;
	Always_use_distant_firepoints = false;
	Discord_presence = true;
	Hotkey_always_hide_hidden_ships = false;
	Use_weapon_class_sounds_for_hits_to_player = false;
	SCPUI_loads_hi_res_animations = true;
	Auto_assign_personas = true;
	Countermeasures_use_capacity = false;
	Play_thruster_sounds_for_player = false;
	Fred_spacemouse_nonlinearity = std::array<std::tuple<float, float>, 6>{{
			std::tuple<float, float>{ 1.0f, 1.0f },
			std::tuple<float, float>{ 1.0f, 1.0f },
			std::tuple<float, float>{ 1.0f, 1.0f },
			std::tuple<float, float>{ 1.0f, 1.0f },
			std::tuple<float, float>{ 1.0f, 1.0f },
			std::tuple<float, float>{ 1.0f, 1.0f }
		}};
}

void mod_table_set_version_flags()
{
	if (mod_supports_version(22, 0, 0)) {
		Fixed_turret_collisions = true;
		Fixed_missile_detonation = true;
		Shockwaves_damage_all_obj_types_once = true;
		Framerate_independent_turning = true;					// this is already true, but let's re-emphasize it
		Use_host_orientation_for_set_camera_facing = true;		// this is essentially a bugfix
	}
	if (mod_supports_version(23, 0, 0)) {
		Shockwaves_inherit_parent_damage_type = true;	// people intuitively expect shockwaves to default to the damage type of the weapon that spawned them
		Fixed_chaining_to_repeat = true;
	}
}
