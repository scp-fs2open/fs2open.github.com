/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#include "globalincs/pstypes.h"
#include "mission/missionbriefcommon.h"
#include "missionui/fictionviewer.h"
#include "missionui/missioncmdbrief.h"
#include "missionui/missionscreencommon.h"
#include "missionui/redalert.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "gamesnd/eventmusic.h"
#include "io/key.h"
#include "freespace2/freespace.h"
#include "globalincs/alphacolors.h"



// ---------------------------------------------------------------------------------------------------------------------------------------
// MISSION FICTION VIEWER DEFINES/VARS
//
#define NUM_FVW_SETTINGS	2

char *Fiction_viewer_screen_filename[NUM_FVW_SETTINGS][GR_NUM_RESOLUTIONS] =
{
	{
		"FictionViewer",		// GR_640
		"2_FictionViewer"		// GR_1024
	},
	{
		"FictionViewerb",		// GR_640
		"2_FictionViewerb"		// GR_1024
	}
};

char *Fiction_viewer_screen_mask[NUM_FVW_SETTINGS][GR_NUM_RESOLUTIONS] =
{
	{
		"FictionViewer-m",		// GR_640
		"2_FictionViewer-m"		// GR_1024
	},
	{
		"FictionViewer-mb",		// GR_640
		"2_FictionViewer-mb"	// GR_1024
	}
};

int Fiction_viewer_text_coordinates[NUM_FVW_SETTINGS][GR_NUM_RESOLUTIONS][4] =
{
	// standard FS2-style interface
	{
		{ // GR_640
			17,		37,		588,	344
		},
		{ // GR_1024
			25,		48,		944,	576
		}
	},
	// WCS-style interface
	{
		{ // GR_640
			44,		50,		522,	348
		},
		{ // GR_1024
			71,		80,		835,	556
		}
	}
};

#define NUM_FVW_BUTTONS			3
#define FVW_BUTTON_ACCEPT		0
#define FVW_BUTTON_SCROLL_UP	1
#define FVW_BUTTON_SCROLL_DOWN	2

// the xt and yt fields aren't normally used for width and height,
// but the fields would go unused here and this is more
// convenient for initialization
ui_button_info Fiction_viewer_buttons[NUM_FVW_SETTINGS][GR_NUM_RESOLUTIONS][NUM_FVW_BUTTONS] =
{
	// standard FS2-style interface
	{
		{ // GR_640
			ui_button_info("fvw_accept_",	571,	425,	69,		55,		FVW_BUTTON_ACCEPT),
			ui_button_info("fvw_up_",		614,	14,		25,		31,		FVW_BUTTON_SCROLL_UP),
			ui_button_info("fvw_down_",		614,	370,	25,		31,		FVW_BUTTON_SCROLL_DOWN),
		//                 Filename          x       y     width  height
		},
		{ // GR_1024
			ui_button_info("2_fvw_accept_",	918,	688,	99,		77,		FVW_BUTTON_ACCEPT),
			ui_button_info("2_fvw_up_",		981,	16,		40,		50,		FVW_BUTTON_SCROLL_UP),
			ui_button_info("2_fvw_down_",	981,	606,	40,		50,		FVW_BUTTON_SCROLL_DOWN),
		}
	},
	// WCS-style interface
	{
		{ // GR_640
			ui_button_info("fvw_accept_",	105,	444,	37,		26,		FVW_BUTTON_ACCEPT),
			ui_button_info("fvw_up_",		576,	51,		37,		33,		FVW_BUTTON_SCROLL_UP),
			ui_button_info("fvw_down_",		576,	364,	37,		33,		FVW_BUTTON_SCROLL_DOWN),
		},
		{ // GR_1024
			ui_button_info("2_fvw_accept_",	168,	710,	59,		41,		FVW_BUTTON_ACCEPT),
			ui_button_info("2_fvw_up_",		922,	81,		59,		53,		FVW_BUTTON_SCROLL_UP),
			ui_button_info("2_fvw_down_",	922,	582,	59,		53,		FVW_BUTTON_SCROLL_DOWN),
		}
	}
};

