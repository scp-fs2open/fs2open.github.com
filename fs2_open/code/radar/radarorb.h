/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Radar/Radarorb.h $
 * $Revision: 1.2 $
 * $Date: 2004-08-11 05:06:33 $
 * $Author: Kazan $
 *
 * Prototypes for radar orb code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2004/08/02 22:42:45  phreak
 * orb radar rendering style
 *
 
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _RADARORB_H
#define _RADARORB_H

extern int Radar_static_looping;

struct object;
struct blip;

extern void radar_init_orb();
extern void radar_plot_object_orb( object *objp );
extern void radar_frame_init_orb();
extern void radar_mission_init_orb();
extern void radar_frame_render_orb(float frametime);

// observer hud rendering code uses this function
void radar_draw_blips_sorted_orb(int distort);
void radar_draw_range_orb();
void radar_blit_gauge_orb();
int radar_blip_color_orb(object *objp);
void radar_null_nblips_orb();
void radar_draw_circle_orb( int x, int y, int rad );
void radar_blip_draw_distorted_orb(blip *b);
void radar_blip_draw_flicker_orb(blip *b);
void radar_page_in_orb();

#endif

