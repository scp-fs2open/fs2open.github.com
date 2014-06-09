/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#include <string.h>
#include <setjmp.h>

#include "gamehelp/contexthelp.h"
#include "gamesequence/gamesequence.h"
#include "menuui/mainhallmenu.h"
#include "missionui/missionbrief.h"
#include "missionui/missionshipchoice.h"
#include "missionui/missionweaponchoice.h"
#include "menuui/barracks.h"
#include "controlconfig/controlsconfig.h"
#include "missionui/missiondebrief.h"
#include "network/multiui.h"
#include "mission/missionhotkey.h"
#include "menuui/readyroom.h"
#include "menuui/techmenu.h"
#include "missionui/missioncmdbrief.h"
#include "graphics/2d.h"
#include "parse/parselo.h"
#include "localization/localize.h"
#include "globalincs/alphacolors.h"
#include "globalincs/systemvars.h"
#include "debugconsole/console.h"



////////////////////////////////////////////////////////////////////
// private function prototypes / structs
////////////////////////////////////////////////////////////////////
void parse_helptbl(const char *filename);
void help_overlay_blit(int overlay_id);
void help_overlay_init();


typedef struct {
	int x_begin, y_begin, x_end, y_end;
} help_line;

typedef struct {
	vec3d vtx[HELP_MAX_PLINE_VERTICES];
	vec3d *pvtx[HELP_MAX_PLINE_VERTICES];
	int vtxcount;
} help_pline;

typedef struct {
	int x_coord, y_coord;
	char* string;
} help_text;

typedef struct {
	int x_coord, y_coord;
} help_left_bracket;

typedef struct {
	int x_coord, y_coord;
} help_right_bracket;

typedef struct {
	char name[HELP_MAX_NAME_LENGTH];
	help_pline				plinelist[GR_NUM_RESOLUTIONS][HELP_MAX_ITEM];
	help_text				textlist[GR_NUM_RESOLUTIONS][HELP_MAX_ITEM];
	help_left_bracket		lbracketlist[GR_NUM_RESOLUTIONS][HELP_MAX_ITEM];
	help_right_bracket	rbracketlist[GR_NUM_RESOLUTIONS][HELP_MAX_ITEM];
	int plinecount;
	int textcount;
	int lbracketcount;
	int rbracketcount;
} help_overlay;

////////////////////////////////////////////////////////////////////
// Game-wide globals
////////////////////////////////////////////////////////////////////
shader Grey_shader;

////////////////////////////////////////////////////////////////////
// Module globals
////////////////////////////////////////////////////////////////////
static int help_left_bracket_bitmap;
static int help_right_bracket_bitmap;
static help_overlay help_overlaylist[MAX_HELP_OVERLAYS];
int num_help_overlays;

static int current_helpid = -1;		// the currently active overlay_id, only really used for the debug console funxions
int Help_overlay_flags;
static int Source_game_state;			// state from where F1 was pressed

////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////


int help_overlay_get_index(const char* overlay_name)
{
	for (int i = 0; i < num_help_overlays; i++) {
		if (!stricmp(overlay_name, help_overlaylist[i].name)) {
			return i;
		}
	}

	return -1;
}

// query whether a help overlay is active (ie being displayed)
int help_overlay_active(int overlay_id)
{
	Assert(overlay_id < MAX_HELP_OVERLAYS);

	if (overlay_id < 0) {
		return 0;
	}

	return Help_overlay_flags & (1<<overlay_id);
}

// stop displaying a help overlay
void help_overlay_set_state(int overlay_id, int state)
{
	Assert(overlay_id < MAX_HELP_OVERLAYS);

	if (overlay_id >= 0 && overlay_id < num_help_overlays) {
		if ( state > 0 ) {
			Help_overlay_flags |= (1<<overlay_id);
			current_helpid = overlay_id;
		} else {
			Help_overlay_flags &= ~(1<<overlay_id);
			//current_helpid = -1;
		}
	}

}

// maybe blit a bitmap of a help overlay to the screen
void help_overlay_maybe_blit(int overlay_id)
{
	Assert(overlay_id < MAX_HELP_OVERLAYS);

	if ( overlay_id >= 0 && (Help_overlay_flags & (1<<overlay_id)) ) {
		context_help_grey_screen();
		help_overlay_blit(overlay_id);				
	}
}

