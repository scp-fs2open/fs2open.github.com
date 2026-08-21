#pragma once

#include "osapi/osapi.h"
#include "osapi/vulkan_surface.h"

#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"
#include "VulkanDescriptorManager.h"
#include "VulkanPipeline.h"
#include "VulkanState.h"
#include "VulkanDraw.h"
#include "VulkanDeletionQueue.h"
#include "VulkanPostProcessing.h"
#include "VulkanQuery.h"
#include "VulkanRaytracing.h"
#include "VulkanRenderFrame.h"
#include "VulkanPresentTarget.h"

#include <vulkan/vulkan.hpp>

namespace graphics::vulkan {

struct QueueIndex {
	// Poor mans std::optional
	bool initialized = false;
	uint32_t index = 0;
};

/**
 * @brief Viewport handling when beginning a tracked render pass
 */
enum class PassViewport {
	FlipY,  // Negative-height viewport for OpenGL-compatible Y-up NDC (scene/swap chain passes)
	NoFlip, // Standard positive viewport (post-processing blits)
	Keep,   // Leave the viewport untouched (off-screen RTs; the engine sets it via gr_set_viewport)
};

/**
 * @brief Parameters for VulkanRenderer::beginTrackedRenderPass()
 *
 * Bundles everything a render-pass begin must keep in sync: the pass/framebuffer,
 * the clear values, and the state-tracker bookkeeping (current pass, attachment
 * count, sample count, render area, viewport). Every render-pass begin on the
 * frame command buffer goes through this so no site can forget part of it.
 */
struct PassBeginDesc {
	vk::RenderPass renderPass;
	vk::Framebuffer framebuffer;
	vk::Extent2D extent;
	ArrayView<vk::ClearValue> clearValues; // may be empty for pure loadOp=eLoad passes
	uint32_t colorAttachmentCount = 1;
	vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
	PassViewport viewport = PassViewport::FlipY;
};

struct PhysicalDeviceValues {
	vk::PhysicalDevice device;
	vk::PhysicalDeviceProperties properties;
	vk::PhysicalDeviceFeatures features;

	// Ray tracing feature support, queried via a VkPhysicalDeviceFeatures2
	// pNext chain in pickPhysicalDevice(). These reflect only whether the
	// device *feature bit* is set -- whether raytraced shadows are actually
	// usable also requires the corresponding device extensions to be present
	// (checked separately in createLogicalDevice() against `extensions` below).
	bool accelerationStructureFeatureSupported = false;
	bool rayQueryFeatureSupported = false;
	bool bufferDeviceAddressFeatureSupported = false;

	// VK_KHR_synchronization2's feature bit. The extension itself is required
	// (RequiredDeviceExtensions), so this only guards against a device that
	// exposes the extension without the feature -- see isDeviceUnsuitable().
	bool synchronization2FeatureSupported = false;

	SCP_vector<vk::ExtensionProperties> extensions;

	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	SCP_vector<vk::SurfaceFormatKHR> surfaceFormats;
	SCP_vector<vk::PresentModeKHR> presentModes;

	SCP_vector<vk::QueueFamilyProperties> queueProperties;
	QueueIndex graphicsQueueIndex;
	QueueIndex presentQueueIndex;
};

class VulkanRenderer {
  public:
	explicit VulkanRenderer(std::unique_ptr<os::GraphicsOperations> graphicsOps);

	bool initialize();

	/**
	 * @brief Setup for a new frame - begins command buffer and render pass
	 * Called at the START of each frame before any draw calls
	 */
	void setupFrame();

