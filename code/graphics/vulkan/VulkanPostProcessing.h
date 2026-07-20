#pragma once

#include "globalincs/pstypes.h"
#include "VulkanMemory.h"
#include "VulkanConstants.h"
#include "VulkanPerFrameUbo.h"

#include <array>
#include <vulkan/vulkan.hpp>


namespace graphics::vulkan {

struct RenderTarget;

/**
 * @brief Shared drawing infrastructure for post-processing subsystems
 *
 * Bundles the handles and helpers that every post-processing pass needs
 * (logical device, memory manager, scene geometry, samplers, the per-draw
 * scratch UBO ring, and the image/fullscreen-draw helpers). Owned by
 * VulkanPostProcessor and intended to be passed by reference to the
 * individual subsystem passes as they are extracted into their own types.
 */
struct PostProcessContext {
	vk::Device device;
	VulkanMemoryManager* memoryManager = nullptr;
	vk::Extent2D sceneExtent;
	vk::Format depthFormat = vk::Format::eUndefined;

	// True when the renderer negotiated an HDR10 swap chain. Drives fp16 LDR
	// intermediates and the HDR scene-tonemap path.
	bool hdrActive = false;
	// Display-referred LDR intermediate format: fp16 in HDR (so the scene can
	// carry values above paper white), 8-bit UNORM in SDR.
	vk::Format ldrFormat = LDR_COLOR_FORMAT;

	// Shared samplers for post-processing texture reads
	vk::Sampler linearSampler;   // maxLod=0
	vk::Sampler mipmapSampler;   // mipmap support (bloom)

	// Per-draw scratch UBO ring shared by all fullscreen effect passes.
	// Per-frame-in-flight regions (see PerFrameUboRing) so the CPU never
	// overwrites slots the previous frame's GPU work is still reading. The
	// cursor is reset exactly once per frame in VulkanPostProcessor::beginFrame()
	// -- the slot budget below covers ALL fullscreen passes of one frame
	// (fog/volumetrics mid-scene + bloom/tonemap/AA/lightshafts/post-effects).
	//
	// Slot size must fit the largest fullscreen-pass UBO: volumetric_fog_data
	// (guarded by a static_assert in VulkanPostProcessingFog.cpp).
	static constexpr size_t   SCRATCH_UBO_SLOT_SIZE = 512; // multiple of minUniformBufferOffsetAlignment
	static constexpr uint32_t SCRATCH_UBO_MAX_SLOTS = 32;
	PerFrameUboRing scratchRing;

	/**
	 * @brief Destroy a RenderTarget's view/image/allocation and reset its fields
	 *
	 * Safe to call on a partially-created or already-destroyed target.
	 */
	void destroyTarget(RenderTarget& rt) const;

	/**
	 * @brief Create a single-mip 2D image + view backed by GPU-only memory
	 */
	bool createImage(uint32_t width, uint32_t height, vk::Format format,
	                 vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect,
	                 vk::Image& outImage, vk::ImageView& outView,
	                 VulkanAllocation& outAllocation,
	                 vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1)
	                 const;

	/**
	 * @brief Draw a fullscreen triangle through the post-processing pipeline
	 *
	 * Begins/ends the given render pass and binds material + per-draw sets.
	 * Optional UBO data is written into the next scratch UBO slot.
	 *
	 * @param sampleCount Pipeline sample count; only needed when renderPass is a
	 *                    multisampled render pass (e.g. an MSAA G-buffer pass).
	 * @param bindGlobalSet Also allocate/write/bind the Global descriptor set.
	 *                      Normal callers rely on Set 0 already being bound from
	 *                      frame setup; pass true for draws that may run before
	 *                      that binding happened this frame (e.g. mid-G-buffer-pass
	 *                      copies that run ahead of any material draw).
	 * @param clearColor When set, clears color attachment 0 to this RGBA value via
	 *                   vkCmdClearAttachments immediately after beginning the render
	 *                   pass, regardless of the render pass's own loadOp. For passes
	 *                   whose shader discards some pixels (e.g. SMAA edge detection)
	 *                   and therefore can't rely on loadOp=eDontCare leaving them at
	 *                   any particular value.
	 */
	void drawFullscreenTriangle(vk::CommandBuffer cmd, vk::RenderPass renderPass,
	                            vk::Framebuffer framebuffer, vk::Extent2D extent,
	                            int shaderType,
	                            vk::ImageView textureView, vk::Sampler sampler,
	                            const void* uboData, size_t uboSize,
	                            int blendMode,
	                            unsigned int shaderFlags = 0,
	                            vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1,
	                            bool bindGlobalSet = false,
	                            const std::array<float, 4>* clearColor = nullptr);

	/**
	 * @brief Draw a fullscreen triangle sampling multiple textures
	 *
	 * Like drawFullscreenTriangle, but binds `viewCount` textures (all through
	 * the shared linear sampler) as elements 0..viewCount-1 of the
	 * Material.TextureArray binding, for shaders that need more than one input
	 * (e.g. SMAA's blending-weight/neighborhood-blending passes).
	 */
	void drawFullscreenTriangleMulti(vk::CommandBuffer cmd, vk::RenderPass renderPass,
	                                 vk::Framebuffer framebuffer, vk::Extent2D extent,
	                                 int shaderType,
	                                 const vk::ImageView* views, uint32_t viewCount,
	                                 const void* uboData, size_t uboSize);

	/**
	 * @brief Generate a mip chain for an image (transitions mip 0 first)
	 */
	static void generateMipmaps(vk::CommandBuffer cmd, vk::Image image,
	                            uint32_t width, uint32_t height, uint32_t mipLevels);

