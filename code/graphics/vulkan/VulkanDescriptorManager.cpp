
#include "VulkanDescriptorManager.h"

#ifdef WITH_VULKAN

namespace graphics {
namespace vulkan {

// Pool sizes based on FSO shader analysis:
// - 9 uniform block types (bindings 0-8)
// - 7-15 combined image samplers typical per shader
// - Storage buffers/images for compute (less common)
static constexpr uint32_t POOL_SIZE_UNIFORM_BUFFER = 64;
static constexpr uint32_t POOL_SIZE_UNIFORM_BUFFER_DYNAMIC = 64;
static constexpr uint32_t POOL_SIZE_COMBINED_IMAGE_SAMPLER = 256;
static constexpr uint32_t POOL_SIZE_SAMPLED_IMAGE = 64;
static constexpr uint32_t POOL_SIZE_STORAGE_BUFFER = 32;
static constexpr uint32_t POOL_SIZE_STORAGE_IMAGE = 32;
static constexpr uint32_t POOL_MAX_SETS = 128;

bool VulkanDescriptorManager::initialize(vk::Device device, vk::PhysicalDevice physicalDevice)
{
	if (m_initialized) {
		mprintf(("VulkanDescriptorManager already initialized\n"));
		return true;
	}

	m_device = device;

	// Query device limits for uniform buffer offset alignment
	auto properties = physicalDevice.getProperties();
	m_minUniformBufferOffsetAlignment = properties.limits.minUniformBufferOffsetAlignment;

	mprintf(("VulkanDescriptorManager initializing\n"));
	mprintf(("  Min uniform buffer offset alignment: %llu bytes\n",
	         static_cast<unsigned long long>(m_minUniformBufferOffsetAlignment)));

	if (!createPool()) {
		mprintf(("VulkanDescriptorManager: Failed to create descriptor pool\n"));
		return false;
	}

	m_initialized = true;
	mprintf(("VulkanDescriptorManager initialized successfully\n"));
	return true;
}

void VulkanDescriptorManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	mprintf(("VulkanDescriptorManager shutting down\n"));

	// Report any leaked descriptor sets
	if (!m_allocatedSets.empty()) {
		mprintf(("VulkanDescriptorManager: WARNING - %zu descriptor sets not freed:\n",
		         m_allocatedSets.size()));
		for (const auto& entry : m_allocatedSets) {
			mprintf(("  %s\n", entry.second.empty() ? "(unnamed)" : entry.second.c_str()));
		}
	}

	// Pool destruction automatically frees all allocated sets
	m_pool.reset();
	m_allocatedSets.clear();

	m_initialized = false;
	mprintf(("VulkanDescriptorManager shut down\n"));
}

bool VulkanDescriptorManager::createPool()
{
	std::vector<vk::DescriptorPoolSize> poolSizes = {
	    {vk::DescriptorType::eUniformBuffer, POOL_SIZE_UNIFORM_BUFFER},
	    {vk::DescriptorType::eUniformBufferDynamic, POOL_SIZE_UNIFORM_BUFFER_DYNAMIC},
	    {vk::DescriptorType::eCombinedImageSampler, POOL_SIZE_COMBINED_IMAGE_SAMPLER},
	    {vk::DescriptorType::eSampledImage, POOL_SIZE_SAMPLED_IMAGE},
	    {vk::DescriptorType::eStorageBuffer, POOL_SIZE_STORAGE_BUFFER},
	    {vk::DescriptorType::eStorageImage, POOL_SIZE_STORAGE_IMAGE},
	};

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	poolInfo.maxSets = POOL_MAX_SETS;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	try {
		m_pool = m_device.createDescriptorPoolUnique(poolInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanDescriptorManager: Failed to create descriptor pool: %s\n", e.what()));
		return false;
	}

	mprintf(("VulkanDescriptorManager: Created descriptor pool (maxSets=%u)\n", POOL_MAX_SETS));
	return true;
}

vk::DescriptorSet VulkanDescriptorManager::allocateSet(vk::DescriptorSetLayout layout, const SCP_string& debugName)
{
	if (!m_initialized) {
		mprintf(("VulkanDescriptorManager: Cannot allocate - not initialized\n"));
		return vk::DescriptorSet();
	}

	if (!layout) {
		mprintf(("VulkanDescriptorManager: Cannot allocate - null layout\n"));
		return vk::DescriptorSet();
	}

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = m_pool.get();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	try {
		auto sets = m_device.allocateDescriptorSets(allocInfo);
		vk::DescriptorSet set = sets[0];

		// Track for debugging
		m_allocatedSets[set] = debugName;

		if (!debugName.empty()) {
			mprintf(("VulkanDescriptorManager: Allocated descriptor set '%s'\n", debugName.c_str()));
		}

		return set;
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanDescriptorManager: Failed to allocate descriptor set '%s': %s\n",
		         debugName.c_str(), e.what()));
		return vk::DescriptorSet();
	}
}

