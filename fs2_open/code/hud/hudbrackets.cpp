/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDbrackets.cpp $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:58 $
 * $Author: penguin $
 *
 * C file that contains functions for drawing target brackets on the HUD
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     8/03/99 5:35p Andsager
 * Dont draw target dot for instructor in training mission
 * 
 * 5     6/07/99 4:20p Andsager
 * Add HUD color for tagged object.  Apply to target and radar.
 * 
 * 4     12/28/98 3:17p Dave
 * Support for multiple hud bitmap filenames for hi-res mode.
 * 
 * 3     12/21/98 5:02p Dave
 * Modified all hud elements to be multi-resolution friendly.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 54    8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 53    6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 52    5/27/98 1:22p Allender
 * make targeting dots work in multiplayer -- as well as many other minor
 * targeting problems
 * 
 * 51    5/23/98 2:26a Lawrance
 * Tweak brackets
 * 
 * 50    5/14/98 11:26a Lawrance
 * ensure fighter bays are drawn with correct bracket color
 * 
 * 49    5/12/98 9:27a Mike
 * num-ships-attacking: always showed an additional ship attacking
 * asteroid or debris.  Fixed.
 * 
 * 48    5/08/98 10:16a Lawrance
 * Add new "ship attacking count" gauge
 * 
 * 47    5/07/98 4:07p Mike
 * Use smaller num-ships-attacking blip.
 * 
 * 46    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 45    5/06/98 2:46p Mike
 * Modify num-ships-attacking system.
 * 
 * 44    5/03/98 1:07a Mike
 * Show + for ships attacking your target, whether hostile or friendly.
 * 
 * 43    4/05/98 7:42p Lawrance
 * fix inconsistent distance display on the HUD
 * 
 * 42    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 41    3/30/98 12:20a Lawrance
 * Draw subsystem targeting brackets gray if subsystem is destroyed.
 * 
 * 40    3/18/98 6:04p Lawrance
 * Improve when brackets get drawn for large objects
 * 
 * 39    3/10/98 6:03p Lawrance
 * Ensure proper bracket color gets drawn for traitors
 * 
 * 38    3/06/98 5:13p Johnson
 * Put in a check for team traitor in hud_brackets_get_iff_color()
 * 
 * 37    3/02/98 11:32p Lawrance
 * Allow asteroids about to impact ships to be bracketed
 * 
 * 36    2/21/98 2:49p Lawrance
 * Don't do facing check for subsystems when deciding whether to draw
 * brackets
 * 
 * 35    2/19/98 12:48a Lawrance
 * Ensure subsystem brackets remain between HUD and target monitor
 * 
 * 34    2/09/98 8:05p Lawrance
 * Add new gauges: cmeasure success, warp-out, and missiontime
 * 
 * 33    2/07/98 5:46p Lawrance
 * Show target distance on brackets
 * 
 * 32    1/19/98 11:37p Lawrance
 * Fixing Optimization build warnings
 * 
 * 31    1/18/98 5:09p Lawrance
 * Added support for TEAM_TRAITOR
 * 
 * 30    1/02/98 10:01p Lawrance
 * Ensure correct subsystem bracket color gets drawn once player goes
 * traitor.
 * 
 * 29    1/02/98 9:10p Lawrance
 * Big changes to how colors get set on the HUD.
 * 
 * 28    12/28/97 5:54p Lawrance
 * Draw HUD brackets the correct IFF color.. taking into account that the
 * player may be TEAM_NEUTRAL
 * 
 * 27    12/18/97 8:46p Lawrance
 * Move IFF_color definitions from HUD->ship, so FRED can use them.
 * 
 * 26    12/17/97 11:14p Mike
 * Change radar and hud color to red for all neutral.
 * 
 * 25    11/27/97 4:24p Lawrance
 * change appearance of subsystem targeting brackets
 * 
 * 24    10/22/97 5:53p Lawrance
 * change name of subsystem_in_sight() function
 * 
 * 23    9/06/97 2:13p Mike
 * Replace support for TEAM_NEUTRAL
 * 
 * 22    7/24/97 10:45a Mike
 * Fix hole in Unknown team support.
 * 
 * 21    7/24/97 10:24a Mike
 * Restore support for Unknown team
 * 
 * 20    6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 19    5/21/97 11:12a Mike
 * Move more stuff out of player struct, mainly subsys stuff.
 * 
 * 18    5/20/97 2:45p Mike
 * Move current_target and a bunch of other stuff out of player struct.
 * 
 * 17    4/09/97 3:30p Lawrance
 * let target brackets grow to bracket ship entirely
 * 
 * 16    4/08/97 1:28p Lawrance
 * get brackets for targeting and messaging drawing right
 * 
 * 15    4/08/97 11:44a Lawrance
 * get selection and target brackets drawing right at close and far
 * distances
 * 
 * 14    4/08/97 10:55a Allender
 * draw purple brackets on ship sending a message
 * 
 * 13    4/08/97 9:58a Lawrance
 * center bracket on target center.  Give min and max dimensions to
 * subsystem target brackets.
 * 
 * 12    4/07/97 6:03p Lawrance
 * draw diamond brackets instead of dashed boxes
 * 
 * 11    4/07/97 3:50p Allender
 * ability to assign > 1 ship to a hotkey.  Enabled use of hotkeys in
 * squadmate messaging
 * 
 * 10    4/02/97 10:08a Lawrance
 * fixed bracket drawing glitches 
 * 
 * 9     3/28/97 2:46p John
 * added code to make debris chunks target properly.
 * 
 * 8     3/27/97 5:44p Lawrance
 * drawing dashed lines for sub-object targeting box that is not in line
 * of sight
 * 
 * 7     3/27/97 3:59p Lawrance
 * made brackets draw even if center of target is offscreen
 * 
 * 6     3/27/97 9:29a Lawrance
 * If reach maximum bounding box size, use radius targeting box method
 * 
 * 5     3/25/97 3:55p Lawrance
 * allowing debris to be targeted and shown on radar
 * 
 * 4     3/23/97 11:55p Lawrance
 * made max targeting bracket size of 200x200
 * 
 * 3     3/07/97 4:37p Mike
 * Make rockeye missile home.
 * Remove UNKNOWN and NEUTRAL teams.
 * 
 * 2     12/24/96 4:30p Lawrance
 * Target bracket drawing code moved to separate files
 *
 * $NoKeywords: $
 */

