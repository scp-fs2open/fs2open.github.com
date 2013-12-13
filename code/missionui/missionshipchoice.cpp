/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "missionui/missionscreencommon.h"
#include "missionui/missionshipchoice.h"
#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "missionui/missionbrief.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "ship/ship.h"
#include "io/key.h"
#include "render/3d.h"
#include "globalincs/linklist.h"
#include "io/mouse.h"
#include "playerman/player.h"
#include "pilotfile/pilotfile.h"
#include "menuui/snazzyui.h"
#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "missionui/missionweaponchoice.h"
#include "gamehelp/contexthelp.h"
#include "gamesnd/gamesnd.h"
#include "mission/missionhotkey.h"
#include "popup/popup.h"
#include "hud/hudwingmanstatus.h"
#include "hud/hudparse.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "lighting/lighting.h"
#include "cmdline/cmdline.h"
#include "cfile/cfile.h"
#include "hud/hudbrackets.h"
#include "species_defs/species_defs.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiui.h"
#include "network/multiteamselect.h"
#include "network/multiutil.h"
#include "ai/aigoals.h"
#include "io/timer.h"
#include "weapon/weapon.h"


//////////////////////////////////////////////////////
// Game-wide Globals
//////////////////////////////////////////////////////
char default_player_ship[255];
int Select_default_ship = 0;
int Ship_select_open = 0;	// This game-wide global flag is set to 1 to indicate that the ship
									// select screen has been opened and memory allocated.  This flag
									// is needed so we can know if ship_select_close() needs to called if
									// restoring a game from the Options screen invoked from ship select

int Commit_pressed;	// flag to indicate that the commit button was pressed
							// use a flag, so the ship_create() can be done at the end of the loop

//////////////////////////////////////////////////////
// Module Globals
//////////////////////////////////////////////////////
static int Ship_anim_class = -1;		// ship class that is playing as an animation
static int Ss_delta_x, Ss_delta_y;	// used to offset the carried icon to make it smoothly leave static position

// UnknownPlayer //
float ShipSelectScreenShipRot = 0.0f;
int ShipSelectModelNum = -1;

int anim_timer_start = 0;
//static matrix ShipScreenOrient = IDENTITY_MATRIX;

//////////////////////////////////////////////////////
// UI Data structs
//////////////////////////////////////////////////////
typedef struct ss_icon_info
{
	int				icon_bmaps[NUM_ICON_FRAMES];
	int				current_icon_bitmap;
	int				model_index;
	generic_anim	ss_anim;
} ss_icon_info;

typedef struct ss_slot_info
{
	int status;			// slot status (WING_SLOT_DISABLED, etc)
	int sa_index;		// index into ship arrival list, -1 if ship is created
	int original_ship_class;
} ss_slot_info;

typedef struct ss_wing_info
{
	int num_slots;
	int wingnum;
	int is_late;
	ss_slot_info ss_slots[MAX_WING_SLOTS];
} ss_wing_info;

//ss_icon_info	Ss_icons[MAX_SHIP_CLASSES];		// holds ui info on different ship icons
//ss_wing_info	Ss_wings[MAX_WING_BLOCKS];		// holds ui info for wings and wing slots

ss_wing_info	Ss_wings_teams[MAX_TVT_TEAMS][MAX_WING_BLOCKS];
ss_wing_info	*Ss_wings = NULL;

ss_icon_info	Ss_icons_teams[MAX_TVT_TEAMS][MAX_SHIP_CLASSES];
ss_icon_info	*Ss_icons = NULL;

int Ss_mouse_down_on_region = -1;

int Selected_ss_class;	// set to ship class of selected ship, -1 if none selected
int Hot_ss_icon;			// index that icon is over in list (0..MAX_WING_SLOTS-1)
int Hot_ss_slot;			// index for slot that mouse is over (0..MAX_WSS_SLOTS)

////////////////////////////////////////////////////////////
// Ship Select UI
////////////////////////////////////////////////////////////
UI_WINDOW	Ship_select_ui_window;	

static int Ship_anim_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		257, 84		// GR_640
	},
	{
		412, 135	// GR_1024
	}
};

static int Ship_info_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		28, 78				// GR_640
	},
	{
		45, 125				// GR_1024
	}
};

// coordinate lookup indicies
#define SHIP_SELECT_X_COORD 0
#define SHIP_SELECT_Y_COORD 1
#define SHIP_SELECT_W_COORD 2
#define SHIP_SELECT_H_COORD 3


// NK: changed from 37 to 51 for new FS2 animations
#define SHIP_ANIM_LOOP_FRAME	51

#define MAX_ICONS_ON_SCREEN	4

// (x,y) pairs for ship icon and ship icon number
int Ship_list_coords[GR_NUM_RESOLUTIONS][MAX_ICONS_ON_SCREEN][4] = {
	{
		{23,331,4,341},
		{23,361,4,371},
		{23,391,4,401},
		{23,421,4,431}
	},
	{
		{29,530,10,540},
		{29,578,10,588},
		{29,626,10,636},
		{29,674,10,684}
	}
};

// Store the x locations for the icons in the wing formations
int Wing_icon_coords[GR_NUM_RESOLUTIONS][MAX_WSS_SLOTS][2] = {
	{
		{124,345},
		{100,376},
		{148,376},
		{124,407},

		{222,345},
		{198,376},
		{246,376},
		{222,407},

		{320,345},
		{296,376},
		{344,376},
		{320,407}
	},
	{
		{218,584},
		{194,615},
		{242,615},
		{218,646},

		{373,584},
		{349,615},
		{397,615},
		{373,646},

		{531,584},
		{507,615},
		{555,615},
		{531,646}
	}
};

//////////////////////////////////////////////////////
// Linked List of icons to show on ship selection list
//////////////////////////////////////////////////////
#define SS_ACTIVE_ITEM_USED	(1<<0)
typedef struct ss_active_item
{
	ss_active_item	*prev, *next;
	int				ship_class;
	int				flags;
} ss_active_item;

static ss_active_item	SS_active_head;
//static ss_active_item	SS_active_items[MAX_WSS_SLOTS];//DTP commented out or else singleplayer will only have a max of MAX_WSS_SLOTS ships
static ss_active_item	SS_active_items[MAX_SHIP_CLASSES];//DTP, now we have all ships in the TBL, as they can all be playerships

static int SS_active_list_start;
static int SS_active_list_size;

//////////////////////////////////////////////////////
// Background bitmaps data for ship_select
//////////////////////////////////////////////////////
static char* Ship_select_background_fname[GR_NUM_RESOLUTIONS] = {
	"ShipSelect",
	"2_ShipSelect"
};

static char* Ship_select_background_mask_fname[GR_NUM_RESOLUTIONS] = {
	"ShipSelect-m",
	"2_ShipSelect-m"
};

int Ship_select_background_bitmap;

//////////////////////////////////////////////////////
// Ship select specific buttons
//////////////////////////////////////////////////////
#define NUM_SS_BUTTONS				4
#define SS_BUTTON_SCROLL_UP		0
#define SS_BUTTON_SCROLL_DOWN		1
#define SS_BUTTON_RESET				2
#define SS_BUTTON_DUMMY				3	// needed to capture mouse for drag/drop icons

// convenient struct for handling all button controls
struct ss_buttons {
	char *filename;
	int x, y, xt, yt;
	int hotspot;
	int scrollable;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	ss_buttons(char *name, int x1, int y1, int xt1, int yt1, int h, int s) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h), scrollable(s) {}
};

static ss_buttons Ship_select_buttons[GR_NUM_RESOLUTIONS][NUM_SS_BUTTONS] = {
	{	// GR_640
		ss_buttons("ssb_08",		5,			303,	-1,	-1,	8,	0),		// SCROLL UP
		ss_buttons("ssb_09",		5,			454,	-1,	-1,	9,	0),		// SCROLL DOWN
		ss_buttons("ssb_39",		571,		347,	-1,	-1,	39,0),		// RESET
		ss_buttons("ssb_39",		0,			0,		-1,	-1,	99,0)			// dummy for drag n' drop
	},
	{	// GR_1024
		ss_buttons("2_ssb_08",	8,			485,	-1,	-1,	8,	0),		// SCROLL UP
		ss_buttons("2_ssb_09",	8,			727,	-1,	-1,	9,	0),		// SCROLL DOWN
		ss_buttons("2_ssb_39",	913,		556,	-1,	-1,	39,0),		// RESET
		ss_buttons("2_ssb_39",	0,			0,		-1,	-1,	99,0)			// dummy for drag n' drop
	}
};

// ship select text
#define SHIP_SELECT_NUM_TEXT			1
UI_XSTR Ship_select_text[GR_NUM_RESOLUTIONS][SHIP_SELECT_NUM_TEXT] = {
	{ // GR_640
		{ "Reset",			1337,		580,	337,	UI_XSTR_COLOR_GREEN, -1, &Ship_select_buttons[0][SS_BUTTON_RESET].button }
	}, 
	{ // GR_1024
		{ "Reset",			1337,		938,	546,	UI_XSTR_COLOR_GREEN, -1, &Ship_select_buttons[1][SS_BUTTON_RESET].button }
	}
};

// Mask bitmap pointer and Mask bitmap_id
static bitmap*	ShipSelectMaskPtr;		// bitmap pointer to the ship select mask bitmap
static ubyte*	ShipSelectMaskData;		// pointer to actual bitmap data
static int		Shipselect_mask_w, Shipselect_mask_h;
static int		ShipSelectMaskBitmap;	// bitmap id of the ship select mask bitmap

static MENU_REGION	Region[NUM_SHIP_SELECT_REGIONS];
static int				Num_mask_regions;

//stuff for ht&l. vars and such
extern float View_zoom, Canv_h2, Canv_w2;
extern int Cmdline_nohtl;

//////////////////////////////////////////////////////
// Drag and Drop variables
//////////////////////////////////////////////////////
typedef struct ss_carry_icon_info
{
	int from_slot;		// slot index (0..MAX_WSS_SLOTS-1), -1 if carried from list
	int ship_class;	// ship class of carried icon 
	int from_x, from_y;
} ss_carry_icon_info;

ss_carry_icon_info Carried_ss_icon;

////////////////////////////////////////////////////////////////////
// Internal function prototypes
////////////////////////////////////////////////////////////////////

// render functions
void draw_ship_icons();
void draw_ship_icon_with_number(int screen_offset, int ship_class);
void start_ship_animation(int ship_class, int play_sound=0);

// pick-up 
int pick_from_ship_list(int screen_offset, int ship_class);
void pick_from_wing(int wb_num, int ws_num);

// ui related
void ship_select_button_do(int i);
void ship_select_common_init();
void ss_reset_selected_ship();
void ss_restore_loadout();
void maybe_change_selected_wing_ship(int wb_num, int ws_num);

// init functions
void ss_init_pool(team_data *pteam);
int create_wings();

// loading/unloading
void ss_unload_all_icons();
void ss_unload_all_anims();
void ss_init_units();
anim* ss_load_individual_animation(int ship_class);

// Carry icon functions
int	ss_icon_being_carried();
void	ss_reset_carried_icon();
void	ss_set_carried_icon(int from_slot, int ship_class);

#define SHIP_DESC_X	445
#define SHIP_DESC_Y	273

const char *ss_tooltip_handler(const char *str)
{
	if (Selected_ss_class < 0)
		return NULL;

	if (!stricmp(str, NOX("@ship_name"))) {
		return Ship_info[Selected_ss_class].name;

	} else if (!stricmp(str, NOX("@ship_type"))) {
		return Ship_info[Selected_ss_class].type_str;

	} else if (!stricmp(str, NOX("@ship_maneuverability"))) {
		return Ship_info[Selected_ss_class].maneuverability_str;

	} else if (!stricmp(str, NOX("@ship_armor"))) {
		return Ship_info[Selected_ss_class].armor_str;

	} else if (!stricmp(str, NOX("@ship_manufacturer"))) {
		return Ship_info[Selected_ss_class].manufacturer_str;

	} else if (!stricmp(str, NOX("@ship_desc"))) {
		char *str2;
		int x, y, w, h;

		str2 = Ship_info[Selected_ss_class].desc;
		if (str2 == NULL)
			return NULL;

		gr_get_string_size(&w, &h, str2);
		x = SHIP_DESC_X - w / 2;
		y = SHIP_DESC_Y - h / 2;

		gr_set_color_fast(&Color_black);
		gr_rect(x - 5, y - 5, w + 10, h + 10);

		gr_set_color_fast(&Color_bright_white);
		gr_string(x, y, str2);
		return str2;
	}

	return NULL;
}

// Is an icon being carried?
int ss_icon_being_carried()
{
	if ( Carried_ss_icon.ship_class >= 0 ) {
		return 1;
	}

	return 0;
}

// Clear out carried icon info
void ss_reset_carried_icon()
{
	Carried_ss_icon.from_slot = -1;
	Carried_ss_icon.ship_class = -1;
}

// return !0 if carried icon has moved from where it was picked up
int ss_carried_icon_moved()
{
	int mx, my;

	mouse_get_pos_unscaled( &mx, &my );
	if ( Carried_ss_icon.from_x != mx || Carried_ss_icon.from_y != my) {
		return 1;
	}

	return 0;
}

// Set carried icon data 
void ss_set_carried_icon(int from_slot, int ship_class)
{
	Carried_ss_icon.from_slot = from_slot;
	Carried_ss_icon.ship_class = ship_class;

	// Set the mouse to captured
	Ship_select_buttons[gr_screen.res][SS_BUTTON_DUMMY].button.capture_mouse();
}

// clear all active list items, and reset the flags inside the SS_active_items[] array
void clear_active_list()
{
	int i;
	for ( i = 0; i < Num_ship_classes; i++ ) { //DTP singleplayer ship choice fix 
	//for ( i = 0; i < MAX_WSS_SLOTS; i++ ) { 
		SS_active_items[i].flags = 0;
		SS_active_items[i].ship_class = -1;
	}
	list_init(&SS_active_head);

	SS_active_list_start = 0;
	SS_active_list_size = 0;
}


