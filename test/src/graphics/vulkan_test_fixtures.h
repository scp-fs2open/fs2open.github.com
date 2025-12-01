#pragma once

#ifdef WITH_VULKAN

#include "vulkan_test_mocks.h"
#include "graphics/vulkan/VulkanBuffer.h"
#include "graphics/vulkan/VulkanDescriptorManager.h"
#include "graphics/vulkan/VulkanFramebuffer.h"
#include "graphics/vulkan/VulkanTexture.h"
#include "graphics/vulkan/VulkanShader.h"
#include "graphics/vulkan/VulkanPipelineManager.h"
#include "graphics/vulkan/VulkanPostProcessing.h"

namespace graphics {
namespace vulkan {
namespace testing {

// ============================================================================
// Buffer Manager Test Fixture
// ============================================================================

/**
 * @brief Fixture for VulkanBufferManager tests
 * 
 * Provides access to buffer manager through the renderer.
 */
class VulkanBufferManagerTest : public VulkanHiddenWindowTest {
protected:
	VulkanBufferManager* bufferManager() {
		if (!m_initialized || !m_renderer) return nullptr;
		// Access through the global pointer set by renderer
		return g_vulkanBufferManager;
	}
};

// ============================================================================
// Descriptor Manager Test Fixture
// ============================================================================

/**
 * @brief Fixture for VulkanDescriptorManager tests
 */
class VulkanDescriptorManagerTest : public VulkanHiddenWindowTest {
protected:
	VulkanDescriptorManager* descriptorManager() {
		if (!m_initialized || !m_renderer) return nullptr;
		return m_renderer->getDescriptorManager();
	}
};

// ============================================================================
// Texture Manager Test Fixture  
// ============================================================================

/**
 * @brief Fixture for VulkanTextureManager integration tests
 */
class VulkanTextureManagerTest : public VulkanHiddenWindowTest {
protected:
	VulkanTextureManager* textureManager() {
		if (!m_initialized || !m_renderer) return nullptr;
		return g_vulkanTextureManager;
	}
};

/**
 * @brief Alias for integration tests to avoid test suite naming conflicts
 */
class VulkanTextureManagerIntegrationTest : public VulkanTextureManagerTest {};

// ============================================================================
// Pipeline Manager Test Fixture
// ============================================================================

/**
 * @brief Fixture for VulkanPipelineManager tests
 */
class VulkanPipelineManagerTest : public VulkanHiddenWindowTest {
protected:
	VulkanPipelineManager* pipelineManager() {
		if (!m_initialized || !m_renderer) return nullptr;
		return g_vulkanPipelineManager;
	}
};

// ============================================================================
// Multi-Frame Test Fixture
// ============================================================================

/**
 * @brief Fixture for tests that need to run multiple frames
 */
class VulkanMultiFrameTest : public VulkanHiddenWindowTest {
protected:
	/**
	 * @brief Run N frames through the renderer
	 * @param numFrames Number of frames to run
	 * @param useScenePass If true, use scene pass path
	 * @return true if all frames completed without exception
	 */
	bool runFrames(int numFrames, bool useScenePass = false) {
		if (!m_initialized) return false;
		
		try {
			for (int i = 0; i < numFrames; ++i) {
				if (useScenePass) {
					m_renderer->beginScenePass();
					m_renderer->endScenePass();
				}
				m_renderer->flip();
			}
			return true;
		} catch (...) {
			return false;
		}
	}
};

// ============================================================================
// Stress Test Fixture
// ============================================================================

/**
 * @brief Fixture for stress tests with resource tracking
 */
class VulkanStressTest : public VulkanHiddenWindowTest {
protected:
	SCP_vector<gr_buffer_handle> m_createdBuffers;
	
	void TearDown() override {
		// Clean up any created buffers
		if (m_initialized && g_vulkanBufferManager) {
			for (auto handle : m_createdBuffers) {
				if (handle.isValid()) {
					g_vulkanBufferManager->deleteBuffer(handle);
				}
			}
		}
		m_createdBuffers.clear();
		
		VulkanHiddenWindowTest::TearDown();
	}
	
	gr_buffer_handle createTrackedBuffer(BufferType type, BufferUsageHint usage) {
		if (!m_initialized || !g_vulkanBufferManager) {
			return gr_buffer_handle();
		}
		auto handle = g_vulkanBufferManager->createBuffer(type, usage);
		if (handle.isValid()) {
			m_createdBuffers.push_back(handle);
		}
		return handle;
	}
};

} // namespace testing
} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN

