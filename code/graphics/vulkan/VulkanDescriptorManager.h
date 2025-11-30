#pragma once

#include "globalincs/pstypes.h"

#ifdef WITH_VULKAN

#include <vulkan/vulkan.hpp>
#include <map>
#include <vector>

namespace graphics {
namespace vulkan {

/**
 * @brief Manages Vulkan descriptor pools and sets
 *
 * Handles allocation, updating, and binding of descriptor sets for shaders.
 * Uses a single pool for simplicity with dynamic uniform buffer offsets
 * for efficient per-frame uniform updates.
 */
class VulkanDescriptorManager {
public:
	VulkanDescriptorManager() = default;
	~VulkanDescriptorManager() = default;

	// Non-copyable, non-movable
	VulkanDescriptorManager(const VulkanDescriptorManager&) = delete;
	VulkanDescriptorManager& operator=(const VulkanDescriptorManager&) = delete;
	VulkanDescriptorManager(VulkanDescriptorManager&&) = delete;
	VulkanDescriptorManager& operator=(VulkanDescriptorManager&&) = delete;

	/**
	 * @brief Initialize the descriptor manager
	 * @param device The Vulkan logical device
	 * @param physicalDevice The Vulkan physical device (for querying limits)
	 * @return true on success
	 */
	bool initialize(vk::Device device, vk::PhysicalDevice physicalDevice);

	/**
	 * @brief Shutdown and cleanup resources
	 */
	void shutdown();

	/**
	 * @brief Allocate a descriptor set from the pool
	 * @param layout The descriptor set layout
	 * @param debugName Optional name for debugging
	 * @return Allocated descriptor set (may be null on failure)
	 */
	vk::DescriptorSet allocateSet(vk::DescriptorSetLayout layout, const SCP_string& debugName = "");

	/**
	 * @brief Free a previously allocated descriptor set
	 * @param set The descriptor set to free
	 */
	void freeSet(vk::DescriptorSet set);

	/**
	 * @brief Update a uniform buffer binding in a descriptor set
	 * @param set The descriptor set to update
	 * @param binding The binding slot
	 * @param buffer The buffer to bind
	 * @param offset Offset into the buffer
	 * @param range Size of the buffer range to bind
	 * @param dynamic If true, uses dynamic uniform buffer type
	 */
	void updateUniformBuffer(vk::DescriptorSet set, uint32_t binding,
	                         vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range,
	                         bool dynamic = true);

	/**
	 * @brief Update a combined image sampler binding in a descriptor set
	 * @param set The descriptor set to update
	 * @param binding The binding slot
	 * @param imageView The image view to bind
	 * @param sampler The sampler to use
	 * @param layout The image layout (default: shader read only)
	 */
	void updateCombinedImageSampler(vk::DescriptorSet set, uint32_t binding,
	                                vk::ImageView imageView, vk::Sampler sampler,
	                                vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);

	/**
	 * @brief Update a storage buffer binding in a descriptor set
	 * @param set The descriptor set to update
	 * @param binding The binding slot
	 * @param buffer The buffer to bind
	 * @param offset Offset into the buffer
	 * @param range Size of the buffer range to bind
	 */
	void updateStorageBuffer(vk::DescriptorSet set, uint32_t binding,
	                         vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range);

	/**
	 * @brief Update a storage image binding in a descriptor set
	 * @param set The descriptor set to update
	 * @param binding The binding slot
	 * @param imageView The image view to bind
	 * @param layout The image layout (default: general for read/write)
	 */
	void updateStorageImage(vk::DescriptorSet set, uint32_t binding,
	                        vk::ImageView imageView,
	                        vk::ImageLayout layout = vk::ImageLayout::eGeneral);

	/**
	 * @brief Bind a descriptor set with dynamic offsets
	 * @param cmd The command buffer
	 * @param pipelineLayout The pipeline layout
	 * @param set The descriptor set to bind
	 * @param dynamicOffsets Dynamic offsets for uniform buffers
	 * @param firstSet The first descriptor set index (default 0)
	 */
	void bindDescriptorSet(vk::CommandBuffer cmd, vk::PipelineLayout pipelineLayout,
	                       vk::DescriptorSet set,
	                       const SCP_vector<uint32_t>& dynamicOffsets = {},
	                       uint32_t firstSet = 0);

	/**
	 * @brief Get the minimum alignment for dynamic uniform buffer offsets
	 * @return Alignment in bytes
	 */
	vk::DeviceSize getMinUniformBufferOffsetAlignment() const { return m_minUniformBufferOffsetAlignment; }

	/**
	 * @brief Check if the manager is initialized
	 */
	bool isInitialized() const { return m_initialized; }

private:
	vk::Device m_device;
	vk::UniqueDescriptorPool m_pool;
	vk::DeviceSize m_minUniformBufferOffsetAlignment = 256;  // Common default, updated at init

	// Track allocated sets for debugging
	std::map<vk::DescriptorSet, SCP_string> m_allocatedSets;

	bool m_initialized = false;

	/**
	 * @brief Create the descriptor pool
	 * @return true on success
	 */
	bool createPool();
};

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
