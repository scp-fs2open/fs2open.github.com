
#include "gr_vulkan.h"

#include <cerrno>
#include <cstring>

#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanPipelineManager.h"
#include "VulkanDescriptorManager.h"
#include "VulkanTexture.h"
#include "vulkan_stubs.h"

#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_vulkan.h"
#include "mod_table/mod_table.h"
#include "graphics/matrix.h"
#include "graphics/util/UniformBuffer.h"
#include "math/vecmat.h"
#include "bmpman/bmpman.h"

namespace graphics {
namespace vulkan {

namespace {
std::unique_ptr<VulkanRenderer> renderer_instance;

// Generic data uniform block layout (matches default-material shader)
// Must be aligned to std140 layout rules
struct alignas(16) VulkanGenericData {
	matrix4 modelMatrix;    // 64 bytes at offset 0
	vec4 color;             // 16 bytes at offset 64
	vec4 clipEquation;      // 16 bytes at offset 80
	int baseMapIndex;       // 4 bytes at offset 96
	int alphaTexture;       // 4 bytes at offset 100
	int noTexturing;        // 4 bytes at offset 104
	int srgb;               // 4 bytes at offset 108
	float intensity;        // 4 bytes at offset 112
	float alphaThreshold;   // 4 bytes at offset 116
	int clipEnabled;        // 4 bytes at offset 120 (bool as int for std140)
	int _pad;               // 4 bytes padding to align to 16 bytes
};

// Helper to set up generic uniform data for Vulkan drawing
void vulkan_set_generic_uniforms(material* mat)
{
	auto buffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1, sizeof(VulkanGenericData));
	auto& aligner = buffer.aligner();

	auto data = aligner.addTypedElement<VulkanGenericData>();

	// Set default values
	vm_matrix4_set_identity(&data->modelMatrix);

	// Get color from material
	const vec4& matColor = mat->get_color();
	data->color = matColor;

	// Clip plane (if enabled)
	if (mat->is_clipped()) {
		const material::clip_plane& clip = mat->get_clip_plane();
		data->clipEquation.xyzw.x = clip.normal.xyz.x;
		data->clipEquation.xyzw.y = clip.normal.xyz.y;
		data->clipEquation.xyzw.z = clip.normal.xyz.z;
		data->clipEquation.xyzw.w = -vm_vec_dot(&clip.normal, &clip.position);
		data->clipEnabled = 1;
	} else {
		data->clipEquation = vm_vec4_new(0.0f, 0.0f, 0.0f, 0.0f);
		data->clipEnabled = 0;
	}

	// Texture settings
	data->baseMapIndex = 0;  // Default to first array slice
	data->alphaTexture = (mat->get_texture_type() == material::TEX_TYPE_AABITMAP) ? 1 : 0;
	data->noTexturing = mat->is_textured() ? 0 : 1;
	data->srgb = 1;  // Assume sRGB input textures

	// Intensity and alpha threshold
	data->intensity = mat->get_color_scale();
	data->alphaThreshold = 0.0f;  // No alpha test by default

