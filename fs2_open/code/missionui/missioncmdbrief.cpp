/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MissionUI/MissionCmdBrief.cpp $
 * $Revision: 2.16 $
 * $Date: 2005-07-02 19:43:54 $
 * $Author: taylor $
 *
 * Mission Command Briefing Screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.15  2005/06/03 06:39:26  taylor
 * better audio pause/unpause support when game window loses focus or is minimized
 *
 * Revision 2.14  2005/03/10 08:00:08  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.13  2005/02/23 05:05:38  taylor
 * compiler warning fixes (for MSVC++ 6)
 * have the warp effect only load as many LODs as will get used
 * head off strange bug in release when corrupt soundtrack number gets used
 *    (will still Assert in debug)
 * don't ever try and save a campaign savefile in multi or standalone modes
 * first try at 32bit->16bit color conversion for TGA code (for TGA only ship textures)
 *
 * Revision 2.12  2005/01/31 23:27:54  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 2.11  2004/07/26 20:47:38  Kazan
 * remove MCD complete
 *
 * Revision 2.10  2004/07/12 16:32:54  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.9  2004/03/05 09:01:55  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.8  2003/11/16 04:08:47  Goober5000
 * fixed briefing scroll and display of "more"
 * --Goober5000
 *
 * Revision 2.7  2003/09/07 18:14:54  randomtiger
 * Checked in new speech code and calls from relevent modules to make it play.
 * Should all work now if setup properly with version 2.4 of the launcher.
 * FS2_SPEECH can be used to make the speech code compile if you have SAPI 5.1 SDK installed.
 * Otherwise the compile flag should not be set and it should all compile OK.
 *
 * - RT
 *
 * Revision 2.6  2003/04/05 11:09:22  Goober5000
 * fixed some fiddly bits with scrolling and ui stuff
 * --Goober5000
 *
 * Revision 2.5  2003/04/05 08:51:04  Goober5000
 * arg - forgot something
 * --Goober5000
 *
 * Revision 2.4  2003/04/05 08:46:51  Goober5000
 * command briefing scroll buttons implemented :)
 * --Goober5000
 *
 * Revision 2.3  2003/03/30 21:08:42  Goober5000
 * preliminary work on adding scroll buttons to command briefings
 * --Goober5000
 *
 * Revision 2.2  2003/03/18 10:07:04  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.1  2002/09/24 18:56:44  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/09 23:02:59  mharris
 * Not using default values for audiostream functions, since they may
 * be macros (if NO_SOUND is defined)
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 14    10/06/99 10:28a Jefff
 * for OEM: cmd brief anim defaults to the default anim if load failure
 * 
 * 13    9/03/99 1:32a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 12    8/27/99 12:04a Dave
 * Campaign loop screen.
 * 
 * 11    8/19/99 6:28p Jefff
 * move animation in 640 a bit
 * 
 * 10    7/15/99 4:11p Andsager
 * Leave command briefs in DEMO
 * 
 * 9     7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 8     7/09/99 10:32p Dave
 * Command brief and red alert screens.
 * 
 * 7     2/08/99 5:06p Johnson
 * Removed reference to a now non-existent palette.
 * 
 * 6     1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 5     1/14/99 5:15p Neilk
 * changed credits, command debrief interfaces to high resolution support
 * 
 * 4     10/16/98 9:40a Andsager
 * Remove ".h" files from model.h
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
 * 42    6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 41    6/05/98 9:54a Lawrance
 * OEM changes
 * 
 * 40    6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 39    5/26/98 11:10a Lawrance
 * Fix bug where window controls get disabled when F1 pressed twice
 * 
 * 38    5/23/98 10:38p Lawrance
 * Avoid doing a cfile refresh when running debug
 * 
 * 37    5/23/98 6:49p Lawrance
 * Fix problems with refreshing the file list when a CD is inserted
 * 
 * 36    5/22/98 11:15a Lawrance
 * Tweak how CD gets asked for
 * 
 * 35    5/21/98 6:57p Lawrance
 * Only ask for the CD once
 * 
 * 34    5/20/98 9:46p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 33    5/20/98 6:41p Lawrance
 * Add hook for command brief stage changes
 * 
 * 32    5/18/98 5:59p Hoffoss
 * Made command briefing advanced now once the speech stops and animation
 * has fully played once, whichever is longer.
 * 
 * 31    5/14/98 3:34p Hoffoss
 * Made command brief screen wait until animation finishes before
 * auto-advancing to the next state (in addition to the wait that was
 * already implemented).
 * 
 * 30    5/08/98 5:32p Lawrance
 * prompt for CD if can't load animations or voice
 * 
 * 29    5/06/98 11:49p Lawrance
 * Add help overlay for command brief
 * 
 * 28    5/05/98 2:44p Hoffoss
 * Fixed bug where not having a valid command brief ani would crash the
 * cmd brief screen.
 * 
 * 27    4/28/98 4:16p Hoffoss
 * Implemented auto-advancing functionality to command briefings.
 * 
 * 26    4/27/98 11:01a Hoffoss
 * Changed code to utilize proper palette.
 * 
 * 25    4/26/98 5:11p Hoffoss
 * Fixed bug where going to options screen and then returning to the cmd
 * brief screen didn't start ani back up.
 * 
 * 24    4/13/98 11:00a Hoffoss
 * Made the ani in a cmd brief continue playing if it's the same as the
 * new stage.
 * 
 * 23    4/06/98 8:37p Hoffoss
 * Fixed a few bugs with command brief screen.  Now the voice starts after
 * the text has printed, and options screen doesn't reset cmd brief.
 * 
 * 22    4/06/98 11:24a Lawrance
 * stop command brief music if returning to main menu
 * 
 * 21    4/02/98 11:40a Lawrance
 * check for #ifdef DEMO instead of #ifdef DEMO_RELEASE
 * 
 * 20    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 19    3/31/98 12:00p Hoffoss
 * Fixed bug with setting palette using <default> as filename.
 * 
 * 18    3/30/98 3:22p Hoffoss
 * Changed Command Brief screen to merge current ani's palette with
 * interface palette for better overall color.
 * 
 * 17    3/30/98 12:18a Lawrance
 * change some DEMO_RELEASE code to not compile code rather than return
 * early
 * 
 * 16    3/29/98 12:55a Lawrance
 * Get demo build working with limited set of data.
 * 
 * 15    3/27/98 9:49a Lawrance
 * AL: Ensure anim stops playing when leaving the command brief
 * 
 * 14    3/26/98 5:24p Hoffoss
 * Changed Command Brief to use memory mapped ani files instead, so we
 * avoid the huge pauses for huge anis that play!
 * 
 * 13    3/24/98 8:52a Jasen
 * Updated coords for new button.
 * 
 * 12    3/23/98 4:21p Hoffoss
 * Fixed bug where command brief screen couldn't be re-entered unless
 * mission was reloaded.
 * 
 * 11    3/19/98 5:59p Hoffoss
 * Added reset to cmd brief shutdown.
 * 
 * 10    3/19/98 5:32p Lawrance
 * Added music to the background of command brief screen.
 * 
 * 9     3/19/98 4:25p Hoffoss
 * Added remaining support for command brief screen (ANI and WAVE file
 * playing).
 * 
 * 8     3/18/98 12:03p John
 * Marked all the new strings as externalized or not.
 * 
 * 7     3/17/98 6:24p Hoffoss
 * Added ani playing to command brief screen, which defaults to
 * CB_default.ani.
 * 
 * 6     3/13/98 6:12p Frank
 * AL: Fix bug caused by conflict with command brief and red alert
 * missions
 * 
 * 5     3/13/98 3:44p Hoffoss
 * Added stage indication to Mission Command Briefing screen.
 * 
 * 4     3/12/98 4:02p Hoffoss
 * Added commit sound
 * 
 * 3     3/05/98 9:38p Hoffoss
 * Finished up command brief screen.
 * 
 * 2     3/05/98 3:59p Hoffoss
 * Added a bunch of new command brief stuff, and asteroid initialization
 * to Fred.
 * 
 * 1     3/02/98 6:13p Hoffoss
 *
 * $NoKeywords: $
 */