// reset the flags for the help overlays
void help_overlay_reset_all()
{
	Help_overlay_flags = 0;
}

// Set up Grey_shader, which is used game-wide to grey out background when using help overlays
void create_grey_shader()
{
	/*float tmp,c;

	tmp = 0.4f/3.0f;

	// The c matrix brightens everything a bit.
//	c = 0.125f;
	c = 0.110f;*/

	gr_create_shader( &Grey_shader, 34, 34, 34, 168 );
}

// called at game startup to init all help related data
void context_help_init() 
{
	create_grey_shader();
	help_overlay_reset_all();
	help_overlay_init();
}

void context_help_grey_screen()
{
	gr_set_shader(&Grey_shader);
	gr_shade(0,0,gr_screen.clip_width, gr_screen.clip_height, GR_RESIZE_NONE);
}

// launch_context_help() will switch to a context sensitive help state
void launch_context_help()
{
	int overlay_id = -1;

	// look at the state the game was in when F1 was pressed
	Source_game_state = gameseq_get_state();

	switch (Source_game_state) {

		case GS_STATE_MAIN_MENU:
			overlay_id = main_hall_get_overlay_id();
			break;

		case GS_STATE_GAME_PLAY:
		case GS_STATE_GAME_PAUSED:
		case GS_STATE_TRAINING_PAUSED:
			gameseq_post_event(GS_EVENT_GAMEPLAY_HELP);
			break;

		case GS_STATE_BRIEFING:
			overlay_id = Briefing_overlay_id;
			break;

		case GS_STATE_SHIP_SELECT:
			overlay_id = Ship_select_overlay_id;
			break;

		case GS_STATE_WEAPON_SELECT:
			overlay_id = Weapon_select_overlay_id;
			break;

		case GS_STATE_BARRACKS_MENU:
			overlay_id = Barracks_overlay_id;
			break;

		case GS_STATE_CONTROL_CONFIG:
			overlay_id = Control_config_overlay_id;
			break;

		case GS_STATE_DEBRIEF:
			overlay_id = Debrief_overlay_id;
			break;

		case GS_STATE_MULTI_HOST_SETUP:
			overlay_id = Multi_create_overlay_id;
			break;

		case GS_STATE_MULTI_START_GAME:
			overlay_id = Multi_sg_overlay_id;
			break;

		case GS_STATE_MULTI_JOIN_GAME:
			overlay_id = Multi_join_overlay_id;
			break;

		case GS_STATE_HOTKEY_SCREEN:
			overlay_id = Hotkey_overlay_id;
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			overlay_id = Campaign_room_overlay_id;
			break;

		case GS_STATE_SIMULATOR_ROOM:
			overlay_id = Sim_room_overlay_id;
			break;

		case GS_STATE_TECH_MENU:
			overlay_id = Techroom_overlay_id;
			break;

		case GS_STATE_CMD_BRIEF:
			overlay_id = Cmd_brief_overlay_id;
			break;

		default:
			nprintf(("Warning","WARNING ==> There is no context help available for state %s\n", GS_state_text[Source_game_state-1]));
			break;

	} // end switch

	if (overlay_id >= 0) {
		if ( !help_overlay_active(overlay_id) ) {
			help_overlay_set_state(overlay_id, 1);
		}
		else {
			help_overlay_set_state(overlay_id, 0);
		}
	}
}

void close_help(){
	for (int overlay_id=0; overlay_id<MAX_HELP_OVERLAYS; overlay_id++){
		for(int i = 0; i<HELP_MAX_ITEM; i++)
		safe_kill(help_overlaylist[overlay_id].textlist[GR_640][i].string);
	}
}

// Called once at the beginning of the game to load help bitmaps & data
void help_overlay_init() 
{
	// load right_bracket bitmap
	help_right_bracket_bitmap = bm_load("right_bracket");
	// we failed to load the bitmap - this is very bad
	Assertion( help_right_bracket_bitmap >= 0, "Failed to load bitmap right_bracket for help overlay\n");

	// load left_bracket bitmap
	help_left_bracket_bitmap = bm_load("left_bracket");
	// we failed to load the bitmap - this is very bad
	Assertion( help_left_bracket_bitmap >= 0, "Failed to load bitmap left_bracket for help overlay\n");

	atexit(close_help);

	num_help_overlays = 0;

	// parse help.tbl
	parse_helptbl(HELP_OVERLAY_FILENAME);

	// look for any modular tables
	parse_modular_table(NOX("*-hlp.tbm"), parse_helptbl);
}