	buffer.submitData();
	gr_bind_uniform_buffer(uniform_block_type::GenericData, buffer.getBufferOffset(0),
	                       sizeof(VulkanGenericData), buffer.bufferHandle());
}

// Helper: Set viewport and scissor to scene extent
void setViewportAndScissor(vk::CommandBuffer cmd, vk::Extent2D extent, VulkanRenderer::DrawState& state)
{
	if (!state.viewportSet) {
		vk::Viewport viewport;
		viewport.x = 0.0f;
		// Use negative height to flip Y-axis from Vulkan's top-down to OpenGL's bottom-up
		// This is required because FSO's rendering code assumes OpenGL's coordinate system
		// Start at bottom of viewport (height) and go negative to flip Y
		viewport.y = static_cast<float>(extent.height);
		viewport.width = static_cast<float>(extent.width);
		viewport.height = -static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd.setViewport(0, viewport);
		state.viewportSet = true;
	}
	
	if (!state.scissorSet) {
		vk::Rect2D scissor;
		scissor.offset = vk::Offset2D{0, 0};
		scissor.extent = extent;
		cmd.setScissor(0, scissor);
		state.scissorSet = true;
	}
}

// Helper: Bind vertex buffer if changed
void bindVertexBuffer(vk::CommandBuffer cmd, gr_buffer_handle handle, vk::DeviceSize offset,
                      VulkanRenderer::DrawState& state)
{
	if (state.boundVertexBuffer == handle && state.boundVertexOffset == offset) {
		return;  // Already bound
	}
	
	vk::Buffer vkBuffer = g_vulkanBufferManager->getBuffer(handle);
	if (!vkBuffer) {
		mprintf(("Vulkan: Invalid vertex buffer handle\n"));
		return;
	}
	
	cmd.bindVertexBuffers(0, {vkBuffer}, {offset});
	state.boundVertexBuffer = handle;
	state.boundVertexOffset = offset;
}

// Helper: Bind index buffer if changed
void bindIndexBuffer(vk::CommandBuffer cmd, gr_buffer_handle handle, vk::DeviceSize offset,
                     vk::IndexType indexType, VulkanRenderer::DrawState& state)
{
	if (state.boundIndexBuffer == handle && state.boundIndexOffset == offset) {
		return;  // Already bound
	}
	
	vk::Buffer vkBuffer = g_vulkanBufferManager->getBuffer(handle);
	if (!vkBuffer) {
		mprintf(("Vulkan: Invalid index buffer handle\n"));
		return;
	}
	
	cmd.bindIndexBuffer(vkBuffer, offset, indexType);
	state.boundIndexBuffer = handle;
	state.boundIndexOffset = offset;
}

// Helper: Bind pipeline if changed
void bindPipeline(vk::CommandBuffer cmd, vk::Pipeline pipeline, VulkanRenderer::DrawState& state)
{
	if (state.boundPipeline == pipeline) {
		return;  // Already bound
	}
	
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	state.boundPipeline = pipeline;
}

// Helper: Push uniform buffer addresses via push constants (BDA)
// This provides GPU addresses for all uniform blocks to shaders using buffer_reference
void pushUniformAddresses(vk::CommandBuffer cmd, vk::PipelineLayout layout)
{
	if (!cmd || !layout || !g_vulkanBufferManager) {
		return;
	}

	// Get all bound uniform buffer addresses
	auto addresses = g_vulkanBufferManager->getUniformAddresses();

	// Push addresses to all shader stages (vertex, fragment, geometry)
	cmd.pushConstants(
	    layout,
	    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eGeometry,
	    0,  // offset
	    VulkanBufferManager::UniformAddressPushConstants::size(),
	    &addresses);
}

// Helper: Bind uniform buffer descriptor set (Set 0)
bool bindUniformDescriptors(vk::CommandBuffer cmd, vk::PipelineLayout layout, VulkanRenderer::DrawState& state)
{
	if (!cmd || !layout || !g_vulkanBufferManager) {
		return false;
	}

	auto* renderer = getRendererInstance();
	if (!renderer) {
		return false;
	}

	auto* descriptorManager = renderer->getDescriptorManager();
	if (!descriptorManager) {
		return false;
	}

	vk::DescriptorSet uniformSet = g_vulkanBufferManager->getUniformDescriptorSet();
	if (!uniformSet) {
		// Uniform set not initialized - this is OK for some draws
		return true;
	}

	// Build dynamic offsets for all bound uniform buffers
	// Note: Dynamic offsets are required because FSO uses ring-buffer suballocation
	// (all uniform blocks share one buffer, accessed via offsets - matches OpenGL's glBindBufferRange)
	SCP_vector<uint32_t> dynamicOffsets;
	for (int i = 0; i < static_cast<int>(uniform_block_type::NUM_BLOCK_TYPES); ++i) {
		auto blockType = static_cast<uniform_block_type>(i);
		auto bound = g_vulkanBufferManager->getBoundUniformBuffer(blockType);
		if (bound.isValid()) {
			// Offset must be aligned to minUniformBufferOffsetAlignment (validated in bindUniformBuffer)
			dynamicOffsets.push_back(static_cast<uint32_t>(bound.offset));
		} else {
			dynamicOffsets.push_back(0); // Unbound buffers use offset 0
		}
	}

	// Bind descriptor set at set 0
	descriptorManager->bindDescriptorSet(cmd, layout, uniformSet, dynamicOffsets, 0);

	// Mark the set as bound - prevents any further descriptor updates this frame
	g_vulkanBufferManager->markUniformSetBound();

	// Also push uniform addresses for BDA-enabled shaders
	// This allows shaders to access uniforms via buffer_reference pointers
	pushUniformAddresses(cmd, layout);

	return true;
}

// Helper: Bind material descriptors (textures and uniforms)
// Returns true if binding was successful
vk::PipelineLayout getMaterialPipelineLayout(material* mat)
{
	if (!mat || !g_vulkanPipelineManager) {
		return nullptr;
	}

	return g_vulkanPipelineManager->getPipelineLayout(
	    mat->get_shader_type(),
	    mat->get_shader_flags());
}

bool bindMaterialDescriptors(vk::CommandBuffer cmd, material* mat,
                             vk::PipelineLayout layout, VulkanRenderer::DrawState& /*state*/)
{
	if (!cmd || !mat || !layout) {
		return false;
	}

	if (!g_vulkanTextureManager || !g_vulkanPipelineManager) {
		return false;
	}

	auto* renderer = getRendererInstance();
	if (!renderer) {
		return false;
	}

	auto* descriptorManager = renderer->getDescriptorManager();
	if (!descriptorManager) {
		return false;
	}

	vk::DescriptorSetLayout descriptorLayout = g_vulkanPipelineManager->getMaterialDescriptorSetLayout();
	if (!descriptorLayout) {
		return false;
	}

	// Allocate descriptor set for this material
	vk::DescriptorSet descriptorSet = descriptorManager->allocateSet(descriptorLayout,
	                                                                  "MaterialTextures");
	if (!descriptorSet) {
		mprintf(("Vulkan: Failed to allocate descriptor set for material textures\n"));
		return false;
	}

	// Get placeholder texture and sampler for unbound slots
	VulkanTexture* placeholderTex = g_vulkanTextureManager->getPlaceholderTexture();
	vk::Sampler defaultSampler = g_vulkanTextureManager->getSamplerCache().getSampler(
	    vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, 1.0f, true);

	// Initialize all bindings to placeholder texture to ensure valid descriptors
	// This prevents validation errors for unused texture slots
	if (placeholderTex && placeholderTex->isValid() && defaultSampler) {
		for (uint32_t binding = 0; binding < VulkanPipelineManager::MATERIAL_DESCRIPTOR_BINDING_COUNT; ++binding) {
			descriptorManager->updateCombinedImageSampler(descriptorSet,
			                                               binding,
			                                               placeholderTex->getImageView(),
			                                               defaultSampler,
			                                               vk::ImageLayout::eShaderReadOnlyOptimal);
		}
	}

	// Helper to bind a texture slot (overrides placeholder if texture exists)
	auto bindTextureSlot = [&](int textureHandle, uint32_t binding, const char* name) {
		if (textureHandle < 0) {
			return; // Texture not used - placeholder already bound
		}

		// Check if texture already exists in our manager
		VulkanTexture* texture = g_vulkanTextureManager->getTexture(textureHandle);
		if (!texture) {
			// Texture not loaded yet - load it on-demand like OpenGL does
			// Determine bitmap type and lock parameters
			int bpp = 32;
			ushort flags = BMP_TEX_XPARENT;

			// First check if this is an AABITMAP (font glyph) by checking bitmap flags
			ushort bm_flags = 0;
			bm_get_info(textureHandle, nullptr, nullptr, &bm_flags);

			if (bm_flags & BMP_AABITMAP) {
				// Font glyph - must use BMP_AABITMAP flag and 8bpp
				flags = BMP_AABITMAP;
				bpp = 8;
			} else if (bm_is_compressed(textureHandle)) {
				// Compressed texture - use DXT flags based on compression type
				int compression = bm_is_compressed(textureHandle);
				if (compression == 1) { // DXT1
					flags = BMP_TEX_DXT1;
					bpp = 24;
				} else if (compression == 2) { // DXT3
					flags = BMP_TEX_DXT3;
					bpp = 32;
				} else if (compression == 3) { // DXT5
					flags = BMP_TEX_DXT5;
					bpp = 32;
				} else if (compression >= 4 && compression <= 6) { // Cubemap variants
					flags = BMP_TEX_CUBEMAP;
					bpp = (compression == 4) ? 24 : 32;
				} else {
					flags = BMP_TEX_OTHER;
					bpp = 32;
				}
			} else {
				// Normal texture - check for alpha channel
				if (bm_has_alpha_channel(textureHandle)) {
					flags = BMP_TEX_XPARENT;
					bpp = 32;
				} else {
					flags = BMP_TEX_OTHER;
					bpp = 24;
				}
			}

			// Lock the bitmap - this triggers bm_load_image_data and gr_bm_data
			bitmap* bmp = bm_lock(textureHandle, bpp, flags);
			if (bmp) {
				bm_unlock(textureHandle);
				// Now try to get the texture again
				texture = g_vulkanTextureManager->getTexture(textureHandle);
			}

			if (!texture) {
				mprintf(("Vulkan: Failed to load texture handle %d for %s\n", textureHandle, name));
				return;
			}
		}

		// Get sampler based on material addressing mode
		vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat;
		int addressing = mat->get_texture_addressing();
		if (addressing == TMAP_ADDRESS_CLAMP) {
			addressMode = vk::SamplerAddressMode::eClampToEdge;
		} else if (addressing == TMAP_ADDRESS_MIRROR) {
			addressMode = vk::SamplerAddressMode::eMirroredRepeat;
		}

		vk::Sampler sampler = g_vulkanTextureManager->getSamplerCache().getSampler(
		    vk::Filter::eLinear, vk::Filter::eLinear, addressMode, 1.0f, true);

		if (!sampler) {
			mprintf(("Vulkan: Failed to acquire sampler for %s\n", name));
			return;
		}

		descriptorManager->updateCombinedImageSampler(descriptorSet,
		                                               binding,
		                                               texture->getImageView(),
		                                               sampler,
		                                               vk::ImageLayout::eShaderReadOnlyOptimal);
	};

	// Bind all texture slots
	bindTextureSlot(mat->get_texture_map(TM_BASE_TYPE), 0, "base");
	bindTextureSlot(mat->get_texture_map(TM_GLOW_TYPE), 1, "glow");
	bindTextureSlot(mat->get_texture_map(TM_NORMAL_TYPE), 2, "normal");
	bindTextureSlot(mat->get_texture_map(TM_SPECULAR_TYPE), 3, "specular");
	bindTextureSlot(mat->get_texture_map(TM_AMBIENT_TYPE), 9, "ambient");
	bindTextureSlot(mat->get_texture_map(TM_MISC_TYPE), 10, "misc");
	
	// Environment maps (if used by shader)
	// Note: These may need to be set globally, not per-material
	// For now, skip if not in material
	
	// Bind descriptor set at set 1
	descriptorManager->bindDescriptorSet(cmd, layout, descriptorSet, {}, 1);
	renderer->queueDescriptorSetFree(descriptorSet);

	return true;
}

}

void initialize_function_pointers() {
	init_stub_pointers();
}

// Debug helper - write to file since log system may not be initialized
// Write to current directory (game directory) for easier access
static void vulkan_debug_log(const char* msg) {
	FILE* f = fopen("vulkan_debug.log", "a");
	if (f) {
		fprintf(f, "%s\n", msg);
		fflush(f);
		fclose(f);
	} else {
		// Report error to stderr if we can't write (helps debugging)
		fprintf(stderr, "gr_vulkan: Failed to write debug log to 'vulkan_debug.log': %s\n", strerror(errno));
	}
}

bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps)
{
	vulkan_debug_log("gr_vulkan::initialize() starting");
	renderer_instance.reset(new VulkanRenderer(std::move(graphicsOps)));
	vulkan_debug_log("VulkanRenderer created, calling initialize()");
	if (!renderer_instance->initialize()) {
		vulkan_debug_log("VulkanRenderer::initialize() FAILED");
		return false;
	}
	vulkan_debug_log("VulkanRenderer::initialize() succeeded");

	gr_screen.gf_flip = []() {
		renderer_instance->flip();
	};

	// Initialize matrices for 2D rendering (same as OpenGL does in gropengl.cpp)
	gr_reset_matrices();
	gr_setup_viewport();

	// Vulkan renderer initialization complete
	// Note: Some features may still be incomplete, but basic rendering should work
	mprintf(("Vulkan renderer initialized successfully\n"));
	vulkan_debug_log("gr_vulkan::initialize() complete");
	return true;
}

VulkanRenderer* getRendererInstance()
{
	return renderer_instance.get();
}

void cleanup()
{
	renderer_instance->shutdown();
	renderer_instance = nullptr;
}

void gr_vulkan_setup_frame()
{
	// No-op: beginFrame is already called in acquireNextSwapChainImage
	// This hook exists for compatibility with the gr_screen interface
	// Any per-frame state reset that doesn't involve Vulkan managers goes here
}

void gr_vulkan_scene_texture_begin()
{
	if (renderer_instance) {
		renderer_instance->beginScenePass();
	}
}

void gr_vulkan_scene_texture_end()
{
	if (renderer_instance) {
		renderer_instance->endScenePass();
	}
}

// ============================================================================
// Irradiance map generation
// ============================================================================

// Uniform data for irradiance map generation (matches genericData in irrmap-f.sdr)
// Must use std140 layout rules - int is aligned to 4 bytes, padded to 16 bytes
struct alignas(16) VulkanIrrmapData {
	int face;
	int _pad[3]; // Pad to 16 bytes for std140
};

void gr_vulkan_calculate_irrmap()
{
	// Validate required resources
	if (!renderer_instance || !g_vulkanTextureManager || !g_vulkanPipelineManager) {
		static bool warned = false;
		if (!warned) {
			mprintf(("Vulkan: gr_vulkan_calculate_irrmap - missing required managers\n"));
			warned = true;
		}
		return;
	}

	// Get envmap texture
	VulkanTexture* envmapTex = g_vulkanTextureManager->getTexture(ENVMAP);
	if (!envmapTex || !envmapTex->isValid()) {
		static bool warned = false;
		if (!warned) {
			mprintf(("Vulkan: gr_vulkan_calculate_irrmap - ENVMAP texture not available\n"));
			warned = true;
		}
		return;
	}

	// Check that irrmap render target exists
	if (gr_screen.irrmap_render_target < 0) {
		static bool warned = false;
		if (!warned) {
			mprintf(("Vulkan: gr_vulkan_calculate_irrmap - irrmap render target not created\n"));
			warned = true;
		}
		return;
	}

	// Save current render target
	int previous_target = gr_screen.rendering_to_texture;

	// Get the render target info for accessing its render pass
	VulkanRenderTarget* irrmapRT = nullptr;

	// Set up the irrmap render target to get the RT info
	bm_set_render_target(gr_screen.irrmap_render_target, 0);
	irrmapRT = g_vulkanTextureManager->getActiveRenderTarget();

	if (!irrmapRT || !irrmapRT->renderPass) {
		mprintf(("Vulkan: gr_vulkan_calculate_irrmap - failed to get irrmap render target\n"));
		bm_set_render_target(previous_target);
		return;
	}

	// Determine a framebuffer we can sample metadata from (cubemaps only have per-face FBs)
	VulkanFramebuffer* pipelineFramebuffer = irrmapRT->framebuffer.get();
	if (!pipelineFramebuffer && irrmapRT->isCubemap) {
		for (auto& faceFramebuffer : irrmapRT->cubeFaceFramebuffers) {
			if (faceFramebuffer) {
				pipelineFramebuffer = faceFramebuffer.get();
				break;
			}
		}
	}

	if (!pipelineFramebuffer) {
		mprintf(("Vulkan: gr_vulkan_calculate_irrmap - unable to find framebuffer for pipeline setup\n"));
		bm_set_render_target(previous_target);
		return;
	}

	// Build pipeline key for irradiance map generation shader
	// This is a fullscreen pass with no vertex input (vertexless drawing)
	PipelineKey pipelineKey;
	pipelineKey.shaderType = SDR_TYPE_IRRADIANCE_MAP_GEN;
	pipelineKey.shaderFlags = 0;
	pipelineKey.vertexLayoutHash = 0;  // No vertex input - fullscreen triangle via gl_VertexIndex
	pipelineKey.primitiveType = PRIM_TYPE_TRIS;
	pipelineKey.cullEnabled = false;
	pipelineKey.cullMode = vk::CullModeFlagBits::eNone;
	pipelineKey.polygonMode = vk::PolygonMode::eFill;
	pipelineKey.depthMode = ZBUFFER_TYPE_NONE;  // No depth testing for post-process
	pipelineKey.stencilEnabled = false;
	pipelineKey.blendMode = ALPHA_BLEND_NONE;  // No blending
	pipelineKey.hasPerBufferBlend = false;
	pipelineKey.colorMask = {true, true, true, true};
	// Dynamic rendering (Vulkan 1.3+) - use formats instead of render pass
	pipelineKey.colorFormat = pipelineFramebuffer->getColorFormat(0);
	pipelineKey.depthFormat = vk::Format::eUndefined;  // No depth for irradiance map
	pipelineKey.sampleCount = vk::SampleCountFlagBits::e1;  // No MSAA for cubemap RT

	// Get or create the pipeline
	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(pipelineKey);
	if (!pipeline) {
		mprintf(("Vulkan: gr_vulkan_calculate_irrmap - failed to create pipeline\n"));
		bm_set_render_target(previous_target);
		return;
	}

	// Get pipeline layout
	vk::PipelineLayout pipelineLayout = g_vulkanPipelineManager->getPipelineLayout(
	    SDR_TYPE_IRRADIANCE_MAP_GEN, 0);
	if (!pipelineLayout) {
		mprintf(("Vulkan: gr_vulkan_calculate_irrmap - failed to get pipeline layout\n"));
		bm_set_render_target(previous_target);
		return;
	}

	auto* descriptorManager = renderer_instance->getDescriptorManager();
	if (!descriptorManager) {
		mprintf(("Vulkan: gr_vulkan_calculate_irrmap - no descriptor manager\n"));
		bm_set_render_target(previous_target);
		return;
	}

	// Get a sampler for the envmap cubemap
	vk::Sampler envmapSampler = g_vulkanTextureManager->getSamplerCache().getSampler(
	    vk::Filter::eLinear, vk::Filter::eLinear,
	    vk::SamplerAddressMode::eClampToEdge, 1.0f, true);

	if (!envmapSampler) {
		mprintf(("Vulkan: gr_vulkan_calculate_irrmap - failed to get sampler\n"));
		bm_set_render_target(previous_target);
		return;
	}

	mprintf(("Vulkan: Generating irradiance map from envmap\n"));

	// Process each face of the irradiance cubemap
	// Use auxiliary render pass to avoid interfering with the main scene pass state
	for (int face = 0; face < 6; face++) {
		// Set render target to this cubemap face (updates active face in RT)
		bm_set_render_target(gr_screen.irrmap_render_target, face);

		// Get the per-face framebuffer from the render target
		VulkanFramebuffer* faceFramebuffer = nullptr;
		if (irrmapRT->isCubemap && irrmapRT->cubeFaceFramebuffers[face]) {
			faceFramebuffer = irrmapRT->cubeFaceFramebuffers[face].get();
		} else {
			faceFramebuffer = irrmapRT->framebuffer.get();
		}

		if (!faceFramebuffer) {
			mprintf(("Vulkan: gr_vulkan_calculate_irrmap - no framebuffer for face %d\n", face));
			continue;
		}

		// Begin auxiliary render pass for this face
		// This does NOT set m_scenePassActive, allowing gr_scene_texture_begin() to work later
		vk::Extent2D faceExtent = faceFramebuffer->getExtent();
		renderer_instance->beginAuxiliaryRenderPass(irrmapRT->renderPass.get(), faceFramebuffer, faceExtent, true);

		auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
		if (!cmdBuffer) {
			mprintf(("Vulkan: gr_vulkan_calculate_irrmap - no command buffer for face %d\n", face));
			renderer_instance->endAuxiliaryRenderPass();
			continue;
		}

		auto& drawState = renderer_instance->getDrawState();

		// Bind pipeline
		bindPipeline(cmdBuffer, pipeline, drawState);

		// Set viewport and scissor for 16x16 irrmap face
		setViewportAndScissor(cmdBuffer, faceExtent, drawState);

		// Set up the face uniform using GenericData (binding 8 in Set 0)
		auto uniformBuffer = gr_get_uniform_buffer(uniform_block_type::GenericData, 1, sizeof(VulkanIrrmapData));
		auto& aligner = uniformBuffer.aligner();
		auto* irrmapData = aligner.addTypedElement<VulkanIrrmapData>();
		irrmapData->face = face;
		uniformBuffer.submitData();
		gr_bind_uniform_buffer(uniform_block_type::GenericData, uniformBuffer.getBufferOffset(0),
		                       sizeof(VulkanIrrmapData), uniformBuffer.bufferHandle());

		// Bind uniform descriptor set (Set 0 - includes GenericData at binding 8)
		bindUniformDescriptors(cmdBuffer, pipelineLayout, drawState);

		// Allocate and update material descriptor set for envmap (Set 1, binding 4)
		vk::DescriptorSetLayout materialLayout = g_vulkanPipelineManager->getMaterialDescriptorSetLayout();
		if (materialLayout) {
			vk::DescriptorSet materialSet = descriptorManager->allocateSet(materialLayout, "IrrmapEnvmap");
			if (materialSet) {
				// Update binding 4 with envmap cubemap
				descriptorManager->updateCombinedImageSampler(
				    materialSet, 4,  // binding 4 = envmap in material layout
				    envmapTex->getImageView(),
				    envmapSampler,
				    vk::ImageLayout::eShaderReadOnlyOptimal);

				// Bind material descriptor set at Set 1
				descriptorManager->bindDescriptorSet(cmdBuffer, pipelineLayout, materialSet, {}, 1);

				// Queue for cleanup after frame
				renderer_instance->queueDescriptorSetFree(materialSet);
			}
		}

		// Draw fullscreen triangle (3 vertices, no vertex buffer - uses gl_VertexIndex)
		cmdBuffer.draw(3, 1, 0, 0);

		// End auxiliary render pass for this face
		renderer_instance->endAuxiliaryRenderPass();
	}

	// Submit recorded auxiliary passes so the irradiance cubemap is ready immediately
	renderer_instance->submitAuxiliaryCommandBuffer();

	// Restore previous render target
	bm_set_render_target(previous_target);

	mprintf(("Vulkan: Irradiance map generation complete\n"));
}

