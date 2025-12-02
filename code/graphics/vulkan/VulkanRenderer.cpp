
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanDebug.h"

#include <vector>

#include <cerrno>
#include <cstring>

#include "globalincs/version.h"

#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_vulkan.h"
#include "def_files/def_files.h"
#include "graphics/2d.h"
#include "libs/renderdoc/renderdoc.h"
#include "mod_table/mod_table.h"

#if SDL_VERSION_ATLEAST(2, 0, 6)
#include <SDL_vulkan.h>
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace graphics {
namespace vulkan {

namespace {
#if SDL_SUPPORTS_VULKAN
const char* EngineName = "FreeSpaceOpen";

const gameversion::version MinVulkanVersion(1, 1, 0, 0);

VkBool32 VKAPI_PTR debugReportCallback(
#if VK_HEADER_VERSION >= 304
	vk::DebugReportFlagsEXT /*flags*/,
	vk::DebugReportObjectTypeEXT /*objectType*/,
#else
	VkDebugReportFlagsEXT /*flags*/,
	VkDebugReportObjectTypeEXT /*objectType*/,
#endif
	uint64_t /*object*/,
	size_t /*location*/,
	int32_t /*messageCode*/,
	const char* pLayerPrefix,
	const char* pMessage,
	void* /*pUserData*/)
{
	// Crash-safe logging - write validation errors before potential DEVICE_LOST
	FILE* f = fopen("vulkan_debug.log", "a");
	if (f) {
		fprintf(f, "VALIDATION [%s]: %s\n", pLayerPrefix, pMessage);
		fflush(f);
		fclose(f);
	}
	mprintf(("Vulkan message: [%s]: %s\n", pLayerPrefix, pMessage));
	return VK_FALSE;
}
#endif

const SCP_vector<const char*> RequiredDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

bool checkDeviceExtensionSupport(PhysicalDeviceValues& values)
{
	values.extensions = values.device.enumerateDeviceExtensionProperties();

	std::set<std::string> requiredExtensions(RequiredDeviceExtensions.cbegin(), RequiredDeviceExtensions.cend());
	for (const auto& extension : values.extensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool checkSwapChainSupport(PhysicalDeviceValues& values, const vk::UniqueSurfaceKHR& surface)
{
	values.surfaceCapabilities = values.device.getSurfaceCapabilitiesKHR(surface.get());
	values.surfaceFormats = values.device.getSurfaceFormatsKHR(surface.get());
	values.presentModes = values.device.getSurfacePresentModesKHR(surface.get());

	return !values.surfaceFormats.empty() && !values.presentModes.empty();
}

bool isDeviceUnsuitable(PhysicalDeviceValues& values, const vk::UniqueSurfaceKHR& surface)
{
	// We need a GPU. Reject CPU or "other" types.
	if (values.properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu &&
		values.properties.deviceType != vk::PhysicalDeviceType::eIntegratedGpu &&
		values.properties.deviceType != vk::PhysicalDeviceType::eVirtualGpu) {
		mprintf(("Rejecting %s (%d) because the device type is unsuitable.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}

	uint32_t i = 0;
	for (const auto& queue : values.queueProperties) {
		if (!values.graphicsQueueIndex.initialized && queue.queueFlags & vk::QueueFlagBits::eGraphics) {
			values.graphicsQueueIndex.initialized = true;
			values.graphicsQueueIndex.index = i;
		}
		if (!values.transferQueueIndex.initialized && queue.queueFlags & vk::QueueFlagBits::eTransfer) {
			values.transferQueueIndex.initialized = true;
			values.transferQueueIndex.index = i;
		} else if (queue.queueFlags & vk::QueueFlagBits::eTransfer &&
				   !(queue.queueFlags & vk::QueueFlagBits::eGraphics)) {
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
		mprintf(("Rejecting %s (%d) because the device does not have a graphics queue.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}
	if (!values.transferQueueIndex.initialized) {
		mprintf(("Rejecting %s (%d) because the device does not have a transfer queue.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}
	if (!values.presentQueueIndex.initialized) {
		mprintf(("Rejecting %s (%d) because the device does not have a presentation queue.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}

	if (!checkDeviceExtensionSupport(values)) {
		mprintf(("Rejecting %s (%d) because the device does not support our required extensions.\n",
			values.properties.deviceName.data(),
			values.properties.deviceID));
		return true;
	}

	if (!checkSwapChainSupport(values, surface)) {
		mprintf(("Rejecting %s (%d) because the device swap chain was not compatible.\n",
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

uint32_t scoreDevice(const PhysicalDeviceValues& device)
{
	uint32_t score = 0;

	score += deviceTypeScore(device.properties.deviceType) * 1000;
	score += device.properties.apiVersion * 100;

	return score;
}

bool compareDevices(const PhysicalDeviceValues& left, const PhysicalDeviceValues& right)
{
	return scoreDevice(left) < scoreDevice(right);
}

void printPhysicalDevice(const PhysicalDeviceValues& values)
{
	mprintf(("  Found %s (%d) of type %s. API version %d.%d.%d, Driver version %d.%d.%d. Scored as %d\n",
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
	// Debug: force file output for HDR debugging
	// Write to current directory (game directory) for easier access
	FILE* dbg = fopen("vulkan_hdr_debug.txt", "w");
	if (dbg) {
		fprintf(dbg, "Vulkan HDR state: capable=%d, mode=%d, enabled=%d\n",
			Gr_hdr_output_capable ? 1 : 0,
			static_cast<int>(Gr_hdr_output_mode),
			gr_hdr_output_enabled() ? 1 : 0);
		fprintf(dbg, "Available surface formats (%zu):\n", values.surfaceFormats.size());
		for (const auto& fmt : values.surfaceFormats) {
			fprintf(dbg, "  Format: %s, ColorSpace: %s\n",
				vk::to_string(fmt.format).c_str(),
				vk::to_string(fmt.colorSpace).c_str());
		}
		fflush(dbg);
		// Note: dbg is closed later in the function at return points
	} else {
		// Report error to stderr if we can't write (helps debugging)
		fprintf(stderr, "VulkanRenderer: Failed to write HDR debug log to 'vulkan_hdr_debug.txt': %s\n", strerror(errno));
	}

	// Debug: show HDR state
	mprintf(("Vulkan HDR state: capable=%d, mode=%d, enabled=%d\n",
		Gr_hdr_output_capable ? 1 : 0,
		static_cast<int>(Gr_hdr_output_mode),
		gr_hdr_output_enabled() ? 1 : 0));

	// HDR10 path: DISABLED for debugging color issues
	// TODO: Re-enable HDR after fixing color channel issues
	if (false && gr_hdr_output_enabled()) {
		for (const auto& availableFormat : values.surfaceFormats) {
			if (availableFormat.format == vk::Format::eA2B10G10R10UnormPack32 &&
				availableFormat.colorSpace == vk::ColorSpaceKHR::eHdr10St2084EXT) {
				if (dbg) { fprintf(dbg, "SELECTED: HDR10 (A2B10G10R10 + ST2084 PQ)\n"); fclose(dbg); }
				mprintf(("Vulkan: SELECTED HDR10 swapchain (A2B10G10R10 + ST2084 PQ)\n"));
				return availableFormat;
			}
		}
		if (dbg) { fprintf(dbg, "HDR requested but no HDR10 format found!\n"); }
		mprintf(("Vulkan: HDR requested but no HDR10 format found, falling back to SDR\n"));
	}

	// SDR path: Try non-sRGB first for debugging (avoids gamma conversion issues)
	for (const auto& availableFormat : values.surfaceFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
			availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			if (dbg) { fprintf(dbg, "SELECTED: SDR BGRA8 Unorm (non-sRGB)\n"); fclose(dbg); }
			mprintf(("Vulkan: SELECTED SDR BGRA8 Unorm swapchain (no gamma)\n"));
			return availableFormat;
		}
	}

	// SDR path: standard 8-bit sRGB as fallback
	for (const auto& availableFormat : values.surfaceFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
			availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			if (dbg) { fprintf(dbg, "SELECTED: SDR sRGB\n"); fclose(dbg); }
			mprintf(("Vulkan: SELECTED SDR sRGB swapchain\n"));
			return availableFormat;
		}
	}

	if (dbg) { fprintf(dbg, "SELECTED: Fallback (first available)\n"); fclose(dbg); }
	mprintf(("Vulkan: SELECTED fallback format\n"));
	return values.surfaceFormats.front();
}

vk::PresentModeKHR choosePresentMode(const PhysicalDeviceValues& values)
{
	// Depending on if we want Vsync or not, choose the best mode
	for (const auto& availablePresentMode : values.presentModes) {
		if (Gr_enable_vsync) {
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
				return availablePresentMode;
			}
		} else {
			if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
				return availablePresentMode;
			}
		}
	}

	// Guaranteed to be supported
	return vk::PresentModeKHR::eFifo;
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

VulkanRenderer::VulkanRenderer(std::unique_ptr<os::GraphicsOperations> graphicsOps)
	: m_graphicsOps(std::move(graphicsOps))
{
}

bool VulkanRenderer::initialize()
{
	vk_debug("initialize() entry");
	mprintf(("Initializing Vulkan graphics device at %ix%i with %i-bit color...\n",
		gr_screen.max_w,
		gr_screen.max_h,
		gr_screen.bits_per_pixel));

	// Load the RenderDoc API if available before doing anything with OpenGL
	renderdoc::loadApi();
	vk_debug("renderdoc loaded");

	if (!initDisplayDevice()) {
		vk_debug("initDisplayDevice FAILED");
		return false;
	}
	vk_debug("initDisplayDevice OK");

	if (!initializeInstance()) {
		vk_debug("initializeInstance FAILED");
		mprintf(("Failed to create Vulkan instance!\n"));
		return false;
	}
	vk_debug("initializeInstance OK");

	if (!initializeSurface()) {
		vk_debug("initializeSurface FAILED");
		mprintf(("Failed to create Vulkan surface!\n"));
		return false;
	}
	vk_debug("initializeSurface OK");

	if (!pickPhysicalDevice(m_cachedDeviceValues)) {
		vk_debug("pickPhysicalDevice FAILED");
		mprintf(("Could not find suitable physical Vulkan device.\n"));
		return false;
	}
	vk_debug("pickPhysicalDevice OK");

	if (!createLogicalDevice(m_cachedDeviceValues)) {
		vk_debug("createLogicalDevice FAILED");
		mprintf(("Failed to create logical device.\n"));
		return false;
	}
	vk_debug("createLogicalDevice OK");

	// Cache physical device and initialize buffer manager
	// Use graphics queue for transfers to ensure proper synchronization with graphics work
	m_physicalDevice = m_cachedDeviceValues.device;
	vk_debug("creating buffer manager");
	m_bufferManager = std::make_unique<VulkanBufferManager>(m_device.get(), m_physicalDevice, m_graphicsQueue, m_cachedDeviceValues.graphicsQueueIndex.index);
	g_vulkanBufferManager = m_bufferManager.get();
	vk_debug("buffer manager OK");

#ifdef FSO_HAVE_SHADERC
	// Initialize shader manager for runtime GLSL->SPIR-V compilation
	vk_debug("creating shader manager");
	m_shaderManager = std::make_unique<VulkanShaderManager>(m_device.get());
	if (!m_shaderManager->initialize()) {
		mprintf(("Warning: Vulkan shader manager initialization failed - will use precompiled shaders only\n"));
		m_shaderManager.reset();
	} else {
		mprintf(("Vulkan shader manager initialized (runtime GLSL compilation available)\n"));
	}
	vk_debug("shader manager OK");
#endif

	// Initialize descriptor manager for allocating and binding descriptor sets
	vk_debug("creating descriptor manager");
	m_descriptorManager = std::make_unique<VulkanDescriptorManager>();
	if (!m_descriptorManager->initialize(m_device.get(), m_physicalDevice)) {
		vk_debug("descriptor manager FAILED");
		mprintf(("Failed to initialize Vulkan descriptor manager\n"));
		return false;
	}
	vk_debug("descriptor manager OK");

	// Initialize pipeline manager for creating and caching graphics pipelines
	vk_debug("creating pipeline manager");
	m_pipelineManager = std::make_unique<VulkanPipelineManager>();
	if (!m_pipelineManager->initialize(m_device.get(), m_physicalDevice, m_shaderManager.get())) {
		vk_debug("pipeline manager FAILED");
		mprintf(("Failed to initialize Vulkan pipeline manager\n"));
		return false;
	}
	g_vulkanPipelineManager = m_pipelineManager.get();
	vk_debug("pipeline manager OK");

	// Initialize uniform buffer descriptor set
	if (m_pipelineManager) {
		vk::DescriptorSetLayout uniformLayout = m_pipelineManager->getUniformDescriptorSetLayout();
		if (uniformLayout) {
			m_bufferManager->setDescriptorManager(m_descriptorManager.get());
			if (!m_bufferManager->initializeUniformDescriptorSet(uniformLayout)) {
				mprintf(("Warning: Failed to initialize uniform buffer descriptor set\n"));
			}
		}
	}

	vk_debug("creating swapchain");
	if (!createSwapChain(m_cachedDeviceValues)) {
		vk_debug("swapchain FAILED");
		mprintf(("Failed to create swap chain.\n"));
		return false;
	}
	vk_debug("swapchain OK");

	// Initialize render pass manager and create render passes
	vk_debug("creating render pass manager");
	m_renderPassManager = std::make_unique<VulkanRenderPassManager>();
	if (!m_renderPassManager->initialize(m_device.get())) {
		vk_debug("render pass manager FAILED");
		mprintf(("Failed to initialize render pass manager\n"));
		return false;
	}
	vk_debug("render pass manager OK");

	vk_debug("creating render passes");
	if (!createRenderPasses()) {
		vk_debug("render passes FAILED");
		mprintf(("Failed to create render passes\n"));
		return false;
	}
	vk_debug("render passes OK");

	// Create scene framebuffer (color + depth render target)
	vk_debug("creating scene framebuffer");
	if (!createSceneFramebuffer()) {
		vk_debug("scene framebuffer FAILED");
		mprintf(("Failed to create scene framebuffer\n"));
		return false;
	}
	vk_debug("scene framebuffer OK");

	// Create swapchain framebuffers for presentation
	vk_debug("creating swapchain framebuffers");
	createSwapchainFramebuffers();
	vk_debug("swapchain framebuffers OK");

	vk_debug("creating graphics pipeline");
	createGraphicsPipeline();
	vk_debug("creating sync objects");
	createPresentSyncObjects();
	vk_debug("creating command pool");
	createCommandPool(m_cachedDeviceValues);
	vk_debug("command pool OK");
	vk_debug("creating transfer command pool");
	createTransferCommandPool(m_cachedDeviceValues);
	vk_debug("transfer command pool OK");

	// Create blit pipeline for scene-to-swapchain copy
	vk_debug("creating blit pipeline");
	if (!createBlitPipeline()) {
		mprintf(("Warning: Failed to create blit pipeline - scene pass will use fallback\n"));
	}
	vk_debug("blit pipeline OK");

	// Initialize texture manager for bmpman integration
	// Texture manager needs graphics queue for mipmap generation (vkCmdBlitImage)
	// Transfer-only queues cannot do blit operations
	vk_debug("creating texture manager");
	m_textureManager = std::make_unique<VulkanTextureManager>();
	if (!m_textureManager->initialize(m_device.get(), m_physicalDevice,
	                                   m_graphicsCommandPool.get(), m_graphicsQueue)) {
		vk_debug("texture manager FAILED");
		mprintf(("Failed to initialize Vulkan texture manager\n"));
		return false;
	}
	g_vulkanTextureManager = m_textureManager.get();
	vk_debug("texture manager OK");

	// Prepare the rendering state by acquiring our first swap chain image
	vk_debug("acquiring first swapchain image");
	acquireNextSwapChainImage();
	vk_debug("initialize() complete - SUCCESS");

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
		mprintf(("Error in first SDL_Vulkan_GetInstanceExtensions: %s\n", SDL_GetError()));
		return false;
	}

	std::vector<const char*> extensions;
	extensions.resize(count);

	if (!SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data())) {
		mprintf(("Error in second SDL_Vulkan_GetInstanceExtensions: %s\n", SDL_GetError()));
		return false;
	}

	const auto instanceVersion = vk::enumerateInstanceVersion();
	gameversion::version vulkanVersion(VK_VERSION_MAJOR(instanceVersion),
		VK_VERSION_MINOR(instanceVersion),
		VK_VERSION_PATCH(instanceVersion),
		0);
	mprintf(("Vulkan instance version %s\n", gameversion::format_version(vulkanVersion).c_str()));

	if (vulkanVersion < MinVulkanVersion) {
		mprintf(("Vulkan version is less than the minimum which is %s.\n",
			gameversion::format_version(MinVulkanVersion).c_str()));
		return false;
	}

	const auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
	mprintf(("Instance extensions:\n"));
	for (const auto& ext : supportedExtensions) {
		mprintf(("  Found support for %s version %" PRIu32 "\n", ext.extensionName.data(), ext.specVersion));
		if (!stricmp(ext.extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			m_debugReportEnabled = true;
		} else if (!stricmp(ext.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)) {
			extensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
		}
	}

	std::vector<const char*> layers;
	const auto supportedLayers = vk::enumerateInstanceLayerProperties();
	mprintf(("Instance layers:\n"));
	for (const auto& layer : supportedLayers) {
		mprintf(("  Found layer %s(%s). Spec version %d.%d.%d and implementation %" PRIu32 "\n",
			layer.layerName.data(),
			layer.description.data(),
			VK_VERSION_MAJOR(layer.specVersion),
			VK_VERSION_MINOR(layer.specVersion),
			VK_VERSION_PATCH(layer.specVersion),
			layer.implementationVersion));
		// Modern unified validation layer (Vulkan SDK 1.1.106+)
		if (!stricmp(layer.layerName, "VK_LAYER_KHRONOS_validation")) {
			layers.push_back("VK_LAYER_KHRONOS_validation");
		}
		// Legacy validation layer (fallback for older SDKs)
		else if (!stricmp(layer.layerName, "VK_LAYER_LUNARG_core_validation")) {
			layers.push_back("VK_LAYER_LUNARG_core_validation");
		}
	}

	vk::ApplicationInfo appInfo(Window_title.c_str(), 1, EngineName, 1, VK_API_VERSION_1_1);

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

	vk::StructureChain<vk::InstanceCreateInfo, vk::DebugReportCallbackCreateInfoEXT> createInstanceChain(createInfo,
		createInstanceReportInfo);

	if (!m_debugReportEnabled) {
		createInstanceChain.unlink<vk::DebugReportCallbackCreateInfoEXT>();
	}

	vk::UniqueInstance instance = vk::createInstanceUnique(createInstanceChain.get<vk::InstanceCreateInfo>(), nullptr);
	if (!instance) {
		return false;
	}

	VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

	if (m_debugReportEnabled) {
		vk::DebugReportCallbackCreateInfoEXT reportCreateInfo(vk::DebugReportFlagBitsEXT::eError |
															  vk::DebugReportFlagBitsEXT::eWarning |
															  vk::DebugReportFlagBitsEXT::ePerformanceWarning);
		reportCreateInfo.pfnCallback = debugReportCallback;

		m_debugReport = instance->createDebugReportCallbackEXTUnique(reportCreateInfo);
	}

	m_vkInstance = std::move(instance);
	return true;
#else
	mprintf(("SDL does not support Vulkan in its current version.\n"));
	return false;
#endif
}

bool VulkanRenderer::initializeSurface()
{
#if SDL_SUPPORTS_VULKAN
	const auto window = os::getSDLMainWindow();

	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(*m_vkInstance), &surface)) {
		mprintf(("Failed to create vulkan surface: %s\n", SDL_GetError()));
		return false;
	}

#if VK_HEADER_VERSION >= 301
	const vk::detail::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> deleter(*m_vkInstance,
#else
	const vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> deleter(*m_vkInstance,
#endif
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
		vals.features = dev.getFeatures2().features;
		vals.queueProperties = dev.getQueueFamilyProperties();
		return vals;
	});

	mprintf(("Physical Vulkan devices:\n"));
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
	mprintf(("Selected device %s (%d) as the primary Vulkan device.\n",
		deviceValues.properties.deviceName.data(),
		deviceValues.properties.deviceID));
	mprintf(("Queue families: graphics=%u, transfer=%u, present=%u\n",
		deviceValues.graphicsQueueIndex.index,
		deviceValues.transferQueueIndex.index,
		deviceValues.presentQueueIndex.index));
	mprintf(("Device extensions:\n"));
	for (const auto& extProp : deviceValues.extensions) {
		mprintf(("  Found support for %s version %" PRIu32 "\n", extProp.extensionName.data(), extProp.specVersion));
	}

	// Detect HDR10 support by checking for the required surface format
	mprintf(("Surface formats:\n"));
	for (const auto& format : deviceValues.surfaceFormats) {
		mprintf(("  Format: %s, ColorSpace: %s\n",
			to_string(format.format).c_str(),
			to_string(format.colorSpace).c_str()));

		if (format.format == vk::Format::eA2B10G10R10UnormPack32 &&
			format.colorSpace == vk::ColorSpaceKHR::eHdr10St2084EXT) {
			deviceValues.supportsHDR10 = true;
			deviceValues.preferredHDRColorSpace = format.colorSpace;
		}
	}

	// FORCE SDR - disable HDR completely for debugging
	Gr_hdr_output_capable = false;
	mprintf(("Vulkan: HDR FORCED OFF for debugging\n"));

	return true;
}

bool VulkanRenderer::createLogicalDevice(const PhysicalDeviceValues& deviceValues)
{
	float queuePriority = 1.0f;

	std::vector<vk::DeviceQueueCreateInfo> queueInfos;
	const std::set<uint32_t> familyIndices{deviceValues.graphicsQueueIndex.index,
	                                       deviceValues.transferQueueIndex.index,
	                                       deviceValues.presentQueueIndex.index};

	queueInfos.reserve(familyIndices.size());
	for (auto index : familyIndices) {
		queueInfos.emplace_back(vk::DeviceQueueCreateFlags(), index, 1, &queuePriority);
	}

	vk::DeviceCreateInfo deviceCreate;
	deviceCreate.pQueueCreateInfos = queueInfos.data();
	deviceCreate.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
	deviceCreate.pEnabledFeatures = &deviceValues.features;

	deviceCreate.ppEnabledExtensionNames = RequiredDeviceExtensions.data();
	deviceCreate.enabledExtensionCount = static_cast<uint32_t>(RequiredDeviceExtensions.size());

	m_device = deviceValues.device.createDeviceUnique(deviceCreate);

	// Create queues
	m_graphicsQueue = m_device->getQueue(deviceValues.graphicsQueueIndex.index, 0);
	m_transferQueue = m_device->getQueue(deviceValues.transferQueueIndex.index, 0);
	m_presentQueue = m_device->getQueue(deviceValues.presentQueueIndex.index, 0);

	return true;
}
bool VulkanRenderer::createSwapChain(const PhysicalDeviceValues& deviceValues)
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
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	const uint32_t queueFamilyIndices[] = {deviceValues.graphicsQueueIndex.index, deviceValues.presentQueueIndex.index};
	if (deviceValues.graphicsQueueIndex.index != deviceValues.presentQueueIndex.index) {
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0;     // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = deviceValues.surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = choosePresentMode(deviceValues);
	createInfo.clipped = true;
	createInfo.oldSwapchain = nullptr;

	m_swapChain = m_device->createSwapchainKHRUnique(createInfo);

	std::vector<vk::Image> swapChainImages = m_device->getSwapchainImagesKHR(m_swapChain.get());
	m_swapChainImages = SCP_vector<vk::Image>(swapChainImages.begin(), swapChainImages.end());
	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainColorSpace = surfaceFormat.colorSpace;
	m_swapChainExtent = createInfo.imageExtent;

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

	m_sceneExtent = m_swapChainExtent;

	return true;
}
vk::UniqueShaderModule VulkanRenderer::loadShader(const SCP_string& name)
{
	const auto def_file = defaults_get_file(name.c_str());

	vk::ShaderModuleCreateInfo createInfo;
	createInfo.codeSize = def_file.size;
	createInfo.pCode = static_cast<const uint32_t*>(def_file.data);

	return m_device->createShaderModuleUnique(createInfo);
}
vk::Format VulkanRenderer::findDepthFormat()
{
	// Prefer D32, fall back to D24S8, then D16
	SCP_vector<vk::Format> candidates = {
	    vk::Format::eD32Sfloat,
	    vk::Format::eD32SfloatS8Uint,
	    vk::Format::eD24UnormS8Uint,
	    vk::Format::eD16Unorm,
	};

	for (vk::Format format : candidates) {
		vk::FormatProperties props = m_physicalDevice.getFormatProperties(format);
		if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
			mprintf(("Vulkan: Selected depth format %s\n", vk::to_string(format).c_str()));
			return format;
		}
	}

	mprintf(("Vulkan: No suitable depth format found, using D16\n"));
	return vk::Format::eD16Unorm; // Fallback
}

bool VulkanRenderer::createRenderPasses()
{
	// Find suitable depth format
	m_depthFormat = findDepthFormat();

	// Create scene render pass (color + depth) for 3D geometry rendering
	// Uses R16G16B16A16_SFLOAT for HDR scene rendering
	if (!m_renderPassManager->createSceneRenderPass(vk::Format::eR16G16B16A16Sfloat, m_depthFormat)) {
		mprintf(("Vulkan: Failed to create scene render pass\n"));
		return false;
	}

	// Create present render pass (color only) for final output to swapchain
	if (!m_renderPassManager->createPresentRenderPass(m_swapChainImageFormat)) {
		mprintf(("Vulkan: Failed to create present render pass\n"));
		return false;
	}

	return true;
}

bool VulkanRenderer::createSceneFramebuffer()
{
	m_sceneFramebuffer = std::make_unique<VulkanFramebuffer>();

	// Create scene framebuffer with HDR color + depth
	if (!m_sceneFramebuffer->create(m_device.get(),
	        m_physicalDevice,
	        m_renderPassManager->getSceneRenderPass(),
	        m_swapChainExtent.width,
	        m_swapChainExtent.height,
	        {vk::Format::eR16G16B16A16Sfloat}, // HDR color
	        m_depthFormat)) {
		mprintf(("Vulkan: Failed to create scene framebuffer\n"));
		return false;
	}

	m_sceneExtent = m_swapChainExtent;

	return true;
}

void VulkanRenderer::createSwapchainFramebuffers()
{
	m_swapchainFramebuffers.clear();
	m_swapchainFramebuffers.reserve(m_swapChainImageViews.size());

	// Create framebuffer for each swapchain image (no depth - present pass doesn't need it)
	for (size_t i = 0; i < m_swapChainImageViews.size(); ++i) {
		auto fb = std::make_unique<VulkanFramebuffer>();
		if (!fb->createFromImageViews(m_device.get(),
		        m_renderPassManager->getPresentRenderPass(),
		        m_swapChainExtent.width,
		        m_swapChainExtent.height,
		        {m_swapChainImageViews[i].get()},
		        nullptr)) { // No depth for present pass
			mprintf(("Vulkan: Failed to create swapchain framebuffer %zu\n", i));
			continue;
		}
		m_swapchainFramebuffers.push_back(std::move(fb));
	}

	mprintf(("Vulkan: Created %zu swapchain framebuffers\n", m_swapchainFramebuffers.size()));
}
void VulkanRenderer::createGraphicsPipeline()
{
	auto vertShaderMod = loadShader("vulkan.vert.spv");
	vk::PipelineShaderStageCreateInfo vertStageCreate;
	vertStageCreate.stage = vk::ShaderStageFlagBits::eVertex;
	vertStageCreate.module = vertShaderMod.get();
	vertStageCreate.pName = "main";

	auto fragShaderMod = loadShader("vulkan.frag.spv");
	vk::PipelineShaderStageCreateInfo fragStageCreate;
	fragStageCreate.stage = vk::ShaderStageFlagBits::eFragment;
	fragStageCreate.module = fragShaderMod.get();
	fragStageCreate.pName = "main";

	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertStageCreate, fragStageCreate};

	vk::PipelineVertexInputStateCreateInfo vertInCreate;
	vertInCreate.vertexBindingDescriptionCount = 0;
	vertInCreate.vertexAttributeDescriptionCount = 0;

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = false;

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = i2fl(gr_screen.max_w);
	viewport.height = i2fl(gr_screen.max_h);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = m_swapChainExtent;

	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.depthClampEnable = false;
	rasterizer.rasterizerDiscardEnable = false;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode |= vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = false;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	vk::PipelineMultisampleStateCreateInfo multisampling;
	multisampling.sampleShadingEnable = false;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = false;
	multisampling.alphaToOneEnable = false;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
										  vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = false;
	colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;  // Optional
	colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
	colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;             // Optional
	colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;  // Optional
	colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
	colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;             // Optional

	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.logicOpEnable = false;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	vk::DynamicState dynamicStates[] = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eLineWidth,
	};

	vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
	dynamicStateInfo.dynamicStateCount = 2;
	dynamicStateInfo.pDynamicStates = dynamicStates;

	vk::PipelineLayoutCreateInfo pipelineLayout;
	pipelineLayout.setLayoutCount = 0;
	pipelineLayout.pSetLayouts = nullptr;
	pipelineLayout.pushConstantRangeCount = 0;
	pipelineLayout.pPushConstantRanges = nullptr;

	m_pipelineLayout = m_device->createPipelineLayoutUnique(pipelineLayout);

	vk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertInCreate;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = m_pipelineLayout.get();
	pipelineInfo.renderPass = m_renderPassManager->getPresentRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;
	pipelineInfo.basePipelineIndex = -1;

	m_graphicsPipeline = m_device->createGraphicsPipelineUnique(nullptr, pipelineInfo).value;
}

bool VulkanRenderer::createBlitPipeline()
{
	// Create sampler for scene texture
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.anisotropyEnable = false;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareEnable = false;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	m_blitSampler = m_device->createSamplerUnique(samplerInfo);

	// Create descriptor set layout for the scene texture sampler
	vk::DescriptorSetLayoutBinding samplerBinding;
	samplerBinding.binding = 0;
	samplerBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerBinding.descriptorCount = 1;
	samplerBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
	samplerBinding.pImmutableSamplers = nullptr;

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerBinding;
	m_blitDescriptorSetLayout = m_device->createDescriptorSetLayoutUnique(layoutInfo);

	// Create pipeline layout with descriptor set
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_blitDescriptorSetLayout.get();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	m_blitPipelineLayout = m_device->createPipelineLayoutUnique(pipelineLayoutInfo);

	// Load shaders - use the same simple shaders for now (fullscreen triangle)
	// These will be replaced with proper blit shaders that sample the scene texture
	auto vertShaderMod = loadShader("vulkan_blit.vert.spv");
	auto fragShaderMod = loadShader("vulkan_blit.frag.spv");

	vk::PipelineShaderStageCreateInfo vertStage;
	vertStage.stage = vk::ShaderStageFlagBits::eVertex;
	vertStage.module = vertShaderMod.get();
	vertStage.pName = "main";

	vk::PipelineShaderStageCreateInfo fragStage;
	fragStage.stage = vk::ShaderStageFlagBits::eFragment;
	fragStage.module = fragShaderMod.get();
	fragStage.pName = "main";

	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertStage, fragStage};

	// No vertex input - fullscreen triangle generated in shader
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = false;

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapChainExtent.width);
	viewport.height = static_cast<float>(m_swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D{0, 0};
	scissor.extent = m_swapChainExtent;

	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.depthClampEnable = false;
	rasterizer.rasterizerDiscardEnable = false;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eNone;  // No culling for fullscreen quad
	rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizer.depthBiasEnable = false;

	vk::PipelineMultisampleStateCreateInfo multisampling;
	multisampling.sampleShadingEnable = false;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
	                                      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = false;

	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.logicOpEnable = false;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	vk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;  // No depth testing for blit
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = m_blitPipelineLayout.get();
	pipelineInfo.renderPass = m_renderPassManager->getPresentRenderPass();
	pipelineInfo.subpass = 0;

	auto result = m_device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
	if (result.result != vk::Result::eSuccess) {
		mprintf(("Vulkan: Failed to create blit pipeline\n"));
		return false;
	}
	m_blitPipeline = std::move(result.value);

	mprintf(("Vulkan: Created blit pipeline for scene-to-swapchain copy\n"));
	return true;
}
void VulkanRenderer::createCommandPool(const PhysicalDeviceValues& values)
{
	vk::CommandPoolCreateInfo poolCreate;
	poolCreate.queueFamilyIndex = values.graphicsQueueIndex.index;
	poolCreate.flags |= vk::CommandPoolCreateFlagBits::eTransient |
	                    vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	m_graphicsCommandPool = m_device->createCommandPoolUnique(poolCreate);
	mprintf(("Vulkan: Created graphics command pool (family=%u)\n", values.graphicsQueueIndex.index));
}

void VulkanRenderer::createTransferCommandPool(const PhysicalDeviceValues& values)
{
	vk::CommandPoolCreateInfo poolCreate;
	poolCreate.queueFamilyIndex = values.transferQueueIndex.index;
	poolCreate.flags |= vk::CommandPoolCreateFlagBits::eTransient |
	                    vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	m_transferCommandPool = m_device->createCommandPoolUnique(poolCreate);
	mprintf(("Vulkan: Created transfer command pool (family=%u)\n", values.transferQueueIndex.index));
}
void VulkanRenderer::createPresentSyncObjects()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		m_frames[i].reset(new RenderFrame(m_device.get(), m_swapChain.get(), m_graphicsQueue, m_presentQueue));
	}

	// Create per-swapchain-image semaphores
	// Acquire semaphores: indexed by frame (we wait for fence before reusing, so they're safe)
	// Render semaphores: indexed by swapchain image (presentation holds onto these until image is re-acquired)
	constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo;
	const size_t imageCount = m_swapChainImages.size();

	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		m_imageAvailableSemaphores[i] = m_device->createSemaphoreUnique(semaphoreCreateInfo);
	}

	m_renderingFinishedSemaphores.resize(imageCount);
	for (size_t i = 0; i < imageCount; ++i) {
		m_renderingFinishedSemaphores[i] = m_device->createSemaphoreUnique(semaphoreCreateInfo);
	}

	m_swapChainImageRenderImage.resize(m_swapChainImages.size(), nullptr);
}
void VulkanRenderer::acquireNextSwapChainImage()
{
	vk_debug("acquireNextSwapChainImage() entry - waiting for frame fence");
	m_frames[m_currentFrame]->waitForFinish();
	vk_debug("acquireNextSwapChainImage() fence signaled");

	// Safe to process deferred deletions - GPU is done with this frame's work
	if (m_bufferManager) {
		m_bufferManager->beginFrame(m_currentFrame);
	}
	if (m_textureManager) {
		m_textureManager->beginFrame(m_currentFrame);
	}
	vk_debug("acquireNextSwapChainImage() managers updated");

	// Update absolute frame counter for descriptor tracking
	m_absoluteFrameCounter++;
	if (m_descriptorManager) {
		m_descriptorManager->setCurrentFrame(m_absoluteFrameCounter);
	}

	uint32_t imageIndex = 0;
	// Use frame-indexed acquire semaphore (safe because we wait for frame fence first)
	vk::Semaphore acquireSem = m_imageAvailableSemaphores[m_currentFrame].get();
	AcquireResult result = m_frames[m_currentFrame]->acquireSwapchainImage(imageIndex, acquireSem);
	vk_debug("acquireNextSwapChainImage() acquired image");
	vk_logf("VulkanRenderer",
		"acquireNextSwapChainImage: frame=%u result=%d imageIndex=%u",
		m_currentFrame,
		static_cast<int>(result),
		imageIndex);

	switch (result) {
	case AcquireResult::Success:
		// Normal case - continue
		break;
	case AcquireResult::Suboptimal:
		// Image acquired but swapchain should be recreated soon
		vk_logf("VulkanRenderer", "Vulkan: Swapchain suboptimal, will recreate after this frame");
		m_swapchainNeedsRecreate = true;
		break;
	case AcquireResult::OutOfDate:
		// Must recreate swapchain immediately
		vk_logf("VulkanRenderer", "Vulkan: Swapchain out of date, recreating now");
		if (!recreateSwapChain()) {
			vk_logf("VulkanRenderer", "Vulkan: Failed to recreate swapchain!");
			return;
		}
		// Try to acquire again after recreation
		result = m_frames[m_currentFrame]->acquireSwapchainImage(imageIndex, acquireSem);
		if (result != AcquireResult::Success && result != AcquireResult::Suboptimal) {
			vk_logf("VulkanRenderer", "Vulkan: Failed to acquire image after swapchain recreation");
			return;
		}
		vk_logf("VulkanRenderer",
			"acquireNextSwapChainImage: reacquired after recreation result=%d imageIndex=%u",
			static_cast<int>(result),
			imageIndex);
		break;
	case AcquireResult::Error:
		vk_logf("VulkanRenderer", "Vulkan: Fatal error acquiring swapchain image");
		return;
	}

	m_currentSwapChainImage = imageIndex;
	vk_logf("VulkanRenderer",
		"acquireNextSwapChainImage: current swapchain image=%u frame=%u",
		m_currentSwapChainImage,
		m_currentFrame);

	// Ensure that this image is no longer in use
	if (m_swapChainImageRenderImage[m_currentSwapChainImage]) {
		m_swapChainImageRenderImage[m_currentSwapChainImage]->waitForFinish();
	}
	// Reserve the image as in use
	m_swapChainImageRenderImage[m_currentSwapChainImage] = m_frames[m_currentFrame].get();
}
void VulkanRenderer::drawScene(vk::Framebuffer destinationFb, vk::CommandBuffer cmdBuffer)
{
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	vk_logf("VulkanRenderer",
		"drawScene: cmd=%p framebuffer=%p extent=%ux%u",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer)),
		reinterpret_cast<void*>(static_cast<VkFramebuffer>(destinationFb)),
		m_swapChainExtent.width,
		m_swapChainExtent.height);

	cmdBuffer.begin(beginInfo);

	vk::RenderPassBeginInfo renderPassBegin;
	renderPassBegin.renderPass = m_renderPassManager->getPresentRenderPass();
	renderPassBegin.framebuffer = destinationFb;
	renderPassBegin.renderArea.offset.x = 0;
	renderPassBegin.renderArea.offset.y = 0;
	renderPassBegin.renderArea.extent = m_swapChainExtent;

	vk::ClearValue clearColor;
	clearColor.color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});

	renderPassBegin.clearValueCount = 1;
	renderPassBegin.pClearValues = &clearColor;

	cmdBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline.get());

	cmdBuffer.draw(3, 1, 0, 0);

	cmdBuffer.endRenderPass();
	vk_logf("VulkanRenderer",
		"drawScene: end render pass cmd=%p",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer)));

	cmdBuffer.end();
}
void VulkanRenderer::beginScenePass()
{
	if (m_scenePassActive) {
		vk_logf("VulkanRenderer",
			"beginScenePass called while scene pass already active (frame=%u)",
			m_currentFrame);
		return;
	}

	// NOTE: Do NOT submit transfers here - transfers are batched and submitted
	// in flip() after proper fence synchronization. Submitting here would break
	// the frame synchronization model.

	// Allocate command buffer for this frame's scene rendering
	vk::CommandBufferAllocateInfo cmdBufferAlloc;
	cmdBufferAlloc.commandPool = m_graphicsCommandPool.get();
	cmdBufferAlloc.level = vk::CommandBufferLevel::ePrimary;
	cmdBufferAlloc.commandBufferCount = 1;

	auto allocatedBuffers = m_device->allocateCommandBuffers(cmdBufferAlloc);
	m_sceneCommandBuffer = allocatedBuffers.front();
	vk_logf("VulkanRenderer",
		"beginScenePass: allocated command buffer %p frame=%u swapImage=%u",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)),
		m_currentFrame,
		m_currentSwapChainImage);

	// Begin command buffer recording
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	m_sceneCommandBuffer.begin(beginInfo);

	// Determine target framebuffer/render pass (scene framebuffer by default)
	vk::RenderPass targetRenderPass = m_renderPassManager->getSceneRenderPass();
	VulkanFramebuffer* targetFramebuffer = m_sceneFramebuffer.get();
	vk::Extent2D targetExtent = m_swapChainExtent;
	bool usingActiveRenderTarget = false;
	bool usingTextureManagerRT = false;

	// Check if a render target is active
	if (m_activeRenderTarget.isActive && m_activeRenderTarget.framebuffer) {
		targetFramebuffer = m_activeRenderTarget.framebuffer;
		targetExtent = m_activeRenderTarget.extent;
		// Use scene render pass for render targets (TODO: Create dedicated render pass if needed)
		targetRenderPass = m_renderPassManager->getSceneRenderPass();
		usingActiveRenderTarget = true;
	} else if (m_textureManager && m_textureManager->isRenderingToTexture()) {
		// Fallback to texture manager's render target tracking (for compatibility)
		if (auto activeRT = m_textureManager->getActiveRenderTarget()) {
			VulkanFramebuffer* rtFramebuffer = nullptr;
			if (activeRT->isCubemap && activeRT->cubeFaceFramebuffers[activeRT->activeFace]) {
				rtFramebuffer = activeRT->cubeFaceFramebuffers[activeRT->activeFace].get();
			} else {
				rtFramebuffer = activeRT->framebuffer.get();
			}

			if (rtFramebuffer && activeRT->renderPass) {
				targetRenderPass = activeRT->renderPass.get();
				targetFramebuffer = rtFramebuffer;
				targetExtent = rtFramebuffer->getExtent();
				usingTextureManagerRT = true;
			}
		}
	}

	if (!targetRenderPass || !targetFramebuffer) {
		vk_logf("VulkanRenderer",
			"beginScenePass failed: missing render pass/framebuffer (renderPass=%p framebuffer=%p)",
			reinterpret_cast<void*>(static_cast<VkRenderPass>(targetRenderPass)),
			targetFramebuffer ? reinterpret_cast<void*>(static_cast<VkFramebuffer>(targetFramebuffer->getFramebuffer())) : nullptr);
		return;
	}

	// Begin scene render pass
	vk::RenderPassBeginInfo renderPassBegin;
	renderPassBegin.renderPass = targetRenderPass;
	renderPassBegin.framebuffer = targetFramebuffer->getFramebuffer();
	renderPassBegin.renderArea.offset = vk::Offset2D{0, 0};
	renderPassBegin.renderArea.extent = targetExtent;

	// Clear values: always clear color; depth only if attachment present
	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]});
	uint32_t clearCount = 1;
	if (targetFramebuffer->hasDepthAttachment()) {
		clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
		clearCount = 2;
	}

	renderPassBegin.clearValueCount = clearCount;
	renderPassBegin.pClearValues = clearValues.data();

	m_sceneExtent = targetExtent;

	vk_logf("VulkanRenderer",
		"beginScenePass: beginRenderPass cmd=%p renderPass=%p framebuffer=%p extent=%ux%u activeRT=%d textureRT=%d",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)),
		reinterpret_cast<void*>(static_cast<VkRenderPass>(targetRenderPass)),
		reinterpret_cast<void*>(static_cast<VkFramebuffer>(targetFramebuffer->getFramebuffer())),
		targetExtent.width,
		targetExtent.height,
		usingActiveRenderTarget ? 1 : 0,
		usingTextureManagerRT ? 1 : 0);

	m_sceneCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

	m_scenePassActive = true;
	
	// Reset draw state for new scene pass
	m_drawState.reset();
}

