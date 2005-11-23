/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUD.h $
 * $Revision: 2.20 $
 * $Date: 2005-11-23 00:49:51 $
 * $Author: phreak $
 *
 * Header file for functions that contain HUD functions at a high level
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.19  2005/10/09 08:03:19  wmcoolmon
 * New SEXP stuff
 *
 * Revision 2.18  2005/09/26 04:08:53  Goober5000
 * some more cleanup
 * --Goober5000
 *
 * Revision 2.17  2005/09/26 02:15:03  Goober5000
 * okay, this should all be working :)
 * --Goober5000
 *
 * Revision 2.16  2005/09/25 22:24:22  Goober5000
 * more fiddly stuff
 * --Goober5000
 *
 * Revision 2.15  2005/08/31 07:17:43  Goober5000
 * removed two unused functions
 * --Goober5000
 *
 * Revision 2.14  2005/07/18 03:44:01  taylor
 * cleanup hudtargetbox rendering from that total hack job that had been done on it (fixes wireframe view as well)
 * more non-standard res fixing
 *  - I think everything should default to resize now (much easier than having to figure that crap out)
 *  - new mouse_get_pos_unscaled() function to return 1024x768/640x480 relative values so we don't have to do it later
 *  - lots of little cleanups which fix several strange offset/size problems
 *  - fix gr_resize/unsize_screen_pos() so that it won't wrap on int (took too long to track this down)
 *
 * Revision 2.13  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.12  2005/02/27 07:07:47  wmcoolmon
 * nonstandard res HUD stuff
 *
 * Revision 2.11  2005/02/13 08:38:54  wmcoolmon
 * nonstandard resolution-friendly function updates
 *
 * Revision 2.10  2005/01/30 03:26:11  wmcoolmon
 * HUD updates
 *
 * Revision 2.9  2005/01/11 21:38:49  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.8  2005/01/01 07:18:47  wmcoolmon
 * NEW_HUD stuff, turned off this time. :) It's in a state of disrepair at the moment, doesn't show anything.
 *
 * Revision 2.7  2004/09/17 00:18:17  Goober5000
 * changed toggle-hud to hud-disable; added hud-disable-except-messages
 * --Goober5000
 *
 * Revision 2.6  2004/08/11 05:06:25  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.5  2004/05/30 08:04:49  wmcoolmon
 * Final draft of the HUD parsing system structure. May change how individual coord positions are specified in the TBL. -C
 *
 * Revision 2.4  2004/03/05 09:02:03  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2003/01/17 07:59:09  Goober5000
 * fixed some really strange behavior with strings not being truncated at the
 * # symbol
 * --Goober5000
 *
 * Revision 2.2  2002/10/17 20:40:50  randomtiger
 * Added ability to remove HUD ingame on keypress shift O
 * So I've added a new key to the bind list and made use of already existing hud removal code.
 *
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 9     8/09/99 3:14p Dave
 * Make "launch" warning gauge draw in code.
 * 
 * 8     8/01/99 12:39p Dave
 * Added HUD contrast control key (for nebula).
 * 
 * 7     7/24/99 1:54p Dave
 * Hud text flash gauge. Reworked dead popup to use 4 buttons in red-alert
 * missions.
 * 
 * 6     6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 5     6/07/99 4:20p Andsager
 * Add HUD color for tagged object.  Apply to target and radar.
 * 
 * 4     5/21/99 1:44p Andsager
 * Add engine wash gauge
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
 * 64    8/28/98 3:28p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 63    8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 62    5/04/98 6:12p Lawrance
 * Write generic function hud_end_string_at_first_hash_symbol(), to use in
 * various spots on the HUD
 * 
 * 61    4/30/98 6:04p Lawrance
 * Make subspace gauge report "aborted" when ESC pressed while starting
 * warp out.
 * 
 * 60    4/23/98 1:49a Allender
 * major rearm/repair fixes for multiplayer.  Fixed respawning of AI ships
 * to not respawn until 5 seconds after they die.  Send escort information
 * to ingame joiners
 * 
 * 59    4/20/98 12:36a Mike
 * Make team vs. team work when player is hostile.  Several targeting
 * problems.
 * 
 * 58    4/13/98 12:50p Allender
 * made rearm shortcut work more appropriately.  Make countermeasure
 * succeed work on clients in multiplayer
 * 
 * 57    3/30/98 1:08a Lawrance
 * Implement "blast" icon.  Blink HUD icon when player ship is hit by a
 * blast.
 * 
 * 56    3/19/98 5:05p Dave
 * Put in support for targeted multiplayer text and voice messaging (all,
 * friendly, hostile, individual).
 * 
 * 55    3/17/98 12:29a Dave
 * Put in hud support for rtvoice. Several ui interface changes.
 * 
 * 54    3/14/98 4:59p Lawrance
 * Totally rework HUD wingman status gauge to work with 5 arbitrary wings
 * 
 * 53    3/11/98 12:13a Lawrance
 * Pop up weapon gauge when rearm time is showing
 * 
 * 52    3/09/98 4:22p Lawrance
 * Don't do certain HUD functions when the hud is disabled
 * 
 * 51    3/07/98 6:27p Lawrance
 * Add support for disabled hud.
 * 
 * 50    3/06/98 5:10p Allender
 * made time to: field in extended targetbox use support time to dock code
 * for all docking shpis.  Only display for waypoints and docking (not
 * undocking).  Small fixups to message menu -- not allowing depart when
 * disabled.  Depart is now by default ignored for all non-small ships
 * 
 * 49    2/23/98 6:49p Lawrance
 * Use gr_aabitmap_ex() instead of clipping regions
 * 
 * 48    2/12/98 4:58p Lawrance
 * Change to new flashing method.
 * 
 * 47    2/11/98 9:44p Allender
 * rearm repair code fixes.  hud support view shows abort status.  New
 * support ship killed message.  More network stats
 * 
 * 46    2/09/98 8:05p Lawrance
 * Add new gauges: cmeasure success, warp-out, and missiontime
 * 
 * 45    1/28/98 7:19p Lawrance
 * Get fading/highlighting animations working
 * 
 * 44    1/24/98 3:21p Lawrance
 * Add flashing when hit, and correct association with the wingman status
 * gauge.
 * 
 * 43    1/21/98 7:20p Lawrance
 * Make subsystem locking only work with line-of-sight, cleaned up locking
 * code, moved globals to player struct.
 * 
 * 42    1/20/98 12:52p Lawrance
 * Draw talking head as alpha-color bitmap, black out region behind
 * animation.
 * 
 * 41    1/19/98 10:01p Lawrance
 * Implement "Electronics" missiles
 * 
 * 40    1/15/98 5:23p Lawrance
 * Add HUD gauge to indicate completed objectives.
 * 
 * 39    1/14/98 11:07p Lawrance
 * Hook in brightness slider to HUD config.
 * 
 * 38    1/12/98 11:16p Lawrance
 * Wonderful HUD config.
 * 
 * 37    1/10/98 12:41a Lawrance
 * start work on new HUD config
 * 
 * 36    1/05/98 9:38p Lawrance
 * Implement flashing HUD gauges.
 * 
 * 35    1/02/98 9:10p Lawrance
 * Big changes to how colors get set on the HUD.
 * 
 * 34    12/18/97 8:46p Lawrance
 * Move IFF_color definitions from HUD->ship, so FRED can use them.
 * 
 * 33    12/01/97 12:27a Lawrance
 * redo default alpha color for HUD, make it easy to modify in the future
 * 
 * 32    11/17/97 6:37p Lawrance
 * new gauges: extended target view, new lock triangles, support ship view
 * 
 * 31    11/13/97 10:46p Lawrance
 * implemented new escort view, damage view and weapons
 * 
 * 30    11/13/97 6:15p Lawrance
 * Add new weapons gauge
 * 
 * 29    11/11/97 5:05p Lawrance
 * use global value for target integrity, to avoid recalculation
 * 
 * 28    11/11/97 11:06a Lawrance
 * add function to convert a numbered string to use mono-spaced 1's
 * 
 * 27    11/09/97 3:25p Lawrance
 * increase default alpha color
 * 
 * 26    11/05/97 11:19p Lawrance
 * create an array of 16 HUD colors, that span the different alpha values
 * 
 * 25    11/04/97 7:50p Lawrance
 * supporting new HUD reticle and shield icons
 * 
 * 24    10/11/97 6:38p Lawrance
 * added functions to manage HUD animations
 * 
 * 23    9/14/97 10:24p Lawrance
 * add damage screen popup window
 * 
 * 22    7/14/97 11:47a Lawrance
 * add function to display hud messages (so navmap can call it)
 * 
 * 21    6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 20    6/06/97 4:41p John
 * Fixed alpha colors to be smoothly integrated into gr_set_color_fast
 * code.
 * 
 * 19    6/05/97 6:47p John
 * First pass at changing HUD translucency.
 * 
 * 18    4/13/97 3:53p Lawrance
 * separate out the non-rendering dependant portions of the HUD ( sounds,
 * updating lock position, changing targets, etc) and put into
 * hud_update_frame()
 * 
 * 17    4/10/97 5:29p Lawrance
 * hud rendering split up into hud_render_3d(), hud_render_2d() and
 * hud_render_target_model()
 * 
 * 16    4/07/97 3:50p Allender
 * ability to assign > 1 ship to a hotkey.  Enabled use of hotkeys in
 * squadmate messaging
 * 
 * 15    3/19/97 5:53p Lawrance
 * integrating new Misc_sounds[] array (replaces old Game_sounds
 * structure)
 * 
 * 14    1/13/97 5:36p Lawrance
 * integrating new Game_sounds structure for general game sounds
 * 
 * 13    1/07/97 6:56p Lawrance
 * adding sound hooks
 * 
 * 12    1/02/97 7:12p Lawrance
 * adding hooks for more sounds
 * 
 * 11    11/26/96 2:35p John
 * Made so you can change HUD colors.
 * 
 * 10    11/19/96 10:16a Lawrance
 * adding colors to header file
 * 
 * 9     11/17/96 5:27p Lawrance
 * added externs for globals that specify the HUD gauge colors
 * 
 * 8     11/15/96 12:11a Lawrance
 * took out old message scrolling and moved to HUDmessage
 *
 * $NoKeywords: $
 *
