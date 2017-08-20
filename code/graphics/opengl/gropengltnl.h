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
#include "gropengl.h"
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

int gr_opengl_create_buffer(BufferType type, BufferUsageHint usage);

void opengl_bind_buffer_object(int handle);
void gr_opengl_update_buffer_data(int handle, size_t size, void* data);
void gr_opengl_update_buffer_data_offset(int handle, size_t offset, size_t size, void* data);
void gr_opengl_delete_buffer(int handle);
void gr_opengl_bind_uniform_buffer(uniform_block_type bind_point, size_t offset, size_t size, int buffer);

void gr_opengl_update_transform_buffer(void* data, size_t size);
void gr_opengl_set_transform_buffer_offset(size_t offset);

void opengl_tnl_init();
void opengl_tnl_shutdown();

void gr_opengl_render_model(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi);
void opengl_render_model_program(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, buffer_data *datap);

void opengl_tnl_set_material(material* material_info, bool set_base_map);
void opengl_tnl_set_material_distortion(distortion_material* material_info);
void opengl_tnl_set_material_particle(particle_material * material_info);
void opengl_tnl_set_material_movie(movie_material* material_info);
void opengl_tnl_set_material_batched(batched_bitmap_material * material_info);

void opengl_tnl_set_model_material(model_material *material_info);

void gr_opengl_set_viewport(int x, int y, int width, int height);

#endif //_GROPENGLTNL_H
