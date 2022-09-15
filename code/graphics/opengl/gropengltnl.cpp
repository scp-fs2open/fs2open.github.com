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

#include "globalincs/alphacolors.h"
#include "globalincs/systemvars.h"

#include "ShaderProgram.h"
#include "gropengldeferred.h"
#include "gropengldraw.h"
#include "gropenglshader.h"
#include "gropenglstate.h"
#include "gropengltexture.h"
#include "gropengltnl.h"

#include "cmdline/cmdline.h"
#include "def_files/def_files.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/light.h"
#include "graphics/material.h"
#include "graphics/matrix.h"
#include "graphics/shadows.h"
#include "graphics/util/uniform_structs.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "options/Option.h"
#include "particle/particle.h"
#include "render/3d.h"
#include "weapon/trails.h"

extern int GLOWMAP;
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
int Shadow_texture_size = 0;

gr_buffer_handle Transform_buffer_handle;

SCP_unordered_map<vertex_layout, GLuint> Stored_vertex_arrays;

static opengl_vertex_bind GL_array_binding_data[] =
	{
		{ vertex_format_data::POSITION4,	4, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::POSITION	},
		{ vertex_format_data::POSITION3,	3, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::POSITION	},
		{ vertex_format_data::POSITION2,	2, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::POSITION	},
		{ vertex_format_data::SCREEN_POS,	2, GL_INT,				GL_FALSE, opengl_vert_attrib::POSITION	},
		{ vertex_format_data::COLOR3,		3, GL_UNSIGNED_BYTE,	GL_TRUE,opengl_vert_attrib::COLOR		},
		{ vertex_format_data::COLOR4,		4, GL_UNSIGNED_BYTE,	GL_TRUE, opengl_vert_attrib::COLOR		},
		{ vertex_format_data::COLOR4F,		4, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::COLOR		},
		{ vertex_format_data::TEX_COORD2,	2, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::TEXCOORD	},
		{ vertex_format_data::TEX_COORD3,	3, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::TEXCOORD	},
		{ vertex_format_data::NORMAL,		3, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::NORMAL	},
		{ vertex_format_data::TANGENT,		4, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::TANGENT	},
		{ vertex_format_data::MODEL_ID,		1, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::MODEL_ID	},
		{ vertex_format_data::RADIUS,		1, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::RADIUS	},
		{ vertex_format_data::UVEC,			3, GL_FLOAT,			GL_FALSE, opengl_vert_attrib::UVEC		},
	};

struct opengl_buffer_object {
	GLuint buffer_id;
	GLenum type;
	GLenum gl_usage;
	BufferUsageHint usage;
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
			UNREACHABLE("Unhandled enum value!");
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
	    case BufferUsageHint::PersistentMapping:
		    return GL_NONE; // Dummy value
	    default:
			UNREACHABLE("Unhandled enum value!");
			return GL_INVALID_ENUM;
	}
}

static GLenum convertStencilOp(const StencilOperation stencil_op) {
	switch (stencil_op) {
	case StencilOperation::Keep:
		return GL_KEEP;
	case StencilOperation::Zero:
		return GL_ZERO;
	case StencilOperation::Replace:
		return GL_REPLACE;
	case StencilOperation::Increment:
		return GL_INCR;
	case StencilOperation::IncrementWrap:
		return GL_INCR_WRAP;
	case StencilOperation::Decrement:
		return GL_DECR;
	case StencilOperation::DecrementWrap:
		return GL_DECR_WRAP;
	case StencilOperation::Invert:
		return GL_INVERT;
	default:
		UNREACHABLE("Unhandled enum value encountered!");
		return GL_NONE;
	}
}

static GLenum convertComparisionFunction(ComparisionFunction func) {
	GLenum mode;
	switch (func) {
	case ComparisionFunction::Always:
		mode = GL_ALWAYS;
		break;
	case ComparisionFunction::Equal:
		mode = GL_EQUAL;
		break;
	case ComparisionFunction::Greater:
		mode = GL_GREATER;
		break;
	case ComparisionFunction::GreaterOrEqual:
		mode = GL_GEQUAL;
		break;
	case ComparisionFunction::Less:
		mode = GL_LESS;
		break;
	case ComparisionFunction::LessOrEqual:
		mode = GL_LEQUAL;
		break;
	case ComparisionFunction::Never:
		mode = GL_NEVER;
		break;
	case ComparisionFunction::NotEqual:
		mode = GL_NOTEQUAL;
		break;
	default:
		UNREACHABLE("Unhandled comparision function value!");
		mode = GL_ALWAYS;
		break;
	}
	return mode;
}

