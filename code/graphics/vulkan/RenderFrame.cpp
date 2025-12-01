
#include "RenderFrame.h"
#include "VulkanDebug.h"

namespace graphics {
namespace vulkan {

RenderFrame::RenderFrame(vk::Device device, vk::SwapchainKHR swapChain, vk::Queue graphicsQueue, vk::Queue presentQueue)
	: m_device(device), m_swapChain(swapChain), m_graphicsQueue(graphicsQueue), m_presentQueue(presentQueue)
{
	constexpr vk::FenceCreateInfo fenceCreateInfo;
	m_frameInFlightFence = device.createFenceUnique(fenceCreateInfo);
}
void RenderFrame::waitForFinish()
{
	if (!m_inFlight) {
		return;
	}

	vk_logf("RenderFrame",
		"waitForFinish fence=%p (inFlight=1)",
		reinterpret_cast<void*>(static_cast<VkFence>(m_frameInFlightFence.get())));

	// waitForFences can theoretically return a timeout, but as this passes the maximum uint64_t value in microseconds,
	// this won't happen in practice, and the result can be ignored.
	(void)m_device.waitForFences(m_frameInFlightFence.get(), true, std::numeric_limits<uint64_t>::max());
	m_device.resetFences(m_frameInFlightFence.get());
	vk_logf("RenderFrame",
		"waitForFinish fence=%p reset complete",
		reinterpret_cast<void*>(static_cast<VkFence>(m_frameInFlightFence.get())));

	// That frame is now definitely not in flight anymore so we can call the functions that depend on that
	for (const auto& finishFunc : m_frameFinishedCallbacks) {
		finishFunc();
	}
	m_frameFinishedCallbacks.clear();

	// Our fence has been signaled so we are no longer in flight and ready to be reused
	m_inFlight = false;
}
void RenderFrame::onFrameFinished(std::function<void()> finishFunc)
{
	m_frameFinishedCallbacks.push_back(std::move(finishFunc));
}
AcquireResult RenderFrame::acquireSwapchainImage(uint32_t& outImageIndex, vk::Semaphore imageAvailableSemaphore)
{
	Assertion(!m_inFlight, "Cannot acquire swapchain image when frame is still in flight.");

	uint32_t imageIndex;
	vk::Result res;
	try {
		res = m_device.acquireNextImageKHR(m_swapChain,
			std::numeric_limits<uint64_t>::max(),
			imageAvailableSemaphore,
			nullptr,
			&imageIndex);
	} catch (const vk::OutOfDateKHRError&) {
		// Swapchain is out of date and must be recreated
		vk_logf("RenderFrame", "acquireSwapchainImage: swapchain out of date");
		return AcquireResult::OutOfDate;
	} catch (const vk::SystemError& e) {
		vk_logf("RenderFrame", "Vulkan: Failed to acquire swapchain image: %s", e.what());
		return AcquireResult::Error;
	}

	m_swapChainIdx = imageIndex;
	outImageIndex = imageIndex;

	vk_logf("RenderFrame",
		"acquireSwapchainImage: result=%s imageIndex=%u",
		vk::to_string(res).c_str(),
		imageIndex);

	if (res == vk::Result::eSuboptimalKHR) {
		// Image was acquired but swapchain should be recreated soon
		return AcquireResult::Suboptimal;
	}

	return AcquireResult::Success;
}
PresentResult RenderFrame::submitAndPresent(const std::vector<vk::CommandBuffer>& cmdBuffers,
                                           vk::Semaphore imageAvailableSemaphore,
                                           vk::Semaphore renderingFinishedSemaphore)
{
	Assertion(!m_inFlight, "Cannot submit a frame for presentation when it is still in flight.");

	vk_logf("RenderFrame",
		"submitAndPresent fence=%p submitCount=%zu",
		reinterpret_cast<void*>(static_cast<VkFence>(m_frameInFlightFence.get())),
		cmdBuffers.size());

	const std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const std::array<vk::Semaphore, 1> waitSemaphores = {imageAvailableSemaphore};

	vk::SubmitInfo submitInfo;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.pWaitSemaphores = waitSemaphores.data();

	submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
	submitInfo.pCommandBuffers = cmdBuffers.data();

	const std::array<vk::Semaphore, 1> signalSemaphores = {renderingFinishedSemaphore};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	try {
		m_graphicsQueue.submit(submitInfo, m_frameInFlightFence.get());
	} catch (const vk::SystemError& e) {
		vk_logf("RenderFrame", "Vulkan: Failed to submit command buffers: %s", e.what());
		return PresentResult::Error;
	}

	// This frame is now officially in flight
	m_inFlight = true;
	vk_logf("RenderFrame",
		"submitAndPresent: submitted fence=%p imageIndex=%u",
		reinterpret_cast<void*>(static_cast<VkFence>(m_frameInFlightFence.get())),
		m_swapChainIdx);

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
	} catch (const vk::OutOfDateKHRError&) {
		// Swapchain became out of date during presentation
		return PresentResult::OutOfDate;
	} catch (const vk::SystemError& e) {
		vk_logf("RenderFrame", "Vulkan: Failed to present frame: %s", e.what());
		return PresentResult::Error;
	}

	vk_logf("RenderFrame",
		"submitAndPresent: present result=%s imageIndex=%u",
		vk::to_string(res).c_str(),
		m_swapChainIdx);

	if (res == vk::Result::eSuboptimalKHR) {
		// Frame was presented but swapchain should be recreated soon
		return PresentResult::Suboptimal;
	}

	return PresentResult::Success;
}

void RenderFrame::updateSwapchain(vk::SwapchainKHR newSwapchain)
{
	m_swapChain = newSwapchain;
}

} // namespace vulkan
} // namespace graphics
