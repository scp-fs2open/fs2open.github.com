/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "ui/ui.h"
#include "missionui/chatbox.h"
#include "network/multimsgs.h"
#include "gamesnd/gamesnd.h"
#include "cmdline/cmdline.h"
#include "io/key.h"
#include "network/multi.h"
#include "network/multiui.h"
#include "network/multi_pmsg.h"
#include "globalincs/alphacolors.h"
#include "playerman/player.h"
#include "parse/parselo.h"
#include "cfile/cfile.h"



///////////////////////////////////////////////////////////
// Chat window UI 
///////////////////////////////////////////////////////////

// a little extra spacing for the team vs. team icons
#define CHATBOX_TEAM_ICON_SPACE			18

// SMALL CHATBOX ----------------------------------------------------------------------------------

// background bitmap
char* Chatbox_small_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"Chatbox",		// GR_640
	"2_Chatbox"		// GR_1024
};

// background mask
char* Chatbox_small_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"Chatbox-m",	// GR_640
	"2_Chatbox-m"	// GR_1024
};

// chatbox coords
int Chatbox_small_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		192, 0
	},
	{ // GR_1024
		308, 0			 // ?
	}
};

// display area coods
int Chatbox_small_display_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		196 + CHATBOX_TEAM_ICON_SPACE, 13, 410 - CHATBOX_TEAM_ICON_SPACE, 74
	},
	{	// GR_1024
		315 + CHATBOX_TEAM_ICON_SPACE, 22, 654 - CHATBOX_TEAM_ICON_SPACE, 116
	}
};

// input box coords
int Chatbox_small_input_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		204, 100, 371, 22
	},
	{	// GR_1024
		328, 163, 591, 34
	}
};

// max # of lines
int Chatbox_small_max_lines[GR_NUM_RESOLUTIONS] = {
	7,				// GR_640
	12				// GR_1024
};

// BIG CHATBOX ----------------------------------------------------------------------------------

// background bitmap
char* Chatbox_big_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"ChatboxBig",		// GR_640
	"2_ChatboxBig"		// GR_1024
};

// mask
char* Chatbox_big_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"Chatbox-m",		// GR_640
	"2_Chatbox-m"			// GR_1024
};

// chatbox coords
int Chatbox_big_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		192, 0
	},
	{ // GR_1024
		307, 0
	}
};

// display area coords
int Chatbox_big_display_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		196 + CHATBOX_TEAM_ICON_SPACE, 13, 410 - CHATBOX_TEAM_ICON_SPACE, 326
	},
	{	// GR_1024
		315 + CHATBOX_TEAM_ICON_SPACE, 22, 654 - CHATBOX_TEAM_ICON_SPACE, 519
	}
};

// input box coords
int Chatbox_big_input_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		204, 352, 371, 22
	},
	{	// GR_1024
		328, 565, 591, 34
	}
};

// max # of lines
int Chatbox_big_max_lines[GR_NUM_RESOLUTIONS] = {
	32,			// GR_640
	51				// GR_1024
};

// PAUSED CHATBOX ----------------------------------------------------------------------------------

// mask
char* Chatbox_p_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"MPPause",			// GR_640
	"2_MPPause"			// GR_1024
};

// mask
char* Chatbox_p_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MPPause-m",		// GR_640
	"2_MPPause-m"		// GR_1024
};

// chatbox coords
int Chatbox_p_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
	//	192, 0
		0, 0
	},
	{ // GR_1024
	//	307, 0
		0, 0
	}
};

// display area coords
int Chatbox_p_display_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		103 + CHATBOX_TEAM_ICON_SPACE, 149, 380 - CHATBOX_TEAM_ICON_SPACE, 143
	},
	{	// GR_1024
		165 + CHATBOX_TEAM_ICON_SPACE, 244, 654 - CHATBOX_TEAM_ICON_SPACE, 263
	}
};

// input box coords
int Chatbox_p_input_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		106, 328, 379, 17
	},
	{	// GR_1024
		165, 525, 607, 27
	}
};

// max # of lines
int Chatbox_p_max_lines[GR_NUM_RESOLUTIONS] = {
	17,			// GR_640
	26				// GR_1024
};

// defines for location and other info of chat box MULTI_PAUSED
/*
#define CHATBOX_MP_FNAME					NOX("MPPause")
#define CHATBOX_MP_MASK						NOX("MPPause-m")
#define CHATBOX_MP_X1						112
#define CHATBOX_MP_Y1						198
#define CHATBOX_MP_X2						477
#define CHATBOX_MP_Y2						308
#define CHATBOX_MP_ICON_X					(CHATBOX_MP_X1)
#define CHATBOX_MP_W							(CHATBOX_MP_X2 - CHATBOX_MP_X1 + 1)
#define CHATBOX_MP_H							(CHATBOX_MP_Y2 - CHATBOX_MP_Y1 + 1)
#define CHATBOX_MP_BEGIN_X					(CHATBOX_MP_X1 + 3 + CHATBOX_TEAM_ICON_SPACE)
#define CHATBOX_MP_BEGIN_Y					CHATBOX_MP_Y1
#define CHATBOX_MP_DISP_W					365
#define CHATBOX_MP_MAX_LINES				11
#define CHATBOX_MP_INPUTBOX_X				(CHATBOX_MP_X1 + 3)
#define CHATBOX_MP_INPUTBOX_W				(CHATBOX_MP_W)
#define CHATBOX_MP_TEXTENTER_Y			329
*/

// CHATBOX ----------------------------------------------------------------------------------

