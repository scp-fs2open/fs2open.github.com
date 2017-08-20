/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifdef _WIN32
#include <windows.h>
#endif

#include "cmdline/cmdline.h"
#include "def_files/def_files.h"
#include "globalincs/alphacolors.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "graphics/grinternal.h"
#include "gropengldraw.h"
#include "gropenglshader.h"
#include "gropenglstate.h"
#include "gropengltexture.h"
#include "gropengldeferred.h"
#include "gropengltnl.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "weapon/trails.h"
#include "particle/particle.h"
#include "graphics/shadows.h"
#include "graphics/material.h"
#include "graphics/light.h"

extern int GLOWMAP;
extern int CLOAKMAP;
extern int SPECMAP;
extern int SPECGLOSSMAP;
extern int NORMMAP;
extern int MISCMAP;
extern int HEIGHTMAP;
extern int G3_user_clip;
extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;

extern bool Basemap_override;
extern bool Envmap_override;
extern bool Specmap_override;
extern bool Normalmap_override;
extern bool Heightmap_override;
extern bool Shadow_override;

size_t GL_vertex_data_in = 0;

GLint GL_max_elements_vertices = 4096;
GLint GL_max_elements_indices = 4096;

GLuint Shadow_map_texture = 0;
GLuint Shadow_map_depth_texture = 0;
GLuint shadow_fbo = 0;
bool Rendering_to_shadow_map = false;

int Transform_buffer_handle = -1;

struct opengl_buffer_object {
	GLuint buffer_id;
	GLenum type;
	GLenum usage;
	size_t size;

	GLuint texture;	// for texture buffer objects
};

static SCP_vector<opengl_buffer_object> GL_buffer_objects;
static int GL_vertex_buffers_in_use = 0;

static GLenum convertBufferType(BufferType type) {
	switch (type) {
		case BufferType::Vertex:
			return GL_ARRAY_BUFFER;
		case BufferType::Index:
			return GL_ELEMENT_ARRAY_BUFFER;
		case BufferType::Uniform:
			return GL_UNIFORM_BUFFER;
		default:
			Assertion(false, "Unhandled enum value!");
			return GL_INVALID_ENUM;
	}
}

static GLenum convertUsageHint(BufferUsageHint usage) {
	switch(usage) {
		case BufferUsageHint::Static:
			return GL_STATIC_DRAW;
		case BufferUsageHint::Dynamic:
			return GL_DYNAMIC_DRAW;
		case BufferUsageHint::Streaming:
			return GL_STREAM_DRAW;
		default:
			Assertion(false, "Unhandled enum value!");
			return GL_INVALID_ENUM;
	}
}

int opengl_create_buffer_object(GLenum type, GLenum usage)
{
	GR_DEBUG_SCOPE("Create buffer object");

	opengl_buffer_object buffer_obj;

	buffer_obj.usage = usage;
	buffer_obj.type = type;
	buffer_obj.size = 0;

	glGenBuffers(1, &buffer_obj.buffer_id);

	GL_buffer_objects.push_back(buffer_obj);

	return (int)(GL_buffer_objects.size() - 1);
}

void opengl_bind_buffer_object(int handle)
{
	GR_DEBUG_SCOPE("Bind buffer handle");

	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	switch ( buffer_obj.type ) {
	case GL_ARRAY_BUFFER:
		GL_state.Array.BindArrayBuffer(buffer_obj.buffer_id);
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		GL_state.Array.BindElementBuffer(buffer_obj.buffer_id);
		break;
	case GL_TEXTURE_BUFFER:
		GL_state.Array.BindTextureBuffer(buffer_obj.buffer_id);
		break;
	case GL_UNIFORM_BUFFER:
		GL_state.Array.BindUniformBuffer(buffer_obj.buffer_id);
		break;
	default:
		Int3();
		return;
		break;
	}
}

