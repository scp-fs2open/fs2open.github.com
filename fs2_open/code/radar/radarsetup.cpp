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
 * $Revision: 2.1 $
 * $Date: 2004-07-01 01:51:54 $
 * $Author: phreak $
 *
 * C module containg functions to manage different radar modes
 *
 * $NoKeywords: $
 */

#include "radar/radar.h"
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
#include "globalincs/linklist.h"
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

void select_radar_mode(int radar_mode)
{
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

			radar_init();
		}
		break;
		default:
			Error(LOCATION,"unknown radar mode specified");
	}
}