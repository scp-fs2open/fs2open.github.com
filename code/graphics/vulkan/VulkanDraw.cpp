#include "VulkanDraw.h"

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
#include "lighting/lighting.h"
#include "graphics/util/UniformBuffer.h"
#include "graphics/shaders/compiled/default-material_structs.vert.h"

namespace graphics {
namespace vulkan {

// Texture slot mapping - material texture types to descriptor binding indices
// Binding 1 in Material set is a texture array with up to 16 textures
static constexpr uint32_t TEXTURE_BINDING_BASE_MAP = 0;
static constexpr uint32_t TEXTURE_BINDING_GLOW_MAP = 1;
static constexpr uint32_t TEXTURE_BINDING_SPEC_MAP = 2;
static constexpr uint32_t TEXTURE_BINDING_NORMAL_MAP = 3;
static constexpr uint32_t TEXTURE_BINDING_HEIGHT_MAP = 4;
static constexpr uint32_t TEXTURE_BINDING_AMBIENT_MAP = 5;
static constexpr uint32_t TEXTURE_BINDING_MISC_MAP = 6;

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
static TransformBufferState g_transformBuffers[MAX_FRAMES_IN_FLIGHT];
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
			mprintf(("vulkan_update_transform_buffer: Failed to create buffer: %s\n", e.what()));
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
	mprintf(("VulkanDrawManager: Initialized\n"));
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
	mprintf(("VulkanDrawManager: Shutdown complete\n"));
}

void VulkanDrawManager::clear()
{
	auto* stateTracker = getStateTracker();

	// Use the current clip/scissor region for clearing, matching OpenGL behavior.
	// In OpenGL, glClear() respects the scissor test - if a clip region is set,
	// only that region is cleared. Without this, HUD code that does
	// gr_set_clip(panel) + gr_clear() would wipe the entire screen in Vulkan.
	vk::ClearAttachment clearAttachment;
	clearAttachment.aspectMask = vk::ImageAspectFlagBits::eColor;
	clearAttachment.colorAttachment = 0;
	clearAttachment.clearValue.color = stateTracker->getClearColor();

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
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;

	auto cmdBuffer = stateTracker->getCommandBuffer();
	cmdBuffer.clearAttachments(1, &clearAttachment, 1, &clearRect);
}

void VulkanDrawManager::setClearColor(int r, int g, int b)
{
	auto* stateTracker = getStateTracker();

	float fr = static_cast<float>(r) / 255.0f;
	float fg = static_cast<float>(g) / 255.0f;
	float fb = static_cast<float>(b) / 255.0f;

	// Apply HDR gamma if needed
	if (High_dynamic_range) {
		const float SRGB_GAMMA = 2.2f;
		fr = powf(fr, SRGB_GAMMA);
		fg = powf(fg, SRGB_GAMMA);
		fb = powf(fb, SRGB_GAMMA);
	}

	stateTracker->setClearColor(fr, fg, fb, 1.0f);

	// Also update gr_screen for compatibility
	gr_screen.current_clear_color.red = static_cast<ubyte>(r);
	gr_screen.current_clear_color.green = static_cast<ubyte>(g);
	gr_screen.current_clear_color.blue = static_cast<ubyte>(b);
	gr_screen.current_clear_color.alpha = 255;
}

void VulkanDrawManager::setClip(int x, int y, int w, int h, int resize_mode)
{
	auto* stateTracker = getStateTracker();

	// Clamp values
	if (x < 0) x = 0;
	if (y < 0) y = 0;

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
		if (w > max_w) w = max_w;
		if (h > max_h) h = max_h;
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

int VulkanDrawManager::zbufferGet()
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

		// Clear depth buffer
		vk::ClearAttachment clearAttachment;
		clearAttachment.aspectMask = vk::ImageAspectFlagBits::eDepth;
		clearAttachment.clearValue.depthStencil.depth = 1.0f;
		clearAttachment.clearValue.depthStencil.stencil = 0;

		vk::ClearRect clearRect;
		clearRect.rect.offset = vk::Offset2D(0, 0);
		clearRect.rect.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.max_w),
		                                      static_cast<uint32_t>(gr_screen.max_h));
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;

		stateTracker->getCommandBuffer().clearAttachments(1, &clearAttachment, 1, &clearRect);
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
	auto* stateTracker = getStateTracker();

	// Clear stencil buffer
	vk::ClearAttachment clearAttachment;
	clearAttachment.aspectMask = vk::ImageAspectFlagBits::eStencil;
	clearAttachment.clearValue.depthStencil.depth = 1.0f;
	clearAttachment.clearValue.depthStencil.stencil = 0;

	vk::ClearRect clearRect;
	clearRect.rect.offset = vk::Offset2D(0, 0);
	clearRect.rect.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.max_w),
	                                      static_cast<uint32_t>(gr_screen.max_h));
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;

	stateTracker->getCommandBuffer().clearAttachments(1, &clearAttachment, 1, &clearRect);
}

int VulkanDrawManager::setCull(int cull)
{
	auto* stateTracker = getStateTracker();

	int prev = m_cullEnabled ? 1 : 0;
	m_cullEnabled = (cull != 0);

	stateTracker->setCullMode(m_cullEnabled);

	return prev;
}

void VulkanDrawManager::renderPrimitives(material* material_info, primitive_type prim_type,
                                          vertex_layout* layout, int offset, int n_verts,
                                          gr_buffer_handle buffer_handle, size_t buffer_offset)
{
	if (!material_info || !layout || n_verts <= 0) {
		return;
	}

	m_frameStats.renderPrimitiveCalls++;

	// Apply material state and bind pipeline
	if (!applyMaterial(material_info, prim_type, layout)) {
		return;
	}

	// Bind vertex buffer
	bindVertexBuffer(buffer_handle, buffer_offset);

	// Issue draw call
	draw(prim_type, offset, n_verts);
}

void VulkanDrawManager::renderPrimitivesBatched(batched_bitmap_material* material_info,
                                                 primitive_type prim_type, vertex_layout* layout,
                                                 int offset, int n_verts, gr_buffer_handle buffer_handle)
{
	if (!material_info || !layout || n_verts <= 0) {
		return;
	}

	m_frameStats.renderBatchedCalls++;

	// Apply base material state and bind pipeline
	if (!applyMaterial(material_info, prim_type, layout)) {
		return;
	}

	// Bind vertex buffer
	bindVertexBuffer(buffer_handle, 0);

	// Issue draw call
	draw(prim_type, offset, n_verts);
}

void VulkanDrawManager::renderPrimitivesParticle(particle_material* material_info,
                                                  primitive_type prim_type, vertex_layout* layout,
                                                  int offset, int n_verts, gr_buffer_handle buffer_handle)
{
	if (!material_info || !layout || n_verts <= 0) {
		return;
	}

	m_frameStats.renderParticleCalls++;

	if (!applyMaterial(material_info, prim_type, layout)) {
		return;
	}
	bindVertexBuffer(buffer_handle, 0);
	draw(prim_type, offset, n_verts);
}

