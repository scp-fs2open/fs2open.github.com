/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/Credits.cpp $
 * $Revision: 2.17 $
 * $Date: 2004-03-05 09:01:53 $
 * $Author: Goober5000 $
 *
 * C source file for displaying game credits
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.16  2004/02/28 14:14:57  randomtiger
 * Removed a few uneeded if DIRECT3D's.
 * Set laser function to only render the effect one sided.
 * Added some stuff to the credits.
 * Set D3D fogging to fall back to vertex fog if table fog not supported.
 *
 * Revision 2.15  2004/02/14 00:18:33  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.14  2003/11/12 06:05:00  Goober5000
 * added Bobboau's real name to the credits, with his permission
 * --Goober5000
 *
 * Revision 2.13  2003/11/11 02:15:43  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.12  2003/11/07 07:12:21  Goober5000
 * added some credit stuff
 * --Goober5000
 *
 * Revision 2.11  2003/11/06 20:22:09  Kazan
 * slight change to .dsp - leave the release target as fs2_open_r.exe already
 * added myself to credit
 * killed some of the stupid warnings (including doing some casting and commenting out unused vars in the graphics modules)
 * Release builds should have warning level set no higher than 2 (default is 1)
 * Why are we getting warning's about function selected for inline expansion... (killing them with warning disables)
 * FS2_SPEECH was not defined (source file doesn't appear to capture preproc defines correctly either)
 *
 * Revision 2.10  2003/11/06 19:35:51  matt
 * Added myself to the credits. -Sticks
 *
 * Revision 2.9  2003/05/05 19:29:24  phreak
 * added my real name
 *
 * Revision 2.8  2003/03/18 10:07:03  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.7  2002/12/03 22:14:06  Goober5000
 * added myself to the credits list :)
 * ~Goober5000
 *
 * Revision 2.6  2002/11/02 22:25:26  inquisitor
 * Cleaned up the presentation a little, added HLP and VW, and, no offense to Kazan, removed Kazan.
 *
 * Revision 2.5  2002/11/02 22:11:35  DTP
 * DOH forgot \n
 *
 * Revision 2.4  2002/11/02 22:10:48  DTP
 * shameless self promotion
 *
 * Revision 2.3  2002/10/05 16:46:10  randomtiger
 * Added us fs2_open people to the credits. Worth looking at just for that.
 * Added timer bar code, by default its not compiled in.
 * Use TIMEBAR_ACTIVE in project and dependancy code settings to activate.
 * Added the new timebar files with the new code.
 *
 * Revision 2.2.2.1  2002/09/24 18:56:43  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.2  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.4  2002/05/17 03:05:08  mharris
 * more porting tweaks
 *
 * Revision 1.3  2002/05/16 06:07:38  mharris
 * more ifndef NO_SOUND
 *
 * Revision 1.2  2002/05/09 23:02:52  mharris
 * Not using default values for audiostream functions, since they may
 * be macros (if NO_SOUND is defined)
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 20    9/14/99 5:14a Dave
 * Fixed credits drawing in Glide.
 * 
 * 19    9/13/99 1:53p Dave
 * Fixed completely brain dead code in credits_init().
 * 
 * 18    9/09/99 10:55a Jefff
 * 
 * 17    9/03/99 11:45a Jefff
 * 
 * 16    9/01/99 5:28p Jefff
 * hi res art shows up now
 * 
 * 15    9/01/99 4:20p Jefff
 * mo' pictures
 * 
 * 14    9/01/99 12:19p Jefff
 * text splitting for long lines
 * 
 * 13    7/19/99 2:13p Dave
 * Added some new strings for Heiko.
 * 
 * 12    2/03/99 6:06p Dave
 * Groundwork for FS2 PXO usertracker support.  Gametracker support next.
 * 
 * 11    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 10    2/01/99 5:55p Dave
 * Removed the idea of explicit bitmaps for buttons. Fixed text
 * highlighting for disabled gadgets.
 * 
 * 9     1/30/99 9:01p Dave
 * Coord fixes.
 * 
 * 8     1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 7     1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 6     1/28/99 1:46a Dave
 * Updated coords and bitmaps.
 * 
 * 5     1/14/99 5:15p Neilk
 * changed credits, command debrief interfaces to high resolution support
 * 
 * 4     11/20/98 4:08p Dave
 * Fixed flak effect in multiplayer.
 * 
 * 3     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 18    6/19/98 3:51p Lawrance
 * deal with foreign chars in the credits
 * 
 * 17    6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 16    5/24/98 9:01p Lawrance
 * Add commit sounds when accept is pressed
 * 
 * 15    5/20/98 1:04p Hoffoss
 * Made credits screen use new artwork and removed rating field usage from
 * Fred (a goal struct member).
 * 
 * 14    5/12/98 4:17p Hoffoss
 * Make ctrl-arrows (up/down) switch between tech room screens.
 * 
 * 13    5/12/98 11:21a Hoffoss
 * Disabled cutscene screen and simulator room.
 * 
 * 12    5/11/98 8:04p Hoffoss
 * Fixed minor bugs.
 * 
 * 11    4/22/98 3:35p John
 * String externalization marking
 * 
 * 10    4/22/98 10:46a Hoffoss
 * Added images to credits screen.
 * 
 * 9     4/21/98 7:07p Hoffoss
 * Fixed problem where when switching screens flashes old tab hilight once
 * before switching to new state.
 * 
 * 8     4/17/98 3:28p Hoffoss
 * Added new credits screen code.
 * 
 * 7     3/05/98 11:15p Hoffoss
 * Changed non-game key checking to use game_check_key() instead of
 * game_poll().
 * 
 * 6     2/22/98 12:19p John
 * Externalized some strings
 * 
 * 5     1/05/98 2:30p John
 * made credits.tbl display
 * 
 * 4     9/19/97 5:14p Lawrance
 * use new naming convention for spooled music
 * 
 * 3     8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 2     4/22/97 11:06a Lawrance
 * credits music playing, credits screen is a separate state
 *
 * $NoKeywords: $
 */