gr_buffer_handle opengl_create_buffer_object(GLenum type, GLenum gl_usage, BufferUsageHint usage)
{
	GR_DEBUG_SCOPE("Create buffer object");

	Assertion(usage != BufferUsageHint::PersistentMapping || GLAD_GL_ARB_buffer_storage != 0,
		"Persistent mapping is not supported by this OpenGL implementation!");

	opengl_buffer_object buffer_obj;

	buffer_obj.gl_usage = gl_usage;
	buffer_obj.usage = usage;
	buffer_obj.type = type;
	buffer_obj.size = 0;

	glGenBuffers(1, &buffer_obj.buffer_id);

	GL_buffer_objects.push_back(buffer_obj);

	return gr_buffer_handle((int)(GL_buffer_objects.size() - 1));
}

void opengl_bind_buffer_object(gr_buffer_handle handle)
{
	GR_DEBUG_SCOPE("Bind buffer handle");

	Assert(handle.isValid());
	Assert((size_t)handle.value() < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle.value()];

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
GLuint opengl_buffer_get_id(GLenum expected_type, gr_buffer_handle handle)
{
	Assert(handle.isValid());
	Assert((size_t)handle.value() < GL_buffer_objects.size());

	opengl_buffer_object& buffer_obj = GL_buffer_objects[handle.value()];

	Assertion(expected_type == buffer_obj.type, "Expected buffer type did not match the actual buffer type!");

	return buffer_obj.buffer_id;
}

void gr_opengl_update_buffer_data(gr_buffer_handle handle, size_t size, const void* data)
{
	// This has to be verified by the caller or else we will run into OPenGL errors
	Assertion(size > 0, "Buffer updates must include some data!");

	GR_DEBUG_SCOPE("Update buffer data");

	Assert(handle.isValid());
	Assert((size_t)handle.value() < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle.value()];

	opengl_bind_buffer_object(handle);

	if (buffer_obj.usage == BufferUsageHint::PersistentMapping) {
		Assertion(buffer_obj.size == 0, "Tried to resize a buffer for persistent mapping! This is not allowed.");
		Assertion(GLAD_GL_ARB_buffer_storage != 0, "Persistent mapping was used when it wasn't supported!");
		glBufferStorage(buffer_obj.type, size, data, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
	} else {
		glBufferData(buffer_obj.type, size, data, buffer_obj.gl_usage);
	}

	GL_vertex_data_in -= buffer_obj.size;
	buffer_obj.size = size;
	GL_vertex_data_in += buffer_obj.size;
}

void gr_opengl_update_buffer_data_offset(gr_buffer_handle handle, size_t offset, size_t size, const void* data)
{
	GR_DEBUG_SCOPE("Update buffer data with offset");

	Assert(handle.isValid());
	Assert((size_t)handle.value() < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle.value()];

	Assertion(buffer_obj.usage != BufferUsageHint::PersistentMapping,
	          "Persistently mapped buffers may not be updated!");

	opengl_bind_buffer_object(handle);

	glBufferSubData(buffer_obj.type, offset, size, data);
}
void* gr_opengl_map_buffer(gr_buffer_handle handle)
{
	GR_DEBUG_SCOPE("Map buffer");

	Assert(handle.isValid());
	Assert((size_t)handle.value() < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle.value()];

	Assertion(buffer_obj.usage == BufferUsageHint::PersistentMapping,
	          "Buffer mapping is only supported for persistently mapped buffers!");
	Assertion(GLAD_GL_ARB_buffer_storage != 0, "Persistent mapping is not available in this OpenGL context!");

	opengl_bind_buffer_object(handle);
	return glMapBufferRange(buffer_obj.type, 0, buffer_obj.size,
	                        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
}
void gr_opengl_flush_mapped_buffer(gr_buffer_handle handle, size_t offset, size_t size)
{
	GR_DEBUG_SCOPE("Flush mapped buffer");

	Assert(handle.isValid());
	Assert((size_t)handle.value() < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle.value()];

	Assertion(buffer_obj.usage == BufferUsageHint::PersistentMapping,
	          "Buffer mapping is only supported for persistently mapped buffers!");
	Assertion(GLAD_GL_ARB_buffer_storage != 0, "Persistent mapping is not available in this OpenGL context!");

	opengl_bind_buffer_object(handle);
	glFlushMappedBufferRange(buffer_obj.type, offset, size);
}

void gr_opengl_delete_buffer(gr_buffer_handle handle)
{
	if (GL_buffer_objects.size() == 0) return;

	GR_DEBUG_SCOPE("Deleting buffer");

	Assert(handle.isValid());
	Assert((size_t)handle.value() < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle.value()];

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

gr_buffer_handle gr_opengl_create_buffer(BufferType type, BufferUsageHint usage)
{
	return opengl_create_buffer_object(convertBufferType(type), convertUsageHint(usage), usage);
}

void gr_opengl_bind_uniform_buffer(uniform_block_type bind_point, size_t offset, size_t size, gr_buffer_handle buffer) {
	GR_DEBUG_SCOPE("Bind uniform buffer range");

	GLuint buffer_handle = 0;

	if (buffer.isValid()) {
		Assert((size_t)buffer.value() < GL_buffer_objects.size());

		opengl_buffer_object &buffer_obj = GL_buffer_objects[buffer.value()];

		Assertion(buffer_obj.type == GL_UNIFORM_BUFFER, "Only uniform buffers are valid for this function!");
		buffer_handle = buffer_obj.buffer_id;
	}

	glBindBufferRange(GL_UNIFORM_BUFFER, static_cast<GLuint>(bind_point), buffer_handle, static_cast<GLintptr>(offset),
					  static_cast<GLsizeiptr>(size));
}

gr_buffer_handle opengl_create_texture_buffer_object()
{
	// create the buffer
	auto buffer_object_handle =
		opengl_create_buffer_object(GL_TEXTURE_BUFFER, GL_DYNAMIC_DRAW, BufferUsageHint::Dynamic);

	opengl_check_for_errors();

	opengl_buffer_object& buffer_obj = GL_buffer_objects[buffer_object_handle.value()];

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
	if (!Transform_buffer_handle.isValid() || size <= 0) {
		return;
	}

	gr_opengl_update_buffer_data(Transform_buffer_handle, size, data);

	opengl_buffer_object &buffer_obj = GL_buffer_objects[Transform_buffer_handle.value()];

	// need to rebind the buffer object to the texture buffer after it's been updated.
	// didn't have to do this on AMD and Nvidia drivers but Intel drivers seem to want it.
	glBindTexture(GL_TEXTURE_BUFFER, buffer_obj.texture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer_obj.buffer_id);
}

GLuint opengl_get_transform_buffer_texture()
{
	if (!Transform_buffer_handle.isValid()) {
		return 0;
	}

	return GL_buffer_objects[Transform_buffer_handle.value()].texture;
}

void opengl_destroy_all_buffers()
{
	for ( uint i = 0; i < GL_buffer_objects.size(); i++ ) {
		gr_opengl_delete_buffer(gr_buffer_handle(i));
	}

	GL_vertex_buffers_in_use = 0;
}

static bool opengl_init_shadow_framebuffer(int size, GLenum color_format)
{
	mprintf(("Trying to create %dx%d %d-bit shadow framebuffer\n", size, size, color_format == GL_RGBA32F ? 32 : 16));

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
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, size, size, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Shadow_map_depth_texture, 0);

	glGenTextures(1, &Shadow_map_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY);
	GL_state.Texture.Enable(Shadow_map_texture);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, color_format, size, size, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Shadow_map_texture, 0);

	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	GL_state.BindFrameBuffer(0);

	if (status == GL_FRAMEBUFFER_COMPLETE) {
		// Everything is fine
		mprintf(("Shadow framebuffer created successfully.\n"));
		Shadow_texture_size = size;
		return true;
	}

	// Clean up resources
	glDeleteTextures(1, &Shadow_map_texture);
	glDeleteTextures(1, &Shadow_map_depth_texture);
	glDeleteFramebuffers(1, &shadow_fbo);

	Shadow_map_texture       = 0;
	Shadow_map_depth_texture = 0;
	shadow_fbo               = 0;

	const char* error;
	switch (status) {
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		error = "Incomplete framebuffer attachment";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		error = "Framebuffer is missing an attachment";
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		error = "Framebuffer configuration is unsupported";
		break;
	default:
		error = "Unknown framebuffer status";
		break;
	}

	mprintf(("Failed to create framebuffer: %s\n", error));
	return false;
}

void opengl_tnl_init()
{
	Transform_buffer_handle = opengl_create_texture_buffer_object();

	if (Shadow_quality != ShadowQuality::Disabled) {
		int size;
		switch (Shadow_quality) {
		case ShadowQuality::Low:
			size = 512;
			break;
		case ShadowQuality::Medium:
			size = 1024;
			break;
		case ShadowQuality::High:
			size = 2048;
			break;
		case ShadowQuality::Ultra:
			size = 4096;
			break;
		default:
			size = 256;
			break;
		}

		if (!opengl_init_shadow_framebuffer(size, GL_RGBA32F)) {
			if (!opengl_init_shadow_framebuffer(size, GL_RGBA16F)) {
				mprintf(("Failed to create either 32 or 16-bit color shadow framebuffer. Disabling shadow support.\n"));
				Shadow_quality = ShadowQuality::Disabled;
			}
		}
	}

	gr_opengl_deferred_init();
}

void opengl_tnl_shutdown()
{
	for (auto& vao_entry : Stored_vertex_arrays) {
		glDeleteVertexArrays(1, &vao_entry.second);
	}
	Stored_vertex_arrays.clear();

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

void opengl_render_model_program(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, buffer_data *datap)
{
	GL_state.Texture.SetShaderMode(GL_TRUE);

	opengl_tnl_set_model_material(material_info);

	auto ibuffer = reinterpret_cast<GLubyte*>(vert_source->Index_offset);

	GLenum element_type = (datap->flags & VB_FLAG_LARGE_INDEX) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

	Assert(vert_source);
	Assertion(vert_source->Vbuffer_handle.isValid(), "The vertex data must be located in a GPU buffer!");
	Assertion(vert_source->Ibuffer_handle.isValid(), "The index values must be located in a GPU buffer!");

	// basic setup of all data
	opengl_bind_vertex_layout(bufferp->layout,
		opengl_buffer_get_id(GL_ARRAY_BUFFER, vert_source->Vbuffer_handle),
		opengl_buffer_get_id(GL_ELEMENT_ARRAY_BUFFER, vert_source->Ibuffer_handle));

	// If GL_ARB_gpu_shader5 is supprted then the instancing is handled by the geometry shader
	if (!GLAD_GL_ARB_gpu_shader5 && Rendering_to_shadow_map) {
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
			(GLsizei)datap->n_verts,
			element_type,
										  ibuffer + datap->index_offset,
										  4,
										  (GLint) (vert_source->Base_vertex_offset + bufferp->vertex_num_offset));
	} else {
		if (Cmdline_drawelements) {
			glDrawElementsBaseVertex(GL_TRIANGLES,
									 (GLsizei) datap->n_verts,
									 element_type,
									 ibuffer + datap->index_offset,
									 (GLint) (vert_source->Base_vertex_offset + bufferp->vertex_num_offset));
		} else {
			glDrawRangeElementsBaseVertex(GL_TRIANGLES,
										  datap->i_first,
										  datap->i_last,
										  (GLsizei) datap->n_verts,
										  element_type,
										  ibuffer + datap->index_offset,
										  (GLint) (vert_source->Base_vertex_offset + bufferp->vertex_num_offset));
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

extern bool Glowpoint_override;
bool Glowpoint_override_save;

extern bool gr_htl_projection_matrix_set;

void gr_opengl_shadow_map_start(matrix4 *shadow_view_matrix, const matrix *light_orient, vec3d* eye_pos)
{
	if (Shadow_quality == ShadowQuality::Disabled)
		return;

	GL_state.PushFramebufferState();
	GL_state.BindFrameBuffer(shadow_fbo);

	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buffers);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Rendering_to_shadow_map = true;
	Glowpoint_override_save = Glowpoint_override;
	Glowpoint_override = true;

	gr_htl_projection_matrix_set = true;

	gr_set_view_matrix(eye_pos, light_orient);

	*shadow_view_matrix = gr_view_matrix;

	glViewport(0, 0, Shadow_texture_size, Shadow_texture_size);
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
	gr_htl_projection_matrix_set = false;

	glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
	glScissor(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
}

void opengl_tnl_set_material(material* material_info, bool set_base_map, bool set_clipping)
{
	int shader_handle = material_info->get_shader_handle();
	int base_map = material_info->get_texture_map(TM_BASE_TYPE);
	vec4 clr = material_info->get_color();

	Assert(shader_handle >= 0);

	opengl_shader_set_current(shader_handle);

	if (material_info->has_buffer_blend_modes()) {
		Assertion(GLAD_GL_ARB_draw_buffers_blend != 0,
				  "Buffer blend modes are not supported at the moment! Query the capability before using this feature.");

		auto enable_blend = false;
		for (auto i = 0; i < (int) material::NUM_BUFFER_BLENDS; ++i) {
			auto mode = material_info->get_blend_mode(i);

			GL_state.SetAlphaBlendModei(i, mode);
			enable_blend = enable_blend || mode != ALPHA_BLEND_NONE;
		}
		GL_state.Blend(enable_blend ? GL_TRUE : GL_FALSE);
	} else {
		GL_state.SetAlphaBlendMode(material_info->get_blend_mode());
	}
	GL_state.SetZbufferType(material_info->get_depth_mode());

	gr_set_cull(material_info->get_cull_mode() ? 1 : 0);

	gr_zbias(material_info->get_depth_bias());

	gr_set_fill_mode(material_info->get_fill_mode());

	gr_set_texture_addressing(material_info->get_texture_addressing());

	if (set_clipping) {
		// Only set the clipping state if explicitly requested by the caller to avoid unnecessary state changes
		auto& clip_params = material_info->get_clip_plane();
		if (!clip_params.enabled) {
			GL_state.ClipDistance(0, false);
		} else {
			Assertion(Current_shader != nullptr && (Current_shader->shader == SDR_TYPE_MODEL
				|| Current_shader->shader == SDR_TYPE_DEFAULT_MATERIAL),
					  "Clip planes are not supported by this shader!");

			GL_state.ClipDistance(0, true);
		}
	}

	GL_state.StencilMask(material_info->get_stencil_mask());

	auto& stencilFunc = material_info->get_stencil_func();
	GL_state.StencilFunc(convertComparisionFunction(stencilFunc.compare), stencilFunc.ref, stencilFunc.mask);

	auto& frontStencilOp = material_info->get_front_stencil_op();
	GL_state.StencilOpSeparate(GL_FRONT,
							   convertStencilOp(frontStencilOp.stencilFailOperation),
							   convertStencilOp(frontStencilOp.depthFailOperation),
							   convertStencilOp(frontStencilOp.successOperation));
	auto& backStencilOp = material_info->get_back_stencil_op();
	GL_state.StencilOpSeparate(GL_BACK,
							   convertStencilOp(backStencilOp.stencilFailOperation),
							   convertStencilOp(backStencilOp.depthFailOperation),
							   convertStencilOp(backStencilOp.successOperation));

	GL_state.StencilTest(material_info->is_stencil_enabled() ? GL_TRUE : GL_FALSE);

	auto& color_mask = material_info->get_color_mask();
	GL_state.ColorMask(color_mask.x, color_mask.y, color_mask.z, color_mask.w);

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
										   material_info->get_clip_plane());
	}
}

void opengl_tnl_set_model_material(model_material *material_info)
{
	float u_scale, v_scale;

	opengl_tnl_set_material(material_info, false, false);

	if ( GL_state.CullFace() ) {
		GL_state.FrontFaceValue(GL_CW);
	}

	gr_set_center_alpha(material_info->get_center_alpha());

	Assert( Current_shader->shader == SDR_TYPE_MODEL );

	GL_state.Texture.SetShaderMode(GL_TRUE);

	if (Current_shader->flags & SDR_FLAG_MODEL_CLIP) {
		GL_state.ClipDistance(0, true);
	} else {
		GL_state.ClipDistance(0, false);
	}

	uint32_t array_index;
	if ( Current_shader->flags & SDR_FLAG_MODEL_DIFFUSE_MAP ) {
		Current_shader->program->Uniforms.setTextureUniform("sBasemap", 0);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_BASE_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, 0);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_GLOW_MAP ) {
		Current_shader->program->Uniforms.setTextureUniform("sGlowmap", 1);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_GLOW_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, 1);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SPEC_MAP ) {
		Current_shader->program->Uniforms.setTextureUniform("sSpecmap", 2);

		if ( material_info->get_texture_map(TM_SPEC_GLOSS_TYPE) > 0 ) {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_SPEC_GLOSS_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, 2);
		} else {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_SPECULAR_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, 2);
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_ENV_MAP ) {
		Current_shader->program->Uniforms.setTextureUniform("sEnvmap", 3);

		gr_opengl_tcache_set(ENVMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, &array_index, 3);

		Current_shader->program->Uniforms.setTextureUniform("sIrrmap", 11);

		gr_opengl_tcache_set(IRRMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, &array_index, 11);
		Assertion(array_index == 0, "Cube map arrays are not supported yet!");
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_NORMAL_MAP ) {
		Current_shader->program->Uniforms.setTextureUniform("sNormalmap", 4);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_NORMAL_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, 4);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_HEIGHT_MAP ) {
		Current_shader->program->Uniforms.setTextureUniform("sHeightmap", 5);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_HEIGHT_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, 5);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_AMBIENT_MAP ) {
		Current_shader->program->Uniforms.setTextureUniform("sAmbientmap", 6);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_AMBIENT_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, 6);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_MISC_MAP ) {
		Current_shader->program->Uniforms.setTextureUniform("sMiscmap", 7);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_MISC_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, 7);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SHADOWS ) {
		Current_shader->program->Uniforms.setTextureUniform("shadow_map", 8);

		GL_state.Texture.Enable(8, GL_TEXTURE_2D_ARRAY, Shadow_map_texture);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_ANIMATED ) {
		Current_shader->program->Uniforms.setTextureUniform("sFramebuffer", 9);

		if ( Scene_framebuffer_in_frame ) {
			GL_state.Texture.Enable(9, GL_TEXTURE_2D, Scene_effect_texture);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		} else {
			GL_state.Texture.Enable(9, GL_TEXTURE_2D, Framebuffer_fallback_texture_id);
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_TRANSFORM ) {
		Current_shader->program->Uniforms.setTextureUniform("transform_tex", 10);
		GL_state.Texture.Enable(10, GL_TEXTURE_BUFFER, opengl_get_transform_buffer_texture());
	}

	if ( Deferred_lighting ) {
		// don't blend if we're drawing to the g-buffers
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	}
}

