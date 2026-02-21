#pragma once

#include "globalincs/pstypes.h"
#include "VulkanMemory.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

/**
 * @brief Manages Vulkan post-processing pipeline
 *
 * Owns offscreen render targets (HDR scene color + depth), render passes,
 * and executes post-processing passes (tonemapping, bloom, FXAA, etc.)
 * between the 3D scene rendering and the final swap chain presentation.
 */
class VulkanPostProcessor {
public:
	VulkanPostProcessor() = default;
	~VulkanPostProcessor() = default;

	// Non-copyable
	VulkanPostProcessor(const VulkanPostProcessor&) = delete;
	VulkanPostProcessor& operator=(const VulkanPostProcessor&) = delete;

	/**
	 * @brief Initialize post-processing resources
	 * @param device Vulkan logical device
	 * @param physDevice Physical device (for format checks)
	 * @param memMgr Memory manager for allocations
	 * @param extent Scene rendering resolution
	 * @param depthFormat Depth format (matches main depth buffer)
	 * @return true on success
	 */
	bool init(vk::Device device, vk::PhysicalDevice physDevice,
	          VulkanMemoryManager* memMgr, vk::Extent2D extent,
	          vk::Format depthFormat);

	/**
	 * @brief Shutdown and free all post-processing resources
	 */
	void shutdown();

	/**
	 * @brief Get the HDR scene render pass (for 3D scene rendering)
	 *
	 * This render pass has RGBA16F color + depth attachments with loadOp=eClear.
	 * Used between scene_texture_begin() and scene_texture_end().
	 */
	vk::RenderPass getSceneRenderPass() const { return m_sceneRenderPass; }

	/**
	 * @brief Get the HDR scene render pass with loadOp=eLoad
	 *
	 * Compatible with getSceneRenderPass() (same formats/samples) so uses
	 * the same framebuffer. Used to resume scene rendering after
	 * copy_effect_texture interrupts the pass.
	 */
	vk::RenderPass getSceneRenderPassLoad() const { return m_sceneRenderPassLoad; }

	/**
	 * @brief Get the HDR scene framebuffer
	 */
	vk::Framebuffer getSceneFramebuffer() const { return m_sceneFramebuffer; }

	/**
	 * @brief Get the scene rendering extent
	 */
	vk::Extent2D getSceneExtent() const { return m_extent; }

	/**
	 * @brief Execute post-processing passes and draw result to swap chain
	 *
	 * Called after the HDR scene render pass ends and before the resumed
	 * swap chain render pass begins. Runs tonemapping (and later bloom,
	 * FXAA, etc.) then draws a fullscreen triangle to blit the result
	 * into the swap chain.
	 *
	 * The caller is responsible for:
	 * 1. Ending the HDR scene render pass before calling this
	 * 2. Beginning the resumed swap chain render pass before calling this
	 *    (the blit draws INTO the resumed pass)
	 *
	 * @param cmd Active command buffer
	 */
	void blitToSwapChain(vk::CommandBuffer cmd);

	/**
	 * @brief Execute bloom post-processing passes
	 *
	 * Called after the HDR scene render pass ends and before the resumed
	 * swap chain render pass begins. Manages its own render passes internally.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeBloom(vk::CommandBuffer cmd);

	/**
	 * @brief Execute tonemapping pass (HDR scene → LDR)
	 *
	 * Called after bloom and before FXAA. Renders to Scene_ldr (RGBA8).
	 * Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeTonemap(vk::CommandBuffer cmd);

	/**
	 * @brief Execute FXAA anti-aliasing passes
	 *
	 * Called after tonemapping. Runs prepass (LDR→luminance) then
	 * FXAA main pass (luminance→LDR). Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeFXAA(vk::CommandBuffer cmd);

	/**
	 * @brief Execute post-processing effects (saturation, brightness, etc.)
	 *
	 * Called after FXAA and before the final blit. Reads Scene_ldr, writes
	 * Scene_luminance (reused as temp target). Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 * @return true if effects were applied (blit should read Scene_luminance)
	 */
	bool executePostEffects(vk::CommandBuffer cmd);

