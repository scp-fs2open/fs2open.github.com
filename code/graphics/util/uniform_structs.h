#pragma once

#include "globalincs/pstypes.h"

namespace graphics {

/**
 * @file
 *
 * This file contains definitions for GPU uniform buffer structs. These structs must respect the std140 layout rules.
 * Read the OpenGL specification for the exact layout and padding rules.
 */

struct deferred_global_data {
	matrix4 shadow_mv_matrix;
	matrix4 shadow_proj_matrix[4];

	matrix4 inv_view_matrix;

	float veryneardist;
	float neardist;
	float middist;
	float fardist;

	float specFactor;
	float invScreenWidth;
	float invScreenHeight;

	float pad;
};

/**
 * @brief Data for one deferred light rendering call
 */
struct deferred_light_data {
	vec3d diffuseLightColor;
	float coneAngle;

	vec3d specLightColor;
	float coneInnerAngle;

	vec3d coneDir;
	float dualCone;

	vec3d scale;
	float lightRadius;

	vec3d lightDir;
	int lightType;

	int enable_shadows;

	float pad0[3]; // Struct size must be 16-bytes aligned because we use vec3s
};

struct model_light {
	vec4 position;
	vec3d diffuse_color;
	int light_type;
	vec3d spec_color;
	float attenuation;
	vec3d direction;

	float pad;
};

const size_t MAX_UNIFORM_LIGHTS = 8;

struct model_uniform_data {
	matrix4 modelViewMatrix;
	matrix4 modelMatrix;
	matrix4 viewMatrix;
	matrix4 projMatrix;
	matrix4 textureMatrix;
	matrix4 shadow_mv_matrix;
	matrix4 shadow_proj_matrix[4];
	matrix4 envMatrix;

	vec4 color;

	model_light lights[MAX_UNIFORM_LIGHTS];

	float extrudeWidth;
	float fogStart;
	float fogScale;
	int buffer_matrix_offset;

	vec4 clip_equation;

	float thruster_scale;
	int use_clip_plane;
	int n_lights;
	float defaultGloss;

	vec3d ambientFactor;
	int desaturate;

	vec3d diffuseFactor;
	int blend_alpha;

	vec3d emissionFactor;
	int overrideDiffuse;

	vec3d diffuseClr;
	int overrideGlow;

	vec3d glowClr;
	int overrideSpec;

	vec3d specClr;
	int alphaGloss;

	int gammaSpec;
	int envGloss;
	int alpha_spec;
	int effect_num;

	vec4 fogColor;

	vec3d base_color;
	float anim_timer;

	vec3d stripe_color;
	float vpwidth;

	float vpheight;
	int team_glow_enabled;
	float znear;
	float zfar;

	float veryneardist;
	float neardist;
	float middist;
	float fardist;

	vec2d normalAlphaMinMax;
	int sBasemapIndex;
	int sGlowmapIndex;

	int sSpecmapIndex;
	int sNormalmapIndex;
	int sAmbientmapIndex;
	int sMiscmapIndex;
};

enum class NanoVGShaderType: int32_t {
	FillGradient = 0, FillImage = 1, Simple = 2, Image = 3
};
struct nanovg_draw_data {
	float scissorMat[12]; // matrices are actually 3 vec4s

	float paintMat[12];

	vec4 innerCol;

	vec4 outerCol;

	vec2d scissorExt;
	vec2d scissorScale;

	vec2d extent;
	float radius;
	float feather;

	float strokeMult;
	float strokeThr;
	int texType;
	NanoVGShaderType type;

	vec2d viewSize;
	int texArrayIndex;

	float pad3;
};

struct decal_globals {
	matrix4 viewMatrix;
	matrix4 projMatrix;
	matrix4 invViewMatrix;
	matrix4 invProjMatrix;

	vec3d ambientLight;
	float pad0;

	vec2d viewportSize;
	float pad1[2];
};

struct decal_info {
	matrix4 model_matrix;
	matrix4 inv_model_matrix;

	vec3d decal_direction;
	float normal_angle_cutoff;

	int diffuse_index;
	int glow_index;
	int normal_index;
	float angle_fade_start;

	float alpha_scale;
	int diffuse_blend_mode;
	int glow_blend_mode;

	float pad;
};

}