// the settings being used for this instance
char Chatbox_mask[50];
int Chatbox_x1;
int Chatbox_y1;
int Chatbox_icon_x;
int Chatbox_w;
int Chatbox_h;
int Chatbox_begin_x;
int Chatbox_begin_y;
int Chatbox_disp_w;
int Chatbox_max_lines;
int Chatbox_inputbox_x;
int Chatbox_inputbox_w;
int Chatbox_textenter_y;
int Chat_scroll_up_coord[2];
int Chat_scroll_down_coord[2];

// how many pixels to indent succesive lines of text from a given player
#define CHAT_LINE_INDENT					20

// what chars other than letters and number's we'll toss
#define CHATBOX_INVALID_CHARS				NOX("~`")			// this is primarily so that we don't interfere with the voice recording keys

// common defines and data
#define CHATBOX_STRING_LEN					(CALLSIGN_LEN + CHATBOX_MAX_LEN + 32)
UI_WINDOW	Chat_window;
UI_INPUTBOX	Chat_inputbox;
UI_BUTTON Chat_enter_text;

// button controls
#define CHATBOX_NUM_BUTTONS				3
#define CHATBOX_SCROLL_UP					0
#define CHATBOX_SCROLL_DOWN				1
#define CHATBOX_TOGGLE_SIZE				2				// used for both big and small

// coordinate indicies
#define CHATBOX_X_COORD 0
#define CHATBOX_Y_COORD 1
#define CHATBOX_W_COORD 2
#define CHATBOX_H_COORD 3

// chatbox buttons
ui_button_info Chatbox_buttons[GR_NUM_RESOLUTIONS][CHATBOX_NUM_BUTTONS+1] = {
	{ // GR_640
		ui_button_info("CHB_00",	613,	3,		-1,	-1,	0),
		ui_button_info("CHB_01",	613,	41,	-1,	-1,	1),
		ui_button_info("CHB_02a",	607,	74,	-1,	-1,	2),
		ui_button_info("CHB_02b",	607,	74,	-1,	-1,	2),
	},	
	{ // GR_1024
		ui_button_info("2_CHB_00",		981,	5,		-1,	-1,	0),
		ui_button_info("2_CHB_01",		981,	67,	-1,	-1,	1),
		ui_button_info("2_CHB_02a",	971,	119,	-1,	-1,	2),
		ui_button_info("2_CHB_02b",	971,	119,	-1,	-1,	2),
	}
};

int Chatbox_mode_flags = 0;

int Chatbox_bitmap = -1;
int Chatbox_big_bitmap = -1;
int Chatbox_small_bitmap = -1;
int Chatbox_mp_bitmap = -1;
int Chatbox_created = 0;

///////////////////////////////////////////////////////////
// Chat window text
///////////////////////////////////////////////////////////
#define MAX_BRIEF_CHAT_LINES 60   // how many lines we can store in the scrollback buffer
#define BRIEF_DISPLAY_SPACING 2   // pixel spacing between chat lines

// the first byte of the text string will be the net player id of the 
char Brief_chat_lines[MAX_BRIEF_CHAT_LINES][CHATBOX_STRING_LEN];
int Brief_chat_indents[MAX_BRIEF_CHAT_LINES];

int Brief_chat_next_index[MAX_BRIEF_CHAT_LINES];
int Brief_chat_prev_index[MAX_BRIEF_CHAT_LINES];
int Num_brief_chat_lines;
int Brief_current_add_line;
int Brief_start_display_index;

// chatbox line recall data
#define CHATBOX_MAX_RECALL_LINES			10
int Chatbox_recall_count = 0;
int Chatbox_recall_index = 0;
int Chatbox_recall_last = -1;
char Chatbox_recall_lines[CHATBOX_MAX_RECALL_LINES][CHATBOX_MAX_LEN+2];

///////////////////////////////////////////////////////////
// forward declarations
///////////////////////////////////////////////////////////
void chatbox_chat_init();
void chatbox_render_chat_lines();
void chatbox_set_mode(int mode_flags);
void chatbox_toggle_size();
void chatbox_toggle_size_adjust_lines();
void chat_autosplit_line(char *msg,char *remainder);
int chatbox_num_displayed_lines();
void chatbox_recall_add(char *string);
void chatbox_recall_up();
void chatbox_recall_down();

