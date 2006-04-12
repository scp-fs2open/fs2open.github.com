/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/fred2/FredRender.cpp $
 * $Revision: 1.3 $
 * $Date: 2006-04-12 05:30:50 $
 * $Author: phreak $
 *
 * Handles rendering the scene in the window for Fred.  Also handles several other
 * miscellaneous tasks.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/01/26 23:04:45  phreak
 * Grid zbuffering issue fix moved to the new fred project folder
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.21  2006/01/14 23:49:01  Goober5000
 * second pass; all the errors are fixed now; one more thing to take care of
 * --Goober5000
 *
 * Revision 1.20  2006/01/14 05:24:18  Goober5000
 * first pass at converting FRED to use new IFF stuff
 * --Goober5000
 *
 * Revision 1.19  2005/11/30 03:07:14  phreak
 * make sure the cube around the current subsystem is rotated
 *
 * Revision 1.18  2005/10/21 23:43:44  phreak
 * centralize htl on/off.  Also add another FOV setting when the briefing window is active
 *
 * Revision 1.17  2005/09/10 03:23:37  phreak
 * Modify subsystem wireframe outline to include child subsystems
 *
 * Revision 1.16  2005/09/10 01:53:43  phreak
 * fov tweaks for fred.  FOV has been changed to 0.485 which seems to stop all text movement.
 * Also set the max draw distance to 300 kilometers.
 *
 * Revision 1.15  2005/08/29 02:20:00  phreak
 * Grid lines now render in HTL correctly.  So does the currently selected subsystem box.
 *
 * Revision 1.14  2005/08/23 07:28:02  Goober5000
 * grammar
 * --Goober5000
 *
 * Revision 1.13  2005/06/20 16:07:24  phreak
 * added game_pause() and game_unpause() to fred stubs.
 * fixed models rendering out of position.
 *
 * Revision 1.12  2005/05/18 06:01:00  phreak
 * Briefing icons render in HTL now? Lets throw a party.
 *
 * Revision 1.11  2005/04/13 20:11:06  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.10  2005/03/29 03:43:11  phreak
 * ai directory fixes as well as fixes for the new jump node code
 *
 * Revision 1.9  2005/03/20 20:04:06  phreak
 * get distances drawing in HTL FRED again
 *
 * Revision 1.8  2005/03/19 23:37:00  phreak
 * Waypoints and Grid Position Markers draw correctly now
 *
 * Revision 1.7  2004/07/29 00:18:42  Kazan
 * taylor and I fixed fred2 by making code.lib and fred2's defines match up
 *
 * Revision 1.6  2004/03/15 12:14:46  randomtiger
 * Fixed a whole heap of problems with Fred introduced by changes to global vars.
 *
 * Revision 1.5  2004/02/13 04:25:37  randomtiger
 * Added messagebox to offer startup choice between htl, non htl and quitting.
 * Tidied up the background dialog box a bit and added some disabled controls for future ambient light feature.
 * Set non htl to use a lower level of model and texture detail to boost poor performance on this path.
 *
 * Revision 1.4  2004/02/05 23:23:19  randomtiger
 * Merging OGL Fred into normal Fred, no conflicts, should be perfect
 *
 * Revision 1.3.2.4  2004/01/24 13:00:43  randomtiger
 * Implemented basic OGL HT&L but some drawing routines missing from cobe.lib so keeping turned off for now.
 * Added lighting menu option and made about dialog more useful.
 * Ship menu now defaults to the first entry.
 * Loads of debug to try and trap crashes.
 *
 * Revision 1.3.2.3  2004/01/20 19:11:29  randomtiger
 * Cut unneeded code, text now works in graphics window.
 *
 * Revision 1.3.2.2  2004/01/19 00:53:37  randomtiger
 * More Fred OGL changes.
 *
 * Revision 1.3.2.1  2004/01/17 22:11:25  randomtiger
 * Large series of changes to make fred run though OGL. Requires code.lib to be up to date to compile.
 * Port still in progress, fred currently unusable, note this has been committed to fred_ogl branch.
 *
 * Revision 1.3  2002/08/15 04:35:44  penguin
 * Changes to build with fs2_open code.lib
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:57  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 6     4/07/99 6:21p Dave
 * Fred and Freespace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 5     3/26/99 4:49p Dave
 * Made cruisers able to dock with stuff. Made docking points and paths
 * visible in fred.
 * 
 * 4     2/07/99 8:51p Andsager
 * Add inner bound to asteroid field.  Inner bound tries to stay astroid
 * free.  Wrap when within and don't throw at ships inside.
 * 
 * 3     1/27/99 4:09p Andsager
 * Added highlight to ship subsystems
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:02p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 162   5/21/98 12:58a Hoffoss
 * Fixed warnings optimized build turned up.
 * 
 * 161   4/11/98 6:53p John
 * Added first rev of subspace effect.
 * 
 * 160   4/07/98 4:17p John
 * Made Fred be able to move suns.  Made suns actually affect the lighting
 * in the game.
 * 
 * 159   3/21/98 7:36p Lawrance
 * Move jump nodes to own lib.
 * 
 * 158   3/19/98 11:41a Hoffoss
 * Fixed problems with rendering and reading of flying controls in Fred.
 * 
 * 157   3/14/98 5:11p Hoffoss
 * Changed the starting camera position and orientation for Fred.
 * 
 * 156   3/10/98 4:24p Hoffoss
 * Made object detection under cursor use the object's radius if it has
 * one.
 * 
 * 155   3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 154   3/10/98 1:41p Sandeep
 * 
 * 153   3/09/98 10:56a Hoffoss
 * Added jump node objects to Fred.
 * 
 * 152   3/03/98 11:00a Andsager
 * Fixed fred bug with control object initialization.
 * 
 * 151   2/18/98 6:45p Hoffoss
 * Added support for lines between icons in briefings for Fred.
 * 
 * 150   2/12/98 3:48p Hoffoss
 * Reduced scaler for extra precision.
 * 
 * 149   2/10/98 6:43p Lawrance
 * Moved asteroid code to a separate lib.
 * 
 * 148   2/06/98 3:08p Mike
 * More asteroid stuff, including resolving conflicts between the two
 * asteroid_field structs!
 * 
 * 147   1/27/98 11:02a John
 * Added first rev of sparks.   Made all code that calls model_render call
 * clear_instance first.   Made debris pieces not render by default when
 * clear_instance is called.
 * 
 * 146   1/16/98 2:22p Hoffoss
 * Changed rendering colors of objects.
 * 
 * 145   12/29/97 5:09p Allender
 * fixed problems with speed not being reported properly in multiplayer
 * games.  Made read_flying_controls take frametime as a parameter.  More
 * ship/weapon select stuff
 * 
 * 144   10/30/97 3:30p Hoffoss
 * Made anti-aliased gridlines an option in Fred.
 * 
 * 143   10/29/97 5:13p John
 * Trying to fix my lighting bugs in Fred.
 * 
 * 142   10/03/97 9:48a John
 * took out alphacolors some grid lines don't draw antialiased.
 * 
 * 141   10/03/97 8:55a John
 * moved Fred's grid_render code out of MissionGrid and into Fred.   Added
 * code to turn background under overlays grey.
 * 
 * 140   9/18/97 9:59a John
 * fixed bug I caused in model_collide
 * 
 * 139   9/16/97 9:41p Hoffoss
 * Changed Fred code around to stop using Parse_player structure for
 * player information, and use actual ships instead.
 * 
 * 138   9/09/97 2:12p Hoffoss
 * Added code to allow briefing editor view to be a 1:1 pixel size fixed
 * as the FreeSpace view will have.
 * 
 * 137   9/06/97 2:13p Mike
 * Replace support for TEAM_NEUTRAL
 * 
 * 136   8/29/97 5:41p Johnson
 * Fixed bug with controlling marked objects without having done any
 * rotations yet of any other type.
 * 
 * 135   8/25/97 5:58p Hoffoss
 * Created menu items for keypress functions in Fred, and fixed bug this
 * uncovered with wing_delete function.
 * 
 * 134   8/19/97 5:46p Hoffoss
 * Changed font used in Fred, and added display to show current eye
 * position.
 * 
 * 133   8/19/97 11:23a Hoffoss
 * Fixes to briefing editor icon select code.
 * 
 * 132   8/17/97 10:22p Hoffoss
 * Fixed several bugs in Fred with Undo feature.  In the process, recoded
 * a lot of CFile.cpp.
 * 
 * 131   8/16/97 2:02a Hoffoss
 * Made docked objects move together in Fred.
 * 
 * 130   8/14/97 7:01p Hoffoss
 * Fixed bug with outline rendering while in background bitmap editing
 * mode.
 * 
 * 129   8/14/97 2:32p Hoffoss
 * fixed bug where controlling an object doesn't cause screen updates, and
 * added a number of cool features to viewpoint/control object code.
 * 
 * 128   8/13/97 1:38p Hoffoss
 * Added ability to update multiple times, which in needed in one case to
 * brute force redraw so deleted ships actually do get removed from the
 * display.
 * 
 * 127   8/12/97 1:55a Hoffoss
 * Made extensive changes to object reference checking and handling for
 * object deletion call.
 * 
 * 126   8/07/97 6:01p Hoffoss
 * Added a rotate about selected object button to toolbar and
 * functionality, as requested by Comet.
 * 
 * 125   8/06/97 7:55p Hoffoss
 * Fixed bug with objects not seeming to be where they are drawn (due to
 * new briefing clip render stuff).  This caused rendering problems,
 * though.  So I fixed those next.
 * 
 * 124   8/06/97 6:10p Hoffoss
 * Changed Fred to display a forced aspect ratio while briefing editor is
 * open.  This aspect ratio is the same as the briefing view in FreeSpace,
 * so icons appear on and off screen the same as would be in FreeSpace.
 * 
 * 123   7/24/97 10:24a Mike
 * Restore support for Unknown team
 * 
 * 122   7/17/97 1:34p Hoffoss
 * Fixed Fred to work with physics changes.
 * 
 * 121   6/26/97 6:03p Hoffoss
 * briefing icon selection now has higher priority of selection than other
 * objects.
 * 
 * 120   6/26/97 5:18p Hoffoss
 * Major rework of briefing editor functionality.
 * 
 * 119   6/26/97 4:20p Hoffoss
 * Changed the behavior of Ctrl-L
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "FREDView.h"
#include "MainFrm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "Management.h"
#include "math/vecmat.h"
#include "graphics/tmapper.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "model/model.h"
#include "bmpman/bmpman.h"
#include "io/key.h"
#include "physics/physics.h"
#include "math/floating.h"
#include "object/object.h"
#include "model/model.h"
#include "palman/palman.h"
#include "editor.h"
#include "ai/ailocal.h"
#include "ship/ship.h"
#include "cfile/cfile.h"
#include "mission/missionparse.h"
#include "globalincs/linklist.h"
#include "math/fvi.h"
#include "render/3dinternal.h"
#include "weapon/weapon.h"
#include "wing.h"
#include "FredRender.h"
#include <windows.h>
#include "starfield/starfield.h"
#include "io/timer.h"
#include "lighting/lighting.h"
#include "asteroid/asteroid.h"
#include "jumpnode/jumpnode.h"
#include "graphics/font.h"
#include "cmdline/cmdline.h"
#include "iff_defs/iff_defs.h"

