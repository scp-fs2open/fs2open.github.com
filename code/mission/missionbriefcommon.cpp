/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "mission/missionbriefcommon.h"
#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "render/3d.h"
#include "io/timer.h"
#include "globalincs/linklist.h"
#include "io/mouse.h"
#include "missionui/missionbrief.h"
#include "mission/missiongrid.h"
#include "anim/animplay.h"
#include "math/fvi.h"
#include "gamesnd/gamesnd.h"
#include "sound/audiostr.h"
#include "missionui/missioncmdbrief.h"
#include "missionui/missiondebrief.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "sound/fsspeech.h"
#include "species_defs/species_defs.h"
#include "iff_defs/iff_defs.h"


// --------------------------------------------------------------------------------------
// briefing screen
// --------------------------------------------------------------------------------------

brief_screen bscreen;

// briefing screen sections
#define BRIEF_CUP_X1			400
#define BRIEF_CUP_Y1			70
#define BRIEF_CUP_X2			639
#define BRIEF_CUP_Y2			245
#define BRIEF_CUPINFO_X1	445
#define BRIEF_CUPINFO_Y1	247
#define BRIEF_CUPINFO_X2	639
#define BRIEF_CUPINFO_Y2	438

char *Brief_static_name[GR_NUM_RESOLUTIONS] = {
	"BriefMap",
	"2_BriefMap"
};

int Brief_static_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		10, 130
	},
	{ // GR_1024
		15, 208
	}
};

int Brief_bmap_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		0, 115
	},
	{ // GR_1024
		0, 184
	}
};

int Brief_grid_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		19, 147, 555, 232
	},
	{ // GR_1024
		30, 235, 888, 371
	}
};

int Brief_text_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		28, 399, 395, 74
	},
	{ // GR_1024
		46, 637, 630, 120
	}
};

int Brief_stage_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		138, 117
	},
	{ // GR_1024
		227, 194
	}
};

int Brief_stage_text_coords_multi[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		479, 385
	},
	{ // GR_1024
		821, 616
	}
};

int Brief_text_max_lines[GR_NUM_RESOLUTIONS] = {
	8, 13
};

#define LOOKAT_DIST	500.0f
#define STAGE_ADVANCE_DELAY	1000		// time in ms to wait after voice stops before advancing stage

// --------------------------------------------------------------------------------------
// Game-wide global data
// --------------------------------------------------------------------------------------
briefing		Briefings[MAX_TVT_TEAMS];			// there is exactly one briefing per mission
debriefing	Debriefings[MAX_TVT_TEAMS];			// there can be multiple debriefings per mission
briefing		*Briefing;							// pointer used in code -- points to correct briefing
debriefing	*Debriefing;						// pointer to correct debriefing

int			Briefing_voice_enabled=1;		// flag which turn on/off voice playback of briefings/debriefings

// --------------------------------------------------------------------------------------
// Module global data
// --------------------------------------------------------------------------------------

static int Last_new_stage;
int	Cur_brief_id;

const char BRIEF_META_CHAR = '$';

// camera related
static vec3d	Current_cam_pos;		// current camera position
static vec3d	Target_cam_pos;		// desired camera position
static matrix	Current_cam_orient;	// current camera orientation
static matrix	Target_cam_orient;	// desired camera orientation
static matrix	Start_cam_orient;		// start camera orientation
static vec3d	Start_cam_pos;			// position of camera at the start of a translation
static vec3d	Cam_vel;					//	camera velocity
static vec3d	Current_lookat_pos;	// lookat point
static vec3d	Target_lookat_pos;	// lookat point
static vec3d	Start_lookat_pos;
static vec3d	Lookat_vel;				//	lookat point velocity

static float	Start_cam_move;		// time at which camera started moving (seconds)
static float	Total_move_time;		// time in which camera should move from current pos to target pos (seconds)
static float	Elapsed_time;

static float	Start_dist;
static float	End_dist;
static float	Dist_change_rate;

static vec3d	Acc_limit;
static vec3d	Vel_limit;

static float	Total_dist;
static float	Peak_speed;
static float	Cam_accel;
static float	Last_dist;
static vec3d	W_init;

// flag to indicate that the sound for a spinning highlight animation has played
static int Brief_stage_highlight_sound_handle = -1;

// used for scrolling briefing text ( if necessary )
int		Num_brief_text_lines[MAX_TEXT_STREAMS];
int		Top_brief_text_line;

// Used to support drawing colored text for the briefing.  Gets complicates since we
// need to be able to draw one character at a time as well when the briefing text
// first appears.
typedef struct colored_char
{
	char	letter;
	ubyte	color;		// index into Brief_text_colors[]
} colored_char;

typedef SCP_vector<colored_char> briefing_line; 
typedef SCP_vector<briefing_line> briefing_stream; 
static briefing_stream Colored_stream[MAX_TEXT_STREAMS];

#define MAX_BRIEF_TEXT_COLORS		20
#define BRIEF_TEXT_WHITE			0
#define BRIEF_TEXT_BRIGHT_WHITE		1
#define BRIEF_TEXT_RED				2
#define BRIEF_TEXT_GREEN			3
#define BRIEF_TEXT_YELLOW			4
#define BRIEF_TEXT_BLUE				5
#define BRIEF_TEXT_FRIENDLY			6
#define BRIEF_TEXT_HOSTILE			7
#define BRIEF_TEXT_NEUTRAL			8
#define BRIEF_TEXT_BRIGHT_BLUE		9
#define BRIEF_TEXT_BRIGHT_GREEN		10
#define BRIEF_TEXT_BRIGHT_RED		11
#define BRIEF_TEXT_BRIGHT_YELLOW	12
#define BRIEF_TEXT_BLACK			13
#define BRIEF_TEXT_GREY				14
#define BRIEF_TEXT_SILVER			15
#define BRIEF_TEXT_VIOLET_GRAY		16
#define BRIEF_TEXT_VIOLET			17
#define BRIEF_TEXT_PINK				18
#define BRIEF_TEXT_LIGHT_PINK		19

color Brief_color_red, Brief_color_green, Brief_color_legacy_neutral;

color *Brief_text_colors[MAX_BRIEF_TEXT_COLORS] = 
{
	&Color_white,
	&Color_bright_white,
	&Color_red,
	&Color_green,
	&Color_yellow,
	&Color_blue,
	&Brief_color_green,
	&Brief_color_red,
	&Brief_color_legacy_neutral,
	&Color_bright_blue,
	&Color_bright_green,
	&Color_bright_red,
	&Color_bright_yellow,
	&Color_black,
	&Color_grey,
	&Color_silver,
	&Color_violet_gray,
	&Color_violet,
	&Color_pink,
	&Color_light_pink,
};

#define BRIGHTEN_LEAD	2

float Brief_text_wipe_time_elapsed;
static int Max_briefing_line_len;

static int Brief_voice_ended;
static int Brief_textdraw_finished;
static int Brief_voice_started;
static int Brief_stage_time;

const float		BRIEF_TEXT_WIPE_TIME	= 1.5f;		// time in seconds for wipe to occur
static int		Brief_text_wipe_snd;					// sound handle of sound effect for text wipe
static int		Play_brief_voice;

// animation stuff
static int		Play_highlight_flag;
static int		Cam_target_reached;
static int		Cam_movement_done;

// moving icons
typedef struct icon_move_info
{
	icon_move_info	*next, *prev;
	int				used;
	int				id;
	vec3d			start;
	vec3d			finish;
	vec3d			current;

	// used to move icons smoothly
	vec3d			direction;
	float				total_dist;
	float				accel;
	float				total_move_time;
	float				peak_speed;
	int				reached_dest;
	float				last_dist;
} icon_move_info;

#define MAX_MOVE_ICONS	10
icon_move_info	Icon_movers[MAX_MOVE_ICONS];
icon_move_info	Icon_move_list;	// head of linked list

// fading out icons
typedef struct fade_icon
{
	hud_anim	fade_anim;		// anim info
	vec3d	pos;
	int		team;
} fade_icon;

#define		MAX_FADE_ICONS	30
fade_icon	Fading_icons[MAX_FADE_ICONS];
int			Num_fade_icons;

// voice id's for briefing text
int Brief_voices[MAX_BRIEF_STAGES];

cmd_brief *Cur_cmd_brief;
cmd_brief Cmd_briefs[MAX_TVT_TEAMS];

// --------------------------------------------------------------------------------------
// forward declarations
// --------------------------------------------------------------------------------------
void	brief_render_elements(vec3d *pos, grid *gridp);
void	brief_render_icons(int stage_num, float frametime);
void	brief_grid_read_camera_controls( control_info * ci, float frametime );
void	brief_maybe_create_new_grid(grid *gridp, vec3d *pos, matrix *orient, int force = 0);
grid	*brief_create_grid(grid *gridp, vec3d *forward, vec3d *right, vec3d *center, int nrows, int ncols, float square_size);
grid	*brief_create_default_grid(void);
void	brief_render_grid(grid *gridp);
void	brief_modify_grid(grid *gridp);
void	brief_rpd_line(vec3d *v0, vec3d *v1);
void	brief_set_text_color(int color_index);
extern void get_camera_limits(matrix *start_camera, matrix *end_camera, float time, vec3d *acc_max, vec3d *w_max);
int brief_text_wipe_finished();

// --------------------------------------------------------------------------------------
//	brief_parse_icon_tbl()
//
//
void brief_parse_icon_tbl()
{
	int			num_icons, current_icon, rval;
	char			name[MAX_FILENAME_LEN];
	generic_anim	*ga;
	hud_anim		*ha;
	int species, icon;
	int num_species_covered;

	const int max_icons = Species_info.size() * MAX_BRIEF_ICONS;
	Assert( max_icons > 0 );

	generic_anim *temp_icon_bitmaps = new generic_anim[max_icons];
	hud_anim *temp_icon_fade_anims = new hud_anim[max_icons];
	hud_anim *temp_icon_highlight_anims = new hud_anim[max_icons];

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "icons.tbl", rval));
		lcl_ext_close();

		// Ensure no memory leak
		delete[] temp_icon_bitmaps;
		delete[] temp_icon_fade_anims;
		delete[] temp_icon_highlight_anims;

		return;
	}

	read_file_text("icons.tbl", CF_TYPE_TABLES);
	reset_parse();		

	required_string("#Start");

	num_icons = 0;
	while (required_string_either("#End","$Name:"))
	{
		if(num_icons >= max_icons) {
			Warning(LOCATION, "Too many icons in icons.tbl; only the first %d will be used", max_icons);
			skip_to_start_of_string("#End");
			break;
		}

		// parse regular frames
		required_string("$Name:");
		stuff_string(name, F_NAME, MAX_FILENAME_LEN);
		ga = &temp_icon_bitmaps[num_icons];
		generic_anim_init(ga, name);
	
		// parse fade frames
		required_string("$Name:");
		stuff_string(name, F_NAME, MAX_FILENAME_LEN);
		ha = &temp_icon_fade_anims[num_icons];
		hud_anim_init(ha, 0, 0, name);

		// parse highlighting frames
		required_string("$Name:");
		stuff_string(name, F_NAME, MAX_FILENAME_LEN);
		ha = &temp_icon_highlight_anims[num_icons];
		hud_anim_init(ha, 0, 0, name);

		// next icon
		num_icons++;
	}
	required_string("#End");

	// close localization
	lcl_ext_close();


	// now assign the icons to their species
	num_species_covered = num_icons / MAX_BRIEF_ICONS;
	current_icon = 0;
	for (icon = 0; icon < MAX_BRIEF_ICONS; icon++)
	{
		for (species = 0; species < num_species_covered; species++)
		{
			Species_info[species].icon_bitmaps[icon] = temp_icon_bitmaps[current_icon];
			Species_info[species].icon_fade_anims[icon] = temp_icon_fade_anims[current_icon];
			Species_info[species].icon_highlight_anims[icon] = temp_icon_highlight_anims[current_icon];

			current_icon++;
		}
	}

	// error check
	if (num_species_covered < (int)Species_info.size())
	{
		char *errormsg = new char[70 + (Species_info.size() * (NAME_LENGTH))];

		strcpy(errormsg, "The following species are missing icon info in icons.tbl:\n");
		for (species = num_species_covered; species < (int)Species_info.size(); species++)
		{
			strcat(errormsg, Species_info[species].species_name);
			strcat(errormsg, "\n");
		}
		strcat(errormsg, "\0");

		Error(LOCATION, errormsg);

		// probably won't get here, but just so we know about it
		delete[] errormsg;
	}

	delete[] temp_icon_bitmaps;
	delete[] temp_icon_fade_anims;
	delete[] temp_icon_highlight_anims;
}


void brief_set_icon_color(int team)
{
	gr_set_color_fast(iff_get_color_by_team(team, -1, 0));
}

// --------------------------------------------------------------------------------------
//	brief_move_icon_reset()
//
//
void brief_move_icon_reset()
{
	int i;

	list_init(&Icon_move_list);
	for ( i = 0; i < MAX_MOVE_ICONS; i++ )
		Icon_movers[i].used = 0;
}


/**
 * Does one time initialization of the briefing and debriefing structures.
 * Namely setting all malloc'ble pointers to NULL.  Called once at game startup.
 */
void mission_brief_common_init()
{
	int i,j;

	// setup brief text colors
	gr_init_alphacolor( &Brief_color_green, 50, 100, 50, 255 );
	gr_init_alphacolor( &Brief_color_red, 140, 20, 20, 255 );
	gr_init_alphacolor( &Brief_color_legacy_neutral, 255, 0, 0, iff_get_alpha_value(false));

	// extra catch to reset everything that's already loaded - taylor
	mission_brief_common_reset();
	mission_debrief_common_reset();

	if ( Fred_running )	{
		// If Fred is running malloc out max space
		for (i = 0; i < MAX_TVT_TEAMS; i++) {
			for (j = 0; j < MAX_BRIEF_STAGES; j++) {
				if (Briefings[i].stages[j].new_text == NULL) {
					Briefings[i].stages[j].new_text = (char *)vm_malloc(MAX_BRIEF_LEN);
					Assert( Briefings[i].stages[j].new_text != NULL );
					memset( Briefings[i].stages[j].new_text, 0, MAX_BRIEF_LEN );
				}

				if (Briefings[i].stages[j].icons == NULL) {
					Briefings[i].stages[j].icons = (brief_icon *)vm_malloc(sizeof(brief_icon) * MAX_STAGE_ICONS);
					Assert( Briefings[i].stages[j].icons != NULL );
					memset( Briefings[i].stages[j].icons, 0, sizeof(brief_icon) * MAX_STAGE_ICONS );
				}

				if (Briefings[i].stages[j].lines == NULL) {
					Briefings[i].stages[j].lines = (brief_line *)vm_malloc(sizeof(brief_line) * MAX_BRIEF_STAGE_LINES);
					Assert( Briefings[i].stages[j].lines != NULL );
					memset( Briefings[i].stages[j].lines, 0, sizeof(brief_line) * MAX_BRIEF_STAGE_LINES );
				}	

				Briefings[i].stages[j].num_icons = 0;
				Briefings[i].stages[j].num_lines = 0;
			}
		}

		for (i = 0; i < MAX_TVT_TEAMS; i++) {
			for (j = 0; j < MAX_DEBRIEF_STAGES; j++) {
				if (Debriefings[i].stages[j].new_text == NULL) {
					Debriefings[i].stages[j].new_text = (char *)vm_malloc(MAX_DEBRIEF_LEN);
					Assert( Debriefings[i].stages[j].new_text != NULL );
					memset( Debriefings[i].stages[j].new_text, 0, MAX_DEBRIEF_LEN );
				}

				if (Debriefings[i].stages[j].new_recommendation_text == NULL) {
					Debriefings[i].stages[j].new_recommendation_text = (char *)vm_malloc(MAX_RECOMMENDATION_LEN);
					Assert(Debriefings[i].stages[j].new_recommendation_text != NULL);
					memset( Debriefings[i].stages[j].new_recommendation_text, 0, MAX_RECOMMENDATION_LEN );
				}
			}
		}

	} else {
		// If game is running don't malloc anything
		for (i=0; i<MAX_TVT_TEAMS; i++ )	{
			for (j=0; j<MAX_BRIEF_STAGES; j++ )	{
				Briefings[i].stages[j].new_text = NULL;
				Briefings[i].stages[j].num_icons = 0;
				Briefings[i].stages[j].icons = NULL;
				Briefings[i].stages[j].num_lines = 0;
				Briefings[i].stages[j].lines = NULL;
			}
		}

		for (i=0; i<MAX_TVT_TEAMS; i++ )	{
			for (j=0; j<MAX_DEBRIEF_STAGES; j++ )	{
				Debriefings[i].stages[j].new_text = NULL;
				Debriefings[i].stages[j].new_recommendation_text = NULL;
			}
		}

	}
}

/**
 * Frees all the memory allocated in the briefing and debriefing structures and sets all pointers to NULL.
 */
void mission_brief_common_reset()
{
	int i, j;

	for (i = 0; i < MAX_TVT_TEAMS; i++) {
		Briefings[i].num_stages = 0;

		for (j = 0; j < MAX_BRIEF_STAGES; j++) {
			Briefings[i].stages[j].num_icons = 0;
			Briefings[i].stages[j].num_lines = 0;

			if (Fred_running) {
				if ( Briefings[i].stages[j].new_text )
					memset( Briefings[i].stages[j].new_text, 0, MAX_BRIEF_LEN );

				if ( Briefings[i].stages[j].icons ) {
					memset( Briefings[i].stages[j].icons, 0, sizeof(brief_icon) * MAX_STAGE_ICONS );
					Briefings[i].stages[j].icons->ship_class = -1;
					Briefings[i].stages[j].icons->modelnum = -1;
					Briefings[i].stages[j].icons->bitmap_id = -1;
				}

				if ( Briefings[i].stages[j].lines )
					memset( Briefings[i].stages[j].lines, 0, sizeof(brief_line) * MAX_BRIEF_STAGE_LINES );
			} else {
				if ( Briefings[i].stages[j].new_text )	{
					vm_free(Briefings[i].stages[j].new_text);
					Briefings[i].stages[j].new_text = NULL;			
				}
	
				if ( Briefings[i].stages[j].icons )	{
					vm_free(Briefings[i].stages[j].icons);
					Briefings[i].stages[j].icons = NULL;
				}

				if ( Briefings[i].stages[j].lines )	{
					vm_free(Briefings[i].stages[j].lines);
					Briefings[i].stages[j].lines = NULL;
				}
			}
		}
	}
}

/**
 * Split from above since we need to clear them separately
 * @see mission_brief_common_reset()
 */
void mission_debrief_common_reset()
{
	int i, j;

	for (i = 0; i < MAX_TVT_TEAMS; i++) {
		Debriefings[i].num_stages = 0;

		for (j = 0; j < MAX_DEBRIEF_STAGES; j++) {
			if (Fred_running) {
				if ( Debriefings[i].stages[j].new_text )
					memset( Debriefings[i].stages[j].new_text, 0, MAX_DEBRIEF_LEN );

				if ( Debriefings[i].stages[j].new_recommendation_text )
					memset( Debriefings[i].stages[j].new_recommendation_text, 0, MAX_RECOMMENDATION_LEN );
			} else {
				if ( Debriefings[i].stages[j].new_text ) {
					vm_free(Debriefings[i].stages[j].new_text);
					Debriefings[i].stages[j].new_text = NULL;
				}

				if ( Debriefings[i].stages[j].new_recommendation_text ) {
					vm_free(Debriefings[i].stages[j].new_recommendation_text);
					Debriefings[i].stages[j].new_recommendation_text = NULL;
				}
			}
		}
	}
}




// --------------------------------------------------------------------------------------
//	brief_reset()
//
//
void brief_reset()
{
	mission_brief_common_reset();

	Briefing = NULL;
	Cur_brief_id = 1;
}

// --------------------------------------------------------------------------------------
//	debrief_reset()
//
//
void debrief_reset()
{
	mission_debrief_common_reset();

	Debriefing = NULL;

	// MWA 4/27/98 -- must initialize this variable here since we cannot do it as debrief
	// init time because race conditions between all players in the game make that type of
	// initialization unsafe.
	Debrief_multi_stages_loaded = 0;
}

/**
 * Set up the screen regions.  A mulitplayer briefing will look different than a single player briefing.
 */
