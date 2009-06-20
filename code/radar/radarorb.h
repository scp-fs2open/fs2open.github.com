/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _RADARORB_H
#define _RADARORB_H

extern int Radar_static_looping;

struct object;
struct blip;
struct color;
struct vec3d;

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
void radar_orb_draw_image(vec3d *out, int rad, int idx, float mult);

#endif