// parses help.tbl and populates help_overlaylist[]
void parse_helptbl(const char *filename)
{
	int overlay_id, currcount;
	char name[HELP_MAX_NAME_LENGTH];
	char buf[HELP_MAX_STRING_LENGTH + 1];
	int i, rval;

	// open localization
	lcl_ext_open();
	
	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		lcl_ext_close();
		return;
	} 

	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();

	// for each overlay...
	while (optional_string("$")) {

		stuff_string(name, F_NAME, HELP_MAX_NAME_LENGTH);

		overlay_id = help_overlay_get_index(name);

		if (overlay_id < 0) {
			if (num_help_overlays >= MAX_HELP_OVERLAYS) {
				Warning(LOCATION, "Could not load help overlay after '%s' as maximum number of help overlays was reached (Max is %d)", help_overlaylist[overlay_id - 1].name, MAX_HELP_OVERLAYS);

				if (!skip_to_string("$end")) {
					Error(LOCATION, "Couldn't find $end. Help.tbl or -hlp.tbm is invalid.\n");
				}

				continue;
			} else {
				overlay_id = num_help_overlays;
				strcpy_s(help_overlaylist[overlay_id].name, name);
				num_help_overlays++;
			}
		}

		// clear out counters in the overlay struct
		help_overlaylist[overlay_id].plinecount = 0;
		help_overlaylist[overlay_id].textcount = 0;
		help_overlaylist[overlay_id].rbracketcount = 0;
		help_overlaylist[overlay_id].lbracketcount = 0;
		
		// read in all elements for this overlay
		while (!(optional_string("$end")))  {

			if (optional_string("+pline")) {

				currcount = help_overlaylist[overlay_id].plinecount;
				int a, b;		// temp vars to read in int before cast to float;

				if (currcount < HELP_MAX_ITEM) {
					// read number of pline vertices
					stuff_int(&help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtxcount);		// note that it is read into GR_640
					// help_overlaylist[overlay_id].plinelist[GR_1024][currcount].vtxcount = help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtxcount;			// set equal to 1024 version vertex count to prevent bugs
					Assert(help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtxcount <= HELP_MAX_PLINE_VERTICES);
					// get 640x480 vertex coordinates
					for (i=0; i<help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtxcount; i++) {
						stuff_int(&a);
						stuff_int(&b);
						help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtx[i].xyz.x = (float)a;
						help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtx[i].xyz.y = (float)b;
						help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtx[i].xyz.z = 0.0f;
						help_overlaylist[overlay_id].plinelist[GR_640][currcount].pvtx[i] = &help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtx[i];
					}
					// get 1024x768 vertex coordinates
					for (i=0; i<help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtxcount; i++) {
						stuff_int(&a);
						stuff_int(&b);
						help_overlaylist[overlay_id].plinelist[GR_1024][currcount].vtx[i].xyz.x = (float)a;
						help_overlaylist[overlay_id].plinelist[GR_1024][currcount].vtx[i].xyz.y = (float)b;
						help_overlaylist[overlay_id].plinelist[GR_1024][currcount].vtx[i].xyz.z = 0.0f;
						help_overlaylist[overlay_id].plinelist[GR_1024][currcount].pvtx[i] = &help_overlaylist[overlay_id].plinelist[GR_1024][currcount].vtx[i];
					}
				}

				//mprintf(("Found pline - start location (%f,%f), end location (%f,%f)\n", help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtx[0].xyz.x, help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtx[0].xyz.y, help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtx[2].xyz.x, help_overlaylist[overlay_id].plinelist[GR_640][currcount].vtx[2].xyz.y));
				help_overlaylist[overlay_id].plinecount++;

			} else if (optional_string("+text")) {

				currcount = help_overlaylist[overlay_id].textcount;

				if (currcount < HELP_MAX_ITEM) {
					// get 640x480 coordinates
					stuff_int(&(help_overlaylist[overlay_id].textlist[GR_640][currcount].x_coord));
					stuff_int(&(help_overlaylist[overlay_id].textlist[GR_640][currcount].y_coord));
					// get 1024x768 coordinates
					stuff_int(&(help_overlaylist[overlay_id].textlist[GR_1024][currcount].x_coord));
					stuff_int(&(help_overlaylist[overlay_id].textlist[GR_1024][currcount].y_coord));

					// get string (always use the GR_640 one)
					stuff_string(buf, F_MESSAGE, sizeof(buf));
					help_overlaylist[overlay_id].textlist[GR_640][currcount].string = vm_strdup(buf);

					//mprintf(("Found text %d on overlay %d - location (%d,%d) @ 640x480 :: location (%d,%d) @ 1024x768\n", currcount, overlay_id, help_overlaylist[overlay_id].textlist[GR_640][currcount].x_coord, help_overlaylist[overlay_id].textlist[GR_640][currcount].y_coord, help_overlaylist[overlay_id].textlist[GR_1024][currcount].x_coord, help_overlaylist[overlay_id].textlist[GR_1024][currcount].x_coord));
					help_overlaylist[overlay_id].textcount++;
				}

			} else if (optional_string("+right_bracket")) {

				currcount = help_overlaylist[overlay_id].rbracketcount;

				if (currcount < HELP_MAX_ITEM) {
					// get 640x480 coordinates
					stuff_int(&(help_overlaylist[overlay_id].rbracketlist[GR_640][currcount].x_coord));
					stuff_int(&(help_overlaylist[overlay_id].rbracketlist[GR_640][currcount].y_coord));
					// get 1024x768 coordinates
					stuff_int(&(help_overlaylist[overlay_id].rbracketlist[GR_1024][currcount].x_coord));
					stuff_int(&(help_overlaylist[overlay_id].rbracketlist[GR_1024][currcount].y_coord));

					//mprintf(("Found rbracket %d on overlay %d - location (%d,%d) @ 640x480 :: location (%d,%d) @ 1024x768\n", currcount, overlay_id, help_overlaylist[overlay_id].rbracketlist[GR_640][currcount].x_coord, help_overlaylist[overlay_id].rbracketlist[GR_640][currcount].y_coord, help_overlaylist[overlay_id].rbracketlist[GR_1024][currcount].x_coord, help_overlaylist[overlay_id].rbracketlist[GR_1024][currcount].y_coord));
					help_overlaylist[overlay_id].rbracketcount++;
				}

			} else if (optional_string("+left_bracket")) {

				currcount = help_overlaylist[overlay_id].lbracketcount;

				if (currcount < HELP_MAX_ITEM) {
					// get 640x480 coordinates
					stuff_int(&(help_overlaylist[overlay_id].lbracketlist[GR_640][currcount].x_coord));
					stuff_int(&(help_overlaylist[overlay_id].lbracketlist[GR_640][currcount].y_coord));
					// get 1024x768 coordinates
					stuff_int(&(help_overlaylist[overlay_id].lbracketlist[GR_1024][currcount].x_coord));
					stuff_int(&(help_overlaylist[overlay_id].lbracketlist[GR_1024][currcount].y_coord));

					//mprintf(("Found lbracket %d on overlay %d - location (%d,%d) @ 640x480 :: location (%d,%d) @ 1024x768\n", currcount, overlay_id, help_overlaylist[overlay_id].lbracketlist[GR_640][currcount].x_coord, help_overlaylist[overlay_id].lbracketlist[GR_640][currcount].y_coord, help_overlaylist[overlay_id].lbracketlist[GR_1024][currcount].x_coord, help_overlaylist[overlay_id].lbracketlist[GR_1024][currcount].y_coord));
					help_overlaylist[overlay_id].lbracketcount++;
				}

			} else {
				// help.tbl is corrupt
				Assert(0);

			}		// end if

		}		// end while
	}		// end while

	// close localization
	lcl_ext_close();
}