	/**
	 * @brief Rebuild the swap chain if the window no longer matches it, restarting the frame
	 *
	 * The swap chain's extent is sampled at the end of the previous flip(), but gr_screen is set at
	 * the start of the current frame's drawing, so anything that resizes the window between those
	 * two points leaves the frame rendering into a swap chain of the wrong size: gr_setup_viewport()
	 * sets the viewport from gr_screen while the render pass area comes from the swap chain, and the
	 * difference shows up as a clipped image with unpainted bars around it. That gap is invisible in
	 * the game, where a resize is a window event outside rendering, but qtFRED calls
	 * gr_screen_resize() every single frame from its (freely resizable) viewport widget.
	 *
	 * Hooked up as gf_viewport_size_changed so gr_screen_resize() calls it, which is the moment both
	 * sizes can be sampled together. If the surface really has changed size, the in-progress frame
	 * is discarded (nothing has been drawn into it yet at that point), the swap chain and every
	 * extent-sized resource is rebuilt, and a fresh frame is started at the new size.
	 *
	 * The comparison is surface-against-swap-chain rather than gr_screen-against-swap-chain, on
	 * purpose: both of those come from the surface, so they agree exactly and this cannot thrash.
	 * gr_screen is computed independently by the caller and can be a pixel off from rounding.
	 *
	 * @return true if the swap chain was rebuilt
	 */
	bool syncToSurfaceExtent();

	/**
	 * @brief Make a viewport's surface the one subsequent drawing presents to
	 *
	 * Backs gr_use_viewport(). Creating the target on first use is deliberate: qtFRED's briefing map
	 * appears and disappears with its dialog, so there is no point in the session at which the full
	 * set of viewports is known.
	 *
	 * A frame is always already open when this is called -- gr_flip() ends with gr_setup_frame() --
	 * and it belongs to the outgoing target, which has also already acquired an image there. That
	 * frame is discarded rather than presented; see discardFrame().
	 *
	 * @return false if the viewport cannot be presented to, in which case the current target is left
	 * alone and the caller keeps drawing where it was
	 */
	bool useViewport(os::Viewport* viewport);

	/**
	 * @brief Drop the target belonging to a viewport that is going away
	 *
	 * Must be called while the renderer is still alive and before the viewport's window is
	 * destroyed. qtFRED's briefing editor is opened with WA_DeleteOnClose, so this happens against a
	 * live device every time the user closes the dialog.
	 */
	void releaseViewport(os::Viewport* viewport);

	/**
	 * @brief Whether the current target is the main one
	 *
	 * The post-processor is sized for and bound to the main target, so the scene-texture path stays
	 * off anywhere else. Nothing is lost by that today: qtFRED's briefing map renders through
	 * brief_render_map() and never opens a ScenePostProcessing scope.
	 */
	bool isMainTargetCurrent() const { return m_current == m_mainTarget; }

	/**
	 * @brief The extent the current target actually presents at, in device pixels
	 *
	 * This is what the render pass area and the framebuffers are sized to, so it is what gr_screen
	 * has to agree with. Callers must not compute it themselves from a window's logical size and a
	 * scale factor -- that rounds differently from the way the surface was sized.
	 */
	vk::Extent2D getCurrentTargetExtent() const { return m_current != nullptr ? m_current->extent : vk::Extent2D(); }

	/**
	 * @brief End frame - ends render pass, submits, and presents
	 * Called at the END of each frame after all draw calls
	 */
	void flip();

	void shutdown();

	/**
	 * @brief Read back the previous frame's framebuffer to CPU memory
	 *
	 * Copies the previously presented swap chain image to a vm_malloc'd RGBA
	 * pixel buffer. Handles the BGRA→RGBA swizzle since the swap chain uses
	 * B8G8R8A8 format. Caller must vm_free the returned buffer.
	 *
	 * @param[out] outPixels Receives the vm_malloc'd RGBA pixel buffer
	 * @param[out] outWidth  Receives the image width
	 * @param[out] outHeight Receives the image height
	 * @return true on success, false on failure
	 */
	bool readbackFramebuffer(ubyte** outPixels, uint32_t* outWidth, uint32_t* outHeight);

