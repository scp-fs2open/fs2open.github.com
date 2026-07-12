#include "VulkanDraw.h"

#include <algorithm>
#include <array>

#include "VulkanState.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanPostProcessing.h"
#include "VulkanDescriptorManager.h"
#include "VulkanDeletionQueue.h"
#include "VulkanMemory.h"
#include "VulkanConstants.h"
#include "gr_vulkan.h"
#include "VulkanVertexFormat.h"
#include "bmpman/bmpman.h"
#include "ddsutils/ddsutils.h"
#include "graphics/grinternal.h"
#include "graphics/material.h"
#include "graphics/matrix.h"
#include "graphics/util/primitives.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/shadows.h"
#include "lighting/lighting.h"
#include "graphics/util/UniformBuffer.h"

#define MODEL_SDR_FLAG_MODE_CPP
#include "def_files/data/effects/model_shader_flags.h"

namespace graphics::vulkan {


// Convert FSO texture addressing mode to Vulkan sampler address mode
static vk::SamplerAddressMode convertTextureAddressing(int mode)
{
	switch (mode) {
	case TMAP_ADDRESS_MIRROR:
		return vk::SamplerAddressMode::eMirroredRepeat;
	case TMAP_ADDRESS_CLAMP:
		return vk::SamplerAddressMode::eClampToEdge;
	case TMAP_ADDRESS_WRAP:
	default:
		return vk::SamplerAddressMode::eRepeat;
	}
}

// Global draw manager pointer
static VulkanDrawManager* g_drawManager = nullptr;

// ========== Transform buffer for batched submodel rendering ==========
// Per-frame sub-allocating buffer. Multiple draw lists may upload transforms
// in a single frame (e.g. space view + HUD targeting). Because Vulkan defers
// command submission until flip(), each upload must be preserved — we append
// rather than overwrite, and bind the SSBO with the per-upload byte offset.

// SSBO descriptor offsets must be aligned to minStorageBufferOffsetAlignment.
// The Vulkan spec guarantees this value is <= 256, so 256 is always safe.
static constexpr size_t SSBO_OFFSET_ALIGNMENT = 256;

struct TransformBufferState {
	vk::Buffer buffer;
	VulkanAllocation allocation;
	size_t capacity = 0;         // allocated bytes
	size_t writeOffset = 0;      // append cursor (resets each frame)
	size_t lastUploadOffset = 0; // byte offset of most recent upload
	size_t lastUploadSize = 0;   // byte size of most recent upload
};
static std::array<TransformBufferState, MAX_FRAMES_IN_FLIGHT> g_transformBuffers;
static uint32_t g_lastTransformWriteFrame = UINT32_MAX;

void vulkan_update_transform_buffer(void* data, size_t size)
{
	if (!data || size == 0) {
		return;
	}

	auto* descManager = getDescriptorManager();
	uint32_t frameIdx = descManager->getCurrentFrame();
	auto& tb = g_transformBuffers[frameIdx];

	// Reset write cursor on first call of each frame
	if (g_lastTransformWriteFrame != frameIdx) {
		tb.writeOffset = 0;
		g_lastTransformWriteFrame = frameIdx;
	}

	// Align the write offset for SSBO descriptor binding
	size_t alignedOffset = (tb.writeOffset + SSBO_OFFSET_ALIGNMENT - 1) & ~(SSBO_OFFSET_ALIGNMENT - 1);
	size_t needed = alignedOffset + size;

	auto* memManager = getMemoryManager();

	// Resize if needed, preserving data already written this frame
	if (needed > tb.capacity) {
		size_t newCapacity = std::max(needed * 2, static_cast<size_t>(4096));

		auto* bufferManager = getBufferManager();
		vk::Device device = bufferManager->getDevice();

		vk::BufferCreateInfo bufferInfo;
		bufferInfo.size = static_cast<vk::DeviceSize>(newCapacity);
		bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		vk::Buffer newBuffer;
		VulkanAllocation newAllocation;

		try {
			newBuffer = device.createBuffer(bufferInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "vulkan_update_transform_buffer: Failed to create buffer: %s\n", e.what()));
			return;
		}

		Verify(memManager->allocateBufferMemory(newBuffer, MemoryUsage::CpuToGpu, newAllocation));

		// Copy data already written this frame from old buffer
		if (tb.buffer && tb.writeOffset > 0) {
			void* oldMapped = memManager->mapMemory(tb.allocation);
			void* newMapped = memManager->mapMemory(newAllocation);
			Verify(oldMapped);
			Verify(newMapped);
			memcpy(newMapped, oldMapped, tb.writeOffset);
			memManager->unmapMemory(tb.allocation);
			memManager->unmapMemory(newAllocation);
		}

		// Defer destruction of old buffer
		if (tb.buffer) {
			auto* deletionQueue = getDeletionQueue();
			deletionQueue->queueBuffer(tb.buffer, tb.allocation);
		}

		tb.buffer = newBuffer;
		tb.allocation = newAllocation;
		tb.capacity = newCapacity;
	}

	// Upload new data at the aligned offset
	void* mapped = memManager->mapMemory(tb.allocation);
	Verify(mapped);
	memcpy(static_cast<char*>(mapped) + alignedOffset, data, size);
	memManager->flushMemory(tb.allocation, alignedOffset, size);
	memManager->unmapMemory(tb.allocation);

	tb.lastUploadOffset = alignedOffset;
	tb.lastUploadSize = size;
	tb.writeOffset = alignedOffset + size;
}

VulkanDrawManager* getDrawManager()
{
	Assertion(g_drawManager != nullptr, "Vulkan DrawManager not initialized!");
	return g_drawManager;
}

void setDrawManager(VulkanDrawManager* manager)
{
	g_drawManager = manager;
}

bool VulkanDrawManager::init(vk::Device device)
{
	if (m_initialized) {
		return true;
	}

	m_device = device;

	initSphereBuffers();

	m_initialized = true;
	nprintf(("vulkan", "VulkanDrawManager: Initialized\n"));
	return true;
}

void VulkanDrawManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Destroy transform SSBO buffers (static globals, not tracked by deletion queue)
	auto* bufferManager = getBufferManager();
	auto* memManager = getMemoryManager();
	if (bufferManager && memManager) {
		vk::Device device = bufferManager->getDevice();
		for (auto& tb : g_transformBuffers) {
			if (tb.buffer) {
				device.destroyBuffer(tb.buffer);
				memManager->freeAllocation(tb.allocation);
				tb.buffer = nullptr;
				tb.capacity = 0;
				tb.writeOffset = 0;
			}
		}
	}

	shutdownSphereBuffers();

	m_initialized = false;
	nprintf(("vulkan", "VulkanDrawManager: Shutdown complete\n"));
}

void VulkanDrawManager::clear()
{
	(void)this;
	auto* stateTracker = getStateTracker();

	// Use the current clip/scissor region for clearing, matching OpenGL behavior.
	// In OpenGL, glClear() respects the scissor test - if a clip region is set,
	// only that region is cleared. Without this, HUD code that does
	// gr_set_clip(panel) + gr_clear() would wipe the entire screen in Vulkan.
	vk::ClearAttachment clearAttachment;
	clearAttachment.aspectMask = vk::ImageAspectFlagBits::eColor;
	clearAttachment.colorAttachment = 0;
	clearAttachment.clearValue.color = graphics::vulkan::VulkanStateTracker::getClearColor();

	vk::ClearRect clearRect;
	if (stateTracker->isScissorEnabled()) {
		// Respect the current clip region (matches OpenGL scissor behavior)
		clearRect.rect.offset = vk::Offset2D(gr_screen.offset_x + gr_screen.clip_left,
		                                      gr_screen.offset_y + gr_screen.clip_top);
		clearRect.rect.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.clip_width),
		                                      static_cast<uint32_t>(gr_screen.clip_height));
	} else {
		clearRect.rect.offset = vk::Offset2D(0, 0);
		clearRect.rect.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.max_w),
		                                      static_cast<uint32_t>(gr_screen.max_h));
	}
	// gr_screen dimensions can exceed the active render pass when rendering to a
	// smaller off-screen target; vkCmdClearAttachments requires rects inside the
	// render area.
	clearRect.rect = stateTracker->clampToRenderArea(clearRect.rect);
	if (clearRect.rect.extent.width == 0 || clearRect.rect.extent.height == 0) {
		return;
	}
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;

	auto cmdBuffer = stateTracker->getCommandBuffer();
	cmdBuffer.clearAttachments(1, &clearAttachment, 1, &clearRect);
}

