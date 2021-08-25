
#include "RenderFrame.h"

namespace graphics {
namespace vulkan {

RenderFrame::RenderFrame(vk::Device device, vk::SwapchainKHR swapChain, vk::Queue graphicsQueue, vk::Queue presentQueue)
	: m_device(device), m_swapChain(swapChain), m_graphicsQueue(graphicsQueue), m_presentQueue(presentQueue)
{
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	vk::FenceCreateInfo fenceCreateInfo;

	m_imageAvailableSemaphore = device.createSemaphoreUnique(semaphoreCreateInfo);
	m_renderingFinishedSemaphore = device.createSemaphoreUnique(semaphoreCreateInfo);
	m_frameInFlightFence = device.createFenceUnique(fenceCreateInfo);
}
void RenderFrame::waitForFinish()
{
	if (!m_inFlight) {
		return;
	}

	m_device.waitForFences(m_frameInFlightFence.get(), true, std::numeric_limits<uint64_t>::max());
	m_device.resetFences(m_frameInFlightFence.get());

	// That frame is now definitely not in flight anymore so we can call the functions that depend on that
	for (const auto& finishFunc : m_frameFinishedCallbacks) {
		finishFunc();
	}
	m_frameFinishedCallbacks.clear();

	// Our fence has been signalled so we are no longer in flight and ready to be reused
	m_inFlight = false;
}
void RenderFrame::onFrameFinished(std::function<void()> finishFunc)
{
	m_frameFinishedCallbacks.push_back(std::move(finishFunc));
}
uint32_t RenderFrame::acquireSwapchainImage()
{
	Assertion(!m_inFlight, "Cannot acquire swapchain image when frame is still in flight.");

	uint32_t imageIndex;
	m_device.acquireNextImageKHR(m_swapChain,
		std::numeric_limits<uint64_t>::max(),
		m_imageAvailableSemaphore.get(),
		nullptr,
		&imageIndex);

	m_swapChainIdx = imageIndex;

	return imageIndex;
}
void RenderFrame::submitAndPresent(const std::vector<vk::CommandBuffer>& cmdBuffers)
{
	Assertion(!m_inFlight, "Cannot submit a frame for presentation when it is still in flight.");

	std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	std::array<vk::Semaphore, 1> waitSemaphores = {m_imageAvailableSemaphore.get()};

	vk::SubmitInfo submitInfo;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.pWaitSemaphores = waitSemaphores.data();

	submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
	submitInfo.pCommandBuffers = cmdBuffers.data();

	std::array<vk::Semaphore, 1> signalSemaphores = {m_renderingFinishedSemaphore.get()};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	m_graphicsQueue.submit(submitInfo, m_frameInFlightFence.get());

	// This frame is now officially in flight
	m_inFlight = true;

	vk::PresentInfoKHR presentInfo;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores.data();

	std::array<vk::SwapchainKHR, 1> swapChains = {m_swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains.data();
	presentInfo.pImageIndices = &m_swapChainIdx;
	presentInfo.pResults = nullptr;

	m_presentQueue.presentKHR(presentInfo);
}

} // namespace vulkan
} // namespace graphics