#include "missionui/missioncmdbrief.h"
#include "missionui/missionscreencommon.h"
#include "ui/uidefs.h"
#include "gamesnd/gamesnd.h"
#include "gamesequence/gamesequence.h"
#include "io/key.h"
#include "graphics/font.h"
#include "mission/missionbriefcommon.h"
#include "missionui/redalert.h"
#include "sound/audiostr.h"
#include "io/timer.h"
#include "gamesnd/eventmusic.h"
#include "playerman/player.h"
#include "gamehelp/contexthelp.h"
#include "globalincs/alphacolors.h"
#include "anim/packunpack.h"
#include "anim/animplay.h"
#include "sound/fsspeech.h"



#define NUM_CMD_SETTINGS	2

char *Cmd_brief_fname[NUM_CMD_SETTINGS][GR_NUM_RESOLUTIONS] = {
	{
		"CommandBrief",
		"2_CommandBrief"
	},
	{
		"CommandBriefb",
		"2_CommandBriefb"
	}
};


char *Cmd_brief_mask[NUM_CMD_SETTINGS][GR_NUM_RESOLUTIONS] = {
	{
		"CommandBrief-m",
		"2_CommandBrief-m"
	},
	{
		"CommandBrief-mb",
		"2_CommandBrief-mb"
	},
};