	/**
	 * @brief Create the shared scratch UBO used by drawFullscreenTriangle
	 * @return false on failure (caller should treat as fatal)
	 */
	bool initScratchUBO();
	void shutdownScratchUBO();
};

/**
 * @brief A single-mip 2D color/depth render target (image + view + allocation)
 *
 * Shared value type used by the post-processor and its subsystems.
 */
struct RenderTarget {
	vk::Image image;
	vk::ImageView view;
	VulkanAllocation allocation;
	vk::Format format = vk::Format::eUndefined;
	uint32_t width = 0;
	uint32_t height = 0;
};

/**
 * @brief Distortion ping-pong textures (32x32 RGBA8)
 *
 * Self-contained subsystem: owns two ping-pong textures plus a LINEAR/REPEAT
 * sampler, and scrolls/refreshes them on a ~30ms timer to match OpenGL's
 * gr_opengl_update_distortion(). Thruster rendering samples the most recently
 * written texture via getTextureInfo().
 */
class VulkanDistortion {
public:
	bool init(PostProcessContext& ctx);
	void shutdown();

	/**
	 * @brief Advance the ping-pong textures (no-op until the ~30ms timer fires)
	 * @param cmd Active command buffer (must be outside a render pass)
	 * @param frametime Time since last frame in seconds
	 */
	void update(vk::CommandBuffer cmd, float frametime);

	bool isInitialized() const { return m_initialized; }

	/**
	 * @brief DescriptorImageInfo for the current (most recently written) texture
	 *
	 * Returns a default-constructed info if not initialized.
	 */
	vk::DescriptorImageInfo getTextureInfo() const;

private:
	PostProcessContext* m_ctx = nullptr;
	std::array<RenderTarget, 2> m_tex;
	int m_switch = 0;          // Which texture is the current read source
	float m_timer = 0.0f;      // Accumulator for ~30ms update interval
	vk::Sampler m_sampler;     // LINEAR filter, REPEAT wrapping
	bool m_initialized = false;
	bool m_firstUpdate = true; // First update needs eUndefined old layout
};

/**
 * @brief Cascaded shadow map (depth-only, hardware PCF) render target
 *
 * Self-contained, lazily-initialized subsystem: owns the layered depth array
 * image, its comparison sampler, render pass, and layered framebuffer. Sized
 * from the current Shadow_quality and the current total cascade count
 * (Num_shadow_cascades + Num_cockpit_shadow_cascades) on init.
 */
class VulkanShadowMap {
public:
	/**
	 * @brief Lazily create shadow resources (sized from Shadow_quality)
	 * @return false if shadows are disabled or creation failed
	 */
	bool init(PostProcessContext& ctx);
	void shutdown();

	bool isInitialized() const { return m_initialized; }
	int textureSize() const { return m_textureSize; }
	vk::ImageView depthView() const { return m_depth.view; }
	vk::Image depthImage() const { return m_depth.image; }
	vk::Sampler compareSampler() const { return m_compareSampler; }
	vk::RenderPass renderPass() const { return m_renderPass; }
	vk::Framebuffer framebuffer() const { return m_framebuffer; }

private:
	PostProcessContext* m_ctx = nullptr;
	RenderTarget m_depth;   // D32F, 2D array (Num_shadow_cascades + Num_cockpit_shadow_cascades layers)
	vk::Sampler m_compareSampler; // Depth-compare sampler for hardware PCF (sampler2DArrayShadow)
	vk::RenderPass m_renderPass;
	vk::Framebuffer m_framebuffer;
	int m_textureSize = 0;
	bool m_initialized = false;
};

/**
 * @brief HDR bloom (bright-pass → mip blur → additive composite)
 *
 * Self-contained subsystem owning two half-resolution mip chains plus the
 * bloom/composite render passes. Composites additively back into the scene
 * color target, which is owned by the post-processor and referenced here.
 */
class VulkanBloom {
public:
	static constexpr int MAX_MIP_BLUR_LEVELS = 4;

	/**
	 * @brief Create bloom resources (sized to half the scene extent)
	 * @param sceneColor Scene HDR color target to composite into (must outlive this)
	 */
	bool init(PostProcessContext& ctx, const RenderTarget& sceneColor);
	void shutdown();

	/**
	 * @brief Recreate the extent-sized targets/framebuffers (render passes kept)
	 *
	 * Device must be idle. Returns false on failure (caller should shut the
	 * subsystem down).
	 */
	bool resize();

	/**
	 * @brief Run the full bloom chain and composite it onto the scene color
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void execute(vk::CommandBuffer cmd);

	bool isInitialized() const { return m_initialized; }

private:
	bool createTargets();
	void destroyTargets();

	struct BloomTarget {
		vk::Image image;
		VulkanAllocation allocation;
		vk::ImageView fullView;                                          // All mips (textureLod sampling)
		std::array<vk::ImageView, MAX_MIP_BLUR_LEVELS> mipViews = {};    // Per-mip views (framebuffer attachment)
		std::array<vk::Framebuffer, MAX_MIP_BLUR_LEVELS> mipFramebuffers = {};
	};

	PostProcessContext* m_ctx = nullptr;
	const RenderTarget* m_sceneColor = nullptr;
	std::array<BloomTarget, 2> m_tex;          // Half-res RGBA16F, 4 mip levels
	uint32_t m_width = 0;                       // Half of scene width
	uint32_t m_height = 0;                      // Half of scene height
	vk::RenderPass m_renderPass;               // Color-only RGBA16F, loadOp=eDontCare
	vk::RenderPass m_compositeRenderPass;      // Color-only RGBA16F, loadOp=eLoad (additive to scene)
	vk::Framebuffer m_sceneColorFB;            // Scene color as attachment for bloom composite
	bool m_initialized = false;
};

/**
 * @brief Deferred geometry buffer (G-buffer) + optional MSAA G-buffer & resolve
 *
 * Cohesive subsystem owning the single-sample G-buffer targets (position, normal,
 * specular, emissive, composite + a samplable normal copy) and, when MSAA is
 * enabled, the multisample G-buffer that resolves into those single-sample
 * targets. Also owns the render-pass / framebuffer builders shared by both paths.
 * Renders into the scene color (attachment 0) and scene depth, both owned by the
 * post-processor and referenced here.
 */
class VulkanDeferredGBuffer {
public:
	// ===== G-Buffer Layout Constants =====
	// Full layout:  [0]=color, [1]=position, [2]=normal, [3]=specular, [4]=emissive, [5]=composite, [6]=depth
	// MSAA layout:  [0]=color, [1]=position, [2]=normal, [3]=specular, [4]=emissive, [5]=depth (no composite)
	static constexpr uint32_t GBUF_ATT_COLOR     = 0;
	static constexpr uint32_t GBUF_ATT_POSITION  = 1;
	static constexpr uint32_t GBUF_ATT_NORMAL    = 2;
	static constexpr uint32_t GBUF_ATT_SPECULAR  = 3;
	static constexpr uint32_t GBUF_ATT_EMISSIVE  = 4;
	static constexpr uint32_t GBUF_ATT_COMPOSITE = 5; // Full layout only

