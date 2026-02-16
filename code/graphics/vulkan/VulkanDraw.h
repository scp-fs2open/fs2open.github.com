#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/material.h"
#include "VulkanPipeline.h"

#include <vulkan/vulkan.hpp>
#include <array>

namespace graphics {
namespace vulkan {

class DescriptorWriter;

/**
 * @brief Tracks a pending uniform buffer binding
 * Stores handle instead of raw vk::Buffer to survive buffer recreation.
 * The offset is fully resolved at bind time (includes frame base offset)
 * to prevent stale lastWriteStreamOffset if the buffer is updated between bind and draw.
 */
struct PendingUniformBinding {
	gr_buffer_handle bufferHandle;  // FSO buffer handle - lookup vk::Buffer at draw time
	vk::DeviceSize offset = 0;     // Fully resolved offset (frame base + caller offset)
	vk::DeviceSize size = 0;
	bool valid = false;
};

/**
 * @brief Handles Vulkan draw command recording
 *
 * Provides functions to record draw commands to the command buffer,
 * including primitive rendering, batched rendering, and special effects.
 */
class VulkanDrawManager {
public:
	VulkanDrawManager() = default;
	~VulkanDrawManager() = default;

	// Non-copyable
	VulkanDrawManager(const VulkanDrawManager&) = delete;
	VulkanDrawManager& operator=(const VulkanDrawManager&) = delete;

	/**
	 * @brief Initialize draw manager
	 */
	bool init(vk::Device device);

	/**
	 * @brief Shutdown and release resources
	 */
	void shutdown();

	// ========== Clear Operations ==========

	/**
	 * @brief Clear the color buffer
	 */
	void clear();

	/**
	 * @brief Set clear color
	 */
	void setClearColor(int r, int g, int b);

	// ========== Clipping ==========

	/**
	 * @brief Set clip region (scissor)
	 */
	void setClip(int x, int y, int w, int h, int resize_mode);

	/**
	 * @brief Reset clip to full screen
	 */
	void resetClip();

	// ========== Z-Buffer ==========

	/**
	 * @brief Get current zbuffer mode
	 */
	int zbufferGet();

	/**
	 * @brief Set zbuffer mode
	 * @return Previous mode
	 */
	int zbufferSet(int mode);

	/**
	 * @brief Clear zbuffer
	 */
	void zbufferClear(int mode);

	// ========== Stencil ==========

	/**
	 * @brief Set stencil mode
	 * @return Previous mode
	 */
	int stencilSet(int mode);

	/**
	 * @brief Clear stencil buffer
	 */
	void stencilClear();

	// ========== Culling ==========

	/**
	 * @brief Set cull mode
	 * @return Previous mode
	 */
	int setCull(int cull);

	// ========== Primitive Rendering ==========

	/**
	 * @brief Render primitives with material
	 */
	void renderPrimitives(material* material_info, primitive_type prim_type,
	                      vertex_layout* layout, int offset, int n_verts,
	                      gr_buffer_handle buffer_handle, size_t buffer_offset);

	/**
	 * @brief Render batched bitmaps
	 */
	void renderPrimitivesBatched(batched_bitmap_material* material_info,
	                             primitive_type prim_type, vertex_layout* layout,
	                             int offset, int n_verts, gr_buffer_handle buffer_handle);

	/**
	 * @brief Render particles
	 */
	void renderPrimitivesParticle(particle_material* material_info,
	                              primitive_type prim_type, vertex_layout* layout,
	                              int offset, int n_verts, gr_buffer_handle buffer_handle);

	/**
	 * @brief Render distortion effect
	 */
	void renderPrimitivesDistortion(distortion_material* material_info,
	                                primitive_type prim_type, vertex_layout* layout,
	                                int n_verts, gr_buffer_handle buffer_handle);

	/**
	 * @brief Render movie frame
	 */
	void renderMovie(movie_material* material_info, primitive_type prim_type,
	                 vertex_layout* layout, int n_verts, gr_buffer_handle buffer_handle);

	/**
	 * @brief Render NanoVG UI
	 */
	void renderNanoVG(nanovg_material* material_info, primitive_type prim_type,
	                  vertex_layout* layout, int offset, int n_verts,
	                  gr_buffer_handle buffer_handle);

	/**
	 * @brief Render Rocket UI primitives (indexed)
	 */
	void renderRocketPrimitives(interface_material* material_info,
	                            primitive_type prim_type, vertex_layout* layout,
	                            int n_indices, gr_buffer_handle vertex_buffer,
	                            gr_buffer_handle index_buffer);

	/**
	 * @brief Render 3D model with indexed geometry
	 * @param material_info Model material settings
	 * @param vert_source Indexed vertex source with buffer handles
	 * @param bufferp Vertex buffer with layout and texture info
	 * @param texi Index into tex_buf array for this draw
	 */
	void renderModel(model_material* material_info, indexed_vertex_source* vert_source,
	                 vertex_buffer* bufferp, size_t texi);