// get a free element from SS_active_items[]
ss_active_item *get_free_active_list_node()
{
	int i;
	for ( i = 0; i < Num_ship_classes; i++ ) { 
	//for ( i = 0; i < MAX_WSS_SLOTS; i++ ) { //DTP, ONLY MAX_WSS_SLOTS SHIPS ???
	if ( SS_active_items[i].flags == 0 ) {
			SS_active_items[i].flags |= SS_ACTIVE_ITEM_USED;
			return &SS_active_items[i];
		}
	}
	return NULL;
}


// add a ship into the active list
void active_list_add(int ship_class)
{
	ss_active_item *sai;

	sai = get_free_active_list_node();
	Assert(sai != NULL);
	sai->ship_class = ship_class;
	list_append(&SS_active_head, sai);
}

// remove a ship from the active list
void active_list_remove(int ship_class)
{
	ss_active_item *sai, *temp;
	
	// next store players not assigned to wings
	sai = GET_FIRST(&SS_active_head);

	while(sai != END_OF_LIST(&SS_active_head)){
		temp = GET_NEXT(sai);
		if ( sai->ship_class == ship_class ) {
			list_remove(&SS_active_head, sai);
			sai->flags = 0;
		}
		sai = temp;
	}
}
	
// Build up the ship selection active list, which is a list of all ships that the player
// can choose from.
void init_active_list()
{
	int i;
	ss_active_item	*sai;

	Assert( Ss_pool != NULL );

	clear_active_list();

	// build the active list
	for ( i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		if ( Ss_pool[i] > 0 ) {
			sai = get_free_active_list_node();
			if ( sai != NULL ) {
				sai->ship_class = i;
				list_append(&SS_active_head, sai);
				SS_active_list_size++;
			}
		}
	}
}

void ship_select_check_buttons()
{
	int			i;
	ss_buttons	*b;

	for ( i = 0; i < NUM_SS_BUTTONS; i++ ) {
		b = &Ship_select_buttons[gr_screen.res][i];
		if ( b->button.pressed() ) {
			ship_select_button_do(b->hotspot);
		}
	}
}

// reset the ship selection to the mission defaults
void ss_reset_to_default()
{
	if ( Game_mode & GM_MULTIPLAYER ) {
		Int3();
		return;
	}

	ss_init_pool(&Team_data[Common_team]);
	ss_init_units();
	init_active_list();
	ss_reset_selected_ship();
	ss_reset_carried_icon();

	// reset weapons 
	wl_reset_to_defaults();

	start_ship_animation(Selected_ss_class, 1);
}

// -------------------------------------------------------------------
// ship_select_redraw_pressed_buttons()
//
// Redraw any ship select buttons that are pressed down.  This function is needed
// since we sometimes need to draw pressed buttons last to ensure the entire
// button gets drawn (and not overlapped by other buttons)
//
void ship_select_redraw_pressed_buttons()
{
	int			i;
	ss_buttons	*b;
	
	common_redraw_pressed_buttons();

	for ( i = 0; i < NUM_SS_BUTTONS; i++ ) {
		b = &Ship_select_buttons[gr_screen.res][i];
		if ( b->button.pressed() ) {
			b->button.draw_forced(2);
		}
	}
}

void ship_select_buttons_init()
{
	ss_buttons	*b;
	int			i;

	for ( i = 0; i < NUM_SS_BUTTONS; i++ ) {
		b = &Ship_select_buttons[gr_screen.res][i];
		b->button.create( &Ship_select_ui_window, "", b->x, b->y, 60, 30, b->scrollable);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action( common_play_highlight_sound );
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// add all xstrs
	for(i=0; i<SHIP_SELECT_NUM_TEXT; i++){
		Ship_select_ui_window.add_XSTR(&Ship_select_text[gr_screen.res][i]);
	}

	// We don't want to have the reset button appear in multiplayer
	if ( Game_mode & GM_MULTIPLAYER ) {
		Ship_select_buttons[gr_screen.res][SS_BUTTON_RESET].button.disable();
		Ship_select_buttons[gr_screen.res][SS_BUTTON_RESET].button.hide();
	}

	Ship_select_buttons[gr_screen.res][SS_BUTTON_DUMMY].button.disable();
	Ship_select_buttons[gr_screen.res][SS_BUTTON_DUMMY].button.hide();
}

// -------------------------------------------------------------------------------------
// ship_select_button_do() do the button action for the specified pressed button
//
void ship_select_button_do(int i)
{
	if ( Background_playing )
		return;

	switch ( i ) {
		case SHIP_SELECT_SHIP_SCROLL_UP:
			if ( Current_screen != ON_SHIP_SELECT )
				break;

			if ( common_scroll_down_pressed(&SS_active_list_start, SS_active_list_size, MAX_ICONS_ON_SCREEN) ) {
				gamesnd_play_iface(SND_SCROLL);
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
			break;

		case SHIP_SELECT_SHIP_SCROLL_DOWN:
			if ( Current_screen != ON_SHIP_SELECT )
				break;

			if ( common_scroll_up_pressed(&SS_active_list_start, SS_active_list_size, MAX_ICONS_ON_SCREEN) ) {
				gamesnd_play_iface(SND_SCROLL);
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}

			break;

		case SHIP_SELECT_RESET:
			ss_reset_to_default();
			break;
	} // end switch
}

// ---------------------------------------------------------------------
// ship_select_init() is called once when the ship select screen begins
//
//
void ship_select_init()
{
//	SS_active_items = new ss_active_item[Num_ship_classes];

	common_set_interface_palette("ShipPalette");
	common_flash_button_init();

	// if in multiplayer -- set my state to be ship select
	if ( Game_mode & GM_MULTIPLAYER ){		
		// also set the ship which is mine as the default
		maybe_change_selected_wing_ship(Net_player->p_info.ship_index/MAX_WING_SLOTS,Net_player->p_info.ship_index%MAX_WING_SLOTS);
	}

	set_active_ui(&Ship_select_ui_window);
	Current_screen = ON_SHIP_SELECT;

	Ss_mouse_down_on_region = -1;

	help_overlay_set_state(SS_OVERLAY,0);

	if ( Ship_select_open ) {
		//reset the animation
		Ship_anim_class = -1;
		start_ship_animation( Selected_ss_class );
		common_buttons_maybe_reload(&Ship_select_ui_window);	// AL 11-21-97: this is necessary since we may returning from the hotkey
																				// screen, which can release common button bitmaps.
		common_reset_buttons();
		nprintf(("Alan","ship_select_init() returning without doing anything\n"));
		return;
	}

	nprintf(("Alan","entering ship_select_init()\n"));
	common_select_init();

	ShipSelectMaskBitmap = bm_load(Ship_select_background_mask_fname[gr_screen.res]);
	if (ShipSelectMaskBitmap < 0) {
		if (gr_screen.res == GR_640) {
			Error(LOCATION,"Could not load in 'shipselect-m'!");
		} else if (gr_screen.res == GR_1024) {
			Error(LOCATION,"Could not load in '2_shipselect-m'!");
		}
	}

	Shipselect_mask_w = -1;
	Shipselect_mask_h = -1;

	// get a pointer to bitmap by using bm_lock()
	ShipSelectMaskPtr = bm_lock(ShipSelectMaskBitmap, 8, BMP_AABITMAP);
	ShipSelectMaskData = (ubyte*)ShipSelectMaskPtr->data;	
	bm_get_info(ShipSelectMaskBitmap, &Shipselect_mask_w, &Shipselect_mask_h);

	help_overlay_load(SS_OVERLAY);

	// Set up the mask regions
   // initialize the different regions of the menu that will react when the mouse moves over it
	Num_mask_regions = 0;
	
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	COMMON_BRIEFING_REGION,				0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	COMMON_SS_REGION,						0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	COMMON_WEAPON_REGION,				0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	COMMON_COMMIT_REGION,				0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	COMMON_HELP_REGION,					0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	COMMON_OPTIONS_REGION,				0);

	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	SHIP_SELECT_SHIP_SCROLL_UP,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	SHIP_SELECT_SHIP_SCROLL_DOWN,		0);

	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	SHIP_SELECT_ICON_0,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	SHIP_SELECT_ICON_1,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	SHIP_SELECT_ICON_2,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	SHIP_SELECT_ICON_3,		0);

	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_0_SHIP_0,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_0_SHIP_1,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_0_SHIP_2,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_0_SHIP_3,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_1_SHIP_0,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_1_SHIP_1,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_1_SHIP_2,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_1_SHIP_3,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_2_SHIP_0,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_2_SHIP_1,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_2_SHIP_2,		0);
	snazzy_menu_add_region(&Region[Num_mask_regions++], "",	WING_2_SHIP_3,		0);

	Ship_select_open = 1;	// This game-wide global flag is set to 1 to indicate that the ship
									// select screen has been opened and memory allocated.  This flag
									// is needed so we can know if ship_select_close() needs to called if
									// restoring a game from the Options screen invoked from ship select

	// init ship selection masks and buttons
	Ship_select_ui_window.create( 0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0 );
	Ship_select_ui_window.set_mask_bmap(Ship_select_background_mask_fname[gr_screen.res]);
	Ship_select_ui_window.tooltip_handler = ss_tooltip_handler;
	common_buttons_init(&Ship_select_ui_window);
	ship_select_buttons_init();

	// init ship selection background bitmap
	Ship_select_background_bitmap = bm_load(Ship_select_background_fname[gr_screen.res]);

	// init ship selection ship model rendering window
	start_ship_animation( Selected_ss_class );
}


// Return the ship class for the icon specified by index.  Need to iterate through the active
// list of icons to find out what ship class for this icon
//
// input: index => list index (0..3)
// exit:  ship class, -1 if none
//
int ss_get_ship_class_from_list(int index)
{
	ss_active_item	*sai;
	int				list_entry, i, count;

	i = 0;
	count = 0;
	list_entry = -1;
	for ( sai = GET_FIRST(&SS_active_head); sai != END_OF_LIST(&SS_active_head); sai = GET_NEXT(sai) ) {
		count++;
		if ( count <= SS_active_list_start )
			continue;

		if ( i >= MAX_ICONS_ON_SCREEN )
			break;

		if ( i == index ) {
			list_entry = sai->ship_class;
			break;
		}

		i++;
	}

	return list_entry;
}

// ---------------------------------------------------------------------
// maybe_pick_up_list_icon()
//
void maybe_pick_up_list_icon(int offset)
{
	int ship_class;

	ship_class = ss_get_ship_class_from_list(offset);
	if ( ship_class != -1 ) {
		pick_from_ship_list(offset, ship_class);
	}
}

// ---------------------------------------------------------------------
// maybe_change_selected_ship()
//
void maybe_change_selected_ship(int offset)
{
	int ship_class;

	ship_class = ss_get_ship_class_from_list(offset);
	if ( ship_class == -1 )
		return;

	if ( Ss_mouse_down_on_region != (SHIP_SELECT_ICON_0+offset) ) {
		return;
	}

	if ( Selected_ss_class == -1 ) {
		Selected_ss_class = ship_class;
		start_ship_animation(Selected_ss_class, 1);
	}
	else if ( Selected_ss_class != ship_class ) {
		Selected_ss_class = ship_class;
		start_ship_animation(Selected_ss_class, 1);
	}
	else
		Assert( Selected_ss_class == ship_class );
}

void maybe_change_selected_wing_ship(int wb_num, int ws_num)
{
	Assert(wb_num >= 0 && wb_num < MAX_WING_BLOCKS);
	Assert(ws_num >= 0 && ws_num < MAX_WING_SLOTS);	
	Assert( (Ss_wings != NULL) && (Wss_slots != NULL) );
	
	if ( Ss_wings[wb_num].wingnum < 0 ) {
		return;
	}

	if ( Selected_ss_class != -1 && Selected_ss_class != Wss_slots[wb_num*MAX_WING_SLOTS+ws_num].ship_class ) {
		Selected_ss_class = Wss_slots[wb_num*MAX_WING_SLOTS+ws_num].ship_class;
		start_ship_animation(Selected_ss_class, 1);
	}
}

// ---------------------------------------------------------------------
// do_mouse_over_wing_slot()
//
// returns:	0 => icon wasn't dropped onto slot
//				1 => icon was dropped onto slot
int do_mouse_over_wing_slot(int block, int slot)
{
	Hot_ss_slot = block*MAX_WING_SLOTS + slot;

	if ( !mouse_down(MOUSE_LEFT_BUTTON) ) {
		if ( ss_icon_being_carried() ) {

			if ( ss_disabled_slot(block*MAX_WING_SLOTS+slot) ) {
				gamesnd_play_iface(SND_ICON_DROP);
				return 0;
			}

			if ( !ss_carried_icon_moved() ) {
				ss_reset_carried_icon();
				return 0;
			}

			ss_drop(Carried_ss_icon.from_slot, Carried_ss_icon.ship_class, Hot_ss_slot, -1);
			ss_reset_carried_icon();
		}
	}
	else {
		if ( Ss_mouse_down_on_region == (WING_0_SHIP_0+block*MAX_WING_SLOTS+slot) ) {
			pick_from_wing(block, slot);
		}
	}

	return 1;
}

void do_mouse_over_list_slot(int index)
{
	Hot_ss_icon = index;

	if ( Ss_mouse_down_on_region != (SHIP_SELECT_ICON_0+index) ){
		return;
	}

	if ( mouse_down(MOUSE_LEFT_BUTTON) )
		maybe_pick_up_list_icon(index);
}

// Icon has been dropped, but not onto a wing slot
void ss_maybe_drop_icon()
{
	if ( Drop_icon_mflag )  {
		if ( ss_icon_being_carried() ) {
			// Add back into the ship entry list
			if ( Carried_ss_icon.from_slot >= 0 ) {
				// return to list
				ss_drop(Carried_ss_icon.from_slot, -1, -1, Carried_ss_icon.ship_class);
			} else {
				if ( ss_carried_icon_moved() ) {
					gamesnd_play_iface(SND_ICON_DROP);
				}
			}
			ss_reset_carried_icon();
		}	
	}
}

