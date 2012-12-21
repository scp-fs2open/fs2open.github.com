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

#include "graphics/gropenglstate.h"

void gr_opengl_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize, bool mirror);
void gr_opengl_aabitmap(int x, int y, bool resize, bool mirror);
void gr_opengl_string(int sx, int sy, const char *s, bool resize = true);
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

inline void opengl_draw_textured_quad(
	GLfloat x1, GLfloat y1, GLfloat u1, GLfloat v1,
	GLfloat x2, GLfloat y2, GLfloat u2, GLfloat v2 )
{
	GLfloat glVertices[4][4] = {
		{ x1, y1, u1, v1 },
		{ x1, y2, u1, v2 },
		{ x2, y1, u2, v1 },
		{ x2, y2, u2, v2 }
	};

	GL_state.Array.BindArrayBuffer(0);

	GL_state.Array.EnableClientVertex();
	GL_state.Array.VertexPointer(2, GL_FLOAT, sizeof(glVertices[0]), glVertices);

	GL_state.Array.SetActiveClientUnit(0);
	GL_state.Array.EnableClientTexture();
	GL_state.Array.TexPointer(2, GL_FLOAT, sizeof(glVertices[0]), &(glVertices[0][2]));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	GL_state.Array.DisableClientVertex();
	GL_state.Array.DisableClientTexture();
}

inline void opengl_draw_coloured_quad(
	GLint x1, GLint y1, 
	GLint x2, GLint y2 )
{
	GLint glVertices[8] = {
		x1, y1,
		x1, y2,
		x2, y1,
		x2, y2
	};

	GL_state.Array.EnableClientVertex();
	GL_state.Array.VertexPointer(2, GL_INT, 0, glVertices);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	GL_state.Array.DisableClientVertex();
}

inline void opengl_draw_coloured_quad(
	GLfloat x1, GLfloat y1,
	GLfloat x2, GLfloat y2 )
{
	GLfloat glVertices[8] = {
		x1, y1,
		x1, y2,
		x2, y1,
		x2, y2
	};

	GL_state.Array.EnableClientVertex();
	GL_state.Array.VertexPointer(2, GL_FLOAT, 0, glVertices);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	GL_state.Array.DisableClientVertex();
}

extern int Scene_texture_initialized;

extern GLuint Scene_color_texture;
extern GLuint Scene_luminance_texture;
extern GLuint Scene_effect_texture;

extern int Scene_texture_width;
extern int Scene_texture_height;

extern float Scene_texture_u_scale;
extern float Scene_texture_v_scale;

#endif	// !GR_OPENGLDRAW_H