// set the chatbox mode without checking for any previous modes which may need to be handled specially
void chatbox_set_mode(int mode_flags)
{
	int size;
	
	// set the stored mode
	Chatbox_mode_flags = mode_flags;
	
	// small pregame chatbox
	if(Chatbox_mode_flags & CHATBOX_FLAG_SMALL){
		size = 0;
	}
	// big pregame chatbox
	else if(Chatbox_mode_flags & CHATBOX_FLAG_BIG){
		size = 1;
	}
	// multiplayer paused
	else {
		size = 2;
	}

	// set up the display/init variables based upon what mode we chode
	switch(size){
	case 0:		
		strcpy_s(Chatbox_mask, Chatbox_small_bitmap_mask_fname[gr_screen.res]);
		Chatbox_x1 = Chatbox_small_coords[gr_screen.res][CHATBOX_X_COORD];
		Chatbox_y1 = Chatbox_small_coords[gr_screen.res][CHATBOX_Y_COORD];		
		Chatbox_icon_x = Chatbox_small_display_coords[gr_screen.res][CHATBOX_X_COORD] - CHATBOX_TEAM_ICON_SPACE;
		Chatbox_w = Chatbox_small_display_coords[gr_screen.res][CHATBOX_W_COORD];
		Chatbox_h = Chatbox_small_display_coords[gr_screen.res][CHATBOX_H_COORD];
		Chatbox_begin_x = Chatbox_small_display_coords[gr_screen.res][CHATBOX_X_COORD];
		Chatbox_begin_y = Chatbox_small_display_coords[gr_screen.res][CHATBOX_Y_COORD];
		Chatbox_disp_w = Chatbox_small_display_coords[gr_screen.res][CHATBOX_W_COORD];
		Chatbox_max_lines = Chatbox_small_max_lines[gr_screen.res];
		Chatbox_inputbox_x = Chatbox_small_input_coords[gr_screen.res][CHATBOX_X_COORD];
		Chatbox_inputbox_w = Chatbox_small_input_coords[gr_screen.res][CHATBOX_W_COORD];
		Chatbox_textenter_y = Chatbox_small_input_coords[gr_screen.res][CHATBOX_Y_COORD];		
		break;

	case 1:		
		strcpy_s(Chatbox_mask, Chatbox_big_bitmap_mask_fname[gr_screen.res]);
		Chatbox_x1 = Chatbox_big_coords[gr_screen.res][CHATBOX_X_COORD];
		Chatbox_y1 = Chatbox_big_coords[gr_screen.res][CHATBOX_Y_COORD];		
		Chatbox_icon_x = Chatbox_big_display_coords[gr_screen.res][CHATBOX_X_COORD] - CHATBOX_TEAM_ICON_SPACE;
		Chatbox_w = Chatbox_big_display_coords[gr_screen.res][CHATBOX_W_COORD];
		Chatbox_h = Chatbox_big_display_coords[gr_screen.res][CHATBOX_H_COORD];
		Chatbox_begin_x = Chatbox_big_display_coords[gr_screen.res][CHATBOX_X_COORD];
		Chatbox_begin_y = Chatbox_big_display_coords[gr_screen.res][CHATBOX_Y_COORD];
		Chatbox_disp_w = Chatbox_big_display_coords[gr_screen.res][CHATBOX_W_COORD];
		Chatbox_max_lines = Chatbox_big_max_lines[gr_screen.res];
		Chatbox_inputbox_x = Chatbox_big_input_coords[gr_screen.res][CHATBOX_X_COORD];
		Chatbox_inputbox_w = Chatbox_big_input_coords[gr_screen.res][CHATBOX_W_COORD];
		Chatbox_textenter_y = Chatbox_big_input_coords[gr_screen.res][CHATBOX_Y_COORD];		
		break;

	case 2:				
		Chatbox_x1 = Chatbox_p_coords[gr_screen.res][CHATBOX_X_COORD];
		Chatbox_y1 = Chatbox_p_coords[gr_screen.res][CHATBOX_Y_COORD];		
		Chatbox_icon_x = Chatbox_p_display_coords[gr_screen.res][CHATBOX_X_COORD] - CHATBOX_TEAM_ICON_SPACE;
		Chatbox_w = Chatbox_p_display_coords[gr_screen.res][CHATBOX_W_COORD];
		Chatbox_h = Chatbox_p_display_coords[gr_screen.res][CHATBOX_H_COORD];
		Chatbox_begin_x = Chatbox_p_display_coords[gr_screen.res][CHATBOX_X_COORD];
		Chatbox_begin_y = Chatbox_p_display_coords[gr_screen.res][CHATBOX_Y_COORD];
		Chatbox_disp_w = Chatbox_p_display_coords[gr_screen.res][CHATBOX_W_COORD];
		Chatbox_max_lines = Chatbox_p_max_lines[gr_screen.res];
		Chatbox_inputbox_x = Chatbox_p_input_coords[gr_screen.res][CHATBOX_X_COORD];
		Chatbox_inputbox_w = Chatbox_p_input_coords[gr_screen.res][CHATBOX_W_COORD];
		Chatbox_textenter_y = Chatbox_p_input_coords[gr_screen.res][CHATBOX_Y_COORD];
		break;
	}
}

int chatbox_get_msg_target_type(char *msg )
{
	if (msg[0] != '/') {
		return MULTI_MSG_ALL;
	}

	if (!strnicmp (msg, "/f:", 3)) {
		return MULTI_MSG_FRIENDLY;
	}	

	if (!strnicmp (msg, "/h:", 3)) {
		return MULTI_MSG_HOSTILE;
	}	

	return MULTI_MSG_EXPR;
}

int chatbox_get_msg_target_length(char *msg)
{
	if ((msg[0] != '/') && (strchr(msg, ':') != NULL) ) {
		return -1;
	}

	// find the first space
	return strcspn(msg, ":") + 1 ;

}