extern float flFrametime;
extern subsys_to_render Render_subsys;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	MAX_FRAMETIME	(F1_0/4)		// Frametime gets saturated at this.
#define	MIN_FRAMETIME	(F1_0/120)
#define	LOLLIPOP_SIZE	2.5f

const float FRED_DEFAULT_HTL_FOV = 0.485f;
const float FRED_BRIEFING_HTL_FOV = 0.325f;
const float FRED_DEAFULT_HTL_DRAW_DIST = 300000.0f;

int	Aa_gridlines = 0;
int	Fred_outline = 0;
int	inited = -1;
int	player_start1;
int	Editing_mode = 1;
int	Control_mode = 0;
int	last_x=0, last_y=0;
int	Show_grid = 1;
int	Show_outlines = 0;
int	Show_stars = 0;
int	Show_grid_positions = 1;
int	Show_coordinates = 0;
int	Show_distances = 0;
int	Show_horizon = 0;
int	Show_asteroid_field = 1;
int	Lookat_mode = 0;
int	Single_axis_constraint = 0;
int	Universal_heading = 0;
int	Flying_controls_mode = 1;
int	Group_rotate = 1;
int	info_popup_active = 0;
int	rendering_order[MAX_SHIPS];
int	render_count = 0;
int	Last_cursor_over = -1;
int	True_rw, True_rh;
int	Fixed_briefing_size = 1;

fix		lasttime = 0;
vec3d	my_pos = {0.0f, 0.0f, -5.0f};
vec3d	view_pos, eye_pos, Viewer_pos, Last_eye_pos = { 0.0f };
vec3d	Last_control_pos = { 0.0f };
vec3d	Grid_center;
vec3d	Constraint = { 1.0f, 0.0f, 1.0f };
vec3d	Anticonstraint = { 0.0f, 1.0f, 0.0f };
vec3d	Tp1, Tp2;  // test points
matrix	Grid_gmatrix;
matrix	my_orient = IDENTITY_MATRIX;
matrix	trackball_orient = IDENTITY_MATRIX;
matrix	view_orient = IDENTITY_MATRIX, eye_orient, Last_eye_orient = IDENTITY_MATRIX;
matrix	Last_control_orient = IDENTITY_MATRIX;
physics_info view_physics;
control_info view_controls;
CWnd		info_popup;

static vec3d Global_light_world = { 0.208758f, -0.688253f, -0.694782f };

void display_distances();
void render_model_x(vec3d *pos, grid *gridp, int col_scheme = 0);
void render_model_x_htl(vec3d *pos, grid *gridp, int col_scheme = 0);
void draw_orient_sphere(object *obj, int r, int g, int b);
void draw_orient_sphere2(int col, object *obj, int r, int g, int b);
void render_compass(void);
void draw_compass_arrow(vec3d *v0);
void process_controls(vec3d *pos, matrix *orient, float frametime, int key, int mode = 0);
void render_one_model(object *objp);
void inc_mission_time();
void draw_asteroid_field();
void hilight_bitmap();

color colour_white;
color colour_grey;
color colour_black;

void fred_enable_htl()
{
	if (!Briefing_dialog) gr_set_proj_matrix( (4.0f/9.0f) * 3.14159f * FRED_DEFAULT_HTL_FOV,  gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height, 1.0f, FRED_DEAFULT_HTL_DRAW_DIST);
	if (Briefing_dialog) gr_set_proj_matrix( (4.0f/9.0f) * 3.14159f * FRED_BRIEFING_HTL_FOV,  gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height, 1.0f, FRED_DEAFULT_HTL_DRAW_DIST);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);
}

void fred_disable_htl()
{
	gr_end_proj_matrix();
	gr_end_view_matrix();
}

// Called every time a new mission is created (and erasing old mission from memory).
// New mission should be blank at this point.
void fred_render_init()
{							    
	vec3d f, u, r;

	physics_init(&view_physics);
	view_physics.max_vel.xyz.z = 5.0f;		//forward/backward
	view_physics.max_rotvel.xyz.x = 1.5f;		//pitch	
	memset(&view_controls, 0, sizeof(control_info));

	vm_vec_make(&view_pos, 0.0f, 150.0f, -200.0f);
	vm_vec_make(&f, 0.0f, -0.5f, 0.866025404f);  // 30 degree angle
	vm_vec_make(&u, 0.0f, 0.866025404f, 0.5f);
	vm_vec_make(&r, 1.0f, 0.0f, 0.0f);
	vm_vector_2_matrix(&view_orient, &f, &u, &r);

	The_grid = create_default_grid();
	maybe_create_new_grid(The_grid, &view_pos, &view_orient, 1);
//	vm_set_identity(&view_orient);

	gr_init_alphacolor( &colour_white, 255,255,255,255);
	gr_init_alphacolor( &colour_grey, 0,200,0,255);
	gr_init_alphacolor( &colour_black, 0,0,0,255);
}

void level_object(matrix *orient)
{
	vec3d u;

	u = orient->vec.uvec = The_grid->gmatrix.vec.uvec;
	if (u.xyz.x)  // y-z plane
	{
		orient->vec.fvec.xyz.x = orient->vec.rvec.xyz.x = 0.0f;

	} else if (u.xyz.y) {  // x-z plane
		orient->vec.fvec.xyz.y = orient->vec.rvec.xyz.y = 0.0f;

	} else if (u.xyz.z) {  // x-y plane
		orient->vec.fvec.xyz.z = orient->vec.rvec.xyz.z = 0.0f;
	}

	vm_fix_matrix(orient);
}

void level_controlled()
{
	int cmode, count = 0;
	object *objp;

	cmode = Control_mode;
	if ((viewpoint == 1) && !cmode)
		cmode = 2;

	switch (cmode) {
		case 0:		//	Control the viewer's location and orientation
			level_object(&view_orient);
			break;

		case 2:  // Control viewpoint object
			level_object(&Objects[view_obj].orient);
			object_moved(&Objects[view_obj]);
			set_modified();
			FREDDoc_ptr->autosave("level object");
			break;

		case 1:  //	Control the current object's location and orientation
			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if (objp->flags & OF_MARKED)
					level_object(&objp->orient);
				
				objp = GET_NEXT(objp);
			}

			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if (objp->flags & OF_MARKED) {
					object_moved(objp);
					count++;
				}
				
				objp = GET_NEXT(objp);
			}

			if (count) {
				if (count > 1)
					FREDDoc_ptr->autosave("level objects");
				else
					FREDDoc_ptr->autosave("level object");

				set_modified();
			}

			break;
	}

	return;
}

void align_vector_to_axis(vec3d *v)
{
	float x, y, z;

	x = v->xyz.x;
	if (x < 0)
		x = -x;

	y = v->xyz.y;
	if (y < 0)
		y = -y;

	z = v->xyz.z;
	if (z < 0)
		z = -z;

	if ((x > y) && (x > z)) {  // x axis
		if (v->xyz.x < 0)  // negative x
			vm_vec_make(v, -1.0f, 0.0f, 0.0f);
		else  // positive x
			vm_vec_make(v, 1.0f, 0.0f, 0.0f);

	} else if (y > z) {  // y axis
		if (v->xyz.y < 0)  // negative y
			vm_vec_make(v, 0.0f, -1.0f, 0.0f);
		else  // positive y
			vm_vec_make(v, 0.0f, 1.0f, 0.0f);

	} else {  // z axis
		if (v->xyz.z < 0)  // negative z
			vm_vec_make(v, 0.0f, 0.0f, -1.0f);
		else  // positive z
			vm_vec_make(v, 0.0f, 0.0f, 1.0f);
	}
}

void verticalize_object(matrix *orient)
{
	align_vector_to_axis(&orient->vec.fvec);
	align_vector_to_axis(&orient->vec.uvec);
	align_vector_to_axis(&orient->vec.rvec);
	vm_fix_matrix(orient);  // just in case something odd occurs.
}

