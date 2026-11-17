#include "VulkanPostProcessing.h"

#include <array>
#include <cmath>

#include "gr_vulkan.h"
#include "VulkanRenderer.h"
#include "VulkanTexture.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorManager.h"
#include "graphics/2d.h"
#include "graphics/cortao_occluder_lut.h"
#include "graphics/grinternal.h"
#include "graphics/matrix.h"
#include "graphics/ssao.h"
#include "lighting/lighting_profiles.h"

namespace graphics::vulkan {

namespace {

constexpr vk::Format PACKED_FORMAT = vk::Format::eR32Sfloat;
constexpr vk::Format AO_FORMAT = vk::Format::eR8Unorm;

// The CORTAO configuration values that the engine does not change. These are the defaults that
// CORTAOParameters.h recommends.
constexpr float CORTAO_MIP_BIAS = -4.0f;
constexpr float CORTAO_MAX_SLOPE_COMPENSATION = 4.0f;
constexpr float CORTAO_BACKFACE_THRESHOLD = 0.5f;
constexpr float CORTAO_BACKFACE_BOOST = 0.0f;
constexpr float CORTAO_FILTER_DEPTH_SENSITIVITY = 512.0f;
constexpr float CORTAO_FILTER_NORMAL_SENSITIVITY = 128.0f;

// A constant seed gives a constant noise pattern. See SsaoNoiseMode.
constexpr uint32_t CORTAO_CONSTANT_SEED = 0x6a09e667u;

// The std140 layout of the genericData block in cortao-prepass-f.sdr
struct CortaoPrepassUniforms {
	uint32_t packedResolution;
	uint32_t randomSeed;
	uint32_t mipLevel;
	float nearPlane;
};

// The std140 layout of the genericData block in cortao-main-f.sdr: CORTAOMainPassParameters
// (CORTAOMainPass.h), then the intensity.
struct CortaoMainUniforms {
	uint32_t packedResolution;
	uint32_t randomSeed;
	float normalBias;
	float radius;

	float unprojectParams[4];
	float projectionParams[4];

	float maxApparentCircleRadiusCoefficient;
	float mipBias;
	float inverseMaxSlopeCompensation;
	float backfaceThreshold;

	float backfaceBoost;
	uint32_t padding0[3];

