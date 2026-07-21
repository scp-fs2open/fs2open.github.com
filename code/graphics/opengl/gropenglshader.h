/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#ifndef _GROPENGLSHADER_H
#define _GROPENGLSHADER_H

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/shader_types.h"
#include "graphics/material.h"
#include "graphics/opengl/gropengl.h"
#include "graphics/util/UniformBuffer.h"

#include <glad/glad.h>

namespace opengl {
// Forward definition to avoid cyclic dependency
class ShaderProgram;
} // namespace opengl

enum shader_stage { SDR_STAGE_VERTEX, SDR_STAGE_FRAGMENT, SDR_STAGE_GEOMETRY };

struct opengl_vert_attrib {
	// Attribute location enum — aliases the shared VertexAttributeLocation enum
	// from shader_types.h.
	using attrib_id = VertexAttributeLocation;
	static constexpr attrib_id POSITION     = VATTRIB_POSITION;
	static constexpr attrib_id COLOR        = VATTRIB_COLOR;
	static constexpr attrib_id TEXCOORD     = VATTRIB_TEXCOORD;
	static constexpr attrib_id NORMAL       = VATTRIB_NORMAL;
	static constexpr attrib_id TANGENT      = VATTRIB_TANGENT;
	static constexpr attrib_id MODEL_ID     = VATTRIB_MODELID;
	static constexpr attrib_id RADIUS       = VATTRIB_RADIUS;
	static constexpr attrib_id UVEC         = VATTRIB_UVEC;
	static constexpr attrib_id MODEL_MATRIX = VATTRIB_MODEL_MATRIX;
	static constexpr attrib_id NUM_ATTRIBS  = NUM_VERTEX_ATTRIBS;

	attrib_id attribute_id;
	SCP_string name;
	vec4 default_value;
};
namespace std {
template<> struct hash<opengl_vert_attrib::attrib_id> {
	size_t operator()(const opengl_vert_attrib::attrib_id& data) const {
		return std::hash<size_t>()(static_cast<size_t>(data));
	}
};
}

extern SCP_vector<opengl_vert_attrib> GL_vertex_attrib_info;

struct geometry_sdr_params
{
	int input_type;
	int output_type;
	int vertices_out;
};

struct opengl_shader_file_t {
	const char *vert;
	const char *frag;
	const char *geo;

	int flags;

	SCP_vector<const char*> uniforms;

	SCP_vector<const char*> attributes;

	const char* description;
};

struct opengl_shader_uniform_reference_t {
	unsigned int flag;

	SCP_vector<const char*> uniforms;

	SCP_vector<const char*> attributes;

	SCP_vector<const char*> uniform_blocks;

	const char* name;
};

typedef struct opengl_shader_uniform_t {
	SCP_string text_id;
	GLint location;

	opengl_shader_uniform_t() : location(-1) {}
} opengl_shader_uniform_t;

typedef struct opengl_shader_t {
	std::unique_ptr<opengl::ShaderProgram> program;

	shader_type shader;
	unsigned int flags;
	int flags2;

	opengl_shader_t();

	opengl_shader_t(opengl_shader_t&& other) noexcept = default;
	opengl_shader_t& operator=(opengl_shader_t&& other) noexcept = default;

	opengl_shader_t(const opengl_shader_t&) = delete;
	opengl_shader_t& operator=(const opengl_shader_t&) = delete;
} opengl_shader_t;

extern SCP_vector<opengl_shader_t> GL_shader;

extern opengl_shader_t *Current_shader;

int gr_opengl_maybe_create_shader(shader_type shader_t, unsigned int flags);
void gr_opengl_recompile_all_shaders(const std::function<void(size_t, size_t)>& progress_callback = nullptr);
void opengl_delete_shader(int sdr_handle);
void opengl_shader_set_current(opengl_shader_t *shader_obj = NULL);
void opengl_shader_set_current(int handle);

void opengl_shader_init();
void opengl_shader_shutdown();

int opengl_compile_shader(shader_type sdr, uint flags);

void opengl_shader_set_passthrough(bool textured, bool hdr);

void opengl_shader_set_default_material(bool textured, bool alpha, vec4* clr, float color_scale, uint32_t array_index, const material::clip_plane& clip_plane);

template <typename T, typename DataCallback>
void opengl_set_generic_uniform_data(DataCallback cb)
{
	auto buffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1, sizeof(T));

	auto bufferPtr = buffer.aligner().addTypedElement<T>();
	cb(bufferPtr);

	buffer.submitData();

	gr_bind_uniform_buffer(uniform_block_type::GenericData, buffer.getBufferOffset(0), sizeof(T),
	                       buffer.bufferHandle());
}

#endif	// _GROPENGLSHADER_H
