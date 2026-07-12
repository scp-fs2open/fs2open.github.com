#pragma once

#include "globalincs/pstypes.h"

#include "VulkanPipeline.h"

#include "graphics/2d.h"
#include "graphics/material.h"

#include <vulkan/vulkan.hpp>

#include <array>

namespace graphics::vulkan {

class DescriptorWriter;

/**
 * @brief Tracks a pending uniform buffer binding
 * Stores handle instead of raw vk::Buffer to survive buffer recreation.
 * The offset is fully resolved at bind time (includes frame base offset)
 * to prevent stale lastWriteStreamOffset if the buffer is updated between bind and draw.
 */
struct PendingUniformBinding {
	gr_buffer_handle bufferHandle; // FSO buffer handle - lookup vk::Buffer at draw time
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
	static void setClearColor(int r, int g, int b);

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
	int zbufferGet() const;

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
	void renderPrimitives(material* material_info,
		primitive_type prim_type,
		vertex_layout* layout,
		int offset,
		int n_verts,
		gr_buffer_handle buffer_handle,
		size_t buffer_offset);

	/**
	 * @brief Render batched bitmaps
	 */
	void renderPrimitivesBatched(batched_bitmap_material* material_info,
		primitive_type prim_type,
		vertex_layout* layout,
		int offset,
		int n_verts,
		gr_buffer_handle buffer_handle);

	/**
	 * @brief Render particles
	 */
	void renderPrimitivesParticle(particle_material* material_info,
		primitive_type prim_type,
		vertex_layout* layout,
		int offset,
		int n_verts,
		gr_buffer_handle buffer_handle);

	/**
	 * @brief Render distortion effect
	 */
	void renderPrimitivesDistortion(distortion_material* material_info,
		primitive_type prim_type,
		vertex_layout* layout,
		int offset,
		int n_verts,
		gr_buffer_handle buffer_handle);

	/**
	 * @brief Render movie frame
	 */
	void renderMovie(movie_material* material_info,
		primitive_type prim_type,
		vertex_layout* layout,
		int n_verts,
		gr_buffer_handle buffer_handle,
		size_t buffer_offset);

	/**
	 * @brief Render NanoVG UI
	 */
	void renderNanoVG(nanovg_material* material_info,
		primitive_type prim_type,
		vertex_layout* layout,
		int offset,
		int n_verts,
		gr_buffer_handle buffer_handle);

	/**
	 * @brief Render Rocket UI primitives (indexed)
	 */
	void renderRocketPrimitives(interface_material* material_info,
		primitive_type prim_type,
		vertex_layout* layout,
		int n_indices,
		gr_buffer_handle vertex_buffer,
		gr_buffer_handle index_buffer);

	/**
	 * @brief Render 3D model with indexed geometry
	 * @param material_info Model material settings
	 * @param vert_source Indexed vertex source with buffer handles
	 * @param bufferp Vertex buffer with layout and texture info
	 * @param texi Index into tex_buf array for this draw
	 */
	void
	renderModel(model_material* material_info, indexed_vertex_source* vert_source, vertex_buffer* bufferp, size_t texi);

	/**
	 * @brief Render one cascaded-shadow-map draw call (shadow_render_list)
	 *
	 * Depth-only, instanced once per active shadow cascade (Shadow_cascade_count),
	 * routed via gl_InstanceIndex -> gl_Layer in the vertex shader. Unlike
	 * renderModel(), this does not go through a model_material - the shadow
	 * pipeline/state is fixed and the only per-draw data is the shadowMapData UBO.
	 */
	void renderShadowDraw(gr_buffer_handle ubo_handle,
		size_t ubo_offset,
		size_t ubo_size,
		vertex_buffer* buffer,
		indexed_vertex_source* vert_src,
		size_t texi) const;

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
	void setDepthTextureOverride(vk::DescriptorImageInfo info);

	/**
	 * @brief Clear depth texture override (reverts to fallback)
	 */
	void clearDepthTextureOverride();

	/**
	 * @brief Set scene color texture override for binding 5 (distortion effects)
	 */
	void setSceneColorOverride(vk::DescriptorImageInfo info);

	/**
	 * @brief Set distortion map texture override for binding 6 (distortion effects)
	 */
	void setDistMapOverride(vk::DescriptorImageInfo info);

	/**
	 * @brief Clear distortion texture overrides (bindings 5 and 6, reverts to fallback)
	 */
	void clearDistortionOverrides();

	/**
	 * @brief Drop cached state that may reference destroyed views (swap chain recreation)
	 */
	void onResize();