void gr_vulkan_render_primitives(material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle,
    size_t buffer_offset)
{
	if (!renderer_instance || !material_info || !layout) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	// Ensure a render pass is active (auto-starts direct pass for menus)
	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		mprintf(("Vulkan: render_primitives - failed to get command buffer\n"));
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	// Get or create pipeline from material and layout
	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, prim_type, colorFormat, depthFormat);
	
	if (!pipeline) {
		mprintf(("Vulkan: Failed to get pipeline for render_primitives\n"));
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	// Bind pipeline
	bindPipeline(cmdBuffer, pipeline, state);
	
	// Set viewport and scissor
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	
	// Bind vertex buffer
	bindVertexBuffer(cmdBuffer, buffer_handle, buffer_offset, state);
	
	// Get pipeline layout
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	
	// Bind uniform buffers (Set 0)
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	
	// Bind material descriptors (Set 1)
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	// Draw
	cmdBuffer.draw(static_cast<uint32_t>(n_verts), 1, static_cast<uint32_t>(offset), 0);
}

void gr_vulkan_render_primitives_particle(particle_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle)
{
	if (!renderer_instance || !material_info || !layout) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, prim_type, colorFormat, depthFormat);
	
	if (!pipeline) {
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	bindPipeline(cmdBuffer, pipeline, state);
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	bindVertexBuffer(cmdBuffer, buffer_handle, 0, state);
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	cmdBuffer.draw(static_cast<uint32_t>(n_verts), 1, static_cast<uint32_t>(offset), 0);
}

