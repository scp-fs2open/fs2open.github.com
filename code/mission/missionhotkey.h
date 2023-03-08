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
#define HOTKEY_LINE_HEADING 1
#define HOTKEY_LINE_WING 2
#define HOTKEY_LINE_SHIP 3
#define HOTKEY_LINE_SUBSHIP 4 // ship that is in a wing

struct hotkey_line {
	SCP_string label;
	int type; // type 0 is an unused line
	int index;
	int y; // Y coordinate of line
};

extern hotkey_line Hotkey_lines[MAX_LINES];

void mission_hotkey_init();
void mission_hotkey_close();
void mission_hotkey_do_frame(float frametime);
void mission_hotkey_set_defaults();
void mission_hotkey_validate();
void mission_hotkey_maybe_save_sets();
void mission_hotkey_reset_saved();
void mission_hotkey_mf_add( int set, int objnum, int how_to_add );

void mission_hotkey_exit();

// function to build the hotkey listing
void hotkey_build_listing();

// function to set the selected hotkey line
void hotkey_set_selected_line(int line);

// function to expand a wing in the hotkey list
// uses the Selected_line var, so set that first
void expand_wing();

// function to return the hotkeys for a wing
extern int get_wing_hotkeys(int n);

// function to return the hotkeys for a ship
extern int get_ship_hotkeys(int n);

// function to return the hotkey set number of the given key
extern int mission_hotkey_get_set_num( int k );

extern int Hotkey_overlay_id;

#endif