	/**
	 * @brief Invalidate the memoized per-frame Global (Set 0) descriptor set
	 *
	 * Call whenever a Global-set input changes outside applyMaterial's own tracking
	 * (currently: the shadow TLAS, via setCurrentShadowTlas). Forces the next
	 * applyMaterial() to rebuild + rewrite Set 0.
	 */
	void invalidateGlobalSet() { m_globalSetDirty = true; }

	/**
	 * @brief Get current texture addressing mode
	 */
	int getTextureAddressing() const
	{
		return m_textureAddressing;
	}

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
	void setPendingUniformBinding(uniform_block_type blockType,
		gr_buffer_handle bufferHandle,
		vk::DeviceSize offset,
		vk::DeviceSize size);

	/**
	 * @brief Clear all pending uniform bindings
	 */
	void clearPendingUniformBindings();

	/**
	 * @brief Get a pending uniform binding by block type index
	 */
	const PendingUniformBinding& getPendingUniformBinding(size_t index) const
	{
		Assertion(index < NUM_UNIFORM_BLOCK_TYPES, "getPendingUniformBinding: index %zu out of range!", index);
		return m_pendingUniformBindings[index];
	}

	/**
	 * @brief Bind material textures to descriptor set (public for decal rendering)
	 * @param writer Texture array is written into the writer's current set via setImageArray
	 */
	bool bindMaterialTextures(material* mat, DescriptorWriter* writer) const;

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
	 * @brief Shared implementation for the non-indexed renderPrimitives* variants
	 *
	 * Applies the material/pipeline, binds the vertex buffer, and issues a
	 * non-indexed draw. The only per-variant differences are the optional frame
	 * stat counter and the concrete material subtype (passed as material*).
	 *
	 * @param statCounter Optional per-variant FrameStats counter to increment (may be null)
	 */
	void renderPrimitivesCommon(material* material_info,
		primitive_type prim_type,
		vertex_layout* layout,
		int offset,
		int n_verts,
		gr_buffer_handle buffer_handle,
		size_t buffer_offset,
		int* statCounter);

	/**
	 * @brief Apply material state and bind pipeline
	 * @return true if pipeline was successfully bound
	 */
	bool applyMaterial(material* mat, primitive_type prim_type, vertex_layout* layout);

	/**
	 * @brief Build pipeline config from material
	 */
	PipelineConfig buildPipelineConfig(material* mat, primitive_type prim_type) const;

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

		// On-demand texture uploads triggered mid-frame from bindMaterialTextures
		// (B6): a texture referenced by a material that wasn't resident, so it had
		// to be locked + uploaded during scene recording. Should be ~0 in steady
		// state (level preload covers residency); sustained nonzero counts flag a
		// preload gap worth chasing.
		int onDemandTextureUploads = 0;
	};
	// Mutable so const paths (bindMaterialTextures) can bump instrumentation counters.
	mutable FrameStats m_frameStats;
	int m_frameStatsFrameNum = 0;

	// ---- Per-frame Global (Set 0) descriptor memoization (B1) ----
	// applyMaterial() rebuilt + rebound the Global set on every draw even though
	// its inputs change at most a few times per frame. Cache it and rebuild only
	// when a Global input actually changes. Dynamic Global inputs are: the three
	// pending Global UBOs (Lights/DeferredGlobals/ShadowCascadeParams, via
	// setPendingUniformBinding), the shadow-map image (shadow lazy-init), and the
	// TLAS (setCurrentShadowTlas). EnvMap/IrradianceMap are permanent dummy
	// fallbacks here (real env/irr live in the deferred pass's own set), so they
	// need no invalidation. The set is pool-allocated, so it is dropped every
	// frame (resetFrameStats) when the frame pool is reset.
	vk::DescriptorSet m_cachedGlobalSet = nullptr;
	bool m_globalSetDirty = true;
	bool m_cachedGlobalHadShadow = false; // shadow-init state the cached set was built with

	// ---- Material (Set 1) previous-set memoization (B1) ----
	// Reuse the previous draw's Material set when EVERY input is unchanged
	// (previous-only cache: rebuilt on any difference, so it is never stale).
	// Hits on batched same-material runs (UI text glyphs, a model's submodels,
	// batched effects) that also share a transform upload. Skips a frame-set
	// allocation, a template write, the material texture resolution (incl. its
	// on-demand upload side effect — safe to skip only because identical inputs
	// mean the textures were already resolved on the previous draw), and the
	// override/UBO writes.
	struct MaterialSetInputs {
		int texHandles[7] = {-1, -1, -1, -1, -1, -1, -1};
		int textureAddressing = -1;
		vk::Buffer transformBuffer = nullptr;
		size_t transformOffset = 0;
		size_t transformSize = 0;
		vk::DescriptorImageInfo depthInfo;
		vk::DescriptorImageInfo sceneColorInfo;
		vk::DescriptorImageInfo distMapInfo;
		// Pending Material UBOs (index 0 = ModelData, 1 = DecalGlobals)
		int uboHandle[2] = {-1, -1};
		vk::DeviceSize uboOffset[2] = {0, 0};
		vk::DeviceSize uboSize[2] = {0, 0};
		bool uboValid[2] = {false, false};

