/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __MISSION_WEAPON_CHOICE_H__
#define __MISSION_WEAPON_CHOICE_H__

class p_object;
struct wss_unit;
class ship_weapon;

// mask regions for icons in the scrollable lists
#define ICON_PRIMARY_0				28
#define ICON_PRIMARY_1				29
#define ICON_PRIMARY_2				30
#define ICON_PRIMARY_3				31
#define ICON_SECONDARY_0			10
#define ICON_SECONDARY_1			11
#define ICON_SECONDARY_2			12
#define ICON_SECONDARY_3			13

// mask regions for icons that sit above the ship
#define ICON_SHIP_PRIMARY_0		32
#define ICON_SHIP_PRIMARY_1		33
#define ICON_SHIP_PRIMARY_2		34
#define ICON_SHIP_SECONDARY_0		35
#define ICON_SHIP_SECONDARY_1		36
#define ICON_SHIP_SECONDARY_2		37
#define ICON_SHIP_SECONDARY_3		38

#define WEAPON_DESC_MAX_LINES			7				// max lines in the description incl. title
#define WEAPON_DESC_MAX_LENGTH		50				// max chars per line of description text

extern int Weapon_select_overlay_id;

void weapon_select_init();
void weapon_select_common_init();
void weapon_select_do(float frametime);
void weapon_select_close();
void weapon_select_close_team();

void draw_3d_overhead_view(int model_num,
	int ship_class,
	float* rotation_buffer,
	float frametime,
	int selected_ship_slot,
	int selected_weapon_class,
	int hovered_weapon_slot,
	int x1,
	int y1,
	int x2,
	int y2,
	int resize_mode,
	int bank1_x,
	int bank1_y,
	int bank2_x,
	int bank2_y,
	int bank3_x,
	int bank3_y,
	int bank4_x,
	int bank4_y,
	int bank5_x,
	int bank5_y,
	int bank6_x,
	int bank6_y,
	int bank7_x,
	int bank7_y,
	int bank_prim_offset = 106,
	int bank_sec_offset = -50,
	int bank_y_offset = 12,
	int style = -1);

void	wl_update_parse_object_weapons(p_object *pobjp, wss_unit *slot);
int	wl_update_ship_weapons(int objnum, wss_unit *slot);
void	wl_bash_ship_weapons(ship_weapon *swp, wss_unit *slot);

void wl_set_default_weapons(int index, int ship_class);
void wl_reset_to_defaults();
void wl_fill_slots();

// Set selected slot to first placed ship
void wl_reset_selected_slot();

void wl_remove_weps_from_pool(int *wep, int *wep_count, int ship_class);
void wl_get_ship_class_weapons(int ship_class, int *wep, int *wep_count);
void wl_get_default_weapons(int ship_class, int slot_num, int *wep, int *wep_count);
int eval_weapon_flag_for_game_type(int weapon_flags);
int wl_calc_missile_fit(int wi_index, int capacity);

void wl_synch_interface();
int wl_apply(int mode,int from_bank,int from_list,int to_bank,int to_list,int ship_slot,int player_index = -1, bool dont_play_sound = false);
int wl_drop(int from_bank,int from_list,int to_bank,int to_list, int ship_slot,int player_index = -1, bool dont_play_sound = false);

#endif /* __MISSION_WEAPON_CHOICE_H__ */
