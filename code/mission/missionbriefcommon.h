/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionBriefCommon.h $
 * $Revision: 2.5 $
 * $Date: 2005-02-21 09:00:17 $
 * $Author: wmcoolmon $
 *
 * Header file for briefing stuff common to FreeSpace and FRED
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2004/08/11 05:06:27  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.3  2004/07/17 18:46:08  taylor
 * various OGL and memory leak fixes
 *
 * Revision 2.2  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 10    8/24/99 11:30a Jefff
 * Increased number of debriefing lines
 * 
 * 9     7/20/99 7:09p Jefff
 * briefing text occupies full window in 1024x768
 * 
 * 8     7/19/99 3:01p Dave
 * Fixed icons. Added single transport icon.
 * 
 * 7     7/18/99 5:20p Dave
 * Jump node icon. Fixed debris fogging. Framerate warning stuff.
 * 
 * 6     7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 5     6/29/99 7:39p Dave
 * Lots of small bug fixes.
 * 
 * 4     1/30/99 4:03p Johnson
 * Include #defines for Fred
 * 
 * 3     1/29/99 4:17p Dave
 * New interface screens.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 71    6/10/98 6:47p Lawrance
 * increase MAX_RECOMMENDATION_LEN to 1024, to accommodate other languages
 * 
 * 70    4/25/98 3:49p Lawrance
 * Save briefing auto-advance pref
 * 
 * 69    4/20/98 3:53p Lawrance
 * Fix various bugs with auto-advancing through briefings.
 * 
 * 68    4/15/98 10:27a Sandeep
 * 
 * 67    4/13/98 7:06p Lawrance
 * implement auto advance of briefing stages
 * 
 * 66    4/06/98 2:51p Hoffoss
 * Added sexp for briefing to FreeSpace.
 * 
 * 65    4/03/98 5:22p Hoffoss
 * Changes: training missions don't show objectives anymore, text for
 * objective stage is different than last briefing stage now, and
 * objectives have a black background now.
 * 
 * 64    4/03/98 10:31a John
 * Made briefing and debriefing arrays be malloc'd
 * 
 * 63    4/01/98 8:38p Lawrance
 * Add support for jump node icons in the briefings.
 * 
 * 62    3/26/98 5:47p Lawrance
 * remove icon text functionality
 * 
 * 61    3/17/98 4:13p Hoffoss
 * finessed the coordinates of the text window in the briefing screen a
 * little to improve visual quality.
 * 
 * 60    3/12/98 4:04p Hoffoss
 * Changed text window size and spaced text out an extra pixel.
 * 
 * 59    3/09/98 12:13a Lawrance
 * Add support for Red Alert missions
 * 
 * 58    3/07/98 11:50a Lawrance
 * Fix bug with displaying the closeup title
 * 
 * 57    3/05/98 9:38p Hoffoss
 * Finished up command brief screen.
 * 
 * 56    3/05/98 10:39a Hoffoss
 * Added command brief stuff.
 * 
 * 55    2/26/98 4:59p Allender
 * groundwork for team vs team briefings.  Moved weaponry pool into the
 * Team_data structure.  Added team field into the p_info structure.
 * Allow for mutliple structures in the briefing code.
 * 
 * 54    2/24/98 6:21p Lawrance
 * Fix up map grid region
 * 
 * 53    2/24/98 12:22a Lawrance
 * New coords for revamped briefing graphics
 * 
 * 52    2/18/98 6:45p Hoffoss
 * Added support for lines between icons in briefings for Fred.
 * 
 * 51    2/13/98 5:16p Lawrance
 * Add support for icon lines in the briefing.
 * 
 * 50    2/09/98 9:25p Allender
 * team v team support.  multiple pools and breifings
 * 
 * 49    2/06/98 4:32p Lawrance
 * Allow briefing voices to be toggled on/off
 * 
 * 48    2/04/98 4:32p Allender
 * support for multiple briefings and debriefings.  Changes to mission
 * type (now a bitfield).  Bitfield defs for multiplayer modes
 * 
 * 47    1/28/98 7:22p Lawrance
 * Put back in highlight member in brief_icon for FRED compatability.
 * 
 * 46    1/28/98 7:19p Lawrance
 * Get fading/highlighting animations working
 * 
 * 45    10/19/97 5:14p Lawrance
 * add flags member to brief_stage
 * 
 * 44    10/15/97 4:45p Lawrance
 * implement scene cut in debriefing
 * 
 * 43    10/13/97 7:40p Lawrance
 * implement new debriefing/recommendation format
 * 
 * 42    10/04/97 12:00a Lawrance
 * grey out background when help overlay is activated
 * 
 * 41    10/03/97 8:25a Lawrance
 * be able to pause brief narration
 * 
 * 40    10/01/97 4:39p Lawrance
 * move chat code into Chatbox.cpp, simplify interface
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef __MISSIONBRIEFCOMMON_H__
#define __MISSIONBRIEFCOMMON_H__

#include "globalincs/globals.h"
#include "anim/packunpack.h"
#include "hud/hud.h"