void verticalize_controlled()
{
	int cmode, count = 0;
	object *objp;

	cmode = Control_mode;
	if ((viewpoint == 1) && !cmode)
		cmode = 2;

	switch (cmode) {
		case 0:		//	Control the viewer's location and orientation
			verticalize_object(&view_orient);
			break;

		case 2:  // Control viewpoint object
			verticalize_object(&Objects[view_obj].orient);
			object_moved(&Objects[view_obj]);
			FREDDoc_ptr->autosave("align object");
			set_modified();
			break;

		case 1:  //	Control the current object's location and orientation
			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if (objp->flags & OF_MARKED)
					verticalize_object(&objp->orient);
				
				objp = GET_NEXT(objp);
			}

			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if (objp->flags & OF_MARKED) {
					object_moved(objp);
					count++;
				}
				
				objp = GET_NEXT(objp);
			}

			if (count) {
				if (count > 1)
					FREDDoc_ptr->autosave("align objects");
				else
					FREDDoc_ptr->autosave("align object");

				set_modified();
			}

			break;
	}

	return;
}

void move_mouse( int btn, int mdx, int mdy )
{
	int dx, dy;

	dx = mdx - last_x;
	dy = mdy - last_y;
	last_x = mdx;
	last_y = mdy;

	if ( btn & 1 )	{
		matrix tempm, mousem;

		if ( dx || dy )	{
			vm_trackball( dx, dy, &mousem );
			vm_matrix_x_matrix(&tempm, &trackball_orient, &mousem);
			trackball_orient = tempm;
			view_orient = trackball_orient;
		}
	}

	if ( btn & 2 )	{
		my_pos.xyz.z += (float)dy;
	}
}

///////////////////////////////////////////////////
void process_system_keys(int key)
{
//	mprintf(("Key = %d\n", key));
	switch (key) {

	case KEY_LAPOSTRO:
		CFREDView::GetView()->cycle_constraint();
		break;

	case KEY_R:  // for some stupid reason, an accelerator for 'R' doesn't work.
		Editing_mode = 2;
		break;

	case KEY_SPACEBAR:
		Selection_lock = !Selection_lock;
		break;

	case KEY_ESC:
		if (button_down)
			cancel_drag();

		break;
	}
}

void render_waypoints(void)
{
	int i, j;
	vertex v;
	waypoint_list *ptr;

	for (i=0; i<Num_waypoint_lists; i++)
	{
		ptr = &Waypoint_lists[i];
		for (j=0; j<ptr->count; j++)
		{
			g3_rotate_vertex(&v, &ptr->waypoints[j]);
			if (!(v.codes & CC_BEHIND))
				if (!(g3_project_vertex(&v) & PF_OVERFLOW))
				{
					if (cur_waypoint_list == i && cur_waypoint == j)
						gr_set_color(255, 255, 255);
					else if (Waypoint_lists[i].flags[j] & WL_MARKED)
						gr_set_color(160, 255, 0);
					else
						gr_set_color(160, 96, 0);

					g3_draw_sphere(&v, LOLLIPOP_SIZE);
					if (j)
						gr_set_color(0, 0, 0);
					else
						gr_set_color(160, 96, 0);

					g3_draw_sphere(&v, LOLLIPOP_SIZE * 0.66667f);
					gr_set_color(160, 96, 0);
					g3_draw_sphere(&v, LOLLIPOP_SIZE * 0.33333f);
				}
		}

		for (j=0; j<ptr->count; j++)
			render_model_x(&ptr->waypoints[j], The_grid, 1);

		gr_set_color(160, 96, 0);
		for (j=1; j<ptr->count; j++)
			rpd_line(&ptr->waypoints[j-1], &ptr->waypoints[j]);
	}
}

// --------------------------------------------------------------------------------
// get_subsystem_world_pos2() returns the world position for a given subobject on a ship
//

vec3d* get_subsystem_world_pos2(object* parent_obj, ship_subsys* subsys, vec3d* world_pos)
{
	if (subsys == NULL) {
		*world_pos = parent_obj->pos;
		return world_pos;
	}
	
	vm_vec_unrotate(world_pos, &subsys->system_info->pnt, &parent_obj->orient);
	vm_vec_add2(world_pos, &parent_obj->pos);

	return world_pos;
}

// returns 1 for valid bounding rect, 0 otherwise
int get_subsys_bounding_rect(object *ship_obj, ship_subsys *subsys, int *x1, int *x2, int *y1, int *y2)
{
	if (subsys != NULL) {
		vertex subobj_vertex;
		vec3d	subobj_pos;

		get_subsystem_world_pos2(ship_obj, subsys, &subobj_pos);

		g3_rotate_vertex(&subobj_vertex, &subobj_pos);

		g3_project_vertex(&subobj_vertex);
		if (subobj_vertex.flags & PF_OVERFLOW)  // if overflow, no point in drawing brackets
			return 0;

		int bound_rc;

		bound_rc = subobj_find_2d_bound(subsys->system_info->radius, &ship_obj->orient, &subobj_pos, x1, y1, x2, y2);
		if ( bound_rc != 0 )
			return 0;

		return 1;
	}

	return 0;
}

void cancel_display_active_ship_subsystem()
{
	Render_subsys.do_render = false;
	Render_subsys.ship_obj = NULL;
	Render_subsys.cur_subsys = NULL;
}

void fredhtl_render_subsystem_bounding_box(subsys_to_render * s2r)
{

	vertex text_center;
	polymodel *pm = model_get(Ships[s2r->ship_obj->instance].modelnum);
	int subobj_num = s2r->cur_subsys->system_info->subobj_num;
	bsp_info *bsp = &pm->submodel[subobj_num];
	char buf[256];
	
	vec3d front_top_left = bsp->bounding_box[7];
	vec3d front_top_right = bsp->bounding_box[6];
	vec3d front_bot_left = bsp->bounding_box[4];
	vec3d front_bot_right = bsp->bounding_box[5];
	vec3d back_top_left  = bsp->bounding_box[3];
	vec3d back_top_right = bsp->bounding_box[2];
	vec3d back_bot_left = bsp->bounding_box[0];
	vec3d back_bot_right = bsp->bounding_box[1];

	gr_set_color(255, 32, 32);

	fred_enable_htl();

	//draw a cube around the subsystem
	g3_start_instance_matrix(&s2r->ship_obj->pos, &s2r->ship_obj->orient, true);
	g3_start_instance_matrix(&bsp->offset, &vmd_identity_matrix, true);
	g3_draw_htl_line(&front_top_left,  &front_top_right);
	g3_draw_htl_line(&front_top_right, &front_bot_right);
	g3_draw_htl_line(&front_bot_right, &front_bot_left);
	g3_draw_htl_line(&front_bot_left,  &front_top_left);
					
	g3_draw_htl_line(&back_top_left,  &back_top_right);
	g3_draw_htl_line(&back_top_right, &back_bot_right);
	g3_draw_htl_line(&back_bot_right, &back_bot_left);
	g3_draw_htl_line(&back_bot_left,  &back_top_left);

	g3_draw_htl_line(&front_top_left,  &back_top_left);
	g3_draw_htl_line(&front_top_right, &back_top_right);
	g3_draw_htl_line(&front_bot_left,  &back_bot_left);
	g3_draw_htl_line(&front_bot_right, &back_bot_right);


	//draw another cube around a gun for a two-part turret
	if ((s2r->cur_subsys->system_info->turret_gun_sobj >= 0) && (s2r->cur_subsys->system_info->turret_gun_sobj != s2r->cur_subsys->system_info->subobj_num))
	{
		bsp_info *bsp_turret = &pm->submodel[s2r->cur_subsys->system_info->turret_gun_sobj];
	
		vec3d front_top_left = bsp_turret->bounding_box[7];
		vec3d front_top_right = bsp_turret->bounding_box[6];
		vec3d front_bot_left = bsp_turret->bounding_box[4];
		vec3d front_bot_right = bsp_turret->bounding_box[5];
		vec3d back_top_left  = bsp_turret->bounding_box[3];
		vec3d back_top_right = bsp_turret->bounding_box[2];
		vec3d back_bot_left = bsp_turret->bounding_box[0];
		vec3d back_bot_right = bsp_turret->bounding_box[1];

		g3_start_instance_matrix(&bsp_turret->offset, &vmd_identity_matrix, true);

		g3_draw_htl_line(&front_top_left,  &front_top_right);
		g3_draw_htl_line(&front_top_right, &front_bot_right);
		g3_draw_htl_line(&front_bot_right, &front_bot_left);
		g3_draw_htl_line(&front_bot_left,  &front_top_left);
					
		g3_draw_htl_line(&back_top_left,  &back_top_right);
		g3_draw_htl_line(&back_top_right, &back_bot_right);
		g3_draw_htl_line(&back_bot_right, &back_bot_left);
		g3_draw_htl_line(&back_bot_left,  &back_top_left);

		g3_draw_htl_line(&front_top_left,  &back_top_left);
		g3_draw_htl_line(&front_top_right, &back_top_right);
		g3_draw_htl_line(&front_bot_left,  &back_bot_left);
		g3_draw_htl_line(&front_bot_right, &back_bot_right);

		g3_done_instance(true);
	}

	g3_done_instance(true);
	g3_done_instance(true);

	fred_disable_htl();
				
	//draw the text.  rotate the center of the subsystem into place before finding out where to put the text
	strcpy(buf, Render_subsys.cur_subsys->system_info->subobj_name);
	vec3d center_pt;
	vm_vec_unrotate(&center_pt, &bsp->offset, &s2r->ship_obj->orient);
	vm_vec_add2(&center_pt, &s2r->ship_obj->pos);
	g3_rotate_vertex(&text_center, &center_pt);
	g3_project_vertex(&text_center);
	gr_set_color_fast(&colour_white);
	gr_string_win( (int)text_center.sx,  (int)text_center.sy, buf);
}

