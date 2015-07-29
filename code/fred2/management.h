/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __MANAGEMENT_H__
#define __MANAGEMENT_H__

#include <afxmt.h>
#include "globalincs/pstypes.h"
#include "ship/ship.h"
#include "ai/aigoals.h"
#include "jumpnode/jumpnode.h"


#define SHIP_FILTER_PLAYERS	(1<<0)  // set: add players to list as well
#define SHIP_FILTER_FLYABLE	(1<<1)  // set: filter out non-flyable ships

extern int cur_object_index;
extern int cur_ship;
extern int cur_wing;
extern int cur_wing_index;
extern int cur_model_index;
extern waypoint *cur_waypoint;
extern waypoint_list *cur_waypoint_list;
extern int Update_ship;
extern int Update_wing;

extern ai_goal_list Ai_goal_list[];
extern int Ai_goal_list_size;

// alternate ship name and callsign stuff
extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH+1];
extern char Fred_callsigns[MAX_SHIPS][NAME_LENGTH+1];

extern int	wing_objects[MAX_WINGS][MAX_SHIPS_PER_WING];

extern char	*Docking_bay_list[];

extern char Fred_exe_dir[512];
extern char Fred_base_dir[512];

// Goober5000 - for voice acting manager
extern char Voice_abbrev_briefing[NAME_LENGTH];
extern char Voice_abbrev_campaign[NAME_LENGTH];
extern char Voice_abbrev_command_briefing[NAME_LENGTH];
extern char Voice_abbrev_debriefing[NAME_LENGTH];
extern char Voice_abbrev_message[NAME_LENGTH];
extern char Voice_abbrev_mission[NAME_LENGTH];
extern bool Voice_no_replace_filenames;
extern char Voice_script_entry_format[NOTES_LENGTH];
extern int Voice_export_selection;

// Goober5000
extern bool Show_iff[];

extern CCriticalSection CS_cur_object_index;

void	string_copy(char *dest, const CString &src, int max_len, int modify = 0);
void	string_copy(SCP_string &dest, const CString &src, int modify = 0);
void	convert_multiline_string(CString &dest, const SCP_string &src);
void	convert_multiline_string(CString &dest, const char *src);
void	deconvert_multiline_string(char *dest, const CString &str, int max_len);
void	deconvert_multiline_string(SCP_string &dest, const CString &str);

bool	fred_init();
void	set_physics_controls();
int	dup_object(object *objp);
int	create_object_on_grid(int waypoint_instance = -1);
int	create_object(vec3d *pos, int waypoint_instance = -1);
int	create_player(int num, vec3d *pos, matrix *orient, int type = -1, int init = 1);
void	create_new_mission();
void	reset_mission();
void	clear_mission();
int	query_valid_object(int index = cur_object_index);
int	query_valid_ship(int index = cur_object_index);
int	query_valid_waypoint(int index = cur_object_index);
void	set_cur_indices(int obj = -1);
void	set_cur_object_index(int obj = -1);
int	delete_object(int obj);
int	delete_object(object *ptr);
int	delete_ship(int ship);
void	delete_marked();
void	delete_reinforcement(int num);
int	delete_ship_from_wing(int ship = cur_ship);
int	find_free_wing();
int	query_object_in_wing(int obj = cur_object_index);
void	mark_object(int obj);
void	unmark_object(int obj);
void	unmark_all();
void	clear_menu(CMenu *ptr);
void	generate_wing_popup_menu(CMenu *mptr, int first_id, int state);
void	generate_ship_popup_menu(CMenu *mptr, int first_id, int state, int filter = 0);
int	string_lookup(const CString &str1, char *strlist[], int max);
int	update_dialog_boxes();
void	set_cur_wing(int wing);
int	gray_menu_tree(CMenu *base);
int	query_initial_orders_conflict(int wing);
int	query_initial_orders_empty(ai_goal *ai_goals);
int	set_reinforcement(char *name, int state);
int	get_docking_list(int model_index);
int	rename_ship(int ship, char *name);
void	fix_ship_name(int ship);
int	internal_integrity_check();
void	correct_marking();
int	get_ship_from_obj(int obj);
int	get_ship_from_obj(object *objp);
void	set_valid_dock_points(int ship, int type, CComboBox *box);
void	ai_update_goal_references(int type, const char *old_name, const char *new_name);
int	query_referenced_in_ai_goals(int type, const char *name);
int	advanced_stricmp(char *one, char *two);
int	reference_handler(char *name, int type, int obj);
int	orders_reference_handler(int code, char *msg);
int	sexp_reference_handler(int node, int code, char *msg);
char	*object_name(int obj);
char	*get_order_name(int order);
void	object_moved(object *ptr);
int	invalidate_references(char *name, int type);
int	query_whole_wing_marked(int wing);
void	generate_weaponry_usage_list(int team, int *arr);
void	generate_weaponry_usage_list(int *arr, int wing);
void	generate_ship_usage_list(int *arr, int wing);

CJumpNode *jumpnode_get_by_name(const CString& name);

// function and defines to use when adding ships to combo boxes
#define SHIPS_2_COMBO_SPECIAL					(1<<0)
#define SHIPS_2_COMBO_ALL_SHIPS				(1<<1)
#define SHIPS_2_COMBO_DOCKING_BAY_ONLY		(1<<2)

extern void management_add_ships_to_combo( CComboBox *box, int flags );

// Goober5000
extern int wing_is_player_wing(int wing);
extern void update_custom_wing_indexes();
extern void stuff_special_arrival_anchor_name(char *buf, int iff_index, int restrict_to_players, int retail_format);
extern void stuff_special_arrival_anchor_name(char *buf, int anchor_num, int retail_format);
extern void update_texture_replacements(const char *old_name, const char *new_name);

#endif
