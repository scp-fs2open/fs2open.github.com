/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Radar/radarsetup.cpp $
 * $Revision: 2.6 $
 * $Date: 2005-02-19 07:52:56 $
 * $Author: wmcoolmon $
 *
 * C module containg functions to manage different radar modes
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2005/02/04 20:06:07  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.4  2004/08/02 22:54:07  phreak
 * orb radar rendering style
 *
 * $NoKeywords: $
 */

#include "radar/radar.h"
#include "radar/radarorb.h"
#include "graphics/font.h"
#include "bmpman/bmpman.h"
#include "object/object.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "weapon/weapon.h"
#include "io/timer.h"
#include "hud/hud.h"
#include "hud/hudconfig.h"
#include "ship/subsysdamage.h"
#include "gamesnd/gamesnd.h"
#include "network/multi.h"
#include "weapon/emp.h"
#include "freespace2/freespace.h"
#include "localization/localize.h"
#include "ship/awacs.h"
#include "radar/radarsetup.h"

//function pointers for assorted radar functions
int  (*radar_blip_color)(object *objp)				= NULL;
void (*radar_blip_draw_distorted)(blip *b)			= NULL;
void (*radar_blip_draw_flicker)(blip *b)			= NULL;
void (*radar_blit_gauge)()							= NULL;
void (*radar_draw_blips_sorted)(int distort)		= NULL;
void (*radar_draw_circle)( int x, int y, int rad )	= NULL;
void (*radar_draw_range)()							= NULL;
void (*radar_frame_init)()							= NULL;
void (*radar_frame_render)(float frametime)			= NULL;
void (*radar_init)()								= NULL;
void (*radar_mission_init)()						= NULL;
void (*radar_null_nblips)()							= NULL;
void (*radar_page_in)()								= NULL;
void (*radar_plot_object)( object *objp )			= NULL;

int Radar_static_looping = -1;

radar_globals Radar_globals[MAX_RADAR_MODES];
radar_globals *Current_radar_global;

rcol Radar_color_rgb[MAX_RADAR_LEVELS][MAX_RADAR_COLORS] = {
	{{ 0xff, 0x00, 0x00},		// hostile			(red)
	{ 0x00, 0xff, 0x00},			// friendly			(green)
	{ 0xff, 0x00, 0xff},			// unknown			(purple)
	{ 0xff, 0x00, 0x00},			//	neutral			(red)
	{ 0x7f, 0x7f, 0x00},			// homing missile (yellow)
	{ 0x7f, 0x7f, 0x7f},			// navbuoy or cargo	(gray)
	{ 0x00, 0x00, 0xff},			// warp ship		(blue)
	{ 0x7f, 0x7f, 0x7f},			// jump node		(gray)
	{ 0xff, 0xff, 0x00}},		// tagged	 		(yellow)

	// 1/3 intensity of above colors
	{{ 0x7f, 0x00, 0x00},		// hostile			(red)
	{ 0x00, 0x7f, 0x00},			// friendly			(green)
	{ 0x7f, 0x00, 0x7f},			// unknown			(purple)
	{ 0x7f, 0x00, 0x00},			//	neutral			(red)
	{ 0x40, 0x40, 0x00},			// homing missile (yellow)
	{ 0x40, 0x40, 0x40},			// navbuoy or cargo	(gray)
	{ 0x00, 0x00, 0x7f},			// warp ship		(blue)
	{ 0x40, 0x40, 0x40},			// jump node		(gray)
	{ 0x7f, 0x7f, 0x00}},		// tagged			(yellow)
};

color Radar_colors[MAX_RADAR_LEVELS][MAX_RADAR_COLORS];

blip	Blip_bright_list[MAX_RADAR_COLORS];		// linked list of bright blips
blip	Blip_dim_list[MAX_RADAR_COLORS];			// linked list of dim blips
blip	Blips[MAX_BLIPS];								// blips pool
int	N_blips;											// next blip index to take from pool

float Radar_farthest_dist=1000.0f;
int Blip_mutate_id;

int Radar_static_playing;			// is static currently playing on the radar?
int Radar_static_next;				// next time to toggle static on radar
int Radar_avail_prev_frame;		// was radar active last frame?
int Radar_death_timer;				// timestamp used to play static on radar

hud_frames Radar_gauge;

float	radx, rady;
float	Radar_dim_range;					// range at which we start dimming the radar blips
int		Radar_calc_dim_dist_timer;		// timestamp at which we recalc Radar_dim_range
int		Radar_flicker_timer[NUM_FLICKER_TIMERS];					// timestamp used to flicker blips on and off
int		Radar_flicker_on[NUM_FLICKER_TIMERS];		

int See_all = 0;

DCF_BOOL(see_all, See_all)

