
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"

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
	}

	// Debug: show HDR state
	mprintf(("Vulkan HDR state: capable=%d, mode=%d, enabled=%d\n",
		Gr_hdr_output_capable ? 1 : 0,
		static_cast<int>(Gr_hdr_output_mode),
		gr_hdr_output_enabled() ? 1 : 0));

	// HDR10 path: select 10-bit format with PQ transfer function when HDR is enabled
	if (gr_hdr_output_enabled()) {
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

	// SDR path: standard 8-bit sRGB
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
	mprintf(("Initializing Vulkan graphics device at %ix%i with %i-bit color...\n",
		gr_screen.max_w,
		gr_screen.max_h,
		gr_screen.bits_per_pixel));

	// Load the RenderDoc API if available before doing anything with OpenGL
	renderdoc::loadApi();

	if (!initDisplayDevice()) {
		return false;
	}

	if (!initializeInstance()) {
		mprintf(("Failed to create Vulkan instance!\n"));
		return false;
	}

	if (!initializeSurface()) {
		mprintf(("Failed to create Vulkan surface!\n"));
		return false;
	}

	PhysicalDeviceValues deviceValues;
	if (!pickPhysicalDevice(deviceValues)) {
		mprintf(("Could not find suitable physical Vulkan device.\n"));
		return false;
	}

	if (!createLogicalDevice(deviceValues)) {
		mprintf(("Failed to create logical device.\n"));
		return false;
	}

	// Cache physical device and initialize buffer manager
	m_physicalDevice = deviceValues.device;
	m_bufferManager = std::make_unique<VulkanBufferManager>(m_device.get(), m_physicalDevice);
	g_vulkanBufferManager = m_bufferManager.get();

#ifdef FSO_HAVE_SHADERC
	// Initialize shader manager for runtime GLSL->SPIR-V compilation
	m_shaderManager = std::make_unique<VulkanShaderManager>(m_device.get());
	if (!m_shaderManager->initialize()) {
		mprintf(("Warning: Vulkan shader manager initialization failed - will use precompiled shaders only\n"));
		m_shaderManager.reset();
	} else {
		mprintf(("Vulkan shader manager initialized (runtime GLSL compilation available)\n"));
	}
#endif

	// Initialize descriptor manager for allocating and binding descriptor sets
	m_descriptorManager = std::make_unique<VulkanDescriptorManager>();
	if (!m_descriptorManager->initialize(m_device.get(), m_physicalDevice)) {
		mprintf(("Failed to initialize Vulkan descriptor manager\n"));
		return false;
	}

	// Initialize pipeline manager for creating and caching graphics pipelines
	m_pipelineManager = std::make_unique<VulkanPipelineManager>();
	if (!m_pipelineManager->initialize(m_device.get(), m_physicalDevice, m_shaderManager.get())) {
		mprintf(("Failed to initialize Vulkan pipeline manager\n"));
		return false;
	}
	g_vulkanPipelineManager = m_pipelineManager.get();

	if (!createSwapChain(deviceValues)) {
		mprintf(("Failed to create swap chain.\n"));
		return false;
	}

	// Initialize render pass manager and create render passes
	m_renderPassManager = std::make_unique<VulkanRenderPassManager>();
	if (!m_renderPassManager->initialize(m_device.get())) {
		mprintf(("Failed to initialize render pass manager\n"));
		return false;
	}

	if (!createRenderPasses()) {
		mprintf(("Failed to create render passes\n"));
		return false;
	}

	// Create scene framebuffer (color + depth render target)
	if (!createSceneFramebuffer()) {
		mprintf(("Failed to create scene framebuffer\n"));
		return false;
	}

	// Create swapchain framebuffers for presentation
	createSwapchainFramebuffers();

	createGraphicsPipeline();
	createPresentSyncObjects();
	createCommandPool(deviceValues);

	// Prepare the rendering state by acquiring our first swap chain image
	acquireNextSwapChainImage();

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
		if (FSO_DEBUG || Cmdline_graphics_debug_output) {
			if (!stricmp(ext.extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				m_debugReportEnabled = true;
			}
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
		if (FSO_DEBUG || Cmdline_graphics_debug_output) {
			if (!stricmp(layer.layerName, "VK_LAYER_LUNARG_core_validation")) {
				layers.push_back("VK_LAYER_LUNARG_core_validation");
			}
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

	if (deviceValues.supportsHDR10) {
		Gr_hdr_output_capable = true;
		mprintf(("Vulkan: HDR10 output is supported\n"));
	} else {
		mprintf(("Vulkan: HDR10 output is not supported (no A2B10G10R10 + ST2084 format)\n"));
	}

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
void VulkanRenderer::createCommandPool(const PhysicalDeviceValues& values)
{
	vk::CommandPoolCreateInfo poolCreate;
	poolCreate.queueFamilyIndex = values.graphicsQueueIndex.index;
	poolCreate.flags |= vk::CommandPoolCreateFlagBits::eTransient;

	m_graphicsCommandPool = m_device->createCommandPoolUnique(poolCreate);
}
void VulkanRenderer::createPresentSyncObjects()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		m_frames[i].reset(new RenderFrame(m_device.get(), m_swapChain.get(), m_graphicsQueue, m_presentQueue));
	}

	m_swapChainImageRenderImage.resize(m_swapChainImages.size(), nullptr);
}
void VulkanRenderer::acquireNextSwapChainImage()
{
	m_frames[m_currentFrame]->waitForFinish();

	m_currentSwapChainImage = m_frames[m_currentFrame]->acquireSwapchainImage();

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

	cmdBuffer.end();
}
void VulkanRenderer::flip()
{
	vk::CommandBufferAllocateInfo cmdBufferAlloc;
	cmdBufferAlloc.commandPool = m_graphicsCommandPool.get();
	cmdBufferAlloc.level = vk::CommandBufferLevel::ePrimary;
	cmdBufferAlloc.commandBufferCount = 1;

	// Uses the non-unique version since we can't get the buffers into the lambda below otherwise. Only C++14 can do
	// that
	auto allocatedBuffers = m_device->allocateCommandBuffers(cmdBufferAlloc);
	auto& cmdBuffer = allocatedBuffers.front();

	drawScene(m_swapchainFramebuffers[m_currentSwapChainImage]->getFramebuffer(), cmdBuffer);
	m_frames[m_currentFrame]->onFrameFinished([this, allocatedBuffers]() mutable {
		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), allocatedBuffers);
		allocatedBuffers.clear();
	});

	m_frames[m_currentFrame]->submitAndPresent(allocatedBuffers);

	// Advance counters to prepare for the next frame
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	acquireNextSwapChainImage();
}
void VulkanRenderer::shutdown()
{
	// Wait for all frames to complete to ensure no drawing is in progress when we destroy the device
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		m_frames[i]->waitForFinish();
	}
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

	// Cleanup buffer manager before device destruction
	g_vulkanBufferManager = nullptr;
	m_bufferManager.reset();
}

} // namespace vulkan
} // namespace graphics