// draw overlay on the screen
void help_overlay_blit(int overlay_id) 
{
	int idx, width, height;
	int plinecount = help_overlaylist[overlay_id].plinecount;
	int textcount = help_overlaylist[overlay_id].textcount;
	int rbracketcount = help_overlaylist[overlay_id].rbracketcount;
	int lbracketcount = help_overlaylist[overlay_id].lbracketcount;

	Assert(overlay_id >= 0 && overlay_id < MAX_HELP_OVERLAYS);

	// this draws each line of help text with white on black text (use the GR_640 index for the string)
	for (idx = 0; idx < textcount; idx++) {
		gr_set_color_fast(&Color_black);
		gr_get_string_size(&width, &height, help_overlaylist[overlay_id].textlist[GR_640][idx].string, strlen(help_overlaylist[overlay_id].textlist[GR_640][idx].string));
		gr_rect(help_overlaylist[overlay_id].textlist[gr_screen.res][idx].x_coord-2*HELP_PADDING, help_overlaylist[overlay_id].textlist[gr_screen.res][idx].y_coord-3*HELP_PADDING, width+4*HELP_PADDING, height+4*HELP_PADDING, GR_RESIZE_MENU);
		gr_set_color_fast(&Color_bright_white);
		gr_printf_menu(help_overlaylist[overlay_id].textlist[gr_screen.res][idx].x_coord, help_overlaylist[overlay_id].textlist[gr_screen.res][idx].y_coord, help_overlaylist[overlay_id].textlist[GR_640][idx].string);
	}

	// this draws each right bracket
	for (idx = 0; idx < rbracketcount; idx++) {
		gr_set_bitmap(help_right_bracket_bitmap);
		gr_bitmap(help_overlaylist[overlay_id].rbracketlist[gr_screen.res][idx].x_coord, help_overlaylist[overlay_id].rbracketlist[gr_screen.res][idx].y_coord, GR_RESIZE_MENU);
	}

	// this draws each left bracket
	for (idx = 0; idx < lbracketcount; idx++) {
		gr_set_bitmap(help_left_bracket_bitmap);
		gr_bitmap(help_overlaylist[overlay_id].lbracketlist[gr_screen.res][idx].x_coord, help_overlaylist[overlay_id].lbracketlist[gr_screen.res][idx].y_coord, GR_RESIZE_MENU);
	}	

	// this draws each 2d line for the help screen
	//gr_set_color_fast(&Color_yellow);
	gr_set_color(255, 255, 0);
	for (idx = 0; idx<plinecount; idx++) {
		gr_pline_special(help_overlaylist[overlay_id].plinelist[gr_screen.res][idx].pvtx	, help_overlaylist[overlay_id].plinelist[GR_640][idx].vtxcount, HELP_PLINE_THICKNESS, GR_RESIZE_MENU);
	}
}