void gr_opengl_update_buffer_data(int handle, size_t size, void* data)
{
	// This has to be verified by the caller or else we will run into OPenGL errors
	Assertion(size > 0, "Buffer updates must include some data!");

	GR_DEBUG_SCOPE("Update buffer data");

	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	opengl_bind_buffer_object(handle);

	if (size <= buffer_obj.size && buffer_obj.type == GL_UNIFORM_BUFFER) {
		// Uniform buffer can use unsychronized buffer mapping since those are always synchronized by us
		// We also don't care about the previous data and tell OpenGL to map the buffer unsynchronized.
		auto ptr = glMapBufferRange(buffer_obj.type,
									0,
									size,
									GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
		if (ptr == nullptr) {
			// Something went wrong, try subdata instead
			glBufferSubData(buffer_obj.type, 0, size, data);
		} else {
			memcpy(ptr, data, size);

			glUnmapBuffer(buffer_obj.type);
		}
	} else {
		GL_vertex_data_in -= buffer_obj.size;
		buffer_obj.size = size;
		GL_vertex_data_in += buffer_obj.size;

		glBufferData(buffer_obj.type, size, data, buffer_obj.usage);
	}
}

void gr_opengl_update_buffer_data_offset(int handle, size_t offset, size_t size, void* data)
{
	GR_DEBUG_SCOPE("Update buffer data with offset");

	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	opengl_bind_buffer_object(handle);
	
	glBufferSubData(buffer_obj.type, offset, size, data);
}

void gr_opengl_delete_buffer(int handle)
{
	GR_DEBUG_SCOPE("Deleting buffer");

	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	// de-bind the buffer point so we can clear the recorded state.
	switch ( buffer_obj.type ) {
	case GL_ARRAY_BUFFER:
		GL_state.Array.BindArrayBuffer(0);
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		GL_state.Array.BindElementBuffer(0);
		break;
	case GL_TEXTURE_BUFFER:
		GL_state.Array.BindTextureBuffer(0);
		break;
	case GL_UNIFORM_BUFFER:
		GL_state.Array.BindUniformBuffer(0);
		break;
	default:
		Int3();
		return;
		break;
	}

	if ( buffer_obj.type == GL_TEXTURE_BUFFER ) {
		glDeleteTextures(1, &buffer_obj.texture);
	}

	GL_vertex_data_in -= buffer_obj.size;

	glDeleteBuffers(1, &buffer_obj.buffer_id);
}

int gr_opengl_create_buffer(BufferType type, BufferUsageHint usage)
{
	return opengl_create_buffer_object(convertBufferType(type), convertUsageHint(usage));
}

void gr_opengl_bind_uniform_buffer(uniform_block_type bind_point, size_t offset, size_t size, int buffer) {
	GR_DEBUG_SCOPE("Bind uniform buffer range");

	GLuint buffer_handle = 0;

	if (buffer != -1) {
		Assert(buffer >= 0);
		Assert((size_t)buffer < GL_buffer_objects.size());

		opengl_buffer_object &buffer_obj = GL_buffer_objects[buffer];

		Assertion(buffer_obj.type == GL_UNIFORM_BUFFER, "Only uniform buffers are valid for this function!");
		buffer_handle = buffer_obj.buffer_id;
	}

	glBindBufferRange(GL_UNIFORM_BUFFER, static_cast<GLuint>(bind_point), buffer_handle, static_cast<GLintptr>(offset),
					  static_cast<GLsizeiptr>(size));
}

int opengl_create_texture_buffer_object()
{
	// create the buffer
	int buffer_object_handle = opengl_create_buffer_object(GL_TEXTURE_BUFFER, GL_DYNAMIC_DRAW);

	opengl_check_for_errors();

	opengl_buffer_object &buffer_obj = GL_buffer_objects[buffer_object_handle];

	// create the texture
	glGenTextures(1, &buffer_obj.texture);
	glBindTexture(GL_TEXTURE_BUFFER, buffer_obj.texture);

	gr_opengl_update_buffer_data(buffer_object_handle, 100, NULL);

	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer_obj.buffer_id);

	opengl_check_for_errors();

	return buffer_object_handle;
}

void gr_opengl_update_transform_buffer(void* data, size_t size)
{
	if ( Transform_buffer_handle < 0 || size <= 0 ) {
		return;
	}

	gr_opengl_update_buffer_data(Transform_buffer_handle, size, data);

	opengl_buffer_object &buffer_obj = GL_buffer_objects[Transform_buffer_handle];

	// need to rebind the buffer object to the texture buffer after it's been updated.
	// didn't have to do this on AMD and Nvidia drivers but Intel drivers seem to want it.
	glBindTexture(GL_TEXTURE_BUFFER, buffer_obj.texture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer_obj.buffer_id);
}

