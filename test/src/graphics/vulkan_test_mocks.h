#pragma once

#ifdef WITH_VULKAN

#include <gtest/gtest.h>
#include <vulkan/vulkan.hpp>
#include <SDL.h>
#include <SDL_vulkan.h>

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "graphics/vulkan/VulkanRenderer.h"

namespace graphics {
namespace vulkan {
namespace testing {

// ============================================================================
// Test Utilities
// ============================================================================

/**
 * @brief Test-specific GraphicsOperations that creates an SDL Vulkan window
 * 
 * Creates a hidden window by default for headless testing.
 */
class TestGraphicsOperations : public os::GraphicsOperations {
public:
	SDL_Window* m_window = nullptr;
	bool m_visible = false;
	int m_width = 800;
	int m_height = 600;

	explicit TestGraphicsOperations(bool visible = false, int width = 800, int height = 600)
		: m_visible(visible), m_width(width), m_height(height) {}

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

/**
 * @brief Minimal test viewport wrapping an SDL window
 */
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

inline std::unique_ptr<os::Viewport> TestGraphicsOperations::createViewport(const os::ViewPortProperties& props) {
	Uint32 windowFlags = SDL_WINDOW_VULKAN;

	if (!m_visible) {
		windowFlags |= SDL_WINDOW_HIDDEN;
	}

	m_window = SDL_CreateWindow(
		props.title.c_str(),
		m_visible ? SDL_WINDOWPOS_CENTERED : SDL_WINDOWPOS_UNDEFINED,
		m_visible ? SDL_WINDOWPOS_CENTERED : SDL_WINDOWPOS_UNDEFINED,
		m_width,
		m_height,
		windowFlags
	);

	if (!m_window) {
		return nullptr;
	}

	return std::make_unique<TestViewport>(m_window);
}

// ============================================================================
// Base Test Fixture for Vulkan Integration Tests
// ============================================================================

/**
 * @brief Base fixture for tests requiring full Vulkan initialization
 * 
 * Handles SDL init, renderer creation, and cleanup.
 * Skips tests gracefully when Vulkan is unavailable.
 */
class VulkanTestFixture : public ::testing::Test {
protected:
	std::unique_ptr<VulkanRenderer> m_renderer;
	bool m_initialized = false;
	bool m_visible = false;

	explicit VulkanTestFixture(bool visible = false) : m_visible(visible) {}

	void SetUp() override {
		// Initialize SDL with video support
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			GTEST_SKIP() << "SDL_Init failed: " << SDL_GetError();
			return;
		}

		// Create test graphics operations
		auto graphicsOps = std::make_unique<TestGraphicsOperations>(m_visible);

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

	bool isInitialized() const { return m_initialized; }

	VulkanRenderer* renderer() { return m_renderer.get(); }
};

/**
 * @brief Fixture for hidden window tests (default)
 */
class VulkanHiddenWindowTest : public VulkanTestFixture {
protected:
	VulkanHiddenWindowTest() : VulkanTestFixture(false) {}
};

/**
 * @brief Fixture for visible window tests (manual verification)
 */
class VulkanVisibleWindowTest : public VulkanTestFixture {
protected:
	VulkanVisibleWindowTest() : VulkanTestFixture(true) {}
};

// ============================================================================
// Mock Device for Unit Tests (No GPU Required)
// ============================================================================

/**
 * @brief Mock device limits for unit testing without GPU
 */
struct MockDeviceLimits {
	size_t minUniformBufferOffsetAlignment = 256;
	size_t maxUniformBufferRange = 65536;
	float maxSamplerAnisotropy = 16.0f;
	uint32_t maxDescriptorSetUniformBuffers = 12;
	uint32_t maxDescriptorSetSampledImages = 16;
};

/**
 * @brief Helper to check if a vk::Format is a depth format
 */
inline bool isDepthFormat(vk::Format format) {
	switch (format) {
		case vk::Format::eD16Unorm:
		case vk::Format::eD32Sfloat:
		case vk::Format::eD16UnormS8Uint:
		case vk::Format::eD24UnormS8Uint:
		case vk::Format::eD32SfloatS8Uint:
			return true;
		default:
			return false;
	}
}

/**
 * @brief Helper to check if a vk::Format has stencil
 */
inline bool hasStencilComponent(vk::Format format) {
	switch (format) {
		case vk::Format::eD16UnormS8Uint:
		case vk::Format::eD24UnormS8Uint:
		case vk::Format::eD32SfloatS8Uint:
		case vk::Format::eS8Uint:
			return true;
		default:
			return false;
	}
}

/**
 * @brief Helper to get bytes per pixel for common formats
 */
inline size_t getBytesPerPixel(vk::Format format) {
	switch (format) {
		case vk::Format::eR8Unorm:
		case vk::Format::eR8Snorm:
		case vk::Format::eR8Uint:
		case vk::Format::eR8Sint:
			return 1;
		case vk::Format::eR8G8Unorm:
		case vk::Format::eR8G8Snorm:
		case vk::Format::eR16Sfloat:
		case vk::Format::eR5G6B5UnormPack16:
		case vk::Format::eR5G5B5A1UnormPack16:
			return 2;
		case vk::Format::eR8G8B8Unorm:
		case vk::Format::eB8G8R8Unorm:
			return 3;
		case vk::Format::eR8G8B8A8Unorm:
		case vk::Format::eR8G8B8A8Srgb:
		case vk::Format::eB8G8R8A8Unorm:
		case vk::Format::eB8G8R8A8Srgb:
		case vk::Format::eR32Sfloat:
		case vk::Format::eR16G16Sfloat:
		case vk::Format::eA2B10G10R10UnormPack32:
			return 4;
		case vk::Format::eR16G16B16A16Sfloat:
		case vk::Format::eR32G32Sfloat:
			return 8;
		case vk::Format::eR32G32B32Sfloat:
			return 12;
		case vk::Format::eR32G32B32A32Sfloat:
			return 16;
		default:
			return 4; // Default assumption
	}
}

/**
 * @brief Generate test data of specified size
 */
inline SCP_vector<uint8_t> generateTestData(size_t size, uint8_t seed = 0) {
	SCP_vector<uint8_t> data(size);
	for (size_t i = 0; i < size; ++i) {
		data[i] = static_cast<uint8_t>((i + seed) & 0xFF);
	}
	return data;
}

/**
 * @brief Compare two data buffers
 */
inline bool compareData(const void* a, const void* b, size_t size) {
	return std::memcmp(a, b, size) == 0;
}

} // namespace testing
} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN

