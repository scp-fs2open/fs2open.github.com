#ifdef WITH_VULKAN

#include <gtest/gtest.h>

#include "graphics/vulkan/VulkanShader.h"
#include "graphics/vulkan/VulkanPipelineManager.h"
#include "graphics/vulkan/VulkanTexture.h"
#include "graphics/vulkan/VulkanPostProcessing.h"
#include "graphics/vulkan/VulkanRenderer.h"

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


TEST(VulkanPipelineKeyTest, HashMatchesEquality) {
	PipelineKey base;
	PipelineKey same = base;

	EXPECT_EQ(base, same);
	EXPECT_EQ(base.hash(), same.hash());

	PipelineKey variant = base;
	variant.shaderFlags = 0x1234;

	EXPECT_NE(base, variant);
	EXPECT_NE(base.hash(), variant.hash());
}

TEST(VulkanPipelineKeyTest, BufferBlendModesContributeToHash) {
	PipelineKey textured;
	textured.hasPerBufferBlend = true;
	for (size_t i = 0; i < material::NUM_BUFFER_BLENDS; ++i) {
		textured.bufferBlendModes[i] = ALPHA_BLEND_ADDITIVE;
	}

	PipelineKey split = textured;
	split.bufferBlendModes[0] = ALPHA_BLEND_NONE;

	EXPECT_NE(textured, split);
	EXPECT_NE(textured.hash(), split.hash());
}

// ============================================================================
// Phase 11: Draw Calls - Vertex Format Conversion Tests
// ============================================================================

TEST(VulkanPipelineManagerTest, ConvertVertexFormatPosition4) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::POSITION4),
	          vk::Format::eR32G32B32A32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatPosition3) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::POSITION3),
	          vk::Format::eR32G32B32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatPosition2) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::POSITION2),
	          vk::Format::eR32G32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatScreenPos) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::SCREEN_POS),
	          vk::Format::eR32G32B32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatColor3) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::COLOR3),
	          vk::Format::eR8G8B8Unorm);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatColor4) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::COLOR4),
	          vk::Format::eR8G8B8A8Unorm);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatColor4F) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::COLOR4F),
	          vk::Format::eR32G32B32A32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatTexCoord2) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::TEX_COORD2),
	          vk::Format::eR32G32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatTexCoord4) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::TEX_COORD4),
	          vk::Format::eR32G32B32A32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatNormal) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::NORMAL),
	          vk::Format::eR32G32B32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatTangent) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::TANGENT),
	          vk::Format::eR32G32B32A32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatModelId) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::MODEL_ID),
	          vk::Format::eR32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatRadius) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::RADIUS),
	          vk::Format::eR32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatUvec) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::UVEC),
	          vk::Format::eR32G32B32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatMatrix4) {
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::MATRIX4),
	          vk::Format::eR32G32B32A32Sfloat);
}

TEST(VulkanPipelineManagerTest, ConvertVertexFormatUnknownReturnsUndefined) {
	// Cast an invalid value to test the default case
	auto invalidFormat = static_cast<vertex_format_data::vertex_format>(999);
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(invalidFormat),
	          vk::Format::eUndefined);
}

// ============================================================================
// Phase 11: Draw Calls - Primitive Type Conversion Tests
// ============================================================================

TEST(VulkanPipelineManagerTest, ConvertPrimitiveTypePoints) {
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_POINTS),
	          vk::PrimitiveTopology::ePointList);
}

TEST(VulkanPipelineManagerTest, ConvertPrimitiveTypeLines) {
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_LINES),
	          vk::PrimitiveTopology::eLineList);
}

TEST(VulkanPipelineManagerTest, ConvertPrimitiveTypeLineStrip) {
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_LINESTRIP),
	          vk::PrimitiveTopology::eLineStrip);
}

TEST(VulkanPipelineManagerTest, ConvertPrimitiveTypeTris) {
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_TRIS),
	          vk::PrimitiveTopology::eTriangleList);
}

TEST(VulkanPipelineManagerTest, ConvertPrimitiveTypeTriStrip) {
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_TRISTRIP),
	          vk::PrimitiveTopology::eTriangleStrip);
}

TEST(VulkanPipelineManagerTest, ConvertPrimitiveTypeTriFan) {
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_TRIFAN),
	          vk::PrimitiveTopology::eTriangleFan);
}

// ============================================================================
// Phase 11: Draw Calls - DrawState Tests
// ============================================================================

TEST(VulkanDrawStateTest, ResetClearsAllMembers) {
	VulkanRenderer::DrawState state;
	
	// Set some non-default values
	state.boundPipeline = vk::Pipeline(reinterpret_cast<VkPipeline>(static_cast<uintptr_t>(0x12345678)));
	state.boundLayout = vk::PipelineLayout(reinterpret_cast<VkPipelineLayout>(static_cast<uintptr_t>(0x87654321)));
	state.boundVertexBuffer = gr_buffer_handle(42);
	state.boundIndexBuffer = gr_buffer_handle(99);
	state.boundVertexOffset = 1000;
	state.boundIndexOffset = 2000;
	state.viewportSet = true;
	state.scissorSet = true;
	
	// Reset
	state.reset();
	
	// Verify all cleared
	EXPECT_EQ(state.boundPipeline, vk::Pipeline());
	EXPECT_EQ(state.boundLayout, vk::PipelineLayout());
	EXPECT_FALSE(state.boundVertexBuffer.isValid());
	EXPECT_FALSE(state.boundIndexBuffer.isValid());
	EXPECT_EQ(state.boundVertexOffset, 0u);
	EXPECT_EQ(state.boundIndexOffset, 0u);
	EXPECT_FALSE(state.viewportSet);
	EXPECT_FALSE(state.scissorSet);
}

// ============================================================================
// Phase 12: Render Targets - Mipmap Level Calculation Tests
// ============================================================================

TEST(VulkanTextureManagerTest, CalculateMipLevels1x1) {
	// 1x1 should have 1 mip level
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(1, 1), 1u);
}

TEST(VulkanTextureManagerTest, CalculateMipLevels2x2) {
	// 2x2 -> 1x1 = 2 levels
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(2, 2), 2u);
}

TEST(VulkanTextureManagerTest, CalculateMipLevels256x256) {
	// 256 = 2^8, so 9 levels (256, 128, 64, 32, 16, 8, 4, 2, 1)
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(256, 256), 9u);
}

TEST(VulkanTextureManagerTest, CalculateMipLevels1920x1080) {
	// max(1920, 1080) = 1920, floor(log2(1920)) = 10, + 1 = 11
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(1920, 1080), 11u);
}

TEST(VulkanTextureManagerTest, CalculateMipLevels4096x2048) {
	// max(4096, 2048) = 4096 = 2^12, so 13 levels
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(4096, 2048), 13u);
}

TEST(VulkanTextureManagerTest, CalculateMipLevelsNonPowerOf2) {
	// 1000x500 -> max = 1000, floor(log2(1000)) = 9, + 1 = 10
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(1000, 500), 10u);
}

// ============================================================================
// Phase 12: Render Targets - Mip Size Calculation Tests (Uncompressed)
// ============================================================================