	static constexpr uint32_t GBUF_COLOR_ATTACHMENT_COUNT = 6; // Full layout (with composite)
	static constexpr uint32_t MSAA_COLOR_ATTACHMENT_COUNT = 5; // MSAA layout (without composite)

	static constexpr vk::Format GBUF_FORMAT_COLOR     = vk::Format::eR16G16B16A16Sfloat;
	static constexpr vk::Format GBUF_FORMAT_POSITION  = vk::Format::eR16G16B16A16Sfloat;
	static constexpr vk::Format GBUF_FORMAT_NORMAL    = vk::Format::eR16G16B16A16Sfloat;
	static constexpr vk::Format GBUF_FORMAT_SPECULAR  = vk::Format::eR8G8B8A8Unorm;
	static constexpr vk::Format GBUF_FORMAT_EMISSIVE  = vk::Format::eR16G16B16A16Sfloat;
	static constexpr vk::Format GBUF_FORMAT_COMPOSITE = vk::Format::eR16G16B16A16Sfloat;

	/**
	 * @brief Create the single-sample G-buffer (sized to the scene extent)
	 * @param sceneColor Scene HDR color (attachment 0); must outlive this
	 * @param sceneDepth Scene depth attachment; must outlive this
	 */
	bool init(PostProcessContext& ctx, const RenderTarget& sceneColor, const RenderTarget& sceneDepth);
	void shutdown();

	/**
	 * @brief Recreate all extent-sized G-buffer (and, if active, MSAA)
	 * targets/framebuffers; render passes are kept. Device must be idle.
	 */
	bool resize();

	/**
	 * @brief Create the multisample G-buffer + resolve resources (lazy)
	 * @return false if MSAA is disabled or creation failed
	 */
	bool initMsaa();
	void shutdownMsaa();

	bool isInitialized() const { return m_gbufInitialized; }
	bool isMsaaInitialized() const { return m_msaaInitialized; }

	// Mid-frame layout transitions / copies (all outside a render pass)
	void transitionForResume(vk::CommandBuffer cmd);
	void copyNormal(vk::CommandBuffer cmd);
	void transitionMsaaForBegin(vk::CommandBuffer cmd);
	void transitionMsaaForResume(vk::CommandBuffer cmd);

	// G-buffer accessors
	vk::RenderPass renderPass() const { return m_gbufRenderPass; }
	vk::RenderPass renderPassLoad() const { return m_gbufRenderPassLoad; }
	vk::Framebuffer framebuffer() const { return m_gbufFramebuffer; }
	vk::ImageView positionView() const { return m_gbufPosition.view; }
	vk::ImageView normalView() const { return m_gbufNormal.view; }
	vk::ImageView specularView() const { return m_gbufSpecular.view; }
	vk::ImageView emissiveView() const { return m_gbufEmissive.view; }
	vk::ImageView compositeView() const { return m_gbufComposite.view; }
	vk::ImageView normalCopyView() const { return m_gbufNormalCopy.view; }
	vk::Image emissiveImage() const { return m_gbufEmissive.image; }
	vk::Image compositeImage() const { return m_gbufComposite.image; }
	vk::Image normalImage() const { return m_gbufNormal.image; }

	// MSAA accessors
	vk::RenderPass msaaRenderPass() const { return m_msaaGbufRenderPass; }
	vk::RenderPass msaaRenderPassLoad() const { return m_msaaGbufRenderPassLoad; }
	vk::Framebuffer msaaFramebuffer() const { return m_msaaGbufFramebuffer; }
	vk::RenderPass msaaResolveRenderPass() const { return m_msaaResolveRenderPass; }
	vk::Framebuffer msaaResolveFramebuffer() const { return m_msaaResolveFramebuffer; }
	vk::RenderPass msaaEmissiveCopyRenderPass() const { return m_msaaEmissiveCopyRenderPass; }
	vk::Framebuffer msaaEmissiveCopyFramebuffer() const { return m_msaaEmissiveCopyFramebuffer; }
	vk::ImageView msaaColorView() const { return m_msaaColor.view; }
	vk::ImageView msaaPositionView() const { return m_msaaPosition.view; }
	vk::ImageView msaaNormalView() const { return m_msaaNormal.view; }
	vk::ImageView msaaSpecularView() const { return m_msaaSpecular.view; }
	vk::ImageView msaaEmissiveView() const { return m_msaaEmissive.view; }
	vk::ImageView msaaDepthView() const { return m_msaaDepthView; }
	vk::Image msaaColorImage() const { return m_msaaColor.image; }
	vk::Image msaaPositionImage() const { return m_msaaPosition.image; }
	vk::Image msaaNormalImage() const { return m_msaaNormal.image; }
	vk::Image msaaSpecularImage() const { return m_msaaSpecular.image; }
	vk::Image msaaEmissiveImage() const { return m_msaaEmissive.image; }
	vk::Image msaaDepthImage() const { return m_msaaDepthImage; }

private:
	struct GbufRenderPassConfig {
		bool includeComposite;
		vk::SampleCountFlagBits samples;
		vk::AttachmentLoadOp colorLoadOp;
		vk::AttachmentLoadOp depthLoadOp;
		vk::ImageLayout colorInitialLayout;
		vk::ImageLayout colorFinalLayout;
		vk::ImageLayout depthInitialLayout;
		bool useResolveDependency = false;
		SCP_unordered_map<uint32_t, vk::AttachmentLoadOp> attachmentLoadOpOverrides = {};
	};
	vk::RenderPass createGbufRenderPass(const GbufRenderPassConfig& config);
	vk::Framebuffer createGbufFramebuffer(vk::RenderPass renderPass, bool includeComposite,
	                                      bool useMsaaImages);

