#include "VulkanDescriptorManager.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanRaytracing.h"


namespace graphics::vulkan {

// ========== Static set templates ==========

static constexpr DescriptorBindingTemplate s_globalBindings[] = {
	{GlobalBinding::Lights,        vk::DescriptorType::eUniformBuffer,        1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{GlobalBinding::DeferredData,  vk::DescriptorType::eUniformBuffer,        1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{GlobalBinding::ShadowMap,     vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, vk::ImageViewType::e2DArray},
	{GlobalBinding::EnvMap,        vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, vk::ImageViewType::eCube},
	{GlobalBinding::IrradianceMap, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, vk::ImageViewType::eCube},
	{GlobalBinding::ShadowCascadeParams, vk::DescriptorType::eUniformBuffer,  1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
};
static constexpr DescriptorBindingTemplate s_globalTlasBinding{
	GlobalBinding::Tlas, vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eFragment};

// The Global set's layout must omit the TLAS binding entirely on devices without
// VK_KHR_acceleration_structure enabled -- a descriptor type the device didn't
// enable the extension for is invalid in vkCreateDescriptorSetLayout. So, unlike
// the other set templates, the Global template is assembled at runtime in
// createSetLayouts() based on the raytracingSupported flag passed to init(), into
// the per-instance m_globalBindingsRuntime/m_globalTemplateRuntime members
// (see VulkanDescriptorManager.h), rather than compiled in as a fixed constexpr array.

static constexpr DescriptorBindingTemplate s_materialBindings[] = {
	{MaterialBinding::ModelData,     vk::DescriptorType::eUniformBuffer,        1,  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::TextureArray,  vk::DescriptorType::eCombinedImageSampler, 16, vk::ShaderStageFlagBits::eFragment, vk::ImageViewType::e2DArray},
	{MaterialBinding::DecalGlobals,  vk::DescriptorType::eUniformBuffer,        1,  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::TransformSSBO, vk::DescriptorType::eStorageBuffer,        1,  vk::ShaderStageFlagBits::eVertex},
	{MaterialBinding::DepthMap,      vk::DescriptorType::eCombinedImageSampler, 1,  vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::SceneColor,    vk::DescriptorType::eCombinedImageSampler, 1,  vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::DistortionMap, vk::DescriptorType::eCombinedImageSampler, 1,  vk::ShaderStageFlagBits::eFragment},
	{MaterialBinding::ShadowMapData, vk::DescriptorType::eUniformBuffer,        1,  vk::ShaderStageFlagBits::eVertex},
};
static constexpr DescriptorSetTemplate s_materialTemplate(s_materialBindings);

static constexpr DescriptorBindingTemplate s_perDrawBindings[] = {
	{PerDrawBinding::GenericData, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{PerDrawBinding::Matrices,    vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{PerDrawBinding::NanoVGData,  vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{PerDrawBinding::DecalInfo,   vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
	{PerDrawBinding::MovieData,   vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
};
static constexpr DescriptorSetTemplate s_perDrawTemplate(s_perDrawBindings);

// ========== Static uniform binding mappings ==========

static constexpr VulkanDescriptorManager::UniformBindingEntry s_globalUBOs[] = {
	{GlobalBinding::Lights,              uniform_block_type::Lights},
	{GlobalBinding::DeferredData,        uniform_block_type::DeferredGlobals},
	{GlobalBinding::ShadowCascadeParams, uniform_block_type::ShadowCascadeParams},
};

static constexpr VulkanDescriptorManager::UniformBindingEntry s_materialUBOs[] = {
	{MaterialBinding::ModelData,    uniform_block_type::ModelData},
	{MaterialBinding::DecalGlobals, uniform_block_type::DecalGlobals},
};
// Note: MaterialBinding::ShadowMapData is intentionally NOT in s_materialUBOs --
// unlike ModelData/DecalGlobals, shadow_render_list passes its UBO handle/offset/size
// directly into gr_render_shadow_draw() rather than through gr_bind_uniform_buffer(),
// so VulkanDrawManager::renderShadowDraw() binds it explicitly instead of relying on
// the generic pending-uniform-binding pass in applyMaterial().

static constexpr VulkanDescriptorManager::UniformBindingEntry s_perDrawUBOs[] = {
	{PerDrawBinding::GenericData, uniform_block_type::GenericData},
	{PerDrawBinding::Matrices,    uniform_block_type::Matrices},
	{PerDrawBinding::NanoVGData,  uniform_block_type::NanoVGData},
	{PerDrawBinding::DecalInfo,   uniform_block_type::DecalInfo},
	{PerDrawBinding::MovieData,   uniform_block_type::MovieData},
};


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

	for (const auto& b : tmpl) {
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
		bool isAccelStruct = (b.type == vk::DescriptorType::eAccelerationStructureKHR);
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
		} else if (isAccelStruct) {
			// No setAccelStruct() override step needed: m_fallbacks->shadowTlas IS
			// the live value, kept fresh once per frame by setCurrentShadowTlas().
			Verify(b.count == 1);
			Verify(m_accelStructInfoCount < MAX_ACCEL_STRUCT_INFOS);
			auto* dst = &m_accelStructInfos[m_accelStructInfoCount];
			*dst = m_fallbacks->shadowTlas;

			auto& asInfo = m_asWriteInfos[m_accelStructInfoCount];
			asInfo = vk::WriteDescriptorSetAccelerationStructureKHR();
			asInfo.accelerationStructureCount = 1;
			asInfo.pAccelerationStructures = dst;

			w.pNext = &asInfo;
			m_accelStructInfoCount++;
		} else {
			Verify(m_bufferInfoCount < MAX_BUFFER_INFOS);
			m_bufferInfos[m_bufferInfoCount] = m_fallbacks->buffer;
			w.pBufferInfo = &m_bufferInfos[m_bufferInfoCount];
			slot.bufferInfo = &m_bufferInfos[m_bufferInfoCount++];
		}
	}
}

void DescriptorWriter::flush()
{
	if (m_writeCount > 0) {
		m_device.updateDescriptorSets(m_writeCount, m_writes.data(), 0, nullptr);
		getDescriptorManager()->noteDescriptorWrites(m_writeCount);
	}
	m_writeCount = 0;
	m_bufferInfoCount = 0;
	m_imageInfoCount = 0;
	m_accelStructInfoCount = 0;
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

void DescriptorWriter::setImageArray(uint32_t binding, ArrayView<vk::DescriptorImageInfo> infos)
{
	Verify(binding < MAX_BINDINGS_PER_SET);
	auto& slot = m_bindingSlots[binding];
	Verify(slot.imageInfo);
	Verify(infos.size <= slot.count);
	memcpy(slot.imageInfo, infos.data, infos.size * sizeof(vk::DescriptorImageInfo));
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

bool VulkanDescriptorManager::init(vk::Device device, bool raytracingSupported)
{
	if (m_initialized) {
		return true;
	}

	m_device = device;
	m_raytracingEnabled = raytracingSupported;

	createSetLayouts();
	createDescriptorPools();

	m_initialized = true;
	nprintf(("vulkan", "VulkanDescriptorManager: Initialized (raytraced shadows %s)\n",
		m_raytracingEnabled ? "enabled" : "disabled"));
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
	nprintf(("vulkan", "VulkanDescriptorManager: Shutdown complete\n"));
}

void VulkanDescriptorManager::buildFallbacks(VulkanBufferManager* bufMgr, VulkanTextureManager* texMgr,
	VulkanRaytracingManager* rtMgr)
{
	m_fallbacks.buffer = bufMgr->getFallbackUniformBufferInfo();
	m_fallbacks.texture2D = texMgr->getFallbackTextureInfo2D();
	m_fallbacks.texture2DArray = texMgr->getFallbackTextureInfo2DArray();
	m_fallbacks.textureCube = texMgr->getFallbackTextureInfoCube();
	m_fallbacks.texture3D = texMgr->getFallbackTextureInfo3D();
	if (m_raytracingEnabled && rtMgr != nullptr) {
		// Seed with the permanent empty TLAS; setCurrentShadowTlas() takes over
		// once the first real TLAS is built.
		m_fallbacks.shadowTlas = rtMgr->getFallbackTlas();
	}
	nprintf(("vulkan", "VulkanDescriptorManager: Fallbacks built\n"));
}

const DescriptorSetTemplate& VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex setIndex)
{
	switch (setIndex) {
	case DescriptorSetIndex::Global:   return getDescriptorManager()->m_globalTemplateRuntime;
	case DescriptorSetIndex::Material: return s_materialTemplate;
	case DescriptorSetIndex::PerDraw:  return s_perDrawTemplate;
	default:
		Assertion(false, "Invalid DescriptorSetIndex!");
		return getDescriptorManager()->m_globalTemplateRuntime;
	}
}

vk::DescriptorSetLayout VulkanDescriptorManager::getSetLayout(DescriptorSetIndex setIndex) const
{
	return m_setLayouts[static_cast<size_t>(setIndex)].get();
}

vk::DescriptorSet VulkanDescriptorManager::allocateFrameSet(DescriptorSetIndex setIndex)
{
	if (!m_initialized) {
		return {};
	}

	vk::DescriptorSetLayout layout = m_setLayouts[static_cast<size_t>(setIndex)].get();
	auto& pools = m_framePools[m_currentFrame];
	++m_setsAllocatedThisFrame;

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
	nprintf(("vulkandescriptor", "VulkanDescriptorManager: Grew frame %u pool count to %zu\n",
		m_currentFrame, pools.size()));

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = pools.back().get();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	try {
		auto sets = m_device.allocateDescriptorSets(allocInfo);
		return sets[0];
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanDescriptorManager: Failed to allocate frame descriptor set after pool growth: %s\n", e.what()));
		return {};
	}
}

void VulkanDescriptorManager::beginFrame()
{
	if (!m_initialized) {
		return;
	}

	auto& pools = m_framePools[m_currentFrame];

	// Reset per-frame diagnostic counters
	m_setsAllocatedThisFrame = 0;
	m_writesThisFrame = 0;

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

ArrayView<VulkanDescriptorManager::UniformBindingEntry>
VulkanDescriptorManager::getUniformBindings(DescriptorSetIndex setIndex)
{
	switch (setIndex) {
	case DescriptorSetIndex::Global:   return {s_globalUBOs, std::size(s_globalUBOs)};
	case DescriptorSetIndex::Material: return {s_materialUBOs, std::size(s_materialUBOs)};
	case DescriptorSetIndex::PerDraw:  return {s_perDrawUBOs, std::size(s_perDrawUBOs)};
	default:                           return {nullptr, 0};
	}
}

void VulkanDescriptorManager::createSetLayouts()
{
	m_globalBindingsRuntime.assign(std::begin(s_globalBindings), std::end(s_globalBindings));
	if (m_raytracingEnabled) {
		m_globalBindingsRuntime.push_back(s_globalTlasBinding);
	}
	m_globalTemplateRuntime = DescriptorSetTemplate(m_globalBindingsRuntime.data(), m_globalBindingsRuntime.size());

	m_setLayouts[static_cast<size_t>(DescriptorSetIndex::Global)]   = createSetLayout(m_globalTemplateRuntime);
	m_setLayouts[static_cast<size_t>(DescriptorSetIndex::Material)] = createSetLayout(s_materialTemplate);
	m_setLayouts[static_cast<size_t>(DescriptorSetIndex::PerDraw)]  = createSetLayout(s_perDrawTemplate);

	nprintf(("vulkan", "VulkanDescriptorManager: Created %zu descriptor set layouts\n",
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
	if (m_raytracingEnabled) {
		poolSizes.emplace_back(vk::DescriptorType::eAccelerationStructureKHR, MAX_SETS_PER_POOL);
	}

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

	nprintf(("vulkan", "VulkanDescriptorManager: Created %u frame pool chains\n",
		MAX_FRAMES_IN_FLIGHT));
}

vk::UniqueDescriptorSetLayout VulkanDescriptorManager::createSetLayout(
	const DescriptorSetTemplate& tmpl)
{
	SCP_vector<vk::DescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(tmpl.size);

	for (const auto& b : tmpl) {
		vk::DescriptorSetLayoutBinding binding;
		binding.binding = b.binding;
		binding.descriptorType = b.type;
		binding.descriptorCount = b.count;
		binding.stageFlags = b.stages;
		binding.pImmutableSamplers = nullptr;
		vkBindings.push_back(binding);
	}

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
	layoutInfo.pBindings = vkBindings.data();

	return m_device.createDescriptorSetLayoutUnique(layoutInfo);
}

} // namespace graphics::vulkan
