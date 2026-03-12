#include "VulkanDescriptorManager.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"


namespace graphics::vulkan {

// ========== Static set templates ==========

static constexpr DescriptorBindingTemplate s_globalBindings[] = {
	{GlobalBinding::Lights,        vk::DescriptorType::eUniformBuffer,        1},
	{GlobalBinding::DeferredData,  vk::DescriptorType::eUniformBuffer,        1},
	{GlobalBinding::ShadowMap,     vk::DescriptorType::eCombinedImageSampler, 1, vk::ImageViewType::e2DArray},
	{GlobalBinding::EnvMap,        vk::DescriptorType::eCombinedImageSampler, 1, vk::ImageViewType::eCube},
	{GlobalBinding::IrradianceMap, vk::DescriptorType::eCombinedImageSampler, 1, vk::ImageViewType::eCube},
};
static constexpr DescriptorSetTemplate s_globalTemplate(s_globalBindings);

static constexpr DescriptorBindingTemplate s_materialBindings[] = {
	{MaterialBinding::ModelData,     vk::DescriptorType::eUniformBuffer,        1},
	{MaterialBinding::TextureArray,  vk::DescriptorType::eCombinedImageSampler, 16, vk::ImageViewType::e2DArray},
	{MaterialBinding::DecalGlobals,  vk::DescriptorType::eUniformBuffer,        1},
	{MaterialBinding::TransformSSBO, vk::DescriptorType::eStorageBuffer,        1},
	{MaterialBinding::DepthMap,      vk::DescriptorType::eCombinedImageSampler, 1, vk::ImageViewType::e2D},
	{MaterialBinding::SceneColor,    vk::DescriptorType::eCombinedImageSampler, 1, vk::ImageViewType::e2D},
	{MaterialBinding::DistortionMap, vk::DescriptorType::eCombinedImageSampler, 1, vk::ImageViewType::e2D},
};
static constexpr DescriptorSetTemplate s_materialTemplate(s_materialBindings);

static constexpr DescriptorBindingTemplate s_perDrawBindings[] = {
	{PerDrawBinding::GenericData, vk::DescriptorType::eUniformBuffer, 1},
	{PerDrawBinding::Matrices,    vk::DescriptorType::eUniformBuffer, 1},
	{PerDrawBinding::NanoVGData,  vk::DescriptorType::eUniformBuffer, 1},
	{PerDrawBinding::DecalInfo,   vk::DescriptorType::eUniformBuffer, 1},
	{PerDrawBinding::MovieData,   vk::DescriptorType::eUniformBuffer, 1},
};
static constexpr DescriptorSetTemplate s_perDrawTemplate(s_perDrawBindings);

// ========== DescriptorFallbacks ==========

const vk::DescriptorImageInfo& DescriptorFallbacks::getImage(vk::ImageViewType t) const
{
	switch (t) {
	case vk::ImageViewType::e2D:      return texture2D;
	case vk::ImageViewType::e2DArray: return texture2DArray;
	case vk::ImageViewType::eCube:    return textureCube;
	case vk::ImageViewType::e3D:      return texture3D;
	default:
		Assertion(false, "DescriptorFallbacks::getImage: unhandled ImageViewType %d", static_cast<int>(t));
		return texture2D;
	}
}

// ========== DescriptorWriter template-based methods ==========

void DescriptorWriter::writeSet(vk::DescriptorSet set, const DescriptorSetTemplate& tmpl)
{
	Verify(m_fallbacks);

	// Clear binding slots for this set
	m_bindingSlots = {};

	for (size_t i = 0; i < tmpl.bindingCount; ++i) {
		const auto& b = tmpl.bindings[i];
		Verify(m_writeCount < MAX_WRITES);
		Verify(b.binding < MAX_BINDINGS_PER_SET);

		auto& w = m_writes[m_writeCount++];
		w = vk::WriteDescriptorSet();
		w.dstSet = set;
		w.dstBinding = b.binding;
		w.descriptorCount = b.count;
		w.descriptorType = b.type;

		auto& slot = m_bindingSlots[b.binding];
		slot.count = b.count;
		slot.viewType = b.viewType;

		bool isImage = (b.type == vk::DescriptorType::eCombinedImageSampler);
		if (isImage) {
			Verify(m_imageInfoCount + b.count <= MAX_IMAGE_INFOS);
			auto* dst = &m_imageInfos[m_imageInfoCount];
			const auto& fallbackImg = m_fallbacks->getImage(b.viewType);
			for (uint32_t j = 0; j < b.count; ++j) {
				dst[j] = fallbackImg;
			}
			w.pImageInfo = dst;
			slot.imageInfo = dst;
			m_imageInfoCount += b.count;
		} else {
			Verify(m_bufferInfoCount < MAX_BUFFER_INFOS);
			m_bufferInfos[m_bufferInfoCount] = m_fallbacks->buffer;
			w.pBufferInfo = &m_bufferInfos[m_bufferInfoCount];
			slot.bufferInfo = &m_bufferInfos[m_bufferInfoCount++];
		}
	}
}

void DescriptorWriter::setBuffer(uint32_t binding, const vk::DescriptorBufferInfo& info)
{
	Verify(binding < MAX_BINDINGS_PER_SET);
	auto& slot = m_bindingSlots[binding];
	Verify(slot.bufferInfo);
	if (info.buffer) {
		*slot.bufferInfo = info;
	} else {
		*slot.bufferInfo = m_fallbacks->buffer;
	}
}

void DescriptorWriter::setImage(uint32_t binding, const vk::DescriptorImageInfo& info)
{
	Verify(binding < MAX_BINDINGS_PER_SET);
	auto& slot = m_bindingSlots[binding];
	Verify(slot.imageInfo);
	if (info.imageView) {
		*slot.imageInfo = info;
	} else {
		*slot.imageInfo = m_fallbacks->getImage(slot.viewType);
	}
}

void DescriptorWriter::setImageArray(uint32_t binding, const vk::DescriptorImageInfo* infos, uint32_t count)
{
	Verify(binding < MAX_BINDINGS_PER_SET);
	auto& slot = m_bindingSlots[binding];
	Verify(slot.imageInfo);
	Verify(count <= slot.count);
	memcpy(slot.imageInfo, infos, count * sizeof(vk::DescriptorImageInfo));
}

// ========== Global descriptor manager ==========

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

void VulkanDescriptorManager::buildFallbacks(VulkanBufferManager* bufMgr, VulkanTextureManager* texMgr)
{
	m_fallbacks.buffer = bufMgr->getFallbackUniformBufferInfo();
	m_fallbacks.texture2D = texMgr->getFallbackTextureInfo2D();
	m_fallbacks.texture2DArray = texMgr->getFallbackTextureInfo2DArray();
	m_fallbacks.textureCube = texMgr->getFallbackTextureInfoCube();
	m_fallbacks.texture3D = texMgr->getFallbackTextureInfo3D();
	mprintf(("VulkanDescriptorManager: Fallbacks built\n"));
}

const DescriptorSetTemplate& VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex setIndex)
{
	switch (setIndex) {
	case DescriptorSetIndex::Global:   return s_globalTemplate;
	case DescriptorSetIndex::Material: return s_materialTemplate;
	case DescriptorSetIndex::PerDraw:  return s_perDrawTemplate;
	default:
		Assertion(false, "Invalid DescriptorSetIndex!");
		return s_globalTemplate;
	}
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
			{ GlobalBinding::Lights,       vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
			{ GlobalBinding::DeferredData, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
			{ GlobalBinding::ShadowMap,    vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },
			{ GlobalBinding::EnvMap,       vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },
			{ GlobalBinding::IrradianceMap, vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },
		};
		m_setLayouts[static_cast<size_t>(DescriptorSetIndex::Global)] = createSetLayout(bindings);
	}

	// Set 1: Material (per-batch data)
	{
		SCP_vector<DescriptorBindingInfo> bindings = {
			{ MaterialBinding::ModelData,     vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
			{ MaterialBinding::TextureArray,  vk::DescriptorType::eCombinedImageSampler, MAX_TEXTURE_BINDINGS,
			  vk::ShaderStageFlagBits::eFragment },
			{ MaterialBinding::DecalGlobals,  vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
			{ MaterialBinding::TransformSSBO, vk::DescriptorType::eStorageBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex },
			{ MaterialBinding::DepthMap,      vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },
			{ MaterialBinding::SceneColor,    vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },
			{ MaterialBinding::DistortionMap, vk::DescriptorType::eCombinedImageSampler, 1,
			  vk::ShaderStageFlagBits::eFragment },
		};
		m_setLayouts[static_cast<size_t>(DescriptorSetIndex::Material)] = createSetLayout(bindings);
	}

	// Set 2: Per-Draw (per-draw-call data)
	{
		SCP_vector<DescriptorBindingInfo> bindings = {
			{ PerDrawBinding::GenericData, vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
			{ PerDrawBinding::Matrices,    vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
			{ PerDrawBinding::NanoVGData,  vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
			{ PerDrawBinding::DecalInfo,   vk::DescriptorType::eUniformBuffer, 1,
			  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
			{ PerDrawBinding::MovieData,   vk::DescriptorType::eUniformBuffer, 1,
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

} // namespace graphics::vulkan
