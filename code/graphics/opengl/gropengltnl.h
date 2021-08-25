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
#include "gropenglshader.h"
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

extern GLuint Shadow_map_texture;

struct opengl_vertex_bind {
	vertex_format_data::vertex_format format;
	GLint size;
	GLenum data_type;
	GLboolean normalized;
	opengl_vert_attrib::attrib_id attribute_id;
};

gr_buffer_handle gr_opengl_create_buffer(BufferType type, BufferUsageHint usage);

void opengl_bind_buffer_object(gr_buffer_handle handle);
void gr_opengl_update_buffer_data(gr_buffer_handle handle, size_t size, const void* data);
void gr_opengl_update_buffer_data_offset(gr_buffer_handle handle, size_t offset, size_t size, const void* data);
void gr_opengl_delete_buffer(gr_buffer_handle handle);
void gr_opengl_bind_uniform_buffer(uniform_block_type bind_point, size_t offset, size_t size, gr_buffer_handle buffer);
void* gr_opengl_map_buffer(gr_buffer_handle handle);
void gr_opengl_flush_mapped_buffer(gr_buffer_handle handle, size_t offset, size_t size);

/**
 * @brief Retrieves the OpenGL handle of a generic buffer handle
 * @param expected_type The expected type of the buffer. Mainly used for debug checking.
 * @param buffer_handle The handle of the generic buffer
 * @return The OpenGL handle ID
 */
GLuint opengl_buffer_get_id(GLenum expected_type, gr_buffer_handle buffer_handle);

void gr_opengl_update_transform_buffer(void* data, size_t size);

void opengl_tnl_init();
void opengl_tnl_shutdown();

void gr_opengl_render_model(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi);
void opengl_render_model_program(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, buffer_data *datap);

void opengl_tnl_set_material(material* material_info, bool set_base_map, bool set_clipping = true);
void opengl_tnl_set_material_distortion(distortion_material* material_info);
void opengl_tnl_set_material_particle(particle_material * material_info);
void opengl_tnl_set_material_movie(movie_material* material_info);
void opengl_tnl_set_material_batched(batched_bitmap_material * material_info);
void opengl_tnl_set_material_nanovg(nanovg_material * material_info);
void opengl_tnl_set_material_decal(decal_material * material_info);
void opengl_tnl_set_rocketui_material(interface_material* material_info);

void opengl_tnl_set_model_material(model_material *material_info);

void gr_opengl_set_viewport(int x, int y, int width, int height);

void opengl_bind_vertex_layout(vertex_layout &layout, GLuint vertexBuffer, GLuint indexBuffer, size_t base_offset = 0);

#endif //_GROPENGLTNL_H