	/**
	 * @brief Read back the currently-bound off-screen render target's color image.
	 *
	 * Unlike readbackFramebuffer (which reads the previous frame's on-screen image),
	 * this captures the render target that is active right now via bm_set_render_target
	 * — the path gr.screenToBlob uses to grab SCPUI-generated icons. Because the target's
	 * draws live in the not-yet-submitted frame command buffer, this flushes that buffer
	 * mid-frame (submit + host wait), copies the target's color image, then resumes
	 * rendering into it with a loadOp=eLoad pass so already-drawn content survives.
	 *
	 * The target is R8G8B8A8_UNORM, so pixels come back as tightly-packed RGBA8 with
	 * alpha preserved (no swizzle, no forced opacity). Caller must vm_free the buffer.
	 *
	 * @param ts         The active render target slot (must be non-cubemap, have a framebuffer)
	 * @param[out] outPixels Receives the vm_malloc'd RGBA8 pixel buffer
	 * @param[out] outWidth  Receives the target width
	 * @param[out] outHeight Receives the target height
	 * @return true on success, false on failure
	 */
	bool readbackRenderTarget(tcache_slot_vulkan* ts, ubyte** outPixels, uint32_t* outWidth, uint32_t* outHeight);

	/**
	 * @brief Get the minimum uniform buffer offset alignment requirement
	 * @return The alignment in bytes (typically 64 or 256)
	 */
	uint32_t getMinUniformBufferOffsetAlignment() const;

	/**
	 * @brief Close out a frame's worth of work that completed without going through flip().
	 *
	 * Advances the monotonic frame counter that sync objects are stamped with, and rewinds the
	 * frame-scoped bump allocator. Deliberately does NOT touch m_currentFrame: that index selects
	 * the swap-chain sync objects, and the image for this index has already been acquired for the
	 * flip that will eventually present.
	 *
	 * Without this, an off-screen renderer leaves m_frameNumber frozen, and waitForSyncPoint()
	 * reports every fence taken since as "still recording" -- which is what makes
	 * UniformBufferManager's segment rotation give up and Error() out.
	 *
	 * Only valid once the work in question has actually retired; see gr_end_offscreen_frame().
	 */
	void endOffscreenFrame();

	/**
	 * @brief Stamp the frame currently being recorded, so it can be waited on later
	 *
	 * Backs gr_sync_fence(). Records *which* fence, not just when: the frame number alone cannot
	 * find it back, because the fences are per-target while the frame number is global, and because
	 * endOffscreenFrame() advances the frame number without moving the frame-in-flight cursor.
	 */
	FrameSyncPoint captureSyncPoint() const;

	/**
	 * @brief Wait for the GPU work recorded during the frame @p point was taken in
	 *
	 * Waits on that frame's own fence rather than stalling the entire device.
	 *
	 * @param timeoutNs Maximum wait in nanoseconds (0 = poll)
	 * @return true if the work is known complete. false if the timeout expired, or if the sync
	 *         point was taken during the frame still being recorded (submission happens on this
	 *         thread, so waiting here would deadlock -- it reports "not complete" instead).
	 */
	bool waitForSyncPoint(const FrameSyncPoint& point, uint64_t timeoutNs = UINT64_MAX);

	/**
	 * @brief Wait for all GPU work to complete
	 */
	void waitIdle();

	/**
	 * @brief Get the current command buffer as a raw Vulkan handle (for ImGui)
	 */
	VkCommandBuffer getVkCurrentCommandBuffer() const;

	/**
	 * @brief Check if VK_EXT_debug_utils is enabled
	 */
	bool isDebugUtilsEnabled() const { return m_debugUtilsEnabled; }

	/**
	 * @brief Get the maximum uniform buffer range
	 */
	uint32_t getMaxUniformBufferSize() const;

	/**
	 * @brief Get the maximum sampler anisotropy
	 */
	float getMaxAnisotropy() const;

	/**
	 * @brief Check if BC texture compression is supported
	 */
	bool isTextureCompressionBCSupported() const;

	/**
	 * @brief Check if depth clamping is supported (used by the shadow pass)
	 */
	bool isDepthClampSupported() const;

	/**
	 * @brief Check if vertex shader layer output is supported (for shadow cascades)
	 */
	bool supportsShaderViewportLayerOutput() const { return m_supportsShaderViewportLayerOutput; }

	/**
	 * @brief Check if raytraced shadows are supported (VK_KHR_ray_query +
	 * VK_KHR_acceleration_structure + VK_KHR_deferred_host_operations, with
	 * the corresponding feature bits enabled on the logical device)
	 */
	bool supportsRaytracedShadows() const { return m_supportsRaytracedShadows; }

