/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __MISSIONHOTKEY_H__
#define __MISSIONHOTKEY_H__

#include "globalincs/globals.h"

#define MAX_LINES MAX_SHIPS // retail was 200, bump it to match MAX_SHIPS

// Types of items that can be in the hotkey list
enum class HotkeyLineType
{
	NONE = 0,
	HEADING,
	WING,
	SHIP,
	SUBSHIP,
};

struct hotkey_line {
	SCP_string label;
	HotkeyLineType type; // NONE is an unused line
	int index;
	int y; // Y coordinate of line
};

extern hotkey_line Hotkey_lines[MAX_LINES];

void mission_hotkey_init();
void mission_hotkey_close();
void mission_hotkey_do_frame(float frametime);
void mission_hotkey_set_defaults(bool restore = true);
void mission_hotkey_validate();
void mission_hotkey_maybe_save_sets();
void mission_hotkey_reset_saved();
void mission_hotkey_mf_add( int set, int objnum, int how_to_add );

void mission_hotkey_exit();

// function to reset the hotkey list
void hotkey_lines_reset_all();

// function to build the hotkey listing
void hotkey_build_listing();

// function to set the selected hotkey line
void hotkey_set_selected_line(int line);

// function to expand a wing in the hotkey list and sets Selected_line to the first ship in the wing
void expand_wing(int line, bool forceExpand = false);

// function to return the hotkeys for a wing
extern int get_wing_hotkeys(int n);

// function to return the hotkeys for a ship
extern int get_ship_hotkeys(int n);

// function to add a hotkey to a hotkey line
void add_hotkey(int hotkey, int line);

// function to remove a hotkey from a hotkey line
void remove_hotkey(int hotkey, int line);

// function to clear all hotkeys from a hotkey line
void clear_hotkeys(int line);

// function to reset the mission hotkeys for this specific session in the ui
// does not reset to mission defaults!
void reset_hotkeys();

// function to save hotkey changes
void save_hotkeys();

// function to return the hotkey set number of the given key
extern int mission_hotkey_get_set_num( int k );

extern int Hotkey_overlay_id;

#endif