GLuint opengl_get_transform_buffer_texture()
{
	if ( Transform_buffer_handle < 0 ) {
		return 0;
	}

	return GL_buffer_objects[Transform_buffer_handle].texture;
}

void opengl_destroy_all_buffers()
{
	for ( uint i = 0; i < GL_buffer_objects.size(); i++ ) {
		gr_opengl_delete_buffer(i);
	}

	GL_vertex_buffers_in_use = 0;
}

void opengl_tnl_init()
{
	Transform_buffer_handle = opengl_create_texture_buffer_object();

	if ( Transform_buffer_handle < 0 ) {
		Cmdline_no_batching = true;
	}

	if(Cmdline_shadow_quality)
	{
		//Setup shadow map framebuffer
		glGenFramebuffers(1, &shadow_fbo);
		GL_state.BindFrameBuffer(shadow_fbo);

		glGenTextures(1, &Shadow_map_depth_texture);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY);
		GL_state.Texture.Enable(Shadow_map_depth_texture);
		
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);
		int size = (Cmdline_shadow_quality == 2 ? 1024 : 512);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, size, size, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Shadow_map_depth_texture, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Shadow_map_depth_texture, 0);

		glGenTextures(1, &Shadow_map_texture);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY);
		GL_state.Texture.Enable(Shadow_map_texture);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, size, size, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, size, size, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Shadow_map_texture, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Shadow_map_texture, 0);

		GL_state.BindFrameBuffer(0);

		opengl_check_for_errors("post_init_framebuffer()");
	}

	gr_opengl_deferred_init();
}

void opengl_tnl_shutdown()
{
	gr_opengl_deferred_shutdown();

	if ( Shadow_map_depth_texture ) {
		glDeleteTextures(1, &Shadow_map_depth_texture);
		Shadow_map_depth_texture = 0;
	}

	if ( Shadow_map_texture ) {
		glDeleteTextures(1, &Shadow_map_texture);
		Shadow_map_texture = 0;
	}

	opengl_destroy_all_buffers();
}

static void opengl_init_arrays(indexed_vertex_source *vert_src, vertex_buffer *bufferp)
{
	Assertion(vert_src->Vbuffer_handle >= 0, "Vertex buffers require a valid buffer handle!");

	opengl_bind_buffer_object(vert_src->Vbuffer_handle);
	
	opengl_bind_vertex_layout(bufferp->layout);
}

void opengl_render_model_program(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, buffer_data *datap)
{
	GL_state.Texture.SetShaderMode(GL_TRUE);

	opengl_tnl_set_model_material(material_info);

	GLubyte *ibuffer = NULL;

	size_t start = 0;
	size_t end = (datap->n_verts - 1);
	size_t count = (end - (start * 3) + 1);

	GLenum element_type = (datap->flags & VB_FLAG_LARGE_INDEX) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

	Assert(vert_source);

	// basic setup of all data
	opengl_init_arrays(vert_source, bufferp);

	Assertion(vert_source->Ibuffer_handle >= 0, "The index values must be located in a GPU buffer!");

	opengl_bind_buffer_object(vert_source->Ibuffer_handle);

	// If GL_ARB_gpu_shader5 is supprted then the instancing is handled by the geometry shader
	if ( !GLAD_GL_ARB_gpu_shader5 && Rendering_to_shadow_map ) {
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, (GLsizei) count, element_type,
										  ibuffer + (datap->index_offset + start), 4, (GLint)bufferp->vertex_num_offset);
	} else {
		if (Cmdline_drawelements) {
			glDrawElementsBaseVertex(GL_TRIANGLES,
									 (GLsizei) count,
									 element_type,
									 ibuffer + (datap->index_offset + start),
									 (GLint) bufferp->vertex_num_offset);
		} else {
			glDrawRangeElementsBaseVertex(GL_TRIANGLES,
										  datap->i_first,
										  datap->i_last,
										  (GLsizei) count,
										  element_type,
										  ibuffer + (datap->index_offset + start),
										  (GLint) bufferp->vertex_num_offset);
		}
	}


	GL_state.Texture.SetShaderMode(GL_FALSE);
}