#include "hudbrackets.h"
#include "hud.h"
#include "player.h"
#include "hudtarget.h"
#include "3d.h"
#include "debris.h"
#include "ai.h"
#include "freespace.h"
#include "bmpman.h"
#include "linklist.h"
#include "emp.h"

#define FADE_FACTOR	2			// how much the bounding brackets get faded
#define LOWEST_RED	50			// lowest r value for bounding bracket
#define LOWEST_GREEN	50			// lowest g value for bounding bracket
#define LOWEST_BLUE	50			// lowest b value for bounding bracket

char Ships_attack_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"attacker",
	"attacker"
};

// resolution adjust factors for the above defines
int Min_target_box_width[GR_NUM_RESOLUTIONS] = { 20, 30 };
int Min_target_box_height[GR_NUM_RESOLUTIONS] = { 20, 30 };
int Min_subtarget_box_width[GR_NUM_RESOLUTIONS] = { 12, 24 };
int Min_subtarget_box_height[GR_NUM_RESOLUTIONS] = { 12, 24 };

void hud_init_brackets()
{
}

// find IFF color index for brackets based on team
int hud_brackets_get_iff_color(int team)
{
	int color=IFF_COLOR_FRIENDLY;

	switch ( team ) {
	case TEAM_FRIENDLY:
	case TEAM_HOSTILE:
	case TEAM_NEUTRAL:
	case TEAM_TRAITOR:
	case TEAM_UNKNOWN:
		if ( (team == Player_ship->team) && (team != TEAM_TRAITOR) ) {
			color = IFF_COLOR_FRIENDLY;
		} else {
			switch (team) {
			case TEAM_NEUTRAL:
				color = IFF_COLOR_NEUTRAL;
				break;
			case TEAM_UNKNOWN:
				color = IFF_COLOR_UNKNOWN;
				break;
			case TEAM_HOSTILE:
			case TEAM_FRIENDLY:
			case TEAM_TRAITOR:
				color = IFF_COLOR_HOSTILE;
				break;
			}
		}
		break;
	case SELECTION_SET:
		color = IFF_COLOR_SELECTION;
		break;
		
	case MESSAGE_SENDER:
		color = IFF_COLOR_MESSAGE;
		break;

	default:
		color = IFF_COLOR_UNKNOWN;
		Int3();
	} // end switch

	return color;
}