	/**
	 * @brief Draw a unit sphere with the given material
	 * Used for debug visualization and deferred light volumes
	 */
	void drawSphere(material* material_def);

	// ========== Render State ==========

	/**
	 * @brief Set polygon fill mode (GR_FILL_MODE_SOLID / GR_FILL_MODE_WIRE)
	 */
	void setFillMode(int mode);

	/**
	 * @brief Set color buffer write enable
	 * @return Previous state (1 = was enabled, 0 = was disabled)
	 */
	int setColorBuffer(int mode);

	/**
	 * @brief Set texture addressing mode (TMAP_ADDRESS_WRAP/MIRROR/CLAMP)
	 */
	void setTextureAddressing(int mode);

	/**
	 * @brief Enable or disable depth bias in pipeline
	 */
	void setDepthBiasEnabled(bool enabled);

	/**
	 * @brief Set depth texture override for soft particle rendering
	 *
	 * When set, applyMaterial() binds this texture to Material set binding 4
	 * instead of the fallback white texture. Must be set before the render call
	 * and cleared afterwards.
	 */
	void setDepthTextureOverride(vk::ImageView view, vk::Sampler sampler);

	/**
	 * @brief Clear depth texture override (reverts to fallback)
	 */
	void clearDepthTextureOverride();

	/**
	 * @brief Set scene color texture override for binding 5 (distortion effects)
	 */
	void setSceneColorOverride(vk::ImageView view, vk::Sampler sampler);

	/**
	 * @brief Set distortion map texture override for binding 6 (distortion effects)
	 */
	void setDistMapOverride(vk::ImageView view, vk::Sampler sampler);

	/**
	 * @brief Clear distortion texture overrides (bindings 5 and 6, reverts to fallback)
	 */
	void clearDistortionOverrides();

	/**
	 * @brief Get current texture addressing mode
	 */
	int getTextureAddressing() const { return m_textureAddressing; }

	/**
	 * @brief Clear all graphics states to defaults
	 */
	void clearStates();

	// ========== Uniform Buffers ==========

	/**
	 * @brief Set a pending uniform buffer binding
	 * @param blockType The uniform block type
	 * @param bufferHandle The FSO buffer handle (looked up at bind time)
	 * @param offset Offset within the buffer
	 * @param size Size of the bound range
	 */
	void setPendingUniformBinding(uniform_block_type blockType, gr_buffer_handle bufferHandle,
	                              vk::DeviceSize offset, vk::DeviceSize size);

	/**
	 * @brief Clear all pending uniform bindings
	 */
	void clearPendingUniformBindings();

	/**
	 * @brief Get a pending uniform binding by block type index
	 */
	const PendingUniformBinding& getPendingUniformBinding(size_t index) const {
		Assertion(index < NUM_UNIFORM_BLOCK_TYPES, "getPendingUniformBinding: index %zu out of range!", index);
		return m_pendingUniformBindings[index];
	}

	/**
	 * @brief Bind material textures to descriptor set (public for decal rendering)
	 * @param writer If non-null, texture array write is batched into writer instead of flushed immediately
	 */
	bool bindMaterialTextures(material* mat, vk::DescriptorSet materialSet,
	                          DescriptorWriter* writer);

	/**
	 * @brief Reset per-frame diagnostic counters (called at start of frame)
	 */
	void resetFrameStats();

	/**
	 * @brief Print per-frame diagnostic summary (called at end of frame)
	 */
	void printFrameStats();

private:
	/**
	 * @brief Apply material state and bind pipeline
	 * @return true if pipeline was successfully bound
	 */
	bool applyMaterial(material* mat, primitive_type prim_type, vertex_layout* layout);

	/**
	 * @brief Build pipeline config from material
	 */
	PipelineConfig buildPipelineConfig(material* mat, primitive_type prim_type);

	/**
	 * @brief Bind vertex buffer from handle
	 */
	void bindVertexBuffer(gr_buffer_handle handle, size_t offset = 0);

	/**
	 * @brief Bind index buffer from handle
	 */
	void bindIndexBuffer(gr_buffer_handle handle);

	/**
	 * @brief Issue draw call
	 */
	void draw(primitive_type prim_type, int first_vertex, int vertex_count);

	/**
	 * @brief Issue indexed draw call
	 */
	void drawIndexed(primitive_type prim_type, int index_count, int first_index, int vertex_offset);

	/**
	 * @brief Create sphere VBO/IBO from shared mesh generator
	 */
	void initSphereBuffers();

	/**
	 * @brief Destroy sphere VBO/IBO
	 */
	void shutdownSphereBuffers();

	vk::Device m_device;

	// Current render state
	int m_zbufferMode = GR_ZBUFF_FULL;
	int m_stencilMode = GR_STENCIL_NONE;
	bool m_cullEnabled = true;
	int m_fillMode = GR_FILL_MODE_SOLID;
	bool m_colorBufferEnabled = true;
	int m_textureAddressing = TMAP_ADDRESS_WRAP;
	bool m_depthBiasEnabled = false;