void VulkanDrawManager::setClearColor(int r, int g, int b)
{
	// Also update gr_screen for compatibility
	gr_screen.current_clear_color.red = static_cast<ubyte>(r);
	gr_screen.current_clear_color.green = static_cast<ubyte>(g);
	gr_screen.current_clear_color.blue = static_cast<ubyte>(b);
	gr_screen.current_clear_color.alpha = 255;
}

void VulkanDrawManager::setClip(int x, int y, int w, int h, int resize_mode)
{
	(void)this;
	auto* stateTracker = getStateTracker();

	// Clamp values
	x = std::max(x, 0);
	y = std::max(y, 0);

	int to_resize = (resize_mode != GR_RESIZE_NONE && resize_mode != GR_RESIZE_REPLACE &&
	                 (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)));

	int max_w = (to_resize) ? gr_screen.max_w_unscaled : gr_screen.max_w;
	int max_h = (to_resize) ? gr_screen.max_h_unscaled : gr_screen.max_h;

	if ((gr_screen.rendering_to_texture != -1) && to_resize) {
		gr_unsize_screen_pos(&max_w, &max_h);
	}

	if (resize_mode != GR_RESIZE_REPLACE) {
		if (x >= max_w) x = max_w - 1;
		if (y >= max_h) y = max_h - 1;
		if (x + w > max_w) w = max_w - x;
		if (y + h > max_h) h = max_h - y;
		w = std::min(w, max_w);
		h = std::min(h, max_h);
	}

	// Store unscaled values
	gr_screen.offset_x_unscaled = x;
	gr_screen.offset_y_unscaled = y;
	gr_screen.clip_left_unscaled = 0;
	gr_screen.clip_right_unscaled = w - 1;
	gr_screen.clip_top_unscaled = 0;
	gr_screen.clip_bottom_unscaled = h - 1;
	gr_screen.clip_width_unscaled = w;
	gr_screen.clip_height_unscaled = h;

	if (to_resize) {
		gr_resize_screen_pos(&x, &y, &w, &h, resize_mode);
	} else {
		gr_unsize_screen_pos(&gr_screen.offset_x_unscaled, &gr_screen.offset_y_unscaled);
		gr_unsize_screen_pos(&gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled);
		gr_unsize_screen_pos(&gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled);
	}

	// Update gr_screen clip state (scaled values)
	gr_screen.offset_x = x;
	gr_screen.offset_y = y;
	gr_screen.clip_left = 0;
	gr_screen.clip_top = 0;
	gr_screen.clip_right = w - 1;
	gr_screen.clip_bottom = h - 1;
	gr_screen.clip_width = w;
	gr_screen.clip_height = h;

	gr_screen.clip_aspect = i2fl(w) / i2fl(h);
	gr_screen.clip_center_x = (gr_screen.clip_left + gr_screen.clip_right) * 0.5f;
	gr_screen.clip_center_y = (gr_screen.clip_top + gr_screen.clip_bottom) * 0.5f;

	// Check if full screen (disable scissor)
	if ((x == 0) && (y == 0) && (w == max_w) && (h == max_h)) {
		stateTracker->setScissorEnabled(false);
		return;
	}

	// Enable scissor test
	stateTracker->setScissorEnabled(true);
	stateTracker->setScissor(x, y, static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

void VulkanDrawManager::resetClip()
{
	(void)this;
	auto* stateTracker = getStateTracker();

	int max_w = gr_screen.max_w;
	int max_h = gr_screen.max_h;

	gr_screen.offset_x = gr_screen.offset_x_unscaled = 0;
	gr_screen.offset_y = gr_screen.offset_y_unscaled = 0;
	gr_screen.clip_left = gr_screen.clip_left_unscaled = 0;
	gr_screen.clip_top = gr_screen.clip_top_unscaled = 0;
	gr_screen.clip_right = gr_screen.clip_right_unscaled = max_w - 1;
	gr_screen.clip_bottom = gr_screen.clip_bottom_unscaled = max_h - 1;
	gr_screen.clip_width = gr_screen.clip_width_unscaled = max_w;
	gr_screen.clip_height = gr_screen.clip_height_unscaled = max_h;

	if (gr_screen.custom_size) {
		gr_unsize_screen_pos(&gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled);
		gr_unsize_screen_pos(&gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled);
	}

	gr_screen.clip_aspect = i2fl(max_w) / i2fl(max_h);
	gr_screen.clip_center_x = (gr_screen.clip_left + gr_screen.clip_right) * 0.5f;
	gr_screen.clip_center_y = (gr_screen.clip_top + gr_screen.clip_bottom) * 0.5f;

	stateTracker->setScissorEnabled(false);
}

int VulkanDrawManager::zbufferGet() const
{
	if (!gr_global_zbuffering) {
		return GR_ZBUFF_NONE;
	}
	return m_zbufferMode;
}

int VulkanDrawManager::zbufferSet(int mode)
{
	auto* stateTracker = getStateTracker();

	int prev = m_zbufferMode;
	m_zbufferMode = mode;

	// Update FSO global state
	if (mode == GR_ZBUFF_NONE) {
		gr_zbuffering = 0;
	} else {
		gr_zbuffering = 1;
	}
	gr_zbuffering_mode = mode;

	gr_zbuffer_type zbufType;
	switch (mode) {
	case GR_ZBUFF_NONE:
		zbufType = ZBUFFER_TYPE_NONE;
		break;
	case GR_ZBUFF_READ:
		zbufType = ZBUFFER_TYPE_READ;
		break;
	case GR_ZBUFF_WRITE:
		zbufType = ZBUFFER_TYPE_WRITE;
		break;
	case GR_ZBUFF_FULL:
	default:
		zbufType = ZBUFFER_TYPE_FULL;
		break;
	}
	stateTracker->setZBufferMode(zbufType);

	return prev;
}

void VulkanDrawManager::zbufferClear(int mode)
{
	auto* stateTracker = getStateTracker();

	if (mode) {
		// Enable zbuffering and clear
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;
		m_zbufferMode = GR_ZBUFF_FULL;
		stateTracker->setZBufferMode(ZBUFFER_TYPE_FULL);

		// Clear depth buffer (clamped: gr_screen can exceed a smaller render target)
		vk::ClearAttachment clearAttachment;
		clearAttachment.aspectMask = vk::ImageAspectFlagBits::eDepth;
		clearAttachment.clearValue.depthStencil.depth = 1.0f;
		clearAttachment.clearValue.depthStencil.stencil = 0;

		vk::ClearRect clearRect;
		clearRect.rect.offset = vk::Offset2D(0, 0);
		clearRect.rect.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.max_w),
		                                      static_cast<uint32_t>(gr_screen.max_h));
		clearRect.rect = stateTracker->clampToRenderArea(clearRect.rect);
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;

		if (clearRect.rect.extent.width > 0 && clearRect.rect.extent.height > 0) {
			stateTracker->getCommandBuffer().clearAttachments(1, &clearAttachment, 1, &clearRect);
		}
	} else {
		// Disable zbuffering
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;
		m_zbufferMode = GR_ZBUFF_NONE;
		stateTracker->setZBufferMode(ZBUFFER_TYPE_NONE);
	}
}

int VulkanDrawManager::stencilSet(int mode)
{
	auto* stateTracker = getStateTracker();

	int prev = m_stencilMode;
	m_stencilMode = mode;
	gr_stencil_mode = mode;

	stateTracker->setStencilMode(mode);

	// Set stencil reference based on mode
	if (mode == GR_STENCIL_READ || mode == GR_STENCIL_WRITE) {
		stateTracker->setStencilReference(1);
	} else {
		stateTracker->setStencilReference(0);
	}

	return prev;
}