void VulkanRenderer::endScenePass()
{
	if (!m_scenePassActive) {
		vk_logf("VulkanRenderer",
			"endScenePass called but no scene pass is active (frame=%u)",
			m_currentFrame);
		return;
	}

	// End the scene render pass (this triggers layout transition to ShaderReadOnlyOptimal)
	vk_logf("VulkanRenderer",
		"endScenePass: ending render pass for cmd=%p",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)));
	m_sceneCommandBuffer.endRenderPass();

	// Note: We don't end the command buffer here - recordBlitToSwapchain will continue using it
	// and flip() will finalize and submit it
}

void VulkanRenderer::beginAuxiliaryRenderPass(vk::RenderPass renderPass, VulkanFramebuffer* framebuffer,
                                               vk::Extent2D extent, bool clearColor)
{
	if (m_auxiliaryPassActive) {
		vk_logf("VulkanRenderer",
			"beginAuxiliaryRenderPass called while auxiliary pass already active (frame=%u)",
			m_currentFrame);
		return;
	}

	if (m_scenePassActive || m_directPassActive) {
		vk_logf("VulkanRenderer",
			"beginAuxiliaryRenderPass called while scene/direct pass active - this is not supported (frame=%u)",
			m_currentFrame);
		return;
	}

	if (!renderPass || !framebuffer) {
		vk_logf("VulkanRenderer",
			"beginAuxiliaryRenderPass: invalid renderPass=%p or framebuffer=%p",
			reinterpret_cast<void*>(static_cast<VkRenderPass>(renderPass)),
			static_cast<void*>(framebuffer));
		return;
	}

	// Allocate command buffer if not already allocated
	if (!m_sceneCommandBuffer) {
		vk::CommandBufferAllocateInfo cmdBufferAlloc;
		cmdBufferAlloc.commandPool = m_graphicsCommandPool.get();
		cmdBufferAlloc.level = vk::CommandBufferLevel::ePrimary;
		cmdBufferAlloc.commandBufferCount = 1;

		auto allocatedBuffers = m_device->allocateCommandBuffers(cmdBufferAlloc);
		m_sceneCommandBuffer = allocatedBuffers.front();
		vk_logf("VulkanRenderer",
			"beginAuxiliaryRenderPass: allocated command buffer %p frame=%u",
			reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)),
			m_currentFrame);

		// Begin command buffer recording
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		m_sceneCommandBuffer.begin(beginInfo);
	}

	// Begin the auxiliary render pass
	vk::RenderPassBeginInfo renderPassBegin;
	renderPassBegin.renderPass = renderPass;
	renderPassBegin.framebuffer = framebuffer->getFramebuffer();
	renderPassBegin.renderArea.offset = vk::Offset2D{0, 0};
	renderPassBegin.renderArea.extent = extent;

	// Set up clear values
	std::array<vk::ClearValue, 2> clearValues;
	uint32_t clearCount = 0;
	if (clearColor) {
		clearValues[0].color.setFloat32({m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]});
		clearCount = 1;
	}
	if (framebuffer->hasDepthAttachment()) {
		clearValues[clearCount].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
		clearCount++;
	}
	renderPassBegin.clearValueCount = clearCount;
	renderPassBegin.pClearValues = clearValues.data();

	m_sceneExtent = extent;

	vk_logf("VulkanRenderer",
		"beginAuxiliaryRenderPass: cmd=%p renderPass=%p framebuffer=%p extent=%ux%u",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)),
		reinterpret_cast<void*>(static_cast<VkRenderPass>(renderPass)),
		reinterpret_cast<void*>(static_cast<VkFramebuffer>(framebuffer->getFramebuffer())),
		extent.width, extent.height);

	m_sceneCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);
	m_auxiliaryPassActive = true;

	// Reset draw state for new pass
	m_drawState.reset();
}

