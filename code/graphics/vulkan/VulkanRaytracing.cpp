#include "VulkanRaytracing.h"

namespace graphics::vulkan {

namespace {
VulkanRaytracingManager* g_raytracingManager = nullptr;
} // namespace

vk::DeviceAddress VulkanRaytracingManager::alignUp(vk::DeviceAddress value, uint32_t alignment)
{
	if (alignment <= 1) {
		return value;
	}
	return (value + alignment - 1) & ~static_cast<vk::DeviceAddress>(alignment - 1);
}

// Converts an FS2Open world orientation + position into Vulkan's row-major 3x4
// VkTransformMatrixKHR, i.e. the LOCAL-TO-WORLD affine transform for AS instance
// placement: world = R*local + pos.
//
// FS2Open's matrix*vec3d (operator*, vm_vec_rotate) is WORLD-TO-LOCAL: it dots
// local-frame basis vectors (rvec/uvec/fvec, as ROWS) against a vector, i.e.
// out = M*v where M's rows are rvec/uvec/fvec. The inverse operation --
// vm_vec_unrotate, LOCAL-TO-WORLD -- is out = v.x*rvec + v.y*uvec + v.z*fvec,
// which is M^T*v, not M*v. Confirmed independently via the known-correct GPU
// modelMatrix path: vm_matrix4_set_transform uploads columns [rvec,uvec,fvec,pos]
// into a column-major matrix4 that the shader applies as world = modelMatrix*local,
// which expands to world.x = rvec.x*local.x + uvec.x*local.y + fvec.x*local.z + pos.x
// -- i.e. row i of the local-to-world transform is (rvec[i], uvec[i], fvec[i]),
// the TRANSPOSE of M, not M itself.
//
// Exposed (not anonymous-namespace-local) so it can be unit tested directly --
// see test/src/graphics/vulkan/test_vulkan_raytracing.cpp.
vk::TransformMatrixKHR toVkTransform(const matrix& orient, const vec3d& pos)
{
	vk::TransformMatrixKHR out;
	out.matrix[0][0] = orient.vec.rvec.xyz.x;
	out.matrix[0][1] = orient.vec.uvec.xyz.x;
	out.matrix[0][2] = orient.vec.fvec.xyz.x;
	out.matrix[0][3] = pos.xyz.x;
	out.matrix[1][0] = orient.vec.rvec.xyz.y;
	out.matrix[1][1] = orient.vec.uvec.xyz.y;
	out.matrix[1][2] = orient.vec.fvec.xyz.y;
	out.matrix[1][3] = pos.xyz.y;
	out.matrix[2][0] = orient.vec.rvec.xyz.z;
	out.matrix[2][1] = orient.vec.uvec.xyz.z;
	out.matrix[2][2] = orient.vec.fvec.xyz.z;
	out.matrix[2][3] = pos.xyz.z;
	return out;
}

VulkanRaytracingManager* getRaytracingManager()
{
	return g_raytracingManager;
}

void setRaytracingManager(VulkanRaytracingManager* manager)
{
	g_raytracingManager = manager;
}

VulkanRaytracingManager::VulkanRaytracingManager() = default;

VulkanRaytracingManager::~VulkanRaytracingManager()
{
	if (m_initialized) {
		shutdown();
	}
}

bool VulkanRaytracingManager::init(vk::Device device,
	vk::PhysicalDevice physicalDevice,
	VulkanMemoryManager* memoryManager,
	VulkanBufferManager* bufferManager,
	vk::CommandPool commandPool,
	vk::Queue graphicsQueue,
	bool enabled)
{
	m_device = device;
	m_physicalDevice = physicalDevice;
	m_memoryManager = memoryManager;
	m_bufferManager = bufferManager;
	m_commandPool = commandPool;
	m_graphicsQueue = graphicsQueue;
	m_enabled = enabled;
	m_initialized = true;

	if (!m_enabled) {
		nprintf(("vulkan", "VulkanRaytracingManager: raytraced shadows disabled, BLAS cache is inactive\n"));
		return true;
	}

	auto propsChain = m_physicalDevice
		.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();
	m_scratchAlignment =
		propsChain.get<vk::PhysicalDeviceAccelerationStructurePropertiesKHR>().minAccelerationStructureScratchOffsetAlignment;
	if (m_scratchAlignment == 0) {
		m_scratchAlignment = 1;
	}

	nprintf(("vulkan", "VulkanRaytracingManager: initialized (scratch alignment %u)\n", m_scratchAlignment));

	if (!buildFallbackTlas()) {
		nprintf(("vulkan", "VulkanRaytracingManager: failed to build fallback TLAS, disabling raytraced shadows\n"));
		m_enabled = false;
	}

	return true;
}

bool VulkanRaytracingManager::buildFallbackTlas()
{
	// One-time, 0-instance TLAS used as a valid (non-null) descriptor value
	// whenever no real TLAS has been built yet. AS descriptor writes require
	// a real handle -- VK_NULL_HANDLE is not valid without the nullDescriptor
	// feature, which this renderer does not negotiate.
	vk::Buffer instanceBuffer;
	VulkanAllocation instanceAllocation;
	if (!createRawBuffer(sizeof(vk::AccelerationStructureInstanceKHR),
			vk::BufferUsageFlagBits::eShaderDeviceAddress |
				vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
			MemoryUsage::GpuOnly,
			instanceBuffer,
			instanceAllocation)) {
		return false;
	}

	vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
	instancesData.arrayOfPointers = VK_FALSE;
	instancesData.data.deviceAddress = m_device.getBufferAddress(vk::BufferDeviceAddressInfo{instanceBuffer});

	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eInstances;
	geometry.geometry = instancesData;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;

	vk::AccelerationStructureBuildGeometryInfoKHR buildInfo;
	buildInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	buildInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	buildInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometry;

	uint32_t primitiveCount = 0;
	vk::AccelerationStructureBuildSizesInfoKHR sizeInfo = m_device.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfo, primitiveCount);