void VulkanDrawManager::stencilClear()
{
	(void)this;
	auto* stateTracker = getStateTracker();

	// Clear stencil buffer (clamped: gr_screen can exceed a smaller render target)
	vk::ClearAttachment clearAttachment;
	clearAttachment.aspectMask = vk::ImageAspectFlagBits::eStencil;
	clearAttachment.clearValue.depthStencil.depth = 1.0f;
	clearAttachment.clearValue.depthStencil.stencil = 0;

	vk::ClearRect clearRect;
	clearRect.rect.offset = vk::Offset2D(0, 0);
	clearRect.rect.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.max_w),
	                                      static_cast<uint32_t>(gr_screen.max_h));
	clearRect.rect = stateTracker->clampToRenderArea(clearRect.rect);
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;

	if (clearRect.rect.extent.width > 0 && clearRect.rect.extent.height > 0) {
		stateTracker->getCommandBuffer().clearAttachments(1, &clearAttachment, 1, &clearRect);
	}
}

int VulkanDrawManager::setCull(int cull)
{
	auto* stateTracker = getStateTracker();

	int prev = m_cullEnabled ? 1 : 0;
	m_cullEnabled = (cull != 0);

	stateTracker->setCullMode(m_cullEnabled);

	return prev;
}

void VulkanDrawManager::renderPrimitivesCommon(material* material_info, primitive_type prim_type,
                                                vertex_layout* layout, int offset, int n_verts,
                                                gr_buffer_handle buffer_handle, size_t buffer_offset,
                                                int* statCounter)
{
	GR_DEBUG_SCOPE("Render primitives");

	if (!material_info || !layout || n_verts <= 0) {
		return;
	}

	if (statCounter != nullptr) {
		(*statCounter)++;
	}

	// Apply material state and bind pipeline
	if (!applyMaterial(material_info, prim_type, layout)) {
		return;
	}

	// Bind vertex buffer and issue the draw call
	bindVertexBuffer(buffer_handle, buffer_offset);
	draw(prim_type, offset, n_verts);
}

void VulkanDrawManager::renderPrimitives(material* material_info, primitive_type prim_type,
                                          vertex_layout* layout, int offset, int n_verts,
                                          gr_buffer_handle buffer_handle, size_t buffer_offset)
{
	renderPrimitivesCommon(material_info, prim_type, layout, offset, n_verts,
		buffer_handle, buffer_offset, &m_frameStats.renderPrimitiveCalls);
}

void VulkanDrawManager::renderPrimitivesBatched(batched_bitmap_material* material_info,
                                                 primitive_type prim_type, vertex_layout* layout,
                                                 int offset, int n_verts, gr_buffer_handle buffer_handle)
{
	GR_DEBUG_SCOPE("Render batched primitives");

	renderPrimitivesCommon(material_info, prim_type, layout, offset, n_verts,
		buffer_handle, 0, &m_frameStats.renderBatchedCalls);
}

void VulkanDrawManager::renderPrimitivesParticle(particle_material* material_info,
                                                  primitive_type prim_type, vertex_layout* layout,
                                                  int offset, int n_verts, gr_buffer_handle buffer_handle)
{
	renderPrimitivesCommon(material_info, prim_type, layout, offset, n_verts,
		buffer_handle, 0, &m_frameStats.renderParticleCalls);
}

void VulkanDrawManager::renderPrimitivesDistortion(distortion_material* material_info,
                                                    primitive_type prim_type, vertex_layout* layout,
                                                    int offset, int n_verts, gr_buffer_handle buffer_handle)
{
	// Distortion intentionally tracks no dedicated frame stat counter.
	renderPrimitivesCommon(material_info, prim_type, layout, offset, n_verts,
		buffer_handle, 0, nullptr);
}

void VulkanDrawManager::renderMovie(movie_material* material_info, primitive_type prim_type,
                                     vertex_layout* layout, int n_verts, gr_buffer_handle buffer_handle,
                                     size_t buffer_offset)
{
	GR_DEBUG_SCOPE("Render movie frame");

	renderPrimitivesCommon(material_info, prim_type, layout, 0, n_verts,
		buffer_handle, buffer_offset, &m_frameStats.renderMovieCalls);
}

void VulkanDrawManager::renderNanoVG(nanovg_material* material_info, primitive_type prim_type,
                                      vertex_layout* layout, int offset, int n_verts,
                                      gr_buffer_handle buffer_handle)
{
	GR_DEBUG_SCOPE("Render NanoVG primitives");

	renderPrimitivesCommon(material_info, prim_type, layout, offset, n_verts,
		buffer_handle, 0, &m_frameStats.renderNanoVGCalls);
}

void VulkanDrawManager::renderRocketPrimitives(interface_material* material_info,
                                                primitive_type prim_type, vertex_layout* layout,
                                                int n_indices, gr_buffer_handle vertex_buffer,
                                                gr_buffer_handle index_buffer)
{
	if (!material_info || !layout || n_indices <= 0) {
		return;
	}

	GR_DEBUG_SCOPE("Render rocket ui primitives");

	m_frameStats.renderRocketCalls++;

	if (!applyMaterial(material_info, prim_type, layout)) {
		return;
	}
	bindVertexBuffer(vertex_buffer, 0);
	bindIndexBuffer(index_buffer);
	drawIndexed(prim_type, n_indices, 0, 0);
}

void VulkanDrawManager::renderModel(model_material* material_info, indexed_vertex_source* vert_source,
                                     vertex_buffer* bufferp, size_t texi)
{
	if (!material_info || !vert_source || !bufferp) {
		return;
	}

	m_frameStats.renderModelCalls++;

	// Validate buffers
	if (!vert_source->Vbuffer_handle.isValid() || !vert_source->Ibuffer_handle.isValid()) {
		nprintf(("Vulkan", "VulkanDrawManager: renderModel called with invalid buffer handles\n"));
		return;
	}

	if (texi >= bufferp->tex_buf.size()) {
		nprintf(("Vulkan", "VulkanDrawManager: renderModel texi out of range\n"));
		return;
	}

	auto* stateTracker = getStateTracker();

	// Get buffer data for this texture/draw
	buffer_data* datap = &bufferp->tex_buf[texi];

	if (datap->n_verts == 0) {
		return;  // Nothing to draw
	}

	// Apply model material state and bind pipeline
	// Model rendering always uses triangles
	if (!applyMaterial(material_info, PRIM_TYPE_TRIS, &bufferp->layout)) {
		return;
	}

	// Bind vertex buffer with the model's vertex offset
	auto* bufferManager = getBufferManager();

	vk::Buffer vbuffer = bufferManager->getVkBuffer(vert_source->Vbuffer_handle);
	vk::Buffer ibuffer = bufferManager->getVkBuffer(vert_source->Ibuffer_handle);

	Assertion(vbuffer, "VulkanDrawManager::renderModel got null vertex buffer from valid handle!");
	Assertion(ibuffer, "VulkanDrawManager::renderModel got null index buffer from valid handle!");

	// Bind vertex buffer at offset 0 (start of heap buffer), matching OpenGL behavior.
	// The Base_vertex_offset in drawIndexed handles the heap allocation offset.
	stateTracker->bindVertexBuffer(0, vbuffer, 0);

	// Determine index type based on VB_FLAG_LARGE_INDEX flag
	vk::IndexType indexType = (datap->flags & VB_FLAG_LARGE_INDEX) ?
	                          vk::IndexType::eUint32 : vk::IndexType::eUint16;

	// Bind index buffer at the model's heap allocation offset.
	// The firstIndex (from datap->index_offset) handles per-mesh offset within the model.
	stateTracker->bindIndexBuffer(ibuffer, static_cast<vk::DeviceSize>(vert_source->Index_offset), indexType);

	// Base vertex offset: accounts for heap allocation position + per-mesh vertex offset.
	// This matches OpenGL's glDrawElementsBaseVertex usage.
	auto baseVertex = static_cast<int32_t>(vert_source->Base_vertex_offset + bufferp->vertex_num_offset);

	// Calculate first index
	// The index_offset in buffer_data is in bytes, need to convert to index count
	uint32_t firstIndex;
	if (indexType == vk::IndexType::eUint32) {
		firstIndex = static_cast<uint32_t>(datap->index_offset / sizeof(uint32_t));
	} else {
		firstIndex = static_cast<uint32_t>(datap->index_offset / sizeof(uint16_t));
	}

	// Issue indexed draw call
	m_frameStats.drawIndexedCalls++;
	m_frameStats.totalIndices += static_cast<int>(datap->n_verts);

	// Flush any dirty dynamic state before draw
	stateTracker->applyDynamicState();

	auto cmdBuffer = stateTracker->getCommandBuffer();
	cmdBuffer.drawIndexed(
		static_cast<uint32_t>(datap->n_verts),  // index count
		1,                                       // instance count
		firstIndex,                              // first index
		baseVertex,                              // vertex offset
		0                                        // first instance
	);
}