// automatically split up any input text, send it, and leave the remainder 
void chatbox_autosplit_line()
{
	char *remainder,msg[150];
	char temp[150];
	int msg_pixel_width;
	int target, target_length = -1; 
	
	// if the chat line is getting too long, fire off the message, putting the last
	// word on the next input line.
	memset(msg,0,150);
	Chat_inputbox.get_text(msg);
	remainder = "";

	// check if this message is supposed to have a recipient
	target = chatbox_get_msg_target_type(msg); 
	target_length = chatbox_get_msg_target_length(msg); 

	// determine if the width of the string in pixels is > than the inputbox width -- if so,
	// then send the message
	gr_get_string_size(&msg_pixel_width, NULL, msg);
	// if ( msg_pixel_width >= (Chatbox_inputbox_w - Player->short_callsign_width) ) {
	if ( msg_pixel_width >= (Chatbox_inputbox_w - 25)) {
		remainder = strrchr(msg, ' ');
		if ( remainder ) {
			*remainder = '\0';
			remainder++;
		} else {
			remainder = "";
		}	
		// if I'm the server, then broadcast the packet		
		chatbox_recall_add(msg);

		if (target != MULTI_MSG_EXPR) {
  			send_game_chat_packet(Net_player, msg, target);
		}
		else {
			// copy the name of the player the message is being sent to
			strncpy(temp, msg+1, target_length-2);
			temp[target_length-2] = '\0';
			send_game_chat_packet(Net_player, msg, target, NULL, temp);
		}
		chatbox_add_line(msg, MY_NET_PLAYER_NUM);

		if (target != MULTI_MSG_ALL) {
			// we need to add the target the message is going to before we add the rest of the string
			strncpy(temp, msg, target_length);
 			temp[target_length] = ' '; 
 			temp[target_length+1] = '\0'; 
			strcat_s(temp, remainder); 
			Chat_inputbox.set_text(temp);
		}
		else {
			// display any remainder of text on the next line
			Chat_inputbox.set_text(remainder);
		}
	} else if((Chat_inputbox.pressed() && (msg[0] != '\0')) || (strlen(msg) >= CHATBOX_MAX_LEN)) { 
		// tack on the null terminator in the boundary case
		int x = strlen(msg);
		if(x >= CHATBOX_MAX_LEN){
			msg[CHATBOX_MAX_LEN-1] = '\0';
		}
	
		// if I'm the server, then broadcast the packet		
		chatbox_recall_add(msg);
		if (target != MULTI_MSG_EXPR) {
  			send_game_chat_packet(Net_player, msg, target);
		}
		else {
			// copy the name of the player the message is being sent to
			strncpy(temp, msg+1, target_length-2);
			temp[target_length-2] = '\0';
			send_game_chat_packet(Net_player, msg, target, NULL, temp);
		}

		chatbox_add_line(msg, MY_NET_PLAYER_NUM);

		// display any remainder of text on the next line
		Chat_inputbox.set_text(remainder);		
	}	
}

// initialize all chatbox details with the given mode flags
int chatbox_create(int mode_flags)
{
	int idx;
	
	// don't do anything if the chatbox is already initialized
	if (Chatbox_created){
		return -1;
	}

	// probably shouldn't be using the chatbox in single player mode
	Assert(Game_mode & GM_MULTIPLAYER);

	// setup all data to correspond to our mode flags
	chatbox_set_mode(mode_flags);
	
	// initialize all low-level details related to chatting
	chatbox_chat_init();		

	// attempt to load in the chatbox background bitmap
	if(Chatbox_mode_flags & CHATBOX_FLAG_DRAW_BOX){
		Chatbox_big_bitmap = bm_load(Chatbox_big_bitmap_fname[gr_screen.res]);
		Chatbox_small_bitmap = bm_load(Chatbox_small_bitmap_fname[gr_screen.res]);
		Chatbox_mp_bitmap = bm_load(Chatbox_p_bitmap_fname[gr_screen.res]);
		
		if(Chatbox_mode_flags & CHATBOX_FLAG_SMALL){
			Chatbox_bitmap = Chatbox_small_bitmap;
		} else if(Chatbox_mode_flags & CHATBOX_FLAG_BIG){
			Chatbox_bitmap = Chatbox_big_bitmap;
		} else {
			Chatbox_bitmap = Chatbox_mp_bitmap;
		}

		if((Chatbox_bitmap == -1) || (Chatbox_small_bitmap == -1) || (Chatbox_big_bitmap == -1) || (Chatbox_mp_bitmap == -1)){
			return -1;
		}
	}
	
	// attempt to create the ui window for the chatbox and assign the mask
	Chat_window.create( 0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0 );
	Chat_window.set_mask_bmap(Chatbox_mask);	

   // create the chat text enter input area	
	Chat_inputbox.create( &Chat_window, Chatbox_inputbox_x, Chatbox_textenter_y, Chatbox_inputbox_w, CHATBOX_MAX_LEN, "", UI_INPUTBOX_FLAG_INVIS | UI_INPUTBOX_FLAG_ESC_CLR | UI_INPUTBOX_FLAG_KEYTHRU | UI_INPUTBOX_FLAG_EAT_USED, Chatbox_w, Color_netplayer[MY_NET_PLAYER_NUM]);	
	Chat_inputbox.set_focus();
	Chat_inputbox.set_invalid_chars(CHATBOX_INVALID_CHARS);

	// if we're supposed to supply and check for out own buttons
	if((Chatbox_mode_flags & CHATBOX_FLAG_BUTTONS) && (Chatbox_mode_flags & CHATBOX_FLAG_DRAW_BOX)){
		for(idx=0; idx<CHATBOX_NUM_BUTTONS; idx++){
			// create the button			
			Chatbox_buttons[gr_screen.res][idx].button.create(&Chat_window, "", Chatbox_buttons[gr_screen.res][idx].x, Chatbox_buttons[gr_screen.res][idx].y, 60, 30, (idx == CHATBOX_TOGGLE_SIZE) ? 0 : 1);
			
			// set the highlight action
			Chatbox_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

			// set the bitmap
			Chatbox_buttons[gr_screen.res][idx].button.set_bmaps(Chatbox_buttons[gr_screen.res][idx].filename);

			// set the hotspot
			Chatbox_buttons[gr_screen.res][idx].button.link_hotspot(Chatbox_buttons[gr_screen.res][idx].hotspot);
		}
		
		// now create the toggle size button with the appropriate button
		if(Chatbox_mode_flags & CHATBOX_FLAG_SMALL){
			Chatbox_buttons[gr_screen.res][CHATBOX_TOGGLE_SIZE].button.set_bmaps(Chatbox_buttons[gr_screen.res][CHATBOX_TOGGLE_SIZE].filename);
		} else {
			Chatbox_buttons[gr_screen.res][CHATBOX_TOGGLE_SIZE].button.set_bmaps(Chatbox_buttons[gr_screen.res][CHATBOX_TOGGLE_SIZE+1].filename);
		}
	}

	// an invisible button that will set focus to input box when clicked on
	Chat_enter_text.create( &Chat_window, "", 0, 0, 60, 30, 0);
	Chat_enter_text.hide();					// button doesn't show up
	Chat_enter_text.link_hotspot(50);	

	Chatbox_created = 1;
	return 0;
}