void VulkanDrawManager::renderPrimitivesDistortion(distortion_material* material_info,
                                                    primitive_type prim_type, vertex_layout* layout,
                                                    int n_verts, gr_buffer_handle buffer_handle)
{
	if (!material_info || !layout || n_verts <= 0) {
		return;
	}

	if (!applyMaterial(material_info, prim_type, layout)) {
		return;
	}
	bindVertexBuffer(buffer_handle, 0);
	draw(prim_type, 0, n_verts);
}

void VulkanDrawManager::renderMovie(movie_material* material_info, primitive_type prim_type,
                                     vertex_layout* layout, int n_verts, gr_buffer_handle buffer_handle)
{
	if (!material_info || !layout || n_verts <= 0) {
		return;
	}

	m_frameStats.renderMovieCalls++;

	if (!applyMaterial(material_info, prim_type, layout)) {
		return;
	}
	bindVertexBuffer(buffer_handle, 0);
	draw(prim_type, 0, n_verts);
}

void VulkanDrawManager::renderNanoVG(nanovg_material* material_info, primitive_type prim_type,
                                      vertex_layout* layout, int offset, int n_verts,
                                      gr_buffer_handle buffer_handle)
{
	if (!material_info || !layout || n_verts <= 0) {
		return;
	}

	m_frameStats.renderNanoVGCalls++;

	if (!applyMaterial(material_info, prim_type, layout)) {
		return;
	}
	bindVertexBuffer(buffer_handle, 0);
	draw(prim_type, offset, n_verts);
}