void brief_init_screen(int multiplayer_flag)
{
	bscreen.map_x1			= Brief_grid_coords[gr_screen.res][0];
	bscreen.map_x2			= Brief_grid_coords[gr_screen.res][0] + Brief_grid_coords[gr_screen.res][2];
	bscreen.map_y1			= Brief_grid_coords[gr_screen.res][1];
	bscreen.map_y2			= Brief_grid_coords[gr_screen.res][1] + Brief_grid_coords[gr_screen.res][3];
}

// --------------------------------------------------------------------------------------
//	brief_init_colors()
//
//
void brief_init_colors()
{
}

void brief_preload_icon_anim(brief_icon *bi)
{
	generic_anim *ga;
	int species = ship_get_species_by_type(bi->ship_class);

	if(species < 0) {
		return;
	}

	ga = &Species_info[species].icon_bitmaps[bi->type];
	if ( !stricmp(NOX("none"), ga->filename) ) {
		return;
	}

	// force read of data from disk, so we don't glitch on initial playback
	if ( ga->first_frame == -1 ) {
		ga->first_frame = bm_load_animation(ga->filename, &ga->num_frames);
		Assert(ga->first_frame >= 0);
	}
}

void brief_preload_fade_anim(brief_icon *bi)
{
	hud_anim *ha;
	int species = ship_get_species_by_type(bi->ship_class);

	if(species < 0){
		return;
	}

	ha = &Species_info[species].icon_fade_anims[bi->type];
	if ( !stricmp(NOX("none"), ha->filename) ) {
		return;
	}

	// force read of data from disk, so we don't glitch on initial playback
	if ( ha->first_frame == -1 ) {
		hud_anim_load(ha);
		Assert(ha->first_frame >= 0);
	}

	gr_set_bitmap(ha->first_frame);
	gr_aabitmap(0, 0);
}

void brief_preload_highlight_anim(brief_icon *bi)
{
	hud_anim *ha;
	int species = ship_get_species_by_type(bi->ship_class);

	if(species < 0){
		return;
	}

	ha = &Species_info[species].icon_highlight_anims[bi->type];
	if ( !stricmp(NOX("none"), ha->filename) ) {
		return;
	}

	// force read of data from disk, so we don't glitch on initial playback
	if ( ha->first_frame == -1 ) {
		hud_anim_load(ha);
		Assert(ha->first_frame >= 0);
	}

	bi->highlight_anim = *ha;

	gr_set_bitmap(ha->first_frame);
	gr_aabitmap(0, 0);
}

/**
 * Preload highlight, fadein and fadeout animations that are used in each stage
 */
void brief_preload_anims()
{
	int			num_icons, num_stages, i, j;
	brief_icon	*bi;

	num_stages = Briefing->num_stages;

	for ( i = 0; i < num_stages; i++ ) {
		num_icons = Briefing->stages[i].num_icons;
		for ( j = 0; j < num_icons; j++ ) {
			bi = &Briefing->stages[i].icons[j];

			brief_preload_icon_anim(bi);
			brief_preload_fade_anim(bi);
			if ( bi->flags & BI_HIGHLIGHT ) {
				brief_preload_highlight_anim(bi);
			}
		}
	}
}

// --------------------------------------------------------------------------------------
//	brief_init_map()
//
//
void brief_init_map()
{
	vec3d *pos;
	matrix *orient;

	Assert( Briefing != NULL );

	pos = &Briefing->stages[0].camera_pos;
	orient = &Briefing->stages[0].camera_orient;
	vm_vec_zero(&Current_lookat_pos);
	vm_vec_zero(&Target_lookat_pos);
	Elapsed_time = 0.0f;
	Total_move_time = 0.0f;

	The_grid = brief_create_default_grid();
	brief_maybe_create_new_grid(The_grid, pos, orient, 1);

	brief_init_colors();
	brief_move_icon_reset();

	brief_preload_anims();

	Brief_text_wipe_snd = -1;
	Last_new_stage = -1;
	Num_fade_icons=0;
}


#pragma optimize("", off)

void brief_render_fade_outs(float frametime)
{
	int			i,bx,by,w,h;
	float			bxf,byf;
	vertex		tv;			// temp vertex used to find screen position for text
	fade_icon	*fi;
	

	for (i=0; i<Num_fade_icons; i++) {
		fi = &Fading_icons[i];

		g3_rotate_vertex(&tv, &fi->pos);

		if (!(tv.flags & PF_PROJECTED))
			g3_project_vertex(&tv);

		if (!(tv.flags & PF_OVERFLOW) ) {  // make sure point projected before drawing text

			brief_set_icon_color(fi->team);

			if ( fi->fade_anim.first_frame < 0 ) {
				continue;
			}

			bm_get_info( fi->fade_anim.first_frame, &w, &h, NULL);

			gr_resize_screen_pos( &w, &h );

			bxf = tv.screen.xyw.x - w / 2.0f + 0.5f;
			byf = tv.screen.xyw.y - h / 2.0f + 0.5f;
			bx = fl2i(bxf);
			by = fl2i(byf);

			if ( fi->fade_anim.first_frame >= 0 ) {
				fi->fade_anim.sx = bx;
				fi->fade_anim.sy = by;
				hud_anim_render(&fi->fade_anim, frametime, 1, 0, 0, 0, false);
			}
		}
	}
}

/**
 * Figure out how far an icon should move based on the elapsed time
 */
float brief_icon_get_dist_moved(icon_move_info *mi, float elapsed_time)
{
	float time, dist_moved=0.0f;
	
	// first half of movement
	if ( elapsed_time < mi->total_move_time/2.0f ) {
		dist_moved=0.5f*mi->accel*elapsed_time*elapsed_time;	// d = 1/2at^2
		return dist_moved;
	}

	// second half of movement
	time=elapsed_time - mi->total_move_time/2.0f;
	dist_moved=(mi->total_dist/2.0f)+(mi->peak_speed*time) - 0.5f*mi->accel*time*time;
	return dist_moved;
}

/**
 * Draw a line between two icons on the briefing screen
 */
void brief_render_icon_line(int stage_num, int line_num)
{
	brief_line	*bl;
	brief_stage *bs;
	brief_icon	*icon[2];
	int			i;
	vertex		icon_vertex[2];
	int			icon_status[2] = {0,0};
	int			icon_w, icon_h;
	float			icon_x[2], icon_y[2];

	bl = &Briefing->stages[stage_num].lines[line_num];
	bs = &Briefing->stages[stage_num];

	if(bl->start_icon < 0 || bl->start_icon >= bs->num_icons)
	{
		Warning(LOCATION, "Start icon (%d/%d) missing for line %d in briefing stage %d", bl->start_icon, bs->num_icons, line_num, stage_num);
		//Remove line
		bs->num_lines--;
		for(i = line_num; i < bs->num_lines; i++)
			bs->lines[i] = bs->lines[i+1];
		return;
	}
	if(bl->end_icon < 0 || bl->end_icon >= Briefing->stages[stage_num].num_icons)
	{
		Warning(LOCATION, "End icon (%d/%d) missing for line %d in briefing stage %d", bl->end_icon, bs->num_icons, line_num, stage_num);
		//Remove line
		bs->num_lines--;
		for(i = line_num; i < bs->num_lines; i++)
			bs->lines[i] = bs->lines[i+1];
		return;
	}

	icon[0] = &Briefing->stages[stage_num].icons[bl->start_icon];
	icon[1] = &Briefing->stages[stage_num].icons[bl->end_icon];

	// project icons
	for (i=0; i<2; i++) {
		g3_rotate_vertex(&icon_vertex[i],&icon[i]->pos);
		if (!(icon_vertex[i].flags&PF_PROJECTED))
			g3_project_vertex(&icon_vertex[i]);

		if (!(icon_vertex[i].flags & PF_OVERFLOW) ) {  // make sure point projected before drawing text
			icon_status[i]=1;
		}
	}

	if ( !icon_status[0] || !icon_status[1] ) {
		return;
	}

	// get screen (x,y) for icons
	for (i=0; i<2; i++) {
		brief_common_get_icon_dimensions(&icon_w, &icon_h, icon[i]->type, icon[i]->ship_class);
		icon_x[i] = icon_vertex[i].screen.xyw.x;
		icon_y[i] = icon_vertex[i].screen.xyw.y;
	}

	brief_set_icon_color(icon[0]->team);

	gr_line(fl2i(icon_x[0]), fl2i(icon_y[0]), fl2i(icon_x[1]), fl2i(icon_y[1]), false);
}

/**
 * Draw a briefing icon
 *
 * @param stage_num	briefing stage number (start at 0)
 * @param icon_num icon number in stage
 * @param frametime	time elapsed in seconds
 * @param selected FRED only (will be 0 or non-zero)
 * @param w_scale_factor scale icon in width by this amount (default 1.0f)
 * @param h_scale_factor scale icon in height by this amount (default 1.0f)
 */