*/

#ifndef __HUD_H__
#define __HUD_H__

#include "hud/hudgauges.h"
#include "graphics/2d.h"
#include "hud/hudparse.h"

#define SCREEN_CENTER_X ((gr_screen.clip_left + gr_screen.clip_right)	/ 2.0f)
#define SCREEN_CENTER_Y ((gr_screen.clip_top + gr_screen.clip_bottom)   / 2.0f)

struct object;

typedef struct hud_anim {
	char filename[MAX_FILENAME_LEN];
	int first_frame;	// the bitmap id for the first frame in the animation... note that
							// all bitmap id's following this frame are numbered sequentially
	int num_frames;		// number of frames in the animation
	int sx, sy;			// screen (x,y) of top-left corner of animation
	float total_time;	// total time in seconds for the animation (depends on animation fps)
	float time_elapsed;	// time that has elapsed (in seconds) since animation started playing
} hud_anim;

typedef struct hud_frames {
	int	first_frame;
	int	num_frames;
} hud_frames;

extern int HUD_contrast;


//Current HUD to use for info -C
#ifndef NEW_HUD
extern hud_info* current_hud;
#endif

#define HUD_NUM_COLOR_LEVELS	16
extern color HUD_color_defaults[HUD_NUM_COLOR_LEVELS];

