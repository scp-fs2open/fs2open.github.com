#pragma once

#include "VulkanMemory.h"
#include "VulkanBuffer.h"

#include <vulkan/vulkan.hpp>

// Forward declarations to avoid pulling model/model.h into every Vulkan
// translation unit that includes this header.
class polymodel;
struct polymodel_instance;
class transform_stack;
struct matrix;
struct vec3d;

namespace graphics::vulkan {

// Converts an FS2Open world orientation + position into Vulkan's row-major 3x4
// local-to-world VkTransformMatrixKHR used for AS instance placement. Exposed
// (defined in VulkanRaytracing.cpp) so it can be unit tested directly -- see
// test/src/graphics/vulkan/test_vulkan_raytracing.cpp.
vk::TransformMatrixKHR toVkTransform(const matrix& orient, const vec3d& pos);

/**
 * @brief Manages bottom-level acceleration structures (BLAS) for raytraced shadows.
 *
 * One BLAS is built per unique (polymodel id, submodel index) pair, keyed off the
 * submodel's own rasterization geometry buffers (not the merged per-LOD detail
 * buffers, which have no BLAS equivalent since they rely on a shader-side transform
 * lookup to let independently-articulating submodels share one draw call).
 *
 * BLASes are built lazily via getOrBuildSubmodelBlas() and cached for the model's
 * lifetime. A cached BLAS is checked against the current generation of its source
 * vertex/index buffers on every lookup (see VulkanBufferManager::getBufferGeneration)
 * and rebuilt if the backing GPU heap was resized since the BLAS was built.
 *
 * All work here is a no-op when raytraced shadows are not supported/enabled --
 * callers do not need to guard calls with a capability check themselves.
 */
class VulkanRaytracingManager {
public:
	VulkanRaytracingManager();
	~VulkanRaytracingManager();

	VulkanRaytracingManager(const VulkanRaytracingManager&) = delete;
	VulkanRaytracingManager& operator=(const VulkanRaytracingManager&) = delete;

	/**
	 * @brief Initialize the manager
	 * @param enabled Whether raytraced shadows were negotiated as supported
	 *        (VulkanRenderer::supportsRaytracedShadows()). If false, every other
	 *        method on this class becomes a no-op.
	 */
	bool init(vk::Device device,
		vk::PhysicalDevice physicalDevice,
		VulkanMemoryManager* memoryManager,
		VulkanBufferManager* bufferManager,
		vk::CommandPool commandPool,
		vk::Queue graphicsQueue,
		bool enabled);

	void shutdown();

	/**
	 * @brief Get or lazily (re)build the BLAS for one submodel.
	 * @param pm_id Polygon model index (polymodel::id)
	 * @param submodel_num Submodel index within the model (index into polymodel::submodel)
	 * @return The BLAS handle, or VK_NULL_HANDLE if the submodel has no renderable
	 *         geometry, the indices are invalid, the build failed, or raytraced
	 *         shadows are disabled.
	 */
	vk::AccelerationStructureKHR getOrBuildSubmodelBlas(int pm_id, int submodel_num);

	/**
	 * @brief Eagerly warm the BLAS cache for a model's LOD0 submodel tree.
	 *
	 * Called after a model finishes loading (its GPU vertex/index buffers are
	 * already uploaded at that point -- see model_interp_submit_buffers). No-op
	 * if raytraced shadows are disabled.
	 */
	void onModelLoaded(int pm_id);

	/**
	 * @brief Purge all cached BLASes for a model and free their GPU resources.
	 * Called from model_unload()/model_free_all() before the model is destroyed.
	 */
	void onModelUnloaded(int pm_id);