#define MAX_TEXT_STREAMS	2		// how many concurrent streams of text can be displayed

// ------------------------------------------------------------------------
// names for the icons that can appear in the briefing.  If you modify this list,
// update the Icons_names[] string array located in MissionParse.cpp
// ------------------------------------------------------------------------
#define MAX_BRIEF_ICONS						35		// keep up to date

#define ICON_FIGHTER							0
#define ICON_FIGHTER_WING					1
#define ICON_CARGO							2
#define ICON_CARGO_WING						3
#define ICON_LARGESHIP						4
#define ICON_LARGESHIP_WING				5
#define ICON_CAPITAL							6
#define ICON_PLANET							7
#define ICON_ASTEROID_FIELD				8
#define ICON_WAYPOINT						9
#define ICON_SUPPORT_SHIP					10
#define ICON_FREIGHTER_NO_CARGO			11
#define ICON_FREIGHTER_WITH_CARGO		12
#define ICON_FREIGHTER_WING_NO_CARGO	13
#define ICON_FREIGHTER_WING_WITH_CARGO	14
#define ICON_INSTALLATION					15
#define ICON_BOMBER							16
#define ICON_BOMBER_WING					17
#define ICON_CRUISER							18
#define ICON_CRUISER_WING					19
#define ICON_UNKNOWN							20
#define ICON_UNKNOWN_WING					21
#define ICON_FIGHTER_PLAYER				22
#define ICON_FIGHTERW_PLAYER				23
#define ICON_BOMBER_PLAYER					24
#define ICON_BOMBERW_PLAYER				25
#define ICON_KNOSSOS_DEVICE				26
#define ICON_TRANSPORT_WING				27
#define ICON_CORVETTE						28
#define ICON_GAS_MINER						29
#define ICON_AWACS							30
#define ICON_SUPERCAP						31
#define ICON_SENTRYGUN						32
#define ICON_JUMP_NODE						33
#define ICON_TRANSPORT						34

// ------------------------------------------------------------------------
// Structures to hold briefing data
// ------------------------------------------------------------------------

#define	MAX_BRIEF_LEN			4096		// size of char array which holds briefing text
#define	MAX_BRIEF_LINES		50
#define	MAX_BRIEF_LINE_LEN	256		// max number of chars in a briefing line
#define	MAX_BRIEF_LINE_W_640		375		// max width of line in pixels in 640x480 mode
#define	MAX_BRIEF_LINE_W_1024	600		// max width of line in pixels in 1024x768 mode

#define	MAX_DEBRIEF_LEN		2048		// size of char array which holds debriefing text
#define	MAX_DEBRIEF_LINES		60
#define	MAX_DEBRIEF_LINE_LEN	256		// max number of chars in a debriefing line
#define	MAX_DEBRIEF_LINE_W	500		// max width of line in pixels

#define	MAX_ICON_TEXT_LEN			1024		// max number of chars for icon info
#define	MAX_ICON_TEXT_LINES		30
#define	MAX_ICON_TEXT_LINE_LEN	256		// max number of chars in icon info line
#define	MAX_ICON_TEXT_LINE_W		170		// max width of line in pixels

#define	MAX_STAGE_ICONS			20
#define	MAX_BRIEF_STAGES			15
#define	MAX_DEBRIEF_STAGES		20
#define	MAX_LABEL_LEN				64

#define	MAX_RECOMMENDATION_LEN	1024

#define		BI_HIGHLIGHT		(1<<0)
#define		BI_SHOWHIGHLIGHT	(1<<1)
#define		BI_FADEIN			(1<<2)

typedef struct brief_icon
{
	int		x,y,w,h;
	int		hold_x, hold_y;	// 2D screen position of icon, used to place animations
	int		ship_class;
	int		modelnum;
	float		radius;
	int		type;					// ICON_* defines from MissionBriefCommon.h
	int		bitmap_id;
	int		id;
	int		team;
	vector	pos;
	char		label[MAX_LABEL_LEN];
	char		closeup_label[MAX_LABEL_LEN];
//	char		text[MAX_ICON_TEXT_LEN];
	hud_anim	fadein_anim;
	hud_anim	fadeout_anim;
	hud_anim	highlight_anim;
	int		flags;				// BI_* flags defined above
} brief_icon;

#define MAX_BRIEF_STAGE_LINES		20

typedef struct brief_line
{
	int start_icon;		// index into icons[], where line starts
	int end_icon;			// index into icons[], where line ends
} brief_line;

#define BS_FORWARD_CUT		(1<<0)
#define BS_BACKWARD_CUT		(1<<1)

typedef struct brief_stage
{
	char			*new_text;
	char			voice[MAX_FILENAME_LEN];
	vector		camera_pos;
	matrix		camera_orient;
	int			camera_time;		// ms
	int			flags;				// see BS_ flags above
	int			formula;
	int			num_icons;
	brief_icon	*icons;
	int			num_lines;
	brief_line	*lines;
} brief_stage;