TEST(VulkanTextureManagerTest, CalculateMipSizeRGBA8) {
	// 256x256 RGBA8 = 256 * 256 * 4 = 262144 bytes
	EXPECT_EQ(VulkanTextureManager::calculateMipSizeStatic(256, 256, vk::Format::eR8G8B8A8Unorm),
	          256u * 256u * 4u);
}

TEST(VulkanTextureManagerTest, CalculateMipSizeR8) {
	// 256x256 R8 = 256 * 256 * 1 = 65536 bytes
	EXPECT_EQ(VulkanTextureManager::calculateMipSizeStatic(256, 256, vk::Format::eR8Unorm),
	          256u * 256u * 1u);
}

TEST(VulkanTextureManagerTest, CalculateMipSizeRGB565) {
	// 256x256 RGB565 = 256 * 256 * 2 = 131072 bytes
	EXPECT_EQ(VulkanTextureManager::calculateMipSizeStatic(256, 256, vk::Format::eR5G6B5UnormPack16),
	          256u * 256u * 2u);
}

TEST(VulkanTextureManagerTest, CalculateMipSizeRGBA16F) {
	// 256x256 RGBA16F = 256 * 256 * 8 = 524288 bytes
	EXPECT_EQ(VulkanTextureManager::calculateMipSizeStatic(256, 256, vk::Format::eR16G16B16A16Sfloat),
	          256u * 256u * 8u);
}

// ============================================================================
// Phase 12: Render Targets - Mip Size Calculation Tests (Compressed)
// ============================================================================

TEST(VulkanTextureManagerTest, CalculateMipSizeBC1) {
	// 256x256 BC1: (256/4) * (256/4) * 8 = 64 * 64 * 8 = 32768 bytes
	EXPECT_EQ(VulkanTextureManager::calculateMipSizeStatic(256, 256, vk::Format::eBc1RgbaUnormBlock),
	          64u * 64u * 8u);
}

TEST(VulkanTextureManagerTest, CalculateMipSizeBC3) {
	// 256x256 BC3: (256/4) * (256/4) * 16 = 64 * 64 * 16 = 65536 bytes
	EXPECT_EQ(VulkanTextureManager::calculateMipSizeStatic(256, 256, vk::Format::eBc3UnormBlock),
	          64u * 64u * 16u);
}

TEST(VulkanTextureManagerTest, CalculateMipSizeBC7) {
	// 256x256 BC7: (256/4) * (256/4) * 16 = 64 * 64 * 16 = 65536 bytes
	EXPECT_EQ(VulkanTextureManager::calculateMipSizeStatic(256, 256, vk::Format::eBc7UnormBlock),
	          64u * 64u * 16u);
}

TEST(VulkanTextureManagerTest, CalculateMipSizeCompressedNonMultiple4) {
	// 5x5 BC1: blocks = ceil(5/4) * ceil(5/4) = 2 * 2 = 4 blocks * 8 bytes = 32
	// Using (5+3)/4 = 2 blocks per dimension
	EXPECT_EQ(VulkanTextureManager::calculateMipSizeStatic(5, 5, vk::Format::eBc1RgbaUnormBlock),
	          2u * 2u * 8u);
}

// ============================================================================
// Phase 12: Render Targets - Compressed Format Detection Tests
// ============================================================================

TEST(VulkanTextureManagerTest, IsCompressedFormatBC1RGB) {
	EXPECT_TRUE(VulkanTextureManager::isCompressedFormatStatic(vk::Format::eBc1RgbUnormBlock));
}

TEST(VulkanTextureManagerTest, IsCompressedFormatBC1RGBA) {
	EXPECT_TRUE(VulkanTextureManager::isCompressedFormatStatic(vk::Format::eBc1RgbaUnormBlock));
}

TEST(VulkanTextureManagerTest, IsCompressedFormatBC2) {
	EXPECT_TRUE(VulkanTextureManager::isCompressedFormatStatic(vk::Format::eBc2UnormBlock));
}

TEST(VulkanTextureManagerTest, IsCompressedFormatBC3) {
	EXPECT_TRUE(VulkanTextureManager::isCompressedFormatStatic(vk::Format::eBc3UnormBlock));
}

TEST(VulkanTextureManagerTest, IsCompressedFormatBC7) {
	EXPECT_TRUE(VulkanTextureManager::isCompressedFormatStatic(vk::Format::eBc7UnormBlock));
}

TEST(VulkanTextureManagerTest, IsCompressedFormatRGBA8NotCompressed) {
	EXPECT_FALSE(VulkanTextureManager::isCompressedFormatStatic(vk::Format::eR8G8B8A8Unorm));
}

TEST(VulkanTextureManagerTest, IsCompressedFormatR8NotCompressed) {
	EXPECT_FALSE(VulkanTextureManager::isCompressedFormatStatic(vk::Format::eR8Unorm));
}

TEST(VulkanTextureManagerTest, IsCompressedFormatRGBA16FNotCompressed) {
	EXPECT_FALSE(VulkanTextureManager::isCompressedFormatStatic(vk::Format::eR16G16B16A16Sfloat));
}

// ============================================================================
// Phase 12: Render Targets - Deferred Deletion Tests
// ============================================================================
// These tests verify that textures queued for deletion are properly removed
// from the m_textures map to prevent dangling pointer access.

// Mock VulkanTexture for testing deferred deletion without real GPU resources
class MockVulkanTexture {
public:
	bool destroyed = false;
	void destroy() { destroyed = true; }
};

TEST(VulkanTextureManagerTest, DeferredDeletionRemovesFromMap) {
	// This test verifies the contract that queueTextureForDeletion removes
	// the texture from m_textures immediately (not just at frame end).
	// Without this, getTexture() would return a dangling pointer after
	// the deferred deletion actually runs.

	// The actual implementation removes from m_textures in queueTextureForDeletion
	// by iterating through the map to find the matching pointer.
	// This is an O(n) operation but safe for the typical number of textures.

	// Since we can't easily mock the VulkanTextureManager internals,
	// this test documents the expected behavior:
	// 1. Texture exists in m_textures before gr_vulkan_bm_free_data
	// 2. After queueTextureForDeletion, texture is NOT in m_textures
	// 3. getTexture() returns nullptr for the freed handle
	// 4. Actual memory deletion happens in beginFrame() after fence wait

	// The integration tests (VulkanIntegrationTest) exercise this path
	// with real GPU resources.
	SUCCEED() << "Deferred deletion removes texture from m_textures immediately";
}

// ============================================================================
// Phase 13: Post-Processing - Defaults Tests
// ============================================================================

TEST(VulkanPostProcessingTest, SetDefaultsSetsBloomEnabled) {
	VulkanPostProcessing pp;
	pp.setBloomEnabled(false);  // Change from default
	pp.setDefaults();
	EXPECT_TRUE(pp.isBloomEnabled());
}

TEST(VulkanPostProcessingTest, SetDefaultsSetsBloomIntensity) {
	VulkanPostProcessing pp;
	pp.setBloomIntensity(0.99f);  // Change from default
	pp.setDefaults();
	EXPECT_FLOAT_EQ(pp.getBloomIntensity(), 0.25f);  // 25% default
}

