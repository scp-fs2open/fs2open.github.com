/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
 */ 

/*
 * $Logfile$
 * $Revision: 1.5 $
 * $Date: 2004-03-10 18:45:09 $
 * $Author: Kazan $
 *
 * C file for implementing PXO-substitute (FS2OX -- "fs2_open exchange") screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2004/03/08 22:02:39  Kazan
 * Lobby GUI screen restored
 *
 * Revision 1.3  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 1.2  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 1.1  2002/07/29 22:24:26  penguin
 * First attempt at "fs2_open exchange" (PXO substitute) screen
 *
 * $NoKeywords: $
 */


#pragma warning(disable:4786)
#include "globalincs/pstypes.h"
#include "ui/ui.h"
#include "bmpman/bmpman.h"
#include "gamesnd/gamesnd.h"
#include "io/key.h"
#include "gamesequence/gamesequence.h"
#include "network/fs2ox.h"
#include "gamehelp/contexthelp.h"
#include "network/multi_log.h"
#include "cmdline/cmdline.h"

//#define RATHAVEN

#if defined(RATHAVEN)
#include "fs2open_pxo/TCP_Socket.h"

TCP_Socket ServerConnection;
#else

#include "irc/irc.h"

irc_client IRCConn;


#endif

// LOCAL function definitions
void fs2ox_check_buttons();
void fs2ox_button_pressed(int n);

//XSTR:OFF
// bitmaps defs
static char *fs2ox_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"PXOChat",		// GR_640
	"2_PXOChat"		// GR_1024
};

static char *fs2ox_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"PXOChat-M",		// GR_640
	"2_PXOChat-M"		// GR_1024
};
//XSTR:ON

UI_WINDOW fs2ox_window;		// the window object for the FS2OX screen
int fs2ox_bitmap;													// the background bitmap
int fs2ox_frame_count;						// keep a count of frames displayed


// button defs
#define FS2OX_NUM_BUTTONS 15

#define FS2OX_USERS_SCROLL_UP			0
#define FS2OX_USERS_SCROLL_DOWN			1
#define FS2OX_WEB_RANKING				2
#define FS2OX_PILOT_INFO				3
#define FS2OX_PILOT_FIND				4
#define FS2OX_MOTD						5
#define FS2OX_CHANNELS_XXX1				6	// ?
#define FS2OX_CHANNELS_XXX2				7	// ?
#define FS2OX_CHANNELS_SCROLL_UP		8
#define FS2OX_CHANNELS_SCROLL_DOWN		9
#define FS2OX_CHATBOX_SCROLL_UP			10
#define FS2OX_CHATBOX_SCROLL_DOWN		11
#define FS2OX_EXIT						12
#define FS2OX_HELP						13
#define FS2OX_ACCEPT					14

//XSTR:OFF
ui_button_info fs2ox_buttons[GR_NUM_RESOLUTIONS][FS2OX_NUM_BUTTONS] = {
	{ // GR_640
		//					 filename   x     y      xt    yt   hotspot
		ui_button_info( "pxb_00",	1,		103,	-1,	-1,	0 ),						// users scroll up
		ui_button_info( "pxb_01",	1,		334,	-1,	-1,	1 ),						// users scroll down
		ui_button_info( "pxb_02",	17,		386,	-1,	-1,	2 ),						// web ranking
		ui_button_info( "pxb_03",	71,		385,	-1,	-1,	3 ),						// pilot info
		ui_button_info( "pxb_04",	115,	385,	-1,	-1,	4 ),						// find pilot
		ui_button_info( "pxb_05",	1,		443,	-1,	-1,	5 ),						// show_motd
		ui_button_info( "pxb_06",	330,	96,	-1,	-1,	6 ),						// channels xxx1
		ui_button_info( "pxb_07",	330,	131,	-1,	-1,	7 ),						// channels xxx2
		ui_button_info( "pxb_08",	618,	92,	-1,	-1,	8 ),						// channels scroll up
		ui_button_info( "pxb_09",	618,	128,	-1,	-1,	9 ),						// channels scroll down
		ui_button_info( "pxb_10",	615,	171,	-1,	-1,	10 ),						// chatbox scroll up
		ui_button_info( "pxb_11",	615,	355,	-1,	-1,	11 ),						// chatbox scroll down
		ui_button_info( "pxb_12",	481,	435,	-1,	-1,	12 ),						// exit
		ui_button_info( "pxb_13",	533,	433,	-1,	-1,	13 ),						// help
		ui_button_info( "pxb_14",	573,	432,	-1,	-1,	14 ),						// accept
	},
	{ // GR_1024
		// Positions fixed by Kazan 3/8/04
		ui_button_info( "2_pxb_00",	1,		164,	-1,	-1,	0 ),						// users scroll up
		ui_button_info( "2_pxb_01",	1,		534,	-1,	-1,	1 ),						// users scroll down
		ui_button_info( "2_pxb_02",	27,		617,	-1,	-1,	2 ),						// web ranking
		ui_button_info( "2_pxb_03",	113,	616,	-1,	-1,	3 ),						// pilot info
		ui_button_info( "2_pxb_04",	184,	616,	-1,	-1,	4 ),						// find pilot
		ui_button_info( "2_pxb_05",	1,		708,	-1,	-1,	5 ),						// show_motd
		ui_button_info( "2_pxb_06",	528,	119,	-1,	-1,	6 ),						// channels xxx1
		ui_button_info( "2_pxb_07",	528,	175,	-1,	-1,	7 ),						// channels xxx2
		ui_button_info( "2_pxb_08",	988,	112,	-1,	-1,	8 ),						// channels scroll up
		ui_button_info( "2_pxb_09",	988,	168,	-1,	-1,	9 ),						// channels scroll down
		ui_button_info( "2_pxb_10",	984,	241,	-1,	-1,	10 ),						// chatbox scroll up
		ui_button_info( "2_pxb_11",	984,	568,	-1,	-1,	11 ),						// chatbox scroll down
		ui_button_info( "2_pxb_12",	771,	695,	-1,	-1,	12 ),						// exit
		ui_button_info( "2_pxb_13",	852,	692,	-1,	-1,	13 ),						// help
		ui_button_info( "2_pxb_14",	916,	691,	-1,	-1,	14 ),						// accept
	}
};
//XSTR:ON


// text def
#define FS2OX_NUM_TEXT			9

UI_XSTR fs2ox_text[GR_NUM_RESOLUTIONS][FS2OX_NUM_TEXT] = {
	{ // GR_640		
		// string					xstr	x		y		color						font UI_GADGET
		{"Web",						1313,	18,		415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_WEB_RANKING].button},
		{"Ranking",					1314,	5,		425,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_WEB_RANKING].button},
		{"Pilot",					1310,	71,		415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_PILOT_INFO].button},
		{"Info",					1311,	74,		425,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_PILOT_INFO].button},
		{"Find",					1315,	120,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_PILOT_FIND].button},
		{"Motd",					1316,	34,		455,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_MOTD].button},
		{"Exit",					1416,	495,	420,	UI_XSTR_COLOR_PINK,  -1, &fs2ox_buttons[0][FS2OX_EXIT].button},	
		{"Help",					928,	540,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_HELP].button},
		{"Games",					1319,	590,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_ACCEPT].button},
	},
	{ // GR_1024		
		// Positions fixed by Kazan 3/8/04
		{"Web",						1313,	28,		664,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_WEB_RANKING].button},
		{"Ranking",					1314,	8,		680,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_WEB_RANKING].button},
		{"Pilot",					1310,	113,	664,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_PILOT_INFO].button},
		{"Info",					1311,	118,	680,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_PILOT_INFO].button},
		{"Find",					1315,	192,	664,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_PILOT_FIND].button},
		{"Motd",					1316,	54,		728,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_MOTD].button},
		{"Exit",					1416,	792,	672,	UI_XSTR_COLOR_PINK,  -1, &fs2ox_buttons[1][FS2OX_EXIT].button},	
		{"Help",					928,	864,	664,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_HELP].button},
		{"Games",					1319,	944,	664,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_ACCEPT].button},
	}
};

UI_INPUTBOX Chat_input;

#define NUM_LISTS	3
#define LIST_CHANS	0
#define LIST_USERS	1
#define LIST_MESGS	2

UI_LISTBOX ListBoxen[NUM_LISTS];
int Listbox_coordinates[GR_NUM_RESOLUTIONS][NUM_LISTS][4] =
{
	{ // GR_640
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 }
	},
	{ // GR_1024
		{ 590 ,120, 380, 85 },
		{ 45, 190, 215, 385 },
		{ 310, 265, 660, 315 }
	}
};


//*void create(UI_WINDOW *wnd, int _x, int _y, int _w, int _h, int _numitem, char **_list);

int Chat_input_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640 -- NOT YET CONFIRMED!
		196,	280,	412,	56
	},
	{ // GR_1024
		315,	615,	660,	90
	}
};

#define MAX_CHAT_MSGS	256
//#define MAX_CHAT_MSGLEN 127
char **MessageList[NUM_LISTS];

extern char Multi_tracker_login[100];
extern char Multi_tracker_passwd[100];
extern char Multi_tracker_squad_name[100];

/**
 * Initialize the FS2OX screen.
 */
void fs2ox_init()
{

#if defined(RATHAVEN)
	ServerConnection.InitSocket("207.215.71.69", 4616);
#else
	// mgo.maxgaming.org server A
	//IRCConn.connect(Multi_tracker_login, Multi_tracker_passwd, "65.85.207.108", 6667);

	
	// mgo.maxgaming.org server b
	IRCConn.connect(Multi_tracker_login, Multi_tracker_passwd, "68.76.67.228", 6667);

	// irc.sevarg.net
	//IRCConn.connect(Multi_tracker_login, Multi_tracker_passwd, "65.110.255.247", 6667);

#endif
	int idx;

	// TODO - init local variable
	
	// clear the message list
	for (idx=0; idx<NUM_LISTS; idx++)
		MessageList[idx] = new char*[MAX_CHAT_MSGS];

	
	// reset frame count
	fs2ox_frame_count = 0;

	// create the interface window
	fs2ox_window.create(0,0,gr_screen.max_w,gr_screen.max_h,0);
	fs2ox_window.set_mask_bmap(fs2ox_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	fs2ox_bitmap = bm_load(fs2ox_bitmap_fname[gr_screen.res]);
	if(fs2ox_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}


	// create the interface buttons
	for(idx=0; idx < FS2OX_NUM_BUTTONS; idx++){
		// create the object
		fs2ox_buttons[gr_screen.res][idx].button.create(&fs2ox_window,"", fs2ox_buttons[gr_screen.res][idx].x, fs2ox_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		fs2ox_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		fs2ox_buttons[gr_screen.res][idx].button.set_bmaps(fs2ox_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		fs2ox_buttons[gr_screen.res][idx].button.link_hotspot(fs2ox_buttons[gr_screen.res][idx].hotspot);
	}		

	for (idx=0; idx < NUM_LISTS; idx++){
		ListBoxen[idx].create(&fs2ox_window, Listbox_coordinates[gr_screen.res][idx][0], Listbox_coordinates[gr_screen.res][idx][1], 
												Listbox_coordinates[gr_screen.res][idx][2], Listbox_coordinates[gr_screen.res][idx][3], 
												0, MessageList[idx], NULL, MAX_CHAT_MSGS);
		ListBoxen[idx].set_drawframe(0);
	}

	// create all xstrs
	for(idx=0; idx < FS2OX_NUM_TEXT; idx++){
		fs2ox_window.add_XSTR(&fs2ox_text[gr_screen.res][idx]);
	}


	//
	Chat_input.create(&fs2ox_window, Chat_input_coords[gr_screen.res][0], Chat_input_coords[gr_screen.res][1], Chat_input_coords[gr_screen.res][2], Chat_input_coords[gr_screen.res][3], "", /*UI_INPUTBOX_FLAG_INVIS |*/ UI_INPUTBOX_FLAG_ESC_FOC);	
	Chat_input.set_text("");

	// TODO - load the help overlay
	//help_overlay_load(FS2OX_OVERLAY);
	//help_overlay_set_state(FS2OX_OVERLAY,0);

}


/**
 * Close the FS2OX screen.
 */
void fs2ox_close()
{
	// unload any bitmaps
	if(!bm_unload(fs2ox_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",fs2ox_bitmap_fname[gr_screen.res]));
	}


	// unload the help overlay
	//help_overlay_unload(FS2OX_OVERLAY);	
	
	// destroy the UI_WINDOW
	fs2ox_window.destroy();
	
	for (int idx=0; idx<NUM_LISTS; idx++)
	{
		delete[] MessageList[idx];
	}

#if defined(RATHAVEN)
	ServerConnection.Close();
#else
	if (IRCConn.isConnected())
		IRCConn.Disconnect("Exiting");
#endif
}


std::string strip_pattern(std::string needle, std::string haystack)
{
	int index = haystack.find(needle);

	while (index != std::string::npos)
	{
		haystack.erase(index, needle.length());

		index = haystack.find(needle);
	}

	return haystack;
}

std::string strip_ansi_codes(std::string str)
// Function to strip ANSI color codes
// search for '\27'
{

	const int num_codes = 66;
	std::string color_codes[66] = 
      		{ "\033[0;1m",  "\033[1;1m",  "\033[2;1m",  "\033[3;1m",  "\033[4;1m",  "\033[5;1m",  "\033[6;1m",  "\033[7;1m", 
			  "\033[9;1m",  "\033[22;1m", "\033[23;1m", "\033[24;1m", "\033[27;1m", "\033[29;1m", "\033[30;1m", "\033[31;1m", 
			  "\033[32;1m", "\033[33;1m", "\033[34;1m", "\033[35;1m", "\033[36;1m", "\033[37;1m", "\033[39;1m", "\033[40;1m", 
			  "\033[41;1m", "\033[42;1m", "\033[43;1m", "\033[44;1m", "\033[45;1m", "\033[46;1m", "\033[47;1m", "\033[49;1m",
			  "\033[0m",  "\033[1m",  "\033[2m",  "\033[3m",  "\033[4m",  "\033[5m",  "\033[6m",  "\033[7m", 
			  "\033[9m",  "\033[22m", "\033[23m", "\033[24m", "\033[27m", "\033[29m", "\033[30m", "\033[31m", 
			  "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m", "\033[39m", "\033[40m", 
			  "\033[41m", "\033[42m", "\033[43m", "\033[44m", "\033[45m", "\033[46m", "\033[47m", "\033[49m",
			  "\377\374\01", "\377\374\03" };

	for (int i = 0; i < num_codes; i++)
		str = strip_pattern(color_codes[i], str);


	return str;
}

/**
 * Frame processing for FS2OX screen.
 */
void fs2ox_do_frame()
{

	static bool firstrecv = true;
	char text[128];
	memset(text, 0, 128);

#if defined(RATHAVEN)
	if (Chat_input.pressed())
	{
		Chat_input.get_text(text);
		Chat_input.set_text("");


		text[strlen(text)] = '\n';
		ServerConnection.SendData(text, strlen(text)+1);

		/*
		if (text[0] == '+')
			ListBoxen[LIST_CHANS].add_string(text);
		else if (text[0] == '@')
			ListBoxen[LIST_USERS].add_string(text);
		else
			ListBoxen[LIST_MESGS].add_string(text);
			*/

	}

	while (ServerConnection.DataReady())
	{
		char input[1024]; // there probably won't be a transfer longer than this
		char temp[256]; //maximum line length
		int i = 0, j;

		int transfer = ServerConnection.GetData(input, 1024);

		while (i < transfer)
		{
			j=0;
			while (input[i] != '\n' && input[i] != '\r' && i < transfer)
			{

				temp[j] = input[i];	

				j++;
				i++;
			}
			temp[j] = '\0';
			if (ListBoxen[LIST_MESGS].CurSize() == ListBoxen[LIST_MESGS].MaxSize()-1)
			{
				ml_printf("Removed first line");
				ListBoxen[LIST_MESGS].RemoveFirstItem();
			}
		
			std::string text = strip_ansi_codes(temp);
			if (text.length() > 0 && text != "\n")
				ListBoxen[LIST_MESGS].add_string((char *)text.c_str());
			ml_printf("Lines Used: %d / %d", ListBoxen[LIST_MESGS].CurSize(), ListBoxen[LIST_MESGS].MaxSize());
			ListBoxen[LIST_MESGS].ScrollEnd();
			i++;
		}

		
	}
#else
	if (Chat_input.pressed())
	{
		Chat_input.get_text(text);
		Chat_input.set_text("");

		IRCConn.ParseForCommand(text);
	}

	std::vector<std::string> lines = IRCConn.Maybe_GetRawLines();

	if (lines.size() != 0)
	{
		for (int i = 0; i < lines.size(); i++)
		{
			if (ListBoxen[LIST_MESGS].CurSize() == ListBoxen[LIST_MESGS].MaxSize()-1)
			{
				ml_printf("Removed first line");
				ListBoxen[LIST_MESGS].RemoveFirstItem();
			}

			if (lines[i].length() > 0 && lines[i] != "\n")
				ListBoxen[LIST_MESGS].add_string((char *)lines[i].c_str());
			ml_printf("Lines Used: %d / %d", ListBoxen[LIST_MESGS].CurSize(), ListBoxen[LIST_MESGS].MaxSize());
			ListBoxen[LIST_MESGS].ScrollEnd();

		}
	}


#endif
	int k = fs2ox_window.process();

	// process any keypresses
	switch(k){
	case KEY_ESC :
  		//if(help_overlay_active(FS2OX_OVERLAY)){
  		//	help_overlay_set_state(FS2OX_OVERLAY,0);
  		//} else {		
			gameseq_post_event(GS_EVENT_MAIN_MENU);			
			gamesnd_play_iface(SND_USER_SELECT);
  		//}
		break;
	}

	// process any button clicks
	fs2ox_check_buttons();

	// draw the background, etc
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(fs2ox_bitmap);
	if(fs2ox_bitmap != -1){		
		gr_set_bitmap(fs2ox_bitmap);
		gr_bitmap(0,0);
	}
	fs2ox_window.draw();

	// flip the buffer
	gr_flip();

	// increment the frame count
	fs2ox_frame_count++;
}



void fs2ox_check_buttons()
{
	int idx;
	for(idx=0;idx<FS2OX_NUM_BUTTONS;idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(fs2ox_buttons[gr_screen.res][idx].button.pressed()){
			fs2ox_button_pressed(idx);
			break;
		}
	}
}



void fs2ox_button_pressed(int n)
{
	switch(n){
	// help overlay
  	case FS2OX_HELP:
  		//if(!help_overlay_active(FS2OX_OVERLAY)){
  		//	help_overlay_set_state(FS2OX_OVERLAY,1);
  		//} else {
  		//	help_overlay_set_state(FS2OX_OVERLAY,0);
  		//}
  		break;

	// go to the games list
	case FS2OX_ACCEPT:
#if !defined(RATHAVEN)
		IRCConn.Disconnect(std::string("Going to play game (With Mod \"") + Cmdline_mod + "\")");
#endif
		gameseq_post_event(GS_EVENT_MULTI_JOIN_GAME);
		break;

	case FS2OX_EXIT:
#if !defined(RATHAVEN)
		IRCConn.Disconnect("Leaving");
#endif
		gameseq_post_event(GS_EVENT_MAIN_MENU);
		break;

	default :
  		//multi_common_add_notify(XSTR("Not implemented yet!",760));
		gamesnd_play_iface(SND_GENERAL_FAIL);
		break;
	}
}