// maybe flash a button if player hasn't done anything for a while
void ss_maybe_flash_button()
{
	if ( common_flash_bright() ) {
		// weapon loadout button
		if ( Common_buttons[Current_screen-1][gr_screen.res][2].button.button_hilighted() ) {
			common_flash_button_init();
		} else {
			Common_buttons[Current_screen-1][gr_screen.res][2].button.draw_forced(1);
		}
	}
}

// blit any active ship information text
void ship_select_blit_ship_info()
{
	int y_start;
	ship_info *sip;
	char str[100];
	color *header = &Color_white;
	color *text = &Color_green;


	// if we don't have a valid ship selected, do nothing
	if(Selected_ss_class == -1){
		return;
	}

	// get the ship class
	sip = &Ship_info[Selected_ss_class];

	// starting line
	y_start = Ship_info_coords[gr_screen.res][SHIP_SELECT_Y_COORD];

	memset(str,0,100);

	// blit the ship class (name)
	gr_set_color_fast(header);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Class",739));
	y_start += 10;
	if(strlen((sip->alt_name[0]) ? sip->alt_name : sip->name)){
		gr_set_color_fast(text);

		// Goober5000
		char temp[NAME_LENGTH];
		strcpy_s(temp, (sip->alt_name[0]) ? sip->alt_name : sip->name);
		end_string_at_first_hash_symbol(temp);

		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, temp);
	}
	y_start += 10;

	// blit the ship type
	gr_set_color_fast(header);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Type",740));
	y_start += 10;
	gr_set_color_fast(text);
	if((sip->type_str != NULL) && strlen(sip->type_str)){
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, sip->type_str);
	}
	else
	{
		ship_get_type(str, sip);
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, str);
	}
	y_start+=10;

	// blit the ship length
	gr_set_color_fast(header);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Length",741));
	y_start += 10;
	gr_set_color_fast(text);
	if((sip->ship_length != NULL) && strlen(sip->ship_length)){
		if (Lcl_gr) {
			// in german, drop the s from Meters and make sure M is caps
			char *sp = strstr(sip->ship_length, "Meters");
			if (sp) {
				sp[5] = ' ';		// make the old s a space now
			}
		}
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, sip->ship_length);
	}
	else if(ShipSelectModelNum >= 0)
	{
		polymodel *pm = model_get(ShipSelectModelNum);
		sprintf( str, "%d", fl2i(pm->maxs.xyz.z - pm->mins.xyz.z) );
		strcat_s(str, " M");
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, str);
	}
	else
	{
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, "Unknown");
	}
	y_start += 10;

	// blit the max velocity
	gr_set_color_fast(header);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Max Velocity",742));	
	y_start += 10;
	sprintf(str, XSTR("%d m/s",743),fl2i((float)sip->max_vel.xyz.z * Hud_speed_multiplier));
	gr_set_color_fast(text);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start,str);	
	y_start += 10;

	// blit the maneuverability
	gr_set_color_fast(header);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Maneuverability",744));
	y_start += 10;
	gr_set_color_fast(text);
	if((sip->maneuverability_str != NULL) && strlen(sip->maneuverability_str)){
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, sip->maneuverability_str);
	}
	else if(ShipSelectModelNum >= 0)
	{
		int sum = fl2i(sip->rotation_time.xyz.x + sip->rotation_time.xyz.y);
		if(sum <= 6)
			strcpy_s(str, "Excellent");
		else if(sum < 7)
			strcpy_s(str, "High");
		else if(sum < 8)
			strcpy_s(str, "Good");
		else if(sum < 9)
			strcpy_s(str, "Average");
		else if(sum < 10)
			strcpy_s(str, "Poor");
		else if(sum < 15)
			strcpy_s(str, "Very Poor");
		else
			strcpy_s(str, "Extremely Poor");

		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, str);
	}
	else
	{
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, "Unknown");
	}
	y_start += 10;

	// blit the armor
	gr_set_color_fast(header);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Armor",745));
	y_start += 10;
	gr_set_color_fast(text);
	if((sip->armor_str != NULL) && strlen(sip->armor_str)){
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, sip->armor_str);
	}
	else
	{
		int sum = fl2i(sip->max_hull_strength + sip->max_shield_strength);
		if(sum <= 600)
			strcpy_s(str, "Light");
		else if(sum <= 700)
			strcpy_s(str, "Average");
		else if(sum <= 900)
			strcpy_s(str, "Medium");
		else if(sum <= 1100)
			strcpy_s(str,	"Heavy");
		else if(sum <= 1300)
			strcpy_s(str, "Very Heavy");
		else if(sum <= 2000)
			strcpy_s(str, "Ultra Heavy");
		else if(sum <= 30000)
			strcpy_s(str, "Light Capital");
		else if(sum <= 75000)
			strcpy_s(str, "Medium Capital");
		else if(sum <= 200000)
			strcpy_s(str, "Heavy Capital");
		else if(sum <= 800000)
			strcpy_s(str, "Very Heavy Capital");
		else
			strcpy_s(str, "Ultra Heavy Capital");
			

		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start,str);
	}
	y_start += 10;

	// blit the gun mounts 
	gr_set_color_fast(header);
	if((sip->gun_mounts != NULL) && strlen(sip->gun_mounts))
	{
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Gun Mounts",746));
		y_start += 10;
		gr_set_color_fast(text);
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, sip->gun_mounts);
	}
	else if(ShipSelectModelNum >= 0)
	{
		//Calculate the number of gun mounts
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Gun Mounts",746));
		y_start += 10;
		gr_set_color_fast(text);
		int i;
		int sum = 0;
		polymodel *pm = model_get(ShipSelectModelNum);
		for(i = 0; i < pm->n_guns; i++)
		{
			sum += pm->gun_banks[i].num_slots;
		}
		if(sum != 0)
			sprintf(str, "%d", sum);
		else
			strcpy_s(str, "None");
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, str);
	}
	else
	{
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Gun Banks",1626));
		y_start += 10;
		gr_set_color_fast(text);
		if(sip->num_primary_banks)
		{
			sprintf(str, "%d", sip->num_primary_banks);
		}
		else
		{
			strcpy_s(str, "None");
		}
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, str);
	}
	y_start += 10;

	// blit the missile banks
	gr_set_color_fast(header);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Missile Banks",747));
	y_start += 10;
	gr_set_color_fast(text);
	if((sip->missile_banks != NULL) && strlen(sip->missile_banks)){
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, sip->missile_banks);
	}
	else
	{
		if(sip->num_secondary_banks)
		{
			sprintf(str, "%d", sip->num_secondary_banks);
		}
		else
		{
			strcpy_s(str, "None");
		}
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, str);
	}
	y_start += 10;

	if(ShipSelectModelNum >= 0)
	{
		int num_turrets = 0;
		int x;
		for(x = 0; x < sip->n_subsystems; x++)
		{
			if(sip->subsystems[x].type == SUBSYSTEM_TURRET)
				num_turrets++;
			/*
			for(y = 0; y < MAX_SHIP_PRIMARY_BANKS || y < MAX_SHIP_SECONDARY_BANKS; y++)
			{
				if(y < MAX_SHIP_PRIMARY_BANKS)
				{
					if(sip->subsystems[x].primary_banks[y] != -1)
					{
						num_turrets++;
						break;
					}
				}
				
				if(y < MAX_SHIP_SECONDARY_BANKS)
				{
					if(sip->subsystems[x].secondary_banks[y] != -1)
					{
						num_turrets++;
						break;
					}
				}
			}
			*/
		}
		if(num_turrets)
		{
			gr_set_color_fast(header);
			gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Turrets",1627));
			y_start += 10;
			gr_set_color_fast(text);
			sprintf(str, "%d", num_turrets);
			gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, str);
			y_start += 10;
		}
	}

	// blit the manufacturer
	gr_set_color_fast(header);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Manufacturer",748));
	y_start += 10;
	gr_set_color_fast(text);
	if((sip->manufacturer_str != NULL) && strlen(sip->manufacturer_str)){
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, sip->manufacturer_str);
	}
	else
	{
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, Species_info[sip->species].species_name);
	}
	y_start += 10;

	// blit the _short_ text description, if it exists
	// split the text info up	
	
	if (sip->desc == NULL)
		return;

	gr_set_color_fast(header);
	gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD], y_start, XSTR("Description",1571));
	y_start += 10;

	Assert(strlen(sip->desc));

	int n_lines;
	int n_chars[MAX_BRIEF_LINES];
	char ship_desc[1000];
	const char *p_str[MAX_BRIEF_LINES];
	char *token;
	char Ship_select_ship_info_text[1500];
	char Ship_select_ship_info_lines[MAX_NUM_SHIP_DESC_LINES][SHIP_SELECT_SHIP_INFO_MAX_LINE_LEN];
	int Ship_select_ship_info_line_count;

	// strip out newlines
	memset(ship_desc,0,1000);
	memset(Ship_select_ship_info_text,0,1500);
	strcpy_s(ship_desc, sip->desc);
	token = strtok(ship_desc,"\n");
	if(token != NULL){
		strcpy_s(Ship_select_ship_info_text, token);
		while(token != NULL){
			token = strtok(NULL,"\n");
			if(token != NULL){
				strcat_s(Ship_select_ship_info_text," ");
				strcat_s(Ship_select_ship_info_text,token);
			}
		}
	}
	
	if(Ship_select_ship_info_text[0] != '\0'){
		// split the string into multiple lines
		n_lines = split_str(Ship_select_ship_info_text, gr_screen.res == GR_640 ? 128 : 350, n_chars, p_str, MAX_NUM_SHIP_DESC_LINES, 0);

		// copy the split up lines into the text lines array
		for (int idx = 0;idx<n_lines;idx++ ) {
			Assert(n_chars[idx] < SHIP_SELECT_SHIP_INFO_MAX_LINE_LEN);
			strncpy(Ship_select_ship_info_lines[idx], p_str[idx], n_chars[idx]);
			Ship_select_ship_info_lines[idx][n_chars[idx]] = 0;
			drop_leading_white_space(Ship_select_ship_info_lines[idx]);		
		}

		// get the line count
		Ship_select_ship_info_line_count = n_lines;
	} else {
		// set the line count to 
		Ship_select_ship_info_line_count = 0;
	}	
	
	Assert(Ship_select_ship_info_line_count < MAX_NUM_SHIP_DESC_LINES);
	gr_set_color_fast(text);
	for(int idx=0;idx<Ship_select_ship_info_line_count;idx++){
		gr_string(Ship_info_coords[gr_screen.res][SHIP_SELECT_X_COORD]+4, y_start, Ship_select_ship_info_lines[idx]);
		y_start += 10;
	}
	
}


// ---------------------------------------------------------------------
// ship_select_do() is called once per game frame, and is responsible for
// updating the ship select screen
//
//	frametime is in seconds

extern int Tech_ship_display_coords[GR_NUM_RESOLUTIONS][4];