	bool createTargets();
	void destroyTargets();
	bool createMsaaTargets();
	void destroyMsaaTargets();

	PostProcessContext* m_ctx = nullptr;
	const RenderTarget* m_sceneColor = nullptr;
	const RenderTarget* m_sceneDepth = nullptr;

	// ---- G-Buffer (single-sample) ----
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

	// ---- MSAA G-buffer (multisample) + resolve ----
	RenderTarget m_msaaColor;       // RGBA16F (MS)
	RenderTarget m_msaaPosition;    // RGBA16F (MS)
	RenderTarget m_msaaNormal;      // RGBA16F (MS)
	RenderTarget m_msaaSpecular;    // RGBA8 (MS)
	RenderTarget m_msaaEmissive;    // RGBA16F (MS)
	vk::Image m_msaaDepthImage;
	vk::ImageView m_msaaDepthView;
	VulkanAllocation m_msaaDepthAlloc;
	vk::RenderPass m_msaaGbufRenderPass;        // eClear, 5 MS color + MS depth
	vk::RenderPass m_msaaGbufRenderPassLoad;    // eLoad (emissive preserved)
	vk::Framebuffer m_msaaGbufFramebuffer;
	vk::RenderPass m_msaaResolveRenderPass;     // 5 non-MSAA color + depth (via gl_FragDepth)
	vk::Framebuffer m_msaaResolveFramebuffer;
	vk::RenderPass m_msaaEmissiveCopyRenderPass;   // 1 MS color att (for upsample)
	vk::Framebuffer m_msaaEmissiveCopyFramebuffer;
	bool m_msaaInitialized = false;
};

/**
 * @brief Deferred light accumulation (light volumes → G-buffer composite)
 *
 * Lazily-initialized subsystem owning the light-volume meshes (sphere/cylinder),
 * the per-frame deferred light UBO, and the light-accumulation render pass that
 * additively blends every scene light into the G-buffer composite. Consumes the
 * G-buffer, the shadow map, and the scene color as read-only inputs.
 */
class VulkanDeferredLighting {
public:
	/**
	 * @brief Wire dependencies (GPU resources are created lazily on first render)
	 */
	void init(PostProcessContext& ctx, const RenderTarget& sceneColor,
	          const VulkanDeferredGBuffer& gbuffer, const VulkanShadowMap& shadow);
	void shutdown();

	/**
	 * @brief Recreate the light-accumulation framebuffer after a resize
	 *
	 * The framebuffer attaches the G-buffer composite view, which the G-buffer
	 * resize just recreated. No-op while the lazy resources don't exist yet.
	 */
	bool onResize();

	/**
	 * @brief Accumulate all scene lights into the G-buffer composite
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void render(vk::CommandBuffer cmd);

private:
	bool createLightAccumFramebuffer();

	// Light volume meshes (sphere + cylinder for positional lights)
	struct LightVolumeMesh {
		vk::Buffer vbo;
		VulkanAllocation vboAlloc;
		vk::Buffer ibo;
		VulkanAllocation iboAlloc;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
	};

	bool initLightVolumes();
	bool initLightAccumPass();

	// Sized for a generous simultaneous-light budget so a large battle (many ships'
	// weapons fire, thrusters, explosions, muzzle flashes) can't silently exceed
	// it -- render()'s overflow check skips the ENTIRE light-accumulation pass for
	// the frame (every light, including directional/ambient, not just whichever
	// light pushed it over) when this is undersized. Confirmed in the wild: a
	// mission log showed "Deferred UBO overflow (270464 > 262144)" repeatedly at
	// the old 256KB size, i.e. the light count was hovering right at capacity and
	// tipping over as dynamic lights came and went, blacking out lighting for
	// those frames entirely.
	//
	// Per-light cost is (sizeof(deferred_light_data) + sizeof(matrix_uniforms)),
	// each independently rounded up to minUniformBufferOffsetAlignment (device-
	// dependent, queried at runtime in render()) -- 256 bytes is a conservative
	// upper bound seen in practice, so budget worst-case 2x that per light.
	//
	// Sized well above the ~5000-object limit because particles, glowpoints, etc.
	// can each contribute lights, so the effective ceiling is much higher than the
	// object count. At ~8MB the buffer is cheap next to the frame's render targets,
	// so we over-provision rather than risk the all-or-nothing overflow above.
	static constexpr uint32_t DEFERRED_MAX_LIGHTS = 16384;
	static constexpr uint32_t DEFERRED_UBO_SIZE = 64 * 1024 + DEFERRED_MAX_LIGHTS * 512; // ~8.06MB

	PostProcessContext* m_ctx = nullptr;
	const RenderTarget* m_sceneColor = nullptr;
	const VulkanDeferredGBuffer* m_gbuffer = nullptr;
	const VulkanShadowMap* m_shadow = nullptr;

	LightVolumeMesh m_sphereMesh;
	LightVolumeMesh m_cylinderMesh;

	// Per-frame UBO for deferred light data (lights + globals + matrices)
	vk::Buffer m_deferredUBO;
	VulkanAllocation m_deferredUBOAlloc;

	vk::RenderPass m_lightAccumRenderPass;    // Single RGBA16F color, loadOp=eLoad, additive blend
	vk::Framebuffer m_lightAccumFramebuffer;  // Composite image as attachment 0
	bool m_lightVolumesInitialized = false;
};

/**
 * @brief Scene fog + volumetric nebula passes
 *
 * Lazily-initialized subsystem owning the fog render pass/framebuffer (which
 * targets the scene color) and the mipmapped emissive copy used for volumetric
 * LOD sampling. Consumes the G-buffer composite/emissive and the scene depth
 * copy, performs its own scene-depth copy, and writes into the scene color.
 */
class VulkanFog {
public:
	/**
	 * @brief Wire dependencies (GPU resources are created lazily on first render)
	 */
	void init(PostProcessContext& ctx, const RenderTarget& sceneColor,
	          const RenderTarget& sceneDepth, const RenderTarget& sceneDepthCopy,
	          const VulkanDeferredGBuffer& gbuffer);
	void shutdown();

