
#include "VulkanRenderer.h"
#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

#include "cmdline/cmdline.h"
#include "globalincs/version.h"
#include "graphics/grinternal.h"
#include "graphics/post_processing.h"

#include "backends/imgui_impl_vulkan.h"
#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "libs/renderdoc/renderdoc.h"
#include "mod_table/mod_table.h"

#if SDL_VERSION_ATLEAST(2, 0, 6)
#include <SDL_vulkan.h>
#endif


extern float flFrametime;

namespace graphics::vulkan {

namespace {
#if SDL_SUPPORTS_VULKAN
const char* EngineName = "FreeSpaceOpen";

const gameversion::version MinVulkanVersion(1, 1, 0, 0);

VkBool32 VKAPI_PTR debugReportCallback(
	vk::DebugReportFlagsEXT /*flags*/,
	vk::DebugReportObjectTypeEXT /*objectType*/,
	uint64_t /*object*/,
	size_t /*location*/,
	int32_t /*messageCode*/,
	const char* pLayerPrefix,
	const char* pMessage,
	void* /*pUserData*/)
{
	nprintf(("vulkan", "Vulkan message: [%s]: %s\n", pLayerPrefix, pMessage));
	return VK_FALSE;
}

VkBool32 VKAPI_PTR debugUtilsMessengerCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT /*types*/,
	const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
	void* /*pUserData*/)
{
	const char* msgId = (callbackData != nullptr && callbackData->pMessageIdName != nullptr)
	                        ? callbackData->pMessageIdName : "unknown";
	const char* msg = (callbackData != nullptr && callbackData->pMessage != nullptr)
	                      ? callbackData->pMessage : "";

	if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError ||
		severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
		// Errors and warnings (including all -gr_sync_validation hazards) go to the
		// main log so they are visible without the "vulkan" debug filter enabled.
		mprintf(("Vulkan validation: [%s]: %s\n", msgId, msg));
	} else {
		nprintf(("vulkan", "Vulkan message: [%s]: %s\n", msgId, msg));
	}
	return VK_FALSE;
}
#endif

const SCP_vector<const char*> RequiredDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