void opengl_tnl_set_material_particle(particle_material * material_info)
{
	opengl_tnl_set_material(material_info, true);

	gr_matrix_set_uniforms();

	opengl_set_generic_uniform_data<graphics::generic_data::effect_data>(
		[&](graphics::generic_data::effect_data* data) {
			data->window_width  = (float)gr_screen.max_w;
			data->window_height = (float)gr_screen.max_h;
			data->nearZ         = Min_draw_distance;
			data->farZ          = Max_draw_distance;
			data->srgb          = High_dynamic_range ? 1 : 0;
			data->blend_alpha   = material_info->get_blend_mode() != ALPHA_BLEND_ADDITIVE;

			if (Cmdline_no_deferred_lighting) {
				data->linear_depth = 0;
			} else {
				data->linear_depth = 1;
			}
		});

	Current_shader->program->Uniforms.setTextureUniform("baseMap", 0);
	Current_shader->program->Uniforms.setTextureUniform("depthMap", 1);

	if (!Cmdline_no_deferred_lighting) {
		Assert(Scene_position_texture != 0);

		GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_position_texture);
	} else {
		Assert(Scene_depth_texture != 0);

		GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_depth_texture);
	}
}
void opengl_tnl_set_material_batched(batched_bitmap_material* material_info)
{
	// This material assumes that the array index is supplied via the vertex attributes
	opengl_tnl_set_material(material_info, true);

	opengl_set_generic_uniform_data<graphics::generic_data::batched_data>(
		[&](graphics::generic_data::batched_data* data) {
			data->intensity = material_info->get_color_scale();
			data->color     = material_info->get_color();
		});

	gr_matrix_set_uniforms();

	Current_shader->program->Uniforms.setTextureUniform("baseMap", 0);
}

