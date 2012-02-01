/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "missionui/missionscreencommon.h"
#include "missionui/missionweaponchoice.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "missionui/missionshipchoice.h"
#include "io/mouse.h"
#include "menuui/snazzyui.h"
#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "missionui/missionbrief.h"
#include "gamesnd/gamesnd.h"
#include "gamehelp/contexthelp.h"
#include "popup/popup.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "render/3d.h"
#include "lighting/lighting.h"
#include "hud/hudbrackets.h"
#include "model/model.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiteamselect.h"
#include "network/multiui.h"
#include "missionui/chatbox.h"
#include "network/multi_pmsg.h"
#include "parse/parselo.h"
#include "io/timer.h"


#define IS_BANK_PRIMARY(x)			(x < MAX_SHIP_PRIMARY_BANKS)
#define IS_BANK_SECONDARY(x)		(x >= MAX_SHIP_PRIMARY_BANKS)

#define IS_LIST_PRIMARY(x)			(Weapon_info[x].subtype != WP_MISSILE)
#define IS_LIST_SECONDARY(x)		(Weapon_info[x].subtype == WP_MISSILE)


//////////////////////////////////////////////////////////////////
// Game-wide globals
//////////////////////////////////////////////////////////////////

// This game-wide global flag is set to 1 to indicate that the weapon
// select screen has been opened and memory allocated.  This flag
// is needed so we can know if weapon_select_close() needs to called if
// restoring a game from the Options screen invoked from weapon select
int Weapon_select_open = 0;

//////////////////////////////////////////////////////////////////
// UI Data
//////////////////////////////////////////////////////////////////

typedef struct wl_bitmap_group
{
	int first_frame;
	int num_frames;
} wl_bitmap_group;

#define WEAPON_ANIM_LOOP_FRAME				52			// frame (from 0) to loop weapon anim

#define WEAPON_ICON_FRAME_NORMAL				0
#define WEAPON_ICON_FRAME_HOT					1
#define WEAPON_ICON_FRAME_SELECTED			2
#define WEAPON_ICON_FRAME_DISABLED			3


#define NUM_WEAPON_SETTINGS	2

#define MAX_WEAPON_BUTTONS	8
#define MIN_WEAPON_BUTTONS	7
#define NUM_WEAPON_BUTTONS	(Uses_apply_all_button ? MAX_WEAPON_BUTTONS : MIN_WEAPON_BUTTONS)

// Weapn loadout specific buttons
#define WL_BUTTON_SCROLL_PRIMARY_UP			0
#define WL_BUTTON_SCROLL_PRIMARY_DOWN		1
#define WL_BUTTON_SCROLL_SECONDARY_UP		2
#define WL_BUTTON_SCROLL_SECONDARY_DOWN		3
#define WL_BUTTON_RESET						4
#define WL_BUTTON_DUMMY						5
#define WL_BUTTON_MULTI_LOCK				6
#define WL_BUTTON_APPLY_ALL					7

extern int anim_timer_start;

// convenient struct for handling all button controls
struct wl_buttons {
	char *filename;
	int x, y, xt, yt;
	int hotspot;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	wl_buttons(char *name, int x1, int y1, int xt1, int yt1, int h) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h) {}
};

static wl_buttons Buttons[GR_NUM_RESOLUTIONS][MAX_WEAPON_BUTTONS] = {
	{
		wl_buttons("WLB_27",		24,		276,	-1,		-1,		27),	// WL_BUTTON_SCROLL_PRIMARY_UP
		wl_buttons("WLB_26",		24,		125,	-1,		-1,		26),	// WL_BUTTON_SCROLL_PRIMARY_DOWN
		wl_buttons("WLB_09",		24,		454,	-1,		-1,		9),		// WL_BUTTON_SCROLL_SECONDARY_UP
		wl_buttons("WLB_08",		24,		303,	-1,		-1,		8),		// WL_BUTTON_SCROLL_SECONDARY_DOWN
		wl_buttons("ssb_39",		571,	347,	-1,		-1,		39),	// WL_BUTTON_RESET
		wl_buttons("ssb_39",		0,		0,		-1,		-1,		99),	// WL_BUTTON_DUMMY
		wl_buttons("TSB_34",		603,	374,	-1,		-1,		34),	// WL_BUTTON_MULTI_LOCK
		wl_buttons("WLB_40",		0,		90,		-1,		-1,		40)		// WL_BUTTON_APPLY_ALL
	},
	{
		wl_buttons("2_WLB_27",		39,		442,	-1,		-1,		27),
		wl_buttons("2_WLB_26",		39,		200,	-1,		-1,		26),
		wl_buttons("2_WLB_09",		39,		727,	-1,		-1,		9),
		wl_buttons("2_WLB_08",		39,		485,	-1,		-1,		8),
		wl_buttons("2_ssb_39",		913,	556,	-1,		-1,		39),
		wl_buttons("2_ssb_39",		0,		0,		-1,		-1,		99),
		wl_buttons("2_TSB_34",		966,	599,	-1,		-1,		34),
		wl_buttons("2_WLB_40",		0,		138,	-1,		-1,		40)
	}
};


static char *Wl_mask_single[NUM_WEAPON_SETTINGS][GR_NUM_RESOLUTIONS] = {
	{
		"weaponloadout-m",
		"2_weaponloadout-m"
	},
	{
		"weaponloadout-mb",
		"2_weaponloadout-mb"
	}
};

static char *Wl_mask_multi[NUM_WEAPON_SETTINGS][GR_NUM_RESOLUTIONS] = {
	{
		"weaponloadoutmulti-m",
		"2_weaponloadoutmulti-m"
	},

	{
		"weaponloadoutmulti-mb",
		"2_weaponloadoutmulti-mb"
	}
};

static char *Wl_loadout_select_mask[NUM_WEAPON_SETTINGS][GR_NUM_RESOLUTIONS] = {
	{
		"weaponloadout-m",
		"2_weaponloadout-m"
	},
	{
		"weaponloadout-mb",
		"2_weaponloadout-mb"
	}
};


static char *Weapon_select_background_fname[NUM_WEAPON_SETTINGS][GR_NUM_RESOLUTIONS] = {
	{
		"WeaponLoadout",
		"2_WeaponLoadout"
	},
	{
		"WeaponLoadoutb",
		"2_WeaponLoadoutb"
	}
};

static char *Weapon_select_multi_background_fname[NUM_WEAPON_SETTINGS][GR_NUM_RESOLUTIONS] = {
	{
		"WeaponLoadoutMulti",
		"2_WeaponLoadoutMulti"
	},
	{
		"WeaponLoadoutMultib",
		"2_WeaponLoadoutMultib"
	}
};

static int Uses_apply_all_button = 0;

int Weapon_select_background_bitmap;	// bitmap for weapon select brackground

static MENU_REGION	Weapon_select_region[NUM_COMMON_REGIONS + 27];	// see initialization
static int				Num_weapon_select_regions;

// Mask bitmap pointer and Mask bitmap_id
static bitmap*	WeaponSelectMaskPtr;		// bitmap pointer to the weapon select mask bitmap
static ubyte*	WeaponSelectMaskData;	// pointer to actual bitmap data
static int		Weaponselect_mask_w, Weaponselect_mask_h;
static int		WeaponSelectMaskBitmap;	// bitmap id of the weapon select mask bitmap
static int		Weapon_slot_bitmap;

UI_WINDOW	Weapon_ui_window;

static int Weapon_button_scrollable[MAX_WEAPON_BUTTONS] = {0, 0, 0, 0, 0, 0, 0, 0};

#define MAX_WEAPON_ICONS_ON_SCREEN 8

// X and Y locations of the weapon icons in the scrollable lists
static int Wl_weapon_icon_coords[GR_NUM_RESOLUTIONS][MAX_WEAPON_ICONS_ON_SCREEN][2] = {
	{
		{27, 152},
		{27, 182},
		{27, 212},
		{27, 242},
		{36, 331},
		{36, 361},
		{36, 391},
		{36, 421}
	},
	{
		{59, 251},
		{59, 299},
		{59, 347},
		{59, 395},
		{59, 538},
		{59, 586},
		{59, 634},
		{59, 682}
	}
};

static int Wl_bank_coords[GR_NUM_RESOLUTIONS][MAX_SHIP_WEAPONS][2] = {
	{
		{106,127},
		{106,158},
		{106,189},
		{322,127},
		{322,158},
		{322,189},
		{322,220},
	},
	{
		{170,203},
		{170,246},
		{170,290},
		{552,203},
		{552,246},
		{552,290},
		{552,333},
	}
};

static int Wl_bank_count_draw_flags[MAX_SHIP_WEAPONS] = { 
	0, 0, 0,			// primaries -- don't draw counts
	1, 1, 1, 1		// secondaries -- do draw counts
};


static int Weapon_anim_class = -1;
static int Last_wl_ship_class;

static int Wl_overhead_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		// GR_640
		91, 117
	},			
	{
		// GR_1024
		156, 183
	}
};

static int Wl_weapon_ani_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		408, 82			// GR_640
	},
	{
		648, 128			// GR_1024
	}
};

static int Wl_weapon_ani_coords_multi[GR_NUM_RESOLUTIONS][2] = {
	{
		408, 143			// GR_640
	},
	{
		648, 226			// GR_1024
	}
};


static int Wl_weapon_desc_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		508, 283		// GR_640
	},
	{
		813, 453		// GR_1024
	}
};

static int Wl_delta_x, Wl_delta_y;

static int Wl_ship_name_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		85, 106
	},
	{
		136, 170
	}
};


///////////////////////////////////////////////////////////////////////
// UI data structs
///////////////////////////////////////////////////////////////////////
typedef struct wl_ship_class_info
{
	int				overhead_bitmap;
	int				model_num;
	generic_anim	animation;
} wl_ship_class_info;

wl_ship_class_info	Wl_ships[MAX_SHIP_CLASSES];

typedef struct wl_icon_info
{
	int				icon_bmaps[NUM_ICON_FRAMES];
	int				laser_bmap;
	int				model_index;
	int				can_use;
	generic_anim	animation;
} wl_icon_info;

wl_icon_info	Wl_icons_teams[MAX_TVT_TEAMS][MAX_WEAPON_TYPES];
wl_icon_info	*Wl_icons = NULL;

int Plist[MAX_WEAPON_TYPES];	// used to track scrolling of primary icon list
int Plist_start, Plist_size;

int Slist[MAX_WEAPON_TYPES];	// used to track scrolling of primary icon list
int Slist_start, Slist_size;

static int Selected_wl_slot = -1;			// Currently selected ship slot
static int Selected_wl_class = -1;			// Class of weapon that is selected
static int Hot_wl_slot = -1;					//	Ship slot that mouse is over (0..MAX_WSS_SLOTS-1)
static int Hot_weapon_icon = -1;				// Icon number (0-7) which has mouse over it
static int Hot_weapon_bank = -1;				// index (0-7) for weapon slot on ship that has a droppable icon over it
static int Hot_weapon_bank_icon = -1;
static generic_anim Cur_Anim;

static int Wl_mouse_down_on_region = -1;


// weapon desc stuff
#define WEAPON_DESC_WIPE_TIME			1.5f			// time in seconds for wipe to occur (over WEAPON_DESC_MAX_LENGTH number of chars)
#define WEAPON_DESC_MAX_LINES			7				// max lines in the description incl. title
#define WEAPON_DESC_MAX_LENGTH		50				// max chars per line of description text
static int Weapon_desc_wipe_done = 0;
static float Weapon_desc_wipe_time_elapsed = 0.0f;
static char Weapon_desc_lines[WEAPON_DESC_MAX_LINES][WEAPON_DESC_MAX_LENGTH];			// 1st 2 lines are title, rest are desc

// maximum width the weapon title can be -- used in the line breaking 
int Weapon_title_max_width[GR_NUM_RESOLUTIONS] = { 200, 320 };

static int Wl_new_weapon_title_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		408, 75		// GR_640
	},
	{
		648, 118		// GR_1024
	}
};

static int Wl_new_weapon_title_coords_multi[GR_NUM_RESOLUTIONS][2] = {
	{
		408, 136		// GR_640
	},
	{
		648, 216		// GR_1024
	}
};

static int Wl_new_weapon_desc_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		408, 247		// GR_640
	},
	{
		648, 395		// GR_1024
	}
};

static int Wl_new_weapon_desc_coords_multi[GR_NUM_RESOLUTIONS][2] = {
	{
		408, 308		// GR_640
	},
	{
		648, 493		// GR_1024
	}
};

// ship select text
#define WEAPON_SELECT_NUM_TEXT			2
UI_XSTR Weapon_select_text[GR_NUM_RESOLUTIONS][WEAPON_SELECT_NUM_TEXT] = {
	{ // GR_640
		{ "Reset",			1337,		580,	337,	UI_XSTR_COLOR_GREEN, -1, &Buttons[0][WL_BUTTON_RESET].button },
		{ "Lock",			1270,		602,	364,	UI_XSTR_COLOR_GREEN, -1, &Buttons[0][WL_BUTTON_MULTI_LOCK].button }
	}, 
	{ // GR_1024
		{ "Reset",			1337,		938,	546,	UI_XSTR_COLOR_GREEN, -1, &Buttons[1][WL_BUTTON_RESET].button },
		{ "Lock",			1270,		964,	584,	UI_XSTR_COLOR_GREEN, -1, &Buttons[1][WL_BUTTON_MULTI_LOCK].button }
	}
};


///////////////////////////////////////////////////////////////////////
// Carried Icon
///////////////////////////////////////////////////////////////////////
typedef struct carried_icon
{
	int weapon_class;		// index Wl_icons[] for carried icon (-1 if carried from bank)
	int num;					// number of units of weapon
	int from_bank;			// bank index that icon came from (0..2 primary, 3..6 secondary).  -1 if from list
	int from_slot;			// ship slot that weapon is part of 
	int from_x, from_y;
} carried_icon;

static carried_icon Carried_wl_icon;

// forward declarations
void draw_wl_icons();
void wl_draw_ship_weapons(int index);
void wl_pick_icon_from_list(int index);
void pick_from_ship_slot(int num);
void start_weapon_animation(int weapon_class);
void stop_weapon_animation();
int wl_get_pilot_subsys_index(p_object *pobjp);

void wl_reset_to_defaults();

void wl_set_selected_slot(int slot_num);
void wl_maybe_reset_selected_slot();
void wl_maybe_reset_selected_weapon_class();

void wl_render_icon_count(int num, int x, int y);
void wl_render_weapon_desc();

void wl_apply_current_loadout_to_all_ships_in_current_wing();

// carry icon functions
void wl_reset_carried_icon();
int wl_icon_being_carried();
void wl_set_carried_icon(int from_bank, int from_slot, int weapon_class);


char *wl_tooltip_handler(char *str)
{
	if (Selected_wl_class < 0)
		return NULL;

	if (!stricmp(str, "@weapon_desc")) {
		char *str2;
		int x, y, w, h;

		str2 = Weapon_info[Selected_wl_class].desc;
		gr_get_string_size(&w, &h, str2);
		x = Wl_weapon_desc_coords[gr_screen.res][0] - w / 2;
		y = Wl_weapon_desc_coords[gr_screen.res][1] - h / 2;

		gr_set_color_fast(&Color_black);
		gr_rect(x - 5, y - 5, w + 10, h + 10);

		gr_set_color_fast(&Color_bright_white);
		gr_string(x, y, str2);
		return NULL;
	}

	return NULL;
}

/**
 * Reset the data inside Carried_wl_icon
 */
void wl_reset_carried_icon()
{
	Carried_wl_icon.weapon_class = -1;
	Carried_wl_icon.num = 0;
	Carried_wl_icon.from_bank = -1;
	Carried_wl_icon.from_slot = -1;
}

/**
 * Is an icon being carried?
 */
int wl_icon_being_carried()
{
	if ( Carried_wl_icon.weapon_class >= 0 ) {
		return 1;
	}

	return 0;
}

/**
 * Set carried icon data 
 */
void wl_set_carried_icon(int from_bank, int from_slot, int weapon_class)
{
	int mx,my;

	Carried_wl_icon.from_bank = from_bank;
	Carried_wl_icon.from_slot = from_slot;
	Carried_wl_icon.weapon_class = weapon_class;

	mouse_get_pos_unscaled( &mx, &my );
	Carried_wl_icon.from_x=mx;
	Carried_wl_icon.from_y=my;

	Buttons[gr_screen.res][WL_BUTTON_DUMMY].button.capture_mouse();
}

/**
 * Determine if the carried icon has moved
 */
int wl_carried_icon_moved()
{
	int mx, my;
	mouse_get_pos_unscaled( &mx, &my );
	if ( Carried_wl_icon.from_x != mx || Carried_wl_icon.from_y != my) {
		return 1;
	}

	return 0;
}

/**
 * @return the index for the pilot subsystem in the parse object
 */
