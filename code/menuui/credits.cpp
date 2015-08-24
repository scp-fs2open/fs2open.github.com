/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <stdlib.h>

#include "cfile/cfile.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/eventmusic.h"	/* for Master_event_music_volume */
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "graphics/font.h"
#include "io/key.h"
#include "io/timer.h"
#include "localization/localize.h"
#include "menuui/credits.h"
#include "missionui/missionscreencommon.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "sound/audiostr.h"
#include "ui/ui.h"



// This is the fs2_open credit list, please only add yourself if you have actually contributed code
// Rules!
char *fs2_open_credit_text = 
"SOURCE CODE PROJECT STAFF:\n"
	"\n"
	"Project Leader:\n"
	"\n"
	"Fabian \"The E\" Woltermann\n"
	"\n"
	"Senior Advisors:\n"
	"\n"
	"Ian \"Goober5000\" Warfield\n"
	"Michael \"Zacam\" LaFleur\n"
	"Taylor Richards\n"
	"Edward \"Inquisitor\" Gardner\n"
	"Cliff \"chief1983\" Gordon\n"
	"\n"
	"Programmers:\n"
	"\n"
	"Hassan \"Karajorma\" Kazmi\n"
	"Derek \"Kazan\" Meek\n"
	"Nick \"phreak\" Iannetta\n"
	"Mike \"Bobboau\" Abegg\n"
	"Backslash, Echelon9, Flaming_Sword\n"
	"FUBAR, Iss Mneur, kkmic\n"
	"Shade, Soulstorm, Sushi\n"
	"Swifty, Wanderer, CommanderDJ\n"
	"Valathil, MageKing17, Yarn\n"
	"m!m, z64555, zookeeper\n"
	"jg18, niffiwan\n"
	"\n"
	"\n"
	"Readme Staff:\n"
	"\n"
	"Peter \"Flipside\" Dibble\n"
	"\n"
	"Web Support:\n"
	"\n"
	"http://www.hard-light.net/\n"
	"http://scp.indiegames.us/\n"
	"\n"
	"Special thanks to:\n"
	"\n"
	"portej05\n"
	"Hery\n"
	"QuantumDelta\n"
	"Fury\n"
	"\n"
	"Linux and OSX version with the support of\n"
	"the icculus.org FreeSpace2 project:\n"
	"Steven \"relnev\" Fuller\n"
	"Ryan Gordon\n"
	"Charles Mason\n"
	"Dan Olson\n"
	"Taylor Richards\n"
	"\"tigital\"\n"
	"\n"
	"Very special thanks to:\n"
	"\n"
	"Volition for making FS2 such a great game\n"
	"Dave Baranec for giving us the code and keeping us sanity checked\n"
	"\n\n"
	"FS2_Open makes use of the following technologies:\n"
	"\n"
	"Ogg Vorbis - (C) 2005, Xiph.Org Foundation\n"
	"JPEG - Independent JPEG Group, (C) 1991-1998, Thomas G. Lane\n"
	"libpng - Copyright (C) 1998-2010 Glenn Randers-Pehrson\n"
	"liblua - Copyright (C) 1994-2008 Lua.org, PUC-Rio\n"
	"zlib - Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler\n"
	"FXAA - Copyright (c) 2010 NVIDIA Corporation. All rights reserved.\n"
	"\n"
	"\n"
	"\n";

char *unmodified_credits = "ORIGINAL VOLITION STAFF:\n\n\n";
char *mod_check = "Design:";

#define NUM_BUTTONS				5
#define DEFAULT_NUM_IMAGES		46

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

static char Credits_music_name[NAME_LENGTH];
static int	Credits_music_handle = -1;
static int	Credits_music_begin_timestamp;

static int	Credits_frametime;		// frametime of credits_do_frame() loop in ms
static int	Credits_last_time;		// timestamp used to calc frametime (in ms)
static float Credits_counter;