	/**
	 * @brief Execute lightshafts (god rays) pass
	 *
	 * Called after FXAA and before post-effects. Additively blends god rays
	 * onto Scene_ldr based on sun position and depth buffer sampling.
	 * Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeLightshafts(vk::CommandBuffer cmd);

	/**
	 * @brief Update distortion ping-pong textures
	 *
	 * Called every frame from endSceneRendering(). Internally tracks a ~30ms
	 * timer. When triggered, scrolls old distortion data right by 1 pixel and
	 * injects random noise at the left edge (matching OpenGL's
	 * gr_opengl_update_distortion()). Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 * @param frametime Time since last frame in seconds
	 */
	void updateDistortion(vk::CommandBuffer cmd, float frametime);

	/**
	 * @brief Get the current distortion texture view for thruster sampling
	 *
	 * Returns the most recently written distortion texture (the one thrusters
	 * should read from). Returns nullptr if distortion textures aren't initialized.
	 */
	vk::ImageView getDistortionTextureView() const;

	/**
	 * @brief Get the distortion texture sampler (LINEAR, REPEAT)
	 */
	vk::Sampler getDistortionSampler() const { return m_distortionSampler; }

	/**
	 * @brief Copy scene color to effect texture for distortion/soft particle sampling
	 *
	 * Must be called outside a render pass. Transitions scene color through
	 * eTransferSrcOptimal and back to eColorAttachmentOptimal (ready for resumed
	 * scene render pass). Transitions effect texture to eShaderReadOnlyOptimal.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void copyEffectTexture(vk::CommandBuffer cmd);

	/**
	 * @brief Copy G-buffer normal to samplable copy for decal angle rejection
	 *
	 * Must be called outside a render pass. Transitions G-buffer normal through
	 * eTransferSrcOptimal and back to eShaderReadOnlyOptimal. Transitions
	 * normal copy to eShaderReadOnlyOptimal for fragment shader sampling.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void copyGbufNormal(vk::CommandBuffer cmd);

	/**
	 * @brief Copy scene depth to samplable depth copy for soft particle rendering
	 *
	 * Must be called outside a render pass. Transitions scene depth through
	 * eTransferSrcOptimal and back to eDepthStencilAttachmentOptimal. Transitions
	 * depth copy to eShaderReadOnlyOptimal for fragment shader sampling.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void copySceneDepth(vk::CommandBuffer cmd);

	/**
	 * @brief Check if LDR targets are available (tonemapping + FXAA ready)
	 */
	bool hasLDRTargets() const { return m_ldrInitialized; }

	/**
	 * @brief Get the scene color image (for layout transitions outside post-processor)
	 */
	vk::Image getSceneColorImage() const { return m_sceneColor.image; }

	/**
	 * @brief Get the scene color image view (for post-processing texture binding)
	 */
	vk::ImageView getSceneColorView() const { return m_sceneColor.view; }

	/**
	 * @brief Get the scene color sampler
	 */
	vk::Sampler getSceneColorSampler() const { return m_linearSampler; }

	/**
	 * @brief Get the effect/composite texture view (snapshot of scene color)
	 *
	 * Available for sampling after copyEffectTexture() has been called.
	 * Used by distortion and soft particle shaders.
	 */
	vk::ImageView getSceneEffectView() const { return m_sceneEffect.view; }

	/**
	 * @brief Get the scene depth copy view (for soft particle depth sampling)
	 *
	 * Available for sampling after copySceneDepth() has been called.
	 */
	vk::ImageView getSceneDepthCopyView() const { return m_sceneDepthCopy.view; }

	/**
	 * @brief Get the effect texture sampler (linear, clamp-to-edge)
	 */
	vk::Sampler getSceneEffectSampler() const { return m_linearSampler; }

	/**
	 * @brief Check if post-processing is initialized
	 */
	bool isInitialized() const { return m_initialized; }

	// ========== G-Buffer (deferred lighting) ==========

	/**
	 * @brief Get the G-buffer render pass (6 color + depth, loadOp=eClear)
	 */
	vk::RenderPass getGbufRenderPass() const { return m_gbufRenderPass; }

	/**
	 * @brief Get the G-buffer render pass with loadOp=eLoad (resume after mid-pass copy)
	 */
	vk::RenderPass getGbufRenderPassLoad() const { return m_gbufRenderPassLoad; }

