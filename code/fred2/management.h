/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/Management.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * This file handles the management of Objects, Ships, Wings, etc.  Basically
 * all the little structures we have that usually inter-relate that need to
 * be handled in a standard way, and thus should be handled by a single
 * function.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.15  2006/01/14 23:49:01  Goober5000
 * second pass; all the errors are fixed now; one more thing to take care of
 * --Goober5000
 *
 * Revision 1.14  2006/01/14 05:24:18  Goober5000
 * first pass at converting FRED to use new IFF stuff
 * --Goober5000
 *
 * Revision 1.13  2005/12/29 08:21:00  wmcoolmon
 * No my widdle FRED, I didn't forget about you ^_^ (codebase commit)
 *
 * Revision 1.12  2005/09/26 07:10:20  Goober5000
 * fix a small bug with custom wing names
 * --Goober5000
 *
 * Revision 1.11  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.10  2005/04/13 20:02:08  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.9  2005/03/06 23:19:27  wmcoolmon
 * Fixed the last of the jump node errors
 *
 * Revision 1.8  2005/01/29 05:40:27  Goober5000
 * regular docking should work again in FRED now (multiple docking still isn't done yet)
 * --Goober5000
 *
 * Revision 1.7  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.6  2004/03/15 12:14:46  randomtiger
 * Fixed a whole heap of problems with Fred introduced by changes to global vars.
 *
 * Revision 1.5  2004/02/13 04:25:37  randomtiger
 * Added messagebox to offer startup choice between htl, non htl and quitting.
 * Tidied up the background dialog box a bit and added some disabled controls for future ambient light feature.
 * Set non htl to use a lower level of model and texture detail to boost poor performance on this path.
 *
 * Revision 1.4  2003/10/15 22:10:34  Kazan
 * Da Species Update :D
 *
 * Revision 1.3  2003/09/05 06:53:41  Goober5000
 * added code to change the player's ship in single player
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:59  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 6     5/20/99 6:59p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 5     4/07/99 6:21p Dave
 * Fred and Freespace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 4     3/20/99 5:09p Dave
 * Fixed release build fred warnings and unhandled exception.
 * 
 * 3     10/29/98 10:41a Dave
 * Change the way cfile initializes exe directory.
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 56    12/05/97 4:07p Hoffoss
 * Changed code to allow WHO_FROM type ship sources to only show flyable
 * ships in list.
 * 
 * 55    11/21/97 2:55p Hoffoss
 * Added Nebula support to Fred.  Implemented loading and saving nebula
 * info to/from mission files.
 * 
 * 54    11/17/97 6:41p Lawrance
 * moved bitmask_2_bitnum to ship lib
 * 
 * 53    11/11/97 2:13p Allender
 * docking bay support for Fred and Freespace.  Added hook to ai code for
 * arrival/departure from dock bays.  Fred support now sufficient.
 * 
 * 52    10/08/97 11:47a Hoffoss
 * Added better fred handling of Weaponry_pool.
 * 
 * 51    9/16/97 9:41p Hoffoss
 * Changed Fred code around to stop using Parse_player structure for
 * player information, and use actual ships instead.
 * 
 * 50    9/09/97 10:29a Hoffoss
 * Added support for neutral team, and fixed changes made to how team is
 * used in ship structure.
 * 
 * 49    8/22/97 4:16p Hoffoss
 * added support for arrival and departure info in ship editor using
 * wing's info if editing marked ships in a wing instead of using ship's.
 * 
 * 48    8/17/97 10:22p Hoffoss
 * Fixed several bugs in Fred with Undo feature.  In the process, recoded
 * a lot of CFile.cpp.
 * 
 * 47    8/16/97 9:24p Hoffoss
 * Added support for team of players in multiplayer.
 * 
 * 46    8/16/97 4:51p Hoffoss
 * Fixed bugs with wing deletion and removing ships from a wing.
 * 
 * 45    8/16/97 2:02a Hoffoss
 * Made docked objects move together in Fred.
 * 
 * 44    8/15/97 5:14p Hoffoss
 * Completely changed around how initial orders dialog worked.  It's
 * pretty awesome now.
 * 
 * 43    8/15/97 11:09a Hoffoss
 * Created a list of order types that can be used for several things, and
 * yet easily changable.  Added order error checking against ship types.
 * 
 * 42    8/14/97 2:32p Hoffoss
 * fixed bug where controlling an object doesn't cause screen updates, and
 * added a number of cool features to viewpoint/control object code.
 * 
 * 41    8/12/97 3:33p Hoffoss
 * Fixed the "press cancel to go to reference" code to work properly.
 * 
 * 40    8/12/97 1:55a Hoffoss
 * Made extensive changes to object reference checking and handling for
 * object deletion call.
 * 
 * 39    8/08/97 1:31p Hoffoss
 * Added syncronization protection to cur_object_index changes.
 * 
 * 38    8/05/97 5:12p Jasen
 * Added advanced_stricmp() function to handle NULL pointers gracefully,
 * and utilized it in message editor query update function.
 * 
 * 37    8/05/97 1:28p Hoffoss
 * Fixed bug: if dockee ship deleted, docker becomes undocked.  Other
 * little changes.
 * 
 * 36    7/09/97 2:38p Allender
 * organized ship/wing editor dialogs.  Added protect ship and ignore
 * count checkboxes to those dialogs.  Changed flag code for
 * parse_objects.  Added unprotect sexpressions
 * 
 * 35    6/18/97 11:46a Hoffoss
 * Fixed initial order object reference updating and added briefing dialog
 * window tracking data.
 * 
 * 34    5/30/97 4:43p Hoffoss
 * Added code for allowing ships to be initially docked at mission start.
 * 
 * 33    5/05/97 5:44p Hoffoss
 * Added specialized popup menu choices, save before running FreeSpace,
 * and display filters.
 * 
 * 32    4/29/97 1:58p Hoffoss
 * Added some debugging to Fred to try and track down sexp corruption
 * causes.
 * 
 * 31    4/24/97 5:15p Hoffoss
 * fixes to Fred.
 * 
 * 30    4/23/97 11:55a Hoffoss
 * Fixed many bugs uncovered while trying to create Mission 6.
 * 
 * 29    3/28/97 5:39p Hoffoss
 * Player ships treated like other ships in sexp tree editor now.
 * 
 * 28    3/27/97 1:43p Hoffoss
 * Ship duplication (cloning) supported now.
 * 
 * 27    3/20/97 3:55p Hoffoss
 * Major changes to how dialog boxes initialize (load) and update (save)
 * their internal data.  This should simplify things and create less
 * problems.
 * 
 * 26    3/12/97 12:40p Hoffoss
 * Fixed bugs in wing object management functions, several small additions
 * and rearrangements.
 * 
 * 25    3/04/97 6:27p Hoffoss
 * Changes to Fred to handle new wing structure.
 * 
 * 24    3/03/97 4:32p Hoffoss
 * Initial orders supports new docking stuff Allender added.
 * 
 * 23    2/25/97 6:10p Hoffoss
 * Fixed bug with modeless dialog box errors on update.
 * 
 * 22    2/20/97 4:28p Hoffoss
 * Added modification tracking to ship editor dialog box, and support
 * functions.
 * 
 * 21    2/20/97 4:03p Hoffoss
 * Several ToDo items: new reinforcement clears arrival cue, reinforcement
 * control from ship and wing dialogs, show grid toggle.
 * 
 * 20    2/12/97 5:50p Hoffoss
 * Expanded on error checking.
 * 
 * 19    2/12/97 12:26p Hoffoss
 * Expanded on global error checker, added initial orders conflict
 * checking and warning, added waypoint editor dialog and code.
 * 
 * 18    2/06/97 3:42p Hoffoss
 * Added mission critical checking in deletion handler.
 * 
 * 17    2/04/97 3:09p Hoffoss
 * Background bitmap editor implemented fully.
 * 
 * 16    1/27/97 10:03a Hoffoss
 * 
 * 15    1/10/97 3:56p Hoffoss
 * Added a recursive menu graying function.
 * 
 * 14    1/02/97 3:50p Hoffoss
 * More fixes to player start support in Fred.
 * 
 * 13    12/11/96 3:29p Hoffoss
 * Worked on getting the Wing dialog changes updated when focus moves.
 * Works now.
 * 
 * 12    12/03/96 3:58p Hoffoss
 * Added Wing editor to Fred.
 * 
 * 11    11/20/96 10:01a Hoffoss
 * A few minor improvements.
 * 
 * 10    11/19/96 9:50a Hoffoss
 * New interface working, but not finished yet.
 * 
 * 9     11/15/96 1:43p Hoffoss
 * Improvements to the Ship Dialog editor window.  It is now an
 * independant window that updates data correctly.
 * 
 * 8     11/14/96 10:43a Hoffoss
 * Made changes to grid display and how it works, etc.
 * 
 * 7     11/13/96 10:15a Hoffoss
 * Waypoint editing added, but not quite finished yet.
 * 
 * 6     11/11/96 9:59a Hoffoss
 * Many great improvements.
 * 
 * 5     11/05/96 2:46p Hoffoss
 * Lots of Fred changes (getting ready for milestone)
 * 
 * 4     10/29/96 6:20p Hoffoss
 * Added ability to delete selected or marked ships.
 * 
 * 3     10/28/96 5:28p Hoffoss
 * Extensive rearrangement, modifications and fixes of the Editor system.
 * 
 * 2     10/25/96 12:05p Hoffoss
 * This new files added to makefile.
 * 
 * 1     10/25/96 12:02p Hoffoss
 *
 * $NoKeywords: $
 */

#ifndef __MANAGEMENT_H__
#define __MANAGEMENT_H__

#include <afxmt.h>
#include "globalincs/pstypes.h"
#include "ship/ship.h"
#include "ai/aigoals.h"

#define SHIP_FILTER_PLAYERS	(1<<0)  // set: add players to list as well
#define SHIP_FILTER_FLYABLE	(1<<1)  // set: filter out non-flyable ships

extern int cur_object_index;
extern int cur_ship;
extern int cur_wing;
extern int cur_wing_index;
extern int cur_model_index;
extern int cur_waypoint;
extern int cur_waypoint_list;
extern int Update_ship;
extern int Update_wing;
extern int Fred_font;

extern ai_goal_list Ai_goal_list[];
extern int Ai_goal_list_size;

// alternate ship name stuff
extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH+1];