int wl_get_pilot_subsys_index(p_object *pobjp)
{
	int pilot_index, start_index, end_index, i;

	// see if there is a PILOT subystem
	start_index = pobjp->subsys_index;
	end_index = start_index + pobjp->subsys_count;
	pilot_index = -1;
	for ( i = start_index; i < end_index; i++ ) {
		if ( !subsystem_stricmp(Subsys_status[i].name, NOX("pilot") ) ) {
			pilot_index = i;
			break;
		}
	}

	if ( pilot_index == -1 ) {
		Error(LOCATION,"Parse object doesn't have a pilot subsystem\n");
		return -1;
	}

	return pilot_index;
}

// ---------------------------------------------------------------------------------
// weapon_button_do()
//
void weapon_button_do(int i)
{
	switch ( i ) {
			case WL_BUTTON_SCROLL_PRIMARY_UP:
				if ( common_scroll_up_pressed(&Plist_start, Plist_size, 4) ) {
					gamesnd_play_iface(SND_SCROLL);
				} else {
					gamesnd_play_iface(SND_GENERAL_FAIL);
				}
			break;

			case WL_BUTTON_SCROLL_PRIMARY_DOWN:
				if ( common_scroll_down_pressed(&Plist_start, Plist_size, 4) ) {
					gamesnd_play_iface(SND_SCROLL);
				} else {
					gamesnd_play_iface(SND_GENERAL_FAIL);
				}
			break;

			case WL_BUTTON_SCROLL_SECONDARY_UP:
				if ( common_scroll_up_pressed(&Slist_start, Slist_size, 4) ) {
					gamesnd_play_iface(SND_SCROLL);
				} else {
					gamesnd_play_iface(SND_GENERAL_FAIL);
				}
			break;

			case WL_BUTTON_SCROLL_SECONDARY_DOWN:
				if ( common_scroll_down_pressed(&Slist_start, Slist_size, 4) ) {
					gamesnd_play_iface(SND_SCROLL);
				} else {
					gamesnd_play_iface(SND_GENERAL_FAIL);
				}
			break;

			case WL_BUTTON_RESET:
				wl_reset_to_defaults();
				break;

			case WL_BUTTON_MULTI_LOCK:
				Assert(Game_mode & GM_MULTIPLAYER);				
				// the "lock" button has been pressed
				multi_ts_lock_pressed();

				// disable the button if it is now locked
				if(multi_ts_is_locked()){
					Buttons[gr_screen.res][WL_BUTTON_MULTI_LOCK].button.disable();
				}
				break;

			case WL_BUTTON_APPLY_ALL:
				wl_apply_current_loadout_to_all_ships_in_current_wing();
				break;

			default:
				popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, "Button %d is not yet implemented", i);
				break;
	}
}

/**
 * Check if any weapons loadout screen buttons have been pressed, and 
 * call weapon_button_do() if they have.
 */
void weapon_check_buttons()
{
	int			i;
	wl_buttons	*b;

	for ( i = 0; i < NUM_WEAPON_BUTTONS; i++ ) {
		b = &Buttons[gr_screen.res][i];
		
		if ( b->button.pressed() ) {
			weapon_button_do(i);
		}
	}
}

/**
 * Redraw any weapon loadout buttons that are pressed down.  This function is needed
 * since we sometimes need to draw pressed buttons last to ensure the entire
 * button gets drawn (and not overlapped by other buttons)
 */
void wl_redraw_pressed_buttons()
{
	int			i;
	wl_buttons	*b;
	
	common_redraw_pressed_buttons();

	for ( i = 0; i < NUM_WEAPON_BUTTONS; i++ ) {
		b = &Buttons[gr_screen.res][i];
		if ( b->button.button_down() ) {
			b->button.draw_forced(2);
		}
	}
}

// ---------------------------------------------------------------------------------
// weapon_buttons_init()
//
void weapon_buttons_init()
{
	wl_buttons	*b;
	int			i;

	for ( i = 0; i < NUM_WEAPON_BUTTONS; i++ ) {
		b = &Buttons[gr_screen.res][i];
		b->button.create( &Weapon_ui_window, "", Buttons[gr_screen.res][i].x, Buttons[gr_screen.res][i].y, 60, 30, Weapon_button_scrollable[i]);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action( common_play_highlight_sound );
		b->button.set_bmaps(Buttons[gr_screen.res][i].filename);
		b->button.link_hotspot(Buttons[gr_screen.res][i].hotspot);
	}

	if ( Game_mode & GM_MULTIPLAYER ) {
		Buttons[gr_screen.res][WL_BUTTON_RESET].button.hide();
		Buttons[gr_screen.res][WL_BUTTON_RESET].button.disable();		

		// if we're not the host of the game (or a team captain in team vs. team mode), disable the lock button
		if(Netgame.type_flags & NG_TYPE_TEAM){
			if(!(Net_player->flags & NETINFO_FLAG_TEAM_CAPTAIN)){
				Buttons[gr_screen.res][WL_BUTTON_MULTI_LOCK].button.disable();		
			}
		} else {
			if(!(Net_player->flags & NETINFO_FLAG_GAME_HOST)){
				Buttons[gr_screen.res][WL_BUTTON_MULTI_LOCK].button.disable();				
			}
		}
	} else {		
		Buttons[gr_screen.res][WL_BUTTON_MULTI_LOCK].button.hide();
		Buttons[gr_screen.res][WL_BUTTON_MULTI_LOCK].button.disable();
	}

	// add all xstrs
	for(i=0; i<WEAPON_SELECT_NUM_TEXT; i++) {
		Weapon_ui_window.add_XSTR(&Weapon_select_text[gr_screen.res][i]);
	}

	Buttons[gr_screen.res][WL_BUTTON_DUMMY].button.hide();
	Buttons[gr_screen.res][WL_BUTTON_DUMMY].button.disable();	
}

// ---------------------------------------------------------------------------------
// wl_render_overhead_view()
//
void wl_render_overhead_view(float frametime)
{
	//For 3d ships
	static float WeapSelectScreenShipRot = 0.0f;
	int new_ship = 0;
	static int display_type = -1;

	if ( Selected_wl_slot == -1 ) {
		return;
	}

	wl_ship_class_info	*wl_ship;
	int						ship_class;

	Assert( Wss_slots != NULL );

	ship_class = Wss_slots[Selected_wl_slot].ship_class;
	if (ship_class < 0 || ship_class > Num_ship_classes)
	{
		Warning(LOCATION, "Invalid ship class (%d) passed for render_overhead_view", ship_class);
		return;
	}
	ship_info * sip = &Ship_info[ship_class];

	// check if ship class has changed and maybe play sound
	if (Last_wl_ship_class != ship_class) {
		if (Last_wl_ship_class != -1) {
			gamesnd_play_iface(SND_ICON_DROP);
		}
		Last_wl_ship_class = ship_class;
		new_ship = 1;
	}

	wl_ship = &Wl_ships[ship_class];
	ship_class = Wss_slots[Selected_wl_slot].ship_class;

	if (new_ship)
	{
		display_type = -1;

		if(Cmdline_ship_choice_3d || !strlen(sip->overhead_filename))
		{
			if (wl_ship->model_num < 0)
			{
				wl_ship->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
				model_page_in_textures(wl_ship->model_num, ship_class);
			}

			if(wl_ship->model_num > -1)
			{
				if(Cmdline_ship_choice_3d)
				{
					display_type = 2;
				}
				else
				{
					display_type = 1;
				}
			}
		}

		if(display_type < 0)
		{
			if ( wl_ship->overhead_bitmap < 0 )
			{
				//Load the anim?
				if (gr_screen.res == GR_640)
				{
					// lo-res
					wl_ship->overhead_bitmap = bm_load_animation(sip->overhead_filename);
				} else {
					// high-res
					char filename[NAME_LENGTH+2] = "2_";
					strcat_s(filename, sip->overhead_filename);
					wl_ship->overhead_bitmap = bm_load_animation(sip->overhead_filename);
				}

				// load the bitmap
				if (gr_screen.res == GR_640)
				{
					// lo-res
					wl_ship->overhead_bitmap = bm_load(sip->overhead_filename);
				} else {
					// high-res
					char filename[NAME_LENGTH+2] = "2_";
					strcat_s(filename, sip->overhead_filename);
					wl_ship->overhead_bitmap = bm_load(filename);
				}
			}

			//Did we load anything?
			if ( wl_ship->overhead_bitmap < 0 )
			{
				Warning(LOCATION, "Unable to load overhead image for ship '%s', generating one instead", sip->name);
				display_type = 1;
			} else {
				display_type = 0;
			}
		}
	}
		
	//Maybe do 2D
	if(display_type == 0 && wl_ship->overhead_bitmap > -1)
	{
		gr_set_bitmap(wl_ship->overhead_bitmap);
		gr_bitmap(Wl_overhead_coords[gr_screen.res][0], Wl_overhead_coords[gr_screen.res][1]);
	}
	else
	{
		// Load the necessary model file, if necessary
		if (wl_ship->model_num < 0)
		{
			wl_ship->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
			model_page_in_textures(wl_ship->model_num, ship_class);
		}
		
		if (wl_ship->model_num < 0)
		{
			mprintf(("Couldn't load model file in missionweaponchoice.cpp\n"));
		}
		else
		{
			matrix	object_orient	= IDENTITY_MATRIX;
			angles rot_angles;
			float zoom;
			zoom = sip->closeup_zoom * 1.3f;

			if(!Cmdline_ship_choice_3d)
			{
				rot_angles.p = -(3.14159f * 0.5f);
				rot_angles.b = 0.0f;
				rot_angles.h = 0.0f;
				vm_angles_2_matrix(&object_orient, &rot_angles);
			}
			else
			{
				float rev_rate;
				rev_rate = REVOLUTION_RATE;
				if (sip->flags & SIF_BIG_SHIP) {
					rev_rate *= 1.7f;
				}
				if (sip->flags & SIF_HUGE_SHIP) {
					rev_rate *= 3.0f;
				}

				WeapSelectScreenShipRot += PI2 * frametime / rev_rate;
				while (WeapSelectScreenShipRot > PI2){
					WeapSelectScreenShipRot -= PI2;	
				}

				rot_angles.p = -0.6f;
				rot_angles.b = 0.0f;
				rot_angles.h = 0.0f;
				vm_angles_2_matrix(&object_orient, &rot_angles);

				rot_angles.p = 0.0f;
				rot_angles.b = 0.0f;
				rot_angles.h = WeapSelectScreenShipRot;
				vm_rotate_matrix_by_angles(&object_orient, &rot_angles);
			}

			gr_set_clip(Wl_overhead_coords[gr_screen.res][0], Wl_overhead_coords[gr_screen.res][1], gr_screen.res == 0 ? 291 : 467, gr_screen.res == 0 ? 226 : 362);
			g3_start_frame(1);
			g3_set_view_matrix( &sip->closeup_pos, &Eye_matrix, zoom);
			model_set_detail_level(0);

			if (!Cmdline_nohtl) {
				gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
				gr_set_view_matrix(&Eye_position, &Eye_matrix);
			}

			light_reset();
			vec3d light_dir = vmd_zero_vector;
			light_dir.xyz.x = 0.5;
			light_dir.xyz.y = 2.0f;
			light_dir.xyz.z = -2.0f;	
			light_add_directional(&light_dir, 0.65f, 1.0f, 1.0f, 1.0f);
			light_rotate_all();

			model_clear_instance(wl_ship->model_num);
			model_render(wl_ship->model_num, &object_orient, &vmd_zero_vector, MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING, -1, -1);

			//NOW render the lines for weapons
			gr_reset_clip();
			vertex draw_point;
			vec3d subobj_pos;
			int x, y;
			int xc, yc;
			polymodel *pm = model_get(wl_ship->model_num);
			int num_found = 2;

			//Render selected primary lines
			for(x = 0; x < pm->n_guns; x++)
			{
				if((Wss_slots[Selected_wl_slot].wep[x] == Selected_wl_class && Hot_weapon_bank < 0) || x == Hot_weapon_bank)
				{
					Assert(num_found < NUM_ICON_FRAMES);
					gr_set_color_fast(&Icon_colors[ICON_FRAME_NORMAL + num_found]);
					gr_circle(Wl_bank_coords[gr_screen.res][x][0] + 106, Wl_bank_coords[gr_screen.res][x][1] + 12, 5);
					for(y = 0; y < pm->gun_banks[x].num_slots; y++)
					{
						//Stuff
						vm_vec_unrotate(&subobj_pos,&pm->gun_banks[x].pnt[y],&object_orient);
						g3_rotate_vertex(&draw_point, &subobj_pos);
						g3_project_vertex(&draw_point);
						gr_unsize_screen_posf(&draw_point.screen.xyw.x, &draw_point.screen.xyw.y);

						xc = fl2i(draw_point.screen.xyw.x + Wl_overhead_coords[gr_screen.res][0]);
						yc = fl2i(draw_point.screen.xyw.y +Wl_overhead_coords[gr_screen.res][1]);

						//get the curve right.
						int curve;
						if ((xc > Wl_bank_coords[gr_screen.res][x][0] + 106) && (Wl_bank_coords[gr_screen.res][x][1] + 12 < yc))
							curve = 1;
						else if ((xc < Wl_bank_coords[gr_screen.res][x][0] + 106) && (Wl_bank_coords[gr_screen.res][x][1] + 12 < yc))
							curve = 0;
						else if ((xc > Wl_bank_coords[gr_screen.res][x][0] + 106) && (Wl_bank_coords[gr_screen.res][x][1] + 12 > yc))
							curve = 3;
						else
							curve = 2;

						int lineendx;
						int lineendy;
						if (curve == 0) {
							lineendx = xc + 4;
						} else {
							lineendx = xc - 4;
						}

						gr_line(Wl_bank_coords[gr_screen.res][x][0] + 106, Wl_bank_coords[gr_screen.res][x][1] + 12, lineendx, Wl_bank_coords[gr_screen.res][x][1] + 12);
						
						if (curve == 0 || curve == 2)
							lineendx = xc;

						if (curve == 0 || curve == 1) {
							lineendy = Wl_bank_coords[gr_screen.res][x][1] + 12;
						} else {
							lineendy = Wl_bank_coords[gr_screen.res][x][1] + 7;
						}
						
						gr_curve(lineendx, lineendy, 5, curve);
						
						if (curve == 0 || curve == 1) {
							lineendy = Wl_bank_coords[gr_screen.res][x][1] + 17;
						} else {
							lineendy = Wl_bank_coords[gr_screen.res][x][1] + 7;
						}

						gr_line(xc, lineendy, xc, yc);
						gr_circle(xc, yc, 5);
					}
					num_found++;
				}
			}

			num_found = 2;
			//Render selected secondary lines
			for(x = 0; x < pm->n_missiles; x++)
			{
				if((Wss_slots[Selected_wl_slot].wep[x + MAX_SHIP_PRIMARY_BANKS] == Selected_wl_class && Hot_weapon_bank < 0) || x + MAX_SHIP_PRIMARY_BANKS == Hot_weapon_bank)
				{
					Assert(num_found < NUM_ICON_FRAMES);
					gr_set_color_fast(&Icon_colors[ICON_FRAME_NORMAL + num_found]);
					gr_circle(Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][0] - 50, Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 12, 5);
					for(y = 0; y < pm->missile_banks[x].num_slots; y++)
					{
						vm_vec_unrotate(&subobj_pos,&pm->missile_banks[x].pnt[y],&object_orient);
						g3_rotate_vertex(&draw_point, &subobj_pos);
						g3_project_vertex(&draw_point);
						gr_unsize_screen_posf(&draw_point.screen.xyw.x, &draw_point.screen.xyw.y);

						xc = fl2i(draw_point.screen.xyw.x + Wl_overhead_coords[gr_screen.res][0]);
						yc = fl2i(draw_point.screen.xyw.y +Wl_overhead_coords[gr_screen.res][1]);

						//get the curve right.
						int curve;
						if ((xc > Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][0] - 50) && (Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 12 < yc))
							curve = 1;
						else if ((xc < Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][0] - 50) && (Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 12 < yc))
							curve = 0;
						else if ((xc > Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][0] - 50) && (Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 12 > yc))
							curve = 3;
						else
							curve = 2;

						int lineendx;
						int lineendy;
						if (curve == 1 || curve == 3)
							lineendx = xc - 4;
						else
							lineendx = xc + 4;

						gr_line(Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][0] - 50, Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 12, lineendx, Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 12);
						
						if (curve == 1 || curve == 2) {
							lineendy = Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 7;
						} else {
							lineendy = Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 12;
						}
						gr_curve(xc, lineendy, 5, curve);
						
						if (curve == 1 || curve == 2) {
							lineendy = Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 7;
						} else {
							lineendy = Wl_bank_coords[gr_screen.res][x + MAX_SHIP_PRIMARY_BANKS][1] + 17;
						}
						
						gr_line(xc, lineendy, xc, yc);
						gr_circle(xc, yc, 5);
					}

					num_found++;
				}
			}

			//Cleanup
			if (!Cmdline_nohtl) 
			{
				gr_end_view_matrix();
				gr_end_proj_matrix();
			}

			g3_end_frame();
			
		}
	}

	//Draw ship name
	char						name[NAME_LENGTH + CALLSIGN_LEN];

	ss_return_name(Selected_wl_slot/MAX_WING_SLOTS, Selected_wl_slot%MAX_WING_SLOTS, name);
	gr_set_color_fast(&Color_normal);
	gr_string(Wl_ship_name_coords[gr_screen.res][0], Wl_ship_name_coords[gr_screen.res][1], name);
}