void brief_render_icon(int stage_num, int icon_num, float frametime, int selected, float w_scale_factor, float h_scale_factor)
{
	brief_icon	*bi, *closeup_icon;
	generic_anim *ga;
	vertex		tv;	// temp vertex used to find screen position for text
	vec3d		*pos = NULL;
	int			bx,by,bc,w,h,icon_w,icon_h,icon_bitmap=-1;
	float			bxf, byf, dist=0.0f;
	bool mirror_icon;

	Assert( Briefing != NULL );
	
	bi = &Briefing->stages[stage_num].icons[icon_num];
	mirror_icon = (bi->flags & BI_MIRROR_ICON)? true:false;

	icon_move_info *mi, *next;
	int interp_pos_found = 0;
	
	mi = GET_FIRST(&Icon_move_list);
	if (mi)
		while ( mi != &Icon_move_list ) {
			next = mi->next;
			if ( ( mi->id != 0 ) && ( mi->id == bi->id ) ) {

				if ( !mi->reached_dest ) {
					dist = brief_icon_get_dist_moved(mi, Elapsed_time);
					if ( dist < mi->last_dist ) {
						mi->reached_dest=1;
						mi->last_dist=0.0f;
					}
					mi->last_dist=dist;
				}

				if ( !mi->reached_dest ) {
					vec3d dist_moved;
					vm_vec_copy_scale(&dist_moved, &mi->direction, dist);
					vm_vec_add(&mi->current, &mi->start, &dist_moved);
				} else {
					mi->current = mi->finish;
				}

				pos = &mi->current;
				interp_pos_found = 1;
				break;
			}
			mi = next;
		}
	
	if ( !interp_pos_found )
		pos = &bi->pos;
		
	brief_render_elements(pos, The_grid);
	g3_rotate_vertex(&tv,pos);

	if (!(tv.flags&PF_PROJECTED))
		g3_project_vertex(&tv);

	if (!(tv.flags & PF_OVERFLOW) ) {  // make sure point projected before drawing text

		brief_set_icon_color(bi->team);

		int species = ship_get_species_by_type(bi->ship_class);

		if(species < 0){
			return;
		}

		ga = &Species_info[species].icon_bitmaps[bi->type];
		if (ga->first_frame < 0) {
			Int3();
			return;
		}

		brief_common_get_icon_dimensions(&icon_w, &icon_h, bi->type, bi->ship_class);

		closeup_icon = brief_get_closeup_icon();
		if ( bi == closeup_icon || selected ) {
			icon_bitmap = ga->first_frame+1;
//			gr_set_bitmap(ga->first_frame+1);
		}
		else {
			icon_bitmap = ga->first_frame;
//			gr_set_bitmap(ga->first_frame);
		}
	
		// draw icon centered at draw position
//		bx = fl2i(tv.sx - ib->icon_w/2.0f);
//		by = fl2i(tv.sy - ib->icon_h/2.0f);
//		bc = bx + fl2i(ib->icon_w/2.0f);
		//gr_aabitmap(bx, by);

		float scaled_w, scaled_h;

		float sx = tv.screen.xyw.x;
		float sy = tv.screen.xyw.y;
		gr_unsize_screen_posf( &sx, &sy );
	
		scaled_w = icon_w * w_scale_factor;
		scaled_h = icon_h * h_scale_factor;
		bxf = sx - scaled_w / 2.0f + 0.5f;
		byf = sy - scaled_h / 2.0f + 0.5f;
		bx = fl2i(bxf);
		by = fl2i(byf);
		bc = fl2i(sx);

		if ( (bx < 0) || (bx > gr_screen.max_w_unscaled) || (by < 0) || (by > gr_screen.max_h_unscaled) && !Fred_running ) {
			bi->x = bx;
			bi->y = by;
			return;
		}
/*
		vertex va, vb;
		va.sx = bxf;
		va.sy = byf;
		va.u = 0.0f;
		va.v = 0.0f;

		vb.sx = bxf+scaled_w-1;
		vb.sy = byf+scaled_h-1;
		vb.u = 1.0f;
		vb.v = 1.0f;
*/
		// render highlight anim frame
		if ( (bi->flags&BI_SHOWHIGHLIGHT) && (bi->flags&BI_HIGHLIGHT) ) {
			hud_anim *ha = &bi->highlight_anim;
			if ( ha->first_frame >= 0 ) {
				ha->sx = bi->hold_x;
				if (bi->label[0] != '\0') {
					ha->sy = bi->hold_y - fl2i(gr_get_font_height()/2.0f +0.5) - 2;
				} else {
					ha->sy = bi->hold_y;
				}

				//hud_set_iff_color(bi->team);
				brief_set_icon_color(bi->team);

				hud_anim_render(ha, frametime, 1, 0, 1, 0, true, mirror_icon);

				if ( Brief_stage_highlight_sound_handle < 0 ) {
					if ( !Fred_running) {
						Brief_stage_highlight_sound_handle = snd_play(&Snds_iface[SND_ICON_HIGHLIGHT]);					
					}
				}
			}
		}

		// render fade-in anim frame
		if ( bi->flags & BI_FADEIN ) {
			hud_anim *ha = &bi->fadein_anim;
			if ( ha->first_frame >= 0 ) {
				ha->sx = bx;
				ha->sy = by;
//				hud_set_iff_color(bi->team);
				brief_set_icon_color(bi->team);

				if ( hud_anim_render(ha, frametime, 1, 0, 0, 1, true, mirror_icon) == 0 ) {
					bi->flags &= ~BI_FADEIN;
				}
			} else {
				bi->flags &= ~BI_FADEIN;
			}
		}		

		if ( !(bi->flags & BI_FADEIN) ) {
			gr_set_bitmap(icon_bitmap);
			gr_aabitmap(bx, by, true,mirror_icon);

			// draw text centered over the icon (make text darker)
			if ( bi->type == ICON_FIGHTER_PLAYER || bi->type == ICON_BOMBER_PLAYER ) {
				gr_get_string_size(&w,&h,Players[Player_num].callsign);
				gr_string(bc - fl2i(w/2.0f), by - h, Players[Player_num].callsign);
			}
			else {
				if (Lcl_gr) {
					char buf[128];
					strcpy_s(buf, bi->label);
					lcl_translate_brief_icon_name(buf);
					gr_get_string_size(&w, &h, buf);
					gr_string(bc - fl2i(w/2.0f), by - h, buf);
				} else {
					gr_get_string_size(&w,&h,bi->label);
					gr_string(bc - fl2i(w/2.0f), by - h, bi->label);
				}
			}

			// show icon as selected (FRED only)
			if ( selected ) {
				gr_get_string_size(&w,&h,NOX("(S)"));
				gr_printf(bc - fl2i(w/2.0f), by - h*2, NOX("(S)"));
			}
		}

		// store screen x,y,w,h
		bi->x = bx;
		bi->y = by;
		bi->w = fl2i(scaled_w);
		bi->h = fl2i(scaled_h);

	}  // end if vertex is projected
}

#pragma optimize("", on)

// -------------------------------------------------------------------------------------
// brief_render_icons()
//
void brief_render_icons(int stage_num, float frametime)
{
	int i, num_icons, num_lines;

	Assert( Briefing != NULL );
	
	num_icons = Briefing->stages[stage_num].num_icons;
	num_lines = Briefing->stages[stage_num].num_lines;

	if ( Cam_target_reached ) {
		for ( i = 0; i < num_lines; i++ ) {
			brief_render_icon_line(stage_num, i);
		}
	}

	for ( i = 0; i < num_icons; i++ ) {
		brief_render_icon(stage_num, i, frametime, 0);
	}
}

// ------------------------------------------------------------------------------------
// brief_start_highlight_anims()
//
//	see if there are any highlight animations to play
//
void brief_start_highlight_anims(int stage_num)
{
	brief_stage		*bs;
	brief_icon		*bi;
	int				x,y,i,anim_w,anim_h;

	Assert( Briefing != NULL );
	bs = &Briefing->stages[stage_num];
	
	for ( i = 0; i < bs->num_icons; i++ ) {
		bi = &bs->icons[i];
		if ( bi->flags & BI_HIGHLIGHT ) {
			bi->flags &= ~BI_SHOWHIGHLIGHT;
			if ( bi->highlight_anim.first_frame < 0 ) {
				continue;
			}

			bi->highlight_anim.time_elapsed=0.0f;

			bm_get_info( bi->highlight_anim.first_frame, &anim_w, &anim_h, NULL);
			x = fl2i( i2fl(bi->x) + bi->w/2.0f - anim_w/2.0f );
			y = fl2i( i2fl(bi->y) + bi->h/2.0f - anim_h/2.0f );
			bi->hold_x = x;
			bi->hold_y = y;
			bi->flags |= BI_SHOWHIGHLIGHT;
			bi->highlight_anim.time_elapsed=0.0f;
		}
	}
}

// -------------------------------------------------------------------------------------
// brief_render_map()
//
//
void brief_render_map(int stage_num, float frametime)
{
	brief_stage *bs;

	gr_set_clip(bscreen.map_x1 + 1, bscreen.map_y1 + 1, bscreen.map_x2 - bscreen.map_x1 - 1, bscreen.map_y2 - bscreen.map_y1 - 2);
	
	// REMOVED by neilk: removed gr_clear for FS2 because interface no longer calls for black background on grid
	//	gr_clear();

  if (stage_num >= Briefing->num_stages) {
		gr_reset_clip();
		return;
	}

	Assert(Briefing);
	bs = &Briefing->stages[stage_num];

	g3_start_frame(0);
	g3_set_view_matrix(&Current_cam_pos, &Current_cam_orient, 0.5f);

	brief_maybe_create_new_grid(The_grid, &Current_cam_pos, &Current_cam_orient);
	brief_render_grid(The_grid);

	brief_render_fade_outs(frametime);

	// go ahead and render everything that is in the active objects list
	brief_render_icons(stage_num, frametime);

	if ( Cam_target_reached && brief_text_wipe_finished() ) {

		if ( Brief_textdraw_finished == 0 ) {
			Brief_textdraw_finished = 1;
			Brief_stage_time = 0;
		}

		if ( Play_highlight_flag ) {
			brief_start_highlight_anims(stage_num);
			Play_highlight_flag = 0;
		}
	}

	anim_render_all(ON_BRIEFING_SELECT, frametime);

	gr_reset_clip();
	g3_end_frame();
}

// Display what stage of the briefing is active
void brief_blit_stage_num(int stage_num, int stage_max)
{
	char buf[64];
	// int w;

	Assert( Briefing != NULL );
	gr_set_color_fast(&Color_text_heading);
	sprintf(buf, XSTR( "Stage %d of %d", 394), stage_num + 1, stage_max);
	if (Game_mode & GM_MULTIPLAYER) {
		gr_printf(Brief_stage_text_coords_multi[gr_screen.res][0], Brief_stage_text_coords_multi[gr_screen.res][1], buf);
	} else {
		gr_printf(Brief_stage_text_coords[gr_screen.res][0], Brief_stage_text_coords[gr_screen.res][1], buf);
	}

	// draw the title of the mission	
	// if this goes above briefing text, it will need to be raised 10 pixels in multiplayer to make
	// room for stage num, which makes toom for chat box
	/*
	if (Game_mode & GM_MULTIPLAYER) {
		gr_get_string_size(&w,NULL,The_mission.name);
		gr_string(bscreen.map_x2 - w, bscreen.map_y2 + 5, The_mission.name);		
	} else {
		gr_get_string_size(&w,NULL,The_mission.name);
		gr_string(bscreen.map_x2 - w, bscreen.map_y2 + 5, The_mission.name);		
	}
	*/
}

