// Per-frame TLAS: gathers the current shadow-casting object set (ships,
// asteroids, debris) into one top-level acceleration structure each frame.
// Split out from VulkanRaytracing.cpp since this is the only part of the
// raytracing manager that reaches into Ships/Asteroids/Debris/Objects, a
// distinctly different dependency set from the BLAS cache in
// VulkanRaytracingBlas.cpp, and the part most likely to churn as shadow-caster
// selection evolves.

#include "VulkanRaytracing.h"
#include "VulkanDeletionQueue.h"

#include "VulkanState.h"

#include "asteroid/asteroid.h"
#include "debris/debris.h"
#include "model/model.h"
#include "object/object.h"
#include "ship/ship.h"
#include "ship/ship_flags.h"

namespace graphics::vulkan {

void VulkanRaytracingManager::pushInstance(SCP_vector<vk::AccelerationStructureInstanceKHR>& instances,
	vk::DeviceAddress blasAddress,
	const matrix& orient,
	const vec3d& pos)
{
	vk::AccelerationStructureInstanceKHR instance;
	instance.transform = toVkTransform(orient, pos);
	instance.instanceCustomIndex = 0;
	instance.mask = 0xFF;
	instance.instanceShaderBindingTableRecordOffset = 0;
	instance.flags = static_cast<VkGeometryInstanceFlagsKHR>(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
	instance.accelerationStructureReference = blasAddress;
	instances.push_back(instance);
}

void VulkanRaytracingManager::addSingleSubmodelInstance(SCP_vector<vk::AccelerationStructureInstanceKHR>& instances,
	const polymodel* pm,
	const polymodel_instance* pmi,
	int submodel_num,
	const matrix& orient,
	const vec3d& pos)
{
	if (submodel_num < 0 || submodel_num >= pm->n_models) {
		return;
	}

	if (pmi != nullptr && pmi->submodel[submodel_num].blown_off) {
		return;
	}

	if (pm->submodel[submodel_num].flags[Model::Submodel_flags::Is_thruster]) {
		return;
	}

	const BlasEntry* entry = getOrBuildBlasEntry(pm->id, submodel_num);
	if (entry == nullptr) {
		return;
	}

	pushInstance(instances, entry->address, orient, pos);
}

void VulkanRaytracingManager::walkSubmodelTree(SCP_vector<vk::AccelerationStructureInstanceKHR>& instances,
	transform_stack& stack,
	const polymodel* pm,
	const polymodel_instance* pmi,
	int submodel_num)
{
	if (submodel_num < 0 || submodel_num >= pm->n_models) {
		return;
	}

	const bsp_info& sm = pm->submodel[submodel_num];
	const submodel_instance* smi = nullptr;

	if (pmi != nullptr) {
		smi = &pmi->submodel[submodel_num];
		if (smi->blown_off) {
			return;
		}
	}

	// Thrusters are visual effects, not solid geometry -- CSM's shadow pass
	// never sets MR_SHOW_THRUSTERS either, so they're skipped unconditionally.
	if (sm.flags[Model::Submodel_flags::Is_thruster]) {
		return;
	}

	// Mirrors model_render_children_buffers' transform composition exactly
	// (modelrender.cpp) so BLAS-local geometry lands in the same world
	// position the rasterized path would draw it at.
	matrix submodel_orient = vmd_identity_matrix;
	vec3d submodel_offset = sm.offset;
	if (smi != nullptr) {
		submodel_orient = smi->canonical_orient;
		vm_vec_add2(&submodel_offset, &smi->canonical_offset);
	}

	stack.push(&submodel_offset, &submodel_orient);

	const BlasEntry* entry = getOrBuildBlasEntry(pm->id, submodel_num);
	if (entry != nullptr) {
		matrix4 world_transform = stack.get_transform();
		matrix world_orient;
		vec3d world_pos;
		vm_matrix4_get_orientation(&world_orient, &world_transform);
		vm_matrix4_get_offset(&world_pos, &world_transform);

		pushInstance(instances, entry->address, world_orient, world_pos);
	}

	for (int child = sm.first_child; child >= 0; child = pm->submodel[child].next_sibling) {
		if (!pm->submodel[child].flags[Model::Submodel_flags::Is_thruster]) {
			walkSubmodelTree(instances, stack, pm, pmi, child);
		}
	}

	stack.pop();
}

void VulkanRaytracingManager::gatherShadowCasterInstances(SCP_vector<vk::AccelerationStructureInstanceKHR>& instances)
{
	// Mirrors the object selection in shadows_render_all() (shadows.cpp) --
	// ships/asteroids/debris -- but without its per-cascade frustum
	// pre-filter, which lives in shadows.cpp's private state (Shadow_frustums)
	// and is specific to the rasterized cascade layout. Starting unfiltered is
	// simpler and safe (never wrongly excludes a caster); spatial culling of
	// the instance set can be added later if profiling shows it's needed.
	object* objp = Objects;
	for (int i = 0; i <= Highest_object_index; i++, objp++) {
		if (objp->flags[Object::Object_Flags::Should_be_dead]) {
			continue;
		}

		switch (objp->type) {
		case OBJ_SHIP: {
			ship* shipp = &Ships[objp->instance];
			if (shipp->flags[Ship::Ship_Flags::Cloaked]) {
				continue; // matches ship_render()'s cloak gate around model_render_queue
			}
			if (shipp->model_instance_num < 0) {
				continue; // model_get_instance() asserts on a negative index
			}

			polymodel_instance* pmi = model_get_instance(shipp->model_instance_num);
			if (pmi == nullptr) {
				continue;
			}
			polymodel* pm = model_get(pmi->model_num);
			if (pm == nullptr || pm->detail[0] < 0) {
				continue;
			}

			transform_stack stack;
			stack.push(&objp->pos, &objp->orient);
			walkSubmodelTree(instances, stack, pm, pmi, pm->detail[0]);
			break;
		}
		case OBJ_ASTEROID: {
			const asteroid& asp = Asteroids[objp->instance];
			int model_num = Asteroid_info[asp.asteroid_type].subtypes[asp.asteroid_subtype].model_number;
			polymodel* pm = model_get(model_num);
			if (pm == nullptr || pm->detail[0] < 0) {
				continue;
			}

			// Asteroids have no polymodel_instance -- no submodel animation.
			transform_stack stack;
			stack.push(&objp->pos, &objp->orient);
			walkSubmodelTree(instances, stack, pm, nullptr, pm->detail[0]);
			break;
		}
		case OBJ_DEBRIS: {
			const debris& db = Debris[objp->instance];
			if (!db.flags[Debris_Flags::Used]) {
				continue;
			}

			polymodel* pm = model_get(db.model_num);
			if (pm == nullptr) {
				continue;
			}
			polymodel_instance* pmi = db.model_instance_num < 0 ? nullptr : model_get_instance(db.model_instance_num);

			// Debris renders a single specific submodel directly in object
			// space (submodel_render_queue pushes only the object transform,
			// no submodel-local offset -- see modelrender.cpp), not a tree walk.
			addSingleSubmodelInstance(instances, pm, pmi, db.submodel_num, objp->orient, objp->pos);
			break;
		}
		default:
			break;
		}
	}
}

bool VulkanRaytracingManager::ensureInstanceCapacity(vk::DeviceSize requiredBytes)
{
	if (requiredBytes <= m_instanceCapacity) {
		return true;
	}

	m_graphicsQueue.waitIdle();

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

	if (!createRawBuffer(requiredBytes,
			vk::BufferUsageFlagBits::eShaderDeviceAddress |
				vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
			MemoryUsage::CpuToGpu,
			m_instanceBuffer,
			m_instanceAllocation)) {
		return false;
	}

	m_instanceMapped = m_memoryManager->mapMemory(m_instanceAllocation);
	if (m_instanceMapped == nullptr) {
		nprintf(("vulkan", "VulkanRaytracingManager: failed to map TLAS instance buffer\n"));
		return false;
	}

	m_instanceCapacity = requiredBytes;
	return true;
}

bool VulkanRaytracingManager::ensureTlasCapacity(vk::DeviceSize requiredBytes)
{
	if (requiredBytes <= m_tlasCapacity && m_tlas) {
		return true;
	}

	// The old TLAS may still be referenced by a descriptor set bound earlier in
	// the command buffer currently being recorded this frame (e.g. an
	// already-rendered model with RT_SHADOWS), not just by prior in-flight
	// frames -- destroying it synchronously here invalidates that command
	// buffer even though it hasn't been submitted yet. Defer the actual
	// destruction via the deletion queue instead of an immediate reset/destroy.
	if (m_tlas) {
		getDeletionQueue()->queueAccelerationStructure(m_tlas.release());
	}
	if (m_tlasBuffer) {
		getDeletionQueue()->queueBuffer(m_tlasBuffer, m_tlasAllocation);
		m_tlasBuffer = nullptr;
		m_tlasAllocation = VulkanAllocation();
	}
	m_tlasCapacity = 0;

	if (!createRawBuffer(requiredBytes,
			vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			MemoryUsage::GpuOnly,
			m_tlasBuffer,
			m_tlasAllocation)) {
		return false;
	}

	vk::AccelerationStructureCreateInfoKHR createInfo;
	createInfo.buffer = m_tlasBuffer;
	createInfo.offset = 0;
	createInfo.size = requiredBytes;
	createInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;

	try {
		m_tlas = m_device.createAccelerationStructureKHRUnique(createInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanRaytracingManager: failed to create TLAS: %s\n", e.what()));
		m_device.destroyBuffer(m_tlasBuffer);
		m_tlasBuffer = nullptr;
		m_memoryManager->freeAllocation(m_tlasAllocation);
		return false;
	}

	m_tlasCapacity = requiredBytes;
	return true;
}

bool VulkanRaytracingManager::ensureScratchCapacity(vk::DeviceSize requiredBytes)
{
	// Over-allocate so the aligned address always fits within the buffer.
	vk::DeviceSize paddedSize = requiredBytes + m_scratchAlignment;
	if (paddedSize <= m_tlasScratchCapacity) {
		return true;
	}

	m_graphicsQueue.waitIdle();

	if (m_tlasScratchBuffer) {
		m_device.destroyBuffer(m_tlasScratchBuffer);
		m_tlasScratchBuffer = nullptr;
	}
	if (m_tlasScratchAllocation.isValid()) {
		m_memoryManager->freeAllocation(m_tlasScratchAllocation);
	}
	m_tlasScratchCapacity = 0;

	if (!createRawBuffer(paddedSize,
			vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			MemoryUsage::GpuOnly,
			m_tlasScratchBuffer,
			m_tlasScratchAllocation)) {
		return false;
	}

	m_tlasScratchCapacity = paddedSize;
	return true;
}

void VulkanRaytracingManager::buildTlas()
{
	if (!m_enabled) {
		return;
	}

	SCP_vector<vk::AccelerationStructureInstanceKHR> instances;
	gatherShadowCasterInstances(instances);

	if (instances.empty()) {
		return; // keep whatever TLAS (if any) was built last frame
	}

	vk::DeviceSize requiredInstanceBytes = sizeof(vk::AccelerationStructureInstanceKHR) * instances.size();
	if (!ensureInstanceCapacity(requiredInstanceBytes)) {
		return;
	}
	memcpy(m_instanceMapped, instances.data(), static_cast<size_t>(requiredInstanceBytes));
	m_memoryManager->flushMemory(m_instanceAllocation, 0, requiredInstanceBytes);

	vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
	instancesData.arrayOfPointers = VK_FALSE;
	instancesData.data.deviceAddress = m_device.getBufferAddress(vk::BufferDeviceAddressInfo{m_instanceBuffer});

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

	auto primitiveCount = static_cast<uint32_t>(instances.size());
	vk::AccelerationStructureBuildSizesInfoKHR sizeInfo = m_device.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfo, primitiveCount);

	if (!ensureTlasCapacity(sizeInfo.accelerationStructureSize)) {
		return;
	}
	if (!ensureScratchCapacity(sizeInfo.buildScratchSize)) {
		return;
	}

	buildInfo.dstAccelerationStructure = m_tlas.get();
	vk::DeviceAddress scratchRaw = m_device.getBufferAddress(vk::BufferDeviceAddressInfo{m_tlasScratchBuffer});
	buildInfo.scratchData.deviceAddress = alignUp(scratchRaw, m_scratchAlignment);

	vk::AccelerationStructureBuildRangeInfoKHR range;
	range.primitiveCount = primitiveCount;
	const vk::AccelerationStructureBuildRangeInfoKHR* pRange = &range;

	// Recorded into the *current frame's* command buffer (not a one-shot
	// submission) so this doesn't stall the CPU/GPU every frame the way
	// buildBlas()'s load-time waitIdle() path would.
	vk::CommandBuffer cmd = getStateTracker()->getCommandBuffer();
	cmd.buildAccelerationStructuresKHR(1, &buildInfo, &pRange);

	// Acceleration structure write -> future shader reads (the fragment shaders
	// that ray-query against the TLAS via GlobalBinding::Tlas).  Legacy
	// (non-sync2) barrier since VK_KHR_synchronization2 isn't negotiated by
	// this renderer.
	vk::MemoryBarrier barrier;
	barrier.srcAccessMask = vk::AccessFlagBits::eAccelerationStructureWriteKHR;
	barrier.dstAccessMask = vk::AccessFlagBits::eAccelerationStructureReadKHR;
	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::DependencyFlags(),
		1,
		&barrier,
		0,
		nullptr,
		0,
		nullptr);
}

} // namespace graphics::vulkan