		static bool imgEq(const vk::DescriptorImageInfo& a, const vk::DescriptorImageInfo& b)
		{
			return a.sampler == b.sampler && a.imageView == b.imageView && a.imageLayout == b.imageLayout;
		}
		bool operator==(const MaterialSetInputs& o) const
		{
			for (int i = 0; i < 7; ++i) {
				if (texHandles[i] != o.texHandles[i]) return false;
			}
			for (int i = 0; i < 2; ++i) {
				if (uboHandle[i] != o.uboHandle[i] || uboOffset[i] != o.uboOffset[i] ||
				    uboSize[i] != o.uboSize[i] || uboValid[i] != o.uboValid[i]) return false;
			}
			return textureAddressing == o.textureAddressing && transformBuffer == o.transformBuffer &&
			       transformOffset == o.transformOffset && transformSize == o.transformSize &&
			       imgEq(depthInfo, o.depthInfo) && imgEq(sceneColorInfo, o.sceneColorInfo) &&
			       imgEq(distMapInfo, o.distMapInfo);
		}
		bool operator!=(const MaterialSetInputs& o) const { return !(*this == o); }
	};
	vk::DescriptorSet m_cachedMaterialSet = nullptr;
	MaterialSetInputs m_cachedMaterialInputs;
	bool m_cachedMaterialValid = false;

	// ---- PerDraw (Set 2) previous-set memoization (B1) ----
	// PerDraw holds only the pending PerDraw UBO bindings (GenericData, Matrices,
	// NanoVGData, DecalInfo, MovieData). In 3D these change per draw (no reuse);
	// the win is UI/2D runs that share them. Previous-only cache, same contract
	// as Material.
	static constexpr int NUM_PERDRAW_UBOS = 5;
	struct PerDrawSetInputs {
		int uboHandle[NUM_PERDRAW_UBOS] = {-1, -1, -1, -1, -1};
		vk::DeviceSize uboOffset[NUM_PERDRAW_UBOS] = {0, 0, 0, 0, 0};
		vk::DeviceSize uboSize[NUM_PERDRAW_UBOS] = {0, 0, 0, 0, 0};
		bool uboValid[NUM_PERDRAW_UBOS] = {false, false, false, false, false};
		bool operator==(const PerDrawSetInputs& o) const
		{
			for (int i = 0; i < NUM_PERDRAW_UBOS; ++i) {
				if (uboHandle[i] != o.uboHandle[i] || uboOffset[i] != o.uboOffset[i] ||
				    uboSize[i] != o.uboSize[i] || uboValid[i] != o.uboValid[i]) return false;
			}
			return true;
		}
	};
	vk::DescriptorSet m_cachedPerDrawSet = nullptr;
	PerDrawSetInputs m_cachedPerDrawInputs;
	bool m_cachedPerDrawValid = false;

	// Texture overrides for material bindings 4-6.
	vk::DescriptorImageInfo m_depthTextureInfo; // binding 4: depth/position for soft particles
	vk::DescriptorImageInfo m_sceneColorInfo;   // binding 5: scene color for distortion
	vk::DescriptorImageInfo m_distMapInfo;      // binding 6: distortion map

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
void vulkan_render_shield_impact(shield_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	gr_buffer_handle buffer_handle,
	int n_verts);
void vulkan_render_model(model_material* material_info,
	indexed_vertex_source* vert_source,
	vertex_buffer* bufferp,
	size_t texi);
void vulkan_render_shadow_draw(gr_buffer_handle ubo_handle,
	size_t ubo_offset,
	size_t ubo_size,
	vertex_buffer* buffer,
	indexed_vertex_source* vert_src,
	size_t texi);
void vulkan_render_primitives(material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle,
	size_t buffer_offset);
void vulkan_render_primitives_particle(particle_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle);
void vulkan_render_primitives_distortion(distortion_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle);
void vulkan_render_primitives_batched(batched_bitmap_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle);
void vulkan_render_movie(movie_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int n_verts,
	gr_buffer_handle buffer,
	size_t buffer_offset);
void vulkan_render_nanovg(nanovg_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle);
void vulkan_render_rocket_primitives(interface_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int n_indices,
	gr_buffer_handle vertex_buffer,
	gr_buffer_handle index_buffer);

// Transform buffer for batched submodel rendering
void vulkan_update_transform_buffer(void* data, size_t size);

// Environment mapping
void vulkan_calculate_irrmap();

} // namespace graphics::vulkan