/**
 * Render a line of text for the briefings.  Lines are drawn in as a wipe, with leading bright
 * white characters.  Have to jump through some hoops since we support colored words.  This means
 * that we need to process the line one character at a time.
 *
 * @param line_num number of the line of the briefing page to be drawn
 * @param x horizontal position where the text is drawn
 * @param y vertical position where the text is drawn
 * @param instance index of Colored_stream of the text page to display
 */
void brief_render_line(int line_num, int x, int y, int instance)
{
	Assert( 0<=instance && instance < (sizeof(Colored_stream)/sizeof(*Colored_stream)) );

	SCP_vector<colored_char> *src = &Colored_stream[instance].at(line_num);

	// empty strings do not have to be drawn
	int src_len = src->size();
	if (src_len == 0){
		return;
	}
	// truncate_len is the number of characters currently displayed including the bright white characters
	int truncate_len = fl2i(Brief_text_wipe_time_elapsed / BRIEF_TEXT_WIPE_TIME * Max_briefing_line_len);
	if (truncate_len > src_len){
		truncate_len = src_len;
	}
	// truncate_len is going to be the number of characters displayed with normal intensity
	// bright_len is the additional characters displayed with high intensity
	// bright_len+truncate_len<len chars are displayed
	int bright_len = 0;
	if (truncate_len < src_len) {
		if (truncate_len <= BRIGHTEN_LEAD) {
			bright_len = truncate_len;
			truncate_len = 0;
		} else {
			bright_len = BRIGHTEN_LEAD;
			truncate_len -= BRIGHTEN_LEAD; 
		}
	}
	int char_seq_pos=0; //Cursor position into the following character sequence
	char char_seq[MAX_BRIEF_LINE_LEN];	
	int offset = 0; //offset is the horizontal position of the screen where strings are drawn
	gr_set_color_fast(&Color_white);

	// PART1: Draw the the briefing line part with normal intensity and word colors.
	// The following algorithm builds a character sequence of color 'last_color' into 'line' buffer.
	// When the color changes, the buffer is drawn and reset.
	{
		int last_color = src->at(0).color;    
		for (int current_pos=0; current_pos<truncate_len; current_pos++) {
			colored_char &current_char=src->at(current_pos);		
			//when the current color changes, the accumulated character sequence is drawn.
			if (current_char.color != last_color){
				//add a 0 terminal character to make line a valid C string
				Assert(char_seq_pos<sizeof(char_seq));
				char_seq[char_seq_pos] = 0;         
				{	// Draw coloured text, and increment cariage position
					int w=0,h=0;
					brief_set_text_color(last_color);        
					gr_string(x + offset, y, char_seq);
					gr_get_string_size(&w, &h, char_seq);
					offset += w;
				}
				//clear char buffer
				char_seq_pos = 0;
				last_color = current_char.color;
			}
			Assert(char_seq_pos<sizeof(char_seq));
			char_seq[char_seq_pos++] = current_char.letter;		
		}
		// Draw the final chunk of acumulated characters
		// Add a 0 terminal character to make line a valid C string
		Assert(char_seq_pos<sizeof(char_seq));
		char_seq[char_seq_pos] = 0;
        {	// Draw coloured text, and increment cariage position
			int w=0,h=0;
			brief_set_text_color(last_color);        
			gr_string(x + offset, y, char_seq);
			gr_get_string_size(&w, &h, char_seq);
			offset += w;
		}
	}

	{	// PART2: Draw leading bright white characters
		char_seq_pos = 0;
		for( int current_pos = truncate_len; current_pos<truncate_len + bright_len; current_pos++){		
			Assert(char_seq_pos<sizeof(char_seq));
			char_seq[char_seq_pos++] = src->at(current_pos).letter;
		}
		Assert(char_seq_pos<sizeof(char_seq));
		char_seq[char_seq_pos] = 0;
		gr_set_color_fast(&Color_bright_white);
		gr_string(x + offset, y, char_seq);    
	}
}

int brief_text_wipe_finished()
{
	if ( Brief_text_wipe_time_elapsed > (BRIEF_TEXT_WIPE_TIME+0.5f) ) {
		return 1;
	}

	return 0;
}

/**
 * brief_render_text()
 *
 * @param line_offset
 * @param x
 * @param y
 * @param h
 * @param frametime	time in seconds of previous frame
 * @param instance optional parameter.  Used to indicate which text stream is used. This value is 0 unless multiple text streams are required
 * @param line_spacing
 * @return
 */
int brief_render_text(int line_offset, int x, int y, int h, float frametime, int instance, int line_spacing)
{
	int fh, line, yy;

	fh = gr_get_font_height();
	if (Brief_text_wipe_time_elapsed == 0) {
		if (snd_is_playing(Brief_text_wipe_snd)) {
			snd_stop(Brief_text_wipe_snd);
		}
		gamesnd_play_iface(SND_BRIEF_TEXT_WIPE);
		Play_brief_voice = 1;
	}

	Brief_text_wipe_time_elapsed += frametime;

	line = line_offset;
	yy = 0;
	while (yy + fh <= h) {
		if (line >= Num_brief_text_lines[instance])
			break;

		brief_render_line(line, x, y + yy, instance);

		line++;
		yy += fh + line_spacing;
	}

	if ( brief_text_wipe_finished() && (Play_brief_voice) ) {
		Play_brief_voice = 0;
		return 1;
	}

	return 0;
}

/** 
 * Draw the lines that show objects positions on the grid
 *
 */
void brief_render_elements(vec3d *pos, grid* gridp)
{
	vec3d	gpos;	//	Location of point on grid.
//	vec3d	tpos;
	float		dxz;
	plane		tplane;
	vec3d	*gv;
	
	if ( pos->xyz.y < 1 && pos->xyz.y > -1 )
		return;

	tplane.A = gridp->gmatrix.vec.uvec.xyz.x;
	tplane.B = gridp->gmatrix.vec.uvec.xyz.y;
	tplane.C = gridp->gmatrix.vec.uvec.xyz.z;
	tplane.D = gridp->planeD;

	compute_point_on_plane(&gpos, &tplane, pos);

	dxz = vm_vec_dist(pos, &gpos)/8.0f;

	gv = &gridp->gmatrix.vec.uvec;
	if (gv->xyz.x * pos->xyz.x + gv->xyz.y * pos->xyz.y + gv->xyz.z * pos->xyz.z < -gridp->planeD)
		gr_set_color(127, 127, 127);
	else
		gr_set_color(255, 255, 255);   // white
}


// ------------------------------------------------------------------------------------
// brief_reset_icons()
//
void brief_reset_icons(int stage_num)
{
	brief_stage		*bs;
	brief_icon		*bi;
	int				i;

	Assert( Briefing != NULL );
	bs = &Briefing->stages[stage_num];

	for ( i = 0; i < bs->num_icons; i++ ) {
		bi = &bs->icons[i];
		bi->flags &= ~BI_SHOWHIGHLIGHT;
	}
}


/**
 * Direct camera to look at target
 *
 * @param pos target position for the camera
 * @param orient target orientation for the camera
 * @param time time in ms to reach target
 */
void brief_set_camera_target(vec3d *pos, matrix *orient, int time)
{
	float time_in_seconds;
	
	time_in_seconds = time / 1000.0f;

	if ( time == 0 ) {
		Current_cam_pos = *pos;
		Current_cam_orient = *orient;
	}

	Target_cam_pos = *pos;
	Target_cam_orient = *orient;
	Start_cam_orient = Current_cam_orient;
	Start_cam_pos = Current_cam_pos;								// we need this when checking if camera movement complete
	Start_cam_move = timer_get_milliseconds()*1000.0f;		// start time, convert to seconds
	Total_move_time = time_in_seconds;
	Elapsed_time = 0.0f;

	vm_vec_scale_add(&Start_lookat_pos, &Start_cam_pos, &Start_cam_orient.vec.fvec, LOOKAT_DIST);
	vm_vec_scale_add(&Target_lookat_pos, &Target_cam_pos, &Target_cam_orient.vec.fvec, LOOKAT_DIST);

	Play_highlight_flag = 1;								// once target reached, play highlight anims
	Cam_target_reached = 0;
	Cam_movement_done=0;
	anim_release_all_instances(ON_BRIEFING_SELECT);	// stop any briefing-specific anims
	
	// calculate camera velocity
	vm_vec_sub(&Cam_vel, pos, &Current_cam_pos);
	
	if ( !IS_VEC_NULL_SQ_SAFE(&Cam_vel) ) {
		vm_vec_normalize(&Cam_vel);
	}

	// calculate lookat point velocity
	vm_vec_sub(&Lookat_vel, &Target_lookat_pos, &Current_lookat_pos);
	vm_vec_scale(&Lookat_vel, 1.0f/time_in_seconds);

	Start_dist = vm_vec_dist(&Start_cam_pos, &Start_lookat_pos);
	End_dist = vm_vec_dist(&Target_cam_pos, &Target_lookat_pos);
	Dist_change_rate = (End_dist - Start_dist) / time_in_seconds;

	Total_dist=vm_vec_dist(&Start_cam_pos, &Target_cam_pos);

	Peak_speed=Total_dist/Total_move_time*2.0f;
	Cam_accel = 4*Total_dist/(Total_move_time*Total_move_time);
	Last_dist=0.0f;

	vm_vec_zero(&W_init);

	get_camera_limits(&Start_cam_orient, &Target_cam_orient, Total_move_time, &Acc_limit, &Vel_limit);
}


ubyte brief_return_color_index(char c)
{
	switch (c) {
		case 'f':
			return BRIEF_TEXT_FRIENDLY;

		case 'h':
			return BRIEF_TEXT_HOSTILE;

		case 'n':
			return BRIEF_TEXT_NEUTRAL;

		case 'r':
			return BRIEF_TEXT_RED;

		case 'g':
			return BRIEF_TEXT_GREEN;

		case 'b':
			return BRIEF_TEXT_BLUE;

//The following added by Zacam for expanded BRIEF colors
 		case 'w':
 			return BRIEF_TEXT_WHITE;

 		case 'y':
 			return BRIEF_TEXT_YELLOW;

		case 'W':
			return BRIEF_TEXT_BRIGHT_WHITE;

		case 'B':
			return BRIEF_TEXT_BRIGHT_BLUE;

		case 'G':
			return BRIEF_TEXT_BRIGHT_GREEN;

		case 'R':
			return BRIEF_TEXT_BRIGHT_RED;

		case 'Y':
			return BRIEF_TEXT_BRIGHT_YELLOW;

		case 'k':
			return BRIEF_TEXT_BLACK;

		case 'e':
			return BRIEF_TEXT_GREY;

		case 'E':
			return BRIEF_TEXT_SILVER;

		case 'v':
			return BRIEF_TEXT_VIOLET_GRAY;

		case 'V':
			return BRIEF_TEXT_VIOLET;

		case 'p':
			return BRIEF_TEXT_PINK;

		case 'P':
			return BRIEF_TEXT_LIGHT_PINK;

		case '|':	//This is not a duplicate, but a Non-Breaking Space case. Do not remove.
			return BRIEF_TEXT_WHITE;

		default:	//Zacam: Changed fron an Int3() in order to provide better feedback while still allowing play.
			Warning(LOCATION, "Unrecognized or undefined case character: '$%c' used in Briefing in mission: '%s'. Tell Zacam.", c, Mission_filename);
	} // end switch

	return BRIEF_TEXT_WHITE;
}