// ---------------------------------------------------------------------------------
// wl_get_ship_class()
//
//
int wl_get_ship_class(int wl_slot)
{
	Assert( Wss_slots != NULL );

	return Wss_slots[wl_slot].ship_class;
}

/**
 * Return true if weapon_flags indicates a weapon that is legal for use in current game type.
 * Function added by MK on 9/6/99 to support separate legal loadouts for dogfight missions.
 * name changed by Goober5000 to better reflect what it actually does
 */
int eval_weapon_flag_for_game_type(int weapon_flags)
{
	int	rval = 0;

	if (MULTI_DOGFIGHT) {
		if (weapon_flags & DOGFIGHT_WEAPON)
			rval = 1;
	}
	else
		if (weapon_flags & REGULAR_WEAPON)
			rval  = 1;

	return rval;
}

/**
 * Go through the possible weapons to choose from, and flag some as disabled since
 * that ship class cannot use that kind of weapon.  The weapon filter is specified
 * in ships.tbl, where each ship has a list of all the possible weapons it can use.
 */
void wl_set_disabled_weapons(int ship_class)
{
	int				i;
	ship_info		*sip;

	if ( ship_class == - 1 )
		return;

	Assert(ship_class >= 0 && ship_class < MAX_SHIP_CLASSES);
	Assert( Wl_icons != NULL );

	sip = &Ship_info[ship_class];

	for ( i = 0; i < MAX_WEAPON_TYPES; i++ )
	{
		//	Determine whether weapon #i is allowed on this ship class in the current type of mission.
		//	As of 9/6/99, the only difference is dogfight missions have a different list of legal weapons.
		Wl_icons[i].can_use = eval_weapon_flag_for_game_type(sip->allowed_weapons[i]);

		// Goober5000: ballistic primaries
		if (Weapon_info[i].wi_flags2 & WIF2_BALLISTIC)
		{
			// not allowed if this ship is not ballistic
			if (!(sip->flags & SIF_BALLISTIC_PRIMARIES))
			{
				Wl_icons[i].can_use = 0;
			}
		}
	}
}

/**
 * A slot index was clicked on, maybe change Selected_wl_slot
 */
void maybe_select_wl_slot(int block, int slot)
{
	int sidx;

	if ( Wss_num_wings <= 0 )
		return;

	Assert( Wss_slots != NULL );

	sidx = block*MAX_WING_SLOTS + slot;
	if ( Wss_slots[sidx].ship_class < 0 ) {
		return;
	}

	wl_set_selected_slot(sidx);
}

/**
 * Change to the weapon that corresponds to index in the weapon list
 * @param index	weapon icon index
 */
void maybe_select_new_weapon(int index)
{
	int weapon_class;

	// if a weapon is being carried, do nothing
	if ( wl_icon_being_carried() ) {
		return;
	}

	int region_index = ICON_PRIMARY_0+index;
	if ( index > 3 ) {
		region_index = ICON_SECONDARY_0 + (index - 4);
	}

	if ( Wl_mouse_down_on_region != region_index ) {
		return;
	}

	if ( index < 4 ) {
		weapon_class = Plist[Plist_start+index];
	} else {
		weapon_class = Slist[Slist_start+index-4];
	}

	if ( weapon_class >= 0 ) {
		Selected_wl_class = weapon_class;
		wl_pick_icon_from_list(index);
	}
}

/**
 * Change to the weapon that corresponds to the ship weapon slot
 * @param index index of bank (0..2 primary, 0..6 secondary)
 */
void maybe_select_new_ship_weapon(int index)
{
	int *wep, *wep_count;

	if ( Selected_wl_slot == -1 )
		return;

	if ( wl_icon_being_carried() ) {
		return;
	}

	Assert( Wss_slots != NULL );

	wep = Wss_slots[Selected_wl_slot].wep;
	wep_count = Wss_slots[Selected_wl_slot].wep_count;

	if ( wep[index] < 0 || wep_count[index] <= 0 ) {
		return;
	}

	if ( Wl_mouse_down_on_region != (ICON_SHIP_PRIMARY_0+index) ) {
		return;
	}

	Selected_wl_class = wep[index];
}

/**
 * Initialize Wl_pool[] to mission default
 */
void wl_init_pool(team_data *td)
{
	int i;

	Assert( Wl_pool != NULL );

	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		Wl_pool[i] = 0;
	}

	for ( i = 0; i < td->num_weapon_choices; i++ ) {
		Wl_pool[td->weaponry_pool[i]] += td->weaponry_count[i];	// read from mission
	}
}

/**
 * Free source anim data if allocated
 */
void wl_unload_all_anims()
{
	if(Cur_Anim.num_frames > 0)
		generic_anim_unload(&Cur_Anim);
}

/**
 * Load the icons for a specific ship class
 */
void wl_load_icons(int weapon_class)
{
	wl_icon_info	*icon;
	int				first_frame = -1;
	int num_frames = 0, i;
	weapon_info *wip = &Weapon_info[weapon_class];

	Assert( Wl_icons != NULL );

	icon = &Wl_icons[weapon_class];

	if(!Cmdline_weapon_choice_3d || (wip->render_type == WRT_LASER && !strlen(wip->tech_model)))
	{
		first_frame = bm_load_animation(Weapon_info[weapon_class].icon_filename, &num_frames);

		for ( i = 0; (i < num_frames) && (i < NUM_ICON_FRAMES); i++ ) {
			icon->icon_bmaps[i] = first_frame+i;
		}
	}

	if ( first_frame == -1 && icon->model_index == -1)
	{
		if(strlen(wip->tech_model))
		{
			icon->model_index = model_load(wip->tech_model, 0, NULL, 0);
		}
		if(wip->render_type != WRT_LASER && icon->model_index == -1)
		{
			icon->model_index = model_load(wip->pofbitmap_name, 0, NULL);
		}
	}
}

/**
 * Load all the icons for weapons in the pool
 */
void wl_load_all_icons()
{

	int i, j;

	Assert( (Wl_icons != NULL) && (Wl_pool != NULL) );

	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		// clear out data
		generic_anim_init(&Wl_icons[i].animation, NULL);
		for ( j = 0; j < NUM_ICON_FRAMES; j++ ) {
			Wl_icons[i].icon_bmaps[j] = -1;
		}
		Wl_icons[i].model_index = -1;
		Wl_icons[i].laser_bmap = -1;

		if ( Wl_pool[i] > 0 ) {
			wl_load_icons(i);
		}
	}
}

/**
 * Frees the bitmaps used for weapon icons 
 */
void wl_unload_icons()
{
	int					i,j;
	wl_icon_info		*icon;

	Assert( Wl_icons != NULL );

	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		icon = &Wl_icons[i];

		for ( j = 0; j < NUM_ICON_FRAMES; j++ ) {
			if ( icon->icon_bmaps[j] >= 0 ) {
				bm_release(icon->icon_bmaps[j]);
				icon->icon_bmaps[j] = -1;
			}
		}

		if(icon->model_index >= 0) {
			model_unload(icon->model_index);
			icon->model_index = -1;
		}
		if(icon->laser_bmap >= 0) {
			bm_unload(icon->laser_bmap);
			icon->laser_bmap = -1;
		}
		if(Cur_Anim.num_frames > 0)
			generic_anim_unload(&Cur_Anim);
	}
}

/**
 * init ship-class specific data
 */
void wl_init_ship_class_data()
{
	int i;
	wl_ship_class_info	*wl_ship;

	for ( i=0; i<MAX_SHIP_CLASSES; i++ ) {
		wl_ship = &Wl_ships[i];
		wl_ship->overhead_bitmap = -1;
		wl_ship->model_num = -1;
		generic_anim_init(&wl_ship->animation, NULL);
	}
}

/**
 * Free any allocated ship-class specific data
 */
void wl_free_ship_class_data()
{
	int i;
	wl_ship_class_info	*wl_ship;

	for ( i=0; i<Num_ship_classes; i++ ) {
		wl_ship = &Wl_ships[i];

		if ( wl_ship->overhead_bitmap != -1 ) {
			bm_release(wl_ship->overhead_bitmap);
			wl_ship->overhead_bitmap = -1;
		}

		if ( wl_ship->model_num != -1 ) {
			// this should unload the model from memory only if it's not used in the mission
			model_unload(wl_ship->model_num);
			wl_ship->model_num = -1;
		}

		if(wl_ship->animation.num_frames)
			generic_anim_unload(&wl_ship->animation);
	}
}

/**
 * Set selected slot to first placed ship
 */
void wl_reset_selected_slot()
{
	int i;
	Selected_wl_slot = -1;

	Assert( Wss_slots != NULL );

	// in multiplayer, select the slot of the player's ship by default
	if((Game_mode & GM_MULTIPLAYER) && !MULTI_PERM_OBSERVER(Net_players[MY_NET_PLAYER_NUM]) && (Wss_slots[Net_player->p_info.ship_index].ship_class >= 0)){
		wl_set_selected_slot(Net_player->p_info.ship_index);
		return;
	}

	for ( i=0; i<MAX_WSS_SLOTS; i++ ) {
		if ( !ss_disabled_slot(i, false) ) {
			if ( ss_wing_slot_is_console_player(i) && (Wss_slots[i].ship_class >= 0) ) {
				wl_set_selected_slot(i);
				return;
			}
		}
	}

	// Didn't locate player ship, so just select the first ship we find
	for ( i=0; i<MAX_WSS_SLOTS; i++ ) {
		if ( Wss_slots[i].ship_class >=  0 ) {
			wl_set_selected_slot(i);
			break;
		}
	}
}

/**
 * Called whenever it is possible that the current selected slot has had it's ship disappear
 */
void wl_maybe_reset_selected_slot()
{
	int reset=0;

	Assert( Wss_slots != NULL );

	if ( Selected_wl_slot == -1 ) {
		reset = 1;
	}

	if ( Wss_slots[Selected_wl_slot].ship_class < 0 ) {
		reset = 1;
	}

	if ( reset ) {
		wl_reset_selected_slot();
	}
}

/**
 * If Selected_wl_class is -1, choose the first weapon available from the pool for an animation
 *  - on second thought, choose the first weapon that is oin the ship, then go to the pools
 */
void wl_maybe_reset_selected_weapon_class()
{
	int i;

	if ( Selected_wl_class >= 0 ) 
		return;

	Assert( Wss_slots != NULL );

	// try to locate a weapon class to show animation for
	// first check for a weapon on the ship
	for (i=0; i<MAX_SHIP_WEAPONS; i++) {
		// if player has a weapon in bank i, set it as the selected type
		if (Wss_slots[0].wep_count[i] >= 0) {
			Selected_wl_class = Wss_slots[0].wep[i];
			return;
		}
	}

	// then check for a primary weapon in the pool
	for ( i = 0; i < Plist_size; i++ ) {
		if ( Plist[i] >= 0 ) {
			Selected_wl_class = Plist[i];
			return;
		}
	}

	// finally, if no others found yet, check for a secondary weapon in the pool
	for ( i = 0; i < Slist_size; i++ ) {
		if ( Slist[i] >= 0 ) {
			Selected_wl_class = Slist[i];
			return;
		}
	}
}

/**
 * Call when Selected_wl_slot needs to be changed
 */
void wl_set_selected_slot(int slot_num)
{
	Selected_wl_slot = slot_num;
	if ( Selected_wl_slot >= 0 ) {
		Assert( Wss_slots != NULL );
		wl_set_disabled_weapons(Wss_slots[slot_num].ship_class);
	}
}

/**
 * Determine how many ballistics of type 'wi_index' will fit into capacity - Goober5000
 */
int wl_calc_ballistic_fit(int wi_index, int capacity)
{
	if ( wi_index < 0 ) {
		return 0;
	}

	Assert(Weapon_info[wi_index].subtype == WP_LASER);

	Assert(Weapon_info[wi_index].wi_flags2 & WIF2_BALLISTIC);

	return fl2i( capacity / Weapon_info[wi_index].cargo_size + 0.5f );
}

/**
 * Determine how many missiles of type 'wi_index' will fit into capacity
 */
int wl_calc_missile_fit(int wi_index, int capacity)
{
	if ( wi_index < 0 ) {
		return 0;
	}

	Assert(Weapon_info[wi_index].subtype == WP_MISSILE);
	return fl2i( capacity / Weapon_info[wi_index].cargo_size + 0.5f );
}

/**
 * Fill out the weapons for this ship_class
 */
void wl_get_ship_class_weapons(int ship_class, int *wep, int *wep_count)
{
	ship_info	*sip;
	int i;

	Assert(ship_class >= 0 && ship_class < Num_ship_classes);
	sip = &Ship_info[ship_class];

	// reset weapons arrays
	for ( i=0; i < MAX_SHIP_WEAPONS; i++ ) {
		wep[i] = -1;
		wep_count[i] = -1;	// -1 means weapon bank doesn't exist.. 0 just means it is empty
	}

	for ( i = 0; i < sip->num_primary_banks; i++ )
	{
		wep[i] = sip->primary_bank_weapons[i];
		wep_count[i] = 1;
	}

	for ( i = 0; i < sip->num_secondary_banks; i++ )
	{
		wep[i+MAX_SHIP_PRIMARY_BANKS] = sip->secondary_bank_weapons[i];
		wep_count[i+MAX_SHIP_PRIMARY_BANKS] = wl_calc_missile_fit(sip->secondary_bank_weapons[i], sip->secondary_bank_ammo_capacity[i]);
	}
}

/**
 * Fill out the wep[] and wep_count[] arrays for a ship
 */
void wl_get_ship_weapons(int ship_index, int *wep, int *wep_count)
{
	int			i;
	ship_weapon	*swp;

	Assert(ship_index >= 0);

	Assert(Ships[ship_index].wingnum >= 0);
	swp = &Ships[ship_index].weapons;

	for ( i = 0; i < swp->num_primary_banks; i++ )
	{
		wep[i] = swp->primary_bank_weapons[i];
		wep_count[i] = 1;

		if ( wep[i] == -1 ) {
			wep_count[i] = 0;
		}
	}

	for ( i = 0; i < swp->num_secondary_banks; i++ )
	{
		wep[i+MAX_SHIP_PRIMARY_BANKS] = swp->secondary_bank_weapons[i];
		wep_count[i+MAX_SHIP_PRIMARY_BANKS] = swp->secondary_bank_ammo[i];

		if ( wep[i+MAX_SHIP_PRIMARY_BANKS] == -1 ) {
			wep_count[i+MAX_SHIP_PRIMARY_BANKS] = 0;
		}
	}
}

/**
 * Set wep and wep_count from a ship which sits in the ship arrivals list at index sa_index
 */
void wl_get_parseobj_weapons(int sa_index, int ship_class, int *wep, int *wep_count)
{
	int				i,	pilot_index;
	subsys_status	*ss;
	ship_info		*sip;
	p_object			*pobjp;

	pobjp = &Parse_objects[sa_index];
	sip = &Ship_info[ship_class];

	pilot_index = wl_get_pilot_subsys_index(pobjp);

	if ( pilot_index == -1 )
		return;

	ss = &Subsys_status[pilot_index];

	if ( ss->primary_banks[0] != SUBSYS_STATUS_NO_CHANGE ) {
		for ( i=0; i < MAX_SHIP_PRIMARY_BANKS; i++ ) {
			wep[i] = ss->primary_banks[i];		
		}
	}

	if ( ss->secondary_banks[0] != SUBSYS_STATUS_NO_CHANGE ) {
		for ( i=0; i < MAX_SHIP_SECONDARY_BANKS; i++ ) {
			wep[i+MAX_SHIP_PRIMARY_BANKS] = ss->secondary_banks[i];	
		}
	}

	// ammo counts could still be modified
	for ( i=0; i < MAX_SHIP_SECONDARY_BANKS; i++ )
	{
		if ( wep[i+MAX_SHIP_PRIMARY_BANKS] >= 0 )
		{
			wep_count[i+MAX_SHIP_PRIMARY_BANKS] = wl_calc_missile_fit(wep[i+MAX_SHIP_PRIMARY_BANKS], fl2i(ss->secondary_ammo[i]/100.0f * sip->secondary_bank_ammo_capacity[i] + 0.5f));
		}
	}
}

/**
 * Ensure that there aren't any bogus weapons assigned by default
 */
