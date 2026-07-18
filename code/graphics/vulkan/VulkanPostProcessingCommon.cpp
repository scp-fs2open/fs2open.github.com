#include "VulkanPostProcessing.h"

#include <array>

#include "VulkanBarrier.h"
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
	return scratchRing.init(device, memoryManager, SCRATCH_UBO_MAX_SLOTS, SCRATCH_UBO_SLOT_SIZE);
}

void PostProcessContext::shutdownScratchUBO()
{
	scratchRing.shutdown();
}

void PostProcessContext::destroyTarget(RenderTarget& rt) const
{
	if (rt.view) {
		device.destroyImageView(rt.view);
		rt.view = nullptr;
	}
	if (rt.image) {
		device.destroyImage(rt.image);
		rt.image = nullptr;
	}
	if (rt.allocation.isValid()) {
		memoryManager->freeAllocation(rt.allocation);
		rt.allocation = {};
	}
	rt.width = 0;
	rt.height = 0;
}

void PostProcessContext::generateMipmaps(vk::CommandBuffer cmd, vk::Image image,
                                           uint32_t width, uint32_t height, uint32_t mipLevels)
{
	// Transition mip 0 from eShaderReadOnlyOptimal (after brightpass) to
	// eTransferSrcOptimal (source for the blit chain below)
	{
		ImageBarrier2 barrier;
		barrier.image = image;
		barrier.baseMipLevel = 0;
		barrier.levelCount = 1;
		barrier.baseArrayLayer = 0;
		barrier.layerCount = 1;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		barrier.srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
		barrier.dstStage = vk::PipelineStageFlagBits2::eBlit;
		barrier.dstAccess = vk::AccessFlagBits2::eTransferRead;

		cmdImageBarrier(cmd, barrier);
	}

	vulkan_generate_mipmap_chain(cmd, image, width, height, mipLevels);
}

void PostProcessContext::drawFullscreenTriangle(vk::CommandBuffer cmd, vk::RenderPass renderPass,
                                                  vk::Framebuffer framebuffer, vk::Extent2D extent,
                                                  int shaderType,
                                                  vk::ImageView textureView, vk::Sampler sampler,
                                                  const void* uboData, size_t uboSize,
                                                  int blendMode,
                                                  unsigned int shaderFlags,
                                                  vk::SampleCountFlagBits sampleCount,
                                                  bool bindGlobalSet,
                                                  const std::array<float, 4>* clearColor)
{
	GR_DEBUG_SCOPE("Draw full screen triangle");

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
	config.sampleCount = sampleCount;

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

	if (clearColor) {
		// Explicit in-pass clear, independent of the render pass's own loadOp: needed
		// for passes whose shader discards some pixels (e.g. SMAA edge detection),
		// which can't rely on loadOp=eDontCare leaving them at any particular value.
		vk::ClearAttachment clearAttachment;
		clearAttachment.aspectMask = vk::ImageAspectFlagBits::eColor;
		clearAttachment.colorAttachment = 0;
		clearAttachment.clearValue.color = vk::ClearColorValue{*clearColor};

		vk::ClearRect clearRect;
		clearRect.rect.offset = vk::Offset2D(0, 0);
		clearRect.rect.extent = extent;
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;

		cmd.clearAttachments(clearAttachment, clearRect);
	}

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

	// Set 0: Global -- normally already bound from frame setup. bindGlobalSet
	// is for draws that may run before that binding happened this frame (e.g.
	// mid-G-buffer-pass copies that run ahead of any material draw).
	vk::DescriptorSet globalSet;
	if (bindGlobalSet) {
		globalSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Global);
		Verify(globalSet);
		writer.writeSet(globalSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Global));
	}

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
	if (uboData != nullptr && uboSize > 0 && scratchRing.isValid()) {
		vk::DeviceSize slotOffset = scratchRing.alloc(descriptorMgr->getCurrentFrame(), uboData, uboSize);
		writer.setBuffer(PerDrawBinding::GenericData, {scratchRing.buffer(), slotOffset, scratchRing.slotSize()});
	}
	writer.flush();

	// Bind descriptor sets. DescriptorSetIndex::{Global,Material,PerDraw} are
	// contiguous (0,1,2), so when Global is (re)bound here it and Material/PerDraw
	// go in one call; otherwise Set 0 is left as whatever frame setup bound.
	if (bindGlobalSet) {
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
			static_cast<uint32_t>(DescriptorSetIndex::Global),
			{globalSet, materialSet, perDrawSet}, {});
	} else {
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
			static_cast<uint32_t>(DescriptorSetIndex::Material),
			{materialSet, perDrawSet}, {});
	}

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();
}

void PostProcessContext::drawFullscreenTriangleMulti(vk::CommandBuffer cmd, vk::RenderPass renderPass,
                                                       vk::Framebuffer framebuffer, vk::Extent2D extent,
                                                       int shaderType,
                                                       const vk::ImageView* views, uint32_t viewCount,
                                                       const void* uboData, size_t uboSize)
{
	GR_DEBUG_SCOPE("Draw full screen triangle (multi-view)");

	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();
	auto* bufferMgr = getBufferManager();

	if (!pipelineMgr || !descriptorMgr || !bufferMgr) {
		return;
	}

	PipelineConfig config;
	config.shaderType = static_cast<shader_type>(shaderType);
	config.shaderFlags = 0;
	config.vertexLayoutHash = 0;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = ALPHA_BLEND_NONE;
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = renderPass;

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

	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);
	writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
	{
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texArrayInfos;
		texArrayInfos.fill(descriptorMgr->getFallbacks().texture2D);
		for (uint32_t i = 0; i < viewCount; i++) {
			texArrayInfos[i] = {linearSampler, views[i], vk::ImageLayout::eShaderReadOnlyOptimal};
		}
		writer.setImageArray(MaterialBinding::TextureArray, texArrayInfos);
	}

	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);
	writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
	if (uboData != nullptr && uboSize > 0 && scratchRing.isValid()) {
		vk::DeviceSize slotOffset = scratchRing.alloc(descriptorMgr->getCurrentFrame(), uboData, uboSize);
		writer.setBuffer(PerDrawBinding::GenericData, {scratchRing.buffer(), slotOffset, scratchRing.slotSize()});
	}
	writer.flush();

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
                                      vk::SampleCountFlagBits sampleCount) const
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
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create image: %s\n", e.what()));
		return false;
	}

	// Allocate memory
	if (!memoryManager->allocateImageMemory(outImage, MemoryUsage::GpuOnly, outAllocation)) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to allocate image memory!\n"));
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
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create image view: %s\n", e.what()));
		device.destroyImage(outImage);
		memoryManager->freeAllocation(outAllocation);
		outImage = nullptr;
		return false;
	}

	return true;
}

// Derive access mask and pipeline stage from a layout, for the transfer-op
// pre/post barriers used by copyImageToImage().
// 'leaving' = true for srcAccessMask (flushing writes before transition),
// false for dstAccessMask (making data available after transition).
static std::pair<vk::AccessFlags2, vk::PipelineStageFlags2> transferLayoutInfo(vk::ImageLayout layout, bool leaving)
{
	switch (layout) {
	case vk::ImageLayout::eUndefined:
		return {{}, vk::PipelineStageFlagBits2::eTopOfPipe};
	case vk::ImageLayout::eShaderReadOnlyOptimal:
		return {leaving ? vk::AccessFlags2{} : vk::AccessFlagBits2::eShaderSampledRead,
		        vk::PipelineStageFlagBits2::eFragmentShader};
	case vk::ImageLayout::eColorAttachmentOptimal:
		return {leaving ? vk::AccessFlagBits2::eColorAttachmentWrite
		               : (vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite),
		        vk::PipelineStageFlagBits2::eColorAttachmentOutput};
	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		return {leaving ? vk::AccessFlagBits2::eDepthStencilAttachmentWrite
		               : (vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite),
		        leaving ? vk::PipelineStageFlagBits2::eLateFragmentTests
		                : vk::PipelineStageFlagBits2::eEarlyFragmentTests};
	case vk::ImageLayout::ePresentSrcKHR:
		return {leaving ? vk::AccessFlags2{} : vk::AccessFlags2{}, vk::PipelineStageFlagBits2::eBottomOfPipe};
	case vk::ImageLayout::eTransferSrcOptimal:
		return {vk::AccessFlagBits2::eTransferRead, vk::PipelineStageFlagBits2::eTransfer};
	case vk::ImageLayout::eTransferDstOptimal:
		return {vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eTransfer};
	default:
		Assertion(false, "transferLayoutInfo: unsupported layout %d", static_cast<int>(layout));
		return {{}, vk::PipelineStageFlagBits2::eAllCommands};
	}
}