// lookups for coordinates
#define CMD_X_COORD 0
#define CMD_Y_COORD 1
#define CMD_W_COORD 2
#define CMD_H_COORD 3

int Cmd_text_wnd_coords[NUM_CMD_SETTINGS][GR_NUM_RESOLUTIONS][4] = {
	// original
	{
		{
			17, 109, 606, 108			// GR_640
		},
		{
			28, 174, 969, 174			// GR_1024
		}
	},
	// buttons
	{
		{
			17, 109, 587, 108			// GR_640
		},
		{
			28, 174, 939, 174			// GR_1024
		}
	}
};


int Cmd_stage_y[GR_NUM_RESOLUTIONS] = {
	90,		// GR_640
	145		// GR_1024
};

int Cmd_image_wnd_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		26, 258, 441, 204				// GR_640
	},
	{
		155, 475, 706, 327		// GR_1024
	}
};

int Top_cmd_brief_text_line;
int Cmd_brief_text_max_lines[GR_NUM_RESOLUTIONS] = {
	10, 17
};

#define MAX_CMD_BRIEF_BUTTONS	10
#define MIN_CMD_BRIEF_BUTTONS	8
#define NUM_CMD_BRIEF_BUTTONS	(Uses_scroll_buttons ? MAX_CMD_BRIEF_BUTTONS : MIN_CMD_BRIEF_BUTTONS)

#define CMD_BRIEF_BUTTON_FIRST_STAGE	0
#define CMD_BRIEF_BUTTON_PREV_STAGE		1
#define CMD_BRIEF_BUTTON_PAUSE			2
#define CMD_BRIEF_BUTTON_NEXT_STAGE		3
#define CMD_BRIEF_BUTTON_LAST_STAGE		4
#define CMD_BRIEF_BUTTON_HELP			5
#define CMD_BRIEF_BUTTON_OPTIONS		6
#define CMD_BRIEF_BUTTON_ACCEPT			7
#define CMD_BRIEF_BUTTON_SCROLL_UP		8
#define CMD_BRIEF_BUTTON_SCROLL_DOWN	9

// buttons
ui_button_info Cmd_brief_buttons[GR_NUM_RESOLUTIONS][MAX_CMD_BRIEF_BUTTONS] = {
	{ // GR_640
		ui_button_info("CBB_00",	504,	221,	-1,	-1,	0),
		ui_button_info("CBB_01",	527,	221,	-1,	-1,	1),
		ui_button_info("CBB_02",	555,	221,	-1,	-1,	2),
		ui_button_info("CBB_03",	583,	221,	-1,	-1,	3),
		ui_button_info("CBB_04",	607,	221,	-1,	-1,	4),
		ui_button_info("CBB_05",	539,	431,	-1,	-1,	5),
		ui_button_info("CBB_06",	538,	455,	-1,	-1,	6),
		ui_button_info("CBB_07",	575,	432,	-1,	-1,	7),
		ui_button_info("CBB_08",	615,	144,	-1,	-1,	8),
		ui_button_info("CBB_09",	615,	186,	-1,	-1,	9),
	},
	{ // GR_1024
		ui_button_info("2_CBB_00",	806,	354,	-1,	-1,	0),
		ui_button_info("2_CBB_01",	844,	354,	-1,	-1,	1),
		ui_button_info("2_CBB_02",	888,	354,	-1,	-1,	2),
		ui_button_info("2_CBB_03",	933,	354,	-1,	-1,	3),
		ui_button_info("2_CBB_04",	971,	354,	-1,	-1,	4),
		ui_button_info("2_CBB_05",	863,	690,	-1,	-1,	5),
		ui_button_info("2_CBB_06",	861,	728,	-1,	-1,	6),
		ui_button_info("2_CBB_07",	920,	692,	-1,	-1,	7),
		ui_button_info("2_CBB_08",	985,	232,	-1,	-1,	8),
		ui_button_info("2_CBB_09",	985,	299,	-1,	-1,	9),
	}
};