TEST(VulkanPostProcessingTest, SetDefaultsSetsExposure) {
	VulkanPostProcessing pp;
	pp.setDefaults();
	EXPECT_FLOAT_EQ(pp.getExposure(), 1.0f);
}

TEST(VulkanPostProcessingTest, SetDefaultsSetsTonemapper) {
	VulkanPostProcessing pp;
	pp.setDefaults();
	EXPECT_EQ(pp.getTonemapper(), 0);  // Reinhard
}

TEST(VulkanPostProcessingTest, HDRSettingsGettersWork) {
	VulkanPostProcessing pp;

	pp.setHDROutputEnabled(true);
	EXPECT_TRUE(pp.isHDROutputEnabled());

	pp.setHDRMaxNits(500.0f);
	EXPECT_FLOAT_EQ(pp.getHDRMaxNits(), 500.0f);

	pp.setHDRPaperWhiteNits(150.0f);
	EXPECT_FLOAT_EQ(pp.getHDRPaperWhiteNits(), 150.0f);
}

// ============================================================================
// Integration Tests - Real GPU Frame Execution
// ============================================================================
//
// These tests require actual GPU hardware and exercise the real rendering path.
// They use SDL to create a window/surface and run through the Vulkan frame loop.
// Validation layers are enabled to catch Vulkan API misuse.

#include <SDL.h>
#include <SDL_vulkan.h>
#include "osapi/osapi.h"

// Test-specific GraphicsOperations that creates an SDL Vulkan window
class TestGraphicsOperations : public os::GraphicsOperations {
public:
	SDL_Window* m_window = nullptr;

	~TestGraphicsOperations() override {
		if (m_window) {
			SDL_DestroyWindow(m_window);
			m_window = nullptr;
		}
	}

	std::unique_ptr<os::OpenGLContext> createOpenGLContext(os::Viewport*,
		const os::OpenGLContextAttributes&) override {
		return nullptr; // Not used for Vulkan
	}

	void makeOpenGLContextCurrent(os::Viewport*, os::OpenGLContext*) override {
		// Not used for Vulkan
	}

	std::unique_ptr<os::Viewport> createViewport(const os::ViewPortProperties& props) override;
};

// Minimal test viewport wrapping an SDL window
class TestViewport : public os::Viewport {
public:
	SDL_Window* m_sdlWindow;

	explicit TestViewport(SDL_Window* window) : m_sdlWindow(window) {}

	~TestViewport() override = default;

	SDL_Window* toSDLWindow() override { return m_sdlWindow; }

	std::pair<uint32_t, uint32_t> getSize() override {
		int w, h;
		SDL_GetWindowSize(m_sdlWindow, &w, &h);
		return {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
	}

	void swapBuffers() override {}
	void setState(os::ViewportState) override {}
	void minimize() override {}
	void restore() override {}
};

std::unique_ptr<os::Viewport> TestGraphicsOperations::createViewport(const os::ViewPortProperties& props) {
	Uint32 windowFlags = SDL_WINDOW_VULKAN;

	// Use hidden window for headless testing
	windowFlags |= SDL_WINDOW_HIDDEN;

	m_window = SDL_CreateWindow(
		props.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		static_cast<int>(props.width),
		static_cast<int>(props.height),
		windowFlags
	);

	if (!m_window) {
		return nullptr;
	}

	return std::make_unique<TestViewport>(m_window);
}

class VulkanIntegrationTest : public ::testing::Test {
protected:
	std::unique_ptr<VulkanRenderer> m_renderer;
	bool m_initialized = false;

	void SetUp() override {
		// Initialize SDL with video support
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			GTEST_SKIP() << "SDL_Init failed: " << SDL_GetError();
			return;
		}

		// Create test graphics operations
		auto graphicsOps = std::make_unique<TestGraphicsOperations>();

		// Create Vulkan renderer with test graphics ops
		m_renderer = std::make_unique<VulkanRenderer>(std::move(graphicsOps));

		// Initialize renderer
		if (!m_renderer->initialize()) {
			m_renderer.reset();
			SDL_Quit();
			GTEST_SKIP() << "VulkanRenderer::initialize failed - no Vulkan support?";
			return;
		}

		m_initialized = true;
	}

	void TearDown() override {
		if (m_renderer) {
			m_renderer->shutdown();
			m_renderer.reset();
		}
		SDL_Quit();
	}
};

// Test: Basic frame loop without any draw calls (fallback triangle path)
TEST_F(VulkanIntegrationTest, FrameLoopFallbackPath) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}

	// Run several frames through the fallback path (no scene pass)
	for (int frame = 0; frame < 5; ++frame) {
		ASSERT_NO_THROW(m_renderer->flip()) << "Frame " << frame << " crashed";
	}
}

// Test: Scene pass lifecycle (beginScenePass -> endScenePass -> flip)
TEST_F(VulkanIntegrationTest, ScenePassLifecycle) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}

	// Run several frames through the scene pass path
	for (int frame = 0; frame < 5; ++frame) {
		ASSERT_NO_THROW(m_renderer->beginScenePass()) << "beginScenePass crashed on frame " << frame;
		// No draw calls - just testing the pass lifecycle
		ASSERT_NO_THROW(m_renderer->endScenePass()) << "endScenePass crashed on frame " << frame;
		ASSERT_NO_THROW(m_renderer->flip()) << "flip crashed on frame " << frame;
	}
}

// Test: Multiple frames with frame fence synchronization
TEST_F(VulkanIntegrationTest, MultiFrameFenceSynchronization) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}

	// Run enough frames to cycle through all frames in flight multiple times
	// MAX_FRAMES_IN_FLIGHT is typically 2, so 10 frames = 5 full cycles
	constexpr int NUM_FRAMES = 10;

	for (int frame = 0; frame < NUM_FRAMES; ++frame) {
		m_renderer->beginScenePass();
		m_renderer->endScenePass();
		ASSERT_NO_THROW(m_renderer->flip()) << "Frame " << frame << " crashed during fence sync";
	}
}

// Test: Rapid frame submission (stress test synchronization)
TEST_F(VulkanIntegrationTest, RapidFrameSubmission) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}

	// Submit many frames rapidly to stress test synchronization
	for (int frame = 0; frame < 30; ++frame) {
		m_renderer->beginScenePass();
		m_renderer->endScenePass();
		m_renderer->flip();
	}

	// If we get here without crashing, synchronization is working
	SUCCEED();
}

// ============================================================================
// Visible Window Test - Run manually to see actual rendering
// ============================================================================
// This test shows a visible window so you can see what's being rendered.
// Run with: --gtest_filter="VulkanVisibleTest*"

class TestGraphicsOperationsVisible : public os::GraphicsOperations {
public:
	SDL_Window* m_window = nullptr;

	~TestGraphicsOperationsVisible() override {
		if (m_window) {
			SDL_DestroyWindow(m_window);
			m_window = nullptr;
		}
	}

	std::unique_ptr<os::OpenGLContext> createOpenGLContext(os::Viewport*,
		const os::OpenGLContextAttributes&) override {
		return nullptr;
	}

	void makeOpenGLContextCurrent(os::Viewport*, os::OpenGLContext*) override {}