	/**
	 * @brief Distance fog pass (composite + depth → scene color)
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void renderScene(vk::CommandBuffer cmd);

	/**
	 * @brief Volumetric nebula pass (composite + emissive mips + depth + 3D vol → scene color)
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void renderVolumetric(vk::CommandBuffer cmd);

	/**
	 * @brief Rewire extent-sized lazy resources after a resize
	 *
	 * Recreates the fog framebuffer (attaches the recreated scene color view)
	 * and drops the emissive mip chain so renderVolumetric() lazily rebuilds it
	 * at the new extent. No-op for resources that don't exist yet.
	 */
	bool onResize();

private:
	bool initFogPass();
	bool createFogFramebuffer();
	void copySceneDepth(vk::CommandBuffer cmd);
	void destroyEmissiveMipmapped();

	PostProcessContext* m_ctx = nullptr;
	const RenderTarget* m_sceneColor = nullptr;
	const RenderTarget* m_sceneDepth = nullptr;
	const RenderTarget* m_sceneDepthCopy = nullptr;
	const VulkanDeferredGBuffer* m_gbuffer = nullptr;

	vk::RenderPass m_fogRenderPass;           // Color-only RGBA16F, loadOp=eDontCare, finalLayout=eColorAttachmentOptimal
	vk::Framebuffer m_fogFramebuffer;         // Scene color as color attachment
	bool m_fogInitialized = false;

	// Mipmapped emissive copy for volumetric fog LOD sampling
	RenderTarget m_emissiveMipmapped;         // RGBA16F with full mip chain
	uint32_t m_emissiveMipLevels = 0;
	vk::ImageView m_emissiveMipmappedFullView;  // View with all mip levels
	bool m_emissiveMipmappedInitialized = false;
};

/**
 * @brief Tonemapping + FXAA + post-effects + lightshafts (the LDR pass chain)
 *
 * Self-contained subsystem owning the full-resolution LDR targets (tonemapped
 * output, luma-for-FXAA scratch, and a [0,1]-clamped detection proxy used by
 * FXAA/SMAA edge detection while HDR output is active) plus their render
 * passes/framebuffers. Reads the HDR scene color (tonemap) and scene depth
 * (lightshafts) as read-only inputs owned by the post-processor, and the
 * bloom subsystem's init state (to decide whether to reset the shared
 * scratch-UBO cursor, since bloom itself resets it when it runs).
 */
class VulkanLDR {
public:
	/**
	 * @brief Create LDR targets (sized to the scene extent)
	 */
	bool init(PostProcessContext& ctx, const RenderTarget& sceneColor, const RenderTarget& sceneDepth,
	          const VulkanBloom& bloom);
	void shutdown();

	/**
	 * @brief Recreate the extent-sized targets/framebuffers (render passes kept)
	 *
	 * Device must be idle. Returns false on failure (caller should shut the
	 * subsystem down).
	 */
	bool resize();

	bool isInitialized() const { return m_initialized; }

	/**
	 * @brief HDR scene → Scene_ldr via the tonemapping shader
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeTonemap(vk::CommandBuffer cmd);

	/**
	 * @brief FXAA prepass (luma) + main pass, both operating on Scene_ldr
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeFXAA(vk::CommandBuffer cmd);

	/**
	 * @brief Saturation/brightness/etc. post effects: Scene_ldr -> Scene_luminance
	 * @param cmd Active command buffer (must be outside a render pass)
	 * @return true if effects were applied (blit should read the luminance target)
	 */
	bool executePostEffects(vk::CommandBuffer cmd);

	/**
	 * @brief Lightshafts (god rays), additively blended onto Scene_ldr
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeLightshafts(vk::CommandBuffer cmd);

	/**
	 * @brief Buffer FXAA/SMAA edge detection should read for luma computation:
	 * the real Scene_ldr when it's already bounded to [0,1], or the compressed
	 * tonemap proxy when HDR output has left it in extended range.
	 */
	vk::ImageView getAADetectionView() const;

	bool postEffectsApplied() const { return m_postEffectsApplied; }

	// Accessors for VulkanSMAA (reuses this render pass/targets) and
	// VulkanPostProcessor::blitToSwapChain (reads the final LDR result).
	vk::RenderPass renderPass() const { return m_ldrRenderPass; }
	vk::RenderPass loadRenderPass() const { return m_ldrLoadRenderPass; }
	vk::ImageView ldrView() const { return m_sceneLdr.view; }
	vk::Framebuffer ldrFramebuffer() const { return m_sceneLdrFB; }
	vk::ImageView luminanceView() const { return m_sceneLuminance.view; }
	vk::Framebuffer luminanceFramebuffer() const { return m_sceneLuminanceFB; }

private:
	bool createTargets();
	void destroyTargets();

	PostProcessContext* m_ctx = nullptr;
	const RenderTarget* m_sceneColor = nullptr;
	const RenderTarget* m_sceneDepth = nullptr;
	const VulkanBloom* m_bloom = nullptr;