void VulkanDrawManager::renderRocketPrimitives(interface_material* material_info,
                                                primitive_type prim_type, vertex_layout* layout,
                                                int n_indices, gr_buffer_handle vertex_buffer,
                                                gr_buffer_handle index_buffer)
{
	if (!material_info || !layout || n_indices <= 0) {
		return;
	}

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
	int32_t baseVertex = static_cast<int32_t>(vert_source->Base_vertex_offset + bufferp->vertex_num_offset);

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
	m_frameStats.totalIndices += datap->n_verts;

	// Flush any dirty dynamic state before draw
	stateTracker->applyDynamicState();

	// Shadow map rendering uses 4 instances (one per cascade), routed via gl_InstanceIndex → gl_Layer
	uint32_t instanceCount = Rendering_to_shadow_map ? 4 : 1;

	auto cmdBuffer = stateTracker->getCommandBuffer();
	cmdBuffer.drawIndexed(
		static_cast<uint32_t>(datap->n_verts),  // index count
		instanceCount,                           // instance count
		firstIndex,                              // first index
		baseVertex,                              // vertex offset
		0                                        // first instance
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

void VulkanDrawManager::setDepthTextureOverride(vk::ImageView view, vk::Sampler sampler)
{
	m_depthTextureOverride = view;
	m_depthSamplerOverride = sampler;
}

void VulkanDrawManager::clearDepthTextureOverride()
{
	m_depthTextureOverride = nullptr;
	m_depthSamplerOverride = nullptr;
}

void VulkanDrawManager::setSceneColorOverride(vk::ImageView view, vk::Sampler sampler)
{
	m_sceneColorOverride = view;
	m_sceneColorSamplerOverride = sampler;
}

void VulkanDrawManager::setDistMapOverride(vk::ImageView view, vk::Sampler sampler)
{
	m_distMapOverride = view;
	m_distMapSamplerOverride = sampler;
}

void VulkanDrawManager::clearDistortionOverrides()
{
	m_sceneColorOverride = nullptr;
	m_sceneColorSamplerOverride = nullptr;
	m_distMapOverride = nullptr;
	m_distMapSamplerOverride = nullptr;
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
	size_t index = static_cast<size_t>(blockType);
	if (index >= NUM_UNIFORM_BLOCK_TYPES) {
		return;
	}

	m_pendingUniformBindings[index].bufferHandle = bufferHandle;
	m_pendingUniformBindings[index].offset = offset;
	m_pendingUniformBindings[index].size = size;
	m_pendingUniformBindings[index].valid = bufferHandle.isValid();
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
}

void VulkanDrawManager::printFrameStats()
{
	// Print summary every frame for the first 200 frames, then every 60 frames
	bool shouldPrint = (m_frameStatsFrameNum < 200) || (m_frameStatsFrameNum % 60 == 0);

	if (shouldPrint) {
		mprintf(("FRAME %d STATS: draws=%d indexed=%d verts=%d idxs=%d | applyMat=%d/%d fails | noPipeline=%d sdrNeg1=%d\n",
			m_frameStatsFrameNum,
			m_frameStats.drawCalls,
			m_frameStats.drawIndexedCalls,
			m_frameStats.totalVertices,
			m_frameStats.totalIndices,
			m_frameStats.applyMaterialFailures,
			m_frameStats.applyMaterialCalls,
			m_frameStats.noPipelineSkips,
			m_frameStats.shaderHandleNeg1));
		mprintf(("  CALLS: prim=%d batch=%d model=%d particle=%d nanovg=%d rocket=%d movie=%d\n",
			m_frameStats.renderPrimitiveCalls,
			m_frameStats.renderBatchedCalls,
			m_frameStats.renderModelCalls,
			m_frameStats.renderParticleCalls,
			m_frameStats.renderNanoVGCalls,
			m_frameStats.renderRocketCalls,
			m_frameStats.renderMovieCalls));
	}

	m_frameStatsFrameNum++;
}


PipelineConfig VulkanDrawManager::buildPipelineConfig(material* mat, primitive_type prim_type)
{
	PipelineConfig config;

	// Get shader info from material
	int shaderHandle = mat->get_shader_handle();
	auto* shaderManager = getShaderManager();
	if (shaderHandle >= 0) {
		const auto* shaderModule = shaderManager->getShaderByHandle(shaderHandle);
		if (shaderModule) {
			config.shaderType = shaderModule->type;
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

	// Override shader for shadow map rendering
	if (Rendering_to_shadow_map && config.shaderType == SDR_TYPE_MODEL) {
		config.shaderType = SDR_TYPE_SHADOW_MAP;
	}

	// Front face winding: match OpenGL which defaults to CCW and only switches to CW
	// for model rendering (opengl_tnl_set_model_material sets GL_CW).
	config.frontFaceCW = (config.shaderType == SDR_TYPE_MODEL || config.shaderType == SDR_TYPE_SHADOW_MAP);

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
	config.fillMode = m_fillMode;
	config.depthBiasEnabled = m_depthBiasEnabled;

	// Get current render pass, attachment count, and sample count from state tracker
	auto* stateTracker = getStateTracker();
	config.renderPass = stateTracker->getCurrentRenderPass();
	config.colorAttachmentCount = stateTracker->getColorAttachmentCount();
	config.sampleCount = stateTracker->getCurrentSampleCount();

	return config;
}

bool VulkanDrawManager::bindMaterialTextures(material* mat, vk::DescriptorSet materialSet,
                                              DescriptorWriter* writer)
{
	auto* texManager = getTextureManager();

	if (!materialSet) {
		return false;
	}

	// Get sampler matching current texture addressing mode and fallback texture
	vk::SamplerAddressMode addressMode = convertTextureAddressing(m_textureAddressing);
	vk::Sampler sampler = texManager->getSampler(
		vk::Filter::eLinear, vk::Filter::eLinear, addressMode, true, 0.0f, true);
	// OpenGL skips applying texture addressing for AABITMAP, INTERFACE, and CUBEMAP
	// types - they always stay clamped. We need a clamp sampler for those cases.
	vk::Sampler clampSampler = texManager->getSampler(
		vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, true, 0.0f, true);
	vk::ImageView fallbackView = texManager->getFallback2DArrayView();

	// Check for movie material - needs special YUV texture handling
	auto* movieMat = dynamic_cast<movie_material*>(mat);
	if (movieMat) {
		// Movie materials use 3 YUV textures in the texture array at indices 0, 1, 2
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> textureInfos;

		// Initialize all slots with fallback
		for (auto& info : textureInfos) {
			info.sampler = sampler;
			info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			info.imageView = fallbackView;
		}

		auto loadYuvTexture = [&](int handle, uint32_t slot) {
			if (handle < 0 || slot >= textureInfos.size()) return;
			auto* texSlot = texManager->getTextureSlot(handle);
			if (!texSlot || !texSlot->imageView) {
				// Load on demand - YUV planes are 8bpp grayscale
				bitmap* bmp = bm_lock(handle, 8, BMP_TEX_OTHER);
				if (bmp) {
					texManager->bm_data(handle, bmp);
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

		writer->writeTextureArray(materialSet, 1, textureInfos.data(), static_cast<uint32_t>(textureInfos.size()));
		return true;
	}

	// Build texture info array for all material texture slots
	std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> textureInfos;

	// Initialize all slots with fallback texture (1x1 white)
	for (auto& info : textureInfos) {
		info.sampler = sampler;
		info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		info.imageView = fallbackView;  // Fallback texture for unbound slots
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
				texManager->bm_data(textureHandle, bmp);
				bm_unlock(textureHandle);

				// Re-get the slot after upload
				texSlot = texManager->getTextureSlot(textureHandle);

				if (texLogCount < 20) {
					mprintf(("bindMaterialTextures: loaded tex %d (type=%d bpp=%d lockFlags=0x%x bmType=%d), slot=%p\n",
						textureHandle, bitmapType, bpp, lockFlags, static_cast<int>(bm_get_type(textureHandle)), texSlot));
					texLogCount++;
				}
			}
		}

		if (texSlot && texSlot->imageView) {
			textureInfos[slot].imageView = texSlot->imageView;
		} else {
			if (texLogCount < 20) {
				mprintf(("bindMaterialTextures: slot %u handle %d FAILED to load\n",
					slot, textureHandle));
				texLogCount++;
			}
		}
	};

	// Bind material textures to their slots
	// Base map uses material's texture type (may be AABITMAP for fonts)
	setTexture(mat->get_texture_map(TM_BASE_TYPE), TEXTURE_BINDING_BASE_MAP, true);
	setTexture(mat->get_texture_map(TM_GLOW_TYPE), TEXTURE_BINDING_GLOW_MAP);

	// Specular - prefer spec_gloss if available
	int specMap = mat->get_texture_map(TM_SPEC_GLOSS_TYPE);
	if (specMap < 0) {
		specMap = mat->get_texture_map(TM_SPECULAR_TYPE);
	}
	setTexture(specMap, TEXTURE_BINDING_SPEC_MAP);

	setTexture(mat->get_texture_map(TM_NORMAL_TYPE), TEXTURE_BINDING_NORMAL_MAP);
	setTexture(mat->get_texture_map(TM_HEIGHT_TYPE), TEXTURE_BINDING_HEIGHT_MAP);
	setTexture(mat->get_texture_map(TM_AMBIENT_TYPE), TEXTURE_BINDING_AMBIENT_MAP);
	setTexture(mat->get_texture_map(TM_MISC_TYPE), TEXTURE_BINDING_MISC_MAP);

	// Update the texture array in the descriptor set
	// All slots now have valid views (either actual texture or fallback)
	writer->writeTextureArray(materialSet, 1, textureInfos.data(), static_cast<uint32_t>(textureInfos.size()));

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

	// Helper to get vk::Buffer from handle at draw time (survives buffer recreation)
	auto getBuffer = [bufferManager](const PendingUniformBinding& binding) -> vk::Buffer {
		return bufferManager->getVkBuffer(binding.bufferHandle);
	};

	// Offset is already fully resolved at bind time (includes frame base offset)
	// to prevent stale lastWriteStreamOffset if the buffer is updated between bind and draw.
	auto getResolvedOffset = [](const PendingUniformBinding& binding) -> vk::DeviceSize {
		return binding.offset;
	};

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
		mprintf(("VulkanDrawManager: applyMaterial FAIL - no render pass (shaderType=%d)\n",
			static_cast<int>(config.shaderType)));
		return false;
	}

	// Get or create pipeline
	vk::Pipeline pipeline = pipelineManager->getPipeline(config, *layout);
	if (!pipeline) {
		m_frameStats.applyMaterialFailures++;
		mprintf(("VulkanDrawManager: applyMaterial FAIL - no pipeline (shaderType=%d handle=%d)\n",
			static_cast<int>(config.shaderType), mat->get_shader_handle()));
		return false;
	}

	// Bind pipeline with layout
	stateTracker->bindPipeline(pipeline, pipelineManager->getPipelineLayout());

	// Bind fallback vertex buffers for attributes the layout doesn't provide but the shader needs
	if (pipelineManager->needsFallbackAttribute(*layout, config.shaderType, VertexAttributeLocation::Color)) {
		vk::Buffer fallbackColor = bufferManager->getFallbackColorBuffer();
		if (fallbackColor) {
			stateTracker->bindVertexBuffer(FALLBACK_COLOR_BINDING, fallbackColor, 0);
		}
	}
	if (pipelineManager->needsFallbackAttribute(*layout, config.shaderType, VertexAttributeLocation::TexCoord)) {
		vk::Buffer fallbackTexCoord = bufferManager->getFallbackTexCoordBuffer();
		if (fallbackTexCoord) {
			stateTracker->bindVertexBuffer(FALLBACK_TEXCOORD_BINDING, fallbackTexCoord, 0);
		}
	}

	// Allocate and bind descriptor sets for this draw.
	// Vulkan requires all bindings in a descriptor set to be valid before use.
	// After pool reset, descriptors contain undefined data. We MUST pre-initialize
	// ALL bindings with fallback values, then overwrite with actual pending data.
	// All writes are batched into a single vkUpdateDescriptorSets call.
	{
		DescriptorWriter writer;
		writer.reset(descManager->getDevice());

		// Get fallback resources for uninitialized bindings
		vk::Buffer fallbackUBO = bufferManager->getFallbackUniformBuffer();
		vk::DeviceSize fallbackUBOSize = static_cast<vk::DeviceSize>(bufferManager->getFallbackUniformBufferSize());
		auto* texManager = getTextureManager();
		vk::Sampler fallbackSampler = texManager->getDefaultSampler();
		vk::ImageView fallbackView = texManager->getFallback2DArrayView();

		// Helper: write a pending UBO or fallback if the buffer is null/invalid
		auto writeUBOOrFallback = [&](DescriptorWriter& w, vk::DescriptorSet set,
		                              uint32_t binding, size_t blockIdx) {
			if (m_pendingUniformBindings[blockIdx].valid) {
				vk::Buffer buf = getBuffer(m_pendingUniformBindings[blockIdx]);
				if (buf) {
					w.writeUniformBuffer(set, binding, buf,
					                     getResolvedOffset(m_pendingUniformBindings[blockIdx]),
					                     m_pendingUniformBindings[blockIdx].size);
					return;
				}
			}
			w.writeUniformBuffer(set, binding, fallbackUBO, 0, fallbackUBOSize);
		};

		// Set 0: Global - bindings: 0=Lights UBO, 1=DeferredGlobals UBO, 2=Shadow tex, 3=Env cube, 4=Irr cube
		vk::DescriptorSet globalSet = descManager->allocateFrameSet(DescriptorSetIndex::Global);
		Verify(globalSet);
		// UBO bindings: write real pending buffer or fallback (one write per binding)
		for (size_t i = 0; i < NUM_UNIFORM_BLOCK_TYPES; ++i) {
			uniform_block_type blockType = static_cast<uniform_block_type>(i);
			DescriptorSetIndex setIndex;
			uint32_t binding;
			if (VulkanDescriptorManager::getUniformBlockBinding(blockType, setIndex, binding) &&
			    setIndex == DescriptorSetIndex::Global) {
				writeUBOOrFallback(writer, globalSet, binding, i);
			}
		}
		// Texture bindings
		writer.writeTexture(globalSet, 2, fallbackView, fallbackSampler);
		vk::ImageView fallbackCubeView = texManager->getFallbackCubeView();
		writer.writeTexture(globalSet, 3, fallbackCubeView, fallbackSampler);
		writer.writeTexture(globalSet, 4, fallbackCubeView, fallbackSampler);
		writer.flush();
		stateTracker->bindDescriptorSet(DescriptorSetIndex::Global, globalSet);

		// Set 1: Material - bindings: 0=ModelData UBO, 1=Texture array, 2=DecalGlobals UBO,
		//                   3=Transform SSBO, 4=depth, 5=scene color, 6=dist map
		vk::DescriptorSet materialSet = descManager->allocateFrameSet(DescriptorSetIndex::Material);
		Verify(materialSet);
		// UBO bindings: write real pending buffer or fallback (one write per binding)
		for (size_t i = 0; i < NUM_UNIFORM_BLOCK_TYPES; ++i) {
			uniform_block_type blockType = static_cast<uniform_block_type>(i);
			DescriptorSetIndex setIndex;
			uint32_t binding;
			if (VulkanDescriptorManager::getUniformBlockBinding(blockType, setIndex, binding) &&
			    setIndex == DescriptorSetIndex::Material) {
				writeUBOOrFallback(writer, materialSet, binding, i);
			}
		}
		// Binding 3: Transform buffer SSBO — real if available, else fallback
		{
			uint32_t tfIdx = descManager->getCurrentFrame();
			auto& tf = g_transformBuffers[tfIdx];
			if (tf.buffer && tf.lastUploadSize > 0) {
				writer.writeStorageBuffer(materialSet, 3, tf.buffer,
				                          static_cast<vk::DeviceSize>(tf.lastUploadOffset),
				                          static_cast<vk::DeviceSize>(tf.lastUploadSize));
			} else {
				writer.writeStorageBuffer(materialSet, 3, fallbackUBO, 0, fallbackUBOSize);
			}
		}
		// Binding 4: depth map for soft particles
		{
			vk::ImageView depthView = m_depthTextureOverride ? m_depthTextureOverride
			                                                  : texManager->getFallbackTextureView2D();
			vk::Sampler depthSampler = m_depthSamplerOverride ? m_depthSamplerOverride
			                                                   : texManager->getDefaultSampler();
			writer.writeTexture(materialSet, 4, depthView, depthSampler);
		}
		// Binding 5: scene color / frameBuffer for distortion
		{
			vk::ImageView sceneView = m_sceneColorOverride ? m_sceneColorOverride
			                                                : texManager->getFallbackTextureView2D();
			vk::Sampler sceneSampler = m_sceneColorSamplerOverride ? m_sceneColorSamplerOverride
			                                                       : texManager->getDefaultSampler();
			writer.writeTexture(materialSet, 5, sceneView, sceneSampler);
		}
		// Binding 6: distortion map
		{
			vk::ImageView distView = m_distMapOverride ? m_distMapOverride
			                                            : texManager->getFallbackTextureView2D();
			vk::Sampler distSampler = m_distMapSamplerOverride ? m_distMapSamplerOverride
			                                                    : texManager->getDefaultSampler();
			writer.writeTexture(materialSet, 6, distView, distSampler);
		}
		// Binding 1: Texture array
		bindMaterialTextures(mat, materialSet, &writer);
		writer.flush();
		stateTracker->bindDescriptorSet(DescriptorSetIndex::Material, materialSet);

		// Set 2: PerDraw - bindings: 0=GenericData, 1=Matrices, 2=NanoVGData, 3=DecalInfo, 4=MovieData
		vk::DescriptorSet perDrawSet = descManager->allocateFrameSet(DescriptorSetIndex::PerDraw);
		Verify(perDrawSet);
		// UBO bindings: write real pending buffer or fallback (one write per binding)
		for (size_t i = 0; i < NUM_UNIFORM_BLOCK_TYPES; ++i) {
			uniform_block_type blockType = static_cast<uniform_block_type>(i);
			DescriptorSetIndex setIndex;
			uint32_t binding;
			if (VulkanDescriptorManager::getUniformBlockBinding(blockType, setIndex, binding) &&
			    setIndex == DescriptorSetIndex::PerDraw) {
				writeUBOOrFallback(writer, perDrawSet, binding, i);
			}
		}
		writer.flush();
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

	mprintf(("VulkanDrawManager: Sphere mesh created (%u vertices, %u indices)\n",
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

} // namespace vulkan
} // namespace graphics

// GL_alpha_threshold is defined in gropengl.cpp
extern float GL_alpha_threshold;

// PostProcessing_override is defined in globalincs/systemvars.cpp
extern bool PostProcessing_override;

namespace graphics {
namespace vulkan {

// ========== gr_screen function pointer implementations ==========
// These free functions are assigned to gr_screen.gf_* in gr_vulkan.cpp.

namespace {

// Helper to set up GenericData uniform for default material shader
// Similar to opengl_shader_set_default_material() in gropenglshader.cpp
void vulkan_set_default_material_uniforms(material* material_info)
{
	if (!material_info) {
		return;
	}

	// Get uniform buffer for GenericData
	auto buffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1, sizeof(genericData_default_material_vert));
	auto* data = buffer.aligner().addTypedElement<genericData_default_material_vert>();

	// Get base map from material
	int base_map = material_info->get_texture_map(TM_BASE_TYPE);
	bool textured = (base_map >= 0);
	bool alpha = (material_info->get_texture_type() == TCACHE_TYPE_AABITMAP);

	// Texturing flags
	if (textured) {
		data->noTexturing = 0;
		// Get array index for animated texture arrays
		auto* texSlot = getTextureManager()->getTextureSlot(base_map);
		data->baseMapIndex = texSlot ? static_cast<int>(texSlot->arrayIndex) : 0;
	} else {
		data->noTexturing = 1;
		data->baseMapIndex = 0;
	}

	// Alpha texture flag
	data->alphaTexture = alpha ? 1 : 0;

	// HDR / intensity settings
	if (High_dynamic_range) {
		data->srgb = 1;
		data->intensity = material_info->get_color_scale();
	} else {
		data->srgb = 0;
		data->intensity = 1.0f;
	}

	// Alpha threshold
	data->alphaThreshold = GL_alpha_threshold;

	// Color from material
	vec4 clr = material_info->get_color();
	data->color.a1d[0] = clr.xyzw.x;
	data->color.a1d[1] = clr.xyzw.y;
	data->color.a1d[2] = clr.xyzw.z;
	data->color.a1d[3] = clr.xyzw.w;

	// Clip plane
	const auto& clip_plane = material_info->get_clip_plane();
	if (clip_plane.enabled) {
		data->clipEnabled = 1;

		data->clipEquation.a1d[0] = clip_plane.normal.xyz.x;
		data->clipEquation.a1d[1] = clip_plane.normal.xyz.y;
		data->clipEquation.a1d[2] = clip_plane.normal.xyz.z;
		// Calculate 'd' value: d = -dot(normal, position)
		data->clipEquation.a1d[3] = -(clip_plane.normal.xyz.x * clip_plane.position.xyz.x +
		                              clip_plane.normal.xyz.y * clip_plane.position.xyz.y +
		                              clip_plane.normal.xyz.z * clip_plane.position.xyz.z);

		// Model matrix (identity for now, material doesn't provide one)
		vm_matrix4_set_identity(&data->modelMatrix);
	} else {
		data->clipEnabled = 0;
		vm_matrix4_set_identity(&data->modelMatrix);
		data->clipEquation.a1d[0] = 0.0f;
		data->clipEquation.a1d[1] = 0.0f;
		data->clipEquation.a1d[2] = 0.0f;
		data->clipEquation.a1d[3] = 0.0f;
	}

	buffer.submitData();
	gr_bind_uniform_buffer(uniform_block_type::GenericData, buffer.getBufferOffset(0),
	                       sizeof(genericData_default_material_vert), buffer.bufferHandle());
}

} // anonymous namespace

int vulkan_zbuffer_get()
{
	auto* drawManager = getDrawManager();
	return drawManager->zbufferGet();
}

int vulkan_zbuffer_set(int mode)
{
	auto* drawManager = getDrawManager();
	return drawManager->zbufferSet(mode);
}

void vulkan_zbuffer_clear(int mode)
{
	auto* drawManager = getDrawManager();
	drawManager->zbufferClear(mode);
}

int vulkan_stencil_set(int mode)
{
	auto* drawManager = getDrawManager();
	return drawManager->stencilSet(mode);
}

void vulkan_stencil_clear()
{
	auto* drawManager = getDrawManager();
	drawManager->stencilClear();
}

void vulkan_set_fill_mode(int mode)
{
	auto* drawManager = getDrawManager();
	// GR_FILL_MODE_WIRE = 1, GR_FILL_MODE_SOLID = 2
	drawManager->setFillMode(mode);
}

void vulkan_clear()
{
	auto* drawManager = getDrawManager();
	drawManager->clear();
}

void vulkan_reset_clip()
{
	auto* drawManager = getDrawManager();
	drawManager->resetClip();
}

void vulkan_set_clear_color(int r, int g, int b)
{
	auto* drawManager = getDrawManager();
	drawManager->setClearColor(r, g, b);
}

void vulkan_set_clip(int x, int y, int w, int h, int resize_mode)
{
	auto* drawManager = getDrawManager();
	drawManager->setClip(x, y, w, h, resize_mode);
}

int vulkan_set_cull(int cull)
{
	auto* drawManager = getDrawManager();
	return drawManager->setCull(cull);
}

int vulkan_set_color_buffer(int mode)
{
	auto* drawManager = getDrawManager();
	return drawManager->setColorBuffer(mode);
}

void vulkan_set_texture_addressing(int mode)
{
	auto* drawManager = getDrawManager();
	drawManager->setTextureAddressing(mode);
}

void vulkan_set_line_width(float width)
{
	auto* stateTracker = getStateTracker();
	if (width <= 1.0f) {
		stateTracker->setLineWidth(width);
	}
	gr_screen.line_width = width;
}

void vulkan_clear_states()
{
	auto* drawManager = getDrawManager();
	drawManager->clearStates();
}

void vulkan_scene_texture_begin()
{
	auto* renderer = getRendererInstance();

	// Switch to HDR scene render pass when post-processing is enabled
	auto* pp = getPostProcessor();
	if (pp && pp->isInitialized() && Gr_post_processing_enabled && !PostProcessing_override) {
		renderer->beginSceneRendering();
		High_dynamic_range = true;
	} else {
		// Fallback: just clear within the current swap chain pass
		auto* stateTracker = getStateTracker();
		auto cmdBuffer = stateTracker->getCommandBuffer();

		vk::ClearAttachment clearAttachments[2];
		clearAttachments[0].aspectMask = vk::ImageAspectFlagBits::eColor;
		clearAttachments[0].colorAttachment = 0;
		clearAttachments[0].clearValue.color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});

		clearAttachments[1].aspectMask = vk::ImageAspectFlagBits::eDepth;
		clearAttachments[1].clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

		vk::ClearRect clearRect;
		clearRect.rect.offset = vk::Offset2D(0, 0);
		clearRect.rect.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.max_w),
		                                      static_cast<uint32_t>(gr_screen.max_h));
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;

		cmdBuffer.clearAttachments(2, clearAttachments, 1, &clearRect);
	}
}

void vulkan_scene_texture_end()
{
	auto* renderer = getRendererInstance();

	// If we were rendering to the HDR scene target, switch back to swap chain
	if (renderer->isSceneRendering()) {
		renderer->endSceneRendering();
	}

	High_dynamic_range = false;
}

void vulkan_copy_effect_texture()
{
	auto* renderer = getRendererInstance();

	// Only copy if we're actively rendering the HDR scene
	if (!renderer->isSceneRendering()) {
		return;
	}

	renderer->copyEffectTexture();
}

void vulkan_draw_sphere(material* material_def, float /*rad*/)
{
	auto* drawManager = getDrawManager();
	drawManager->drawSphere(material_def);
}

void vulkan_render_shield_impact(shield_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	gr_buffer_handle buffer_handle,
	int n_verts)
{
	auto* drawManager = getDrawManager();

	// Compute impact projection matrices
	float radius = material_info->get_impact_radius();
	vec3d min_v, max_v;
	min_v.xyz.x = min_v.xyz.y = min_v.xyz.z = -radius;
	max_v.xyz.x = max_v.xyz.y = max_v.xyz.z = radius;

	matrix4 impact_projection;
	vm_matrix4_set_orthographic(&impact_projection, &max_v, &min_v);

	matrix impact_orient = material_info->get_impact_orient();
	vec3d impact_pos = material_info->get_impact_pos();

	matrix4 impact_transform;
	vm_matrix4_set_inverse_transform(&impact_transform, &impact_orient, &impact_pos);

	// Set shield impact uniform data (GenericData UBO)
	auto buffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1,
	                                     sizeof(graphics::generic_data::shield_impact_data));
	auto* data = buffer.aligner().addTypedElement<graphics::generic_data::shield_impact_data>();
	data->hitNormal             = impact_orient.vec.fvec;
	data->shieldProjMatrix      = impact_projection;
	data->shieldModelViewMatrix = impact_transform;
	data->shieldMapIndex        = 0; // Vulkan binds textures individually, always layer 0
	data->srgb                  = High_dynamic_range ? 1 : 0;
	data->color                 = material_info->get_color();
	buffer.submitData();
	gr_bind_uniform_buffer(uniform_block_type::GenericData, buffer.getBufferOffset(0),
	                       sizeof(graphics::generic_data::shield_impact_data), buffer.bufferHandle());

	// Set matrix uniforms
	gr_matrix_set_uniforms();

	// Draw the shield mesh
	drawManager->renderPrimitives(material_info, prim_type, layout, 0, n_verts, buffer_handle, 0);
}

void vulkan_render_model(model_material* material_info,
	indexed_vertex_source* vert_source,
	vertex_buffer* bufferp,
	size_t texi)
{
	// ModelData UBO (matrices, lights, material params) is already bound by the model
	// rendering pipeline (model_draw_list::render_buffer) before this function is called.
	// Do NOT call vulkan_set_default_material_uniforms here - that would set GenericData
	// uniforms for SDR_TYPE_DEFAULT_MATERIAL, but models use SDR_TYPE_MODEL with ModelData.

	auto* drawManager = getDrawManager();
	drawManager->renderModel(material_info, vert_source, bufferp, texi);
}

void vulkan_render_primitives(material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle,
	size_t buffer_offset)
{
	// Set up uniform buffers before rendering (like OpenGL does)
	gr_matrix_set_uniforms();
	vulkan_set_default_material_uniforms(material_info);

	auto* drawManager = getDrawManager();
	drawManager->renderPrimitives(material_info, prim_type, layout, offset, n_verts, buffer_handle, buffer_offset);
}

void vulkan_render_primitives_particle(particle_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle)
{
	auto* renderer = getRendererInstance();
	auto* drawManager = getDrawManager();
	auto* pp = getPostProcessor();

	// In deferred mode, once the G-buffer pass has ended the position texture
	// (view-space XYZ) is in eShaderReadOnlyOptimal and free to sample.
	bool usePosTexture = light_deferred_enabled()
	                     && !renderer->isUsingGbufRenderPass()
	                     && pp && pp->isGbufInitialized();

	if (!usePosTexture) {
		// Non-deferred path: copy hardware depth buffer
		renderer->copySceneDepthForParticles();
	}

	// Set up matrices
	gr_matrix_set_uniforms();

	// Set effect_data GenericData UBO (matching OpenGL's opengl_tnl_set_material_particle)
	{
		auto buffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1,
		                                     sizeof(graphics::generic_data::effect_data));
		auto* data = buffer.aligner().addTypedElement<graphics::generic_data::effect_data>();

		data->window_width  = static_cast<float>(gr_screen.max_w);
		data->window_height = static_cast<float>(gr_screen.max_h);
		data->nearZ         = Min_draw_distance;
		data->farZ          = Max_draw_distance;
		data->srgb          = High_dynamic_range ? 1 : 0;
		data->blend_alpha   = material_info->get_blend_mode() != ALPHA_BLEND_ADDITIVE ? 1 : 0;
		// In deferred mode, bind the G-buffer position texture (view-space XYZ)
		// so linear_depth=1 reads .z directly (matches OpenGL behavior).
		// Otherwise use the NDC conversion path with the hardware depth copy.
		data->linear_depth  = usePosTexture ? 1 : 0;

		buffer.submitData();
		gr_bind_uniform_buffer(uniform_block_type::GenericData, buffer.getBufferOffset(0),
		                       sizeof(graphics::generic_data::effect_data), buffer.bufferHandle());
	}

	// Set depth texture override
	if (usePosTexture) {
		// Deferred path: bind G-buffer position texture directly
		auto* texMgr = getTextureManager();
		drawManager->setDepthTextureOverride(
			pp->getGbufPositionView(),
			texMgr->getSampler(vk::Filter::eNearest, vk::Filter::eNearest,
			                   vk::SamplerAddressMode::eClampToEdge, false, 0.0f, false));
	} else if (renderer->isSceneDepthCopied() && pp) {
		// Non-deferred path: bind the hardware depth copy
		auto* texMgr = getTextureManager();
		drawManager->setDepthTextureOverride(
			pp->getSceneDepthCopyView(),
			texMgr->getSampler(vk::Filter::eNearest, vk::Filter::eNearest,
			                   vk::SamplerAddressMode::eClampToEdge, false, 0.0f, false));
	}

	drawManager->renderPrimitivesParticle(material_info, prim_type, layout, offset, n_verts, buffer_handle);

	// Clear the override
	drawManager->clearDepthTextureOverride();
}

void vulkan_render_primitives_distortion(distortion_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle)
{
	auto* drawManager = getDrawManager();
	auto* pp = getPostProcessor();

	// Set up matrices
	gr_matrix_set_uniforms();

	// Set effect_distort_data GenericData UBO (16 bytes — NOT genericData_default_material_vert!)
	{
		auto buffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1,
		                                     sizeof(graphics::generic_data::effect_distort_data));
		auto* data = buffer.aligner().addTypedElement<graphics::generic_data::effect_distort_data>();

		data->window_width  = static_cast<float>(gr_screen.max_w);
		data->window_height = static_cast<float>(gr_screen.max_h);
		data->use_offset    = material_info->get_thruster_rendering() ? 1.0f : 0.0f;

		buffer.submitData();
		gr_bind_uniform_buffer(uniform_block_type::GenericData, buffer.getBufferOffset(0),
		                       sizeof(graphics::generic_data::effect_distort_data), buffer.bufferHandle());
	}

	// Set scene color override (binding 5) — snapshot of scene color for distortion sampling
	if (pp && pp->getSceneEffectView()) {
		drawManager->setSceneColorOverride(
			pp->getSceneEffectView(), pp->getSceneEffectSampler());
	}

	// Set distortion map override (binding 6) — ping-pong noise texture for thrusters
	if (material_info->get_thruster_rendering() && pp && pp->getDistortionTextureView()) {
		drawManager->setDistMapOverride(
			pp->getDistortionTextureView(), pp->getDistortionSampler());
	}

	drawManager->renderPrimitivesDistortion(material_info, prim_type, layout, n_verts, buffer_handle);

	// Clear overrides so subsequent draws use fallback textures
	drawManager->clearDistortionOverrides();
}