void gr_opengl_render_model(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi)
{
	Verify(bufferp != NULL);

	GL_CHECK_FOR_ERRORS("start of render_buffer()");

	buffer_data *datap = &bufferp->tex_buf[texi];

	opengl_render_model_program(material_info, vert_source, bufferp, datap);

	GL_CHECK_FOR_ERRORS("end of render_buffer()");
}

extern bool Scene_framebuffer_in_frame;
extern GLuint Framebuffer_fallback_texture_id;
extern GLuint Scene_depth_texture;
extern GLuint Scene_position_texture;
extern GLuint Distortion_texture[2];
extern int Distortion_switch;
void opengl_create_perspective_projection_matrix(matrix4 *out, float left, float right, float bottom, float top, float near_dist, float far_dist)
{
	memset(out, 0, sizeof(matrix4));

	out->a1d[0] = 2.0f * near_dist / (right - left);
	out->a1d[5] = 2.0f * near_dist / (top - bottom);
	out->a1d[8] = (right + left) / (right - left);
	out->a1d[9] = (top + bottom) / (top - bottom);
	out->a1d[10] = -(far_dist + near_dist) / (far_dist - near_dist);
	out->a1d[11] = -1.0f;
	out->a1d[14] = -2.0f * far_dist * near_dist / (far_dist - near_dist);
}

void opengl_create_orthographic_projection_matrix(matrix4* out, float left, float right, float bottom, float top, float near_dist, float far_dist)
{
	memset(out, 0, sizeof(matrix4));

	out->a1d[0] = 2.0f / (right - left);
	out->a1d[5] = 2.0f / (top - bottom);
	out->a1d[10] = -2.0f / (far_dist - near_dist);
	out->a1d[12] = -(right + left) / (right - left);
	out->a1d[13] = -(top + bottom) / (top - bottom);
	out->a1d[14] = -(far_dist + near_dist) / (far_dist - near_dist);
	out->a1d[15] = 1.0f;
}

void gr_opengl_set_clip_plane(const vec3d *clip_normal, const vec3d *clip_point)
{
	if ( clip_normal == NULL || clip_point == NULL ) {
		GL_state.ClipDistance(0, false);
	} else {
		Assertion(Current_shader != NULL &&
				  (Current_shader->shader == SDR_TYPE_MODEL || Current_shader->shader == SDR_TYPE_PASSTHROUGH_RENDER),
				  "Clip planes are not supported by this shader!");

		GL_state.ClipDistance(0, true);
	}
}

extern bool Glowpoint_override;
bool Glowpoint_override_save;

void gr_opengl_shadow_map_start(matrix4 *shadow_view_matrix, const matrix *light_orient)
{
	if ( !Cmdline_shadow_quality )
		return;

	GL_state.PushFramebufferState();
	GL_state.BindFrameBuffer(shadow_fbo);

	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buffers);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gr_set_lighting(false,false);
	
	Rendering_to_shadow_map = true;
	Glowpoint_override_save = Glowpoint_override;
	Glowpoint_override = true;

	gr_set_view_matrix(&Eye_position, light_orient);

	*shadow_view_matrix = gr_view_matrix;

	int size = (Cmdline_shadow_quality == 2 ? 1024 : 512);
	glViewport(0, 0, size, size);
}

