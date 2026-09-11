// Per-frame TLAS: gathers the current shadow-casting object set (ships,
// asteroids, debris) into one top-level acceleration structure each frame.
// Split out from VulkanRaytracing.cpp since this is the only part of the
// raytracing manager that reaches into Ships/Asteroids/Debris/Objects, a
// distinctly different dependency set from the BLAS cache in
// VulkanRaytracingBlas.cpp, and the part most likely to churn as shadow-caster
// selection evolves.

#include "VulkanRaytracing.h"
#include "VulkanBarrier.h"
#include "VulkanDeletionQueue.h"

#include "VulkanState.h"

#include "asteroid/asteroid.h"
#include "debris/debris.h"
#include "model/model.h"
#include "model/modelrender.h"
#include "object/object.h"
#include "render/3d.h"
#include "ship/ship.h"
#include "ship/ship_flags.h"

namespace graphics::vulkan {

// Mirrors model_draw_list::get_view_position() (modelrender.cpp): expresses the camera
// position in the local space of a submodel given that submodel's already-composed
// world orientation/position, which is exactly what model_render_check_detail_box()'s
// render-box/render-sphere distance checks are evaluated against.
static bool submodelPassesDetailBox(const polymodel* pm, int submodel_num, const matrix& world_orient, const vec3d& world_pos)
{
	vec3d eye_to_submodel;
	vm_vec_sub(&eye_to_submodel, &Eye_position, &world_pos);
	vec3d local_view_pos;
	vm_vec_rotate(&local_view_pos, &eye_to_submodel, &world_orient);

	return model_render_check_detail_box(&local_view_pos, pm, submodel_num, 0);
}

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

