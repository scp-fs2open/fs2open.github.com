/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef _GLOBAL_ALPHACOLORS_HEADER_FILE
#define _GLOBAL_ALPHACOLORS_HEADER_FILE

#include "graphics/2d.h"
#include "parse/parselo.h"
#include "globalincs/def_files.h"

// -----------------------------------------------------------------------------------
// ALPHA DEFINES/VARS
//

// Colors for UI
// See FreeSpace.cpp for usage

#define INTERFACE_COLORS 12
extern color Color_text_normal, Color_text_subselected, Color_text_selected;
extern color Color_text_error, Color_text_error_hi, Color_text_active, Color_text_active_hi;
extern color Color_text_heading, Color_more_indicator, Color_more_bright, Color_bright, Color_normal;

#define TOTAL_COLORS 22
extern color Color_blue, Color_bright_blue, Color_green, Color_bright_green;
extern color Color_black, Color_grey, Color_silver, Color_white, Color_bright_white;
extern color Color_violet_gray, Color_violet, Color_pink, Color_light_pink;
extern color Color_dim_red, Color_red, Color_bright_red, Color_yellow, Color_bright_yellow;

extern color Color_ui_light_green, Color_ui_green;
extern color Color_ui_light_pink, Color_ui_pink;

// briefing hostile/friendly/neutral colors
extern color Brief_color_red, Brief_color_green, Brief_color_legacy_neutral;

// netplayer colors
#define NETPLAYER_COLORS 20
extern color *Color_netplayer[NETPLAYER_COLORS];

// Team colors
extern SCP_map<SCP_string, team_color> Team_Colors;
extern SCP_vector<SCP_string> Team_Names;

extern SCP_map<char, color*> Tagged_Colors;
extern SCP_vector<char> Color_Tags;

#define MAX_DEFAULT_TEXT_COLORS	7
extern char default_fiction_viewer_color;
extern char default_command_briefing_color;
extern char default_briefing_color;
extern char default_redalert_briefing_color;
extern char default_debriefing_color;
extern char default_recommendation_color;
extern char default_loop_briefing_color;

// -----------------------------------------------------------------------------------
// ALPHA FUNCTIONS
//

//initialize alpha colors based on colors.tbl
void alpha_colors_init();

#endif
