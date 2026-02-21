#pragma once

#include "globalincs/pstypes.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

enum class SwapChainStatus {
	eSuccess,
	eSuboptimal,   // Swap chain works but should be recreated
	eOutOfDate,    // Must recreate before next use
};

class VulkanRenderFrame {
  public:
	VulkanRenderFrame(vk::Device device, vk::SwapchainKHR swapChain, vk::Queue graphicsQueue, vk::Queue presentQueue);

	void waitForFinish();

	SwapChainStatus acquireSwapchainImage(uint32_t& outImageIndex);

	void onFrameFinished(std::function<void()> finishFunc);

	SwapChainStatus submitAndPresent(const SCP_vector<vk::CommandBuffer>& cmdBuffers);

	void updateSwapChain(vk::SwapchainKHR swapChain);

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

} // namespace vulkan
} // namespace graphics