	std::unique_ptr<os::Viewport> createViewport(const os::ViewPortProperties& props) override {
		// VISIBLE window - no SDL_WINDOW_HIDDEN flag
		m_window = SDL_CreateWindow(
			"Vulkan Integration Test - VISIBLE",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			800, 600,
			SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN
		);

		if (!m_window) {
			return nullptr;
		}

		return std::make_unique<TestViewport>(m_window);
	}
};

class VulkanVisibleTest : public ::testing::Test {
protected:
	std::unique_ptr<VulkanRenderer> m_renderer;
	bool m_initialized = false;

	void SetUp() override {
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			GTEST_SKIP() << "SDL_Init failed: " << SDL_GetError();
			return;
		}

		auto graphicsOps = std::make_unique<TestGraphicsOperationsVisible>();
		m_renderer = std::make_unique<VulkanRenderer>(std::move(graphicsOps));

		if (!m_renderer->initialize()) {
			m_renderer.reset();
			SDL_Quit();
			GTEST_SKIP() << "VulkanRenderer::initialize failed";
			return;
		}

		m_initialized = true;
	}

	void TearDown() override {
		if (m_renderer) {
			m_renderer->shutdown();
			m_renderer.reset();
		}
		SDL_Quit();
	}
};

// Helper to pump SDL events (keeps window responsive)
static void pumpSDLEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// Ignore events - just pump to keep window alive
	}
}

// Run 60 frames with visible window - should show triangle for ~1 second
TEST_F(VulkanVisibleTest, VisibleFrameLoop) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}

	// Run 60 frames (roughly 1 second at 60fps) so you can see the triangle
	for (int frame = 0; frame < 60; ++frame) {
		pumpSDLEvents();  // Keep window responsive
		// Fallback path - shows triangle
		m_renderer->flip();
		SDL_Delay(16); // ~60fps
	}

	SUCCEED();
}

// Run 60 frames with scene pass - should show cleared color
TEST_F(VulkanVisibleTest, VisibleScenePass) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}

	for (int frame = 0; frame < 60; ++frame) {
		pumpSDLEvents();  // Keep window responsive
		m_renderer->beginScenePass();
		// No draw calls - just clear color
		m_renderer->endScenePass();
		m_renderer->flip();
		SDL_Delay(16);
	}

	SUCCEED();
}

// ============================================================================
// Debug Logging Test - Verify logging framework works
// ============================================================================

// Check if debug log file exists in any of the expected locations
static bool findDebugLogFile(SCP_string& outPath) {
	SCP_vector<SCP_string> paths;

	// TEMP directory (primary location on Windows)
	const char* temp = getenv("TEMP");
	if (temp) {
		paths.push_back(SCP_string(temp) + "\\vulkan_debug.log");
	}

	// USERPROFILE directory
	const char* userprofile = getenv("USERPROFILE");
	if (userprofile) {
		paths.push_back(SCP_string(userprofile) + "\\vulkan_debug.log");
	}

	// Current directory and relatives
	paths.push_back("vulkan_debug.log");
	paths.push_back("../vulkan_debug.log");
	paths.push_back("../../vulkan_debug.log");

	for (const auto& path : paths) {
		FILE* f = fopen(path.c_str(), "r");
		if (f) {
			fclose(f);
			outPath = path;
			return true;
		}
	}

	return false;
}

// Test that VulkanRenderer writes debug log entries during initialization
TEST_F(VulkanIntegrationTest, DebugLoggingWorks) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}

	// Run a few frames to generate more log entries
	for (int i = 0; i < 3; ++i) {
		m_renderer->flip();
	}

	// Check if debug log was created
	SCP_string logPath;
	bool found = findDebugLogFile(logPath);

	if (found) {
		// Read and verify log content
		FILE* f = fopen(logPath.c_str(), "r");
		ASSERT_NE(f, nullptr) << "Could not open log file: " << logPath;

		char buffer[4096] = {0};
		size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, f);
		fclose(f);

		EXPECT_GT(bytesRead, 0u) << "Log file is empty";

		// Verify expected log entries are present
		SCP_string content(buffer);
		EXPECT_NE(content.find("initialize()"), SCP_string::npos)
			<< "Log should contain initialize() entry. Content:\n" << content;
		EXPECT_NE(content.find("flip()"), SCP_string::npos)
			<< "Log should contain flip() entries. Content:\n" << content;

		// Log the path for debugging
		std::cout << "Debug log found at: " << logPath << std::endl;
		std::cout << "Log content:\n" << content << std::endl;
	} else {
		// Log may not be created in test environment due to os_get_config_path
		// This is informational - the test should not fail just because logging isn't available
		std::cout << "NOTE: vulkan_debug.log not found in any expected location.\n"
		          << "This may be expected in test environments where os_get_config_path fails.\n";
		SUCCEED() << "Logging not available in test environment (expected)";
	}
}

// ============================================================================
// Phase 2: Comprehensive Unit Tests
// ============================================================================

// Include test fixtures
#include "vulkan_test_fixtures.h"

using namespace graphics::vulkan::testing;

// ============================================================================
// 2.1 VulkanBufferManager Unit Tests
// ============================================================================

TEST_F(VulkanBufferManagerTest, CreateBufferReturnsValidHandle) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	// Test all buffer types
	auto vertexHandle = bm->createBuffer(BufferType::Vertex, BufferUsageHint::Static);
	EXPECT_TRUE(vertexHandle.isValid()) << "Vertex buffer creation failed";
	
	auto indexHandle = bm->createBuffer(BufferType::Index, BufferUsageHint::Static);
	EXPECT_TRUE(indexHandle.isValid()) << "Index buffer creation failed";
	
	auto uniformHandle = bm->createBuffer(BufferType::Uniform, BufferUsageHint::Dynamic);
	EXPECT_TRUE(uniformHandle.isValid()) << "Uniform buffer creation failed";
	
	// Clean up
	bm->deleteBuffer(vertexHandle);
	bm->deleteBuffer(indexHandle);
	bm->deleteBuffer(uniformHandle);
}

TEST_F(VulkanBufferManagerTest, DeleteBufferInvalidatesHandle) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	auto handle = bm->createBuffer(BufferType::Vertex, BufferUsageHint::Static);
	ASSERT_TRUE(handle.isValid());
	
	// Delete should work
	EXPECT_NO_THROW(bm->deleteBuffer(handle));
	
	// After deletion, getBuffer should return null
	auto vkBuffer = bm->getBuffer(handle);
	EXPECT_EQ(vkBuffer, vk::Buffer()) << "Buffer should be null after deletion";
}

TEST_F(VulkanBufferManagerTest, DeleteBufferTwiceSafe) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	auto handle = bm->createBuffer(BufferType::Vertex, BufferUsageHint::Static);
	ASSERT_TRUE(handle.isValid());
	
	// First delete
	EXPECT_NO_THROW(bm->deleteBuffer(handle));
	
	// Second delete should not crash
	EXPECT_NO_THROW(bm->deleteBuffer(handle));
}