void display_active_ship_subsystem()
{
	if (cur_object_index != -1)
	{
		if (Objects[cur_object_index].type == OBJ_SHIP)
		{

			object *objp = &Objects[cur_object_index];
			int x1, y1, x2, y2;
			char buf[256];

			// switching to a new ship, so reset
			if (objp != Render_subsys.ship_obj) {
				cancel_display_active_ship_subsystem();
				return;
			}

			if (Render_subsys.do_render) 
			{
				
				// get subsys name
				strcpy(buf, Render_subsys.cur_subsys->system_info->subobj_name);
	
				if (Cmdline_nohtl)
				{
					// get bounding box
					if ( get_subsys_bounding_rect(objp, Render_subsys.cur_subsys, &x1, &x2, &y1, &y2) )
					{
	
						// set color
						gr_set_color(255, 32, 32);
	
						// draw box
						gr_line(x1, y1, x1, y2);  gr_line(x1-1, y1, x1-1, y2);
						gr_line(x1, y2, x2, y2);  gr_line(x1, y2+1, x2, y2+1);
						gr_line(x2, y2, x2, y1);  gr_line(x2+1, y2, x2+1, y1);
						gr_line(x2, y1, x1, y1);  gr_line(x2, y1-1, x1, y1-1);

						// draw text
						gr_set_color_fast(&colour_white);
						gr_string_win( (x1+x2)/2,  y2 + 10, buf);
					}
				}
				else
				{		
					fredhtl_render_subsystem_bounding_box(&Render_subsys);
				}
			}
			else
			{
				cancel_display_active_ship_subsystem();
			}
		}
	}
}

void render_one_model_htl(object *objp);
void render_one_model_nohtl(object *objp);
void render_one_model_briefing_screen(object *objp);

void render_models(void)
{	   
	gr_set_color_fast(&colour_white);

	render_count = 0;

	bool f=false;
	if (Cmdline_nohtl)
	{
		obj_render_all(render_one_model_nohtl,&f);
	}
	else
	{
		fred_enable_htl();
		
		obj_render_all(render_one_model_htl,&f);

		fred_disable_htl();

	}

	if (Briefing_dialog)
	{
		obj_render_all(render_one_model_briefing_screen, &f);
		Briefing_dialog->batch_render();
	}

}

void render_one_model_briefing_screen(object *objp)
{
	if (objp->type == OBJ_POINT)
	{
		if (objp->instance != BRIEFING_LOOKAT_POINT_ID)
		{
			Assert(Briefing_dialog);
			Briefing_dialog->draw_icon(objp);
			render_model_x_htl(&objp->pos, The_grid);
			render_count++;
		}
			
	}
}

void render_one_model_nohtl(object *objp)
{
	int j, z;
	object *o2;

	Assert(objp->type != OBJ_NONE);

	if ( objp->type == OBJ_JUMP_NODE ) {
		return;
	}

	if ((objp->type == OBJ_WAYPOINT) && !Show_waypoints)
		return;

	if ((objp->type == OBJ_START) && !Show_starts)
		return;

	if (objp->type == OBJ_SHIP) {
		if (!Show_ships)
			return;

		if (!Show_iff[Ships[objp->instance].team])
			return;
	}

	if (objp->flags & OF_HIDDEN)
		return;

	rendering_order[render_count] = OBJ_INDEX(objp);
	Fred_outline = 0;
	if ((OBJ_INDEX(objp) == cur_object_index) && !Bg_bitmap_dialog)
		Fred_outline = 0xffffff;

	else if ((objp->flags & OF_MARKED) && !Bg_bitmap_dialog)  // is it a marked object?
		Fred_outline = 0x9fff00;

	else if ((objp->type == OBJ_SHIP) && Show_outlines) {
		color *iff_color = iff_get_color_by_team(Ships[objp->instance].team, -1, true);

		Fred_outline = (iff_color->red << 16) | (iff_color->green << 8) | (iff_color->blue);

	} else if ((objp->type == OBJ_START) && Show_outlines) {
		Fred_outline = 0x007f00;

	} else
		Fred_outline = 0;

	// build flags
	if ((Show_ship_models || Show_outlines) && ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START))){
		if (Show_ship_models){
			j = MR_NORMAL;
		} else {
			j = MR_NO_POLYS;
		}
		
		if(Show_dock_points){
			j |= MR_BAY_PATHS;	
		}

		if(Show_paths_fred){
			j |= MR_SHOW_PATHS;
		}

		z = objp->instance;

		model_clear_instance( Ships[z].modelnum );

//		if (!viewpoint || OBJ_INDEX(objp) != cur_object_index)
		{
			if (Fred_outline)	{
				model_set_outline_color(Fred_outline >> 16, (Fred_outline >> 8) & 0xff, Fred_outline & 0xff);
				model_render(Ships[z].modelnum, &objp->orient, &objp->pos, j | MR_SHOW_OUTLINE);
			} else {
				model_render(Ships[z].modelnum, &objp->orient, &objp->pos, j);
			}
		}
	
	} else {
		int r = 0, g = 0, b = 0;

		if (objp->type == OBJ_SHIP)
		{
			if (!Show_ships)
				return;

			color *iff_color = iff_get_color_by_team(Ships[objp->instance].team, -1, true);

			r = iff_color->red;
			g = iff_color->green;
			b = iff_color->blue;

		} else if (objp->type == OBJ_START) {
			r = 0;	g = 127;	b = 0;

		} else if (objp->type == OBJ_WAYPOINT) {
			r = 96;	g = 0;	b = 112;

		} else if (objp->type == OBJ_POINT) {
			if (objp->instance != BRIEFING_LOOKAT_POINT_ID)
			{
				return;
			}

			r = 196;	g = 32;	b = 196;

		} else
			Assert(0);

		if (Fred_outline)
			draw_orient_sphere2(Fred_outline, objp, r, g, b);
		else
			draw_orient_sphere(objp, r, g, b);
	}

	if (objp->type == OBJ_WAYPOINT)
	{
		for (j=0; j<render_count; j++)
		{
			o2 = &Objects[rendering_order[j]];
			if (o2->type == OBJ_WAYPOINT)
				if ((o2->instance == objp->instance - 1) || (o2->instance == objp->instance + 1))
					rpd_line(&o2->pos, &objp->pos);
		}
	}

	render_model_x(&objp->pos, The_grid);
	render_count++;
}

void render_one_model_htl(object *objp)
{
	int j, z;
	object *o2;

	Assert(objp->type != OBJ_NONE);

	if ( objp->type == OBJ_JUMP_NODE ) {
		return;
	}

	if ((objp->type == OBJ_WAYPOINT) && !Show_waypoints)
		return;

	if ((objp->type == OBJ_START) && !Show_starts)
		return;

	if (objp->type == OBJ_SHIP) {
		if (!Show_ships)
			return;

		if (!Show_iff[Ships[objp->instance].team])
			return;
	}

	if (objp->flags & OF_HIDDEN)
		return;

	rendering_order[render_count] = OBJ_INDEX(objp);
	Fred_outline = 0;
	if ((OBJ_INDEX(objp) == cur_object_index) && !Bg_bitmap_dialog)
		Fred_outline = 0xffffff;

	else if ((objp->flags & OF_MARKED) && !Bg_bitmap_dialog)  // is it a marked object?
		Fred_outline = 0x9fff00;

	else if ((objp->type == OBJ_SHIP) && Show_outlines) {
		color *iff_color = iff_get_color_by_team(Ships[objp->instance].team, -1, true);

		Fred_outline = (iff_color->red << 16) | (iff_color->green << 8) | (iff_color->blue);

	} else if ((objp->type == OBJ_START) && Show_outlines) {
		Fred_outline = 0x007f00;

	} else
		Fred_outline = 0;

	// build flags
	if ((Show_ship_models || Show_outlines) && ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START))){
		g3_start_instance_matrix(&Eye_position, &Eye_matrix, 0);
		if (Show_ship_models){
			j = MR_NORMAL;
		} else {
			j = MR_NO_POLYS;
		}
		
		if(Show_dock_points){
			j |= MR_BAY_PATHS;	
		}

		if(Show_paths_fred){
			j |= MR_SHOW_PATHS;
		}

		z = objp->instance;

		model_clear_instance( Ships[z].modelnum );

		if(!Lighting_on) {
			j |= MR_NO_LIGHTING;
			gr_set_lighting(false,false);
		}

		if (Fred_outline)	{
			model_set_outline_color(Fred_outline >> 16, (Fred_outline >> 8) & 0xff, Fred_outline & 0xff);
			j |= MR_SHOW_OUTLINE;
		}

		g3_done_instance(0);
	  	model_render(Ships[z].modelnum, &objp->orient, &objp->pos, j);
	} else {
		int r = 0, g = 0, b = 0;

		if (objp->type == OBJ_SHIP)
		{
			if (!Show_ships) {
				return;
			}

			color *iff_color = iff_get_color_by_team(Ships[objp->instance].team, -1, true);

			r = iff_color->red;
			g = iff_color->green;
			b = iff_color->blue;

		} else if (objp->type == OBJ_START) {
			r = 0;	g = 127;	b = 0;

		} else if (objp->type == OBJ_WAYPOINT) {
			r = 96;	g = 0;	b = 112;

		} else if (objp->type == OBJ_POINT) {
			if (objp->instance != BRIEFING_LOOKAT_POINT_ID) {
				Assert(Briefing_dialog);
				return;
			}

			r = 196;	g = 32;	b = 196;

		} else
			Assert(0);

		float size = fl_sqrt(vm_vec_dist(&eye_pos, &objp->pos) / 20.0f);
	
		if (size < LOLLIPOP_SIZE)
			size = LOLLIPOP_SIZE;

		if (Fred_outline)
		{
			gr_set_color(__min(r*2,255),__min(g*2,255),__min(b*2,255));
			g3_draw_htl_sphere(&objp->pos,  size * 1.5f);
		}
		else
		{
			gr_set_color(r,g,b);
			g3_draw_htl_sphere(&objp->pos, size);
		}
	}

	if (objp->type == OBJ_WAYPOINT)
	{
		for (j=0; j<render_count; j++)
		{
			o2 = &Objects[rendering_order[j]];
			if (o2->type == OBJ_WAYPOINT)
			{
				if ((o2->instance == objp->instance - 1) || (o2->instance == objp->instance + 1))
				{
					g3_draw_htl_line(&o2->pos, &objp->pos);
				}
			}
		}
	}

	render_model_x_htl(&objp->pos, The_grid);
	render_count++;
}