void gr_opengl_shadow_map_end()
{
	if(!Rendering_to_shadow_map)
		return;

	gr_end_view_matrix();
	Rendering_to_shadow_map = false;

	gr_zbuffer_set(ZBUFFER_TYPE_FULL);
	GL_state.PopFramebufferState();

	Glowpoint_override = Glowpoint_override_save;

	glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
	glScissor(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
}

void opengl_tnl_set_material(material* material_info, bool set_base_map)
{
	int shader_handle = material_info->get_shader_handle();
	int base_map = material_info->get_texture_map(TM_BASE_TYPE);
	vec4 clr = material_info->get_color();

	Assert(shader_handle >= 0);

	opengl_shader_set_current(shader_handle);

	GL_state.SetAlphaBlendMode(material_info->get_blend_mode());
	GL_state.SetZbufferType(material_info->get_depth_mode());

	gr_set_cull(material_info->get_cull_mode() ? 1 : 0);

	gr_zbias(material_info->get_depth_bias());

	gr_set_fill_mode(material_info->get_fill_mode());

	auto& fog_params = material_info->get_fog();

	if ( fog_params.enabled ) {
		gr_fog_set(GR_FOGMODE_FOG, fog_params.r, fog_params.g, fog_params.b, fog_params.dist_near, fog_params.dist_far);
	} else {
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	gr_set_texture_addressing(material_info->get_texture_addressing());

	auto& clip_params = material_info->get_clip_plane();

	if ( material_info->is_clipped() ) {
		gr_opengl_set_clip_plane(&clip_params.normal, &clip_params.position);
	} else {
		gr_opengl_set_clip_plane(NULL, NULL);
	}

	// This is only needed for the passthrough shader
	uint32_t array_index = 0;
	if ( set_base_map && base_map >= 0 ) {
		float u_scale, v_scale;

		if ( !gr_opengl_tcache_set(base_map, material_info->get_texture_type(), &u_scale, &v_scale, &array_index) ) {
			mprintf(("WARNING: Error setting bitmap texture (%i)!\n", base_map));
		}
	}

	if ( Current_shader->shader == SDR_TYPE_DEFAULT_MATERIAL ) {
		opengl_shader_set_default_material(base_map >= 0,
										   material_info->get_texture_type() == TCACHE_TYPE_AABITMAP,
										   &clr,
										   material_info->get_color_scale(),
										   array_index,
										   clip_params);
	}
}

void opengl_tnl_set_model_material(model_material *material_info)
{
	float u_scale, v_scale;
	int render_pass = 0;

	opengl_tnl_set_material(material_info, false);

	if ( GL_state.CullFace() ) {
		GL_state.FrontFaceValue(GL_CW);
	}
	
	gr_set_center_alpha(material_info->get_center_alpha());

	Assert( Current_shader->shader == SDR_TYPE_MODEL );

	GL_state.Texture.SetShaderMode(GL_TRUE);

	uint32_t array_index;
	if ( Current_shader->flags & SDR_FLAG_MODEL_DIFFUSE_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sBasemap", render_pass);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_BASE_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);
		
		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_GLOW_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sGlowmap", render_pass);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_GLOW_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SPEC_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sSpecmap", render_pass);

		if ( material_info->get_texture_map(TM_SPEC_GLOSS_TYPE) > 0 ) {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_SPEC_GLOSS_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);
		} else {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_SPECULAR_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);
		}
		
		++render_pass;

		if ( Current_shader->flags & SDR_FLAG_MODEL_ENV_MAP ) {
			Current_shader->program->Uniforms.setUniformi("sEnvmap", render_pass);

			gr_opengl_tcache_set(ENVMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, &array_index, render_pass);
			Assertion(array_index == 0, "Cube map arrays are not supported yet!");

			++render_pass;
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_NORMAL_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sNormalmap", render_pass);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_NORMAL_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_HEIGHT_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sHeightmap", render_pass);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_HEIGHT_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_AMBIENT_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sAmbientmap", render_pass);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_AMBIENT_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_MISC_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sMiscmap", render_pass);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_MISC_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SHADOWS ) {
		Current_shader->program->Uniforms.setUniformi("shadow_map", render_pass);

		GL_state.Texture.Enable(render_pass, GL_TEXTURE_2D_ARRAY, Shadow_map_texture);

		++render_pass; // bump!
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_ANIMATED ) {
		Current_shader->program->Uniforms.setUniformi("sFramebuffer", render_pass);

		if ( Scene_framebuffer_in_frame ) {
			GL_state.Texture.Enable(render_pass, GL_TEXTURE_2D, Scene_effect_texture);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		} else {
			GL_state.Texture.Enable(render_pass, GL_TEXTURE_2D, Framebuffer_fallback_texture_id);
		}

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_TRANSFORM ) {
		Current_shader->program->Uniforms.setUniformi("transform_tex", render_pass);
		GL_state.Texture.Enable(render_pass, GL_TEXTURE_BUFFER, opengl_get_transform_buffer_texture());

		++render_pass;
	}

	if ( Deferred_lighting ) {
		// don't blend if we're drawing to the g-buffers
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	}
}