	/**
	 * @brief Get the G-buffer framebuffer (6 color + depth)
	 */
	vk::Framebuffer getGbufFramebuffer() const { return m_gbufFramebuffer; }

	/**
	 * @brief Check if G-buffer resources are initialized
	 */
	bool isGbufInitialized() const { return m_gbufInitialized; }

	// G-buffer image views (for future light pass texture sampling)
	vk::ImageView getGbufPositionView() const { return m_gbufPosition.view; }
	vk::ImageView getGbufNormalView() const { return m_gbufNormal.view; }
	vk::ImageView getGbufSpecularView() const { return m_gbufSpecular.view; }
	vk::ImageView getGbufEmissiveView() const { return m_gbufEmissive.view; }
	vk::ImageView getGbufCompositeView() const { return m_gbufComposite.view; }

	// G-buffer images (for copy operations)
	vk::Image getGbufEmissiveImage() const { return m_gbufEmissive.image; }
	vk::Image getGbufCompositeImage() const { return m_gbufComposite.image; }
	vk::Image getGbufNormalImage() const { return m_gbufNormal.image; }

	// G-buffer normal copy (for decal angle rejection sampling)
	vk::ImageView getGbufNormalCopyView() const { return m_gbufNormalCopy.view; }

	/**
	 * @brief Transition G-buffer color attachments 1-5 for render pass resume
	 *
	 * After ending the G-buffer render pass, all color attachments are in
	 * eShaderReadOnlyOptimal. The eLoad pass expects eColorAttachmentOptimal.
	 * The caller handles attachment 0 (scene color); this transitions the rest.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void transitionGbufForResume(vk::CommandBuffer cmd);

	// ========== Deferred Light Accumulation ==========

	/**
	 * @brief Render deferred lights into the composite buffer
	 *
	 * Reads G-buffer textures, renders light volumes (fullscreen, sphere, cylinder)
	 * with additive blending into the composite attachment. Manages its own render
	 * pass internally.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void renderDeferredLights(vk::CommandBuffer cmd);

	/**
	 * @brief Get the light accumulation render pass
	 */
	vk::RenderPass getLightAccumRenderPass() const { return m_lightAccumRenderPass; }

	/**
	 * @brief Get the light accumulation framebuffer
	 */
	vk::Framebuffer getLightAccumFramebuffer() const { return m_lightAccumFramebuffer; }

	// ========== Shadow Map ==========

	/**
	 * @brief Initialize shadow map resources (lazy, called on first use)
	 * @return true on success
	 */
	bool initShadowPass();

	/**
	 * @brief Shutdown shadow map resources
	 */
	void shutdownShadowPass();

	/**
	 * @brief Check if shadow map resources are initialized
	 */
	bool isShadowInitialized() const { return m_shadowInitialized; }

	/**
	 * @brief Get shadow map texture size (square)
	 */
	int getShadowTextureSize() const { return m_shadowTextureSize; }

	/**
	 * @brief Get shadow color image view (2D array, 4 layers) for descriptor binding
	 */
	vk::ImageView getShadowColorView() const { return m_shadowColor.view; }

	/**
	 * @brief Get shadow color image (for layout transitions)
	 */
	vk::Image getShadowColorImage() const { return m_shadowColor.image; }

	/**
	 * @brief Get shadow depth image (for layout transitions)
	 */
	vk::Image getShadowDepthImage() const { return m_shadowDepth.image; }

	/**
	 * @brief Get shadow render pass
	 */
	vk::RenderPass getShadowRenderPass() const { return m_shadowRenderPass; }

	/**
	 * @brief Get shadow framebuffer
	 */
	vk::Framebuffer getShadowFramebuffer() const { return m_shadowFramebuffer; }

	/**
	 * @brief Get shadow map sampler (linear, clamp-to-edge)
	 */
	vk::Sampler getShadowSampler() const { return m_linearSampler; }

	// ========== MSAA (deferred lighting) ==========

	/**
	 * @brief Check if MSAA G-buffer resources are initialized
	 */
	bool isMsaaInitialized() const { return m_msaaInitialized; }

	/**
	 * @brief Get the MSAA G-buffer render pass (eClear variant)
	 */
	vk::RenderPass getMsaaGbufRenderPass() const { return m_msaaGbufRenderPass; }

