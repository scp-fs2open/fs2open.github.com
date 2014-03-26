/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */


#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "globalincs/systemvars.h"

extern int Directive_wait_time;
extern bool True_loop_argument_sexps;
extern bool Fixed_turret_collisions;
extern bool Damage_impacted_subsystem_first;
extern bool Cutscene_camera_displays_hud;
extern bool Alternate_chaining_behavior;
extern int Default_ship_select_effect;
extern int Default_weapon_select_effect;
extern bool Enable_external_shaders;
extern int Default_detail_level;
extern bool Full_color_head_anis;
extern bool Weapons_inherit_parent_collision_group;
extern bool Flight_controls_follow_eyepoint_orientation;
extern int FS2NetD_port;

void mod_table_init();