	/**
	 * @brief Switch from swap chain pass to HDR scene pass
	 *
	 * Called by vulkan_scene_texture_begin(). Ends the current swap chain
	 * render pass and begins the HDR scene render pass.
	 */
	void beginSceneRendering();

	void resumeSceneRendering();

	/**
	 * @brief Switch from HDR scene pass back to swap chain
	 *
	 * Called by vulkan_scene_texture_end(). Ends the HDR scene render pass,
	 * runs post-processing, and begins the resumed swap chain render pass.
	 */
	void endSceneRendering();

	/**
	 * @brief Copy scene color to effect texture mid-scene
	 *
	 * Called by vulkan_copy_effect_texture(). Ends the current scene render
	 * pass, copies scene color → effect texture, then resumes the scene
	 * render pass with loadOp=eLoad to preserve existing content.
	 */
	void copyEffectTexture();

	/**
	 * @brief Copy scene depth mid-scene for soft particle sampling
	 *
	 * Called lazily from the first particle draw per frame. Ends the current
	 * scene render pass, copies depth → samplable copy, then resumes the
	 * scene render pass with loadOp=eLoad. No-op if already copied this frame.
	 */
	void copySceneDepthForParticles();

	/**
	 * @brief Park the scene depth so the cockpit can render on a cleared depth buffer
	 *
	 * Called by vulkan_post_process_save_zbuffer(). Ends the current scene render
	 * pass, copies scene depth → backup image, then resumes the pass with
	 * loadOp=eLoad. The caller clears the depth buffer afterwards. No-op when a
	 * save is already outstanding, or outside scene rendering.
	 */
	void saveSceneDepth();

	/**
	 * @brief Put the parked scene depth back, discarding what the cockpit wrote
	 *
	 * Called by vulkan_post_process_restore_zbuffer(). No-op unless saveSceneDepth()
	 * parked something.
	 */
	void restoreSceneDepth();

	/**
	 * @brief Check if scene depth copy is available for sampling this frame
	 */
	bool isSceneDepthCopied() const { return m_sceneDepthCopiedThisFrame; }

	/**
	 * @brief Check if we're currently rendering to the HDR scene target
	 */
	bool isSceneRendering() const { return m_sceneRendering; }

	/**
	 * @brief Begin rendering to an off-screen render target
	 *
	 * Ends the current render pass (swap chain or previous RT face) and begins
	 * a new render pass targeting the given texture's framebuffer.
	 */
	void beginRenderTarget(tcache_slot_vulkan* ts, int face);

	/**
	 * @brief End render target and resume the swap chain pass
	 *
	 * Ends the current RT render pass and resumes the swap chain render pass
	 * with loadOp=eLoad to preserve existing content.
	 *
	 * @param ts The render target just finished, or nullptr. When it has more
	 * than one mip level (e.g. the env/irradiance cubemaps), the mip chain is
	 * regenerated here via blits since nothing else populates it -- render
	 * passes only ever write to mip 0.
	 */
	void endRenderTarget(tcache_slot_vulkan* ts = nullptr);

	/**
	 * @brief Resume the swap chain render pass with loadOp=eLoad
	 *
	 * Begins a new render pass targeting the current swap chain image,
	 * preserving existing content. Used after off-screen rendering
	 * (render targets, irradiance map generation) to return to swap chain.
	 */
	void resumeSwapChainPass();

	/**
	 * @brief Resume rendering into a render target with loadOp=eLoad (preserving content)
	 *
	 * Used after a mid-frame render-target readback (readbackRenderTarget) to continue
	 * drawing into the same target without clearing what was already rendered.
	 */
	void resumeRenderTargetPass(tcache_slot_vulkan* ts);

	/**
	 * @brief Check if we're currently rendering to an off-screen render target
	 */
	bool isRenderTargetActive() const { return m_renderTargetActive; }