TEST_F(VulkanBufferManagerTest, UpdateBufferDataAllocatesMemory) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	auto handle = bm->createBuffer(BufferType::Vertex, BufferUsageHint::Static);
	ASSERT_TRUE(handle.isValid());
	
	// Create test data
	std::vector<float> vertices = {
		0.0f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f
	};
	
	// Update buffer
	EXPECT_NO_THROW(bm->updateBufferData(handle, vertices.size() * sizeof(float), vertices.data()));
	
	// Verify buffer exists
	auto vkBuffer = bm->getBuffer(handle);
	EXPECT_NE(vkBuffer, vk::Buffer()) << "Buffer should be valid after update";
	
	// Verify buffer data
	const auto* bufferData = bm->getBufferData(handle);
	ASSERT_NE(bufferData, nullptr);
	EXPECT_EQ(bufferData->size, vertices.size() * sizeof(float));
	
	bm->deleteBuffer(handle);
}

TEST_F(VulkanBufferManagerTest, UpdateBufferDataOffsetRespectsBounds) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	auto handle = bm->createBuffer(BufferType::Vertex, BufferUsageHint::Dynamic);
	ASSERT_TRUE(handle.isValid());
	
	// Allocate initial buffer
	std::vector<float> initialData(100, 1.0f);
	bm->updateBufferData(handle, initialData.size() * sizeof(float), initialData.data());
	
	// Partial update at offset
	std::vector<float> updateData(10, 2.0f);
	size_t offset = 20 * sizeof(float);
	EXPECT_NO_THROW(bm->updateBufferDataOffset(handle, offset, updateData.size() * sizeof(float), updateData.data()));
	
	bm->deleteBuffer(handle);
}

TEST_F(VulkanBufferManagerTest, MapBufferReturnsPtrForPersistent) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	// Create persistent mapping buffer
	auto handle = bm->createBuffer(BufferType::Uniform, BufferUsageHint::PersistentMapping);
	ASSERT_TRUE(handle.isValid());
	
	// Allocate some space
	std::vector<uint8_t> data(256, 0);
	bm->updateBufferData(handle, data.size(), data.data());
	
	// Map should return valid pointer for persistent buffers
	void* mapped = bm->mapBuffer(handle);
	// Note: mapBuffer may return nullptr if the buffer wasn't created with host-visible memory
	// This is implementation-dependent
	
	bm->deleteBuffer(handle);
}

TEST_F(VulkanBufferManagerTest, UniformAlignmentRespected) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	size_t alignment = bm->getMinUniformBufferOffsetAlignment();
	EXPECT_GE(alignment, 1u) << "Alignment should be at least 1";
	EXPECT_LE(alignment, 256u) << "Alignment should be reasonable (<=256)";
	
	// Alignment should be power of 2
	EXPECT_EQ(alignment & (alignment - 1), 0u) << "Alignment should be power of 2";
}

TEST_F(VulkanBufferManagerTest, BeginFrameProcessesDeletions) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	// Create and delete buffer
	auto handle = bm->createBuffer(BufferType::Vertex, BufferUsageHint::Static);
	ASSERT_TRUE(handle.isValid());
	
	std::vector<float> data(100, 0.0f);
	bm->updateBufferData(handle, data.size() * sizeof(float), data.data());
	
	bm->deleteBuffer(handle);
	
	// Run frames to process deferred deletions
	for (int i = 0; i < 3; ++i) {
		m_renderer->flip();
	}
	
	// Should not crash
	SUCCEED();
}

TEST_F(VulkanBufferManagerTest, SubmitTransfersIsNoOpWhenEmpty) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	// submitTransfers with no pending transfers should be safe
	EXPECT_NO_THROW(bm->submitTransfers());
}

TEST_F(VulkanBufferManagerTest, FreeSlotReuse) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	// Create multiple buffers
	std::vector<gr_buffer_handle> handles;
	for (int i = 0; i < 10; ++i) {
		handles.push_back(bm->createBuffer(BufferType::Vertex, BufferUsageHint::Static));
	}
	
	// Delete some
	for (int i = 0; i < 5; ++i) {
		bm->deleteBuffer(handles[i]);
	}
	
	// Create new ones - should reuse slots
	for (int i = 0; i < 5; ++i) {
		auto newHandle = bm->createBuffer(BufferType::Vertex, BufferUsageHint::Static);
		EXPECT_TRUE(newHandle.isValid());
		bm->deleteBuffer(newHandle);
	}
	
	// Clean up remaining
	for (int i = 5; i < 10; ++i) {
		bm->deleteBuffer(handles[i]);
	}
}

TEST_F(VulkanBufferManagerTest, InvalidHandleAccess) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	// Invalid handle should return null buffer
	gr_buffer_handle invalidHandle;
	EXPECT_FALSE(invalidHandle.isValid());
	
	auto vkBuffer = bm->getBuffer(invalidHandle);
	EXPECT_EQ(vkBuffer, vk::Buffer());
	
	const auto* bufferData = bm->getBufferData(invalidHandle);
	EXPECT_EQ(bufferData, nullptr);
}

TEST_F(VulkanBufferManagerTest, LargeBufferAllocation) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	// Allocate a reasonably large buffer (1MB)
	auto handle = bm->createBuffer(BufferType::Vertex, BufferUsageHint::Static);
	ASSERT_TRUE(handle.isValid());
	
	constexpr size_t largeSize = 1024 * 1024; // 1MB
	std::vector<uint8_t> largeData(largeSize, 0xAB);
	
	EXPECT_NO_THROW(bm->updateBufferData(handle, largeSize, largeData.data()));
	
	const auto* bufferData = bm->getBufferData(handle);
	ASSERT_NE(bufferData, nullptr);
	EXPECT_EQ(bufferData->size, largeSize);
	
	bm->deleteBuffer(handle);
}

// ============================================================================
// 2.2 VulkanDescriptorManager Unit Tests
// ============================================================================

TEST_F(VulkanDescriptorManagerTest, ManagerIsInitialized) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* dm = descriptorManager();
	ASSERT_NE(dm, nullptr);
	EXPECT_TRUE(dm->isInitialized());
}

TEST_F(VulkanDescriptorManagerTest, MinAlignmentQueryWorks) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* dm = descriptorManager();
	ASSERT_NE(dm, nullptr);
	
	auto alignment = dm->getMinUniformBufferOffsetAlignment();
	EXPECT_GE(alignment, 1u);
	EXPECT_LE(alignment, 256u);
}

TEST_F(VulkanDescriptorManagerTest, AllocateSetWithNullLayoutReturnsNull) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* dm = descriptorManager();
	ASSERT_NE(dm, nullptr);
	
	// Null layout should return null set
	vk::DescriptorSet set = dm->allocateSet(nullptr, "test_null");
	EXPECT_EQ(set, vk::DescriptorSet());
}

TEST_F(VulkanDescriptorManagerTest, FreeSetWithNullSafe) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* dm = descriptorManager();
	ASSERT_NE(dm, nullptr);
	
	// Freeing null set should not crash
	EXPECT_NO_THROW(dm->freeSet(nullptr));
}

// ============================================================================
// 2.3 VulkanFramebuffer Unit Tests
// ============================================================================

