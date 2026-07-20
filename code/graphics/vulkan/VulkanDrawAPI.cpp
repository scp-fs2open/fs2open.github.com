#include "VulkanDraw.h"

#include <algorithm>
#include <array>

#include "VulkanState.h"
#include "VulkanBarrier.h"
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

// PostProcessing_override is defined in globalincs/systemvars.cpp

namespace graphics::vulkan {

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
	auto buffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1, sizeof(genericData_default_material_v_sdr));
	auto* data = buffer.aligner().addTypedElement<genericData_default_material_v_sdr>();

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
	data->srgb = High_dynamic_range ? 1 : 0;
	data->intensity = material_info->get_color_scale();

	// Alpha threshold
	data->alphaThreshold = getStateTracker()->getAlphaThreshold();

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
		data->clipEquation.a1d[3] = -((clip_plane.normal.xyz.x * clip_plane.position.xyz.x) +
		                              (clip_plane.normal.xyz.y * clip_plane.position.xyz.y) +
		                              (clip_plane.normal.xyz.z * clip_plane.position.xyz.z));

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
	                       sizeof(genericData_default_material_v_sdr), buffer.bufferHandle());
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
	graphics::vulkan::VulkanDrawManager::setClearColor(r, g, b);
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
	GR_DEBUG_SCOPE("Begin scene texture");

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

		std::array<vk::ClearAttachment, 2> clearAttachments;
		clearAttachments[0].aspectMask = vk::ImageAspectFlagBits::eColor;
		clearAttachments[0].colorAttachment = 0;
		clearAttachments[0].clearValue.color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});

		clearAttachments[1].aspectMask = vk::ImageAspectFlagBits::eDepth;
		clearAttachments[1].clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

		vk::ClearRect clearRect;
		clearRect.rect.offset = vk::Offset2D(0, 0);
		clearRect.rect.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.max_w),
		                                      static_cast<uint32_t>(gr_screen.max_h));
		clearRect.rect = stateTracker->clampToRenderArea(clearRect.rect);
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;

		if (clearRect.rect.extent.width > 0 && clearRect.rect.extent.height > 0) {
			cmdBuffer.clearAttachments(clearAttachments, clearRect);
		}
	}
}

void vulkan_scene_texture_end()
{
	GR_DEBUG_SCOPE("End scene texture");

	auto* renderer = getRendererInstance();

	// If we were rendering to the HDR scene target, switch back to swap chain
	if (renderer->isSceneRendering()) {
		renderer->endSceneRendering();
	}

	High_dynamic_range = false;
}

void vulkan_copy_effect_texture()
{
	GR_DEBUG_SCOPE("Copy effect texture");

	auto* renderer = getRendererInstance();

	// Only copy if we're actively rendering the HDR scene
	if (!renderer->isSceneRendering()) {
		return;
	}

	renderer->copyEffectTexture();
}

