#pragma once

#include "globalincs/pstypes.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

/**
 * @brief Result of swapchain image acquisition
 */
enum class AcquireResult {
	Success,     ///< Image acquired successfully
	Suboptimal,  ///< Image acquired but swapchain is suboptimal and should be recreated
	OutOfDate,   ///< Swapchain is out of date and must be recreated before next acquire
	Error        ///< Fatal error during acquisition
};

/**
 * @brief Result of frame presentation
 */
enum class PresentResult {
	Success,     ///< Frame presented successfully
	Suboptimal,  ///< Frame presented but swapchain is suboptimal and should be recreated
	OutOfDate,   ///< Swapchain became out of date during presentation
	Error        ///< Fatal error during presentation
};

class RenderFrame {
  public:
	RenderFrame(vk::Device device, vk::SwapchainKHR swapChain, vk::Queue graphicsQueue, vk::Queue presentQueue);

	void waitForFinish();

	/**
	 * @brief Acquire the next swapchain image for rendering
	 * @param outImageIndex Receives the acquired image index on success
	 * @param imageAvailableSemaphore Semaphore to signal when image is available (per-image semaphore)
	 * @return Result indicating success or need for swapchain recreation
	 */
	AcquireResult acquireSwapchainImage(uint32_t& outImageIndex, vk::Semaphore imageAvailableSemaphore);

	void onFrameFinished(std::function<void()> finishFunc);

	/**
	 * @brief Submit command buffers and present the frame
	 * @param cmdBuffers Command buffers to submit
	 * @param imageAvailableSemaphore Semaphore to wait on (acquired image's semaphore)
	 * @param renderingFinishedSemaphore Semaphore to signal when rendering is done (per-image)
	 * @return Result indicating success or need for swapchain recreation
	 */
	PresentResult submitAndPresent(const std::vector<vk::CommandBuffer>& cmdBuffers,
	                               vk::Semaphore imageAvailableSemaphore,
	                               vk::Semaphore renderingFinishedSemaphore);

	/**
	 * @brief Update the swapchain handle after recreation
	 * @param newSwapchain The newly created swapchain
	 */
	void updateSwapchain(vk::SwapchainKHR newSwapchain);

	// Debug/inspection helpers
	vk::Fence getFence() const { return m_frameInFlightFence.get(); }
	bool isInFlight() const { return m_inFlight; }

  private:
	vk::Device m_device;
	vk::SwapchainKHR m_swapChain;
	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;

	vk::UniqueFence m_frameInFlightFence;
	SCP_vector<std::function<void()>> m_frameFinishedCallbacks;

	bool m_inFlight = false;

	uint32_t m_swapChainIdx = 0;
};

} // namespace vulkan
} // namespace graphics
