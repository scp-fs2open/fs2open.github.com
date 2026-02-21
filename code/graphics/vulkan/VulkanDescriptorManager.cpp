#include "VulkanDescriptorManager.h"

namespace graphics {
namespace vulkan {

// Global descriptor manager pointer
static VulkanDescriptorManager* g_descriptorManager = nullptr;

VulkanDescriptorManager* getDescriptorManager()
{
	Assertion(g_descriptorManager != nullptr, "Vulkan DescriptorManager not initialized!");
	return g_descriptorManager;
}

void setDescriptorManager(VulkanDescriptorManager* manager)
{
	g_descriptorManager = manager;
}

bool VulkanDescriptorManager::init(vk::Device device)
{
	if (m_initialized) {
		return true;
	}

	m_device = device;

	createSetLayouts();
	createDescriptorPools();

	m_initialized = true;
	mprintf(("VulkanDescriptorManager: Initialized\n"));
	return true;
}

void VulkanDescriptorManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Wait for device idle before destroying
	m_device.waitIdle();

	// Destroy pools (automatically frees allocated sets)
	for (auto& poolChain : m_framePools) {
		poolChain.clear();
	}

	// Destroy layouts
	for (auto& layout : m_setLayouts) {
		layout.reset();
	}

	m_initialized = false;
	mprintf(("VulkanDescriptorManager: Shutdown complete\n"));
}

vk::DescriptorSetLayout VulkanDescriptorManager::getSetLayout(DescriptorSetIndex setIndex) const
{
	return m_setLayouts[static_cast<size_t>(setIndex)].get();
}

SCP_vector<vk::DescriptorSetLayout> VulkanDescriptorManager::getAllSetLayouts() const
{
	SCP_vector<vk::DescriptorSetLayout> layouts;
	layouts.reserve(static_cast<size_t>(DescriptorSetIndex::Count));

	for (const auto& layout : m_setLayouts) {
		layouts.push_back(layout.get());
	}

	return layouts;
}

vk::DescriptorSet VulkanDescriptorManager::allocateFrameSet(DescriptorSetIndex setIndex)
{
	if (!m_initialized) {
		return {};
	}

	vk::DescriptorSetLayout layout = m_setLayouts[static_cast<size_t>(setIndex)].get();
	auto& pools = m_framePools[m_currentFrame];

	// Try allocating from the last pool in the list
	if (!pools.empty()) {
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = pools.back().get();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		try {
			auto sets = m_device.allocateDescriptorSets(allocInfo);
			return sets[0];
		} catch (const vk::OutOfPoolMemoryError&) {
			// Pool exhausted, fall through to create a new one
		} catch (const vk::FragmentedPoolError&) {
			// Pool fragmented, fall through to create a new one
		}
	}

	// Create a new pool and retry
	pools.push_back(createFramePool());
	mprintf(("VulkanDescriptorManager: Grew frame %u pool count to %zu\n",
		m_currentFrame, pools.size()));

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = pools.back().get();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	try {
		auto sets = m_device.allocateDescriptorSets(allocInfo);
		return sets[0];
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanDescriptorManager: Failed to allocate frame descriptor set after pool growth: %s\n", e.what()));
		return {};
	}
}

void VulkanDescriptorManager::beginFrame()
{
	if (!m_initialized) {
		return;
	}

	auto& pools = m_framePools[m_currentFrame];

	// Reset all pools for the current frame
	for (auto& pool : pools) {
		m_device.resetDescriptorPool(pool.get());
	}

	// If we grew beyond the initial pool, shrink back to 1 to reclaim memory
	// (the single pool will grow again next frame if needed)
	if (pools.size() > 1) {
		vk::UniqueDescriptorPool first = std::move(pools[0]);
		pools.clear();
		pools.push_back(std::move(first));
	}
}

void VulkanDescriptorManager::endFrame()
{
	// Advance to next frame
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool VulkanDescriptorManager::getUniformBlockBinding(uniform_block_type blockType,
                                                      DescriptorSetIndex& setIndex, uint32_t& binding)
{
	// Map uniform_block_type to descriptor set and binding
	// Based on the descriptor layout design in the plan
	switch (blockType) {
	case uniform_block_type::Lights:
		setIndex = DescriptorSetIndex::Global;
		binding = 0;
		return true;

	case uniform_block_type::DeferredGlobals:
		setIndex = DescriptorSetIndex::Global;
		binding = 1;
		return true;

	case uniform_block_type::ModelData:
		setIndex = DescriptorSetIndex::Material;
		binding = 0;
		return true;

	case uniform_block_type::DecalGlobals:
		setIndex = DescriptorSetIndex::Material;
		binding = 2;
		return true;

	case uniform_block_type::GenericData:
		setIndex = DescriptorSetIndex::PerDraw;
		binding = 0;
		return true;

	case uniform_block_type::Matrices:
		setIndex = DescriptorSetIndex::PerDraw;
		binding = 1;
		return true;

	case uniform_block_type::NanoVGData:
		setIndex = DescriptorSetIndex::PerDraw;
		binding = 2;
		return true;

	case uniform_block_type::DecalInfo:
		setIndex = DescriptorSetIndex::PerDraw;
		binding = 3;
		return true;

	case uniform_block_type::MovieData:
		setIndex = DescriptorSetIndex::PerDraw;
		binding = 4;
		return true;

	default:
		return false;
	}
}

void VulkanDescriptorManager::createSetLayouts()
{
	// Set 0: Global (per-frame data)
	// NOTE: Using regular UBOs for now; dynamic UBOs need offset tracking
	{
		SCP_vector<DescriptorBindingInfo> bindings = {
			// Binding 0: Lights UBO
			{ 0, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },

			// Binding 1: DeferredGlobals UBO
			{ 1, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },

			// Binding 2: Shadow map texture
			{ 2, vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },

			// Binding 3: Environment map (samplerCube)
			{ 3, vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },

			// Binding 4: Irradiance map (samplerCube)
			{ 4, vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },
		};
		m_setLayouts[static_cast<size_t>(DescriptorSetIndex::Global)] = createSetLayout(bindings);
	}

	// Set 1: Material (per-batch data)
	{
		SCP_vector<DescriptorBindingInfo> bindings = {
			// Binding 0: ModelData UBO
			{ 0, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },

			// Binding 1: Texture array (diffuse, glow, spec, normal, ambient, misc, etc.)
			{ 1, vk::DescriptorType::eCombinedImageSampler, MAX_TEXTURE_BINDINGS,
			  vk::ShaderStageFlagBits::eFragment },

			// Binding 2: DecalGlobals UBO
			{ 2, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },

			// Binding 3: Transform buffer SSBO (for batched submodel transforms)
			{ 3, vk::DescriptorType::eStorageBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex },

			// Binding 4: Depth map (sampler2D for soft particles)
			{ 4, vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },

			// Binding 5: Scene color / frameBuffer (distortion effects)
			{ 5, vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },

			// Binding 6: Distortion map (distortion effects)
			{ 6, vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },
		};
		m_setLayouts[static_cast<size_t>(DescriptorSetIndex::Material)] = createSetLayout(bindings);
	}

	// Set 2: Per-Draw (per-draw-call data)
	{
		SCP_vector<DescriptorBindingInfo> bindings = {
			// Binding 0: GenericData UBO
			{ 0, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },

			// Binding 1: Matrices UBO
			{ 1, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },

			// Binding 2: NanoVGData UBO
			{ 2, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },

			// Binding 3: DecalInfo UBO
			{ 3, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },

			// Binding 4: MovieData UBO
			{ 4, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eFragment },
		};
		m_setLayouts[static_cast<size_t>(DescriptorSetIndex::PerDraw)] = createSetLayout(bindings);
	}

	mprintf(("VulkanDescriptorManager: Created %zu descriptor set layouts\n",
		static_cast<size_t>(DescriptorSetIndex::Count)));
}

vk::UniqueDescriptorPool VulkanDescriptorManager::createFramePool()
{
	// Pool sizes per chunk - supports ~330 draw calls (3 sets each)
	// If more are needed, additional pools are created automatically
	constexpr uint32_t MAX_SETS_PER_POOL = 1024;
	constexpr uint32_t MAX_UNIFORM_BUFFERS = MAX_SETS_PER_POOL * 9;   // up to 9 UBOs per draw
	constexpr uint32_t MAX_SAMPLERS = MAX_SETS_PER_POOL * 16;         // up to 16 samplers per material set

	SCP_vector<vk::DescriptorPoolSize> poolSizes = {
		{ vk::DescriptorType::eUniformBuffer, MAX_UNIFORM_BUFFERS },
		{ vk::DescriptorType::eCombinedImageSampler, MAX_SAMPLERS },
		{ vk::DescriptorType::eStorageBuffer, MAX_SETS_PER_POOL },
	};

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.maxSets = MAX_SETS_PER_POOL;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	return m_device.createDescriptorPoolUnique(poolInfo);
}

void VulkanDescriptorManager::createDescriptorPools()
{
	// Create one initial pool per frame (more will be added on demand)
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		m_framePools[i].push_back(createFramePool());
	}

	mprintf(("VulkanDescriptorManager: Created %u frame pool chains\n",
		MAX_FRAMES_IN_FLIGHT));
}

vk::UniqueDescriptorSetLayout VulkanDescriptorManager::createSetLayout(
	const SCP_vector<DescriptorBindingInfo>& bindings)
{
	SCP_vector<vk::DescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(bindings.size());

	for (const auto& info : bindings) {
		vk::DescriptorSetLayoutBinding binding;
		binding.binding = info.binding;
		binding.descriptorType = info.type;
		binding.descriptorCount = info.count;
		binding.stageFlags = info.stages;
		binding.pImmutableSamplers = nullptr;
		vkBindings.push_back(binding);
	}

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
	layoutInfo.pBindings = vkBindings.data();

	return m_device.createDescriptorSetLayoutUnique(layoutInfo);
}

} // namespace vulkan
} // namespace graphics