void VulkanDrawManager::renderShadowDraw(gr_buffer_handle ubo_handle, size_t ubo_offset, size_t ubo_size,
                                          vertex_buffer* buffer, indexed_vertex_source* vert_src, size_t texi) const
{
	if (!buffer || !vert_src) {
		return;
	}

	if (texi >= buffer->tex_buf.size()) {
		return;
	}

	buffer_data* datap = &buffer->tex_buf[texi];
	if (datap->n_verts == 0) {
		return;
	}

	auto* stateTracker = getStateTracker();
	auto* pipelineManager = getPipelineManager();
	auto* descManager = getDescriptorManager();
	auto* bufferManager = getBufferManager();

	int shaderHandle = gr_maybe_create_shader(SDR_TYPE_SHADOW_MAP_GEN,
		gr_is_capable(gr_capability::CAPABILITY_FAST_SHADOWS) ? 0 : SDR_FLAG_SHADOW_FALLBACK);
	auto* shaderManager = getShaderManager();
	const VulkanShaderModule* shaderModule = shaderHandle >= 0 ? shaderManager->getShaderByHandle(shaderHandle) : nullptr;
	if (!shaderModule) {
		return;
	}

	PipelineConfig config;
	config.shaderType = shaderModule->type;
	config.shaderFlags = shaderModule->flags;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_FULL;
	config.depthWriteEnabled = true;
	config.cullEnabled = true;
	config.frontFaceCW = true;
	config.blendMode = ALPHA_BLEND_NONE;
	config.colorWriteMask = {false, false, false, false};
	config.depthBiasEnabled = true;
	config.renderPass = stateTracker->getCurrentRenderPass();
	config.colorAttachmentCount = stateTracker->getColorAttachmentCount();
	config.sampleCount = stateTracker->getCurrentSampleCount();

	if (!config.renderPass) {
		return;
	}

	vk::Pipeline pipeline = pipelineManager->getPipeline(config, buffer->layout);
	if (!pipeline) {
		nprintf(("vulkan", "VulkanDrawManager::renderShadowDraw: Failed to get pipeline!\n"));
		return;
	}

	stateTracker->bindPipeline(pipeline, pipelineManager->getPipelineLayout());
	stateTracker->setDepthBias(-1024.0f, 0.0f);

	// Bind descriptor sets: shadowMapData (per-draw) and shadowCascadeParams
	// (bound separately, per-frame, via shadow_cascade_params_bind) both live in
	// the fixed 3-tier layout alongside the batched-submodel transform buffer;
	// no material textures are needed for depth-only rendering.
	{
		DescriptorWriter writer;
		writer.reset(descManager->getDevice(), descManager->getFallbacks());

		vk::DescriptorSet globalSet = descManager->allocateFrameSet(DescriptorSetIndex::Global);
		Verify(globalSet);
		writer.writeSet(globalSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Global));
		{
			const auto& pending = getPendingUniformBinding(static_cast<size_t>(uniform_block_type::ShadowCascadeParams));
			if (pending.valid) {
				vk::Buffer buf = bufferManager->getVkBuffer(pending.bufferHandle);
				if (buf) {
					writer.setBuffer(GlobalBinding::ShadowCascadeParams, {buf, pending.offset, pending.size});
				}
			}
		}

		vk::DescriptorSet materialSet = descManager->allocateFrameSet(DescriptorSetIndex::Material);
		Verify(materialSet);
		writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
		{
			vk::Buffer buf = bufferManager->getVkBuffer(ubo_handle);
			if (buf) {
				writer.setBuffer(MaterialBinding::ShadowMapData,
					{buf, static_cast<vk::DeviceSize>(ubo_offset), static_cast<vk::DeviceSize>(ubo_size)});
			}
		}
		{
			uint32_t tfIdx = descManager->getCurrentFrame();
			auto& tf = g_transformBuffers[tfIdx];
			if (tf.buffer && tf.lastUploadSize > 0) {
				writer.setBuffer(MaterialBinding::TransformSSBO, {tf.buffer,
				                 static_cast<vk::DeviceSize>(tf.lastUploadOffset),
				                 static_cast<vk::DeviceSize>(tf.lastUploadSize)});
			}
		}

		vk::DescriptorSet perDrawSet = descManager->allocateFrameSet(DescriptorSetIndex::PerDraw);
		Verify(perDrawSet);
		writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
		writer.flush();

		stateTracker->bindDescriptorSet(DescriptorSetIndex::Global, globalSet);
		stateTracker->bindDescriptorSet(DescriptorSetIndex::Material, materialSet);
		stateTracker->bindDescriptorSet(DescriptorSetIndex::PerDraw, perDrawSet);
	}

	vk::Buffer vbuffer = bufferManager->getVkBuffer(vert_src->Vbuffer_handle);
	vk::Buffer ibuffer = bufferManager->getVkBuffer(vert_src->Ibuffer_handle);
	if (!vbuffer || !ibuffer) {
		return;
	}

	stateTracker->bindVertexBuffer(0, vbuffer, 0);

	vk::IndexType indexType = (datap->flags & VB_FLAG_LARGE_INDEX) ?
	                          vk::IndexType::eUint32 : vk::IndexType::eUint16;
	stateTracker->bindIndexBuffer(ibuffer, static_cast<vk::DeviceSize>(vert_src->Index_offset), indexType);

	auto baseVertex = static_cast<int32_t>(vert_src->Base_vertex_offset + buffer->vertex_num_offset);

	uint32_t firstIndex;
	if (indexType == vk::IndexType::eUint32) {
		firstIndex = static_cast<uint32_t>(datap->index_offset / sizeof(uint32_t));
	} else {
		firstIndex = static_cast<uint32_t>(datap->index_offset / sizeof(uint16_t));
	}

	stateTracker->applyDynamicState();

	// One instance per active shadow cascade, routed via gl_InstanceIndex → gl_Layer
	// in the vertex shader (CAPABILITY_FAST_SHADOWS path -- Vulkan always writes
	// gl_Layer directly, no geometry-shader fallback needed).
	uint32_t instanceCount = static_cast<uint32_t>(std::max(Shadow_cascade_count, 1));

	auto cmdBuffer = stateTracker->getCommandBuffer();
	cmdBuffer.drawIndexed(
		static_cast<uint32_t>(datap->n_verts),
		instanceCount,
		firstIndex,
		baseVertex,
		0
	);
}

void VulkanDrawManager::setFillMode(int mode)
{
	m_fillMode = mode;
}

int VulkanDrawManager::setColorBuffer(int mode)
{
	int prev = m_colorBufferEnabled ? 1 : 0;
	m_colorBufferEnabled = (mode != 0);
	return prev;
}

void VulkanDrawManager::setTextureAddressing(int mode)
{
	m_textureAddressing = mode;
}

void VulkanDrawManager::setDepthBiasEnabled(bool enabled)
{
	m_depthBiasEnabled = enabled;
}

void VulkanDrawManager::setDepthTextureOverride(vk::DescriptorImageInfo info)
{
	m_depthTextureInfo = info;
}

void VulkanDrawManager::clearDepthTextureOverride()
{
	m_depthTextureInfo = vk::DescriptorImageInfo();
}