void opengl_tnl_set_material_distortion(distortion_material* material_info)
{
	opengl_tnl_set_material(material_info, true);

	gr_matrix_set_uniforms();

	Current_shader->program->Uniforms.setTextureUniform("baseMap", 0);
	Current_shader->program->Uniforms.setTextureUniform("depthMap", 1);

	opengl_set_generic_uniform_data<graphics::generic_data::effect_distort_data>(
		[&](graphics::generic_data::effect_distort_data* data) {
			data->window_width  = (float)gr_screen.max_w;
			data->window_height = (float)gr_screen.max_h;

			if (material_info->get_thruster_rendering()) {
				data->use_offset = 1.0f;
			} else {
				data->use_offset = 0.0f;
			}
		});

	Current_shader->program->Uniforms.setTextureUniform("frameBuffer", 2);
	GL_state.Texture.Enable(2, GL_TEXTURE_2D, Scene_effect_texture);

	Current_shader->program->Uniforms.setTextureUniform("distMap", 3);
	if (material_info->get_thruster_rendering()) {
		GL_state.Texture.Enable(3, GL_TEXTURE_2D, Distortion_texture[!Distortion_switch]);
	} else {
		// Disable this texture unit
		GL_state.Texture.Enable(3, GL_TEXTURE_2D, 0);
	}

	Assert(Scene_depth_texture != 0);

	GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_depth_texture);
}

