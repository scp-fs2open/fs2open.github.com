
#include "VulkanRenderFrame.h"

namespace graphics {
namespace vulkan {

VulkanRenderFrame::VulkanRenderFrame(vk::Device device, vk::SwapchainKHR swapChain, vk::Queue graphicsQueue, vk::Queue presentQueue)
	: m_device(device), m_swapChain(swapChain), m_graphicsQueue(graphicsQueue), m_presentQueue(presentQueue)
{
	constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo;
	constexpr vk::FenceCreateInfo fenceCreateInfo;

	m_imageAvailableSemaphore = device.createSemaphoreUnique(semaphoreCreateInfo);
	m_renderingFinishedSemaphore = device.createSemaphoreUnique(semaphoreCreateInfo);
	m_frameInFlightFence = device.createFenceUnique(fenceCreateInfo);
}
void VulkanRenderFrame::waitForFinish()
{
	if (!m_inFlight) {
		return;
	}

	// waitForFences can theoretically return a timeout, but as this passes the maximum uint64_t value in microseconds,
	// this won't happen in practice, and the result can be ignored.
	(void)m_device.waitForFences(m_frameInFlightFence.get(), true, std::numeric_limits<uint64_t>::max());
	m_device.resetFences(m_frameInFlightFence.get());

	// That frame is now definitely not in flight anymore so we can call the functions that depend on that
	for (const auto& finishFunc : m_frameFinishedCallbacks) {
		finishFunc();
	}
	m_frameFinishedCallbacks.clear();

	// Our fence has been signaled so we are no longer in flight and ready to be reused
	m_inFlight = false;
}
void VulkanRenderFrame::onFrameFinished(std::function<void()> finishFunc)
{
	m_frameFinishedCallbacks.push_back(std::move(finishFunc));
}
SwapChainStatus VulkanRenderFrame::acquireSwapchainImage(uint32_t& outImageIndex)
{
	Assertion(!m_inFlight, "Cannot acquire swapchain image when frame is still in flight.");

	uint32_t imageIndex;
	vk::Result res;
	try {
		res = m_device.acquireNextImageKHR(m_swapChain,
			std::numeric_limits<uint64_t>::max(),
			m_imageAvailableSemaphore.get(),
			nullptr,
			&imageIndex);
	} catch (vk::OutOfDateKHRError&) {
		return SwapChainStatus::eOutOfDate;
	}

	m_swapChainIdx = imageIndex;
	outImageIndex = imageIndex;

	if (res == vk::Result::eSuboptimalKHR) {
		return SwapChainStatus::eSuboptimal;
	}
	return SwapChainStatus::eSuccess;
}
SwapChainStatus VulkanRenderFrame::submitAndPresent(const SCP_vector<vk::CommandBuffer>& cmdBuffers)
{
	Assertion(!m_inFlight, "Cannot submit a frame for presentation when it is still in flight.");

	// Wait at color attachment output stage — the first use of the swap chain image
	// is loadOp=eClear at the start of the render pass, which is a color attachment write.
	const std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const std::array<vk::Semaphore, 1> waitSemaphores = {m_imageAvailableSemaphore.get()};

	vk::SubmitInfo submitInfo;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.pWaitSemaphores = waitSemaphores.data();

	submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
	submitInfo.pCommandBuffers = cmdBuffers.data();

	const std::array<vk::Semaphore, 1> signalSemaphores = {m_renderingFinishedSemaphore.get()};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	m_graphicsQueue.submit(submitInfo, m_frameInFlightFence.get());

	// This frame is now officially in flight (fence pending even if present fails)
	m_inFlight = true;

	vk::PresentInfoKHR presentInfo;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores.data();

	const std::array<vk::SwapchainKHR, 1> swapChains = {m_swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains.data();
	presentInfo.pImageIndices = &m_swapChainIdx;
	presentInfo.pResults = nullptr;

	vk::Result res;
	try {
		res = m_presentQueue.presentKHR(presentInfo);
	} catch (vk::OutOfDateKHRError&) {
		return SwapChainStatus::eOutOfDate;
	}

	if (res == vk::Result::eSuboptimalKHR) {
		return SwapChainStatus::eSuboptimal;
	}
	return SwapChainStatus::eSuccess;
}
void VulkanRenderFrame::updateSwapChain(vk::SwapchainKHR swapChain)
{
	m_swapChain = swapChain;
}

} // namespace vulkan
} // namespace graphics