void display_distances()
{
	char buf[20];
	object *objp, *o2;
	vec3d pos;
	vertex v;


	gr_set_color(255, 0, 0);
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list))
	{
		if (objp->flags & OF_MARKED)
		{
			o2 = GET_NEXT(objp);
			while (o2 != END_OF_LIST(&obj_used_list))
			{
				if (o2->flags & OF_MARKED)
				{
					rpd_line(&objp->pos, &o2->pos);
					vm_vec_avg(&pos, &objp->pos, &o2->pos);
					g3_rotate_vertex(&v, &pos);
					if (!(v.codes & CC_BEHIND))
						if (!(g3_project_vertex(&v) & PF_OVERFLOW))	{
							sprintf(buf, "%.1f", vm_vec_dist(&objp->pos, &o2->pos));
							gr_set_color_fast(&colour_white);
							gr_string_win((int) v.sx, (int) v.sy, buf);
						}
				}

				o2 = GET_NEXT(o2);
			}
		}

		objp = GET_NEXT(objp);
	}
}

void display_ship_info()
{
	char buf[512], pos[80];
	int render = 1;
	object *objp;
	vertex	v;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list))	{
		Assert(objp->type != OBJ_NONE);
		Fred_outline = 0;
		render = 1;
		if (OBJ_INDEX(objp) == cur_object_index)
			Fred_outline = 0xffffff;
		else if (objp->flags & OF_MARKED)  // is it a marked object?
			Fred_outline = 0x9fff00;
		else
			Fred_outline = 0;

		if ((objp->type == OBJ_WAYPOINT) && !Show_waypoints)
			render = 0;

		if ((objp->type == OBJ_START) && !Show_starts)
			render = 0;
		
		if (objp->type == OBJ_SHIP) {
			if (!Show_ships)
				render = 0;

			if (!Show_iff[Ships[objp->instance].team])
				render = 0;
		}

		if (objp->flags & OF_HIDDEN)
			render = 0;
		
		g3_rotate_vertex(&v, &objp->pos);
		if (!(v.codes & CC_BEHIND) && render)
			if (!(g3_project_vertex(&v) & PF_OVERFLOW))	{
				*buf = 0;
				if (Show_ship_info)
				{
					if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
						ship *shipp;
						int ship_type;

						shipp = &Ships[objp->instance];
						ship_type = shipp->ship_info_index;
						ASSERT(ship_type >= 0);
						sprintf(buf, "%s\n%s", shipp->ship_name, Ship_info[ship_type].short_name);

					} else if (objp->type == OBJ_WAYPOINT) {
						sprintf(buf, "%s\nWaypoint %d",
							Waypoint_lists[objp->instance / 65536].name,
							(objp->instance & 0xffff) + 1);

					} else if (objp->type == OBJ_POINT) {
						if (objp->instance == BRIEFING_LOOKAT_POINT_ID)
							strcpy(buf, "Camera lookat point");
						else
							strcpy(buf, "Briefing icon");

					} else if (objp->type == OBJ_JUMP_NODE) {
						strcpy(buf, "Jump Node");
					} else
						Assert(0);
				}

				if (Show_coordinates)
				{
					sprintf(pos, "(%.0f,%.0f,%.0f)", objp->pos.xyz.x, objp->pos.xyz.y, objp->pos.xyz.z);
					if (*buf)
						strcat(buf, "\n");

					strcat(buf, pos);
				}

				if (*buf)
				{
					if (Fred_outline)
						gr_set_color_fast(&colour_grey);
					else
						gr_set_color_fast(&colour_white);

#if 0
						gr_set_color(Fred_outline >> 16, (Fred_outline >> 8) & 0xff, Fred_outline & 0xff);
					else
						gr_set_color(160, 160, 160);
#endif

					gr_string_win((int) v.sx, (int) v.sy, buf);
				}
			}

		objp = GET_NEXT(objp);
	}
}

void draw_orient_sphere(object *obj, int r, int g, int b)
{
	int		flag = 0;
	vertex	v;
	vec3d	v1, v2;
	float		size;

	size = fl_sqrt(vm_vec_dist(&eye_pos, &obj->pos) / 20.0f);
	if (size < LOLLIPOP_SIZE)
		size = LOLLIPOP_SIZE;

	if ((obj->type != OBJ_WAYPOINT) && (obj->type != OBJ_POINT))
	{
		flag = (vm_vec_dotprod(&eye_orient.vec.fvec, &obj->orient.vec.fvec) < 0.0f);
		v1 = v2 = obj->pos;
		vm_vec_scale_add2(&v1, &obj->orient.vec.fvec, size);
		vm_vec_scale_add2(&v2, &obj->orient.vec.fvec, size * 1.5f);

		if (!flag)	{
			gr_set_color(192, 192, 192);
			rpd_line(&v1, &v2);
		}
	}

	gr_set_color(r, g, b);
	g3_rotate_vertex(&v, &obj->pos);
	if (!(v.codes & CC_BEHIND))
		if (!(g3_project_vertex(&v) & PF_OVERFLOW))
			g3_draw_sphere(&v, size);

	if (flag)	{
		gr_set_color(192, 192, 192);
		rpd_line(&v1, &v2);
	}
}

void draw_orient_sphere2(int col, object *obj, int r, int g, int b)
{
	int		flag = 0;
	vertex	v;
	vec3d	v1, v2;
	float		size;

	size = fl_sqrt(vm_vec_dist(&eye_pos, &obj->pos) / 20.0f);
	if (size < LOLLIPOP_SIZE)
		size = LOLLIPOP_SIZE;

	if ((obj->type != OBJ_WAYPOINT) && (obj->type != OBJ_POINT))
	{
		flag = (vm_vec_dotprod(&eye_orient.vec.fvec, &obj->orient.vec.fvec) < 0.0f);

		v1 = v2 = obj->pos;
		vm_vec_scale_add2(&v1, &obj->orient.vec.fvec, size);
		vm_vec_scale_add2(&v2, &obj->orient.vec.fvec, size * 1.5f);

		if (!flag)	{
			gr_set_color(192, 192, 192);
			rpd_line(&v1, &v2);
		}
	}

	g3_rotate_vertex(&v, &obj->pos);
	if (!(v.codes & CC_BEHIND))
		if (!(g3_project_vertex(&v) & PF_OVERFLOW))
		{
			gr_set_color((col >> 16) & 0xff, (col >> 8) & 0xff, col & 0xff);
			g3_draw_sphere(&v, size);
			gr_set_color(r, g, b);
			g3_draw_sphere(&v, size * 0.75f);
		}

	if (flag)	{
		gr_set_color(192, 192, 192);
		rpd_line(&v1, &v2);
	}
}

void render_model_x(vec3d *pos, grid *gridp, int col_scheme)
{
	vec3d	gpos;	//	Location of point on grid.
	vec3d	tpos;
	float	dxz;
	plane	tplane;
	vec3d	*gv;

	if (!Show_grid_positions)
		return;

	tplane.A = gridp->gmatrix.vec.uvec.xyz.x;
	tplane.B = gridp->gmatrix.vec.uvec.xyz.y;
	tplane.C = gridp->gmatrix.vec.uvec.xyz.z;
	tplane.D = gridp->planeD;

	compute_point_on_plane(&gpos, &tplane, pos);
	dxz = vm_vec_dist(pos, &gpos)/8.0f;
	gv = &gridp->gmatrix.vec.uvec;
	if (gv->xyz.x * pos->xyz.x + gv->xyz.y * pos->xyz.y + gv->xyz.z * pos->xyz.z < -gridp->planeD)
		gr_set_color(0, 127, 0);
	else
		gr_set_color(192, 192, 192);


	rpd_line(&gpos, pos);	//	Line from grid to object center.

	tpos = gpos;

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, -dxz/2);
	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.fvec, -dxz/2);
	
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, dxz/2);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.fvec, dxz/2);
	
	rpd_line(&gpos, &tpos);

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, dxz);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, -dxz);

	rpd_line(&gpos, &tpos);
}

void render_model_x_htl(vec3d *pos, grid *gridp, int col_scheme)
{
	vec3d	gpos;	//	Location of point on grid.
	vec3d	tpos;
	float	dxz;
	plane	tplane;
	vec3d	*gv;

	if (!Show_grid_positions)
		return;

	tplane.A = gridp->gmatrix.vec.uvec.xyz.x;
	tplane.B = gridp->gmatrix.vec.uvec.xyz.y;
	tplane.C = gridp->gmatrix.vec.uvec.xyz.z;
	tplane.D = gridp->planeD;

	compute_point_on_plane(&gpos, &tplane, pos);
	dxz = vm_vec_dist(pos, &gpos)/8.0f;
	gv = &gridp->gmatrix.vec.uvec;
	if (gv->xyz.x * pos->xyz.x + gv->xyz.y * pos->xyz.y + gv->xyz.z * pos->xyz.z < -gridp->planeD)
		gr_set_color(0, 127, 0);
	else
		gr_set_color(192, 192, 192);


	g3_draw_htl_line(&gpos, pos);	//	Line from grid to object center.

	tpos = gpos;

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, -dxz/2);
	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.fvec, -dxz/2);
	
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, dxz/2);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.fvec, dxz/2);
	
	g3_draw_htl_line(&gpos, &tpos);

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, dxz);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, -dxz);

	g3_draw_htl_line(&gpos, &tpos);
}

