#pragma once
/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 * This file is in charge of the "game_settings.tbl", colloquially referred to
 * as the "mod table", and contains many misc FSO specific settings.
 */

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "hud/hudtarget.h"

// Typedef for Overhead View styles
typedef enum {
	OH_TOP_VIEW,
	OH_ROTATING
} overhead_style;

extern int Directive_wait_time;
extern bool True_loop_argument_sexps;
extern bool Fixed_turret_collisions;
extern bool Fixed_missile_detonation;
extern bool Damage_impacted_subsystem_first;
extern bool Cutscene_camera_displays_hud;
extern bool Alternate_chaining_behavior;
extern bool Fixed_chaining_to_repeat;
extern bool Use_host_orientation_for_set_camera_facing;
extern bool Use_3d_ship_select;
extern int Default_ship_select_effect;
extern bool Use_3d_ship_icons;
extern bool Use_3d_weapon_select;
extern int Default_weapon_select_effect;
extern bool Use_3d_weapon_icons;
extern bool Use_3d_overhead_ship;
extern overhead_style Default_overhead_ship_style;
extern int Default_fiction_viewer_ui;
extern bool Enable_external_shaders;
extern bool Enable_external_default_scripts;
extern int Default_detail_level;
extern bool Full_color_head_anis;
extern bool Dont_automatically_select_turret_when_targeting_ship;
extern bool Weapons_inherit_parent_collision_group;
extern bool Flight_controls_follow_eyepoint_orientation;
extern int FS2NetD_port;
extern int Default_multi_object_update_level;
extern float Briefing_window_FOV;
extern int Briefing_window_resolution[2];
extern bool Disable_hc_message_ani;
extern SCP_vector<SCP_string> Custom_head_anis;
extern SCP_vector<SCP_string> Ignored_music_player_files;
extern bool Red_alert_applies_to_delayed_ships;
extern bool Beams_use_damage_factors;
extern float Generic_pain_flash_factor;
extern float Shield_pain_flash_factor;
extern float Emp_pain_flash_factor;
extern std::tuple<float, float, float> Emp_pain_flash_color;
extern SCP_string Window_title;
extern SCP_string Mod_title;
extern SCP_string Mod_version;
extern bool Unicode_text_mode;
extern SCP_vector<SCP_string> Splash_screens;
extern bool Use_tabled_strings_for_default_language;
extern bool Dont_preempt_training_voice;
extern SCP_string Movie_subtitle_font;
extern bool Enable_scripts_in_fred;
extern SCP_string Window_icon_path;
extern bool Disable_built_in_translations;
extern bool Weapon_shockwaves_respect_huge;
extern bool Using_in_game_options;
extern float Dinky_shockwave_default_multiplier;
extern bool Shockwaves_always_damage_bombs;
extern bool Shockwaves_damage_all_obj_types_once;
extern bool Shockwaves_inherit_parent_damage_type;
extern SCP_string Inherited_shockwave_damage_type_suffix;
extern SCP_string Inherited_dinky_shockwave_damage_type_suffix;
extern SCP_string Default_shockwave_damage_type;
extern SCP_string Default_dinky_shockwave_damage_type;
extern color Arc_color_damage_p1;
extern color Arc_color_damage_p2;
extern color Arc_color_damage_s1;
extern float Arc_width_default_damage;
extern float Arc_width_radius_multiplier_damage;
extern float Arc_width_no_multiply_over_radius_damage;
extern float Arc_width_minimum_damage;
extern color Arc_color_emp_p1;
extern color Arc_color_emp_p2;
extern color Arc_color_emp_s1;
extern float Arc_width_default_emp;
extern float Arc_width_radius_multiplier_emp;
extern float Arc_width_no_multiply_over_radius_emp;
extern float Arc_width_minimum_emp;
extern bool Use_engine_wash_intensity;
extern bool Apply_shudder_to_chase_view;
extern bool Swarmers_lead_targets;
extern SCP_vector<gr_capability> Required_render_ext;
extern float Weapon_SS_Threshold_Turret_Inaccuracy;
extern bool Framerate_independent_turning;
extern bool Ai_respect_tabled_turntime_rotdamp;
extern bool Default_start_chase_view;
extern bool Render_player_mflash;
extern bool Neb_affects_beams;
extern bool Neb_affects_weapons;
extern bool Neb_affects_particles;
extern bool Neb_affects_fireballs;
extern std::tuple<float, float, float, float> Shadow_distances;
extern std::tuple<float, float, float, float> Shadow_distances_cockpit;
extern bool Show_ship_casts_shadow;
extern bool Cockpit_shares_coordinate_space;
extern bool Custom_briefing_icons_always_override_standard_icons;
extern float Min_pixel_size_thruster;
extern float Min_pixel_size_beam;
extern float Min_pizel_size_muzzleflash;
extern float Min_pixel_size_trail;
extern float Min_pixel_size_laser;
extern bool Supernova_hits_at_zero;
extern bool Show_subtitle_uses_pixels;
extern int Show_subtitle_screen_base_res[];
extern int Show_subtitle_screen_adjusted_res[];
extern bool Always_warn_player_about_unbound_keys;
extern leadIndicatorBehavior Lead_indicator_behavior;
extern struct shadow_disable_overrides {
	bool disable_techroom, disable_mission_select_weapons, disable_mission_select_ships, disable_cockpit;
} Shadow_disable_overrides;
extern float Thruster_easing;
extern bool Always_use_distant_firepoints;
extern bool Discord_presence;
extern bool Hotkey_always_hide_hidden_ships;
extern bool Use_weapon_class_sounds_for_hits_to_player;
extern bool SCPUI_loads_hi_res_animations;
extern bool Auto_assign_personas;
extern bool Countermeasures_use_capacity;
extern bool Play_thruster_sounds_for_player;
extern std::array<std::tuple<float, float>, 6> Fred_spacemouse_nonlinearity;

void mod_table_init();
void mod_table_post_process();

/**
 * @brief Resets the mod values back to their default values
 *
 * This is mostly useful for the unit tests where mod value changes can interfere with other tests
 */
void mod_table_reset();

/**
 * @brief Checks if the mod specified support for the given engine version
 *
 * This is the function for implementing backwards-incompatible changes while not actually breaking backwards
 * compatibility. If you want to introduce a change that may impact compatibility with an earlier version (e.g. retail)
 * then you can use this function to check if the current mod targets a recent enough version.
 *
 * @param major The major version to check support for
 * @param minor The minor version to check support for
 * @param build The build version to check support for
 * @return @c true if the mod specified support for this or a later version, @c false otherwise
 */
bool mod_supports_version(int major, int minor, int build);

bool mod_supports_version(const gameversion::version& version);