void VulkanDrawManager::setSceneColorOverride(vk::DescriptorImageInfo info)
{
	m_sceneColorInfo = info;
}

void VulkanDrawManager::setDistMapOverride(vk::DescriptorImageInfo info)
{
	m_distMapInfo = info;
}

void VulkanDrawManager::clearDistortionOverrides()
{
	m_sceneColorInfo = vk::DescriptorImageInfo();
	m_distMapInfo    = vk::DescriptorImageInfo();
}

void VulkanDrawManager::onResize()
{
	// The cached override infos can reference views destroyed by the swap chain
	// / post-processor resize; drop them so applyMaterial() falls back until the
	// next frame sets fresh ones.
	clearDepthTextureOverride();
	clearDistortionOverrides();
}

void VulkanDrawManager::clearStates()
{
	auto* stateTracker = getStateTracker();

	// Match OpenGL's gr_opengl_clear_states() behavior:
	//   gr_zbias(0), gr_zbuffer_set(ZBUFFER_TYPE_READ), gr_set_cull(0),
	//   gr_set_fill_mode(GR_FILL_MODE_SOLID)
	m_zbufferMode = GR_ZBUFF_READ;
	m_stencilMode = GR_STENCIL_NONE;
	m_cullEnabled = false;
	m_fillMode = GR_FILL_MODE_SOLID;
	m_colorBufferEnabled = true;
	m_textureAddressing = TMAP_ADDRESS_WRAP;
	m_depthBiasEnabled = false;

	gr_zbuffering = 1;
	gr_zbuffering_mode = GR_ZBUFF_READ;
	gr_global_zbuffering = 1;
	gr_stencil_mode = GR_STENCIL_NONE;

	stateTracker->setZBufferMode(ZBUFFER_TYPE_READ);
	stateTracker->setStencilMode(GR_STENCIL_NONE);
	stateTracker->setCullMode(false);
	stateTracker->setScissorEnabled(false);
	stateTracker->setDepthBias(0.0f, 0.0f);
	stateTracker->setLineWidth(1.0f);

	// Clear pending uniform bindings
	clearPendingUniformBindings();

	// NOTE: Do NOT call resetClip() here. OpenGL's gr_opengl_clear_states() does
	// not reset the clip region, and callers (e.g. model_render_immediate) rely on
	// the clip/offset state surviving through clear_states for subsequent 2D draws.
}

void VulkanDrawManager::setPendingUniformBinding(uniform_block_type blockType, gr_buffer_handle bufferHandle,
                                                  vk::DeviceSize offset, vk::DeviceSize size)
{
	auto index = static_cast<size_t>(blockType);
	if (index >= NUM_UNIFORM_BLOCK_TYPES) {
		return;
	}

	m_pendingUniformBindings[index].bufferHandle = bufferHandle;
	m_pendingUniformBindings[index].offset = offset;
	m_pendingUniformBindings[index].size = size;
	m_pendingUniformBindings[index].valid = bufferHandle.isValid();

	// The memoized Global (Set 0) descriptor set binds these three block types; a
	// change to any of them must force a rebuild (B1).
	if (blockType == uniform_block_type::Lights ||
	    blockType == uniform_block_type::DeferredGlobals ||
	    blockType == uniform_block_type::ShadowCascadeParams) {
		m_globalSetDirty = true;
	}
}

void VulkanDrawManager::clearPendingUniformBindings()
{
	for (auto& binding : m_pendingUniformBindings) {
		binding.valid = false;
		binding.bufferHandle = gr_buffer_handle();
		binding.offset = 0;
		binding.size = 0;
	}
}

void VulkanDrawManager::resetFrameStats()
{
	m_frameStats = {};

	// Per-frame reset of the memoized descriptor sets. Called from setupFrame()
	// AFTER the descriptor manager reset its frame pool, so the cached
	// VkDescriptorSets have just been freed -- drop them and force a rebuild on
	// the frame's first applyMaterial().
	m_cachedGlobalSet = nullptr;
	m_globalSetDirty = true;
	m_cachedMaterialSet = nullptr;
	m_cachedMaterialValid = false;
	m_cachedPerDrawSet = nullptr;
	m_cachedPerDrawValid = false;
}

void VulkanDrawManager::printFrameStats()
{
	// Print summary every frame for the first 200 frames, then every 60 frames
	bool shouldPrint = (m_frameStatsFrameNum < 200) || (m_frameStatsFrameNum % 60 == 0);

	if (shouldPrint) {
		nprintf(("vulkanframe", "FRAME %d STATS: draws=%d indexed=%d verts=%d idxs=%d | applyMat=%d/%d fails | noPipeline=%d sdrNeg1=%d\n",
			m_frameStatsFrameNum,
			m_frameStats.drawCalls,
			m_frameStats.drawIndexedCalls,
			m_frameStats.totalVertices,
			m_frameStats.totalIndices,
			m_frameStats.applyMaterialFailures,
			m_frameStats.applyMaterialCalls,
			m_frameStats.noPipelineSkips,
			m_frameStats.shaderHandleNeg1));
		nprintf(("vulkanframe", "  CALLS: prim=%d batch=%d model=%d particle=%d nanovg=%d rocket=%d movie=%d\n",
			m_frameStats.renderPrimitiveCalls,
			m_frameStats.renderBatchedCalls,
			m_frameStats.renderModelCalls,
			m_frameStats.renderParticleCalls,
			m_frameStats.renderNanoVGCalls,
			m_frameStats.renderRocketCalls,
			m_frameStats.renderMovieCalls));
		// Descriptor/pipeline pressure: baseline for the descriptor-reuse work and
		// regression canary for pipeline-cache misses (unexpected growth = new
		// pipeline permutations being created mid-mission).
		auto* descMgr = getDescriptorManager();
		auto* pipeMgr = getPipelineManager();
		nprintf(("vulkanframe", "  DESC: sets=%u writes=%u | pipelines total=%zu\n",
			descMgr->getSetsAllocatedThisFrame(),
			descMgr->getWritesThisFrame(),
			pipeMgr->getPipelineCount()));
	}

	m_frameStatsFrameNum++;
}


PipelineConfig VulkanDrawManager::buildPipelineConfig(material* mat, primitive_type prim_type) const
{
	PipelineConfig config;

	// Get shader info from material
	int shaderHandle = mat->get_shader_handle();
	auto* shaderManager = getShaderManager();
	if (shaderHandle >= 0) {
		const auto* shaderModule = shaderManager->getShaderByHandle(shaderHandle);
		if (shaderModule) {
			config.shaderType = shaderModule->type;
			config.shaderFlags = shaderModule->flags;
		}
	}

	// Primitive type
	config.primitiveType = prim_type;

	// Depth mode
	config.depthMode = mat->get_depth_mode();

	// Blend mode
	config.blendMode = mat->get_blend_mode();

	// Cull mode
	config.cullEnabled = mat->get_cull_mode();

	// Fill mode
	config.fillMode = mat->get_fill_mode();

	// Front face winding: match OpenGL which defaults to CCW and only switches to CW
	// for model rendering (opengl_tnl_set_model_material sets GL_CW).
	config.frontFaceCW = (config.shaderType == SDR_TYPE_MODEL);

	// Depth write
	config.depthWriteEnabled = (config.depthMode == ZBUFFER_TYPE_FULL ||
	                             config.depthMode == ZBUFFER_TYPE_WRITE);

	// Stencil state
	config.stencilEnabled = mat->is_stencil_enabled();
	if (config.stencilEnabled) {
		config.stencilFunc = mat->get_stencil_func().compare;
		config.stencilMask = mat->get_stencil_func().mask;
		config.frontStencilOp = mat->get_front_stencil_op();
		config.backStencilOp = mat->get_back_stencil_op();
	}

	// Color write mask
	config.colorWriteMask = mat->get_color_mask();

	// Override color write mask if color buffer writes are disabled
	if (!m_colorBufferEnabled) {
		config.colorWriteMask = {false, false, false, false};
	}

	// Fill mode and depth bias from draw manager state
	config.depthBiasEnabled = m_depthBiasEnabled;

	// Get current render pass, attachment count, and sample count from state tracker
	auto* stateTracker = getStateTracker();
	config.renderPass = stateTracker->getCurrentRenderPass();
	config.colorAttachmentCount = stateTracker->getColorAttachmentCount();
	config.sampleCount = stateTracker->getCurrentSampleCount();

	return config;
}