// process this frame for the chatbox
int chatbox_process(int key_in)
{	
	int key_out;

	key_out = key_in;

	// if the chatbox hasn't explicitly been created, we can't do any processing
	if(!Chatbox_created){
		return key_out;
	}

	// process the incoming key appropriately
	if (key_in == -1) {
		key_out = Chat_window.process();
	} else {
		key_out = Chat_window.process(key_in);
	}

	// look for special keypresses
	switch(key_out){
	// line recall up one
	case KEY_UP:
		chatbox_recall_up();
		key_out = 0;
		break;
	
	// line recall down one
	case KEY_DOWN:
		chatbox_recall_down();
		key_out = 0;
		break;
	}

	// if we're supposed to be checking our own scroll buttons
	if((Chatbox_mode_flags & CHATBOX_FLAG_BUTTONS) && (Chatbox_mode_flags & CHATBOX_FLAG_DRAW_BOX)){
		if ( Chatbox_buttons[gr_screen.res][CHATBOX_SCROLL_UP].button.pressed() ) {
			chatbox_scroll_up();
		}

		if ( Chatbox_buttons[gr_screen.res][CHATBOX_SCROLL_DOWN].button.pressed() ) {
			chatbox_scroll_down();
		}

		if ( Chatbox_buttons[gr_screen.res][CHATBOX_TOGGLE_SIZE].button.pressed() ){
			chatbox_toggle_size();
		}
	}

	// check to see if the enter text button has been pressed
	if ( Chat_enter_text.pressed() ) {
		Chat_inputbox.set_focus();
	}

	// check to see if the current input text needs to be split up and sent automaticall
	chatbox_autosplit_line();	

	return key_out;
}

void chatbox_close()
{
	if (!Chatbox_created)
		return;

	// destory the UI window
	Chat_window.destroy();

	// unload any bitmaps
	if (Chatbox_small_bitmap != -1) {
		bm_release(Chatbox_small_bitmap);
		Chatbox_small_bitmap = -1;
	}

	if (Chatbox_big_bitmap != -1) {
		bm_release(Chatbox_big_bitmap);
		Chatbox_big_bitmap = -1;
	}

	if (Chatbox_mp_bitmap != -1) {
		bm_release(Chatbox_mp_bitmap);
		Chatbox_mp_bitmap = -1;
	}	

	// clear all the text lines in the
	chatbox_clear();
	Chatbox_created = 0;
}

// shutdown all chatbox functionality
void chatbox_clear()
{
	// only do this if we have valid data
	if(Chatbox_created){
		// clear all chat data
		memset(Brief_chat_lines,0,(MAX_BRIEF_CHAT_LINES * (CHATBOX_MAX_LEN + 2)));		
		Brief_start_display_index=0;
		Num_brief_chat_lines=0;
		Brief_current_add_line=0;

		// clear all line recall data
		Chatbox_recall_count = 0;
		Chatbox_recall_index = 0;
		Chatbox_recall_last = -1;
		memset(Chatbox_recall_lines,0,CHATBOX_MAX_RECALL_LINES * (CHATBOX_MAX_LEN + 2));	
	}
}

// render the chatbox for this frame
void chatbox_render()
{
	if (!Chatbox_created){
		return;
	}

	// clear the multiplayer chat window
	// gr_set_clip(Chatbox_x1, Chatbox_y1, Chatbox_w, Chatbox_h);
	// gr_clear();
	// gr_reset_clip();

	// draw the background bitmap if we're supposed to
	if ( (Chatbox_bitmap != -1) && (Chatbox_mode_flags & CHATBOX_FLAG_DRAW_BOX)) {
		gr_set_bitmap( Chatbox_bitmap );
		gr_bitmap(Chatbox_x1, Chatbox_y1);		
	}

	// render the chat lines
	chatbox_render_chat_lines();

	// render any UI window stuff
	Chat_window.draw();
}

// try and scroll the chatbox up. return 0 or 1 on fail or success
int chatbox_scroll_up()
{	
	if(Num_brief_chat_lines > Chatbox_max_lines){
		int prev = Brief_chat_prev_index[Brief_start_display_index];
	
		// check to make sure we won't be going "up" above the "beginning" of the text array
	   if(Brief_chat_prev_index[Brief_current_add_line] != prev && !(Num_brief_chat_lines < MAX_BRIEF_CHAT_LINES && Brief_start_display_index==0)){
			Brief_start_display_index = prev;
			return 1;
		}
		
	} 
	return 0;
}

// try and scroll the chatbox down, return 0 or 1 on fail or success
int chatbox_scroll_down()
{
	int idx;
	int next;
	
   if(Num_brief_chat_lines > Chatbox_max_lines){
		// yuck. There's got to be a better way to do this.
		next = Brief_chat_next_index[Brief_start_display_index];
		for(idx = 1;idx <= Chatbox_max_lines; idx++){
			next = Brief_chat_next_index[next];
		}
			
		// check to make sure we won't be going "down" below the "bottom" of the text array
		if(Brief_chat_next_index[Brief_current_add_line] != next){
			Brief_start_display_index = Brief_chat_next_index[Brief_start_display_index];
			return 1;
		}
	}
	return 0;
}
	
