/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef GR_OPENGLDRAW_H
#define GR_OPENGLDRAW_H

void gr_opengl_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize, bool mirror);
void gr_opengl_aabitmap(int x, int y, bool resize, bool mirror);
void gr_opengl_string(int sx, int sy, char *s, bool resize = true);
void gr_opengl_string(float sx, float sy, char *s, bool resize = true);
void gr_opengl_line(int x1,int y1,int x2,int y2, bool resize);
void gr_opengl_aaline(vertex *v1, vertex *v2);
void gr_opengl_pixel(int x, int y, bool resize);
void gr_opengl_gradient(int x1, int y1, int x2, int y2, bool resize);
void gr_opengl_circle(int xc, int yc, int d, bool resize);
void gr_opengl_curve(int xc, int yc, int r, int direction);
void gr_opengl_scaler(vertex *va, vertex *vb, bool bw_bitmap );
void gr_opengl_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);
void gr_opengl_shade(int x, int y, int w, int h, bool resize);
void gr_opengl_flash(int r, int g, int b);
void gr_opengl_flash_alpha(int r, int g, int b, int a);
void gr_opengl_fade_in(int instantaneous);
void gr_opengl_fade_out(int instantaneous);
void gr_opengl_tmapper(int nverts, vertex **verts, uint flags);
void gr_opengl_render(int nverts, vertex *verts, uint flags);
void gr_opengl_render_effect(int nverts, vertex *verts, float *radius_list, uint flags);
void gr_opengl_bitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize);
void gr_opengl_update_distortion();

void opengl_render_timer_bar(int colour, float x, float y, float w, float h);
void opengl_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale, int stage = 0);
void opengl_reset_spec_mapping();

void gr_opengl_line_htl(vec3d *start, vec3d *end);
void gr_opengl_sphere_htl(float rad);

void gr_opengl_draw_line_list(colored_vector *lines, int num);

void opengl_setup_scene_textures();
void opengl_scene_texture_shutdown();
void gr_opengl_scene_texture_begin();
void gr_opengl_scene_texture_end();

extern int Scene_texture_initialized;

extern GLuint Scene_color_texture;
extern GLuint Scene_effect_texture;

extern int Scene_texture_width;
extern int Scene_texture_height;

#endif	// !GR_OPENGLDRAW_H