void opengl_tnl_set_material_movie(movie_material* material_info) {
	opengl_tnl_set_material(material_info, false);

	gr_matrix_set_uniforms();

	Current_shader->program->Uniforms.setTextureUniform("ytex", 0);
	Current_shader->program->Uniforms.setTextureUniform("utex", 1);
	Current_shader->program->Uniforms.setTextureUniform("vtex", 2);

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
void opengl_tnl_set_material_nanovg(nanovg_material* material_info) {
	opengl_tnl_set_material(material_info, true);

	Current_shader->program->Uniforms.setTextureUniform("nvg_tex", 0);
}

void opengl_tnl_set_material_decal(decal_material* material_info) {
	opengl_tnl_set_material(material_info, false);

	float u_scale, v_scale;
	uint32_t array_index;

	if (gr_opengl_tcache_set(material_info->get_texture_map(TM_BASE_TYPE),
							 material_info->get_texture_type(),
							 &u_scale,
							 &v_scale,
							 &array_index,
							 0)) {
		Current_shader->program->Uniforms.setTextureUniform("diffuseMap", 0);
	}

	if (gr_opengl_tcache_set(material_info->get_texture_map(TM_GLOW_TYPE),
							 material_info->get_texture_type(),
							 &u_scale,
							 &v_scale,
							 &array_index,
							 1)) {
		Current_shader->program->Uniforms.setTextureUniform("glowMap", 1);
	}

	if (gr_opengl_tcache_set(material_info->get_texture_map(TM_NORMAL_TYPE),
							 material_info->get_texture_type(),
							 &u_scale,
							 &v_scale,
							 &array_index,
							 2)) {
		Current_shader->program->Uniforms.setTextureUniform("normalMap", 2);
	}

	GL_state.Texture.Enable(3, GL_TEXTURE_2D, Scene_depth_texture);
	Current_shader->program->Uniforms.setTextureUniform("gDepthBuffer", 3);

	if (Current_shader->flags & SDR_FLAG_DECAL_USE_NORMAL_MAP) {
		GL_state.Texture.Enable(4, GL_TEXTURE_2D, Scene_normal_texture);
		Current_shader->program->Uniforms.setTextureUniform("gNormalBuffer", 4);
	}
}

void opengl_tnl_set_rocketui_material(interface_material* material_info)
{
	opengl_tnl_set_material(material_info, false);

	Current_shader->program->Uniforms.setTextureUniform("baseMap", 0);

	uint32_t baseMapIndex = 0;
	if (material_info->is_textured()) {
		float u_scale, v_scale;
		if (!gr_opengl_tcache_set(material_info->get_texture_map(TM_BASE_TYPE), material_info->get_texture_type(),
								  &u_scale, &v_scale, &baseMapIndex)) {
			mprintf(("WARNING: Error setting bitmap texture (%i)!\n", material_info->get_texture_map(TM_BASE_TYPE)));
		}
	}

	const vec2d& offset = material_info->get_offset();
	opengl_set_generic_uniform_data<graphics::generic_data::rocketui_data>(
		[&](graphics::generic_data::rocketui_data* data) {
			data->projMatrix = gr_projection_matrix;

			data->offset = offset;
			data->textured = material_info->is_textured() ? GL_TRUE : GL_FALSE;
			data->baseMapIndex = baseMapIndex;

			data->horizontalSwipeOffset = material_info->get_horizontal_swipe();
		});
}

void gr_opengl_set_viewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
}