void VulkanRenderer::endAuxiliaryRenderPass()
{
	if (!m_auxiliaryPassActive) {
		vk_logf("VulkanRenderer",
			"endAuxiliaryRenderPass called but no auxiliary pass is active (frame=%u)",
			m_currentFrame);
		return;
	}

	vk_logf("VulkanRenderer",
		"endAuxiliaryRenderPass: ending render pass for cmd=%p",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)));
	m_sceneCommandBuffer.endRenderPass();
	m_auxiliaryPassActive = false;

	// Note: Command buffer is left open for further commands (more auxiliary passes or scene pass)
}

void VulkanRenderer::ensureRenderPassActive()
{
	// If scene pass or direct pass already active, nothing to do
	if (m_scenePassActive || m_directPassActive) {
		return;
	}

	vk_logf("VulkanRenderer",
		"ensureRenderPassActive: starting direct pass frame=%u swapImage=%u",
		m_currentFrame,
		m_currentSwapChainImage);

	// Start a direct-to-swapchain render pass for menu/UI rendering
	// This is similar to beginScenePass but renders directly to the swapchain image

	// NOTE: Do NOT submit transfers here - transfers are batched and submitted
	// in flip() after proper fence synchronization. Submitting here would break
	// the frame synchronization model.

	// Allocate command buffer for this frame
	vk::CommandBufferAllocateInfo cmdBufferAlloc;
	cmdBufferAlloc.commandPool = m_graphicsCommandPool.get();
	cmdBufferAlloc.level = vk::CommandBufferLevel::ePrimary;
	cmdBufferAlloc.commandBufferCount = 1;

	auto allocatedBuffers = m_device->allocateCommandBuffers(cmdBufferAlloc);
	m_sceneCommandBuffer = allocatedBuffers.front();
	vk_logf("VulkanRenderer",
		"ensureRenderPassActive: allocated command buffer %p",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)));

	// Begin command buffer recording
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	m_sceneCommandBuffer.begin(beginInfo);

	// Begin present render pass directly on swapchain
	vk::RenderPassBeginInfo renderPassBegin;
	renderPassBegin.renderPass = m_renderPassManager->getPresentRenderPass();
	renderPassBegin.framebuffer = m_swapchainFramebuffers[m_currentSwapChainImage]->getFramebuffer();
	renderPassBegin.renderArea.offset = vk::Offset2D{0, 0};
	renderPassBegin.renderArea.extent = m_swapChainExtent;

	// Clear to background color
	vk::ClearValue clearColor;
	clearColor.color.setFloat32({m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]});
	renderPassBegin.clearValueCount = 1;
	renderPassBegin.pClearValues = &clearColor;

	vk_logf("VulkanRenderer",
		"ensureRenderPassActive: beginRenderPass cmd=%p renderPass=%p framebuffer=%p extent=%ux%u",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)),
		reinterpret_cast<void*>(static_cast<VkRenderPass>(m_renderPassManager->getPresentRenderPass())),
		reinterpret_cast<void*>(static_cast<VkFramebuffer>(m_swapchainFramebuffers[m_currentSwapChainImage]->getFramebuffer())),
		m_swapChainExtent.width,
		m_swapChainExtent.height);

	m_sceneCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

	m_directPassActive = true;
	m_sceneExtent = m_swapChainExtent;
	resetDrawState();
}

