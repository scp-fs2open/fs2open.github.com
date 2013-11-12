/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __MISSIONBRIEFCOMMON_H__
#define __MISSIONBRIEFCOMMON_H__

#include "globalincs/globals.h"
#include "anim/packunpack.h"
#include "hud/hud.h"
#include "graphics/generic.h"

#define MAX_TEXT_STREAMS	2		// how many concurrent streams of text can be displayed

// ------------------------------------------------------------------------
// names for the icons that can appear in the briefing.  If you modify this list,
// update the Icons_names[] string array located in MissionParse.cpp
// ------------------------------------------------------------------------
#define MIN_BRIEF_ICONS						35		// keep up to date

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

typedef struct briefing_icon_info {
	generic_anim	regular;
	hud_anim		fade;
	hud_anim		highlight;
} briefing_icon_type;

extern SCP_vector<briefing_icon_info> Briefing_icon_info;

struct brief_icon;
extern briefing_icon_info *brief_get_icon_info(brief_icon *bi);

// ------------------------------------------------------------------------
// Structures to hold briefing data
// ------------------------------------------------------------------------

#define	MAX_BRIEF_LINES		70
#define	MAX_BRIEF_LINE_LEN	256		// max number of chars in a briefing line
#define	MAX_BRIEF_LINE_W_640		375		// max width of line in pixels in 640x480 mode
#define	MAX_BRIEF_LINE_W_1024	600		// max width of line in pixels in 1024x768 mode

#define	MAX_DEBRIEF_LINES		60
#define	MAX_DEBRIEF_LINE_LEN	256		// max number of chars in a debriefing line
#define	MAX_DEBRIEF_LINE_W	500		// max width of line in pixels

#define	MAX_ICON_TEXT_LEN			1024		// max number of chars for icon info
#define	MAX_ICON_TEXT_LINES		30
#define	MAX_ICON_TEXT_LINE_LEN	256		// max number of chars in icon info line
#define	MAX_ICON_TEXT_LINE_W		170		// max width of line in pixels

#define	MAX_STAGE_ICONS			20
#define	MAX_BRIEF_STAGES			15
#define	MAX_DEBRIEF_STAGES		40
#define	MAX_LABEL_LEN				64

#define		BI_HIGHLIGHT		(1<<0)
#define		BI_SHOWHIGHLIGHT	(1<<1)
#define		BI_FADEIN			(1<<2)
#define		BI_MIRROR_ICON		(1<<3)	// mirror the briefing icon so it points the other way - phreak
#define		BI_USE_WING_ICON	(1<<4)	// use wing variant of briefing icon

typedef struct brief_icon {
	int		x,y,w,h;
	int		hold_x, hold_y;	// 2D screen position of icon, used to place animations
	int		ship_class;
	int		modelnum;
	float		radius;
	int		type;					// ICON_* defines from MissionBriefCommon.h
	int		bitmap_id;
	int		id;
	int		team;
	vec3d	pos;
	char		label[MAX_LABEL_LEN];
	char		closeup_label[MAX_LABEL_LEN];
//	char		text[MAX_ICON_TEXT_LEN];
	hud_anim	fadein_anim;
	hud_anim	fadeout_anim;
	hud_anim	highlight_anim;
	int		flags;				// BI_* flags defined above
} brief_icon;

#define MAX_BRIEF_STAGE_LINES		20

typedef struct brief_line {
	int start_icon;		// index into icons[], where line starts
	int end_icon;		// index into icons[], where line ends
} brief_line;

#define BS_FORWARD_CUT		(1<<0)
#define BS_BACKWARD_CUT		(1<<1)

class brief_stage
{
public:
	SCP_string	text;
	char			voice[MAX_FILENAME_LEN];
	vec3d		camera_pos;
	matrix		camera_orient;
	int			camera_time;		// ms
	int			flags;				// see BS_ flags above
	int			formula;
	int			num_icons;
	brief_icon	*icons;
	int			num_lines;
	brief_line	*lines;

	brief_stage( ) 
		: text( ), camera_time( 0 ), flags( 0 ), formula( -1 ),
		  num_icons( 0 ), icons( NULL ), num_lines( 0 ), lines( NULL )
	{ 
		voice[ 0 ] = 0;
		camera_pos = vmd_zero_vector;
		camera_orient = vmd_identity_matrix;
	}
};

class debrief_stage
{
public:
	int			formula;
	SCP_string	text;
	char			voice[MAX_FILENAME_LEN];
	SCP_string	recommendation_text;

	debrief_stage( ) 
		: formula( -1 ), text( ),
		  recommendation_text( )
	{ 
		voice[ 0 ] = 0;
	}
};

class briefing
{
public:
	int			num_stages;
	brief_stage	stages[MAX_BRIEF_STAGES];

	briefing()
		: num_stages(0)
	{}
};

class debriefing
{
public:
	int				num_stages;
	debrief_stage	stages[MAX_DEBRIEF_STAGES];

	debriefing()
		: num_stages(0)
	{}
};



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
extern briefing		Briefings[MAX_TVT_TEAMS];
extern debriefing		Debriefings[MAX_TVT_TEAMS];
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
void brief_init_screen(int multiplayer_flag);
void brief_render_map(int stage_num, float frametime);
void brief_set_new_stage(vec3d *pos, matrix *orient, int time, int stage_num);
void brief_camera_move(float frametime, int stage_num);
void brief_render_icon(int stage_num, int icon_num, float frametime, int selected = 0, float w_scale_factor = 1.0f, float h_scale_factor = 1.0f);
void brief_render_icon_line(int stage_num, int line_num);
void brief_init_map();
void brief_parse_icon_tbl();
void brief_common_close();
void brief_reset_icons(int stage_num);
void brief_restart_text_wipe();
void brief_reset_last_new_stage();
void brief_blit_stage_num(int stage_num, int stage_max);

void brief_common_get_icon_dimensions(int *w, int *h, brief_icon *bi);

// voice streaming interface
void brief_voice_init();
void brief_voice_load_all();
void brief_voice_unload_all();
void brief_voice_play(int stage_num);
void brief_voice_stop(int stage_num);
void brief_voice_pause(int stage_num);
void brief_voice_unpause(int stage_num);

// fancy briefing style text functions for use in other modules.
int brief_color_text_init(const char *src, int w, int instance = 0, int max_lines = MAX_BRIEF_LINES);
int brief_render_text(int line_offset, int x, int y, int h, float frametime, int instance = 0, int line_spacing = 0);

void cmd_brief_reset();

int brief_time_to_advance(int stage_num);

#endif
