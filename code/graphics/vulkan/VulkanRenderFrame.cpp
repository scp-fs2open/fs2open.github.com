
#include "VulkanRenderFrame.h"

namespace graphics::vulkan {

VulkanRenderFrame::VulkanRenderFrame(vk::Device device, vk::SwapchainKHR swapChain, vk::Queue graphicsQueue, vk::Queue presentQueue)
	: m_device(device), m_swapChain(swapChain), m_graphicsQueue(graphicsQueue), m_presentQueue(presentQueue)
{
	constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo;
	constexpr vk::FenceCreateInfo fenceCreateInfo;

	m_imageAvailableSemaphore = device.createSemaphoreUnique(semaphoreCreateInfo);
	m_renderingFinishedSemaphore = device.createSemaphoreUnique(semaphoreCreateInfo);
	m_frameInFlightFence = device.createFenceUnique(fenceCreateInfo);
}
bool VulkanRenderFrame::waitForFinish(uint64_t timeoutNs)
{
	if (!m_inFlight) {
		return true;
	}

	auto result = m_device.waitForFences(m_frameInFlightFence.get(), true, timeoutNs);
	if (result == vk::Result::eTimeout) {
		// Leave everything pending so the wait can be retried
		return false;
	}
	m_device.resetFences(m_frameInFlightFence.get());

	// That frame is now definitely not in flight anymore so we can call the functions that depend on that
	for (const auto& finishFunc : m_frameFinishedCallbacks) {
		finishFunc();
	}
	m_frameFinishedCallbacks.clear();

	// Our fence has been signaled so we are no longer in flight and ready to be reused
	m_inFlight = false;
	return true;
}
void VulkanRenderFrame::onFrameFinished(std::function<void()> finishFunc)
{
	m_frameFinishedCallbacks.push_back(std::move(finishFunc));
}
SwapChainStatus VulkanRenderFrame::acquireSwapchainImage(uint32_t& outImageIndex)
{
	Assertion(!m_inFlight, "Cannot acquire swapchain image when frame is still in flight.");

	// Initialized to a safe value: the pointer overload below only writes this
	// on success, so it must never be left indeterminate if the acquire fails.
	uint32_t imageIndex = 0;
	vk::Result res = vk::Result::eErrorOutOfDateKHR;
	try {
		res = m_device.acquireNextImageKHR(m_swapChain,
			std::numeric_limits<uint64_t>::max(),
			m_imageAvailableSemaphore.get(),
			nullptr,
			&imageIndex);
	} catch (const vk::OutOfDateKHRError&) {
		return SwapChainStatus::eOutOfDate;
	}

	// IMPORTANT: this overload of acquireNextImageKHR takes a pImageIndex pointer
	// and returns the raw vk::Result *without throwing* on error codes. The
	// try/catch above therefore does NOT catch eErrorOutOfDateKHR (and other
	// errors); they arrive here as a Result. We must inspect it explicitly,
	// otherwise an error result would fall through as "success" while leaving
	// imageIndex unwritten, producing a garbage swap chain index.
	if (res == vk::Result::eErrorOutOfDateKHR) {
		return SwapChainStatus::eOutOfDate;
	}
	if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR) {
		// Surface lost, device lost, timeout, etc. No image was acquired, so the
		// index is invalid. Force a swap chain recreation rather than indexing
		// with a bogus value.
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
	} catch (const vk::OutOfDateKHRError&) {
		return SwapChainStatus::eOutOfDate;
	} catch (const vk::SystemError& e) {
		// eErrorSurfaceLostKHR, device loss, etc. Map everything to eOutOfDate so
		// the renderer recreates the swap chain (and fails visibly inside
		// recreateSwapChain if the surface is really gone) instead of crashing on
		// an uncaught exception here.
		mprintf(("Vulkan: presentKHR failed (%s), flagging swap chain recreation\n", e.what()));
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
void VulkanRenderFrame::recreateSyncObjects()
{
	Assertion(!m_inFlight, "Cannot recreate sync objects while the frame is in flight.");

	constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo;
	m_imageAvailableSemaphore = m_device.createSemaphoreUnique(semaphoreCreateInfo);
	m_renderingFinishedSemaphore = m_device.createSemaphoreUnique(semaphoreCreateInfo);
}

} // namespace graphics::vulkan