void render_active_rect(void)
{
	if (box_marking) {
		gr_set_color(255, 255, 255);
		gr_line(marking_box.x1, marking_box.y1, marking_box.x1, marking_box.y2);
		gr_line(marking_box.x1, marking_box.y2, marking_box.x2, marking_box.y2);
		gr_line(marking_box.x2, marking_box.y2, marking_box.x2, marking_box.y1);
		gr_line(marking_box.x2, marking_box.y1, marking_box.x1, marking_box.y1);
	}
}


void process_movement_keys(int key, vec3d *mvec, angles *angs)
{
	int	raw_key;

	mvec->xyz.x = 0.0f;
	mvec->xyz.y = 0.0f;
	mvec->xyz.z = 0.0f;
	angs->p = 0.0f;
	angs->b = 0.0f;
	angs->h = 0.0f;

	raw_key = key & 0xff;

	switch (raw_key) {
	case KEY_PAD1:		mvec->xyz.x += -1.0f;	break;
	case KEY_PAD3:		mvec->xyz.x += +1.0f;	break;
	case KEY_PADPLUS:	mvec->xyz.y += -1.0f;	break;
	case KEY_PADMINUS:	mvec->xyz.y += +1.0f;	break;
	case KEY_A:			mvec->xyz.z += +1.0f;	break;
	case KEY_Z:			mvec->xyz.z += -1.0f;	break;
	case KEY_PAD4:		angs->h += -0.1f;	break;
	case KEY_PAD6:		angs->h += +0.1f;	break;
	case KEY_PAD8:		angs->p += -0.1f;	break;
	case KEY_PAD2:		angs->p += +0.1f;	break;
	case KEY_PAD7:		angs->b += -0.1f;	break;
	case KEY_PAD9:		angs->b += +0.1f;	break;

	}

	if (key & KEY_SHIFTED) {
		vm_vec_scale(mvec, 5.0f);
		angs->p *= 5.0f;
		angs->b *= 5.0f;
		angs->h *= 5.0f;
	}
}

void process_controls(vec3d *pos, matrix *orient, float frametime, int key, int mode)
{
	if (Flying_controls_mode)	{
		grid_read_camera_controls(&view_controls, frametime);

		if (key_get_shift_status())
			memset(&view_controls, 0, sizeof(control_info));

		if ((fabs(view_controls.pitch) > (frametime / 100)) &&
			(fabs(view_controls.vertical) > (frametime / 100)) &&
			(fabs(view_controls.heading) > (frametime / 100)) &&
			(fabs(view_controls.sideways) > (frametime / 100)) &&
			(fabs(view_controls.bank) > (frametime / 100)) &&
			(fabs(view_controls.forward) > (frametime / 100)))
				Update_window = 1;

		flFrametime = frametime;
		physics_read_flying_controls(orient, &view_physics, &view_controls, flFrametime);
		if (mode)
			physics_sim_editor(pos, orient, &view_physics, frametime);
		else
			physics_sim(pos, orient, &view_physics, frametime);

	} else {
		vec3d		movement_vec, rel_movement_vec;
		angles		rotangs;
		matrix		newmat, rotmat;

		process_movement_keys(key, &movement_vec, &rotangs);
		vm_vec_rotate(&rel_movement_vec, &movement_vec, &The_grid->gmatrix);
		vm_vec_add2(pos, &rel_movement_vec);

		vm_angles_2_matrix(&rotmat, &rotangs);
		if (rotangs.h && Universal_heading)
			vm_transpose_matrix(orient);
		vm_matrix_x_matrix(&newmat, orient, &rotmat);
		*orient = newmat;
		if (rotangs.h && Universal_heading)
			vm_transpose_matrix(orient);
	}
}

int Fred_grid_colors_inited = 0;
color Fred_grid_bright, Fred_grid_dark, Fred_grid_bright_aa, Fred_grid_dark_aa;

//	Renders a grid defined in a grid struct
void fred_render_grid(grid *gridp)
{
	int	i, ncols, nrows;

	if (!Cmdline_nohtl)
	{
		fred_enable_htl();
		gr_zbuffer_set(0);
	}	
	
	if ( !Fred_grid_colors_inited )	{
		Fred_grid_colors_inited = 1;

		gr_init_alphacolor( &Fred_grid_dark_aa, 64, 64, 64, 255 );
		gr_init_alphacolor( &Fred_grid_bright_aa, 128, 128, 128, 255 );
		gr_init_color( &Fred_grid_dark, 64, 64, 64 );
		gr_init_color( &Fred_grid_bright, 128, 128, 128 );
	}

	ncols = gridp->ncols;
	nrows = gridp->nrows;
	if (double_fine_gridlines) {
		ncols *= 2;
		nrows *= 2;
	}

	if (Aa_gridlines)
		gr_set_color_fast(&Fred_grid_dark_aa);
	else
		gr_set_color_fast(&Fred_grid_dark);

	//	Draw the column lines.
	for (i=0; i<=ncols; i++)
	{
		if (Cmdline_nohtl) rpd_line(&gridp->gpoints1[i], &gridp->gpoints2[i]);
		else g3_draw_htl_line(&gridp->gpoints1[i], &gridp->gpoints2[i]);
	}
	//	Draw the row lines.
	for (i=0; i<=nrows; i++)
	{
		if (Cmdline_nohtl) rpd_line(&gridp->gpoints3[i], &gridp->gpoints4[i]);
		else g3_draw_htl_line(&gridp->gpoints3[i], &gridp->gpoints4[i]);
	}

	ncols = gridp->ncols / 2;
	nrows = gridp->nrows / 2;

	// now draw the larger, brighter gridlines that is x10 the scale of smaller one.
	if (Aa_gridlines)
		gr_set_color_fast(&Fred_grid_bright_aa);
	else
		gr_set_color_fast(&Fred_grid_bright);
	
	for (i=0; i<=ncols; i++)
	{
		if (Cmdline_nohtl) rpd_line(&gridp->gpoints5[i], &gridp->gpoints6[i]);
		else g3_draw_htl_line(&gridp->gpoints5[i], &gridp->gpoints6[i]);
	}

	for (i=0; i<=nrows; i++)
	{
		if (Cmdline_nohtl) rpd_line(&gridp->gpoints7[i], &gridp->gpoints8[i]);
		else g3_draw_htl_line(&gridp->gpoints7[i], &gridp->gpoints8[i]);
	}

	if (!Cmdline_nohtl)
	{
		fred_disable_htl();
		gr_zbuffer_set(1);
	}
}

void render_frame()
{
	char buf[256];
	int x, y, w, h, inst;
	vec3d pos;
	vertex v;
	angles a;

	g3_end_frame();	 // ** Accounted for

	gr_reset_clip();
	gr_clear();

	if (Briefing_dialog) {
		CRect rect;

		Fred_main_wnd->GetClientRect(rect);
		True_rw = rect.Width();
		True_rh = rect.Height();
		if (Fixed_briefing_size) {
			True_rw = BRIEF_GRID_W;
			True_rh = BRIEF_GRID_H;

		} else {
			if ((float) True_rh / (float) True_rw > (float) BRIEF_GRID_H / (float) BRIEF_GRID_W) {
				True_rh = (int) ((float) BRIEF_GRID_H * (float) True_rw / (float) BRIEF_GRID_W);

			} else {  // Fred is wider than briefing window
				True_rw = (int) ((float) BRIEF_GRID_W * (float) True_rh / (float) BRIEF_GRID_H);
			}
		}

		g3_start_frame(0); // ** Accounted for
		gr_set_color(255, 255, 255);
		gr_line(0, True_rh, True_rw, True_rh);
		gr_line(True_rw, 0, True_rw, True_rh);
		g3_end_frame();	 // ** Accounted for
		gr_set_clip(0, 0, True_rw, True_rh);
	}

	g3_start_frame(1);  // ** Accounted for
	// 1 means use zbuffering
// RT FONT	gr_set_font(Fred_font);
	gr_set_font(FONT1);
	light_reset();

	g3_set_view_matrix(&eye_pos, &eye_orient, 0.5f);
	Viewer_pos = eye_pos;  // for starfield code
	
	if ( Bg_bitmap_dialog )	{
		stars_draw( Show_stars, 1, Show_stars, 0, 0 );
	} else {
		stars_draw( Show_stars, Show_stars, Show_stars, 0, 0 );
	}

	if (Show_horizon) {
		gr_set_color(128, 128, 64);
		g3_draw_horizon_line();
	}

	if (Show_asteroid_field) {
		gr_set_color(192, 96, 16);
		draw_asteroid_field();
	}

	if (Show_grid)
		fred_render_grid(The_grid);
	if (Bg_bitmap_dialog)
		hilight_bitmap();

	gr_set_color(0, 0, 64);
	render_models();

  	if (Show_distances)
	{
		display_distances();	
	}

	display_ship_info();
	display_active_ship_subsystem();
	render_active_rect();

	if (query_valid_object(Cursor_over)) {  // display a tool-tip like infobox
		pos = Objects[Cursor_over].pos;
		inst = Objects[Cursor_over].instance;
		if ((Objects[Cursor_over].type == OBJ_SHIP) || (Objects[Cursor_over].type == OBJ_START)) {
			vm_extract_angles_matrix(&a, &Objects[Cursor_over].orient);
			sprintf(buf, "%s\n%s\n( %.1f , %.1f , %.1f ) \nHeading: %.2f\nPitch: %.2f\nBank: %.2f",
				Ships[inst].ship_name, Ship_info[Ships[inst].ship_info_index].short_name,
				pos.xyz.x, pos.xyz.y, pos.xyz.z, a.h, a.p, a.b);

		} else if (Objects[Cursor_over].type == OBJ_WAYPOINT) {
			sprintf(buf, "%s\nWaypoint %d\n( %.1f , %.1f , %.1f ) ",
				Waypoint_lists[inst / 65536].name, (inst & 0xffff) + 1, pos.xyz.x, pos.xyz.y, pos.xyz.z);

		} else if (Objects[Cursor_over].type == OBJ_POINT) {
			sprintf(buf, "Briefing icon\n( %.1f , %.1f , %.1f ) ", pos.xyz.x, pos.xyz.y, pos.xyz.z);

		} else
			sprintf(buf, "( %.1f , %.1f , %.1f ) ", pos.xyz.x, pos.xyz.y, pos.xyz.z);

		g3_rotate_vertex(&v, &pos);
	 	if (!(v.codes & CC_BEHIND))
	 		if (!(g3_project_vertex(&v) & PF_OVERFLOW))	{

				gr_get_string_size(&w, &h, buf);

				x = (int) v.sx;
				y = (int) v.sy + 20;

				gr_set_color_fast(&colour_white);
				gr_rect(x-7, y-6, w+8, h+7);

				gr_set_color_fast(&colour_black);
				gr_rect(x-5, y-5, w+5, h+5);

				gr_set_color_fast(&colour_white);
				gr_string_win(x, y, buf);
			}
	}

	gr_set_color(0, 160, 0);

	fred_enable_htl();
	jumpnode_render_all();
	fred_disable_htl();

	sprintf(buf, "(%.1f,%.1f,%.1f)", eye_pos.xyz.x, eye_pos.xyz.y, eye_pos.xyz.z);
	gr_get_string_size(&w, &h, buf);
	gr_set_color_fast(&colour_white);
	gr_string_win(gr_screen.max_w - w - 2, 2, buf);

	g3_end_frame();	 // ** Accounted for
	render_compass();

	gr_flip();

	gr_reset_clip();
	if (Briefing_dialog)
		gr_set_clip(0, 0, True_rw, True_rh);

	g3_start_frame(0);	 // ** Accounted for
	g3_set_view_matrix(&eye_pos, &eye_orient, 0.5f);
}