void opengl_tnl_set_material_particle(particle_material * material_info)
{
	opengl_tnl_set_material(material_info, true);

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", gr_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", gr_projection_matrix);

	Current_shader->program->Uniforms.setUniformi("baseMap", 0);
	Current_shader->program->Uniforms.setUniformi("depthMap", 1);
	Current_shader->program->Uniforms.setUniformf("window_width", (float)gr_screen.max_w);
	Current_shader->program->Uniforms.setUniformf("window_height", (float)gr_screen.max_h);
	Current_shader->program->Uniforms.setUniformf("nearZ", Min_draw_distance);
	Current_shader->program->Uniforms.setUniformf("farZ", Max_draw_distance);
	Current_shader->program->Uniforms.setUniformi("srgb", High_dynamic_range ? 1 : 0);
	Current_shader->program->Uniforms.setUniformi("blend_alpha", material_info->get_blend_mode() != ALPHA_BLEND_ADDITIVE);

	if ( Cmdline_no_deferred_lighting ) {
		Current_shader->program->Uniforms.setUniformi("linear_depth", 0);
	} else {
		Current_shader->program->Uniforms.setUniformi("linear_depth", 1);
	}

	if ( !Cmdline_no_deferred_lighting ) {
		Assert(Scene_position_texture != 0);

		GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_position_texture);
	} else {
		Assert(Scene_depth_texture != 0);

		GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_depth_texture);
	}
}
void opengl_tnl_set_material_batched(batched_bitmap_material* material_info) {
	// This material assumes that the array index is supplied via the vertex attributes
	opengl_tnl_set_material(material_info, true);

	Current_shader->program->Uniforms.setUniformf("intensity", material_info->get_color_scale());

	Current_shader->program->Uniforms.setUniform4f("color", material_info->get_color());

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", gr_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", gr_projection_matrix);

	Current_shader->program->Uniforms.setUniformi("baseMap", 0);
}

void opengl_tnl_set_material_distortion(distortion_material* material_info)
{
	opengl_tnl_set_material(material_info, true);

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", gr_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", gr_projection_matrix);

	Current_shader->program->Uniforms.setUniformi("baseMap", 0);
	Current_shader->program->Uniforms.setUniformi("depthMap", 1);
	Current_shader->program->Uniforms.setUniformf("window_width", (float)gr_screen.max_w);
	Current_shader->program->Uniforms.setUniformf("window_height", (float)gr_screen.max_h);
	Current_shader->program->Uniforms.setUniformf("nearZ", Min_draw_distance);
	Current_shader->program->Uniforms.setUniformf("farZ", Max_draw_distance);
	Current_shader->program->Uniforms.setUniformi("frameBuffer", 2);

	GL_state.Texture.Enable(2, GL_TEXTURE_2D, Scene_effect_texture);

	Current_shader->program->Uniforms.setUniformi("distMap", 3);

	if(material_info->get_thruster_rendering()) {
		GL_state.Texture.Enable(3, GL_TEXTURE_2D, Distortion_texture[!Distortion_switch]);

		Current_shader->program->Uniforms.setUniformf("use_offset", 1.0f);
	} else {
		// Disable this texture unit
		GL_state.Texture.Enable(3, GL_TEXTURE_2D, 0);

		Current_shader->program->Uniforms.setUniformf("use_offset", 0.0f);
	}

	Assert(Scene_depth_texture != 0);

	GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_depth_texture);
}

void opengl_tnl_set_material_movie(movie_material* material_info) {
	opengl_tnl_set_material(material_info, false);

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", gr_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", gr_projection_matrix);

	Current_shader->program->Uniforms.setUniformi("ytex", 0);
	Current_shader->program->Uniforms.setUniformi("utex", 1);
	Current_shader->program->Uniforms.setUniformi("vtex", 2);

	float u_scale, v_scale;
	uint32_t index;
	if ( !gr_opengl_tcache_set(material_info->getYtex(), material_info->get_texture_type(), &u_scale, &v_scale, &index, 0) ) {
		mprintf(("WARNING: Error setting bitmap texture (%i)!\n", material_info->getYtex()));
	}
	if ( !gr_opengl_tcache_set(material_info->getUtex(), material_info->get_texture_type(), &u_scale, &v_scale, &index, 1) ) {
		mprintf(("WARNING: Error setting bitmap texture (%i)!\n", material_info->getUtex()));
	}
	if ( !gr_opengl_tcache_set(material_info->getVtex(), material_info->get_texture_type(), &u_scale, &v_scale, &index, 2) ) {
		mprintf(("WARNING: Error setting bitmap texture (%i)!\n", material_info->getVtex()));
	}
}
void gr_opengl_set_viewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
}