void wl_cull_illegal_weapons(int ship_class, int *wep, int *wep_count)
{
	int i, check_flag;
	for ( i=0; i < MAX_SHIP_WEAPONS; i++ )
	{
		if ( wep[i] < 0 ) {
			continue;
		}

		check_flag = Ship_info[ship_class].allowed_weapons[wep[i]];

		// possibly change flag if it's restricted
		if (eval_weapon_flag_for_game_type(Ship_info[ship_class].restricted_loadout_flag[i]))
		{
			check_flag = Ship_info[ship_class].allowed_bank_restricted_weapons[i][wep[i]];
		}


		if ( !eval_weapon_flag_for_game_type(check_flag) ) {
//			wep[i] = -1;
			wep_count[i] = 0;
		}
	}
}

/**
 * Get the weapons info that should be on ship by default
 */
void wl_get_default_weapons(int ship_class, int slot_num, int *wep, int *wep_count)
{
	int original_ship_class, i;

	Assert(slot_num >= 0 && slot_num < MAX_WSS_SLOTS);

	// clear out wep and wep_count
	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ ) {
		wep[i] = -1;
		wep_count[i] = -1;
	}

	if ( ship_class < 0 )
		return;

	original_ship_class = ss_return_original_ship_class(slot_num);

	if ( original_ship_class != ship_class ) {
		wl_get_ship_class_weapons(ship_class, wep, wep_count);
	} else {
		int sa_index;	// ship arrival index
		sa_index = ss_return_saindex(slot_num);

		if ( sa_index >= 0 ) {
			// still a parse object
			wl_get_ship_class_weapons(ship_class, wep, wep_count);
			wl_get_parseobj_weapons(sa_index, ship_class, wep, wep_count);
		} else {
			// ship has been created
			int ship_index = -1;
			p_object *pobjp;
			ss_return_ship(slot_num/MAX_WING_SLOTS, slot_num%MAX_WING_SLOTS, &ship_index, &pobjp);
			Assert(ship_index != -1);
			wl_get_ship_weapons(ship_index, wep, wep_count);
		}
	}

	// ensure that there aren't any bogus weapons assigned by default
	wl_cull_illegal_weapons(ship_class, wep, wep_count);	
}

/**
 * Add a weapon_class to ui lists
 */
void wl_add_index_to_list(int wi_index)
{
	int i;
	if ( Weapon_info[wi_index].subtype == WP_MISSILE ) {

		for ( i=0; i<Slist_size; i++ ) {
			if ( Slist[i] == wi_index )
				break;
		}

		if ( i == Slist_size ) 
			Slist[Slist_size++] = wi_index;

	} else {
		for ( i=0; i<Plist_size; i++ ) {
			if ( Plist[i] == wi_index )
				break;
		}

		if ( i == Plist_size ) 
			Plist[Plist_size++] = wi_index;
	}
}

/**
 * Remove the weapons specified by wep[] and wep_count[] from Wl_pool[].
 */
void wl_remove_weps_from_pool(int *wep, int *wep_count, int ship_class)
{
	int i, wi_index;

	Assert( Wl_pool != NULL );

	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ ) {
		wi_index = wep[i];
		if ( wi_index >= 0 ) {
			if ( (wep_count[i] > 0) && ((Wl_pool[wi_index] - wep_count[i]) >= 0) ) {
				Wl_pool[wi_index] -= wep_count[i];
			} else {
				// not enough weapons in pool
				// TEMP HACK: FRED doesn't fill in a weapons pool if there are no starting wings... so
				//            add to the pool.  This should be fixed.
				if ( Wss_num_wings <= 0 ) {
					wl_add_index_to_list(wi_index);
				} else {
	
					if ( (Wl_pool[wi_index] <= 0) || (wep_count[i] == 0) ) {
						// fresh out of this weapon, pick an alternate pool weapon if we can
						int wep_pool_index, wep_precedence_index, new_wi_index = -1;
						for ( wep_pool_index = 0; wep_pool_index < MAX_WEAPON_TYPES; wep_pool_index++ ) {

							if ( Wl_pool[wep_pool_index] <= 0 ) {
								continue;
							}

							// AL 3-31-98: Only pick another primary if primary, etc
							if ( Weapon_info[wi_index].subtype != Weapon_info[wep_pool_index].subtype ) {
								continue;
							}

							if ( !eval_weapon_flag_for_game_type(Ship_info[ship_class].allowed_weapons[wep_pool_index]) ) {
								continue;
							}

							for ( wep_precedence_index = 0; wep_precedence_index < Num_player_weapon_precedence; wep_precedence_index++ ) {
								if ( wep_pool_index == Player_weapon_precedence[wep_precedence_index] ) {
									new_wi_index = wep_pool_index;
									break;
								}
							}

							if ( new_wi_index >= 0 ) {
								break;
							}
						}

						if ( new_wi_index >= 0 ) {
							wep[i] = new_wi_index;
							wi_index = new_wi_index;
						}
					}

					int new_wep_count = wep_count[i];
					if ( Weapon_info[wi_index].subtype == WP_MISSILE )
					{
						int secondary_bank_index;
						secondary_bank_index = i-3;
						if ( secondary_bank_index < 0 ) {
							Int3();
							secondary_bank_index = 0;
						}
						new_wep_count = wl_calc_missile_fit(wi_index, Ship_info[ship_class].secondary_bank_ammo_capacity[secondary_bank_index]);
					}

					wep_count[i] = MIN(new_wep_count, Wl_pool[wi_index]);
					Assert(wep_count[i] >= 0);
					Wl_pool[wi_index] -= wep_count[i];
					if ( wep_count[i] <= 0 ) {
						wep[i] = -1;
					}
				}
			}
		}
	}
}

/**
 * Init the weapons portion of Wss_slots[] and the ui data in Wl_slots[]
 * @note It is assumed that Wl_pool[] has been initialized, and Wss_slots[].ship_class is correctly set
 */
void wl_fill_slots()
{
	int i, j;	
	int wep[MAX_SHIP_WEAPONS];
	int wep_count[MAX_SHIP_WEAPONS];

	Assert( Wss_slots != NULL );

	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		if ( Wss_slots[i].ship_class < 0 ){
			continue;
		}

		// get the weapons info that should be on ship by default
		wl_get_default_weapons(Wss_slots[i].ship_class, i, wep, wep_count);
		wl_remove_weps_from_pool(wep, wep_count, Wss_slots[i].ship_class);

		// copy to Wss_slots[]
		for ( j = 0; j < MAX_SHIP_WEAPONS; j++ ) {
			Wss_slots[i].wep[j] = wep[j];
			Wss_slots[i].wep_count[j] = wep_count[j];
		}
	}	
}

/**
 * Set up the primary and secondary icons lists that hold the weapons the player can choose from
 */
void wl_init_icon_lists()
{
	int i;

	Assert( Wl_pool != NULL );

	Plist_start = 0;		// offset into Plist[]
	Slist_start = 0;

	Plist_size = 0;		// number of active elements in Plist[]
	Slist_size = 0;

	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		Plist[i] = -1;
		Slist[i] = -1;
	}

	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		if ( Wl_pool[i] > 0 ) {
			if ( Weapon_info[i].subtype == WP_MISSILE ) {
				Slist[Slist_size++] = i;
			} else {
				Plist[Plist_size++] = i;
			}
		}
	}
}

/**
 * Set the necessary pointers
 */
void wl_set_team_pointers(int team)
{
	Assert( (team >= 0) && (team < MAX_TVT_TEAMS) );
	
	Wl_icons = Wl_icons_teams[team];
}

/**
 * Reset the necessary pointers to defaults
 */
void wl_reset_team_pointers()
{
	Assert( !Weapon_select_open );
	
	if ( Weapon_select_open )
		return;
	
	Wl_icons = NULL;
}

/**
 * Initialize team specific weapon select data structures
 */
void weapon_select_init_team(int team_num)
{
	common_set_team_pointers(team_num);

	wl_init_pool(&Team_data[team_num]);
	wl_init_icon_lists();
	wl_init_ship_class_data();

	wl_load_all_icons();

	wl_fill_slots();
}

/**
 * Close out what weapon_select_init_team() set up but only when we are not acutally
 * in the weapon select screen - taylor
 */
void weapon_select_close_team()
{
	if (Weapon_select_open)
		return;

	wl_unload_icons();
	wl_unload_all_anims();
}

/**
 * This init is called even before the weapons loadout screen is entered.  It is called when the
 * briefing state is entered.
 */
void weapon_select_common_init()
{
	int idx;

	if(MULTI_TEAM){
		// initialize for all teams
		for(idx=0;idx<MULTI_TS_MAX_TVT_TEAMS;idx++){
			weapon_select_init_team(idx);
		}

		// re-initialize for me specifically
		weapon_select_init_team(Common_team);
	} else {	
		// initialize for my own team
		weapon_select_init_team(Common_team);
	}

	wl_reset_selected_slot();
	wl_reset_carried_icon();
	wl_maybe_reset_selected_weapon_class();
}

/**
 * Called to load the bitmaps and set up the mask regions for
 * the weapon loadout screen.  common_select_init() is called to load the animations
 * and bitmaps which are in common with the ship select and briefing screens.
 *
 * @note The Weapon_select_open flag is set to 1 when weapon_select_init() completes successfully
 */
void weapon_select_init()
{
	common_set_interface_palette("WeaponPalette");
	common_flash_button_init();

	// for multiplayer, change the state in my netplayer structure
	if ( Game_mode & GM_MULTIPLAYER )
		Net_player->state = NETPLAYER_STATE_WEAPON_SELECT;

	Weapon_anim_class = -1;

	set_active_ui(&Weapon_ui_window);
	Current_screen = ON_WEAPON_SELECT;
	Last_wl_ship_class = -1;

	wl_maybe_reset_selected_slot();
	Assert( Wss_slots != NULL );
	wl_set_disabled_weapons(Wss_slots[Selected_wl_slot].ship_class);

	help_overlay_set_state(WL_OVERLAY,0);

	if ( Weapon_select_open ) {
		wl_maybe_reset_selected_weapon_class();
		common_buttons_maybe_reload(&Weapon_ui_window);	// AL 11-21-97: this is necessary since we may returning from the hotkey
																		// screen, which can release common button bitmaps.
		common_reset_buttons();
		nprintf(("Alan","weapon_select_init() returning without doing anything\n"));

		// if we're in multiplayer always select the player's ship
		wl_reset_selected_slot();

		return;
	}

	nprintf(("Alan","entering weapon_select_init()\n"));
	common_select_init();


	// Goober5000
	// first determine which layout to use
	Uses_apply_all_button = 1;	// assume true
	Weapon_select_background_bitmap = bm_load((Game_mode & GM_MULTIPLAYER) ? Weapon_select_multi_background_fname[Uses_apply_all_button][gr_screen.res] : Weapon_select_background_fname[Uses_apply_all_button][gr_screen.res]);
	if (Weapon_select_background_bitmap < 0)	// failed to load
	{
		Uses_apply_all_button = 0;	// nope, sorry
		Weapon_select_background_bitmap = bm_load((Game_mode & GM_MULTIPLAYER) ? Weapon_select_multi_background_fname[Uses_apply_all_button][gr_screen.res] : Weapon_select_background_fname[Uses_apply_all_button][gr_screen.res]);
	}


	WeaponSelectMaskBitmap = bm_load(Wl_loadout_select_mask[Uses_apply_all_button][gr_screen.res]);
	if (WeaponSelectMaskBitmap < 0) {
		if (gr_screen.res == GR_640) {
			Error(LOCATION,"Could not load in 'weaponloadout-m'!");
		} else if (gr_screen.res == GR_1024) {
			Error(LOCATION,"Could not load in '2_weaponloadout-m'!");
		} else {
			Error(LOCATION,"Could not load in 'weaponloadout-m'!");
		}
	}

	Weaponselect_mask_w = -1;
	Weaponselect_mask_h = -1;

	// get a pointer to bitmap by using bm_lock()
	WeaponSelectMaskPtr = bm_lock(WeaponSelectMaskBitmap, 8, BMP_AABITMAP);
	WeaponSelectMaskData = (ubyte*)WeaponSelectMaskPtr->data;
	Assert(WeaponSelectMaskData != NULL);
	bm_get_info(WeaponSelectMaskBitmap, &Weaponselect_mask_w, &Weaponselect_mask_h);


	// Set up the mask regions
   // initialize the different regions of the menu that will react when the mouse moves over it
	Num_weapon_select_regions = 0;
	
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	COMMON_BRIEFING_REGION,				0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	COMMON_SS_REGION,						0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	COMMON_WEAPON_REGION,				0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	COMMON_COMMIT_REGION,				0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	COMMON_HELP_REGION,					0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	COMMON_OPTIONS_REGION,				0);

	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_0_SHIP_0,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_0_SHIP_1,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_0_SHIP_2,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_0_SHIP_3,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_1_SHIP_0,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_1_SHIP_1,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_1_SHIP_2,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_1_SHIP_3,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_2_SHIP_0,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_2_SHIP_1,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_2_SHIP_2,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	WING_2_SHIP_3,		0);

	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_PRIMARY_0,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_PRIMARY_1,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_PRIMARY_2,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_PRIMARY_3,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SECONDARY_0,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SECONDARY_1,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SECONDARY_2,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SECONDARY_3,		0);

	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SHIP_PRIMARY_0,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SHIP_PRIMARY_1,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SHIP_PRIMARY_2,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SHIP_SECONDARY_0,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SHIP_SECONDARY_1,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SHIP_SECONDARY_2,		0);
	snazzy_menu_add_region(&Weapon_select_region[Num_weapon_select_regions++], "",	ICON_SHIP_SECONDARY_3,		0);

	// init common UI
	Weapon_ui_window.create( 0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0 );

	if(Game_mode & GM_MULTIPLAYER){
		Weapon_ui_window.set_mask_bmap(Wl_mask_multi[Uses_apply_all_button][gr_screen.res]);
	} else {
		Weapon_ui_window.set_mask_bmap(Wl_mask_single[Uses_apply_all_button][gr_screen.res]);
	}

	Weapon_ui_window.tooltip_handler = wl_tooltip_handler;
	common_buttons_init(&Weapon_ui_window);
	weapon_buttons_init();
	Weapon_select_open = 1;

	// if we're in multiplayer always select the player's ship
	wl_reset_selected_slot();

	//Load the slot bitmap
	Weapon_slot_bitmap = bm_load("weapon_slot");
}

// ----------------------------------------------------------------
// wl_dump_carried_icon()
void wl_dump_carried_icon()
{
	if ( wl_icon_being_carried() ) {
		// Add back into the weapon pool
		if ( Carried_wl_icon.from_bank >= 0 ) {
			// return to list
			wl_drop(Carried_wl_icon.from_bank, -1, -1, Carried_wl_icon.weapon_class, Carried_wl_icon.from_slot);
		} else {
			if ( wl_carried_icon_moved() ) {
				gamesnd_play_iface(SND_ICON_DROP);
			}			
		}

		wl_reset_carried_icon();
	}
}

/**
 * Drop the Carried_wl_icon onto the specified slot.  The slot numbering is:
 *
 * 0->2: primary weapons
 * 3-6: secondary weapons
 *
 * @note These are the slots that exist beside the overhead view of the ship.
 * on the weapons loadout screen.
 */
int drop_icon_on_slot(int bank_num)
{
	if ( Selected_wl_slot == -1 ) {
		return 0;
	}

	if(Game_mode & GM_MULTIPLAYER){
		if(multi_ts_disabled_slot(Selected_wl_slot)){
			return 0;
		}
	} else {
		if ( ss_disabled_slot( Selected_wl_slot, false ) ){
			return 0;
		}
	}

	Assert( Wss_slots != NULL );

	// check if slot exists
	if ( Wss_slots[Selected_wl_slot].wep_count[bank_num] < 0 ) {
		return 0;
	}

	if ( !wl_carried_icon_moved() ) {
		wl_reset_carried_icon();
		return 0;
	}

	wl_drop(Carried_wl_icon.from_bank, Carried_wl_icon.weapon_class, bank_num, -1, Selected_wl_slot);
	return 1;
}

// ----------------------------------------------------------------
// maybe_drop_icon_on_slot()
//
void maybe_drop_icon_on_slot(int bank_num)
{
	int dropped=0;
	if ( !mouse_down(MOUSE_LEFT_BUTTON) ) {
		if ( wl_icon_being_carried() )
			dropped = drop_icon_on_slot(bank_num);

		if ( dropped ) {
			wl_reset_carried_icon();
		}
	}
}		

// ---------------------------------------------------------------------------------
// do_mouse_over_list_weapon()
//
void do_mouse_over_list_weapon(int index)
{
	Hot_weapon_icon = index;

	int region_index = ICON_PRIMARY_0+index;
	if ( index > 3 ) {
		region_index = ICON_SECONDARY_0 + (index - 4);
	}
	
	if ( Wl_mouse_down_on_region != region_index ){
		return;
	}

	if ( mouse_down(MOUSE_LEFT_BUTTON) )
		wl_pick_icon_from_list(index);
}

