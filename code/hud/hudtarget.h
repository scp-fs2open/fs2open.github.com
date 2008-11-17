/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDtarget.h $
 * $Revision: 2.9.2.1 $
 * $Date: 2007-12-28 02:10:37 $
 * $Author: Backslash $
 *
 * Header file for HUD targeting functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.9  2006/01/13 03:30:59  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 2.8  2005/10/25 01:21:52  wmcoolmon
 * Bumped MAX_HOTKEY_TARGET_ITEMS
 *
 * Revision 2.7  2005/10/09 08:03:20  wmcoolmon
 * New SEXP stuff
 *
 * Revision 2.6  2005/07/22 09:19:40  wmcoolmon
 * Dynamic AI Class number commit. KNOWN BUG: When AI_CLASS_INCREMENT is hit and vm_realloc is called, memory corruption seems to
 * result. Not at all sure what causes this; if this can't be resolved soon, we can always treat _INCREMENT like _MAX
 *
 * Revision 2.5  2005/07/13 03:15:52  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.4  2005/04/05 05:53:17  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.3  2004/08/11 05:06:25  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/03/05 09:02:04  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 8     9/07/99 11:26p Andsager
 * Fix "r" targeting key, making evaluate_ship_as_closest_target() and
 * hud_target_live_turret() consider if turret is targeting player
 * 
 * 7     8/24/99 2:55p Andsager
 * Add new prioritized turret selection code.
 * 
 * 6     7/09/99 12:00a Andsager
 * Added target box with distance for remote detonate weapons
 * 
 * 5     5/28/99 10:00a Andsager
 * Make player hud target affected by Nebula range
 * 
 * 4     2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 3     12/21/98 5:03p Dave
 * Modified all hud elements to be multi-resolution friendly.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 101   8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 100   5/15/98 8:36p Lawrance
 * Add 'target ship that last sent transmission' target key
 * 
 * 99    5/04/98 6:12p Lawrance
 * Write generic function hud_end_string_at_first_hash_symbol(), to use in
 * various spots on the HUD
 * 
 * 98    4/13/98 5:06p Lawrance
 * truncate secondary weapons names at the '#' char
 * 
 * 97    4/08/98 1:18a Lawrance
 * Externalize function to find distance between two objects.
 * 
 * 96    4/07/98 5:30p Lawrance
 * Player can't send/receive messages when comm is destroyed.  Garble
 * messages when comm is damaged.
 * 
 * 95    3/30/98 12:20a Lawrance
 * remove CARGO_REVEAL time, it is now in ship_info
 * 
 * 94    3/20/98 5:40p Lawrance
 * Change targeting of uninspected cargo to select the next closest.
 * 
 * 93    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 92    3/11/98 12:14a Lawrance
 * Added hud_target_auto_target_next()
 * 
 * 91    3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 90    3/07/98 3:50p Lawrance
 * Add lead indicator for asteroids
 * 
 * 89    3/04/98 3:44p Lawrance
 * Only flash docking text for a couple of seconds
 * 
 * 88    3/02/98 11:32p Lawrance
 * Allow asteroids about to impact ships to be bracketed
 * 
 * 87    2/26/98 12:33a Lawrance
 * Added back slew mode,  lots of changes to external and chase views.
 * 
 * 86    2/24/98 2:49p Lawrance
 * Enable cheat key to disable 'hidden from sensors' targeting
 * 
 * 85    2/06/98 4:33p Lawrance
 * extern hud_get_best_primary_bank()
 * 
 * 84    2/02/98 7:00p Lawrance
 * Adding new targeting keys (bomb, uninspected cargo, new ship, live
 * turrets).
 * 
 * 83    1/29/98 11:53p Lawrance
 * Change repair ship targeting to favor support ships that are headed for
 * the player.
 * 
 * 82    1/20/98 9:30p Lawrance
 * Have lead indicator, offscreen indicator and distance be relative to
 * targeted subsystem when possible.
 * 
 * 81    1/17/98 1:30a Lawrance
 * Add countermeasure gauge
 * 
 * 80    1/10/98 12:42a Lawrance
 * make cargo inspection more realistic
 * 
 * 79    1/08/98 11:34p Lawrance
 * change cargo reveal formula to max(150,1.5*radius)
 * 
 * 78    1/02/98 9:10p Lawrance
 * Big changes to how colors get set on the HUD.
 * 
 * 77    12/16/97 9:13p Lawrance
 * Integrate new gauges into HUD config.
 * 
 * 76    12/09/97 8:12a Allender
 * changes to hotkey stuff.  Don't allow mission defined hotkeys to
 * override user defined ones once the mission starts
 * 
 * 75    12/04/97 10:23a Lawrance
 * don't allow matching speed unless target speed is above
 * MATCH_SPEED_THRESHOLD
 * 
 * 74    12/01/97 5:12p Lawrance
 * have minimial targeting gauges shown in chase and external view 
 * 
 * 73    12/01/97 12:27a Lawrance
 * redo default alpha color for HUD, make it easy to modify in the future
 * 
 * 72    11/27/97 4:23p Lawrance
 * add subsys_in_view to the player struct, which indicates that the
 * player has a line of sight to his targeted subsystem
 * 
 * 71    11/20/97 5:40p Lawrance
 * Make cargo revealing work so that anyone (including AI) on your team
 * can reveal cargo for everyone else on the team.
 * 
 * 70    11/20/97 1:09a Lawrance
 * add support for 'target closest friendly repair ship'
 * 
 * 69    11/11/97 12:58a Lawrance
 * implement new target monitor view
 * 
 * 68    10/30/97 12:33a Lawrance
 * update to show AIM_STRAFE debug output
 * 
 * 67    10/27/97 10:48p Lawrance
 * get previous hostile/friendly targeting working.. simplify code a lot
 * 
 * 66    10/22/97 5:53p Lawrance
 * move out subsystem_in_sight function
 * 
 * 65    10/12/97 3:44p Lawrance
 * use target lists to cycle through closest hostile and friendly ships
 * 
 * 64    10/11/97 6:38p Lawrance
 * having damage affect targeting
 * 
 * 63    10/10/97 6:15p Hoffoss
 * Implemented a training objective list display.
 * 
 * 62    10/08/97 5:07p Lawrance
 * make sensors and communication subsystem damage affect player
 * 
 * 61    9/22/97 4:55p Hoffoss
 * Added a training message window display thingy.
 * 
 * 60    8/31/97 6:40p Lawrance
 * make auto-target always target fighters/bombers first
 * 
 * 59    8/25/97 12:24a Lawrance
 * implemented HUD shield management
 * 
 * 58    8/19/97 11:46p Lawrance
 * adding new hud gauges for shileds, escort view, and weapons
 * 
 * 57    8/15/97 9:26a Lawrance
 * split off target box code into HUDtargetbox.cpp
 * 
 * 56    8/14/97 5:29p Lawrance
 * restructure target monitor code... support missile view and red shading
 * 
 * 55    8/12/97 5:51p Lawrance
 * allow targeting of missiles
 * 
 * 54    7/13/97 5:54p Lawrance
 * fix bug with restore game and the keyed targets
 * 
 * 53    7/01/97 11:53a Lawrance
 * allow cycling through targets in the reticle
 * 
 * 52    5/14/97 8:55a Lawrance
 * fix autotargeting so it switches to a new target as soon as current
 * target is dying
 * 
 * 51    5/02/97 2:11p Lawrance
 * added hud_prune_hotkeys()
 * 
 * 50    4/28/97 2:18p Lawrance
 * made hotkey_add_remove more generic
 * 
 * 49    4/22/97 4:53p Lawrance
 * allow auto-targeting to not play target fail sound when searching for
 * the closest target
 * 
 * 48    4/15/97 4:00p Mike
 * Intermediate checkin caused by getting other files.  Working on camera
 * slewing system.
 * 
 * 47    4/13/97 3:53p Lawrance
 * separate out the non-rendering dependant portions of the HUD ( sounds,
 * updating lock position, changing targets, etc) and put into
 * hud_update_frame()
 * 
 * 46    4/12/97 4:29p Lawrance
 * get missle locking and offscreen indicator working properly with
 * different sized screens
 * 
 * 45    4/10/97 5:29p Lawrance
 * hud rendering split up into hud_render_3d(), hud_render_2d() and
 * hud_render_target_model()
 * 
 * 44    4/08/97 10:55a Allender
 * draw purple brackets on ship sending a message
 * 
 * 43    4/07/97 3:50p Allender
 * ability to assign > 1 ship to a hotkey.  Enabled use of hotkeys in
 * squadmate messaging
 * 
 * 42    3/27/97 3:58p Lawrance
 * modified target_closest() to allow targeting closest ship that is
 * attacking a given objnum
 * 
 * 41    3/27/97 9:29a Lawrance
 * If reach maximum bounding box size, use radius targeting box method
 * 
 * 40    3/26/97 6:43p Lawrance
 * implemented new method for detecting target in reticle, that doesn't
 * require center point of target to be in reticle
 * 
 * 39    3/26/97 12:44p Lawrance
 * modified targeting functions to take team into accout for Target
 * Next/Prev/Closest
 * 
 * 38    3/19/97 5:53p Lawrance
 * integrating new Misc_sounds[] array (replaces old Game_sounds
 * structure)
 * 
 * 37    3/17/97 3:47p Mike
 * Homing missile lock sound.
 * More on AI ships firing missiles.
 * 
 * 36    3/10/97 8:53a Lawrance
 * using hud_stop_looped_locking_sounds() in place of
 * hud_stop_looped_sounds()
 * 
 * 35    1/02/97 10:32a Lawrance
 * fixed some bugs related to stopping looped sounds when targets die and
 * going to menus
 * 
 * 34    12/24/96 4:31p Lawrance
 * Target bracket drawing code moved to separate files
 * 
 * 33    12/23/96 7:53p Lawrance
 * took out missile lock code and moved to HUDlock.cpp and HUDlock.h
 * 
 * 32    12/17/96 11:10a Lawrance
 * added targeting of subsystem in reticle
 * 
