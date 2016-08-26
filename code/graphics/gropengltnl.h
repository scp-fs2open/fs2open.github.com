/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifndef _GROPENGLTNL_H
#define _GROPENGLTNL_H


#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"
#include "graphics/shadows.h"
#include "model/model.h"

#include <glad/glad.h>

extern GLint GL_max_elements_vertices;
extern GLint GL_max_elements_indices;

class poly_list;
class vertex_buffer;

extern float shadow_veryneardist;
extern float shadow_neardist;
extern float shadow_middist;
extern float shadow_fardist;
extern bool Rendering_to_shadow_map;

extern matrix4 GL_view_matrix;
extern matrix4 GL_model_view_matrix;
extern matrix4 GL_projection_matrix;
extern matrix4 GL_last_projection_matrix;

extern int GL_immediate_buffer_handle;

void gr_opengl_start_instance_matrix(const vec3d *offset, const matrix *rotation);
void gr_opengl_start_instance_angles(const vec3d *pos, const angles *rotation);
void gr_opengl_end_instance_matrix();
void gr_opengl_set_projection_matrix(float fov, float aspect, float z_near, float z_far);
void gr_opengl_end_projection_matrix();
void gr_opengl_set_view_matrix(const vec3d *pos, const matrix *orient);
void gr_opengl_end_view_matrix();
void gr_opengl_set_2d_matrix(/*int x, int y, int w, int h*/);
void gr_opengl_end_2d_matrix();
void gr_opengl_push_scale_matrix(const vec3d *scale_factor);
void gr_opengl_pop_scale_matrix();

void opengl_create_perspective_projection_matrix(matrix4 *out, float left, float right, float bottom, float top, float near_dist, float far_dist);
void opengl_create_orthographic_projection_matrix(matrix4* out, float left, float right, float bottom, float top, float near_dist, float far_dist);
void opengl_create_view_matrix(matrix4 *out, const vec3d *pos, const matrix *orient);

void gr_opengl_start_clip_plane();
void gr_opengl_end_clip_plane();

int gr_opengl_create_vertex_buffer(bool static_buffer);
int gr_opengl_create_index_buffer(bool static_buffer);
int gr_opengl_create_buffer();
bool gr_opengl_pack_buffer(const int buffer_id, vertex_buffer *vb);
bool gr_opengl_config_buffer(const int buffer_id, vertex_buffer *vb, bool update_ibuffer_only);
void gr_opengl_destroy_buffer(int idx);
void gr_opengl_set_buffer(int idx);
void gr_opengl_render_buffer(int start, vertex_buffer *bufferp, size_t texi, int flags);
void gr_opengl_render_to_env(int FACE);

void opengl_bind_buffer_object(int handle);
void gr_opengl_update_buffer_data(int handle, size_t size, void* data);
void gr_opengl_delete_buffer(int handle);

void gr_opengl_update_transform_buffer(void* data, size_t size);
void gr_opengl_set_transform_buffer_offset(size_t offset);

uint opengl_add_to_immediate_buffer(uint size, void *data);
void opengl_reset_immediate_buffer();

void gr_opengl_render_stream_buffer(int buffer_handle, size_t offset, size_t n_verts, int flags);

void gr_opengl_set_thrust_scale(float scale = -1.0f);
void gr_opengl_set_team_color(const team_color *colors);

void opengl_tnl_init();
void opengl_tnl_shutdown();

void gr_opengl_render_model(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi);
void opengl_render_model_program(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, buffer_data *datap);
void opengl_render_model_fixed(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer *bufferp, buffer_data *datap);

void opengl_tnl_set_material(material* material_info, bool set_base_map);
void opengl_tnl_set_material_distortion(distortion_material* material_info);
void opengl_tnl_set_material_particle(particle_material * material_info);

void opengl_tnl_set_model_material(model_material *material_info);

#endif //_GROPENGLTNL_H