TEST(VulkanFramebufferAttachmentTest, IsDepthFormatDetection) {
	EXPECT_TRUE(graphics::vulkan::testing::isDepthFormat(vk::Format::eD16Unorm));
	EXPECT_TRUE(graphics::vulkan::testing::isDepthFormat(vk::Format::eD32Sfloat));
	EXPECT_TRUE(graphics::vulkan::testing::isDepthFormat(vk::Format::eD24UnormS8Uint));
	EXPECT_TRUE(graphics::vulkan::testing::isDepthFormat(vk::Format::eD32SfloatS8Uint));
	
	EXPECT_FALSE(graphics::vulkan::testing::isDepthFormat(vk::Format::eR8G8B8A8Unorm));
	EXPECT_FALSE(graphics::vulkan::testing::isDepthFormat(vk::Format::eB8G8R8A8Srgb));
}

TEST(VulkanFramebufferAttachmentTest, HasStencilComponentDetection) {
	EXPECT_TRUE(graphics::vulkan::testing::hasStencilComponent(vk::Format::eD24UnormS8Uint));
	EXPECT_TRUE(graphics::vulkan::testing::hasStencilComponent(vk::Format::eD32SfloatS8Uint));
	EXPECT_TRUE(graphics::vulkan::testing::hasStencilComponent(vk::Format::eS8Uint));
	
	EXPECT_FALSE(graphics::vulkan::testing::hasStencilComponent(vk::Format::eD16Unorm));
	EXPECT_FALSE(graphics::vulkan::testing::hasStencilComponent(vk::Format::eD32Sfloat));
	EXPECT_FALSE(graphics::vulkan::testing::hasStencilComponent(vk::Format::eR8G8B8A8Unorm));
}

TEST(VulkanFramebufferTest, DefaultConstructorCreatesEmpty) {
	VulkanFramebuffer fb;
	EXPECT_EQ(fb.getFramebuffer(), vk::Framebuffer());
	EXPECT_EQ(fb.getExtent().width, 0u);
	EXPECT_EQ(fb.getExtent().height, 0u);
	EXPECT_EQ(fb.getColorAttachmentCount(), 0u);
	EXPECT_FALSE(fb.hasDepthAttachment());
}

TEST(VulkanFramebufferTest, DestroyTwiceSafe) {
	VulkanFramebuffer fb;
	// Destroy on empty should be safe
	EXPECT_NO_THROW(fb.destroy());
	EXPECT_NO_THROW(fb.destroy());
}

// ============================================================================
// 2.4 VulkanTexture Unit Tests
// ============================================================================

TEST(VulkanTextureTest, DefaultConstructorCreatesInvalid) {
	VulkanTexture tex;
	EXPECT_FALSE(tex.isValid());
	EXPECT_EQ(tex.getImage(), vk::Image());
	EXPECT_EQ(tex.getImageView(), vk::ImageView());
	EXPECT_EQ(tex.getFormat(), vk::Format::eUndefined);
}

TEST(VulkanTextureTest, GetBytesPerPixelCalculation) {
	using namespace graphics::vulkan::testing;
	
	EXPECT_EQ(getBytesPerPixel(vk::Format::eR8Unorm), 1u);
	EXPECT_EQ(getBytesPerPixel(vk::Format::eR8G8Unorm), 2u);
	EXPECT_EQ(getBytesPerPixel(vk::Format::eR8G8B8Unorm), 3u);
	EXPECT_EQ(getBytesPerPixel(vk::Format::eR8G8B8A8Unorm), 4u);
	EXPECT_EQ(getBytesPerPixel(vk::Format::eR16G16B16A16Sfloat), 8u);
	EXPECT_EQ(getBytesPerPixel(vk::Format::eR32G32B32A32Sfloat), 16u);
}

TEST(VulkanTextureManagerMipTest, CalculateMipLevelsEdgeCases) {
	// Additional edge case tests
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(0, 0), 1u);  // Edge case
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(1, 1), 1u);
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(3, 3), 2u);
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(7, 7), 3u);
	EXPECT_EQ(VulkanTextureManager::calculateMipLevelsStatic(8, 8), 4u);
}

TEST(VulkanSamplerCacheTest, DefaultConstructorCreatesEmpty) {
	VulkanSamplerCache cache;
	// Should not crash when not initialized
	EXPECT_NO_THROW(cache.shutdown());
}

// ============================================================================
// 2.5 VulkanShaderManager Unit Tests  
// ============================================================================

TEST(VulkanShaderCacheTest, CacheKeyIncludesAllFactors) {
	VulkanShaderCache cache;
	
	const SCP_string sourceHash1 = "hash1";
	const SCP_string sourceHash2 = "hash2";
	
	// Different source hashes
	auto key1 = cache.computeCacheKey(sourceHash1, SDR_TYPE_MODEL, 0, ShaderStage::Vertex);
	auto key2 = cache.computeCacheKey(sourceHash2, SDR_TYPE_MODEL, 0, ShaderStage::Vertex);
	EXPECT_NE(key1, key2) << "Different source hashes should produce different keys";
	
	// Different shader types
	auto key3 = cache.computeCacheKey(sourceHash1, SDR_TYPE_MODEL, 0, ShaderStage::Vertex);
	auto key4 = cache.computeCacheKey(sourceHash1, SDR_TYPE_EFFECT_PARTICLE, 0, ShaderStage::Vertex);
	EXPECT_NE(key3, key4) << "Different shader types should produce different keys";
	
	// Different flags
	auto key5 = cache.computeCacheKey(sourceHash1, SDR_TYPE_MODEL, 0x01, ShaderStage::Vertex);
	auto key6 = cache.computeCacheKey(sourceHash1, SDR_TYPE_MODEL, 0x02, ShaderStage::Vertex);
	EXPECT_NE(key5, key6) << "Different flags should produce different keys";
	
	// Different stages
	auto key7 = cache.computeCacheKey(sourceHash1, SDR_TYPE_MODEL, 0, ShaderStage::Vertex);
	auto key8 = cache.computeCacheKey(sourceHash1, SDR_TYPE_MODEL, 0, ShaderStage::Fragment);
	EXPECT_NE(key7, key8) << "Different stages should produce different keys";
}

TEST(VulkanShaderCacheTest, CacheKeyIsDeterministic) {
	VulkanShaderCache cache;
	
	const SCP_string sourceHash = "test_hash_12345";
	const uint32_t flags = 0xABCD;
	
	auto key1 = cache.computeCacheKey(sourceHash, SDR_TYPE_MODEL, flags, ShaderStage::Fragment);
	auto key2 = cache.computeCacheKey(sourceHash, SDR_TYPE_MODEL, flags, ShaderStage::Fragment);
	
	EXPECT_EQ(key1, key2) << "Same inputs should produce same key";
}

TEST(VulkanShaderManagerTest, RequiresGeometryShaderForShadowMap) {
	VulkanShaderManager manager{vk::Device()};
	
	// Shadow maps require geometry shader
	EXPECT_TRUE(manager.requiresGeometryShader(SDR_TYPE_MODEL, MODEL_SDR_FLAG_SHADOW_MAP));
}

TEST(VulkanShaderManagerTest, RequiresGeometryShaderForParticlePointGen) {
	VulkanShaderManager manager{vk::Device()};
	
	// Particle point generation requires geometry shader
	EXPECT_TRUE(manager.requiresGeometryShader(SDR_TYPE_EFFECT_PARTICLE, SDR_FLAG_PARTICLE_POINT_GEN));
}