*/

#ifndef _HUDTARGET_H
#define _HUDTARGET_H

#include "graphics/2d.h"

struct ship;
struct ship_subsys;
struct object;
struct weapon_info;

#define INCREASING	0
#define DECREASING	1
#define NO_CHANGE		2	

#define MATCH_SPEED_THRESHOLD				0.1f		// minimum speed target must be moving for match speed to apply
#define CARGO_RADIUS_DELTA					100		// distance added to radius required for cargo scanning
#define CAPITAL_CARGO_RADIUS_DELTA		250		// distance added to radius required for cargo scanning
#define CARGO_REVEAL_MIN_DIST				150		// minimum distance for reveal cargo (used if radius+CARGO_RADIUS_DELTA < CARGO_REVEAL_MIN_DIST)
#define CAP_CARGO_REVEAL_MIN_DIST		300		// minimum distance for reveal cargo (used if radius+CARGO_RADIUS_DELTA < CARGO_REVEAL_MIN_DIST)
#define CARGO_MIN_DOT_TO_REVEAL			0.95		// min dot to proceed to have cargo scanning take place

// structure and defines used for hotkey targeting
//WMC - bumped from 50 to 150; 10/24/2005
#define MAX_HOTKEY_TARGET_ITEMS		150		// maximum number of ships that can be targeted on *all* keys
#define SELECTION_SET					0x5000	// variable used for drawing brackets.  The bracketing code uses
															// TEAM_* values.  I picked this value to be totally out of that
															// range.  Only used for drawing selection sets
