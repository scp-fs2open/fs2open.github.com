
#include "VulkanOpenXR.h"

#include "VulkanBarrier.h"
#include "VulkanRenderer.h"
#include "gr_vulkan.h"

#include "graphics/2d.h"
#include "graphics/openxr.h"

#ifdef FS_OPENXR

#include "io/cursor.h"
#include "io/mouse.h"

#include <SDL3/SDL_vulkan.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include "graphics/openxr_internal.h"

#endif

namespace graphics::vulkan {

namespace {

#ifdef FS_OPENXR

// Set while creating the Vulkan instance (the only point at which the runtime's
// graphics requirements can be checked, since they must be queried *before*
// instance creation) and read back later by test_capabilities().
bool Xr_graphics_requirements_met = false;

std::array<SCP_vector<XrSwapchainImageVulkan2KHR>, 2> Xr_swapchain_images;

// Which eyes hold an acquired swapchain image for the frame currently being
// built. A stereo frame spans two gr_flip() calls, so the left eye's
// acquisition has to survive until the right eye's call submits and releases
// both -- the release cannot happen before the work is submitted.
std::array<bool, 2> Xr_eye_acquired = {false, false};

/**
 * @brief Copy the frame the engine just rendered into one eye's swapchain image
 *
 * The source is the renderer's composition image, which is the Vulkan
 * equivalent of the backbuffer the OpenGL path blits from: the whole frame --
 * 3D scene, HUD, menus -- has landed there by the time gr_flip() runs. Note
 * that it is sampled before the output-encode pass, so the user's gamma slider
 * (Gr_gamma) applies to the desktop mirror but not to the HMD image.
 *
 * Must be called with no render pass active on the frame command buffer.
 */
void blit_to_swapchain_image(VulkanRenderer* renderer, vk::Image dst, uint32_t dstWidth, uint32_t dstHeight)
{
	const vk::CommandBuffer cmd = renderer->getCurrentCommandBuffer();
	const vk::Image composition = renderer->getCurrentCompositionImage();
	const vk::Extent2D srcExtent = renderer->getRenderExtent();

	if (!cmd || !composition || !dst) {
		return;
	}

	// The composition image is left in eShaderReadOnlyOptimal by the render
	// pass that just ended, and must be put back that way afterwards: it is
	// what both m_renderPassLoad's initialLayout and the output-encode pass
	// expect to find.
	std::array<ImageBarrier2, 2> preBarriers;
	preBarriers[0].image = composition;
	preBarriers[0].oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	preBarriers[0].newLayout = vk::ImageLayout::eTransferSrcOptimal;
	preBarriers[0].srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	preBarriers[0].srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
	preBarriers[0].dstStage = vk::PipelineStageFlagBits2::eBlit;
	preBarriers[0].dstAccess = vk::AccessFlagBits2::eTransferRead;

	// eUndefined discards whatever the runtime last left in the image, which is
	// exactly right: the blit overwrites every pixel of it.
	preBarriers[1].image = dst;
	preBarriers[1].oldLayout = vk::ImageLayout::eUndefined;
	preBarriers[1].newLayout = vk::ImageLayout::eTransferDstOptimal;
	preBarriers[1].srcStage = vk::PipelineStageFlagBits2::eNone;
	preBarriers[1].srcAccess = {};
	preBarriers[1].dstStage = vk::PipelineStageFlagBits2::eBlit;
	preBarriers[1].dstAccess = vk::AccessFlagBits2::eTransferWrite;

	cmdImageBarriers(cmd, ArrayView<const ImageBarrier2>(preBarriers.data(), preBarriers.size()));

	vk::ImageBlit blit;
	blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
	blit.srcOffsets[1] = vk::Offset3D(static_cast<int32_t>(srcExtent.width), static_cast<int32_t>(srcExtent.height), 1);

	blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;
	blit.dstOffsets[0] = vk::Offset3D(0, 0, 0);
	blit.dstOffsets[1] = vk::Offset3D(static_cast<int32_t>(dstWidth), static_cast<int32_t>(dstHeight), 1);

	// eLinear because the eye resolution rarely matches the window resolution,
	// matching the OpenGL path's GL_LINEAR blit into the swapchain.
	cmd.blitImage(composition,
		vk::ImageLayout::eTransferSrcOptimal,
		dst,
		vk::ImageLayout::eTransferDstOptimal,
		1,
		&blit,
		vk::Filter::eLinear);

	std::array<ImageBarrier2, 2> postBarriers;
	// Runtimes expect a released swapchain image in eColorAttachmentOptimal.
	postBarriers[0].image = dst;
	postBarriers[0].oldLayout = vk::ImageLayout::eTransferDstOptimal;
	postBarriers[0].newLayout = vk::ImageLayout::eColorAttachmentOptimal;
	postBarriers[0].srcStage = vk::PipelineStageFlagBits2::eBlit;
	postBarriers[0].srcAccess = vk::AccessFlagBits2::eTransferWrite;
	postBarriers[0].dstStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	postBarriers[0].dstAccess = vk::AccessFlagBits2::eColorAttachmentWrite;

	postBarriers[1].image = composition;
	postBarriers[1].oldLayout = vk::ImageLayout::eTransferSrcOptimal;
	postBarriers[1].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	postBarriers[1].srcStage = vk::PipelineStageFlagBits2::eBlit;
	postBarriers[1].srcAccess = vk::AccessFlagBits2::eTransferRead;
	postBarriers[1].dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
	postBarriers[1].dstAccess = vk::AccessFlagBits2::eShaderSampledRead;

	cmdImageBarriers(cmd, ArrayView<const ImageBarrier2>(postBarriers.data(), postBarriers.size()));
}

/**
 * @brief Acquire one eye's swapchain image and blit the current frame into it
 *
 * Ends the in-flight render pass, since a blit cannot be recorded inside one.
 */
bool render_eye(VulkanRenderer* renderer, uint32_t eye)
{
	renderer->endCurrentRenderPass();

	XrSwapchain swapchain = xr_swapchains[eye]->swapchain;

	XrSwapchainImageAcquireInfo acquireImageInfo{
		XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO,
		nullptr
	};

	uint32_t activeIndex = 0;
	if (xrAcquireSwapchainImage(swapchain, &acquireImageInfo, &activeIndex) != XR_SUCCESS) {
		return false;
	}

	XrSwapchainImageWaitInfo waitImageInfo{
		XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
		nullptr,
		std::numeric_limits<int64_t>::max()
	};
	xrWaitSwapchainImage(swapchain, &waitImageInfo);

	Xr_eye_acquired[eye] = true;

	blit_to_swapchain_image(renderer,
		vk::Image(Xr_swapchain_images[eye][activeIndex].image),
		xr_swapchains[eye]->width,
		xr_swapchains[eye]->height);

	return true;
}

void release_acquired_images()
{
	for (uint32_t i = 0; i < 2; i++) {
		if (!Xr_eye_acquired[i]) {
			continue;
		}

		XrSwapchainImageReleaseInfo releaseImageInfo{
			XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO,
			nullptr
		};
		xrReleaseSwapchainImage(xr_swapchains[i]->swapchain, &releaseImageInfo);

		Xr_eye_acquired[i] = false;
	}
}

/**
 * @brief Draw the mouse cursor into the frame
 *
 * The cursor is an OS cursor, so it is drawn by the compositor over the window
 * and never appears in anything we render. On a flat screen inside the headset
 * that would leave the player clicking blind, so it has to be stamped into the
 * image by hand -- as the OpenGL path also does. Must run while a render pass
 * is still active.
 */
void draw_cursor()
{
	auto* cursorManager = io::mouse::CursorManager::get();
	if (cursorManager == nullptr || !cursorManager->isCursorShown()) {
		return;
	}

	auto* cursor = cursorManager->getCurrentCursor();
	if (cursor == nullptr) {
		return;
	}

	const int cursorBitmap = cursor->getBitmapHandle();
	if (cursorBitmap < 0) {
		return;
	}

	int mouseX, mouseY;
	mouse_get_pos(&mouseX, &mouseY);

	gr_set_bitmap(cursorBitmap);
	gr_bitmap(mouseX, mouseY, GR_RESIZE_NONE);
}

/**
 * @brief Submit the composed frame to both the HMD and the desktop mirror
 */
void end_frame(VulkanRenderer* renderer, bool stereo)
{
	// Submit first: OpenXR requires every command that writes a swapchain image
	// to have been submitted to the queue the session was created with before
	// xrEndFrame is called. This also presents the desktop mirror and acquires
	// the next swap chain image.
	renderer->flip();

	release_acquired_images();

	XrCompositionLayerProjectionView projectedViews[2];
	XrCompositionLayerProjection projectionLayer{};
	XrCompositionLayerQuad quadLayer{};
	const XrCompositionLayerBaseHeader* pLayer = nullptr;

	if (stereo) {
		for (uint32_t i = 0; i < 2; i++) {
			projectedViews[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
			projectedViews[i].next = nullptr;
			projectedViews[i].pose = xr_views[i].pose;
			projectedViews[i].fov = xr_views[i].fov;
			projectedViews[i].subImage = {
				xr_swapchains[i]->swapchain,
				{
					{ 0, 0 },
					{ (int32_t)xr_swapchains[i]->width, (int32_t)xr_swapchains[i]->height }
				},
				0
			};
		}

		projectionLayer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
		projectionLayer.next = nullptr;
		projectionLayer.layerFlags = 0;
		projectionLayer.space = xr_space;
		projectionLayer.viewCount = 2;
		projectionLayer.views = projectedViews;

		pLayer = (const XrCompositionLayerBaseHeader*)&projectionLayer;
	}
	else {
		// A flat frame (menus, briefings, anything not rendered per-eye) becomes
		// a quad layer rather than a projection layer, letting the runtime place
		// and filter a single 2D image instead of us reprojecting it per eye.
		// Position and size reproduce the proportions of the OpenGL path's quad:
		// 4 units ahead of the origin, 4 units tall, widened by the aspect ratio.
		// OpenXR's forward axis is -Z, FSO's is +Z.
		quadLayer.type = XR_TYPE_COMPOSITION_LAYER_QUAD;
		quadLayer.next = nullptr;
		quadLayer.layerFlags = 0;
		quadLayer.space = xr_space;
		quadLayer.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
		quadLayer.subImage = {
			xr_swapchains[0]->swapchain,
			{
				{ 0, 0 },
				{ (int32_t)xr_swapchains[0]->width, (int32_t)xr_swapchains[0]->height }
			},
			0
		};
		quadLayer.pose = XrPosef{ XrQuaternionf{ 0, 0, 0, 1 }, XrVector3f{ 0, 0, -4.0f } };
		quadLayer.size = XrExtent2Df{ 4.0f * gr_screen.clip_aspect, 4.0f };

		pLayer = (const XrCompositionLayerBaseHeader*)&quadLayer;
	}

	XrFrameEndInfo frameEndInfo{
		XR_TYPE_FRAME_END_INFO,
		nullptr,
		xr_state.predictedDisplayTime,
		XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
		0,
		nullptr
	};

	if (xr_state.shouldRender) {
		frameEndInfo.layerCount = 1;
		frameEndInfo.layers = &pLayer;
	}

	xrEndFrame(xr_session, &frameEndInfo);
}

#endif

} // namespace

