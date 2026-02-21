#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shader_viewport_layer_array : enable

// Vertex inputs - match FSO vertex layout (VertexAttributeLocation enum)
layout(location = 0) in vec4 vertPosition;
layout(location = 5) in float vertModelID;

// Model shader flags (from model_shader_flags.h)
const int MODEL_SDR_FLAG_TRANSFORM   = (1 << 11);
const int MODEL_SDR_FLAG_THRUSTER    = (1 << 13);

layout(set = 1, binding = 0, std140) uniform modelData {
	mat4 modelViewMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 textureMatrix;
	mat4 shadow_mv_matrix;
	mat4 shadow_proj_matrix[4];

	vec4 color;

	// lights[8] — 8 * 48 bytes = 384 bytes
	// We don't use lights in shadow shader but must match UBO layout
	vec4 _light_pad[24];

	float outlineWidth;
	float fogStart;
	float fogScale;
	int buffer_matrix_offset;

	vec4 clip_equation;

	float thruster_scale;
	int use_clip_plane;
	int n_lights;
	float defaultGloss;

	vec3 ambientFactor;
	int desaturate;

	vec3 diffuseFactor;
	int blend_alpha;

	vec3 emissionFactor;
	int alphaGloss;

	int gammaSpec;
	int envGloss;
	int effect_num;
	int sBasemapIndex;

	vec4 fogColor;

	vec3 base_color;
	float anim_timer;

	vec3 stripe_color;
	float vpwidth;

	float vpheight;
	int team_glow_enabled;
	float znear;
	float zfar;

	float veryneardist;
	float neardist;
	float middist;
	float fardist;

	int sGlowmapIndex;
	int sSpecmapIndex;
	int sNormalmapIndex;
	int sAmbientmapIndex;

	int sMiscmapIndex;
	float alphaMult;
	int flags;
	float _pad0;
};

// Transform buffer for batched submodel rendering (set 1, binding 3)
layout(set = 1, binding = 3, std430) readonly buffer TransformBuffer {
	mat4 transforms[];
} transformBuf;

void main()
{
	mat4 orient = mat4(1.0);
	bool clipModel = false;

	// Batched submodel transforms
	if ((flags & MODEL_SDR_FLAG_TRANSFORM) != 0) {
		int id = int(vertModelID);
		orient = transformBuf.transforms[buffer_matrix_offset + id];
		clipModel = (orient[3].w >= 0.9);
		orient[3].w = 1.0;
	}

	vec4 vertex = vertPosition;

	// Thruster scale
	if ((flags & MODEL_SDR_FLAG_THRUSTER) != 0) {
		if (vertex.z < -1.5) {
			vertex.z *= thruster_scale;
		}
	}

	// modelViewMatrix = light_view * model_transform (set by gr_set_view_matrix)
	vec4 lightViewPos = modelViewMatrix * orient * vertex;
	gl_Position = shadow_proj_matrix[gl_InstanceIndex] * lightViewPos;

	// Clamp depth to [0, w] for Vulkan
	gl_Position.z = clamp(gl_Position.z, 0.0, gl_Position.w);

	// Route to cascade layer via instanced rendering
	gl_Layer = gl_InstanceIndex;

	// No clip plane in shadow pass
	gl_ClipDistance[0] = 1.0;

	// Clip invisible submodels
	if ((flags & MODEL_SDR_FLAG_TRANSFORM) != 0 && clipModel) {
		gl_Position = vec4(-2.0, -2.0, -2.0, 1.0);
	}
}