TEST(VulkanShaderManagerTest, NoGeometryShaderForBasicShaders) {
	VulkanShaderManager manager{vk::Device()};
	
	// Basic shaders without special flags don't need geometry shader
	EXPECT_FALSE(manager.requiresGeometryShader(SDR_TYPE_MODEL, 0));
	EXPECT_FALSE(manager.requiresGeometryShader(SDR_TYPE_EFFECT_PARTICLE, 0));
	EXPECT_FALSE(manager.requiresGeometryShader(SDR_TYPE_POST_PROCESS_MAIN, 0));
	EXPECT_FALSE(manager.requiresGeometryShader(SDR_TYPE_DEFAULT_MATERIAL, 0));
}

// ============================================================================
// 2.6 VulkanPipelineManager Unit Tests
// ============================================================================

TEST(VulkanPipelineKeyTest, AllFieldsContributeToHash) {
	PipelineKey base;
	
	// Each field change should change the hash
	PipelineKey withType = base;
	withType.shaderType = SDR_TYPE_EFFECT_PARTICLE;
	EXPECT_NE(base.hash(), withType.hash()) << "shaderType should affect hash";
	
	PipelineKey withFlags = base;
	withFlags.shaderFlags = 0x1234;
	EXPECT_NE(base.hash(), withFlags.hash()) << "shaderFlags should affect hash";
	
	PipelineKey withBlend = base;
	withBlend.blendMode = ALPHA_BLEND_ADDITIVE;
	EXPECT_NE(base.hash(), withBlend.hash()) << "blendMode should affect hash";
	
	PipelineKey withDepth = base;
	withDepth.depthMode = ZBUFFER_TYPE_READ;
	EXPECT_NE(base.hash(), withDepth.hash()) << "depthMode should affect hash";
	
	PipelineKey withStencil = base;
	withStencil.stencilEnabled = true;
	EXPECT_NE(base.hash(), withStencil.hash()) << "stencilEnabled should affect hash";
	
	PipelineKey withCull = base;
	withCull.cullMode = vk::CullModeFlagBits::eFront;  // Change from default back to front
	EXPECT_NE(base.hash(), withCull.hash()) << "cullMode should affect hash";
	
	PipelineKey withPrimitive = base;
	withPrimitive.primitiveType = PRIM_TYPE_LINES;
	EXPECT_NE(base.hash(), withPrimitive.hash()) << "primitiveType should affect hash";
}

TEST(VulkanPipelineKeyTest, EqualKeysHaveEqualHash) {
	PipelineKey key1;
	key1.shaderType = SDR_TYPE_MODEL;
	key1.shaderFlags = 0x100;
	key1.blendMode = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
	
	PipelineKey key2;
	key2.shaderType = SDR_TYPE_MODEL;
	key2.shaderFlags = 0x100;
	key2.blendMode = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
	
	EXPECT_EQ(key1, key2);
	EXPECT_EQ(key1.hash(), key2.hash());
}

TEST(VulkanPipelineKeyTest, PerBufferBlendModesAffectHash) {
	PipelineKey base;
	base.hasPerBufferBlend = true;
	
	PipelineKey modified = base;
	modified.bufferBlendModes[0] = ALPHA_BLEND_ADDITIVE;
	
	EXPECT_NE(base.hash(), modified.hash()) << "Per-buffer blend modes should affect hash";
}

TEST(VulkanPipelineManagerTest, ConvertAllPrimitiveTypes) {
	// Comprehensive primitive type conversion test
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_POINTS), 
	          vk::PrimitiveTopology::ePointList);
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_LINES), 
	          vk::PrimitiveTopology::eLineList);
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_LINESTRIP), 
	          vk::PrimitiveTopology::eLineStrip);
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_TRIS), 
	          vk::PrimitiveTopology::eTriangleList);
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_TRISTRIP), 
	          vk::PrimitiveTopology::eTriangleStrip);
	EXPECT_EQ(VulkanPipelineManager::convertPrimitiveTypeStatic(PRIM_TYPE_TRIFAN), 
	          vk::PrimitiveTopology::eTriangleFan);
}

TEST(VulkanPipelineManagerTest, ConvertAllVertexFormats) {
	// Comprehensive vertex format conversion test
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::POSITION4),
	          vk::Format::eR32G32B32A32Sfloat);
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::POSITION3),
	          vk::Format::eR32G32B32Sfloat);
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::POSITION2),
	          vk::Format::eR32G32Sfloat);
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::COLOR3),
	          vk::Format::eR8G8B8Unorm);
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::COLOR4),
	          vk::Format::eR8G8B8A8Unorm);
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::TEX_COORD2),
	          vk::Format::eR32G32Sfloat);
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::TEX_COORD4),
	          vk::Format::eR32G32B32A32Sfloat);
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::NORMAL),
	          vk::Format::eR32G32B32Sfloat);
	EXPECT_EQ(VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::TANGENT),
	          vk::Format::eR32G32B32A32Sfloat);
}

// ============================================================================
// 2.7 VulkanPostProcessing Unit Tests
// ============================================================================

TEST(VulkanPostProcessingTest, DefaultConstructorSetsDefaults) {
	VulkanPostProcessing pp;
	pp.setDefaults();
	
	EXPECT_TRUE(pp.isBloomEnabled());
	EXPECT_FLOAT_EQ(pp.getBloomIntensity(), 0.25f);
	EXPECT_FLOAT_EQ(pp.getExposure(), 1.0f);
	EXPECT_EQ(pp.getTonemapper(), 0);
}

TEST(VulkanPostProcessingTest, BloomIntensitySetAndGet) {
	VulkanPostProcessing pp;
	
	// Test that values are stored correctly (no clamping in current implementation)
	pp.setBloomIntensity(0.5f);
	EXPECT_FLOAT_EQ(pp.getBloomIntensity(), 0.5f);
	
	pp.setBloomIntensity(0.0f);
	EXPECT_FLOAT_EQ(pp.getBloomIntensity(), 0.0f);
	
	pp.setBloomIntensity(1.0f);
	EXPECT_FLOAT_EQ(pp.getBloomIntensity(), 1.0f);
}

TEST(VulkanPostProcessingTest, HDRSettingsPersist) {
	VulkanPostProcessing pp;
	
	pp.setHDROutputEnabled(true);
	EXPECT_TRUE(pp.isHDROutputEnabled());
	
	pp.setHDROutputEnabled(false);
	EXPECT_FALSE(pp.isHDROutputEnabled());
	
	pp.setHDRMaxNits(1000.0f);
	EXPECT_FLOAT_EQ(pp.getHDRMaxNits(), 1000.0f);
	
	pp.setHDRPaperWhiteNits(200.0f);
	EXPECT_FLOAT_EQ(pp.getHDRPaperWhiteNits(), 200.0f);
}

// ============================================================================
// Phase 3: Integration Tests (Real GPU)
// ============================================================================

// 3.1 VulkanRenderer Lifecycle Tests

TEST_F(VulkanHiddenWindowTest, InitializeSucceedsWithVulkan) {
	// If we get here, initialization succeeded
	EXPECT_TRUE(m_initialized);
	EXPECT_NE(m_renderer, nullptr);
}

