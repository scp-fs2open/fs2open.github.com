#include "VulkanState.h"
#include "VulkanDraw.h"

#include <algorithm>


namespace graphics::vulkan {

// Global state tracker pointer
static VulkanStateTracker* g_stateTracker = nullptr;

VulkanStateTracker* getStateTracker()
{
	Assertion(g_stateTracker != nullptr, "Vulkan StateTracker not initialized!");
	return g_stateTracker;
}

void setStateTracker(VulkanStateTracker* tracker)
{
	g_stateTracker = tracker;
}

bool VulkanStateTracker::init(vk::Device device)
{
	if (m_initialized) {
		return true;
	}

	m_device = device;

	// Initialize default viewport
	m_viewport.x = 0.0f;
	m_viewport.y = 0.0f;
	m_viewport.width = static_cast<float>(gr_screen.max_w);
	m_viewport.height = static_cast<float>(gr_screen.max_h);
	m_viewport.minDepth = 0.0f;
	m_viewport.maxDepth = 1.0f;

	// Initialize default scissor
	m_scissor.offset.x = 0;
	m_scissor.offset.y = 0;
	m_scissor.extent.width = gr_screen.max_w;
	m_scissor.extent.height = gr_screen.max_h;

	// Default render area until the first render pass is begun
	m_renderArea.width = gr_screen.max_w;
	m_renderArea.height = gr_screen.max_h;

	m_initialized = true;
	mprintf(("VulkanStateTracker: Initialized\n"));
	return true;
}

void VulkanStateTracker::shutdown()
{
	if (!m_initialized) {
		return;
	}

	m_cmdBuffer = nullptr;
	m_currentPipeline = nullptr;
	m_currentRenderPass = nullptr;

	m_initialized = false;
	mprintf(("VulkanStateTracker: Shutdown complete\n"));
}

void VulkanStateTracker::beginFrame(vk::CommandBuffer cmdBuffer)
{
	nprintf(("vulkanstate", "VulkanStateTracker::beginFrame - cmdBuffer=%p\n",
		static_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer))));

	m_cmdBuffer = cmdBuffer;

	// Reset state for new frame
	m_currentPipeline = nullptr;
	m_currentRenderPass = nullptr;

	for (auto& set : m_boundDescriptorSets) {
		set = nullptr;
	}

	// Mark all dynamic state as dirty
	m_viewportDirty = true;
	m_scissorDirty = true;
	m_depthBiasDirty = true;
	m_stencilRefDirty = true;
	m_lineWidthDirty = true;
}

void VulkanStateTracker::endFrame()
{
	nprintf(("vulkanstate", "VulkanStateTracker::endFrame - clearing cmdBuffer (was %p)\n",
		static_cast<void*>(static_cast<VkCommandBuffer>(m_cmdBuffer))));
	m_cmdBuffer = nullptr;
}

void VulkanStateTracker::setRenderPass(vk::RenderPass renderPass, uint32_t subpass)
{
	m_currentRenderPass = renderPass;
	m_currentSubpass = subpass;

	// Pipeline needs to be rebound when render pass changes
	m_currentPipeline = nullptr;

	// Invalidate cached descriptor-set bindings at every render-pass boundary.
	// This is the single recovery point for raw recorders (drawFullscreenTriangle,
	// encodeOutput, the MSAA resolve, irradiance-map generation) that bind
	// descriptor sets directly on the command buffer behind this tracker's back --
	// they always run in their own render pass, so clearing here forces the first
	// tracked draw of the NEXT pass to rebind. Because all pipelines share one
	// VkPipelineLayout, sets otherwise survive pipeline binds by layout
	// compatibility, so bindPipeline() does not need to clear them per-bind --
	// only this per-pass reset is needed while the raw recorders exist.
	for (auto& set : m_boundDescriptorSets) {
		set = nullptr;
	}

	// Re-dirty viewport/scissor at the pass boundary. NOT because Vulkan drops
	// dynamic state across render-pass instances -- it doesn't; dynamic state
	// persists for the lifetime of the command buffer. The reason is the same as
	// for the descriptor sets above: the raw recorders (drawFullscreenTriangle,
	// encode, MSAA resolve, irrmap) and mid-frame passes set viewport/scissor
	// directly on the command buffer behind this tracker, so the tracker's cached
	// values may no longer match the command buffer's. Forcing a re-apply on the
	// next tracked draw restores agreement. This can be re-evaluated if the raw
	// recorders are ever removed.
	m_viewportDirty = true;
	m_scissorDirty = true;
}