void copyImageToImage(
    vk::CommandBuffer cmd,
    vk::Image src, vk::ImageLayout srcOldLayout, vk::ImageLayout srcNewLayout,
    vk::Image dst, vk::ImageLayout dstOldLayout, vk::ImageLayout dstNewLayout,
    vk::Extent2D extent,
    vk::ImageAspectFlags aspect,
    uint32_t dstMipLevels)
{
	auto layoutInfo = transferLayoutInfo;

	// 1. Pre-barriers: transition src → eTransferSrcOptimal, dst → eTransferDstOptimal
	// (destination stage is eCopy: both barriers exist to feed the cmd.copyImage() below)
	{
		auto [srcAccess, srcStage] = layoutInfo(srcOldLayout, true);
		auto [dstAccess, dstStage] = layoutInfo(dstOldLayout, true);

		std::array<ImageBarrier2, 2> barriers;

		barriers[0].image = src;
		barriers[0].aspectMask = aspect;
		barriers[0].levelCount = 1;
		barriers[0].layerCount = 1;
		barriers[0].oldLayout = srcOldLayout;
		barriers[0].newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barriers[0].srcStage = srcStage;
		barriers[0].srcAccess = srcAccess;
		barriers[0].dstStage = vk::PipelineStageFlagBits2::eCopy;
		barriers[0].dstAccess = vk::AccessFlagBits2::eTransferRead;

		barriers[1].image = dst;
		barriers[1].aspectMask = aspect;
		barriers[1].levelCount = dstMipLevels;
		barriers[1].layerCount = 1;
		barriers[1].oldLayout = dstOldLayout;
		barriers[1].newLayout = vk::ImageLayout::eTransferDstOptimal;
		barriers[1].srcStage = dstStage;
		barriers[1].srcAccess = dstAccess;
		barriers[1].dstStage = vk::PipelineStageFlagBits2::eCopy;
		barriers[1].dstAccess = vk::AccessFlagBits2::eTransferWrite;

		cmdImageBarriers(cmd, ArrayView<const ImageBarrier2>(barriers.data(), barriers.size()));
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

		std::array<ImageBarrier2, 2> barriers;
		uint32_t count = 0;

		if (!skipSrc) {
			auto [access, stage] = layoutInfo(srcNewLayout, false);
			barriers[count].image = src;
			barriers[count].aspectMask = aspect;
			barriers[count].levelCount = 1;
			barriers[count].layerCount = 1;
			barriers[count].oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barriers[count].newLayout = srcNewLayout;
			barriers[count].srcStage = vk::PipelineStageFlagBits2::eCopy;
			barriers[count].srcAccess = vk::AccessFlagBits2::eTransferRead;
			barriers[count].dstStage = stage;
			barriers[count].dstAccess = access;
			count++;
		}

		if (!skipDst) {
			auto [access, stage] = layoutInfo(dstNewLayout, false);
			barriers[count].image = dst;
			barriers[count].aspectMask = aspect;
			barriers[count].levelCount = dstMipLevels;
			barriers[count].layerCount = 1;
			barriers[count].oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barriers[count].newLayout = dstNewLayout;
			barriers[count].srcStage = vk::PipelineStageFlagBits2::eCopy;
			barriers[count].srcAccess = vk::AccessFlagBits2::eTransferWrite;
			barriers[count].dstStage = stage;
			barriers[count].dstAccess = access;
			count++;
		}

		cmdImageBarriers(cmd, ArrayView<const ImageBarrier2>(barriers.data(), count));
	}
}

} // namespace graphics::vulkan