vk::CommandBuffer VulkanRenderer::getCurrentCommandBuffer() const
{
	// Return the scene command buffer if we're in an active pass (scene, direct, or auxiliary)
	if ((m_scenePassActive || m_directPassActive || m_auxiliaryPassActive) && m_sceneCommandBuffer) {
		return m_sceneCommandBuffer;
	}
	return nullptr;
}

vk::RenderPass VulkanRenderer::getCurrentRenderPass() const
{
	// Return appropriate render pass based on which pass is active
	if (m_scenePassActive && m_renderPassManager) {
		return m_renderPassManager->getSceneRenderPass();
	}
	if (m_directPassActive && m_renderPassManager) {
		return m_renderPassManager->getPresentRenderPass();
	}
	return nullptr;
}

void VulkanRenderer::recordBlitToSwapchain(vk::CommandBuffer cmdBuffer)
{
	// Begin present render pass on swapchain framebuffer
	vk_logf("VulkanRenderer",
		"recordBlitToSwapchain: cmd=%p swapImage=%u framebuffer=%p",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer)),
		m_currentSwapChainImage,
		reinterpret_cast<void*>(static_cast<VkFramebuffer>(m_swapchainFramebuffers[m_currentSwapChainImage]->getFramebuffer())));
	vk::RenderPassBeginInfo renderPassBegin;
	renderPassBegin.renderPass = m_renderPassManager->getPresentRenderPass();
	renderPassBegin.framebuffer = m_swapchainFramebuffers[m_currentSwapChainImage]->getFramebuffer();
	renderPassBegin.renderArea.offset = vk::Offset2D{0, 0};
	renderPassBegin.renderArea.extent = m_swapChainExtent;

	// Don't care about clear - fullscreen quad overwrites everything
	vk::ClearValue clearColor;
	clearColor.color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	renderPassBegin.clearValueCount = 1;
	renderPassBegin.pClearValues = &clearColor;

	cmdBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

	// Allocate descriptor set for scene texture
	if (m_descriptorManager && m_blitDescriptorSetLayout) {
		vk::DescriptorSet blitDescSet = m_descriptorManager->allocateSet(m_blitDescriptorSetLayout.get(), "BlitSceneTexture");

		// Update descriptor with scene color texture
		m_descriptorManager->updateCombinedImageSampler(blitDescSet, 0,
		    m_sceneFramebuffer->getColorImageView(0),
		    m_blitSampler.get(),
		    vk::ImageLayout::eShaderReadOnlyOptimal);
		vk_logf("VulkanRenderer",
			"recordBlitToSwapchain: blit descriptor set=%p",
			reinterpret_cast<void*>(static_cast<VkDescriptorSet>(blitDescSet)));

		// Bind pipeline and descriptor set
		cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_blitPipeline.get());
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		    m_blitPipelineLayout.get(), 0, {blitDescSet}, {});

		// Draw fullscreen triangle (3 vertices, generated in vertex shader)
		cmdBuffer.draw(3, 1, 0, 0);
	}

	cmdBuffer.endRenderPass();
	vk_logf("VulkanRenderer",
		"recordBlitToSwapchain: end render pass cmd=%p",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer)));
}