void VulkanStateTracker::invalidateExternalBindings()
{
	m_currentPipeline = nullptr;

	for (auto& set : m_boundDescriptorSets) {
		set = nullptr;
	}

	m_viewportDirty = true;
	m_scissorDirty = true;
}

vk::Rect2D VulkanStateTracker::clampToRenderArea(const vk::Rect2D& rect) const
{
	// Intersect [offset, offset+extent) with [0, renderArea)
	const int64_t reqX0 = rect.offset.x;
	const int64_t reqY0 = rect.offset.y;
	const int64_t reqX1 = reqX0 + rect.extent.width;
	const int64_t reqY1 = reqY0 + rect.extent.height;

	const int64_t x0 = std::max<int64_t>(reqX0, 0);
	const int64_t y0 = std::max<int64_t>(reqY0, 0);
	const int64_t x1 = std::min<int64_t>(reqX1, m_renderArea.width);
	const int64_t y1 = std::min<int64_t>(reqY1, m_renderArea.height);

	vk::Rect2D result;
	if (x1 <= x0 || y1 <= y0) {
		return result; // empty; caller skips the clear
	}
	result.offset.x = static_cast<int32_t>(x0);
	result.offset.y = static_cast<int32_t>(y0);
	result.extent.width = static_cast<uint32_t>(x1 - x0);
	result.extent.height = static_cast<uint32_t>(y1 - y0);
	return result;
}

void VulkanStateTracker::setViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	if (m_viewport.x != x || m_viewport.y != y ||
	    m_viewport.width != width || m_viewport.height != height ||
	    m_viewport.minDepth != minDepth || m_viewport.maxDepth != maxDepth) {
		m_viewport.x = x;
		m_viewport.y = y;
		m_viewport.width = width;
		m_viewport.height = height;
		m_viewport.minDepth = minDepth;
		m_viewport.maxDepth = maxDepth;
		m_viewportDirty = true;

		// When scissor is disabled, applyDynamicState derives the scissor rect
		// from the viewport dimensions. So a viewport change invalidates that
		// computed scissor and must trigger a re-flush.
		if (!m_scissorEnabled) {
			m_scissorDirty = true;
		}
	}
}

void VulkanStateTracker::setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	if (m_scissor.offset.x != x || m_scissor.offset.y != y ||
	    m_scissor.extent.width != width || m_scissor.extent.height != height) {
		m_scissor.offset.x = x;
		m_scissor.offset.y = y;
		m_scissor.extent.width = width;
		m_scissor.extent.height = height;
		m_scissorDirty = true;
	}
}

void VulkanStateTracker::setScissorEnabled(bool enabled)
{
	if (m_scissorEnabled != enabled) {
		m_scissorEnabled = enabled;
		m_scissorDirty = true;
	}
}

void VulkanStateTracker::setDepthBias(float constantFactor, float slopeFactor)
{
	if (m_depthBiasConstant != constantFactor || m_depthBiasSlope != slopeFactor) {
		m_depthBiasConstant = constantFactor;
		m_depthBiasSlope = slopeFactor;
		m_depthBiasDirty = true;
	}
}

void VulkanStateTracker::setStencilReference(uint32_t reference)
{
	if (m_stencilReference != reference) {
		m_stencilReference = reference;
		m_stencilRefDirty = true;
	}
}

void VulkanStateTracker::setLineWidth(float width)
{
	if (m_lineWidth != width) {
		m_lineWidth = width;
		m_lineWidthDirty = true;
	}
}

void VulkanStateTracker::bindPipeline(vk::Pipeline pipeline, vk::PipelineLayout layout)
{
	if (m_currentPipeline != pipeline && pipeline && m_cmdBuffer) {
		m_cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		m_currentPipeline = pipeline;
		m_currentPipelineLayout = layout;

		// After binding new pipeline, need to re-apply dynamic state
		applyDynamicState();

		// NOTE: descriptor sets are intentionally NOT cleared here. All graphics
		// pipelines share one VkPipelineLayout, so already-bound sets remain valid
		// across a pipeline bind (Vulkan pipeline-layout compatibility). Clearing
		// them here would force a redundant rebind of all sets on every draw with
		// a new pipeline. Raw-recorder staleness is instead recovered at
		// render-pass boundaries in setRenderPass().
	}
}

