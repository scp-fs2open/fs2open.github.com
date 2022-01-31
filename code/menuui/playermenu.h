/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _PLAYER_SELECT_MENU_HEADER_FILE
#define _PLAYER_SELECT_MENU_HEADER_FILE

#include "globalincs/pstypes.h"

// general defines
#define PLAYER_SELECT_MODE_SINGLE	0							// looking through single player pilots
#define PLAYER_SELECT_MODE_MULTI    1							// looking through multi player pilots

// flag indicating if this is the absolute first pilot created and selected. Used to determine
// if the main hall should display the help overlay screen
extern int Player_select_very_first_pilot;			

// functions for selecting single/multiplayer pilots at the very beginning of FreeSpace
void player_select_init();
void player_select_do();

/**
 * Playermenu closing handler
 * 
 * @details Called by game_leave_state when leaving GS_STATE_INITIAL_PLAYER_SELECT, finalizes player selection and
 * prepares to enter the mainhall.
 */
void player_select_close();

// function to check whether we found a "last pilot". loads this pilot in if possible and returns true, or false otherwise
int player_select_get_last_pilot();

// tooltips
void player_tips_init();
void player_tips_close();
void player_tips_popup();
void player_tips_close();

// quick check to make sure we always load default campaign savefile values when loading from the pilot
// select screen but let us not overwrite current values with defaults when we aren't - taylor
extern int Player_select_screen_active;

// check the pilots language
bool valid_pilot_lang(const char *callsign);

SCP_vector<SCP_string> player_select_enumerate_pilots();

SCP_string player_get_last_player();

void player_finish_select(const char* callsign, bool is_multi);

bool player_create_new_pilot(const char* callsign, bool is_multi, const char* copy_from_callsign = nullptr);

#endif