void brief_set_text_color(int color_index)
{
	Assert(color_index < MAX_BRIEF_TEXT_COLORS);
	gr_set_color_fast(Brief_text_colors[color_index]);
}

/**
 * Checks if a character is a word separator
 * @param character the character to be analysed.
 * @return true when the given character is a word separator, and false when the character is part of a word.
 */
bool is_a_word_separator(char character){
	return character<=33					//  2 characters including (space) and !
		|| (35<=character && character<=38)	//  4 characters #$%&
		|| (42<=character && character<=44)	//  3 characters *+,
		|| (character == 47)				//  1 character  /
		|| (59<=character && character<=64)	//  6 characters ;<=>?@
		|| (91<=character && character<=95)	//  5 characters [\]^_
		|| (123<=character);				//  5 characters {|}~
}

/**
 * Builds a vector of colored characters from a string containing color markups
 * and stores it to Colored_stream table.
 *
 * A color markup is made of a minimum of three characters: 
 *   '$' + a char standing for a color + contigous multiple spaces (chars t n and ' ')
 * The markup is completely removed from the resulting character sequence.
 *
 * @param src a not null pointer to a C string terminated by a /0 char.
 * @param instance index into Colored_stream where the result should be placed. Value is 0 unless multiple text streams are required.
 * @return number of character of the resulting sequence.
 */
int brief_text_colorize(char *src, int instance)
{
	Assert(src);
	Assert( 0<=instance && instance < (sizeof(Colored_stream)/sizeof(*Colored_stream)) );

	briefing_line dest_line; //the resulting vector of colored character
	ubyte active_color_index = BRIEF_TEXT_WHITE; //the current drawing color

	int src_len = strlen(src);
	for (int i=0; i<src_len; i++) {
		// Is the character a color markup?
		// text markup consists of a '$' plus a character plus an optional space
		if ( (i < src_len - 1)  && (src[i] == BRIEF_META_CHAR) ) {
			i++;   // Consume the $ character
			active_color_index = brief_return_color_index(src[i]);
			i++; // Consume the color identifier and focus on the white character (if any)
 
			// Skip every whitespace until the next word is reached
			while ( (i < src_len) && is_white_space(src[i]) )
				i++;

			//The next character is not a whitespace, let's process it as usual
			//(subtract 1 because the for loop will add it again)
			i--;
			continue;
 		}

		// When the word is terminated reset color to white
		if ( (is_white_space(src[i]) ) || ( is_a_word_separator(src[i]) )) {
			active_color_index = BRIEF_TEXT_WHITE;
		}

		// Append the character to the result structure
		colored_char dest;
		dest.letter = src[i];
		dest.color = active_color_index;
		dest_line.push_back(dest);
	} 
	Colored_stream[instance].push_back(dest_line); 	
	return dest_line.size();
}

/**
 * Initialise briefing coloured text
 *
 * @param src paragraph of text to process
 * @param w	max width of line in pixels
 * @param instance optional parameter, used when multiple text streams are required (default value is 0)
 * @param max_lines maximum number of lines
 */
int brief_color_text_init(const char* src, int w, int instance, int max_lines)
{
	int i, n_lines, len;
	SCP_vector<int> n_chars;
	SCP_vector<const char*> p_str;
	char brief_line[MAX_BRIEF_LINE_LEN];

	Assert(src != NULL);
	n_lines = split_str(src, w, &n_chars, &p_str, BRIEF_META_CHAR);
	Assert(n_lines >= 0);

	//for compatability reasons truncate text from everything except the fiction viewer
	if ((max_lines > 0) && (n_lines > max_lines)) {
		n_lines = max_lines; 
	}

	Max_briefing_line_len = 1;
	Colored_stream[instance].clear();
	for (i=0; i<n_lines; i++) {
		Assert(n_chars[i] < MAX_BRIEF_LINE_LEN);
		strncpy(brief_line, p_str[i], n_chars[i]);
		brief_line[n_chars[i]] = 0;
		drop_leading_white_space(brief_line);
		len = brief_text_colorize(&brief_line[0], instance);
		if (len > Max_briefing_line_len)
			Max_briefing_line_len = len;
	}

	Brief_text_wipe_time_elapsed = 0.0f;
	Play_brief_voice = 0;

	Num_brief_text_lines[instance] = n_lines;
	return n_lines;
}

/**
 * Get free handle to a move icon
 *
 * @return -1 on failure, a handle to a free move icon struct on success
 */
int brief_get_free_move_icon()
{
	int i;

	for ( i = 0; i < MAX_MOVE_ICONS; i++ ) {
		if ( Icon_movers[i].used == 0 )
			break;
	}
	
	if ( i == MAX_MOVE_ICONS ) 
		return -1;

	Icon_movers[i].used = 1;
	return i;
}

/**
 * Set move list in briefing 
 *
 * @param new_stage new stage number that briefing is now moving to
 * @param current_stage	current stage that the briefing is on
 * @param time	time in seconds
 */
int brief_set_move_list(int new_stage, int current_stage, float time)
{
	brief_stage		*newb, *cb;	
	icon_move_info	*imi;	
	int				i,j,k,num_movers,is_gone=0;
	vec3d			zero_v = ZERO_VECTOR;

	Assert(new_stage != current_stage);
	
	Assert( Briefing != NULL );
	newb = &Briefing->stages[new_stage];
	cb = &Briefing->stages[current_stage];
	num_movers = 0;
	
	for ( i = 0; i < cb->num_icons; i++ ) {
		is_gone=1;
		for ( j = 0; j < newb->num_icons; j++ ) {
			if ( ( cb->icons[i].id != 0 ) && ( cb->icons[i].id == newb->icons[j].id ) ) {
				is_gone=0;
				if ( vm_vec_cmp(&cb->icons[i].pos, &newb->icons[j].pos) ) {

					k = brief_get_free_move_icon();				
					if ( k == -1 ) {
						Int3();	// should never happen, get Alan
						return 0;
					}
					imi = &Icon_movers[k];
					imi->id = cb->icons[i].id;
					imi->start = cb->icons[i].pos;
					imi->finish = newb->icons[j].pos;
					imi->current = imi->start;
					list_append(&Icon_move_list, imi);

					imi->total_dist = vm_vec_dist(&imi->start, &imi->finish);
					imi->total_move_time = time;
					imi->peak_speed = imi->total_dist/imi->total_move_time*2.0f;
					imi->accel = 4*imi->total_dist/(time*time);
					imi->last_dist=0.0f;
					imi->reached_dest=0;
					imi->direction = zero_v;

					vm_vec_sub(&imi->direction, &imi->finish, &imi->start);
					if ( !IS_VEC_NULL_SQ_SAFE(&imi->direction) ) {
						vm_vec_normalize(&imi->direction);
					}

					num_movers++;
				}
			}
		}

		// Set up fading icon (to fade out)
		if (is_gone == 1) {
			if ( Num_fade_icons >= MAX_FADE_ICONS ) {
				Int3();
				Num_fade_icons=0;
			}

			int species = ship_get_species_by_type(cb->icons[i].ship_class);
			if(species < 0) {
				return 0;
			}

			Fading_icons[Num_fade_icons].fade_anim = Species_info[species].icon_fade_anims[cb->icons[i].type];
			Fading_icons[Num_fade_icons].pos = cb->icons[i].pos;
			Fading_icons[Num_fade_icons].team = cb->icons[i].team;
			Num_fade_icons++;
		}
	}

	// flag new icons for fading in
	for ( i=0; i<newb->num_icons; i++ ) {
		int is_new = 1;
		newb->icons[i].flags &= ~BI_FADEIN;
		for ( j=0; j<cb->num_icons; j++ ) {
			if ( ( cb->icons[j].id != 0 ) && ( cb->icons[j].id == newb->icons[i].id ) ) {
				is_new=0;
			}
		}
		if ( is_new ) {
			int species = ship_get_species_by_type(newb->icons[i].ship_class);
			if(species < 0) {
				return 0;
			}

			newb->icons[i].flags |= BI_FADEIN;
			newb->icons[i].fadein_anim = Species_info[species].icon_fade_anims[newb->icons[i].type];
			newb->icons[i].fadein_anim.time_elapsed = 0.0f;
		}
	}

	return num_movers;
}

void brief_clear_fade_out_icons()
{
	Num_fade_icons = 0;
}


/**
 * Set new stage in briefing
 *
 * @param pos target position for the camera
 * @param orient target orientation for the camera
 * @param time time in ms to reach target
 * @param stage_num	stage number of briefing (start numbering at 0)
 */
