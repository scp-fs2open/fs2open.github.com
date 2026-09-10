#pragma once

#include "globalincs/pstypes.h"

#include <limits>
#include <vulkan/vulkan.hpp>

namespace graphics::vulkan {

enum class SwapChainStatus {
	eSuccess,
	eSuboptimal,   // Swap chain works but should be recreated
	eOutOfDate,    // Must recreate before next use
};

class VulkanRenderFrame {
  public:
	VulkanRenderFrame(vk::Device device, vk::SwapchainKHR swapChain, vk::Queue graphicsQueue, vk::Queue presentQueue);

	/**
	 * @brief Wait for this frame's GPU work to complete
	 * @param timeoutNs Maximum wait in nanoseconds (default: unbounded)
	 * @return true if the frame is complete (or was never in flight); false if
	 *         the timeout expired first (fence, callbacks, and in-flight state
	 *         are left untouched so the wait can be retried)
	 */
	bool waitForFinish(uint64_t timeoutNs = std::numeric_limits<uint64_t>::max());

	/**
	 * @brief Acquire an image, signalling a semaphore the caller owns
	 *
	 * The semaphore belongs to the present target rather than to this frame: an acquire can outlive
	 * the frame slot that made it, because switching viewports mid-frame retains the acquired image
	 * and hands it back when that viewport becomes current again -- by which time the shared
	 * frame-in-flight cursor has moved on, and it cannot move back (the descriptor manager asserts
	 * that it only ever advances).
	 */
	SwapChainStatus acquireSwapchainImage(uint32_t& outImageIndex, vk::Semaphore imageAvailable);

	void onFrameFinished(std::function<void()> finishFunc);

	/**
	 * @brief Submit this frame's work and present the acquired image
	 *
	 * Neither semaphore belongs to this frame. Both are owned by the present target: the acquire
	 * because it can outlive the frame slot that made it (see acquireSwapchainImage()), and the
	 * render-finished one because it is keyed on the swap chain image rather than on the frame
	 * slot -- the presentation engine keeps hold of it until that image is acquired again.
	 *
	 * @param imageAvailable the semaphore the acquire signalled
	 * @param renderFinished the semaphore for @p imageIndex; signalled by the submit, waited on by
	 *                       the present
	 * @param imageIndex     the image that acquire returned
	 * @param frameNumber    the monotonic frame number this submission covers; remembered so a
	 *                       sync point taken during that frame can tell whether this fence is
	 *                       still the one guarding its work (see getSubmittedFrameNumber())
	 */
	SwapChainStatus submitAndPresent(const SCP_vector<vk::CommandBuffer>& cmdBuffers,
		vk::Semaphore imageAvailable, vk::Semaphore renderFinished, uint32_t imageIndex,
		uint64_t frameNumber);

	/**
	 * @brief The frame number of the submission this fence currently guards
	 *
	 * NEVER_SUBMITTED until the first submit. A frame slot is reused every MAX_FRAMES_IN_FLIGHT
	 * frames, so the fence alone does not say *which* frame's work it covers -- this does, which is
	 * what lets VulkanRenderer::waitForSyncPoint() tell "still guarding the work I care about" from
	 * "already recycled by a later frame".
	 */
	uint64_t getSubmittedFrameNumber() const { return m_submittedFrameNumber; }

	static constexpr uint64_t NEVER_SUBMITTED = std::numeric_limits<uint64_t>::max();

	void updateSwapChain(vk::SwapchainKHR swapChain);

  private:
	vk::Device m_device;
	vk::SwapchainKHR m_swapChain;
	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;

	vk::UniqueFence m_frameInFlightFence;
	SCP_vector<std::function<void()>> m_frameFinishedCallbacks;

	bool m_inFlight = false;

	uint64_t m_submittedFrameNumber = NEVER_SUBMITTED;
};

} // namespace graphics::vulkan