	/**
	 * @brief Set whether the G-buffer render pass is active
	 *
	 * Called by deferred_lighting_finish() to switch from G-buffer to
	 * scene render pass mid-frame for forward transparent rendering.
	 */
	void setUseGbufRenderPass(bool use) { m_useGbufRenderPass = use; }
	bool isUsingGbufRenderPass() const { return m_useGbufRenderPass; }

	/**
	 * @brief Get the validated MSAA sample count for deferred lighting
	 */
	vk::SampleCountFlagBits getMsaaSampleCount() const { return m_msaaSampleCount; }

	/**
	 * @brief Get the depth/stencil format chosen at device init
	 *
	 * Used by render targets created with BMP_FLAG_RENDER_TARGET_DEPTH_ATTACHMENT so
	 * their depth buffer matches the swap-chain depth format. Returns eUndefined if
	 * depth resources have not been created yet.
	 */
	vk::Format getDepthFormat() const { return m_depthFormat; }

  private:
	/**
	 * @brief Begin a render pass on the frame command buffer with full state-tracker sync
	 *
	 * Records vkCmdBeginRenderPass and updates the state tracker (render pass,
	 * color attachment count, sample count, render area, viewport) in one place.
	 */
	void beginTrackedRenderPass(const PassBeginDesc& desc);

	/**
	 * @brief Resume the scene (or G-buffer) render pass with loadOp=eLoad
	 * after a mid-scene copy (copyEffectTexture / copySceneDepthForParticles)
	 */
	void resumeScenePassAfterCopy();

	/**
	 * @brief Restore the color attachment layouts a depth-only copy left behind,
	 * then resume the scene (or G-buffer) render pass
	 */
	void resumeScenePassAfterDepthCopy();

	bool initDisplayDevice() const;

	bool initializeInstance();

	/**
	 * @brief Ask the windowing system for @p target's viewport surface and take ownership of it
	 */
	bool createTargetSurface(VulkanPresentTarget& target);

	bool pickPhysicalDevice(PhysicalDeviceValues& deviceValues);

	bool createLogicalDevice(const PhysicalDeviceValues& deviceValues);

	// Everything a target owns is built by one of these. They take the target explicitly rather than
	// working on m_current: the setup path builds targets that are not current yet (and, when
	// creation fails part-way, never become current), so "the current target" is the wrong answer
	// there -- and an implicit one is impossible to check at the call site.
	bool createSwapChain(VulkanPresentTarget& target,
		const PhysicalDeviceValues& deviceValues,
		vk::SwapchainKHR oldSwapchain = nullptr);

	void createRenderPass();

	void createFrameBuffers(VulkanPresentTarget& target);

	// HDR composition + output-encode resources
	void createCompositionResources(VulkanPresentTarget& target);

	/**
	 * @brief Build the shared output-encode render pass for @p swapChainFormat
	 *
	 * Takes the format rather than a target because, unlike its neighbours here, it does not build
	 * into one: m_encodeRenderPass is shared by every target. That is also the constraint
	 * createTargetResources() has to check -- a target whose surface negotiates a different format
	 * cannot use this pass.
	 */
	void createEncodeRenderPass(vk::Format swapChainFormat);
	void encodeToSwapChain();

	void createDepthResources(VulkanPresentTarget& target);
	void destroyDepthResources(VulkanPresentTarget& target);

	/**
	 * @brief Give back everything in a target that the memory manager owns
	 *
	 * The depth and composition images are the only parts of a target backed by VulkanMemoryManager
	 * allocations, so they have to be released before it shuts down; everything else is vk::Unique*
	 * and can wait for the target's own destructor.
	 */
	void releaseTargetMemory(VulkanPresentTarget& target);

	/**
	 * @brief Tear down everything a target derived from its surface, in the order Vulkan requires
	 *
	 * A VkSurfaceKHR must outlive every swap chain made from it. Relying on member-declaration order
	 * to get that right is too subtle to be safe here, because the surface is not destroyed by us at
	 * all: it belongs to the windowing system, and under Qt it goes when the window does. So the
	 * swap chain and everything holding its images are released explicitly first.
	 */
	static void destroyTargetSwapChain(VulkanPresentTarget& target);

