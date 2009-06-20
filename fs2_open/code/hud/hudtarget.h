/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _HUDTARGET_H
#define _HUDTARGET_H

#include "graphics/2d.h"

struct ship;
struct ship_subsys;
struct object;
struct weapon_info;

#define INCREASING	0
#define DECREASING	1
#define NO_CHANGE		2	

#define MATCH_SPEED_THRESHOLD				0.1f		// minimum speed target must be moving for match speed to apply
#define CARGO_RADIUS_DELTA					100		// distance added to radius required for cargo scanning
#define CAPITAL_CARGO_RADIUS_DELTA		250		// distance added to radius required for cargo scanning
#define CARGO_REVEAL_MIN_DIST				150		// minimum distance for reveal cargo (used if radius+CARGO_RADIUS_DELTA < CARGO_REVEAL_MIN_DIST)
#define CAP_CARGO_REVEAL_MIN_DIST		300		// minimum distance for reveal cargo (used if radius+CARGO_RADIUS_DELTA < CARGO_REVEAL_MIN_DIST)
#define CARGO_MIN_DOT_TO_REVEAL			0.95		// min dot to proceed to have cargo scanning take place

// structure and defines used for hotkey targeting
//WMC - bumped from 50 to 150; 10/24/2005
#define MAX_HOTKEY_TARGET_ITEMS		150		// maximum number of ships that can be targeted on *all* keys
#define SELECTION_SET					0x5000	// variable used for drawing brackets.  The bracketing code uses
															// TEAM_* values.  I picked this value to be totally out of that
															// range.  Only used for drawing selection sets
#define MESSAGE_SENDER					0x5001	// variable used for drawing brackets around a message sender.
															// See above comments for SELECTION_SET

// defines used to tell how a particular hotkey was added
#define HOTKEY_USER_ADDED				1
#define HOTKEY_MISSION_FILE_ADDED	2

typedef struct htarget_list {
	struct htarget_list	*next, *prev;		// for linked lists
	int						how_added;			// determines how this hotkey was added (mission default or player)
	object					*objp;				// the actual object
} htarget_list;

//for nebula toggle SEXP
#define		TOGGLE_TEXT_NEBULA_ALPHA	127
#define		TOGGLE_TEXT_NORMAL_ALPHA	160
extern int Toggle_text_alpha;

extern htarget_list htarget_free_list;
extern int Hud_target_w, Hud_target_h;

extern shader Training_msg_glass;

extern char **Ai_class_names;
extern char *Submode_text[];
extern char *Strafe_submode_text[];

extern void hud_init_targeting_colors();

void	hud_init_targeting();
void	hud_target_next(int team_mask = -1);
void	hud_target_prev(int team_mask = -1);
int	hud_target_closest(int team_mask = -1, int attacked_objnum = -1, int play_fail_sound = TRUE, int filter = 0, int turret_attacking_target = 0);
void	hud_target_in_reticle_old();
void	hud_target_in_reticle_new();
void	hud_target_subsystem_in_reticle();
void	hud_show_targeting_gauges(float frametime, int in_cockpit=1);
void	hud_target_targets_target();
void	hud_check_reticle_list();
void	hud_target_closest_locked_missile(object *A);
void	hud_target_missile(object *source_obj, int next_flag);
void	hud_target_next_list(int hostile=1, int next_flag=1);
int	hud_target_closest_repair_ship(int goal_objnum=-1);
void	hud_target_auto_target_next();
void	hud_show_remote_detonate_missile();

void	hud_target_uninspected_object(int next_flag);
void	hud_target_newest_ship();
void	hud_target_live_turret(int next_flag, int auto_advance=0, int turret_attacking_target=0);

void hud_target_last_transmit_level_init();
void hud_target_last_transmit();
void hud_target_last_transmit_add(int ship_num);

void hud_target_random_ship();

void	hud_target_next_subobject();
void	hud_target_prev_subobject();
void	hud_cease_subsystem_targeting(int print_message=1);
void	hud_cease_targeting();
void	hud_restore_subsystem_target(ship* shipp);
int	subsystem_in_sight(object* objp, ship_subsys* subsys, vec3d *eye, vec3d* subsystem);
vec3d* get_subsystem_world_pos(object* parent_obj, ship_subsys* subsys, vec3d* world_pos);
void	hud_target_change_check();

void hud_show_target_triangle_indicator(vertex *projected_v);
void hud_show_lead_indicator(vec3d *target_world_pos);
void hud_show_lead_indicator_quick(vec3d *target_world_pos, object *targetp);
void hud_show_orientation_tee();
void hud_show_hostile_triangle();
void hud_show_target_data();
void hud_show_afterburner_gauge();
void hud_show_weapons();
void hud_start_flash_weapon(int index);
void hud_show_auto_icons();
void hud_show_weapon_energy_gauge();
void hud_show_cmeasure_gauge();
void hud_show_brackets(object *targetp, vertex *projected_v);
void hud_draw_offscreen_indicator(vertex* target_point, vec3d *tpos, float distance=0.0f, int draw_solid=1);
void hud_show_homing_missiles(void);

int hud_sensors_ok(ship *sp, int show_msg = 1);
int hud_communications_state(ship *sp, int show_msg = 0);

int hud_get_best_primary_bank(float *range);
void hud_target_toggle_hidden_from_sensors();
void hud_maybe_flash_docking_text(object *objp);
int hud_target_invalid_awacs(object *objp);

// functions for hotkey selection sets

extern void hud_target_hotkey_select( int k );
extern void hud_target_hotkey_clear( int k );

extern void hud_target_hotkey_add_remove( int k, object *objp, int how_to_add);
extern void hud_show_selection_set();
extern void hud_show_message_sender();
void			hud_prune_hotkeys();
void			hud_keyed_targets_clear();

// Code to draw filled triangles
void hud_tri(float x1,float y1,float x2,float y2,float x3,float y3);
// Code to draw empty triangles.
void hud_tri_empty(float x1,float y1,float x2,float y2,float x3,float y3);

float hud_find_target_distance( object *targetee, object *targeter );

extern void polish_predicted_target_pos(weapon_info *wip, object *targetp, vec3d *enemy_pos, vec3d *predicted_enemy_pos, float dist_to_enemy, vec3d *last_delta_vec, int num_polish_steps);

void hud_stuff_ship_name(char *ship_name_text, ship *shipp);
void hud_stuff_ship_callsign(char *ship_callsign_text, ship *shipp);
void hud_stuff_ship_class(char *ship_class_text, ship *shipp);

#endif