void VulkanRenderer::queueDescriptorSetFree(vk::DescriptorSet set)
{
	if (!set || !m_descriptorManager) {
		return;
	}

	if (m_frames[m_currentFrame]) {
		m_frames[m_currentFrame]->onFrameFinished([this, set]() {
			if (m_descriptorManager) {
				m_descriptorManager->freeSet(set);
			}
		});
	} else {
		m_descriptorManager->freeSet(set);
	}
}

void VulkanRenderer::submitAuxiliaryCommandBuffer()
{
	if (m_auxiliaryPassActive) {
		vk_logf("VulkanRenderer",
			"submitAuxiliaryCommandBuffer called while auxiliary pass still active (frame=%u)",
			m_currentFrame);
		return;
	}

	if (!m_sceneCommandBuffer) {
		return; // Nothing recorded
	}

	vk_logf("VulkanRenderer",
		"submitAuxiliaryCommandBuffer: ending and submitting cmd=%p frame=%u",
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)),
		m_currentFrame);

	// Finish recording
	m_sceneCommandBuffer.end();

	std::vector<vk::CommandBuffer> cmdBuffers = {m_sceneCommandBuffer};

	// Submit and block until complete so results are usable immediately
	if (m_frames[m_currentFrame]) {
		m_frames[m_currentFrame]->submitImmediateBlocking(cmdBuffers);
	} else {
		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
		submitInfo.pCommandBuffers = cmdBuffers.data();
		m_graphicsQueue.submit(submitInfo, nullptr);
		m_graphicsQueue.waitIdle();
	}

	// Command buffer is no longer needed
	m_device->freeCommandBuffers(m_graphicsCommandPool.get(), {m_sceneCommandBuffer});
	m_sceneCommandBuffer = nullptr;
	m_drawState.reset();
	m_sceneExtent = vk::Extent2D{0, 0};
}