/**
 * Mouse over ship weapon
 *
 * @param index Bank index on ship (0..6)
 * @return 0 icon was not dropped on a slot
 * @return 1 icon was dropped on a slot
 */
int do_mouse_over_ship_weapon(int index)
{
	int dropped_on_slot, is_moved, mx, my;

	dropped_on_slot = 0;
	Assert(Selected_wl_slot >= 0);
	Assert(Wss_slots != NULL);

	if ( ss_disabled_slot( Selected_wl_slot , false) )
		return 0;

	Hot_weapon_bank_icon = index;	// bank icon will be drawn highlighted

	if ( mouse_down(MOUSE_LEFT_BUTTON) ) {
		if ( Wl_mouse_down_on_region == (ICON_SHIP_PRIMARY_0+index) ){
			pick_from_ship_slot(index);
		}
	} else {
		int was_carried = wl_icon_being_carried();
		maybe_drop_icon_on_slot(index);
		if ( was_carried && !wl_icon_being_carried() ) {
			mouse_get_pos_unscaled( &mx, &my );
			if ( Carried_wl_icon.from_x != mx || Carried_wl_icon.from_y != my) {
				dropped_on_slot = 1;
			}
		}
	}

	// set Hot_weapon_bank if a droppable icon is being held over a slot that
	// can accept that icon
	is_moved = 0;
	mouse_get_pos_unscaled( &mx, &my );
	if ( Carried_wl_icon.from_x != mx || Carried_wl_icon.from_y != my) {
		is_moved = 1;
	}

	if ( wl_icon_being_carried() && is_moved ) {
		if ( Weapon_info[Carried_wl_icon.weapon_class].subtype != WP_MISSILE ) {
			if ( (index < 3) && (Wss_slots[Selected_wl_slot].wep_count[index] >= 0) ) 
				Hot_weapon_bank = index;
		} else {
			if ( index >= 3 && ( Wss_slots[Selected_wl_slot].wep_count[index] >= 0) ) 
				Hot_weapon_bank = index;
		}
	}

	return dropped_on_slot;
}


/**
 * Maybe flash a button if player hasn't done anything for a while
 */
void wl_maybe_flash_button()
{
	if ( common_flash_bright() ) {
		// commit button
		if ( Common_buttons[Current_screen-1][gr_screen.res][3].button.button_hilighted() ) {
			common_flash_button_init();
		} else {
			Common_buttons[Current_screen-1][gr_screen.res][3].button.draw_forced(1);
		}
	}
}


void weapon_select_render(float frametime)
{
	if ( !Background_playing ) {
		gr_set_bitmap(Weapon_select_background_bitmap);
		gr_bitmap(0, 0);
	}
}

/**
 * Draw the weapon description text
 * @note this wipes in
 */
void wl_render_weapon_desc(float frametime)
{
	int *weapon_desc_coords;
	int *weapon_title_coords;

	// retrieve the correct set of text coordinates
	if (Game_mode & GM_MULTIPLAYER) {
		weapon_desc_coords = Wl_new_weapon_desc_coords_multi[gr_screen.res];
		weapon_title_coords = Wl_new_weapon_title_coords_multi[gr_screen.res];
	} else {
		weapon_desc_coords = Wl_new_weapon_desc_coords[gr_screen.res];
		weapon_title_coords = Wl_new_weapon_title_coords[gr_screen.res];
	}

	// render the normal version of the weapom desc
	char bright_char[WEAPON_DESC_MAX_LINES];			// one bright char per line
	if (!Weapon_desc_wipe_done) {
		// draw mid-wipe version
		// decide which char is last (and bright)
		int bright_char_index = (int)(Weapon_desc_wipe_time_elapsed * WEAPON_DESC_MAX_LENGTH / WEAPON_DESC_WIPE_TIME);
		int i, w, h, curr_len;
		
		// draw weapon title (above weapon anim)
		for (i=0; i<2; i++) {
			curr_len = strlen(Weapon_desc_lines[i]);

			if (bright_char_index < curr_len) {
				// save bright char and plunk in some nulls to shorten string
				bright_char[i] = Weapon_desc_lines[i][bright_char_index];
				Weapon_desc_lines[i][bright_char_index] = '\0';

				// draw the strings
				gr_set_color_fast(&Color_white);			
				gr_string(weapon_title_coords[0], weapon_title_coords[1]+(10*i), Weapon_desc_lines[i]);

				// draw the bright letters
				gr_set_color_fast(&Color_bright_white);
				gr_get_string_size(&w, &h, Weapon_desc_lines[i], curr_len);
				gr_printf(weapon_title_coords[0]+w, weapon_title_coords[1]+(10*i), "%c", bright_char[i]);

				// restore the bright char to the string
				Weapon_desc_lines[i][bright_char_index] = bright_char[i];

			} else {
				// draw the string
				gr_set_color_fast(&Color_white);
				gr_string(weapon_title_coords[0], weapon_title_coords[1]+(10*i), Weapon_desc_lines[i]);
			}
		}

		// draw weapon desc (below weapon anim)
		for (i=2; i<WEAPON_DESC_MAX_LINES; i++) {
			curr_len = strlen(Weapon_desc_lines[i]);

			if (bright_char_index < curr_len) {
				// save bright char and plunk in some nulls to shorten string
				bright_char[i] = Weapon_desc_lines[i][bright_char_index];
				Weapon_desc_lines[i][bright_char_index] = '\0';

				// draw the string
				gr_set_color_fast(&Color_white);
				gr_string(weapon_desc_coords[0], weapon_desc_coords[1]+(10*(i-2)), Weapon_desc_lines[i]);

				// draw the bright letters
				gr_set_color_fast(&Color_bright_white);
				gr_get_string_size(&w, &h, Weapon_desc_lines[i], curr_len);
				gr_printf(weapon_desc_coords[0]+w, weapon_desc_coords[1]+(10*(i-2)), "%c", bright_char[i]);

				// restore the bright char to the string
				Weapon_desc_lines[i][bright_char_index] = bright_char[i];

			} else {
				// draw the string
				gr_set_color_fast(&Color_white);
				gr_string(weapon_desc_coords[0], weapon_desc_coords[1]+(10*(i-2)), Weapon_desc_lines[i]);
			}
		}

		// update time
		Weapon_desc_wipe_time_elapsed += frametime;
		if (Weapon_desc_wipe_time_elapsed >= WEAPON_DESC_WIPE_TIME) {
			// wipe is done,set flag and stop sound
			Weapon_desc_wipe_done = 1;
		}

	} else {

		// draw full version
		// FIXME - change to use a for loop 
		gr_set_color_fast(&Color_white);
		gr_string(weapon_title_coords[0], weapon_title_coords[1], Weapon_desc_lines[0]);
		gr_string(weapon_title_coords[0], weapon_title_coords[1] + 10, Weapon_desc_lines[1]);
		gr_string(weapon_desc_coords[0], weapon_desc_coords[1], Weapon_desc_lines[2]);
		gr_string(weapon_desc_coords[0], weapon_desc_coords[1] + 10, Weapon_desc_lines[3]);
		gr_string(weapon_desc_coords[0], weapon_desc_coords[1] + 20, Weapon_desc_lines[4]);
		gr_string(weapon_desc_coords[0], weapon_desc_coords[1] + 30, Weapon_desc_lines[5]);
	}
}



/**
 * Re-inits wiping vars and causes the current text to wipe in again
 */
void wl_weapon_desc_start_wipe()
{
	int currchar_src = 0, currline_dest = 2, currchar_dest = 0, i;
	int w, h;
	int title_len = strlen(Weapon_info[Selected_wl_class].title);

	// init wipe vars
	Weapon_desc_wipe_time_elapsed = 0.0f;
	Weapon_desc_wipe_done = 0;

	// break title into two lines if too long
	strcpy_s(Weapon_desc_lines[0], Weapon_info[Selected_wl_class].title);
	gr_get_string_size(&w, &h, Weapon_info[Selected_wl_class].title, title_len);
	if (w > Weapon_title_max_width[gr_screen.res]) {
		// split
		currchar_src = (int)(((float)title_len / (float)w) * Weapon_title_max_width[gr_screen.res]);			// char to start space search at
		while (Weapon_desc_lines[0][currchar_src] != ' ') {
			currchar_src--;
			if (currchar_src <= 0) {
				currchar_src = title_len;
				break;
			}
		}

		Weapon_desc_lines[0][currchar_src] = '\0';										// shorten line 0
		strcpy_s(Weapon_desc_lines[1], &(Weapon_desc_lines[0][currchar_src+1]));		// copy remainder into line 1
	} else {
		// entire title in line 0, thus line 1 is empty
		Weapon_desc_lines[1][0] = '\0';
	}
	
	// break current description into lines (break at the /n's)
	currchar_src = 0;
	if (Weapon_info[Selected_wl_class].desc != NULL) {
		while (Weapon_info[Selected_wl_class].desc[currchar_src] != '\0') {
			if (Weapon_info[Selected_wl_class].desc[currchar_src] == '\n') {
				// break here
				if (currchar_src != 0) {					// protect against leading /n's
					Weapon_desc_lines[currline_dest][currchar_dest] = '\0';
					currline_dest++;
					currchar_dest = 0;
				}
			} else {
				// straight copy
				Weapon_desc_lines[currline_dest][currchar_dest] = Weapon_info[Selected_wl_class].desc[currchar_src];
				currchar_dest++;
			}

			currchar_src++;

			Assert(currline_dest < WEAPON_DESC_MAX_LINES);
			Assert(currchar_dest < WEAPON_DESC_MAX_LENGTH);
		}
	}

	// wrap up the line processing
	Weapon_desc_lines[currline_dest][currchar_dest] = '\0';
	for (i=currline_dest+1; i<WEAPON_DESC_MAX_LINES; i++) {
		Weapon_desc_lines[i][0] = '\0';
	}
}



/**
 * Calls to common_ functions are made for those functions which are common to the
 * ship select and briefing screens.
 */
