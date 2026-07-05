#include "graphics/shader_types.h"

#include "graphics/shadows.h"
#include "mod_table/mod_table.h"

// Pull in MODEL_SDR_FLAG_* constants for the variant table
#define MODEL_SDR_FLAG_MODE_CPP
#include "def_files/data/effects/model_shader_flags.h"
#undef MODEL_SDR_FLAG_MODE_CPP
#undef SDR_FLAG

// ========== Shared shader type table ==========
// Moved from gropenglshader.cpp — single source of truth for both backends.
// clang-format off
static ShaderTypeInfo SHADER_TYPES[] = {
	{ SDR_TYPE_MODEL, "main-v.sdr", "main-f.sdr", "main-g.sdr",
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD, VATTRIB_NORMAL, VATTRIB_TANGENT, VATTRIB_MODELID }, "Model Rendering", false },

	{ SDR_TYPE_EFFECT_PARTICLE, "effect-v.sdr", "effect-f.sdr", "effect-g.sdr",
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD, VATTRIB_RADIUS, VATTRIB_COLOR }, "Particle Effects", false },

	{ SDR_TYPE_EFFECT_DISTORTION, "effect-distort-v.sdr", "effect-distort-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD, VATTRIB_RADIUS, VATTRIB_COLOR }, "Distortion Effects", false },

	{ SDR_TYPE_POST_PROCESS_MAIN, "post-v.sdr", "post-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Post Processing", false },

	{ SDR_TYPE_POST_PROCESS_BLUR, "post-v.sdr", "blur-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Gaussian Blur", false },

	{ SDR_TYPE_POST_PROCESS_BLOOM_COMP, "post-v.sdr", "bloom-comp-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Bloom Compositing", false },

	{ SDR_TYPE_POST_PROCESS_BRIGHTPASS, "post-v.sdr", "brightpass-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Bloom Brightpass", false },

	{ SDR_TYPE_POST_PROCESS_FXAA, "fxaa-v.sdr", "fxaa-f.sdr", nullptr,
		{ VATTRIB_POSITION }, "FXAA", false },

	{ SDR_TYPE_POST_PROCESS_FXAA_PREPASS, "post-v.sdr", "fxaapre-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "FXAA Prepass", false },

	{ SDR_TYPE_POST_PROCESS_LIGHTSHAFTS, "post-v.sdr", "ls-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Lightshafts", false },

	{ SDR_TYPE_POST_PROCESS_TONEMAPPING, "post-v.sdr", "tonemapping-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Tonemapping", false },

	{ SDR_TYPE_DEFERRED_LIGHTING, "deferred-v.sdr", "deferred-f.sdr", nullptr,
		{ VATTRIB_POSITION }, "Deferred Lighting", false },

	{ SDR_TYPE_DEFERRED_CLEAR, "deferred-clear-v.sdr", "deferred-clear-f.sdr", nullptr,
		{ VATTRIB_POSITION }, "Clear Deferred Lighting Buffer", false },

	{ SDR_TYPE_VIDEO_PROCESS, "video-v.sdr", "video-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Video Playback", false },

	{ SDR_TYPE_PASSTHROUGH_RENDER, "passthrough-v.sdr", "passthrough-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD, VATTRIB_COLOR }, "Passthrough", false },

	{ SDR_TYPE_SHIELD_DECAL, "shield-impact-v.sdr",	"shield-impact-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_NORMAL }, "Shield Decals", false },

	{ SDR_TYPE_BATCHED_BITMAP, "batched-v.sdr", "batched-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD, VATTRIB_COLOR }, "Batched bitmaps", false },

	{ SDR_TYPE_DEFAULT_MATERIAL, "default-material-v.sdr", "default-material-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD, VATTRIB_COLOR }, "Default material", false },

	{ SDR_TYPE_NANOVG, "nanovg-v.sdr", "nanovg-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "NanoVG shader", false },

	{ SDR_TYPE_DECAL, "decal-v.sdr", "decal-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_MODEL_MATRIX }, "Decal rendering", false },

	{ SDR_TYPE_SCENE_FOG, "post-v.sdr", "fog-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Scene fogging", false },

	{ SDR_TYPE_VOLUMETRIC_FOG, "post-v.sdr", "volumetric-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Volumetric fogging", false },

	{ SDR_TYPE_ROCKET_UI, "rocketui-v.sdr",	"rocketui-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_COLOR, VATTRIB_TEXCOORD }, "libRocket UI", false },

	{ SDR_TYPE_COPY, "post-v.sdr",	"copy-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Texture copy", false },

	{ SDR_TYPE_COPY_WORLD, "passthrough-v.sdr",	"copy-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Texture copy world space", false },

	{ SDR_TYPE_MSAA_RESOLVE, "post-v.sdr",	"msaa-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "MSAA resolve shader", false },

	{ SDR_TYPE_POST_PROCESS_SMAA_EDGE, "smaa-edge-v.sdr", "smaa-edge-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "SMAA Edge detection", false },

	{ SDR_TYPE_POST_PROCESS_SMAA_BLENDING_WEIGHT, "smaa-blend-v.sdr", "smaa-blend-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "SMAA Blending weight calculation", false },

	{ SDR_TYPE_POST_PROCESS_SMAA_NEIGHBORHOOD_BLENDING, "smaa-neighbour-v.sdr", "smaa-neighbour-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "SMAA Neighborhood Blending", false },

	// Vulkan-only: copies the SMAA neighborhood-blend result back into Scene_ldr
	// (OpenGL does the equivalent copy directly via opengl_shader_set_passthrough).
	{ SDR_TYPE_POST_PROCESS_SMAA_RESOLVE, "post-v.sdr", "smaa-resolve-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "SMAA Resolve", false },

	{ SDR_TYPE_ENVMAP_SPHERE_WARP, "post-v.sdr", "envmap-sphere-warp-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Environment Map Export", false },

	{ SDR_TYPE_IRRADIANCE_MAP_GEN, "post-v.sdr", "irrmap-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Irradiance Map Generation", false },

	{ SDR_TYPE_SHADOW_MAP_GEN, "shadow_map-v.sdr", "shadow_map-f.sdr", "shadow_map-g.sdr",
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD, VATTRIB_NORMAL, VATTRIB_TANGENT, VATTRIB_MODELID }, "Shadow Map Generation", false },

	{ SDR_TYPE_GAMMA_BLIT, "post-v.sdr", "gamma-correct-f.sdr", nullptr,
		{ VATTRIB_POSITION, VATTRIB_TEXCOORD }, "Gamma correct blit", false },
};
// clang-format on

// ========== Shared shader variant table ==========
// Moved from gropenglshader.cpp — single source of truth for both backends.
// MODEL flags come from model_shader_flags.h; others from 2d.h defines.
static ShaderVariantInfo SHADER_VARIANTS[] = {
#define MODEL_SDR_FLAG_MODE_CPP_ARRAY
#include "def_files/data/effects/model_shader_flags.h"
#undef MODEL_SDR_FLAG_MODE_CPP_ARRAY

	{SDR_TYPE_EFFECT_PARTICLE, true, SDR_FLAG_PARTICLE_POINT_GEN, "FLAG_EFFECT_GEOMETRY", {VATTRIB_UVEC}, "Geometry shader point-based particles"},

	{SDR_TYPE_DEFERRED_LIGHTING, false, SDR_FLAG_ENV_MAP, "ENV_MAP", {}, "Render ambient light with env and irrmaps"},

	{SDR_TYPE_DEFERRED_LIGHTING, false, SDR_FLAG_DEFERRED_RT_SHADOWS, "RT_SHADOWS", {}, "Use raytraced (TLAS ray query) shadows instead of cascaded shadow maps"},

	{SDR_TYPE_POST_PROCESS_BLUR, false, SDR_FLAG_BLUR_HORIZONTAL, "PASS_0", {}, "Horizontal blur pass"},

	{SDR_TYPE_POST_PROCESS_BLUR, false, SDR_FLAG_BLUR_VERTICAL, "PASS_1", {}, "Vertical blur pass"},

	{SDR_TYPE_NANOVG, false, SDR_FLAG_NANOVG_EDGE_AA, "EDGE_AA", {}, "NanoVG edge anti-alias"},

	{SDR_TYPE_DECAL, false, SDR_FLAG_DECAL_USE_NORMAL_MAP, "USE_NORMAL_MAP", {}, "Decal use scene normal map"},

	{SDR_TYPE_MSAA_RESOLVE, false, SDR_FLAG_MSAA_SAMPLES_4, "SAMPLES_4", {}, "Sets the MSAA resolve shader to 4 samples"},

	{SDR_TYPE_MSAA_RESOLVE, false, SDR_FLAG_MSAA_SAMPLES_8, "SAMPLES_8", {}, "Sets the MSAA resolve shader to 8 samples"},

	{SDR_TYPE_MSAA_RESOLVE, false, SDR_FLAG_MSAA_SAMPLES_16, "SAMPLES_16", {}, "Sets the MSAA resolve shader to 16 samples"},

	{SDR_TYPE_VOLUMETRIC_FOG, false, SDR_FLAG_VOLUMETRICS_DO_EDGE_SMOOTHING, "DO_EDGE_SMOOTHING", {}, "Perform costly edge smoothing lookups"},

	{SDR_TYPE_VOLUMETRIC_FOG, false, SDR_FLAG_VOLUMETRICS_NOISE, "NOISE", {}, "Add noise to volumetrics"},

	{SDR_TYPE_COPY_WORLD, false, SDR_FLAG_COPY_FROM_ARRAY, "COPY_ARRAY", {}, "Expects to copy from an array texture"},

	{SDR_TYPE_POST_PROCESS_TONEMAPPING, false, SDR_FLAG_TONEMAPPING_LINEAR_OUT, "LINEAR_OUT", {}, "Will make the tonemapper output in linear color space and not in sRGB"},

	{SDR_TYPE_SHADOW_MAP_GEN, true, SDR_FLAG_SHADOW_FALLBACK, "GEOMETRY_FALLBACK", {}, "Use a geometry shader to select the cascade layer instead of writing gl_Layer from the vertex shader"}
};

const ShaderTypeInfo* shader_get_type_info(shader_type type)
{
	for (auto & i : SHADER_TYPES) {
		if (i.type_id == type) {
			return &i;
		}
	}
	return nullptr;
}

const ShaderVariantInfo* shader_get_variant_info(shader_type type, int flag)
{
	for (auto & i : SHADER_VARIANTS) {
		if (i.type_id == type && i.flag == flag) {
			return &i;
		}
	}
	return nullptr;
}

void shader_for_each_active_variant(shader_type type, unsigned int flags, const std::function<void(const ShaderVariantInfo&)>& fn)
{
	for (auto & i : SHADER_VARIANTS) {
		if (i.type_id == type && (flags & i.flag)) {
			fn(i);
		}
	}
}

SCP_string shader_build_variant_defines(shader_type type, unsigned int flags)
{
	SCP_string header;
	shader_for_each_active_variant(type, flags, [&](const ShaderVariantInfo& v) {
		header += "#define ";
		header += v.flag_text;
		header += "\n";
	});
	return header;
}

bool shader_variant_requires_raytracing(shader_type type, unsigned int flags)
{
	switch (type) {
	case SDR_TYPE_MODEL:
		return (flags & MODEL_SDR_FLAG_RT_SHADOWS) != 0;
	case SDR_TYPE_DEFERRED_LIGHTING:
		return (flags & SDR_FLAG_DEFERRED_RT_SHADOWS) != 0;
	default:
		return false;
	}
}

SCP_string shader_get_fxaa_defines(AntiAliasMode aa_mode, bool gather4_alpha)
{
	SCP_string defines;
	defines.reserve(256);

	defines += "#define FXAA_GLSL_120 0\n";
	defines += "#define FXAA_GLSL_130 1\n";

	if (gather4_alpha) {
		defines += "#define FXAA_GATHER4_ALPHA 1\n";
	}

	switch (aa_mode) {
	case AntiAliasMode::None:
		defines += "#define FXAA_QUALITY_PRESET 10\n";
		defines += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/6.0)\n";
		defines += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/12.0)\n";
		defines += "#define FXAA_QUALITY_SUBPIX 0.33\n";
		break;
	case AntiAliasMode::FXAA_Low:
		defines += "#define FXAA_QUALITY_PRESET 12\n";
		defines += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/8.0)\n";
		defines += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/16.0)\n";
		defines += "#define FXAA_QUALITY_SUBPIX 0.33\n";
		break;
	case AntiAliasMode::FXAA_Medium:
		defines += "#define FXAA_QUALITY_PRESET 26\n";
		defines += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/12.0)\n";
		defines += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/24.0)\n";
		defines += "#define FXAA_QUALITY_SUBPIX 0.33\n";
		break;
	case AntiAliasMode::FXAA_High:
		defines += "#define FXAA_QUALITY_PRESET 39\n";
		defines += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/15.0)\n";
		defines += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/32.0)\n";
		defines += "#define FXAA_QUALITY_SUBPIX 0.33\n";
		break;
	default:
		UNREACHABLE("Unhandled FXAA mode!");
	}

	return defines;
}

SCP_string shader_get_smaa_defines(AntiAliasMode aa_mode, bool glsl4)
{
	SCP_string defines;
	defines.reserve(64);

	defines += glsl4 ? "#define SMAA_GLSL_4\n" : "#define SMAA_GLSL_3\n";

	switch (aa_mode) {
	case AntiAliasMode::SMAA_Low:
		defines += "#define SMAA_PRESET_LOW\n";
		break;
	case AntiAliasMode::SMAA_Medium:
		defines += "#define SMAA_PRESET_MEDIUM\n";
		break;
	case AntiAliasMode::SMAA_High:
		defines += "#define SMAA_PRESET_HIGH\n";
		break;
	case AntiAliasMode::SMAA_Ultra:
		defines += "#define SMAA_PRESET_ULTRA\n";
		break;
	default:
		UNREACHABLE("Unhandled SMAA mode!");
	}

	return defines;
}

SCP_string shader_get_shadow_cascade_defines()
{
	SCP_string header;
	header += "#define NUM_SHADOW_CASCADES ";
	header += std::to_string(Num_shadow_cascades + Num_cockpit_shadow_cascades);
	header += "\n";
	return header;
}

SCP_string shader_get_rt_shadow_light_limit_define()
{
	SCP_string header;
	header += "#define MAX_RT_SHADOW_LIGHTS ";
	header += std::to_string(Max_rt_shadow_lights);
	header += "\n";
	return header;
}