bool checkDeviceExtensionSupport(PhysicalDeviceValues& values)
{
	auto exts = values.device.enumerateDeviceExtensionProperties();
	values.extensions.assign(exts.begin(), exts.end());

	std::set<std::string> requiredExtensions(RequiredDeviceExtensions.cbegin(), RequiredDeviceExtensions.cend());
	for (const auto& extension : values.extensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool checkSwapChainSupport(PhysicalDeviceValues& values, const vk::UniqueSurfaceKHR& surface)
{
	values.surfaceCapabilities = values.device.getSurfaceCapabilitiesKHR(surface.get());
	auto fmts = values.device.getSurfaceFormatsKHR(surface.get());
	values.surfaceFormats.assign(fmts.begin(), fmts.end());
	auto modes = values.device.getSurfacePresentModesKHR(surface.get());
	values.presentModes.assign(modes.begin(), modes.end());

	return !values.surfaceFormats.empty() && !values.presentModes.empty();
}

bool isDeviceUnsuitable(PhysicalDeviceValues& values, const vk::UniqueSurfaceKHR& surface)
{
	// We need a GPU. Reject CPU or "other" types.
	if (values.properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu &&
		values.properties.deviceType != vk::PhysicalDeviceType::eIntegratedGpu &&
		values.properties.deviceType != vk::PhysicalDeviceType::eVirtualGpu) {
		nprintf(("vulkan", "Rejecting %s (%d) because the device type is unsuitable.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}

	uint32_t i = 0;
	for (const auto& queue : values.queueProperties) {
		if (queue.queueFlags & vk::QueueFlagBits::eGraphics) {
			if (!values.graphicsQueueIndex.initialized) {
				values.graphicsQueueIndex.initialized = true;
				values.graphicsQueueIndex.index = i;
			}
			// "All commands that are allowed on a queue that supports transfer operations
			// are also allowed on a queue that supports either graphics or compute operations
			if (!values.transferQueueIndex.initialized) {
				values.transferQueueIndex.initialized = true;
				values.transferQueueIndex.index = i;
			}
		}
		if (queue.queueFlags & vk::QueueFlagBits::eTransfer &&
			!(queue.queueFlags & vk::QueueFlagBits::eGraphics) &&
			!(queue.queueFlags & vk::QueueFlagBits::eCompute)) {
			// Found a dedicated transfer queue and we prefer that
			values.transferQueueIndex.initialized = true;
			values.transferQueueIndex.index = i;
		}
		if (!values.presentQueueIndex.initialized && values.device.getSurfaceSupportKHR(i, surface.get())) {
			values.presentQueueIndex.initialized = true;
			values.presentQueueIndex.index = i;
		}

		++i;
	}

	if (!values.graphicsQueueIndex.initialized) {
		nprintf(("vulkan", "Rejecting %s (%d) because the device does not have a graphics queue.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}
	if (!values.transferQueueIndex.initialized) {
		nprintf(("vulkan", "Rejecting %s (%d) because the device does not have a transfer queue.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}
	if (!values.presentQueueIndex.initialized) {
		nprintf(("vulkan", "Rejecting %s (%d) because the device does not have a presentation queue.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}

	if (!checkDeviceExtensionSupport(values)) {
		nprintf(("vulkan", "Rejecting %s (%d) because the device does not support our required extensions.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}

	if (!checkSwapChainSupport(values, surface)) {
		nprintf(("vulkan", "Rejecting %s (%d) because the device swap chain was not compatible.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}

	return false;
}

uint32_t deviceTypeScore(vk::PhysicalDeviceType type)
{
	switch (type) {
	case vk::PhysicalDeviceType::eIntegratedGpu:
		return 1;
	case vk::PhysicalDeviceType::eDiscreteGpu:
		return 2;
	case vk::PhysicalDeviceType::eVirtualGpu:
	case vk::PhysicalDeviceType::eCpu:
	case vk::PhysicalDeviceType::eOther:
	default:
		return 0;
	}
}

uint64_t scoreDevice(const PhysicalDeviceValues& device)
{
	// Device type is the primary selection criterion (a discrete GPU must always
	// beat an integrated one). The API version is only a tiebreaker between
	// devices of the same type. We pack the type into the high bits and the
	// (already 32-bit) API version into the low bits so the type can never be
	// overwhelmed by the magnitude of the packed apiVersion value.
	uint64_t score = 0;

	score |= static_cast<uint64_t>(deviceTypeScore(device.properties.deviceType)) << 32;
	score |= static_cast<uint64_t>(device.properties.apiVersion);

	return score;
}

bool compareDevices(const PhysicalDeviceValues& left, const PhysicalDeviceValues& right)
{
	return scoreDevice(left) < scoreDevice(right);
}

void printPhysicalDevice(const PhysicalDeviceValues& values)
{
	nprintf(("vulkan", "  Found %s (%d) of type %s. API version %d.%d.%d, Driver version %d.%d.%d. Scored as %" PRIu64 "\n",
		values.properties.deviceName.data(),
		values.properties.deviceID,
		to_string(values.properties.deviceType).c_str(),
		VK_VERSION_MAJOR(values.properties.apiVersion),
		VK_VERSION_MINOR(values.properties.apiVersion),
		VK_VERSION_PATCH(values.properties.apiVersion),
		VK_VERSION_MAJOR(values.properties.driverVersion),
		VK_VERSION_MINOR(values.properties.driverVersion),
		VK_VERSION_PATCH(values.properties.driverVersion),
		scoreDevice(values)));
}

vk::SurfaceFormatKHR chooseSurfaceFormat(const PhysicalDeviceValues& values)
{
	// When HDR output is requested, prefer a 10-bit HDR10 (PQ / ST.2084) surface
	// using BT.2020 primaries. The final output-encode pass writes PQ-encoded
	// BT.2020 values into this surface.
	if (Gr_enable_hdr) {
		for (const auto& availableFormat : values.surfaceFormats) {
			if ((availableFormat.format == vk::Format::eA2B10G10R10UnormPack32 ||
			     availableFormat.format == vk::Format::eA2R10G10B10UnormPack32) &&
			    availableFormat.colorSpace == vk::ColorSpaceKHR::eHdr10St2084EXT) {
				nprintf(("vulkan", "Vulkan: Selected HDR10 surface (10-bit, ST.2084/BT.2020)\n"));
				return availableFormat;
			}
		}
		nprintf(("vulkan", "Vulkan: HDR requested but no HDR10 surface format available; falling back to SDR\n"));
	}

	// Use a non-sRGB (UNORM) format to match OpenGL's default framebuffer behavior.
	// The FSO shaders handle gamma correction manually in the fragment shader and
	// post-processing pipeline, so hardware sRGB conversion would double-correct.
	for (const auto& availableFormat : values.surfaceFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
			availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

	return values.surfaceFormats.front();
}

vk::PresentModeKHR choosePresentMode(const PhysicalDeviceValues& values)
{
	vk::PresentModeKHR chosen = vk::PresentModeKHR::eFifo; // guaranteed to be supported

	// Depending on if we want Vsync or not, choose the best mode
	for (const auto& availablePresentMode : values.presentModes) {
		if (Gr_enable_vsync) {
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
				chosen = availablePresentMode;
				break;
			}
		} else {
			if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
				chosen = availablePresentMode;
				break;
			}
		}
	}

	const char* name = "Unknown";
	switch (chosen) {
		case vk::PresentModeKHR::eImmediate:    name = "Immediate"; break;
		case vk::PresentModeKHR::eMailbox:       name = "Mailbox"; break;
		case vk::PresentModeKHR::eFifo:          name = "FIFO (vsync)"; break;
		case vk::PresentModeKHR::eFifoRelaxed:   name = "FIFO Relaxed"; break;
		default: break;
	}
	nprintf(("vulkan", "Vulkan: Present mode: %s (Gr_enable_vsync=%d)\n", name, Gr_enable_vsync ? 1 : 0));

	return chosen;
}

vk::Extent2D chooseSwapChainExtent(const PhysicalDeviceValues& values, uint32_t width, uint32_t height)
{
	if (values.surfaceCapabilities.currentExtent.width != UINT32_MAX) {
		return values.surfaceCapabilities.currentExtent;
	} else {
		VkExtent2D actualExtent = {width, height};

		actualExtent.width = std::max(values.surfaceCapabilities.minImageExtent.width,
			std::min(values.surfaceCapabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(values.surfaceCapabilities.minImageExtent.height,
			std::min(values.surfaceCapabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

} // namespace
bool VulkanRenderer::initialize()
{
	nprintf(("vulkan", "Initializing Vulkan graphics device at %ix%i with %i-bit color...\n",
		gr_screen.max_w,
		gr_screen.max_h,
		gr_screen.bits_per_pixel));

	// Load the RenderDoc API if available before doing anything with OpenGL
	renderdoc::loadApi();

	if (!initDisplayDevice()) {
		return false;
	}

	if (!initializeInstance()) {
		nprintf(("vulkan", "Failed to create Vulkan instance!\n"));
		return false;
	}

	if (!initializeSurface()) {
		nprintf(("vulkan", "Failed to create Vulkan surface!\n"));
		return false;
	}

	PhysicalDeviceValues deviceValues;
	if (!pickPhysicalDevice(deviceValues)) {
		nprintf(("vulkan", "Could not find suitable physical Vulkan device.\n"));
		return false;
	}

	// Validate MSAA sample count against device limits
	if (Cmdline_msaa_enabled > 0) {
		auto limits = deviceValues.properties.limits;
		vk::SampleCountFlags supported = limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts;

		// Map requested count to vk::SampleCountFlagBits
		vk::SampleCountFlagBits requested = vk::SampleCountFlagBits::e1;
		switch (Cmdline_msaa_enabled) {
		case 4:  requested = vk::SampleCountFlagBits::e4; break;
		case 8:  requested = vk::SampleCountFlagBits::e8; break;
		case 16: requested = vk::SampleCountFlagBits::e16; break;
		default:
			nprintf(("vulkan", "Vulkan: Unsupported MSAA count %d, disabling MSAA\n", Cmdline_msaa_enabled));
			Cmdline_msaa_enabled = 0;
			break;
		}

		if (Cmdline_msaa_enabled > 0) {
			if (supported & requested) {
				m_msaaSampleCount = requested;
				nprintf(("vulkan", "Vulkan: MSAA enabled with %dx sample count\n", Cmdline_msaa_enabled));
			} else {
				// Clamp down to highest supported
				vk::SampleCountFlagBits fallback = vk::SampleCountFlagBits::e1;
				int fallbackCount = 0;
				if ((supported & vk::SampleCountFlagBits::e8) && Cmdline_msaa_enabled >= 8) {
					fallback = vk::SampleCountFlagBits::e8; fallbackCount = 8;
				} else if (supported & vk::SampleCountFlagBits::e4) {
					fallback = vk::SampleCountFlagBits::e4; fallbackCount = 4;
				}

				if (fallbackCount > 0) {
					nprintf(("vulkan", "Vulkan: Requested MSAA %dx not supported, falling back to %dx\n",
						Cmdline_msaa_enabled, fallbackCount));
					Cmdline_msaa_enabled = fallbackCount;
					m_msaaSampleCount = fallback;
				} else {
					nprintf(("vulkan", "Vulkan: No suitable MSAA support, disabling MSAA\n"));
					Cmdline_msaa_enabled = 0;
				}
			}
		}
	}

	if (!createLogicalDevice(deviceValues)) {
		nprintf(("vulkan", "Failed to create logical device.\n"));
		return false;
	}

	createCommandPool(deviceValues);

	if (!createSwapChain(deviceValues)) {
		nprintf(("vulkan", "Failed to create swap chain.\n"));
		return false;
	}

	createDepthResources();
	createCompositionResources();
	createEncodeRenderPass();
	createRenderPass();
	createFrameBuffers();

	createPresentSyncObjects();

	// Initialize texture manager (needs command pool for uploads)
	m_textureManager = std::unique_ptr<VulkanTextureManager>(new VulkanTextureManager());
	if (!m_textureManager->init(m_device.get(), m_physicalDevice, m_memoryManager.get(),
	                            m_graphicsCommandPool.get(), m_graphicsQueue)) {
		nprintf(("vulkan", "Failed to initialize Vulkan texture manager!\n"));
		return false;
	}
	setTextureManager(m_textureManager.get());

	// Initialize shader manager
	m_shaderManager = std::unique_ptr<VulkanShaderManager>(new VulkanShaderManager());
	if (!m_shaderManager->init(m_device.get())) {
		nprintf(("vulkan", "Failed to initialize Vulkan shader manager!\n"));
		return false;
	}
	setShaderManager(m_shaderManager.get());

	// Initialize raytraced shadow BLAS/TLAS manager (no-op internally if unsupported).
	// Must happen before the descriptor manager below, since the Global set's
	// layout needs to know whether to include the TLAS binding, and the
	// descriptor fallbacks need the raytracing manager's fallback TLAS.
	m_raytracingManager = std::unique_ptr<VulkanRaytracingManager>(new VulkanRaytracingManager());
	bool raytracingReady = m_raytracingManager->init(m_device.get(), m_physicalDevice, m_memoryManager.get(),
	                                                  m_bufferManager.get(), m_graphicsCommandPool.get(),
	                                                  m_graphicsQueue, m_supportsRaytracedShadows);
	if (!raytracingReady) {
		if (m_supportsRaytracedShadows) {
			mprintf(("Warning: Failed to initialize Vulkan raytracing manager, raytraced shadows will be disabled\n"));
		}
		m_raytracingManager.reset();
	} else {
		setRaytracingManager(m_raytracingManager.get());
	}
	bool rtDescriptorSupport = m_raytracingManager && m_raytracingManager->isEnabled();

	// Keep the public capability query (gr_is_capable(CAPABILITY_RAYTRACED_SHADOWS),
	// used to decide the shader-side RT path) in lockstep with whether the Global
	// set's layout actually declares the TLAS binding. m_supportsRaytracedShadows
	// was originally set from device-extension negotiation alone; downgrade it here
	// if the raytracing manager itself failed to come up (e.g. fallback TLAS build
	// failure) so the two can never disagree about whether binding 5 exists.
	m_supportsRaytracedShadows = rtDescriptorSupport;

	// Initialize descriptor manager
	m_descriptorManager = std::unique_ptr<VulkanDescriptorManager>(new VulkanDescriptorManager());
	if (!m_descriptorManager->init(m_device.get(), rtDescriptorSupport)) {
		mprintf(("Failed to initialize Vulkan descriptor manager!\n"));
		return false;
	}
	setDescriptorManager(m_descriptorManager.get());
	m_descriptorManager->buildFallbacks(m_bufferManager.get(), m_textureManager.get(), m_raytracingManager.get());

	// Initialize pipeline manager
	m_pipelineManager = std::unique_ptr<VulkanPipelineManager>(new VulkanPipelineManager());
	if (!m_pipelineManager->init(m_device.get(), m_shaderManager.get(), m_descriptorManager.get())) {
		nprintf(("vulkan", "Failed to initialize Vulkan pipeline manager!\n"));
		return false;
	}
	setPipelineManager(m_pipelineManager.get());
	m_pipelineManager->loadPipelineCache("vulkan_pipeline.cache");

	// Initialize state tracker
	m_stateTracker = std::unique_ptr<VulkanStateTracker>(new VulkanStateTracker());
	if (!m_stateTracker->init(m_device.get())) {
		nprintf(("vulkan", "Failed to initialize Vulkan state tracker!\n"));
		return false;
	}
	setStateTracker(m_stateTracker.get());

	// Initialize draw manager
	m_drawManager = std::unique_ptr<VulkanDrawManager>(new VulkanDrawManager());
	if (!m_drawManager->init(m_device.get())) {
		nprintf(("vulkan", "Failed to initialize Vulkan draw manager!\n"));
		return false;
	}
	setDrawManager(m_drawManager.get());

	// Initialize post-processing
	m_postProcessor = std::unique_ptr<VulkanPostProcessor>(new VulkanPostProcessor());
	if (!m_postProcessor->init(m_device.get(), m_physicalDevice, m_memoryManager.get(),
	                           m_swapChainExtent, m_depthFormat, m_hdrActive)) {
		mprintf(("Warning: Failed to initialize Vulkan post-processor, post-processing will be disabled\n"));
		m_postProcessor.reset();
	} else {
		setPostProcessor(m_postProcessor.get());
	}

	// Initialize shared post-processing manager (bloom/lightshaft settings, post-effect table)
	// This is renderer-agnostic; OpenGL creates it in opengl_post_process_init().
	if (!graphics::Post_processing_manager) {
		graphics::Post_processing_manager.reset(new graphics::PostProcessingManager());
		if (!graphics::Post_processing_manager->parse_table()) {
			nprintf(("vulkan", "Warning: Unable to read post-processing table\n"));
		}
	}

	// Initialize shared uniform buffer managers; this is renderer-agnostic and OpenGL
	// creates it in gr_opengl_init().
	gr_uniform_buffer_managers_init();

	// Initialize query manager for GPU timestamp profiling
	m_queryManager = std::unique_ptr<VulkanQueryManager>(new VulkanQueryManager());
	if (!m_queryManager->init(m_device.get(), m_physicalDevice.getProperties().limits.timestampPeriod,
	                          m_graphicsCommandPool.get(), m_graphicsQueue)) {
		nprintf(("vulkan", "Warning: Failed to initialize Vulkan query manager, GPU profiling will be disabled\n"));
		m_queryManager.reset();
	} else {
		setQueryManager(m_queryManager.get());
	}

	// Prepare the rendering state by acquiring our first swap chain image
	acquireNextSwapChainImage();

	// Initialize ImGui Vulkan rendering backend
	initImGui();

	return true;
}

bool VulkanRenderer::initDisplayDevice() const
{
	os::ViewPortProperties attrs;
	attrs.enable_opengl = false;
	attrs.enable_vulkan = true;

	attrs.display = os_config_read_uint("Video", "Display", 0);
	attrs.width = static_cast<uint32_t>(gr_screen.max_w);
	attrs.height = static_cast<uint32_t>(gr_screen.max_h);

	attrs.title = Osreg_title;
	if (!Window_title.empty()) {
		attrs.title = Window_title;
	}

	if (Using_in_game_options) {
		switch (Gr_configured_window_state) {
		case os::ViewportState::Windowed:
			// That's the default
			break;
		case os::ViewportState::Borderless:
			attrs.flags.set(os::ViewPortFlags::Borderless);
			break;
		case os::ViewportState::Fullscreen:
			attrs.flags.set(os::ViewPortFlags::Fullscreen);
			break;
		}
	} else {
		if (!Cmdline_window && !Cmdline_fullscreen_window) {
			attrs.flags.set(os::ViewPortFlags::Fullscreen);
		} else if (Cmdline_fullscreen_window) {
			attrs.flags.set(os::ViewPortFlags::Borderless);
		}
	}

	if (Cmdline_capture_mouse)
		attrs.flags.set(os::ViewPortFlags::Capture_Mouse);

	auto viewPort = m_graphicsOps->createViewport(attrs);
	if (!viewPort) {
		return false;
	}

	const auto port = os::addViewport(std::move(viewPort));
	os::setMainViewPort(port);

	return true;
}
bool VulkanRenderer::initializeInstance()
{
#if SDL_SUPPORTS_VULKAN
	const auto vkGetInstanceProcAddr =
		reinterpret_cast<PFN_vkGetInstanceProcAddr>(SDL_Vulkan_GetVkGetInstanceProcAddr());

	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	const auto window = os::getSDLMainWindow();

	unsigned int count;
	if (!SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr)) {
		nprintf(("vulkan", "Error in first SDL_Vulkan_GetInstanceExtensions: %s\n", SDL_GetError()));
		return false;
	}

	SCP_vector<const char*> extensions;
	extensions.resize(count);

	if (!SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data())) {
		nprintf(("vulkan", "Error in second SDL_Vulkan_GetInstanceExtensions: %s\n", SDL_GetError()));
		return false;
	}

	const auto instanceVersion = vk::enumerateInstanceVersion();
	gameversion::version vulkanVersion(VK_VERSION_MAJOR(instanceVersion),
		VK_VERSION_MINOR(instanceVersion),
		VK_VERSION_PATCH(instanceVersion),
		0);
	nprintf(("vulkan", "Vulkan instance version %s\n", gameversion::format_version(vulkanVersion).c_str()));

	if (vulkanVersion < MinVulkanVersion) {
		nprintf(("vulkan", "Vulkan version is less than the minimum which is %s.\n",
			gameversion::format_version(MinVulkanVersion).c_str()));
		return false;
	}

	const auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
	nprintf(("vulkan", "Instance extensions:\n"));
	for (const auto& ext : supportedExtensions) {
		mprintf(("  Found support for %s version %" PRIu32 "\n", ext.extensionName.data(), ext.specVersion));
		// Enables the driver to advertise HDR color spaces (e.g. HDR10 ST.2084)
		// in vkGetPhysicalDeviceSurfaceFormatsKHR. Required to negotiate an HDR
		// swap chain. Harmless to enable even when HDR output is disabled.
		if (!stricmp(ext.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)) {
			extensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
			mprintf(("  Enabling %s (HDR color space support)\n", VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME));
		}
		if (FSO_DEBUG || Cmdline_graphics_debug_output) {
			if (!stricmp(ext.extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				m_debugReportEnabled = true;
			}
			if (!stricmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				m_debugUtilsEnabled = true;
			}
		}
	}

	SCP_vector<const char*> layers;
	bool khronosValidationEnabled = false;
	const auto supportedLayers = vk::enumerateInstanceLayerProperties();
	nprintf(("vulkan", "Instance layers:\n"));
	for (const auto& layer : supportedLayers) {
		nprintf(("vulkan", "  Found layer %s(%s). Spec version %d.%d.%d and implementation %" PRIu32 "\n",
			layer.layerName.data(),
			layer.description.data(),
			VK_VERSION_MAJOR(layer.specVersion),
			VK_VERSION_MINOR(layer.specVersion),
			VK_VERSION_PATCH(layer.specVersion),
			layer.implementationVersion));
		if (FSO_DEBUG || Cmdline_graphics_debug_output) {
			if (!stricmp(layer.layerName, "VK_LAYER_KHRONOS_validation")) {
				layers.push_back("VK_LAYER_KHRONOS_validation");
				khronosValidationEnabled = true;
			} else if (!stricmp(layer.layerName, "VK_LAYER_LUNARG_core_validation")) {
				layers.push_back("VK_LAYER_LUNARG_core_validation");
			}
		}
	}

	vk::ApplicationInfo appInfo(Window_title.c_str(), 1, EngineName, 1, VulkanApiVersion);

	// Now we can make the Vulkan instance
	vk::InstanceCreateInfo createInfo(vk::Flags<vk::InstanceCreateFlagBits>(), &appInfo);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	createInfo.ppEnabledLayerNames = layers.data();

	vk::DebugReportCallbackCreateInfoEXT createInstanceReportInfo(vk::DebugReportFlagBitsEXT::eError |
																  vk::DebugReportFlagBitsEXT::eWarning |
																  vk::DebugReportFlagBitsEXT::ePerformanceWarning);
	createInstanceReportInfo.pfnCallback = debugReportCallback;

	// Debug-utils messenger config. Chained into instance creation (so messages
	// emitted during vkCreateInstance itself are captured) and reused below for
	// the persistent messenger.
	vk::DebugUtilsMessengerCreateInfoEXT messengerInfo;
	messengerInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
	                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
	messengerInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
	                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
	                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	messengerInfo.pfnUserCallback = debugUtilsMessengerCallback;

	// Opt-in GPU synchronization validation (-gr_sync_validation). The struct is
	// consumed by the Khronos validation layer, so it is only chained when that
	// layer was actually enabled above.
	const std::array<vk::ValidationFeatureEnableEXT, 1> enabledValidationFeatures = {
		vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
	};
	vk::ValidationFeaturesEXT validationFeatures;
	validationFeatures.enabledValidationFeatureCount = static_cast<uint32_t>(enabledValidationFeatures.size());
	validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures.data();

	vk::StructureChain<vk::InstanceCreateInfo,
		vk::DebugReportCallbackCreateInfoEXT,
		vk::DebugUtilsMessengerCreateInfoEXT,
		vk::ValidationFeaturesEXT>
		createInstanceChain(createInfo, createInstanceReportInfo, messengerInfo, validationFeatures);

	// Prefer the debug-utils messenger; the deprecated debug-report callback is
	// only a fallback for loaders without VK_EXT_debug_utils.
	if (m_debugUtilsEnabled || !m_debugReportEnabled) {
		createInstanceChain.unlink<vk::DebugReportCallbackCreateInfoEXT>();
	}
	if (!m_debugUtilsEnabled) {
		createInstanceChain.unlink<vk::DebugUtilsMessengerCreateInfoEXT>();
	}
	if (Cmdline_gr_sync_validation && khronosValidationEnabled) {
		mprintf(("Vulkan: Synchronization validation enabled (-gr_sync_validation)\n"));
	} else {
		if (Cmdline_gr_sync_validation) {
			mprintf(("Vulkan: -gr_sync_validation requested but VK_LAYER_KHRONOS_validation is not available; "
			         "synchronization validation disabled\n"));
		}
		createInstanceChain.unlink<vk::ValidationFeaturesEXT>();
	}

	vk::UniqueInstance instance = vk::createInstanceUnique(createInstanceChain.get<vk::InstanceCreateInfo>(), nullptr);
	if (!instance) {
		return false;
	}

	VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

	if (m_debugUtilsEnabled) {
		m_debugMessenger = instance->createDebugUtilsMessengerEXTUnique(messengerInfo);
	} else if (m_debugReportEnabled) {
		vk::DebugReportCallbackCreateInfoEXT reportCreateInfo(vk::DebugReportFlagBitsEXT::eError |
															  vk::DebugReportFlagBitsEXT::eWarning |
															  vk::DebugReportFlagBitsEXT::ePerformanceWarning);
		reportCreateInfo.pfnCallback = debugReportCallback;

		m_debugReport = instance->createDebugReportCallbackEXTUnique(reportCreateInfo);
	}

	m_vkInstance = std::move(instance);
	return true;
#else
	nprintf(("vulkan", "SDL does not support Vulkan in its current version.\n"));
	return false;
#endif
}

bool VulkanRenderer::initializeSurface()
{
#if SDL_SUPPORTS_VULKAN
	const auto window = os::getSDLMainWindow();

	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(*m_vkInstance), &surface)) {
		nprintf(("vulkan", "Failed to create vulkan surface: %s\n", SDL_GetError()));
		return false;
	}

	const vk::detail::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> deleter(*m_vkInstance,
		nullptr,
		VULKAN_HPP_DEFAULT_DISPATCHER);
	m_vkSurface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(surface), deleter);
	return true;
#else
	return false;
#endif
}

bool VulkanRenderer::pickPhysicalDevice(PhysicalDeviceValues& deviceValues)
{
	const auto devices = m_vkInstance->enumeratePhysicalDevices();
	if (devices.empty()) {
		return false;
	}

	SCP_vector<PhysicalDeviceValues> values;
	std::transform(devices.cbegin(), devices.cend(), std::back_inserter(values), [](const vk::PhysicalDevice& dev) {
		PhysicalDeviceValues vals;
		vals.device = dev;
		vals.properties = dev.getProperties2().properties;

		// Query ray tracing feature bits alongside the base features. It is
		// safe to include extension feature structs here even if the device
		// doesn't support the corresponding extension -- this is the standard
		// way to discover whether to request it below. Extension *presence*
		// is checked separately in createLogicalDevice() before enabling.
		auto featureChain = dev.getFeatures2<vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
			vk::PhysicalDeviceRayQueryFeaturesKHR,
			vk::PhysicalDeviceBufferDeviceAddressFeatures>();
		vals.features = featureChain.get<vk::PhysicalDeviceFeatures2>().features;
		vals.accelerationStructureFeatureSupported =
			featureChain.get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>().accelerationStructure != VK_FALSE;
		vals.rayQueryFeatureSupported =
			featureChain.get<vk::PhysicalDeviceRayQueryFeaturesKHR>().rayQuery != VK_FALSE;
		vals.bufferDeviceAddressFeatureSupported =
			featureChain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>().bufferDeviceAddress != VK_FALSE;

		auto qprops = dev.getQueueFamilyProperties();
		vals.queueProperties.assign(qprops.begin(), qprops.end());
		return vals;
	});

	nprintf(("vulkan", "Physical Vulkan devices:\n"));
	std::for_each(values.cbegin(), values.cend(), printPhysicalDevice);

	// Remove devices that do not have the features we need
	values.erase(std::remove_if(values.begin(),
					 values.end(),
					 [this](PhysicalDeviceValues& value) { return isDeviceUnsuitable(value, m_vkSurface); }),
		values.end());
	if (values.empty()) {
		return false;
	}

	// Sort the suitability of the devices in increasing order
	std::sort(values.begin(), values.end(), compareDevices);

	deviceValues = values.back();
	nprintf(("vulkan", "Selected device %s (%d) as the primary Vulkan device.\n",
		deviceValues.properties.deviceName.data(),
		deviceValues.properties.deviceID));
	nprintf(("vulkan", "Device extensions:\n"));
	for (const auto& extProp : deviceValues.extensions) {
		nprintf(("vulkan", "  Found support for %s version %" PRIu32 "\n", extProp.extensionName.data(), extProp.specVersion));
	}

	return true;
}

bool VulkanRenderer::createLogicalDevice(const PhysicalDeviceValues& deviceValues)
{
	float queuePriority = 1.0f;

	SCP_vector<vk::DeviceQueueCreateInfo> queueInfos;
	const std::set<uint32_t> familyIndices{deviceValues.graphicsQueueIndex.index,
	                                       deviceValues.transferQueueIndex.index,
	                                       deviceValues.presentQueueIndex.index};

	queueInfos.reserve(familyIndices.size());
	for (auto index : familyIndices) {
		queueInfos.emplace_back(vk::DeviceQueueCreateFlags(), index, 1, &queuePriority);
	}

	// Build extension list: required + optional
	SCP_vector<const char*> enabledExtensions(RequiredDeviceExtensions.begin(), RequiredDeviceExtensions.end());

	// Check for VK_EXT_shader_viewport_index_layer (needed for shadow cascade routing)
	m_supportsShaderViewportLayerOutput = false;
	m_hdrMetadataSupported = false;
	bool hasAccelerationStructureExt = false;
	bool hasRayQueryExt = false;
	bool hasDeferredHostOperationsExt = false;
	for (const auto& ext : deviceValues.extensions) {
		if (strcmp(ext.extensionName, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0) {
			m_supportsShaderViewportLayerOutput = true;
			enabledExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
			mprintf(("Vulkan: Enabling %s (shadow cascade support)\n", VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME));
		}
		// VK_EXT_hdr_metadata lets us advertise mastering display / content
		// luminance to the compositor for HDR10 output. Optional.
		if (strcmp(ext.extensionName, VK_EXT_HDR_METADATA_EXTENSION_NAME) == 0) {
			m_hdrMetadataSupported = true;
			enabledExtensions.push_back(VK_EXT_HDR_METADATA_EXTENSION_NAME);
			mprintf(("Vulkan: Enabling %s (HDR10 metadata)\n", VK_EXT_HDR_METADATA_EXTENSION_NAME));
		}
		if (strcmp(ext.extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0) {
			hasAccelerationStructureExt = true;
		}
		if (strcmp(ext.extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0) {
			hasRayQueryExt = true;
		}
		if (strcmp(ext.extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0) {
			hasDeferredHostOperationsExt = true;
		}
	}

	// Raytraced shadows need the extension trio present AND their feature bits
	// enabled. bufferDeviceAddress is core in Vulkan 1.2 (no extension name to
	// check for), but its feature bit still has to be queried/enabled
	// explicitly -- core promotion only drops the extension-name requirement.
	m_supportsRaytracedShadows = hasAccelerationStructureExt && hasRayQueryExt && hasDeferredHostOperationsExt &&
		deviceValues.accelerationStructureFeatureSupported && deviceValues.rayQueryFeatureSupported &&
		deviceValues.bufferDeviceAddressFeatureSupported;

	if (m_supportsRaytracedShadows) {
		enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		enabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		enabledExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		mprintf(("Vulkan: Enabling ray tracing extensions (raytraced shadow support)\n"));
	} else {
		nprintf(("vulkan", "Vulkan: Raytraced shadows not available on this device "
			"(extensions: accel_struct=%d ray_query=%d deferred_host_ops=%d; "
			"features: accel_struct=%d ray_query=%d buffer_device_address=%d)\n",
			hasAccelerationStructureExt, hasRayQueryExt, hasDeferredHostOperationsExt,
			deviceValues.accelerationStructureFeatureSupported, deviceValues.rayQueryFeatureSupported,
			deviceValues.bufferDeviceAddressFeatureSupported));
	}

	vk::DeviceCreateInfo deviceCreate;
	deviceCreate.pQueueCreateInfos = queueInfos.data();
	deviceCreate.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());

	deviceCreate.ppEnabledExtensionNames = enabledExtensions.data();
	deviceCreate.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());

	// Device features are enabled via a pNext-chained VkPhysicalDeviceFeatures2
	// rather than the legacy DeviceCreateInfo::pEnabledFeatures, since the spec
	// requires pEnabledFeatures to be null once a VkPhysicalDeviceFeatures2 is
	// linked. The RT feature structs are always linked into the chain and
	// unlinked when unsupported, mirroring the pattern used for the instance's
	// optional debug report callback above.
	vk::PhysicalDeviceFeatures2 deviceFeatures2;
	deviceFeatures2.features = deviceValues.features;

	vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures;
	accelStructFeatures.accelerationStructure = VK_TRUE;

	vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures;
	rayQueryFeatures.rayQuery = VK_TRUE;

	vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures;
	bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

	vk::StructureChain<vk::DeviceCreateInfo,
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
		vk::PhysicalDeviceRayQueryFeaturesKHR,
		vk::PhysicalDeviceBufferDeviceAddressFeatures>
		deviceCreateChain(deviceCreate, deviceFeatures2, accelStructFeatures, rayQueryFeatures, bufferDeviceAddressFeatures);

	if (!m_supportsRaytracedShadows) {
		deviceCreateChain.unlink<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();
		deviceCreateChain.unlink<vk::PhysicalDeviceRayQueryFeaturesKHR>();
		deviceCreateChain.unlink<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
	}

	m_device = deviceValues.device.createDeviceUnique(deviceCreateChain.get<vk::DeviceCreateInfo>());

	// Load device-level function pointers for the dynamic dispatcher
	VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device.get());

	// Create queues
	m_graphicsQueue = m_device->getQueue(deviceValues.graphicsQueueIndex.index, 0);
	m_transferQueue = m_device->getQueue(deviceValues.transferQueueIndex.index, 0);
	m_presentQueue = m_device->getQueue(deviceValues.presentQueueIndex.index, 0);

	// Store physical device and queue family indices for later use
	m_physicalDevice = deviceValues.device;
	m_graphicsQueueFamilyIndex = deviceValues.graphicsQueueIndex.index;
	m_transferQueueFamilyIndex = deviceValues.transferQueueIndex.index;
	m_presentQueueFamilyIndex = deviceValues.presentQueueIndex.index;

	// Initialize memory manager
	m_memoryManager = std::unique_ptr<VulkanMemoryManager>(new VulkanMemoryManager());
	if (!m_memoryManager->init(m_vkInstance.get(), m_physicalDevice, m_device.get(), m_supportsRaytracedShadows)) {
		mprintf(("Failed to initialize Vulkan memory manager!\n"));
		return false;
	}
	setMemoryManager(m_memoryManager.get());

	// Initialize deletion queue for deferred resource destruction
	m_deletionQueue = std::unique_ptr<VulkanDeletionQueue>(new VulkanDeletionQueue());
	m_deletionQueue->init(m_device.get(), m_memoryManager.get());
	setDeletionQueue(m_deletionQueue.get());

	// Initialize buffer manager
	m_bufferManager = std::unique_ptr<VulkanBufferManager>(new VulkanBufferManager());
	if (!m_bufferManager->init(m_device.get(), m_memoryManager.get(),
	                           m_graphicsQueueFamilyIndex, m_transferQueueFamilyIndex,
	                           getMinUniformBufferOffsetAlignment())) {
		nprintf(("vulkan", "Failed to initialize Vulkan buffer manager!\n"));
		return false;
	}
	setBufferManager(m_bufferManager.get());
	// Set initial frame index for buffer manager
	m_bufferManager->setCurrentFrame(m_currentFrame);

	return true;
}
bool VulkanRenderer::createSwapChain(const PhysicalDeviceValues& deviceValues, vk::SwapchainKHR oldSwapchain)
{
	// Choose one more than the minimum to avoid driver synchronization if it is not done with a thread yet
	uint32_t imageCount = deviceValues.surfaceCapabilities.minImageCount + 1;
	if (deviceValues.surfaceCapabilities.maxImageCount > 0 &&
		imageCount > deviceValues.surfaceCapabilities.maxImageCount) {
		imageCount = deviceValues.surfaceCapabilities.maxImageCount;
	}

	const auto surfaceFormat = chooseSurfaceFormat(deviceValues);

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = m_vkSurface.get();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = chooseSwapChainExtent(deviceValues, gr_screen.max_w, gr_screen.max_h);
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment
	                      | vk::ImageUsageFlagBits::eTransferSrc
	                      | vk::ImageUsageFlagBits::eTransferDst;

	const uint32_t queueFamilyIndices[] = {deviceValues.graphicsQueueIndex.index, deviceValues.presentQueueIndex.index};
	if (deviceValues.graphicsQueueIndex.index != deviceValues.presentQueueIndex.index) {
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}

	createInfo.preTransform = deviceValues.surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = choosePresentMode(deviceValues);
	createInfo.clipped = true;
	createInfo.oldSwapchain = oldSwapchain;

	auto newSwapChain = m_device->createSwapchainKHRUnique(createInfo);

	// Clear old resources before replacing the swap chain
	m_swapChainFramebuffers.clear();
	m_swapChainImageViews.clear();

	m_swapChain = std::move(newSwapChain);

	auto swapChainImages = m_device->getSwapchainImagesKHR(m_swapChain.get());
	m_swapChainImages.assign(swapChainImages.begin(), swapChainImages.end());
	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainColorSpace = surfaceFormat.colorSpace;
	m_hdrActive = (surfaceFormat.colorSpace == vk::ColorSpaceKHR::eHdr10St2084EXT);
	Gr_hdr_output_active = m_hdrActive;
	m_swapChainExtent = createInfo.imageExtent;
	mprintf(("Vulkan: Swap chain output mode: %s\n", m_hdrActive ? "HDR10 (PQ/BT.2020)" : "SDR (sRGB)"));

	m_swapChainImageViews.reserve(m_swapChainImages.size());
	for (const auto& image : m_swapChainImages) {
		vk::ImageViewCreateInfo viewCreateInfo;
		viewCreateInfo.image = image;
		viewCreateInfo.viewType = vk::ImageViewType::e2D;
		viewCreateInfo.format = m_swapChainImageFormat;

		viewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;

		viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		m_swapChainImageViews.push_back(m_device->createImageViewUnique(viewCreateInfo));
	}

	// Transition new images eUndefined → ePresentSrcKHR so the render pass
	// can use initialLayout=ePresentSrcKHR from the start.
	{
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.commandPool = m_graphicsCommandPool.get();
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		auto cmdBuffers = m_device->allocateCommandBuffers(allocInfo);
		auto cmd = cmdBuffers.front();

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		cmd.begin(beginInfo);

		for (auto& image : m_swapChainImages) {
			vk::ImageMemoryBarrier barrier;
			barrier.oldLayout = vk::ImageLayout::eUndefined;
			barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = {};

			cmd.pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				{}, nullptr, nullptr, barrier);
		}

		cmd.end();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		m_graphicsQueue.submit(submitInfo, nullptr);
		m_graphicsQueue.waitIdle();

		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), cmdBuffers);
	}

	// Advertise HDR10 mastering/content metadata to the compositor when active.
	if (m_hdrActive && m_hdrMetadataSupported) {
		vk::HdrMetadataEXT metadata;
		// BT.2020 display primaries and D65 white point
		metadata.displayPrimaryRed   = vk::XYColorEXT{0.708f, 0.292f};
		metadata.displayPrimaryGreen = vk::XYColorEXT{0.170f, 0.797f};
		metadata.displayPrimaryBlue  = vk::XYColorEXT{0.131f, 0.046f};
		metadata.whitePoint          = vk::XYColorEXT{0.3127f, 0.3290f};
		metadata.maxLuminance        = Gr_hdr_peak_nits;
		metadata.minLuminance        = 0.0f;
		metadata.maxContentLightLevel        = Gr_hdr_peak_nits;
		metadata.maxFrameAverageLightLevel   = Gr_hdr_paperwhite_nits;
		m_device->setHdrMetadataEXT(m_swapChain.get(), metadata);
		mprintf(("Vulkan: HDR10 metadata set (peak %.0f nits, paper white %.0f nits)\n",
			Gr_hdr_peak_nits, Gr_hdr_paperwhite_nits));
	}

	return true;
}

bool VulkanRenderer::recreateSwapChain()
{
	nprintf(("vulkan", "Vulkan: Recreating swap chain...\n"));

	// Wait for all frames to finish so no resources are in use
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		m_frames[i]->waitForFinish();
	}
	m_device->waitIdle();

	// Re-query surface state (may have changed due to resize/compositor)
	PhysicalDeviceValues freshValues;
	freshValues.device = m_physicalDevice;
	freshValues.surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_vkSurface.get());
	auto fmts = m_physicalDevice.getSurfaceFormatsKHR(m_vkSurface.get());
	freshValues.surfaceFormats.assign(fmts.begin(), fmts.end());
	auto modes = m_physicalDevice.getSurfacePresentModesKHR(m_vkSurface.get());
	freshValues.presentModes.assign(modes.begin(), modes.end());
	freshValues.graphicsQueueIndex = {true, m_graphicsQueueFamilyIndex};
	freshValues.presentQueueIndex = {true, m_presentQueueFamilyIndex};

	// Check for 0x0 extent (minimized window) — caller should retry later
	auto extent = chooseSwapChainExtent(freshValues, gr_screen.max_w, gr_screen.max_h);
	if (extent.width == 0 || extent.height == 0) {
		nprintf(("vulkan", "Vulkan: Surface extent is 0x0 (minimized), deferring swap chain recreation\n"));
		return false;
	}

	// Recreate swap chain, image views, and framebuffers
	// (createSwapChain clears old resources and transitions new images internally).
	// The render passes (including m_encodeRenderPass) are intentionally NOT
	// recreated so cached pipelines remain valid; only the size-dependent
	// composition images and framebuffers are rebuilt.
	createSwapChain(freshValues, m_swapChain.get());
	createCompositionResources();
	createFrameBuffers();

	// Update VulkanRenderFrame handles to point to the new swap chain
	for (auto& frame : m_frames) {
		frame->updateSwapChain(m_swapChain.get());
	}

	// Reset swap chain image tracking
	m_swapChainImageRenderImage.clear();
	m_swapChainImageRenderImage.resize(m_swapChainImages.size(), nullptr);
	m_previousSwapChainImage = UINT32_MAX;

	m_swapChainNeedsRecreation = false;

	nprintf(("vulkan", "Vulkan: Swap chain recreated successfully (%ux%u, %zu images)\n",
		m_swapChainExtent.width, m_swapChainExtent.height, m_swapChainImages.size()));

	return true;
}

} // namespace graphics::vulkan
