/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionBriefCommon.cpp $
 * $Revision: 2.51 $
 * $Date: 2006-09-11 06:50:42 $
 * $Author: taylor $
 *
 * C module for briefing code common to FreeSpace and FRED
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.50  2006/09/11 06:08:09  taylor
 * make Species_info[] and Asteroid_info[] dynamic
 *
 * Revision 2.49  2006/01/13 03:31:09  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 2.48  2005/12/29 00:49:43  phreak
 * Briefing icons can now be mirrored so one can point them in the opposite direction.
 *
 * Revision 2.47  2005/11/21 23:55:57  taylor
 * move generic_* functions to their own file
 *
 * Revision 2.46  2005/11/08 01:04:00  wmcoolmon
 * More warnings instead of Int3s/Asserts, better Lua scripting, weapons_expl.tbl is no longer needed nor read, added "$Disarmed ImpactSnd:", fire-beam fix
 *
 * Revision 2.45  2005/10/24 12:42:13  taylor
 * init thruster stuff properly so that bmpman doesn't have a fit
 *
 * Revision 2.44  2005/09/27 02:36:57  Goober5000
 * clarification
 * --Goober5000
 *
 * Revision 2.43  2005/09/26 06:00:58  Goober5000
 * this should fix the rest of the briefing icon bugs
 * --Goober5000
 *
 * Revision 2.42  2005/09/26 05:24:25  Goober5000
 * small fix
 * --Goober5000
 *
 * Revision 2.41  2005/09/26 04:53:19  Goober5000
 * moved these per taylor's recommendation
 * --Goober5000
 *
 * Revision 2.40  2005/09/26 04:51:00  Goober5000
 * some more cleanup
 * --Goober5000
 *
 * Revision 2.39  2005/09/26 04:08:54  Goober5000
 * some more cleanup
 * --Goober5000
 *
 * Revision 2.38  2005/09/26 02:15:03  Goober5000
 * okay, this should all be working :)
 * --Goober5000
 *
 * Revision 2.37  2005/09/25 23:02:29  Goober5000
 * and again, gah
 * --Goober5000
 *
 * Revision 2.36  2005/09/25 22:24:22  Goober5000
 * more fiddly stuff
 * --Goober5000
 *
 * Revision 2.35  2005/09/25 21:00:40  taylor
 * gah.
 *
 * Revision 2.34  2005/09/25 07:07:34  Goober5000
 * partial commit; hang on
 * --Goober5000
 *
 * Revision 2.33  2005/09/25 05:13:07  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.32  2005/09/24 07:07:16  Goober5000
 * another species overhaul
 * --Goober5000
 *
 * Revision 2.31  2005/07/18 03:45:07  taylor
 * more non-standard res fixing
 *  - I think everything should default to resize now (much easier than having to figure that crap out)
 *  - new mouse_get_pos_unscaled() function to return 1024x768/640x480 relative values so we don't have to do it later
 *  - lots of little cleanups which fix several strange offset/size problems
 *  - fix gr_resize/unsize_screen_pos() so that it won't wrap on int (took too long to track this down)
 *
 * Revision 2.30  2005/07/15 03:48:40  Goober5000
 * fixed a typo of Kazan's dating all the way back to October 15, 2003 (!!) that
 * created a nasty stealth bug
 * --Goober5000
 *
 * Revision 2.29  2005/07/13 03:25:59  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.28  2005/07/13 00:44:22  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.27  2005/07/02 19:43:54  taylor
 * ton of non-standard resolution fixes
 *
 * Revision 2.26  2005/06/03 06:39:27  taylor
 * better audio pause/unpause support when game window loses focus or is minimized
 *
 * Revision 2.25  2005/05/30 05:33:11  taylor
 * if a briefing does have any stages (no text) then don't carry over text from a previous mission
 * some generic cleanup to V code that isn't a real problem now but could easily have been a memleak
 *
 * Revision 2.24  2005/05/12 17:49:13  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.23  2005/04/22 00:34:55  wmcoolmon
 * Minor updates to the GUI, and added some code that will (hopefully) resize HUD images in nonstandard resolutions. I couldn't test it; got an out of memory error.
 *
 * Revision 2.22  2005/04/12 05:26:36  taylor
 * many, many compiler warning and header fixes (Jens Granseuer)
 * fix free on possible NULL in modelinterp.cpp (Jens Granseuer)
 *
 * Revision 2.21  2005/04/05 05:53:19  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.20  2005/03/10 08:00:08  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.19  2005/03/06 11:23:45  wmcoolmon
 * RE-fixed stuff. Ogg support. Briefings.
 *
 * Revision 2.18  2005/03/01 06:55:41  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.17  2005/02/23 04:55:07  taylor
 * more bm_unload() -> bm_release() changes
 *
 * Revision 2.16  2005/02/21 09:00:17  wmcoolmon
 * Multi-res support
 *
 * Revision 2.15  2005/01/31 23:27:54  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 2.14  2004/07/31 08:53:46  et1
 * Make missing briefing icon message to tell the icon missing
 *
 * Revision 2.13  2004/07/26 20:47:37  Kazan
 * remove MCD complete
 *
 * Revision 2.12  2004/07/17 18:46:08  taylor
 * various OGL and memory leak fixes
 *
 * Revision 2.11  2004/07/12 16:32:53  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.10  2004/04/26 01:41:52  taylor
 * s/idex/idx/ for brief_unload_anims()
 *
 * Revision 2.9  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.8  2004/02/14 00:18:33  randomtiger
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
 * Revision 2.7  2004/02/13 04:17:13  randomtiger
 * Turned off fog in OGL for Fred.
 * Simulated speech doesnt say tags marked by $ now.
 * The following are fixes to issues that came up testing TBP in fs2_open and fred2_open:
 * Changed vm_vec_mag and parse_tmap to fail gracefully on bad data.
 * Error now given on missing briefing icon and bad ship normal data.
 * Solved more species divide by zero error.
 * Fixed neb cube crash.
 *
 * Revision 2.6  2004/01/24 12:47:48  randomtiger
 * Font and other small changes for Fred
 *
 * Revision 2.5  2003/11/16 04:08:47  Goober5000
 * fixed briefing scroll and display of "more"
 * --Goober5000
 *
 * Revision 2.4  2003/10/15 22:03:25  Kazan
 * Da Species Update :D
 *
 * Revision 2.3  2003/09/07 18:14:53  randomtiger
 * Checked in new speech code and calls from relevent modules to make it play.
 * Should all work now if setup properly with version 2.4 of the launcher.
 * FS2_SPEECH can be used to make the speech code compile if you have SAPI 5.1 SDK installed.
 * Otherwise the compile flag should not be set and it should all compile OK.
 *
 * - RT
 *
 * Revision 2.2  2003/04/05 11:09:21  Goober5000
 * fixed some fiddly bits with scrolling and ui stuff
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/09 23:02:57  mharris
 * Not using default values for audiostream functions, since they may
 * be macros (if NO_SOUND is defined)
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 19    11/02/99 3:23p Jefff
 * translate briefing icon names
 * 
 * 18    9/09/99 9:44a Jefff
 * doh, fixed reversed brief text color thingy.  i am stoopid.
 * 
 * 17    9/08/99 11:14a Jefff
 * toned down hostile/friendly colors in briefing text
 * 
 * 16    9/07/99 12:20p Mikeb
 * return pos of briefing icon even it does not fit on screen.
 * 
 * 15    9/03/99 1:32a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 14    8/10/99 7:28p Jefff
 * shuffled some text around
 * 
 * 13    7/30/99 3:05p Jefff
 * Fixed briefing icon fades -- in and out were reversed.
 * 
 * 12    7/26/99 1:52p Mikeb
 * Fixed strange briefing bug where a NULL wasn't being checked for when
 * copying briefing stage text. Odd.
 * 
 * 11    7/24/99 6:15p Jefff
 * moved "stage x of y" text in multiplayer mode so its not covered by the
 * chatbox
 * 
 * 10    7/20/99 7:09p Jefff
 * briefing text occupies full window in 1024x768
 * 
 * 9     7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 8     6/29/99 7:39p Dave
 * Lots of small bug fixes.
 * 
 * 7     2/05/99 7:19p Neilk
 * Removed black part from mission screen, fixed info text coords
 * 
 * 6     1/29/99 4:17p Dave
 * New interface screens.
 * 
 * 5     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 4     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
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
 * 122   6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 121   6/05/98 9:54a Lawrance
 * OEM changes
 * 
 * 120   6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 119   5/23/98 10:38p Lawrance
 * Avoid doing a cfile refresh when running debug
 * 
 * 118   5/23/98 6:49p Lawrance
 * Fix problems with refreshing the file list when a CD is inserted
 * 
 * 117   5/21/98 6:57p Lawrance
 * Don't prompt for the CD if voice not found
 * 
 * 116   5/21/98 12:35a Lawrance
 * Tweak how CD is checked for
 * 
 * 115   5/12/98 11:46a John
 * Changed the way the "glowing movement" type text draw.   Use Hoffoss's
 * gr_get_string_size optional length parameter to determine length of
 * string which accounts for kerning on the last character and then I only
 * draw each character only once.
 * 
 * 114   5/08/98 5:32p Lawrance
 * prompt for CD if can't load animations or voice
 * 
 * 113   5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 112   4/27/98 9:08p Allender
 * fix the debriefing stage problems when clients get to screen long after
 * server
 * 
 * 111   4/25/98 3:49p Lawrance
 * Save briefing auto-advance pref
 * 
 * 110   4/20/98 3:53p Lawrance
 * Fix various bugs with auto-advancing through briefings.
 * 
 * $NoKeywords: $
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