#define MESSAGE_SENDER					0x5001	// variable used for drawing brackets around a message sender.
															// See above comments for SELECTION_SET

// defines used to tell how a particular hotkey was added
#define HOTKEY_USER_ADDED				1
#define HOTKEY_MISSION_FILE_ADDED	2

typedef struct htarget_list {
	struct htarget_list	*next, *prev;		// for linked lists
	int						how_added;			// determines how this hotkey was added (mission default or player)
	object					*objp;				// the actual object
} htarget_list;

//for nebula toggle SEXP
#define		TOGGLE_TEXT_NEBULA_ALPHA	127
#define		TOGGLE_TEXT_NORMAL_ALPHA	160
extern int Toggle_text_alpha;

extern htarget_list htarget_free_list;
extern int Hud_target_w, Hud_target_h;

extern shader Training_msg_glass;

extern char **Ai_class_names;
extern char *Submode_text[];
extern char *Strafe_submode_text[];

extern void hud_init_targeting_colors();

void	hud_init_targeting();
void	hud_target_next(int team_mask = -1);
void	hud_target_prev(int team_mask = -1);
int	hud_target_closest(int team_mask = -1, int attacked_objnum = -1, int play_fail_sound = TRUE, int filter = 0, int turret_attacking_target = 0);
void	hud_target_in_reticle_old();
void	hud_target_in_reticle_new();
void	hud_target_subsystem_in_reticle();
void	hud_show_targeting_gauges(float frametime, int in_cockpit=1);
void	hud_target_targets_target();
void	hud_check_reticle_list();
void	hud_target_closest_locked_missile(object *A);
void	hud_target_missile(object *source_obj, int next_flag);
void	hud_target_next_list(int hostile=1, int next_flag=1);
int	hud_target_closest_repair_ship(int goal_objnum=-1);
void	hud_target_auto_target_next();
void	hud_show_remote_detonate_missile();