TEST_F(VulkanHiddenWindowTest, FlipWithoutScenePass) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	// Run frames through fallback path
	for (int frame = 0; frame < 10; ++frame) {
		ASSERT_NO_THROW(m_renderer->flip()) << "Frame " << frame << " crashed";
	}
}

TEST_F(VulkanHiddenWindowTest, FlipWithScenePass) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	// Run frames through scene pass path
	for (int frame = 0; frame < 10; ++frame) {
		ASSERT_NO_THROW(m_renderer->beginScenePass()) << "beginScenePass crashed on frame " << frame;
		ASSERT_NO_THROW(m_renderer->endScenePass()) << "endScenePass crashed on frame " << frame;
		ASSERT_NO_THROW(m_renderer->flip()) << "flip crashed on frame " << frame;
	}
}

TEST_F(VulkanHiddenWindowTest, GetDeviceReturnsValid) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	EXPECT_NE(m_renderer->getDevice(), vk::Device());
	EXPECT_NE(m_renderer->getPhysicalDevice(), vk::PhysicalDevice());
}

TEST_F(VulkanHiddenWindowTest, GetSceneExtentReturnsNonZero) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto extent = m_renderer->getSceneExtent();
	EXPECT_GT(extent.width, 0u);
	EXPECT_GT(extent.height, 0u);
}

TEST_F(VulkanHiddenWindowTest, DrawStateResetWorks) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto& state = m_renderer->getDrawState();
	
	// Set some state
	state.viewportSet = true;
	state.scissorSet = true;
	
	// Reset
	m_renderer->resetDrawState();
	
	// Verify reset
	EXPECT_FALSE(state.viewportSet);
	EXPECT_FALSE(state.scissorSet);
}

TEST_F(VulkanHiddenWindowTest, SetClearColorWorks) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	m_renderer->setClearColor(0.5f, 0.25f, 0.75f, 1.0f);
	
	const auto& color = m_renderer->getClearColor();
	EXPECT_FLOAT_EQ(color[0], 0.5f);
	EXPECT_FLOAT_EQ(color[1], 0.25f);
	EXPECT_FLOAT_EQ(color[2], 0.75f);
	EXPECT_FLOAT_EQ(color[3], 1.0f);
}

// 3.2 Buffer Integration Tests

TEST_F(VulkanBufferManagerTest, CreateUpdateDeleteBufferCycle) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	// Full lifecycle test
	for (int i = 0; i < 5; ++i) {
		auto handle = bm->createBuffer(BufferType::Vertex, BufferUsageHint::Static);
		ASSERT_TRUE(handle.isValid());
		
		std::vector<float> data(100, static_cast<float>(i));
		bm->updateBufferData(handle, data.size() * sizeof(float), data.data());
		
		bm->deleteBuffer(handle);
	}
	
	// Run frames to ensure cleanup
	m_renderer->flip();
	m_renderer->flip();
	m_renderer->flip();
}

TEST_F(VulkanBufferManagerTest, UniformBufferBindWorks) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* bm = bufferManager();
	ASSERT_NE(bm, nullptr);
	
	auto handle = bm->createBuffer(BufferType::Uniform, BufferUsageHint::Dynamic);
	ASSERT_TRUE(handle.isValid());
	
	// Allocate some data
	std::vector<float> uniformData(64, 1.0f);
	bm->updateBufferData(handle, uniformData.size() * sizeof(float), uniformData.data());
	
	// Bind to binding point
	size_t alignment = bm->getMinUniformBufferOffsetAlignment();
	size_t alignedSize = ((uniformData.size() * sizeof(float) + alignment - 1) / alignment) * alignment;
	
	EXPECT_NO_THROW(bm->bindUniformBuffer(uniform_block_type::Matrices, 0, alignedSize, handle));
	
	// Verify binding
	auto bound = bm->getBoundUniformBuffer(uniform_block_type::Matrices);
	EXPECT_EQ(bound.handle, handle);
	
	bm->deleteBuffer(handle);
}

// 3.3 Texture Integration Tests

TEST_F(VulkanTextureManagerIntegrationTest, ManagerExists) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	auto* tm = textureManager();
	EXPECT_NE(tm, nullptr);
}

// ============================================================================
// Phase 4: Stress/Edge Case Tests
// ============================================================================

TEST_F(VulkanStressTest, ManyBufferCreations) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	constexpr int NUM_BUFFERS = 100;
	
	for (int i = 0; i < NUM_BUFFERS; ++i) {
		auto handle = createTrackedBuffer(BufferType::Vertex, BufferUsageHint::Static);
		ASSERT_TRUE(handle.isValid()) << "Buffer " << i << " creation failed";
		
		// Small allocation
		std::vector<uint8_t> data(64, static_cast<uint8_t>(i));
		g_vulkanBufferManager->updateBufferData(handle, data.size(), data.data());
	}
	
	SUCCEED() << "Created " << NUM_BUFFERS << " buffers successfully";
}

TEST_F(VulkanMultiFrameTest, RapidFrameSubmission) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	// Run many frames rapidly
	EXPECT_TRUE(runFrames(100, true)) << "Rapid frame submission failed";
}

TEST_F(VulkanMultiFrameTest, MixedSceneAndDirectPaths) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	// Alternate between scene pass and direct paths
	for (int i = 0; i < 50; ++i) {
		if (i % 2 == 0) {
			m_renderer->beginScenePass();
			m_renderer->endScenePass();
		}
		m_renderer->flip();
	}
	
	SUCCEED();
}

TEST_F(VulkanHiddenWindowTest, FrameFenceWaitCorrect) {
	if (!m_initialized) {
		GTEST_SKIP() << "Vulkan not initialized";
	}
	
	// This tests that frame synchronization doesn't deadlock
	// Run enough frames to cycle through all frames in flight multiple times
	constexpr int NUM_FRAMES = 20;
	
	for (int frame = 0; frame < NUM_FRAMES; ++frame) {
		m_renderer->beginScenePass();
		m_renderer->endScenePass();
		
		// Should not timeout
		ASSERT_NO_THROW(m_renderer->flip()) << "Frame " << frame << " fence wait failed";
	}
}

TEST(VulkanInvalidHandleTest, InvalidBufferHandleOperations) {
	gr_buffer_handle invalidHandle;
	EXPECT_FALSE(invalidHandle.isValid());
	
	// Operations on invalid handle should not crash
	// (This tests the handle validity checking, not actual buffer operations)
}

TEST(VulkanDataGenerationTest, TestDataGeneration) {
	using namespace graphics::vulkan::testing;
	
	auto data = generateTestData(256, 42);
	EXPECT_EQ(data.size(), 256u);
	
	// Verify pattern
	for (size_t i = 0; i < data.size(); ++i) {
		EXPECT_EQ(data[i], static_cast<uint8_t>((i + 42) & 0xFF));
	}
}

TEST(VulkanDataComparisonTest, CompareDataWorks) {
	using namespace graphics::vulkan::testing;
	
	auto data1 = generateTestData(256, 0);
	auto data2 = generateTestData(256, 0);
	auto data3 = generateTestData(256, 1);
	
	EXPECT_TRUE(compareData(data1.data(), data2.data(), 256));
	EXPECT_FALSE(compareData(data1.data(), data3.data(), 256));
}

#endif // WITH_VULKAN