void VulkanDescriptorManager::freeSet(vk::DescriptorSet set)
{
	if (!m_initialized || !set) {
		return;
	}

	auto it = m_allocatedSets.find(set);
	if (it != m_allocatedSets.end()) {
		if (!it->second.empty()) {
			mprintf(("VulkanDescriptorManager: Freeing descriptor set '%s'\n", it->second.c_str()));
		}
		m_allocatedSets.erase(it);
	}

	try {
		m_device.freeDescriptorSets(m_pool.get(), set);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanDescriptorManager: Failed to free descriptor set: %s\n", e.what()));
	}
}

void VulkanDescriptorManager::updateUniformBuffer(vk::DescriptorSet set, uint32_t binding, vk::Buffer buffer,
                                                   vk::DeviceSize offset, vk::DeviceSize range, bool dynamic)
{
	if (!set || !buffer) {
		return;
	}

	vk::DescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = buffer;
	bufferInfo.offset = offset;
	bufferInfo.range = range;

	vk::WriteDescriptorSet write;
	write.dstSet = set;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = dynamic ? vk::DescriptorType::eUniformBufferDynamic : vk::DescriptorType::eUniformBuffer;
	write.pBufferInfo = &bufferInfo;

	m_device.updateDescriptorSets(write, nullptr);
}

void VulkanDescriptorManager::updateCombinedImageSampler(vk::DescriptorSet set, uint32_t binding,
                                                          vk::ImageView imageView, vk::Sampler sampler,
                                                          vk::ImageLayout layout)
{
	if (!set || !imageView || !sampler) {
		return;
	}

	vk::DescriptorImageInfo imageInfo;
	imageInfo.imageView = imageView;
	imageInfo.sampler = sampler;
	imageInfo.imageLayout = layout;

	vk::WriteDescriptorSet write;
	write.dstSet = set;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	write.pImageInfo = &imageInfo;

	m_device.updateDescriptorSets(write, nullptr);
}

void VulkanDescriptorManager::updateStorageBuffer(vk::DescriptorSet set, uint32_t binding, vk::Buffer buffer,
                                                   vk::DeviceSize offset, vk::DeviceSize range)
{
	if (!set || !buffer) {
		return;
	}

	vk::DescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = buffer;
	bufferInfo.offset = offset;
	bufferInfo.range = range;

	vk::WriteDescriptorSet write;
	write.dstSet = set;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = vk::DescriptorType::eStorageBuffer;
	write.pBufferInfo = &bufferInfo;

	m_device.updateDescriptorSets(write, nullptr);
}

void VulkanDescriptorManager::updateStorageImage(vk::DescriptorSet set, uint32_t binding, vk::ImageView imageView,
                                                  vk::ImageLayout layout)
{
	if (!set || !imageView) {
		return;
	}

	vk::DescriptorImageInfo imageInfo;
	imageInfo.imageView = imageView;
	imageInfo.imageLayout = layout;

	vk::WriteDescriptorSet write;
	write.dstSet = set;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = vk::DescriptorType::eStorageImage;
	write.pImageInfo = &imageInfo;

	m_device.updateDescriptorSets(write, nullptr);
}

void VulkanDescriptorManager::bindDescriptorSet(vk::CommandBuffer cmd, vk::PipelineLayout pipelineLayout,
                                                 vk::DescriptorSet set, const SCP_vector<uint32_t>& dynamicOffsets,
                                                 uint32_t firstSet)
{
	if (!cmd || !pipelineLayout || !set) {
		return;
	}

	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, firstSet, set, dynamicOffsets);
}

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
