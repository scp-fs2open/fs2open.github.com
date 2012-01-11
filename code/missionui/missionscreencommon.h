/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MISSION_SCREEN_COMMON_HEADER_FILE
#define _MISSION_SCREEN_COMMON_HEADER_FILE

#include "globalincs/globals.h"
#include "ui/ui.h"
#include "model/model.h"

#define BACKGROUND_FRAME_TO_START_SHIP_ANIM	87
#define BUTTON_SLIDE_IN_FRAME						1

///////////////////////////////////////////////////////
// Common to briefing/ship selection/weapons loadout
///////////////////////////////////////////////////////
#define REVOLUTION_RATE								5.2f

#define COMMON_BRIEFING_REGION					0
#define COMMON_SS_REGION							1
#define COMMON_WEAPON_REGION						2
#define COMMON_COMMIT_REGION						5
#define COMMON_HELP_REGION							6
#define COMMON_OPTIONS_REGION						7
#define NUM_COMMON_REGIONS							6

#define NUM_COMMON_BUTTONS	6

struct brief_common_buttons {	
	char *filename;
	int x, y;
	int xt, yt;
	int hotspot;
	int repeat;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	brief_common_buttons(char *name, int x1, int y1, int xt1, int yt1, int h, int r = 0) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h), repeat(r) {}
};

extern brief_common_buttons Common_buttons[3][GR_NUM_RESOLUTIONS][NUM_COMMON_BUTTONS];

extern int Background_playing;

extern int Common_select_inited;
extern int Current_screen;

extern int Common_team;

extern int Drop_icon_mflag;
extern int Drop_on_wing_mflag;
extern int Brief_mouse_up_flag;
extern int Mouse_down_last_frame;


extern int Wing_slot_empty_bitmap;
extern int Wing_slot_disabled_bitmap;

extern int Flash_timer;				//	timestamp used to start flashing
extern int Flash_toggle;			// timestamp used to toggle flashing
extern int Flash_bright;			// state of button to flash

void common_button_do(int i);

// common_select_init() performs initialization common to the briefing/ship select/weapon select
// screens.  This includes loading/setting the palette, loading the background animation, loading
// the screen switching animations, loading the button animation frames
void	common_select_init();	
int	common_select_do(float frametime);
void	common_select_close();
void	common_draw_buttons();
void	common_check_buttons();
void	common_check_keys(int k);
void	commit_pressed();
void	common_render(float frametime);
void	common_buttons_init(UI_WINDOW *ui_window);
void	common_buttons_maybe_reload(UI_WINDOW *ui_window);
void 	common_render_selected_screen_button();
void	common_reset_buttons();
void	common_redraw_pressed_buttons();
void  common_maybe_clear_focus();
void ship_select_common_init();

void common_set_interface_palette(char *filename = NULL);		// set the interface palette
void common_free_interface_palette();		// restore game palette

void load_wing_icons(char *filename);
void unload_wing_icons();

void	common_flash_button_init();
int	common_flash_bright();

// functions for the multiplayer chat window
void common_render_chat_window();
void multi_chat_scroll_up();
void multi_chat_scroll_down();

void	set_active_ui(UI_WINDOW *ui_window);

// music functions exported for multiplayer team selection screen to start briefing music
void common_music_init( int score_index );
void common_music_do();
void common_music_close();

void common_maybe_play_cutscene(int movie_type);

int common_scroll_down_pressed(int *start, int size, int max_show);
int common_scroll_up_pressed(int *start, int size, int max_show);

//////////////////////////////////////////////////////////////////////////////////////
// NEWSTUFF BEGIN
//////////////////////////////////////////////////////////////////////////////////////

#define MAX_WING_SLOTS	4
#define MAX_WING_BLOCKS	3
#define	MAX_WSS_SLOTS	(MAX_WING_BLOCKS*MAX_WING_SLOTS)

