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
 * $Revision: 1.4 $
 * $Date: 2006-01-13 03:31:09 $
 * $Author: Goober5000 $
 *
 * Prototypes for radar orb code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/07/13 03:35:35  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.2  2004/08/11 05:06:33  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.1  2004/08/02 22:42:45  phreak
 * orb radar rendering style
 *
 
 * $NoKeywords: $
 */

#ifndef _RADARORB_H
#define _RADARORB_H

extern int Radar_static_looping;

struct object;
struct blip;
struct color;

extern void radar_init_orb();
extern void radar_plot_object_orb( object *objp );
extern void radar_frame_init_orb();
extern void radar_mission_init_orb();
extern void radar_frame_render_orb(float frametime);

// observer hud rendering code uses this function
void radar_draw_blips_sorted_orb(int distort);
void radar_draw_range_orb();
void radar_blit_gauge_orb();
void radar_stuff_blip_info_orb(object *objp, int is_bright, color **blip_color, int *blip_type);
void radar_null_nblips_orb();
void radar_draw_circle_orb( int x, int y, int rad );
void radar_blip_draw_distorted_orb(blip *b);
void radar_blip_draw_flicker_orb(blip *b);
void radar_page_in_orb();

#endif

