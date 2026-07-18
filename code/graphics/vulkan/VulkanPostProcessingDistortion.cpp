#include "VulkanPostProcessing.h"

#include <array>

#include "VulkanBarrier.h"
#include "VulkanConstants.h"
#include "VulkanDeletionQueue.h"
#include "VulkanRenderer.h"
#include "utils/Random.h"


namespace graphics::vulkan {


bool VulkanDistortion::init(PostProcessContext& ctx)
{
	m_ctx = &ctx;

	for (size_t i = 0; i < m_tex.size(); i++) {
		if (!m_ctx->createImage(32, 32, LDR_COLOR_FORMAT,
		                        vk::ImageUsageFlagBits::eTransferSrc
		                        | vk::ImageUsageFlagBits::eTransferDst
		                        | vk::ImageUsageFlagBits::eSampled,
		                        vk::ImageAspectFlagBits::eColor,
		                        m_tex[i].image, m_tex[i].view, m_tex[i].allocation)) {
			nprintf(("vulkan", "VulkanDistortion: Failed to create distortion texture %zu\n", i));
			return false;
		}
		m_tex[i].format = LDR_COLOR_FORMAT;
		m_tex[i].width = 32;
		m_tex[i].height = 32;
	}

	// Create LINEAR/REPEAT sampler for distortion textures
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;

	try {
		m_sampler = m_ctx->device.createSampler(samplerInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanDistortion: Failed to create distortion sampler: %s\n", e.what()));
		return false;
	}

	m_initialized = true;
	nprintf(("vulkan", "VulkanDistortion: Distortion textures initialized\n"));
	return true;
}

void VulkanDistortion::shutdown()
{
	if (!m_ctx) {
		return;
	}

	if (m_sampler) {
		m_ctx->device.destroySampler(m_sampler);
		m_sampler = nullptr;
	}
	for (auto& t : m_tex) {
		if (t.view) {
			m_ctx->device.destroyImageView(t.view);
			t.view = nullptr;
		}
		if (t.image) {
			m_ctx->device.destroyImage(t.image);
			t.image = nullptr;
		}
		if (t.allocation.isValid()) {
			m_ctx->memoryManager->freeAllocation(t.allocation);
		}
	}
	m_initialized = false;
}

void VulkanDistortion::update(vk::CommandBuffer cmd, float frametime)
{
	if (!m_initialized) {
		return;
	}

	m_timer += frametime;
	if (m_timer < 0.03f) {
		return;
	}
	m_timer = 0.0f;

	int dst = !m_switch;  // Write target
	int src = m_switch;   // Read source

	// On first update, images are still in eUndefined layout
	vk::ImageLayout srcOldLayout = m_firstUpdate
		? vk::ImageLayout::eUndefined : vk::ImageLayout::eShaderReadOnlyOptimal;
	vk::AccessFlags2 srcOldAccess = m_firstUpdate
		? vk::AccessFlags2{} : vk::AccessFlagBits2::eShaderSampledRead;

	// Transition both distortion textures for transfer operations. dst is
	// written by three different-stage ops below -- clearColorImage (eClear),
	// blitImage as dst (eBlit), and (conditionally) copyBufferToImage for the
	// noise column (eCopy) -- so its dst scope must cover all three, not just
	// the first one issued. src only feeds the blitImage() as source (eBlit).
	{
		std::array<ImageBarrier2, 2> barriers;

		// dst: eShaderReadOnlyOptimal (or eUndefined on first use) → eTransferDstOptimal
		barriers[0].image = m_tex[dst].image;
		barriers[0].levelCount = 1;
		barriers[0].layerCount = 1;
		barriers[0].oldLayout = srcOldLayout;
		barriers[0].newLayout = vk::ImageLayout::eTransferDstOptimal;
		barriers[0].srcStage = vk::PipelineStageFlagBits2::eFragmentShader;
		barriers[0].srcAccess = srcOldAccess;
		barriers[0].dstStage = vk::PipelineStageFlagBits2::eClear
		                     | vk::PipelineStageFlagBits2::eBlit
		                     | vk::PipelineStageFlagBits2::eCopy;
		barriers[0].dstAccess = vk::AccessFlagBits2::eTransferWrite;

		// src: eShaderReadOnlyOptimal (or eUndefined on first use) → eTransferSrcOptimal
		barriers[1].image = m_tex[src].image;
		barriers[1].levelCount = 1;
		barriers[1].layerCount = 1;
		barriers[1].oldLayout = srcOldLayout;
		barriers[1].newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barriers[1].srcStage = vk::PipelineStageFlagBits2::eFragmentShader;
		barriers[1].srcAccess = srcOldAccess;
		barriers[1].dstStage = vk::PipelineStageFlagBits2::eBlit;
		barriers[1].dstAccess = vk::AccessFlagBits2::eTransferRead;

		cmdImageBarriers(cmd, ArrayView<const ImageBarrier2>(barriers.data(), barriers.size()));
	}

	// Clear dest to mid-gray (0.5, 0.5, 0.0, 1.0) = no distortion
	{
		vk::ClearColorValue clearColor;
		clearColor.setFloat32({0.5f, 0.5f, 0.0f, 1.0f});
		vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
		cmd.clearColorImage(m_tex[dst].image,
			vk::ImageLayout::eTransferDstOptimal, clearColor, range);
	}

	// Blit: scroll old data right by 1 pixel
	// src columns 0-30 → dst columns 1-31 (with LINEAR filtering)
	{
		vk::ImageBlit blit;
		blit.srcSubresource = vk::ImageSubresourceLayers(
			vk::ImageAspectFlagBits::eColor, 0, 0, 1);
		blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
		blit.srcOffsets[1] = vk::Offset3D(31, 32, 1);
		blit.dstSubresource = vk::ImageSubresourceLayers(
			vk::ImageAspectFlagBits::eColor, 0, 0, 1);
		blit.dstOffsets[0] = vk::Offset3D(1, 0, 0);
		blit.dstOffsets[1] = vk::Offset3D(32, 32, 1);

		cmd.blitImage(
			m_tex[src].image, vk::ImageLayout::eTransferSrcOptimal,
			m_tex[dst].image, vk::ImageLayout::eTransferDstOptimal,
			blit, vk::Filter::eLinear);
	}

	// Generate random noise and copy to column 0 of dst
	// OpenGL draws 33 GL_POINTS at x=0 with random R,G values — we write 32 pixels
	{
		// Create a small host-visible staging buffer for 32 RGBA8 pixels (128 bytes)
		vk::BufferCreateInfo bufInfo;
		bufInfo.size = 32 * 4;
		bufInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		vk::Buffer stagingBuf;
		VulkanAllocation stagingAlloc;
		try {
			stagingBuf = m_ctx->device.createBuffer(bufInfo);
		} catch (const vk::SystemError&) {
			// Non-fatal: skip noise injection this frame
			goto skip_noise;
		}

		Verify(m_ctx->memoryManager->allocateBufferMemory(stagingBuf, MemoryUsage::CpuOnly, stagingAlloc));

		{
			auto* pixels = static_cast<uint8_t*>(m_ctx->memoryManager->mapMemory(stagingAlloc));
			Verify(pixels);
			for (int i = 0; i < 32; i++) {
				pixels[(i * 4) + 0] = static_cast<uint8_t>(::util::Random::next(256));  // R
				pixels[(i * 4) + 1] = static_cast<uint8_t>(::util::Random::next(256));  // G
				pixels[(i * 4) + 2] = 255;  // B
				pixels[(i * 4) + 3] = 255;  // A
			}
			m_ctx->memoryManager->unmapMemory(stagingAlloc);

			// Copy staging buffer → column 0 of dst (1 pixel wide, 32 pixels tall)
			vk::BufferImageCopy region;
			region.bufferOffset = 0;
			region.bufferRowLength = 0;    // Tightly packed
			region.bufferImageHeight = 0;
			region.imageSubresource = vk::ImageSubresourceLayers(
				vk::ImageAspectFlagBits::eColor, 0, 0, 1);
			region.imageOffset = vk::Offset3D(0, 0, 0);
			region.imageExtent = vk::Extent3D(1, 32, 1);

			cmd.copyBufferToImage(stagingBuf, m_tex[dst].image,
				vk::ImageLayout::eTransferDstOptimal, region);
		}

		// Schedule staging buffer for deferred destruction (GPU may still be reading)
		auto* delQueue = getDeletionQueue();
		if (delQueue) {
			delQueue->queueBuffer(stagingBuf, stagingAlloc);
		} else {
			m_ctx->device.destroyBuffer(stagingBuf);
			m_ctx->memoryManager->freeAllocation(stagingAlloc);
		}
	}

skip_noise:
	// Transition both textures back to eShaderReadOnlyOptimal
	{
		std::array<ImageBarrier2, 2> barriers;

		// dst: eTransferDstOptimal → eShaderReadOnlyOptimal. srcStage covers
		// both possible producers: the blit above always writes it, and the
		// noise copyBufferToImage() conditionally also writes it (skipped via
		// goto on staging-buffer failure) -- both are known code paths in this
		// function, not an unknown caller, so this is a precise union rather
		// than a generic catch-all.
		barriers[0].image = m_tex[dst].image;
		barriers[0].levelCount = 1;
		barriers[0].layerCount = 1;
		barriers[0].oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barriers[0].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barriers[0].srcStage = vk::PipelineStageFlagBits2::eBlit | vk::PipelineStageFlagBits2::eCopy;
		barriers[0].srcAccess = vk::AccessFlagBits2::eTransferWrite;
		barriers[0].dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
		barriers[0].dstAccess = vk::AccessFlagBits2::eShaderSampledRead;

		// src: eTransferSrcOptimal → eShaderReadOnlyOptimal (only the blit reads it)
		barriers[1].image = m_tex[src].image;
		barriers[1].levelCount = 1;
		barriers[1].layerCount = 1;
		barriers[1].oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barriers[1].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barriers[1].srcStage = vk::PipelineStageFlagBits2::eBlit;
		barriers[1].srcAccess = vk::AccessFlagBits2::eTransferRead;
		barriers[1].dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
		barriers[1].dstAccess = vk::AccessFlagBits2::eShaderSampledRead;

		cmdImageBarriers(cmd, ArrayView<const ImageBarrier2>(barriers.data(), barriers.size()));
	}

	m_switch = !m_switch;
	m_firstUpdate = false;
}

vk::DescriptorImageInfo VulkanDistortion::getTextureInfo() const
{
	if (!m_initialized) {
		return {};
	}
	// Return the most recently written texture (matching OpenGL's
	// Distortion_texture[!Distortion_switch] binding for thrusters).
	// After update() toggles the switch, m_switch points to the old read
	// source, which is the texture that was just written.
	return {m_ctx->linearSampler, m_tex[m_switch].view,
	        vk::ImageLayout::eShaderReadOnlyOptimal};
}

} // namespace graphics::vulkan