void vulkan_draw_sphere(material* material_def, float /*rad*/)
{
	// Set up uniform buffers before rendering (like OpenGL does)
	gr_matrix_set_uniforms();
	vulkan_set_default_material_uniforms(material_def);

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

void vulkan_render_shadow_draw(gr_buffer_handle ubo_handle, size_t ubo_offset, size_t ubo_size,
                               vertex_buffer* buffer, indexed_vertex_source* vert_src, size_t texi)
{
	auto* drawManager = getDrawManager();
	drawManager->renderShadowDraw(ubo_handle, ubo_offset, ubo_size, buffer, vert_src, texi);
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
	                     && pp && pp->deferred().isInitialized();

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
		auto nearestSampler = texMgr->getSampler(vk::Filter::eNearest, vk::Filter::eNearest,
		                                          vk::SamplerAddressMode::eClampToEdge, false, 0.0f, false);
		drawManager->setDepthTextureOverride(
			{nearestSampler, pp->deferred().positionView(), vk::ImageLayout::eShaderReadOnlyOptimal});
	} else if (renderer->isSceneDepthCopied() && pp) {
		// Non-deferred path: bind the hardware depth copy
		auto* texMgr = getTextureManager();
		auto nearestSampler = texMgr->getSampler(vk::Filter::eNearest, vk::Filter::eNearest,
		                                          vk::SamplerAddressMode::eClampToEdge, false, 0.0f, false);
		drawManager->setDepthTextureOverride(
			{nearestSampler, pp->getSceneDepthCopyView(), vk::ImageLayout::eShaderReadOnlyOptimal});
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

	// Set effect_distort_data GenericData UBO (16 bytes)
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
	if (pp) drawManager->setSceneColorOverride(pp->getSceneEffectTextureInfo());

	// Set distortion map override (binding 6) — ping-pong noise texture for thrusters
	if (material_info->get_thruster_rendering() && pp)
		drawManager->setDistMapOverride(pp->getDistortionTextureInfo());

	drawManager->renderPrimitivesDistortion(material_info, prim_type, layout, offset, n_verts, buffer_handle);

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

	// Movie shader reads alpha from the MovieData UBO (set 2, binding 4), not from
	// GenericData - matches opengl_tnl_set_material_movie in gropengltnl.cpp.
	auto uniform_buffer = gr_get_uniform_buffer(uniform_block_type::MovieData, 1);
	auto& aligner = uniform_buffer.aligner();
	auto movie_data = aligner.addTypedElement<movie_uniforms>();
	movie_data->alpha = material_info->get_color().xyzw.w;
	uniform_buffer.submitData();
	gr_bind_uniform_buffer(uniform_block_type::MovieData,
		uniform_buffer.getBufferOffset(0),
		sizeof(movie_uniforms),
		uniform_buffer.bufferHandle());

	auto* drawManager = getDrawManager();
	drawManager->renderMovie(material_info, prim_type, layout, n_verts, buffer, buffer_offset);
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

	// Set batched_data GenericData UBO (matching OpenGL's opengl_tnl_set_material_batched)
	auto buffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1,
	                                     sizeof(graphics::generic_data::batched_data));
	auto* data = buffer.aligner().addTypedElement<graphics::generic_data::batched_data>();
	data->intensity = material_info->get_color_scale();
	data->color     = material_info->get_color();
	buffer.submitData();
	gr_bind_uniform_buffer(uniform_block_type::GenericData, buffer.getBufferOffset(0),
	                       sizeof(graphics::generic_data::batched_data), buffer.bufferHandle());

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
	if (!renderer || !stateTracker || !texManager || !descManager || !bufferManager || !pipelineManager) {
		return;
	}

	// Get envmap cubemap view
	auto* envSlot = bm_get_slot(ENVMAP, true);
	if (!envSlot || !envSlot->gr_info) {
		return;
	}
	auto* envTs = static_cast<tcache_slot_vulkan*>(envSlot->gr_info);
	// Both the render-target and static-file cubemap upload paths store the
	// cube-sampling view in imageView, so that's what must be used here regardless
	// of isCubemap.
	vk::ImageView envmapView = envTs->imageView;
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

	// End the current swap chain render pass
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
		nprintf(("vulkan", "vulkan_calculate_irrmap: Failed to get pipeline!\n"));
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
		nprintf(("vulkan", "vulkan_calculate_irrmap: Failed to create face UBO: %s\n", e.what()));
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
		*reinterpret_cast<int*>(mapped + (i * UBO_SLOT_SIZE)) = i;
	}
	// Flush before unmap — CpuToGpu memory is not guaranteed coherent, so without
	// this the GPU could sample stale face indices. (No-op on coherent memory.)
	memManager->flushMemory(faceUBOAlloc, 0, UBO_TOTAL_SIZE);
	memManager->unmapMemory(faceUBOAlloc);

	// This buffer is recreated per call (and queued for deletion at the end).
	// vulkan_calculate_irrmap runs only on env-map (re)generation, not per frame,
	// so the cold allocation isn't worth caching as renderer-lifetime state — and a
	// function-lifetime static VkBuffer would dangle past device destruction.

	vk::Extent2D irrExtent(irrTs->width, irrTs->height);

	for (size_t face = 0; face < irrTs->cubeFaceFramebuffers.size(); face++) {
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
		writer.reset(device, descManager->getFallbacks());

		// Set 0: Global (all fallback)
		vk::DescriptorSet globalSet = descManager->allocateFrameSet(DescriptorSetIndex::Global);
		Assert(globalSet);
		writer.writeSet(globalSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Global));

		// Set 1: Material (envmap cubemap at element 0 of texture array)
		vk::DescriptorSet materialSet = descManager->allocateFrameSet(DescriptorSetIndex::Material);
		Assert(materialSet);
		writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
		{
			std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texImages;
			texImages.fill(descManager->getFallbacks().texture2D);
			texImages[0].imageView = envmapView;
			writer.setImageArray(MaterialBinding::TextureArray, texImages);
		}

		// Set 2: PerDraw (face UBO at binding 0)
		vk::DescriptorSet perDrawSet = descManager->allocateFrameSet(DescriptorSetIndex::PerDraw);
		Assert(perDrawSet);
		writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
		writer.setBuffer(PerDrawBinding::GenericData, {faceUBO,
			static_cast<vk::DeviceSize>(face) * UBO_SLOT_SIZE, UBO_SLOT_SIZE});
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

	// This path renders each face directly rather than going through
	// bm_set_render_target()/endRenderTarget(), so the mip chain that
	// deferred-f.sdr's implicit-LOD `texture(sIrrmap, ...)` can reach
	// (high-frequency normals/silhouettes push the derivative-based LOD up)
	// would otherwise be left uninitialized above mip 0.
	if (irrTs->mipLevels > 1) {
		ImageBarrier2 barrier;
		barrier.image = irrTs->image;
		barrier.levelCount = 1;
		barrier.layerCount = 6;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		barrier.srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
		barrier.dstStage = vk::PipelineStageFlagBits2::eBlit;
		barrier.dstAccess = vk::AccessFlagBits2::eTransferRead;

		cmdImageBarrier(cmd, barrier);

		vulkan_generate_mipmap_chain(cmd, irrTs->image, irrTs->width, irrTs->height, irrTs->mipLevels, 6);
	}

	// Resume the swap chain pass (irrmap is always called before scene rendering begins)
	renderer->resumeSwapChainPass();

	nprintf(("vulkan", "vulkan_calculate_irrmap: Generated irradiance cubemap (%ux%u)\n", irrTs->width, irrTs->height));
}

} // namespace graphics::vulkan