// text
#define CMD_BRIEF_NUM_TEXT		3
UI_XSTR Cmd_brief_text[GR_NUM_RESOLUTIONS][CMD_BRIEF_NUM_TEXT] = {
	{ // GR_640
		{ "Help",		928,	500,	440,	UI_XSTR_COLOR_GREEN,	-1,	&Cmd_brief_buttons[0][CMD_BRIEF_BUTTON_HELP].button },
		{ "Options",	1036,	479,	464,	UI_XSTR_COLOR_GREEN,	-1,	&Cmd_brief_buttons[0][CMD_BRIEF_BUTTON_OPTIONS].button },
		{ "Continue",	1069,	564,	413,	UI_XSTR_COLOR_PINK,	-1,	&Cmd_brief_buttons[0][CMD_BRIEF_BUTTON_ACCEPT].button },
	},
	{ // GR_1024
		{ "Help",		928,	800,	704,	UI_XSTR_COLOR_GREEN,	-1,	&Cmd_brief_buttons[1][CMD_BRIEF_BUTTON_HELP].button },
		{ "Options",	1036,	797,	743,	UI_XSTR_COLOR_GREEN,	-1,	&Cmd_brief_buttons[1][CMD_BRIEF_BUTTON_OPTIONS].button },
		{ "Continue",	1069,	917,	661,	UI_XSTR_COLOR_PINK,	-1,	&Cmd_brief_buttons[1][CMD_BRIEF_BUTTON_ACCEPT].button },
	}
};

static UI_WINDOW Ui_window;
static int Cmd_brief_background_bitmap;					// bitmap for the background of the cmd_briefing
static int Cur_stage;
static int Cmd_brief_inited = 0;
// static int Cmd_brief_ask_for_cd;
static int Voice_good_to_go = 0;
static int Voice_started_time = 0;
static int Voice_ended_time;
static int Anim_playing_id = -1;
static anim_instance *Cur_anim_instance = NULL;
static int Last_anim_frame_num;

static int Cmd_brief_last_voice;
static int Cmd_brief_paused = 0;
//static int Palette_bmp = -1;
static ubyte Palette[768];
//static char Palette_name[128];

static int Uses_scroll_buttons = 0;

void cmd_brief_init_voice()
{
	int i;

	Assert(Cur_cmd_brief);
	for (i=0; i<Cur_cmd_brief->num_stages; i++) {
		Cur_cmd_brief->stage[i].wave = -1;
		if (stricmp(Cur_cmd_brief->stage[i].wave_filename, NOX("none")) && Cur_cmd_brief->stage[i].wave_filename[0]) {
			Cur_cmd_brief->stage[i].wave = audiostream_open(Cur_cmd_brief->stage[i].wave_filename, ASF_VOICE);
			if (Cur_cmd_brief->stage[i].wave < 0) {
				nprintf(("General", "Failed to load \"%s\"", Cur_cmd_brief->stage[i].wave_filename));
			}
		}
	}

	Cmd_brief_last_voice = -1;
}

int cmd_brief_check_stage_done()
{
	if (!Voice_good_to_go)
		return 0;

	if (Cmd_brief_paused)
		return 0;

	if (Voice_ended_time && (timer_get_milliseconds() - Voice_ended_time >= 1000))
		return 1;

	if (Briefing_voice_enabled && (Cmd_brief_last_voice >= 0)) {
		if (audiostream_is_playing(Cmd_brief_last_voice)){
			return 0;
		}

		if (!Voice_ended_time){
			Voice_ended_time = timer_get_milliseconds();
		}

		return 0;
	}

	// if we get here, there is no voice, so we simulate the time it would take instead
	if (!Voice_ended_time)
		Voice_ended_time = Voice_started_time + MAX(5000, Num_brief_text_lines[0] * 3500);

	return 0;
}