// extern globals that will control the color of the HUD gauges
#define HUD_COLOR_ALPHA_USER_MAX		13		// max user-settable alpha, absolute max is 15
#define HUD_COLOR_ALPHA_USER_MIN		3		// min user-settable alpha, absolute min is 0

#define HUD_COLOR_ALPHA_MAX			15
#define HUD_COLOR_ALPHA_DEFAULT		8

#define HUD_BRIGHT_DELTA				7		// Level added to HUD_color_alpha to make brightness used for flashing

// hud macro for maybe flickering all gauges
#define GR_AABITMAP(a, b, c)						{ int jx, jy; if(emp_should_blit_gauge()) { gr_set_bitmap(a); jx = b; jy = c; emp_hud_jitter(&jx, &jy); gr_aabitmap(jx, jy); } }
#define GR_AABITMAP_EX(a, b, c, d, e, f, g)	{ int jx, jy; if(emp_should_blit_gauge()) { gr_set_bitmap(a); jx = b; jy = c; emp_hud_jitter(&jx, &jy); gr_aabitmap_ex(jx, jy, d, e, f, g); } }

extern int HUD_color_red;
extern int HUD_color_green;
extern int HUD_color_blue;
extern int HUD_color_alpha;

extern color HUD_color_debug;

// animations for damages gauges
extern hud_anim Target_static;
extern hud_anim Radar_static;

