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

	SwapChainStatus acquireSwapchainImage(uint32_t& outImageIndex);

	void onFrameFinished(std::function<void()> finishFunc);

	SwapChainStatus submitAndPresent(const SCP_vector<vk::CommandBuffer>& cmdBuffers);

	void updateSwapChain(vk::SwapchainKHR swapChain);

	/**
	 * @brief Recreate the per-frame semaphores (frame must not be in flight)
	 *
	 * Called during swap chain recreation: an acquire that succeeded against the
	 * old swap chain but was never consumed by a submit leaves the
	 * image-available semaphore signaled, which would corrupt the next acquire.
	 */
	void recreateSyncObjects();

  private:
	vk::Device m_device;
	vk::SwapchainKHR m_swapChain;
	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;

	vk::UniqueSemaphore m_imageAvailableSemaphore;
	vk::UniqueSemaphore m_renderingFinishedSemaphore;
	vk::UniqueFence m_frameInFlightFence;
	SCP_vector<std::function<void()>> m_frameFinishedCallbacks;

	bool m_inFlight = false;

	uint32_t m_swapChainIdx = 0;
};

} // namespace graphics::vulkan
