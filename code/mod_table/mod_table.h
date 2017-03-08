/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */


#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"

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
extern SCP_unordered_set<SCP_string> Suppressed_warning_categories;

void mod_table_init();

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