static const char radar_deafult_filenames[2][16]=
{
	"radar1","2_radar1"
};

//just use the standard defaults that were in radar.cpp
//if a table is going to be loaded, the code will go here to get the various values
void create_radar_global_vars()
{
	for (int i=0; i < MAX_RADAR_MODES; i++)
	{
		Radar_globals[i].Radar_radius[0][0]=120;
		Radar_globals[i].Radar_radius[0][1]=100;
		Radar_globals[i].Radar_radius[1][0]=192;
		Radar_globals[i].Radar_radius[1][1]=160;

		Radar_globals[i].Radar_center[0][0]=322.0f;
		Radar_globals[i].Radar_center[0][1]=422.0f;
		Radar_globals[i].Radar_center[1][0]=515.0f;
		Radar_globals[i].Radar_center[1][1]=675.0f;

		Radar_globals[i].Radar_coords[0][0]=257;
		Radar_globals[i].Radar_coords[0][1]=369;
		Radar_globals[i].Radar_coords[1][0]=411;
		Radar_globals[i].Radar_coords[1][1]=590;

		strcpy(Radar_globals[i].Radar_fname[0],radar_deafult_filenames[0]);
		strcpy(Radar_globals[i].Radar_fname[1],radar_deafult_filenames[1]);

		Radar_globals[i].Radar_blip_radius_normal[0]=2;
		Radar_globals[i].Radar_blip_radius_normal[1]=4;
		Radar_globals[i].Radar_blip_radius_target[0]=5;
		Radar_globals[i].Radar_blip_radius_target[1]=8;

		Radar_globals[i].Radar_dist_coords[0][0][0]=367;
		Radar_globals[i].Radar_dist_coords[0][0][1]=461;

		Radar_globals[i].Radar_dist_coords[0][1][0]=364;
		Radar_globals[i].Radar_dist_coords[0][1][1]=461;

		Radar_globals[i].Radar_dist_coords[0][2][0]=368;
		Radar_globals[i].Radar_dist_coords[0][2][1]=461;

		Radar_globals[i].Radar_dist_coords[1][0][0]=595;
		Radar_globals[i].Radar_dist_coords[1][0][1]=740;

		Radar_globals[i].Radar_dist_coords[1][1][0]=592;
		Radar_globals[i].Radar_dist_coords[1][1][1]=740;

		Radar_globals[i].Radar_dist_coords[1][2][0]=596;
		Radar_globals[i].Radar_dist_coords[1][2][1]=740;
	}
}

void select_radar_mode(int radar_mode)
{
	static int radar_globals_inited=0;
	
	if (!radar_globals_inited)
	{
		radar_globals_inited=1;
		create_radar_global_vars();
	}	

	switch (radar_mode)
	{
		//selected normal radar mode.  thats the only one implemented now
		case RADAR_MODE_STANDARD:
		{
			radar_blip_color = radar_blip_color_std;
			radar_blip_draw_distorted = radar_blip_draw_distorted_std;
			radar_blip_draw_flicker = radar_blip_draw_flicker_std;
			radar_blit_gauge = radar_blit_gauge_std;
			radar_draw_blips_sorted = radar_draw_blips_sorted_std;
			radar_draw_circle = radar_draw_circle_std;
			radar_draw_range = radar_draw_range_std;
			radar_frame_init = radar_frame_init_std;
			radar_frame_render = radar_frame_render_std;
			radar_init = radar_init_std;
			radar_mission_init = radar_mission_init_std;
			radar_null_nblips = radar_null_nblips_std;
			radar_page_in = radar_page_in_std;
			radar_plot_object = radar_plot_object_std;
			Current_radar_global=&Radar_globals[RADAR_MODE_STANDARD];
			radar_init();
		}
		break;

		case RADAR_MODE_ORB:
		{
			radar_blip_color = radar_blip_color_orb;
			radar_blip_draw_distorted = radar_blip_draw_distorted_orb;
			radar_blip_draw_flicker = radar_blip_draw_flicker_orb;
			radar_blit_gauge = radar_blit_gauge_orb;
			radar_draw_blips_sorted = radar_draw_blips_sorted_orb;
			radar_draw_circle = radar_draw_circle_orb;
			radar_draw_range = radar_draw_range_orb;
			radar_frame_init = radar_frame_init_orb;
			radar_frame_render = radar_frame_render_orb;
			radar_init = radar_init_orb;
			radar_mission_init = radar_mission_init_orb;
			radar_null_nblips = radar_null_nblips_orb;
			radar_page_in = radar_page_in_orb;
			radar_plot_object = radar_plot_object_orb;
			Current_radar_global=&Radar_globals[RADAR_MODE_ORB];
			radar_init();
		}
		break;

		default:
			Error(LOCATION,"unknown radar mode specified");
	}
}