void vulkan_render_movie(movie_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int n_verts,
	gr_buffer_handle buffer,
	size_t buffer_offset)
{
	gr_matrix_set_uniforms();
	vulkan_set_default_material_uniforms(material_info);

	auto* drawManager = getDrawManager();
	drawManager->renderMovie(material_info, prim_type, layout, n_verts, buffer);
}

void vulkan_render_nanovg(nanovg_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle)
{
	// NanoVG shader reads from NanoVGData UBO (set 2 binding 2), not GenericData.
	// The NanoVGRenderer binds NanoVGData before calling gr_render_nanovg().

	// NanoVG uses its own software scissor (scissorMat/scissorExt in the fragment shader).
	// Disable hardware scissor to match nanovg_gl.h which calls glDisable(GL_SCISSOR_TEST).
	// Without this, NanoVG draws get clipped by gr_set_clip's hardware scissor.
	auto* stateTracker = getStateTracker();
	bool savedScissorEnabled = stateTracker->isScissorEnabled();
	stateTracker->setScissorEnabled(false);

	auto* drawManager = getDrawManager();
	drawManager->renderNanoVG(material_info, prim_type, layout, offset, n_verts, buffer_handle);

	// Restore scissor state
	stateTracker->setScissorEnabled(savedScissorEnabled);
}