	vk::Format findDepthFormat();

	void createCommandPool(const PhysicalDeviceValues& values);

	void createPresentSyncObjects(VulkanPresentTarget& target);

	/**
	 * @brief Build a target's per-image render-finished semaphores
	 *
	 * Sized to the swap chain's image count and indexed by image index, so it has to be rebuilt
	 * whenever the swap chain is, alongside the images themselves.
	 */
	void createRenderFinishedSemaphores(VulkanPresentTarget& target);

	/**
	 * @brief Build a target's surface, swap chain and everything sized to it
	 *
	 * Leaves @p target untouched by the renderer's notion of "current": it is only safe to present
	 * to once this has returned true, and useViewport() switches to it then.
	 *
	 * @return false if the surface could not be created, or if it negotiated a format the shared
	 * encode render pass was not built for
	 */
	bool createTargetResources(VulkanPresentTarget& target);

	void acquireNextSwapChainImage();

	/**
	 * @brief Wait until every target has finished the work it put in a frame-in-flight slot
	 *
	 * The slot indexes per-target sync objects but also the shared command pool and descriptor
	 * pools, so it cannot be recycled until all targets are done with it.
	 */
	void waitForFrameSlot(uint32_t slot);

	/**
	 * @brief Wait for a frame, naming it in the log if the wait is not a normal one
	 *
	 * Every wait on a fence in the present path goes through here. A legitimate wait is one frame
	 * time; anything beyond a fraction of a second means the queue is wedged, and blocking forever
	 * on it just produces an editor that stops responding and gets killed before any long timeout
	 * could say which wait it was.
	 *
	 * @param what description of the wait, for the log -- caller-built so the message names the
	 *             target and slot involved
	 */
	static void waitOrReport(VulkanRenderFrame& frame, const char* what);

	/**
	 * @brief Throw away the in-progress frame without submitting or presenting it
	 *
	 * Ends the open render pass and command buffer and frees it -- safe to free immediately, since
	 * nothing was ever submitted and so no GPU work can reference it. The swap chain image this
	 * frame acquired is left unconsumed, which leaves its image-available semaphore signaled; the
	 * caller must therefore recreate the swap chain (which recreates every frame's sync objects)
	 * before the next acquire.
	 */
	void discardFrame();

	bool recreateSwapChain(VulkanPresentTarget& target);

	void createImGuiDescriptorPool();
	void initImGui();
	void shutdownImGui();

	std::unique_ptr<os::GraphicsOperations> m_graphicsOps;

	vk::UniqueInstance m_vkInstance;
	vk::UniqueDebugReportCallbackEXT m_debugReport;            // legacy fallback (no VK_EXT_debug_utils)
	vk::UniqueDebugUtilsMessengerEXT m_debugMessenger;         // preferred debug callback

	vk::UniqueDevice m_device;

	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;

	// Everything downstream of a surface lives in the target it belongs to. There is exactly one in
	// the game; qtFRED presents through two (its main viewport and the briefing map).
	//
	// m_current means only "where drawing goes right now" -- the frame loop reads it, the setup path
	// does not. Everything that builds or tears down a target takes it as a parameter, so a target
	// can be built before it is ever current and abandoned if that fails.
	//
	// Keyed by os::Viewport* rather than by index into os::viewports, deliberately: qtFRED's
	// briefing map hands its viewport straight to gr_use_viewport() and never registers it with
	// os::addViewport(), so that list does not contain every viewport we present to.
	SCP_unordered_map<os::Viewport*, std::unique_ptr<VulkanPresentTarget>> m_targets;
	VulkanPresentTarget* m_mainTarget = nullptr;
	VulkanPresentTarget* m_current = nullptr;

	bool m_hdrMetadataSupported = false; // VK_EXT_hdr_metadata device extension enabled

	// Shared by every target, and deliberately so: the render passes bake in the composition (fp16)
	// and depth formats, which are the same everywhere, so keeping one set keeps every cached
	// pipeline valid across a target switch. m_encodeRenderPass is the exception -- it bakes in the
	// *swap chain* format, so a target whose surface negotiates a different one cannot use it; see
	// createTargetResources().
	vk::UniqueSampler m_compositionSampler;
	vk::UniqueRenderPass m_encodeRenderPass;

