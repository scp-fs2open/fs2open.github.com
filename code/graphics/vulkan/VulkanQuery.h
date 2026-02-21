#pragma once

#include "graphics/2d.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

class VulkanQueryManager {
  public:
	bool init(vk::Device device, float timestampPeriod,
	          vk::CommandPool commandPool, vk::Queue queue);
	void shutdown();

	void beginFrame(vk::CommandBuffer commandBuffer);
	void notifySubmission();

	int createQueryObject();
	void queryValue(int obj, QueryType type);
	bool queryValueAvailable(int obj);
	std::uint64_t getQueryValue(int obj);
	void deleteQueryObject(int obj);

  private:
	static const uint32_t POOL_CAPACITY = 4096;

	struct QuerySlot {
		bool inUse = false;      // true after createQueryObject, false after deleteQueryObject
		bool submitted = false;  // true after notifySubmission confirms the write was submitted
		bool wasReset = true;    // true after reset (init or beginFrame), false after write
		bool orphaned = false;   // true if write was on an abandoned command buffer
	};

	vk::Device m_device;
	vk::QueryPool m_queryPool;
	SCP_vector<QuerySlot> m_slots;
	SCP_queue<uint32_t> m_freeSlots;          // available slot indices
	SCP_vector<uint32_t> m_resetList;          // slots to reset in next beginFrame
	SCP_vector<uint32_t> m_inflightResets;     // resets recorded but not yet confirmed submitted
	SCP_vector<uint32_t> m_pendingWrites;      // writes recorded but not yet confirmed submitted
	SCP_vector<uint32_t> m_deferredFreeSlots;  // deleted slots waiting for reset before returning to freeSlots
	float m_timestampPeriod = 0.0f;
	bool m_lastFrameSubmitted = true;          // false after beginFrame, true after notifySubmission
	uint32_t m_exhaustionMessageCount = 0;     // throttle exhaustion log spam
};

VulkanQueryManager* getQueryManager();
void setQueryManager(VulkanQueryManager* mgr);

// Free functions for gr_screen function pointers
int vulkan_create_query_object();
void vulkan_query_value(int obj, QueryType type);
bool vulkan_query_value_available(int obj);
std::uint64_t vulkan_get_query_value(int obj);
void vulkan_delete_query_object(int obj);

} // namespace vulkan
} // namespace graphics