// Values used "wiggle" the HUD.  In the 2D HUD case, the clip region accounts
// for these, but for the 3d-type hud stuff, you need to add these in manually.
extern float HUD_offset_x;
extern float HUD_offset_y;

// Global: integrity of player's target
extern float Pl_target_integrity;
extern float Player_rearm_eta;

void HUD_init_colors();
void HUD_init();
void hud_update_frame();		// updates hud systems not dependant on rendering
void HUD_render_3d(float frametime);			// renders 3d dependant gauges
void HUD_render_2d(float frametime);			// renders the 2d gauges
void hud_stop_looped_engine_sounds();
void hud_show_messages();
void hud_damage_popup_toggle();

// set the offset values for this render frame
void HUD_set_offsets(object *viewer_obj, int wiggedy_wack);

// Basically like gr_reset_clip only it accounts for hud jittering
void HUD_reset_clip();
void hud_save_restore_camera_data(int save);

// Basically like gr_set_clip only it accounts for hud jittering
void HUD_set_clip(int x, int y, int w, int h);

// do flashing text gauge
void hud_start_text_flash(char *txt, int t);

// convert a string to use mono spaced numbers
void hud_num_make_mono(char *num_str);

// functions for handling hud animations
void hud_anim_init(hud_anim *ha, int sx, int sy, char *filename);
void hud_frames_init(hud_frames *hf);
int	hud_anim_render(hud_anim *ha, float frametime, int draw_alpha=0, int loop=1, int hold_last=0, int reverse=0,bool resize=true);
int	hud_anim_load(hud_anim *ha);

// flash text at the given y
void hud_show_text_flash_icon(char *txt, int y, int bright);

// functions for displaying the support view popup
void hud_support_view_start();
void hud_support_view_stop(int stop_now=0);
void hud_support_view_abort();
void hud_support_view_blit();

void HUD_init_hud_color_array();

// setting HUD colors
void hud_set_default_color();
void hud_set_iff_color(object *objp, int is_bright=0);
void hud_set_bright_color();
void hud_set_dim_color();

// HUD gauge functions
#define HUD_C_NONE			-4
#define HUD_C_BRIGHT			-3
#define HUD_C_DIM				-2
#define HUD_C_NORMAL			-1
void	hud_set_gauge_color(int gauge_index, int bright_index = HUD_C_NONE);
int	hud_gauge_active(int gauge_index);
void	hud_gauge_start_flash(int gauge_index);
int	hud_gauge_maybe_flash(int gauge_index);

// popup gauges
void	hud_init_popup_timers();
void	hud_gauge_popup_start(int gauge_index, int time=4000);
int	hud_gauge_is_popup(int gauge_index);
int	hud_gauge_popup_active(int gauge_index);

// objective status gauge
void hud_add_objective_messsage(int type, int status);

int	hud_team_matches_filter(int team_filter, int ship_team);
void	hud_maybe_clear_head_area();

int	hud_get_dock_time( object *docker_objp );
void	hud_show_radar();
void	hud_show_target_model();
void	hud_show_voice_status();

void	hud_subspace_notify_abort();

// render multiplayer text message currently being entered if any
void hud_maybe_render_multi_text();

void hud_toggle_draw();
int	hud_disabled();
int hud_support_find_closest( int objnum );

// Goober5000
void hud_set_draw(int draw);
void hud_disable_except_messages(int disable);
int hud_disabled_except_messages();

// contrast stuff
void hud_toggle_contrast();
void hud_set_contrast(int high);

//	Return mask of enemies.
//	Works in team vs. team multiplayer.
int opposing_team_mask(int team_mask);

#endif	/* __HUD_H__ */

