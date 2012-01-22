/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "globalincs/pstypes.h"
#include "mission/missionhotkey.h"
#include "gamesequence/gamesequence.h"
#include "freespace2/freespace.h"
#include "io/key.h"
#include "io/timer.h"
#include "gamesnd/gamesnd.h"
#include "sound/audiostr.h"
#include "ship/ship.h"
#include "object/object.h"
#include "globalincs/linklist.h"
#include "playerman/player.h"
#include "ui/ui.h"
#include "ui/uidefs.h"
#include "missionui/missionscreencommon.h"
#include "graphics/font.h"
#include "gamehelp/contexthelp.h"
#include "globalincs/alphacolors.h"
#include "weapon/beam.h"
#include "iff_defs/iff_defs.h"
#include "parse/parselo.h"


static int Key_sets[MAX_KEYED_TARGETS] = {
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12
};

/////////////////////////////

static int Hotkey_bits[MAX_SHIPS];  // bitfield indicating which hotkeys are used by each ship

static int Hotkey_sets_saved;			// have we saved the sets for this mission

static int Mission_hotkey_save_timestamp;		// timestamp used to tell us when we can save
#define HOTKEY_SAVE_TIME				15000		// save sets this number of milliseconds into the mission

typedef struct {
	int setnum;
	char name[NAME_LENGTH];
} HK_save_info;

HK_save_info Hotkey_saved_info[MAX_HOTKEY_TARGET_ITEMS];
int Num_hotkeys_saved;


static char *Hotkey_background_fname[GR_NUM_RESOLUTIONS] = {
	"Hotkeys",		// GR_640
	"2_Hotkeys"		// GR_1024
};

static char *Hotkey_mask_fname[GR_NUM_RESOLUTIONS] = {
	"Hotkeys-M",		// GR_640
	"2_Hotkeys-M"	// GR_1024
};

//#define GROUP_LIST_X	40
//#define GROUP_LIST_W	160

// #define ICON_LIST_X	219
// #define ICON_LIST_W	8

// #define ICON_LIST_X	280
// #define ICON_LIST_W	8

//#define SHIP_LIST_X	242
//#define SHIP_LIST_X2	259
//#define SHIP_LIST_W	341
//#define SHIP_LIST_W2	324

// #define SHIP_LIST_X	302
// #define SHIP_LIST_X2	319
// #define SHIP_LIST_W	281
// #define SHIP_LIST_W2	264

// #define LIST_Y			70
// #define LIST_H			280

/*
#define HOTKEY_X		575
#define HOTKEY_Y		41
*/
 
#define HOTKEY_LINE_HEADING	1
#define HOTKEY_LINE_WING		2
#define HOTKEY_LINE_SHIP		3
#define HOTKEY_LINE_SUBSHIP	4  // ship that is in a wing

#define WING_FLAG	0x80000

#define MAX_LINES					200
#define NUM_BUTTONS				10
#define LIST_BUTTONS_MAX		50

#define SCROLL_UP_BUTTON		0
#define SCROLL_DOWN_BUTTON		1
#define CANCEL_BUTTON			2
#define CLEAR_BUTTON				3
#define RESET_BUTTON				4
#define ADD_HOTKEY_BUTTON		5
#define REMOVE_HOTKEY_BUTTON	6
#define HELP_BUTTON				7
#define OPTIONS_BUTTON			8
#define ACCEPT_BUTTON			9

// coords for entire ship box
static int Hotkey_list_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		// GR_640
		29,			// x
		22,			// y
		502,			// w
		315			// h
	},
	{
		// GR_1024
		47,			// x
		35,			// y
		802,			// w
		505			// h
	}
};

// coords for big "F9" thing in the corner
static int Hotkey_function_name_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		// GR_640
		570,			// x
		14,			// y
		59,			// w
		22				// h
	},
	{
		// GR_1024
		912,			// x
		22,			// y
		94,			// w
		36				// h
	}
};

/*
#define FIELD_LEFT_EDGE		0
#define FIELD_F5				1
#define FIELD_F6				2
#define FIELD_F7				3
#define FIELD_F8				4
#define FIELD_F9				5
#define FIELD_F10				6
#define FIELD_F11				7
#define FIELD_F12				8
#define FIELD_ICON			9
#define FIELD_RIGHT_EDGE	10
// x coords of unseen field boundaries (  | field1 | field2 | ... |  )
// entried will all be centered in fields except FIELD_SHIP which will be left justified
// an edge is named by the field on its left
static int Hotkey_field_edge[GR_NUM_RESOLUTIONS][11] = {
	{
		29, 56, 83, 110, 137, 164, 191, 218, 245, 280, 531
	},
	{
		47, 91, 135, 179, 223, 267, 311, 355, 399, 448, 849
	}
}
*/

static int Hotkey_function_field_width[GR_NUM_RESOLUTIONS] = {
	27,			// GR_640
	44				// GR_1024
};
static int Hotkey_wing_icon_x[GR_NUM_RESOLUTIONS] = {
	246,			// GR_640
	400			// GR_1024
};
static int Hotkey_ship_x[GR_NUM_RESOLUTIONS] = {
	280,			// GR_640
	448			// GR_1024
};

struct hotkey_buttons {
	char *filename;
	int x, y;
	int hotspot;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	hotkey_buttons(char *name, int x1, int y1, int h) : filename(name), x(x1), y(y1), hotspot(h) {}
};

