#pragma once

#include "osapi/osapi.h"
#include "osapi/vulkan_surface.h"

#include "VulkanConstants.h"
#include "VulkanMemory.h"
#include "VulkanRenderFrame.h"

#include <vulkan/vulkan.hpp>

#include <array>
#include <memory>

namespace graphics::vulkan {

// Filled in by checkSwapChainSupport() below; defined in VulkanRenderer.h, which owns
// device selection. Only referenced here, so a declaration is enough.
struct PhysicalDeviceValues;

/**
 * @brief Owns the VkSurfaceKHR the windowing system handed us
 *
 * Not a vk::UniqueSurfaceKHR, because the surface is not ours to destroy with vkDestroySurfaceKHR:
 * os::VulkanSurfaceProvider created it and only it knows how to get rid of it (a Qt-backed
 * implementation, for instance, hands out a surface owned by QVulkanInstance). Destruction order is
 * still the usual one -- declare this after the instance and before the swap chain, so the swap
 * chain goes first, then the surface, then the instance.
 */
class VulkanSurfaceHandle {
  public:
	VulkanSurfaceHandle() = default;
	VulkanSurfaceHandle(os::VulkanSurfaceProvider* provider, vk::Instance instance, vk::SurfaceKHR surface);
	~VulkanSurfaceHandle();

	VulkanSurfaceHandle(const VulkanSurfaceHandle&) = delete;
	VulkanSurfaceHandle& operator=(const VulkanSurfaceHandle&) = delete;
	VulkanSurfaceHandle(VulkanSurfaceHandle&& other) noexcept;
	VulkanSurfaceHandle& operator=(VulkanSurfaceHandle&& other) noexcept;

	vk::SurfaceKHR get() const { return m_surface; }
	explicit operator bool() const { return static_cast<bool>(m_surface); }

	void reset();

  private:
	os::VulkanSurfaceProvider* m_provider = nullptr;
	vk::Instance m_instance;
	vk::SurfaceKHR m_surface;
};

/**
 * @brief One presentable surface and everything sized to it
 *
 * The renderer used to hold exactly one of each of these, which is all the game ever needs. qtFRED
 * presents through two independent windows -- its main viewport and the briefing map, which renders
 * on its own timer -- and switches between them with gr_use_viewport(), so each needs its own
 * surface, swap chain and extent-sized resources.
 *
 * What is *not* here is as deliberate as what is: the render passes, the post-processor and the
 * frame-in-flight cursor stay on the renderer. See the comments on those members.
 *
 * The sync objects are per-target but indexed by the renderer's shared m_currentFrame, so a target
 * that has not been drawn to for a while still has its slot waited on before reuse.
 */
struct VulkanPresentTarget {
	os::Viewport* viewport = nullptr;

	VulkanSurfaceHandle surface;

	vk::UniqueSwapchainKHR swapChain;
	vk::Format imageFormat = vk::Format::eUndefined;
	vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	bool hdrActive = false; // True when an HDR10 (PQ/BT.2020) swap chain was negotiated
	vk::Extent2D extent;

	SCP_vector<vk::Image> images;
	SCP_vector<vk::UniqueImageView> imageViews;
	SCP_vector<vk::UniqueFramebuffer> framebuffers;
	SCP_vector<VulkanRenderFrame*> imageRenderFrame;

	// HDR composition pipeline: the whole frame is rendered into these fp16 images (via
	// m_renderPass / framebuffers) instead of directly into the swap chain image.
	// encodeToSwapChain() converts composition -> swap chain: a direct blit (or
	// encodeOutputPassthrough() as a fallback) for SDR, or encodeOutput() (m_encodeRenderPass +
	// encodeFramebuffers) for the HDR10 PQ/BT.2020 transfer.
	SCP_vector<vk::UniqueImage> compositionImages;
	SCP_vector<vk::UniqueImageView> compositionImageViews;
	SCP_vector<VulkanAllocation> compositionAllocations;
	SCP_vector<vk::UniqueFramebuffer> encodeFramebuffers;

	// Depth buffer
	vk::UniqueImage depthImage;
	vk::UniqueImageView depthImageView;
	VulkanAllocation depthImageMemory;

	std::array<std::unique_ptr<VulkanRenderFrame>, MAX_FRAMES_IN_FLIGHT> frames;

	// Acquire semaphores live here rather than in the frames because an acquire can outlive the
	// frame slot that made it -- see the retained acquire below. Each remembers the frame whose
	// submit waits on it, so it is not handed out again until that frame has completed.
	struct AcquireSemaphore {
		vk::UniqueSemaphore semaphore;
		VulkanRenderFrame* consumer = nullptr;
	};
	SCP_vector<AcquireSemaphore> acquireSemaphores;
	uint32_t nextAcquire = 0;    // round-robin cursor into acquireSemaphores
	uint32_t currentAcquire = 0; // the one the in-progress frame will present with

	// One per swap chain image, indexed by image index -- not per frame-in-flight. A binary
	// semaphore handed to vkQueuePresentKHR stays in use by the presentation engine until that
	// image is acquired again, so the only safe moment to signal it once more is after an acquire
	// has returned that same image. Keying it on the image is what makes that automatic; keying it
	// on the frame slot (2 of them against 4 images) meant a submit could re-signal a semaphore the
	// presentation engine still held (VUID-vkQueueSubmit-pSignalSemaphores-00067). That normally
	// resolves itself once the image comes round again -- but a target that stops presenting (the
	// main viewport, once qtFRED's briefing map is driving the render loop) never re-acquires it,
	// and the pending question hangs the validation layer's state tracking for good.
	SCP_vector<vk::UniqueSemaphore> renderFinishedSemaphores;

	// vkAcquireNextImageKHR hands out an image that only a present gives back, so a viewport switch
	// cannot simply walk away from one: doing that leaks an image per switch and wedges the swap
	// chain within a few frames. The acquire is kept here instead and reused when this target
	// becomes current again.
	bool hasRetainedAcquire = false;
	uint32_t retainedAcquire = 0;
	uint32_t retainedImage = 0;

	uint32_t currentImage = 0;
	uint32_t previousImage = UINT32_MAX; // For saveScreen() readback of previous frame

	bool needsRecreation = false;
};

/**
 * @brief Identifies the exact fence a gr_sync_fence() was taken against
 *
 * All three fields are needed to find it back. The frame number says which frame's work is meant,
 * but it does not locate the fence: those live on the frame-in-flight slots of a *target*, and both
 * of the other two can have moved on by the time the wait happens -- a viewport switch changes the
 * target, and the slot cycles every MAX_FRAMES_IN_FLIGHT frames.
 */
struct FrameSyncPoint {
	// The target's viewport rather than the target itself, deliberately: a target is destroyed when
	// its viewport closes, and a sync point can outlive it. Looking the viewport up in m_targets
	// answers "is that target still around?" instead of dereferencing a dangling pointer.
	os::Viewport* viewport = nullptr;

	uint32_t slot = 0;         // index into VulkanPresentTarget::frames
	uint64_t frameNumber = 0;  // VulkanRenderer::m_frameNumber at the time
};

/**
 * @brief Fill in the parts of @p values that depend on a particular surface
 *
 * Every one of these can differ per surface, so this has to be re-run for each one rather than
 * carried over from the surface the device was picked against -- see createTargetResources().
 *
 * @return false if the surface reports no usable formats or present modes, i.e. cannot be presented
 *         to at all
 */
bool checkSwapChainSupport(PhysicalDeviceValues& values, vk::SurfaceKHR surface);

} // namespace graphics::vulkan