void	hud_target_uninspected_object(int next_flag);
void	hud_target_newest_ship();
void	hud_target_live_turret(int next_flag, int auto_advance=0, int turret_attacking_target=0);

void hud_target_last_transmit_level_init();
void hud_target_last_transmit();
void hud_target_last_transmit_add(int ship_num);

void hud_target_random_ship();

void	hud_target_next_subobject();
void	hud_target_prev_subobject();
void	hud_cease_subsystem_targeting(int print_message=1);
void	hud_cease_targeting();
void	hud_restore_subsystem_target(ship* shipp);
int	subsystem_in_sight(object* objp, ship_subsys* subsys, vec3d *eye, vec3d* subsystem);
vec3d* get_subsystem_world_pos(object* parent_obj, ship_subsys* subsys, vec3d* world_pos);
void	hud_target_change_check();

void hud_show_target_triangle_indicator(vertex *projected_v);
void hud_show_lead_indicator(vec3d *target_world_pos);
void hud_show_lead_indicator_quick(vec3d *target_world_pos, object *targetp);
void hud_show_orientation_tee();
void hud_show_hostile_triangle();
void hud_show_target_data();
void hud_show_afterburner_gauge();
void hud_show_weapons();
void hud_start_flash_weapon(int index);
void hud_show_auto_icons();
void hud_show_weapon_energy_gauge();
void hud_show_cmeasure_gauge();
void hud_show_brackets(object *targetp, vertex *projected_v);
void hud_draw_offscreen_indicator(vertex* target_point, vec3d *tpos, float distance=0.0f, int draw_solid=1);
void hud_show_homing_missiles(void);

int hud_sensors_ok(ship *sp, int show_msg = 1);
int hud_communications_state(ship *sp, int show_msg = 0);

int hud_get_best_primary_bank(float *range);
void hud_target_toggle_hidden_from_sensors();
void hud_maybe_flash_docking_text(object *objp);
int hud_target_invalid_awacs(object *objp);

// functions for hotkey selection sets

extern void hud_target_hotkey_select( int k );
extern void hud_target_hotkey_clear( int k );

extern void hud_target_hotkey_add_remove( int k, object *objp, int how_to_add);
extern void hud_show_selection_set();
extern void hud_show_message_sender();
void			hud_prune_hotkeys();
void			hud_keyed_targets_clear();

// Code to draw filled triangles
void hud_tri(float x1,float y1,float x2,float y2,float x3,float y3);
// Code to draw empty triangles.
void hud_tri_empty(float x1,float y1,float x2,float y2,float x3,float y3);

float hud_find_target_distance( object *targetee, object *targeter );

extern void polish_predicted_target_pos(weapon_info *wip, object *targetp, vec3d *enemy_pos, vec3d *predicted_enemy_pos, float dist_to_enemy, vec3d *last_delta_vec, int num_polish_steps);

void hud_stuff_ship_name(char *ship_name_text, ship *shipp);
void hud_stuff_ship_callsign(char *ship_callsign_text, ship *shipp);
void hud_stuff_ship_class(char *ship_class_text, ship *shipp);

#endif
