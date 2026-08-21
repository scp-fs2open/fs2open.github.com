#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "VulkanConstants.h"

#include <array>
#include <vulkan/vulkan.hpp>


namespace graphics::vulkan {

class VulkanBufferManager;
class VulkanTextureManager;
class VulkanRaytracingManager;

// ========== Descriptor Set Templates ==========

struct DescriptorBindingTemplate {
	uint32_t binding;
	vk::DescriptorType type;
	uint32_t count;                      // 1 for most, 16 for texture array
	vk::ShaderStageFlags stages;
	vk::ImageViewType viewType;          // only meaningful for eCombinedImageSampler

	constexpr DescriptorBindingTemplate(uint32_t binding_, vk::DescriptorType type_,
	                                     uint32_t count_, vk::ShaderStageFlags stages_,
	                                     vk::ImageViewType viewType_ = vk::ImageViewType::e2D)
		: binding(binding_), type(type_), count(count_), stages(stages_), viewType(viewType_) {}
};

struct DescriptorSetTemplate : ArrayView<DescriptorBindingTemplate> {
	using ArrayView::ArrayView;
};

struct DescriptorFallbacks {
	vk::DescriptorBufferInfo buffer;
	vk::DescriptorImageInfo texture2D;
	vk::DescriptorImageInfo texture2DArray;
	vk::DescriptorImageInfo textureCube;
	vk::DescriptorImageInfo texture3D;

	// Unlike the fields above (fixed dummy defaults, set once by buildFallbacks()),
	// this is refreshed every frame by VulkanDescriptorManager::setCurrentShadowTlas()
	// to the live shadow TLAS -- or the permanent 0-instance fallback before the
	// first build / when raytraced shadows are disabled. Only valid/used when the
	// Global set's template actually declares GlobalBinding::Tlas. writeSet()'s
	// generic per-binding prefill is what makes every Global-set write pick this
	// up automatically, so callers never need to bind it explicitly.
	vk::AccelerationStructureKHR shadowTlas;

	const vk::DescriptorImageInfo& getImage(vk::ImageViewType t) const;
};

/**
 * @brief Descriptor set indices for the 3-tier layout
 *
 * Set 0: Global - per-frame data (lights, deferred globals, shadow maps)
 * Set 1: Material - per-material data (model data, textures)
 * Set 2: Per-Draw - per-draw-call data (generic data, matrices, etc.)
 */
enum class DescriptorSetIndex : uint32_t {
	Global = 0,
	Material = 1,
	PerDraw = 2,

	Count = 3
};

// ========== Descriptor Binding Constants ==========

// Global Set (Set 0) bindings — per-frame data
namespace GlobalBinding {
	static constexpr uint32_t Lights       = 0; // UBO: light data
	static constexpr uint32_t DeferredData = 1; // UBO: deferred globals
	static constexpr uint32_t ShadowMap    = 2; // sampler2DArrayShadow: cascaded shadow map (depth-compare)
	static constexpr uint32_t EnvMap       = 3; // samplerCube: environment map
	static constexpr uint32_t IrradianceMap = 4; // samplerCube: irradiance map
	static constexpr uint32_t Tlas         = 5; // accelerationStructureEXT: raytraced shadow TLAS (only present when raytraced shadows are supported)
	static constexpr uint32_t ShadowCascadeParams = 6; // UBO: shadow cascade projection matrices/distances (per-frame, shared by all consumers)
}

// Material Set (Set 1) bindings — per-material data
// ModelData and ShadowMapData are *dynamic* UBOs: their offset moves every draw
// while the rest of the set is unchanged, so keeping the offset out of the
// descriptor is what makes the set cacheable across draws.
namespace MaterialBinding {
static constexpr uint32_t ModelData = 0;     // dynamic UBO: model/material data
static constexpr uint32_t TextureArray = 1;  // sampler2D[16]: material textures
static constexpr uint32_t DecalGlobals = 2;  // UBO: decal globals
static constexpr uint32_t TransformSSBO = 3; // SSBO: batched transforms
static constexpr uint32_t DepthMap = 4;      // sampler2D: depth (soft particles)
static constexpr uint32_t SceneColor = 5;    // sampler2D: scene color (distortion)
static constexpr uint32_t DistortionMap = 6; // sampler2D: distortion texture
static constexpr uint32_t ShadowMapData = 7; // dynamic UBO: shadow map generation per-draw data (shadow_render_list)
}

// Texture array slot indices (elements within MaterialBinding::TextureArray)
namespace TextureSlot {
	static constexpr uint32_t BaseMap    = 0;
	static constexpr uint32_t GlowMap   = 1;
	static constexpr uint32_t SpecMap   = 2;
	static constexpr uint32_t NormalMap = 3;
	static constexpr uint32_t HeightMap = 4;
	static constexpr uint32_t AmbientMap = 5;
	static constexpr uint32_t MiscMap   = 6;
}

// PerDraw Set (Set 2) bindings — per-draw-call data
// GenericData and Matrices are *dynamic* UBOs, for the same reason as the
// Material set's ModelData.
namespace PerDrawBinding {
static constexpr uint32_t GenericData = 0; // dynamic UBO: generic shader data
static constexpr uint32_t Matrices = 1;    // dynamic UBO: transform matrices
static constexpr uint32_t NanoVGData = 2;  // UBO: NanoVG data
static constexpr uint32_t DecalInfo = 3;   // UBO: per-decal info
static constexpr uint32_t MovieData = 4;   // UBO: movie playback data
}

// ========== Set layout templates ==========
//
// These describe the fixed set layouts. Everything downstream that needs to know
// which bindings are dynamic, how many there are, or which dynamic-offset slot a
// given binding occupies is derived from them below rather than restated, so the
// declarations here are the only place that ordering exists.

inline constexpr DescriptorBindingTemplate MaterialSetBindings[] = {
	{MaterialBinding::ModelData,
		vk::DescriptorType::eUniformBufferDynamic,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::TextureArray,
		vk::DescriptorType::eCombinedImageSampler,
		16,
		vk::ShaderStageFlagBits::eFragment,
		vk::ImageViewType::e2DArray},
	{MaterialBinding::DecalGlobals,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::TransformSSBO, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex},
	{MaterialBinding::DepthMap, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::SceneColor, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::DistortionMap, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::ShadowMapData, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex},
};

inline constexpr DescriptorBindingTemplate PerDrawSetBindings[] = {
	{PerDrawBinding::GenericData,
		vk::DescriptorType::eUniformBufferDynamic,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{PerDrawBinding::Matrices,
		vk::DescriptorType::eUniformBufferDynamic,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{PerDrawBinding::NanoVGData,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{PerDrawBinding::DecalInfo,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{PerDrawBinding::MovieData, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
};

// ========== Dynamic-offset layout, derived from the templates above ==========
//
// vkCmdBindDescriptorSets consumes a set's dynamic offsets in ascending binding
// order, one entry per eUniformBufferDynamic descriptor. Both the count and the
// slot each binding occupies are computed from the set templates so a call site
// can never disagree with the layout it is binding against — reordering a template
// moves the derived slots with it instead of silently feeding a shader the wrong
// uniforms.

template <size_t N>
constexpr uint32_t dynamic_offset_count(const DescriptorBindingTemplate (&bindings)[N])
{
	uint32_t count = 0;
	for (size_t i = 0; i < N; ++i) {
		if (bindings[i].type == vk::DescriptorType::eUniformBufferDynamic) {
			count += bindings[i].count;
		}
	}
	return count;
}

// Returned by dynamic_offset_slot() when the binding is not a dynamic descriptor of the set.
constexpr uint32_t DYNAMIC_SLOT_INVALID = ~0u;

/**
 * @brief Position of a dynamic binding within its set's dynamic-offset array
 *
 * If @c binding is not a dynamic binding of the given set, @c DYNAMIC_SLOT_INVALID is returned.
 * Every use below pairs its constant with a static_assert against that sentinel, so a mistyped or
 * non-dynamic binding constant is a compile error rather than a silent 0 — these constants index
 * fixed-size dynamic-offset arrays at the bind call sites, so a leaked sentinel would be an
 * out-of-bounds write.
 *
 * Do not "clean this up" back into a `throw` on the not-found path: MSVC rejects a constexpr
 * function whose flow analysis can reach a throw (C3615), even when no evaluation takes that path.
 */
template <size_t N>
constexpr uint32_t dynamic_offset_slot(const DescriptorBindingTemplate (&bindings)[N], uint32_t binding)
{
	uint32_t slot = 0;
	for (size_t i = 0; i < N; ++i) {
		if (bindings[i].type != vk::DescriptorType::eUniformBufferDynamic) {
			continue;
		}
		if (bindings[i].binding == binding) {
			return slot;
		}
		slot += bindings[i].count;
	}
	return DYNAMIC_SLOT_INVALID;
}

// The Global set declares no dynamic bindings; it contributes nothing to a bind call.
constexpr uint32_t GLOBAL_DYNAMIC_OFFSET_COUNT = 0;
constexpr uint32_t MATERIAL_DYNAMIC_OFFSET_COUNT = dynamic_offset_count(MaterialSetBindings);
constexpr uint32_t PERDRAW_DYNAMIC_OFFSET_COUNT = dynamic_offset_count(PerDrawSetBindings);

namespace MaterialDynamicSlot {
constexpr uint32_t ModelData = dynamic_offset_slot(MaterialSetBindings, MaterialBinding::ModelData);
constexpr uint32_t ShadowMapData = dynamic_offset_slot(MaterialSetBindings, MaterialBinding::ShadowMapData);
static_assert(ModelData != DYNAMIC_SLOT_INVALID,
	"MaterialBinding::ModelData is not a dynamic descriptor of the Material set.");
static_assert(ShadowMapData != DYNAMIC_SLOT_INVALID,
	"MaterialBinding::ShadowMapData is not a dynamic descriptor of the Material set.");
}

namespace PerDrawDynamicSlot {
constexpr uint32_t GenericData = dynamic_offset_slot(PerDrawSetBindings, PerDrawBinding::GenericData);
constexpr uint32_t Matrices = dynamic_offset_slot(PerDrawSetBindings, PerDrawBinding::Matrices);
static_assert(GenericData != DYNAMIC_SLOT_INVALID,
	"PerDrawBinding::GenericData is not a dynamic descriptor of the PerDraw set.");
static_assert(Matrices != DYNAMIC_SLOT_INVALID,
	"PerDrawBinding::Matrices is not a dynamic descriptor of the PerDraw set.");
}

/**
 * @brief Stack-allocated batch writer for descriptor set updates.
 *
 * Usage: reset() + writeSet() (pre-fills all bindings with fallbacks)
 * + setBuffer/setImage overrides for real data + flush().
 *
 * Bindings declared as eUniformBufferDynamic get their offset split out of the
 * descriptor and into the dynamic-offset array (see dynamicOffsets()): setBuffer
 * writes {buffer, 0, range} into the descriptor and stashes the caller's offset.
 * That is what lets a set whose only per-draw change is a UBO offset be written
 * once and rebound many times. Callers must hand dynamicOffsets()/
 * VulkanDescriptorManager::getDynamicOffsetCount() for a set to
 * vkCmdBindDescriptorSets when binding it.
 */
class DescriptorWriter {
public:
	// Fixed capacities for the stack-allocated staging arrays below — NOT Vulkan
	// hardware limits. They bound a single vkUpdateDescriptorSets batch (one
	// writeSet), sized to comfortably cover the largest descriptor set layout the
	// engine builds. Because the backing arrays live on the stack (see m_writes
	// etc.), the numbers carry no runtime cost beyond that fixed footprint; the
	// only failure mode is under-sizing, which is Assert-guarded at fill time in
	// DescriptorWriter::writeSet (e.g. `Assert(m_writeCount < MAX_WRITES)`) so an
	// oversized layout trips loudly in debug rather than silently overflowing.
	// Raise these if a future set layout legitimately needs more bindings/infos.
	static constexpr uint32_t MAX_WRITES = 32;           // distinct bindings written per batch
	static constexpr uint32_t MAX_BUFFER_INFOS = 20;     // buffer descriptors staged per batch
	static constexpr uint32_t MAX_IMAGE_INFOS = 24;      // image/sampler descriptors staged per batch
	static constexpr uint32_t MAX_ACCEL_STRUCT_INFOS = 2; // TLAS descriptors staged per batch (RT)
	static constexpr uint32_t MAX_BINDINGS_PER_SET = 16; // highest binding number addressable in a set
	// Most dynamic (eUniformBufferDynamic) bindings any one set layout declares. Derived from
	// the templates rather than counted by hand, so adding a dynamic binding resizes the
	// storage that holds its offset automatically.
	static constexpr uint32_t MAX_DYNAMIC_OFFSETS_PER_SET =
		MATERIAL_DYNAMIC_OFFSET_COUNT > PERDRAW_DYNAMIC_OFFSET_COUNT ? MATERIAL_DYNAMIC_OFFSET_COUNT
																	 : PERDRAW_DYNAMIC_OFFSET_COUNT;

	void reset(vk::Device device, const DescriptorFallbacks& fallbacks) {
		m_device = device;
		m_fallbacks = &fallbacks;
		m_writeCount = 0;
		m_bufferInfoCount = 0;
		m_imageInfoCount = 0;
		m_accelStructInfoCount = 0;
		m_dynOffsets = {};
	}

	void writeSet(DescriptorSetIndex setIndex, vk::DescriptorSet set);

	void setBuffer(uint32_t binding, const vk::DescriptorBufferInfo& info);
	void setImage(uint32_t binding, const vk::DescriptorImageInfo& info);
	void setImageArray(uint32_t binding, ArrayView<vk::DescriptorImageInfo> infos);

	/**
	 * @brief Dynamic offsets accumulated for a single set, ordered by binding number.
	 *
	 * Only meaningful for a set this writer actually wrote; a set reused from a
	 * memoization cache was not written here, so its caller owns the offsets.
	 * Always sized to the set layout's dynamic descriptor count (offsets for
	 * bindings left at their fallback stay 0). Points at stable per-set storage —
	 * valid until the next writeSet() of that set.
	 */
	const uint32_t* dynamicOffsets(DescriptorSetIndex setIndex) const
	{
		return m_dynOffsets[static_cast<size_t>(setIndex)].data();
	}

	/**
	 * @brief Binds a contiguous run of sets in one vkCmdBindDescriptorSets call
	 *
	 * Concatenates the run's dynamic offsets in set order, which is the layout
	 * vkCmdBindDescriptorSets expects: each set contributes exactly its layout's
	 * dynamic descriptor count (a set declaring none contributes nothing, rather
	 * than padding). @c sets must list the sets for @c firstSet onwards, in order.
	 *
	 * Binding here rather than handing the offsets back to the caller keeps the
	 * concatenation buffer's lifetime contained: it is writer-owned scratch, reused
	 * by the next call, and never outlives the command it was built for.
	 */
	void bindSets(vk::CommandBuffer cmd,
		vk::PipelineLayout layout,
		DescriptorSetIndex firstSet,
		ArrayView<vk::DescriptorSet> sets);

	void flush(); // defined in VulkanDescriptorManager.cpp (reports write stats to the manager)

private:
	// Per-binding lookup for the current writeSet, indexed by binding number.
	// Populated by writeSet, used by setBuffer/setImage/setImageArray for O(1) access.
	struct BindingSlot {
		vk::DescriptorBufferInfo* bufferInfo = nullptr;  // non-null for buffer bindings
		vk::DescriptorImageInfo* imageInfo = nullptr;    // non-null for image bindings
		uint32_t count = 0;                              // descriptor count (1 or 16 for arrays)
		vk::ImageViewType viewType = vk::ImageViewType::e2D;  // for fallback lookup
		int dynIndex = -1;                                    // slot in m_dynOffsets, or -1 if not dynamic
	};

	vk::Device m_device;
	const DescriptorFallbacks* m_fallbacks = nullptr;

	std::array<vk::WriteDescriptorSet, MAX_WRITES> m_writes;
	std::array<vk::DescriptorBufferInfo, MAX_BUFFER_INFOS> m_bufferInfos;
	std::array<vk::DescriptorImageInfo, MAX_IMAGE_INFOS> m_imageInfos;
	std::array<vk::AccelerationStructureKHR, MAX_ACCEL_STRUCT_INFOS> m_accelStructInfos;
	std::array<vk::WriteDescriptorSetAccelerationStructureKHR, MAX_ACCEL_STRUCT_INFOS> m_asWriteInfos;
	std::array<BindingSlot, MAX_BINDINGS_PER_SET> m_bindingSlots;
	std::array<std::array<uint32_t, MAX_DYNAMIC_OFFSETS_PER_SET>, static_cast<size_t>(DescriptorSetIndex::Count)>
		m_dynOffsets{};
	// Scratch used by bindSets() to concatenate a run of sets' offsets.
	std::array<uint32_t, static_cast<size_t>(DescriptorSetIndex::Count) * MAX_DYNAMIC_OFFSETS_PER_SET>
		m_dynOffsetScratch{};
	DescriptorSetIndex m_currentSet = DescriptorSetIndex::Global;
	uint32_t m_writeCount = 0;
	uint32_t m_bufferInfoCount = 0;
	uint32_t m_imageInfoCount = 0;
	uint32_t m_accelStructInfoCount = 0;
};

/**
 * @brief Manages Vulkan descriptor sets, pools, and layouts
 *
 * Provides descriptor set allocation and update functionality.
 * Uses per-frame pools for transient descriptors.
 */
class VulkanDescriptorManager {
public:
	static constexpr uint32_t MAX_TEXTURE_BINDINGS = 16;  // Texture array size

	// Per-frame descriptor pool sizing. These are not hard limits: createFramePool()
	// allocates one pool chunk of this size, and additional chunks are created
	// automatically when a chunk is exhausted (see allocateSet's OutOfPoolMemory /
	// FragmentedPool fallthrough). The number is a growth-granularity tradeoff --
	// larger chunks waste memory, smaller chunks allocate pools more often.
	// MAX_SETS_PER_POOL supports ~330 draw calls (3 sets each) per chunk; the per-
	// type multipliers cover the worst-case bindings a single set can request.
	// One chunk now covers a dense scene outright: with the dynamic-UBO descriptor
	// memoization in applyMaterial/renderShadowDraw, a big asteroid field at ~1350
	// model draws settles at ~250 sets/frame, so growth is back to being the rare
	// case it was meant to be. (The same scene needed ~2500 sets/frame before that.)
	// Deliberately left at the original size rather than raised: the per-frame
	// growth this used to log was a symptom of the churn, not of undersizing, and
	// FSO targets low-end GPUs where a larger pool is real wasted memory.
	static constexpr uint32_t MAX_SETS_PER_POOL = 1024;
	// Worst case per set is 3 static UBOs (Global, PerDraw) and 2 dynamic (Material:
	// ModelData + ShadowMapData; PerDraw: GenericData + Matrices). Total UBO
	// capacity per chunk is slightly below what this was before the dynamic split.
	static constexpr uint32_t MAX_UNIFORM_BUFFERS_PER_POOL = MAX_SETS_PER_POOL * 5;         // static UBOs
	static constexpr uint32_t MAX_DYNAMIC_UNIFORM_BUFFERS_PER_POOL = MAX_SETS_PER_POOL * 2; // dynamic UBOs
	static constexpr uint32_t MAX_SAMPLERS_PER_POOL = MAX_SETS_PER_POOL * 16;         // up to 16 samplers per material set

	VulkanDescriptorManager() = default;
	~VulkanDescriptorManager() = default;

	// Non-copyable
	VulkanDescriptorManager(const VulkanDescriptorManager&) = delete;
	VulkanDescriptorManager& operator=(const VulkanDescriptorManager&) = delete;

	/**
	 * @brief Initialize descriptor manager
	 * @param device Vulkan logical device
	 * @param raytracingSupported Whether the Global set's layout should include
	 *        the acceleration-structure (TLAS) binding. Must reflect whether the
	 *        raytracing manager actually finished initializing, not just whether
	 *        the device extensions were negotiated -- a device without
	 *        VK_KHR_acceleration_structure enabled cannot create a descriptor set
	 *        layout containing an eAccelerationStructureKHR binding at all.
	 * @return true on success
	 */
	bool init(vk::Device device, bool raytracingSupported);

	/**
	 * @brief Shutdown and release resources
	 */
	void shutdown();

	/**
	 * @brief Build fallback descriptor values from buffer/texture/raytracing managers.
	 * Must be called after those managers are initialized. rtMgr may be null
	 * if raytraced shadows are unavailable (its fallback is unused in that case).
	 */
	void buildFallbacks(VulkanBufferManager* bufMgr, VulkanTextureManager* texMgr, VulkanRaytracingManager* rtMgr);

	/**
	 * @brief Get the fallback descriptor values
	 */
	const DescriptorFallbacks& getFallbacks() const { return m_fallbacks; }

	/**
	 * @brief Refresh the TLAS every Global-set write picks up automatically.
	 * Called once per frame (after VulkanRaytracingManager::buildTlas()) with
	 * that manager's getTlasForShaderBinding() -- a no-op if raytraced shadows
	 * aren't enabled, since the Global template has no Tlas binding to write in
	 * that case. See DescriptorFallbacks::shadowTlas.
	 */
	void setCurrentShadowTlas(vk::AccelerationStructureKHR tlas) {
		if (m_raytracingEnabled) {
			m_fallbacks.shadowTlas = tlas;
		}
	}

	/**
	 * @brief Get the set template for a given set index
	 */
	static const DescriptorSetTemplate& getSetTemplate(DescriptorSetIndex setIndex);

	/**
	 * @brief Number of dynamic (eUniformBufferDynamic) descriptors in a set layout
	 *
	 * vkCmdBindDescriptorSets requires exactly this many entries in pDynamicOffsets,
	 * so every bind site must agree with the layout. VulkanStateTracker asserts on
	 * it; raw cmd.bindDescriptorSets() callers should pass the matching count.
	 */
	static uint32_t getDynamicOffsetCount(DescriptorSetIndex setIndex);

	/**
	 * @brief Get descriptor set layout for a given set index
	 */
	vk::DescriptorSetLayout getSetLayout(DescriptorSetIndex setIndex) const;

	/**
	 * @brief Get all descriptor set layouts (for pipeline layout creation)
	 * @return Reference to the UniqueDescriptorSetLayout array (Global, Material, PerDraw)
	 */
	const auto& getAllSetLayouts() const { return m_setLayouts; }

	/**
	 * @brief Allocate a descriptor set from the per-frame pool
	 * @param setIndex Which set type to allocate
	 * @return Allocated descriptor set, or null handle on failure
	 */
	vk::DescriptorSet allocateFrameSet(DescriptorSetIndex setIndex);

	/**
	 * @brief Set the current frame-in-flight index (single source: VulkanRenderer)
	 *
	 * Replaces the manager's old self-advancing counter. The renderer advances its
	 * one frame index in flip() and pushes it here (alongside the buffer manager),
	 * so every consumer of getCurrentFrame() agrees by construction rather than by
	 * coincidence. Must be called exactly once per frame.
	 */
	void setCurrentFrame(uint32_t frameIndex);

	/**
	 * @brief Begin a new frame - reset current frame's pool
	 */
	void beginFrame();

	/**
	 * @brief Get current frame index
	 */
	uint32_t getCurrentFrame() const { return m_currentFrame; }

	// ========== Per-frame diagnostics (reset in beginFrame) ==========
	void noteDescriptorWrites(uint32_t count) { m_writesThisFrame += count; }
	uint32_t getSetsAllocatedThisFrame() const { return m_setsAllocatedThisFrame; }
	uint32_t getWritesThisFrame() const { return m_writesThisFrame; }

	/**
	 * @brief Get the Vulkan device (for DescriptorWriter)
	 */
	vk::Device getDevice() const { return m_device; }

	/**
	 * @brief Entry mapping a UBO binding to its uniform_block_type
	 */
	struct UniformBindingEntry {
		uint32_t binding;
		uniform_block_type blockType;
	};

	/**
	 * @brief Get the UBO bindings for a given descriptor set
	 */
	static ArrayView<UniformBindingEntry> getUniformBindings(DescriptorSetIndex setIndex);

private:
	/**
	 * @brief Create all descriptor set layouts
	 */
	void createSetLayouts();

	/**
	 * @brief Create descriptor pools
	 */
	void createDescriptorPools();

	/**
	 * @brief Create a single descriptor set layout
	 */
	vk::UniqueDescriptorSetLayout createSetLayout(const DescriptorSetTemplate& tmpl);

	/**
	 * @brief Create a new descriptor pool with standard sizes
	 */
	vk::UniqueDescriptorPool createFramePool();

	vk::Device m_device;

	// Descriptor set layouts (one per set type)
	std::array<vk::UniqueDescriptorSetLayout, static_cast<size_t>(DescriptorSetIndex::Count)> m_setLayouts;

	// Per-frame descriptor pools (growable - new pools added on demand)
	std::array<SCP_vector<vk::UniqueDescriptorPool>, MAX_FRAMES_IN_FLIGHT> m_framePools;

	// Pre-built fallback descriptor values
	DescriptorFallbacks m_fallbacks{};

	// The Global set's binding list, unlike the other sets', is assembled at runtime
	// in createSetLayouts() (see the comment there) rather than compiled in as a
	// fixed constexpr array, so it needs real per-instance storage instead of a
	// constexpr template. getSetTemplate() stays a static method (all ~30 call
	// sites across the renderer use it that way) by reaching these through the
	// singleton (getDescriptorManager()) for the Global case only.
	SCP_vector<DescriptorBindingTemplate> m_globalBindingsRuntime;
	DescriptorSetTemplate m_globalTemplateRuntime;

	uint32_t m_currentFrame = 0;
	bool m_initialized = false;
	bool m_raytracingEnabled = false;

	// Per-frame diagnostic counters (see VulkanDrawManager::printFrameStats)
	uint32_t m_setsAllocatedThisFrame = 0;
	uint32_t m_writesThisFrame = 0;
};

// Global descriptor manager access
VulkanDescriptorManager* getDescriptorManager();
void setDescriptorManager(VulkanDescriptorManager* manager);

} // namespace graphics::vulkan