// button definitions
static hotkey_buttons Buttons[GR_NUM_RESOLUTIONS][NUM_BUTTONS] = {
//XSTR:OFF
	{
		// GR_640
		hotkey_buttons("HKB_00",	1,		94,	0),
		hotkey_buttons("HKB_01",	1,		133,	1),
		hotkey_buttons("HKB_02",	15,	342,	2),
		hotkey_buttons("HKB_03",	84,	342,	3),
		hotkey_buttons("HKB_04",	161,	342,	4),
		hotkey_buttons("HKB_05",	539,	5,		5),
		hotkey_buttons("HKB_06",	539,	44,	6),
		hotkey_buttons("HKB_07",	539,	431,	7),
		hotkey_buttons("HKB_08",	539,	455,	8),
		hotkey_buttons("HKB_09",	575,	432,	9)
	},
	{
		// GR_1024
		hotkey_buttons("2_HKB_00",		2,		150,	0),
		hotkey_buttons("2_HKB_01",		2,		213,	1),
		hotkey_buttons("2_HKB_02",		24,	548,	2),
		hotkey_buttons("2_HKB_03",		135,	548,	3),
		hotkey_buttons("2_HKB_04",		258,	548,	4),
		hotkey_buttons("2_HKB_05",		862,	8,		5),
		hotkey_buttons("2_HKB_06",		862,	71,	6),
		hotkey_buttons("2_HKB_07",		863,	690,	7),
		hotkey_buttons("2_HKB_08",		862,	728,	8),
		hotkey_buttons("2_HKB_09",		920,	692,	9)
	}
//XSTR:ON
};

#define HOTKEY_NUM_TEXT		6
static UI_XSTR Hotkey_text[GR_NUM_RESOLUTIONS][HOTKEY_NUM_TEXT] = {
	{ 
		// GR_640
		{ "Cancel",		1516,	7,	392,		UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_640][CANCEL_BUTTON].button },
		{ "Clear",		1517,	85, 392,		UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_640][CLEAR_BUTTON].button },
		{ "Reset",		1518,	159, 392,	UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_640][RESET_BUTTON].button },
		{ "Help",		1519,	500, 440,	UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_640][HELP_BUTTON].button },
		{ "Options",	1520,	479, 464,	UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_640][OPTIONS_BUTTON].button },
		{ "Accept",		1521,	573, 413,	UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_640][ACCEPT_BUTTON].button }
	}, 
	{ 
		// GR_1024
		{ "Cancel",		1516,	30, 629,		UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_1024][CANCEL_BUTTON].button },
		{ "Clear",		1517,	151, 629,	UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_1024][CLEAR_BUTTON].button },
		{ "Reset",		1518,	269, 629,	UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_1024][RESET_BUTTON].button },
		{ "Help",		1519,	800, 704,	UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_1024][HELP_BUTTON].button },
		{ "Options",	1520,	797, 743,	UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_1024][OPTIONS_BUTTON].button },
		{ "Accept",		1521,	902, 661,	UI_XSTR_COLOR_GREEN, -1, &Buttons[GR_1024][ACCEPT_BUTTON].button }	
	}
};



static struct {
	char *label;
	int type;
	int index;
	int y;  // Y coordinate of line
} Hotkey_lines[MAX_LINES];

static int Cur_hotkey = 0;
static int Scroll_offset;
static int Num_lines;
static int Selected_line;
static int Background_bitmap;
static int Wing_bmp;
static UI_WINDOW Ui_window;
static UI_BUTTON List_buttons[LIST_BUTTONS_MAX];  // buttons for each line of text in list
//static UI_BUTTON List_region;

//////////////////////


// function used in a couple of places to get the actual hotkey set number from a key value.
// not trivial since our current keysets (F5 - F12) do not have sequential keycodes
int mission_hotkey_get_set_num( int k )
{
	int i;

	for (i = 0; i < MAX_KEYED_TARGETS; i++ ) {
		if ( Key_sets[i] == k ) {
			return i;
		}
	}

	Int3();		// get allender
	return 0;
}
		
// function to maybe restore some hotkeys during the first N seconds of the mission
void mission_hotkey_maybe_restore()
{
	int i, index;

	for ( i = 0; i < Num_hotkeys_saved; i++ ) {
		// don't process something that has no set
		if ( Hotkey_saved_info[i].setnum == -1 )
			continue;

		// the ship is present, add it to the given set.
		index = ship_name_lookup(Hotkey_saved_info[i].name);
		if ( index != -1 ) {
			hud_target_hotkey_add_remove( Hotkey_saved_info[i].setnum, &Objects[Ships[index].objnum], HOTKEY_USER_ADDED );
			Hotkey_saved_info[i].setnum = -1;
		}
	}
}

