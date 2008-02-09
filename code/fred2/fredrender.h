/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/FredRender.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Handles all view rendering in FRED.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.4  2005/04/13 20:02:08  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.3  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:57  inquisitor
 * Initial FRED2 Checking
 *
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
 * 26    3/19/98 11:41a Hoffoss
 * Fixed problems with rendering and reading of flying controls in Fred.
 * 
 * 25    10/30/97 3:30p Hoffoss
 * Made anti-aliased gridlines an option in Fred.
 * 
 * 24    9/09/97 2:12p Hoffoss
 * Added code to allow briefing editor view to be a 1:1 pixel size fixed
 * as the FreeSpace view will have.
 * 
 * 23    8/25/97 5:58p Hoffoss
 * Created menu items for keypress functions in Fred, and fixed bug this
 * uncovered with wing_delete function.
 * 
 * 22    8/07/97 6:01p Hoffoss
 * Added a rotate about selected object button to toolbar and
 * functionality, as requested by Comet.
 * 
 * 21    8/06/97 6:10p Hoffoss
 * Changed Fred to display a forced aspect ratio while briefing editor is
 * open.  This aspect ratio is the same as the briefing view in FreeSpace,
 * so icons appear on and off screen the same as would be in FreeSpace.
 * 
 * 20    6/26/97 5:18p Hoffoss
 * Major rework of briefing editor functionality.
 * 
 * 19    6/23/97 3:00p Hoffoss
 * Added a define.
 * 
 * 18    6/23/97 2:58p Hoffoss
 * Added briefing lookat point variables.
 * 
 * 17    6/18/97 11:36p Lawrance
 * move grid rendering code to MissionGrid.cpp
 * 
 * 16    6/12/97 11:27a Lawrance
 * separating FRED dependant briefing code
 * 
 * 15    3/06/97 3:35p Hoffoss
 * Added Show_outline stuff, moved show options to the view menu, fixed a
 * bug in message dialog editor.
 * 
 * 14    2/20/97 4:03p Hoffoss
 * Several ToDo items: new reinforcement clears arrival cue, reinforcement
 * control from ship and wing dialogs, show grid toggle.
 * 
 * 13    12/02/96 3:36p Hoffoss
 * Show horizon now implemented.
 * 
 * 12    11/22/96 12:24p Hoffoss
 * Editor functionality added.
 * 
 * 11    11/21/96 12:52p Hoffoss
 * Changes to flying controls, etc.
 * 
 * 10    11/21/96 9:20a Hoffoss
 * New show distances feature.
 * 
 * 9     11/20/96 5:16p Hoffoss
 * New grid system working as suggested.
 * 
 * 8     11/20/96 10:01a Hoffoss
 * A few minor improvements.
 * 
 * 7     11/19/96 9:50a Hoffoss
 * New interface working, but not finished yet.
 * 
 * 6     11/15/96 1:43p Hoffoss
 * Improvements to the Ship Dialog editor window.  It is now an
 * independant window that updates data correctly.
 * 
 * 5     11/14/96 10:43a Hoffoss
 * Made changes to grid display and how it works, etc.
 * 
 * 4     11/13/96 10:15a Hoffoss
 * Waypoint editing added, but not quite finished yet.
 * 
 * 3     11/12/96 11:14a Hoffoss
 * Everything check in because I don't know what's changed and what's not
 * to prevent compiling.
 * 
 * 2     11/11/96 3:47p Hoffoss
 * Milestone Checkin.
 * 
 * 1     10/29/96 12:17p Hoffoss
 * 
 * $NoKeywords: $
 */

#include "mission/missiongrid.h"

#define BRIEFING_LOOKAT_POINT_ID	99999

extern int	Aa_gridlines;
extern int	player_start1;
extern int	Editing_mode;
extern int	Control_mode;
extern int	Show_grid;
extern int	Show_grid_positions;
extern int	Show_coordinates;
extern int	Show_outlines;
extern int	Single_axis_constraint;
extern int	Show_distances;
extern int	Universal_heading;
extern int	Flying_controls_mode;
extern int	Group_rotate;
extern int	Show_horizon;
extern int	Lookat_mode;
extern int	True_rw, True_rh;
extern int	Fixed_briefing_size;
extern vec3d	Constraint, Anticonstraint;
extern vec3d	Tp1, Tp2;  // test points
extern physics_info view_physics;
extern vec3d view_pos, eye_pos;
extern matrix view_orient, eye_orient;

void fred_render_init();
void generate_starfield();
void move_mouse(int btn, int mdx, int mdy);
void game_do_frame();
void render_frame();
void level_controlled();
void verticalize_controlled();