typedef struct debrief_stage
{
	int			formula;
	char			*new_text;
	char			voice[MAX_FILENAME_LEN];
	char			*new_recommendation_text;
} debrief_stage;

typedef struct briefing {
	int			num_stages;
	brief_stage	stages[MAX_BRIEF_STAGES];
} briefing;

typedef struct debriefing {
	int				num_stages;
	debrief_stage	stages[MAX_DEBRIEF_STAGES];
} debriefing;


// Code to free/init the above structures between levels:

// --------------------------------------------------------------------------------------
// Does one time initialization of the briefing and debriefing structures.
// Namely setting all malloc'ble pointers to NULL.  Called once at game startup.
void mission_brief_common_init();

//--------------------------------------------------------------------------------------
// Frees all the memory allocated in the briefing and debriefing structures
// and sets all pointers to NULL.
void mission_brief_common_reset();
void mission_debrief_common_reset();


// --------------------------------------------------------------------------------------
// briefing screen
// --------------------------------------------------------------------------------------
extern int Brief_bmap_coords[GR_NUM_RESOLUTIONS][2];
extern int Brief_grid_coords[GR_NUM_RESOLUTIONS][4];
extern int Brief_text_coords[GR_NUM_RESOLUTIONS][4];
extern int Brief_text_max_lines[GR_NUM_RESOLUTIONS];
extern char *Brief_static_name[GR_NUM_RESOLUTIONS];
extern int Brief_static_coords[GR_NUM_RESOLUTIONS][2];

// Needed for Fred
#define BRIEF_GRID3_X1						42
#define BRIEF_GRID3_Y1						122
#define BRIEF_GRID0_X2						585
#define BRIEF_GRID0_Y2						371
#define BRIEF_GRID_W							(BRIEF_GRID0_X2-BRIEF_GRID3_X1+1)
#define BRIEF_GRID_H							(BRIEF_GRID0_Y2-BRIEF_GRID3_Y1+1)
/*
#define BRIEF_GRID0_X1						63
#define BRIEF_GRID0_Y1						122
#define BRIEF_GRID1_X1						575
#define BRIEF_GRID1_Y1						122
#define BRIEF_GRID2_X1						63
#define BRIEF_GRID2_Y1						350

#define BRIEF_TEXT_X1						0
#define BRIEF_TEXT_Y1						397
#define BRIEF_TEXT_X2						441
#define BRIEF_TEXT_Y2						477
#define BRIEF_TEXT_BEGIN_X					50
#define BRIEF_TEXT_BEGIN_Y					414
#define BRIEF_TEXT_H							54

*/

typedef struct brief_screen
{
	int map_x1, map_x2, map_y1, map_y2;
/*	int btext_x1, btext_x2, btext_y1, btext_y2;
	int cup_x1, cup_x2, cup_y1, cup_y2;
	int cupinfo_x1, cupinfo_x2, cupinfo_y1, cupinfo_y2;*/
} brief_sceen;

extern brief_screen bscreen;

// ------------------------------------------------------------------------
// Global briefing/debriefing data
// ------------------------------------------------------------------------
extern briefing		Briefings[MAX_TEAMS];
extern debriefing		Debriefings[MAX_TEAMS];
extern briefing		*Briefing;
extern debriefing		*Debriefing;
extern float			Brief_text_wipe_time_elapsed;

extern int Cur_brief_id;
extern int Briefing_voice_enabled;

extern int Num_brief_text_lines[MAX_TEXT_STREAMS];
extern int Top_brief_text_line;
extern int Current_screen;

// ------------------------------------------------------------------------
// External interface
// ------------------------------------------------------------------------
void brief_reset();
void debrief_reset(); 
void brief_close_map();
void brief_init_map();
void brief_init_screen(int multiplayer_flag);
void brief_render_map(int stage_num, float frametime);
void brief_set_new_stage(vector *pos, matrix *orient, int time, int stage_num);
void brief_camera_move(float frametime, int stage_num);
void brief_render_icon(int stage_num, int icon_num, float frametime, int selected = 0, float w_scale_factor = 1.0f, float h_scale_factor = 1.0f);
void brief_render_icon_line(int stage_num, int line_num);
void brief_init_icons();
void brief_load_icons();
void brief_unload_icons();
void brief_common_close();
void brief_reset_icons(int stage_num);
void brief_restart_text_wipe();
void brief_reset_last_new_stage();
void brief_blit_stage_num(int stage_num, int stage_max);

void brief_common_get_icon_dimensions(int *w, int *h, int type, int ship_class);

// voice streaming interface
void brief_voice_init();
void brief_voice_load_all();
void brief_voice_unload_all();
void brief_voice_play(int stage_num);
void brief_voice_stop(int stage_num);
void brief_voice_pause(int stage_num);
void brief_voice_unpause(int stage_num);

// fancy briefing style text functions for use in other modules.
int brief_color_text_init(char *src, int w, int instance = 0);
int brief_render_text(int line_offset, int x, int y, int h, float frametime, int instance = 0, int line_spacing = 0);

void cmd_brief_reset();

int brief_time_to_advance(int stage_num, float frametime);

#endif
