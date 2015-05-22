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
#include "graphics/shadows.h"
#include "graphics/2d.h"

void gr_opengl_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode, bool mirror);
void gr_opengl_aabitmap(int x, int y, int resize_mode, bool mirror);
void gr_opengl_string(int sx, int sy, const char *s, int resize_mode = GR_RESIZE_FULL);
void gr_opengl_line(int x1,int y1,int x2,int y2, int resize_mode);
void gr_opengl_aaline(vertex *v1, vertex *v2);
void gr_opengl_pixel(int x, int y, int resize_mode);
void gr_opengl_gradient(int x1, int y1, int x2, int y2, int resize_mode);
void gr_opengl_circle(int xc, int yc, int d, int resize_mode);
void gr_opengl_unfilled_circle(int xc, int yc, int d, int resize_mode);
void gr_opengl_arc(int xc, int yc, float r, float angle_start, float angle_end, bool fill, int resize_mode);
void gr_opengl_curve(int xc, int yc, int r, int direction, int resize_mode);
void gr_opengl_scaler(vertex *va, vertex *vb, bool bw_bitmap );
void gr_opengl_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct, int resize_mode);
void gr_opengl_shade(int x, int y, int w, int h, int resize_mode);
void gr_opengl_flash(int r, int g, int b);
void gr_opengl_flash_alpha(int r, int g, int b, int a);
void gr_opengl_fade_in(int instantaneous);
void gr_opengl_fade_out(int instantaneous);
void gr_opengl_tmapper(int nverts, vertex **verts, uint flags);
void gr_opengl_render(int nverts, vertex *verts, uint flags);
void gr_opengl_render_effect(int nverts, vertex *verts, float *radius_list, uint flags);
void gr_opengl_bitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode);
void gr_opengl_update_distortion();

void opengl_render_timer_bar(int colour, float x, float y, float w, float h);
void opengl_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale, int stage = 0);
void opengl_reset_spec_mapping();

void gr_opengl_line_htl(vec3d *start, vec3d *end);
void gr_opengl_sphere_htl(float rad);
void gr_opengl_deferred_light_sphere_init(int rings, int segments);
void gr_opengl_draw_deferred_light_sphere(vec3d *position, float rad, bool clearStencil);
void gr_opengl_deferred_light_cylinder_init(int segments);
void gr_opengl_draw_deferred_light_cylinder(vec3d *position,matrix *orient, float rad, float length, bool clearStencil);

void gr_opengl_draw_line_list(colored_vector *lines, int num);

void gr_opengl_shadow_map_start(matrix4 *shadow_view_matrix, matrix *light_orient);
void gr_opengl_shadow_map_end();

void opengl_setup_scene_textures();
void opengl_scene_texture_shutdown();
void gr_opengl_scene_texture_begin();
void gr_opengl_scene_texture_end();
void gr_opengl_copy_effect_texture();

void opengl_clear_deferred_buffers();
void gr_opengl_deferred_lighting_begin();
void gr_opengl_deferred_lighting_end();
void gr_opengl_deferred_lighting_finish();

void opengl_bind_vertex_layout(vertex_layout &layout);

inline void opengl_draw_textured_quad_instanced(
	GLfloat x1, GLfloat y1, GLfloat u1, GLfloat v1,
	GLfloat x2, GLfloat y2, GLfloat u2, GLfloat v2, 
	int count )
{
	GLfloat glVertices[4][4] = {
		{ x1, y1, u1, v1 },
		{ x1, y2, u1, v2 },
		{ x2, y1, u2, v1 },
		{ x2, y2, u2, v2 }
	};

	GL_state.Array.BindArrayBuffer(0);

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(glVertices[0]), glVertices);
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(glVertices[0]), &(glVertices[0][2]));

	opengl_bind_vertex_layout(vert_def);

	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	vglDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, 4, count);
}

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

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(glVertices[0]), glVertices);
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(glVertices[0]), &(glVertices[0][2]));

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::SCREEN_POS, 0, glVertices);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, glVertices);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

extern int Scene_texture_initialized;

extern GLuint Scene_color_texture;
extern GLuint Scene_luminance_texture;
extern GLuint Scene_effect_texture;

extern int Scene_texture_width;
extern int Scene_texture_height;

extern float Scene_texture_u_scale;
extern float Scene_texture_v_scale;

extern bool Deferred_lighting;

extern bool Use_Shaders_for_effect_rendering;

#endif	// !GR_OPENGLDRAW_H