// ---------------------------------------------------------------------
// mission_hotkey_set_defaults()
//
// Set up the hotkey lists for the player based on the mission designer
// defaults.  
//
void mission_hotkey_set_defaults()
{
	int		i,j;
	wing		*wp;
	ship		*sp;
	object	*A;

	for ( i = 0; i < MAX_KEYED_TARGETS; i++ ) {
		hud_target_hotkey_clear(i);
	}

	// set the variable letting us know that we should save the hotkey sets
	Hotkey_sets_saved = 0;
	Mission_hotkey_save_timestamp = timestamp(HOTKEY_SAVE_TIME);

	// if we have hotkeys saved from the previous run of this mission, then simply keep the cleared
	// sets, and let the restore code take care of it!  This works because this function is currently
	// only called from one place -- after the mission loads.
	if ( Num_hotkeys_saved > 0 ) {
		mission_hotkey_maybe_restore();
		return;
	}

	// Check for ships with a hotkey assigned
	obj_merge_created_list();
	for ( A = GET_FIRST(&obj_used_list); A !=END_OF_LIST(&obj_used_list); A = GET_NEXT(A) ) {

		if ( (A == &obj_used_list) || (A->type != OBJ_SHIP) || ((Game_mode & GM_NORMAL) && (A == Player_obj)) ) {
			continue;
		}

		Assert(A->instance >= 0 && A->instance < MAX_SHIPS);
		sp = &Ships[A->instance];		

		if ( sp->hotkey == -1 )
			continue;

		// if the hotkey is the last hotkey in the list, then don't add it either since this hotkey is a special
		// marker to indicate that this ship should remain invisible in the hotkey screen until after mission
		// starts
		if ( sp->hotkey == MAX_KEYED_TARGETS )
			continue;

		Assert(sp->objnum >= 0);
		hud_target_hotkey_add_remove( sp->hotkey, &Objects[sp->objnum], HOTKEY_MISSION_FILE_ADDED );
	}

	// Check for wings with a hotkey assigned
	for ( i = 0; i < Num_wings; i++ ) {
		wp = &Wings[i];

		if ( wp->hotkey == -1 )  
			continue;

		// like ships, skip this wing if the hotkey is the last hotkey item
		if ( wp->hotkey == MAX_KEYED_TARGETS )
			continue;

		for ( j = 0; j < wp->current_count; j++ ) {
			if ( wp->ship_index[j] == -1 )
				continue;

			sp = &Ships[wp->ship_index[j]];
			hud_target_hotkey_add_remove( wp->hotkey, &Objects[sp->objnum], HOTKEY_MISSION_FILE_ADDED );
		}				
	}
}

// function to reset the saved hotkeys -- called when a new mission is loaded
void mission_hotkey_reset_saved()
{
	Num_hotkeys_saved = 0;
}

// next function called when we might want to save the hotkey sets for the player.  We will save the hotkey
// sets N seconds into the mission
void mission_hotkey_maybe_save_sets()
{
	int i;
	htarget_list	*hitem, *plist;
	HK_save_info *hkp;

	if ( !timestamp_elapsed(Mission_hotkey_save_timestamp) ) {
		mission_hotkey_maybe_restore();
		return;
	}

	// no processing if we have saved them.
	if ( Hotkey_sets_saved )
		return;

	for ( i = 0; i < MAX_HOTKEY_TARGET_ITEMS; i++ )
		Hotkey_saved_info[i].setnum = -1;

	Num_hotkeys_saved = 0;
	hkp = &(Hotkey_saved_info[0]);

	for ( i = 0; i < MAX_KEYED_TARGETS; i++ ) {

		// get the list.  do nothing if list is empty
		plist = &(Player->keyed_targets[i]);
		if ( EMPTY(plist) )
			continue;

		for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) ) {
			Assert( Num_hotkeys_saved < MAX_HOTKEY_TARGET_ITEMS );
			hkp->setnum = i;
			strcpy_s( hkp->name, Ships[hitem->objp->instance].ship_name );
			hkp++;
			Num_hotkeys_saved++;
		}
	}

	Hotkey_sets_saved = 1;
}

// function which gets called from MissionParse to maybe add a ship or wing to a hotkey set.
// this intermediate function is needed so that we don't blast over possibly saved hotkey sets
void mission_hotkey_mf_add( int set, int objnum, int how_to_add )
{
	// if we are restoring hotkeys, and the timer hasn't elapsed, then return and let the
	// hotkey restoration code deal with it
	if ( Num_hotkeys_saved && !timestamp_elapsed(Mission_hotkey_save_timestamp) )
		return;

	// we can add it to the set
	hud_target_hotkey_add_remove( set, &Objects[objnum], how_to_add );
}

void mission_hotkey_validate()
{
	htarget_list	*hitem, *plist;
	object			*A;
	int				obj_valid, i;

	for ( i = 0; i < MAX_KEYED_TARGETS; i++ ) {
		plist = &(Players[Player_num].keyed_targets[i]);
		if ( EMPTY( plist ) )			// no items in list, then do nothing
			continue;

		hitem = GET_FIRST(plist);
		while ( hitem != END_OF_LIST(plist) ) {

			// ensure this object is still valid and in the obj_used_list
			obj_valid = FALSE;
			for ( A = GET_FIRST(&obj_used_list); A !=END_OF_LIST(&obj_used_list); A = GET_NEXT(A) ) {
				if ( A->signature == hitem->objp->signature ) {
					obj_valid = TRUE;
					break;
				}
			}
			if ( obj_valid == FALSE ) {
				htarget_list *temp;

				temp = GET_NEXT(hitem);
				list_remove( plist, hitem );
				list_append( &htarget_free_list, hitem );
				hitem->objp = NULL;
				hitem = temp;
				continue;
			}
			hitem = GET_NEXT( hitem );
		}	// end while
	} // end for
}


