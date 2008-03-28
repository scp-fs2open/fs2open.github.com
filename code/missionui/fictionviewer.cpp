/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/MissionUI/FictionViewer.cpp $
 * $Revision: 1.1.2.2 $
 * $Date: 2007-11-22 05:25:22 $
 * $Author: taylor $
 *
 * Fiction Viewer briefing screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1.2.1  2007/11/21 07:27:48  Goober5000
 * add Wing Commander Saga's fiction viewer
 *
 *
 * $NoKeywords: $
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

char *Fiction_viewer_screen_filename[GR_NUM_RESOLUTIONS] =
{
	"fvw",			// GR_640
	"2_fvw",		// GR_1024
};

char *Fiction_viewer_screen_mask[GR_NUM_RESOLUTIONS] =
{
	"fvw-m",		// GR_640
	"2_fvw-m",		// GR_1024
};

#define NUM_FVW_BUTTONS			3
#define FVW_BUTTON_ACCEPT		0
#define FVW_BUTTON_SCROLL_UP	1
#define FVW_BUTTON_SCROLL_DOWN	2

// the xt and yt fields aren't normally used for width and height,
// but the fields would go unused here and this is more
// convenient for initialization
ui_button_info Fiction_viewer_buttons[GR_NUM_RESOLUTIONS][NUM_FVW_BUTTONS] =
{
	{ // GR_640
		ui_button_info("fvw_accept_",	105,	444,	37,		26,		0),
		ui_button_info("fvw_up_",		576,	51,		37,		33,		1),
		ui_button_info("fvw_down_",		576,	364,	37,		33,		2),
	},
	{ // GR_1024
		ui_button_info("2_fvw_accept_",	168,	710,	59,		41,		0),
		ui_button_info("2_fvw_up_",		922,	81,		59,		53,		1),
		ui_button_info("2_fvw_down_",	922,	582,	59,		53,		2),
	}
};

char *Fiction_viewer_slider_filename[GR_NUM_RESOLUTIONS] =
{
	"fvw_slider_",
	"2_fvw_slider_"
};

int Fiction_viewer_slider_coordinates[GR_NUM_RESOLUTIONS][4] =
{
	{ // GR_640
		589,	83,		16,		280
	},
	{ // GR_1024
		944,	132,	25,		451
	}
};

int Fiction_viewer_text_coordinates[GR_NUM_RESOLUTIONS][4] =
{
	{ // GR_640
		44,		50,		522,	348
	},
	{ // GR_1024
		71,		80,		835,	556
	}
};

int Top_fiction_viewer_text_line = 0;
int Fiction_viewer_text_max_lines[GR_NUM_RESOLUTIONS] =
{
	38, 61
};

static UI_WINDOW Fiction_viewer_window;
static UI_SLIDER2 Fiction_viewer_slider;
static int Fiction_viewer_bitmap;
static int Fiction_viewer_inited = 0;