void weapon_select_do(float frametime)
{
	int k, wl_choice, snazzy_action;

	if ( !Weapon_select_open )
		weapon_select_init();

	wl_choice = snazzy_menu_do(WeaponSelectMaskData, Weaponselect_mask_w, Weaponselect_mask_h, Num_weapon_select_regions, Weapon_select_region, &snazzy_action, 0);

	if ( wl_choice >= 0 ) {
		if ( snazzy_action == SNAZZY_CLICKED ) {
			nprintf(("Alan","got one\n"));
		} 
	}

	Hot_wl_slot = -1;
	Hot_weapon_icon = -1;
	Hot_weapon_bank = -1;
	Hot_weapon_bank_icon = -1;

	k = common_select_do(frametime);
	
	// Check common keypresses
	common_check_keys(k);

	if ( Mouse_down_last_frame ) {
		Wl_mouse_down_on_region = wl_choice;
	}

	if ( wl_choice > -1 ) {
		switch(wl_choice) {
			case ICON_PRIMARY_0:
				do_mouse_over_list_weapon(0);
				break;
			case ICON_PRIMARY_1:
				do_mouse_over_list_weapon(1);
				break;
			case ICON_PRIMARY_2:
				do_mouse_over_list_weapon(2);
				break;
			case ICON_PRIMARY_3:
				do_mouse_over_list_weapon(3);
				break;
			case ICON_SECONDARY_0:
				do_mouse_over_list_weapon(4);
				break;
			case ICON_SECONDARY_1:
				do_mouse_over_list_weapon(5);
				break;
			case ICON_SECONDARY_2:
				do_mouse_over_list_weapon(6);
				break;
			case ICON_SECONDARY_3:
				do_mouse_over_list_weapon(7);
				break;
			case ICON_SHIP_PRIMARY_0:
				if ( do_mouse_over_ship_weapon(0) )
					wl_choice = -1;
				break;
			case ICON_SHIP_PRIMARY_1:
				if ( do_mouse_over_ship_weapon(1) )
					wl_choice = -1;
				break;
			case ICON_SHIP_PRIMARY_2:
				if ( do_mouse_over_ship_weapon(2) )
					wl_choice = -1;
				break;
			case ICON_SHIP_SECONDARY_0:
				if ( do_mouse_over_ship_weapon(3) )
					wl_choice = -1;
				break;
			case ICON_SHIP_SECONDARY_1:
				if ( do_mouse_over_ship_weapon(4) )
					wl_choice = -1;
				break;
			case ICON_SHIP_SECONDARY_2:
				if ( do_mouse_over_ship_weapon(5) )
					wl_choice = -1;
				break;
			case ICON_SHIP_SECONDARY_3:
				if ( do_mouse_over_ship_weapon(6) )
					wl_choice = -1;
				break;
			case WING_0_SHIP_0:
				Hot_wl_slot = 0;
				break;
			case WING_0_SHIP_1:
				Hot_wl_slot = 1;
				break;
			case WING_0_SHIP_2:
				Hot_wl_slot = 2;
				break;
			case WING_0_SHIP_3:
				Hot_wl_slot = 3;
				break;
			case WING_1_SHIP_0:
				Hot_wl_slot = 4;
				break;
			case WING_1_SHIP_1:
				Hot_wl_slot = 5;
				break;
			case WING_1_SHIP_2:
				Hot_wl_slot = 6;
				break;
			case WING_1_SHIP_3:
				Hot_wl_slot = 7;
				break;
			case WING_2_SHIP_0:
				Hot_wl_slot = 8;
				break;
			case WING_2_SHIP_1:
				Hot_wl_slot = 9;
				break;
			case WING_2_SHIP_2:
				Hot_wl_slot = 10;
				break;
			case WING_2_SHIP_3:
				Hot_wl_slot = 11;
				break;

			default:
				break;
		}	// end switch
	}

	if ( !mouse_down(MOUSE_LEFT_BUTTON) )  {
		wl_dump_carried_icon();
	}

	// Check for a mouse click on buttons
	common_check_buttons();
	weapon_check_buttons();

	// Check for the mouse clicks over a region
	if ( wl_choice > -1 && snazzy_action == SNAZZY_CLICKED ) {
		switch (wl_choice) {
				case ICON_PRIMARY_0:
					maybe_select_new_weapon(0);
					break;
				case ICON_PRIMARY_1:
					maybe_select_new_weapon(1);
					break;
				case ICON_PRIMARY_2:
					maybe_select_new_weapon(2);
					break;
				case ICON_PRIMARY_3:
					maybe_select_new_weapon(3);
					break;
				case ICON_SECONDARY_0:
					maybe_select_new_weapon(4);
					break;
				case ICON_SECONDARY_1:
					maybe_select_new_weapon(5);
					break;
				case ICON_SECONDARY_2:
					maybe_select_new_weapon(6);
					break;
				case ICON_SECONDARY_3:
					maybe_select_new_weapon(7);
					break;
				case ICON_SHIP_PRIMARY_0:
					maybe_select_new_ship_weapon(0);
					break;
				case ICON_SHIP_PRIMARY_1:
					maybe_select_new_ship_weapon(1);
					break;
				case ICON_SHIP_PRIMARY_2:
					maybe_select_new_ship_weapon(2);
					break;
				case ICON_SHIP_SECONDARY_0:
					maybe_select_new_ship_weapon(3);
					break;
				case ICON_SHIP_SECONDARY_1:
					maybe_select_new_ship_weapon(4);
					break;
				case ICON_SHIP_SECONDARY_2:
					maybe_select_new_ship_weapon(5);
					break;
				case ICON_SHIP_SECONDARY_3:
					maybe_select_new_ship_weapon(6);
					break;
				case WING_0_SHIP_0:
					maybe_select_wl_slot(0,0);
					break;
				case WING_0_SHIP_1:
					maybe_select_wl_slot(0,1);
					break;
				case WING_0_SHIP_2:
					maybe_select_wl_slot(0,2);
					break;
				case WING_0_SHIP_3:
					maybe_select_wl_slot(0,3);
					break;
				case WING_1_SHIP_0:
					maybe_select_wl_slot(1,0);
					break;
				case WING_1_SHIP_1:
					maybe_select_wl_slot(1,1);
					break;
				case WING_1_SHIP_2:
					maybe_select_wl_slot(1,2);
					break;
				case WING_1_SHIP_3:
					maybe_select_wl_slot(1,3);
					break;
				case WING_2_SHIP_0:
					maybe_select_wl_slot(2,0);
					break;
				case WING_2_SHIP_1:
					maybe_select_wl_slot(2,1);
					break;
				case WING_2_SHIP_2:
					maybe_select_wl_slot(2,2);
					break;
				case WING_2_SHIP_3:
					maybe_select_wl_slot(2,3);
					break;

				default:
					break;

			}	// end switch
		}

	gr_reset_clip();

	weapon_select_render(frametime);
	int *weapon_ani_coords;
	if (Game_mode & GM_MULTIPLAYER) {
		weapon_ani_coords = Wl_weapon_ani_coords_multi[gr_screen.res];
	} else {
		weapon_ani_coords = Wl_weapon_ani_coords[gr_screen.res];
	}

	if(Wl_icons[Selected_wl_class].model_index != -1) {
		static float WeapSelectScreenWeapRot = 0.0f;
		wl_icon_info *sel_icon	= &Wl_icons[Selected_wl_class];
		weapon_info *wip = &Weapon_info[Selected_wl_class];
		draw_model_rotating(sel_icon->model_index,
			weapon_ani_coords[0],
			weapon_ani_coords[1],
			gr_screen.res == 0 ? 202 : 332,
			gr_screen.res == 0 ? 185 : 260,
			&WeapSelectScreenWeapRot,
			&Weapon_info->closeup_pos,
			Weapon_info->closeup_zoom * 0.65f,
			REVOLUTION_RATE,
			MR_IS_MISSILE | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING,
			true,
			wip->selection_effect);
	}

	else if ( Weapon_anim_class != -1 && ( Selected_wl_class == Weapon_anim_class )) {
		Assert(Selected_wl_class >= 0 && Selected_wl_class < MAX_WEAPON_TYPES );
		if ( Weapon_anim_class != Selected_wl_class )
			start_weapon_animation(Selected_wl_class);
 
		generic_anim_render(&Cur_Anim, (help_overlay_active(WL_OVERLAY)) ? 0 : frametime, weapon_ani_coords[0], weapon_ani_coords[1]);
	}

	if ( !Background_playing ) {
		Weapon_ui_window.draw();
		wl_redraw_pressed_buttons();
		draw_wl_icons();
		wl_render_overhead_view(frametime);
		wl_draw_ship_weapons(Selected_wl_slot);
		for ( int i = 0; i < MAX_WING_BLOCKS; i++ ) {
			draw_wing_block(i, Hot_wl_slot, Selected_wl_slot, -1, false);
		}
		common_render_selected_screen_button();
	}

	// maybe blit the multiplayer "locked" button
	if((Game_mode & GM_MULTIPLAYER) && multi_ts_is_locked()){
		Buttons[gr_screen.res][WL_BUTTON_MULTI_LOCK].button.draw_forced(2);
	}

	if ( wl_icon_being_carried() ) {
		int mx, my, sx, sy;
		Assert(Carried_wl_icon.weapon_class < MAX_WEAPON_TYPES);
		Assert( (Wss_slots != NULL) && (Wl_icons != NULL) );
		mouse_get_pos_unscaled( &mx, &my );
		sx = mx + Wl_delta_x;
		sy = my + Wl_delta_y;

		if ( Wl_icons[Carried_wl_icon.weapon_class].can_use > 0)
		{
			wl_icon_info *icon = &Wl_icons[Carried_wl_icon.weapon_class];
			if(icon->icon_bmaps[WEAPON_ICON_FRAME_SELECTED] != -1)
			{
				gr_set_color_fast(&Color_blue);
				gr_set_bitmap(icon->icon_bmaps[WEAPON_ICON_FRAME_SELECTED]);
				gr_bitmap(sx, sy);
			}
			else
			{
				gr_set_color_fast(&Icon_colors[ICON_FRAME_SELECTED]);
				int w = 56;
				int h = 24;
				draw_brackets_square(sx, sy, sx+w, sy+h);
				if(icon->model_index != -1)
				{
					//Draw the model
					draw_model_icon(icon->model_index, MR_LOCK_DETAIL | MR_NO_FOGGING | MR_NO_LIGHTING, Weapon_info->closeup_zoom / 2.5f, sx, sy, w, h, NULL);
				}
				else if(icon->laser_bmap != -1)
				{
					//Draw laser bitmap
					gr_set_clip(sx, sy, 56, 24);
					gr_set_bitmap(icon->laser_bmap);
					gr_bitmap(0, 0);
					gr_reset_clip();
				}
				else
				{
					//Draw the weapon name, crappy last-ditch effort to not crash.
					int half_x, half_y;
					char *print_name = (Weapon_info[Carried_wl_icon.weapon_class].alt_name[0]) ? Weapon_info[Carried_wl_icon.weapon_class].alt_name : Weapon_info[Carried_wl_icon.weapon_class].name;

					// Truncate the # and everything to the right. Zacam
					end_string_at_first_hash_symbol(print_name);
					
					// Center-align and fit the text for display
					gr_get_string_size(&half_x, &half_y, print_name);
					half_x = sx +((56 - half_x) / 2);
					half_y = sy +((28 - half_y) / 2); // Was ((24 - half_y) / 2) Zacam
					gr_string(half_x, half_y, print_name);
				}
			}
		}

		// draw number to prevent it from disappearing on clicks
		if ( Carried_wl_icon.from_bank >= MAX_SHIP_PRIMARY_BANKS ) {
			if ( mx == Carried_wl_icon.from_x && my == Carried_wl_icon.from_y ) {
				int num_missiles = Wss_slots[Carried_wl_icon.from_slot].wep_count[Carried_wl_icon.from_bank];

				wl_render_icon_count(num_missiles, Wl_bank_coords[gr_screen.res][Carried_wl_icon.from_bank][0], Wl_bank_coords[gr_screen.res][Carried_wl_icon.from_bank][1]);
			}
		}

		// check so see if this is really a legal weapon to carry away
		if ( !Wl_icons[Carried_wl_icon.weapon_class].can_use )
		{
			int diffx, diffy;
			diffx = abs(Carried_wl_icon.from_x-mx);
			diffy = abs(Carried_wl_icon.from_y-my);
			if ( (diffx > 2) || (diffy > 2) ) {
				int ship_class = Wss_slots[Selected_wl_slot].ship_class;

				// might have to get weapon name translation
				if (Lcl_gr)
				{
					char display_name[NAME_LENGTH];
					strncpy(display_name, (Weapon_info[Carried_wl_icon.weapon_class].alt_name[0] != '\0' ) ? Weapon_info[Carried_wl_icon.weapon_class].alt_name : Weapon_info[Carried_wl_icon.weapon_class].name, NAME_LENGTH);
					lcl_translate_wep_name(display_name);
					popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR("A %s is unable to carry %s weaponry", 633), (Ship_info[ship_class].alt_name[0] != '\0') ? Ship_info[ship_class].alt_name : Ship_info[ship_class].name, display_name);
				}
				else
				{
					popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR("A %s is unable to carry %s weaponry", 633), (Ship_info[ship_class].alt_name[0] != '\0') ? Ship_info[ship_class].alt_name : Ship_info[ship_class].name, Weapon_info[Carried_wl_icon.weapon_class].name);
				}

				wl_dump_carried_icon();
			}
		}
	}

	if ( Weapon_anim_class != Selected_wl_class) {
		start_weapon_animation(Selected_wl_class);
	}

	// render weapon description text
	wl_render_weapon_desc(frametime);

	wl_maybe_flash_button();

	// should render the chatbox as close to the end as possible so it overlaps all controls
	if(!Background_playing){
		// render some extra stuff in multiplayer
		if(Game_mode & GM_MULTIPLAYER){				
			// render the chatbox
			chatbox_render();

			// draw tooltips
			Weapon_ui_window.draw_tooltip();

			// render the status indicator for the voice system
			multi_common_voice_display_status();
		}
	}

	// blit help overlay if active
	help_overlay_maybe_blit(WL_OVERLAY);
	gr_flip();	

	// If the commit button was pressed, do the commit button actions.  Done at the end of the
	// loop so there isn't a skip in the animation (since ship_create() can take a long time if
	// the ship model is not in memory
	if ( Commit_pressed ) {
		if(Game_mode & GM_MULTIPLAYER){
			multi_ts_commit_pressed();			
		} else {
			commit_pressed();
		}		
		Commit_pressed = 0;
	}
}

/**
 * Free the bitmap slot and memory that was allocated
 * to store the mask bitmap.
 *
 * @note Weapon_select_open is cleared when this function completes.
 */
void weapon_select_close()
{
	if ( !Weapon_select_open ) {
		nprintf(("Alan","weapon_select_close() returning without doing anything\n"));
		return;
	}

	nprintf(("Alan", "Entering weapon_select_close()\n"));

	// done with the bitmaps, so unlock it
	bm_unlock(WeaponSelectMaskBitmap);

	Weapon_ui_window.destroy();

	// unload bitmaps
	help_overlay_unload(WL_OVERLAY);

	bm_release(WeaponSelectMaskBitmap);

	wl_unload_icons();
	wl_unload_all_anims();
	// ...must be last...
	wl_free_ship_class_data();

	Selected_wl_class = -1;
	Weapon_select_open = 0;

	if(Weapon_slot_bitmap != -1)
	{
		bm_release(Weapon_slot_bitmap);
	}
}


/**
 * Renders the number next to the weapon icon
 *
 * @param num the actual count to be printed
 * @param x x screen position OF THE ICON (NOT where you want the text, this is calculated to prevent overlapping)
 * @param y y screen position OF THE ICON (NOT where you want the text, this is calculated to prevent overlapping)
 */
void wl_render_icon_count(int num, int x, int y)
{
	char buf[32];
	int num_w, num_h;
	int number_to_draw = (num >= 10000) ? 9999 : num;	// cap count @ 9999 - Goober5000 bumped from 999
	Assert(number_to_draw >= 0);

	sprintf(buf, "%d", number_to_draw);
	gr_get_string_size(&num_w, &num_h, buf, strlen(buf));

	// render
	gr_set_color_fast(&Color_white);
	gr_string(x-num_w-4, y+8, buf);
}


/**
 * Render icon 
 *
 * @param index             index into Wl_icons[], identifying which weapon to draw
 * @param x                 x screen position to draw icon at
 * @param y                 y screen position to draw icon at
 * @param num               count for weapon
 * @param draw_num_flag     0 if not to draw count for weapon, nonzero otherwise
 * @param hot_mask          value that should match Hot_weapon_icon to show mouse is over
 * @param hot_bank_mask     value that should match Hot_weapon_bank_icon to show mouse is over
 * @param select_mask       value that should match Selected_wl_class to show icon is selected
 */
void wl_render_icon(int index, int x, int y, int num, int draw_num_flag, int hot_mask, int hot_bank_mask, int select_mask)
{
	int				bitmap_id = -1;
	color			*color_to_draw = NULL;
	wl_icon_info	*icon;

	if ( Selected_wl_slot == -1 )
		return;

	icon = &Wl_icons[index];

	if ( icon->icon_bmaps[0] == -1 && icon->model_index == -1 && icon->laser_bmap == -1) {
		wl_load_icons(index);
	}
		
	// assume default bitmap is to be used
	if(icon->icon_bmaps[0] != -1)
		bitmap_id = icon->icon_bmaps[WEAPON_ICON_FRAME_NORMAL];	// normal frame
	else
		color_to_draw = &Icon_colors[ICON_FRAME_NORMAL];

	// next check if ship has mouse over it
	if ( !wl_icon_being_carried() ) {
		if ( (Hot_weapon_icon > -1 && Hot_weapon_icon == hot_mask) 
			|| (Hot_weapon_bank_icon > -1 && Hot_weapon_bank_icon == hot_bank_mask))
		{
			if(icon->icon_bmaps[0] != -1)
				bitmap_id = icon->icon_bmaps[WEAPON_ICON_FRAME_HOT];	// normal frame
			else
				color_to_draw = &Icon_colors[ICON_FRAME_HOT];
		}
	}

	// if icon is selected
	if ( Selected_wl_class > -1 ) {
		if ( Selected_wl_class == select_mask) {
			if(icon->icon_bmaps[0] != -1)
				bitmap_id = icon->icon_bmaps[WEAPON_ICON_FRAME_SELECTED];	// selected icon
			else
				color_to_draw = &Icon_colors[ICON_FRAME_SELECTED];
		}
	}

	// if icon is disabled
	if ( !icon->can_use ) {
		if(icon->icon_bmaps[0] != -1)
			bitmap_id = icon->icon_bmaps[WEAPON_ICON_FRAME_DISABLED];	// disabled icon
		else
			color_to_draw = &Icon_colors[ICON_FRAME_DISABLED];
	}

	if(bitmap_id != -1)
	{
		gr_set_color_fast(&Color_blue);
		gr_set_bitmap(bitmap_id);
		gr_bitmap(x, y);
	}
	else
	{
		gr_set_color_fast(color_to_draw);
		draw_brackets_square(x, y, x + 56, y + 24);

		if(icon->model_index != -1)
		{
			//Draw the model
			draw_model_icon(icon->model_index, MR_LOCK_DETAIL | MR_NO_FOGGING | MR_NO_LIGHTING, Weapon_info->closeup_zoom * 0.4f, x, y, 56, 24, NULL);
		}
		else if(icon->laser_bmap != -1)
		{
			gr_set_clip(x, y, 56, 24);
			gr_set_bitmap(icon->laser_bmap);
			gr_bitmap(0, 0);
			gr_reset_clip();
		}
		else
		{
			//Draw the weapon name, crappy last-ditch effort to not crash.
			int half_x, half_y;
			char *print_name = (Weapon_info[index].alt_name[0]) ? Weapon_info[index].alt_name : Weapon_info[index].name;

			// Truncate the # and everything to the right. Zacam
			end_string_at_first_hash_symbol(print_name);

			// Center-align and fit the text for display
			gr_get_string_size(&half_x, &half_y, print_name);
			half_x = x +((56 - half_x) / 2);
			half_y = y +((28 - half_y) / 2); // Was ((24 - half_y) / 2) Zacam
			gr_string(half_x, half_y, print_name);
		}
	}

	// draw the number of the item
	// now, right justified
	if ( draw_num_flag != 0 ) {
		wl_render_icon_count(num, x, y);
	}
}

/**
 * Draw the icons for the weapons that are currently on the selected ship
 *
 * @param slot_num Slot to draw weapons for
 */
void wl_draw_ship_weapons(int index)
{
	int		i;
	int		*wep, *wep_count;

	if ( index == -1 )
		return;

	Assert(index >= 0 && index < MAX_WSS_SLOTS);
	Assert(Wss_slots != NULL);
	wep = Wss_slots[index].wep;
	wep_count = Wss_slots[index].wep_count;

	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ )
	{
		if(i < Ship_info[Wss_slots[index].ship_class].num_primary_banks || (i >= MAX_SHIP_PRIMARY_BANKS && ((i - MAX_SHIP_PRIMARY_BANKS) < Ship_info[Wss_slots[index].ship_class].num_secondary_banks)))
		{
			if(Weapon_slot_bitmap != -1)
			{
				gr_set_bitmap(Weapon_slot_bitmap);
				gr_bitmap( Wl_bank_coords[gr_screen.res][i][0] - 2,  Wl_bank_coords[gr_screen.res][i][1] - 2);
			}
			else
			{
				gr_set_color_fast(&Icon_colors[WEAPON_ICON_FRAME_NORMAL]);
				draw_brackets_square( Wl_bank_coords[gr_screen.res][i][0],  Wl_bank_coords[gr_screen.res][i][1],  Wl_bank_coords[gr_screen.res][i][0] + 56,  Wl_bank_coords[gr_screen.res][i][1] + 24);
			}
		}

		if ( Carried_wl_icon.from_bank == i && Carried_wl_icon.from_slot == index ) {
			continue;
		}

		if ( (wep[i] != -1) && (wep_count[i] > 0) )
		{
			wl_render_icon( wep[i], Wl_bank_coords[gr_screen.res][i][0], Wl_bank_coords[gr_screen.res][i][1], wep_count[i], Wl_bank_count_draw_flags[i], -1, i, wep[i]);
		}
	}
}

/**
 * Draw icon with number
 *
 * @param list_count    list position on screen (0-7)
 * @param weapon_class  class of weapon
 */
void draw_wl_icon_with_number(int list_count, int weapon_class)
{
	Assert( list_count >= 0 && list_count < 8 );
	Assert( (Wl_icons != NULL) && (Wl_pool != NULL) );


	if(Wl_icons[weapon_class].can_use)
	{
		gr_set_color_fast(&Icon_colors[WEAPON_ICON_FRAME_NORMAL]);
	}
	else
	{
		gr_set_color_fast(&Icon_colors[WEAPON_ICON_FRAME_DISABLED]);
	}

	wl_render_icon(weapon_class, Wl_weapon_icon_coords[gr_screen.res][list_count][0], Wl_weapon_icon_coords[gr_screen.res][list_count][1],
					   Wl_pool[weapon_class], 1, list_count, -1, weapon_class);
}

/**
 * Draw the weapon icons that are available
 */
void draw_wl_icons()
{
	int i, count;

	count=0;
	for ( i = Plist_start; i < Plist_size; i++ ) {
		draw_wl_icon_with_number(count, Plist[i]);
		if ( ++count > 3 )
			break;
	}

	count=0;
	for ( i = Slist_start; i < Slist_size; i++ ) {
		draw_wl_icon_with_number(count+4, Slist[i]);
		if ( ++count > 3 )
			break;
	}
}

/**
 * Determine if an icon from the scrollable weapon list can be picked up 
 * (for drag and drop).
 *
 * It calculates the difference in x & y between the icon
 * and the mouse, so we can move the icon with the mouse in a realistic way
 * 
 * @param index (0..7) 
 */
