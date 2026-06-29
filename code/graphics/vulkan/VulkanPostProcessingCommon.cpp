#include "VulkanPostProcessing.h"

#include <array>

#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorManager.h"
#include "graphics/grinternal.h"
#include "graphics/2d.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "nebula/neb.h"
#include "nebula/volumetrics.h"
#include "mission/missionparse.h"

extern float Sun_spot;
extern int Game_subspace_effect;
extern SCP_vector<light> Lights;
extern int Num_lights;


namespace graphics::vulkan {


bool PostProcessContext::initScratchUBO()
{
	vk::BufferCreateInfo bufInfo;
	bufInfo.size = SCRATCH_UBO_MAX_SLOTS * SCRATCH_UBO_SLOT_SIZE;
	bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	bufInfo.sharingMode = vk::SharingMode::eExclusive;

	try {
		scratchUBO = device.createBuffer(bufInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("PostProcessContext: Failed to create scratch UBO: %s\n", e.what()));
		return false;
	}

	if (!memoryManager->allocateBufferMemory(scratchUBO, MemoryUsage::CpuToGpu, scratchUBOAlloc)) {
		mprintf(("PostProcessContext: Failed to allocate scratch UBO memory!\n"));
		device.destroyBuffer(scratchUBO);
		scratchUBO = nullptr;
		return false;
	}

	return true;
}

void PostProcessContext::shutdownScratchUBO()
{
	if (scratchUBO) {
		device.destroyBuffer(scratchUBO);
		scratchUBO = nullptr;
	}
	if (scratchUBOAlloc.isValid()) {
		memoryManager->freeAllocation(scratchUBOAlloc);
	}
}

void PostProcessContext::generateMipmaps(vk::CommandBuffer cmd, vk::Image image,
                                           uint32_t width, uint32_t height, uint32_t mipLevels)
{
	// Transition mip 0 from eShaderReadOnlyOptimal (after brightpass) to eTransferSrcOptimal
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eTransfer,
			{}, {}, {}, barrier);
	}

	vulkan_generate_mipmap_chain(cmd, image, width, height, mipLevels);
}

void PostProcessContext::drawFullscreenTriangle(vk::CommandBuffer cmd, vk::RenderPass renderPass,
                                                  vk::Framebuffer framebuffer, vk::Extent2D extent,
                                                  int shaderType,
                                                  vk::ImageView textureView, vk::Sampler sampler,
                                                  const void* uboData, size_t uboSize,
                                                  int blendMode,
                                                  unsigned int shaderFlags)
{
	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();
	auto* bufferMgr = getBufferManager();
	auto* texMgr = getTextureManager();

	if (!pipelineMgr || !descriptorMgr || !bufferMgr || !texMgr) {
		return;
	}

	// Get/create pipeline for this shader + render pass combination
	PipelineConfig config;
	config.shaderType = static_cast<shader_type>(shaderType);
	config.shaderFlags = shaderFlags;
	config.vertexLayoutHash = 0;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = static_cast<gr_alpha_blend>(blendMode);
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = renderPass;

	vertex_layout emptyLayout;
	vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
	if (!pipeline) {
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	// Begin render pass
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = renderPass;
	rpBegin.framebuffer = framebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = extent;

	cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	// Set viewport and scissor
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
	writer.reset(device, descriptorMgr->getFallbacks());

	// Set 1: Material
	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);
	writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
	{
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texArrayInfos;
		texArrayInfos.fill(descriptorMgr->getFallbacks().texture2D);
		texArrayInfos[0].sampler = sampler;
		texArrayInfos[0].imageView = textureView;
		writer.setImageArray(MaterialBinding::TextureArray, texArrayInfos);
	}

	// Set 2: PerDraw
	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);
	writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
	if (uboData && uboSize > 0 && scratchUBOMapped) {
		Assertion(scratchUBOCursor < SCRATCH_UBO_MAX_SLOTS, "Scratch UBO slot overflow!");
		uint32_t slotOffset = scratchUBOCursor * static_cast<uint32_t>(SCRATCH_UBO_SLOT_SIZE);
		memcpy(static_cast<uint8_t*>(scratchUBOMapped) + slotOffset, uboData, uboSize);
		scratchUBOCursor++;
		writer.setBuffer(PerDrawBinding::GenericData, {scratchUBO, slotOffset, SCRATCH_UBO_SLOT_SIZE});
	}
	writer.flush();

	// Bind descriptor sets (Set 0 already bound from frame setup)
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
		static_cast<uint32_t>(DescriptorSetIndex::Material),
		{materialSet, perDrawSet}, {});

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();
}

bool PostProcessContext::createImage(uint32_t width, uint32_t height, vk::Format format,
                                      vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect,
                                      vk::Image& outImage, vk::ImageView& outView,
                                      VulkanAllocation& outAllocation,
                                      vk::SampleCountFlagBits sampleCount)
{
	// Create image
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = format;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = sampleCount;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	try {
		outImage = device.createImage(imageInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create image: %s\n", e.what()));
		return false;
	}

	// Allocate memory
	if (!memoryManager->allocateImageMemory(outImage, MemoryUsage::GpuOnly, outAllocation)) {
		mprintf(("VulkanPostProcessor: Failed to allocate image memory!\n"));
		device.destroyImage(outImage);
		outImage = nullptr;
		return false;
	}

	// Create image view (plain 2D, not array)
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = outImage;
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	try {
		outView = device.createImageView(viewInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create image view: %s\n", e.what()));
		device.destroyImage(outImage);
		memoryManager->freeAllocation(outAllocation);
		outImage = nullptr;
		return false;
	}

	return true;
}

void copyImageToImage(
    vk::CommandBuffer cmd,
    vk::Image src, vk::ImageLayout srcOldLayout, vk::ImageLayout srcNewLayout,
    vk::Image dst, vk::ImageLayout dstOldLayout, vk::ImageLayout dstNewLayout,
    vk::Extent2D extent,
    vk::ImageAspectFlags aspect,
    uint32_t dstMipLevels)
{
	// Derive access mask and pipeline stage from a layout.
	// 'leaving' = true for srcAccessMask (flushing writes before transition),
	// false for dstAccessMask (making data available after transition).
	auto layoutInfo = [](vk::ImageLayout layout, bool leaving)
	    -> std::pair<vk::AccessFlags, vk::PipelineStageFlags> {
		switch (layout) {
		case vk::ImageLayout::eUndefined:
			return {{}, vk::PipelineStageFlagBits::eTopOfPipe};
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			return {leaving ? vk::AccessFlags{} : vk::AccessFlagBits::eShaderRead,
			        vk::PipelineStageFlagBits::eFragmentShader};
		case vk::ImageLayout::eColorAttachmentOptimal:
			return {leaving ? vk::AccessFlagBits::eColorAttachmentWrite
			               : (vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite),
			        vk::PipelineStageFlagBits::eColorAttachmentOutput};
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			return {leaving ? vk::AccessFlagBits::eDepthStencilAttachmentWrite
			               : (vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite),
			        leaving ? vk::PipelineStageFlagBits::eLateFragmentTests
			                : vk::PipelineStageFlagBits::eEarlyFragmentTests};
		case vk::ImageLayout::eTransferSrcOptimal:
			return {vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eTransfer};
		case vk::ImageLayout::eTransferDstOptimal:
			return {vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTransfer};
		default:
			Assertion(false, "copyImageToImage: unsupported layout %d", static_cast<int>(layout));
			return {{}, vk::PipelineStageFlagBits::eAllCommands};
		}
	};

	// 1. Pre-barriers: transition src → eTransferSrcOptimal, dst → eTransferDstOptimal
	{
		auto [srcAccess, srcStage] = layoutInfo(srcOldLayout, true);
		auto [dstAccess, dstStage] = layoutInfo(dstOldLayout, true);

		std::array<vk::ImageMemoryBarrier, 2> barriers;

		barriers[0].srcAccessMask = srcAccess;
		barriers[0].dstAccessMask = vk::AccessFlagBits::eTransferRead;
		barriers[0].oldLayout = srcOldLayout;
		barriers[0].newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].image = src;
		barriers[0].subresourceRange = {aspect, 0, 1, 0, 1};

		barriers[1].srcAccessMask = dstAccess;
		barriers[1].dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		barriers[1].oldLayout = dstOldLayout;
		barriers[1].newLayout = vk::ImageLayout::eTransferDstOptimal;
		barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].image = dst;
		barriers[1].subresourceRange = {aspect, 0, dstMipLevels, 0, 1};

		cmd.pipelineBarrier(
			srcStage | dstStage,
			vk::PipelineStageFlagBits::eTransfer,
			{}, nullptr, nullptr, barriers);
	}

	// 2. Copy (always mip 0, layer 0)
	{
		vk::ImageCopy region;
		region.srcSubresource = {aspect, 0, 0, 1};
		region.dstSubresource = {aspect, 0, 0, 1};
		region.extent = vk::Extent3D(extent.width, extent.height, 1);

		cmd.copyImage(
			src, vk::ImageLayout::eTransferSrcOptimal,
			dst, vk::ImageLayout::eTransferDstOptimal,
			region);
	}

	// 3. Post-barriers: transition src → srcNewLayout, dst → dstNewLayout
	// Skip rule: if newLayout matches the transfer layout, skip that barrier
	{
		bool skipSrc = (srcNewLayout == vk::ImageLayout::eTransferSrcOptimal);
		bool skipDst = (dstNewLayout == vk::ImageLayout::eTransferDstOptimal);

		if (skipSrc && skipDst) {
			return;
		}

		std::array<vk::ImageMemoryBarrier, 2> barriers;
		uint32_t count = 0;
		vk::PipelineStageFlags postDstStage = {};

		if (!skipSrc) {
			auto [access, stage] = layoutInfo(srcNewLayout, false);
			barriers[count].srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barriers[count].dstAccessMask = access;
			barriers[count].oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barriers[count].newLayout = srcNewLayout;
			barriers[count].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[count].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[count].image = src;
			barriers[count].subresourceRange = {aspect, 0, 1, 0, 1};
			count++;
			postDstStage |= stage;
		}

		if (!skipDst) {
			auto [access, stage] = layoutInfo(dstNewLayout, false);
			barriers[count].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barriers[count].dstAccessMask = access;
			barriers[count].oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barriers[count].newLayout = dstNewLayout;
			barriers[count].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[count].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[count].image = dst;
			barriers[count].subresourceRange = {aspect, 0, dstMipLevels, 0, 1};
			count++;
			postDstStage |= stage;
		}

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			postDstStage,
			{}, nullptr, nullptr,
			vk::ArrayProxy<const vk::ImageMemoryBarrier>(count, barriers.data()));
	}
}

} // namespace graphics::vulkan
