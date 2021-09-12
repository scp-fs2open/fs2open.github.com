/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */


#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"

extern int Directive_wait_time;
extern bool True_loop_argument_sexps;
extern bool Fixed_turret_collisions;
extern bool Damage_impacted_subsystem_first;
extern bool Cutscene_camera_displays_hud;
extern bool Alternate_chaining_behavior;
extern int Default_ship_select_effect;
extern int Default_weapon_select_effect;
extern int Default_fiction_viewer_ui;
extern bool Enable_external_shaders;
extern bool Enable_external_default_scripts;
extern int Default_detail_level;
extern bool Full_color_head_anis;
extern bool Weapons_inherit_parent_collision_group;
extern bool Flight_controls_follow_eyepoint_orientation;
extern int FS2NetD_port;
extern float Briefing_window_FOV;
extern bool Disable_hc_message_ani;
extern bool Red_alert_applies_to_delayed_ships;
extern bool Beams_use_damage_factors;
extern float Generic_pain_flash_factor;
extern float Shield_pain_flash_factor;
extern SCP_string Window_title;
extern bool Unicode_text_mode;
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
extern std::tuple<ubyte, ubyte, ubyte> Arc_color_damage_p1;
extern std::tuple<ubyte, ubyte, ubyte> Arc_color_damage_p2;
extern std::tuple<ubyte, ubyte, ubyte> Arc_color_damage_s1;
extern std::tuple<ubyte, ubyte, ubyte> Arc_color_emp_p1;
extern std::tuple<ubyte, ubyte, ubyte> Arc_color_emp_p2;
extern std::tuple<ubyte, ubyte, ubyte> Arc_color_emp_s1;
extern bool Use_engine_wash_intensity;
extern bool Swarmers_lead_targets;
extern SCP_vector<gr_capability> Required_render_ext;
extern float Weapon_SS_Threshold_Turret_Inaccuracy;
extern bool Framerate_independent_turning;
extern bool Ai_respect_tabled_turntime_rotdamp;
extern bool Chase_view_default;
extern bool Render_player_mflash;
extern bool Neb_affects_beams;
extern bool Neb_affects_weapons;
extern bool Neb_affects_particles;
extern bool Neb_affects_fireballs;
extern std::tuple<float, float, float, float> Shadow_distances;
extern std::tuple<float, float, float, float> Shadow_distances_cockpit;
extern bool Custom_briefing_icons_always_override_standard_icons;
extern float Min_pixel_size_thruster;
extern float Min_pixel_size_beam;
extern float Min_pizel_size_muzzleflash;
extern float Min_pixel_size_trail;
extern float Min_pixel_size_laser;

void mod_table_init();

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