char *Fiction_viewer_slider_filename[NUM_FVW_SETTINGS][GR_NUM_RESOLUTIONS] =
{
	// standard FS2-style interface
	{
		"slider",
		"2_slider"
	},
	// WCS-style interface
	{
		"fvw_slider_",
		"2_fvw_slider_"
	}
};

int Fiction_viewer_slider_coordinates[NUM_FVW_SETTINGS][GR_NUM_RESOLUTIONS][4] =
{
	// standard FS2-style interface
	{
		{ // GR_640
			618,	48,		18,		320		//Initial position x, initial position y, width, height of the slider column
		},
		{ // GR_1024
			988,	70,		28,		532
		}
	},
	// WCS-style interface
	{
		{ // GR_640
			589,	83,		16,		280
		},
		{ // GR_1024
			944,	132,	25,		451
		}
	}
};

int Top_fiction_viewer_text_line = 0;
int Fiction_viewer_text_max_lines = 0;

static UI_WINDOW Fiction_viewer_window;
static UI_SLIDER2 Fiction_viewer_slider;
static int Fiction_viewer_bitmap = -1;
static int Fiction_viewer_inited = 0;

static int Fiction_viewer_old_fontnum = -1;
static int Fiction_viewer_fontnum = -1;

static char Fiction_viewer_filename[MAX_FILENAME_LEN];
static char Fiction_viewer_font_filename[MAX_FILENAME_LEN];
static char *Fiction_viewer_text = NULL;

static int Fiction_viewer_ui = -1;

static void use_fv_font()
{
	// save old font and set new one
	if (Fiction_viewer_fontnum >= 0)
	{
		Fiction_viewer_old_fontnum = gr_get_current_fontnum();
		gr_set_font(Fiction_viewer_fontnum);
	}
	else
	{
		Fiction_viewer_old_fontnum = -1;
	}
}

static void use_std_font()
{
	// restore the old font
	if (Fiction_viewer_old_fontnum >= 0)
		gr_set_font(Fiction_viewer_old_fontnum);
}

// ---------------------------------------------------------------------------------------------------------------------------------------
// FICTION VIEWER FUNCTIONS
//

void fiction_viewer_exit()
{
	if (mission_has_cmd_brief())
		gameseq_post_event(GS_EVENT_CMD_BRIEF);
	else if (red_alert_mission())
		gameseq_post_event(GS_EVENT_RED_ALERT);
	else
		gameseq_post_event(GS_EVENT_START_BRIEFING);
}