extern int	wing_objects[MAX_WINGS][MAX_SHIPS_PER_WING];

extern char	*Docking_bay_list[];

extern char Fred_exe_dir[512];
extern char Fred_base_dir[512];

// Goober5000
extern bool Show_iff[];

extern CCriticalSection CS_cur_object_index;

void	string_copy(char *dest, CString &src, int max_len, int modify = 0);
CString convert_multiline_string(char *src);
void	deconvert_multiline_string(char *buf, CString &str, int max_len);
bool	fred_init();
void	set_physics_controls();
int	dup_object(object *objp);
int	create_object_on_grid(int list);
int	create_object(vec3d *pos, int list = cur_waypoint_list);
int	create_player(int num, vec3d *pos, matrix *orient, int type = -1, int init = 1);
void	create_new_mission();
void	reset_mission();
void	clear_mission();
int	query_valid_object(int index = cur_object_index);
int	query_valid_ship(int index = cur_object_index);
int	query_valid_waypoint(int index = cur_object_index);
void	set_cur_indices(int obj = -1);
void	set_cur_object_index(int obj = -1);
int	delete_object(int obj);
int	delete_object(object *ptr);
int	delete_ship(int ship);
void	delete_marked();
void	delete_reinforcement(int num);
int	delete_ship_from_wing(int ship = cur_ship);
int	find_free_wing();
void	add_ship_to_wing();
int	query_object_in_wing(int obj = cur_object_index);
void	mark_object(int obj);
void	unmark_object(int obj);
void	unmark_all();
void	clear_menu(CMenu *ptr);
void	generate_wing_popup_menu(CMenu *mptr, int first_id, int state);
void	generate_ship_popup_menu(CMenu *mptr, int first_id, int state, int filter = 0);
int	string_lookup(CString str1, char *strlist[], int max);
int	update_dialog_boxes();
void	set_cur_wing(int wing);
int	gray_menu_tree(CMenu *base);
int	query_initial_orders_conflict(int wing);
int	query_initial_orders_empty(ai_goal *ai_goals);
int	set_reinforcement(char *name, int state);
int	get_docking_list(int model_index);
int	rename_ship(int ship, char *name);
void	fix_ship_name(int ship);
int	internal_integrity_check();
void	correct_marking();
int	get_ship_from_obj(int obj);
int	get_ship_from_obj(object *objp);
void	set_valid_dock_points(int ship, int type, CComboBox *box);
void	ai_update_goal_references(int type, char *old_name, char *new_name);
int	query_referenced_in_ai_goals(int type, char *name);
int	advanced_stricmp(char *one, char *two);
int	reference_handler(char *name, int type, int obj);
int	orders_reference_handler(int code, char *msg);
int	sexp_reference_handler(int node, int code, char *msg);
char	*object_name(int obj);
char	*get_order_name(int order);
void	object_moved(object *ptr);
int	invalidate_references(char *name, int type);
int	query_whole_wing_marked(int wing);
void	generate_weaponry_usage_list(int *arr);
jump_node *jumpnode_get_by_name(CString& name);

// function and defines to use when adding ships to combo boxes
#define SHIPS_2_COMBO_SPECIAL					(1<<0)
#define SHIPS_2_COMBO_ALL_SHIPS				(1<<1)
#define SHIPS_2_COMBO_DOCKING_BAY_ONLY		(1<<2)

extern void management_add_ships_to_combo( CComboBox *box, int flags );

// Goober5000
extern int wing_is_player_wing(int wing);
extern void update_custom_wing_indexes();
extern void stuff_special_arrival_anchor_name(char *buf, int iff_index, int restrict_to_players, int retail_format);
extern void stuff_special_arrival_anchor_name(char *buf, int anchor_num, int retail_format);

#endif
