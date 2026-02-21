#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/material.h"

#include "VulkanPipeline.h"
#include "VulkanDescriptorManager.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

/**
 * @brief Tracks current Vulkan render state
 *
 * Unlike OpenGL where state is set globally, Vulkan requires explicit
 * command buffer recording. This class tracks what state has been set
 * and what needs to be updated before draw calls.
 */
class VulkanStateTracker {
public:
	VulkanStateTracker() = default;
	~VulkanStateTracker() = default;

	// Non-copyable
	VulkanStateTracker(const VulkanStateTracker&) = delete;
	VulkanStateTracker& operator=(const VulkanStateTracker&) = delete;

	/**
	 * @brief Initialize state tracker
	 */
	bool init(vk::Device device);

	/**
	 * @brief Shutdown and release resources
	 */
	void shutdown();

	/**
	 * @brief Begin recording for a new frame
	 * @param cmdBuffer Command buffer to record to
	 */
	void beginFrame(vk::CommandBuffer cmdBuffer);

	/**
	 * @brief End frame recording
	 */
	void endFrame();

	/**
	 * @brief Set the current render pass
	 */
	void setRenderPass(vk::RenderPass renderPass, uint32_t subpass = 0);

	/**
	 * @brief Get current render pass
	 */
	vk::RenderPass getCurrentRenderPass() const { return m_currentRenderPass; }

	// ========== Dynamic State ==========

	/**
	 * @brief Set viewport (dynamic state)
	 */
	void setViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

	/**
	 * @brief Set scissor rectangle (dynamic state)
	 */
	void setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

	/**
	 * @brief Enable or disable scissor test
	 */
	void setScissorEnabled(bool enabled);

	/**
	 * @brief Set depth bias (dynamic state)
	 */
	void setDepthBias(float constantFactor, float slopeFactor);

	/**
	 * @brief Set stencil reference value (dynamic state)
	 */
	void setStencilReference(uint32_t reference);

	/**
	 * @brief Set line width (dynamic state)
	 */
	void setLineWidth(float width);

	// ========== Pipeline State ==========

	/**
	 * @brief Bind a pipeline
	 */
	void bindPipeline(vk::Pipeline pipeline, vk::PipelineLayout layout);

	/**
	 * @brief Get currently bound pipeline
	 */
	vk::Pipeline getCurrentPipeline() const { return m_currentPipeline; }

	/**
	 * @brief Get current pipeline layout
	 */
	vk::PipelineLayout getCurrentPipelineLayout() const { return m_currentPipelineLayout; }

	// ========== Descriptor State ==========

	/**
	 * @brief Bind descriptor set
	 */
	void bindDescriptorSet(DescriptorSetIndex setIndex, vk::DescriptorSet set,
	                       const SCP_vector<uint32_t>& dynamicOffsets = {});

	// ========== Buffer Binding ==========

	/**
	 * @brief Bind vertex buffer
	 */
	void bindVertexBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset = 0);

	/**
	 * @brief Bind index buffer
	 */
	void bindIndexBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::IndexType indexType);

	// ========== State Queries ==========

	/**
	 * @brief Get current command buffer.
	 * Asserts if no command buffer is active — rendering outside a frame is always a bug.
	 */
	vk::CommandBuffer getCommandBuffer() const {
		Assertion(m_cmdBuffer, "No active command buffer — rendering outside a frame?");
		return m_cmdBuffer;
	}

	/**
	 * @brief Check if scissor test is enabled
	 */
	bool isScissorEnabled() const { return m_scissorEnabled; }

	// ========== Clear Operations ==========

	/**
	 * @brief Set clear color for next clear operation
	 */
	void setClearColor(float r, float g, float b, float a);

	/**
	 * @brief Get current clear color
	 */
	const vk::ClearColorValue& getClearColor() const { return m_clearColor; }

	// ========== Render State Tracking ==========

	/**
	 * @brief Set current zbuffer mode (for tracking)
	 */
	void setZBufferMode(gr_zbuffer_type mode) { m_zbufferMode = mode; }
	gr_zbuffer_type getZBufferMode() const { return m_zbufferMode; }

	/**
	 * @brief Set current stencil mode (for tracking)
	 */
	void setStencilMode(int mode) { m_stencilMode = mode; }
	int getStencilMode() const { return m_stencilMode; }

	/**
	 * @brief Set current cull mode (for tracking)
	 */
	void setCullMode(bool enabled) { m_cullEnabled = enabled; }
	bool getCullMode() const { return m_cullEnabled; }

	/**
	 * @brief Set color attachment count for current render pass
	 */
	void setColorAttachmentCount(uint32_t count) { m_colorAttachmentCount = count; }
	uint32_t getColorAttachmentCount() const { return m_colorAttachmentCount; }

	/**
	 * @brief Set current MSAA sample count for pipeline creation
	 */
	void setCurrentSampleCount(vk::SampleCountFlagBits count) { m_currentSampleCount = count; }
	vk::SampleCountFlagBits getCurrentSampleCount() const { return m_currentSampleCount; }

	/**
	 * @brief Apply pending dynamic state to command buffer
	 *
	 * Must be called before every draw command to ensure dirty dynamic state
	 * (viewport, scissor, depth bias, stencil ref, line width) is flushed.
	 * applyMaterial() sets depth bias/stencil AFTER bindPipeline(), so if
	 * the pipeline didn't change, those changes would be lost without this.
	 */
	void applyDynamicState();

private:

	vk::Device m_device;
	vk::CommandBuffer m_cmdBuffer;

	// Current render pass state
	vk::RenderPass m_currentRenderPass;
	uint32_t m_currentSubpass = 0;

	// Current pipeline state
	vk::Pipeline m_currentPipeline;
	vk::PipelineLayout m_currentPipelineLayout;

	// Descriptor sets
	std::array<vk::DescriptorSet, static_cast<size_t>(DescriptorSetIndex::Count)> m_boundDescriptorSets;

	// Dynamic state
	vk::Viewport m_viewport;
	vk::Rect2D m_scissor;
	bool m_scissorEnabled = false;
	float m_depthBiasConstant = 0.0f;
	float m_depthBiasSlope = 0.0f;
	uint32_t m_stencilReference = 0;
	float m_lineWidth = 1.0f;

	// Dirty flags for dynamic state
	bool m_viewportDirty = true;
	bool m_scissorDirty = true;
	bool m_depthBiasDirty = false;
	bool m_stencilRefDirty = false;
	bool m_lineWidthDirty = false;

	// Clear values
	vk::ClearColorValue m_clearColor;

	// Render state tracking (for FSO compatibility)
	gr_zbuffer_type m_zbufferMode = ZBUFFER_TYPE_NONE;
	int m_stencilMode = 0;
	bool m_cullEnabled = true;
	uint32_t m_colorAttachmentCount = 1;
	vk::SampleCountFlagBits m_currentSampleCount = vk::SampleCountFlagBits::e1;

	bool m_initialized = false;
};

// Global state tracker access
VulkanStateTracker* getStateTracker();
void setStateTracker(VulkanStateTracker* tracker);

// ========== gr_screen function pointer implementations ==========

void vulkan_zbias(int bias);
int vulkan_alpha_mask_set(int mode, float alpha);
void vulkan_set_viewport(int x, int y, int width, int height);

} // namespace vulkan
} // namespace graphics