#include <stdlib.h>

#include "menuui/credits.h"
#include "gamesequence/gamesequence.h"
#include "graphics/font.h"
#include "io/key.h"
#include "io/timer.h"
#include "gamesnd/gamesnd.h"
#include "sound/audiostr.h"
#include "gamesnd/eventmusic.h"	/* for Master_event_music_volume */
#include "ui/ui.h"
#include "missionui/missionscreencommon.h"
#include "playerman/player.h"
#include "freespace2/freespace.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "cfile/cfile.h"
#include "parse/parselo.h"

// This is the fs2_open credit list, please only add yourself if you have actually contributed code// Rules!
char *fs2_open_credit_text = 
	"FS2_OPEN STAFF:\n"
	"\n"
	"Project Leaders:\n"
	"\n"
	"Edward \"Inquisitor\" Gardner\n"
	"\n"
	"Ian \"Goober5000\" Warfield\n"
	"\n"
	"Programmers:\n"
	"\n"
	"Mike \"Bobboau\" Abegg\n"
	"Dennis \"DTP\" Pedersen\n"
	"Derek \"Kazan\" Meek\n"
	"Ian \"Goober5000\" Warfield\n"
	"Joe \"Righteous1\" Dowd\n"
	"Jon \"Sesquipedalian\" Stovell\n"
	"Matt \"Sticks\" Nischan\n"
	"Mike \"penguin\" Harris\n"
	"\"Mysterial\"\n"
	"Nick \"PhReAk\" Iannetta\n"
	"Thomas \"RandomTiger\" Whittaker\n"
	"Will \"##Unknown Player##\" Rousnel\n"
	"\"WMCoolmon\"\n"
	"\n"
	"Lead QA\n"
	"\n"
	"Chris \"Rga\" Pfingsten\n"
	"\n"
	"Readme Staff:\n"
	"\n"
	"\"Bottomfan\"\n"
	"\"Flipside\"\n"
	"\"Redmenace\"\n"
	"\n"
	"Web Support:\n"
	"\n"
	"Colin \"IceFire\" Czerneda and the staff at VolitionWatch.com\n"
	"Alex \"Thunder\" Avery and the staff at Hard Light Productions\n"
	"\n"
	"Special thanks to:\n"
	"\n"
	"Martin \"Maeglamor\"\n"
	"for donating a graphics card!\n"
	"\n"
	"\"Lightspeed\"\n"
	"for helping out whenever and whereever he could\n"
	"\n"
	"Very special thanks to:\n"
	"\n"
	"Volition for making FS2 such a great game\n"
	"Dave Baranec for giving us the code and keeping us sanity checked\n"
	"\n\n\n\n";