void vulkan_render_primitives_batched(batched_bitmap_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle)
{
	gr_matrix_set_uniforms();
	vulkan_set_default_material_uniforms(material_info);

	auto* drawManager = getDrawManager();
	drawManager->renderPrimitivesBatched(material_info, prim_type, layout, offset, n_verts, buffer_handle);
}

void vulkan_render_rocket_primitives(interface_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int n_indices,
	gr_buffer_handle vertex_buffer,
	gr_buffer_handle index_buffer)
{
	// Set up 2D orthographic projection (matches OpenGL's gr_opengl_render_rocket_primitives)
	gr_set_2d_matrix();

	// Fill GenericData UBO with rocketui_data layout (NOT default material layout).
	// The rocketui shader reads projMatrix, offset, textured, baseMapIndex, and
	// horizontalSwipeOffset from GenericData — a completely different layout than
	// the default material shader's genericData.
	{
		auto buffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1,
		                                     sizeof(graphics::generic_data::rocketui_data));
		auto* data = buffer.aligner().addTypedElement<graphics::generic_data::rocketui_data>();

		data->projMatrix = gr_projection_matrix;

		const vec2d& offset = material_info->get_offset();
		data->offset = offset;
		data->textured = material_info->is_textured() ? 1 : 0;
		data->baseMapIndex = 0;  // Vulkan texture array: always layer 0
		data->horizontalSwipeOffset = material_info->get_horizontal_swipe();

		buffer.submitData();
		gr_bind_uniform_buffer(uniform_block_type::GenericData, buffer.getBufferOffset(0),
		                       sizeof(graphics::generic_data::rocketui_data), buffer.bufferHandle());
	}

	// Matrices UBO is still needed for descriptor set completeness
	gr_matrix_set_uniforms();

	auto* drawManager = getDrawManager();
	drawManager->renderRocketPrimitives(material_info, prim_type, layout, n_indices, vertex_buffer, index_buffer);

	gr_end_2d_matrix();
}