bool VulkanDrawManager::bindMaterialTextures(material* mat, DescriptorWriter* writer) const
{
	auto* texManager = getTextureManager();

	// Get sampler matching current texture addressing mode and fallback texture
	vk::SamplerAddressMode addressMode = convertTextureAddressing(m_textureAddressing);
	vk::Sampler sampler = texManager->getSampler(
		vk::Filter::eLinear, vk::Filter::eLinear, addressMode, true, 0.0f, true);
	// OpenGL skips applying texture addressing for AABITMAP, INTERFACE, and CUBEMAP
	// types - they always stay clamped. We need a clamp sampler for those cases.
	vk::Sampler clampSampler = texManager->getSampler(
		vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, true, 0.0f, true);
	auto fallbackTexInfo = texManager->getFallbackTextureInfo2DArray();
	fallbackTexInfo.sampler = sampler;

	std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> textureInfos;
	textureInfos.fill(fallbackTexInfo);

	// Check for movie material - needs special YUV texture handling
	auto* movieMat = dynamic_cast<movie_material*>(mat);
	if (movieMat) {
		auto loadYuvTexture = [&](int handle, uint32_t slot) {
			if (handle < 0 || slot >= textureInfos.size()) return;
			auto* texSlot = texManager->getTextureSlot(handle);
			if (!texSlot || !texSlot->imageView) {
				// Load on demand - YUV planes are 8bpp grayscale
				bitmap* bmp = bm_lock(handle, 8, BMP_TEX_OTHER);
				if (bmp) {
					texManager->bm_data(handle, bmp, bm_is_compressed(handle));
					bm_unlock(handle);
					texSlot = texManager->getTextureSlot(handle);
				}
			}
			if (texSlot && texSlot->imageView) {
				textureInfos[slot].imageView = texSlot->imageView;
			}
		};

		loadYuvTexture(movieMat->getYtex(), 0);  // Y at index 0
		loadYuvTexture(movieMat->getUtex(), 1);  // U at index 1
		loadYuvTexture(movieMat->getVtex(), 2);  // V at index 2

		writer->setImageArray(MaterialBinding::TextureArray, textureInfos);
		return true;
	}

	// Helper to set texture at a specific slot - loads on-demand if not present
	static int texLogCount = 0;

	// Get material's expected texture type for the base map
	int materialTextureType = mat->get_texture_type();

	auto setTexture = [&](int textureHandle, uint32_t slot, bool isBaseMap = false) {
		if (textureHandle < 0 || slot >= textureInfos.size()) {
			return;
		}

		// Determine bitmap type - match OpenGL's gr_opengl_tcache_set logic:
		// Override material texture type with bitmap's own type if not NORMAL
		int bitmapType = isBaseMap ? materialTextureType : TCACHE_TYPE_NORMAL;
		int overrideType = bm_get_tcache_type(textureHandle);
		if (overrideType != TCACHE_TYPE_NORMAL) {
			bitmapType = overrideType;
		}

		// OpenGL skips applying texture addressing for AABITMAP, INTERFACE, and
		// CUBEMAP types - they always stay clamped (gropengltexture.cpp:1140-1141).
		// Match that behavior by using a clamp sampler for these types.
		if (bitmapType == TCACHE_TYPE_AABITMAP || bitmapType == TCACHE_TYPE_INTERFACE
			|| bitmapType == TCACHE_TYPE_CUBEMAP) {
			textureInfos[slot].sampler = clampSampler;
		}

		auto* texSlot = texManager->getTextureSlot(textureHandle);

		// If texture isn't loaded, try to load it on-demand (like OpenGL does)
		if (!texSlot || !texSlot->imageView) {
			// Determine bpp and flags - matches OpenGL's opengl_determine_bpp_and_flags
			ushort lockFlags = 0;
			int bpp = 16;

			switch (bitmapType) {
				case TCACHE_TYPE_AABITMAP:
					lockFlags = BMP_AABITMAP;
					bpp = 8;
					break;
				case TCACHE_TYPE_INTERFACE:
				case TCACHE_TYPE_XPARENT:
					lockFlags = BMP_TEX_XPARENT;
					if (bm_get_type(textureHandle) == BM_TYPE_PCX) {
						bpp = 16;
					} else {
						bpp = 32;
					}
					break;
				case TCACHE_TYPE_COMPRESSED:
					switch (bm_is_compressed(textureHandle)) {
						case DDS_DXT1:
							bpp = 24;
							lockFlags = BMP_TEX_DXT1;
							break;
						case DDS_DXT3:
							bpp = 32;
							lockFlags = BMP_TEX_DXT3;
							break;
						case DDS_DXT5:
							bpp = 32;
							lockFlags = BMP_TEX_DXT5;
							break;
						default:
							bpp = 32;
							lockFlags = BMP_TEX_OTHER;
							break;
					}
					break;
				case TCACHE_TYPE_NORMAL:
				default:
					lockFlags = BMP_TEX_OTHER;
					if (bm_get_type(textureHandle) == BM_TYPE_PCX) {
						bpp = 16;  // PCX locking only works with bpp=16
					} else {
						if (bm_has_alpha_channel(textureHandle)) {
							bpp = 32;
						} else {
							bpp = 24;
						}
					}
					break;
			}

			// Lock bitmap with appropriate flags
			bitmap* bmp = bm_lock(textureHandle, bpp, lockFlags);
			if (bmp) {
				// Upload texture
				texManager->bm_data(textureHandle, bmp, bm_is_compressed(textureHandle));
				bm_unlock(textureHandle);

				// Re-get the slot after upload
				texSlot = texManager->getTextureSlot(textureHandle);

				if (texLogCount < 20) {
					nprintf(("vulkan", "bindMaterialTextures: loaded tex %d (type=%d bpp=%d lockFlags=0x%x bmType=%d), slot=%p\n",
						textureHandle, bitmapType, bpp, lockFlags, static_cast<int>(bm_get_type(textureHandle)), texSlot));
					texLogCount++;
				}
			}
		}

		if (texSlot && texSlot->imageView) {
			textureInfos[slot].imageView = texSlot->imageView;
		} else {
			if (texLogCount < 20) {
				nprintf(("vulkan", "bindMaterialTextures: slot %u handle %d FAILED to load\n",
					slot, textureHandle));
				texLogCount++;
			}
		}
	};

	// Bind material textures to their slots
	// Base map uses material's texture type (may be AABITMAP for fonts)
	setTexture(mat->get_texture_map(TM_BASE_TYPE), TextureSlot::BaseMap, true);
	setTexture(mat->get_texture_map(TM_GLOW_TYPE), TextureSlot::GlowMap);

	// Specular - prefer spec_gloss if available
	int specMap = mat->get_texture_map(TM_SPEC_GLOSS_TYPE);
	if (specMap < 0) {
		specMap = mat->get_texture_map(TM_SPECULAR_TYPE);
	}
	setTexture(specMap, TextureSlot::SpecMap);

	setTexture(mat->get_texture_map(TM_NORMAL_TYPE), TextureSlot::NormalMap);
	setTexture(mat->get_texture_map(TM_HEIGHT_TYPE), TextureSlot::HeightMap);
	setTexture(mat->get_texture_map(TM_AMBIENT_TYPE), TextureSlot::AmbientMap);
	setTexture(mat->get_texture_map(TM_MISC_TYPE), TextureSlot::MiscMap);

	// Update the texture array in the descriptor set
	// All slots now have valid views (either actual texture or fallback)
	writer->setImageArray(MaterialBinding::TextureArray, textureInfos);

	return true;
}