	if (!submodelPassesDetailBox(pm, submodel_num, orient, pos)) {
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

	matrix4 world_transform = stack.get_transform();
	matrix world_orient;
	vec3d world_pos;
	vm_matrix4_get_orientation(&world_orient, &world_transform);
	vm_matrix4_get_offset(&world_pos, &world_transform);

	// Mirrors the detail-box gate model_render_children_buffers applies (modelrender.cpp)
	// before rendering or recursing into a submodel's children. Without this, a submodel
	// the rasterizer only draws inside (or outside) a configured render-box/render-sphere
	// distance -- e.g. small greeble/antennae visible up close -- still cast raytraced
	// shadows regardless of camera distance, mismatching what was actually on screen and
	// producing self-shadowing flicker that only stabilized once the camera cleared the
	// model's detail-box distance. A failing check also skips this submodel's whole
	// subtree, exactly like the rasterized path (a culled parent hides its children too).
	if (!submodelPassesDetailBox(pm, submodel_num, world_orient, world_pos)) {
		stack.pop();
		return;
	}

	const BlasEntry* entry = getOrBuildBlasEntry(pm->id, submodel_num);
	if (entry != nullptr) {
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

bool VulkanRaytracingManager::ensureInstanceCapacity(FrameTlasResources& frame, vk::DeviceSize requiredBytes)
{
	if (requiredBytes <= frame.instanceCapacity) {
		return true;
	}

	m_graphicsQueue.waitIdle();

	if (frame.instanceBuffer) {
		if (frame.instanceMapped) {
			m_memoryManager->unmapMemory(frame.instanceAllocation);
			frame.instanceMapped = nullptr;
		}
		m_device.destroyBuffer(frame.instanceBuffer);
		frame.instanceBuffer = nullptr;
	}
	if (frame.instanceAllocation.isValid()) {
		m_memoryManager->freeAllocation(frame.instanceAllocation);
	}
	frame.instanceCapacity = 0;

	if (!createRawBuffer(requiredBytes,
			vk::BufferUsageFlagBits::eShaderDeviceAddress |
				vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
			MemoryUsage::CpuToGpu,
			frame.instanceBuffer,
			frame.instanceAllocation)) {
		return false;
	}

	frame.instanceMapped = m_memoryManager->mapMemory(frame.instanceAllocation);
	if (frame.instanceMapped == nullptr) {
		nprintf(("vulkan", "VulkanRaytracingManager: failed to map TLAS instance buffer\n"));
		return false;
	}

	frame.instanceCapacity = requiredBytes;
	return true;
}

bool VulkanRaytracingManager::ensureTlasCapacity(FrameTlasResources& frame, vk::DeviceSize requiredBytes)
{
	if (requiredBytes <= frame.tlasCapacity && frame.tlas) {
		return true;
	}

	// The old TLAS may still be referenced by a descriptor set bound earlier in
	// the command buffer currently being recorded this frame (e.g. an
	// already-rendered model with RT_SHADOWS), not just by prior in-flight
	// frames -- destroying it synchronously here invalidates that command
	// buffer even though it hasn't been submitted yet. Defer the actual
	// destruction via the deletion queue instead of an immediate reset/destroy.
	if (frame.tlas) {
		getDeletionQueue()->queueAccelerationStructure(frame.tlas.release());
	}
	if (frame.tlasBuffer) {
		getDeletionQueue()->queueBuffer(frame.tlasBuffer, frame.tlasAllocation);
		frame.tlasBuffer = nullptr;
		frame.tlasAllocation = VulkanAllocation();
	}
	frame.tlasCapacity = 0;

	if (!createRawBuffer(requiredBytes,
			vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			MemoryUsage::GpuOnly,
			frame.tlasBuffer,
			frame.tlasAllocation)) {
		return false;
	}

	vk::AccelerationStructureCreateInfoKHR createInfo;
	createInfo.buffer = frame.tlasBuffer;
	createInfo.offset = 0;
	createInfo.size = requiredBytes;
	createInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;

	try {
		frame.tlas = m_device.createAccelerationStructureKHRUnique(createInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanRaytracingManager: failed to create TLAS: %s\n", e.what()));
		m_device.destroyBuffer(frame.tlasBuffer);
		frame.tlasBuffer = nullptr;
		m_memoryManager->freeAllocation(frame.tlasAllocation);
		return false;
	}

	frame.tlasCapacity = requiredBytes;
	return true;
}

bool VulkanRaytracingManager::ensureScratchCapacity(FrameTlasResources& frame, vk::DeviceSize requiredBytes)
{
	// Over-allocate so the aligned address always fits within the buffer.
	vk::DeviceSize paddedSize = requiredBytes + m_scratchAlignment;
	if (paddedSize <= frame.tlasScratchCapacity) {
		return true;
	}

	m_graphicsQueue.waitIdle();

	if (frame.tlasScratchBuffer) {
		m_device.destroyBuffer(frame.tlasScratchBuffer);
		frame.tlasScratchBuffer = nullptr;
	}
	if (frame.tlasScratchAllocation.isValid()) {
		m_memoryManager->freeAllocation(frame.tlasScratchAllocation);
	}
	frame.tlasScratchCapacity = 0;

	if (!createRawBuffer(paddedSize,
			vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			MemoryUsage::GpuOnly,
			frame.tlasScratchBuffer,
			frame.tlasScratchAllocation)) {
		return false;
	}

	frame.tlasScratchCapacity = paddedSize;
	return true;
}

void VulkanRaytracingManager::buildTlas()
{
	if (!m_enabled) {
		return;
	}

	// Each frame-in-flight slot owns its own instance/TLAS/scratch buffers (see
	// FrameTlasResources' declaration for why a single shared set would race
	// across overlapping in-flight frames).
	FrameTlasResources& frame = m_frameTlas[currentFrameIndex()];

	SCP_vector<vk::AccelerationStructureInstanceKHR> instances;
	gatherShadowCasterInstances(instances);

	if (instances.empty()) {
		return; // keep whatever TLAS (if any) was built last time this slot was used
	}

	vk::DeviceSize requiredInstanceBytes = sizeof(vk::AccelerationStructureInstanceKHR) * instances.size();
	if (!ensureInstanceCapacity(frame, requiredInstanceBytes)) {
		return;
	}
	memcpy(frame.instanceMapped, instances.data(), static_cast<size_t>(requiredInstanceBytes));
	m_memoryManager->flushMemory(frame.instanceAllocation, 0, requiredInstanceBytes);

	vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
	instancesData.arrayOfPointers = VK_FALSE;
	instancesData.data.deviceAddress = m_device.getBufferAddress(vk::BufferDeviceAddressInfo{frame.instanceBuffer});

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

	if (!ensureTlasCapacity(frame, sizeInfo.accelerationStructureSize)) {
		return;
	}
	if (!ensureScratchCapacity(frame, sizeInfo.buildScratchSize)) {
		return;
	}

	buildInfo.dstAccelerationStructure = frame.tlas.get();
	vk::DeviceAddress scratchRaw = m_device.getBufferAddress(vk::BufferDeviceAddressInfo{frame.tlasScratchBuffer});
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
	// that ray-query against the TLAS via GlobalBinding::Tlas).
	cmdMemoryBarrier(cmd,
		vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
		vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eAccelerationStructureReadKHR);
}

} // namespace graphics::vulkan