void brief_set_new_stage(vec3d *pos, matrix *orient, int time, int stage_num)
{
	char msg[MAX_BRIEF_LEN];
	int num_movers, new_time, not_objv = 1;

	Assert( Briefing != NULL );
	new_time = time;

	if (stage_num >= Briefing->num_stages) {
		not_objv = 0;  // turns out this is an objectives stage
		new_time = 0;
	}

	if ( stage_num == Last_new_stage ) {
		return;
	}

	num_movers = 0;
	brief_move_icon_reset();
	brief_clear_fade_out_icons();
	if ( (Last_new_stage != -1) && not_objv ) {
		num_movers = brief_set_move_list(stage_num, Last_new_stage, new_time / 1000.0f);
	}

	if ( (Last_new_stage != -1) && (num_movers == 0) && not_objv ) {
		if ( !vm_vec_cmp( &Briefing->stages[stage_num].camera_pos, &Briefing->stages[Last_new_stage].camera_pos) ) {
			if ( !vm_vec_cmp( &Briefing->stages[stage_num].camera_orient.vec.fvec, &Briefing->stages[Last_new_stage].camera_orient.vec.fvec) ){
				new_time = 0;
			}
		}
	}

	if (not_objv) {
		if(Briefing->stages[stage_num].new_text == NULL){
			strcpy_s(msg, "");
		} else {
			strcpy_s(msg, Briefing->stages[stage_num].new_text);
		}
	} else {
		strcpy_s(msg, XSTR( "Please review your objectives for this mission.", 395));
	}

	if (gr_screen.res == GR_640) {
		// GR_640
		Num_brief_text_lines[0] = brief_color_text_init(msg, MAX_BRIEF_LINE_W_640);
	} else {
		// GR_1024
		Num_brief_text_lines[0] = brief_color_text_init(msg, MAX_BRIEF_LINE_W_1024);		
	}

	if ( Brief_voices[stage_num] == -1 ) {
		fsspeech_play(FSSPEECH_FROM_BRIEFING, msg);
	}

	Top_brief_text_line = 0;

	if (not_objv){
		brief_set_camera_target(pos, orient, new_time);
	}

	if ( snd_is_playing(Brief_stage_highlight_sound_handle) ) {
		snd_stop(Brief_stage_highlight_sound_handle);
	}

	Brief_voice_ended = 0;
	Brief_textdraw_finished = 0;
	Brief_voice_started = 0;
	Brief_stage_time = 0;


	Brief_stage_highlight_sound_handle = -1;
	Last_new_stage = stage_num;
}

// ------------------------------------------------------------------------------------
// camera_pos_past_target()
//
//
int camera_pos_past_target(vec3d *start, vec3d *current, vec3d *dest)
{
	vec3d num, den;
	float ratio;

	vm_vec_sub(&num, current, start);
	vm_vec_sub(&den, start, dest);

	ratio = vm_vec_mag_quick(&num) / vm_vec_mag_quick(&den);
	if (ratio >= 1.0f)
		return TRUE;
	
	return FALSE;
}

/**
 * Interpolate between matrices.
 * elapsed_time/total_time gives percentage of interpolation between cur and goal.
 */
void interpolate_matrix(matrix *result, matrix *goal, matrix *start, float elapsed_time, float total_time)
{
	vec3d fvec, rvec;
	float	time0, time1;
	
	if ( !vm_matrix_cmp( goal, start ) ) {
		return;
	}	

	time0 = elapsed_time / total_time;
	time1 = (total_time - elapsed_time) / total_time;

	vm_vec_copy_scale(&fvec, &start->vec.fvec, time1);
	vm_vec_scale_add2(&fvec, &goal->vec.fvec, time0);

	vm_vec_copy_scale(&rvec, &start->vec.rvec, time1);
	vm_vec_scale_add2(&rvec, &goal->vec.rvec, time0);

	vm_vector_2_matrix(result, &fvec, NULL, &rvec);
 }

/**
 * Calculate how far the camera should have moved
 */
float brief_camera_get_dist_moved(float elapsed_time)
{
	float time, dist_moved=0.0f;
	
	// first half of movement
	if ( elapsed_time < Total_move_time/2.0f ) {
		dist_moved=0.5f*Cam_accel*elapsed_time*elapsed_time;	// d = 1/2at^2
		return dist_moved;
	}

	// second half of movement
	time=elapsed_time - Total_move_time/2.0f;
	dist_moved=(Total_dist/2.0f)+(Peak_speed*time) - 0.5f*Cam_accel*time*time;
	return dist_moved;

}

/**
 * Update the camera position
 */
void brief_camera_move(float frametime, int stage_num)
{
	vec3d	dist_moved;
	float		dist;
	vec3d	w_out;
	matrix	result;

	Elapsed_time += frametime;

	if ( Cam_target_reached )
		return;

	// Update orientation
	if ( (Elapsed_time < Total_move_time) ) {
		vm_matrix_interpolate(&Target_cam_orient, &Current_cam_orient, &W_init, frametime, &result, &w_out, &Vel_limit, &Acc_limit);
		Current_cam_orient = result;
		W_init = w_out;
	}

	// use absolute pos to update position
	if ( vm_vec_cmp( &Current_cam_pos, &Target_cam_pos ) ) {
		dist = brief_camera_get_dist_moved(Elapsed_time);
		if ( dist < Last_dist ) {
			Cam_movement_done=1;
			Last_dist=0.0f;
		}
		Last_dist=dist;

		if ( Cam_movement_done == 0 ) {
			vm_vec_copy_scale(&dist_moved, &Cam_vel, dist);
			vm_vec_add(&Current_cam_pos, &Start_cam_pos, &dist_moved);
		} else {
			Current_cam_pos=Target_cam_pos;
		}
	}
	else {
		Cam_movement_done=1;
		Current_cam_pos=Target_cam_pos;
	}

	if ( Cam_movement_done && (Elapsed_time >= Total_move_time) ) {
		Cam_target_reached=1;
	}
}

/**
 * Project the viewer's position onto the grid plane.
 * If more than threshold distance from grid center, move grid center.
 */
void brief_maybe_create_new_grid(grid* gridp, vec3d *pos, matrix *orient, int force)
{
	int roundoff;
	plane	tplane;
	vec3d	gpos, tmp, c;
	float	dist_to_plane;
	float	square_size, ux, uy, uz;

	ux = tplane.A = gridp->gmatrix.vec.uvec.xyz.x;
	uy = tplane.B = gridp->gmatrix.vec.uvec.xyz.y;
	uz = tplane.C = gridp->gmatrix.vec.uvec.xyz.z;
	tplane.D = gridp->planeD;

	compute_point_on_plane(&c, &tplane, pos);
	dist_to_plane = fl_abs(vm_dist_to_plane(pos, &gridp->gmatrix.vec.uvec, &c));
	square_size = 1.0f;

	while (dist_to_plane >= 25.0f)
	{
		square_size *= 10.0f;
		dist_to_plane /= 10.0f;
	}
	
	if (fvi_ray_plane(&gpos, &gridp->center, &gridp->gmatrix.vec.uvec, pos, &orient->vec.fvec, 0.0f)<0.0f)	{
		vec3d p;
		vm_vec_scale_add(&p,pos,&orient->vec.fvec, 100.0f );
		compute_point_on_plane(&gpos, &tplane, &p );
	}

	if (vm_vec_dist(&gpos, &c) > 50.0f * square_size)
	{
		vm_vec_sub(&tmp, &gpos, &c);
		vm_vec_normalize(&tmp);
		vm_vec_scale_add(&gpos, &c, &tmp, 50.0f * square_size);
	}

	roundoff = (int) square_size * 10;
	if (!ux)
		gpos.xyz.x = fl_roundoff(gpos.xyz.x, roundoff);
	if (!uy)
		gpos.xyz.y = fl_roundoff(gpos.xyz.y, roundoff);
	if (!uz)
		gpos.xyz.z = fl_roundoff(gpos.xyz.z, roundoff);

	if ((square_size != gridp->square_size) ||
		(gpos.xyz.x != gridp->center.xyz.x) ||
		(gpos.xyz.y != gridp->center.xyz.y) ||
		(gpos.xyz.z != gridp->center.xyz.z) || force)
	{
		gridp->square_size = square_size;
		gridp->center = gpos;
		brief_modify_grid(gridp);
	}
}

//	Create a grid
//	*forward is vector pointing forward
//	*right is vector pointing right
//	*center is center point of grid
//	length is length of grid
//	width is width of grid
//	square_size is size of a grid square
//	For example:
//		*forward = (0.0, 0.0, 1.0)
//		*right   = (1.0, 0.0, 0.0)
//		*center = (0.0, 0.0, 0.0)
//		nrows = 10
//		ncols =  50.0
//		square_size = 10.0
//	will generate a grid of squares 10 long by 5 wide.
//	Each grid square will be 10.0 x 10.0 units.
//	The center of the grid will be at the global origin.
//	The grid will be parallel to the xz plane (because the normal is 0,1,0).
//	(In fact, it will be the xz plane because it is centered on the origin.)
//
//	Stuffs grid in *gridp.  If gridp == NULL, mallocs and returns a grid.
grid *brief_create_grid(grid *gridp, vec3d *forward, vec3d *right, vec3d *center, int nrows, int ncols, float square_size)
{
	int	i, ncols2, nrows2, d = 1;
	vec3d	dfvec, drvec, cur, cur2, tvec, uvec, save, save2;

	Assert(square_size > 0.0);
	if (double_fine_gridlines)
		d = 2;

	if (gridp == NULL)
		gridp = (grid *) vm_malloc(sizeof(grid));

	Assert(gridp);

	gridp->center = *center;
	gridp->square_size = square_size;

	//	Create the plane equation.
	Assert(!IS_VEC_NULL(forward));
	Assert(!IS_VEC_NULL(right));

	vm_vec_copy_normalize(&dfvec, forward);
	vm_vec_copy_normalize(&drvec, right);

	vm_vec_cross(&uvec, &dfvec, &drvec);
	
	Assert(!IS_VEC_NULL(&uvec));

	gridp->gmatrix.vec.uvec = uvec;

	gridp->planeD = -(center->xyz.x * uvec.xyz.x + center->xyz.y * uvec.xyz.y + center->xyz.z * uvec.xyz.z);
	Assert(!_isnan(gridp->planeD));

	gridp->gmatrix.vec.fvec = dfvec;
	gridp->gmatrix.vec.rvec = drvec;

	vm_vec_scale(&dfvec, square_size);
	vm_vec_scale(&drvec, square_size);

	vm_vec_scale_add(&cur, center, &dfvec, (float) -nrows * d / 2);
	vm_vec_scale_add2(&cur, &drvec, (float) -ncols * d / 2);
	vm_vec_scale_add(&cur2, center, &dfvec, (float) -nrows * 5 / 2);
	vm_vec_scale_add2(&cur2, &drvec, (float) -ncols * 5 / 2);
	save = cur;
	save2 = cur2;

	gridp->ncols = ncols;
	gridp->nrows = nrows;
	ncols2 = ncols / 2;
	nrows2 = nrows / 2;
	Assert(ncols < MAX_GRIDLINE_POINTS && nrows < MAX_GRIDLINE_POINTS);

	// Create the points along the edges of the grid, so we can just draw lines
	// between them to form the grid.  
	for (i=0; i<=ncols*d; i++) {
		gridp->gpoints1[i] = cur;  // small, dark gridline points
		vm_vec_scale_add(&tvec, &cur, &dfvec, (float) nrows * d);
		gridp->gpoints2[i] = tvec;
		vm_vec_add2(&cur, &drvec);
	}

	for (i=0; i<=ncols2; i++) {
		gridp->gpoints5[i] = cur2;  // large, brighter gridline points
		vm_vec_scale_add(&tvec, &cur2, &dfvec, (float) nrows2 * 10);
		gridp->gpoints6[i] = tvec;
		vm_vec_scale_add2(&cur2, &drvec, 10.0f);
	}

	cur = save;
	cur2 = save2;
	for (i=0; i<=nrows*d; i++) {
		gridp->gpoints3[i] = cur;  // small, dark gridline points
		vm_vec_scale_add(&tvec, &cur, &drvec, (float) ncols * d);
		gridp->gpoints4[i] = tvec;
		vm_vec_add2(&cur, &dfvec);
	}

	for (i=0; i<=nrows2; i++) {
		gridp->gpoints7[i] = cur2;  // large, brighter gridline points
		vm_vec_scale_add(&tvec, &cur2, &drvec, (float) ncols2 * 10);
		gridp->gpoints8[i] = tvec;
		vm_vec_scale_add2(&cur2, &dfvec, 10.0f);
	}

	return gridp;
}