// static int Brief_voice_ask_for_cd;

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
static char		Brief_text[MAX_BRIEF_LINES][MAX_BRIEF_LINE_LEN];

// Used to support drawing colored text for the briefing.  Gets complicates since we
// need to be able to draw one character at a time as well when the briefing text
// first appears.
typedef struct colored_char
{
	char	letter;
	ubyte	color;		// index into Brief_text_colors[]
} colored_char;

static colored_char Colored_text[MAX_TEXT_STREAMS][MAX_BRIEF_LINES][MAX_BRIEF_LINE_LEN];
static int Colored_text_len[MAX_TEXT_STREAMS][MAX_BRIEF_LINES];

#define MAX_BRIEF_TEXT_COLORS			9
#define BRIEF_TEXT_WHITE				0
#define BRIEF_TEXT_BRIGHT_WHITE		1
#define BRIEF_TEXT_RED					2
#define BRIEF_TEXT_GREEN				3
#define BRIEF_TEXT_YELLOW				4
#define BRIEF_TEXT_BLUE					5
#define BRIEF_TEXT_FRIENDLY				6
#define BRIEF_TEXT_HOSTILE				7
#define BRIEF_TEXT_NEUTRAL				8

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
		Error(LOCATION, "Unable to parse icons.tbl!  Code = %i.\n", rval);
	}
	else {
		read_file_text("icons.tbl");
		reset_parse();		
	}

	required_string("#Start");

	num_icons = 0;
	while (required_string_either("#End","$Name:"))
	{
		Verify( num_icons < max_icons );

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


// --------------------------------------------------------------------------------------
// Does one time initialization of the briefing and debriefing structures.
// Namely setting all malloc'ble pointers to NULL.  Called once at game startup.
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
		for (i=0; i<MAX_TVT_TEAMS; i++ )	{
			for (j=0; j<MAX_BRIEF_STAGES; j++ )	{
				Briefings[i].stages[j].new_text = (char *)vm_malloc(MAX_BRIEF_LEN);
				Assert(Briefings[i].stages[j].new_text!=NULL);
				Briefings[i].stages[j].icons = (brief_icon *)vm_malloc(sizeof(brief_icon)*MAX_STAGE_ICONS);
				Assert(Briefings[i].stages[j].icons!=NULL);
				Briefings[i].stages[j].lines = (brief_line *)vm_malloc(sizeof(brief_line)*MAX_BRIEF_STAGE_LINES);
				Assert(Briefings[i].stages[j].lines!=NULL);
				Briefings[i].stages[j].num_icons = 0;
				Briefings[i].stages[j].num_lines = 0;
			}
		}

		for (i=0; i<MAX_TVT_TEAMS; i++ )	{
			for (j=0; j<MAX_DEBRIEF_STAGES; j++ )	{
				Debriefings[i].stages[j].new_text = (char *)vm_malloc(MAX_DEBRIEF_LEN);
				Assert(Debriefings[i].stages[j].new_text!=NULL);
				Debriefings[i].stages[j].new_recommendation_text = (char *)vm_malloc(MAX_RECOMMENDATION_LEN);
				Assert(Debriefings[i].stages[j].new_recommendation_text!=NULL);
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

//--------------------------------------------------------------------------------------
// Frees all the memory allocated in the briefing and debriefing structures
// and sets all pointers to NULL.
void mission_brief_common_reset()
{
	int i,j;

	if ( Fred_running )	{
		return;						// Don't free these under Fred.
	}

	for (i=0; i<MAX_TVT_TEAMS; i++ )	{
		for (j=0; j<MAX_BRIEF_STAGES; j++ )	{
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

// split from above since we need to clear them separately - taylor
void mission_debrief_common_reset()
{
	int i, j;

	if ( Fred_running ) {
		return;						// Don't free these under Fred.
	}

	for (i=0; i<MAX_TVT_TEAMS; i++ )	{
		for (j=0; j<MAX_DEBRIEF_STAGES; j++ )	{
			if ( Debriefings[i].stages[j].new_text )	{
				vm_free(Debriefings[i].stages[j].new_text);
				Debriefings[i].stages[j].new_text = NULL;
			}

			if ( Debriefings[i].stages[j].new_recommendation_text )	{
				vm_free(Debriefings[i].stages[j].new_recommendation_text);
				Debriefings[i].stages[j].new_recommendation_text = NULL;
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
	int i;

	Briefing = NULL;
	for ( i = 0; i < MAX_TVT_TEAMS; i++ ) 
		Briefings[i].num_stages = 0;
	Cur_brief_id = 1;
}

// --------------------------------------------------------------------------------------
//	debrief_reset()
//
//
void debrief_reset()
{
	int i;

	Debriefing = NULL;
	for ( i = 0; i < MAX_TVT_TEAMS; i++ ) {
		Debriefings[i].num_stages = 0;
	}

	// MWA 4/27/98 -- must initialize this variable here since we cannot do it as debrief
	// init time because race conditions between all players in the game make that type of
	// initialization unsafe.
	Debrief_multi_stages_loaded = 0;
}

// --------------------------------------------------------------------------------------
//	brief_init_screen()
//
//	Set up the screen regions.  A mulitplayer briefing will look different than a single player
// briefing.
//
void brief_init_screen(int multiplayer_flag)
{
	bscreen.map_x1			= Brief_grid_coords[gr_screen.res][0];
	bscreen.map_x2			= Brief_grid_coords[gr_screen.res][0] + Brief_grid_coords[gr_screen.res][2];
	bscreen.map_y1			= Brief_grid_coords[gr_screen.res][1];
	bscreen.map_y2			= Brief_grid_coords[gr_screen.res][1] + Brief_grid_coords[gr_screen.res][3];
	/*
	bscreen.map_x1			= BRIEF_GRID3_X1;
	bscreen.map_x2			= BRIEF_GRID0_X2;
	bscreen.map_y1			= BRIEF_GRID3_Y1;
	bscreen.map_y2			= BRIEF_GRID0_Y2+4;
	bscreen.btext_x1		= BRIEF_TEXT_X1;
	bscreen.btext_x2		= BRIEF_TEXT_X2;
	bscreen.btext_y1		= BRIEF_TEXT_Y1;
	bscreen.btext_y2		= BRIEF_TEXT_Y2;
	bscreen.cup_x1			= BRIEF_CUP_X1;
	bscreen.cup_y1			= BRIEF_CUP_Y1;
	bscreen.cup_x2			= BRIEF_CUP_X2;
	bscreen.cup_y2			= BRIEF_CUP_Y2;
	bscreen.cupinfo_x1	= BRIEF_CUPINFO_X1;
	bscreen.cupinfo_y1	= BRIEF_CUPINFO_Y1;
	bscreen.cupinfo_x2	= BRIEF_CUPINFO_X2;
	bscreen.cupinfo_y2	= BRIEF_CUPINFO_Y2;
	*/
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

// preload highlight, fadein and fadeout animations that are used in each stage
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

// render fade-out anim frame
//static int Fade_frame_count[128];			// for debug

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

			bxf = tv.sx - w / 2.0f + 0.5f;
			byf = tv.sy - h / 2.0f + 0.5f;
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

// figure out how far an icon should move based on the elapsed time
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

// Draw a line between two icons on the briefing screen
void brief_render_icon_line(int stage_num, int line_num)
{
	brief_line	*bl;
	brief_icon	*icon[2];
	vertex		icon_vertex[2];
	int			icon_status[2] = {0,0};
	int			icon_w, icon_h, i;
	float			icon_x[2], icon_y[2];

	bl = &Briefing->stages[stage_num].lines[line_num];
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
		icon_x[i] = icon_vertex[i].sx;
		icon_y[i] = icon_vertex[i].sy;
	}

	brief_set_icon_color(icon[0]->team);

	gr_line(fl2i(icon_x[0]), fl2i(icon_y[0]), fl2i(icon_x[1]), fl2i(icon_y[1]), false);
}

// -------------------------------------------------------------------------------------
// Draw a briefing icon
//
// parameters:		stage_num		=>		briefing stage number (start at 0)
//						icon_num			=>		icon number in stage
//						frametime		=>		time elapsed in seconds
//						selected			=>		FRED only (will be 0 or non-zero)
//						w_scale_factor	=>		scale icon in width by this amount (default 1.0f)
//						h_scale_factor	=>		scale icon in height by this amount (default 1.0f)
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

		float sx = tv.sx;
		float sy = tv.sy;
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
				if ( strlen(bi->label) > 0 ) {
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
					strcpy(buf, bi->label);
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

// Render a line of text for the briefings.  Lines are drawn in as a wipe, with leading bright
// white characters.  Have to jump through some hoops since we support colored words.  This means
// that we need to process the line one character at a time.
void brief_render_line(int line_num, int x, int y, int instance)
{
	int len, count, next, truncate_len, last_color, offset, w, h, bright_len, i;
	colored_char *src;
	char line[MAX_BRIEF_LINE_LEN];

	src = &Colored_text[instance][line_num][0];
	len = Colored_text_len[instance][line_num];

	if (len <= 0){
		return;
	}

	truncate_len = fl2i(Brief_text_wipe_time_elapsed / BRIEF_TEXT_WIPE_TIME * Max_briefing_line_len);
	if (truncate_len > len){
		truncate_len = len;
	}

	bright_len = 0;
	if (truncate_len < len) {
		if (truncate_len <= BRIGHTEN_LEAD) {
			bright_len = truncate_len;
			truncate_len = 0;

		} else {
			bright_len = BRIGHTEN_LEAD;
			truncate_len -= BRIGHTEN_LEAD; 
		}
	}

	offset = 0;
	count  = 0;
	next	 = 0;

	gr_set_color_fast(&Color_white);
	last_color = BRIEF_TEXT_WHITE;
	for (i=0; i<truncate_len; i++) {
		if (count >= truncate_len){
			break;
		}

		line[next] = src[count].letter;

		if (is_white_space(line[next])) {
			// end of word reached, blit it
			line[next + 1] = 0;
			gr_string(x + offset, y, line);
			gr_get_string_size(&w, &h, line);
			offset += w;
			next = 0;

			// reset color
			if (last_color != BRIEF_TEXT_WHITE) {
				brief_set_text_color(BRIEF_TEXT_WHITE);
				last_color = BRIEF_TEXT_WHITE;
			}

			count++;
			continue;
		}

		if (src[count].color != last_color) {
			brief_set_text_color(src[count].color);
			last_color = src[count].color;
		}

		count++;
		next++;
	}	// end for

	line[next] = 0;
	gr_string(x + offset, y, line);


	// draw leading portion of the line bright white
	if (bright_len) {

		gr_set_color_fast(&Color_bright_white);
		for (i=0; i<truncate_len+bright_len; i++) {
			line[i] = src[i].letter;
		}

		line[i] = 0;


		if ( truncate_len > 0 )	{
			int width_dim, height_dim;
			gr_get_string_size(&width_dim, &height_dim, line, truncate_len );
			gr_string(x+width_dim, y, &line[truncate_len]);
		} else {
			gr_string(x, y, line);
		}

		// JAS: Not needed?
		//		// now erase the part we don't want to be bright white
		//		gr_set_color_fast(&Color_black);
		//		if (i > BRIGHTEN_LEAD) {
		//			line[i - BRIGHTEN_LEAD] = 0;
		//			gr_get_string_size(&w, &h, line);
		//			gr_set_clip(x, y, w, gr_get_font_height());
		//			gr_clear();
		//			gr_reset_clip();
		//		}
	}
}

int brief_text_wipe_finished()
{
	if ( Brief_text_wipe_time_elapsed > (BRIEF_TEXT_WIPE_TIME+0.5f) ) {
		return 1;
	}

	return 0;
}

// -------------------------------------------------------------------------------------
// brief_render_text()
//
// input:	frametime	=>	Time in seconds of previous frame
//				instance		=>	Optional parameter.  Used to indicate which text stream is used.
//									This value is 0 unless multiple text streams are required
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

// ------------------------------------------------------------------------------------
// brief_render_elements()
//
// Draw the lines that show objects positions on the grid
//
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

// AL 11-20-97: don't draw elevation lines.. they are confusing
/*
	brief_rpd_line(&gpos, pos);	//	Line from grid to object center.

	tpos = gpos;

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, -dxz/2);
	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.fvec, -dxz/2);
	
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, dxz/2);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.fvec, dxz/2);
	
	brief_rpd_line(&gpos, &tpos);

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, dxz);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, -dxz);

	brief_rpd_line(&gpos, &tpos);
*/
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

// ------------------------------------------------------------------------------------
// brief_set_camera_target()
//
//	input:	pos		=>		target position for the camera
//				orient	=>		target orientation for the camera
//				time		=>		time in ms to reach target
//
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
//	vm_vec_scale(&Cam_vel, 1.0f/time_in_seconds);
	if ( !IS_VEC_NULL(&Cam_vel) ) {
		vm_vec_normalize(&Cam_vel);
	}

	// calculate lookat point velocity
	vm_vec_sub(&Lookat_vel, &Target_lookat_pos, &Current_lookat_pos);
	vm_vec_scale(&Lookat_vel, 1.0f/time_in_seconds);

	Start_dist = vm_vec_dist(&Start_cam_pos, &Start_lookat_pos);
	End_dist = vm_vec_dist(&Target_cam_pos, &Target_lookat_pos);
	Dist_change_rate = (End_dist - Start_dist) / time_in_seconds;

	Total_dist=vm_vec_dist(&Start_cam_pos, &Target_cam_pos);

//	Peak_speed=Total_dist/Total_move_time*1.5f;
//	Cam_accel = Peak_speed/Total_move_time*3.0f;

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

		default:	
			Int3();	// unsupported meta-code
			break;
	} // end switch

	return BRIEF_TEXT_WHITE;
}

void brief_set_text_color(int color_index)
{
	Assert(color_index < MAX_BRIEF_TEXT_COLORS);
	gr_set_color_fast(Brief_text_colors[color_index]);
}

// Set up the Colored_text array.
// input:		index		=>		Index into Brief_text[] for source text.
//					instance	=>		Which instance of Colored_text[] to use.  
//										Value is 0 unless multiple text streams are required.
int brief_text_colorize(int index, int instance)
{
	char *src;
	int len, i, skip_to_next_word, dest_len;
	colored_char *dest;
	ubyte active_color_index;

	src = Brief_text[index];
	dest = &Colored_text[instance][index][0];
	len = strlen(src);

	skip_to_next_word = 0;
	dest_len = 0;
	active_color_index = BRIEF_TEXT_WHITE;
	for (i=0; i<len; i++) {
		if (skip_to_next_word) {
			if (is_white_space(src[i])) {
				skip_to_next_word = 0;
			}

			continue;
		}

		if ( src[i] == BRIEF_META_CHAR && is_white_space(src[i + 2]) ) {
			active_color_index = brief_return_color_index(src[i + 1]);
			skip_to_next_word = 1;
			continue;
		}

		if (is_white_space(src[i])) {
			active_color_index = BRIEF_TEXT_WHITE;
		}

		dest[dest_len].letter = src[i];
		dest[dest_len].color  = active_color_index;
		dest_len++;
	} // end for

	dest[dest_len].letter = 0;
	Colored_text_len[instance][index] = dest_len;
	return len;
}

// ------------------------------------------------------------------------------------
// brief_color_text_init()
//
//	input:	src		=>		paragraph of text to process
//				w			=>		max width of line in pixels
//				instance	=>		optional parameter, used when multiple text streams are required
//									(default value is 0)
int brief_color_text_init(char *src, int w, int instance)
{
	int i, n_lines, len;
	int n_chars[MAX_BRIEF_LINES];
	char *p_str[MAX_BRIEF_LINES];
	
	Assert(src);
	n_lines = split_str(src, w, n_chars, p_str, MAX_BRIEF_LINES, BRIEF_META_CHAR);
	Assert(n_lines >= 0);

	Max_briefing_line_len = 1;
	for (i=0; i<n_lines; i++) {
		Assert(n_chars[i] < MAX_BRIEF_LINE_LEN);
		strncpy(Brief_text[i], p_str[i], n_chars[i]);
		Brief_text[i][n_chars[i]] = 0;
		drop_leading_white_space(Brief_text[i]);
		len = brief_text_colorize(i, instance);
		if (len > Max_briefing_line_len)
			Max_briefing_line_len = len;
	}

	Brief_text_wipe_time_elapsed = 0.0f;
	Play_brief_voice = 0;

	Num_brief_text_lines[instance] = n_lines;
	return n_lines;
}

// ------------------------------------------------------------------------------------
// brief_get_free_move_icon()
//
//	returns:		failure	=>		-1
//					success	=>		handle to a free move icon struct
//
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


// ------------------------------------------------------------------------------------
// brief_set_move_list()
//
//	input:	new_stage		=>		new stage number that briefing is now moving to
//				current_stage	=>		current stage that the briefing is on
//				time				=>		time in seconds
//
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
					//nprintf(("Alan","We found a match in icon %s\n", cb->icons[i].label));
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
					if ( !IS_VEC_NULL(&imi->direction) ) {
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


// ------------------------------------------------------------------------------------
// brief_set_new_stage()
//
//	input:	pos			=>		target position for the camera
//				orient		=>		target orientation for the camera
//				time			=>		time in ms to reach target
//				stage_num	=>		stage number of briefing (start numbering at 0)
//

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
			strcpy(msg, "");
		} else {
			strcpy(msg, Briefing->stages[stage_num].new_text);
		}
	} else {
		strcpy(msg, XSTR( "Please review your objectives for this mission.", 395));
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

// ------------------------------------------------------------------------------------
// Interpolate between matrices.
// elapsed_time/total_time gives percentage of interpolation between cur
// and goal.
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

// calculate how far the camera should have moved
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

// ------------------------------------------------------------------------------------
// Update the camera position
void brief_camera_move(float frametime, int stage_num)
{
	vec3d	dist_moved;
	float		dist;
	vec3d	w_out;
	matrix	result;

	Elapsed_time += frametime;

	if ( Cam_target_reached ) { 
//		Current_cam_pos = Target_cam_pos;
//		Current_lookat_pos = Target_lookat_pos;
//		Current_cam_orient = Target_cam_orient;
		return;
	}

	// Update orientation
	if ( (Elapsed_time < Total_move_time) ) {
//		interpolate_matrix(&Current_cam_orient, &Target_cam_orient, &Start_cam_orient, Elapsed_time, Total_move_time );
		vm_matrix_interpolate(&Target_cam_orient, &Current_cam_orient, &W_init, frametime, &result, &w_out, &Vel_limit, &Acc_limit);
		Current_cam_orient = result;
		W_init = w_out;
	}

	/*
	// interpolate lookat position
	if ( vm_vec_cmp( &Current_lookat_pos, &Target_lookat_pos ) ) {
		vm_vec_copy_scale(&dist_moved, &Lookat_vel, Elapsed_time);
		vm_vec_add(&Current_lookat_pos, &Start_lookat_pos, &dist_moved);

		if ( camera_pos_past_target(&Start_lookat_pos, &Current_lookat_pos, &Target_lookat_pos) ) {
			Current_lookat_pos = Target_lookat_pos;
		}
	}

	cur_dist = Start_dist + Dist_change_rate * Elapsed_time;
	vm_vec_copy_scale(&dist_moved, &Current_cam_orient.vec.fvec, -cur_dist);
	vm_vec_add(&Current_cam_pos, &Current_lookat_pos, &dist_moved);
	*/

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

//	Project the viewer's position onto the grid plane.  If more than threshold distance
//	from grid center, move grid center.
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

//	Create a nice grid -- centered at origin, 10x10, 10.0 size squares, in xz plane.
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

//	Rotate and project points and draw a line.
void brief_rpd_line(vec3d *v0, vec3d *v1)
{
	vertex	tv0, tv1;
	g3_rotate_vertex(&tv0, v0);
	g3_rotate_vertex(&tv1, v1);

/*
	g3_project_vertex(&tv0);	
	g3_project_vertex(&tv1);

	if ( (tv0.flags & PF_OVERFLOW) || (tv1.flags & PF_OVERFLOW) )
		return;
*/

	gr_set_color_fast(&Color_grey);
	g3_draw_line(&tv0, &tv1);
}

//	Renders a grid defined in a grid struct
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
//	SET_DARK;

	//	Draw the column lines.
	for (i=0; i<=ncols; i++)
		brief_rpd_line(&gridp->gpoints1[i], &gridp->gpoints2[i]);

	//	Draw the row lines.
	for (i=0; i<=nrows; i++)
		brief_rpd_line(&gridp->gpoints3[i], &gridp->gpoints4[i]);

	ncols = gridp->ncols / 2;
	nrows = gridp->nrows / 2;

	// now draw the larger, brighter gridlines that is x10 the scale of smaller one.
//	SET_MEDIUM;
/*
	for (i=0; i<=ncols; i++)
		brief_rpd_line(&gridp->gpoints5[i], &gridp->gpoints6[i]);

	for (i=0; i<=nrows; i++)
		brief_rpd_line(&gridp->gpoints7[i], &gridp->gpoints8[i]);
*/
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

// initialize the array of handles to the different voice streams
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

		// couldn't load animation, ask user to insert CD (if necessary)
		// if ( Brief_voice_ask_for_cd ) {
			// if ( game_do_cd_check() == 0 ) {
				// Brief_voice_ask_for_cd = 0;
				// break;
			// }
		// }
	}
}

// open and pre-load the stream buffers for the different voice streams
void brief_voice_load_all()
{
	int			i;
	brief_stage	*bs;

	// Brief_voice_ask_for_cd = 1;

	Assert( Briefing != NULL );
	for ( i = 0; i < Briefing->num_stages; i++ ) {
		bs = &Briefing->stages[i];
		if ( strnicmp(bs->voice, NOX("none"), 4) ) {
			brief_load_voice_file(i, bs->voice);
//			Brief_voices[i] = audiostream_open( bs->voice, ASF_VOICE );
		}
	}
}

// close all the briefing voice streams
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

// start playback of the voice for a particular briefing stage
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

// stop playback of the voice for a particular briefing stage
void brief_voice_stop(int stage_num)
{
	if ( Brief_voices[stage_num] == -1 )
		return;

	audiostream_stop(Brief_voices[stage_num], 1, 0);	// stream is automatically rewound
}

// pause playback of the voice for a particular briefing stage, to resume just
// call brief_voice_unpause() again
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

// get the dimensions for a briefing icon
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

#define STAGE_ADVANCE_DELAY	1000		// time in ms to wait after voice stops before advancing stage

// should briefing advance to the next stage?
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