void ship_select_do(float frametime)
{
	int k, ship_select_choice, snazzy_action;

	ship_select_choice = snazzy_menu_do(ShipSelectMaskData, Shipselect_mask_w, Shipselect_mask_h, Num_mask_regions, Region, &snazzy_action, 0);

	Hot_ss_icon = -1;
	Hot_ss_slot = -1;

	k = common_select_do(frametime);

	// Check common keypresses
	common_check_keys(k);

	if ( Mouse_down_last_frame ) {
		Ss_mouse_down_on_region = ship_select_choice;
	}

	// Check for the mouse over a region (not clicked, just over)
	if ( ship_select_choice > -1 ) {

		switch(ship_select_choice) {
			case SHIP_SELECT_ICON_0:
				do_mouse_over_list_slot(0);
				break;
			case SHIP_SELECT_ICON_1:
				do_mouse_over_list_slot(1);
				break;
			case SHIP_SELECT_ICON_2:
				do_mouse_over_list_slot(2);
				break;
			case SHIP_SELECT_ICON_3:
				do_mouse_over_list_slot(3);
				break;
			case WING_0_SHIP_0:
				if ( do_mouse_over_wing_slot(0,0) )
					ship_select_choice = -1;
				break;
			case WING_0_SHIP_1:
				if ( do_mouse_over_wing_slot(0,1) )
					ship_select_choice = -1;
				break;
			case WING_0_SHIP_2:
				if ( do_mouse_over_wing_slot(0,2) )
					ship_select_choice = -1;
				break;
			case WING_0_SHIP_3:
				if ( do_mouse_over_wing_slot(0,3) )
					ship_select_choice = -1;
				break;
			case WING_1_SHIP_0:
				if ( do_mouse_over_wing_slot(1,0) )
					ship_select_choice = -1;
				break;
			case WING_1_SHIP_1:
				if ( do_mouse_over_wing_slot(1,1) )
					ship_select_choice = -1;
				break;
			case WING_1_SHIP_2:
				if ( do_mouse_over_wing_slot(1,2) )
					ship_select_choice = -1;
				break;
			case WING_1_SHIP_3:
				if ( do_mouse_over_wing_slot(1,3) )
					ship_select_choice = -1;
				break;
			case WING_2_SHIP_0:
				if ( do_mouse_over_wing_slot(2,0) )
					ship_select_choice = -1;
				break;
			case WING_2_SHIP_1:
				if ( do_mouse_over_wing_slot(2,1) )
					ship_select_choice = -1;
				break;
			case WING_2_SHIP_2:
				if ( do_mouse_over_wing_slot(2,2) )
					ship_select_choice = -1;
				break;
			case WING_2_SHIP_3:
				if ( do_mouse_over_wing_slot(2,3) )
					ship_select_choice = -1;
				break;

			default:
				break;
		}	// end switch
	}

	// check buttons
	common_check_buttons();
	ship_select_check_buttons();

	// Check for the mouse clicks over a region
	if ( ship_select_choice > -1 && snazzy_action == SNAZZY_CLICKED ) {
		switch (ship_select_choice) {

			case SHIP_SELECT_ICON_0:
				maybe_change_selected_ship(0);
				break;

			case SHIP_SELECT_ICON_1:
				maybe_change_selected_ship(1);
				break;

			case SHIP_SELECT_ICON_2:
				maybe_change_selected_ship(2);
				break;

			case SHIP_SELECT_ICON_3:
				maybe_change_selected_ship(3);
				break;

			case WING_0_SHIP_0:
				maybe_change_selected_wing_ship(0,0);
				break;

			case WING_0_SHIP_1:
				maybe_change_selected_wing_ship(0,1);
				break;

			case WING_0_SHIP_2:
				maybe_change_selected_wing_ship(0,2);
				break;

			case WING_0_SHIP_3:
				maybe_change_selected_wing_ship(0,3);
				break;

			case WING_1_SHIP_0:
				maybe_change_selected_wing_ship(1,0);
				break;

			case WING_1_SHIP_1:
				maybe_change_selected_wing_ship(1,1);
				break;

			case WING_1_SHIP_2:
				maybe_change_selected_wing_ship(1,2);
				break;

			case WING_1_SHIP_3:
				maybe_change_selected_wing_ship(1,3);
				break;

			case WING_2_SHIP_0:
				maybe_change_selected_wing_ship(2,0);
				break;

			case WING_2_SHIP_1:
				maybe_change_selected_wing_ship(2,1);
				break;

			case WING_2_SHIP_2:
				maybe_change_selected_wing_ship(2,2);
				break;

			case WING_2_SHIP_3:
				maybe_change_selected_wing_ship(2,3);
				break;

			default:
				break;

		}	// end switch
	}

	ss_maybe_drop_icon();

	Assert( Ss_icons != NULL );

	if ( !Background_playing ) {		
		gr_set_bitmap(Ship_select_background_bitmap);
		gr_bitmap(0, 0);
		Ship_select_ui_window.draw();
		ship_select_redraw_pressed_buttons();
		common_render_selected_screen_button();
	}
	if(!Cmdline_ship_choice_3d)
	{
		if ( (Selected_ss_class >= 0) && (Ss_icons[Selected_ss_class].ss_anim.num_frames > 0) ) {
			generic_anim_render(&Ss_icons[Selected_ss_class].ss_anim, (help_overlay_active(SS_OVERLAY)) ? 0 : frametime, Ship_anim_coords[gr_screen.res][0], Ship_anim_coords[gr_screen.res][1]);
		}
	}

	// The background transition plays once. Display ship icons after Background done playing
	if ( !Background_playing ) {
		draw_ship_icons();
		for ( int i = 0; i < MAX_WING_BLOCKS; i++ ) {
			draw_wing_block(i, Hot_ss_slot, -1, Selected_ss_class);
		}		
	}
	
	if ( ss_icon_being_carried() ) {
		int mouse_x, mouse_y, sx, sy;
		mouse_get_pos_unscaled( &mouse_x, &mouse_y );
		sx = mouse_x + Ss_delta_x;
		sy = mouse_y + Ss_delta_y;
		if(Ss_icons[Carried_ss_icon.ship_class].icon_bmaps[ICON_FRAME_SELECTED] != -1)
		{
			gr_set_bitmap(Ss_icons[Carried_ss_icon.ship_class].icon_bmaps[ICON_FRAME_SELECTED]);
			gr_bitmap(sx, sy);
		}
		else
		{
			ship_info *sip = &Ship_info[Carried_ss_icon.ship_class];
			if(Ss_icons[Carried_ss_icon.ship_class].model_index == -1) {
				Ss_icons[Carried_ss_icon.ship_class].model_index = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
				mprintf(("SL WARNING: Had to attempt to page in model for %s paged in manually! Result: %d\n", sip->name, Ss_icons[Carried_ss_icon.ship_class].model_index));
			}
			gr_set_color_fast(&Icon_colors[ICON_FRAME_SELECTED]);
			//gr_set_shader(&Icon_shaders[ICON_FRAME_SELECTED]);

			int w = 32;
			int h = 28;

			if(Ss_icons[Carried_ss_icon.ship_class].model_index != -1)
			{
				draw_model_icon(Ss_icons[Carried_ss_icon.ship_class].model_index, MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING | MR_NO_LIGHTING, sip->closeup_zoom / 1.25f, sx, sy, w, h, sip);
			}
			draw_brackets_square(sx, sy, sx + w, sy + h);
			//gr_shade(mouse_x + Ss_delta_x, mouse_y + Ss_delta_y, 32, 28);
		}
	}

	// draw out ship information
	ship_select_blit_ship_info();


	ss_maybe_flash_button();

	// blit help overlay if active
	help_overlay_maybe_blit(SS_OVERLAY);

	// If the commit button was pressed, do the commit button actions.  Done at the end of the
	// loop so there isn't a skip in the animation (since ship_create() can take a long time if
	// the ship model is not in memory
	if ( Commit_pressed ) {		
		commit_pressed();		
		Commit_pressed = 0;
	}

	// The new rendering code for 3D ships courtesy your friendly UnknownPlayer :)

	//////////////////////////////////
	// Render and draw the 3D model //
	//////////////////////////////////
	if( Cmdline_ship_choice_3d || ( (Selected_ss_class >= 0) && (Ss_icons[Selected_ss_class].ss_anim.num_frames == 0)) )
	{
		// check we have a valid ship class selected
		if ( (Selected_ss_class >= 0) && (ShipSelectModelNum >= 0) )
		{
			ship_info *sip = &Ship_info[Selected_ss_class];
			float rev_rate = REVOLUTION_RATE;
			if (sip->flags & SIF_BIG_SHIP) {
				rev_rate *= 1.7f;
			}
			if (sip->flags & SIF_HUGE_SHIP) {
				rev_rate *= 3.0f;
			}

			if (sip->uses_team_colors) {
				gr_set_team_color(sip->default_team_name, "<none>", 0, 0);
			}

			draw_model_rotating(ShipSelectModelNum,
				Ship_anim_coords[gr_screen.res][0],
				Ship_anim_coords[gr_screen.res][1],
				Tech_ship_display_coords[gr_screen.res][2],
				Tech_ship_display_coords[gr_screen.res][3],
				&ShipSelectScreenShipRot,
				&sip->closeup_pos,
				sip->closeup_zoom * 1.3f,
				rev_rate,
				MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING,
				true,
				sip->selection_effect);

			gr_disable_team_color();
		}
	}

	gr_reset_clip();
	gr_flip();

	///////////////////////////////////
	// Done Rendering and Drawing 3D //
	///////////////////////////////////

	if ( Game_mode & GM_MULTIPLAYER ) {
		if ( Selected_ss_class >= 0 )
			Net_player->p_info.ship_class = Selected_ss_class;
	}	 

	// If the commit button was pressed, do the commit button actions.  Done at the end of the
	// loop so there isn't a skip in the animation (since ship_create() can take a long time if
	// the ship model is not in memory
	if ( Commit_pressed ) {		
		commit_pressed();		
		Commit_pressed = 0;
	}
}


// ------------------------------------------------------------------------
//	ship_select_close() is called once when the ship select screen is exited
//
//
void ship_select_close()
{
	key_flush();

	ship_select_common_close();

	if ( !Ship_select_open ) {
		nprintf(("Alan","ship_select_close() returning without doing anything\n"));
		return;
	}

	nprintf(("Alan", "Entering ship_select_close()\n"));

	// done with the bitmaps, so unlock it
	bm_unlock(ShipSelectMaskBitmap);

	// unload the bitmaps
	bm_release(ShipSelectMaskBitmap);
	help_overlay_unload(SS_OVERLAY);

	Ship_select_ui_window.destroy();

	Ship_anim_class = -1;
	Ship_select_open = 0;	// This game-wide global flag is set to 0 to indicate that the ship
									// select screen has been closed and memory freed.  This flag
									// is needed so we can know if ship_select_close() needs to called if
									// restoring a game from the Options screen invoked from ship select

//	delete[] SS_active_items;
}

//	ss_unload_icons() frees the bitmaps used for ship icons 
void ss_unload_all_icons()
{
	int					i,j;
	ss_icon_info		*icon;

	Assert( Ss_icons != NULL );

	for ( i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		icon = &Ss_icons[i];

		for ( j = 0; j < NUM_ICON_FRAMES; j++ ) {
			if ( icon->icon_bmaps[j] >= 0 ) {
				bm_release(icon->icon_bmaps[j]);
				icon->icon_bmaps[j] = -1;
			}
		}
	}
}

// ------------------------------------------------------------------------
//	draw_ship_icons() will request which icons to draw on screen.
void draw_ship_icons()
{
	int i;
	int count=0;

	ss_active_item	*sai;
	i = 0;
	for ( sai = GET_FIRST(&SS_active_head); sai != END_OF_LIST(&SS_active_head); sai = GET_NEXT(sai) ) {
		count++;
		if ( count <= SS_active_list_start )
			continue;

		if ( i >= MAX_ICONS_ON_SCREEN )
			break;

		draw_ship_icon_with_number(i, sai->ship_class);
		i++;
	}
}

// ------------------------------------------------------------------------
//	draw_ship_icon_with_number() will draw a ship icon on screen with the
// number of available ships to the left.
//
//
void draw_ship_icon_with_number(int screen_offset, int ship_class)
{
	char	buf[32];
	int	num_x,num_y;
	ss_icon_info *ss_icon;
	color *color_to_draw = NULL;
	//shader *shader_to_use;


	Assert( screen_offset >= 0 && screen_offset <= 3 );
	Assert( ship_class >= 0 );
	Assert( (Ss_pool != NULL) && (Ss_icons != NULL) );
	ss_icon = &Ss_icons[ship_class];

	num_x = Ship_list_coords[gr_screen.res][screen_offset][2];
	num_y = Ship_list_coords[gr_screen.res][screen_offset][3];
	
	// assume default bitmap is to be used
	if(ss_icon->current_icon_bitmap != -1)
		ss_icon->current_icon_bitmap = ss_icon->icon_bmaps[ICON_FRAME_NORMAL];
	else
		color_to_draw = &Icon_colors[ICON_FRAME_NORMAL];

	// next check if ship has mouse over it
	if ( Hot_ss_icon > -1 ) {
		Assert(Hot_ss_icon <= 3);
		if ( Hot_ss_icon == screen_offset )
		{
			if(ss_icon->model_index == -1)
				ss_icon->current_icon_bitmap = ss_icon->icon_bmaps[ICON_FRAME_HOT];
			else
				color_to_draw = &Icon_colors[ICON_FRAME_HOT];
		}
	}

	// highest precedence is if the ship is selected
	if ( Selected_ss_class > -1 ) {
		if ( Selected_ss_class == ship_class )
		{
			if(ss_icon->model_index == -1)
				ss_icon->current_icon_bitmap = ss_icon->icon_bmaps[ICON_FRAME_SELECTED];
			else
				color_to_draw = &Icon_colors[ICON_FRAME_SELECTED];
		}
	}

	if ( Ss_pool[ship_class] <= 0 ) {
		return;
	}

	// blit the icon
	if(ss_icon->current_icon_bitmap != -1)
	{
		gr_set_bitmap(ss_icon->current_icon_bitmap);
		gr_bitmap(Ship_list_coords[gr_screen.res][screen_offset][0], Ship_list_coords[gr_screen.res][screen_offset][1]);
	}
	else
	{
		ship_info *sip = &Ship_info[ship_class];
		if(ss_icon->model_index == -1) {
			ss_icon->model_index = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
			mprintf(("SL WARNING: Had to attempt to page in model for %s paged in manually! Result: %d\n", sip->name, ss_icon->model_index));
		}
		gr_set_color_fast(color_to_draw);
		//gr_set_shader(shader_to_use);
		if(ss_icon->model_index != -1)
		{
			draw_model_icon(ss_icon->model_index, MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING | MR_NO_LIGHTING, sip->closeup_zoom / 1.25f, Ship_list_coords[gr_screen.res][screen_offset][0],Ship_list_coords[gr_screen.res][screen_offset][1], 32, 28, sip);
		}
		draw_brackets_square(Ship_list_coords[gr_screen.res][screen_offset][0], Ship_list_coords[gr_screen.res][screen_offset][1], Ship_list_coords[gr_screen.res][screen_offset][0] + 32, Ship_list_coords[gr_screen.res][screen_offset][1] + 28);
		//gr_shade(Ship_list_coords[gr_screen.res][screen_offset][0],Ship_list_coords[gr_screen.res][screen_offset][1], 32, 28);
	}

	// blit the number
	sprintf(buf, "%d", Ss_pool[ship_class] );
	gr_set_color_fast(&Color_white);
	gr_string(num_x, num_y, buf);
}

// ------------------------------------------------------------------------
// this loads an individual animation file
// it attempts to load a hires version (ie, it attaches a "2_" in front of the
// filename. if no hires version is available, it defaults to the lowres