	/**
	 * @brief Get the MSAA G-buffer render pass (eLoad variant, emissive preserving)
	 */
	vk::RenderPass getMsaaGbufRenderPassLoad() const { return m_msaaGbufRenderPassLoad; }

	/**
	 * @brief Get the MSAA G-buffer framebuffer
	 */
	vk::Framebuffer getMsaaGbufFramebuffer() const { return m_msaaGbufFramebuffer; }

	/**
	 * @brief Get the MSAA resolve render pass (writes to non-MSAA G-buffer)
	 */
	vk::RenderPass getMsaaResolveRenderPass() const { return m_msaaResolveRenderPass; }

	/**
	 * @brief Get the MSAA resolve framebuffer (non-MSAA G-buffer images)
	 */
	vk::Framebuffer getMsaaResolveFramebuffer() const { return m_msaaResolveFramebuffer; }

	/**
	 * @brief Get the emissive copy render pass (for upsampling to MSAA)
	 */
	vk::RenderPass getMsaaEmissiveCopyRenderPass() const { return m_msaaEmissiveCopyRenderPass; }

	/**
	 * @brief Get the emissive copy framebuffer (MSAA emissive target)
	 */
	vk::Framebuffer getMsaaEmissiveCopyFramebuffer() const { return m_msaaEmissiveCopyFramebuffer; }

	/**
	 * @brief Get MSAA image views for resolve shader binding
	 */
	vk::ImageView getMsaaColorView() const { return m_msaaColor.view; }
	vk::ImageView getMsaaPositionView() const { return m_msaaPosition.view; }
	vk::ImageView getMsaaNormalView() const { return m_msaaNormal.view; }
	vk::ImageView getMsaaSpecularView() const { return m_msaaSpecular.view; }
	vk::ImageView getMsaaEmissiveView() const { return m_msaaEmissive.view; }
	vk::ImageView getMsaaDepthView() const { return m_msaaDepthView; }
	vk::Image getMsaaColorImage() const { return m_msaaColor.image; }
	vk::Image getMsaaPositionImage() const { return m_msaaPosition.image; }
	vk::Image getMsaaNormalImage() const { return m_msaaNormal.image; }
	vk::Image getMsaaSpecularImage() const { return m_msaaSpecular.image; }
	vk::Image getMsaaEmissiveImage() const { return m_msaaEmissive.image; }
	vk::Image getMsaaDepthImage() const { return m_msaaDepthImage; }

	/**
	 * @brief Get MSAA resolve UBO buffer and mapped pointer
	 *
	 * Per-frame slots (one per MAX_FRAMES_IN_FLIGHT) hold {samples, fov} data.
	 * Persistently mapped. Caller writes to the current frame's slot.
	 */
	vk::Buffer getMsaaResolveUBO() const { return m_msaaResolveUBO; }
	void* getMsaaResolveUBOMapped() const { return m_msaaResolveUBOMapped; }

	/**
	 * @brief Transition MSAA images to expected layout before eClear render pass
	 *
	 * Uses oldLayout=eUndefined so it works regardless of current layout (first
	 * frame: UNDEFINED, subsequent: eShaderReadOnlyOptimal from resolve).
	 * Content is discarded — caller must use eClear loadOp.
	 */
	void transitionMsaaGbufForBegin(vk::CommandBuffer cmd);

	/**
	 * @brief Get MSAA color attachment count (5 — no composite in MSAA pass)
	 */
	static constexpr uint32_t MSAA_COLOR_ATTACHMENT_COUNT = 5;

	/**
	 * @brief Transition MSAA G-buffer color attachments for render pass resume
	 */
	void transitionMsaaGbufForResume(vk::CommandBuffer cmd);

	// ========== Fog / Volumetric Nebula ==========

