
#include "VulkanRenderer.h"

#include "globalincs/version.h"

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

VkBool32 VKAPI_PTR debugReportCallback(VkDebugReportFlagsEXT /*flags*/,
	VkDebugReportObjectTypeEXT /*objectType*/,
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
			values.properties.deviceName,
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
			values.properties.deviceName,
			values.properties.deviceID));
		return true;
	}
	if (!values.transferQueueIndex.initialized) {
		mprintf(("Rejecting %s (%d) because the device does not have a transfer queue.\n",
			values.properties.deviceName,
			values.properties.deviceID));
		return true;
	}
	if (!values.presentQueueIndex.initialized) {
		mprintf(("Rejecting %s (%d) because the device does not have a presentation queue.\n",
			values.properties.deviceName,
			values.properties.deviceID));
		return true;
	}

	if (!checkDeviceExtensionSupport(values)) {
		mprintf(("Rejecting %s (%d) because the device does not support our required extensions.\n",
			values.properties.deviceName,
			values.properties.deviceID));
		return true;
	}

	if (!checkSwapChainSupport(values, surface)) {
		mprintf(("Rejecting %s (%d) because the device swap chain was not compatible.\n",
			values.properties.deviceName,
			values.properties.deviceID));
		return true;
	}

	return false;
}

int32_t deviceTypeScore(vk::PhysicalDeviceType type)
{
	switch (type) {
	case vk::PhysicalDeviceType::eIntegratedGpu:
		return 1;
	case vk::PhysicalDeviceType::eDiscreteGpu:
		return 2;
	case vk::PhysicalDeviceType::eVirtualGpu:
		return 0;
	default:
		return -1;
	}
}

int32_t scoreDevice(const PhysicalDeviceValues& device)
{
	int32_t score = 0;

	score += deviceTypeScore(device.properties.deviceType) * 1000;

	return score;
}

bool compareDevices(const PhysicalDeviceValues& left, const PhysicalDeviceValues& right)
{
	return scoreDevice(left) < scoreDevice(right);
}

void printPhysicalDevice(const PhysicalDeviceValues& values)
{
	mprintf(("  Found %s (%d) of type %s. API version %d.%d.%d, Driver version %d.%d.%d. Scored as %d\n",
		values.properties.deviceName,
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
	for (const auto& availableFormat : values.surfaceFormats) {
		// Simple check is enough for now
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
			availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

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

	if (!createSwapChain(deviceValues)) {
		mprintf(("Failed to create swap chain.\n"));
		return false;
	}

	createFrameBuffers();

	return true;
}

bool VulkanRenderer::initDisplayDevice()
{
	os::ViewPortProperties attrs;
	attrs.enable_opengl = false;
	attrs.enable_vulkan = true;

	attrs.display = os_config_read_uint("Video", "Display", 0);
	attrs.width = (uint32_t)gr_screen.max_w;
	attrs.height = (uint32_t)gr_screen.max_h;

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

	auto viewPort = m_graphicsOps->createViewport(attrs);
	if (!viewPort) {
		return false;
	}

	auto port = os::addViewport(std::move(viewPort));
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
		mprintf(("  Found support for %s version %" PRIu32 "\n", ext.extensionName, ext.specVersion));
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
			layer.layerName,
			layer.description,
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

	vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> deleter(*m_vkInstance,
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
		deviceValues.properties.deviceName,
		deviceValues.properties.deviceID));
	mprintf(("Device extensions:\n"));
	for (const auto& extProp : deviceValues.extensions) {
		mprintf(("  Found support for %s version %" PRIu32 "\n", extProp.extensionName, extProp.specVersion));
	}

	return true;
}

bool VulkanRenderer::createLogicalDevice(PhysicalDeviceValues& deviceValues)
{
	float queuePriority = 1.0f;

	std::vector<vk::DeviceQueueCreateInfo> queueInfos;
	std::set<uint32_t> familyIndices{deviceValues.graphicsQueueIndex.index,
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
bool VulkanRenderer::createSwapChain(PhysicalDeviceValues& deviceValues)
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

	uint32_t queueFamilyIndices[] = {deviceValues.graphicsQueueIndex.index, deviceValues.presentQueueIndex.index};
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

	m_swapChainImages = m_device->getSwapchainImagesKHR(m_swapChain.get());
	m_swapChainImageFormat = surfaceFormat.format;
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
void VulkanRenderer::createFrameBuffers() {
	m_swapChainFramebuffers.reserve(m_swapChainImageViews.size());
}

} // namespace vulkan
} // namespace graphics