void gr_vulkan_render_primitives_batched(batched_bitmap_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle)
{
	if (!renderer_instance || !material_info || !layout) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, prim_type, colorFormat, depthFormat);
	
	if (!pipeline) {
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	bindPipeline(cmdBuffer, pipeline, state);
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	bindVertexBuffer(cmdBuffer, buffer_handle, 0, state);
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	cmdBuffer.draw(static_cast<uint32_t>(n_verts), 1, static_cast<uint32_t>(offset), 0);
}

void gr_vulkan_render_primitives_distortion(distortion_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle)
{
	if (!renderer_instance || !material_info || !layout) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, prim_type, colorFormat, depthFormat);
	
	if (!pipeline) {
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	bindPipeline(cmdBuffer, pipeline, state);
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	bindVertexBuffer(cmdBuffer, buffer_handle, 0, state);
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	cmdBuffer.draw(static_cast<uint32_t>(n_verts), 1, static_cast<uint32_t>(offset), 0);
}

void gr_vulkan_render_movie(movie_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int n_verts,
    gr_buffer_handle buffer,
    size_t buffer_offset)
{
	if (!renderer_instance || !material_info || !layout) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, prim_type, colorFormat, depthFormat);
	
	if (!pipeline) {
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	bindPipeline(cmdBuffer, pipeline, state);
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	bindVertexBuffer(cmdBuffer, buffer, buffer_offset, state);
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	cmdBuffer.draw(static_cast<uint32_t>(n_verts), 1, 0, 0);
}