	// SETUP FUNCTIONS VULKAN

#ifdef FSO_OPENXR
SCP_vector<const char*> vulkan_openxr_get_extensions()
{
	return { XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME };
}

bool vulkan_openxr_test_capabilities()
{
	// The real check happened back when the Vulkan instance was created -- the
	// runtime requires its graphics requirements to be queried before that
	// point, which is far earlier than this function is called.
	return Xr_graphics_requirements_met;
}

bool vulkan_openxr_create_session()
{
	auto* renderer = getRendererInstance();

	XrGraphicsBindingVulkan2KHR graphicsBinding {
		XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR,
		nullptr,
		renderer->getVkInstance(),
		renderer->getPhysicalDevice(),
		renderer->getDevice(),
		renderer->getGraphicsQueueFamilyIndex(),
		0
	};

	XrSessionCreateInfo sessionCreateInfo {
		XR_TYPE_SESSION_CREATE_INFO,
		&graphicsBinding,
		0,
		xr_system
	};

	XrResult sessionInit = xrCreateSession(xr_instance, &sessionCreateInfo, &xr_session);
	if (sessionInit != XR_SUCCESS) {
		mprintf(("Failed to create OpenXR session with code %d\n", static_cast<int>(sessionInit)));
		return false;
	}

	return true;
}

int64_t vulkan_openxr_get_swapchain_format(const SCP_vector<int64_t>& allowed)
{
	// Prefer the format of our own composition image, which makes the blit into
	// the swapchain a straight copy with no conversion.
	const int64_t preferred[] = {
		static_cast<int64_t>(VK_FORMAT_R16G16B16A16_SFLOAT),
		static_cast<int64_t>(VK_FORMAT_B8G8R8A8_UNORM),
	};

	for (int64_t format : preferred) {
		if (std::find(allowed.cbegin(), allowed.cend(), format) != allowed.cend()) {
			return format;
		}
	}

	return allowed.front();
}

bool vulkan_openxr_acquire_swapchain_buffers()
{
	for (uint32_t i = 0; i < 2; i++) {
		uint32_t imageCount = 0;

		XrResult swapchainAcq = xrEnumerateSwapchainImages(xr_swapchains[i]->swapchain, 0, &imageCount, nullptr);
		if (swapchainAcq != XR_SUCCESS) {
			mprintf(("Failed to acquire OpenXR swapchain %d with code %d\n", i, static_cast<int>(swapchainAcq)));
			return false;
		}

		Xr_swapchain_images[i] = SCP_vector<XrSwapchainImageVulkan2KHR>(imageCount,
			{ XR_TYPE_SWAPCHAIN_IMAGE_VULKAN2_KHR, nullptr, VK_NULL_HANDLE });

		swapchainAcq = xrEnumerateSwapchainImages(xr_swapchains[i]->swapchain,
			imageCount,
			&imageCount,
			(XrSwapchainImageBaseHeader*)Xr_swapchain_images[i].data());
		if (swapchainAcq != XR_SUCCESS) {
			mprintf(("Failed to acquire OpenXR swapchain %d with code %d\n", i, static_cast<int>(swapchainAcq)));
			return false;
		}
	}

	return true;
}

bool vulkan_openxr_flip()
{
	if (!openxr_enabled()) {
		return false;
	}

	auto* renderer = getRendererInstance();

	static bool was_multiframe = false;

	switch (xr_stage) {
	case OpenXRFBStage::FIRST:
		// openxr_start_frame() already ran in openxr_start_stereo_frame().
		was_multiframe = true;

		render_eye(renderer, 0);

		// The right eye is rendered into the same composition image over the
		// next gr_flip() cycle, so start it from a clean slate rather than on
		// top of the left eye's picture.
		renderer->restartCompositionPass();

		xr_stage = OpenXRFBStage::SECOND;
		return true;

	case OpenXRFBStage::SECOND:
		render_eye(renderer, 1);

		end_frame(renderer, true);
		break;

	case OpenXRFBStage::NONE:
		// A flat frame that nothing has told OpenXR about yet.
		openxr_start_frame();

		if (was_multiframe) {
			openxr_reset_offset();
			was_multiframe = false;
		}

		draw_cursor();

		render_eye(renderer, 0);

		end_frame(renderer, false);
		break;
	}

	xr_stage = OpenXRFBStage::NONE;

	// gr_flip() skips its own page flip when we return true, so the new frame
	// has to be opened here instead.
	gr_setup_frame();

	return true;
}

#else
	// Stubs for builds without OpenXR support.