// get the Hotkey_bits of a whole wing (bits must be set in all ships of wing for a hotkey bit to be set)
int get_wing_hotkeys(int n)
{
	int i, total = 0xffffffff;

	Assert((n >= 0) && (n < Num_wings));
	for (i=0; i<Wings[n].current_count; i++) {
		int ship_index;

		// don't count the player ship for the total -- you cannot assign the player since bad things
		// can happen on the hud.
		ship_index = Wings[n].ship_index[i];
		if ( &Ships[ship_index] == Player_ship )
			continue;

		total &= Hotkey_bits[Wings[n].ship_index[i]];
	}

	return total;
}

// add a line of hotkey smuck to end of list
int hotkey_line_add(char *text, int type, int index, int y)
{
	if (Num_lines >= MAX_LINES)
		return 0;

	Hotkey_lines[Num_lines].label = text;
	Hotkey_lines[Num_lines].type = type;
	Hotkey_lines[Num_lines].index = index;
	Hotkey_lines[Num_lines].y = y;
	return Num_lines++;
}

// insert a line of hotkey smuck before line 'n'.
int hotkey_line_insert(int n, char *text, int type, int index)
{
	int z;

	if (Num_lines >= MAX_LINES)
		return 0;

	z = Num_lines++;
	while (z > n) {
		Hotkey_lines[z] = Hotkey_lines[z - 1];
		z--;
	}

	Hotkey_lines[z].label = text;
	Hotkey_lines[z].type = type;
	Hotkey_lines[z].index = index;
	return z;
}

// insert a line of hotkey smuck somewhere between 'start' and end of list such that it is
// sorted by name
int hotkey_line_add_sorted(char *text, int type, int index, int start)
{
	int z;

	if (Num_lines >= MAX_LINES)
		return -1;

	z = Num_lines - 1;
	while ((z >= start) && ((Hotkey_lines[z].type == HOTKEY_LINE_SUBSHIP) || (stricmp(text, Hotkey_lines[z].label) < 0)))
		z--;

	z++;
	while ((z < Num_lines) && (Hotkey_lines[z].type == HOTKEY_LINE_SUBSHIP))
		z++;

	return hotkey_line_insert(z, text, type, index);
}

int hotkey_build_team_listing(int enemy_team_mask, int y, bool list_enemies)
{
	ship_obj *so;
	char *str = NULL;
	int i, j, s, z, start, team_mask;
	int font_height = gr_get_font_height();

	if (list_enemies)
	{
		str = XSTR( "Enemy ships", 403);
		team_mask = enemy_team_mask;
	}
	else
	{
		str = XSTR( "Friendly ships", 402);
		team_mask = ~enemy_team_mask;
	}

	for (i=0; i<MAX_SHIPS; i++)
		if (iff_matches_mask(Ships[i].team, team_mask))
			break;

	if (i >= MAX_SHIPS)
		return y;

	hotkey_line_add(str, HOTKEY_LINE_HEADING, 0, y);
	y += 2;

	start = Num_lines;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		int shipnum;

		// don't process non-ships, or the player ship
		if ( (Game_mode & GM_NORMAL) && (so->objnum == OBJ_INDEX(Player_obj)) )
			continue;

		shipnum = Objects[so->objnum].instance;

		// filter out cargo containers, navbouys, etc, and non-ships
		if ( Ship_info[Ships[shipnum].ship_info_index].class_type < 0 || !(Ship_types[Ship_info[Ships[shipnum].ship_info_index].class_type].hud_bools & STI_HUD_HOTKEY_ON_LIST ))
			continue;

		// don't process ships invisible to sensors, dying or departing
		if ( Ships[shipnum].flags & (SF_HIDDEN_FROM_SENSORS|SF_DYING|SF_DEPARTING) )
			continue;

		// if a ship's hotkey is the last hotkey on the list, then maybe make the hotkey -1 if
		// we are now in mission.  Otherwise, skip this ship
		if ( Ships[shipnum].hotkey == MAX_KEYED_TARGETS ) {
			if ( !(Game_mode & GM_IN_MISSION) )
				continue;										// skip to next ship
			Ships[shipnum].hotkey = -1;
		}

		// be sure this ship isn't in a wing, and that the teams match
		if ( iff_matches_mask(Ships[shipnum].team, team_mask) && (Ships[shipnum].wingnum < 0) ) {
			hotkey_line_add_sorted(Ships[shipnum].ship_name, HOTKEY_LINE_SHIP, shipnum, start);
		}
	}

	for (i=0; i<Num_wings; i++) {
		if (Wings[i].current_count && (Wings[i].ship_index[Wings[i].special_ship] >= 0) && iff_matches_mask(Ships[Wings[i].ship_index[Wings[i].special_ship]].team, team_mask)) {
			// special check for the player's wing.  If he's in a wing, and the only guy left, don't
			// do anything
			if ( (Player_ship->wingnum == i) && (Wings[i].current_count == 1) )
				continue;

			// if a ship's hotkey is the last hotkey on the list, then maybe make the hotkey -1 if
			// we are now in mission.  Otherwise, skip this ship
			if ( Wings[i].hotkey == MAX_KEYED_TARGETS ) {
				if ( !(Game_mode & GM_IN_MISSION) )
					continue;										// skip to next ship
				Wings[i].hotkey = -1;
			}

			// don't add any wing data whose ships are hidden from sensors
			for ( j = 0; j < Wings[i].current_count; j++ ) {
				if ( Ships[Wings[i].ship_index[j]].flags & SF_HIDDEN_FROM_SENSORS )
					break;
			}
			// if we didn't reach the end of the list, don't display the wing
			if ( j < Wings[i].current_count )
				continue;

			z = hotkey_line_add_sorted(Wings[i].name, HOTKEY_LINE_WING, i, start);
			if (Wings[i].flags & WF_EXPANDED) {
				for (j=0; j<Wings[i].current_count; j++) {
					s = Wings[i].ship_index[j];
					z = hotkey_line_insert(z + 1, Ships[s].ship_name, HOTKEY_LINE_SUBSHIP, s);
				}
			}
		}
	}

	for (i=start; i<Num_lines; i++) {
		if (Hotkey_lines[i].type == HOTKEY_LINE_SUBSHIP)
			y += font_height;
		else
			y += font_height + 2;

		Hotkey_lines[i].y = y;
	}

	y += font_height + 8;
	return y;
}

