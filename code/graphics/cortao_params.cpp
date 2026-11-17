#include "graphics/cortao_params.h"

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "graphics/ssao.h"
#include "lighting/lighting_profiles.h"

#include <algorithm>
#include <cmath>

namespace graphics {

namespace {

// The CORTAO configuration values that the engine does not change. These are the defaults that
// CORTAOParameters.h recommends.
constexpr float CORTAO_MIP_BIAS = -4.0f;
constexpr float CORTAO_MAX_SLOPE_COMPENSATION = 4.0f;
constexpr float CORTAO_BACKFACE_THRESHOLD = 0.5f;
constexpr float CORTAO_BACKFACE_BOOST = 0.0f;
constexpr float CORTAO_FILTER_DEPTH_SENSITIVITY = 512.0f;
constexpr float CORTAO_FILTER_NORMAL_SENSITIVITY = 128.0f;

constexpr uint32_t CORTAO_CONSTANT_SEED = 0x6a09e667u;

uint32_t pack_resolution(uint32_t width, uint32_t height)
{
	return width | (height << 16u);
}

} // namespace

uint32_t cortao_mip_levels(uint32_t width, uint32_t height)
{
	uint32_t levels = 1;
	while (levels < CORTAO_MAX_MIP_LEVELS && (std::max(width, height) >> levels) > 0) {
		++levels;
	}
	return levels;
}

uint32_t cortao_random_seed(bool animate, uint64_t frameNumber)
{
	if (!animate) {
		return CORTAO_CONSTANT_SEED;
	}
	// A 32-bit integer hash (lowbias32, Chris Wellons)
	auto x = static_cast<uint32_t>(frameNumber);
	x ^= x >> 16;
	x *= 0x7feb352du;
	x ^= x >> 15;
	x *= 0x846ca68bu;
	x ^= x >> 16;
	return x;
}

unsigned int cortao_main_shader_flags()
{
	switch (ssao_sample_count()) {
	case 4:
		return SDR_FLAG_CORTAO_SAMPLES_4;
	case 8:
		return SDR_FLAG_CORTAO_SAMPLES_8;
	default:
		return 0; // 16 samples: the shader default
	}
}

void cortao_fill_prepass_uniforms(cortao_prepass_uniforms& out, uint32_t width, uint32_t height, uint32_t seed,
	uint32_t mipLevel)
{
	out.packedResolution = pack_resolution(width, height);
	out.randomSeed = seed;
	out.mipLevel = mipLevel;
	out.nearPlane = gr_near_plane;
}

void cortao_fill_main_uniforms(cortao_main_uniforms& out, uint32_t width, uint32_t height, uint32_t seed, bool flipY)
{
	// gr_projection_matrix is column-major. The main pass uses only the X and Y terms, so the
	// different near planes of the scene and cockpit passes do not change the result. The signs
	// stay in the projection terms. The mip bias and the radius limit use the magnitudes, because
	// a projection with a negative Y scale (a Y-flip) would make them NaN.
	const float projX = gr_projection_matrix.a1d[0];
	const float projY = gr_projection_matrix.a1d[5];
	const float projOffsetX = gr_projection_matrix.a1d[8];
	const float projOffsetY = gr_projection_matrix.a1d[9];

	// The terms of the inverse projection matrix that CORTAO uses
	const float invX = 1.0f / projX;
	const float invY = 1.0f / projY;
	const float invOffsetX = projOffsetX / projX;
	const float invOffsetY = projOffsetY / projY;

	memset(&out, 0, sizeof(out));
	out.packedResolution = pack_resolution(width, height);
	out.randomSeed = seed;
	out.normalBias = std::fabs(invX) / static_cast<float>(width) * std::sqrt(2.0f);
	out.radius = lighting_profiles::current_ssao_max_radius();

	if (flipY) {
		out.unprojectParams[0] = +2.0f * invX;
		out.unprojectParams[1] = -2.0f * invY;
		out.unprojectParams[2] = -invX + invOffsetX;
		out.unprojectParams[3] = +invY + invOffsetY;

		out.projectionParams[0] = +0.5f * projX;
		out.projectionParams[1] = -0.5f * projY;
		out.projectionParams[2] = -0.5f + 0.5f * projOffsetX;
		out.projectionParams[3] = -0.5f - 0.5f * projOffsetY;
	} else {
		out.unprojectParams[0] = +2.0f * invX;
		out.unprojectParams[1] = +2.0f * invY;
		out.unprojectParams[2] = -invX + invOffsetX;
		out.unprojectParams[3] = -invY + invOffsetY;

		out.projectionParams[0] = +0.5f * projX;
		out.projectionParams[1] = +0.5f * projY;
		out.projectionParams[2] = -0.5f + 0.5f * projOffsetX;
		out.projectionParams[3] = -0.5f + 0.5f * projOffsetY;
	}

	const float maxApparentCircleRadius = std::fabs(invY) * lighting_profiles::current_ssao_screen_radius() * 2.0f;
	out.maxApparentCircleRadiusCoefficient =
		maxApparentCircleRadius / std::sqrt(maxApparentCircleRadius * maxApparentCircleRadius + 1.0f);
	out.mipBias = std::log2(std::fabs(projY) * static_cast<float>(height) * 0.5f) + CORTAO_MIP_BIAS;
	out.inverseMaxSlopeCompensation = 1.0f / CORTAO_MAX_SLOPE_COMPENSATION;
	out.backfaceThreshold = CORTAO_BACKFACE_THRESHOLD;
	out.backfaceBoost = CORTAO_BACKFACE_BOOST;

	out.intensity = lighting_profiles::current_ssao_intensity();
}

void cortao_fill_filter_uniforms(cortao_filter_uniforms& out, uint32_t width, uint32_t height)
{
	out.packedResolution = pack_resolution(width, height);
	out.depthSensitivity = CORTAO_FILTER_DEPTH_SENSITIVITY;
	out.normalSensitivity = CORTAO_FILTER_NORMAL_SENSITIVITY;
	out.padding0 = 0;
}

} // namespace graphics