// ------------------------------------------------------------------------
//	start_ship_animation() will start a ship animation playing, and will 
// load the compressed anim from disk if required.
//
// UPDATE: this code now initializes a 3d model of a ship to spin like it does
// in the tech room - UnknownPlayer
void start_ship_animation(int ship_class, int play_sound)
{
	ship_info *sip = &Ship_info[ship_class];
	char *p;
	char animation_filename[CF_MAX_FILENAME_LENGTH+4];

	anim_timer_start = timer_get_milliseconds();

	if ( Cmdline_ship_choice_3d || !strlen(sip->anim_filename) ) {
		if (ship_class < 0) {
			mprintf(("No ship class passed in to start_ship_animation\n"));
			ShipSelectModelNum = -1;
			return;
		}

		//Unload Anim if one was playing
		if(Ship_anim_class > 0 && Ss_icons[Ship_anim_class].ss_anim.num_frames > 0) {
			generic_anim_unload(&Ss_icons[Ship_anim_class].ss_anim);
			Ship_anim_class = -1;
		}

		// Load the necessary model file
		ShipSelectModelNum = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
		
		// page in ship textures properly (takes care of nondimming pixels)
		model_page_in_textures(ShipSelectModelNum, ship_class);
		
		if (sip->model_num < 0) {
			mprintf(("Couldn't load model file %s in missionshipchoice.cpp\n", sip->pof_file));
		}
	} else {
		Assert( ship_class >= 0 );
		Assert( Ss_icons != NULL );
		
		if (Ship_anim_class == ship_class) {
			return;
		}

		//If there was a model loaded for the previous ship, unload it
		if (ShipSelectModelNum >= 0 ) {
			model_unload(ShipSelectModelNum);
			ShipSelectModelNum = -1;
		}

		//unload the previous anim
		if(Ship_anim_class > 0 && Ss_icons[Ship_anim_class].ss_anim.num_frames > 0)
			generic_anim_unload(&Ss_icons[Ship_anim_class].ss_anim);
		//load animation here, we now only have one loaded
		p = strchr(Ship_info[ship_class].anim_filename, '.' );
		if(p)
			*p = '\0';
		if (gr_screen.res == GR_1024) {
			strcpy_s(animation_filename, "2_");
			strcat_s(animation_filename, Ship_info[ship_class].anim_filename);
		}
		else {
			strcpy_s(animation_filename, Ship_info[ship_class].anim_filename);
		}

		generic_anim_init(&Ss_icons[ship_class].ss_anim, animation_filename);
		Ss_icons[ship_class].ss_anim.ani.bg_type = bm_get_type(Ship_select_background_bitmap);
		if(generic_anim_stream(&Ss_icons[ship_class].ss_anim) == -1) {
			//we've failed to load an animation, load an image and treat it like a 1 frame animation
			Ss_icons[ship_class].ss_anim.first_frame = bm_load(Ship_info[ship_class].anim_filename);	//if we fail here, the value is still -1
			if(Ss_icons[ship_class].ss_anim.first_frame != -1) {
				Ss_icons[ship_class].ss_anim.num_frames = 1;
			}
		}

		Ship_anim_class = ship_class;
	}

//	if ( play_sound ) {
		gamesnd_play_iface(SND_SHIP_ICON_CHANGE);
//	}
}

void ss_unload_all_anims()
{
	Assert( Ss_icons != NULL );

	for ( int i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		if ( Ss_icons[i].ss_anim.num_frames ) {
			generic_anim_unload(&Ss_icons[i].ss_anim);
		}
	}
}

bool is_weapon_carried(int weapon_index)
{
	for (int slot = 0; slot < MAX_WING_BLOCKS*MAX_WING_SLOTS; slot++)
	{
		// a ship must exist in this slot
		if (Wss_slots[slot].ship_class >= 0)
		{
			for (int bank = 0; bank < MAX_SHIP_WEAPONS; bank++)
			{
				// there must be a weapon here
				if (Wss_slots[slot].wep_count[bank] > 0)
				{
					if (Wss_slots[slot].wep[bank] == weapon_index)
						return true;
				}
			}
		}
	}

	return false;
}

// ------------------------------------------------------------------------
// commit_pressed() is called when the commit button from any of the briefing/ship select/ weapon
// select screens is pressed.  The ship selected is created, and the interface music is stopped.
void commit_pressed()
{
	int j, player_ship_info_index;
	
	if ( Wss_num_wings > 0 ) {
		if(!(Game_mode & GM_MULTIPLAYER)){
			int rc;
			rc = create_wings();
			if (rc != 0) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				return;
			}
		}
	}
	else if(Player_obj != NULL)
	{
		if ( Selected_ss_class == -1 ) {
			player_ship_info_index = Team_data[Common_team].default_ship;

		} else {
			Assert(Selected_ss_class >= 0 );
			player_ship_info_index = Selected_ss_class;
		}

		update_player_ship( player_ship_info_index );
		if ( wl_update_ship_weapons(Ships[Player_obj->instance].objnum, &Wss_slots[0]) == -1 ) {
			popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR( "Player ship has no weapons", 461));
			return;
		}
	}

	// Goober5000 - mjn.mixael's required weapon feature
	int num_required_weapons = 0;
	int num_satisfied_weapons = 0;
	SCP_string weapon_list;
	for (j=0; j<MAX_WEAPON_TYPES; j++)
	{
		if (Team_data[Common_team].weapon_required[j])
		{
			// add it to the message list
			num_required_weapons++;
			if (num_required_weapons > 1)
				weapon_list.append(1, EOLN);
			weapon_list.append(Weapon_info[j].name);

			// see if it's carried by any ship
			if (is_weapon_carried(j))
				num_satisfied_weapons++;
		}
	}
	if (num_satisfied_weapons < num_required_weapons)
	{
		if (num_required_weapons == 1)
		{
			popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR("The %s is required for this mission, but it has not been added to any ship loadout.", 1624), weapon_list.c_str());
			return;
		}
		else if (num_required_weapons > 1)
		{
			popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR("The following weapons are required for this mission, but at least one of them has not been added to any ship loadout:\n\n%s", 1625), weapon_list.c_str());
			return;
		}
	}

	// Check to ensure that the hotkeys are still pointing to valid objects.  It is possible
	// for the player to assign a ship to a hotkey, then go and delete that ship in the 
	// ship selection, and then try to start the mission.  This function will detect those objects,
	// and remove them from the hotkey linked lists.
	mission_hotkey_validate();

	// Goober5000 - no sound when skipping briefing
	if (!(The_mission.flags & MISSION_FLAG_NO_BRIEFING))
		gamesnd_play_iface(SND_COMMIT_PRESSED);

	// save the player loadout
	if ( !(Game_mode & GM_MULTIPLAYER) ) {
		strcpy_s(Player_loadout.filename, Game_current_mission_filename);
		strcpy_s(Player_loadout.last_modified, The_mission.modified);
		wss_save_loadout();
	}

	// warp the mouse cursor the the middle of the screen for those who control with a mouse
	mouse_set_pos( gr_screen.max_w/2, gr_screen.max_h/2 );

	// move to the next stage
	// in multiplayer this is the final mission sync
	if(Game_mode & GM_MULTIPLAYER){	
		if (MULTIPLAYER_MASTER) {
			// process the initial orders now (moved from post_process_mission()in missionparse) 
			mission_parse_fixup_players(); 
			ai_post_process_mission();
		}

		Multi_sync_mode = MULTI_SYNC_POST_BRIEFING;
		gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);	
		
		// otherwise tell the standalone to move everyone into this state and continue
		if((Net_player->flags & NETINFO_FLAG_GAME_HOST) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
			send_mission_sync_packet(MULTI_SYNC_POST_BRIEFING);
		}
	}
	// in single player we jump directly into the mission
	else {
		Pilot.save_savefile();
		gameseq_post_event(GS_EVENT_ENTER_GAME);
	}
}

// ------------------------------------------------------------------------
// pick_from_ship_list() will determine if an icon from the ship selection
// list can be picked up (for drag and drop).  It calculates the difference
// in x & y between the icon and the mouse, so we can move the icon with the
// mouse in a realistic way
int pick_from_ship_list(int screen_offset, int ship_class)
{
	int rval = -1;
	Assert(ship_class >= 0);
	Assert( Ss_pool != NULL );

	if ( Wss_num_wings == 0 )
		return rval;

	// If carrying an icon, then do nothing
	if ( ss_icon_being_carried() )
		return rval;

	if ( Ss_pool[ship_class] > 0 ) {
		int mouse_x, mouse_y;

		ss_set_carried_icon(-1, ship_class);
		mouse_get_pos_unscaled( &mouse_x, &mouse_y );
		Ss_delta_x = Ship_list_coords[gr_screen.res][screen_offset][0] - mouse_x;
		Ss_delta_y = Ship_list_coords[gr_screen.res][screen_offset][1] - mouse_y;
		Assert( Ss_pool[ship_class] >= 0 );
		rval = 0;
	}

	common_flash_button_init();
	return rval;
}

// ------------------------------------------------------------------------
// pick_from_wing() will determine if an icon from the wing formation (wb_num)
// and slot number (ws_num) can be picked up (for drag and drop).  It calculates
// the difference in x & y between the icon and the mouse, so we can move the icon with the
// mouse in a realistic way
void pick_from_wing(int wb_num, int ws_num)
{
	int slot_index;
	Assert(wb_num >= 0 && wb_num < MAX_WING_BLOCKS);
	Assert(ws_num >= 0 && ws_num < MAX_WING_SLOTS);
	Assert( (Ss_wings != NULL) && (Wss_slots != NULL) );
	
	ss_wing_info *wb;
	ss_slot_info *ws;
	wb = &Ss_wings[wb_num];
	ws = &wb->ss_slots[ws_num];
	slot_index = wb_num*MAX_WING_SLOTS+ws_num;

	if ( wb->wingnum < 0 )
		return;

	// Take care of case where the mouse button goes from up to down in one frame while
	// carrying an icon
	if ( Drop_on_wing_mflag && ss_icon_being_carried() ) {
		if ( !ss_disabled_slot(slot_index) ) {
			ss_drop(Carried_ss_icon.from_slot, Carried_ss_icon.ship_class, slot_index, -1);
			ss_reset_carried_icon();
			gamesnd_play_iface(SND_ICON_DROP_ON_WING);
		}
	}

	if ( ss_icon_being_carried() )
		return;

	if ( ss_disabled_slot(slot_index) ) {
		return;
	}

	if ( ws->status & WING_SLOT_IGNORE_SHIPS) {
		return;
	}

	switch ( ws->status ) {
		case WING_SLOT_EMPTY:
		case WING_SLOT_EMPTY|WING_SLOT_IS_PLAYER:
			// TODO: add fail sound
			return;
			break;

		case WING_SLOT_FILLED|WING_SLOT_IS_PLAYER:
		case WING_SLOT_FILLED:
			{
			int mouse_x, mouse_y;
			Assert(Wss_slots[slot_index].ship_class >= 0);
			ss_set_carried_icon(slot_index, Wss_slots[slot_index].ship_class);

			mouse_get_pos_unscaled( &mouse_x, &mouse_y );
			Ss_delta_x = Wing_icon_coords[gr_screen.res][slot_index][0] - mouse_x;
			Ss_delta_y = Wing_icon_coords[gr_screen.res][slot_index][1] - mouse_y;
			Carried_ss_icon.from_x = mouse_x;
			Carried_ss_icon.from_y = mouse_y;
			}
			break;
	
		default:
			Int3();
			break;

	} // end switch

	common_flash_button_init();
}

// ------------------------------------------------------------------------
// draw_wing_block() will draw the wing icons for the wing formation number
// passed in as a parameter.  
//
// input:	wb_num	=>		wing block number (numbering starts at 0)
//				hot_slot	=>		index of slot that mouse is over
//				selected_slot	=>	index of slot that is selected
//				class_select	=>	all ships of this class are drawn selected (send -1 to not use)
void draw_wing_block(int wb_num, int hot_slot, int selected_slot, int class_select, bool ship_selection )
{
	ss_wing_info	*wb;
	ss_slot_info	*ws;
	ss_icon_info	*icon;
	wing				*wp;
	int				i, w, h, sx, sy, slot_index, mask;
	int				bitmap_to_draw = -1;
	color			*color_to_draw = NULL;
	//shader			*shader_to_use = NULL;

	Assert(wb_num >= 0 && wb_num < MAX_WING_BLOCKS);
	Assert( (Ss_wings != NULL) && (Wss_slots != NULL) && (Ss_icons != NULL) );

	wb = &Ss_wings[wb_num];
	
	if ( wb->wingnum == -1 )
		return;	
	
	// print the wing name under the wing
	wp = &Wings[wb->wingnum];
	gr_get_string_size(&w, &h, wp->name);
	sx = Wing_icon_coords[gr_screen.res][wb_num*MAX_WING_SLOTS][0] + 16 - w/2;
	sy = Wing_icon_coords[gr_screen.res][wb_num*MAX_WING_SLOTS + 3][1] + 32 + h;
	gr_set_color_fast(&Color_normal);
	gr_string(sx, sy, wp->name);

	for ( i = 0; i < MAX_WING_SLOTS; i++ ) {
		bitmap_to_draw = -1;
		ws = &wb->ss_slots[i];
		slot_index = wb_num*MAX_WING_SLOTS + i;

		if ( Wss_slots[slot_index].ship_class >= 0 ) {
			icon = &Ss_icons[Wss_slots[slot_index].ship_class];
		} else {
			icon = NULL;
		}

		mask = ~WING_SLOT_LOCKED; 
		mask &= ~WING_SLOT_IS_PLAYER;
		if (ship_selection) {
			mask &= ~WING_SLOT_WEAPONS_DISABLED;
		} 
		else {
			mask &= ~WING_SLOT_SHIPS_DISABLED;
		} 

		switch(ws->status & mask ) {
			case WING_SLOT_FILLED:
			case WING_SLOT_FILLED|WING_SLOT_IS_PLAYER:

				Assert(icon);

				if ( class_select >= 0 ) {	// only ship select
					if ( Carried_ss_icon.from_slot == slot_index ) {
						if ( ss_carried_icon_moved() ) {
							bitmap_to_draw = Wing_slot_empty_bitmap;
						} else {
							bitmap_to_draw = -1;
						}
						break;
					}
				}

				if ( ws->status & WING_SLOT_LOCKED ) {
					if(icon->model_index == -1)
						bitmap_to_draw = icon->icon_bmaps[ICON_FRAME_DISABLED];
					else
						color_to_draw = &Icon_colors[ICON_FRAME_DISABLED];

					// in multiplayer, determine if this it the special case where the slot is disabled, and 
					// it is also _my_ slot (ie, team capatains/host have not locked players yet)
					if((Game_mode & GM_MULTIPLAYER) && multi_ts_disabled_high_slot(slot_index)){
						if(icon->model_index == -1)
							bitmap_to_draw = icon->icon_bmaps[ICON_FRAME_DISABLED_HIGH];
						else
							color_to_draw = &Icon_colors[ICON_FRAME_DISABLED_HIGH];
					}
					break;
				}

				if(icon->model_index == -1)
					bitmap_to_draw = icon->icon_bmaps[ICON_FRAME_NORMAL];
				else
					color_to_draw = &Icon_colors[ICON_FRAME_NORMAL];
				if ( selected_slot == slot_index || class_select == Wss_slots[slot_index].ship_class)
				{
					if(icon->model_index == -1)
						bitmap_to_draw = icon->icon_bmaps[ICON_FRAME_SELECTED];
					else
						color_to_draw = &Icon_colors[ICON_FRAME_SELECTED];
				}
				else if ( hot_slot == slot_index )
				{
					if ( mouse_down(MOUSE_LEFT_BUTTON) )
					{
						if(icon->model_index == -1)
							bitmap_to_draw = icon->icon_bmaps[ICON_FRAME_SELECTED];
						else
							color_to_draw = &Icon_colors[ICON_FRAME_SELECTED];
					}
					else
					{
						if(icon->model_index == -1)
							bitmap_to_draw = icon->icon_bmaps[ICON_FRAME_HOT];
						else
							color_to_draw = &Icon_colors[ICON_FRAME_HOT];
					}
				}

				if ( ws->status & WING_SLOT_IS_PLAYER && (selected_slot != slot_index) )
				{
					if(icon->model_index == -1)
						bitmap_to_draw = icon->icon_bmaps[ICON_FRAME_PLAYER];
					else
						color_to_draw = &Icon_colors[ICON_FRAME_PLAYER];
				}
				break;

			case WING_SLOT_EMPTY:
			case WING_SLOT_EMPTY|WING_SLOT_IS_PLAYER:
				bitmap_to_draw = Wing_slot_empty_bitmap;
				break;

			default:
				if ( icon ) {
					if(icon->model_index == -1)
						bitmap_to_draw = icon->icon_bmaps[ICON_FRAME_DISABLED];
					else
						color_to_draw = &Icon_colors[ICON_FRAME_DISABLED];
				} else {
					bitmap_to_draw = Wing_slot_disabled_bitmap;
				}
				break;
		}	// end switch

		
		if ( bitmap_to_draw != -1 ) {
			gr_set_bitmap(bitmap_to_draw);
			gr_bitmap(Wing_icon_coords[gr_screen.res][slot_index][0], Wing_icon_coords[gr_screen.res][slot_index][1]);
		}
		else if(color_to_draw != NULL)
		{
			ship_info *sip = &Ship_info[Wss_slots[slot_index].ship_class];
			gr_set_color_fast(color_to_draw);
			//gr_set_shader(shader_to_use);
			draw_model_icon(icon->model_index, MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING | MR_NO_LIGHTING, sip->closeup_zoom / 1.25f, Wing_icon_coords[gr_screen.res][slot_index][0], Wing_icon_coords[gr_screen.res][slot_index][1], 32, 28, sip);
			draw_brackets_square(Wing_icon_coords[gr_screen.res][slot_index][0], Wing_icon_coords[gr_screen.res][slot_index][1], Wing_icon_coords[gr_screen.res][slot_index][0] + 32, Wing_icon_coords[gr_screen.res][slot_index][1] + 28);
			//gr_shade(Wing_icon_coords[gr_screen.res][slot_index][0], Wing_icon_coords[gr_screen.res][slot_index][1], 32, 28);
		}
	}
}