void VulkanStateTracker::bindDescriptorSet(DescriptorSetIndex setIndex, vk::DescriptorSet set,
                                            const SCP_vector<uint32_t>& dynamicOffsets)
{
	Assertion(m_cmdBuffer, "bindDescriptorSet called without active command buffer!");
	Assertion(m_currentPipelineLayout, "bindDescriptorSet called without bound pipeline layout!");
	Assertion(set, "bindDescriptorSet called with null descriptor set!");

	auto index = static_cast<uint32_t>(setIndex);

	if (m_boundDescriptorSets[index] != set) {
		m_cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			m_currentPipelineLayout,
			index,
			1, &set,
			static_cast<uint32_t>(dynamicOffsets.size()),
			dynamicOffsets.empty() ? nullptr : dynamicOffsets.data());

		m_boundDescriptorSets[index] = set;
	}
}

void VulkanStateTracker::bindVertexBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset)
{
	Assertion(m_cmdBuffer, "bindVertexBuffer called without active command buffer!");
	Assertion(buffer, "bindVertexBuffer called with null buffer!");
	m_cmdBuffer.bindVertexBuffers(binding, 1, &buffer, &offset);
}

void VulkanStateTracker::bindIndexBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::IndexType indexType)
{
	Assertion(m_cmdBuffer, "bindIndexBuffer called without active command buffer!");
	Assertion(buffer, "bindIndexBuffer called with null buffer!");
	m_cmdBuffer.bindIndexBuffer(buffer, offset, indexType);
}

void VulkanStateTracker::applyDynamicState()
{
	Assertion(m_cmdBuffer, "applyDynamicState called without active command buffer!");

	if (m_viewportDirty) {
		m_cmdBuffer.setViewport(0, 1, &m_viewport);
		m_viewportDirty = false;
	}

	if (m_scissorDirty) {
		if (m_scissorEnabled) {
			m_cmdBuffer.setScissor(0, 1, &m_scissor);
		} else {
			// Set scissor to full viewport when disabled.
			// Handle negative viewport height (VK_KHR_maintenance1 Y-flip):
			// when height < 0, the viewport covers [y+height, y] in framebuffer Y.
			vk::Rect2D fullScissor;
			float vy = m_viewport.y;
			float vh = m_viewport.height;
			if (vh < 0.0f) {
				vy = vy + vh;
				vh = -vh;
			}
			fullScissor.offset.x = static_cast<int32_t>(m_viewport.x);
			fullScissor.offset.y = static_cast<int32_t>(vy);
			fullScissor.extent.width = static_cast<uint32_t>(m_viewport.width);
			fullScissor.extent.height = static_cast<uint32_t>(vh);
			m_cmdBuffer.setScissor(0, 1, &fullScissor);
		}
		m_scissorDirty = false;
	}

	if (m_depthBiasDirty) {
		m_cmdBuffer.setDepthBias(m_depthBiasConstant, 0.0f, m_depthBiasSlope);
		m_depthBiasDirty = false;
	}

	if (m_stencilRefDirty) {
		m_cmdBuffer.setStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, m_stencilReference);
		m_stencilRefDirty = false;
	}

	if (m_lineWidthDirty) {
		m_cmdBuffer.setLineWidth(m_lineWidth);
		m_lineWidthDirty = false;
	}
}

} // namespace graphics::vulkan



namespace graphics::vulkan {

// ========== gr_screen function pointer implementations ==========

void vulkan_zbias(int bias)
{
	auto* stateTracker = getStateTracker();
	auto* drawManager = getDrawManager();

	if (bias) {
		drawManager->setDepthBiasEnabled(true);
		if (bias < 0) {
			stateTracker->setDepthBias(1.0f, static_cast<float>(-bias));
		} else {
			stateTracker->setDepthBias(0.0f, static_cast<float>(-bias));
		}
	} else {
		drawManager->setDepthBiasEnabled(false);
		stateTracker->setDepthBias(0.0f, 0.0f);
	}
}

int vulkan_alpha_mask_set(int mode, float alpha)
{
	if (mode) {
		getStateTracker()->setAlphaThreshold(alpha);
	} else {
		getStateTracker()->setAlphaThreshold(0.0f);
	}
	return mode;
}

void vulkan_set_viewport(int x, int y, int width, int height)
{
	auto* stateTracker = getStateTracker();
	if (gr_screen.rendering_to_texture == -1) {
		// Screen rendering: use negative viewport height for OpenGL-compatible Y-up NDC
		// (VK_KHR_maintenance1, core since Vulkan 1.1)
		stateTracker->setViewport(
			static_cast<float>(x),
			static_cast<float>(gr_screen.max_h - y),
			static_cast<float>(width),
			static_cast<float>(-height));
	} else {
		// RTT: standard positive viewport (RTT projection matrix handles Y-flip)
		stateTracker->setViewport(
			static_cast<float>(x), static_cast<float>(y),
			static_cast<float>(width), static_cast<float>(height));
	}
}

} // namespace graphics::vulkan

