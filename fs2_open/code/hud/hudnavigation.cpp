// HUDNavigation.cpp
// Derek Meek
// 4-30-2004

/*
 * $Logfile: /Freespace2/code/Hud/HUDNavigation.cpp $
 * $Revision: 1.1 $
 * $Date: 2004-05-24 07:23:09 $
 * $Author: taylor $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2004/05/07 23:50:14  Kazan
 * Sorry Guys!
 *
 *
 *
 *
 */


#include "Hud/HUDNavigation.h"
#include "Autopilot/Autopilot.h"
#include "Hud/HUDtarget.h"
#include "ship/ship.h"
#include "object/object.h"
#include "render/3d.h"


extern float Cmdline_fov;
// Draws the Navigation stuff on the HUD
void HUD_Draw_Navigation()
{
	if (CurrentNav != -1 && Navs[CurrentNav].flags & NP_VALIDTYPE && !(Navs[CurrentNav].flags & NP_NOSELECT))
	{
		
		gr_string( 20, 120, Navs[CurrentNav].NavName);
		// Draw offscreen indicator if appropriate
		/*vertex target_point;
		vector target_pos = Navs[CurrentNav].GetPosition();

		g3_rotate_vertex(&target_point,&target_pos);
		g3_project_vertex(&target_point);


		color NavTrackColor;	
		gr_init_alphacolor( &NavTrackColor, 0x00, 0xDD, 0xFF, 0xFF );
		gr_set_color_fast(&NavTrackColor);

		float dist = vm_vec_dist_quick(&target_pos, &Player_obj->pos);
		hud_draw_offscreen_indicator(&target_point, &target_pos, dist);

		// Render Nav Indicator
		HUD_Draw_Nav_Brackets();
		*/
	}
	else
	{
		gr_string( 20, 120, "No Nav Point Selected");
	}
}



void HUD_Draw_Nav_Brackets()
{
	// figure out if it's within the field of view
	vector player_to_nav, target_pos = Navs[CurrentNav].GetPosition();

	vm_vec_sub(&player_to_nav, &Player_obj->pos, &target_pos);

	float angle = vm_vec_delta_ang(&player_to_nav,&Eye_matrix.vec.fvec,NULL);
		
	if (angle > Cmdline_fov)
		return;

	
	float updown, leftright;
	vector screen_relative;
		
	vm_vec_rotate(&screen_relative,&player_to_nav,&View_matrix);

	// trig
	// adjacent = Z
	// updown opposite = Y
	// leftright opposite = X

	// Soh Coa Toa
	updown = (float)atan(screen_relative.xyz.y/screen_relative.xyz.z);
	leftright = (float)atan(screen_relative.xyz.x/screen_relative.xyz.z);


	// figure out where on the screen to draw based upon the angle to from players eye pos to nav point location
	vector center;

	if (gr_screen.res == 0)
	{
		center.xyz.x = 320 + (320 * (leftright/Cmdline_fov));
		center.xyz.y = 240 + (240 * (updown/Cmdline_fov));

	
	}
	else
	{
		center.xyz.x = 512 + (512 * (leftright/Cmdline_fov));
		center.xyz.y = 384 + (384 * (updown/Cmdline_fov));

	}

	// draw
	vector UpLeft, DownLeft, UpRight, DownRight;

	UpLeft.xyz.x = center.xyz.x-10;
	UpLeft.xyz.y = center.xyz.x-10;

	DownLeft.xyz.x = center.xyz.x-10;
	DownLeft.xyz.y = center.xyz.x+10;


	UpRight.xyz.x = center.xyz.x+10;
	UpRight.xyz.y = center.xyz.x-10;

	DownRight.xyz.x = center.xyz.x+10;
	DownRight.xyz.y = center.xyz.x+10;
	

	//      <-    x    ->
	// *******         ******* 
	// *   *             *   *
	// *  *               *  * 
	// * *                 * * ^
	// *                     * |
	//
	//                         Y
    //
    // *                     * |
	// * *                 * * v
	// *  *               *  * 
	// *   *             *   *
	// *******         ******* 


	hud_tri_empty(UpLeft.xyz.x,		UpLeft.xyz.y,
				  UpLeft.xyz.x+7,	UpLeft.xyz.y,
				  UpLeft.xyz.x,		UpLeft.xyz.y+7);

	hud_tri_empty(DownLeft.xyz.x,	DownLeft.xyz.y,
				  DownLeft.xyz.x+7,	DownLeft.xyz.y,
				  DownLeft.xyz.x,	DownLeft.xyz.y-7);

	hud_tri_empty(UpRight.xyz.x,	UpRight.xyz.y,
				  UpRight.xyz.x-7,	UpRight.xyz.y,
				  UpRight.xyz.x,	UpRight.xyz.y+7);

	hud_tri_empty(UpLeft.xyz.x,		UpLeft.xyz.y,
				  UpLeft.xyz.x-7,	UpLeft.xyz.y,
				  UpLeft.xyz.x,		UpLeft.xyz.y-7);
}