char *unmodified_credits = "ORIGINAL VOLITION STAFF:\n\n\n";
char *mod_check = "Design:";

#define CREDITS_MUSIC_DELAY	2000
#define CREDITS_SCROLL_RATE	15.0f
#define CREDITS_ARTWORK_DISPLAY_TIME	9.0f
#define CREDITS_ARTWORK_FADE_TIME		1.0f

#define NUM_BUTTONS				5
#define NUM_IMAGES				46

#define TECH_DATABASE_BUTTON	0
#define SIMULATOR_BUTTON		1
#define CUTSCENES_BUTTON		2
#define CREDITS_BUTTON			3
#define EXIT_BUTTON				4

// inidicies for coordinates
#define CREDITS_X_COORD 0
#define CREDITS_Y_COORD 1
#define CREDITS_W_COORD 2
#define CREDITS_H_COORD 3

static char* Credits_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"Credits",			// GR_640
	"2_Credits"
};

static char* Credits_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"Credits-M",			// GR_640
	"2_Credits-M"
};

int Credits_image_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		219, 15, 394, 286			// GR_640
	},
	{
		351, 25, 629, 455			// GR_1024
	}
};

// x, y, w, h
int Credits_text_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		26, 316, 482, 157			// GR_640
	},
	{
		144, 507, 568, 249			// GR_640
	}
};

struct credits_screen_buttons {
	char *filename;
	int x, y, xt, yt;
	int hotspot;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	credits_screen_buttons(char *name, int x1, int y1, int xt1, int yt1, int h) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h) {}
};

static int Background_bitmap;
/*
static int CreditsWin01 = -1;
static int CreditsWin02 = -1;
static int CreditsWin03 = -1;
static int CreditsWin04 = -1;
*/
static UI_WINDOW Ui_window;

static credits_screen_buttons Buttons[NUM_BUTTONS][GR_NUM_RESOLUTIONS] = {
//XSTR:OFF
	{
			credits_screen_buttons("TDB_00", 7, 3, 37, 7, 0),			// GR_640
			credits_screen_buttons("2_TDB_00", 12, 5, 59, 12, 0)			// GR_1024
	},
	{
			credits_screen_buttons("TDB_01", 7, 18, 37, 23,	1),		// GR_640
			credits_screen_buttons("2_TDB_01", 12, 31, 59, 37, 1)		// GR_1024
	},
	{
			credits_screen_buttons("TDB_02", 7, 34, 37, 38, 2),		// GR_640
			credits_screen_buttons("2_TDB_02", 12, 56, 59, 62, 2)		// GR_1024
	},
	{
			credits_screen_buttons("TDB_03", 7, 49, 37, 54,	3),		// GR_640
			credits_screen_buttons("2_TDB_03", 12, 81, 59, 88, 3)		// GR_1024
	},
	{
			credits_screen_buttons("CRB_04", 571, 425, 588, 413, 4),	// GR_640
			credits_screen_buttons("2_CRB_04", 914, 681, 953, 668, 4)	// GR_1024
	}
//XSTR:ON
};

static int	Credits_music_handle = -1;
static int	Credits_music_begin_timestamp;

static int	Credits_frametime;		// frametime of credits_do_frame() loop in ms
static int	Credits_last_time;		// timestamp used to calc frametime (in ms)
static float Credits_counter;
static int Credits_artwork_index;
static int Credits_bmps[NUM_IMAGES];