static int Credits_num_images;
static int Credits_artwork_index;
static SCP_vector<int> Credits_bmps;

// Positions for credits...
float Credit_start_pos, Credit_stop_pos, Credit_position = 0.0f;

static int Credits_music_delay				= 2000;
static float Credits_scroll_rate			= 15.0f;
static float Credits_artwork_display_time	= 9.0f;
static float Credits_artwork_fade_time		= 1.0f;

static SCP_vector<SCP_string> Credit_text_parts;

static bool Credits_parsed;

enum CreditsPosition
{
	START,
	END
};

static CreditsPosition SCP_credits_position	= START;

void credits_stop_music(bool fade)
{
	if ( Credits_music_handle != -1 ) {
		audiostream_close_file(Credits_music_handle, fade);
		Credits_music_handle = -1;
	}
}

void credits_load_music(char* fname)
{
	if ( Credits_music_handle != -1 ){
		return;
	}

	if ( fname && *fname ){
		Credits_music_handle = audiostream_open( fname, ASF_MENUMUSIC );
	}
}

void credits_start_music()
{
	if (Credits_music_handle != -1) {
		if ( !audiostream_is_playing(Credits_music_handle) ){
			audiostream_play(Credits_music_handle, Master_event_music_volume, 1);
		}
	} else {
		nprintf(("Warning", "Cannot play credits music\n"));
	}
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

void credits_parse_table(const char* filename)
{	
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// any metadata?
		if (optional_string("$Music:"))
		{
			stuff_string(Credits_music_name, F_NAME, NAME_LENGTH);
		}
		if (optional_string("$Number of Images:"))
		{
			int temp;
			stuff_int(&temp);
			if (temp > 0)
				Credits_num_images = temp;
		}
		if (optional_string("$Start Image Index:"))
		{
			stuff_int(&Credits_artwork_index);

			// bounds check
			if (Credits_artwork_index < 0)
			{
				Credits_artwork_index = 0;
			}
			else if (Credits_artwork_index >= Credits_num_images)
			{
				Credits_artwork_index = Credits_num_images - 1;
			}
		}
		if (optional_string("$Text scroll rate:"))
		{
			stuff_float(&Credits_scroll_rate);
			if (Credits_scroll_rate < 0.01f)
				Credits_scroll_rate = 0.01f;
		}
		if (optional_string("$Artworks display time:"))
		{
			stuff_float(&Credits_artwork_display_time);
			if (Credits_artwork_display_time < 0.01f)
				Credits_artwork_display_time = 0.01f;
		}
		if (optional_string("$Artworks fade time:"))
		{
			stuff_float(&Credits_artwork_fade_time);
			if (Credits_artwork_fade_time < 0.01f)
				Credits_artwork_fade_time = 0.01f;
		}
		if (optional_string("$SCP Credits position:"))
		{
			char mode[NAME_LENGTH];

			stuff_string(mode, F_NAME, NAME_LENGTH);

			if (!stricmp(mode, "Start"))
				SCP_credits_position = START;
			else if (!stricmp(mode, "End"))
				SCP_credits_position = END;
			else
				Warning(LOCATION, "Unknown credits position mode \"%s\".", mode);
		}

		ignore_white_space();

		SCP_string credits_text;
		SCP_string line;

		SCP_vector<int> charNum;
		SCP_vector<const char*> lines;
		int numLines = -1;

		bool first_run = true;
		while (!check_for_string_raw("#end"))
		{
			// Read in a line of text			
			stuff_string_line(line);

			// This is a bit odd but it means if a total conversion uses different credits the 
			// Volition credit won't happen
			// Also don't append the default credits anymore when there was already a parsed table
			if (first_run && !Credits_parsed && !line.compare(mod_check))
			{
				credits_text.append(unmodified_credits);
			}

			first_run = false;

			if (line.empty())
			{
				// If the line is empty then just append a newline, don't bother with splitting it first
				credits_text.append("\n");
			}
			else
			{
				// split_str doesn't take care of this.
				charNum.clear();

				// Split the string into multiple lines if it's too long
				numLines = split_str(line.c_str(), Credits_text_coords[gr_screen.res][2], charNum, lines, -1);

				// Make sure that we have valid data
				Assertion(lines.size() == (size_t)numLines, "split_str reported %d lines but vector contains " SIZE_T_ARG " entries!", numLines, lines.size());

				Assertion(lines.size() <= charNum.size(),
					"Something has gone wrong while splitting strings. Got " SIZE_T_ARG " lines but only " SIZE_T_ARG " chacter lengths.",
					lines.size(), charNum.size());

				// Now add all splitted lines to the credit text and append a newline to the end
				for (int i = 0; i < numLines; i++)
				{
					credits_text.append(SCP_string(lines[i], charNum[i]));
					credits_text.append("\n");
				}
			}
		}

		Credit_text_parts.push_back(credits_text);

		Credits_parsed = true;
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

void credits_parse()
{
	// Parse main table
	credits_parse_table("credits.tbl");

	// Parse modular tables
	parse_modular_table("*-crd.tbm", credits_parse_table);
}

void credits_init()
{
	int i;
	credits_screen_buttons *b;

	// pre-initialize
	Credits_num_images = DEFAULT_NUM_IMAGES;
	Credits_artwork_index = -1;

	// this is moved up here so we can override it if desired
	strcpy_s(Credits_music_name, "Cinema");

	// parse credits early so as to set up any overrides (for music and such)
	Credits_parsed = false;
	credits_parse();

	// we could conceivably have specified a number of images but not an index,
	// so if that's the case, set the value here
	if (Credits_artwork_index < 0) {
		Credits_artwork_index = rand() % Credits_num_images;
	}

	int credits_spooled_music_index = event_music_get_spooled_music_index(Credits_music_name);	
	if(credits_spooled_music_index != -1){
		char *credits_wavfile_name = Spooled_music[credits_spooled_music_index].filename;		
		if(credits_wavfile_name != NULL){
			credits_load_music(credits_wavfile_name);
		}
	}

	// Use this id to trigger the start of music playing on the briefing screen
	Credits_music_begin_timestamp = timestamp(Credits_music_delay);

	Credits_frametime = 0;
	Credits_last_time = timer_get_milliseconds();
	
	if (!Credits_parsed)
	{
		Credit_text_parts.push_back(SCP_string("No credits available.\n"));
	}
	else
	{
		switch (SCP_credits_position)
		{
			case START:
				Credit_text_parts.insert(Credit_text_parts.begin(), fs2_open_credit_text);
				break;

			case END:
				Credit_text_parts.push_back(fs2_open_credit_text);
				break;

			default:
				Error(LOCATION, "Unimplemented credits position %d. Get a coder!", (int) SCP_credits_position);
				break;
		}
	}

	int ch;
	SCP_vector<SCP_string>::iterator iter;

	for (iter = Credit_text_parts.begin(); iter != Credit_text_parts.end(); ++iter)
	{
		for (SCP_string::iterator ii = iter->begin(); ii != iter->end(); ++ii)
		{
			ch = *ii;
			switch (ch)
			{
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

			*ii = (char) ch;
		}
	}

	int temp_h;
	int h = 0;

	for (iter = Credit_text_parts.begin(); iter != Credit_text_parts.end(); ++iter)
	{
		gr_get_string_size(NULL, &temp_h, iter->c_str(), iter->length());

		h = h + temp_h;
	}

	Credit_start_pos = i2fl(Credits_text_coords[gr_screen.res][CREDITS_H_COORD]);
	Credit_stop_pos = -i2fl(h);
	Credit_position = Credit_start_pos;

	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
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

	Credits_bmps.resize(Credits_num_images);
	for (i=0; i<Credits_num_images; i++) {
		Credits_bmps[i] = -1;
	}
}

void credits_close()
{	
	int i;

	for (i=0; i<Credits_num_images; i++) {
		if (Credits_bmps[i] >= 0){
			bm_release(Credits_bmps[i]);
		}
	}	
	Credits_bmps.clear();

	credits_stop_music(true);

	Credit_text_parts.clear();

	if (Background_bitmap){
		bm_release(Background_bitmap);
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
		gr_bitmap(0, 0, GR_RESIZE_MENU);
	} 

	percent = (int) (100.0f - (Credits_artwork_display_time - Credits_counter) * 100.0f / Credits_artwork_fade_time);
	if (percent < 0){
		percent = 0;
	}

	next = Credits_artwork_index + 1;
	if (next >= Credits_num_images){
		next = 0;
	}

	if (Credits_bmps[Credits_artwork_index] < 0) {
		char buf[40];

		if (gr_screen.res == GR_1024) {
			sprintf(buf, NOX("2_CrIm%.2d"), Credits_artwork_index);
		} else {
			sprintf(buf, NOX("CrIm%.2d"), Credits_artwork_index);
		}
		Credits_bmps[Credits_artwork_index] = bm_load(buf);
	}

	if (Credits_bmps[next] < 0) {
		char buf[40];

		if (gr_screen.res == GR_1024) {
			sprintf(buf, NOX("2_CrIm%.2d"), next);
		} else {
			sprintf(buf, NOX("CrIm%.2d"), next);
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

		gr_cross_fade(bm1, bm2, bx1, by1, bx2, by2, (float)percent / 100.0f, GR_RESIZE_MENU);
	}

	Ui_window.draw();

	for (i=TECH_DATABASE_BUTTON; i<=CREDITS_BUTTON; i++){
		if (Buttons[i][gr_screen.res].button.button_down()){
			break;
		}
	}

	if (i > CREDITS_BUTTON){
		Buttons[CREDITS_BUTTON][gr_screen.res].button.draw_forced(2);
	}

	gr_set_clip(Credits_text_coords[gr_screen.res][CREDITS_X_COORD], Credits_text_coords[gr_screen.res][CREDITS_Y_COORD], Credits_text_coords[gr_screen.res][CREDITS_W_COORD], Credits_text_coords[gr_screen.res][CREDITS_H_COORD], GR_RESIZE_MENU);
	gr_set_font(FONT1);
	gr_set_color_fast(&Color_normal);
	
	int y_offset = 0;
	for (SCP_vector<SCP_string>::iterator iter = Credit_text_parts.begin(); iter != Credit_text_parts.end(); ++iter)
	{
		int height;

		gr_get_string_size(NULL, &height, iter->c_str(), iter->length());

		// Check if the text part is actually visible
		if (Credit_position + y_offset + height > 0.0f)
		{
			extern void gr_opengl_string(float sx, float sy, const char *s, int resize_mode);
			gr_opengl_string((float)0x8000, Credit_position + y_offset, iter->c_str(), GR_RESIZE_MENU);
		}

		y_offset += height;
	}

	int temp_time;
	temp_time = timer_get_milliseconds();

	Credits_frametime = temp_time - Credits_last_time;
	Credits_last_time = temp_time;
	timestamp_inc(Credits_frametime / 1000.0f);

	float fl_frametime = i2fl(Credits_frametime) / 1000.f;
	if (keyd_pressed[KEY_LSHIFT]) {
		Credit_position -= fl_frametime * Credits_scroll_rate * 4.0f;
	} else {
		Credit_position -= fl_frametime * Credits_scroll_rate;
	}

	if (Credit_position < Credit_stop_pos){
		Credit_position = Credit_start_pos;
	}

	Credits_counter += fl_frametime;
	while (Credits_counter >= Credits_artwork_display_time) {
		Credits_counter -= Credits_artwork_display_time;
		Credits_artwork_index = next;
	}

	gr_flip();
}