	SCP_vector<const char*> vulkan_openxr_get_extensions() { return SCP_vector<const char*>{}; }

	bool vulkan_openxr_test_capabilities() { return false; }

	bool vulkan_openxr_create_session() { return false; }

	int64_t vulkan_openxr_get_swapchain_format(const SCP_vector<int64_t>& /*allowed*/) { return 0; }

	bool vulkan_openxr_acquire_swapchain_buffers() { return false; }

	bool vulkan_openxr_flip() { return false; }
#endif

vk::UniqueInstance vulkan_openxr_create_instance(const vk::InstanceCreateInfo& createInfo)
{
#ifdef FS_OPENXR
	// Must be queried before the instance is created, per the extension.
	XrGraphicsRequirementsVulkan2KHR requirements{
		XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR,
		nullptr,
		0,
		0
	};

	auto reqResult = openxr_callExtensionFunction<PFN_xrGetVulkanGraphicsRequirements2KHR>(
		"xrGetVulkanGraphicsRequirements2KHR", xr_instance, xr_system, &requirements);

	if (!reqResult.has_value() || *reqResult != XR_SUCCESS) {
		mprintf(("Failed to query OpenXR graphics requirements!\n"));
		return vk::createInstanceUnique(createInfo, nullptr);
	}

	const XrVersion ourVersion = XR_MAKE_VERSION(VK_API_VERSION_MAJOR(VulkanApiVersion),
		VK_API_VERSION_MINOR(VulkanApiVersion),
		0);

	if (ourVersion < requirements.minApiVersionSupported || ourVersion > requirements.maxApiVersionSupported) {
		mprintf(("System doesn't meet OpenXR graphics requirements (min %" PRIu64 ", max %" PRIu64 ", using %" PRIu64 ")!\n",
			requirements.minApiVersionSupported,
			requirements.maxApiVersionSupported,
			ourVersion));
		return vk::createInstanceUnique(createInfo, nullptr);
	}

	XrVulkanInstanceCreateInfoKHR xrCreateInfo{
		XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR,
		nullptr,
		xr_system,
		0,
		reinterpret_cast<PFN_vkGetInstanceProcAddr>(SDL_Vulkan_GetVkGetInstanceProcAddr()),
		reinterpret_cast<const VkInstanceCreateInfo*>(&createInfo),
		nullptr
	};

	VkInstance rawInstance = VK_NULL_HANDLE;
	VkResult vulkanResult = VK_SUCCESS;

	auto created = openxr_callExtensionFunction<PFN_xrCreateVulkanInstanceKHR>(
		"xrCreateVulkanInstanceKHR", xr_instance, &xrCreateInfo, &rawInstance, &vulkanResult);

	if (!created.has_value() || *created != XR_SUCCESS || vulkanResult != VK_SUCCESS) {
		mprintf(("OpenXR failed to create the Vulkan instance (xr %d, vk %d)!\n",
			created.has_value() ? static_cast<int>(*created) : -1,
			static_cast<int>(vulkanResult)));
		return vk::createInstanceUnique(createInfo, nullptr);
	}

	Xr_graphics_requirements_met = true;

	const vk::detail::ObjectDestroy<vk::detail::NoParent, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> deleter(nullptr,
		VULKAN_HPP_DEFAULT_DISPATCHER);
	return vk::UniqueInstance(vk::Instance(rawInstance), deleter);
#else
	return vk::createInstanceUnique(createInfo, nullptr);
#endif
}

vk::PhysicalDevice vulkan_openxr_required_physical_device([[maybe_unused]] vk::Instance instance)
{
#ifdef FS_OPENXR
	if (openxr_requested() && Xr_graphics_requirements_met) {
		XrVulkanGraphicsDeviceGetInfoKHR getInfo{
			XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR,
			nullptr,
			xr_system,
			instance
		};

		VkPhysicalDevice rawDevice = VK_NULL_HANDLE;
		auto result = openxr_callExtensionFunction<PFN_xrGetVulkanGraphicsDevice2KHR>(
			"xrGetVulkanGraphicsDevice2KHR", xr_instance, &getInfo, &rawDevice);

		if (result.has_value() && *result == XR_SUCCESS && rawDevice != VK_NULL_HANDLE) {
			vk::PhysicalDevice device(rawDevice);
			mprintf(("OpenXR requires Vulkan device %s.\n", device.getProperties().deviceName.data()));
			return device;
		}

		mprintf(("Failed to query the Vulkan device required by OpenXR!\n"));
	}
#endif

	return {};
}

vk::UniqueDevice vulkan_openxr_create_device(vk::PhysicalDevice physicalDevice, const vk::DeviceCreateInfo& createInfo)
{
#ifdef FS_OPENXR
	if (Xr_graphics_requirements_met) {
		XrVulkanDeviceCreateInfoKHR xrCreateInfo{
			XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR,
			nullptr,
			xr_system,
			0,
			reinterpret_cast<PFN_vkGetInstanceProcAddr>(SDL_Vulkan_GetVkGetInstanceProcAddr()),
			physicalDevice,
			reinterpret_cast<const VkDeviceCreateInfo*>(&createInfo),
			nullptr
		};

		VkDevice rawDevice = VK_NULL_HANDLE;
		VkResult vulkanResult = VK_SUCCESS;

		auto created = openxr_callExtensionFunction<PFN_xrCreateVulkanDeviceKHR>(
			"xrCreateVulkanDeviceKHR", xr_instance, &xrCreateInfo, &rawDevice, &vulkanResult);

		if (created.has_value() && *created == XR_SUCCESS && vulkanResult == VK_SUCCESS) {
			const vk::detail::ObjectDestroy<vk::detail::NoParent, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> deleter(nullptr, VULKAN_HPP_DEFAULT_DISPATCHER);
			return vk::UniqueDevice(vk::Device(rawDevice), deleter);
		}

		// Falling back to a device the runtime did not create means no VR, but
		// it is better than failing to start the game at all.
		mprintf(("OpenXR failed to create the Vulkan device (xr %d, vk %d)!\n",
			created.has_value() ? static_cast<int>(*created) : -1,
			static_cast<int>(vulkanResult)));
		Xr_graphics_requirements_met = false;
	}
#endif
	return physicalDevice.createDeviceUnique(createInfo);
}

} // namespace graphics::vulkan