void gr_vulkan_render_nanovg(nanovg_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle)
{
	if (!renderer_instance || !material_info || !layout) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, prim_type, colorFormat, depthFormat);
	
	if (!pipeline) {
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	bindPipeline(cmdBuffer, pipeline, state);
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	bindVertexBuffer(cmdBuffer, buffer_handle, 0, state);
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	cmdBuffer.draw(static_cast<uint32_t>(n_verts), 1, static_cast<uint32_t>(offset), 0);
}

void gr_vulkan_render_decals(decal_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int num_elements,
    const indexed_vertex_source& binding,
    const gr_buffer_handle& instance_buffer,
    int num_instances)
{
	if (!renderer_instance || !material_info || !layout) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, prim_type, colorFormat, depthFormat);
	
	if (!pipeline) {
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	bindPipeline(cmdBuffer, pipeline, state);
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	
	// Bind vertex and index buffers from indexed_vertex_source
	bindVertexBuffer(cmdBuffer, binding.Vbuffer_handle, binding.Vertex_offset, state);
	bindIndexBuffer(cmdBuffer, binding.Ibuffer_handle, binding.Index_offset, vk::IndexType::eUint16, state);
	
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	// Draw indexed instanced
	cmdBuffer.drawIndexed(static_cast<uint32_t>(num_elements), 
	                       static_cast<uint32_t>(num_instances),
	                       0, 0, 0);
}

void gr_vulkan_render_rocket_primitives(interface_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int n_indices,
    gr_buffer_handle vertex_buffer,
    gr_buffer_handle index_buffer)
{
	if (!renderer_instance || !material_info || !layout) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, prim_type, colorFormat, depthFormat);
	
	if (!pipeline) {
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	bindPipeline(cmdBuffer, pipeline, state);
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	bindVertexBuffer(cmdBuffer, vertex_buffer, 0, state);
	bindIndexBuffer(cmdBuffer, index_buffer, 0, vk::IndexType::eUint16, state);
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	cmdBuffer.drawIndexed(static_cast<uint32_t>(n_indices), 1, 0, 0, 0);
}