void hotkey_build_listing()
{
	int y, enemy_team_mask;

	Num_lines = y = 0;

	enemy_team_mask = iff_get_attackee_mask(Player_ship->team);

	y = hotkey_build_team_listing(enemy_team_mask, y, false);
	y = hotkey_build_team_listing(enemy_team_mask, y, true);
}

int hotkey_line_query_visible(int n)
{
	int y;

	if ((n < 0) || (n >= Num_lines))
		return 0;
	
	y = Hotkey_lines[n].y - Hotkey_lines[Scroll_offset].y;
	if ((y < 0) || (y + gr_get_font_height() > Hotkey_list_coords[gr_screen.res][3]))
		return 0;

	return 1;
}

void hotkey_scroll_screen_up()
{
	if (Scroll_offset) {
		Scroll_offset--;
		Assert(Selected_line > Scroll_offset);
		while (!hotkey_line_query_visible(Selected_line) || (Hotkey_lines[Selected_line].type == HOTKEY_LINE_HEADING))
			Selected_line--;

		gamesnd_play_iface(SND_SCROLL);

	} else
		gamesnd_play_iface(SND_GENERAL_FAIL);
}

void hotkey_scroll_line_up()
{
	if (Selected_line > 1) {
		Selected_line--;
		while (Hotkey_lines[Selected_line].type == HOTKEY_LINE_HEADING)
			Selected_line--;

		if (Selected_line < Scroll_offset)
			Scroll_offset = Selected_line;

		gamesnd_play_iface(SND_SCROLL);

	} else
		gamesnd_play_iface(SND_GENERAL_FAIL);
}

void hotkey_scroll_screen_down()
{
	if (Hotkey_lines[Num_lines - 1].y + gr_get_font_height() > Hotkey_lines[Scroll_offset].y + Hotkey_list_coords[gr_screen.res][3]) {
		Scroll_offset++;
		while (!hotkey_line_query_visible(Selected_line) || (Hotkey_lines[Selected_line].type == HOTKEY_LINE_HEADING)) {
			Selected_line++;
			Assert(Selected_line < Num_lines);
		}

		gamesnd_play_iface(SND_SCROLL);

	} else
		gamesnd_play_iface(SND_GENERAL_FAIL);
}

void hotkey_scroll_line_down()
{
	if (Selected_line < Num_lines - 1) {
		Selected_line++;
		while (Hotkey_lines[Selected_line].type == HOTKEY_LINE_HEADING)
			Selected_line++;

		Assert(Selected_line > Scroll_offset);
		while (!hotkey_line_query_visible(Selected_line))
			Scroll_offset++;

		gamesnd_play_iface(SND_SCROLL);

	} else
		gamesnd_play_iface(SND_GENERAL_FAIL);
}

void expand_wing()
{
	int i, z;

	if (Hotkey_lines[Selected_line].type == HOTKEY_LINE_WING) {
		i = Hotkey_lines[Selected_line].index;
		Wings[i].flags ^= WF_EXPANDED;
		hotkey_build_listing();
		for (z=0; z<Num_lines; z++)
			if ((Hotkey_lines[z].type == HOTKEY_LINE_WING) && (Hotkey_lines[z].index == i)) {
				Selected_line = z;
				break;
			}
	}
}

void reset_hotkeys()
{
	int i;
	htarget_list *hitem, *plist;

	for (i=0; i<MAX_SHIPS; i++)
		Hotkey_bits[i] = 0;

	for ( i=0; i<MAX_KEYED_TARGETS; i++ ) {
		plist = &(Players[Player_num].keyed_targets[i]);
		if ( EMPTY(plist) ) // no items in list, then do nothing
			continue;

		for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) ) {
			Assert(hitem->objp->type == OBJ_SHIP);
			Hotkey_bits[hitem->objp->instance] |= (1 << i);
		}
	}
}

void clear_hotkeys()
{
	int i, b, z;

	z = Hotkey_lines[Selected_line].type;
	if (z == HOTKEY_LINE_WING) {
		z = Hotkey_lines[Selected_line].index;
		b = ~get_wing_hotkeys(z);
		for (i=0; i<Wings[z].current_count; i++)
			Hotkey_bits[Wings[z].ship_index[i]] &= b;

	} else if ((z == HOTKEY_LINE_SHIP) || (z == HOTKEY_LINE_SUBSHIP)) {
		Hotkey_bits[Hotkey_lines[Selected_line].index] = 0;
	}
}