	/**
	 * @brief Render scene fog into scene color
	 *
	 * Reads composite (lit result) + depth copy -> writes scene color.
	 * Must be called outside a render pass. After return, scene color
	 * is in eColorAttachmentOptimal.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void renderSceneFog(vk::CommandBuffer cmd);

	/**
	 * @brief Render volumetric nebula fog into scene color
	 *
	 * Reads composite + mipmapped emissive + depth copy + 3D volume textures
	 * -> writes scene color. Must be called outside a render pass.
	 * After return, scene color is in eColorAttachmentOptimal.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void renderVolumetricFog(vk::CommandBuffer cmd);

private:
	void updateTonemappingUBO();

	bool createImage(uint32_t width, uint32_t height, vk::Format format,
	                 vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect,
	                 vk::Image& outImage, vk::ImageView& outView,
	                 VulkanAllocation& outAllocation,
	                 vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1);

	// G-buffer methods (deferred lighting)
	bool initGBuffer();
	void shutdownGBuffer();

	// Light volume methods (deferred lighting)
	bool initLightVolumes();
	void shutdownLightVolumes();
	bool initLightAccumPass();

	// LDR target methods
	bool initLDRTargets();
	void shutdownLDRTargets();

	// Bloom pipeline methods
	bool initBloom();
	void shutdownBloom();
	void generateMipmaps(vk::CommandBuffer cmd, vk::Image image,
	                     uint32_t width, uint32_t height, uint32_t mipLevels);
	void drawFullscreenTriangle(vk::CommandBuffer cmd, vk::RenderPass renderPass,
	                            vk::Framebuffer framebuffer, vk::Extent2D extent,
	                            int shaderType,
	                            vk::ImageView textureView, vk::Sampler sampler,
	                            const void* uboData, size_t uboSize,
	                            int blendMode);

	struct RenderTarget {
		vk::Image image;
		vk::ImageView view;
		VulkanAllocation allocation;
		vk::Format format = vk::Format::eUndefined;
		uint32_t width = 0;
		uint32_t height = 0;
	};

	RenderTarget m_sceneColor;      // RGBA16F HDR scene color
	RenderTarget m_sceneDepth;      // Depth buffer for scene
	RenderTarget m_sceneDepthCopy;  // Samplable copy of scene depth (for soft particles)
	RenderTarget m_sceneEffect;     // RGBA16F effect/composite (snapshot of scene color)

	// Scene render pass and framebuffer
	vk::RenderPass m_sceneRenderPass;       // loadOp=eClear (initial scene begin)
	vk::RenderPass m_sceneRenderPassLoad;   // loadOp=eLoad (resume after copy_effect_texture)
	vk::Framebuffer m_sceneFramebuffer;     // Shared by both scene render passes (compatible)

	// Sampler for post-processing texture reads (maxLod=0)
	vk::Sampler m_linearSampler;
	// Sampler with mipmap support for bloom textures
	vk::Sampler m_mipmapSampler;

	// Persistent UBO for tonemapping shader parameters
	vk::Buffer m_tonemapUBO;
	VulkanAllocation m_tonemapUBOAlloc;

	// ---- Bloom resources ----
	static constexpr int MAX_MIP_BLUR_LEVELS = 4;
	static constexpr size_t BLOOM_UBO_SLOT_SIZE = 256; // >= minUniformBufferOffsetAlignment

	struct BloomTarget {
		vk::Image image;
		VulkanAllocation allocation;
		vk::ImageView fullView;                      // All mip levels (for textureLod sampling)
		vk::ImageView mipViews[MAX_MIP_BLUR_LEVELS]; // Per-mip views (for framebuffer attachment)
		vk::Framebuffer mipFramebuffers[MAX_MIP_BLUR_LEVELS];
	};

	BloomTarget m_bloomTex[2];                 // Half-res RGBA16F, 4 mip levels
	uint32_t m_bloomWidth = 0;                 // Half of scene width
	uint32_t m_bloomHeight = 0;                // Half of scene height
	vk::RenderPass m_bloomRenderPass;          // Color-only RGBA16F, loadOp=eDontCare
	vk::RenderPass m_bloomCompositeRenderPass; // Color-only RGBA16F, loadOp=eLoad (additive to scene)
	vk::Framebuffer m_sceneColorBloomFB;       // Scene_color as color attachment for bloom composite

	// Per-draw UBO for bloom passes (each draw uses different offset)
	vk::Buffer m_bloomUBO;
	VulkanAllocation m_bloomUBOAlloc;
	void* m_bloomUBOMapped = nullptr;
	uint32_t m_bloomUBOCursor = 0;             // Current slot index (reset per frame)
	static constexpr uint32_t BLOOM_UBO_MAX_SLOTS = 24;

	bool m_bloomInitialized = false;

	// ---- LDR / FXAA resources ----
	RenderTarget m_sceneLdr;           // RGBA8 LDR after tonemapping
	RenderTarget m_sceneLuminance;     // RGBA8 LDR with luma in alpha (for FXAA)
	vk::RenderPass m_ldrRenderPass;    // Color-only RGBA8, loadOp=eDontCare
	vk::RenderPass m_ldrLoadRenderPass; // Color-only RGBA8, loadOp=eLoad (for additive blending)
	vk::Framebuffer m_sceneLdrFB;
	vk::Framebuffer m_sceneLuminanceFB;
	bool m_ldrInitialized = false;
	bool m_postEffectsApplied = false; // Set per-frame by executePostEffects

public:
	// Attachment layout: [0]=color, [1]=position, [2]=normal, [3]=specular, [4]=emissive, [5]=composite, [6]=depth
	static constexpr uint32_t GBUF_COLOR_ATTACHMENT_COUNT = 6;

private:
	// ---- G-Buffer (deferred lighting) ----
	RenderTarget m_gbufPosition;   // RGBA16F - view-space position (xyz) + AO (w)
	RenderTarget m_gbufNormal;     // RGBA16F - view-space normal (xyz) + gloss (w)
	RenderTarget m_gbufNormalCopy; // RGBA16F - samplable copy of G-buffer normal (for decals)
	RenderTarget m_gbufSpecular;   // RGBA8   - specular color (rgb) + fresnel (a)
	RenderTarget m_gbufEmissive;   // RGBA16F - emissive / pre-lit color
	RenderTarget m_gbufComposite;  // RGBA16F - light accumulation scratch buffer
	vk::RenderPass m_gbufRenderPass;      // loadOp=eClear (initial)
	vk::RenderPass m_gbufRenderPassLoad;  // loadOp=eLoad (resume after mid-pass copy)
	vk::Framebuffer m_gbufFramebuffer;
	bool m_gbufInitialized = false;

	// ---- Light accumulation (deferred lighting) ----
	vk::RenderPass m_lightAccumRenderPass;    // Single RGBA16F color, loadOp=eLoad, additive blend
	vk::Framebuffer m_lightAccumFramebuffer;  // Composite image as attachment 0

	// Light volume meshes (sphere + cylinder for positional lights)
	struct LightVolumeMesh {
		vk::Buffer vbo;
		VulkanAllocation vboAlloc;
		vk::Buffer ibo;
		VulkanAllocation iboAlloc;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
	};
	LightVolumeMesh m_sphereMesh;
	LightVolumeMesh m_cylinderMesh;

	// Per-frame UBO for deferred light data (lights + globals + matrices)
	vk::Buffer m_deferredUBO;
	VulkanAllocation m_deferredUBOAlloc;
	static constexpr uint32_t DEFERRED_UBO_SIZE = 256 * 1024;  // 256KB for light data

	bool m_lightVolumesInitialized = false;

	// ---- MSAA G-buffer ----
	RenderTarget m_msaaColor;       // RGBA16F (MS)
	RenderTarget m_msaaPosition;    // RGBA16F (MS)
	RenderTarget m_msaaNormal;      // RGBA16F (MS)
	RenderTarget m_msaaSpecular;    // RGBA8 (MS)
	RenderTarget m_msaaEmissive;    // RGBA16F (MS)
	vk::Image m_msaaDepthImage;
	vk::ImageView m_msaaDepthView;
	VulkanAllocation m_msaaDepthAlloc;
	vk::RenderPass m_msaaGbufRenderPass;        // eClear, 5 MS color + MS depth
	vk::RenderPass m_msaaGbufRenderPassLoad;    // eLoad (emissive preserved), 5 MS color + MS depth
	vk::Framebuffer m_msaaGbufFramebuffer;
	vk::RenderPass m_msaaResolveRenderPass;     // 5 non-MSAA color + depth (via gl_FragDepth)
	vk::Framebuffer m_msaaResolveFramebuffer;
	vk::RenderPass m_msaaEmissiveCopyRenderPass;   // 1 MS color att (for upsample)
	vk::Framebuffer m_msaaEmissiveCopyFramebuffer;
	// Per-frame UBO for MSAA resolve shader data (samples, fov)
	vk::Buffer m_msaaResolveUBO;
	VulkanAllocation m_msaaResolveUBOAlloc;
	void* m_msaaResolveUBOMapped = nullptr;
	bool m_msaaInitialized = false;
	bool initMSAA();
	void shutdownMSAA();

	// ---- Shadow map (cascaded VSM) ----
	RenderTarget m_shadowColor;       // RGBA16F, 2D array (4 layers)
	RenderTarget m_shadowDepth;       // D32F, 2D array (4 layers)
	vk::RenderPass m_shadowRenderPass;
	vk::Framebuffer m_shadowFramebuffer;
	int m_shadowTextureSize = 0;
	bool m_shadowInitialized = false;

	// ---- Fog resources ----
	vk::RenderPass m_fogRenderPass;           // Color-only RGBA16F, loadOp=eDontCare, finalLayout=eColorAttachmentOptimal
	vk::Framebuffer m_fogFramebuffer;         // Scene color as color attachment
	bool m_fogInitialized = false;
	bool initFogPass();
	void shutdownFogPass();

	// Mipmapped emissive copy for volumetric fog LOD sampling
	RenderTarget m_emissiveMipmapped;         // RGBA16F with full mip chain
	uint32_t m_emissiveMipLevels = 0;
	vk::ImageView m_emissiveMipmappedFullView;  // View with all mip levels
	bool m_emissiveMipmappedInitialized = false;

	// ---- Distortion ping-pong textures (32x32 RGBA8) ----
	RenderTarget m_distortionTex[2];
	int m_distortionSwitch = 0;        // Which texture is the current read source
	float m_distortionTimer = 0.0f;    // Accumulator for ~30ms update interval
	vk::Sampler m_distortionSampler;   // LINEAR filter, REPEAT wrapping
	bool m_distortionInitialized = false;
	bool m_distortionFirstUpdate = true;  // First update needs eUndefined old layout

	vk::Device m_device;
	VulkanMemoryManager* m_memoryManager = nullptr;
	vk::Extent2D m_extent;
	vk::Format m_depthFormat = vk::Format::eUndefined;

	bool m_initialized = false;
};

// Global post-processor access
VulkanPostProcessor* getPostProcessor();
void setPostProcessor(VulkanPostProcessor* pp);

// gr_screen function pointer implementations for post-processing
void vulkan_post_process_begin();
void vulkan_post_process_end();
void vulkan_post_process_save_zbuffer();
void vulkan_post_process_restore_zbuffer();
void vulkan_post_process_set_effect(const char* name, int value, const vec3d* rgb);
void vulkan_post_process_set_defaults();

/**
 * @brief Copy one image to another with automatic barrier management
 *
 * Handles pre-barriers (src→eTransferSrcOptimal, dst→eTransferDstOptimal),
 * the copy command, and post-barriers (eTransferSrc→srcNewLayout, eTransferDst→dstNewLayout).
 * Access masks and pipeline stages are derived from the layouts automatically.
 *
 * Skip rule: if srcNewLayout == eTransferSrcOptimal, the src post-barrier is skipped
 * (image stays in transfer source layout). Same for dst + eTransferDstOptimal.
 *
 * @param cmd         Active command buffer (must be outside a render pass)
 * @param src         Source image
 * @param srcOldLayout Current layout of source image
 * @param srcNewLayout Desired layout of source image after copy
 * @param dst         Destination image
 * @param dstOldLayout Current layout of destination image
 * @param dstNewLayout Desired layout of destination image after copy
 * @param extent      Copy region (width x height)
 * @param aspect      Image aspect (eColor or eDepth)
 * @param dstMipLevels Number of mip levels in dst subresource range (for pre-barrier)
 */
void copyImageToImage(
    vk::CommandBuffer cmd,
    vk::Image src, vk::ImageLayout srcOldLayout, vk::ImageLayout srcNewLayout,
    vk::Image dst, vk::ImageLayout dstOldLayout, vk::ImageLayout dstNewLayout,
    vk::Extent2D extent,
    vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor,
    uint32_t dstMipLevels = 1);

} // namespace vulkan
} // namespace graphics