	float intensity;
	float padding1[3];
};
static_assert(sizeof(CortaoMainUniforms) == 96, "CortaoMainUniforms must match the std140 layout of cortao-main-f.sdr");

// The std140 layout of CORTAOFilterPassParameters (CORTAOFilterPass.h) in cortao-filter-f.sdr
struct CortaoFilterUniforms {
	uint32_t packedResolution;
	float depthSensitivity;
	float normalSensitivity;
	uint32_t padding0;
};

uint32_t hash_frame_number(uint64_t frame)
{
	// A 32-bit integer hash (lowbias32, Chris Wellons)
	auto x = static_cast<uint32_t>(frame);
	x ^= x >> 16;
	x *= 0x7feb352du;
	x ^= x >> 15;
	x *= 0x846ca68bu;
	x ^= x >> 16;
	return x;
}

/**
 * @brief Fill the main pass parameters from the current projection matrix
 *
 * This does the work of CORTAO::populateMainPassParameters() in CORTAOParameters.h with flipY = true.
 * We do not compile CORTAOParameters.h, because it uses std::sqrtf, which libstdc++ does not have.
 *
 * flipY is true because the G-buffer pass uses a negative viewport height: NDC +Y is texel row 0.
 */
void fill_main_uniforms(CortaoMainUniforms& out, uint32_t width, uint32_t height, uint32_t seed)
{
	// gr_projection_matrix is column-major. The main pass uses only the X and Y terms, so the
	// different near planes of the scene and cockpit passes do not change the result.
	// A render-to-texture projection can have a negative Y scale for its Y-flip. The G-buffer row
	// order does not change, so use the magnitude. A negative value would make the mip bias NaN.
	const float projX = std::fabs(gr_projection_matrix.a1d[0]);
	const float projY = std::fabs(gr_projection_matrix.a1d[5]);
	const float projOffsetX = gr_projection_matrix.a1d[8];
	const float projOffsetY = gr_projection_matrix.a1d[9];

	// The terms of the inverse projection matrix that CORTAO uses
	const float invX = 1.0f / projX;
	const float invY = 1.0f / projY;
	const float invOffsetX = projOffsetX / projX;
	const float invOffsetY = projOffsetY / projY;

	memset(&out, 0, sizeof(out));
	out.packedResolution = width | (height << 16u);
	out.randomSeed = seed;
	out.normalBias = invX / static_cast<float>(width) * std::sqrt(2.0f);
	out.radius = lighting_profiles::current_ssao_max_radius();

	out.unprojectParams[0] = +2.0f * invX;
	out.unprojectParams[1] = -2.0f * invY;
	out.unprojectParams[2] = -invX + invOffsetX;
	out.unprojectParams[3] = +invY + invOffsetY;

	out.projectionParams[0] = +0.5f * projX;
	out.projectionParams[1] = -0.5f * projY;
	out.projectionParams[2] = -0.5f + 0.5f * projOffsetX;
	out.projectionParams[3] = -0.5f - 0.5f * projOffsetY;

	const float maxApparentCircleRadius = invY * lighting_profiles::current_ssao_screen_radius() * 2.0f;
	out.maxApparentCircleRadiusCoefficient =
		maxApparentCircleRadius / std::sqrt(maxApparentCircleRadius * maxApparentCircleRadius + 1.0f);
	out.mipBias = std::log2(projY * static_cast<float>(height) * 0.5f) + CORTAO_MIP_BIAS;
	out.inverseMaxSlopeCompensation = 1.0f / CORTAO_MAX_SLOPE_COMPENSATION;
	out.backfaceThreshold = CORTAO_BACKFACE_THRESHOLD;
	out.backfaceBoost = CORTAO_BACKFACE_BOOST;

	out.intensity = lighting_profiles::current_ssao_intensity();
}

} // namespace

void VulkanCortao::init(PostProcessContext& ctx, const VulkanDeferredGBuffer& gbuffer)
{
	m_ctx = &ctx;
	m_gbuffer = &gbuffer;
}

static vk::RenderPass create_single_color_render_pass(vk::Device device, vk::Format format)
{
	vk::AttachmentDescription att;
	att.format = format;
	att.samples = vk::SampleCountFlagBits::e1;
	att.loadOp = vk::AttachmentLoadOp::eDontCare;
	att.storeOp = vk::AttachmentStoreOp::eStore;
	att.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	att.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	att.initialLayout = vk::ImageLayout::eUndefined;
	att.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::AttachmentReference colorRef;
	colorRef.attachment = 0;
	colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;

	// The first dependency makes the G-buffer writes visible to the fragment shader, which samples
	// the G-buffer. It also orders the write after the reads of the previous CORTAO run (the scene
	// and the cockpit passes use the same targets). The second one makes the result visible to
	// the next pass, which samples it.
	std::array<vk::SubpassDependency, 2> deps;
	deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	deps[0].dstSubpass = 0;
	deps[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eColorAttachmentOutput;
	deps[0].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eColorAttachmentOutput;
	deps[0].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	deps[0].dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentWrite;

	deps[1].srcSubpass = 0;
	deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	deps[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	deps[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
	deps[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	deps[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;

	vk::RenderPassCreateInfo rpInfo;
	rpInfo.attachmentCount = 1;
	rpInfo.pAttachments = &att;
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;
	rpInfo.dependencyCount = static_cast<uint32_t>(deps.size());
	rpInfo.pDependencies = deps.data();

	try {
		return device.createRenderPass(rpInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanCortao: Failed to create render pass: %s\n", e.what()));
		return nullptr;
	}
}

bool VulkanCortao::initResources()
{
	auto* texMgr = getTextureManager();
	if (!texMgr) {
		return false;
	}

	m_packedRenderPass = create_single_color_render_pass(m_ctx->device, PACKED_FORMAT);
	m_aoRenderPass = create_single_color_render_pass(m_ctx->device, AO_FORMAT);
	if (!m_packedRenderPass || !m_aoRenderPass) {
		return false;
	}

	// CORTAO requires NEAREST filtering and a border of 0 for the packed data.
	{
		vk::SamplerCreateInfo info;
		info.magFilter = vk::Filter::eNearest;
		info.minFilter = vk::Filter::eNearest;
		info.mipmapMode = vk::SamplerMipmapMode::eNearest;
		info.addressModeU = vk::SamplerAddressMode::eClampToBorder;
		info.addressModeV = vk::SamplerAddressMode::eClampToBorder;
		info.addressModeW = vk::SamplerAddressMode::eClampToBorder;
		info.borderColor = vk::BorderColor::eFloatTransparentBlack;
		info.minLod = 0.0f;
		info.maxLod = VK_LOD_CLAMP_NONE;

		try {
			m_nearestBorderSampler = m_ctx->device.createSampler(info);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanCortao: Failed to create sampler: %s\n", e.what()));
			return false;
		}
	}

	// The 3D lookup table is stored as a 2D texture with the slices one after the other.
	// See cortao_sampleOccluderLookupTable() in cortao-main-f.sdr.
	if (!texMgr->createStaticTexture2D(CORTAO_OCCLUDER_LUT_WIDTH, CORTAO_OCCLUDER_LUT_HEIGHT * CORTAO_OCCLUDER_LUT_DEPTH,
			vk::Format::eR16Unorm, Cortao_occluder_lut, sizeof(Cortao_occluder_lut), "CORTAO occluder lookup table",
			m_lutImage, m_lutView, m_lutAlloc)) {
		nprintf(("vulkan", "VulkanCortao: Failed to upload the occluder lookup table\n"));
		return false;
	}

	{
		const auto align = static_cast<vk::DeviceSize>(getRendererInstance()->getMinUniformBufferOffsetAlignment());
		const vk::DeviceSize largest = std::max(sizeof(CortaoPrepassUniforms), sizeof(CortaoMainUniforms));
		m_uboBlockSize = (largest + align - 1) / align * align;

		vk::BufferCreateInfo bufInfo;
		bufInfo.size = m_uboBlockSize * UBO_BLOCKS_PER_REGION * UBO_REGIONS;
		bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_ubo = m_ctx->device.createBuffer(bufInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanCortao: Failed to create UBO: %s\n", e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateBufferMemory(m_ubo, MemoryUsage::CpuToGpu, m_uboAlloc)) {
			nprintf(("vulkan", "VulkanCortao: Failed to allocate UBO memory\n"));
			return false;
		}
	}

	if (!createTargets()) {
		return false;
	}

	m_resourcesInitialized = true;
	nprintf(("vulkan", "VulkanCortao: Initialized (%ux%u, %u packed mip levels)\n",
		m_ctx->sceneExtent.width, m_ctx->sceneExtent.height, m_mipLevels));
	return true;
}

bool VulkanCortao::createTargets()
{
	const uint32_t w = m_ctx->sceneExtent.width;
	const uint32_t h = m_ctx->sceneExtent.height;

	// One mip level for each halving of the larger side, at most MAX_MIP_LEVELS.
	m_mipLevels = 1;
	while (m_mipLevels < MAX_MIP_LEVELS && (std::max(w, h) >> m_mipLevels) > 0) {
		++m_mipLevels;
	}

	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = PACKED_FORMAT;
		imageInfo.extent = vk::Extent3D(w, h, 1);
		imageInfo.mipLevels = m_mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_packedImage = m_ctx->device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanCortao: Failed to create packed data image: %s\n", e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateImageMemory(
				m_packedImage, MemoryUsage::GpuOnly, m_packedAlloc, MemoryPurpose::RenderTarget)) {
			nprintf(("vulkan", "VulkanCortao: Failed to allocate packed data image memory\n"));
			return false;
		}

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_packedImage;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = PACKED_FORMAT;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = m_mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		try {
			m_packedFullView = m_ctx->device.createImageView(viewInfo);
			for (uint32_t mip = 0; mip < m_mipLevels; ++mip) {
				auto mipViewInfo = viewInfo;
				mipViewInfo.subresourceRange.baseMipLevel = mip;
				mipViewInfo.subresourceRange.levelCount = 1;
				m_packedMipViews[mip] = m_ctx->device.createImageView(mipViewInfo);

				vk::FramebufferCreateInfo fbInfo;
				fbInfo.renderPass = m_packedRenderPass;
				fbInfo.attachmentCount = 1;
				fbInfo.pAttachments = &m_packedMipViews[mip];
				fbInfo.width = std::max(1u, w >> mip);
				fbInfo.height = std::max(1u, h >> mip);
				fbInfo.layers = 1;
				m_packedMipFramebuffers[mip] = m_ctx->device.createFramebuffer(fbInfo);
			}
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanCortao: Failed to create packed data views: %s\n", e.what()));
			return false;
		}
	}

	// The main pass writes m_aoRaw, and the filter pass writes m_ao.
	for (auto* target : {&m_aoRaw, &m_ao}) {
		if (!m_ctx->createImage(w, h, AO_FORMAT,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
				vk::ImageAspectFlagBits::eColor, target->image, target->view, target->allocation)) {
			nprintf(("vulkan", "VulkanCortao: Failed to create occlusion image\n"));
			return false;
		}
		target->format = AO_FORMAT;
		target->width = w;
		target->height = h;

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_aoRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &target->view;
		fbInfo.width = w;
		fbInfo.height = h;
		fbInfo.layers = 1;

		try {
			(target == &m_aoRaw ? m_aoRawFramebuffer : m_aoFramebuffer) = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanCortao: Failed to create occlusion framebuffer: %s\n", e.what()));
			return false;
		}
	}

	return true;
}

void VulkanCortao::destroyTargets()
{
	auto device = m_ctx->device;

	for (auto* fb : {&m_aoFramebuffer, &m_aoRawFramebuffer}) {
		if (*fb) {
			device.destroyFramebuffer(*fb);
			*fb = nullptr;
		}
	}
	m_ctx->destroyTarget(m_ao);
	m_ctx->destroyTarget(m_aoRaw);

	for (auto& fb : m_packedMipFramebuffers) {
		if (fb) {
			device.destroyFramebuffer(fb);
			fb = nullptr;
		}
	}
	for (auto& view : m_packedMipViews) {
		if (view) {
			device.destroyImageView(view);
			view = nullptr;
		}
	}
	if (m_packedFullView) {
		device.destroyImageView(m_packedFullView);
		m_packedFullView = nullptr;
	}
	if (m_packedImage) {
		device.destroyImage(m_packedImage);
		m_packedImage = nullptr;
	}
	if (m_packedAlloc.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_packedAlloc);
	}
	m_mipLevels = 0;
}

void VulkanCortao::shutdown()
{
	// m_ctx and m_gbuffer stay set, so that the next execute() can create the resources again.
	if (!m_ctx || !m_ctx->device) {
		return;
	}
	auto device = m_ctx->device;

	destroyTargets();

	if (m_ubo) {
		device.destroyBuffer(m_ubo);
		m_ubo = nullptr;
	}
	if (m_uboAlloc.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_uboAlloc);
	}
	if (m_lutView) {
		device.destroyImageView(m_lutView);
		m_lutView = nullptr;
	}
	if (m_lutImage) {
		device.destroyImage(m_lutImage);
		m_lutImage = nullptr;
	}
	if (m_lutAlloc.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_lutAlloc);
	}
	if (m_nearestBorderSampler) {
		device.destroySampler(m_nearestBorderSampler);
		m_nearestBorderSampler = nullptr;
	}
	if (m_aoRenderPass) {
		device.destroyRenderPass(m_aoRenderPass);
		m_aoRenderPass = nullptr;
	}
	if (m_packedRenderPass) {
		device.destroyRenderPass(m_packedRenderPass);
		m_packedRenderPass = nullptr;
	}

	m_resourcesInitialized = false;
}

bool VulkanCortao::onResize()
{
	m_initFailed = false;
	if (!m_resourcesInitialized) {
		return true;
	}
	destroyTargets();
	return createTargets();
}

void VulkanCortao::drawPass(vk::CommandBuffer cmd, vk::RenderPass renderPass, vk::Framebuffer framebuffer,
                            vk::Extent2D extent, int shaderType, unsigned int shaderFlags,
                            const vk::ImageView* views, uint32_t viewCount, vk::DeviceSize uboOffset, vk::DeviceSize uboSize)
{
	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();

	PipelineConfig config;
	config.shaderType = static_cast<shader_type>(shaderType);
	config.shaderFlags = shaderFlags;
	config.vertexLayoutHash = 0;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = ALPHA_BLEND_NONE;
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = renderPass;
	config.colorAttachmentCount = 1;

	vertex_layout emptyLayout;
	vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
	if (!pipeline) {
		return;
	}
	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = renderPass;
	rpBegin.framebuffer = framebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = extent;

	cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	// A positive viewport height: gl_FragCoord is the texel of the target, as the shaders expect.
	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmd.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = extent;
	cmd.setScissor(0, scissor);

	DescriptorWriter writer;
	writer.reset(m_ctx->device, descriptorMgr->getFallbacks());

	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	if (!materialSet || !perDrawSet) {
		cmd.endRenderPass();
		return;
	}

	writer.writeSet(DescriptorSetIndex::Material, materialSet);
	{
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texArrayInfos;
		texArrayInfos.fill(descriptorMgr->getFallbacks().texture2D);
		for (uint32_t i = 0; i < viewCount; i++) {
			texArrayInfos[i] = {m_nearestBorderSampler, views[i], vk::ImageLayout::eShaderReadOnlyOptimal};
		}
		writer.setImageArray(MaterialBinding::TextureArray, texArrayInfos);
	}

	writer.writeSet(DescriptorSetIndex::PerDraw, perDrawSet);
	writer.setBuffer(PerDrawBinding::GenericData, {m_ubo, uboOffset, uboSize});
	writer.flush();

	const vk::DescriptorSet sets[] = {materialSet, perDrawSet};
	writer.bindSets(cmd, pipelineLayout, DescriptorSetIndex::Material, sets);

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();
}

vk::ImageView VulkanCortao::execute(vk::CommandBuffer cmd, bool temporalFilterActive)
{
	if (!m_ctx || !m_gbuffer || !m_gbuffer->isInitialized() || !ssao_enabled() || m_initFailed) {
		return nullptr;
	}

	if (!m_resourcesInitialized && !initResources()) {
		// Do not try again each frame. A resize clears this flag.
		m_initFailed = true;
		shutdown();
		return nullptr;
	}

	auto* descriptorMgr = getDescriptorManager();
	if (!getPipelineManager() || !descriptorMgr) {
		return nullptr;
	}

	GR_DEBUG_SCOPE("CORTAO SSAO");

	const uint32_t w = m_ctx->sceneExtent.width;
	const uint32_t h = m_ctx->sceneExtent.height;
	const uint32_t seed = ssao_animate_noise(temporalFilterActive)
		? hash_frame_number(getRendererInstance()->getCurrentFrameNumber())
		: CORTAO_CONSTANT_SEED;
	const SsaoFilterMode filterMode = ssao_filter_mode(temporalFilterActive);

	unsigned int mainFlags = 0;
	switch (ssao_sample_count()) {
	case 4:
		mainFlags = SDR_FLAG_CORTAO_SAMPLES_4;
		break;
	case 8:
		mainFlags = SDR_FLAG_CORTAO_SAMPLES_8;
		break;
	default:
		break; // 16 samples: the shader default
	}

	// The UBO region of this (frame in flight, lighting mode). See UBO_REGIONS.
	const uint32_t regionIndex = descriptorMgr->getCurrentFrame() * static_cast<uint32_t>(lighting_mode::MAX_LIGHTING_MODES)
	                           + static_cast<uint32_t>(Lighting_mode);
	const vk::DeviceSize regionBase = regionIndex * m_uboBlockSize * UBO_BLOCKS_PER_REGION;
	const vk::DeviceSize mainBlockOffset = regionBase + MAX_MIP_LEVELS * m_uboBlockSize;
	const vk::DeviceSize filterBlockOffset = mainBlockOffset + m_uboBlockSize;

	{
		auto* mapped = static_cast<uint8_t*>(m_ctx->memoryManager->mapMemory(m_uboAlloc));
		if (!mapped) {
			return nullptr;
		}

		for (uint32_t mip = 0; mip < m_mipLevels; ++mip) {
			CortaoPrepassUniforms prepass;
			prepass.packedResolution = w | (h << 16u);
			prepass.randomSeed = seed;
			prepass.mipLevel = mip;
			prepass.nearPlane = gr_near_plane;
			memcpy(mapped + regionBase + mip * m_uboBlockSize, &prepass, sizeof(prepass));
		}

		CortaoMainUniforms mainUniforms;
		fill_main_uniforms(mainUniforms, w, h, seed);
		memcpy(mapped + mainBlockOffset, &mainUniforms, sizeof(mainUniforms));

		CortaoFilterUniforms filterUniforms;
		filterUniforms.packedResolution = w | (h << 16u);
		filterUniforms.depthSensitivity = CORTAO_FILTER_DEPTH_SENSITIVITY;
		filterUniforms.normalSensitivity = CORTAO_FILTER_NORMAL_SENSITIVITY;
		filterUniforms.padding0 = 0;
		memcpy(mapped + filterBlockOffset, &filterUniforms, sizeof(filterUniforms));

		m_ctx->memoryManager->flushMemory(m_uboAlloc, regionBase, m_uboBlockSize * UBO_BLOCKS_PER_REGION);
		m_ctx->memoryManager->unmapMemory(m_uboAlloc);
	}

	{
		GR_DEBUG_SCOPE("CORTAO prepass");
		const std::array<vk::ImageView, 2> views = {m_gbuffer->positionView(), m_gbuffer->normalView()};
		for (uint32_t mip = 0; mip < m_mipLevels; ++mip) {
			const vk::Extent2D extent(std::max(1u, w >> mip), std::max(1u, h >> mip));
			drawPass(cmd, m_packedRenderPass, m_packedMipFramebuffers[mip], extent, SDR_TYPE_CORTAO_PREPASS, 0,
				views.data(), static_cast<uint32_t>(views.size()), regionBase + mip * m_uboBlockSize, m_uboBlockSize);
		}
	}

	{
		GR_DEBUG_SCOPE("CORTAO main pass");
		const std::array<vk::ImageView, 3> views = {m_packedFullView, m_gbuffer->normalView(), m_lutView};
		drawPass(cmd, m_aoRenderPass, m_aoRawFramebuffer, m_ctx->sceneExtent, SDR_TYPE_CORTAO_MAIN, mainFlags,
			views.data(), static_cast<uint32_t>(views.size()), mainBlockOffset, m_uboBlockSize);
	}

	if (filterMode == SsaoFilterMode::Off) {
		return m_aoRaw.view;
	}

	{
		GR_DEBUG_SCOPE("CORTAO filter pass");
		// The filter reads only mip 0 of the packed data, with texelFetch().
		const std::array<vk::ImageView, 2> views = {m_packedFullView, m_aoRaw.view};
		const unsigned int filterFlags = (filterMode == SsaoFilterMode::Large) ? SDR_FLAG_CORTAO_LARGE_FILTER : 0;
		drawPass(cmd, m_aoRenderPass, m_aoFramebuffer, m_ctx->sceneExtent, SDR_TYPE_CORTAO_FILTER, filterFlags,
			views.data(), static_cast<uint32_t>(views.size()), filterBlockOffset, m_uboBlockSize);
	}

	return m_ao.view;
}

} // namespace graphics::vulkan