void opengl_bind_vertex_component(const vertex_format_data &vert_component, size_t base_offset)
{
	opengl_vertex_bind &bind_info = GL_array_binding_data[vert_component.format_type];
	opengl_vert_attrib &attrib_info = GL_vertex_attrib_info[bind_info.attribute_id];

	Assert(bind_info.attribute_id == attrib_info.attribute_id);

	GLubyte *data_src = reinterpret_cast<GLubyte*>(base_offset) + vert_component.offset;

	if ( Current_shader != NULL ) {
		// grabbing a vertex attribute is dependent on what current shader has been set. i hope no one calls opengl_bind_vertex_layout before opengl_set_current_shader
		GLint index = opengl_shader_get_attribute(attrib_info.attribute_id);

		if ( index >= 0 ) {
			GL_state.Array.EnableVertexAttrib(index);
			GL_state.Array.VertexAttribPointer(index, bind_info.size, bind_info.data_type, bind_info.normalized, (GLsizei)vert_component.stride, data_src);
		}
	}
}

void opengl_bind_dynamic_layout(vertex_layout& layout, size_t base_offset) {
	GL_state.BindVertexArray(GL_vao);
	GL_state.Array.BindPointersBegin();

	size_t num_vertex_bindings = layout.get_num_vertex_components();

	for ( size_t i = 0; i < num_vertex_bindings; ++i ) {
		opengl_bind_vertex_component(*layout.get_vertex_component(i), base_offset);
	}

	GL_state.Array.BindPointersEnd();
}