	// Pending uniform buffer bindings (indexed by uniform_block_type)
	static constexpr size_t NUM_UNIFORM_BLOCK_TYPES = static_cast<size_t>(uniform_block_type::NUM_BLOCK_TYPES);
	std::array<PendingUniformBinding, NUM_UNIFORM_BLOCK_TYPES> m_pendingUniformBindings;

	// Per-frame diagnostic counters
	struct FrameStats {
		int drawCalls = 0;
		int drawIndexedCalls = 0;
		int applyMaterialCalls = 0;
		int applyMaterialFailures = 0;
		int noPipelineSkips = 0;
		int shaderHandleNeg1 = 0;
		int totalVertices = 0;
		int totalIndices = 0;

		// Per-function call counters
		int renderPrimitiveCalls = 0;
		int renderBatchedCalls = 0;
		int renderModelCalls = 0;
		int renderParticleCalls = 0;
		int renderNanoVGCalls = 0;
		int renderRocketCalls = 0;
		int renderMovieCalls = 0;
	};
	FrameStats m_frameStats;
	int m_frameStatsFrameNum = 0;

	// Depth texture override for soft particle rendering
	// Set before applyMaterial() so binding 4 gets the real depth texture instead of fallback
	vk::ImageView m_depthTextureOverride;
	vk::Sampler m_depthSamplerOverride;

	// Scene color override for distortion rendering (binding 5)
	vk::ImageView m_sceneColorOverride;
	vk::Sampler m_sceneColorSamplerOverride;

	// Distortion map override for distortion rendering (binding 6)
	vk::ImageView m_distMapOverride;
	vk::Sampler m_distMapSamplerOverride;

	// Pre-built sphere mesh for draw_sphere / deferred light volumes
	gr_buffer_handle m_sphereVBO;
	gr_buffer_handle m_sphereIBO;
	unsigned int m_sphereIndexCount = 0;
	vertex_layout m_sphereVertexLayout;

	bool m_initialized = false;
};

// Global draw manager access
VulkanDrawManager* getDrawManager();
void setDrawManager(VulkanDrawManager* manager);

// ========== gr_screen function pointer implementations ==========
// These free functions implement gr_screen.gf_* function pointers.
// They are assigned in gr_vulkan.cpp::init_function_pointers().

// Clear operations
void vulkan_clear();
void vulkan_set_clear_color(int r, int g, int b);

// Clipping
void vulkan_set_clip(int x, int y, int w, int h, int resize_mode);
void vulkan_reset_clip();

// Z-buffer
int vulkan_zbuffer_get();
int vulkan_zbuffer_set(int mode);
void vulkan_zbuffer_clear(int mode);

// Stencil
int vulkan_stencil_set(int mode);
void vulkan_stencil_clear();

// Render state
int vulkan_set_cull(int cull);
int vulkan_set_color_buffer(int mode);
void vulkan_set_fill_mode(int mode);
void vulkan_set_texture_addressing(int mode);
void vulkan_set_line_width(float width);
void vulkan_clear_states();

// Scene texture
void vulkan_scene_texture_begin();
void vulkan_scene_texture_end();
void vulkan_copy_effect_texture();

// 3D primitives
void vulkan_draw_sphere(material* material_def, float rad);
void vulkan_render_shield_impact(shield_material* material_info, primitive_type prim_type,
	vertex_layout* layout, gr_buffer_handle buffer_handle, int n_verts);
void vulkan_render_model(model_material* material_info, indexed_vertex_source* vert_source,
	vertex_buffer* bufferp, size_t texi);
void vulkan_render_primitives(material* material_info, primitive_type prim_type,
	vertex_layout* layout, int offset, int n_verts, gr_buffer_handle buffer_handle, size_t buffer_offset);
void vulkan_render_primitives_particle(particle_material* material_info,
	primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, gr_buffer_handle buffer_handle);
void vulkan_render_primitives_distortion(distortion_material* material_info,
	primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, gr_buffer_handle buffer_handle);
void vulkan_render_primitives_batched(batched_bitmap_material* material_info,
	primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, gr_buffer_handle buffer_handle);
void vulkan_render_movie(movie_material* material_info, primitive_type prim_type,
	vertex_layout* layout, int n_verts, gr_buffer_handle buffer, size_t buffer_offset);
void vulkan_render_nanovg(nanovg_material* material_info, primitive_type prim_type,
	vertex_layout* layout, int offset, int n_verts, gr_buffer_handle buffer_handle);
void vulkan_render_rocket_primitives(interface_material* material_info,
	primitive_type prim_type, vertex_layout* layout, int n_indices,
	gr_buffer_handle vertex_buffer, gr_buffer_handle index_buffer);

// Transform buffer for batched submodel rendering
void vulkan_update_transform_buffer(void* data, size_t size);

// Environment mapping
void vulkan_calculate_irrmap();

} // namespace vulkan
} // namespace graphics