	vk::AccelerationStructureBuildRangeInfoKHR range;
	range.primitiveCount = 0;
	const vk::AccelerationStructureBuildRangeInfoKHR* pRange = &range;

	bool built = buildAccelerationStructureOneShot(vk::AccelerationStructureTypeKHR::eTopLevel, buildInfo, sizeInfo,
		&pRange, m_fallbackTlas, m_fallbackTlasBuffer, m_fallbackTlasAllocation);

	m_device.destroyBuffer(instanceBuffer);
	m_memoryManager->freeAllocation(instanceAllocation);

	return built;
}

void VulkanRaytracingManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	for (auto& kv : m_blasCache) {
		destroyBlasEntry(kv.second);
	}
	m_blasCache.clear();

	m_fallbackTlas.reset();
	if (m_fallbackTlasBuffer) {
		m_device.destroyBuffer(m_fallbackTlasBuffer);
		m_fallbackTlasBuffer = nullptr;
	}
	if (m_fallbackTlasAllocation.isValid()) {
		m_memoryManager->freeAllocation(m_fallbackTlasAllocation);
	}

	m_tlas.reset();
	if (m_tlasBuffer) {
		m_device.destroyBuffer(m_tlasBuffer);
		m_tlasBuffer = nullptr;
	}
	if (m_tlasAllocation.isValid()) {
		m_memoryManager->freeAllocation(m_tlasAllocation);
	}
	m_tlasCapacity = 0;

	if (m_tlasScratchBuffer) {
		m_device.destroyBuffer(m_tlasScratchBuffer);
		m_tlasScratchBuffer = nullptr;
	}
	if (m_tlasScratchAllocation.isValid()) {
		m_memoryManager->freeAllocation(m_tlasScratchAllocation);
	}
	m_tlasScratchCapacity = 0;

	if (m_instanceBuffer) {
		if (m_instanceMapped) {
			m_memoryManager->unmapMemory(m_instanceAllocation);
			m_instanceMapped = nullptr;
		}
		m_device.destroyBuffer(m_instanceBuffer);
		m_instanceBuffer = nullptr;
	}
	if (m_instanceAllocation.isValid()) {
		m_memoryManager->freeAllocation(m_instanceAllocation);
	}
	m_instanceCapacity = 0;

	m_initialized = false;
}