	RenderTarget m_sceneLdr;           // RGBA8 LDR after tonemapping
	RenderTarget m_sceneLuminance;     // RGBA8 LDR with luma in alpha (for FXAA)
	// RGBA8, a [0,1]-clamped copy of Scene_ldr (see executeTonemap); used only
	// as an edge-detection input for FXAA/SMAA (see getAADetectionView) so
	// their fixed luma thresholds stay valid even though Scene_ldr itself can
	// carry values above 1.0 while HDR is active. Unused (and left as a
	// duplicate of Scene_ldr's own data) when HDR is inactive.
	RenderTarget m_sceneLdrCompressed;
	vk::RenderPass m_ldrRenderPass;    // Color-only RGBA8, loadOp=eDontCare
	vk::RenderPass m_ldrLoadRenderPass; // Color-only RGBA8, loadOp=eLoad (for additive blending)
	vk::Framebuffer m_sceneLdrFB;
	vk::Framebuffer m_sceneLuminanceFB;
	vk::Framebuffer m_sceneLdrCompressedFB;
	bool m_initialized = false;
	bool m_postEffectsApplied = false; // Set per-frame by executePostEffects
};

/**
 * @brief SMAA anti-aliasing (edge detection -> blending weights -> neighborhood blend -> resolve)
 *
 * Self-contained subsystem owning the static SMAA area/search lookup textures
 * and the edge/blend-weight intermediate targets. Reuses VulkanLDR's render
 * pass and Scene_ldr/Scene_luminance targets (same format/loadOp/finalLayout),
 * so it depends on VulkanLDR rather than owning its own LDR-shaped state.
 */
class VulkanSMAA {
public:
	/**
	 * @brief Upload SMAA lookup textures and create edge/blend intermediate targets
	 * @param ldr Must already be initialized; its render pass and targets are reused.
	 */
	bool init(PostProcessContext& ctx, const VulkanLDR& ldr);
	void shutdown();

	/**
	 * @brief Recreate the extent-sized edge/blend targets and framebuffers
	 *
	 * The static area/search lookup textures are extent-independent and kept.
	 * Device must be idle. Returns false on failure (caller should shut the
	 * subsystem down).
	 */
	bool resize();

	bool isInitialized() const { return m_initialized; }

	/**
	 * @brief Run the full SMAA chain, resolving the result back into Scene_ldr
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void execute(vk::CommandBuffer cmd);

private:
	bool createTargets();
	void destroyTargets();

	PostProcessContext* m_ctx = nullptr;
	const VulkanLDR* m_ldr = nullptr;

	// Static lookup textures (uploaded once at init, never change)
	vk::Image m_smaaAreaTexImage;
	vk::ImageView m_smaaAreaTexView;
	VulkanAllocation m_smaaAreaTexAlloc;
	vk::Image m_smaaSearchTexImage;
	vk::ImageView m_smaaSearchTexView;
	VulkanAllocation m_smaaSearchTexAlloc;

	// Intermediate targets (reuse VulkanLDR's render pass: same format/loadOp/finalLayout)
	RenderTarget m_smaaEdges;  // RGBA8 edge detection output
	RenderTarget m_smaaBlend;  // RGBA8 blending weight output
	vk::Framebuffer m_smaaEdgesFB;
	vk::Framebuffer m_smaaBlendFB;
	bool m_initialized = false;
};

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
	          vk::Format depthFormat, bool hdrActive = false);

	/**
	 * @brief Shutdown and free all post-processing resources
	 */
	void shutdown();

	/**
	 * @brief Recreate all extent-sized targets/framebuffers for a new resolution
	 *
	 * Called from VulkanRenderer::recreateSwapChain() after the device-idle
	 * wait. Render passes, samplers, and the scratch UBO ring are extent-
	 * independent and kept alive, so every cached pipeline stays valid.
	 * Individual subsystem failures are non-fatal (the subsystem is disabled,
	 * matching init()); returns false only if the core scene targets could not
	 * be recreated.
	 */
	bool resize(vk::Extent2D newExtent);

	/**
	 * @brief Per-frame reset (called from VulkanRenderer::setupFrame after the frame fence wait)
	 *
	 * Resets the scratch UBO ring cursor for this frame-in-flight index. This is
	 * the ONLY place the cursor is reset -- the per-frame slot budget covers every
	 * fullscreen pass of the frame (mid-scene fog included), so subsystems must
	 * not reset it themselves.
	 */
	void beginFrame(uint32_t frameIndex) { m_ctx.scratchRing.resetCursor(frameIndex); }

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
	vk::Extent2D getSceneExtent() const { return m_ctx.sceneExtent; }

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
	 * @brief Final output-encode pass: composition image -> swap chain image (HDR10 leg)
	 *
	 * Begins/ends its own render pass on the swap chain framebuffer and draws a
	 * fullscreen triangle that samples the fp16 composition image, applying the
	 * user gamma/brightness slider followed by the PQ / BT.2020 (HDR10)
	 * transfer. Only called when the swap chain negotiated HDR10 output --
	 * the SDR leg uses encodeOutputSdr() instead. See
	 * VulkanRenderer::encodeToSwapChain().
	 */
	void encodeOutput(vk::CommandBuffer cmd, vk::RenderPass renderPass, vk::Framebuffer framebuffer,
	                  vk::Extent2D extent, vk::ImageView sourceView, vk::Sampler sampler,
	                  float paperwhiteNits, float peakNits, float gamma);

	/**
	 * @brief Final output-encode pass: composition image -> swap chain image (SDR leg)
	 *
	 * Begins/ends its own render pass on the swap chain framebuffer and draws a
	 * fullscreen triangle (SDR_TYPE_GAMMA_BLIT) that samples the fp16
	 * composition image -- which already carries the final display-ready
	 * sRGB-encoded frame (3D scene + menus/HUD alike) -- and applies the user
	 * gamma/brightness slider before writing it into the swap chain image.
	 * Always used for the SDR leg; there is no shader-less fast path, so the
	 * slider works uniformly regardless of device blit support.
	 */
	void encodeOutputSdr(vk::CommandBuffer cmd, vk::RenderPass renderPass, vk::Framebuffer framebuffer,
	                  vk::Extent2D extent, vk::ImageView sourceView, vk::Sampler sampler, float gamma);