	vk::Format m_depthFormat = vk::Format::eUndefined;

	vk::UniqueRenderPass m_renderPass;        // Swap chain pass with loadOp=eClear
	vk::UniqueRenderPass m_renderPassLoad;    // Swap chain pass with loadOp=eLoad (resumed after post-processing)
	vk::UniqueDescriptorPool m_imguiDescriptorPool;
	bool m_imguiInitialized = false; // false in the editors, which have no ImGui context at all

	// The frame-in-flight cursor stays global rather than moving into the target: the buffer and
	// descriptor managers keep one ring keyed off it (setCurrentFrame() below), so a per-target
	// cursor would hand them conflicting indices and corrupt descriptors a few frames later. Each
	// target instead keeps its own sync objects and indexes them with this shared cursor.
	uint32_t m_currentFrame = 0;
	uint64_t m_frameNumber = 0;  // Total frames rendered (for sync tracking)

	vk::UniqueCommandPool m_graphicsCommandPool;

	// Current frame command buffer (valid between setupFrame and flip)
	vk::CommandBuffer m_currentCommandBuffer;
	SCP_vector<vk::CommandBuffer> m_currentCommandBuffers;  // For cleanup
	bool m_frameInProgress = false;

	// Physical device info (needed for memory manager)
	vk::PhysicalDevice m_physicalDevice;
	// Cached once at device selection: the limit/feature getters below are
	// hot-ish and querying the driver each call is wasteful. Populated alongside
	// m_physicalDevice in createLogicalDevice().
	vk::PhysicalDeviceProperties m_deviceProperties{};
	vk::PhysicalDeviceFeatures m_deviceFeatures{};
	uint32_t m_graphicsQueueFamilyIndex = 0;
	uint32_t m_presentQueueFamilyIndex = 0;

	// Memory, buffer, and texture management
	std::unique_ptr<VulkanMemoryManager> m_memoryManager;
	std::unique_ptr<VulkanBufferManager> m_bufferManager;
	std::unique_ptr<VulkanTextureManager> m_textureManager;
	std::unique_ptr<VulkanDeletionQueue> m_deletionQueue;

	// Shader, descriptor, and pipeline management
	std::unique_ptr<VulkanShaderManager> m_shaderManager;
	std::unique_ptr<VulkanDescriptorManager> m_descriptorManager;
	std::unique_ptr<VulkanPipelineManager> m_pipelineManager;

	// State tracking and draw management
	std::unique_ptr<VulkanStateTracker> m_stateTracker;
	std::unique_ptr<VulkanDrawManager> m_drawManager;

	// Query management (GPU timestamp profiling)
	std::unique_ptr<VulkanQueryManager> m_queryManager;

	// Raytraced shadow BLAS cache
	std::unique_ptr<VulkanRaytracingManager> m_raytracingManager;

	// Post-processing
	std::unique_ptr<VulkanPostProcessor> m_postProcessor;
	bool m_sceneRendering = false;
	bool m_sceneDepthCopiedThisFrame = false;
	bool m_sceneDepthSaved = false;    // True between saveSceneDepth() and restoreSceneDepth()
	bool m_useGbufRenderPass = false;  // True when scene uses G-buffer (deferred lighting)

	bool m_supportsShaderViewportLayerOutput = false;  // VK_EXT_shader_viewport_index_layer
	bool m_supportsRaytracedShadows = false;  // VK_KHR_ray_query + VK_KHR_acceleration_structure + VK_KHR_deferred_host_operations
	vk::SampleCountFlagBits m_msaaSampleCount = vk::SampleCountFlagBits::e1;  // Validated MSAA sample count
	bool m_renderTargetActive = false;  // True when rendering to off-screen RT (bm_set_render_target)

	bool m_debugReportEnabled = false;
	bool m_debugUtilsEnabled = false;

};

} // namespace graphics::vulkan