void fiction_viewer_scroll_up()
{
	Top_fiction_viewer_text_line--;
	if (Top_fiction_viewer_text_line < 0)
	{
		Top_fiction_viewer_text_line = 0;
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
	else
	{
		gamesnd_play_iface(SND_SCROLL);
	}
}

void fiction_viewer_scroll_down()
{
	Top_fiction_viewer_text_line++;
	if ((Num_brief_text_lines[0] - Top_fiction_viewer_text_line) < Fiction_viewer_text_max_lines)
	{
		Top_fiction_viewer_text_line--;
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
	else
	{
		gamesnd_play_iface(SND_SCROLL);
	}
}

void fiction_viewer_scroll_capture()
{
	// nothing to do
}

// button press
void fiction_viewer_button_pressed(int button)
{
	switch (button)
	{
		case FVW_BUTTON_ACCEPT:
			fiction_viewer_exit();
			gamesnd_play_iface(SND_COMMIT_PRESSED);
			break;

		case FVW_BUTTON_SCROLL_UP:
			fiction_viewer_scroll_up();
			Fiction_viewer_slider.forceUp();
			break;

		case FVW_BUTTON_SCROLL_DOWN:
			fiction_viewer_scroll_down();
			Fiction_viewer_slider.forceDown();
			break;

		default:
			Int3();	// unrecognized button
			break;
	}
}

// init
void fiction_viewer_init()
{
	if (Fiction_viewer_inited)
		return;

	// no fiction viewer?
	if (!mission_has_fiction())
		return;

	// music
	common_music_init(SCORE_FICTION_VIEWER);

	// see if we have a background bitmap, and if so, which one
	// currently, we prioritize the UI that comes latest in the array;
	// in the future we might specify this in the mission or in a tbl
	for (Fiction_viewer_ui = NUM_FVW_SETTINGS - 1; Fiction_viewer_ui >= 0; Fiction_viewer_ui--)
	{
		// load the first available background bitmap
		Fiction_viewer_bitmap = bm_load(Fiction_viewer_screen_filename[Fiction_viewer_ui][gr_screen.res]);
		if (Fiction_viewer_bitmap >= 0)
			break;
	}
	
	// no ui is valid?
	if (Fiction_viewer_ui < 0)
	{
		Warning(LOCATION, "No fiction viewer graphics -- cannot display fiction viewer!");
		return;
	}

	// set up fiction viewer font
	use_fv_font();

	// calculate text area lines from font
	Fiction_viewer_text_max_lines = Fiction_viewer_text_coordinates[Fiction_viewer_ui][gr_screen.res][3] / gr_get_font_height();

	// window
	Fiction_viewer_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0, Fiction_viewer_fontnum);
	Fiction_viewer_window.set_mask_bmap(Fiction_viewer_screen_mask[Fiction_viewer_ui][gr_screen.res]);	

	// add the buttons
	for (int i = 0; i < NUM_FVW_BUTTONS; i++)
	{
		int repeat = (i == FVW_BUTTON_SCROLL_UP || i == FVW_BUTTON_SCROLL_DOWN);
		ui_button_info *b = &Fiction_viewer_buttons[Fiction_viewer_ui][gr_screen.res][i];

		b->button.create(&Fiction_viewer_window, "", b->x, b->y, b->xt, b->yt, repeat, 1);		
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// set up hotkeys for buttons
	Fiction_viewer_buttons[Fiction_viewer_ui][gr_screen.res][FVW_BUTTON_ACCEPT].button.set_hotkey(KEY_CTRLED | KEY_ENTER);
	Fiction_viewer_buttons[Fiction_viewer_ui][gr_screen.res][FVW_BUTTON_SCROLL_UP].button.set_hotkey(KEY_UP);
	Fiction_viewer_buttons[Fiction_viewer_ui][gr_screen.res][FVW_BUTTON_SCROLL_DOWN].button.set_hotkey(KEY_DOWN);

	// init brief text
	brief_color_text_init(Fiction_viewer_text, Fiction_viewer_text_coordinates[Fiction_viewer_ui][gr_screen.res][2], 0, 0);

	// if the story is going to overflow the screen, add a slider
	if (Num_brief_text_lines[0] > Fiction_viewer_text_max_lines)
	{
		Fiction_viewer_slider.create(&Fiction_viewer_window,
			Fiction_viewer_slider_coordinates[Fiction_viewer_ui][gr_screen.res][0],
			Fiction_viewer_slider_coordinates[Fiction_viewer_ui][gr_screen.res][1],
			Fiction_viewer_slider_coordinates[Fiction_viewer_ui][gr_screen.res][2],
			Fiction_viewer_slider_coordinates[Fiction_viewer_ui][gr_screen.res][3],
			Num_brief_text_lines[0] - Fiction_viewer_text_max_lines,
			Fiction_viewer_slider_filename[Fiction_viewer_ui][gr_screen.res],
			&fiction_viewer_scroll_up,
			&fiction_viewer_scroll_down,
			&fiction_viewer_scroll_capture);
	}
	
	Fiction_viewer_inited = 1;
}

// close
void fiction_viewer_close()
{
	if (!Fiction_viewer_inited)
		return;

	// free the fiction
	fiction_viewer_reset();

	// destroy the window
	Fiction_viewer_window.destroy();

	// restore the old font
	use_std_font();

	// free the bitmap
	if (Fiction_viewer_bitmap >= 0)
		bm_release(Fiction_viewer_bitmap);
	Fiction_viewer_bitmap = -1;

	// maybe stop music
	if (Mission_music[SCORE_FICTION_VIEWER] != Mission_music[SCORE_BRIEFING])
		common_music_close();
	
	game_flush();

	Fiction_viewer_inited = 0;
}

// do
void fiction_viewer_do_frame(float frametime)
{
	int i, k, w, h;

	// make sure we exist
	if (!Fiction_viewer_inited)
	{
		fiction_viewer_exit();
		return;
	}

	// process keys
	k = Fiction_viewer_window.process() & ~KEY_DEBUGGED;	

	switch (k)
	{
		case KEY_ESC:
			common_music_close();
			gameseq_post_event(GS_EVENT_MAIN_MENU);
			return;
	}

	// process button presses
	for (i = 0; i < NUM_FVW_BUTTONS; i++)
		if (Fiction_viewer_buttons[Fiction_viewer_ui][gr_screen.res][i].button.pressed())
			fiction_viewer_button_pressed(i);
	
	common_music_do();

	// clear
	GR_MAYBE_CLEAR_RES(Fiction_viewer_bitmap);
	if (Fiction_viewer_bitmap >= 0)
	{
		gr_set_bitmap(Fiction_viewer_bitmap);
		gr_bitmap(0, 0);
	} 
	
	// draw the window
	Fiction_viewer_window.draw();		

	// render the briefing text
	brief_render_text(Top_fiction_viewer_text_line, Fiction_viewer_text_coordinates[Fiction_viewer_ui][gr_screen.res][0], Fiction_viewer_text_coordinates[Fiction_viewer_ui][gr_screen.res][1], Fiction_viewer_text_coordinates[Fiction_viewer_ui][gr_screen.res][3], frametime);

	// maybe output the "more" indicator
	if ((Fiction_viewer_text_max_lines + Top_fiction_viewer_text_line) < Num_brief_text_lines[0])
	{
		use_std_font();

		// can be scrolled down
		int more_txt_x = Fiction_viewer_text_coordinates[Fiction_viewer_ui][gr_screen.res][0] + (Fiction_viewer_text_coordinates[Fiction_viewer_ui][gr_screen.res][2]/2) - 10;
		int more_txt_y = Fiction_viewer_text_coordinates[Fiction_viewer_ui][gr_screen.res][1] + Fiction_viewer_text_coordinates[Fiction_viewer_ui][gr_screen.res][3];				// located below text, centered

		gr_get_string_size(&w, &h, XSTR("more", 1469), strlen(XSTR("more", 1469)));
		gr_set_color_fast(&Color_black);
		gr_rect(more_txt_x-2, more_txt_y, w+3, h);
		gr_set_color_fast(&Color_red);
		gr_string(more_txt_x, more_txt_y, XSTR("more", 1469));  // base location on the input x and y?

		use_fv_font();
	}

	gr_flip();
}

int mission_has_fiction()
{
	if (Fred_running)
		return *Fiction_viewer_filename != 0;
	else
		return (Fiction_viewer_text != NULL);
}

char *fiction_file()
{
	return Fiction_viewer_filename;
}

char *fiction_font()
{
	return Fiction_viewer_font_filename;
}

void fiction_viewer_reset()
{
	if (Fiction_viewer_text != NULL)
		vm_free(Fiction_viewer_text);
	Fiction_viewer_text = NULL;

	*Fiction_viewer_filename = 0;
	*Fiction_viewer_font_filename = 0;

	Top_fiction_viewer_text_line = 0;
}

void fiction_viewer_load(char *filename, char *font_filename)
{
	int file_length;
	Assert(filename && font_filename);

	// just to be sure
	if (Fiction_viewer_text != NULL)
	{
		Int3();
		fiction_viewer_reset();
	}

	// save our filenames
	strcpy_s(Fiction_viewer_filename, filename);
	strcpy_s(Fiction_viewer_font_filename, font_filename);

	// see if we have a matching font
	Fiction_viewer_fontnum = gr_get_fontnum(Fiction_viewer_font_filename);
	if (Fiction_viewer_fontnum < 0 && !Fred_running)
		strcpy_s(Fiction_viewer_font_filename, "");

	if (!strlen(filename))
		return;

	// load up the text
	CFILE *fp = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_FICTION);
	if (fp == NULL)
	{
		Warning(LOCATION, "Unable to load fiction file '%s'.", filename);
		return;
	}

	// we don't need to copy the text in Fred
	if (!Fred_running)
	{
		// allocate space
		file_length = cfilelength(fp);
		Fiction_viewer_text = (char *) vm_malloc(file_length + 1);
		Fiction_viewer_text[file_length] = '\0';

		// copy all the text
		cfread(Fiction_viewer_text, file_length, 1, fp);
	}

	// we're done, close it out
	cfclose(fp);
}