void gr_vulkan_render_model(model_material* material_info,
    indexed_vertex_source* vert_source,
    vertex_buffer* bufferp,
    size_t texi)
{
	if (!renderer_instance || !material_info || !vert_source || !bufferp) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	// Get vertex layout from vertex_buffer
	vertex_layout* layout = &bufferp->layout;

	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, PRIM_TYPE_TRIS, colorFormat, depthFormat);
	
	if (!pipeline) {
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	bindPipeline(cmdBuffer, pipeline, state);
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	
	// Bind vertex and index buffers
	bindVertexBuffer(cmdBuffer, vert_source->Vbuffer_handle, vert_source->Vertex_offset, state);

	if (vert_source->Ibuffer_handle.isValid()) {
		bindIndexBuffer(cmdBuffer, vert_source->Ibuffer_handle, vert_source->Index_offset, vk::IndexType::eUint32, state);
	}
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	// Get texture info for this draw
	if (texi < bufferp->tex_buf.size()) {
		auto& texBuf = bufferp->tex_buf[texi];
		
		// Draw indexed - use index_offset for first index, vertex_num_offset for vertex offset
		if (vert_source->Ibuffer_handle.isValid()) {
			cmdBuffer.drawIndexed(static_cast<uint32_t>(texBuf.n_verts),
			                       1,
			                       static_cast<uint32_t>(texBuf.index_offset),
			                       static_cast<int32_t>(bufferp->vertex_num_offset),
			                       0);
		} else {
			cmdBuffer.draw(static_cast<uint32_t>(texBuf.n_verts),
			               1,
			               static_cast<uint32_t>(bufferp->vertex_num_offset),
			               0);
		}
	}
}

void gr_vulkan_render_shield_impact(shield_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    gr_buffer_handle buffer_handle,
    int n_verts)
{
	if (!renderer_instance || !material_info || !layout) {
		return;
	}

	// Update uniform buffers before drawing
	gr_matrix_set_uniforms();
	vulkan_set_generic_uniforms(material_info);

	renderer_instance->ensureRenderPassActive();
	
	auto cmdBuffer = renderer_instance->getCurrentCommandBuffer();
	if (!cmdBuffer) {
		return;
	}
	
	// Get current rendering formats for pipeline creation
	vk::Format colorFormat = renderer_instance->getCurrentColorFormat();
	vk::Format depthFormat = renderer_instance->getCurrentDepthFormat();
	if (colorFormat == vk::Format::eUndefined) {
		return;  // No active rendering
	}

	vk::Pipeline pipeline = g_vulkanPipelineManager->getOrCreatePipeline(
	    *material_info, *layout, prim_type, colorFormat, depthFormat);
	
	if (!pipeline) {
		return;
	}
	
	auto& state = renderer_instance->getDrawState();
	
	bindPipeline(cmdBuffer, pipeline, state);
	setViewportAndScissor(cmdBuffer, renderer_instance->getSceneExtent(), state);
	
	// Bind vertex buffer
	if (buffer_handle.isValid()) {
		bindVertexBuffer(cmdBuffer, buffer_handle, 0, state);
	}
	
	auto pipelineLayout = getMaterialPipelineLayout(material_info);
	bindUniformDescriptors(cmdBuffer, pipelineLayout, state);
	bindMaterialDescriptors(cmdBuffer, material_info, pipelineLayout, state);

	cmdBuffer.draw(static_cast<uint32_t>(n_verts), 1, 0, 0);
}

// ============================================================================
// Capability and Property Queries
// ============================================================================

bool gr_vulkan_is_capable(gr_capability capability)
{
	// Be conservative - only claim capabilities we've actually tested
	// Claiming unsupported capabilities enables code paths that may crash
	switch (capability) {
		// Basic texture features - these should work
		case gr_capability::CAPABILITY_ENVIRONMENT_MAP:
		case gr_capability::CAPABILITY_NORMAL_MAP:
		case gr_capability::CAPABILITY_HEIGHT_MAP:
			return true;

		// Buffer features - implemented
		case gr_capability::CAPABILITY_PERSISTENT_BUFFER_MAPPING:
			return true;

		// Texture compression - Vulkan supports BC formats
		case gr_capability::CAPABILITY_BPTC:
			return true;

		// Blend functions - Vulkan supports separate blend
		case gr_capability::CAPABILITY_SEPARATE_BLEND_FUNCTIONS:
			return true;

		// Features that need more implementation work - disable for now
		case gr_capability::CAPABILITY_SOFT_PARTICLES:
		case gr_capability::CAPABILITY_DISTORTION:
		case gr_capability::CAPABILITY_POST_PROCESSING:
		case gr_capability::CAPABILITY_DEFERRED_LIGHTING:
		case gr_capability::CAPABILITY_SHADOWS:
		case gr_capability::CAPABILITY_THICK_OUTLINE:
		case gr_capability::CAPABILITY_BATCHED_SUBMODELS:
		case gr_capability::CAPABILITY_LARGE_SHADER:
		case gr_capability::CAPABILITY_INSTANCED_RENDERING:
		case gr_capability::CAPABILITY_TIMESTAMP_QUERY:
			return false;

		default:
			return false;
	}
}