static char Fiction_viewer_filename[MAX_FILENAME_LEN];
static char *Fiction_viewer_text = NULL;

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
	if ((Num_brief_text_lines[0] - Top_fiction_viewer_text_line) < Fiction_viewer_text_max_lines[gr_screen.res])
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

	// load the background bitmap
	Fiction_viewer_bitmap = bm_load(Fiction_viewer_screen_filename[gr_screen.res]);

	// no graphics?
	if (Fiction_viewer_bitmap < 0)
	{
		Int3();
		mprintf(("No fiction viewer graphics -- cannot display fiction viewer!\n"));
		return;
	}

	// window
	Fiction_viewer_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Fiction_viewer_window.set_mask_bmap(Fiction_viewer_screen_mask[gr_screen.res]);	

	// add the buttons
	for (int i = 0; i < NUM_FVW_BUTTONS; i++)
	{
		int repeat = (i == FVW_BUTTON_SCROLL_UP || i == FVW_BUTTON_SCROLL_DOWN);
		ui_button_info *b = &Fiction_viewer_buttons[gr_screen.res][i];

		b->button.create(&Fiction_viewer_window, "", b->x, b->y, b->xt, b->yt, repeat, 1);		
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// set up hotkeys for buttons
	Fiction_viewer_buttons[gr_screen.res][FVW_BUTTON_ACCEPT].button.set_hotkey(KEY_CTRLED | KEY_ENTER);
	Fiction_viewer_buttons[gr_screen.res][FVW_BUTTON_SCROLL_UP].button.set_hotkey(KEY_UP);
	Fiction_viewer_buttons[gr_screen.res][FVW_BUTTON_SCROLL_DOWN].button.set_hotkey(KEY_DOWN);

	// init brief text
	brief_color_text_init(Fiction_viewer_text, Fiction_viewer_text_coordinates[gr_screen.res][2]);

	// if the story is going to overflow the screen, add a slider
	if (Num_brief_text_lines[0] > Fiction_viewer_text_max_lines[gr_screen.res])
	{
		Fiction_viewer_slider.create(&Fiction_viewer_window,
			Fiction_viewer_slider_coordinates[gr_screen.res][0],
			Fiction_viewer_slider_coordinates[gr_screen.res][1],
			Fiction_viewer_slider_coordinates[gr_screen.res][2],
			Fiction_viewer_slider_coordinates[gr_screen.res][3],
			Num_brief_text_lines[0] - Fiction_viewer_text_max_lines[gr_screen.res],
			Fiction_viewer_slider_filename[gr_screen.res],
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

	// free the bitmap
	if (Fiction_viewer_bitmap >= 0)
		bm_release(Fiction_viewer_bitmap);
	Fiction_viewer_bitmap = -1;

	// destroy the window
	Fiction_viewer_window.destroy();

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
		if (Fiction_viewer_buttons[gr_screen.res][i].button.pressed())
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
	brief_render_text(Top_fiction_viewer_text_line, Fiction_viewer_text_coordinates[gr_screen.res][0], Fiction_viewer_text_coordinates[gr_screen.res][1], Fiction_viewer_text_coordinates[gr_screen.res][3], frametime);

	// maybe output the "more" indicator
	if ((Fiction_viewer_text_max_lines[gr_screen.res] + Top_fiction_viewer_text_line) < Num_brief_text_lines[0])
	{
		// can be scrolled down
		int more_txt_x = Fiction_viewer_text_coordinates[gr_screen.res][0] + (Fiction_viewer_text_coordinates[gr_screen.res][2]/2) - 10;
		int more_txt_y = Fiction_viewer_text_coordinates[gr_screen.res][1] + Fiction_viewer_text_coordinates[gr_screen.res][3] - 2;				// located below text, centered

		gr_get_string_size(&w, &h, XSTR("more", 1469), strlen(XSTR("more", 1469)));
		gr_set_color_fast(&Color_black);
		gr_rect(more_txt_x-2, more_txt_y, w+3, h);
		gr_set_color_fast(&Color_red);
		gr_string(more_txt_x, more_txt_y, XSTR("more", 1469));  // base location on the input x and y?
	}

	gr_flip();
}

int mission_has_fiction()
{
	return (Fiction_viewer_text != NULL);
}

char *fiction_file()
{
	return Fiction_viewer_filename;
}

void fiction_viewer_reset()
{
	if (Fiction_viewer_text != NULL)
		vm_free(Fiction_viewer_text);
	Fiction_viewer_text = NULL;

	Top_fiction_viewer_text_line = 0;
}

void fiction_viewer_load(char *filename)
{
	int file_length;

	// just to be sure
	if (Fiction_viewer_text != NULL)
	{
		Int3();
		fiction_viewer_reset();
	}

	// save our filename
	strcpy(Fiction_viewer_filename, filename);

	// load up the file
	CFILE *fp = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_FICTION);
	if (fp == NULL)
	{
		Warning(LOCATION, "Unable to load fiction file '%s'.", filename);
		return;
	}

	// allocate space
	file_length = cfilelength(fp);
	Fiction_viewer_text = (char *) vm_malloc(file_length + 1);
	Fiction_viewer_text[file_length] = '\0';

	// copy all the text
	cfread(Fiction_viewer_text, file_length, 1, fp);

	// we're done, close it out
	cfclose(fp);
}
