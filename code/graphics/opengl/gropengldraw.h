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

#include "graphics/2d.h"
#include "gropenglstate.h"
#include "gropenglshader.h"
#include "graphics/shadows.h"
#include <glad/glad.h>

extern GLuint Scene_framebuffer;
extern GLuint Scene_ldr_texture;
extern GLuint Scene_color_texture;
extern GLuint Scene_position_texture;
extern GLuint Scene_normal_texture;
extern GLuint Scene_specular_texture;
extern GLuint Scene_luminance_texture;
extern GLuint Scene_effect_texture;
extern GLuint Scene_depth_texture;
extern GLuint Cockpit_depth_texture;
extern GLuint Scene_stencil_buffer;

void gr_opengl_update_distortion();

void opengl_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale, int stage = 0);
void opengl_reset_spec_mapping();

void gr_opengl_sphere(material *material_def, float rad);

void gr_opengl_shadow_map_start(matrix4 *shadow_view_matrix, const matrix *light_orient);
void gr_opengl_shadow_map_end();

void gr_opengl_render_shield_impact(shield_material *material_info, primitive_type prim_type, vertex_layout *layout, int buffer_handle, int n_verts);

void opengl_setup_scene_textures();
void opengl_scene_texture_shutdown();
void gr_opengl_scene_texture_begin();
void gr_opengl_scene_texture_end();
void gr_opengl_copy_effect_texture();

void opengl_render_primitives(primitive_type prim_type, vertex_layout* layout, int n_verts, int buffer_handle, size_t vert_offset, size_t byte_offset);
void opengl_render_primitives_immediate(primitive_type prim_type, vertex_layout* layout, int n_verts, void* data, int size);

void gr_opengl_render_primitives(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle, size_t buffer_offset);
void gr_opengl_render_primitives_particle(particle_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
void gr_opengl_render_primitives_batched(batched_bitmap_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
void gr_opengl_render_primitives_distortion(distortion_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
void gr_opengl_render_movie(movie_material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, int buffer);

void opengl_draw_textured_quad(GLfloat x1, GLfloat y1, GLfloat u1, GLfloat v1,
							   GLfloat x2, GLfloat y2, GLfloat u2, GLfloat v2 );

inline GLenum opengl_primitive_type(primitive_type prim_type);

extern int Scene_texture_initialized;

extern GLuint Scene_color_texture;
extern GLuint Scene_ldr_texture;
extern GLuint Scene_luminance_texture;
extern GLuint Scene_effect_texture;

extern int Scene_texture_width;
extern int Scene_texture_height;

extern float Scene_texture_u_scale;
extern float Scene_texture_v_scale;

extern bool Deferred_lighting;
extern bool High_dynamic_range;

extern bool Use_Shaders_for_effect_rendering;

#endif	// !GR_OPENGLDRAW_H