// start playback of the voice for a particular briefing stage
void cmd_brief_voice_play(int stage_num)
{
	int voice = -1;

	if (!Voice_good_to_go) {
		Voice_started_time = 0;
		return;
	}

	if (!Voice_started_time) {
		Voice_started_time = timer_get_milliseconds();
		Voice_ended_time = 0;
	}

	if (!Briefing_voice_enabled){
		return;
	}

	if (Cur_stage >= 0 && Cur_stage < Cur_cmd_brief->num_stages){
		voice = Cur_cmd_brief->stage[stage_num].wave;
	}

	// are we still on same voice that is currently playing/played?
	if (Cmd_brief_last_voice == voice){
		return;  // no changes, nothing to do.
	}

	// if previous wave is still playing, stop it first.
	if (Cmd_brief_last_voice >= 0) {
		audiostream_stop(Cmd_brief_last_voice, 1, 0);  // stream is automatically rewound
		Cmd_brief_last_voice = -1;
	}

	// ok, new wave needs playing, so we can start playing it now (and it becomes the current wave)
	Cmd_brief_last_voice = voice;
	if (voice >= 0){
		audiostream_play(voice, Master_voice_volume, 0);
	}
}

// called to leave the command briefing screen
void cmd_brief_exit()
{
	gameseq_post_event(GS_EVENT_START_BRIEFING);
}

void cmd_brief_stop_anim(int id)
{
	if (Cur_anim_instance && (id != Anim_playing_id)) {
		anim_stop_playing(Cur_anim_instance);
		Cur_anim_instance = NULL;
	}

	Voice_good_to_go = 0;
	if (Cmd_brief_last_voice >= 0) {
		audiostream_stop(Cmd_brief_last_voice, 1, 0);  // stream is automatically rewound
		Cmd_brief_last_voice = -1;
	}
}

void cmd_brief_new_stage(int stage)
{
	int i;
	anim_play_struct aps;

	if (stage < 0) {
		cmd_brief_stop_anim(-1);
		Cur_stage = -1;
		Anim_playing_id = -1;
	}

	// If the briefing has no wave to play use simulated speach
	if(Cur_cmd_brief->stage[stage].wave <= 0) {
		fsspeech_play(FSSPEECH_FROM_BRIEFING, Cur_cmd_brief->stage[stage].text);
	}

	Cur_stage = stage;
	brief_color_text_init(Cur_cmd_brief->stage[stage].text, Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_W_COORD]);

	i = Cur_cmd_brief->stage[Cur_stage].anim_ref;
	if (i < 0)
		i = Cur_stage;

	cmd_brief_stop_anim(i);

	if (i != Anim_playing_id) {
		if (Cur_cmd_brief->stage[i].cmd_anim) {
			anim_play_init(&aps, Cur_cmd_brief->stage[i].cmd_anim,Cmd_image_wnd_coords[gr_screen.res][CMD_X_COORD], Cmd_image_wnd_coords[gr_screen.res][CMD_Y_COORD]);
			aps.looped = 1;
			Cur_anim_instance = anim_play(&aps);
			Last_anim_frame_num = 0;
		}

		Anim_playing_id = i;
	}

	if (Cur_cmd_brief->stage[i].cmd_anim) {
		memcpy(Palette, Cur_cmd_brief->stage[i].cmd_anim->palette, 384);
		gr_set_palette(Cur_cmd_brief->stage[i].ani_filename, Palette, 1);
	}

	Top_cmd_brief_text_line = 0;
}

void cmd_brief_hold()
{
	cmd_brief_stop_anim(-1);
	Anim_playing_id = -1;
}

void cmd_brief_unhold()
{
	cmd_brief_new_stage(Cur_stage);
}

extern int Briefing_music_handle;

void cmd_brief_pause()
{
	if (Cmd_brief_paused)
		return;

	Cmd_brief_paused = 1;

	if (Cur_anim_instance != NULL) {
		anim_pause(Cur_anim_instance);
	}

	if (Cmd_brief_last_voice >= 0) {
		audiostream_pause(Cmd_brief_last_voice);
	}

	if (Briefing_music_handle >= 0) {
		audiostream_pause(Briefing_music_handle);
	}

	fsspeech_pause(true);
}

void cmd_brief_unpause()
{
	if (!Cmd_brief_paused)
		return;

	Cmd_brief_paused = 0;

	if (Cur_anim_instance != NULL) {
		anim_unpause(Cur_anim_instance);
	}

	if (Cmd_brief_last_voice >= 0) {
		audiostream_unpause(Cmd_brief_last_voice);
	}

	if (Briefing_music_handle >= 0) {
		audiostream_unpause(Briefing_music_handle);
	}

	fsspeech_pause(false);
}