void VulkanRenderer::flip()
{
	static int flipCount = 0;
	char buf[64];
	snprintf(buf, sizeof(buf), "flip() entry #%d", ++flipCount);
	vk_debug(buf);

	// Submit any pending transfers FIRST - this must happen before any graphics submission
	// and after fence synchronization (which happened in acquireNextSwapChainImage)
	if (m_bufferManager) {
		if (m_frames[m_currentFrame]) {
			vk_logf("VulkanRenderer",
				"pre-transfer fence=%p inFlight=%d frame=%u",
				reinterpret_cast<void*>(static_cast<VkFence>(m_frames[m_currentFrame]->getFence())),
				m_frames[m_currentFrame]->isInFlight() ? 1 : 0,
				m_currentFrame);
		}
		vk_debug("flip() submitting buffer transfers");
		m_bufferManager->submitTransfers();
	}
	if (m_textureManager) {
		vk_debug("flip() submitting texture uploads");
		m_textureManager->submitUploads();
	}

	vk_logf("VulkanRenderer",
		"flip state: scenePass=%d directPass=%d frame=%u",
		m_scenePassActive ? 1 : 0,
		m_directPassActive ? 1 : 0,
		m_currentFrame);

	PresentResult presentResult = PresentResult::Success;

	if (m_scenePassActive) {
		vk_logf("VulkanRenderer",
			"flip path: scene pass blit+present (cmd=%p swapImage=%u)",
			reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)),
			m_currentSwapChainImage);
		// Scene pass was used - blit scene to swapchain
		recordBlitToSwapchain(m_sceneCommandBuffer);

		// End the command buffer
		vk_logf("VulkanRenderer",
			"flip scene: ending command buffer %p",
			reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)));
		m_sceneCommandBuffer.end();

		// Schedule command buffer cleanup
		vk::CommandBuffer cmdToFree = m_sceneCommandBuffer;
		m_frames[m_currentFrame]->onFrameFinished([this, cmdToFree]() {
			m_device->freeCommandBuffers(m_graphicsCommandPool.get(), {cmdToFree});
		});

		// Submit and present with per-image semaphores
		vk::Semaphore acquireSem = m_imageAvailableSemaphores[m_currentFrame].get();
		vk::Semaphore renderSem = m_renderingFinishedSemaphores[m_currentSwapChainImage].get();
		presentResult = m_frames[m_currentFrame]->submitAndPresent({m_sceneCommandBuffer}, acquireSem, renderSem);

		m_scenePassActive = false;
		m_sceneCommandBuffer = nullptr;
	} else if (m_directPassActive) {
		vk_logf("VulkanRenderer",
			"flip path: direct pass present (cmd=%p swapImage=%u)",
			reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)),
			m_currentSwapChainImage);
		// Direct pass was used (menu/UI rendering) - already rendered to swapchain
		// Just need to end the render pass and submit
		m_sceneCommandBuffer.endRenderPass();
		vk_logf("VulkanRenderer",
			"flip direct: ending command buffer %p",
			reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_sceneCommandBuffer)));
		m_sceneCommandBuffer.end();

		// Schedule command buffer cleanup
		vk::CommandBuffer cmdToFree = m_sceneCommandBuffer;
		m_frames[m_currentFrame]->onFrameFinished([this, cmdToFree]() {
			m_device->freeCommandBuffers(m_graphicsCommandPool.get(), {cmdToFree});
		});

		// Submit and present with per-image semaphores
		vk::Semaphore acquireSem = m_imageAvailableSemaphores[m_currentFrame].get();
		vk::Semaphore renderSem = m_renderingFinishedSemaphores[m_currentSwapChainImage].get();
		presentResult = m_frames[m_currentFrame]->submitAndPresent({m_sceneCommandBuffer}, acquireSem, renderSem);

		m_directPassActive = false;
		m_sceneCommandBuffer = nullptr;
	} else {
		vk_logf("VulkanRenderer", "flip path: fallback triangle");
		// No pass was started - draw debug triangle (fallback)
		vk_debug("flip() fallback triangle path");

		// NOTE: Transfers already submitted at start of flip()

		vk_debug("flip() allocating command buffer");
		vk::CommandBufferAllocateInfo cmdBufferAlloc;
		cmdBufferAlloc.commandPool = m_graphicsCommandPool.get();
		cmdBufferAlloc.level = vk::CommandBufferLevel::ePrimary;
		cmdBufferAlloc.commandBufferCount = 1;

		auto allocatedBuffers = m_device->allocateCommandBuffers(cmdBufferAlloc);
		auto& cmdBuffer = allocatedBuffers.front();

		vk_logf("VulkanRenderer",
			"flip fallback: command buffer %p swapImage=%u",
			reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer)),
			m_currentSwapChainImage);

		vk_debug("flip() drawing scene");
		drawScene(m_swapchainFramebuffers[m_currentSwapChainImage]->getFramebuffer(), cmdBuffer);
		m_frames[m_currentFrame]->onFrameFinished([this, allocatedBuffers]() mutable {
			m_device->freeCommandBuffers(m_graphicsCommandPool.get(), allocatedBuffers);
			allocatedBuffers.clear();
		});

		vk_debug("flip() submitting and presenting");
		// Submit and present with per-image semaphores
		vk::Semaphore acquireSem = m_imageAvailableSemaphores[m_currentFrame].get();
		vk::Semaphore renderSem = m_renderingFinishedSemaphores[m_currentSwapChainImage].get();
		presentResult = m_frames[m_currentFrame]->submitAndPresent(allocatedBuffers, acquireSem, renderSem);
		vk_debug("flip() present complete");
	}

	// Handle present result
	switch (presentResult) {
	case PresentResult::Success:
		vk_logf("VulkanRenderer", "present result: success");
		break;
	case PresentResult::Suboptimal:
		// Frame was presented but swapchain should be recreated
		vk_logf("VulkanRenderer", "Vulkan: Present suboptimal, marking swapchain for recreation");
		m_swapchainNeedsRecreate = true;
		break;
	case PresentResult::OutOfDate:
		// Swapchain became out of date - will be recreated on next acquire
		vk_logf("VulkanRenderer", "Vulkan: Present out of date, swapchain will be recreated");
		m_swapchainNeedsRecreate = true;
		break;
	case PresentResult::Error:
		vk_logf("VulkanRenderer", "Vulkan: Fatal error during presentation");
		break;
	}

	// Check if we should recreate the swapchain now (deferred from suboptimal acquire)
	if (m_swapchainNeedsRecreate) {
		vk_logf("VulkanRenderer", "flip swapchain recreate needed");
		if (!recreateSwapChain()) {
			// Recreation failed (e.g., window minimized) - skip this frame
			vk_logf("VulkanRenderer", "flip swapchain recreate FAILED, skipping frame");
			m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
			return;
		}
	}

	// Advance counters to prepare for the next frame
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	vk_debug("flip() acquiring next swapchain image");
	acquireNextSwapChainImage();
	vk_debug("flip() complete");
}
void VulkanRenderer::waitForAllFrames()
{
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (m_frames[i]) {
			m_frames[i]->waitForFinish();
		}
	}
}