/**
 * Create a nice default grid.
 * Centered at origin, 10x10, 10.0 size squares, in xz plane.
 */
grid *brief_create_default_grid(void)
{
	grid	*rgrid;
	vec3d	fvec, rvec, cvec;

	vm_vec_make(&fvec, 0.0f, 0.0f, 1.0f);
	vm_vec_make(&rvec, 1.0f, 0.0f, 0.0f);
	vm_vec_make(&cvec, 0.0f, -10.0f, 0.0f);

	rgrid = brief_create_grid(&Global_grid, &fvec, &rvec, &cvec, 100, 100, 5.0f);

	physics_init(&rgrid->physics);
	rgrid->physics.flags |= (PF_ACCELERATES | PF_SLIDE_ENABLED);
	return rgrid;
}

/**
 * Rotate and project points and draw a line.
 */
void brief_rpd_line(vec3d *v0, vec3d *v1)
{
	vertex	tv0, tv1;
	g3_rotate_vertex(&tv0, v0);
	g3_rotate_vertex(&tv1, v1);

	gr_set_color_fast(&Color_grey);
	g3_draw_line(&tv0, &tv1);
}

/**
 * Renders a grid 
 *
 * @param gridp Grid defined in a grid struct to render
 */
void brief_render_grid(grid *gridp)
{
	int	i, ncols, nrows;

	ncols = gridp->ncols;
	nrows = gridp->nrows;
	if (double_fine_gridlines)
	{
		ncols *= 2;
		nrows *= 2;
	}

	gr_set_color(30,30,30);

	//	Draw the column lines.
	for (i=0; i<=ncols; i++)
		brief_rpd_line(&gridp->gpoints1[i], &gridp->gpoints2[i]);

	//	Draw the row lines.
	for (i=0; i<=nrows; i++)
		brief_rpd_line(&gridp->gpoints3[i], &gridp->gpoints4[i]);

	ncols = gridp->ncols / 2;
	nrows = gridp->nrows / 2;
}

void brief_modify_grid(grid *gridp)
{
	brief_create_grid(gridp, &gridp->gmatrix.vec.fvec, &gridp->gmatrix.vec.rvec, &gridp->center,
		gridp->nrows, gridp->ncols, gridp->square_size);
}

void brief_unload_anims()
{
	int icon, species;
	species_info *spinfo;
	
	for (species = 0; species < (int)Species_info.size(); species++)
	{
		spinfo = &Species_info[species];

		for (icon=0; icon<MAX_BRIEF_ICONS; icon++)
		{
			if (spinfo->icon_bitmaps[icon].first_frame >= 0)
			{
				bm_unload(spinfo->icon_bitmaps[icon].first_frame);
				spinfo->icon_bitmaps[icon].first_frame = -1;
			}

			if (spinfo->icon_fade_anims[icon].first_frame >= 0)
			{
				bm_unload(spinfo->icon_fade_anims[icon].first_frame);
				spinfo->icon_fade_anims[icon].first_frame = -1;
			}

			if (spinfo->icon_highlight_anims[icon].first_frame >= 0)
			{
				bm_unload(spinfo->icon_highlight_anims[icon].first_frame);
				spinfo->icon_highlight_anims[icon].first_frame = -1;
			}
		}
	}
}

void brief_common_close()
{
	brief_unload_anims();
}

void brief_restart_text_wipe()
{
	Brief_stage_time = 0;
	Brief_voice_ended = 0;
	Brief_voice_started = 0;
	Brief_text_wipe_time_elapsed = 0.0f;
}

/**
 * Initialize the array of handles to the different voice streams
 */
void brief_voice_init()
{
	int i;
	for ( i = 0; i < MAX_BRIEF_STAGES; i++ ) {
		Brief_voices[i] = -1;
	}
}

void brief_load_voice_file(int voice_num, char *name)
{
	int load_attempts = 0;
	while(1) {

		if ( load_attempts++ > 5 ) {
			break;
		}

		Brief_voices[voice_num] = audiostream_open( name, ASF_VOICE );
		if ( Brief_voices[voice_num] >= 0 ) {
			break;
		}

		// Don't bother to ask for the CD in multiplayer
		if ( Game_mode & GM_MULTIPLAYER ) {
			break;
		}
	}
}

/**
 * Open and pre-load the stream buffers for the different voice streams
 */
void brief_voice_load_all()
{
	int			i;
	brief_stage	*bs;

	Assert( Briefing != NULL );
	for ( i = 0; i < Briefing->num_stages; i++ ) {
		bs = &Briefing->stages[i];
		if ( strnicmp(bs->voice, NOX("none"), 4) ) {
			brief_load_voice_file(i, bs->voice);
		}
	}
}

/**
 * Close all the briefing voice streams
 */
void brief_voice_unload_all()
{
	int i;

	for ( i = 0; i < MAX_BRIEF_STAGES; i++ ) {
		if ( Brief_voices[i] != -1 ) {
			audiostream_close_file(Brief_voices[i], 0);
			Brief_voices[i] = -1;
		}
	}
}

/**
 * Start playback of the voice for a particular briefing stage
 */
void brief_voice_play(int stage_num)
{
	if ( Brief_voices[stage_num] == -1 )
		return;	// voice file doesn't exist

	if ( !Briefing_voice_enabled ) {
		return;
	}

	if ( audiostream_is_playing( Brief_voices[stage_num]) )
		return;

	audiostream_play(Brief_voices[stage_num], Master_voice_volume, 0);
	Brief_voice_started = 1;
}

/**
 * Stop playback of the voice for a particular briefing stage
 */
void brief_voice_stop(int stage_num)
{
	if ( Brief_voices[stage_num] == -1 )
		return;

	audiostream_stop(Brief_voices[stage_num], 1, 0);	// stream is automatically rewound
}

/**
 * Pause playback of the voice for a particular briefing stage, to resume just
 * call brief_voice_unpause() again
 */
void brief_voice_pause(int stage_num)
{
	if ( Brief_voices[stage_num] == -1 )
		return;

	audiostream_pause(Brief_voices[stage_num]);
}

void brief_voice_unpause(int stage_num)
{
	if ( Brief_voices[stage_num] == -1 )
		return;

	audiostream_unpause(Brief_voices[stage_num]);
}

void brief_reset_last_new_stage()
{
	Last_new_stage = -1;
}

/**
 * Get the dimensions for a briefing icon
 */
void brief_common_get_icon_dimensions(int *w, int *h, int type, int ship_class)
{
	Assert(type >= 0 && type < MAX_BRIEF_ICONS);

	// in case anything goes wrong
	*w=0;
	*h=0;

	int species = ship_get_species_by_type(ship_class);
	if(species < 0){
		return;
	}

	if (Species_info[species].icon_bitmaps[type].first_frame >= 0 ) {
		bm_get_info(Species_info[species].icon_bitmaps[type].first_frame, w, h, NULL);
	} else {
		*w=0;
		*h=0;
	}
}

void cmd_brief_reset()
{
	int i, j;
	static int inited = 0;

	if (inited) {
		for (i=0; i<MAX_TVT_TEAMS; i++) {
			for (j=0; j<Cmd_briefs[i].num_stages; j++) {
				if (Cmd_briefs[i].stage[j].text)
					vm_free(Cmd_briefs[i].stage[j].text);
			}
		}
	}

	inited = 1;
	for (i=0; i<MAX_TVT_TEAMS; i++)
		Cmd_briefs[i].num_stages = 0;
}

/**
 * Should briefing advance to the next stage?
 */
int brief_time_to_advance(int stage_num, float frametime)
{
	int voice_active, advance = 0;
	brief_icon *closeup_icon;

	closeup_icon = brief_get_closeup_icon();
	if ( closeup_icon ) {
		return 0;
	}

	if ( !Player->auto_advance ) {
		return 0;
	}

	Brief_stage_time += fl2i(frametime*1000 + 0.5f);

	// we do this after the stage time gets set so that we can continue the voice
	// and current stage rather than jumping to the next
	if (Briefing_paused)
		return 0;

	if ( (Brief_voices[stage_num] >= 0) && Briefing_voice_enabled ) {
		voice_active = 1;
	} else {
		voice_active = 0;
	}

	if ( voice_active && (Brief_voice_ended == 0) && Brief_voice_started) {
		if ( !audiostream_is_playing( Brief_voices[stage_num]) ) {
			Brief_voice_ended = 1;
			Brief_stage_time = 0;
		}
	}
	
	if ( Brief_voice_ended ) {
		if ( Brief_stage_time > STAGE_ADVANCE_DELAY ) {
			advance = 1;
		}
	}

	if ( !voice_active && (Brief_textdraw_finished > 0) ) {
		if ( Brief_stage_time > MAX(5000, Num_brief_text_lines[0] * 3500) ) {
			advance = 1;
		}
	}

	return advance;
}