void wl_pick_icon_from_list(int index)
{
	int weapon_class, mx, my;

	// if this is a multiplayer game and the player is an observer, he can never pick any weapons up
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)){
		return;
	}

	// if a weapon is being carried, do nothing
	if ( wl_icon_being_carried() ) {
		return;
	}

	if ( index < 4 ) {
		weapon_class = Plist[Plist_start+index];
	} else {
		weapon_class = Slist[Slist_start+index-4];
	}

	// there isn't a weapon there at all!
	if ( weapon_class < 0 ) 
		return;

	Assert( Wl_pool != NULL );

	// no weapons left of that class
	if ( Wl_pool[weapon_class] <= 0 ) {
		return;
	}

	wl_set_carried_icon(-1, -1, weapon_class);
	common_flash_button_init();

	mouse_get_pos_unscaled( &mx, &my );
	Wl_delta_x = Wl_weapon_icon_coords[gr_screen.res][index][0] - mx;
	Wl_delta_y = Wl_weapon_icon_coords[gr_screen.res][index][1] - my;
}

/**
 * Pick from ship slot
 *
 * @param num index into shipb banks (0..2 primary, 3..6 secondary)
 */
void pick_from_ship_slot(int num)
{
	int mx, my, *wep, *wep_count;
		
	Assert(num < 7);

	if ( Selected_wl_slot == -1 )
		return;

	if ( wl_icon_being_carried() )
		return;

	if ( ss_disabled_slot( Selected_wl_slot, false ) )
		return;

	Assert( (Wss_slots != NULL) && (Wl_icons != NULL) );

	wep = Wss_slots[Selected_wl_slot].wep;
	wep_count = Wss_slots[Selected_wl_slot].wep_count;

	// check if a weapon even exists in that slot
	if ( (wep[num] < 0) || (wep_count[num] <= 0) ) {
		return;
	}

	Assert(Wl_icons[wep[num]].can_use);

	wl_set_carried_icon(num, Selected_wl_slot, wep[num]);
	common_flash_button_init();
			
	mouse_get_pos_unscaled( &mx, &my );

	Wl_delta_x = Wl_bank_coords[gr_screen.res][num][0] - mx;
	Wl_delta_y = Wl_bank_coords[gr_screen.res][num][1] - my;

	Carried_wl_icon.from_x = mx;
	Carried_wl_icon.from_y = my;
}

/**
 * Determine if this slot has no weapons
 */
int wl_slots_all_empty(wss_unit *slot)
{
	int			i;

	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ ) {
		if ( (slot->wep_count[i] > 0) && (slot->wep[i] >= 0) ) 
			return 0;
	}

	return 1;
}

/**
 * Change a ship's weapons based on the information contained in the
 * Weapon_data[] structure that is filled in during weapon loadout
 *
 * @return -1 if the player ship has no weapons
 * @return 0  if function finished without errors
 */
int wl_update_ship_weapons(int objnum, wss_unit *slot )
{
	ship_info *sip = &Ship_info[Ships[Player_obj->instance].ship_info_index];
	if(sip->num_secondary_banks <= 0 && sip->num_primary_banks <= 0)
	{
		return 0;
	}
	// AL 11-15-97: Ensure that the player ship hasn't removed all 
	//					 weapons from their ship.  This will cause a warning to appear.
	if ( objnum == OBJ_INDEX(Player_obj) && Weapon_select_open ) {
		if ( wl_slots_all_empty(slot) && (sip->num_primary_banks > 0 || sip->num_secondary_banks > 0)) {
			return -1;
		}
	}

	wl_bash_ship_weapons(&Ships[Objects[objnum].instance].weapons, slot);
	return 0;
}


/**
 * Set the Pilot subsystem of a parse_object to the weapons that are setup
 * for the wing_block,wing_slot ship
 *
 * @param pobjp	Pointer to parse object that references Pilot subsystem
 * @param slot	Pointer to slot object
 */
void wl_update_parse_object_weapons(p_object *pobjp, wss_unit *slot)
{
	int				i,	j, sidx, pilot_index, max_count;
	subsys_status	*ss;

	Assert(slot->ship_class >= 0);

	pilot_index = wl_get_pilot_subsys_index(pobjp);

	if ( pilot_index == -1 ) 
		return;
						
	ss = &Subsys_status[pilot_index];

	for ( i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++ ) {
		ss->primary_banks[i] = -1;		
	}

	for ( i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++ ) {
		ss->secondary_banks[i] = -1;		
	}

	j = 0;
	for ( i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++ )
	{
		if ( (slot->wep_count[i] > 0) && (slot->wep[i] >= 0) )
		{
			ss->primary_banks[j] = slot->wep[i];

			// ballistic primaries - Goober5000
			if (Weapon_info[slot->wep[i]].wi_flags2 & WIF2_BALLISTIC)
			{
				// Important: the primary_ammo[] value is a percentage of max capacity!
				// which means that it's always 100%, since ballistic primaries are completely
				// full upon launch - Goober5000
				ss->primary_ammo[j] = 100;
			}
			else
			{
				ss->primary_ammo[j] = 0;
			}

			j++;
		}
	}

	j = 0;
	for ( i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++ )
	{
		sidx = i+MAX_SHIP_PRIMARY_BANKS;
		if ( (slot->wep_count[sidx] > 0) && (slot->wep[sidx] >= 0) )
		{
			ss->secondary_banks[j] = slot->wep[sidx];

			// Important: the secondary_ammo[] value is a percentage of max capacity!
			max_count = wl_calc_missile_fit(slot->wep[sidx], Ship_info[slot->ship_class].secondary_bank_ammo_capacity[j]);
			ss->secondary_ammo[j] = fl2i( i2fl(slot->wep_count[sidx]) / max_count * 100.0f + 0.5f);
			
			j++;
		}
	}
}

/**
 * Start the current weapon animation from playing.
 */
void start_weapon_animation(int weapon_class) 
{
	char *p;
	char animation_filename[CF_MAX_FILENAME_LENGTH+4];

	anim_timer_start = timer_get_milliseconds();

	if ( weapon_class < 0 )
		return;

	if ( weapon_class == Weapon_anim_class ) 
		return;

	gamesnd_play_iface(SND_WEAPON_ANIM_START);

	//load a new animation if it's different to what's already playing
	if(strcmp(Cur_Anim.filename, Weapon_info[weapon_class].anim_filename) != 0) {
		//unload the previous anim
		generic_anim_unload(&Cur_Anim);
		//load animation here, we now only have one loaded
		p = strchr( Weapon_info[weapon_class].anim_filename, '.' );
		if(p)
			*p = '\0';
		if (gr_screen.res == GR_1024) {
			strcpy_s(animation_filename, "2_");
			strcat_s(animation_filename, Weapon_info[weapon_class].anim_filename);
		}
		else {
			strcpy_s(animation_filename, Weapon_info[weapon_class].anim_filename);
		}

		generic_anim_init(&Cur_Anim, animation_filename);
		Cur_Anim.ani.bg_type = bm_get_type(Weapon_select_background_bitmap);
		if(generic_anim_stream(&Cur_Anim) == -1) {
			//we've failed to load an animation, load an image and treat it like a 1 frame animation
			Cur_Anim.first_frame = bm_load(Weapon_info[weapon_class].anim_filename);	//if we fail here, the value is still -1
			if(Cur_Anim.first_frame != -1) {
				Cur_Anim.num_frames = 1;
			}
		}
	}

	// start the text wipe
	wl_weapon_desc_start_wipe();

	Weapon_anim_class = weapon_class;

	if ( Wl_icons[weapon_class].model_index >= 0 )
		return;
}

/**
 * Reset the weapons loadout to the defaults in the mission
 */
void wl_reset_to_defaults()
{
	// don't reset of weapons pool in multiplayer
	if(Game_mode & GM_MULTIPLAYER){
		return;
	}

	wl_init_pool(&Team_data[Common_team]);
	wl_init_icon_lists();
	wl_fill_slots();
	wl_reset_selected_slot();
	wl_reset_carried_icon();
	wl_maybe_reset_selected_weapon_class();
}

/**
 * Bash ship weapons, based on what is stored in the stored weapons loadout
 * @note Wss_slots[] is assumed to be correctly set
 */
void wl_bash_ship_weapons(ship_weapon *swp, wss_unit *slot)
{
	int i, j, sidx;

	j = 0;
	for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
		// set to default value first thing
		swp->primary_bank_weapons[i] = -1;

		// now set to true value
		if ( (slot->wep_count[i] > 0) && (slot->wep[i] >= 0) ) {
			swp->primary_bank_weapons[j] = slot->wep[i];

			// ballistic primaries - Goober5000
			if (Weapon_info[swp->primary_bank_weapons[j]].wi_flags2 & WIF2_BALLISTIC) {
				// this is a bit tricky: we must recalculate ammo for full capacity
				// since wep_count for primaries does not store the ammo; ballistic
				// primaries always come with a full magazine
				swp->primary_bank_ammo[j] = wl_calc_ballistic_fit(swp->primary_bank_weapons[j], Ship_info[slot->ship_class].primary_bank_ammo_capacity[i]);
			} else {
				swp->primary_bank_ammo[j] = 0;
			}

			j++;
		}
	}

	// update our # of primary banks
	swp->num_primary_banks = j;

	j = 0;
	for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
		// set to default value first thing
		swp->secondary_bank_weapons[i] = -1;

		// now set the true value
		sidx = i+MAX_SHIP_PRIMARY_BANKS;
		if ( (slot->wep_count[sidx] > 0) && (slot->wep[sidx] >= 0) ) {
			swp->secondary_bank_weapons[j] = slot->wep[sidx];
			swp->secondary_bank_ammo[j] = slot->wep_count[sidx];
			j++;
		}
	}

	// update our # of secondary banks
	swp->num_secondary_banks = j;
}

/**
 * Utility function for swapping two banks
 */
void wl_swap_weapons(int ship_slot, int from_bank, int to_bank)
{
	wss_unit	*slot;
	int	tmp;

	Assert( Wss_slots != NULL );

	slot = &Wss_slots[ship_slot];

	if ( from_bank == to_bank || (from_bank >= MAX_SHIP_WEAPONS || from_bank < 0 ) || (to_bank >= MAX_SHIP_WEAPONS || to_bank < 0 ) ) {
		return;
	}

	// swap weapon type
	tmp = slot->wep[from_bank];
	slot->wep[from_bank] = slot->wep[to_bank];
	slot->wep[to_bank] = tmp;

	// swap weapon count
	tmp = slot->wep_count[from_bank];
	slot->wep_count[from_bank] = slot->wep_count[to_bank];
	slot->wep_count[to_bank] = tmp;
}

/**
 * Utility function used to put back overflow into the weapons pool
 */
void wl_saturate_bank(int ship_slot, int bank)
{
	wss_unit	*slot;
	int		max_count, overflow;

	Assert( Wss_slots != NULL );

	slot = &Wss_slots[ship_slot];

	if ( (slot->wep[bank] < 0) || (slot->wep_count[bank] <= 0) ) {
		return;
	}

	max_count = wl_calc_missile_fit(slot->wep[bank], Ship_info[slot->ship_class].secondary_bank_ammo_capacity[bank-3]);
	overflow = slot->wep_count[bank] - max_count;
	if ( overflow > 0 ) {
		Assert( Wl_pool != NULL );

		slot->wep_count[bank] -= overflow;
		// add overflow back to pool
		Wl_pool[slot->wep[bank]] += overflow;
	}
}

// exit: 0 -> no data changed
//			1 -> data changed
//       sound => gets filled with sound id to play
// updated for specific bank by Goober5000
int wl_swap_slot_slot(int from_bank, int to_bank, int ship_slot, int *sound, net_player *pl)
{
	wss_unit	*slot;
	int class_mismatch_flag, forced_update;

	Assert( Wss_slots != NULL );

	slot = &Wss_slots[ship_slot];

	// usually zero, unless we have a strange update thingy
	forced_update = 0;

	if ( slot->ship_class == -1 ) {
		Int3();	// should not be possible
		return forced_update;
	}
	
	// do nothing if swapping with self
	if ( from_bank == to_bank ) {
		*sound=SND_ICON_DROP_ON_WING;
		return forced_update;	// no update
	}

	// ensure that source bank exists and has something to pick from
	if ( slot->wep[from_bank] == -1 || slot->wep_count[from_bank] <= 0 ) {
		return forced_update;
	}

	// ensure that the dest bank exists
	if ( slot->wep_count[to_bank] < 0 ) {
		return forced_update;
	}

	Assert( Wl_pool != NULL );

	// ensure banks are compatible as far as primary and secondary
	class_mismatch_flag = (IS_BANK_PRIMARY(from_bank) && IS_BANK_SECONDARY(to_bank)) || (IS_BANK_SECONDARY(from_bank) && IS_BANK_PRIMARY(to_bank));
	
	// further ensure that restrictions aren't breached
	if (!class_mismatch_flag) {
		ship_info *sip = &Ship_info[slot->ship_class];

		// check the to-bank first
		if (eval_weapon_flag_for_game_type(sip->restricted_loadout_flag[to_bank])) {
			if (!eval_weapon_flag_for_game_type(sip->allowed_bank_restricted_weapons[to_bank][slot->wep[from_bank]])) {
				char display_name[NAME_LENGTH];
				char txt[100];

				strncpy(display_name, (Weapon_info[slot->wep[from_bank]].alt_name[0]) ? Weapon_info[slot->wep[from_bank]].alt_name : Weapon_info[slot->wep[from_bank]].name, NAME_LENGTH);

				// might have to get weapon name translation
				if (Lcl_gr) {
					lcl_translate_wep_name(display_name);
				}

				sprintf(txt, NOX("This bank is unable to carry '%s' weaponry"), display_name);

				if ( !(Game_mode & GM_MULTIPLAYER) || (Netgame.host == pl) ) {
					popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, txt);
				} else if (pl != NULL) {
					send_game_chat_packet(Netgame.host, txt, MULTI_MSG_TARGET, pl, NULL, 1);
				}

				return forced_update;
			}
		}

		// check the from-bank
		if (eval_weapon_flag_for_game_type(sip->restricted_loadout_flag[from_bank])) {
			if (!eval_weapon_flag_for_game_type(sip->allowed_bank_restricted_weapons[from_bank][slot->wep[to_bank]])) {
				// going from "from" to "to" is valid (from previous if), but going the other way isn't...
				// so return the "to" to the list and just move the "from"

				// put to_bank back into list
				Wl_pool[slot->wep[to_bank]] += slot->wep_count[to_bank];			// return to list
				slot->wep[to_bank] = -1;											// remove from slot
				slot->wep_count[to_bank] = 0;
				*sound=SND_ICON_DROP;				// unless it changes later
				forced_update = 1;					// because we can't return right away
			}
		}
	}

	if ( class_mismatch_flag ) {
		// put from_bank back into list
		Wl_pool[slot->wep[from_bank]] += slot->wep_count[from_bank];		// return to list
		slot->wep[from_bank] = -1;														// remove from slot
		slot->wep_count[from_bank] = 0;
		*sound=SND_ICON_DROP;
		return 1;
	}

	// case 1: primaries (easy, even with ballistics, because ammo is always maximized)
	if ( IS_BANK_PRIMARY(from_bank) && IS_BANK_PRIMARY(to_bank) ) {
		wl_swap_weapons(ship_slot, from_bank, to_bank);
		*sound=SND_ICON_DROP_ON_WING;
		return 1;
	}

	// case 2: secondaries (harder)
	if ( IS_BANK_SECONDARY(from_bank) && IS_BANK_SECONDARY(to_bank) ) {
		// case 2a: secondaries are the same type
		if ( slot->wep[from_bank] == slot->wep[to_bank] ) {
			int dest_max, dest_can_fit, source_can_give;
			dest_max = wl_calc_missile_fit(slot->wep[to_bank], Ship_info[slot->ship_class].secondary_bank_ammo_capacity[to_bank-3]);

			dest_can_fit = dest_max - slot->wep_count[to_bank];

			if ( dest_can_fit <= 0 ) {
				// dest bank is already full.. nothing to do here
				return forced_update;
			}

			// see how much source can give
			source_can_give = MIN(dest_can_fit, slot->wep_count[from_bank]);

			if ( source_can_give > 0 ) {			
				slot->wep_count[to_bank] += source_can_give;		// add to dest
				slot->wep_count[from_bank] -= source_can_give;	// take from source
				*sound=SND_ICON_DROP_ON_WING;
				return 1;
			} else {
				return forced_update;
			}
		}

		// case 2b: secondaries are different types
		if ( slot->wep[from_bank] != slot->wep[to_bank] ) {
			// swap 'em 
			wl_swap_weapons(ship_slot, from_bank, to_bank);

			// put back some on list if required
			wl_saturate_bank(ship_slot, from_bank);
			wl_saturate_bank(ship_slot, to_bank);
			*sound=SND_ICON_DROP_ON_WING;
			return 1;
		}
	}

	Int3();		// should never get here
	return forced_update;
}

