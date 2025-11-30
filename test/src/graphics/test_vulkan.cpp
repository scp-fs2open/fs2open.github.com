#ifdef WITH_VULKAN

#include <gtest/gtest.h>

#include "graphics/vulkan/VulkanShader.h"

#define MODEL_SDR_FLAG_MODE_CPP
#include "def_files/data/effects/model_shader_flags.h"

using namespace graphics::vulkan;

// Cache keys must be deterministic for identical inputs and vary on stage/type/flags.
TEST(VulkanShaderCacheTest, CacheKeyDeterministicAndUnique) {
	VulkanShaderCache cache;

	const SCP_string sourceHash = "abcd1234";
	const uint32_t flagsA = 0u;
	const uint32_t flagsB = 0xFFu;

	auto key_vert = cache.computeCacheKey(sourceHash, SDR_TYPE_MODEL, flagsA, ShaderStage::Vertex);
	auto key_vert_again = cache.computeCacheKey(sourceHash, SDR_TYPE_MODEL, flagsA, ShaderStage::Vertex);
	auto key_frag = cache.computeCacheKey(sourceHash, SDR_TYPE_MODEL, flagsA, ShaderStage::Fragment);
	auto key_geo = cache.computeCacheKey(sourceHash, SDR_TYPE_MODEL, flagsA, ShaderStage::Geometry);
	auto key_other_type = cache.computeCacheKey(sourceHash, SDR_TYPE_DEFAULT_MATERIAL, flagsA, ShaderStage::Vertex);
	auto key_other_flags = cache.computeCacheKey(sourceHash, SDR_TYPE_MODEL, flagsB, ShaderStage::Vertex);

	// Deterministic
	EXPECT_EQ(key_vert, key_vert_again);

	// Stage/type/flags must perturb the key
	EXPECT_NE(key_vert, key_frag);
	EXPECT_NE(key_vert, key_geo);
	EXPECT_NE(key_vert, key_other_type);
	EXPECT_NE(key_vert, key_other_flags);

	// Stage tag is present as a single-letter component
	EXPECT_NE(key_vert.find("_v_"), SCP_string::npos);
	EXPECT_NE(key_frag.find("_f_"), SCP_string::npos);
	EXPECT_NE(key_geo.find("_g_"), SCP_string::npos);
}

// Geometry requirements should match the flag rules in VulkanShaderManager.
TEST(VulkanShaderManagerTest, RequiresGeometryShaderMatchesFlags) {
	VulkanShaderManager manager{vk::Device()};

	// Model shadows require geometry shader
	EXPECT_TRUE(manager.requiresGeometryShader(SDR_TYPE_MODEL, MODEL_SDR_FLAG_SHADOW_MAP));
	EXPECT_FALSE(manager.requiresGeometryShader(SDR_TYPE_MODEL, 0));

	// Particle point generation requires geometry shader
	EXPECT_TRUE(manager.requiresGeometryShader(SDR_TYPE_EFFECT_PARTICLE, SDR_FLAG_PARTICLE_POINT_GEN));
	EXPECT_FALSE(manager.requiresGeometryShader(SDR_TYPE_EFFECT_PARTICLE, 0));

	// Types without geometry shaders should not request one
	EXPECT_FALSE(manager.requiresGeometryShader(SDR_TYPE_POST_PROCESS_MAIN, 0));
}

#endif // WITH_VULKAN