// called by multiplayer team select to set the slot based flags
void ss_make_slot_empty(int slot_index)
{
	int wing_num,slot_num;
	ss_wing_info	*wb;
	ss_slot_info	*ws;

	Assert( Ss_wings != NULL );

	// calculate the wing #
	wing_num = slot_index / MAX_WING_SLOTS;
	slot_num = slot_index % MAX_WING_SLOTS;

	// get the wing and slot entries
	wb = &Ss_wings[wing_num];
	ws = &wb->ss_slots[slot_num];

	// set the flags
	ws->status &= ~(WING_SLOT_FILLED | WING_SLOT_SHIPS_DISABLED | WING_SLOT_WEAPONS_DISABLED);
	ws->status |= WING_SLOT_EMPTY;
}

// called by multiplayer team select to set the slot based flags
void ss_make_slot_full(int slot_index)
{
	int wing_num,slot_num;
	ss_wing_info	*wb;
	ss_slot_info	*ws;

	Assert( Ss_wings != NULL );

	// calculate the wing #
	wing_num = slot_index / MAX_WING_SLOTS;
	slot_num = slot_index % MAX_WING_SLOTS;

	// get the wing and slot entries
	wb = &Ss_wings[wing_num];
	ws = &wb->ss_slots[slot_num];

	// set the flags
	ws->status &= ~(WING_SLOT_EMPTY | WING_SLOT_SHIPS_DISABLED | WING_SLOT_WEAPONS_DISABLED);
	ws->status |= WING_SLOT_FILLED;
}

void ss_blit_ship_icon(int x,int y,int ship_class,int bmap_num)
{
	// blit the bitmap in the correct location
	if(ship_class == -1)
	{
		gr_set_bitmap(Wing_slot_empty_bitmap);
		gr_bitmap(x,y);
	}
	else
	{
		Assert( Ss_icons != NULL );

		ss_icon_info *icon = &Ss_icons[ship_class];
		if(icon->icon_bmaps[bmap_num] != -1)
		{
			Assert(icon->icon_bmaps[bmap_num] != -1);	
			gr_set_bitmap(icon->icon_bmaps[bmap_num]);
			gr_bitmap(x,y);	
		}
		else
		{
			ship_info *sip = &Ship_info[ship_class];
			if(icon->model_index == -1) {
				icon->model_index = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
				mprintf(("SL WARNING: Had to attempt to page in model for %s paged in manually! Result: %d\n", sip->name, icon->model_index));
			}
			if(icon->model_index != -1)
			{
				gr_set_color_fast(&Icon_colors[bmap_num]);
				//gr_set_shader(&Icon_shaders[bmap_num]);
				draw_model_icon(icon->model_index, MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING | MR_NO_LIGHTING, sip->closeup_zoom / 1.25f, x, y, 32, 28, sip);
				draw_brackets_square(x, y, x + 32, y + 28);
				//gr_shade(x, y, 32, 28);
			}
		}
	}
	
}

// ------------------------------------------------------------------------
//	unload_ship_icons() frees the memory that was used to hold the bitmaps
// for ship icons 
//
void unload_wing_icons()
{
	if ( Wing_slot_empty_bitmap != -1 ) {
		bm_release(Wing_slot_empty_bitmap);
		Wing_slot_empty_bitmap = -1;
	}

	if ( Wing_slot_disabled_bitmap != -1 ) {
		bm_release(Wing_slot_disabled_bitmap);
		Wing_slot_disabled_bitmap = -1;
	}
}

// ------------------------------------------------------------------------
//	create_wings() will ensure the correct ships are in the player wings
// for the game.  It works by calling change_ship_type() on the wing ships
// so they match what the player selected.   ship_create() is called for the
// player ship (and current_count, ship_index[] is updated), since it is not yet
// part of the wing structure.
//
// returns:   0 ==> success
//           !0 ==> failure
int create_wings()
{
	ss_wing_info		*wb;
	ss_slot_info		*ws;
	wing					*wp;
	p_object				*p_objp;

	int shipnum, objnum, slot_index;
	int cleanup_ship_index[MAX_WING_SLOTS];
	int i,j,k;
	int found_pobj;

	Assert( (Ss_wings != NULL) && (Wss_slots != NULL) );

	for ( i = 0; i < MAX_WING_BLOCKS; i++ ) {
		
		wb = &Ss_wings[i];

		if ( wb->wingnum ==  -1 )
			continue;

		wp = &Wings[wb->wingnum];		
		
		for ( j = 0; j < MAX_WING_SLOTS; j++ ) {
			slot_index = i*MAX_WING_SLOTS+j;
			ws = &wb->ss_slots[j];
			if ((ws->status & WING_SLOT_FILLED ) || (ws->status & WING_SLOT_SHIPS_DISABLED ) || (ws->status & WING_SLOT_WEAPONS_DISABLED )){
				if ( wp->ship_index[j] >= 0 ) {
					Assert(Ships[wp->ship_index[j]].objnum >= 0);
				}

				if ( ws->status & WING_SLOT_IS_PLAYER ) {
					update_player_ship(Wss_slots[slot_index].ship_class);

					if ( wl_update_ship_weapons(Ships[Player_obj->instance].objnum, &Wss_slots[i*MAX_WING_SLOTS+j]) == -1 ) {
						popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR( "Player ship has no weapons", 461));
						return -1;
					}

					objnum = OBJ_INDEX(Player_obj);
					shipnum = Objects[objnum].instance;
				} else {
					if ( wb->is_late) {
						found_pobj = 0;
						for ( p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp) ) {
							if ( p_objp->wingnum == WING_INDEX(wp) ) {
								if ( ws->sa_index == POBJ_INDEX(p_objp) ) {
									p_objp->ship_class = Wss_slots[slot_index].ship_class;
									wl_update_parse_object_weapons(p_objp, &Wss_slots[i*MAX_WING_SLOTS+j]);
									found_pobj = 1;
									break;
								}
							}
						}
						Assert(found_pobj);
					}
					else {
						// AL 10/04/97
						// Change the ship type of the ship if different than current.
						// NOTE: This will reset the weapons for this ship.  I think this is
						//       the right thing to do, since the ships may have different numbers
						//			of weapons and may not have the same allowed weapon types
						if ( Ships[wp->ship_index[j]].ship_info_index != Wss_slots[slot_index].ship_class )
							change_ship_type(wp->ship_index[j], Wss_slots[slot_index].ship_class);
						wl_update_ship_weapons(Ships[wp->ship_index[j]].objnum, &Wss_slots[i*MAX_WING_SLOTS+j]);
					}
				}
			}
			else if (ws->status & WING_SLOT_EMPTY) {
				if ( ws->status & WING_SLOT_IS_PLAYER ) {						
					popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR( "Player %s must select a place in player wing", 462), Player->callsign);
					return -1;
				}
			}
		}	// end for (wing slot)	
	}	// end for (wing block)

	for ( i = 0; i < MAX_WING_BLOCKS; i++ ) {
		wb = &Ss_wings[i];

		if ( wb->wingnum == -1 )
			continue;

		wp = &Wings[wb->wingnum];

		for ( k = 0; k < MAX_WING_SLOTS; k++ ) {
			cleanup_ship_index[k] = -1;
		}

		for ( j = 0; j < MAX_WING_SLOTS; j++ ) {
			ws = &wb->ss_slots[j];
			switch( ws->status ) {
				case WING_SLOT_EMPTY:	
					// delete ship that is not going to be used by the wing
					if ( wb->is_late ) {
						list_remove( &Ship_arrival_list, &Parse_objects[ws->sa_index]);
						wp->wave_count--;
						Assert(wp->wave_count >= 0);
					}
					else {
						shipnum = wp->ship_index[j];
						Assert( shipnum >= 0 && shipnum < MAX_SHIPS );
						cleanup_ship_index[j] = shipnum;
						ship_add_exited_ship( &Ships[shipnum], SEF_PLAYER_DELETED );
						obj_delete(Ships[shipnum].objnum);
						hud_set_wingman_status_none( Ships[shipnum].wing_status_wing_index, Ships[shipnum].wing_status_wing_pos);
					}
					break;

				default:
					break;

			} // end switch

		}	// end for (wing slot)	

		for ( k = 0; k < MAX_WING_SLOTS; k++ ) {
			if ( cleanup_ship_index[k] != -1 ) {
				ship_wing_cleanup( cleanup_ship_index[k], wp );
			}
		}

	}	// end for (wing block)
	
	return 0;
}

// ----------------------------------------------------------------------------
// update_player_ship()
//
// Updates the ship class of the player ship
//
//	parameters:	si_index  => ship info index of ship class to change to
//
//
void update_player_ship(int si_index)
{
	Assert( si_index >= 0 );
	Assert( Player_obj != NULL);

	// AL 10/04/97
	// Change the ship type of the player ship if different than current.
	// NOTE: This will reset the weapons for this ship.  I think this is
	//       the right thing to do, since the ships may have different numbers
	//			of weapons and may not have the same allowed weapon types
	if ( Player_ship->ship_info_index != si_index ) 
		change_ship_type(Player_obj->instance, si_index);

	Player->last_ship_flown_si_index = si_index;
}

/*
 * create a default player ship
 *
 * @note: only used for quick start missions
 *
 * @param	use_last_flown	select ship that was last flown on a mission (default parameter set to 1)
 *
 * @return	0 => success, !0 => failure
 */
int create_default_player_ship(int use_last_flown)
{
	int	player_ship_class=-1, i;

	// find the ship that matches the string stored in default_player_ship

	if ( use_last_flown ) {
		player_ship_class = Players[Player_num].last_ship_flown_si_index;
	}
	else {
		for (i = 0; i < Num_ship_classes; i++) {
			if ( !stricmp(Ship_info[i].name, default_player_ship) ) {
				player_ship_class = i;
				Players[Player_num].last_ship_flown_si_index = player_ship_class;
				break;
			}
		}

		if (i == Num_ship_classes)
			return 1;
	}

	// if we still haven't found the last flown ship, handle the error semi-gracefully
	if (player_ship_class == -1) {
		popup(PF_TITLE_BIG | PF_TITLE_RED | PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR("Error!\n\nCannot find "
			"a valid last flown ship\n\nHave you played any missions since activating this mod/campaign?", 1619));
		return 1;
	} else {
		update_player_ship(player_ship_class);
	}

	// debug code to keep using descent style physics if the player starts a new game
#ifndef NDEBUG
	if ( use_descent ) {
		use_descent = 0;
		toggle_player_object();
	}
#endif

	return 0;
}

// return the original ship class for the specified slot
int ss_return_original_ship_class(int slot_num)
{
	int wnum, snum;

	Assert( Ss_wings != NULL );

	wnum = slot_num/MAX_WING_SLOTS;
	snum = slot_num%MAX_WING_SLOTS;

	return Ss_wings[wnum].ss_slots[snum].original_ship_class;
}

// return the ship arrival index for the slot (-1 means no ship arrival index)
int ss_return_saindex(int slot_num)
{
	int wnum, snum;

	Assert( Ss_wings != NULL );

	wnum = slot_num/MAX_WING_SLOTS;
	snum = slot_num%MAX_WING_SLOTS;

	return Ss_wings[wnum].ss_slots[snum].sa_index;
}