bool VulkanDrawManager::applyMaterial(material* mat, primitive_type prim_type, vertex_layout* layout)
{
	auto* stateTracker = getStateTracker();
	auto* pipelineManager = getPipelineManager();
	auto* descManager = getDescriptorManager();
	auto* bufferManager = getBufferManager();

	if (!mat || !layout) {
		return false;
	}

	m_frameStats.applyMaterialCalls++;

	// Build pipeline configuration from material
	PipelineConfig config = buildPipelineConfig(mat, prim_type);

	// Track shader handle issues
	if (mat->get_shader_handle() < 0) {
		m_frameStats.shaderHandleNeg1++;
	}

	// Check if we have a valid render pass
	if (!config.renderPass) {
		m_frameStats.applyMaterialFailures++;
		nprintf(("vulkan", "VulkanDrawManager: applyMaterial FAIL - no render pass (shaderType=%d)\n",
			static_cast<int>(config.shaderType)));
		return false;
	}

	// Get or create pipeline
	vk::Pipeline pipeline = pipelineManager->getPipeline(config, *layout);
	if (!pipeline) {
		m_frameStats.applyMaterialFailures++;
		nprintf(("vulkan", "VulkanDrawManager: applyMaterial FAIL - no pipeline (shaderType=%d handle=%d)\n",
			static_cast<int>(config.shaderType), mat->get_shader_handle()));
		return false;
	}

	// Bind pipeline with layout
	stateTracker->bindPipeline(pipeline, pipelineManager->getPipelineLayout());

	// Bind fallback vertex buffers for attributes the layout doesn't provide but the shader needs
	if (pipelineManager->needsFallbackAttribute(*layout, config.shaderType, VATTRIB_COLOR)) {
		vk::Buffer fallbackColor = bufferManager->getFallbackColorBuffer();
		if (fallbackColor) {
			stateTracker->bindVertexBuffer(FALLBACK_COLOR_BINDING, fallbackColor, 0);
		}
	}
	if (pipelineManager->needsFallbackAttribute(*layout, config.shaderType, VATTRIB_TEXCOORD)) {
		vk::Buffer fallbackTexCoord = bufferManager->getFallbackTexCoordBuffer();
		if (fallbackTexCoord) {
			stateTracker->bindVertexBuffer(FALLBACK_TEXCOORD_BINDING, fallbackTexCoord, 0);
		}
	}

	// Allocate and bind descriptor sets for this draw.
	// Template-based writer pre-fills all bindings with fallbacks,
	// then we overwrite only the bindings that have real data.
	{
		DescriptorWriter writer;
		writer.reset(descManager->getDevice(), descManager->getFallbacks());

		// Bind pending UBOs for a given descriptor set
		auto bindPendingUBOs = [&](DescriptorSetIndex targetSet) {
			for (const auto& entry : VulkanDescriptorManager::getUniformBindings(targetSet)) {
				vk::DescriptorBufferInfo bufInfo;
				const auto& pending = m_pendingUniformBindings[static_cast<size_t>(entry.blockType)];
				if (pending.valid) {
					vk::Buffer buf = bufferManager->getVkBuffer(pending.bufferHandle);
					if (buf) {
						bufInfo = vk::DescriptorBufferInfo(buf, pending.offset, pending.size);
					}
				}
				writer.setBuffer(entry.binding, bufInfo);
			}
		};

		// Set 0: Global (memoized per frame — see m_cachedGlobalSet). Rebuilt only
		// when a Global input changed: a pending Global UBO (m_globalSetDirty set in
		// setPendingUniformBinding), the shadow TLAS (invalidateGlobalSet from
		// setCurrentShadowTlas), or the shadow-map lazy-init transition (compared
		// below). A descriptor set retains its written contents until overwritten
		// or its pool is reset, so subsequent draws just rebind the cached set.
		auto* pp = getPostProcessor();
		const bool shadowReady = (pp && pp->shadow().isInitialized());
		if (m_globalSetDirty || !m_cachedGlobalSet || shadowReady != m_cachedGlobalHadShadow) {
			m_cachedGlobalSet = descManager->allocateFrameSet(DescriptorSetIndex::Global);
			Verify(m_cachedGlobalSet);
			writer.writeSet(m_cachedGlobalSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Global));
			bindPendingUBOs(DescriptorSetIndex::Global);
			if (shadowReady) {
				writer.setImage(GlobalBinding::ShadowMap, pp->getShadowTextureInfo());
			}
			m_cachedGlobalHadShadow = shadowReady;
			m_globalSetDirty = false;
		}
		vk::DescriptorSet globalSet = m_cachedGlobalSet;

		// Set 1: Material (previous-set memoized — see m_cachedMaterialSet).
		// Snapshot every input; reuse the cached set only on an exact match.
		MaterialSetInputs matInputs;
		matInputs.texHandles[0] = mat->get_texture_map(TM_BASE_TYPE);
		matInputs.texHandles[1] = mat->get_texture_map(TM_GLOW_TYPE);
		int specMap = mat->get_texture_map(TM_SPEC_GLOSS_TYPE);
		if (specMap < 0) {
			specMap = mat->get_texture_map(TM_SPECULAR_TYPE);
		}
		matInputs.texHandles[2] = specMap;
		matInputs.texHandles[3] = mat->get_texture_map(TM_NORMAL_TYPE);
		matInputs.texHandles[4] = mat->get_texture_map(TM_HEIGHT_TYPE);
		matInputs.texHandles[5] = mat->get_texture_map(TM_AMBIENT_TYPE);
		matInputs.texHandles[6] = mat->get_texture_map(TM_MISC_TYPE);
		matInputs.textureAddressing = m_textureAddressing;
		{
			auto& tf = g_transformBuffers[descManager->getCurrentFrame()];
			if (tf.buffer && tf.lastUploadSize > 0) {
				matInputs.transformBuffer = tf.buffer;
				matInputs.transformOffset = tf.lastUploadOffset;
				matInputs.transformSize = tf.lastUploadSize;
			}
		}
		matInputs.depthInfo = m_depthTextureInfo;
		matInputs.sceneColorInfo = m_sceneColorInfo;
		matInputs.distMapInfo = m_distMapInfo;
		{
			const auto& md = m_pendingUniformBindings[static_cast<size_t>(uniform_block_type::ModelData)];
			const auto& dg = m_pendingUniformBindings[static_cast<size_t>(uniform_block_type::DecalGlobals)];
			matInputs.uboHandle[0] = md.bufferHandle.value(); matInputs.uboOffset[0] = md.offset;
			matInputs.uboSize[0] = md.size; matInputs.uboValid[0] = md.valid;
			matInputs.uboHandle[1] = dg.bufferHandle.value(); matInputs.uboOffset[1] = dg.offset;
			matInputs.uboSize[1] = dg.size; matInputs.uboValid[1] = dg.valid;
		}

		vk::DescriptorSet materialSet;
		if (m_cachedMaterialValid && m_cachedMaterialSet && matInputs == m_cachedMaterialInputs) {
			// Identical to the previous draw: reuse the set (skips allocation, the
			// template write, texture resolution/upload, and the override/UBO writes).
			materialSet = m_cachedMaterialSet;
		} else {
			materialSet = descManager->allocateFrameSet(DescriptorSetIndex::Material);
			Verify(materialSet);
			writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
			bindPendingUBOs(DescriptorSetIndex::Material);
			if (matInputs.transformBuffer) {
				writer.setBuffer(MaterialBinding::TransformSSBO,
				                 {matInputs.transformBuffer,
				                  static_cast<vk::DeviceSize>(matInputs.transformOffset),
				                  static_cast<vk::DeviceSize>(matInputs.transformSize)});
			}
			writer.setImage(MaterialBinding::DepthMap, m_depthTextureInfo);
			writer.setImage(MaterialBinding::SceneColor, m_sceneColorInfo);
			writer.setImage(MaterialBinding::DistortionMap, m_distMapInfo);
			bindMaterialTextures(mat, &writer);
			m_cachedMaterialSet = materialSet;
			m_cachedMaterialInputs = matInputs;
			m_cachedMaterialValid = true;
		}

		// Set 2: PerDraw (previous-set memoized — see m_cachedPerDrawSet)
		PerDrawSetInputs pdInputs;
		{
			static constexpr uniform_block_type pdTypes[NUM_PERDRAW_UBOS] = {
				uniform_block_type::GenericData, uniform_block_type::Matrices,
				uniform_block_type::NanoVGData, uniform_block_type::DecalInfo,
				uniform_block_type::MovieData};
			for (int i = 0; i < NUM_PERDRAW_UBOS; ++i) {
				const auto& p = m_pendingUniformBindings[static_cast<size_t>(pdTypes[i])];
				pdInputs.uboHandle[i] = p.bufferHandle.value();
				pdInputs.uboOffset[i] = p.offset;
				pdInputs.uboSize[i] = p.size;
				pdInputs.uboValid[i] = p.valid;
			}
		}

		vk::DescriptorSet perDrawSet;
		if (m_cachedPerDrawValid && m_cachedPerDrawSet && pdInputs == m_cachedPerDrawInputs) {
			perDrawSet = m_cachedPerDrawSet;
		} else {
			perDrawSet = descManager->allocateFrameSet(DescriptorSetIndex::PerDraw);
			Verify(perDrawSet);
			writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
			bindPendingUBOs(DescriptorSetIndex::PerDraw);
			m_cachedPerDrawSet = perDrawSet;
			m_cachedPerDrawInputs = pdInputs;
			m_cachedPerDrawValid = true;
		}
		writer.flush();
		stateTracker->bindDescriptorSet(DescriptorSetIndex::Global, globalSet);
		stateTracker->bindDescriptorSet(DescriptorSetIndex::Material, materialSet);
		stateTracker->bindDescriptorSet(DescriptorSetIndex::PerDraw, perDrawSet);
	}

	// Update tracked state for FSO compatibility
	stateTracker->setZBufferMode(mat->get_depth_mode());
	stateTracker->setCullMode(mat->get_cull_mode());

	if (mat->is_stencil_enabled()) {
		stateTracker->setStencilMode(GR_STENCIL_READ);
		stateTracker->setStencilReference(mat->get_stencil_func().ref);
	} else {
		stateTracker->setStencilMode(GR_STENCIL_NONE);
	}

	// Set depth bias if needed
	stateTracker->setDepthBias(static_cast<float>(mat->get_depth_bias()), 0.0f);

	return true;
}