void VulkanRenderer::cleanupSwapChain()
{
	// Cleanup swapchain framebuffers
	m_swapchainFramebuffers.clear();

	// Cleanup swapchain image views
	m_swapChainImageViews.clear();

	// Cleanup per-image render semaphores
	m_renderingFinishedSemaphores.clear();

	// Reset swapchain image tracking
	m_swapChainImageRenderImage.clear();
	m_swapChainImages.clear();

	// The swapchain itself will be destroyed when we create a new one with oldSwapchain set
}

bool VulkanRenderer::recreateSwapChain()
{
	vk_logf("VulkanRenderer", "Vulkan: Recreating swapchain...");

	// Wait for all GPU work to complete
	waitForAllFrames();
	m_device->waitIdle();

	// Re-query surface capabilities (size may have changed)
	m_cachedDeviceValues.surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_vkSurface.get());

	// Check for zero-size window (minimized)
	if (m_cachedDeviceValues.surfaceCapabilities.currentExtent.width == 0 ||
	    m_cachedDeviceValues.surfaceCapabilities.currentExtent.height == 0) {
		vk_logf("VulkanRenderer", "Vulkan: Window minimized, skipping swapchain recreation");
		return false;
	}

	// Store old swapchain for passing to createSwapChain
	vk::SwapchainKHR oldSwapchain = m_swapChain.get();

	// Cleanup swapchain-dependent resources
	cleanupSwapChain();

	// Recreate swapchain (will use oldSwapchain internally if supported)
	if (!createSwapChain(m_cachedDeviceValues)) {
		vk_logf("VulkanRenderer", "Vulkan: Failed to recreate swapchain");
		return false;
	}

	// Destroy old swapchain now that new one is created
	if (oldSwapchain) {
		m_device->destroySwapchainKHR(oldSwapchain);
	}

	// Recreate swapchain framebuffers
	createSwapchainFramebuffers();

	// Recreate per-image render semaphores (one per swapchain image)
	constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo;
	const size_t imageCount = m_swapChainImages.size();
	m_renderingFinishedSemaphores.resize(imageCount);
	for (size_t i = 0; i < imageCount; ++i) {
		m_renderingFinishedSemaphores[i] = m_device->createSemaphoreUnique(semaphoreCreateInfo);
	}

	// Check if scene framebuffer needs recreation (size changed)
	if (m_sceneFramebuffer) {
		auto extent = m_swapChainExtent;
		// Recreate scene framebuffer if size changed
		m_sceneFramebuffer.reset();
		if (!createSceneFramebuffer()) {
			vk_logf("VulkanRenderer", "Vulkan: Failed to recreate scene framebuffer");
			return false;
		}
	}

	// Update RenderFrame objects with new swapchain
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (m_frames[i]) {
			m_frames[i]->updateSwapchain(m_swapChain.get());
		}
	}

	// Reset the swapchain image tracking array
	m_swapChainImageRenderImage.resize(m_swapChainImages.size(), nullptr);

	// Clear the recreation flag
	m_swapchainNeedsRecreate = false;

	vk_logf("VulkanRenderer",
		"Vulkan: Swapchain recreated successfully (%dx%d)",
		m_swapChainExtent.width,
		m_swapChainExtent.height);

	return true;
}