// --------------------------------------------------
// DEBUGGING STUFF
// --------------------------------------------------
// z64: These DCF's really need a do-over.
DCF(help_reload, "Reloads help overlay data from help.tbl")
{
	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: sample\nCrashes your machine.\n" );
	}

	num_help_overlays = 0;
	parse_helptbl(HELP_OVERLAY_FILENAME);
	parse_modular_table(NOX("*-hlp.tbm"), parse_helptbl);
}

int h_textnum=0, h_amt=0, h_vtx = 0;

void nudgetext_x(int textnum, int amount)
{
	help_overlaylist[current_helpid].textlist[gr_screen.res][textnum].x_coord += amount;
}
void nudgetext_y(int textnum, int amount)
{
	help_overlaylist[current_helpid].textlist[gr_screen.res][textnum].y_coord += amount;
}
void nudgepline_x(int plinenum, int plinevert, int amount)
{
	help_overlaylist[current_helpid].plinelist[gr_screen.res][plinenum].vtx[plinevert].xyz.x += amount;
}
void nudgepline_y(int plinenum, int plinevert, int amount)
{
	help_overlaylist[current_helpid].plinelist[gr_screen.res][plinenum].vtx[plinevert].xyz.y += amount;
}
void nudgerbracket_x(int num, int amount)
{
	help_overlaylist[current_helpid].rbracketlist[gr_screen.res][num].x_coord += amount;
}
void nudgerbracket_y(int num, int amount)
{
	help_overlaylist[current_helpid].rbracketlist[gr_screen.res][num].y_coord += amount;
}
void nudgelbracket_x(int num, int amount)
{
	help_overlaylist[current_helpid].lbracketlist[gr_screen.res][num].x_coord += amount;
}
void nudgelbracket_y(int num, int amount)
{
	help_overlaylist[current_helpid].lbracketlist[gr_screen.res][num].y_coord += amount;
}
void showtextpos(int textnum)
{
	dc_printf("text %d is now located at (%d, %d)", textnum, help_overlaylist[current_helpid].textlist[gr_screen.res][textnum].x_coord, help_overlaylist[current_helpid].textlist[gr_screen.res][textnum].y_coord );
}
void showrbracketpos(int num)
{
	dc_printf("rbracket %d is now located at (%d, %d)", num, help_overlaylist[current_helpid].rbracketlist[gr_screen.res][num].x_coord, help_overlaylist[current_helpid].rbracketlist[gr_screen.res][num].y_coord );
}
void showlbracketpos(int num)
{
	dc_printf("lbracket %d on overlay %d is now located at (%d, %d)", num, current_helpid, help_overlaylist[current_helpid].lbracketlist[gr_screen.res][num].x_coord, help_overlaylist[current_helpid].lbracketlist[gr_screen.res][num].y_coord );
}
void showplinepos(int plinenum)
{
	int i;
	dc_printf("pline %d on overlay %d vertices are now ", plinenum, current_helpid, help_overlaylist[current_helpid].textlist[gr_screen.res][plinenum].y_coord );
	for (i=0; i<help_overlaylist[current_helpid].plinelist[GR_640][plinenum].vtxcount; i++)
	{
		dc_printf("(%3.0f %3.0f) ", help_overlaylist[current_helpid].plinelist[gr_screen.res][plinenum].vtx[i].xyz.x, help_overlaylist[current_helpid].plinelist[gr_screen.res][plinenum].vtx[i].xyz.y);
	}
}