void cmd_brief_button_pressed(int n)
{
	switch (n) {
		case CMD_BRIEF_BUTTON_HELP:
			launch_context_help();
			gamesnd_play_iface(SND_HELP_PRESSED);
			break;

		case CMD_BRIEF_BUTTON_OPTIONS:
			gamesnd_play_iface(SND_SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_OPTIONS_MENU);
			break;

		case CMD_BRIEF_BUTTON_FIRST_STAGE:
			if (Cur_stage) {
				cmd_brief_new_stage(0);
				gamesnd_play_iface(SND_BRIEF_STAGE_CHG);
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}

			break;

		case CMD_BRIEF_BUTTON_PREV_STAGE:
			if (Cur_stage) {
				cmd_brief_new_stage(Cur_stage - 1);
				gamesnd_play_iface(SND_BRIEF_STAGE_CHG);
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}

			break;

		case CMD_BRIEF_BUTTON_NEXT_STAGE:
			if (Cur_stage < Cur_cmd_brief->num_stages - 1) {
				cmd_brief_new_stage(Cur_stage + 1);
				gamesnd_play_iface(SND_BRIEF_STAGE_CHG);
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}

			break;

		case CMD_BRIEF_BUTTON_LAST_STAGE:
			if (Cur_stage < Cur_cmd_brief->num_stages - 1) {
				cmd_brief_new_stage(Cur_cmd_brief->num_stages - 1);
				gamesnd_play_iface(SND_BRIEF_STAGE_CHG);
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
			break;

		case CMD_BRIEF_BUTTON_ACCEPT:
			cmd_brief_exit();
			gamesnd_play_iface(SND_COMMIT_PRESSED);
			break;

		case CMD_BRIEF_BUTTON_PAUSE:
			gamesnd_play_iface(SND_USER_SELECT);
			fsspeech_pause(Player->auto_advance != 0);
			Player->auto_advance ^= 1;
			break;

		case CMD_BRIEF_BUTTON_SCROLL_UP:
			Top_cmd_brief_text_line--;
			if ( Top_cmd_brief_text_line < 0 ) {
				Top_cmd_brief_text_line = 0;
				gamesnd_play_iface(SND_GENERAL_FAIL);
			} else {
				gamesnd_play_iface(SND_SCROLL);
			}
			break;

		case CMD_BRIEF_BUTTON_SCROLL_DOWN:
			Top_cmd_brief_text_line++;
			if ( (Num_brief_text_lines[0] - Top_cmd_brief_text_line) < Cmd_brief_text_max_lines[gr_screen.res]) {
				Top_cmd_brief_text_line--;
				gamesnd_play_iface(SND_GENERAL_FAIL);
			} else {
				gamesnd_play_iface(SND_SCROLL);
			}
			break;
	}
}

void cmd_brief_ani_wave_init(int index)
{
	char *name;
	int i;

	// first, search and see if anim is already used in another stage
	for (i=0; i<index; i++) {
		if (!stricmp(Cur_cmd_brief->stage[i].ani_filename, Cur_cmd_brief->stage[index].ani_filename)) {
			if (Cur_cmd_brief->stage[i].anim_ref >= 0)
				Cur_cmd_brief->stage[index].anim_ref = Cur_cmd_brief->stage[i].anim_ref;
			else
				Cur_cmd_brief->stage[index].anim_ref = i;

			return;
		}
	}

	// this is the first instance of the given anim filename
	Cur_cmd_brief->stage[index].anim_ref = -1;
	name = Cur_cmd_brief->stage[index].ani_filename;
	if (!name[0] || !stricmp(name, NOX("<default>")) || !stricmp(name, NOX("none.ani"))) {
		name = NOX("CB_default");
		strcpy(Cur_cmd_brief->stage[index].ani_filename, name);
	}

	int load_attempts = 0;
	while (1) {

		if ( load_attempts++ > 5 ) {
			break;
		}

		Cur_cmd_brief->stage[index].cmd_anim = anim_load(name, 1);
		if ( Cur_cmd_brief->stage[index].cmd_anim ) {
			break;
		}

		// couldn't load animation, ask user to insert CD (if necessary)
		// if ( Cmd_brief_ask_for_cd ) {
			// if ( game_do_cd_check() == 0 ) {
				// Cmd_brief_ask_for_cd = 0;
				// break;
			// }
		// }
	}

	// check to see if cb anim loaded, if not, try the default one
	if ( !Cur_cmd_brief->stage[index].cmd_anim ) {
		Cur_cmd_brief->stage[index].cmd_anim = anim_load(NOX("CB_default"), 1);
	}
}

void cmd_brief_init(int team)
{
	common_music_init(SCORE_BRIEFING);

//#ifndef FS2_DEMO

	int i;
	ui_button_info *b;

	Cmd_brief_inited = 0;
	Cur_cmd_brief = &Cmd_briefs[team];

	if ( red_alert_mission() ) {
		gameseq_post_event(GS_EVENT_RED_ALERT);
		return;
	}

	if (Cur_cmd_brief->num_stages <= 0)
		return;

	gr_reset_clip();
	gr_clear();
	Mouse_hidden++;
	gr_flip();
	Mouse_hidden--;

	/*
	Palette_bmp = bm_load("BarracksPalette");	//CommandBriefPalette");
	Assert(Palette_bmp);
	bm_get_palette(Palette_bmp, Palette, Palette_name);  // get the palette for this bitmap
	gr_set_palette(Palette_name, Palette, 1);
	*/

	// first determine which layout to use
	Uses_scroll_buttons = 1;	// assume true
	Cmd_brief_background_bitmap = bm_load(Cmd_brief_fname[Uses_scroll_buttons][gr_screen.res]);	// try to load extra one first
	if (Cmd_brief_background_bitmap < 0)	// failed to load
	{
		Uses_scroll_buttons = 0;	// nope, sorry
		Cmd_brief_background_bitmap = bm_load(Cmd_brief_fname[Uses_scroll_buttons][gr_screen.res]);
	}

	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Ui_window.set_mask_bmap(Cmd_brief_mask[Uses_scroll_buttons][gr_screen.res]);

	// Cmd_brief_ask_for_cd = 1;

	for (i=0; i<NUM_CMD_BRIEF_BUTTONS; i++) {
		b = &Cmd_brief_buttons[gr_screen.res][i];

		b->button.create(&Ui_window, "", b->x, b->y, 60, 30, 0, 1);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// add text
	for(i=0; i<CMD_BRIEF_NUM_TEXT; i++){
		Ui_window.add_XSTR(&Cmd_brief_text[gr_screen.res][i]);
	}

	// set up readyrooms for buttons so we draw the correct animation frame when a key is pressed
	Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_FIRST_STAGE].button.set_hotkey(KEY_SHIFTED | KEY_LEFT);
	Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_LAST_STAGE].button.set_hotkey(KEY_SHIFTED | KEY_RIGHT);
	Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_PREV_STAGE].button.set_hotkey(KEY_LEFT);
	Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_NEXT_STAGE].button.set_hotkey(KEY_RIGHT);
	Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_ACCEPT].button.set_hotkey(KEY_CTRLED | KEY_ENTER);
	Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_HELP].button.set_hotkey(KEY_F1);
	Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_OPTIONS].button.set_hotkey(KEY_F2);

	// extra - Goober5000
	if (Uses_scroll_buttons)
	{
		Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_SCROLL_UP].button.set_hotkey(KEY_UP);
		Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_SCROLL_DOWN].button.set_hotkey(KEY_DOWN);
	}

	// load in help overlay bitmap	
	help_overlay_load(CMD_BRIEF_OVERLAY);
	help_overlay_set_state(CMD_BRIEF_OVERLAY,0);

	for (i=0; i<Cur_cmd_brief->num_stages; i++)
		cmd_brief_ani_wave_init(i);

	cmd_brief_init_voice();
	Cur_anim_instance = NULL;
	cmd_brief_new_stage(0);
	Cmd_brief_paused = 0;
	Cmd_brief_inited = 1;

