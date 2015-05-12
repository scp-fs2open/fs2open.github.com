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
void help_overlay_blit(int overlay_id, int resolution_index);
void help_overlay_init();


typedef struct {
	int x_begin, y_begin, x_end, y_end;
} help_line;

typedef struct {
	SCP_vector<vec3d> vtx;
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
	int num_resolutions;
	SCP_vector<int>	fontlist;
	SCP_vector<SCP_vector<help_pline> >			plinelist;
	SCP_vector<SCP_vector<help_text> >			textlist;
	SCP_vector<SCP_vector<help_left_bracket> >	lbracketlist;
	SCP_vector<SCP_vector<help_right_bracket> >	rbracketlist;
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
static int current_resolution = -1;
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
void help_overlay_set_state(int overlay_id, int resolution_index, int state)
{
	Assert(overlay_id < MAX_HELP_OVERLAYS);

	if ( (overlay_id >= 0) && (overlay_id < num_help_overlays) &&
			(resolution_index >= 0) && (resolution_index < help_overlaylist[overlay_id].num_resolutions) ) {
		if ( state > 0 ) {
			Help_overlay_flags |= (1<<overlay_id);
			current_helpid = overlay_id;
			current_resolution = resolution_index;
		} else {
			Help_overlay_flags &= ~(1<<overlay_id);
			//current_helpid = -1;
		}
	}

}

// maybe blit a bitmap of a help overlay to the screen
void help_overlay_maybe_blit(int overlay_id, int resolution_index)
{
	Assert(overlay_id < MAX_HELP_OVERLAYS);

	if ( (overlay_id >= 0) && (Help_overlay_flags & (1<<overlay_id)) &&
			(resolution_index >= 0) && (resolution_index < help_overlaylist[overlay_id].num_resolutions) ) {
		context_help_grey_screen();
		help_overlay_blit(overlay_id, resolution_index);
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
	int resolution_index = gr_screen.res;

	// look at the state the game was in when F1 was pressed
	Source_game_state = gameseq_get_state();

	switch (Source_game_state) {

		case GS_STATE_MAIN_MENU:
			overlay_id = main_hall_get_overlay_id();
			resolution_index = main_hall_get_overlay_resolution_index();
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
			help_overlay_set_state(overlay_id, resolution_index, 1);
		}
		else {
			help_overlay_set_state(overlay_id, resolution_index, 0);
		}
	}
}

void close_help(){
	for (int overlay_id=0; overlay_id<MAX_HELP_OVERLAYS; overlay_id++){
		if (help_overlaylist[overlay_id].textlist.size() > 0) {
			for(SCP_vector<help_text>::iterator ii = help_overlaylist[overlay_id].textlist.at(0).begin(); ii != help_overlaylist[overlay_id].textlist.at(0).end(); ++ii) {
				safe_kill(ii->string);
			}
		}
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
	int overlay_id, currcount, vtxcount;
	char name[HELP_MAX_NAME_LENGTH];
	char buf[HELP_MAX_STRING_LENGTH + 1];
	int i, j;

	SCP_vector<help_pline> pline_temp;
	help_pline pline_temp2;
	SCP_vector<help_text> text_temp;
	help_text text_temp2;
	SCP_vector<help_right_bracket> rbracket_temp;
	help_right_bracket rbracket_temp2;
	SCP_vector<help_left_bracket> lbracket_temp;
	help_left_bracket lbracket_temp2;
	vec3d vec3d_temp;
	
	try
	{
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
				}
				else {
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

			help_overlaylist[overlay_id].fontlist.clear();
			help_overlaylist[overlay_id].plinelist.clear();
			help_overlaylist[overlay_id].textlist.clear();
			help_overlaylist[overlay_id].rbracketlist.clear();
			help_overlaylist[overlay_id].lbracketlist.clear();

			if (optional_string("+resolutions")) {
				stuff_int(&help_overlaylist[overlay_id].num_resolutions);
			}
			else {
				help_overlaylist[overlay_id].num_resolutions = 2;
			}

			if (help_overlaylist[overlay_id].num_resolutions < 1) {
				Error(LOCATION, "+resolutions in %s is %d. (Must be 1 or greater)", filename, help_overlaylist[overlay_id].num_resolutions);
			}

			if (optional_string("+font")) {
				int font;
				for (i = 0; i < help_overlaylist[overlay_id].num_resolutions; i++) {
					stuff_int(&font);
					help_overlaylist[overlay_id].fontlist.push_back(font);
				}
			}
			else {
				for (i = 0; i < help_overlaylist[overlay_id].num_resolutions; i++) {
					help_overlaylist[overlay_id].fontlist.push_back(FONT1);
				}
			}

			for (i = 0; i < help_overlaylist[overlay_id].num_resolutions; i++) {
				help_overlaylist[overlay_id].plinelist.push_back(pline_temp);
				help_overlaylist[overlay_id].textlist.push_back(text_temp);
				help_overlaylist[overlay_id].rbracketlist.push_back(rbracket_temp);
				help_overlaylist[overlay_id].lbracketlist.push_back(lbracket_temp);
			}

			int type;
			// read in all elements for this overlay
			while ((type = required_string_one_of(5, "+pline", "+text", "+right_bracket", "+left_bracket", "$end")) != 4) {	// Doing it this way means an error lists "$end" at the end, which seems appropriate. -MageKing17

				switch (type) {
				case 0:	// +pline
					required_string("+pline");
					currcount = help_overlaylist[overlay_id].plinecount;
					int a, b;		// temp vars to read in int before cast to float;

					// read number of pline vertices
					stuff_int(&vtxcount);
					// get vertex coordinates for each resolution
					for (i = 0; i < help_overlaylist[overlay_id].num_resolutions; i++) {
						help_overlaylist[overlay_id].plinelist.at(i).push_back(pline_temp2);
						for (j = 0; j < vtxcount; j++) {
							help_overlaylist[overlay_id].plinelist.at(i).at(currcount).vtx.push_back(vec3d_temp);
							help_overlaylist[overlay_id].plinelist.at(i).at(currcount).vtxcount = vtxcount;
							stuff_int(&a);
							stuff_int(&b);
							help_overlaylist[overlay_id].plinelist.at(i).at(currcount).vtx.at(j).xyz.x = (float)a;
							help_overlaylist[overlay_id].plinelist.at(i).at(currcount).vtx.at(j).xyz.y = (float)b;
							help_overlaylist[overlay_id].plinelist.at(i).at(currcount).vtx.at(j).xyz.z = 0.0f;
						}
					}

					help_overlaylist[overlay_id].plinecount++;
					break;
				case 1:	// +text
					required_string("+text");
					currcount = help_overlaylist[overlay_id].textcount;

					// get coordinates for each resolution
					for (i = 0; i < help_overlaylist[overlay_id].num_resolutions; i++) {
						help_overlaylist[overlay_id].textlist.at(i).push_back(text_temp2);
						stuff_int(&(help_overlaylist[overlay_id].textlist.at(i).at(currcount).x_coord));
						stuff_int(&(help_overlaylist[overlay_id].textlist.at(i).at(currcount).y_coord));
					}

					// get string (always use the first resolution)
					stuff_string(buf, F_MESSAGE, sizeof(buf));
					help_overlaylist[overlay_id].textlist.at(0).at(currcount).string = vm_strdup(buf);

					help_overlaylist[overlay_id].textcount++;
					break;
				case 2: // +right_bracket
					required_string("+right_bracket");
					currcount = help_overlaylist[overlay_id].rbracketcount;

					// get coordinates for each resolution
					for (i = 0; i < help_overlaylist[overlay_id].num_resolutions; i++) {
						help_overlaylist[overlay_id].rbracketlist.at(i).push_back(rbracket_temp2);
						stuff_int(&(help_overlaylist[overlay_id].rbracketlist.at(i).at(currcount).x_coord));
						stuff_int(&(help_overlaylist[overlay_id].rbracketlist.at(i).at(currcount).y_coord));
					}

					help_overlaylist[overlay_id].rbracketcount++;
					break;
				case 3: // +left_bracket
					required_string("+left_bracket");
					currcount = help_overlaylist[overlay_id].lbracketcount;

					// get coordinates for each resolution
					for (i = 0; i < help_overlaylist[overlay_id].num_resolutions; i++) {
						help_overlaylist[overlay_id].lbracketlist.at(i).push_back(lbracket_temp2);
						stuff_int(&(help_overlaylist[overlay_id].lbracketlist.at(i).at(currcount).x_coord));
						stuff_int(&(help_overlaylist[overlay_id].lbracketlist.at(i).at(currcount).y_coord));
					}

					help_overlaylist[overlay_id].lbracketcount++;
					break;
				case -1:
					// -noparseerrors is set
					break;
				case 4: // $end
				default:
					Assertion(false, "This should never happen.\n");
					break;
				}
			}		// end while
			required_string("$end");
		}		// end while
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}



// draw overlay on the screen
void help_overlay_blit(int overlay_id, int resolution_index)
{
	int idx, width, height;
	int plinecount = help_overlaylist[overlay_id].plinecount;
	int textcount = help_overlaylist[overlay_id].textcount;
	int rbracketcount = help_overlaylist[overlay_id].rbracketcount;
	int lbracketcount = help_overlaylist[overlay_id].lbracketcount;

	Assert(overlay_id >= 0 && overlay_id < MAX_HELP_OVERLAYS);

	// this draws each line of help text with white on black text (use the first resolution index for the string)
	gr_set_font(help_overlaylist[overlay_id].fontlist.at(resolution_index));
	for (idx = 0; idx < textcount; idx++) {
		gr_set_color_fast(&Color_black);
		gr_get_string_size(&width, &height, help_overlaylist[overlay_id].textlist.at(0).at(idx).string, strlen(help_overlaylist[overlay_id].textlist.at(0).at(idx).string));
		gr_rect(help_overlaylist[overlay_id].textlist.at(resolution_index).at(idx).x_coord-2*HELP_PADDING, help_overlaylist[overlay_id].textlist.at(resolution_index).at(idx).y_coord-3*HELP_PADDING, width+4*HELP_PADDING, height+4*HELP_PADDING, GR_RESIZE_MENU);
		gr_set_color_fast(&Color_bright_white);
		gr_printf_menu(help_overlaylist[overlay_id].textlist.at(resolution_index).at(idx).x_coord, help_overlaylist[overlay_id].textlist.at(resolution_index).at(idx).y_coord, help_overlaylist[overlay_id].textlist.at(0).at(idx).string);
	}
	gr_set_font(FONT1);

	// this draws each right bracket
	for (idx = 0; idx < rbracketcount; idx++) {
		gr_set_bitmap(help_right_bracket_bitmap);
		gr_bitmap(help_overlaylist[overlay_id].rbracketlist.at(resolution_index).at(idx).x_coord, help_overlaylist[overlay_id].rbracketlist.at(resolution_index).at(idx).y_coord, GR_RESIZE_MENU);
	}

	// this draws each left bracket
	for (idx = 0; idx < lbracketcount; idx++) {
		gr_set_bitmap(help_left_bracket_bitmap);
		gr_bitmap(help_overlaylist[overlay_id].lbracketlist.at(resolution_index).at(idx).x_coord, help_overlaylist[overlay_id].lbracketlist.at(resolution_index).at(idx).y_coord, GR_RESIZE_MENU);
	}	

	// this draws each 2d line for the help screen
	//gr_set_color_fast(&Color_yellow);
	gr_set_color(255, 255, 0);
	for (idx = 0; idx<plinecount; idx++) {
		gr_pline_special(&help_overlaylist[overlay_id].plinelist.at(resolution_index).at(idx).vtx, HELP_PLINE_THICKNESS, GR_RESIZE_MENU);
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
	help_overlaylist[current_helpid].textlist.at(current_resolution).at(textnum).x_coord += amount;
}
void nudgetext_y(int textnum, int amount)
{
	help_overlaylist[current_helpid].textlist.at(current_resolution).at(textnum).y_coord += amount;
}
void nudgepline_x(int plinenum, int plinevert, int amount)
{
	help_overlaylist[current_helpid].plinelist.at(current_resolution).at(plinenum).vtx[plinevert].xyz.x += amount;
}
void nudgepline_y(int plinenum, int plinevert, int amount)
{
	help_overlaylist[current_helpid].plinelist.at(current_resolution).at(plinenum).vtx[plinevert].xyz.y += amount;
}
void nudgerbracket_x(int num, int amount)
{
	help_overlaylist[current_helpid].rbracketlist.at(current_resolution).at(num).x_coord += amount;
}
void nudgerbracket_y(int num, int amount)
{
	help_overlaylist[current_helpid].rbracketlist.at(current_resolution).at(num).y_coord += amount;
}
void nudgelbracket_x(int num, int amount)
{
	help_overlaylist[current_helpid].lbracketlist.at(current_resolution).at(num).x_coord += amount;
}
void nudgelbracket_y(int num, int amount)
{
	help_overlaylist[current_helpid].lbracketlist.at(current_resolution).at(num).y_coord += amount;
}
void showtextpos(int textnum)
{
	dc_printf("text %d is now located at (%d, %d)", textnum, help_overlaylist[current_helpid].textlist.at(current_resolution).at(textnum).x_coord, help_overlaylist[current_helpid].textlist.at(current_resolution).at(textnum).y_coord );
}
void showrbracketpos(int num)
{
	dc_printf("rbracket %d is now located at (%d, %d)", num, help_overlaylist[current_helpid].rbracketlist.at(current_resolution).at(num).x_coord, help_overlaylist[current_helpid].rbracketlist.at(current_resolution).at(num).y_coord );
}
void showlbracketpos(int num)
{
	dc_printf("lbracket %d on overlay %d is now located at (%d, %d)", num, current_helpid, help_overlaylist[current_helpid].lbracketlist.at(current_resolution).at(num).x_coord, help_overlaylist[current_helpid].lbracketlist.at(current_resolution).at(num).y_coord );
}
void showplinepos(int plinenum)
{
	int i;
	dc_printf("pline %d on overlay %d vertices are now ", plinenum, current_helpid, help_overlaylist[current_helpid].textlist.at(current_resolution).at(plinenum).y_coord );
	for (i=0; i<help_overlaylist[current_helpid].plinelist.at(gr_screen.res).at(plinenum).vtxcount; i++)
	{
		dc_printf("(%3.0f %3.0f) ", help_overlaylist[current_helpid].plinelist.at(current_resolution).at(plinenum).vtx.at(i).xyz.x, help_overlaylist[current_helpid].plinelist.at(current_resolution).at(plinenum).vtx.at(i).xyz.y);
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