// exit: 0 -> no data changed
//			1 -> data changed
//       sound => gets filled with sound id to play
int wl_dump_to_list(int from_bank, int to_list, int ship_slot, int *sound)
{
	wss_unit	*slot;

	Assert( (Wss_slots != NULL) && (Wl_pool != NULL) );

	slot = &Wss_slots[ship_slot];

	// ensure that source bank exists and has something to pick from
	if ( slot->wep[from_bank] == -1 || slot->wep_count[from_bank] <= 0 ) {
		return 0;
	}

	// put weapon bank to the list
	Wl_pool[to_list] += slot->wep_count[from_bank];			// return to list
	slot->wep[from_bank] = -1;										// remove from slot
	slot->wep_count[from_bank] = 0;
	*sound=SND_ICON_DROP;

	return 1;
}

// exit: 0 -> no data changed
//			1 -> data changed
//       sound => gets filled with sound id to play
int wl_grab_from_list(int from_list, int to_bank, int ship_slot, int *sound, net_player *pl)
{
	int update=0;
	wss_unit	*slot;

	Assert( (Wss_slots != NULL) && (Wl_pool != NULL) );

	slot = &Wss_slots[ship_slot];
	int max_fit;

	// ensure that the banks are both of the same class
	if ( (IS_LIST_PRIMARY(from_list) && IS_BANK_SECONDARY(to_bank)) || (IS_LIST_SECONDARY(from_list) && IS_BANK_PRIMARY(to_bank)) )
	{
		// do nothing
		*sound=SND_ICON_DROP;
		return 0;
	}

	// ensure that dest bank exists
	if ( slot->wep_count[to_bank] < 0 ) {
		*sound=SND_ICON_DROP;
		return 0;
	}

	// bank should be empty:
	Assert(slot->wep_count[to_bank] == 0);
	Assert(slot->wep[to_bank] < 0);

	// ensure that pool has weapon
	if ( Wl_pool[from_list] <= 0 ) {
		return 0;
	}

	ship_info *sip = &Ship_info[slot->ship_class];

	// ensure that this bank will accept the weapon...
	if (eval_weapon_flag_for_game_type(sip->restricted_loadout_flag[to_bank])) {
		if (!eval_weapon_flag_for_game_type(sip->allowed_bank_restricted_weapons[to_bank][from_list])) {
			char display_name[NAME_LENGTH];
			char txt[100];

			strncpy(display_name, Weapon_info[from_list].name, NAME_LENGTH);

			// might have to get weapon name translation
			if (Lcl_gr) {
				lcl_translate_wep_name(display_name);
			}

			sprintf(txt, NOX("This bank is unable to carry '%s' weaponry"), display_name);

			if ( !(Game_mode & GM_MULTIPLAYER) || (Netgame.host == pl) ) {
				popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, txt);
			} else if (pl != NULL) {
				send_game_chat_packet(Netgame.host, txt, MULTI_MSG_TARGET, pl, NULL, 1);
			}

			return 0;
		}
	}

	// we will have some sort of success from this point
	update = 1;

	// find how much dest bank can fit
	if ( to_bank < MAX_SHIP_PRIMARY_BANKS ) {
		max_fit = 1;
	} else {
		max_fit = wl_calc_missile_fit(from_list, Ship_info[slot->ship_class].secondary_bank_ammo_capacity[to_bank-MAX_SHIP_PRIMARY_BANKS]);
	}

	// take weapon from list
	if ( Wl_pool[from_list] < max_fit ) {
		max_fit = Wl_pool[from_list];
		update=2;
	}
	Wl_pool[from_list] -= max_fit;

	// put on the slot
	slot->wep[to_bank] = from_list;
	slot->wep_count[to_bank] = max_fit;

	*sound=SND_ICON_DROP_ON_WING;

	return update;
}

// exit: 0 -> no data changed
//			1 -> data changed
//       sound => gets filled with sound id to play
int wl_swap_list_slot(int from_list, int to_bank, int ship_slot, int *sound, net_player *pl)
{
	wss_unit	*slot;

	Assert( (Wss_slots != NULL) && (Wl_pool != NULL) );

	slot = &Wss_slots[ship_slot];
	int max_fit;

	// ensure that the banks are both of the same class
	if ( (IS_LIST_PRIMARY(from_list) && IS_BANK_SECONDARY(to_bank)) || (IS_LIST_SECONDARY(from_list) && IS_BANK_PRIMARY(to_bank)) ) {
		// do nothing
		*sound=SND_ICON_DROP;
		return 0;
	}

	// ensure that dest bank exists
	if ( slot->wep_count[to_bank] < 0 ) {
		return 0;
	}

	// bank should have something in it
	Assert(slot->wep_count[to_bank] > 0);
	Assert(slot->wep[to_bank] >= 0);

	// ensure that pool has weapon
	if ( Wl_pool[from_list] <= 0 ) {
		return 0;
	}

	ship_info *sip = &Ship_info[slot->ship_class];

	// ensure that this bank will accept the weapon
	if (eval_weapon_flag_for_game_type(sip->restricted_loadout_flag[to_bank])) {
		if (!eval_weapon_flag_for_game_type(sip->allowed_bank_restricted_weapons[to_bank][from_list])) {
			char display_name[NAME_LENGTH];
			char txt[100];

			strncpy(display_name, (Weapon_info[from_list].alt_name[0]) ? Weapon_info[from_list].alt_name : Weapon_info[from_list].name, NAME_LENGTH);

			// might have to get weapon name translation
			if (Lcl_gr) {
				lcl_translate_wep_name(display_name);
			}

			sprintf(txt, NOX("This bank is unable to carry '%s' weaponry"), display_name);

			if ( !(Game_mode & GM_MULTIPLAYER) || (Netgame.host == pl) ) {
				popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, txt);
			} else if (pl != NULL) {
				send_game_chat_packet(Netgame.host, txt, MULTI_MSG_TARGET, pl, NULL, 1);
			}

			return 0;
		}
	}

	// dump slot weapon back into list
	Wl_pool[slot->wep[to_bank]] += slot->wep_count[to_bank];
	slot->wep_count[to_bank] = 0;
	slot->wep[to_bank] = -1;

	// put weapon on ship from list
	
	// find how much dest bank can fit
	if ( to_bank < MAX_SHIP_PRIMARY_BANKS ) {
		max_fit = 1;
	} else {
		max_fit = wl_calc_missile_fit(from_list, Ship_info[slot->ship_class].secondary_bank_ammo_capacity[to_bank-MAX_SHIP_PRIMARY_BANKS]);
	}

	// take weapon from list
	if ( Wl_pool[from_list] < max_fit ) {
		max_fit = Wl_pool[from_list];
	}
	Wl_pool[from_list] -= max_fit;

	// put on the slot
	slot->wep[to_bank] = from_list;
	slot->wep_count[to_bank] = max_fit;

	*sound=SND_ICON_DROP_ON_WING;
	return 1;
}

/**
 * Update any interface data that may be dependent on Wss_slots[] 
 * @todo Implement
 */
void wl_synch_interface()
{
}

int wl_apply(int mode,int from_bank,int from_list,int to_bank,int to_list,int ship_slot,int player_index, bool dont_play_sound)
{
	int update=0;
	int sound=-1;
	net_player *pl;

	// get the appropriate net player
	if(Game_mode & GM_MULTIPLAYER){
		if(player_index == -1){
			pl = Net_player;
		} else {
			pl = &Net_players[player_index];
		}
	} else {
		pl = NULL;
	}

	switch(mode){
	case WSS_SWAP_SLOT_SLOT:
		update = wl_swap_slot_slot(from_bank, to_bank, ship_slot, &sound, pl);
		break;
	case WSS_DUMP_TO_LIST:
		update = wl_dump_to_list(from_bank, to_list, ship_slot, &sound);
		break;
	case WSS_GRAB_FROM_LIST:
		update = wl_grab_from_list(from_list, to_bank, ship_slot, &sound, pl);
		break;
	case WSS_SWAP_LIST_SLOT:
		update = wl_swap_list_slot(from_list, to_bank, ship_slot, &sound, pl);
		break;
	}

	// only play this sound if the move was done locally (by the host in other words)
	if ( (sound >= 0) && (player_index == -1) && !dont_play_sound) {	
		gamesnd_play_iface(sound);	
	}	

	if ( update ) {
		if ( MULTIPLAYER_HOST ) {
			int size;
			ubyte wss_data[MAX_PACKET_SIZE-20];

			size = store_wss_data(wss_data, MAX_PACKET_SIZE-20,sound,player_index);			
			Assert(pl != NULL);
			send_wss_update_packet(pl->p_info.team,wss_data, size);
		}

		if(Game_mode & GM_MULTIPLAYER){
			Assert(pl != NULL);

			// if the pool we're using has changed, synch stuff up
			if(pl->p_info.team == Net_player->p_info.team){
				wl_synch_interface();			
			}
		} else {
			wl_synch_interface();
		}
	}
	
	return update;
}

int wl_drop(int from_bank,int from_list,int to_bank,int to_list, int ship_slot, int player_index, bool dont_play_sound)
{
	int mode;
	int update=0;
	net_player *pl;

	// get the appropriate net player
	if(Game_mode & GM_MULTIPLAYER){
		if(player_index == -1){
			pl = Net_player;
		} else {
			pl = &Net_players[player_index];
		}
	} else {
		pl = NULL;
	}

	common_flash_button_init();
	if ( !(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_HOST ) {
		if(MULTI_TEAM){
            Assert(pl != NULL);
			// set the global pointers to the right pools
			common_set_team_pointers(pl->p_info.team);
		}

		mode = wss_get_mode(from_bank, from_list, to_bank, to_list, ship_slot);
		if ( mode >= 0 ) {
			update = wl_apply(mode, from_bank, from_list, to_bank, to_list, ship_slot, player_index, dont_play_sound);
		}		

		if(MULTI_TEAM){
			// set the global pointers to the right pools
			common_set_team_pointers(Net_player->p_info.team);
		}
	} else {
		send_wss_request_packet(Net_player->player_id, from_bank, from_list, to_bank, to_list, ship_slot, -1, WSS_WEAPON_SELECT);
	}

	return update;
}

// Goober5000
void wl_apply_current_loadout_to_all_ships_in_current_wing()
{
	bool error_flag;
	int source_wss_slot, cur_wss_slot;
	int cur_wing_block, cur_wing_slot, cur_bank;
	int weapon_type_to_add, result;
	int i;

	ship_info *sip, *source_sip;
	weapon_info *wip;

	char ship_name[NAME_LENGTH];
	char error_messages[MAX_WING_SLOTS * MAX_SHIP_WEAPONS][50 + NAME_LENGTH * 2];
	char *wep_display_name;
	char buf[NAME_LENGTH];

	// clear error stuff
	error_flag = false;
	for (i = 0; i < MAX_WING_SLOTS * MAX_SHIP_WEAPONS; i++)
		*error_messages[i] = '\0';

	// make sure we're not holding anything
	wl_dump_carried_icon();

	// find the currently selected ship (or the squadron leader if none)
	source_wss_slot = Selected_wl_slot;
	if (source_wss_slot == -1)
		source_wss_slot = 0;

	// get its class
	source_sip = &Ship_info[Wss_slots[source_wss_slot].ship_class];

	// find which wing this ship is part of
	cur_wing_block = source_wss_slot / MAX_WING_SLOTS;

	// for all ships in the wing
	for (cur_wing_slot = 0; cur_wing_slot < MAX_WING_SLOTS; cur_wing_slot++)
	{
		Assert( Wss_slots != NULL );

		// get the slot for this ship
		cur_wss_slot = cur_wing_block * MAX_WING_SLOTS + cur_wing_slot;

		// get the ship's name and class
		sip = &Ship_info[Wss_slots[cur_wss_slot].ship_class];	
		ss_return_name(cur_wing_block, cur_wing_slot, ship_name);

		// don't process the selected ship
		if (cur_wss_slot == source_wss_slot)
			continue;

		// must be valid for us to change
		if (!ss_valid_slot(cur_wss_slot))
			continue;

		// copy weapons from source slot to this slot
		for (cur_bank = 0; cur_bank < MAX_SHIP_WEAPONS; cur_bank++)
		{
			// this bank must exist on both the source ship and the destination ship
			if (cur_bank < MAX_SHIP_PRIMARY_BANKS)
			{
				if (cur_bank >= source_sip->num_primary_banks || cur_bank >= sip->num_primary_banks)
					continue;
			}
			else
			{
				if (cur_bank-MAX_SHIP_PRIMARY_BANKS >= source_sip->num_secondary_banks || cur_bank-MAX_SHIP_PRIMARY_BANKS >= sip->num_secondary_banks)
					continue;
			}

			// dump the destination ship's weapons
			wl_drop(cur_bank, -1, -1, Wss_slots[cur_wss_slot].wep[cur_bank], cur_wss_slot, -1, true);

			// weapons must be present on the source ship
			if (Wss_slots[source_wss_slot].wep[cur_bank] < 0)
				continue;

			// determine the weapon we need
			weapon_type_to_add = Wss_slots[source_wss_slot].wep[cur_bank];
			wip = &Weapon_info[weapon_type_to_add];

			// maybe localize
			if (Lcl_gr)
			{
				strncpy(buf, (Weapon_info[weapon_type_to_add].alt_name[0]) ? Weapon_info[weapon_type_to_add].alt_name : Weapon_info[weapon_type_to_add].name, NAME_LENGTH);
				lcl_translate_wep_name(buf);
				wep_display_name = buf;
			}
			else
			{
				wep_display_name = (Weapon_info[weapon_type_to_add].alt_name[0]) ? Weapon_info[weapon_type_to_add].alt_name : Weapon_info[weapon_type_to_add].name;
			}

			// make sure this ship can accept this weapon
			if (!eval_weapon_flag_for_game_type(sip->allowed_weapons[weapon_type_to_add])
				|| ((wip->wi_flags2 & WIF2_BALLISTIC) && !(sip->flags & SIF_BALLISTIC_PRIMARIES)))
			{
				sprintf(error_messages[cur_wing_slot * MAX_SHIP_WEAPONS + cur_bank], NOX("%s is unable to carry %s weaponry"), ship_name, wep_display_name);
				error_flag = true;
				continue;
			}

			// make sure this bank can accept this weapon
			if (eval_weapon_flag_for_game_type(sip->restricted_loadout_flag[cur_bank]))
			{
				if (!eval_weapon_flag_for_game_type(sip->allowed_bank_restricted_weapons[cur_bank][weapon_type_to_add]))
				{
					if (cur_bank < MAX_SHIP_PRIMARY_BANKS)
						sprintf(error_messages[cur_wing_slot * MAX_SHIP_WEAPONS + cur_bank], NOX("%s is unable to carry %s weaponry in primary bank %d"), ship_name, wep_display_name, cur_bank+1);
					else
						sprintf(error_messages[cur_wing_slot * MAX_SHIP_WEAPONS + cur_bank], NOX("%s is unable to carry %s weaponry in secondary bank %d"), ship_name, wep_display_name, cur_bank+1-MAX_SHIP_PRIMARY_BANKS);

					error_flag = true;
					continue;
				}
			}

			// add from the weapon pool
			result = wl_drop(-1, weapon_type_to_add, cur_bank, -1, cur_wss_slot, -1, true);

			// bank left unfilled or partially filled
			if ((result == 0) || (result == 2))
			{
				sprintf(error_messages[cur_wing_slot * MAX_SHIP_WEAPONS + cur_bank], NOX("Insufficient %s available to arm %s"), (Weapon_info[weapon_type_to_add].alt_name[0]) ? Weapon_info[weapon_type_to_add].alt_name : Weapon_info[weapon_type_to_add].name, ship_name);
				error_flag = true;
				continue;
			}
		}
	}

	// play sound
	gamesnd_play_iface(SND_ICON_DROP_ON_WING);

	// display error messages
	if (error_flag)
	{
		int j;
		bool is_duplicate;
		char error_msg[MAX_WING_SLOTS * MAX_SHIP_WEAPONS * (50 + NAME_LENGTH * 2) + 40];
		strcpy_s(error_msg, "The following errors were encountered:\n\n");

		// copy all messages
		for (i = 0; i < (MAX_WING_SLOTS * MAX_SHIP_WEAPONS); i++)
		{
			// there should be a message here
			if (*error_messages[i] == '\0')
				continue;

			// check for duplicate messages
			is_duplicate = false;
			for (j = 0; j < i; j++)
			{
				if (strcmp(error_messages[i], error_messages[j]) == 0)
				{
					is_duplicate = true;
					break;
				}
			}
			if (is_duplicate)
			{
				continue;
			}

			// copy message
			strcat_s(error_msg, error_messages[i]);
			strcat_s(error_msg, "\n");
		}

		// remove last endline
		error_msg[strlen(error_msg)-1] = '\0';

		// display popup
		popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, error_msg);
	}
}