DCF(help_nudgetext_x, "Use to visually position overlay text.")
{

	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: sample\nCrashes your machine.\n" );
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		showtextpos(h_textnum);
		return;
	}

	dc_stuff_int(&h_textnum);
	dc_stuff_int(&h_amt);

	nudgetext_x(h_textnum, h_amt);
}

DCF(help_nudgetext_y, "Use to visually position overlay text.")
{
	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: sample\nCrashes your machine.\n" );
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		showtextpos(h_textnum);
		return;
	}

	dc_stuff_int(&h_textnum);
	dc_stuff_int(&h_amt);
	
	nudgetext_y(h_textnum, h_amt);
}

DCF(help_nudgepline_x, "Use to visually position overlay polylines.")
{
		if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: help_nudgepline [pline_number] [vertex_number] [distance]\n" );
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?"))	{
		showplinepos(h_textnum);
		return;
	}

	dc_stuff_int(&h_textnum);
	dc_stuff_int(&h_vtx);
	dc_stuff_int(&h_amt);

	nudgepline_x(h_textnum, h_vtx, h_amt);
}


DCF(help_nudgepline_y, "Use to visually position overlay polylines.")
{
	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: help_nudgepline [pline_number] [vertex_number] [distance]\n" );
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?"))	{
		showplinepos(h_textnum);
		return;
	}

	dc_stuff_int(&h_textnum);
	dc_stuff_int(&h_vtx);
	dc_stuff_int(&h_amt);

	nudgepline_y(h_textnum, h_vtx, h_amt);
}


DCF(help_nudgerbracket_x, "Use to visually position overlay right bracket.")
{
	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: help_nudgerbracket_x [num] [amount]\n" );
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?"))	{
		showrbracketpos(h_textnum);
		return;
	}

	dc_stuff_int(&h_textnum);
	dc_stuff_int(&h_amt);

	nudgerbracket_x(h_textnum, h_amt);
}

DCF(help_nudgerbracket_y, "Use to visually position overlay right bracket.")
{
	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: help_nudgerbracket_y [num] [amount]\n" );
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?"))	{
		showrbracketpos(h_textnum);
		return;
	}

	dc_stuff_int(&h_textnum);
	dc_stuff_int(&h_amt);
	
	nudgerbracket_y(h_textnum, h_amt);
}




DCF(help_nudgelbracket_x, "Use to visually position overlay left bracket.")
{

	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: help_nudgelbracket_x [num] [amount]\n" );
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		showlbracketpos(h_textnum);
		return;
	}

	dc_stuff_int(&h_textnum);
	dc_stuff_int(&h_amt);

	nudgelbracket_x(h_textnum, h_amt);
}

DCF(help_nudgelbracket_y, "Use to visually position overlay left bracket.")
{
	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: help_nudgelbracket_y [num] [amount]\n" );
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?"))	{
		showlbracketpos(h_textnum);
		return;
	}

	dc_stuff_int(&h_textnum);
	dc_stuff_int(&h_amt);

	nudgelbracket_y(h_textnum, h_amt);
}