  private:
	/**
	 * @brief Shared body of encodeOutput()/encodeOutputSdr()
	 *
	 * Begins/ends the given render pass on the swap chain framebuffer and draws a
	 * fullscreen triangle that samples sourceView, binding uboData into the
	 * per-draw GenericData slot from the scratch ring. The two public legs share
	 * the gamma-correct shader and differ only in shaderFlags (SDR vs
	 * HDR10_OUTPUT) + the UBO values.
	 */
	void encodeToSwapChainPass(vk::CommandBuffer cmd, vk::RenderPass renderPass, vk::Framebuffer framebuffer,
	                           vk::Extent2D extent, vk::ImageView sourceView, vk::Sampler sampler,
	                           int shaderType, unsigned int shaderFlags, const void* uboData, size_t uboSize);

  public:

	/**
	 * @brief Execute bloom post-processing passes
	 *
	 * Called after the HDR scene render pass ends and before the resumed
	 * swap chain render pass begins. Manages its own render passes internally.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeBloom(vk::CommandBuffer cmd) { m_bloom.execute(cmd); }

	/**
	 * @brief Execute tonemapping pass (HDR scene → LDR)
	 *
	 * Called after bloom and before FXAA. Renders to Scene_ldr (RGBA8).
	 * Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeTonemap(vk::CommandBuffer cmd) { m_ldr.executeTonemap(cmd); }

	/**
	 * @brief Execute FXAA anti-aliasing passes
	 *
	 * Called after tonemapping. Runs prepass (LDR→luminance) then
	 * FXAA main pass (luminance→LDR). Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeFXAA(vk::CommandBuffer cmd) { m_ldr.executeFXAA(cmd); }

	/**
	 * @brief Execute SMAA anti-aliasing passes
	 *
	 * Called after tonemapping (mutually exclusive with FXAA, selected via
	 * Gr_aa_mode). Runs edge detection, blending weight calculation, and
	 * neighborhood blending, then resolves the result back into Scene_ldr.
	 * Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeSMAA(vk::CommandBuffer cmd) { m_smaa.execute(cmd); }

	/**
	 * @brief Execute post-processing effects (saturation, brightness, etc.)
	 *
	 * Called after FXAA and before the final blit. Reads Scene_ldr, writes
	 * Scene_luminance (reused as temp target). Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 * @return true if effects were applied (blit should read Scene_luminance)
	 */
	bool executePostEffects(vk::CommandBuffer cmd) { return m_ldr.executePostEffects(cmd); }

	/**
	 * @brief Execute lightshafts (god rays) pass
	 *
	 * Called after FXAA and before post-effects. Additively blends god rays
	 * onto Scene_ldr based on sun position and depth buffer sampling.
	 * Must be called outside a render pass.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void executeLightshafts(vk::CommandBuffer cmd) { m_ldr.executeLightshafts(cmd); }

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
	void updateDistortion(vk::CommandBuffer cmd, float frametime) { m_distortion.update(cmd, frametime); }

	/**
	 * @brief Get a ready-to-use DescriptorImageInfo for the current distortion texture
	 *
	 * Returns the most recently written distortion texture (the one thrusters
	 * should read from). Returns a default-constructed info if not initialized.
	 */
	vk::DescriptorImageInfo getDistortionTextureInfo() const { return m_distortion.getTextureInfo(); }

	/**
	 * @brief Copy scene color to effect texture for distortion/soft particle sampling
	 *
	 * Must be called outside a render pass. Transitions scene color through
	 * eTransferSrcOptimal and back to eColorAttachmentOptimal (ready for resumed
	 * scene render pass). Transitions effect texture to eShaderReadOnlyOptimal.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void copyEffectTexture(vk::CommandBuffer cmd) const;

	/**
	 * @brief Copy scene depth to samplable depth copy for soft particle rendering
	 *
	 * Must be called outside a render pass. Transitions scene depth through
	 * eTransferSrcOptimal and back to eDepthStencilAttachmentOptimal. Transitions
	 * depth copy to eShaderReadOnlyOptimal for fragment shader sampling.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void copySceneDepth(vk::CommandBuffer cmd) const;

	/**
	 * @brief Check if LDR targets are available (tonemapping + FXAA ready)
	 */
	bool hasLDRTargets() const { return m_ldr.isInitialized(); }

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
	vk::Sampler getSceneColorSampler() const { return m_ctx.linearSampler; }

	/**
	 * @brief Get a ready-to-use DescriptorImageInfo for the scene effect texture
	 *
	 * Available for sampling after copyEffectTexture() has been called.
	 * Used by distortion and soft particle shaders.
	 * Returns default-constructed info if the effect texture doesn't exist.
	 */
	vk::DescriptorImageInfo getSceneEffectTextureInfo() const {
		if (!m_sceneEffect.view) return {};
		return {m_ctx.linearSampler, m_sceneEffect.view, vk::ImageLayout::eShaderReadOnlyOptimal};
	}

	/**
	 * @brief Get the scene depth copy view (for soft particle depth sampling)
	 *
	 * Available for sampling after copySceneDepth() has been called.
	 */
	vk::ImageView getSceneDepthCopyView() const { return m_sceneDepthCopy.view; }

	vk::Format getDepthFormat() const { return m_ctx.depthFormat; }

	/**
	 * @brief Check if post-processing is initialized
	 */
	bool isInitialized() const { return m_initialized; }

	// ========== Subsystem access ==========
	// Consumers that drive the deferred / shadow passes operate directly on the
	// owning subsystem (its resources, render passes, and mid-frame transitions)
	// rather than through dozens of per-resource forwarding accessors.
	VulkanDeferredGBuffer& deferred() { return m_deferred; }
	const VulkanDeferredGBuffer& deferred() const { return m_deferred; }
	VulkanShadowMap& shadow() { return m_shadow; }
	const VulkanShadowMap& shadow() const { return m_shadow; }