char *Credit_text = NULL;
int Credit_text_malloced = 0;			// TRUE if credit_text was malloced

// Positions for credits...
float Credit_start_pos, Credit_stop_pos, Credit_position = 0.0f;

void credits_stop_music()
{
#ifndef NO_SOUND
	if ( Credits_music_handle != -1 ) {
		audiostream_close_file(Credits_music_handle, 1);
		Credits_music_handle = -1;
	}
#endif
}

void credits_load_music(char* fname)
{
#ifndef NO_SOUND
	if ( Credits_music_handle != -1 ){
		return;
	}

	if ( fname ){
		Credits_music_handle = audiostream_open( fname, ASF_EVENTMUSIC );
	}
#endif
}

void credits_start_music()
{
#ifndef NO_SOUND
	if (Credits_music_handle != -1) {
		if ( !audiostream_is_playing(Credits_music_handle) ){
			audiostream_play(Credits_music_handle, Master_event_music_volume, 1);
		}
	} else {
		nprintf(("Warning", "Cannot play credits music\n"));
	}
#endif
}

int credits_screen_button_pressed(int n)
{
	switch (n) {
	case TECH_DATABASE_BUTTON:
		gamesnd_play_iface(SND_SWITCH_SCREENS);
		gameseq_post_event(GS_EVENT_TECH_MENU);
		return 1;

	case SIMULATOR_BUTTON:
		gamesnd_play_iface(SND_SWITCH_SCREENS);
		gameseq_post_event(GS_EVENT_SIMULATOR_ROOM);
		return 1;

	case CUTSCENES_BUTTON:
		gamesnd_play_iface(SND_SWITCH_SCREENS);
		gameseq_post_event(GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN);
		return 1;

	case EXIT_BUTTON:
		gamesnd_play_iface(SND_COMMIT_PRESSED);
		gameseq_post_event(GS_EVENT_MAIN_MENU);
		game_flush();
		break;
	}

	return 0;
}

