
#include "VulkanQuery.h"
#include "VulkanState.h"

namespace graphics {
namespace vulkan {

static VulkanQueryManager* g_queryManager = nullptr;

VulkanQueryManager* getQueryManager()
{
	return g_queryManager;
}

void setQueryManager(VulkanQueryManager* mgr)
{
	g_queryManager = mgr;
}

bool VulkanQueryManager::init(vk::Device device, float timestampPeriod,
                               vk::CommandPool commandPool, vk::Queue queue)
{
	m_device = device;
	m_timestampPeriod = timestampPeriod;

	vk::QueryPoolCreateInfo poolInfo;
	poolInfo.queryType = vk::QueryType::eTimestamp;
	poolInfo.queryCount = POOL_CAPACITY;

	m_queryPool = m_device.createQueryPool(poolInfo);
	if (!m_queryPool) {
		mprintf(("Vulkan: Failed to create timestamp query pool!\n"));
		return false;
	}

	// Reset the entire pool via a one-shot command buffer so all queries
	// start in the "unavailable" state required by the spec.
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.commandPool = commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = 1;

	auto cmdBuffers = m_device.allocateCommandBuffers(allocInfo);
	auto cmd = cmdBuffers.front();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	cmd.begin(beginInfo);
	cmd.resetQueryPool(m_queryPool, 0, POOL_CAPACITY);
	cmd.end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	queue.submit(submitInfo, nullptr);
	queue.waitIdle();

	m_device.freeCommandBuffers(commandPool, cmdBuffers);

	m_slots.clear();
	m_slots.resize(POOL_CAPACITY);
	for (uint32_t idx = 0; idx < POOL_CAPACITY; ++idx) {
		m_freeSlots.push(idx);
	}

	m_resetList.clear();
	m_inflightResets.clear();
	m_pendingWrites.clear();
	m_deferredFreeSlots.clear();
	m_lastFrameSubmitted = true;

	mprintf(("Vulkan: Created timestamp query pool (capacity %u, period %.1f ns/tick)\n",
		POOL_CAPACITY, m_timestampPeriod));

	return true;
}

void VulkanQueryManager::shutdown()
{
	if (m_device && m_queryPool) {
		m_device.destroyQueryPool(m_queryPool);
		m_queryPool = nullptr;
	}
	m_slots.clear();
	while (!m_freeSlots.empty()) {
		m_freeSlots.pop();
	}
	m_resetList.clear();
	m_inflightResets.clear();
	m_pendingWrites.clear();
	m_deferredFreeSlots.clear();
	m_device = nullptr;
}

void VulkanQueryManager::beginFrame(vk::CommandBuffer commandBuffer)
{
	// If the previous frame's command buffer was abandoned (no flip/submit),
	// the resets and writes we recorded never executed on the GPU.
	if (!m_lastFrameSubmitted) {
		// Orphaned writes: the vkCmdWriteTimestamp never executed, so the
		// slot is still in its pre-write state. Mark as orphaned so that
		// queryValueAvailable returns true and getQueryValue returns 0,
		// letting the tracing drain proceed to deleteQueryObject.
		for (auto idx : m_pendingWrites) {
			m_slots[idx].orphaned = true;
			m_slots[idx].wasReset = true;
		}
		m_pendingWrites.clear();
		// Orphaned resets: the vkCmdResetQueryPool never executed. Override
		// wasReset back to false for slots whose reset was ALSO on the
		// abandoned command buffer, and re-schedule the reset.
		for (auto idx : m_inflightResets) {
			m_slots[idx].wasReset = false;
			m_resetList.push_back(idx);
		}
	}
	m_inflightResets.clear();

	// Record resets for this frame. Only slots returned via deleteQueryObject
	// are in this list. Must happen outside render passes (vkCmdResetQueryPool).
	for (auto idx : m_resetList) {
		Assertion(!m_slots[idx].inUse,
			"Query slot %u in resetList but inUse=true!", idx);
		commandBuffer.resetQueryPool(m_queryPool, idx, 1);
		m_slots[idx].wasReset = true;
		m_inflightResets.push_back(idx);
	}
	m_resetList.clear();

	// Slots that were deleted while awaiting reset can now return to the free pool.
	// The vkCmdResetQueryPool recorded above makes them safe for new writes on
	// this same command buffer.
	if (!m_deferredFreeSlots.empty()) {
		for (auto idx : m_deferredFreeSlots) {
			m_freeSlots.push(idx);
		}
		m_deferredFreeSlots.clear();
	}

	// Report and reset exhaustion counter from previous frame
	if (m_exhaustionMessageCount > 0) {
		mprintf(("Vulkan: Query pool exhaustion — %u queries dropped last frame (free: %u)\n",
			m_exhaustionMessageCount, static_cast<uint32_t>(m_freeSlots.size())));
		m_exhaustionMessageCount = 0;
	}

	m_lastFrameSubmitted = false;
}

void VulkanQueryManager::notifySubmission()
{
	m_lastFrameSubmitted = true;
	m_inflightResets.clear();

	// Confirm all pending writes were submitted to the GPU.
	for (auto idx : m_pendingWrites) {
		m_slots[idx].submitted = true;
	}
	m_pendingWrites.clear();
}

int VulkanQueryManager::createQueryObject()
{
	if (!m_freeSlots.empty()) {
		auto idx = m_freeSlots.front();
		m_freeSlots.pop();
		m_slots[idx].inUse = true;
		return static_cast<int>(idx);
	} else {
		if (m_exhaustionMessageCount == 0) {
			uint32_t inUseCount = 0, pendingResetCount = 0;
			for (const auto& s : m_slots) {
				if (s.inUse) inUseCount++;
			}
			pendingResetCount = static_cast<uint32_t>(m_resetList.size() + m_inflightResets.size() + m_deferredFreeSlots.size());
			mprintf(("Vulkan: Query pool exhausted (%u slots: %u in-use, %u pending-reset, %u pending-write)\n",
				POOL_CAPACITY, inUseCount, pendingResetCount, static_cast<uint32_t>(m_pendingWrites.size())));
		}
		m_exhaustionMessageCount++;
		return -1;
	}
}

void VulkanQueryManager::queryValue(int obj, QueryType type)
{
	Assertion(obj >= 0 && obj < static_cast<int>(m_slots.size()),
		"Query object index %d is invalid!", obj);
	auto& slot = m_slots[obj];

	switch (type) {
	case QueryType::Timestamp: {
		// Slots must be reset by beginFrame before a new write.
		Assertion(slot.wasReset,
			"Query slot %d written before reset! wasReset=%d inUse=%d",
			obj, (int)slot.wasReset, (int)slot.inUse);

		getStateTracker()->getCommandBuffer().writeTimestamp(
			vk::PipelineStageFlagBits::eBottomOfPipe,
			m_queryPool, static_cast<uint32_t>(obj));

		slot.submitted = false;
		slot.wasReset = false;
		m_pendingWrites.push_back(static_cast<uint32_t>(obj));
		break;
	}
	default:
		UNREACHABLE("Unhandled QueryType value!");
		break;
	}
}

bool VulkanQueryManager::queryValueAvailable(int obj)
{
	Assertion(obj >= 0 && obj < static_cast<int>(m_slots.size()),
		"Query object index %d is invalid!", obj);
	auto& slot = m_slots[obj];

	if (!slot.inUse || slot.orphaned) {
		return true;
	}

	// Written on current frame but not yet submitted (flip hasn't happened).
	// Return false so process_gpu_events skips this and tries next frame.
	if (!slot.submitted) {
		return false;
	}

	uint64_t dummy;
	auto result = m_device.getQueryPoolResults(
		m_queryPool,
		static_cast<uint32_t>(obj), 1,
		sizeof(uint64_t), &dummy, sizeof(uint64_t),
		vk::QueryResultFlagBits::e64);

	return (result == vk::Result::eSuccess);
}

std::uint64_t VulkanQueryManager::getQueryValue(int obj)
{
	Assertion(obj >= 0 && obj < static_cast<int>(m_slots.size()),
		"Query object index %d is invalid!", obj);
	auto& slot = m_slots[obj];

	if (!slot.inUse || slot.orphaned) {
		return 0;
	}

	if (!slot.submitted) {
		return 0;
	}

	uint64_t ticks;
	auto result = m_device.getQueryPoolResults(
		m_queryPool,
		static_cast<uint32_t>(obj), 1,
		sizeof(uint64_t), &ticks, sizeof(uint64_t),
		vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);
	Assertion(result == vk::Result::eSuccess, "Failed to read query %d result!", obj);

	return static_cast<std::uint64_t>(static_cast<double>(ticks) * static_cast<double>(m_timestampPeriod));
}

void VulkanQueryManager::deleteQueryObject(int obj)
{
	Assertion(obj >= 0 && obj < static_cast<int>(m_slots.size()),
		"Query object index %d is invalid!", obj);
	auto& slot = m_slots[obj];

	slot.inUse = false;
	slot.orphaned = false;

	if (!slot.wasReset) {
		m_resetList.push_back(static_cast<uint32_t>(obj));
		m_deferredFreeSlots.push_back(static_cast<uint32_t>(obj));
	} else {
		m_freeSlots.push(static_cast<uint32_t>(obj));
	}
}

// Free function wrappers for gr_screen function pointers
int vulkan_create_query_object()
{
	return getQueryManager()->createQueryObject();
}

void vulkan_query_value(int obj, QueryType type)
{
	if (obj < 0) return;
	getQueryManager()->queryValue(obj, type);
}

bool vulkan_query_value_available(int obj)
{
	if (obj < 0) return true;
	return getQueryManager()->queryValueAvailable(obj);
}

std::uint64_t vulkan_get_query_value(int obj)
{
	if (obj < 0) return 0;
	return getQueryManager()->getQueryValue(obj);
}

void vulkan_delete_query_object(int obj)
{
	if (obj < 0) return;
	getQueryManager()->deleteQueryObject(obj);
}

} // namespace vulkan
} // namespace graphics