//	Called by draw_bounding_brackets.  
void draw_brackets_square(int x1, int y1, int x2, int y2)
{
	int	width, height;
	
	width = x2 - x1;
	Assert( width > 0);
	height = y2 - y1;
	Assert( height > 0);

	// make the brackets extend 25% of the way along the width or height
	int bracket_width = width/4;
	int bracket_height = height/4;

	// horizontal lines
	if ( (x1 + bracket_width > 0) && (x1 < gr_screen.clip_width) ){
		gr_gradient(x1,y1,x1+bracket_width-1,y1);	// top left
		gr_gradient(x1,y2,x1+bracket_width-1,y2);	// bottom left
	}

	if ( (x2 - bracket_width < gr_screen.clip_width) && (x2 > 0) )	{
		gr_gradient(x2, y1, x2-bracket_width+1,y1);	// top right
		gr_gradient(x2, y2, x2-bracket_width+1,y2);	// bottom right
	}

	// vertical lines
	if ( (y1 + bracket_height > 0) && (y1 < gr_screen.clip_height) ) {
		gr_gradient(x1,y1,x1,y1+bracket_height-1);		// top left
		gr_gradient(x2,y1,x2,y1+bracket_height-1);		// top right
	}

	if ( (y2 - bracket_height < gr_screen.clip_height) && (y2 > 0) )	{
		gr_gradient(x1,y2,x1,y2-bracket_height+1);	// bottom left
		gr_gradient(x2,y2,x2,y2-bracket_height+1);	// bottom right
	}
}

void draw_brackets_square_quick(int x1, int y1, int x2, int y2, int thick)
{
	int	width, height;

	width = x2 - x1;
	height = y2 - y1;

	// make the brackets extend 25% of the way along the width or height
	int bracket_width = width/4;
	int bracket_height = height/4;

	// top line
	gr_line(x1,y1,x1+bracket_width,y1);
	gr_line(x2,y1,x2-bracket_width,y1);
	if ( thick ) {
		gr_line(x1,y1+1,x1+bracket_width,y1+1);
		gr_line(x2,y1+1,x2-bracket_width,y1+1);
	}

	// bottom line
	gr_line(x1,y2,x1+bracket_width,y2);
	gr_line(x2,y2,x2-bracket_width,y2);
	if ( thick ) {
		gr_line(x1,y2-1,x1+bracket_width,y2-1);
		gr_line(x2,y2-1,x2-bracket_width,y2-1);
	}

	// left line
	if ( thick ) {
		gr_line(x1,y1+2,x1,y1+bracket_height);
		gr_line(x1,y2-2,x1,y2-bracket_height);
		gr_line(x1+1,y1+2,x1+1,y1+bracket_height);
		gr_line(x1+1,y2-2,x1+1,y2-bracket_height);
	} else {
		gr_line(x1,y1+1,x1,y1+bracket_height);
		gr_line(x1,y2-1,x1,y2-bracket_height);
	}

	// right line
	if ( thick ) {
		gr_line(x2,y1+2,x2,y1+bracket_height);
		gr_line(x2,y2-2,x2,y2-bracket_height);
		gr_line(x2-1,y1+2,x2-1,y1+bracket_height);
		gr_line(x2-1,y2-2,x2-1,y2-bracket_height);
	} else {
		gr_line(x2,y1+1,x2,y1+bracket_height);
		gr_line(x2,y2-1,x2,y2-bracket_height);
	}
}


#define NUM_DASHES	2
void draw_brackets_dashed_square_quick(int x1, int y1, int x2, int y2)
{
	int	width, height, i;

	width = x2 - x1;
	height = y2 - y1;

	// make the brackets extend 25% of the way along the width or height
	float bracket_width = width/4.0f;
	float bracket_height = height/4.0f;

	int dash_width;
	dash_width = fl2i(bracket_width / ( NUM_DASHES*2 - 1 ) + 0.5f);

	if ( dash_width < 1 ) {
		draw_brackets_square_quick(x1, y1, x2, y2);
		return;
	}

	int dash_height;
	dash_height = fl2i(bracket_height / ( NUM_DASHES*2 - 1 ) + 0.5f);

	if ( dash_height < 1 ) {
		draw_brackets_square_quick(x1, y1, x2, y2);
		return;
	}
	
	int dash_x1, dash_x2, dash_y1, dash_y2;

	dash_x1 = x1;
	dash_x2 = x2;
	dash_y1 = y1;
	dash_y2 = y2;

	for ( i = 0; i < NUM_DASHES; i++ ) {
		// top line
		gr_line(dash_x1, y1, dash_x1+(dash_width-1), y1);
		gr_line(dash_x2, y1, dash_x2-(dash_width-1), y1);

		// bottom line
		gr_line(dash_x1, y2, dash_x1+(dash_width-1), y2);
		gr_line(dash_x2, y2, dash_x2-(dash_width-1), y2);
		
		dash_x1 += dash_width*2;
		dash_x2 -= dash_width*2;

		// left line
		gr_line(x1, dash_y1, x1, dash_y1+(dash_height-1));
		gr_line(x1, dash_y2, x1, dash_y2-(dash_height-1));

		// right line
		gr_line(x2, dash_y1, x2, dash_y1+(dash_height-1));
		gr_line(x2, dash_y2, x2, dash_y2-(dash_height-1));

		dash_y1 += dash_height*2;
		dash_y2 -= dash_height*2;
	}

}



// draw_brackets_diamond()
//	Called by draw_bounding_brackets.  

void draw_brackets_diamond(int x1, int y1, int x2, int y2)
{
	int width, height, half_width, half_height;
	int center_x, center_y;
	int x_delta, y_delta;

	float side_len, bracket_len;

	width = x2 - x1;
	height = y2 - y1;

	half_width = fl2i( width/2.0f + 0.5f );
	half_height = fl2i( height/2.0f +0.5f );

	side_len = (float)_hypot(half_width, half_height);
	bracket_len = side_len / 8;
	
	x_delta = fl2i(bracket_len * width / side_len + 0.5f);
	y_delta = fl2i(bracket_len * height / side_len + 0.5f);


	center_x = x1 + half_width;
	center_y = y1 + half_height;

	// top left line
	gr_gradient(center_x - x_delta, y1 + y_delta,center_x, y1);
	gr_gradient(x1 + x_delta, center_y - y_delta, x1, center_y);

	// top right line
	gr_gradient(center_x + x_delta, y1 + y_delta,center_x, y1);
	gr_gradient(x2 - x_delta, center_y - y_delta, x2, center_y);

	// bottom left line
	gr_gradient(x1 + x_delta, center_y + y_delta, x1, center_y);
	gr_gradient(center_x - x_delta, y2 - y_delta, center_x, y2);

	// bottom right line
	gr_gradient(x2 - x_delta, center_y + y_delta, x2, center_y);
	gr_gradient(center_x + x_delta, y2 - y_delta, center_x, y2);
}

void draw_brackets_diamond_quick(int x1, int y1, int x2, int y2, int thick)
{
	int width, height, half_width, half_height;
	int center_x, center_y;
	int x_delta, y_delta;

	float side_len, bracket_len;

	width = x2 - x1;
	height = y2 - y1;

	half_width = fl2i( width/2.0f + 0.5f);
	half_height = fl2i( height/2.0f + 0.5f);

	side_len = (float)_hypot(half_width, half_height);
	bracket_len = side_len / 8;
	
	x_delta = fl2i(bracket_len * width / side_len + 0.5f);
	y_delta = fl2i(bracket_len * height / side_len + 0.5f);

	center_x = x1 + half_width;
	center_y = y1 + half_height;

	// top left line
	gr_line(center_x - x_delta, y1 + y_delta,center_x, y1);
	gr_line(x1 + x_delta, center_y - y_delta, x1, center_y);

	// top right line
	gr_line(center_x + x_delta, y1 + y_delta,center_x, y1);
	gr_line(x2 - x_delta, center_y - y_delta, x2, center_y);

	// bottom left line
	gr_line(x1 + x_delta, center_y + y_delta, x1, center_y);
	gr_line(center_x - x_delta, y2 - y_delta, center_x, y2);

	// bottom right line
	gr_line(x2 - x_delta, center_y + y_delta, x2, center_y);
	gr_line(center_x + x_delta, y2 - y_delta, center_x, y2);

	// draw an 'X' in the middle of the brackets
	gr_line(center_x-x_delta, center_y-y_delta, center_x+x_delta, center_y+y_delta);
	gr_line(center_x-x_delta, center_y+y_delta, center_x+x_delta, center_y-y_delta);
}


int subsys_is_fighterbay(ship_subsys *ss)
{
	if ( !strnicmp(NOX("fighter"), ss->system_info->name, 7) ) {
		return 1;
	}

	return 0;
}

//	Draw bounding brackets for a subobject.
void draw_bounding_brackets_subobject()
{
	if (Player_ai->targeted_subsys_parent == Player_ai->target_objnum)
		if (Player_ai->targeted_subsys != NULL) {
			ship_subsys	*subsys;
			int		target_objnum;
			object* targetp;
			vertex subobj_vertex;
			vector	subobj_pos;
			int x1,x2,y1,y2;

			subsys = Player_ai->targeted_subsys;
			target_objnum = Player_ai->target_objnum;
			Assert(target_objnum != -1);
			targetp = &Objects[target_objnum];
			Assert( targetp->type == OBJ_SHIP );

			get_subsystem_world_pos(targetp, subsys, &subobj_pos);

			g3_rotate_vertex(&subobj_vertex,&subobj_pos);

			g3_project_vertex(&subobj_vertex);
			if (subobj_vertex.flags & PF_OVERFLOW)  // if overflow, no point in drawing brackets
				return;

			int subobj_x = fl2i(subobj_vertex.sx + 0.5f);
			int subobj_y = fl2i(subobj_vertex.sy + 0.5f);
			int hud_subtarget_w, hud_subtarget_h, bound_rc;

			bound_rc = subobj_find_2d_bound(subsys->system_info->radius, &targetp->orient, &subobj_pos, &x1,&y1,&x2,&y2);
			if ( bound_rc != 0 )
				return;

			hud_subtarget_w = x2-x1+1;
			if ( hud_subtarget_w > gr_screen.clip_width ) {
				hud_subtarget_w = gr_screen.clip_width;
			}

			hud_subtarget_h = y2-y1+1;
			if ( hud_subtarget_h > gr_screen.clip_height ) {
				hud_subtarget_h = gr_screen.clip_height;
			}

			if ( hud_subtarget_w > gr_screen.max_w ) {
				x1 = subobj_x - (gr_screen.max_w>>1);
				x2 = subobj_x + (gr_screen.max_w>>1);
			}
			if ( hud_subtarget_h > gr_screen.max_h ) {
				y1 = subobj_y - (gr_screen.max_h>>1);
				y2 = subobj_y + (gr_screen.max_h>>1);
			}

			if ( hud_subtarget_w < Min_subtarget_box_width[gr_screen.res] ) {
				x1 = subobj_x - (Min_subtarget_box_width[gr_screen.res]>>1);
				x2 = subobj_x + (Min_subtarget_box_width[gr_screen.res]>>1);
			}
			if ( hud_subtarget_h < Min_subtarget_box_height[gr_screen.res] ) {
				y1 = subobj_y - (Min_subtarget_box_height[gr_screen.res]>>1);
				y2 = subobj_y + (Min_subtarget_box_height[gr_screen.res]>>1);
			}

			// determine if subsystem is on far or near side of the ship
			Player->subsys_in_view = ship_subsystem_in_sight(targetp, subsys, &View_position, &subobj_pos, 0);

			// AL 29-3-98: If subsystem is destroyed, draw gray brackets					
			if ( (Player_ai->targeted_subsys->current_hits <= 0) && (!subsys_is_fighterbay(Player_ai->targeted_subsys)) ) {
				gr_set_color_fast(&IFF_colors[IFF_COLOR_MESSAGE][1]);
			} else {
				hud_set_iff_color( targetp, 1 );
			}

			if ( Player->subsys_in_view ) {
				draw_brackets_square_quick(x1, y1, x2, y2);
			} else {
				draw_brackets_diamond_quick(x1, y1, x2, y2);
			}
			// mprintf(("Drawing subobject brackets at %4i, %4i\n", sx, sy));
		}
}

extern int HUD_drew_selection_bracket_on_target;

// Display the current target distance, right justified at (x,y)
void hud_target_show_dist_on_bracket(int x, int y, float distance)
{
	char	text_dist[64];
	int	w,h;

	if ( y < 0 || y > gr_screen.clip_height ) {
		return;
	}

	if ( x < 0 || x > gr_screen.clip_width ) {
		return;
	}

	sprintf(text_dist, "%d", fl2i(distance+0.5f));
	hud_num_make_mono(text_dist);
	gr_get_string_size(&w,&h,text_dist);

	int y_delta = 4;
	if ( HUD_drew_selection_bracket_on_target ) {
		y += 4;
	}

	gr_string(x - w+2, y+y_delta, text_dist);
}


// !!!!!!!!!!!!!!!
//	Given an object number, return the number of ships attacking it.
// MWA 5/26/98 -- copied from aicode num_attacking_ships()!!!
// !!!!!!!!!!!!!!!
int hud_bracket_num_ships_attacking(int objnum)
{
	object	*objp;
	ship_obj	*so;
	int		count = 0;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		if (objp->instance != -1) {
			ai_info	*aip;
			aip = &Ai_info[Ships[objp->instance].ai_index];

			// don't count instructor
			int is_training_mission();
			if ( is_training_mission() && stricmp(Ships[objp->instance].ship_name, "Instructor") == 0) {
				break;
			}

			if ( ((Game_mode & GM_MULTIPLAYER) || (aip->mode == AIM_CHASE)) && (aip->target_objnum == objnum))
				if (Ships[objp->instance].team != Ships[Objects[objnum].instance].team)
					count++;
		}
	}

	return count;
}


int	Ships_attacking_bitmap = -1;

// draw_bounding_brackets() will draw the faded brackets that surround the current target
void draw_bounding_brackets(int x1, int y1, int x2, int y2, int w_correction, int h_correction, float distance, int target_objnum)
{
	int width, height;

	if ( ( x1 < 0 && x2 < 0 ) || ( y1 < 0 && y2 < 0 ) )
		return;

	if ( ( x1 > gr_screen.clip_width && x2 > gr_screen.clip_width ) ||
		  ( y1 > gr_screen.clip_height && y2 > gr_screen.clip_height ) )
		return;

	width = x2-x1;
	Assert(width>=0);

	height = y2-y1;
	Assert(height>=0);

	if ( (width>(gr_screen.max_w - 1)) && (height>(gr_screen.max_h - 1)) ) {
		return;
	}
	if ( width > 1200 ) {
		return;

	}
	if ( height > 1200) {
		return;
	}

	if (width < Min_target_box_width[gr_screen.res]) {
		x1 = x1 - (Min_target_box_width[gr_screen.res]-width)/2;
		x2 = x2 + (Min_target_box_width[gr_screen.res]-width)/2;
	}

	if (height < Min_target_box_height[gr_screen.res]) {
		y1 = y1 - (Min_target_box_height[gr_screen.res]-height)/2;
		y2 = y2 + (Min_target_box_height[gr_screen.res]-height)/2;
	}
		
	draw_brackets_square(x1-w_correction, y1-h_correction, x2+w_correction, y2+h_correction);

	// draw distance to target in lower right corner of box
	if ( distance > 0 ) {
		hud_target_show_dist_on_bracket(x2+w_correction,y2+h_correction,distance);
	}

	//	Maybe show + for each additional fighter or bomber attacking target.
	if ( (target_objnum != -1) && hud_gauge_active(HUD_ATTACKING_TARGET_COUNT) ) {
		int num_attacking = hud_bracket_num_ships_attacking(target_objnum);

		if (Ships_attacking_bitmap == -1){
			Ships_attacking_bitmap = bm_load(Ships_attack_fname[gr_screen.res]);
		}

		if (Ships_attacking_bitmap == -1) {
			Int3();
			return;
		}

		//	If a ship not on player's team, show one fewer plus since it is targeted and attacked by player.
		int	k=0;
		if (Objects[target_objnum].type == OBJ_SHIP) {
			if (Ships[Objects[target_objnum].instance].team != Player_ship->team){
				k = 1;
			}
		} else {
			k = 1;
		}

		if (num_attacking > k) {
			int	i, num_blips;
			
			num_blips = num_attacking-k;
			if (num_blips > 4){
				num_blips = 4;
			}

			//int	bitmap = get_blip_bitmap();

			if (Ships_attacking_bitmap > -1) {
				if (num_blips > 3)
					y1 -= 3;

				for (i=0; i<num_blips; i++) {
					GR_AABITMAP(Ships_attacking_bitmap, x2+3, y1+i*7);					
				}
			}
		}
	}
}