void credits_init()
{
	int i, w, h;
	credits_screen_buttons *b;
	char line[512] = "";	
	char *linep1, *linep2;	

#ifndef NO_SOUND
	int credits_spooled_music_index = event_music_get_spooled_music_index("Cinema");	
	if(credits_spooled_music_index != -1){
		char *credits_wavfile_name = Spooled_music[credits_spooled_music_index].filename;		
		if(credits_wavfile_name != NULL){
			credits_load_music(credits_wavfile_name);
		}
	}
#endif

	// Use this id to trigger the start of music playing on the briefing screen
	Credits_music_begin_timestamp = timestamp(CREDITS_MUSIC_DELAY);

	Credits_frametime = 0;
	Credits_last_time = timer_get_milliseconds();

	Credit_text = NULL;
	Credit_text_malloced = 0;

	// allocate enough space for credits text
	CFILE *fp = cfopen( NOX("credits.tbl"), "rb" );
	if(fp != NULL){
		int size;
		size = cfilelength(fp);
		Credit_text = (char *) malloc(size + 200 + strlen(fs2_open_credit_text) + strlen(unmodified_credits));
		cfclose(fp);

		// open localization and parse
		lcl_ext_open();
		read_file_text("credits.tbl");
		reset_parse();

		// keep reading everything in
		strcpy(Credit_text,fs2_open_credit_text); 
	   
		bool first_run = true;
		while(!check_for_string_raw("#end")){ 
			
			stuff_string_line(line, 511);

			// This is a bit odd but it means if a total conversion uses different credits the 
			// Volition credit wont happen
			if(first_run == true)
			{
				if(strcmp(line, mod_check) == 0)
				{
					strcat(Credit_text,	unmodified_credits);	
				}

				first_run = false;
			}

			linep1 = line;

			do {
				linep2 = split_str_once(linep1, Credits_text_coords[gr_screen.res][2]);
				strcat(Credit_text, linep1);
				strcat(Credit_text, "\n");			
				linep1 = linep2;
			} while (linep2 != NULL);
		}		

		// close localization
		lcl_ext_close();	
	} else {
		Credit_text = NOX("No credits available.\n");
	}	

	int ch;
	for ( i = 0; Credit_text[i]; i++ ) {
			ch = Credit_text[i];
			switch (ch) {
			case -4:
				ch = 129;
				break;

			case -28:
				ch = 132;
				break;

			case -10:
				ch = 148;
				break;

			case -23:
				ch = 130;
				break;

			case -30:
				ch = 131;
				break;

			case -25:
				ch = 135;
				break;

			case -21:
				ch = 137;
				break;

			case -24:
				ch = 138;
				break;

			case -17:
				ch = 139;
				break;

			case -18:
				ch = 140;
				break;

			case -60:
				ch = 142;
				break;

			case -55:
				ch = 144;
				break;

			case -12:
				ch = 147;
				break;

			case -14:
				ch = 149;
				break;

			case -5:
				ch = 150;
				break;

			case -7:
				ch = 151;
				break;

			case -42:
				ch = 153;
				break;

			case -36:
				ch = 154;
				break;

			case -31:
				ch = 160;
				break;

			case -19:
				ch = 161;
				break;

			case -13:
				ch = 162;
				break;

			case -6:
				ch = 163;
				break;

			case -32:
				ch = 133;
				break;

			case -22:
				ch = 136;
				break;

			case -20:
				ch = 141;
				break;
			}
			Credit_text[i] = (char)ch;
	}

	gr_get_string_size(&w, &h, Credit_text);

	Credit_start_pos = i2fl(Credits_text_coords[gr_screen.res][CREDITS_H_COORD]);
	Credit_stop_pos = -i2fl(h);
	Credit_position = Credit_start_pos;

	Ui_window.create(0, 0, gr_screen.max_w, gr_screen.max_h, 0);
	Ui_window.set_mask_bmap(Credits_bitmap_mask_fname[gr_screen.res]);
	common_set_interface_palette("InterfacePalette");  // set the interface palette

	for (i=0; i<NUM_BUTTONS; i++) {
		b = &Buttons[i][gr_screen.res];

		b->button.create(&Ui_window, "", b->x, b->y, 60, 30, (i < 2), 1);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// add some text
	Ui_window.add_XSTR("Technical Database", 1055, Buttons[TECH_DATABASE_BUTTON][gr_screen.res].xt,  Buttons[TECH_DATABASE_BUTTON][gr_screen.res].yt, &Buttons[TECH_DATABASE_BUTTON][gr_screen.res].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Mission Simulator", 1056, Buttons[SIMULATOR_BUTTON][gr_screen.res].xt,  Buttons[SIMULATOR_BUTTON][gr_screen.res].yt, &Buttons[SIMULATOR_BUTTON][gr_screen.res].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Cutscenes", 1057, Buttons[CUTSCENES_BUTTON][gr_screen.res].xt,  Buttons[CUTSCENES_BUTTON][gr_screen.res].yt, &Buttons[CUTSCENES_BUTTON][gr_screen.res].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Credits", 1058, Buttons[CREDITS_BUTTON][gr_screen.res].xt,  Buttons[CREDITS_BUTTON][gr_screen.res].yt, &Buttons[CREDITS_BUTTON][gr_screen.res].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Exit", 1420, Buttons[EXIT_BUTTON][gr_screen.res].xt,  Buttons[EXIT_BUTTON][gr_screen.res].yt, &Buttons[EXIT_BUTTON][gr_screen.res].button, UI_XSTR_COLOR_PINK);

	if (Player->flags & PLAYER_FLAGS_IS_MULTI) {
		Buttons[SIMULATOR_BUTTON][gr_screen.res].button.disable();
		Buttons[CUTSCENES_BUTTON][gr_screen.res].button.disable();
	}

	Buttons[EXIT_BUTTON][gr_screen.res].button.set_hotkey(KEY_CTRLED | KEY_ENTER);

	Background_bitmap = bm_load(Credits_bitmap_fname[gr_screen.res]);
	Credits_artwork_index = rand() % NUM_IMAGES;
	for (i=0; i<NUM_IMAGES; i++){
		Credits_bmps[i] = -1;
	}

	// CreditsWin01 = bm_load(NOX("CreditsWin01"));
	// CreditsWin02 = bm_load(NOX("CreditsWin02"));
	// CreditsWin03 = bm_load(NOX("CreditsWin03"));
	// CreditsWin04 = bm_load(NOX("CreditsWin04"));
}

void credits_close()
{	
	int i;

	/*
	if (CreditsWin01 != -1){
		bm_unload(CreditsWin01);
		CreditsWin01 = -1;
	}
	if (CreditsWin02 != -1){
		bm_unload(CreditsWin02);
		CreditsWin02 = -1;
	}
	if (CreditsWin03 != -1){
		bm_unload(CreditsWin03);
		CreditsWin03 = -1;
	}
	if (CreditsWin04 != -1){
		bm_unload(CreditsWin04);
		CreditsWin04 = -1;
	}
	*/

	for (i=0; i<NUM_IMAGES; i++){
		if (Credits_bmps[i] >= 0){
			bm_unload(Credits_bmps[i]);
			Credits_bmps[i] = -1;
		}
	}	

	credits_stop_music();

	if (Credit_text) {
		if (Credit_text_malloced){
			free(Credit_text);
		}

		Credit_text = NULL;
	}

	if (Background_bitmap){
		bm_unload(Background_bitmap);
	}

	Ui_window.destroy();
	common_free_interface_palette();		// restore game palette
}

void credits_do_frame(float frametime)
{
	int i, k, next, percent, bm1, bm2;
	int bx1, by1, bw1, bh1;
	int bx2, by2, bw2, bh2;

	// Use this id to trigger the start of music playing on the credits screen
	if ( timestamp_elapsed(Credits_music_begin_timestamp) ) {
		Credits_music_begin_timestamp = 0;
		credits_start_music();
	}

	k = Ui_window.process();
	switch (k) {
	case KEY_ESC:
		gameseq_post_event(GS_EVENT_MAIN_MENU);
		key_flush();
		break;

	case KEY_CTRLED | KEY_UP:
	case KEY_SHIFTED | KEY_TAB:
		if ( !(Player->flags & PLAYER_FLAGS_IS_MULTI) ) {
			credits_screen_button_pressed(CUTSCENES_BUTTON);
			break;
		}
		// else, react like tab key.

	case KEY_CTRLED | KEY_DOWN:
	case KEY_TAB:
		credits_screen_button_pressed(TECH_DATABASE_BUTTON);
		break;

	default:
		break;
	} // end switch

	for (i=0; i<NUM_BUTTONS; i++){
		if (Buttons[i][gr_screen.res].button.pressed()){
			if (credits_screen_button_pressed(i)){
				return;
			}
		}
	}

	gr_reset_clip();	
	GR_MAYBE_CLEAR_RES(Background_bitmap);
	if (Background_bitmap >= 0) {
		gr_set_bitmap(Background_bitmap);
		gr_bitmap(0, 0);
	} 

	percent = (int) (100.0f - (CREDITS_ARTWORK_DISPLAY_TIME - Credits_counter) * 100.0f / CREDITS_ARTWORK_FADE_TIME);
	if (percent < 0){
		percent = 0;
	}

	next = Credits_artwork_index + 1;
	if (next >= NUM_IMAGES){
		next = 0;
	}

	if (Credits_bmps[Credits_artwork_index] < 0) {
		char buf[40];

		if (gr_screen.res == GR_1024) {
			sprintf(buf, NOX("2_CrIm%0.2d"), Credits_artwork_index);
		} else {
			sprintf(buf, NOX("CrIm%0.2d"), Credits_artwork_index);
		}
		Credits_bmps[Credits_artwork_index] = bm_load(buf);
	}

	if (Credits_bmps[next] < 0) {
		char buf[40];

		if (gr_screen.res == GR_1024) {
			sprintf(buf, NOX("2_CrIm%0.2d"), Credits_artwork_index);
		} else {
			sprintf(buf, NOX("CrIm%0.2d"), next);
		}
		Credits_bmps[next] = bm_load(buf);
	}

	bm1 = Credits_bmps[Credits_artwork_index];
	bm2 = Credits_bmps[next];

	if((bm1 != -1) && (bm2 != -1)){
		Assert(percent >= 0 && percent <= 100);

		// get width and height
		bm_get_info(bm1, &bw1, &bh1, NULL, NULL, NULL);	
		bm_get_info(bm2, &bw2, &bh2, NULL, NULL, NULL);	
	
		// determine where to draw the coords
		bx1 = Credits_image_coords[gr_screen.res][CREDITS_X_COORD] + ((Credits_image_coords[gr_screen.res][CREDITS_W_COORD] - bw1)/2);
		by1 = Credits_image_coords[gr_screen.res][CREDITS_Y_COORD] + ((Credits_image_coords[gr_screen.res][CREDITS_H_COORD] - bh1)/2);
		bx2 = Credits_image_coords[gr_screen.res][CREDITS_X_COORD] + ((Credits_image_coords[gr_screen.res][CREDITS_W_COORD] - bw2)/2);
		by2 = Credits_image_coords[gr_screen.res][CREDITS_Y_COORD] + ((Credits_image_coords[gr_screen.res][CREDITS_H_COORD] - bh2)/2);

		gr_cross_fade(bm1, bm2, bx1, by1, bx2, by2, (float)percent / 100.0f);
	}

	/*
	if (CreditsWin01 != -1) {
		gr_set_bitmap(CreditsWin01);
		gr_bitmap(233, 5);
	}

	if (CreditsWin02 != -1) {
		gr_set_bitmap(CreditsWin02);
		gr_bitmap(616, 8);
	}

	if (CreditsWin03 != -1) {
		gr_set_bitmap(CreditsWin03);
		gr_bitmap(233, 299);
	}

	if (CreditsWin04 != -1) {
		gr_set_bitmap(CreditsWin04);
		gr_bitmap(215, 8);
	}
	*/

	Ui_window.draw();

	for (i=TECH_DATABASE_BUTTON; i<=CREDITS_BUTTON; i++){
		if (Buttons[i][gr_screen.res].button.button_down()){
			break;
		}
	}

	if (i > CREDITS_BUTTON){
		Buttons[CREDITS_BUTTON][gr_screen.res].button.draw_forced(2);
	}

	gr_set_clip(Credits_text_coords[gr_screen.res][CREDITS_X_COORD], Credits_text_coords[gr_screen.res][CREDITS_Y_COORD], Credits_text_coords[gr_screen.res][CREDITS_W_COORD], Credits_text_coords[gr_screen.res][CREDITS_H_COORD]);
	gr_set_font(FONT1);
	gr_set_color_fast(&Color_normal);

	int sy;
	if ( Credit_position > 0 ) {
		sy = fl2i(Credit_position+0.5f);
	} else {
		sy = fl2i(Credit_position-0.5f);
	}

	gr_string(0x8000, sy, Credit_text);

	int temp_time;
	temp_time = timer_get_milliseconds();

	Credits_frametime = temp_time - Credits_last_time;
	Credits_last_time = temp_time;
	timestamp_inc(Credits_frametime / 1000.0f);

	float fl_frametime = i2fl(Credits_frametime) / 1000.f;
	if (keyd_pressed[KEY_LSHIFT]) {
		Credit_position -= fl_frametime * CREDITS_SCROLL_RATE * 4.0f;
	} else {
		Credit_position -= fl_frametime * CREDITS_SCROLL_RATE;
	}

	if (Credit_position < Credit_stop_pos){
		Credit_position = Credit_start_pos;
	}

	Credits_counter += fl_frametime;
	while (Credits_counter >= CREDITS_ARTWORK_DISPLAY_TIME) {
		Credits_counter -= CREDITS_ARTWORK_DISPLAY_TIME;
		Credits_artwork_index = next;
	}

	gr_flip();
}
