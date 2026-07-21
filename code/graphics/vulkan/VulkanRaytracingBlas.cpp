// BLAS cache: one bottom-level acceleration structure per unique (polymodel id,
// submodel index) pair, built lazily from the submodel's own rasterization
// geometry buffers and kept for the model's lifetime (see VulkanRaytracing.h's
// class comment). Split out from VulkanRaytracing.cpp since this is the only
// part of the raytracing manager that reaches into model/model.h and the
// per-model load/unload lifecycle, a distinctly different dependency set from
// the per-frame TLAS/scene-gather code in VulkanRaytracingTlas.cpp.

#include "VulkanRaytracing.h"

#include "model/model.h"

namespace graphics::vulkan {

bool VulkanRaytracingManager::buildBlas(BlasEntry& entry, int pm_id, int submodel_num)
{
	polymodel* pm = model_get(pm_id);
	Assertion(pm != nullptr, "VulkanRaytracingManager::buildBlas got invalid pm_id %d!", pm_id);
	Assertion(submodel_num >= 0 && submodel_num < pm->n_models,
		"VulkanRaytracingManager::buildBlas got out-of-range submodel_num %d for model '%s' (n_models=%d)!",
		submodel_num, pm->filename, pm->n_models);

	const bsp_info& sm = pm->submodel[submodel_num];
	const vertex_buffer& vb = sm.buffer;

	if (!(vb.flags & VB_FLAG_POSITION) || vb.tex_buf.empty()) {
		return false; // pure hierarchy node / no renderable geometry -- not an error
	}

	if (!pm->vert_source.Vbuffer_handle.isValid() || !pm->vert_source.Ibuffer_handle.isValid()) {
		nprintf(("vulkan", "VulkanRaytracingManager: model '%s' has no GPU geometry buffers yet\n", pm->filename));
		return false;
	}

	// Locate the POSITION3 attribute within this submodel's interleaved vertex layout.
	const vertex_format_data* posComponent = nullptr;
	for (size_t i = 0; i < vb.layout.get_num_vertex_components(); ++i) {
		const vertex_format_data* comp = vb.layout.get_vertex_component(i);
		if (comp->format_type == vertex_format_data::POSITION3) {
			posComponent = comp;
			break;
		}
	}
	if (posComponent == nullptr) {
		nprintf(("vulkan", "VulkanRaytracingManager: submodel %d of '%s' has no POSITION3 vertex component\n",
			submodel_num, pm->filename));
		return false;
	}

	vk::Buffer vbuffer = m_bufferManager->getVkBuffer(pm->vert_source.Vbuffer_handle);
	vk::Buffer ibuffer = m_bufferManager->getVkBuffer(pm->vert_source.Ibuffer_handle);
	if (!vbuffer || !ibuffer) {
		return false;
	}

	// Same base-vertex arithmetic as the rasterized draw path (VulkanDraw.cpp
	// renderModel): a polymodel-level heap offset plus a submodel-local offset,
	// both in vertex units. Baked directly into the vertexData address below
	// (firstVertex left at 0) to avoid any ambiguity in how maxVertex interacts
	// with a nonzero firstVertex.
	size_t baseVertex = pm->vert_source.Base_vertex_offset + vb.vertex_num_offset;

	vk::DeviceAddress vertexBase = m_device.getBufferAddress(vk::BufferDeviceAddressInfo{vbuffer}) +
		posComponent->offset + baseVertex * vb.stride;
	vk::DeviceAddress indexBase =
		m_device.getBufferAddress(vk::BufferDeviceAddressInfo{ibuffer}) + pm->vert_source.Index_offset;

	// One geometry entry per non-transparent texture bucket; each bucket has its
	// own index range within the submodel.
	SCP_vector<vk::AccelerationStructureGeometryKHR> geometries;
	SCP_vector<uint32_t> maxPrimitiveCounts;
	SCP_vector<vk::AccelerationStructureBuildRangeInfoKHR> buildRanges;
	geometries.reserve(vb.tex_buf.size());
	maxPrimitiveCounts.reserve(vb.tex_buf.size());
	buildRanges.reserve(vb.tex_buf.size());

	for (const buffer_data& bucket : vb.tex_buf) {
		if (bucket.n_verts == 0) {
			continue;
		}

		// Skip buckets whose texture is transparent (or has no base texture at
		// all), mirroring the rasterized shadow path's bucket filter in
		// shadow_render_list::add_model_draws (shadows.cpp) -- glass, forcefields,
		// nameplates, etc. must not occlude raytraced shadow rays either.
		int tmap_num = bucket.texture;
		if (tmap_num < 0 || pm->maps[tmap_num].is_transparent) {
			continue;
		}
		if (pm->maps[tmap_num].textures[TM_BASE_TYPE].GetTexture() < 0) {
			continue;
		}

		// n_verts is actually an index count (see VulkanDraw.cpp renderModel).
		auto primitiveCount = static_cast<uint32_t>(bucket.n_verts / 3);
		if (primitiveCount == 0) {
			continue;
		}

		bool largeIndex = (bucket.flags & VB_FLAG_LARGE_INDEX) != 0;

		vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
		triangles.vertexFormat = vk::Format::eR32G32B32Sfloat; // POSITION3
		triangles.vertexData.deviceAddress = vertexBase;
		triangles.vertexStride = vb.stride;
		triangles.maxVertex = static_cast<uint32_t>(bucket.i_last);
		triangles.indexType = largeIndex ? vk::IndexType::eUint32 : vk::IndexType::eUint16;
		triangles.indexData.deviceAddress = indexBase + bucket.index_offset;

		vk::AccelerationStructureGeometryKHR geometry;
		geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
		geometry.geometry = triangles;
		geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
		geometries.push_back(geometry);

		maxPrimitiveCounts.push_back(primitiveCount);

		vk::AccelerationStructureBuildRangeInfoKHR range;
		range.primitiveCount = primitiveCount;
		range.primitiveOffset = 0;
		range.firstVertex = 0;
		range.transformOffset = 0;
		buildRanges.push_back(range);
	}

	if (geometries.empty()) {
		return false;
	}

	vk::AccelerationStructureBuildGeometryInfoKHR buildInfo;
	buildInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	buildInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	buildInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	buildInfo.geometryCount = static_cast<uint32_t>(geometries.size());
	buildInfo.pGeometries = geometries.data();

	vk::AccelerationStructureBuildSizesInfoKHR sizeInfo = m_device.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfo, maxPrimitiveCounts);