void save_hotkeys()
{
	ship_obj *so;
	int i;

	for (i=0; i<MAX_KEYED_TARGETS; i++) {
		hud_target_hotkey_clear(i);
		for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
			if ( Hotkey_bits[Objects[so->objnum].instance] & (1 << i) ) {
				hud_target_hotkey_add_remove(i, &Objects[so->objnum], HOTKEY_USER_ADDED );
			}
		}
	}
}

void add_hotkey(int hotkey)
{
	int i, z;

	z = Hotkey_lines[Selected_line].type;
	if (z == HOTKEY_LINE_WING) {
		z = Hotkey_lines[Selected_line].index;
		for (i=0; i<Wings[z].current_count; i++)
			Hotkey_bits[Wings[z].ship_index[i]] |= (1 << hotkey);

	} else if ((z == HOTKEY_LINE_SHIP) || (z == HOTKEY_LINE_SUBSHIP)) {
		Hotkey_bits[Hotkey_lines[Selected_line].index] |= (1 << hotkey);
	}
}

void remove_hotkey()
{
	int i, z;

	z = Hotkey_lines[Selected_line].type;
	if (z == HOTKEY_LINE_WING) {
		z = Hotkey_lines[Selected_line].index;
		for (i=0; i<Wings[z].current_count; i++)
			Hotkey_bits[Wings[z].ship_index[i]] &= ~(1 << Cur_hotkey);

	} else if ((z == HOTKEY_LINE_SHIP) || (z == HOTKEY_LINE_SUBSHIP)) {
		Hotkey_bits[Hotkey_lines[Selected_line].index] &= ~(1 << Cur_hotkey);
	}
}

void hotkey_button_pressed(int n)
{
	switch (n) {
		case SCROLL_UP_BUTTON:
			hotkey_scroll_screen_up();
			break;

		case SCROLL_DOWN_BUTTON:
			hotkey_scroll_screen_down();
			break;

		case ADD_HOTKEY_BUTTON:
			add_hotkey(Cur_hotkey);
			gamesnd_play_iface(SND_USER_SELECT);
			break;

		case REMOVE_HOTKEY_BUTTON:
			remove_hotkey();
			gamesnd_play_iface(SND_USER_SELECT);
			break;

		case ACCEPT_BUTTON:
			save_hotkeys();
			// fall through to CANCEL_BUTTON

		case CANCEL_BUTTON:			
			mission_hotkey_exit();
			gamesnd_play_iface(SND_USER_SELECT);
			break;

		case HELP_BUTTON:
			launch_context_help();
			gamesnd_play_iface(SND_HELP_PRESSED);
			break;

		case OPTIONS_BUTTON:			
			gameseq_post_event(GS_EVENT_OPTIONS_MENU);
			gamesnd_play_iface(SND_USER_SELECT);
			break;

		case CLEAR_BUTTON:
			clear_hotkeys();
			gamesnd_play_iface(SND_USER_SELECT);
			break;

		case RESET_BUTTON:
			reset_hotkeys();
			gamesnd_play_iface(SND_USER_SELECT);
			break;
	}
}