	/**
	 * @brief Rebuild the top-level acceleration structure for this frame from
	 * the current set of shadow-casting objects (ships/asteroids/debris),
	 * mirroring the object selection in shadows_render_all() (minus the
	 * per-cascade frustum pre-filter, which lives in shadows.cpp's private
	 * state -- see .cpp for rationale). Recorded into the *current* frame's
	 * command buffer (via VulkanStateTracker) with a barrier before future
	 * shader reads, so this does not stall the GPU/CPU on the common path.
	 * No-op if raytraced shadows are disabled.
	 *
	 * Known simplifications vs. the rasterized shadow pass, both affecting a
	 * small minority of models and not correctness-critical for occlusion:
	 * no per-submodel detail-box culling (depends on legacy 3D-pipeline
	 * global state not reproduced standalone) and no model autocentering
	 * offset.
	 */
	void buildTlas();

	/**
	 * @brief Whether raytraced shadows were negotiated as supported AND this
	 * manager finished initializing successfully (including its fallback TLAS).
	 */
	bool isEnabled() const { return m_enabled; }

	/**
	 * @brief A permanent, 0-instance TLAS built once at init(). Used as a
	 * valid (non-null) descriptor value in place of getTlasForShaderBinding()
	 * whenever no real TLAS has been built yet -- VK_NULL_HANDLE is not a legal
	 * acceleration-structure descriptor value without the nullDescriptor
	 * feature, which this renderer does not negotiate.
	 */
	vk::AccelerationStructureKHR getFallbackTlas() const { return m_fallbackTlas ? m_fallbackTlas.get() : vk::AccelerationStructureKHR{}; }

	/**
	 * @brief The TLAS to bind into the Global descriptor set this frame:
	 * the current real TLAS if one has been built, otherwise the fallback.
	 */
	vk::AccelerationStructureKHR getTlasForShaderBinding() const {
		return m_tlas ? m_tlas.get() : getFallbackTlas();
	}

private:
	// One-time build of the 0-instance fallback TLAS (see getFallbackTlas()).
	// Called from init(); on failure, raytraced shadows are disabled for the
	// session (m_enabled is cleared) since the descriptor-writing path
	// requires a valid fallback whenever the Global set declares the binding.
	bool buildFallbackTlas();

	struct BlasEntry {
		vk::UniqueAccelerationStructureKHR accelStruct;
		vk::Buffer buffer = nullptr;
		VulkanAllocation allocation;
		vk::DeviceAddress address = 0;
		uint64_t vertexBufferGeneration = 0;
		uint64_t indexBufferGeneration = 0;
	};

	static uint64_t makeKey(int pm_id, int submodel_num);

	// Rounds a device address up to `alignment` (used for scratch-buffer offsets,
	// which must satisfy minAccelerationStructureScratchOffsetAlignment). A
	// private static method (not an anonymous-namespace free function) since it's
	// called from both VulkanRaytracing.cpp and VulkanRaytracingTlas.cpp.
	static vk::DeviceAddress alignUp(vk::DeviceAddress value, uint32_t alignment);

	// Builds (or rebuilds) the BLAS for one submodel into `entry`. Returns false
	// (leaving `entry` untouched) if the submodel has no renderable geometry.
	bool buildBlas(BlasEntry& entry, int pm_id, int submodel_num);

	// Shared cache lookup/build used by both getOrBuildSubmodelBlas() and the
	// TLAS instance walk (which also needs the BLAS device address, not just
	// the handle). Returns nullptr if the submodel has no renderable geometry.
	const BlasEntry* getOrBuildBlasEntry(int pm_id, int submodel_num);

	void destroyBlasEntry(BlasEntry& entry);

	bool createRawBuffer(vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		MemoryUsage memUsage,
		vk::Buffer& outBuffer,
		VulkanAllocation& outAllocation);

	vk::CommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