// quick explanation as to how the scrolling works :
// Brief_chat_next_index is an array A of size n, where A[i] = i+1 and A[n] = 0
// Brief_chat_prev_index is an array A of size n, where A[i] = i-1 and A[0] = n
// in other words, if you increment an integer i = A[i], you get the next index (or the prev)
// with wrapping. In this way, as chat lines are entered, they are continuously wrapped 
// around the Brief_chat_lines array so we can keep it at a fixed size. These arrays are used
// for both entering new chat strings as well as moving the Brief_start_display_index 
// integer, which is self-explanatory

void chatbox_chat_init()
{
	int idx;	
		
   chatbox_clear();

	// setup the wraparound arrays
	for(idx=0;idx<MAX_BRIEF_CHAT_LINES;idx++){
      Brief_chat_next_index[idx] = idx+1;
	}
	Brief_chat_next_index[MAX_BRIEF_CHAT_LINES-1]=0;

	for(idx=MAX_BRIEF_CHAT_LINES-1; idx > 0 ; idx--){
		Brief_chat_prev_index[idx] = idx - 1;
	}
	Brief_chat_prev_index[0] = MAX_BRIEF_CHAT_LINES-1;	

	// initialize the line recall data
	Chatbox_recall_count = 0;
	Chatbox_recall_index = 0;
	Chatbox_recall_last = -1;
	memset(Chatbox_recall_lines,0,CHATBOX_MAX_RECALL_LINES * (CHATBOX_MAX_LEN + 2));	
}

// int Test_color = 0;
void chatbox_add_line(char *msg, int pid, int add_id)
{
	int backup;
	int	n_lines,idx;
	int	n_chars[3];		
	char	*p_str[3];			// for the initial line (unindented)
	char msg_extra[CHATBOX_STRING_LEN];

	if(!Chatbox_created){
		return;
	}

	// maybe stick on who sent the message	
	if(add_id){
		if(MULTI_STANDALONE(Net_players[pid])){
			sprintf(msg_extra, NOX("%s %s"), NOX("<SERVER>"), msg );
		} else {
			sprintf(msg_extra, NOX("%s: %s"), Net_players[pid].m_player->short_callsign, msg );
		}
	} else {
		strcpy_s(msg_extra,msg);
	}	
	Assert(strlen(msg_extra) < (CHATBOX_STRING_LEN - 2));	

	// split the text up into as many lines as necessary
	n_lines = split_str(msg_extra, Chatbox_disp_w, n_chars, p_str, 3);
	Assert(n_lines != -1);	

	// setup the first line -- be sure to clear out the line
	memset( Brief_chat_lines[Brief_current_add_line], 0, CHATBOX_STRING_LEN );

	// add the player id #
	Brief_chat_lines[Brief_current_add_line][0] = (char)(pid % 16);	
	// Brief_chat_lines[Brief_current_add_line][0] = (char)Test_color;	
	// Test_color = (Test_color == MAX_PLAYERS - 1) ? 0 : Test_color++;

	// set the indent to 0
	Brief_chat_indents[Brief_current_add_line] = 0;

	// copy in the chars
	strncpy(&Brief_chat_lines[Brief_current_add_line][1],p_str[0],CHATBOX_STRING_LEN - 1);
	if(n_chars[0] >= CHATBOX_STRING_LEN){
		Brief_chat_lines[Brief_current_add_line][CHATBOX_STRING_LEN - 1] = '\0';
	} else {
		Brief_chat_lines[Brief_current_add_line][n_chars[0] + 1] = '\0';
	}
	
	// increment the total line count if we haven't reached the max already
	if(Num_brief_chat_lines<MAX_BRIEF_CHAT_LINES){
		Num_brief_chat_lines++;
	}
	
	// get the index of the next string to add text to
	Brief_current_add_line = Brief_chat_next_index[Brief_current_add_line];	
	
	// if we have more than 1 line, re-split everything so that the rest are indented
	if(n_lines > 1){
		// split up the string after the first break-marker
		n_lines = split_str(msg_extra + n_chars[0],Chatbox_disp_w - CHAT_LINE_INDENT,n_chars,p_str,3);
		Assert(n_lines != -1);		

		// setup these remaining lines
		for(idx=0;idx<n_lines;idx++){
			// add the player id#
			Brief_chat_lines[Brief_current_add_line][0] = (char)(pid % 16);	

			// add the proper indent
			Brief_chat_indents[Brief_current_add_line] = CHAT_LINE_INDENT;

			// copy in the line text itself
			strncpy(&Brief_chat_lines[Brief_current_add_line][1],p_str[idx],CHATBOX_STRING_LEN-1); 
			if(n_chars[idx] >= CHATBOX_STRING_LEN){
				Brief_chat_lines[Brief_current_add_line][CHATBOX_STRING_LEN - 1] = '\0';
			} else {
				Brief_chat_lines[Brief_current_add_line][n_chars[idx] + 1] = '\0';
			}
			
			// increment the total line count if we haven't reached the max already
			if(Num_brief_chat_lines<MAX_BRIEF_CHAT_LINES){
				Num_brief_chat_lines++;
			}
			
			// get the index of the next line to add text to
			Brief_current_add_line = Brief_chat_next_index[Brief_current_add_line];				
		}
	}
			
	// COMMAND LINE OPTION
	if(Cmdline_multi_stream_chat_to_file && Multi_chat_stream!=NULL && msg[0] != '\0'){ // stream to the file if we're supposed to
		cfwrite_string(msg,Multi_chat_stream);
		cfwrite_char('\n',Multi_chat_stream);
	}	

	// if this line of text is from the player himself, automatically go to the bottom of
	// the chat list
	if(pid == MY_NET_PLAYER_NUM){
		if(Num_brief_chat_lines > Chatbox_max_lines){
			Brief_start_display_index = Brief_current_add_line;
			for(backup = 1;backup <= Chatbox_max_lines;backup++){
				Brief_start_display_index = Brief_chat_prev_index[Brief_start_display_index];
			}
		}
	}
	// if we have enough lines of text to require scrolling, scroll down by one.
	else { 
		chatbox_scroll_down();
	}	
}

void chatbox_render_chat_lines()
{
   int started_at,player_num,count,ly;	
	
	started_at = Brief_start_display_index;
	count = 0;	
	ly = Chatbox_begin_y;
	while((count < Chatbox_max_lines) && (count < Num_brief_chat_lines)){	
		// determine what player this chat line came from, and set the appropriate text color
		player_num = Brief_chat_lines[started_at][0];	   		
		
		// print the line out
		gr_set_color_fast(Color_netplayer[player_num]);

		// draw the first line (no indent)						
		gr_string(Chatbox_begin_x + Brief_chat_indents[started_at],ly,&Brief_chat_lines[started_at][1]);		

		// if we're in a team vs. team game, blit the player color icon
		if(Netgame.type_flags & NG_TYPE_TEAM){
			switch(Net_players[player_num].p_info.team){
			case 0 :
				// if he's a team captain
				if(Net_players[player_num].flags & NETINFO_FLAG_TEAM_CAPTAIN){
					if(Multi_common_icons[MICON_TEAM0_SELECT] != -1){
						gr_set_bitmap(Multi_common_icons[MICON_TEAM0_SELECT]);
						gr_bitmap(Chatbox_icon_x,ly-2);
					} 
				}
				// just you're average peon
				else {
					if(Multi_common_icons[MICON_TEAM0] != -1){
						gr_set_bitmap(Multi_common_icons[MICON_TEAM0]);
						gr_bitmap(Chatbox_icon_x,ly-2);
					}
				}
				break;
			case 1 :
				// if he's a team captain
				if(Net_players[player_num].flags & NETINFO_FLAG_TEAM_CAPTAIN){
					if(Multi_common_icons[MICON_TEAM1_SELECT] != -1){
						gr_set_bitmap(Multi_common_icons[MICON_TEAM1_SELECT]);
						gr_bitmap(Chatbox_icon_x,ly-2);
					}
				}
				// just your average peon
				else {
					if(Multi_common_icons[MICON_TEAM1] != -1){
						gr_set_bitmap(Multi_common_icons[MICON_TEAM1]);
						gr_bitmap(Chatbox_icon_x,ly-2);
					}
				}
				break;
			}
		}

		// increment the count and line position
		count++;		
		ly += 10;
		
		// increment the started at index
		started_at = Brief_chat_next_index[started_at];
	}		
}

void chatbox_toggle_size()
{
	// if we're in "small" mode
	if(Chatbox_mode_flags & CHATBOX_FLAG_SMALL){
		chatbox_force_big();
		
		// play a sound
		gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
	}
	// if we're in "big" mode
	else if(Chatbox_mode_flags & CHATBOX_FLAG_BIG){
		chatbox_force_small();
		
		// play a sound
		gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
	}
}

void chatbox_toggle_size_adjust_lines()
{
	int count_diff;
	
	// if the chatbox is now big, move the starting display index _up_ as far as possible
	if(Chatbox_mode_flags & CHATBOX_FLAG_BIG){				
		// if we've wrapped around	or we have more chatlines then we can display, move back as far as we can
		if((Num_brief_chat_lines > MAX_BRIEF_CHAT_LINES) || (Num_brief_chat_lines > Chatbox_max_lines)){			
			count_diff = Chatbox_max_lines - chatbox_num_displayed_lines();		
			while(count_diff > 0){
				Brief_start_display_index = Brief_chat_prev_index[Brief_start_display_index];
				count_diff--;
			}
		}
		// otherwise start displaying from position 0
		else {					
			Brief_start_display_index = 0;			
		}			
	}
	// if the chatbox is now small, move the starting display index down as far as we need
	else if(Chatbox_mode_flags & CHATBOX_FLAG_SMALL){
		count_diff = chatbox_num_displayed_lines();
		// if we were displaying more lines than we can now
		if(count_diff > Chatbox_max_lines){
			count_diff -= Chatbox_max_lines;
			while(count_diff > 0){
				Brief_start_display_index = Brief_chat_next_index[Brief_start_display_index];
				count_diff--;
			}
		}
	}
}

int chatbox_num_displayed_lines()
{
	int idx = Brief_start_display_index;
	int count;

	// count the lines up
	count = 0;
	while(idx != Brief_current_add_line){
		idx = Brief_chat_next_index[idx];
		count++;
	}

	return count;
}

// force the chatbox to go into small mode (if its in large mode) - will not wotk if in multi paused chatbox mode
void chatbox_force_small()
{
	int new_mode_flags;

	// don't do anything unless we're currently in "big" mode
	if(!(Chatbox_mode_flags & CHATBOX_FLAG_BIG)){
		return;
	}

	new_mode_flags = Chatbox_mode_flags;

	// switch to the appropriate mode
	new_mode_flags &= ~(CHATBOX_FLAG_SMALL | CHATBOX_FLAG_BIG);	
	new_mode_flags |= CHATBOX_FLAG_SMALL;
	Chatbox_bitmap = Chatbox_small_bitmap;
		
	// flip the up/down arrow
	Chatbox_buttons[gr_screen.res][CHATBOX_TOGGLE_SIZE].button.set_bmaps(Chatbox_buttons[gr_screen.res][CHATBOX_TOGGLE_SIZE].filename);
	
	// call this to set everything up correctly
	chatbox_set_mode(new_mode_flags);		
	
	// change the location of the input box
	Chat_inputbox.update_dimensions(Chatbox_inputbox_x, Chatbox_textenter_y, Chatbox_inputbox_w,15);
	Chat_inputbox.set_focus();

	// adjust what line we start displaying from based upon the new size of the window
	chatbox_toggle_size_adjust_lines();
}