void game_do_frame()
{
	int key, cmode;
	vec3d viewer_position, control_pos;
	object *objp;
	matrix control_orient;

	inc_mission_time();

	viewer_position = my_orient.vec.fvec;
	vm_vec_scale(&viewer_position,my_pos.xyz.z);

	if ((viewpoint == 1) && !query_valid_object(view_obj))
		viewpoint = 0;

	key = key_inkey();
	process_system_keys(key);
	cmode = Control_mode;
	if ((viewpoint == 1) && !cmode)
		cmode = 2;

	control_pos = Last_control_pos;
	control_orient = Last_control_orient;

//	if ((key & KEY_MASK) == key)  // unmodified
		switch (cmode) {
			case 0:		//	Control the viewer's location and orientation
				process_controls(&view_pos, &view_orient, f2fl(Frametime), key, 1);
				control_pos = view_pos;
				control_orient = view_orient;
				break;

			case 2:  // Control viewpoint object
				process_controls(&Objects[view_obj].pos, &Objects[view_obj].orient, f2fl(Frametime), key);
				object_moved(&Objects[view_obj]);
				control_pos = Objects[view_obj].pos;
				control_orient = Objects[view_obj].orient;
				break;

			case 1:  //	Control the current object's location and orientation
				if (query_valid_object()) {
					vec3d delta_pos, leader_old_pos;
					matrix leader_orient, leader_transpose, tmp;
					object *leader;
					
					leader = &Objects[cur_object_index];
					leader_old_pos = leader->pos;  // save original position
					leader_orient = leader->orient;			// save original orientation
					vm_copy_transpose_matrix(&leader_transpose, &leader_orient);

					process_controls(&leader->pos, &leader->orient, f2fl(Frametime), key);
					vm_vec_sub(&delta_pos, &leader->pos, &leader_old_pos);  // get position change
					control_pos = leader->pos;
					control_orient = leader->orient;

					objp = GET_FIRST(&obj_used_list);
					while (objp != END_OF_LIST(&obj_used_list))			{
						Assert(objp->type != OBJ_NONE);
						if ((objp->flags & OF_MARKED) && (cur_object_index != OBJ_INDEX(objp)))	{
							if (Group_rotate) {
								matrix rot_trans;
								vec3d tmpv1, tmpv2;

								// change rotation matrix to rotate in opposite direction.  This rotation
								// matrix is what the leader ship has rotated by.
								vm_copy_transpose_matrix(&rot_trans, &view_physics.last_rotmat);

								// get point relative to our point of rotation (make POR the origin).  Since
								// only the leader has been moved yet, and not the objects, we have to use
								// the old leader's position.
								vm_vec_sub(&tmpv1, &objp->pos, &leader_old_pos);

								// convert point from real-world coordinates to leader's relative coordinate
								// system (z=forward vec, y=up vec, x=right vec
								vm_vec_rotate(&tmpv2, &tmpv1, &leader_orient);

								// now rotate the point by the transpose from above.
								vm_vec_rotate(&tmpv1, &tmpv2, &rot_trans);

								// convert point back into real-world coordinates
								vm_vec_rotate(&tmpv2, &tmpv1, &leader_transpose);

								// and move origin back to real-world origin.  Object is now at its correct
								// position.  Note we used the leader's new position, instead of old position.
								vm_vec_add(&objp->pos, &leader->pos, &tmpv2);

								// Now fix the object's orientation to what it should be.
								vm_matrix_x_matrix(&tmp, &objp->orient, &view_physics.last_rotmat);
								vm_orthogonalize_matrix(&tmp);  // safety check
								objp->orient = tmp;

							} else {
								vm_vec_add2(&objp->pos, &delta_pos);
								vm_matrix_x_matrix(&tmp, &objp->orient, &view_physics.last_rotmat);
								objp->orient = tmp;
							}
						}
						
						objp = GET_NEXT(objp);
					}

					objp = GET_FIRST(&obj_used_list);
					while (objp != END_OF_LIST(&obj_used_list)) {
						if (objp->flags & OF_MARKED)
							object_moved(objp);
						
						objp = GET_NEXT(objp);
					}

					set_modified();
				}

				break;
			
			default:
				Assert(0);
		}

	if (Lookat_mode && query_valid_object()) {
		float dist;

		dist = vm_vec_dist(&view_pos, &Objects[cur_object_index].pos);
		vm_vec_scale_add(&view_pos, &Objects[cur_object_index].pos, &view_orient.vec.fvec, -dist);
	}

	switch (viewpoint)
	{
		case 0:
			eye_pos = view_pos;
			eye_orient = view_orient;
			break;

		case 1:
			eye_pos = Objects[view_obj].pos;
			eye_orient = Objects[view_obj].orient;
			break;

		default:
			Assert(0);
	}

	maybe_create_new_grid(The_grid, &eye_pos, &eye_orient);

	if (Cursor_over != Last_cursor_over) {
		Last_cursor_over = Cursor_over;
		Update_window = 1;
	}

	// redraw screen if controlled object moved or rotated
	if (vm_vec_cmp(&control_pos, &Last_control_pos) || vm_matrix_cmp(&control_orient, &Last_control_orient)) {
		Update_window = 1;
		Last_control_pos = control_pos;
		Last_control_orient = control_orient;
	}

	// redraw screen if current viewpoint moved or rotated
	if (vm_vec_cmp(&eye_pos, &Last_eye_pos) || vm_matrix_cmp(&eye_orient, &Last_eye_orient)) {
		Update_window = 1;
		Last_eye_pos = eye_pos;
		Last_eye_orient = eye_orient;
	}
}

void hilight_bitmap()
{
	/*
	int i;
	vertex p[4];

	if (Starfield_bitmaps[Cur_bitmap].bitmap_index == -1)  // can't draw if no bitmap
		return;

	for (i=0; i<4; i++)
	{
		g3_rotate_faraway_vertex(&p[i], &Starfield_bitmaps[Cur_bitmap].points[i]);
		if (p[i].codes & CC_BEHIND)
			return;

		g3_project_vertex(&p[i]);
		if (p[i].flags & PF_OVERFLOW)
			return;
	}

	gr_set_color(255, 255, 255);
	g3_draw_line(&p[0], &p[1]);
	g3_draw_line(&p[1], &p[2]);
	g3_draw_line(&p[2], &p[3]);
	g3_draw_line(&p[3], &p[0]);
	*/
}