	// Shared by buildBlas()/buildFallbackTlas(): allocates a fresh backing buffer
	// and scratch buffer sized from `sizeInfo`, creates the AS object, records the
	// build via a synchronous one-shot command buffer, and frees the scratch
	// buffer afterward. `buildInfo` must already have geometryCount/pGeometries/
	// flags/mode set; dstAccelerationStructure/scratchData are filled in here.
	// On failure, outAS/outBuffer/outAllocation are left untouched (nothing to
	// clean up on the caller's side). Does NOT query the resulting AS's device
	// address -- callers that need it (buildBlas) do that themselves afterward.
	bool buildAccelerationStructureOneShot(vk::AccelerationStructureTypeKHR type,
		vk::AccelerationStructureBuildGeometryInfoKHR buildInfo,
		const vk::AccelerationStructureBuildSizesInfoKHR& sizeInfo,
		const vk::AccelerationStructureBuildRangeInfoKHR* const* pBuildRanges,
		vk::UniqueAccelerationStructureKHR& outAS,
		vk::Buffer& outBuffer,
		VulkanAllocation& outAllocation);

	// ---- TLAS (per-frame) ----

	// Shared by walkSubmodelTree/addSingleSubmodelInstance: appends one TLAS
	// instance referencing blasAddress, placed at the given world orient/pos.
	static void pushInstance(SCP_vector<vk::AccelerationStructureInstanceKHR>& instances,
		vk::DeviceAddress blasAddress,
		const matrix& orient,
		const vec3d& pos);

	void gatherShadowCasterInstances(SCP_vector<vk::AccelerationStructureInstanceKHR>& instances);
	void walkSubmodelTree(SCP_vector<vk::AccelerationStructureInstanceKHR>& instances,
		transform_stack& stack,
		const polymodel* pm,
		const polymodel_instance* pmi,
		int submodel_num);
	void addSingleSubmodelInstance(SCP_vector<vk::AccelerationStructureInstanceKHR>& instances,
		const polymodel* pm,
		const polymodel_instance* pmi,
		int submodel_num,
		const matrix& orient,
		const vec3d& pos);

	// Grow-only: (re)allocates the buffer if `requiredBytes` exceeds current
	// capacity. Returns false on allocation failure. A regrow stalls the
	// queue (safe but rare -- only happens while capacity ramps up to a
	// mission's peak instance/AS-size high-water mark).
	bool ensureInstanceCapacity(vk::DeviceSize requiredBytes);
	bool ensureTlasCapacity(vk::DeviceSize requiredBytes);
	bool ensureScratchCapacity(vk::DeviceSize requiredBytes);

	vk::Device m_device;
	vk::PhysicalDevice m_physicalDevice;
	VulkanMemoryManager* m_memoryManager = nullptr;
	VulkanBufferManager* m_bufferManager = nullptr;
	vk::CommandPool m_commandPool;
	vk::Queue m_graphicsQueue;

	uint32_t m_scratchAlignment = 1;

	bool m_enabled = false;
	bool m_initialized = false;

	SCP_unordered_map<uint64_t, BlasEntry> m_blasCache;

	// Persistent, grow-only per-frame TLAS resources.
	vk::Buffer m_instanceBuffer;
	VulkanAllocation m_instanceAllocation;
	void* m_instanceMapped = nullptr;
	vk::DeviceSize m_instanceCapacity = 0;

	vk::UniqueAccelerationStructureKHR m_tlas;
	vk::Buffer m_tlasBuffer;
	VulkanAllocation m_tlasAllocation;
	vk::DeviceSize m_tlasCapacity = 0;

	vk::Buffer m_tlasScratchBuffer;
	VulkanAllocation m_tlasScratchAllocation;
	vk::DeviceSize m_tlasScratchCapacity = 0;

	// Permanent, 0-instance fallback TLAS (see getFallbackTlas()).
	vk::UniqueAccelerationStructureKHR m_fallbackTlas;
	vk::Buffer m_fallbackTlasBuffer = nullptr;
	VulkanAllocation m_fallbackTlasAllocation;
};

// Global raytracing manager instance (set during renderer init)
VulkanRaytracingManager* getRaytracingManager();
void setRaytracingManager(VulkanRaytracingManager* manager);

} // namespace graphics::vulkan