#define WING_SLOT_FILLED				(1<<0)
#define WING_SLOT_EMPTY					(1<<1)
#define WING_SLOT_IS_PLAYER			(1<<3)
#define WING_SLOT_LOCKED				(1<<4)
#define WING_SLOT_SHIPS_DISABLED		(1<<5)
#define WING_SLOT_WEAPONS_DISABLED		(1<<6)

#define WING_SLOT_DISABLED			(WING_SLOT_SHIPS_DISABLED|WING_SLOT_WEAPONS_DISABLED)
#define WING_SLOT_IGNORE_SHIPS		(WING_SLOT_SHIPS_DISABLED|WING_SLOT_LOCKED)
#define WING_SLOT_IGNORE_WEAPONS	(WING_SLOT_WEAPONS_DISABLED|WING_SLOT_LOCKED)

// different operations used in xx_apply()
#define WSS_DUMP_TO_LIST		0
#define WSS_GRAB_FROM_LIST		1
#define WSS_SWAP_SLOT_SLOT		2
#define WSS_SWAP_LIST_SLOT		3

// icons
#define NUM_ICON_FRAMES					6
#define ICON_FRAME_NORMAL				0
#define ICON_FRAME_HOT					1
#define ICON_FRAME_SELECTED			2
#define ICON_FRAME_PLAYER				3
#define ICON_FRAME_DISABLED			4
#define ICON_FRAME_DISABLED_HIGH		5

//Colors
extern color Icon_colors[NUM_ICON_FRAMES];
extern shader Icon_shaders[NUM_ICON_FRAMES];

//////////////////////////////////////////////
// Slots
//////////////////////////////////////////////
typedef struct wss_unit {
	int	ship_class;
	int	wep[MAX_SHIP_WEAPONS];
	int	wep_count[MAX_SHIP_WEAPONS];
} wss_unit;

extern wss_unit Wss_slots_teams[MAX_TVT_TEAMS][MAX_WSS_SLOTS];
extern wss_unit *Wss_slots;

extern int Wss_num_wings; // number of player wings
extern int Wss_num_wings_teams[MAX_TVT_TEAMS];

//////////////////////////////////////////////
// Weapon pool
//////////////////////////////////////////////
extern int Wl_pool_teams[MAX_TVT_TEAMS][MAX_WEAPON_TYPES];
extern int *Wl_pool;

//////////////////////////////////////////////
// Ship pool
//////////////////////////////////////////////
extern int Ss_pool_teams[MAX_TVT_TEAMS][MAX_SHIP_CLASSES];
extern int *Ss_pool;

//////////////////////////////////////////////
// Saving loadout
//////////////////////////////////////////////
typedef struct loadout_data 
{
	char				filename[MAX_FILENAME_LEN];				// mission filename
	char				last_modified[DATE_TIME_LENGTH];	// when mission was last modified
	wss_unit			unit_data[MAX_WSS_SLOTS];			// ship and weapon data
	int				weapon_pool[MAX_WEAPON_TYPES];	// available weapons
	int				ship_pool[MAX_SHIP_CLASSES];			// available ships
} loadout_data;

extern loadout_data Player_loadout;

void wss_save_loadout();
void wss_restore_loadout();
void wss_direct_restore_loadout();

int wss_get_mode(int from_slot, int from_list, int to_slot, int to_list, int wl_ship_slot);
int store_wss_data(ubyte *block, int max_size, int sound,int player_index);
int restore_wss_data(ubyte *block);

struct ship_info;
void draw_model_icon(int model_id, int flags, float closeup_zoom, int x1, int x2, int y1, int y2, ship_info* sip=NULL, bool resize=true);
void draw_model_rotating(int model_id, int x1, int y1, int x2, int y2, float *rotation_buffer, vec3d *closeup_pos=NULL, float closeup_zoom = .65f, float rev_rate = REVOLUTION_RATE, int flags = MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING, bool resize=true, int effect = 2);

void common_set_team_pointers(int team);
void common_reset_team_pointers();

///////////////////////////////////////////////////////////
// NEWSTUFF END
///////////////////////////////////////////////////////////

#endif