uint64_t VulkanRaytracingManager::makeKey(int pm_id, int submodel_num)
{
	return (static_cast<uint64_t>(static_cast<uint32_t>(pm_id)) << 32) | static_cast<uint32_t>(submodel_num);
}

void VulkanRaytracingManager::destroyBlasEntry(BlasEntry& entry)
{
	entry.accelStruct.reset();

	if (entry.buffer) {
		m_device.destroyBuffer(entry.buffer);
		entry.buffer = nullptr;
	}
	if (entry.allocation.isValid()) {
		m_memoryManager->freeAllocation(entry.allocation);
	}
}

vk::CommandBuffer VulkanRaytracingManager::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer = m_device.allocateCommandBuffers(allocInfo)[0];

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

void VulkanRaytracingManager::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	m_graphicsQueue.submit(submitInfo, nullptr);
	m_graphicsQueue.waitIdle();

	m_device.freeCommandBuffers(m_commandPool, commandBuffer);
}

bool VulkanRaytracingManager::createRawBuffer(vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	MemoryUsage memUsage,
	vk::Buffer& outBuffer,
	VulkanAllocation& outAllocation)
{
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	try {
		outBuffer = m_device.createBuffer(bufferInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanRaytracingManager: failed to create buffer: %s\n", e.what()));
		return false;
	}

	if (!m_memoryManager->allocateBufferMemory(outBuffer, memUsage, outAllocation)) {
		m_device.destroyBuffer(outBuffer);
		outBuffer = nullptr;
		nprintf(("vulkan", "VulkanRaytracingManager: failed to allocate buffer memory\n"));
		return false;
	}

	return true;
}

bool VulkanRaytracingManager::buildAccelerationStructureOneShot(vk::AccelerationStructureTypeKHR type,
	vk::AccelerationStructureBuildGeometryInfoKHR buildInfo,
	const vk::AccelerationStructureBuildSizesInfoKHR& sizeInfo,
	const vk::AccelerationStructureBuildRangeInfoKHR* const* pBuildRanges,
	vk::UniqueAccelerationStructureKHR& outAS,
	vk::Buffer& outBuffer,
	VulkanAllocation& outAllocation)
{
	if (!createRawBuffer(sizeInfo.accelerationStructureSize,
			vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			MemoryUsage::GpuOnly,
			outBuffer,
			outAllocation)) {
		return false;
	}

	vk::AccelerationStructureCreateInfoKHR createInfo;
	createInfo.buffer = outBuffer;
	createInfo.offset = 0;
	createInfo.size = sizeInfo.accelerationStructureSize;
	createInfo.type = type;

	try {
		outAS = m_device.createAccelerationStructureKHRUnique(createInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanRaytracingManager: failed to create acceleration structure: %s\n", e.what()));
		m_device.destroyBuffer(outBuffer);
		outBuffer = nullptr;
		m_memoryManager->freeAllocation(outAllocation);
		return false;
	}

	// Over-allocate the scratch buffer so we can round its device address up to
	// the required alignment without moving the allocation.
	vk::Buffer scratchBuffer;
	VulkanAllocation scratchAllocation;
	vk::DeviceSize scratchAllocSize = sizeInfo.buildScratchSize + m_scratchAlignment;
	if (!createRawBuffer(scratchAllocSize,
			vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			MemoryUsage::GpuOnly,
			scratchBuffer,
			scratchAllocation)) {
		outAS.reset();
		m_device.destroyBuffer(outBuffer);
		outBuffer = nullptr;
		m_memoryManager->freeAllocation(outAllocation);
		return false;
	}

	vk::DeviceAddress scratchRawAddress = m_device.getBufferAddress(vk::BufferDeviceAddressInfo{scratchBuffer});
	buildInfo.dstAccelerationStructure = outAS.get();
	buildInfo.scratchData.deviceAddress = alignUp(scratchRawAddress, m_scratchAlignment);

	vk::CommandBuffer cmd = beginSingleTimeCommands();
	cmd.buildAccelerationStructuresKHR(1, &buildInfo, pBuildRanges);
	endSingleTimeCommands(cmd);

	m_device.destroyBuffer(scratchBuffer);
	m_memoryManager->freeAllocation(scratchAllocation);

	return true;
}

} // namespace graphics::vulkan
