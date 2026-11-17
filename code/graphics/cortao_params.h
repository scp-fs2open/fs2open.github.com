#pragma once

// The uniform data of the CORTAO passes (graphics/ssao.h), shared by the Vulkan and OpenGL backends.
// Each struct is the std140 layout of the genericData block of one CORTAO shader.

#include "graphics/ssao.h"

#include <cstdint>

namespace graphics {

// The genericData block of cortao-prepass-f.sdr
struct cortao_prepass_uniforms {
	uint32_t packedResolution;
	uint32_t randomSeed;
	uint32_t mipLevel;
	float nearPlane;
};

// The genericData block of cortao-main-f.sdr: CORTAOMainPassParameters (CORTAOMainPass.h), then
// the intensity.
struct cortao_main_uniforms {
	uint32_t packedResolution;
	uint32_t randomSeed;
	float normalBias;
	float radius;

	float unprojectParams[4];
	float projectionParams[4];

	float maxApparentCircleRadiusCoefficient;
	float mipBias;
	float inverseMaxSlopeCompensation;
	float backfaceThreshold;

	float backfaceBoost;
	uint32_t padding0[3];

	float intensity;
	float padding1[3];
};
static_assert(sizeof(cortao_main_uniforms) == 96, "cortao_main_uniforms must match the std140 layout of cortao-main-f.sdr");

// The genericData block of cortao-filter-f.sdr: CORTAOFilterPassParameters (CORTAOFilterPass.h)
struct cortao_filter_uniforms {
	uint32_t packedResolution;
	float depthSensitivity;
	float normalSensitivity;
	uint32_t padding0;
};

// The prepass writes at most this many mip levels. This is also the CORTAO maximum.
constexpr uint32_t CORTAO_MAX_MIP_LEVELS = 8;

// The number of packed data mip levels for a target of this size: one for each halving of the
// larger side, at most CORTAO_MAX_MIP_LEVELS.
uint32_t cortao_mip_levels(uint32_t width, uint32_t height);

// The random seed of this frame. A constant seed gives a constant noise pattern.
// See ssao_animate_noise().
uint32_t cortao_random_seed(bool animate, uint64_t frameNumber);

// The shader flags of the main pass: the sample count of ssao_sample_count() and the bent normals
unsigned int cortao_main_shader_flags();

// The shader flags of the filter pass for a filter mode that is not Off
unsigned int cortao_filter_shader_flags(SsaoFilterMode mode);

// The SSAO shader flags of the deferred light pass, when the CORTAO passes wrote an occlusion target
unsigned int cortao_deferred_shader_flags();

void cortao_fill_prepass_uniforms(cortao_prepass_uniforms& out, uint32_t width, uint32_t height, uint32_t seed,
	uint32_t mipLevel);

/**
 * @brief Fill the main pass parameters from the current projection matrix and lighting profile
 *
 * This does the work of CORTAO::populateMainPassParameters() in CORTAOParameters.h. We do not compile
 * CORTAOParameters.h, because it uses std::sqrtf, which libstdc++ does not have.
 *
 * @param flipY True when NDC +Y is texel row 0 of the G-buffer. This is true for the Vulkan G-buffer
 *              pass, which uses a negative viewport height, and false for OpenGL.
 */
void cortao_fill_main_uniforms(cortao_main_uniforms& out, uint32_t width, uint32_t height, uint32_t seed, bool flipY);

void cortao_fill_filter_uniforms(cortao_filter_uniforms& out, uint32_t width, uint32_t height);

} // namespace graphics
