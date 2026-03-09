#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

// Shared vertex attribute locations.
// Both backends (OpenGL and Vulkan) use these same location numbers
// for vertex shader inputs.
enum VertexAttributeLocation : uint32_t {
	VATTRIB_POSITION     = 0,   // vec2, vec3, or vec4
	VATTRIB_COLOR        = 1,   // vec3/vec4 (normalized u8 or float)
	VATTRIB_TEXCOORD     = 2,   // vec2 or vec4
	VATTRIB_NORMAL       = 3,   // vec3
	VATTRIB_TANGENT      = 4,   // vec4
	VATTRIB_MODELID      = 5,   // float
	VATTRIB_RADIUS       = 6,   // float
	VATTRIB_UVEC         = 7,   // vec3
	VATTRIB_MODEL_MATRIX = 8,   // Occupies locations 8-11

	NUM_VERTEX_ATTRIBS   = 9,
};

// Shared shader type info — matches the original opengl_shader_type_t layout.
struct ShaderTypeInfo {
	shader_type type_id;

	const char *vert;
	const char *frag;
	const char *geo;

	SCP_vector<VertexAttributeLocation> attributes;

	const char* description;

	bool spirv_shader;
};

// Shared shader variant info — matches the original opengl_shader_variant_t layout.
struct ShaderVariantInfo {
	shader_type type_id;

	bool use_geometry_sdr;

	int flag;
	SCP_string flag_text;

	SCP_vector<VertexAttributeLocation> attributes;

	const char* description;
};

// Lookup helper: find type info by shader_type. Returns nullptr if not found.
const ShaderTypeInfo* shader_get_type_info(shader_type type);

// Lookup helper: find variant info by (type, flag). Returns nullptr if not found.
const ShaderVariantInfo* shader_get_variant_info(shader_type type, int flag);

// Iterate all variants matching type whose flag bit is set in flags.
void shader_for_each_active_variant(shader_type type, unsigned int flags, const std::function<void(const ShaderVariantInfo&)>& fn);

// Build the variant #define header for a shader given type+flags.
SCP_string shader_build_variant_defines(shader_type type, unsigned int flags);

// Returns FXAA preprocessor defines for the given AA mode.
// gather4_alpha: whether the backend supports textureGather.
SCP_string shader_get_fxaa_defines(AntiAliasMode aa_mode, bool gather4_alpha);