void opengl_bind_vertex_array(const vertex_layout& layout) {
	auto iter = Stored_vertex_arrays.find(layout);
	if (iter != Stored_vertex_arrays.end()) {
		// Found existing vertex array!
		GL_state.BindVertexArray(iter->second);
		return;
	}

	GR_DEBUG_SCOPE("Create Vertex array");

	GLuint vao;
	glGenVertexArrays(1, &vao);
	GL_state.BindVertexArray(vao);

	for (size_t i = 0; i < layout.get_num_vertex_components(); ++i) {
		auto component = layout.get_vertex_component(i);

		auto& bind_info = GL_array_binding_data[component->format_type];
		auto& attrib_info = GL_vertex_attrib_info[bind_info.attribute_id];

		auto attribIndex = attrib_info.attribute_id;

		glEnableVertexAttribArray(attribIndex);
		glVertexAttribFormat(attribIndex,
							 bind_info.size,
							 bind_info.data_type,
							 bind_info.normalized,
							 static_cast<GLuint>(component->offset));

		// Currently, all vertex data comes from one buffer.
		glVertexAttribBinding(attribIndex, 0);
	}

	Stored_vertex_arrays.insert(std::make_pair(layout, vao));
}
void opengl_bind_vertex_layout(vertex_layout &layout, GLuint vertexBuffer, GLuint indexBuffer, size_t base_offset)
{
	GR_DEBUG_SCOPE("Bind vertex layout");

	if (!GLAD_GL_ARB_vertex_attrib_binding) {
		// We don't have support for the new vertex binding functions so fall back to the single VAO implementation
		GL_state.Array.BindArrayBuffer(vertexBuffer);
		GL_state.Array.BindElementBuffer(indexBuffer);

		opengl_bind_dynamic_layout(layout, base_offset);
		return;
	}

	opengl_bind_vertex_array(layout);

	GL_state.Array.BindVertexBuffer(0,
									vertexBuffer,
									static_cast<GLintptr>(base_offset),
									static_cast<GLsizei>(layout.get_vertex_stride()));
	GL_state.Array.BindElementBuffer(indexBuffer);
}
