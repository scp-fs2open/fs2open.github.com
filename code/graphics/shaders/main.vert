#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex inputs - match FSO vertex layout (VertexAttributeLocation enum)
layout(location = 0) in vec4 vertPosition;
layout(location = 1) in vec4 vertColor;       // Not used by model shader, declared for pipeline compatibility
layout(location = 2) in vec4 vertTexCoord;
layout(location = 3) in vec3 vertNormal;
layout(location = 4) in vec4 vertTangent;
layout(location = 5) in float vertModelID;

// Model shader flags (from model_shader_flags.h)
const int MODEL_SDR_FLAG_LIGHT       = (1 << 0);
const int MODEL_SDR_FLAG_FOG         = (1 << 10);
const int MODEL_SDR_FLAG_TRANSFORM   = (1 << 11);
const int MODEL_SDR_FLAG_THRUSTER    = (1 << 13);

#define MAX_LIGHTS 8

struct model_light {
	vec4 position;

	vec3 diffuse_color;
	int light_type;

	vec3 direction;
	float attenuation;

	float ml_sourceRadius;
};

layout(set = 1, binding = 0, std140) uniform modelData {
	mat4 modelViewMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 textureMatrix;
	mat4 shadow_mv_matrix;
	mat4 shadow_proj_matrix[4];

	vec4 color;

	model_light lights[MAX_LIGHTS];

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
// Contains per-submodel transform matrices indexed by vertModelID + buffer_matrix_offset.
// The visibility flag is stored in transform[3].w: >= 0.9 means invisible.
layout(set = 1, binding = 3, std430) readonly buffer TransformBuffer {
	mat4 transforms[];
} transformBuf;

// Outputs to fragment shader
layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outTexCoord;
layout(location = 3) out vec3 outTangent;
layout(location = 4) out vec3 outBitangent;
layout(location = 5) out vec3 outTangentNormal;
layout(location = 6) out float outFogDist;

void main()
{
	mat4 orient = mat4(1.0);
	bool clipModel = false;

	// Batched submodel transforms: read per-submodel matrix from the SSBO
	if ((flags & MODEL_SDR_FLAG_TRANSFORM) != 0) {
		int id = int(vertModelID);
		orient = transformBuf.transforms[buffer_matrix_offset + id];
		clipModel = (orient[3].w >= 0.9);
		orient[3].w = 1.0;
	}

	vec4 texCoord = textureMatrix * vertTexCoord;
	vec4 vertex = vertPosition;

	// Thruster scale
	if ((flags & MODEL_SDR_FLAG_THRUSTER) != 0) {
		if (vertex.z < -1.5) {
			vertex.z *= thruster_scale;
		}
	}

	// Transform the normal into eye space and normalize the result.
	vec3 normal = normalize(mat3(modelViewMatrix) * mat3(orient) * vertNormal);
	vec4 position = modelViewMatrix * orient * vertex;

	gl_Position = projMatrix * position;

	// Clip invisible submodels by moving vertices off-screen
	if ((flags & MODEL_SDR_FLAG_TRANSFORM) != 0 && clipModel) {
		gl_Position = vec4(-2.0, -2.0, -2.0, 1.0);
	}

	// Setup stuff for normal maps and envmaps
	vec3 t = normalize(mat3(modelViewMatrix) * mat3(orient) * vertTangent.xyz);
	vec3 b = cross(normal, t) * vertTangent.w;
	outTangent = t;
	outBitangent = b;
	outTangentNormal = normal;

	// Fog
	if ((flags & MODEL_SDR_FLAG_FOG) != 0) {
		outFogDist = clamp((gl_Position.z - fogStart) * 0.75 * fogScale, 0.0, 1.0);
	} else {
		outFogDist = 0.0;
	}

	// Clip plane
	if (use_clip_plane != 0) {
		gl_ClipDistance[0] = dot(clip_equation, modelMatrix * orient * vertex);
	} else {
		gl_ClipDistance[0] = 1.0;
	}

	outPosition = position;
	outNormal = normal;
	outTexCoord = texCoord;
}