	SCP_vector<const vk::AccelerationStructureBuildRangeInfoKHR*> pBuildRanges = {buildRanges.data()};

	vk::UniqueAccelerationStructureKHR accelStruct;
	vk::Buffer asBuffer;
	VulkanAllocation asAllocation;
	if (!buildAccelerationStructureOneShot(vk::AccelerationStructureTypeKHR::eBottomLevel, buildInfo, sizeInfo,
			pBuildRanges.data(), accelStruct, asBuffer, asAllocation)) {
		return false;
	}

	entry.address =
		m_device.getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR{accelStruct.get()});
	entry.accelStruct = std::move(accelStruct);
	entry.buffer = asBuffer;
	entry.allocation = asAllocation;
	entry.vertexBufferGeneration = m_bufferManager->getBufferGeneration(pm->vert_source.Vbuffer_handle);
	entry.indexBufferGeneration = m_bufferManager->getBufferGeneration(pm->vert_source.Ibuffer_handle);

	return true;
}

const VulkanRaytracingManager::BlasEntry* VulkanRaytracingManager::getOrBuildBlasEntry(int pm_id, int submodel_num)
{
	if (!m_enabled) {
		return nullptr;
	}

	polymodel* pm = model_get(pm_id);
	if (pm == nullptr || submodel_num < 0 || submodel_num >= pm->n_models) {
		return nullptr;
	}

	uint64_t key = makeKey(pm_id, submodel_num);
	auto it = m_blasCache.find(key);
	if (it != m_blasCache.end()) {
		uint64_t curVertGen = m_bufferManager->getBufferGeneration(pm->vert_source.Vbuffer_handle);
		uint64_t curIndexGen = m_bufferManager->getBufferGeneration(pm->vert_source.Ibuffer_handle);
		if (curVertGen == it->second.vertexBufferGeneration && curIndexGen == it->second.indexBufferGeneration) {
			return &it->second;
		}

		// Backing GPU heap was resized since this BLAS was built (stale device
		// address) -- destroy and fall through to rebuild.
		destroyBlasEntry(it->second);
		m_blasCache.erase(it);
	}

	BlasEntry entry;
	if (!buildBlas(entry, pm_id, submodel_num)) {
		return nullptr;
	}

	auto insertResult = m_blasCache.emplace(key, std::move(entry));
	return &insertResult.first->second;
}

vk::AccelerationStructureKHR VulkanRaytracingManager::getOrBuildSubmodelBlas(int pm_id, int submodel_num)
{
	const BlasEntry* entry = getOrBuildBlasEntry(pm_id, submodel_num);
	return entry != nullptr ? entry->accelStruct.get() : vk::AccelerationStructureKHR{};
}

void VulkanRaytracingManager::onModelLoaded(int pm_id)
{
	if (!m_enabled) {
		return;
	}

	polymodel* pm = model_get(pm_id);
	if (pm == nullptr) {
		return;
	}

	int root = pm->detail[0];
	if (root < 0 || root >= pm->n_models) {
		return;
	}

	// Walk the LOD0 submodel tree and warm the BLAS cache for every submodel
	// with renderable geometry. This is bounded to LOD0 (not every LOD/debris
	// variant) to keep load-time cost reasonable.
	SCP_vector<int> stack;
	stack.push_back(root);
	while (!stack.empty()) {
		int idx = stack.back();
		stack.pop_back();
		if (idx < 0 || idx >= pm->n_models) {
			continue;
		}

		getOrBuildSubmodelBlas(pm_id, idx);

		const bsp_info& sm = pm->submodel[idx];
		for (int child = sm.first_child; child >= 0; child = pm->submodel[child].next_sibling) {
			stack.push_back(child);
		}
	}
}

void VulkanRaytracingManager::onModelUnloaded(int pm_id)
{
	if (!m_enabled) {
		return;
	}

	for (auto it = m_blasCache.begin(); it != m_blasCache.end();) {
		if (static_cast<int>(it->first >> 32) == pm_id) {
			destroyBlasEntry(it->second);
			it = m_blasCache.erase(it);
		} else {
			++it;
		}
	}
}

} // namespace graphics::vulkan