// force the chatbox to go into big mode (if its in small mode) - will not work if in multi paused chatbox mode
void chatbox_force_big()
{
	int new_mode_flags;

	// don't do anything unless we're currently in "small" mode
	if(!(Chatbox_mode_flags & CHATBOX_FLAG_SMALL)){
		return;
	}

	new_mode_flags = Chatbox_mode_flags;	

	// switch to the appropriate mode
	new_mode_flags &= ~(CHATBOX_FLAG_SMALL | CHATBOX_FLAG_BIG);	
	new_mode_flags |= CHATBOX_FLAG_BIG;
	Chatbox_bitmap = Chatbox_big_bitmap;
		
	// flip the up/down arrow
	Chatbox_buttons[gr_screen.res][CHATBOX_TOGGLE_SIZE].button.set_bmaps(Chatbox_buttons[gr_screen.res][CHATBOX_TOGGLE_SIZE+1].filename);
	
	// call this to set everything up correctly
	chatbox_set_mode(new_mode_flags);		
	
	// change the location of the input box
	Chat_inputbox.update_dimensions(Chatbox_inputbox_x, Chatbox_textenter_y, Chatbox_inputbox_w,15);
	Chat_inputbox.set_focus();

	// adjust what line we start displaying from based upon the new size of the window
	chatbox_toggle_size_adjust_lines();
}

// "lose" the focus on the chatbox inputbox
void chatbox_lose_focus()
{
	if(!Chatbox_created){
		return;
	}

	// clear the focus on the inputbox
	Chat_inputbox.clear_focus();
}

// return if the inputbox for the chatbox currently has focus
int chatbox_has_focus()
{
	return Chat_inputbox.has_focus();
}

// grab the focus for the chatbox inputbox
void chatbox_set_focus()
{
	Chat_inputbox.set_focus();
}

// return if the inputbox was pressed - "clicked on"
int chatbox_pressed()
{
	return Chat_inputbox.pressed();
}

// add the string to the line recall list
void chatbox_recall_add(char *string)
{
	int idx;	

	// aleays reset the recall index when adding
	Chatbox_recall_index = 0;
	Chatbox_recall_last = -1;

	// if this string matches the last string we entered, don't add it again
	if(!strcmp(string,Chatbox_recall_lines[0])){
		return;
	}

	// move all items up (this works fine for small #'s of recall lines
	for(idx=CHATBOX_MAX_RECALL_LINES-1;idx > 0;idx--){
		memcpy(&Chatbox_recall_lines[idx],&Chatbox_recall_lines[idx-1],CHATBOX_MAX_LEN+2);
	}

	// copy the new item into spot 0
	strcpy_s(Chatbox_recall_lines[0],string);

	// increment the recall count if necessary
	if(Chatbox_recall_count < CHATBOX_MAX_RECALL_LINES){
		Chatbox_recall_count++;
	}	
}

// user has pressed the "up" key
void chatbox_recall_up()
{
	// if we've got no recall lines, do nothing
	if(Chatbox_recall_count <= 0){
		return;
	}

	// if we can increment up
	if(Chatbox_recall_index < (Chatbox_recall_count - 1)){
		// if this is the last line we recalled, pre-increment
		if(Chatbox_recall_last == Chatbox_recall_index){
			Chat_inputbox.set_text(Chatbox_recall_lines[++Chatbox_recall_index]);
			Chatbox_recall_last = Chatbox_recall_index;
		}
		// otherwise, post increment
		else {
			Chat_inputbox.set_text(Chatbox_recall_lines[Chatbox_recall_index++]);
			Chatbox_recall_last = Chatbox_recall_index - 1;
		}
	} 
	// if we can't increment up
	else {
		Chat_inputbox.set_text(Chatbox_recall_lines[Chatbox_recall_index]);
		Chatbox_recall_last = Chatbox_recall_index;
	}	
}

// user has pressed the "down" key
void chatbox_recall_down()
{	
	// if we've got no recall lines, do nothing
	if(Chatbox_recall_count <= 0){
		return;
	}

	// if we can decrement down
	if(Chatbox_recall_index > 0){
		// if this is the last line we recalled, pre-decrement
		if(Chatbox_recall_last == Chatbox_recall_index){
			Chat_inputbox.set_text(Chatbox_recall_lines[--Chatbox_recall_index]);
			Chatbox_recall_last = Chatbox_recall_index;
		} 
		// otherwise post,decrement
		else {
			Chat_inputbox.set_text(Chatbox_recall_lines[Chatbox_recall_index--]);
			Chatbox_recall_last = Chatbox_recall_index + 1;
		}
	} 
	// if we can't decrement down
	else {		
		Chat_inputbox.set_text("");
		Chatbox_recall_last = -1;
	}	
}

// reset all timestamps associated with the chatbox
void chatbox_reset_timestamps()
{
	int idx;

	// if there is no chatbox created, do nothing
	if(!Chatbox_created){
		return;
	}

	// otherwise clear all timestamps on all the buttons
	for(idx=0; idx<CHATBOX_NUM_BUTTONS+1; idx++){
		Chatbox_buttons[gr_screen.res][idx].button.reset_timestamps();
	}
}