// ----------------------------------------------------------------------------
// ss_return_ship()
//
// For a given wing slot, return the ship index if the ship has been created.  
// Otherwise, find the index into Parse_objects[] for the ship
//
//	input:	wing_block	=>		wing block of ship to find
//				wing_slot	=>		wing slot of ship to find
//				ship_index	=>		OUTPUT parameter: the Ships[] index of the ship in the wing slot
//										This value will be -1 if there is no ship created yet
//				ppobjp		=>		OUTPUT parameter: returns a pointer to a parse object for
//										the ship that hasn't been created yet.  Set to NULL if the
//										ship has already been created
//
// returns:	the original ship class of the ship, or -1 if the ship doesn't exist
//
// NOTE: For the player wing, the player is not yet in the wp->ship_index[].. so
// that is why there is an offset of 1 when getting ship indicies from the player
// wing.  The player is special cased by looking at the status of the wing slot
//
int ss_return_ship(int wing_block, int wing_slot, int *ship_index, p_object **ppobjp)
{
	*ship_index = -1;
	*ppobjp = NULL;

	ss_slot_info	*ws;

	Assert( Ss_wings != NULL );

	if (!Wss_num_wings) {
		*ppobjp = NULL;
		*ship_index = Player_obj->instance;
		return Player_ship->ship_info_index;
	}

	if ( Ss_wings[wing_block].wingnum < 0 ) {
		return -1;
	}

	ws = &Ss_wings[wing_block].ss_slots[wing_slot];

	// Check to see if ship is on the ship arrivals list
	if ( ws->sa_index != -1 ) {
		*ship_index = -1;
		*ppobjp = &Parse_objects[ws->sa_index];
	} else {
		*ship_index = Wings[Ss_wings[wing_block].wingnum].ship_index[wing_slot];
		Assert(*ship_index != -1);		
	}

	return ws->original_ship_class;
}

// return the name of the ship in the specified wing position... if the ship is the
// player ship, return the player callsign
//
// input: ensure at least NAME_LENGTH bytes allocated for name buffer
void ss_return_name(int wing_block, int wing_slot, char *name)
{
	ss_slot_info	*ws;
	wing				*wp;

	Assert( Ss_wings != NULL );

	ws = &Ss_wings[wing_block].ss_slots[wing_slot];
	wp = &Wings[Ss_wings[wing_block].wingnum];		

	if (!Wss_num_wings) {
		strcpy(name, Player->callsign);
		return;
	}

	// Check to see if ship is on the ship arrivals list
	if ( ws->sa_index != -1 ) {
		strcpy(name, Parse_objects[ws->sa_index].name);
	} else {
		ship *sp;
		sp = &Ships[wp->ship_index[wing_slot]];

		// in multiplayer, return the callsigns of the players who are in the ships
		if(Game_mode & GM_MULTIPLAYER){
			int player_index = multi_find_player_by_object(&Objects[sp->objnum]);
			if(player_index != -1){
				strcpy(name,Net_players[player_index].m_player->callsign);
			} else {
				strcpy(name,sp->ship_name);
			}
		} else {		
			strcpy(name, sp->ship_name);
		}
	}
}

int ss_get_selected_ship()
{
	return Selected_ss_class;
}

// Set selected ship to the first occupied wing slot, or first ship in pool if no slots are occupied
void ss_reset_selected_ship()
{
	int i;

	Assert( (Ss_pool != NULL) && (Wss_slots != NULL) );

	Selected_ss_class = -1;

	if ( Wss_num_wings <= 0 ) {
		Selected_ss_class = Team_data[Common_team].default_ship;
		return;
	}

	// get first ship class found on slots
	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		if ( Wss_slots[i].ship_class >= 0 ) {
			Selected_ss_class = Wss_slots[i].ship_class;
			break;
		}
	}

	if ( Selected_ss_class == -1 ) {
		Int3();
		for ( i = 0; i < MAX_SHIP_CLASSES; i++ ) {
			if ( Ss_pool[i] > 0 ) {
				Selected_ss_class = i;
			}
		}
	}

	if ( Selected_ss_class == -1 ) {
		Int3();
		return;
	}
}

// There may be ships that are in wings but not in Team_data[0].  Since we still want to show those
// icons in the ship selection list, the code below checks for these cases.  If a ship is found in
// a wing, and is not in Team_data[0], it is appended to the end of the ship_count[] and ship_list[] arrays
// that are in Team_data[0]
//
// exit: number of distinct ship classes available to choose from
int ss_fixup_team_data(team_data *tdata)
{
	int i, j, k, ship_in_parse_player, list_size;
	p_object		*p_objp;
	team_data	*p_team_data;

	p_team_data = tdata;
	ship_in_parse_player = 0;
	list_size = p_team_data->num_ship_choices;

	for ( i = 0; i < MAX_STARTING_WINGS; i++ ) {
		wing *wp;
		if ( Starting_wings[i] == -1 )
			continue;
		wp = &Wings[Starting_wings[i]];
		for ( j = 0; j < wp->current_count; j++ ) {
			ship_in_parse_player = 0;
			
			for ( k = 0; k < p_team_data->num_ship_choices; k++ ) {
				Assert( p_team_data->ship_count[k] >= 0 );
				if ( p_team_data->ship_list[k] == Ships[wp->ship_index[j]].ship_info_index ) {
					ship_in_parse_player = 1;
					break;
				}
			}	// end for, go to next item in parse player

			if ( !ship_in_parse_player ) {
				p_team_data->ship_count[list_size] = 0;
				p_team_data->ship_list[list_size] = Ships[wp->ship_index[j]].ship_info_index;
				p_team_data->num_ship_choices++;
				list_size++;
			}
		}	// end for, go get next ship in wing

		if ( wp->current_count == 0 ) {

			for ( p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp) ) {
				if ( p_objp->wingnum == WING_INDEX(wp) ) {
					ship_in_parse_player = 0;
			
					for ( k = 0; k < p_team_data->num_ship_choices; k++ ) {
						Assert( p_team_data->ship_count[k] >= 0 );
						if ( p_team_data->ship_list[k] == p_objp->ship_class ) {
							ship_in_parse_player = 1;
							break;
						}
					}	// end for, go to next item in parse player

					if ( !ship_in_parse_player ) {
						p_team_data->ship_count[list_size] = 0;
						p_team_data->ship_list[list_size] = p_objp->ship_class;
						p_team_data->num_ship_choices++;
						list_size++;
					}
				}
			}
		}
	}	// end for, go to next wing

	if ( list_size == 0 ) {
		// ensure that the default player ship is in the ship_list too
		ship_in_parse_player = 0;
		for ( k = 0; k < p_team_data->num_ship_choices; k++ ) {
			Assert( p_team_data->ship_count[k] >= 0 );
			if ( p_team_data->ship_list[k] == p_team_data->default_ship ) {
				ship_in_parse_player = 1;
				break;
			}
		}
		if ( !ship_in_parse_player ) {
			p_team_data->ship_count[list_size] = 0;
			p_team_data->ship_list[list_size] = p_team_data->default_ship;
			p_team_data->num_ship_choices++;
			list_size++;
		}
	}

	return list_size;
}

// set numbers of ships in pool to default values
void ss_init_pool(team_data *pteam)
{
	int i;

	Assert( Ss_pool != NULL );

	for ( i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		Ss_pool[i] = -1;
	}

	// set number of available ships based on counts in team_data
	for ( i = 0; i < pteam->num_ship_choices; i++ ) {
		if (Ss_pool[pteam->ship_list[i]] == -1) {
			Ss_pool[pteam->ship_list[i]] = 0; 
		}
		Ss_pool[pteam->ship_list[i]] += pteam->ship_count[i];
	}
}

// load the icons for a specific ship class
void ss_load_icons(int ship_class)
{
	ss_icon_info	*icon;

	Assert( Ss_icons != NULL );

	icon = &Ss_icons[ship_class];
	ship_info *sip = &Ship_info[ship_class];

	if(!Cmdline_ship_choice_3d && strlen(sip->icon_filename))
	{
		int				first_frame, num_frames, i;
		first_frame = bm_load_animation(sip->icon_filename, &num_frames);
		
		Assertion(first_frame != -1, "Failed to load icon %s\n", sip->icon_filename);	// Could not load in icon frames.. get Alan
		if (first_frame == -1)
			return;
	
		for ( i = 0; i < num_frames; i++ ) {
			icon->icon_bmaps[i] = first_frame+i;
		}

		// set the current bitmap for the ship icon
		icon->current_icon_bitmap = icon->icon_bmaps[ICON_FRAME_NORMAL];
	}
	else
	{
		icon->model_index = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
		model_page_in_textures(icon->model_index, ship_class);
	}
}

// load all the icons for ships in the pool
void ss_load_all_icons()
{
	int i, j;

	Assert( (Ss_pool != NULL) && (Ss_icons != NULL) );

	for ( i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		// clear out data
		Ss_icons[i].current_icon_bitmap = -1;
		for ( j = 0; j < NUM_ICON_FRAMES; j++ ) {
			Ss_icons[i].icon_bmaps[j] = -1;
		}
		Ss_icons[i].model_index = -1;

		if ( Ss_pool[i] >= 0 ) {
			ss_load_icons(i);
		}
	}
}

// determine if the slot is disabled
int ss_disabled_slot(int slot_num, bool ship_selection)
{
	int status; 

	if ( Wss_num_wings <= 0 ){
		return 0;
	}

	Assert( Ss_wings != NULL );

	// HACK HACK HACK - call the team select function in multiplayer
	if(Game_mode & GM_MULTIPLAYER) {
		return multi_ts_disabled_slot(slot_num);
	} 

	status = Ss_wings[slot_num/MAX_WING_SLOTS].ss_slots[slot_num%MAX_WING_SLOTS].status;

	if (ship_selection) {
		return ( status & WING_SLOT_IGNORE_SHIPS );
	}
	else {
		return ( status & WING_SLOT_IGNORE_WEAPONS );
	}
}

// Goober5000 - determine if the slot is valid
int ss_valid_slot(int slot_num)
{
	int status;

	if (ss_disabled_slot(slot_num))
		return 0;

	Assert( Ss_wings != NULL );

	status = Ss_wings[slot_num/MAX_WING_SLOTS].ss_slots[slot_num%MAX_WING_SLOTS].status;

	return (status & WING_SLOT_FILLED) && !(status & WING_SLOT_EMPTY);
}

// reset the slot data
void ss_clear_slots()
{
	int				i,j;
	ss_slot_info	*slot;

	Assert( (Wss_slots != NULL) && (Ss_wings != NULL) );

	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		Wss_slots[i].ship_class = -1;
		for ( j = 0; j < MAX_SHIP_WEAPONS; j++ ) {
			Wss_slots[i].wep[j] = 0;
			Wss_slots[i].wep_count[j] = 0;
		}
	}

	for ( i = 0; i < MAX_WING_BLOCKS; i++ ) {
		for ( j = 0; j < MAX_WING_SLOTS; j++ ) {
			slot = &Ss_wings[i].ss_slots[j];
			slot->status = WING_SLOT_LOCKED;
			slot->sa_index = -1;
			slot->original_ship_class = -1;
		}
	}
}

// initialize all wing struct stuff
void ss_clear_wings()
{
	int idx;

	Assert( Ss_wings != NULL );

	for(idx=0;idx<MAX_STARTING_WINGS;idx++){
		Ss_wings[idx].wingnum = -1;
		Ss_wings[idx].num_slots = 0;
		Ss_wings[idx].is_late = 0;
	}
}

// set up Wss_num_wings and Wss_wings[] based on Starting_wings[] info
void ss_init_wing_info(int wing_num,int starting_wing_num)
{
	wing				*wp;
	ss_wing_info	*ss_wing;
	ss_slot_info	*slot;

	Assert( Ss_wings != NULL );

	ss_wing = &Ss_wings[wing_num];

	if ( Starting_wings[starting_wing_num] < 0 ) {
		return;
	}

	ss_wing->wingnum = Starting_wings[starting_wing_num];
	Wss_num_wings++;

	wp = &Wings[ss_wing->wingnum];
	// niffiwan: don't overrun the array
	if (wp->current_count > MAX_WING_SLOTS) {
		Warning(LOCATION, "Starting Wing '%s' has '%d' ships. Truncating ship selection to 'MAX_WING_SLOTS'\n", Starting_wing_names[ss_wing->wingnum],wp->current_count);
		ss_wing->num_slots = MAX_WING_SLOTS;
	} else {
		ss_wing->num_slots = wp->current_count;
	}

	// deal with wing arriving after mission start
	if ( wp->current_count == 0 || wp->ship_index[0] == -1 ) {
		p_object *p_objp;
		// Temporarily fill in the current count and initialize the ship list in the wing
		// This gets cleaned up before the mission is started
		for ( p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp) ) {
			if ( p_objp->wingnum == WING_INDEX(wp) ) {
				// niffiwan: don't overrun the array
				if (ss_wing->num_slots >= MAX_WING_SLOTS) {
					Warning(LOCATION, "Starting Wing '%s' has more than 'MAX_WING_SLOTS' ships\n", Starting_wing_names[ss_wing->wingnum]);
					break;
				}
				slot = &ss_wing->ss_slots[ss_wing->num_slots++];
				slot->sa_index = POBJ_INDEX(p_objp);
				slot->original_ship_class = p_objp->ship_class;
			}
			ss_wing->is_late = 1;
		}
	}	
}

// Determine if a ship is actually a console player ship
int ss_wing_slot_is_console_player(int index)
{
	int wingnum, slotnum;
	
	wingnum=index/MAX_WING_SLOTS;
	slotnum=index%MAX_WING_SLOTS;

	if ( wingnum >= Wss_num_wings ) {
		return 0;
	}

	Assert( Ss_wings != NULL );

	if ( Ss_wings[wingnum].ss_slots[slotnum].status & WING_SLOT_IS_PLAYER ) {
		return 1;
	}

	return 0;
}