void vulkan_calculate_irrmap()
{
	if (ENVMAP < 0 || gr_screen.irrmap_render_target < 0) {
		return;
	}

	auto* renderer = getRendererInstance();
	auto* stateTracker = getStateTracker();
	auto* texManager = getTextureManager();
	auto* descManager = getDescriptorManager();
	auto* bufferManager = getBufferManager();
	auto* pipelineManager = getPipelineManager();
	auto* pp = getPostProcessor();

	if (!renderer || !stateTracker || !texManager || !descManager || !bufferManager || !pipelineManager || !pp) {
		return;
	}

	// Get envmap cubemap view
	auto* envSlot = bm_get_slot(ENVMAP, true);
	if (!envSlot || !envSlot->gr_info) {
		return;
	}
	auto* envTs = static_cast<tcache_slot_vulkan*>(envSlot->gr_info);
	vk::ImageView envmapView = envTs->isCubemap ? envTs->cubeImageView : envTs->imageView;
	if (!envmapView) {
		return;
	}

	// Get irrmap render target (cubemap with per-face framebuffers)
	auto* irrSlot = bm_get_slot(gr_screen.irrmap_render_target, true);
	if (!irrSlot || !irrSlot->gr_info) {
		return;
	}
	auto* irrTs = static_cast<tcache_slot_vulkan*>(irrSlot->gr_info);
	if (!irrTs->isCubemap || !irrTs->renderPass) {
		return;
	}

	vk::CommandBuffer cmd = stateTracker->getCommandBuffer();

	// End the current render pass (G-buffer or scene)
	cmd.endRenderPass();

	// Create pipeline for irradiance map generation
	PipelineConfig config;
	config.shaderType = SDR_TYPE_IRRADIANCE_MAP_GEN;
	config.vertexLayoutHash = 0;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = ALPHA_BLEND_NONE;
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = irrTs->renderPass;

	vertex_layout emptyLayout;
	vk::Pipeline pipeline = pipelineManager->getPipeline(config, emptyLayout);
	if (!pipeline) {
		mprintf(("vulkan_calculate_irrmap: Failed to get pipeline!\n"));
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineManager->getPipelineLayout();

	// Create a small host-visible UBO for the 6 face indices
	// minUniformBufferOffsetAlignment is typically 256 bytes
	const uint32_t UBO_SLOT_SIZE = 256;  // Safe alignment for all GPUs
	const uint32_t UBO_TOTAL_SIZE = 6 * UBO_SLOT_SIZE;

	vk::Device device = bufferManager->getDevice();
	auto* memManager = getMemoryManager();

	vk::BufferCreateInfo uboBufInfo;
	uboBufInfo.size = UBO_TOTAL_SIZE;
	uboBufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	uboBufInfo.sharingMode = vk::SharingMode::eExclusive;

	vk::Buffer faceUBO;
	VulkanAllocation faceUBOAlloc;
	try {
		faceUBO = device.createBuffer(uboBufInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("vulkan_calculate_irrmap: Failed to create face UBO: %s\n", e.what()));
		return;
	}

	if (!memManager->allocateBufferMemory(faceUBO, MemoryUsage::CpuToGpu, faceUBOAlloc)) {
		device.destroyBuffer(faceUBO);
		return;
	}

	// Map and write face indices
	auto* mapped = static_cast<uint8_t*>(memManager->mapMemory(faceUBOAlloc));
	if (!mapped) {
		device.destroyBuffer(faceUBO);
		memManager->freeAllocation(faceUBOAlloc);
		return;
	}
	memset(mapped, 0, UBO_TOTAL_SIZE);
	for (int i = 0; i < 6; i++) {
		*reinterpret_cast<int*>(mapped + i * UBO_SLOT_SIZE) = i;
	}
	memManager->unmapMemory(faceUBOAlloc);

	// Get fallback resources
	vk::Buffer fallbackUBO = bufferManager->getFallbackUniformBuffer();
	vk::DeviceSize fallbackUBOSize = static_cast<vk::DeviceSize>(bufferManager->getFallbackUniformBufferSize());
	vk::Sampler defaultSampler = texManager->getDefaultSampler();
	vk::ImageView fallbackView = texManager->getFallback2DArrayView();
	vk::ImageView fallbackView2D = texManager->getFallbackTextureView2D();
	vk::ImageView fallbackCubeView = texManager->getFallbackCubeView();

	vk::Extent2D irrExtent(irrTs->width, irrTs->height);

	for (int face = 0; face < 6; face++) {
		vk::Framebuffer fb = irrTs->cubeFaceFramebuffers[face];
		if (!fb) {
			continue;
		}

		// Begin render pass for this face (loadOp=eClear, finalLayout=eShaderReadOnlyOptimal)
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = irrTs->renderPass;
		rpBegin.framebuffer = fb;
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = irrExtent;

		vk::ClearValue clearValue;
		clearValue.color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		rpBegin.clearValueCount = 1;
		rpBegin.pClearValues = &clearValue;

		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

		// Set viewport and scissor
		vk::Viewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(irrExtent.width);
		viewport.height = static_cast<float>(irrExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd.setViewport(0, viewport);

		vk::Rect2D scissor;
		scissor.offset = vk::Offset2D(0, 0);
		scissor.extent = irrExtent;
		cmd.setScissor(0, scissor);

		DescriptorWriter writer;
		writer.reset(device);

		// Set 0: Global (all fallback)
		vk::DescriptorSet globalSet = descManager->allocateFrameSet(DescriptorSetIndex::Global);
		Verify(globalSet);
		writer.writeUniformBuffer(globalSet, 0, fallbackUBO, 0, fallbackUBOSize);
		writer.writeUniformBuffer(globalSet, 1, fallbackUBO, 0, fallbackUBOSize);
		writer.writeTexture(globalSet, 2, fallbackView, defaultSampler);
		writer.writeTexture(globalSet, 3, fallbackCubeView, defaultSampler);
		writer.writeTexture(globalSet, 4, fallbackCubeView, defaultSampler);
		writer.flush();

		// Set 1: Material (envmap cubemap at binding 1)
		vk::DescriptorSet materialSet = descManager->allocateFrameSet(DescriptorSetIndex::Material);
		Verify(materialSet);
		writer.writeUniformBuffer(materialSet, 0, fallbackUBO, 0, fallbackUBOSize);
		writer.writeUniformBuffer(materialSet, 2, fallbackUBO, 0, fallbackUBOSize);
		writer.writeStorageBuffer(materialSet, 3, fallbackUBO, 0, fallbackUBOSize);

		// Binding 1: envmap cubemap (element 0) + fallback for rest of array
		{
			std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texImages;
			texImages[0].sampler = defaultSampler;
			texImages[0].imageView = envmapView;
			texImages[0].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			for (uint32_t slot = 1; slot < VulkanDescriptorManager::MAX_TEXTURE_BINDINGS; ++slot) {
				texImages[slot].sampler = defaultSampler;
				texImages[slot].imageView = fallbackView2D;
				texImages[slot].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			}
			writer.writeTextureArray(materialSet, 1, texImages.data(), static_cast<uint32_t>(texImages.size()));
		}
		writer.writeTexture(materialSet, 4, fallbackView2D, defaultSampler);
		writer.writeTexture(materialSet, 5, fallbackView2D, defaultSampler);
		writer.writeTexture(materialSet, 6, fallbackView2D, defaultSampler);
		writer.flush();

		// Set 2: PerDraw (face UBO at binding 0)
		vk::DescriptorSet perDrawSet = descManager->allocateFrameSet(DescriptorSetIndex::PerDraw);
		Verify(perDrawSet);
		writer.writeUniformBuffer(perDrawSet, 0, faceUBO,
			static_cast<vk::DeviceSize>(face) * UBO_SLOT_SIZE, UBO_SLOT_SIZE);
		for (uint32_t b = 1; b <= 4; ++b) {
			writer.writeUniformBuffer(perDrawSet, b, fallbackUBO, 0, fallbackUBOSize);
		}
		writer.flush();

		// Bind all descriptor sets
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
			0, {globalSet, materialSet, perDrawSet}, {});

		// Draw fullscreen triangle
		cmd.draw(3, 1, 0, 0);
		cmd.endRenderPass();
	}

	// Queue UBO for deferred destruction (safe to destroy after frame submission)
	getDeletionQueue()->queueBuffer(faceUBO, faceUBOAlloc);

	// Resume the scene/G-buffer render pass
	bool useGbuf = renderer->isSceneRendering() && pp->isGbufInitialized() && light_deferred_enabled();
	if (useGbuf) {
		// Transition G-buffer attachments for resume
		{
			vk::ImageMemoryBarrier barrier;
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = pp->getSceneColorImage();
			barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

			cmd.pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				{}, nullptr, nullptr, barrier);
		}

		pp->transitionGbufForResume(cmd);

		auto extent = pp->getSceneExtent();
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = pp->getGbufRenderPassLoad();
		rpBegin.framebuffer = pp->getGbufFramebuffer();
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = extent;

		std::array<vk::ClearValue, 7> clearValues{};
		clearValues[6].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();

		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		stateTracker->setRenderPass(pp->getGbufRenderPassLoad(), 0);
		stateTracker->setColorAttachmentCount(VulkanPostProcessor::GBUF_COLOR_ATTACHMENT_COUNT);
	} else {
		// Resume simple scene render pass
		auto extent = pp->getSceneExtent();
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = pp->getSceneRenderPassLoad();
		rpBegin.framebuffer = pp->getSceneFramebuffer();
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = extent;

		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();

		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		stateTracker->setRenderPass(pp->getSceneRenderPassLoad(), 0);
	}

	// Restore viewport and scissor
	{
		vk::Viewport viewport;
		viewport.x = static_cast<float>(gr_screen.offset_x);
		viewport.y = static_cast<float>(gr_screen.offset_y);
		viewport.width = static_cast<float>(gr_screen.clip_width);
		viewport.height = static_cast<float>(gr_screen.clip_height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd.setViewport(0, viewport);

		vk::Rect2D scissor;
		scissor.offset = vk::Offset2D(gr_screen.offset_x, gr_screen.offset_y);
		scissor.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.clip_width),
		                               static_cast<uint32_t>(gr_screen.clip_height));
		cmd.setScissor(0, scissor);
	}

	mprintf(("vulkan_calculate_irrmap: Generated irradiance cubemap (%ux%u)\n", irrTs->width, irrTs->height));
}

} // namespace vulkan
} // namespace graphics
