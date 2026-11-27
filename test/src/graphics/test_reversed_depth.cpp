#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "graphics/shader_preprocess.h"
#include "graphics/shadows.h"

#include <gtest/gtest.h>

#include <cmath>

// not in a header; defined in shadows.cpp
void shadows_construct_light_proj(light_frustum_info* shadow_data);

namespace {
float shadow_depth(const light_frustum_info& info, float z)
{
	return info.proj_matrix.a1d[10] * z + info.proj_matrix.a1d[14];
}
} // namespace

TEST(ReversedDepth, ShadowProjectionFlipsTheDepthRange)
{
	const auto saved_mode = gr_screen.mode;
	gr_screen.mode = GraphicsAPI::Vulkan;

	light_frustum_info info{};
	info.min.xyz = {-50.0f, -50.0f, 10.0f};
	info.max.xyz = {50.0f, 50.0f, 500.0f};

	Gr_reversed_depth_active = false;
	shadows_construct_light_proj(&info);
	const light_frustum_info normal = info;

	Gr_reversed_depth_active = true;
	shadows_construct_light_proj(&info);
	Gr_reversed_depth_active = false;
	gr_screen.mode = saved_mode;

	for (float z : {-10.0f, -100.0f, -255.0f, -500.0f}) {
		EXPECT_NEAR(shadow_depth(info, z), 1.0f - shadow_depth(normal, z), 1e-5f) << "z " << z;
	}
	EXPECT_NEAR(shadow_depth(normal, -10.0f), 0.0f, 1e-5f);
	EXPECT_NEAR(shadow_depth(info, -10.0f), 1.0f, 1e-5f);
}

TEST(ReversedDepth, OpenGLShadowProjectionUsesTheReversedRange)
{
	const auto saved_mode = gr_screen.mode;
	gr_screen.mode = GraphicsAPI::OpenGL;

	light_frustum_info info{};
	info.min.xyz = {-50.0f, -50.0f, 10.0f};
	info.max.xyz = {50.0f, 50.0f, 500.0f};
	Gr_reversed_depth_active = true;
	shadows_construct_light_proj(&info);
	Gr_reversed_depth_active = false;
	gr_screen.mode = saved_mode;

	EXPECT_NEAR(shadow_depth(info, -10.0f), 1.0f, 1e-5f);
	EXPECT_NEAR(shadow_depth(info, -500.0f), 0.0f, 1e-5f);
}

TEST(ReversedDepth, FarDepthIsTheClearValue)
{
	Gr_reversed_depth_active = false;
	EXPECT_EQ(gr_far_depth(), 1.0f);
	Gr_reversed_depth_active = true;
	EXPECT_EQ(gr_far_depth(), 0.0f);
	Gr_reversed_depth_active = false;
}

namespace {
// window depth of a point at view distance d for the current projection; OpenGL without reversed depth maps [-1, 1]
float window_depth(float d)
{
	const float z = gr_projection_matrix.a1d[10] * -d + gr_projection_matrix.a1d[14];
	const float ndc = z / d;
	const bool zero_to_one = gr_screen.mode == GraphicsAPI::Vulkan || Gr_reversed_depth_active;
	return zero_to_one ? ndc : ndc * 0.5f + 0.5f;
}

// sets up a perspective projection with near 1 m and far 1000 km, then restores the engine state
class ReversedDepthProjection : public ::testing::TestWithParam<GraphicsAPI> {
  protected:
	void SetUp() override
	{
		saved_mode = gr_screen.mode;
		saved_viewport = gr_screen.gf_set_viewport;
		saved_matrix = gr_projection_matrix;
		gr_screen.mode = GetParam();
		gr_screen.gf_set_viewport = [](int, int, int, int) {};
	}
	void TearDown() override
	{
		Gr_reversed_depth_active = false;
		gr_screen.mode = saved_mode;
		gr_screen.gf_set_viewport = saved_viewport;
		gr_projection_matrix = saved_matrix;
	}

	GraphicsAPI saved_mode = GraphicsAPI::OpenGL;
	decltype(gr_screen.gf_set_viewport) saved_viewport;
	matrix4 saved_matrix{};
};
} // namespace

TEST_P(ReversedDepthProjection, NearIsOneAndFarIsZero)
{
	Gr_reversed_depth_active = true;
	gr_set_proj_matrix(0.75f, 1.6f, 1.0f, 1e6f);
	EXPECT_NEAR(window_depth(1.0f), 1.0f, 1e-5f);
	EXPECT_NEAR(window_depth(1e6f), 0.0f, 1e-5f);
	float previous = 2.0f;
	for (float d : {1.0f, 10.0f, 1e3f, 1e5f, 1e6f}) {
		const float depth = window_depth(d);
		EXPECT_LT(depth, previous) << "distance " << d;
		// the shaders' linear distance for reversed depth gives the distance back
		EXPECT_NEAR(1.0f * 1e6f / (1.0f + depth * (1e6f - 1.0f)), d, d * 1e-3f) << "distance " << d;
		previous = depth;
	}
}

TEST_P(ReversedDepthProjection, SeparatesSurfacesAtLongRange)
{
	// two surfaces half a meter apart, 20 km away
	gr_set_proj_matrix(0.75f, 1.6f, 1.0f, 1e6f);
	const auto normal24 = [](float depth) { return std::lround(static_cast<double>(depth) * ((1 << 24) - 1)); };
	EXPECT_EQ(normal24(window_depth(20000.0f)), normal24(window_depth(20000.5f)));

	Gr_reversed_depth_active = true;
	gr_set_proj_matrix(0.75f, 1.6f, 1.0f, 1e6f);
	EXPECT_NE(window_depth(20000.0f), window_depth(20000.5f));
}

TEST_P(ReversedDepthProjection, OrthographicNearIsOneAndFarIsZero)
{
	// gr_end_proj_matrix() sets the 2D orthographic projection with near -1 and far 1
	Gr_reversed_depth_active = true;
	gr_end_proj_matrix();
	const auto ortho_depth = [](float z) { return gr_projection_matrix.a1d[10] * z + gr_projection_matrix.a1d[14]; };
	EXPECT_NEAR(ortho_depth(1.0f), 1.0f, 1e-5f);
	EXPECT_NEAR(ortho_depth(-1.0f), 0.0f, 1e-5f);
}

INSTANTIATE_TEST_SUITE_P(BothRenderers,
	ReversedDepthProjection,
	::testing::Values(GraphicsAPI::OpenGL, GraphicsAPI::Vulkan));

TEST(ReversedDepth, ShadersGetTheDefineOnlyWhenActive)
{
	EXPECT_EQ(shader_preprocess_defines("test.sdr", "").find("#define REVERSED_Z"), SCP_string::npos);
	Gr_reversed_depth_active = true;
	const auto defines = shader_preprocess_defines("test.sdr", "");
	Gr_reversed_depth_active = false;
	EXPECT_NE(defines.find("#define REVERSED_Z"), SCP_string::npos);
}