void VulkanDrawManager::bindVertexBuffer(gr_buffer_handle handle, size_t offset)
{
	(void)this;
	auto* bufferManager = getBufferManager();
	auto* stateTracker = getStateTracker();

	if (!handle.isValid()) {
		return;
	}

	vk::Buffer buffer = bufferManager->getVkBuffer(handle);
	if (buffer) {
		// Add frame base offset for ring buffer support
		// This maps the caller's offset into the current frame's span
		size_t frameOffset = bufferManager->getFrameBaseOffset(handle);
		size_t totalOffset = frameOffset + offset;
		stateTracker->bindVertexBuffer(0, buffer, static_cast<vk::DeviceSize>(totalOffset));
	}
}

void VulkanDrawManager::bindIndexBuffer(gr_buffer_handle handle)
{
	(void)this;
	auto* bufferManager = getBufferManager();
	auto* stateTracker = getStateTracker();

	if (!handle.isValid()) {
		return;
	}

	vk::Buffer buffer = bufferManager->getVkBuffer(handle);
	if (buffer) {
		// Add frame base offset for ring buffer support (mirrors bindVertexBuffer)
		size_t frameOffset = bufferManager->getFrameBaseOffset(handle);
		stateTracker->bindIndexBuffer(buffer, static_cast<vk::DeviceSize>(frameOffset), vk::IndexType::eUint32);
	}
}

void VulkanDrawManager::draw(primitive_type prim_type, int first_vertex, int vertex_count)
{
	auto* stateTracker = getStateTracker();

	Assertion(stateTracker->getCurrentPipeline(),
		"draw() called with no bound pipeline! prim_type=%d first_vertex=%d vertex_count=%d",
		static_cast<int>(prim_type), first_vertex, vertex_count);
	if (!stateTracker->getCurrentPipeline()) {
		m_frameStats.noPipelineSkips++;
		return;
	}

	m_frameStats.drawCalls++;
	m_frameStats.totalVertices += vertex_count;

	// Flush any dirty dynamic state (viewport, scissor, depth bias, stencil ref)
	// before issuing the draw command. applyMaterial sets these AFTER bindPipeline,
	// so they may be dirty even when the pipeline didn't change.
	stateTracker->applyDynamicState();

	auto cmdBuffer = stateTracker->getCommandBuffer();
	cmdBuffer.draw(static_cast<uint32_t>(vertex_count),
	               1,
	               static_cast<uint32_t>(first_vertex),
	               0);
}

void VulkanDrawManager::drawIndexed(primitive_type prim_type, int index_count, int first_index, int vertex_offset)
{
	auto* stateTracker = getStateTracker();

	Assertion(stateTracker->getCurrentPipeline(),
		"drawIndexed() called with no bound pipeline! prim_type=%d index_count=%d first_index=%d vertex_offset=%d",
		static_cast<int>(prim_type), index_count, first_index, vertex_offset);
	if (!stateTracker->getCurrentPipeline()) {
		m_frameStats.noPipelineSkips++;
		return;
	}

	m_frameStats.drawIndexedCalls++;
	m_frameStats.totalIndices += index_count;

	// Flush any dirty dynamic state before draw
	stateTracker->applyDynamicState();

	auto cmdBuffer = stateTracker->getCommandBuffer();
	cmdBuffer.drawIndexed(static_cast<uint32_t>(index_count),
	                      1,
	                      static_cast<uint32_t>(first_index),
	                      vertex_offset,
	                      0);
}

void VulkanDrawManager::initSphereBuffers()
{
	auto* bufferManager = getBufferManager();

	auto mesh = graphics::util::generate_sphere_mesh(16, 16);

	m_sphereIndexCount = mesh.index_count;

	m_sphereVBO = bufferManager->createBuffer(BufferType::Vertex, BufferUsageHint::Static);
	bufferManager->updateBufferData(m_sphereVBO, mesh.vertices.size() * sizeof(float), mesh.vertices.data());

	m_sphereIBO = bufferManager->createBuffer(BufferType::Index, BufferUsageHint::Static);
	bufferManager->updateBufferData(m_sphereIBO, mesh.indices.size() * sizeof(ushort), mesh.indices.data());

	m_sphereVertexLayout.add_vertex_component(vertex_format_data::POSITION3, sizeof(float) * 3, 0);

	nprintf(("vulkan", "VulkanDrawManager: Sphere mesh created (%u vertices, %u indices)\n",
		mesh.vertex_count, mesh.index_count));
}

void VulkanDrawManager::shutdownSphereBuffers()
{
	auto* bufferManager = getBufferManager();

	if (m_sphereVBO.isValid()) {
		bufferManager->deleteBuffer(m_sphereVBO);
		m_sphereVBO = gr_buffer_handle::invalid();
	}
	if (m_sphereIBO.isValid()) {
		bufferManager->deleteBuffer(m_sphereIBO);
		m_sphereIBO = gr_buffer_handle::invalid();
	}
}

void VulkanDrawManager::drawSphere(material* material_def)
{
	if (!material_def || m_sphereIndexCount == 0) {
		return;
	}

	auto* stateTracker = getStateTracker();

	auto* bufferManager = getBufferManager();

	if (!applyMaterial(material_def, PRIM_TYPE_TRIS, &m_sphereVertexLayout)) {
		return;
	}

	// Bind sphere vertex buffer
	vk::Buffer vbo = bufferManager->getVkBuffer(m_sphereVBO);
	if (!vbo) {
		return;
	}
	stateTracker->bindVertexBuffer(0, vbo, 0);

	// Bind sphere index buffer with uint16 indices (matching the ushort mesh data)
	vk::Buffer ibo = bufferManager->getVkBuffer(m_sphereIBO);
	if (!ibo) {
		return;
	}
	stateTracker->bindIndexBuffer(ibo, 0, vk::IndexType::eUint16);

	drawIndexed(PRIM_TYPE_TRIS, static_cast<int>(m_sphereIndexCount), 0, 0);
}

} // namespace graphics::vulkan