void VulkanRenderer::shutdown()
{
	// Wait for all frames to complete to ensure no drawing is in progress when we destroy the device
	waitForAllFrames();
	// For good measure, also wait until the device is idle
	m_device->waitIdle();

	// Cleanup framebuffers before render passes (framebuffers reference render passes)
	m_sceneFramebuffer.reset();
	m_swapchainFramebuffers.clear();

	// Cleanup render pass manager
	if (m_renderPassManager) {
		m_renderPassManager->shutdown();
		m_renderPassManager.reset();
	}

	// Cleanup shader manager before device destruction
	if (m_shaderManager) {
		m_shaderManager->shutdown();
		m_shaderManager.reset();
	}

	// Cleanup pipeline manager before device destruction
	if (m_pipelineManager) {
		m_pipelineManager->shutdown();
		m_pipelineManager.reset();
	}
	g_vulkanPipelineManager = nullptr;

	// Cleanup descriptor manager before device destruction
	if (m_descriptorManager) {
		m_descriptorManager->shutdown();
		m_descriptorManager.reset();
	}

	// Cleanup texture manager before buffer manager and device destruction
	g_vulkanTextureManager = nullptr;
	m_textureManager.reset();

	// Cleanup buffer manager before device destruction
	g_vulkanBufferManager = nullptr;
	m_bufferManager.reset();
}

void VulkanRenderer::setActiveRenderTarget(VulkanFramebuffer* framebuffer, vk::Extent2D extent, int bitmapHandle)
{
	if (framebuffer) {
		m_activeRenderTarget.framebuffer = framebuffer;
		m_activeRenderTarget.extent = extent;
		m_activeRenderTarget.bitmapHandle = bitmapHandle;
		m_activeRenderTarget.isActive = true;
	} else {
		// Unbind render target, switch back to scene framebuffer
		m_activeRenderTarget.framebuffer = nullptr;
		m_activeRenderTarget.extent = vk::Extent2D{};
		m_activeRenderTarget.bitmapHandle = -1;
		m_activeRenderTarget.isActive = false;
	}
}

void VulkanRenderer::storeRenderTargetFramebuffer(int bitmapHandle, std::unique_ptr<VulkanFramebuffer> framebuffer)
{
	m_renderTargetFramebuffers[bitmapHandle] = std::move(framebuffer);
}

vk::RenderPass VulkanRenderer::getSceneRenderPass() const
{
	return m_renderPassManager ? m_renderPassManager->getSceneRenderPass() : vk::RenderPass();
}

} // namespace vulkan
} // namespace graphics