void draw_asteroid_field()
{
	int i, j;
	vec3d p[8], ip[8];
	vertex v[8], iv[8];

	for (i=0; i<1 /*MAX_ASTEROID_FIELDS*/; i++)
		if (Asteroid_field.num_initial_asteroids) {
			p[0].xyz.x = p[2].xyz.x = p[4].xyz.x = p[6].xyz.x = Asteroid_field.min_bound.xyz.x;
			p[1].xyz.x = p[3].xyz.x = p[5].xyz.x = p[7].xyz.x = Asteroid_field.max_bound.xyz.x;
			p[0].xyz.y = p[1].xyz.y = p[4].xyz.y = p[5].xyz.y = Asteroid_field.min_bound.xyz.y;
			p[2].xyz.y = p[3].xyz.y = p[6].xyz.y = p[7].xyz.y = Asteroid_field.max_bound.xyz.y;
			p[0].xyz.z = p[1].xyz.z = p[2].xyz.z = p[3].xyz.z = Asteroid_field.min_bound.xyz.z;
			p[4].xyz.z = p[5].xyz.z = p[6].xyz.z = p[7].xyz.z = Asteroid_field.max_bound.xyz.z;

			for (j=0; j<8; j++)
				g3_rotate_vertex(&v[j], &p[j]);

			g3_draw_line(&v[0], &v[1]);
			g3_draw_line(&v[2], &v[3]);
			g3_draw_line(&v[4], &v[5]);
			g3_draw_line(&v[6], &v[7]);
			g3_draw_line(&v[0], &v[2]);
			g3_draw_line(&v[1], &v[3]);
			g3_draw_line(&v[4], &v[6]);
			g3_draw_line(&v[5], &v[7]);
			g3_draw_line(&v[0], &v[4]);
			g3_draw_line(&v[1], &v[5]);
			g3_draw_line(&v[2], &v[6]);
			g3_draw_line(&v[3], &v[7]);


			// maybe draw inner box
			if (Asteroid_field.has_inner_bound) {

				gr_set_color(16, 192, 92);

				ip[0].xyz.x = ip[2].xyz.x = ip[4].xyz.x = ip[6].xyz.x = Asteroid_field.inner_min_bound.xyz.x;
				ip[1].xyz.x = ip[3].xyz.x = ip[5].xyz.x = ip[7].xyz.x = Asteroid_field.inner_max_bound.xyz.x;
				ip[0].xyz.y = ip[1].xyz.y = ip[4].xyz.y = ip[5].xyz.y = Asteroid_field.inner_min_bound.xyz.y;
				ip[2].xyz.y = ip[3].xyz.y = ip[6].xyz.y = ip[7].xyz.y = Asteroid_field.inner_max_bound.xyz.y;
				ip[0].xyz.z = ip[1].xyz.z = ip[2].xyz.z = ip[3].xyz.z = Asteroid_field.inner_min_bound.xyz.z;
				ip[4].xyz.z = ip[5].xyz.z = ip[6].xyz.z = ip[7].xyz.z = Asteroid_field.inner_max_bound.xyz.z;

				for (j=0; j<8; j++)
					g3_rotate_vertex(&iv[j], &ip[j]);

				g3_draw_line(&iv[0], &iv[1]);
				g3_draw_line(&iv[2], &iv[3]);
				g3_draw_line(&iv[4], &iv[5]);
				g3_draw_line(&iv[6], &iv[7]);
				g3_draw_line(&iv[0], &iv[2]);
				g3_draw_line(&iv[1], &iv[3]);
				g3_draw_line(&iv[4], &iv[6]);
				g3_draw_line(&iv[5], &iv[7]);
				g3_draw_line(&iv[0], &iv[4]);
				g3_draw_line(&iv[1], &iv[5]);
				g3_draw_line(&iv[2], &iv[6]);
				g3_draw_line(&iv[3], &iv[7]);
			}

		}
}

//	See if object "objnum" obstructs the vector from point p0 to point p1.
//	If so, return true and stuff hit point in *hitpos.
//	If not, return false.
int object_check_collision(object *objp, vec3d *p0, vec3d *p1, vec3d *hitpos)
{
	mc_info mc;

	if ((objp->type == OBJ_NONE) || (objp->type == OBJ_POINT))
		return 0;

	if ((objp->type == OBJ_WAYPOINT) && !Show_waypoints)
		return 0;

	if ((objp->type == OBJ_START) && !Show_starts)
		return 0;

	if (objp->type == OBJ_SHIP) {
		if (!Show_ships)
			return 0;

		if (!Show_iff[Ships[objp->instance].team])
			return 0;
	}

	if (objp->flags & OF_HIDDEN)
		return 0;

	if ((Show_ship_models || Show_outlines) && (objp->type == OBJ_SHIP))	{
		mc.model_num = Ships[objp->instance].modelnum;			// Fill in the model to check

	} else if ((Show_ship_models || Show_outlines) && (objp->type == OBJ_START))	{
		mc.model_num = Ships[objp->instance].modelnum;			// Fill in the model to check

	} else
		return fvi_ray_sphere(hitpos, p0, p1, &objp->pos, (objp->radius > 0.1f) ? objp->radius : LOLLIPOP_SIZE);

	mc.orient = &objp->orient;	// The object's orient
	mc.pos = &objp->pos;			// The object's position
	mc.p0 = p0;					// Point 1 of ray to check
	mc.p1 = p1;					// Point 2 of ray to check
	mc.flags = MC_CHECK_MODEL | MC_CHECK_RAY;  // flags
	model_collide(&mc);
	*hitpos = mc.hit_point_world;
	if ( mc.num_hits < 1 )	{
		// check shield
		mc.orient = &objp->orient;	// The object's orient
		mc.pos = &objp->pos;			// The object's position
		mc.p0 = p0;					// Point 1 of ray to check
		mc.p1 = p1;					// Point 2 of ray to check
		mc.flags = MC_CHECK_SHIELD;	// flags
		model_collide(&mc);
		*hitpos = mc.hit_point_world;
	}

	return mc.num_hits;
}

// Finds the closest object or waypoint under the mouse cursor and returns
// its index, or -1 if nothing there.
int select_object(int cx, int cy)
{
	int		best = -1;
	double	dist, best_dist = 9e99;
	vec3d	p0, p1, v, hitpos;
	vertex	vt;
	object *ptr;

	if (Briefing_dialog) {
		best = Briefing_dialog->check_mouse_hit(cx, cy);
		if (best >= 0)
			return best;
	}

/*	gr_reset_clip();
	g3_start_frame(0); ////////////////
	g3_set_view_matrix(&eye_pos, &eye_orient, 0.5f);*/

	//	Get 3d vector specified by mouse cursor location.
	g3_point_to_vec(&v, cx, cy);

//	g3_end_frame();
	if (!v.xyz.x && !v.xyz.y && !v.xyz.z)  // zero vector
		return -1;

	p0 = view_pos;
	vm_vec_scale_add(&p1, &p0, &v, 100.0f);

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		if (object_check_collision(ptr, &p0, &p1, &hitpos))	{
			hitpos.xyz.x = ptr->pos.xyz.x - view_pos.xyz.x;
			hitpos.xyz.y = ptr->pos.xyz.y - view_pos.xyz.y;
			hitpos.xyz.z = ptr->pos.xyz.z - view_pos.xyz.z;
			dist = hitpos.xyz.x * hitpos.xyz.x + hitpos.xyz.y * hitpos.xyz.y + hitpos.xyz.z * hitpos.xyz.z;
			if (dist < best_dist) {
				best = OBJ_INDEX(ptr);
				best_dist = dist;
			}
		}
		
		ptr = GET_NEXT(ptr);
	}

	if (best >= 0)
		return best;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		g3_rotate_vertex(&vt, &ptr->pos);
		if (!(vt.codes & CC_BEHIND))
			if (!(g3_project_vertex(&vt) & PF_OVERFLOW)) {
				hitpos.xyz.x = vt.sx - cx;
				hitpos.xyz.y = vt.sy - cy;
				dist = hitpos.xyz.x * hitpos.xyz.x + hitpos.xyz.y * hitpos.xyz.y;
				if ((dist < 8) && (dist < best_dist)) {
					best = OBJ_INDEX(ptr);
					best_dist = dist;
				}
			}

		ptr = GET_NEXT(ptr);
	}

	return best;
}

void render_compass(void)
{
	vec3d v, eye = { 0.0f };
	
	if (!Show_compass)
		return;

	gr_set_clip(gr_screen.max_w - 100, 0, 100, 100);
	g3_start_frame(0);  // ** Accounted for
	// required !!!
	vm_vec_scale_add2(&eye, &eye_orient.vec.fvec, -1.5f);
	g3_set_view_matrix(&eye, &eye_orient, 1.0f);

	v.xyz.x = 1.0f;
	v.xyz.y = v.xyz.z = 0.0f;
	if (vm_vec_dotprod(&eye, &v) < 0.0f)
		gr_set_color(159, 20, 20);
	else
		gr_set_color(255, 32, 32);
	draw_compass_arrow(&v);

	v.xyz.y = 1.0f;
	v.xyz.x = v.xyz.z = 0.0f;
	if (vm_vec_dotprod(&eye, &v) < 0.0f)
		gr_set_color(20, 159, 20);
	else
		gr_set_color(32, 255, 32);
	draw_compass_arrow(&v);

	v.xyz.z = 1.0f;
	v.xyz.x = v.xyz.y = 0.0f;
	if (vm_vec_dotprod(&eye, &v) < 0.0f)
		gr_set_color(20, 20, 159);
	else
		gr_set_color(32, 32, 255);
	draw_compass_arrow(&v);

	g3_end_frame(); // ** Accounted for

}

void draw_compass_arrow(vec3d *v0)
{
	vec3d	v1 = { 0.0f };
	vertex	tv0, tv1;

	g3_rotate_vertex(&tv0, v0);
	g3_rotate_vertex(&tv1, &v1);
	g3_project_vertex(&tv0);	
	g3_project_vertex(&tv1);
//	tv0.sx = (tv0.sx - tv1.sx) * 1 + tv1.sx;
//	tv0.sy = (tv0.sy - tv1.sy) * 1 + tv1.sy;
	g3_draw_line(&tv0, &tv1);
}


void inc_mission_time()
{
	fix thistime;

	thistime = timer_get_fixed_seconds();
	if (!lasttime)
		Frametime = F1_0 / 30;
	else
		Frametime = thistime - lasttime;

	if (Frametime > MAX_FRAMETIME)
		Frametime = MAX_FRAMETIME;

	if (Frametime < MIN_FRAMETIME)
		Frametime = MIN_FRAMETIME;

	Missiontime += Frametime;
	lasttime = thistime;
}