// ---------------------------------------------------------------------
// mission_hotkey_init()
//
// Initialize the hotkey assignment screen system.  Called when GS_STATE_HOTKEY_SCREEN
// is entered.
//
void mission_hotkey_init()
{
	int i;
	hotkey_buttons *b;

	// pause all beam weapon sounds
	beam_pause_sounds();

	// pause all game music
	audiostream_pause_all();

	reset_hotkeys();
	common_set_interface_palette();  // set the interface palette
	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Ui_window.set_mask_bmap(Hotkey_mask_fname[gr_screen.res]);

	for (i=0; i<NUM_BUTTONS; i++) {
		b = &Buttons[gr_screen.res][i];

		b->button.create(&Ui_window, "", b->x, b->y, 60, 30, i < 2 ? 1 : 0, 1);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// add all xstr text
	for(i=0; i<HOTKEY_NUM_TEXT; i++) {
		Ui_window.add_XSTR(&Hotkey_text[gr_screen.res][i]);
	}

	for (i=0; i<LIST_BUTTONS_MAX; i++) {
		List_buttons[i].create(&Ui_window, "", 0, 0, 60, 30, (i < 2), 1);
		List_buttons[i].hide();
		List_buttons[i].disable();
	}

	// set up hotkeys for buttons so we draw the correct animation frame when a key is pressed
	Buttons[gr_screen.res][SCROLL_UP_BUTTON].button.set_hotkey(KEY_PAGEUP);
	Buttons[gr_screen.res][SCROLL_DOWN_BUTTON].button.set_hotkey(KEY_PAGEDOWN);

	// ensure help overlay is off
	help_overlay_set_state(HOTKEY_OVERLAY,0);

	// load in relevant bitmaps
	Background_bitmap = bm_load(Hotkey_background_fname[gr_screen.res]);
	if (Background_bitmap < 0) {
		// bitmap didnt load -- this is bad
		Int3();
	}
	Wing_bmp = bm_load("WingDesignator");
	if (Wing_bmp < 0) {
		// bitmap didnt load -- this is bad
		Int3();
	}

	Scroll_offset = 0;
	Selected_line = 1;
	hotkey_build_listing();
}

// ---------------------------------------------------------------------
// mission_hotkey_close()
//
// Cleanup the hotkey assignment screen system.  Called when GS_STATE_HOTKEY_SCREEN
// is left.
//
void mission_hotkey_close()
{
	if (Background_bitmap)
		bm_release(Background_bitmap);
	if (Wing_bmp >= 0)
		bm_release(Wing_bmp);

	// unload the overlay bitmap
//	help_overlay_unload(HOTKEY_OVERLAY);

	// unpause all beam weapon sounds
	beam_unpause_sounds();

	// unpause all game music
	audiostream_unpause_all();

	Ui_window.destroy();
	common_free_interface_palette();		// restore game palette
	game_flush();
}

// ---------------------------------------------------------------------
// mission_hotkey_do_frame()
//
// Called once per frame to process user input for the Hotkey Assignment Screen
//
void mission_hotkey_do_frame(float frametime)
{
	char buf[256];
	int i, k, w, h, y, z, line, hotkeys;
	int font_height = gr_get_font_height();
	int select_tease_line = -1;  // line mouse is down on, but won't be selected until button released
	color circle_color;

	if ( help_overlay_active(HOTKEY_OVERLAY) ) {
		Buttons[gr_screen.res][HELP_BUTTON].button.reset_status();
		Ui_window.set_ignore_gadgets(1);
	}

	k = Ui_window.process() & ~KEY_DEBUGGED;

	if ( (k > 0) || B1_JUST_RELEASED ) {
		if ( help_overlay_active(HOTKEY_OVERLAY) ) {
			help_overlay_set_state(HOTKEY_OVERLAY, 0);
			Ui_window.set_ignore_gadgets(0);
			k = 0;
		}
	}

	if ( !help_overlay_active(HOTKEY_OVERLAY) ) {
		Ui_window.set_ignore_gadgets(0);
	}

	switch (k) {
		case KEY_DOWN:  // scroll list down
			hotkey_scroll_line_down();
			break;

		case KEY_UP:  // scroll list up
			hotkey_scroll_line_up();
			break;

		case KEY_PAGEDOWN:  // scroll list down
			hotkey_scroll_screen_down();
			break;

		case KEY_PAGEUP:  // scroll list up
			hotkey_scroll_screen_up();
			break;

		case KEY_CTRLED | KEY_ENTER:
			save_hotkeys();
			// fall through to next state -- allender changed this behavior since ESC should always cancel, no?

		case KEY_ESC:			
			mission_hotkey_exit();
			break;

		case KEY_TAB:
		case KEY_ENTER:
		case KEY_PADENTER:
			expand_wing();
			break;

		case KEY_EQUAL:
		case KEY_PADPLUS:
			add_hotkey(Cur_hotkey);
			break;

		case KEY_MINUS:
		case KEY_PADMINUS:
			remove_hotkey();
			break;

		case KEY_F2:			
			gameseq_post_event(GS_EVENT_OPTIONS_MENU);			
			break;

		case KEY_CTRLED | KEY_R:
			reset_hotkeys();
			break;

		case KEY_CTRLED | KEY_C:
			clear_hotkeys();
			break;
	}	// end switch

	// ?
	for (i=0; i<MAX_KEYED_TARGETS; i++) {
		if (k == Key_sets[i])
			Cur_hotkey = i;

		if (k == (Key_sets[i] | KEY_SHIFTED))
			add_hotkey(i);
	}

	// handle pressed buttons
	for (i=0; i<NUM_BUTTONS; i++) {
		if (Buttons[gr_screen.res][i].button.pressed()) {
			hotkey_button_pressed(i);
			break;					// only need to handle 1 button @ a time
		}
	}

	for (i=0; i<LIST_BUTTONS_MAX; i++) {
		// check for tease line
		if (List_buttons[i].button_down()) {
			select_tease_line = i + Scroll_offset;
		}
	
		// check for selected list item
		if (List_buttons[i].pressed()) {
			Selected_line = i + Scroll_offset;
			List_buttons[i].get_mouse_pos(&z, NULL);
			z += Hotkey_list_coords[gr_screen.res][0];		// adjust to full screen space
			if ((z >= Hotkey_wing_icon_x[gr_screen.res]) && (z < (Hotkey_wing_icon_x[gr_screen.res]) + Hotkey_function_field_width[gr_screen.res])) {
				expand_wing();
			}
		}

		if (List_buttons[i].double_clicked()) {
			Selected_line = i + Scroll_offset;
			hotkeys = -1;
			switch (Hotkey_lines[Selected_line].type) {
				case HOTKEY_LINE_WING:
					hotkeys = get_wing_hotkeys(Hotkey_lines[Selected_line].index);
					break;

				case HOTKEY_LINE_SHIP:
				case HOTKEY_LINE_SUBSHIP:
					hotkeys = Hotkey_bits[Hotkey_lines[Selected_line].index];
					break;
			}

			if (hotkeys != -1) {
				if (hotkeys & (1 << Cur_hotkey))
					remove_hotkey();
				else
					add_hotkey(Cur_hotkey);
			}
		}
	}

	if (Background_bitmap >= 0) {
		gr_set_bitmap(Background_bitmap);
		gr_bitmap(0, 0);

	} else
		gr_clear();

	Ui_window.draw();
	gr_init_color(&circle_color, 160, 160, 0);

	// draw the big "F10" in the little box	
	gr_set_font(FONT2);
	gr_set_color_fast(&Color_text_normal);
	strcpy_s(buf, Scan_code_text[Key_sets[Cur_hotkey]]);
	gr_get_string_size(&w, &h, buf);
	gr_printf(Hotkey_function_name_coords[gr_screen.res][0] + (Hotkey_function_name_coords[gr_screen.res][2] - w) / 2, Hotkey_function_name_coords[gr_screen.res][1], buf);

	gr_set_font(FONT1);
	line = Scroll_offset;
	while (hotkey_line_query_visible(line)) {
		z = Hotkey_lines[line].index;
		y = Hotkey_list_coords[gr_screen.res][1] + Hotkey_lines[line].y - Hotkey_lines[Scroll_offset].y;
		hotkeys = 0;
		switch (Hotkey_lines[line].type) {
			case HOTKEY_LINE_HEADING:
				gr_set_color_fast(&Color_text_heading);

				gr_get_string_size(&w, &h, Hotkey_lines[line].label);
				i = y + h / 2 - 1;
				gr_line(Hotkey_list_coords[gr_screen.res][0], i, Hotkey_ship_x[gr_screen.res] - 2, i);
				gr_line(Hotkey_ship_x[gr_screen.res] + w + 1, i, Hotkey_list_coords[gr_screen.res][0] + Hotkey_list_coords[gr_screen.res][2], i);
				break;

			case HOTKEY_LINE_WING:
				gr_set_bitmap(Wing_bmp);
				bm_get_info(Wing_bmp, NULL, &h, NULL);
				i = y + font_height / 2 - h / 2 - 1;
				gr_bitmap(Hotkey_wing_icon_x[gr_screen.res], i);

//				i = y + font_height / 2 - 1;
//				gr_set_color_fast(&circle_color);
//				gr_circle(ICON_LIST_X + 4, i, 5);

//				gr_set_color_fast(&Color_bright);
//				gr_line(ICON_LIST_X, i, ICON_LIST_X + 2, i);
//				gr_line(ICON_LIST_X + 4, i - 4, ICON_LIST_X + 4, i - 2);
//				gr_line(ICON_LIST_X + 6, i, ICON_LIST_X + 8, i);
//				gr_line(ICON_LIST_X + 4, i + 2, ICON_LIST_X + 4, i + 4);

				hotkeys = get_wing_hotkeys(Hotkey_lines[line].index);
				break;

			case HOTKEY_LINE_SHIP:
			case HOTKEY_LINE_SUBSHIP:
				hotkeys = Hotkey_bits[Hotkey_lines[line].index];
				break;

			default:
				Int3();
		}

		if (Hotkey_lines[line].type != HOTKEY_LINE_HEADING) {
			Assert( (line - Scroll_offset) < LIST_BUTTONS_MAX );
			List_buttons[line - Scroll_offset].update_dimensions(Hotkey_list_coords[gr_screen.res][0], y, Hotkey_list_coords[gr_screen.res][0] + Hotkey_list_coords[gr_screen.res][2] - Hotkey_list_coords[gr_screen.res][0], font_height);
			List_buttons[line - Scroll_offset].enable();
			if (hotkeys & (1 << Cur_hotkey)) {
				gr_set_color_fast(&Color_text_active);

			} else {
				if (line == Selected_line)
					gr_set_color_fast(&Color_text_selected);
				else if (line == select_tease_line)
					gr_set_color_fast(&Color_text_subselected);
				else
					gr_set_color_fast(&Color_text_normal);
			}

		} else {
			Assert( (line - Scroll_offset) < LIST_BUTTONS_MAX );
			List_buttons[line - Scroll_offset].disable();
		}

		// print active hotkeys associated for this line
		if (hotkeys) {
			for (i=0; i<MAX_KEYED_TARGETS; i++) {
				if (hotkeys & (1 << i)) {
					gr_printf(Hotkey_list_coords[gr_screen.res][0] + Hotkey_function_field_width[gr_screen.res]*i, y, Scan_code_text[Key_sets[i]]);
				}
			}
/*
			*buf = 0;
			for (i=0; i<MAX_KEYED_TARGETS; i++) {
				if (hotkeys & (1 << i)) {
					strcat_s(buf, Scan_code_text[Key_sets[i]]);
					strcat_s(buf, ", ");
				}
			}

			Assert(strlen(buf) > 1);
			buf[strlen(buf) - 2] = 0;  // lose the ", " on the end

			gr_force_fit_string(buf, 255, GROUP_LIST_W);
			gr_printf(GROUP_LIST_X, y, buf);*/
		}
	
		// draw ship/wing name
		strcpy_s(buf, Hotkey_lines[line].label);
		end_string_at_first_hash_symbol(buf);
		if (Hotkey_lines[line].type == HOTKEY_LINE_SUBSHIP) {
			// indent
			gr_force_fit_string(buf, 255, Hotkey_list_coords[gr_screen.res][0] + Hotkey_list_coords[gr_screen.res][2] - (Hotkey_ship_x[gr_screen.res]+20));
			gr_printf(Hotkey_ship_x[gr_screen.res]+20, y, buf);
		} else {
			gr_force_fit_string(buf, 255, Hotkey_list_coords[gr_screen.res][0] + Hotkey_list_coords[gr_screen.res][2] - Hotkey_ship_x[gr_screen.res]);
			gr_printf(Hotkey_ship_x[gr_screen.res], y, buf);
		}

		line++;
	}

	i = line - Scroll_offset;
	while (i < LIST_BUTTONS_MAX)
		List_buttons[i++].disable();

	// blit help overlay if active
	help_overlay_maybe_blit(HOTKEY_OVERLAY);

	gr_flip();
}

void mission_hotkey_exit()
{
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}