	// Shared draw/image helpers (fullscreen-triangle draws, image creation) for
	// free-function callers outside VulkanPostProcessor itself, e.g. the deferred
	// lighting pass driver in VulkanDeferred.cpp.
	PostProcessContext& context() { return m_ctx; }

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
	void renderDeferredLights(vk::CommandBuffer cmd) { m_lighting.render(cmd); }

	// ========== Shadow Map ==========

	/**
	 * @brief Initialize shadow map resources (lazy, called on first use)
	 * @return true on success
	 */
	bool initShadowPass() { return m_shadow.init(m_ctx); }

	/**
	 * @brief Shutdown shadow map resources
	 */
	void shutdownShadowPass() { m_shadow.shutdown(); }

	/**
	 * @brief Get a ready-to-use DescriptorImageInfo for the shadow map texture
	 *
	 * Uses the shadow map's depth-compare sampler so the shader can declare
	 * this binding as sampler2DArrayShadow and get hardware PCF.
	 */
	vk::DescriptorImageInfo getShadowTextureInfo() const {
		return {m_shadow.compareSampler(), m_shadow.depthView(), vk::ImageLayout::eShaderReadOnlyOptimal};
	}

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
	void renderSceneFog(vk::CommandBuffer cmd) { m_fog.renderScene(cmd); }

	/**
	 * @brief Render volumetric nebula fog into scene color
	 *
	 * Reads composite + mipmapped emissive + depth copy + 3D volume textures
	 * -> writes scene color. Must be called outside a render pass.
	 * After return, scene color is in eColorAttachmentOptimal.
	 *
	 * @param cmd Active command buffer (must be outside a render pass)
	 */
	void renderVolumetricFog(vk::CommandBuffer cmd) { m_fog.renderVolumetric(cmd); }

private:
	bool createSceneTargets(vk::Extent2D extent);
	void destroySceneTargets();
	bool createSceneFramebuffer();

	bool createImage(uint32_t width, uint32_t height, vk::Format format,
	                 vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect,
	                 vk::Image& outImage, vk::ImageView& outView,
	                 VulkanAllocation& outAllocation,
	                 vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1)
	                 const
	{
		return m_ctx.createImage(width, height, format, usage, aspect,
		                         outImage, outView, outAllocation, sampleCount);
	}

	// G-buffer / MSAA (forwards to the VulkanDeferredGBuffer subsystem)
	bool initGBuffer() { return m_deferred.init(m_ctx, m_sceneColor, m_sceneDepth); }
	void shutdownGBuffer() { m_deferred.shutdown(); }
	bool initMSAA() { return m_deferred.initMsaa(); }
	void shutdownMSAA() { m_deferred.shutdownMsaa(); }

	// Bloom pipeline (forwards to the VulkanBloom subsystem)
	bool initBloom() { return m_bloom.init(m_ctx, m_sceneColor); }
	void shutdownBloom() { m_bloom.shutdown(); }
	static void generateMipmaps(vk::CommandBuffer cmd, vk::Image image,
	                     uint32_t width, uint32_t height, uint32_t mipLevels)
	{
		PostProcessContext::generateMipmaps(cmd, image, width, height, mipLevels);
	}
	void drawFullscreenTriangle(vk::CommandBuffer cmd, vk::RenderPass renderPass,
	                            vk::Framebuffer framebuffer, vk::Extent2D extent,
	                            int shaderType,
	                            vk::ImageView textureView, vk::Sampler sampler,
	                            const void* uboData, size_t uboSize,
	                            int blendMode,
	                            unsigned int shaderFlags = 0)
	{
		m_ctx.drawFullscreenTriangle(cmd, renderPass, framebuffer, extent, shaderType,
		                             textureView, sampler, uboData, uboSize, blendMode, shaderFlags);
	}
	void drawFullscreenTriangleMulti(vk::CommandBuffer cmd, vk::RenderPass renderPass,
	                                 vk::Framebuffer framebuffer, vk::Extent2D extent,
	                                 int shaderType,
	                                 const vk::ImageView* views, uint32_t viewCount,
	                                 const void* uboData, size_t uboSize)
	{
		m_ctx.drawFullscreenTriangleMulti(cmd, renderPass, framebuffer, extent, shaderType,
		                                  views, viewCount, uboData, uboSize);
	}

	RenderTarget m_sceneColor;      // RGBA16F HDR scene color
	RenderTarget m_sceneDepth;      // Depth buffer for scene
	RenderTarget m_sceneDepthCopy;  // Samplable copy of scene depth (for soft particles)
	RenderTarget m_sceneEffect;     // RGBA16F effect/composite (snapshot of scene color)

	// Scene render pass and framebuffer
	vk::RenderPass m_sceneRenderPass;       // loadOp=eClear (initial scene begin)
	vk::RenderPass m_sceneRenderPassLoad;   // loadOp=eLoad (resume after copy_effect_texture)
	vk::Framebuffer m_sceneFramebuffer;     // Shared by both scene render passes (compatible)

	// ---- Bloom (self-contained subsystem) ----
	VulkanBloom m_bloom;

	// ---- LDR / FXAA / post-effects / lightshafts (self-contained subsystem) ----
	VulkanLDR m_ldr;

	// ---- SMAA (self-contained subsystem, depends on VulkanLDR) ----
	VulkanSMAA m_smaa;

	// ---- Deferred G-buffer + MSAA (self-contained subsystem) ----
	VulkanDeferredGBuffer m_deferred;

	// ---- Deferred light accumulation (self-contained subsystem) ----
	VulkanDeferredLighting m_lighting;

	// ---- Shadow map (cascaded VSM, self-contained subsystem) ----
	VulkanShadowMap m_shadow;

	// ---- Fog / volumetric nebula (self-contained subsystem) ----
	VulkanFog m_fog;

	// ---- Distortion (ping-pong textures, self-contained subsystem) ----
	VulkanDistortion m_distortion;

	PostProcessContext m_ctx;

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

} // namespace graphics::vulkan