bool gr_vulkan_get_property(gr_property prop, void* dest)
{
	auto* renderer = getRendererInstance();
	if (!renderer || !dest) {
		return false;
	}

	switch (prop) {
		case gr_property::UNIFORM_BUFFER_OFFSET_ALIGNMENT: {
			// Get from buffer manager which queried device limits
			size_t alignment = g_vulkanBufferManager ? 
				g_vulkanBufferManager->getMinUniformBufferOffsetAlignment() : 256;
			*static_cast<size_t*>(dest) = alignment;
			return true;
		}

		case gr_property::UNIFORM_BUFFER_MAX_SIZE: {
			// Vulkan guarantees at least 16KB, most GPUs support 64KB+
			// Query from device limits if available
			*static_cast<size_t*>(dest) = 65536;
			return true;
		}

		case gr_property::MAX_ANISOTROPY: {
			// Most modern GPUs support 16x anisotropy
			*static_cast<float*>(dest) = 16.0f;
			return true;
		}

		default:
			return false;
	}
}

// ============================================================================
// Shader Management
// ============================================================================

int gr_vulkan_maybe_create_shader(shader_type type, unsigned int flags)
{
	// In the Vulkan renderer, shaders are created on-demand by VulkanPipelineManager
	// when pipelines are requested. This function returns a "handle" that represents
	// the shader type + flags combination.
	//
	// The handle is encoded as: (type << 16) | (flags & 0xFFFF)
	// This allows FSO to track which shader combinations exist.
	
	if (type < SDR_TYPE_MODEL || type >= NUM_SHADER_TYPES) {
		return -1;
	}

	// Encode shader type and flags into a single handle
	// Upper 16 bits = shader type, lower 16 bits = flags (truncated)
	int handle = (static_cast<int>(type) << 16) | (static_cast<int>(flags) & 0xFFFF);
	
	return handle;
}

// ============================================================================
// Z-Buffer Control
// ============================================================================

// Current z-buffer mode (mirrors gr_zbuffer_type in 2d.h)
static int s_zbuffer_mode = ZBUFFER_TYPE_FULL;

int gr_vulkan_zbuffer_get()
{
	return s_zbuffer_mode;
}

int gr_vulkan_zbuffer_set(int mode)
{
	int old_mode = s_zbuffer_mode;
	s_zbuffer_mode = mode;
	
	// The actual depth test/write state is set per-pipeline based on material settings
	// This function just tracks the global state that FSO uses to decide what to request
	
	return old_mode;
}

void gr_vulkan_zbuffer_clear(int mode)
{
	(void)mode;
	// Z-buffer is cleared at the start of each frame via render pass clear values
	// This function is called mid-frame to request a clear, which we handle
	// by ending and restarting the render pass with clear values
	
	// For now, this is a no-op since we clear at frame start
	// A proper implementation would insert a clear command
}

// ============================================================================
// Stencil Control
// ============================================================================

static int s_stencil_mode = 0;

int gr_vulkan_stencil_set(int mode)
{
	int old_mode = s_stencil_mode;
	s_stencil_mode = mode;
	
	// Stencil state is set per-pipeline based on material settings
	// This tracks the global state
	
	return old_mode;
}

void gr_vulkan_stencil_clear()
{
	// Stencil is cleared at frame start via render pass clear values
	// Mid-frame clears would need special handling
}

// ============================================================================
// Clipping (Scissor Rect)
// ============================================================================

// Clip state tracking (scissor applied during draw)
static int s_clip_x = 0;
static int s_clip_y = 0;
static int s_clip_w = 0;
static int s_clip_h = 0;
static bool s_clip_enabled = false;

void gr_vulkan_set_clip(int x, int y, int w, int h, int resize_mode)
{
	(void)resize_mode;
	
	// Just track the clip rect - actual scissor is set during draw commands
	// Setting scissor requires an active command buffer which may not exist yet
	s_clip_x = x;
	s_clip_y = y;
	s_clip_w = w;
	s_clip_h = h;
	s_clip_enabled = true;
}

void gr_vulkan_reset_clip()
{
	// Clear clip tracking - scissor will be set to full viewport during draw
	s_clip_enabled = false;
}

// ============================================================================
// Screen Clearing
// ============================================================================

static ubyte s_clear_red = 0;
static ubyte s_clear_green = 0;
static ubyte s_clear_blue = 0;

void gr_vulkan_set_clear_color(int r, int g, int b)
{
	s_clear_red = static_cast<ubyte>(r);
	s_clear_green = static_cast<ubyte>(g);
	s_clear_blue = static_cast<ubyte>(b);
	
	// The clear color is used when beginning the render pass
	// Update the renderer's clear color
	auto* renderer = getRendererInstance();
	if (renderer) {
		renderer->setClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
	}
}

void gr_vulkan_clear()
{
	// Clear is handled by render pass load ops
	// If called mid-frame, we'd need to insert a clear attachment command
	// For now, the clear happens automatically at scene pass begin
}

// ============================================================================
// Culling
// ============================================================================

static int s_cull_mode = 1; // Default: backface culling enabled

int gr_vulkan_set_cull(int cull)
{
	int old_mode = s_cull_mode;
	s_cull_mode = cull;
	
	// Cull mode is set per-pipeline based on material settings
	// This tracks the global state that materials inherit
	
	return old_mode;
}

// ============================================================================
// Color Buffer Control
// ============================================================================

static int s_color_buffer_mode = 1; // Default: color writes enabled

int gr_vulkan_set_color_buffer(int mode)
{
	int old_mode = s_color_buffer_mode;
	s_color_buffer_mode = mode;
	
	// Color write mask is set per-pipeline
	// This tracks the global state
	
	return old_mode;
}

} // namespace vulkan
} // namespace graphics
