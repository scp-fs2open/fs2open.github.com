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

struct opengl_vertex_bind {
	vertex_format_data::vertex_format format;
	GLint size;
	GLenum data_type;
	GLboolean normalized;
	opengl_vert_attrib::attrib_id attribute_id;
};

void gr_opengl_update_distortion();

void opengl_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale, int stage = 0);
void opengl_reset_spec_mapping();

void opengl_draw_sphere();
void gr_opengl_sphere(material *material_def, float rad);
void gr_opengl_deferred_light_sphere_init(int rings, int segments);
void gr_opengl_draw_deferred_light_sphere(const vec3d *position, float rad, bool clearStencil);
void gr_opengl_deferred_light_cylinder_init(int segments);
void gr_opengl_draw_deferred_light_cylinder(const vec3d *position, const matrix *orient, float rad, float length, bool clearStencil);

void gr_opengl_shadow_map_start(glm::mat4& shadow_view_matrix, const matrix *light_orient);
void gr_opengl_shadow_map_end();

void gr_opengl_render_shield_impact(shield_material *material_info, primitive_type prim_type, vertex_layout *layout, int buffer_handle, int n_verts);

void opengl_setup_scene_textures();
void opengl_scene_texture_shutdown();
void gr_opengl_scene_texture_begin();
void gr_opengl_scene_texture_end();
void gr_opengl_copy_effect_texture();

void opengl_clear_deferred_buffers();
void gr_opengl_deferred_lighting_begin();
void gr_opengl_deferred_lighting_end();
void gr_opengl_deferred_lighting_finish();

void opengl_render_primitives(primitive_type prim_type, vertex_layout* layout, int n_verts, int buffer_handle, size_t vert_offset, size_t byte_offset);
void opengl_render_primitives_immediate(primitive_type prim_type, vertex_layout* layout, int n_verts, void* data, int size);

void gr_opengl_render_primitives(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
void gr_opengl_render_primitives_2d(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
void gr_opengl_render_primitives_particle(particle_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
void gr_opengl_render_primitives_batched(batched_bitmap_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
void gr_opengl_render_primitives_distortion(distortion_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
void gr_opengl_render_movie(movie_material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, int buffer);

void opengl_bind_vertex_layout(vertex_layout &layout, uint base_vertex = 0, ubyte* base_ptr = NULL);

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