// init the ship selection portion of the units, and set up the ui data
void ss_init_units()
{
	int				i,j;
	wing				*wp;
	ss_slot_info	*ss_slot;
	ss_wing_info	*ss_wing;	

	Assert( (Ss_wings != NULL) && (Wss_slots != NULL) );

	for ( i = 0; i < Wss_num_wings; i++ ) {

		ss_wing = &Ss_wings[i];

		if ( ss_wing->wingnum < 0 ) {
			Int3();
			continue;
		}

		wp = &Wings[ss_wing->wingnum];

		for ( j = 0; j < ss_wing->num_slots; j++ ) {
				
			ss_slot = &ss_wing->ss_slots[j];

			if ( ss_slot->sa_index == -1 ) {
				ss_slot->original_ship_class = Ships[wp->ship_index[j]].ship_info_index;
			}

			//set the lock to the default
			if (Game_mode & GM_MULTIPLAYER) {
				ss_slot->status = WING_SLOT_LOCKED;
			}
			else {
				ss_slot->status = 0;
			}

			// Set the type of slot.  Check if the slot is marked as locked, if so then the player is not
			// going to be able to modify that ship.
			if ( ss_slot->sa_index == -1 ) {
				int objnum;
				if ( Ships[wp->ship_index[j]].flags2 & SF2_SHIP_LOCKED ) {
					ss_slot->status |= WING_SLOT_SHIPS_DISABLED;
				} 
				if ( Ships[wp->ship_index[j]].flags2 & SF2_WEAPONS_LOCKED ) {
					ss_slot->status |= WING_SLOT_WEAPONS_DISABLED;
				}  

				// if neither the ship or weapon has been locked, mark the slot as filled
				if (!(ss_slot->status & WING_SLOT_DISABLED)) {
					ss_slot->status = WING_SLOT_FILLED;
				}

				objnum = Ships[wp->ship_index[j]].objnum;
				if ( Objects[objnum].flags & OF_PLAYER_SHIP ) {
					if ( ss_slot->status & WING_SLOT_LOCKED ) {
						// Int3();	// Get Alan
						
						// just unflag it
						ss_slot->status &= ~(WING_SLOT_LOCKED);
					}
					ss_slot->status |= WING_SLOT_FILLED;
					if ( objnum == OBJ_INDEX(Player_obj) ) {
						ss_slot->status |= WING_SLOT_IS_PLAYER;
					}
				}
			} else {
				if ( Parse_objects[ss_slot->sa_index].flags2 & P2_SF2_SHIP_LOCKED ) {
					ss_slot->status |= WING_SLOT_SHIPS_DISABLED;
				} 
				if ( Parse_objects[ss_slot->sa_index].flags2 & P2_SF2_WEAPONS_LOCKED ) {
					ss_slot->status |= WING_SLOT_WEAPONS_DISABLED;
				} 
				
				// if the slot is not marked as locked, it's filled
				if (!(ss_slot->status & WING_SLOT_DISABLED)) {
					ss_slot->status = WING_SLOT_FILLED;
				}
				if ( Parse_objects[ss_slot->sa_index].flags & P_OF_PLAYER_START ) {
					if ( ss_slot->status & WING_SLOT_LOCKED ) {
						// Int3();	// Get Alan

						// just unflag it
						ss_slot->status &= ~(WING_SLOT_LOCKED);
					}
					ss_slot->status |= WING_SLOT_FILLED;
					ss_slot->status |= WING_SLOT_IS_PLAYER;
				}
			}

			// Assign the ship class to the unit
			Wss_slots[i*MAX_WING_SLOTS+j].ship_class = ss_slot->original_ship_class;

		}	// end for
	}	// end for

	// lock/unlock any necessary slots for multiplayer
	if(Game_mode & GM_MULTIPLAYER){
		ss_recalc_multiplayer_slots();
	}
}

// set the necessary pointers
void ss_set_team_pointers(int team)
{
	Assert( (team >= 0) && (team < MAX_TVT_TEAMS) );

	Ss_wings = Ss_wings_teams[team];
	Ss_icons = Ss_icons_teams[team];
}

// reset the necessary pointers to defaults
void ss_reset_team_pointers()
{
	Assert( !Ship_select_open );

	if ( Ship_select_open )
		return;

	Ss_wings = NULL;
	Ss_icons = NULL;
}

// initialize team specific stuff
void ship_select_init_team_data(int team_num)
{			
	int idx;

	// set up the pointers to initialize the data structures.
	common_set_team_pointers(team_num);
	
	ss_fixup_team_data(&Team_data[team_num]);
	ss_init_pool(&Team_data[team_num]);
	
	ss_clear_slots();		// reset data for slots	
	ss_clear_wings();

	// determine how many wings we should be checking for
	Wss_num_wings = 0;

	if(MULTI_TEAM){
		// now setup wings for easy reference		
		ss_init_wing_info(0,team_num);			
	} else {			
		// now setup wings for easy reference
		for (idx = 0; idx < MAX_STARTING_WINGS; idx++) {
			ss_init_wing_info(Wss_num_wings, idx);	
		}
	}
	

	// if there are no wings, don't call the init_units() function
	if ( Wss_num_wings <= 0 ) {
		Wss_slots[0].ship_class = Team_data[team_num].default_ship;
		return;
	}

	ss_init_units();	
}

// called when the briefing is entered
void ship_select_common_init()
{		
	// initialize team critical data for all teams
	int idx;

	if(MULTI_TEAM){		
		// initialize for all teams in the game
		for(idx=0;idx<MULTI_TS_MAX_TVT_TEAMS;idx++){	
			ship_select_init_team_data(idx);
		}		

		// finally, intialize team data for myself
		ship_select_init_team_data(Common_team);
	} else {			
		ship_select_init_team_data(Common_team);
	}
	
	init_active_list();

	// load the necessary icons/animations
	ss_load_all_icons();

	ss_reset_selected_ship();
	ss_reset_carried_icon();
}

void ship_select_common_close()
{
	ss_unload_all_icons();
	ss_unload_all_anims();
}

// change any interface data based on updated Wss_slots[] and Ss_pool[]
void ss_synch_interface()
{
	int				i;
	ss_slot_info	*slot;

	Assert( Ss_wings != NULL );

	int old_list_start = SS_active_list_start;

	init_active_list();	// build the list of pool ships

	if ( old_list_start < SS_active_list_size ) {
		SS_active_list_start = old_list_start;
	}

	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		slot = &Ss_wings[i/MAX_WING_SLOTS].ss_slots[i%MAX_WING_SLOTS];

		if ( Wss_slots[i].ship_class == -1 ) {
			if ( slot->status & WING_SLOT_FILLED ) {
				slot->status &= ~WING_SLOT_FILLED;
				slot->status |= WING_SLOT_EMPTY;
			}
		} else {
			if ( slot->status & WING_SLOT_EMPTY ) {
				slot->status &= ~WING_SLOT_EMPTY;
				slot->status |= WING_SLOT_FILLED;
			}
		}
	}
}

// exit: data changed flag
int ss_swap_slot_slot(int from_slot, int to_slot, int *sound)
{
	int i, tmp;

	if ( from_slot == to_slot ) {
		*sound=SND_ICON_DROP_ON_WING;
		return 0;
	}

	// ensure from_slot has a ship to pick up
	if ( Wss_slots[from_slot].ship_class < 0 ) {
		*sound=SND_ICON_DROP;
		return 0;
	}

	// swap ship class
	tmp = Wss_slots[from_slot].ship_class;
	Wss_slots[from_slot].ship_class = Wss_slots[to_slot].ship_class;
	Wss_slots[to_slot].ship_class = tmp;

	// swap weapons
	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ ) {
		tmp = Wss_slots[from_slot].wep[i];
		Wss_slots[from_slot].wep[i] = Wss_slots[to_slot].wep[i];
		Wss_slots[to_slot].wep[i] = tmp;

		tmp = Wss_slots[from_slot].wep_count[i];
		Wss_slots[from_slot].wep_count[i] = Wss_slots[to_slot].wep_count[i];
		Wss_slots[to_slot].wep_count[i] = tmp;
	}

	*sound=SND_ICON_DROP_ON_WING;
	return 1;
}

// exit: data changed flag
int ss_dump_to_list(int from_slot, int to_list, int *sound)
{
	int i;
	wss_unit	*slot;

	Assert( (Ss_pool != NULL) && (Wl_pool != NULL) && (Wss_slots != NULL) );

	slot = &Wss_slots[from_slot];

	// ensure from_slot has a ship to pick up
	if ( slot->ship_class < 0 ) {
		*sound=SND_ICON_DROP;
		return 0;
	}

	// put ship back in list
	Ss_pool[to_list]++;		// return to list
	slot->ship_class = -1;	// remove from slot

	// put weapons back in list
	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ ) {
		if ( (slot->wep[i] >= 0) && (slot->wep_count[i] > 0) ) {
			Wl_pool[slot->wep[i]] += slot->wep_count[i];
			slot->wep[i] = -1;
			slot->wep_count[i] = 0;
		}
	}

	*sound=SND_ICON_DROP;
	return 1;
}

// exit: data changed flag
int ss_grab_from_list(int from_list, int to_slot, int *sound)
{
	wss_unit        *slot;
	int i, wep[MAX_SHIP_WEAPONS], wep_count[MAX_SHIP_WEAPONS];

	Assert( (Ss_pool != NULL) && (Wss_slots != NULL) );

	slot = &Wss_slots[to_slot];

	// ensure that pool has ship
	if ( Ss_pool[from_list] <= 0 )
	{
		*sound=SND_ICON_DROP;
		return 0;
	}

	Assert(slot->ship_class < 0 );	// slot should be empty

	// take ship from list->slot
	Ss_pool[from_list]--;
	slot->ship_class = from_list;

	// take weapons from list->slot
	wl_get_default_weapons(from_list, to_slot, wep, wep_count);
	wl_remove_weps_from_pool(wep, wep_count, slot->ship_class);
	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ )
	{
		slot->wep[i] = wep[i];
		slot->wep_count[i] = wep_count[i];
	}

	*sound=SND_ICON_DROP_ON_WING;
	return 1;
}
                        
// exit: data changed flag
int ss_swap_list_slot(int from_list, int to_slot, int *sound)
{
	int i, wep[MAX_SHIP_WEAPONS], wep_count[MAX_SHIP_WEAPONS];
	wss_unit        *slot;

	Assert( (Ss_pool != NULL) && (Wl_pool != NULL) && (Wss_slots != NULL) );

	// ensure that pool has ship
	if ( Ss_pool[from_list] <= 0 )
	{
		*sound=SND_ICON_DROP;
		return 0;
	}

	slot = &Wss_slots[to_slot];
	Assert(slot->ship_class >= 0 );        // slot should be filled

	// put ship from slot->list
	Ss_pool[Wss_slots[to_slot].ship_class]++;

	// put weapons from slot->list
	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ )
	{
		if ( (slot->wep[i] >= 0) && (slot->wep_count[i] > 0) )
		{
			Wl_pool[slot->wep[i]] += slot->wep_count[i];
			slot->wep[i] = -1;
			slot->wep_count[i] = 0;
		}
	}

	// take ship from list->slot
	Ss_pool[from_list]--;
	slot->ship_class = from_list;

	// take weapons from list->slot
	wl_get_default_weapons(from_list, to_slot, wep, wep_count);
	wl_remove_weps_from_pool(wep, wep_count, slot->ship_class);
	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ )
	{
		slot->wep[i] = wep[i];
		slot->wep_count[i] = wep_count[i];
	}

	*sound=SND_ICON_DROP_ON_WING;
	return 1;
}
                        
void ss_apply(int mode, int from_slot, int from_list, int to_slot, int to_list,int player_index)
{
	int update=0;
	int sound=-1;

	switch(mode){
	case WSS_SWAP_SLOT_SLOT:
		update = ss_swap_slot_slot(from_slot, to_slot, &sound);
		break;
	case WSS_DUMP_TO_LIST:
		update = ss_dump_to_list(from_slot, to_list, &sound);
		break;
	case WSS_GRAB_FROM_LIST:
		update = ss_grab_from_list(from_list, to_slot, &sound);
		break;
	case WSS_SWAP_LIST_SLOT:
		update = ss_swap_list_slot(from_list, to_slot, &sound);
		break;
	}

	// only play this sound if the move was done locally (by the host in other words)
	if ( (sound >= 0) && (player_index == -1) ) {
		gamesnd_play_iface(sound);		
	}

	if ( update ) {
		// NO LONGER USED - THERE IS A MULTIPLAYER VERSION OF THIS SCREEN NOW
		/*
		if ( MULTIPLAYER_HOST ) {
			int size;
			ubyte wss_data[MAX_PACKET_SIZE-20];
			size = store_wss_data(wss_data, MAX_PACKET_SIZE-20, sound);
			send_wss_update_packet(wss_data, size, player_index);
		}
		*/

		ss_synch_interface();
	}
}

void ss_drop(int from_slot,int from_list,int to_slot,int to_list,int player_index)
{
	int mode;
	common_flash_button_init();
	
	mode = wss_get_mode(from_slot, from_list, to_slot, to_list, -1);
	if ( mode >= 0 ) {
		ss_apply(mode, from_slot, from_list, to_slot, to_list,player_index);
	}	
}

// lock/unlock any necessary slots for multiplayer
void ss_recalc_multiplayer_slots()
{
	int				i,j;
	ss_slot_info	*ss_slot;
	ss_wing_info	*ss_wing;
	
	// no wings
	if ( Wss_num_wings <= 0 ) {
		Assert( Wss_slots != NULL );
		Wss_slots[0].ship_class = Team_data[Common_team].default_ship;
		return;
	}

	Assert( Ss_wings != NULL );

	for ( i = 0; i < Wss_num_wings; i++ ) {
		ss_wing = &Ss_wings[i];
		if ( ss_wing->wingnum < 0 ) {
			Int3();
			continue;
		}

		// NOTE : the method below will eventually have to change to account for all possible netgame options	
		for ( j = 0; j < ss_wing->num_slots; j++ ) {				
			// get the slot pointer
			ss_slot = &ss_wing->ss_slots[j];			
			
			if (ss_slot->sa_index == -1) {					
				// lock all slots by default
				ss_slot->status |= WING_SLOT_LOCKED;
				
				// if this is my slot, then unlock it
				if(!multi_ts_disabled_slot((i*MAX_WING_SLOTS)+j)){				
					ss_slot->status &= ~WING_SLOT_LOCKED;
				}
			}
		}
	}
}
