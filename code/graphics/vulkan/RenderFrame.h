#pragma once

#include "globalincs/pstypes.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

class RenderFrame {
  public:
	RenderFrame(vk::Device device, vk::SwapchainKHR swapChain, vk::Queue graphicsQueue, vk::Queue presentQueue);

	void waitForFinish();

	uint32_t acquireSwapchainImage();

	void onFrameFinished(std::function<void()> finishFunc);

	void submitAndPresent(const std::vector<vk::CommandBuffer>& cmdBuffers);

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