//#endif
}

void cmd_brief_close()
{
	int i;

	if (Cmd_brief_inited) {
		cmd_brief_stop_anim(-1);
		Anim_playing_id = -1;
		for (i=0; i<Cur_cmd_brief->num_stages; i++) {
			if (Cur_cmd_brief->stage[i].wave >= 0)
				audiostream_close_file(Cur_cmd_brief->stage[i].wave, 0);

			if (Cur_cmd_brief->stage[i].anim_ref < 0)
				if (Cur_cmd_brief->stage[i].cmd_anim)
					anim_free(Cur_cmd_brief->stage[i].cmd_anim);
		}

		if (Cmd_brief_background_bitmap >= 0)
			bm_release(Cmd_brief_background_bitmap);

		// unload the overlay bitmap
		help_overlay_unload(CMD_BRIEF_OVERLAY);

		Ui_window.destroy();
		/*
		if (Palette_bmp){
			bm_unload(Palette_bmp);
		}
		*/

		game_flush();
		Cmd_brief_inited = 0;
	}

	// Stop any speech from running over
	fsspeech_stop();
}

void cmd_brief_do_frame(float frametime)
{
	char buf[40];
	int i, k, w, h;		

	// if no command briefing exists, skip this screen.
	if (!Cmd_brief_inited) {
		cmd_brief_exit();
		return;
	}

	if ( help_overlay_active(CMD_BRIEF_OVERLAY) ) {
		Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_HELP].button.reset_status();
		Ui_window.set_ignore_gadgets(1);
	}

	k = Ui_window.process() & ~KEY_DEBUGGED;

	if ( (k > 0) || B1_JUST_RELEASED ) {
		if ( help_overlay_active(CMD_BRIEF_OVERLAY) ) {
			help_overlay_set_state(CMD_BRIEF_OVERLAY, 0);
			Ui_window.set_ignore_gadgets(0);
			k = 0;
		}
	}

	if ( !help_overlay_active(CMD_BRIEF_OVERLAY) ) {
		Ui_window.set_ignore_gadgets(0);
	}

	switch (k) {
	case KEY_ESC:
		common_music_close();
		gameseq_post_event(GS_EVENT_MAIN_MENU);
		break;
	}	// end switch

	for (i=0; i<NUM_CMD_BRIEF_BUTTONS; i++){
		if (Cmd_brief_buttons[gr_screen.res][i].button.pressed()){
			cmd_brief_button_pressed(i);
		}
	}

	cmd_brief_voice_play(Cur_stage);
	common_music_do();

	if (cmd_brief_check_stage_done() && Player->auto_advance && (Cur_stage < Cur_cmd_brief->num_stages - 1)){
//		if (!Cur_anim_instance || (Cur_anim_instance->frame_num < Last_anim_frame_num))
		if (!Cur_anim_instance || Cur_anim_instance->loop_count){
			cmd_brief_new_stage(Cur_stage + 1);
		}
	}

	if (Cur_anim_instance){
		Last_anim_frame_num = Cur_anim_instance->frame_num;
	}

	GR_MAYBE_CLEAR_RES(Cmd_brief_background_bitmap);
	if (Cmd_brief_background_bitmap >= 0) {
		gr_set_bitmap(Cmd_brief_background_bitmap);
		gr_bitmap(0, 0);
	} 

	{
		// JAS: This code is hacked to allow the animation to use all 256 colors
		extern int Palman_allow_any_color;
		Palman_allow_any_color = 1;
		anim_render_all(0, frametime);
		Palman_allow_any_color = 0;
	}
	Ui_window.draw();

	if (!Player->auto_advance){
		Cmd_brief_buttons[gr_screen.res][CMD_BRIEF_BUTTON_PAUSE].button.draw_forced(2);
	}

	gr_set_font(FONT1);
	gr_set_color_fast(&Color_text_heading);

	sprintf(buf, XSTR( "Stage %d of %d", 464), Cur_stage + 1, Cur_cmd_brief->num_stages);
	gr_get_string_size(&w, NULL, buf);
	gr_string(Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_X_COORD] + Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_W_COORD] - w, Cmd_stage_y[gr_screen.res], buf);

	if (brief_render_text(Top_cmd_brief_text_line, Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_X_COORD], Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_Y_COORD], Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_H_COORD], frametime, 0, 1)){
		Voice_good_to_go = 1;
	}

	// maybe output the "more" indicator
	if ( (Cmd_brief_text_max_lines[gr_screen.res] + Top_cmd_brief_text_line) < Num_brief_text_lines[0] ) {
		// can be scrolled down
		int more_txt_x = Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_X_COORD] + (Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_W_COORD]/2) - 10;
		int more_txt_y = Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_Y_COORD] + Cmd_text_wnd_coords[Uses_scroll_buttons][gr_screen.res][CMD_H_COORD] - 2;				// located below brief text, centered

		gr_get_string_size(&w, &h, XSTR("more", 1469), strlen(XSTR("more", 1469)));
		gr_set_color_fast(&Color_black);
		gr_rect(more_txt_x-2, more_txt_y, w+3, h);
		gr_set_color_fast(&Color_red);
		gr_string(more_txt_x, more_txt_y, XSTR("more", 1469));  // base location on the input x and y?
	}

	// blit help overlay if active
	help_overlay_maybe_blit(CMD_BRIEF_OVERLAY);

	gr_flip();
}
