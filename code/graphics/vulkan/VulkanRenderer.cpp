
#include "VulkanRenderer.h"
#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "globalincs/version.h"
#include "graphics/grinternal.h"
#include "graphics/post_processing.h"

#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_vulkan.h"
#include "def_files/def_files.h"
#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "libs/renderdoc/renderdoc.h"
#include "mod_table/mod_table.h"

#if SDL_VERSION_ATLEAST(2, 0, 6)
#include <SDL_vulkan.h>
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

extern float flFrametime;

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
	mprintf(("Vulkan: Present mode: %s (Gr_enable_vsync=%d)\n", name, Gr_enable_vsync ? 1 : 0));

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
			mprintf(("Vulkan: Unsupported MSAA count %d, disabling MSAA\n", Cmdline_msaa_enabled));
			Cmdline_msaa_enabled = 0;
			break;
		}

		if (Cmdline_msaa_enabled > 0) {
			if (supported & requested) {
				m_msaaSampleCount = requested;
				mprintf(("Vulkan: MSAA enabled with %dx sample count\n", Cmdline_msaa_enabled));
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
					mprintf(("Vulkan: Requested MSAA %dx not supported, falling back to %dx\n",
						Cmdline_msaa_enabled, fallbackCount));
					Cmdline_msaa_enabled = fallbackCount;
					m_msaaSampleCount = fallback;
				} else {
					mprintf(("Vulkan: No suitable MSAA support, disabling MSAA\n"));
					Cmdline_msaa_enabled = 0;
				}
			}
		}
	}

	if (!createLogicalDevice(deviceValues)) {
		mprintf(("Failed to create logical device.\n"));
		return false;
	}

	createCommandPool(deviceValues);

	if (!createSwapChain(deviceValues)) {
		mprintf(("Failed to create swap chain.\n"));
		return false;
	}

	createDepthResources();
	createRenderPass();
	createFrameBuffers();

	createPresentSyncObjects();

	// Initialize texture manager (needs command pool for uploads)
	m_textureManager = std::unique_ptr<VulkanTextureManager>(new VulkanTextureManager());
	if (!m_textureManager->init(m_device.get(), m_physicalDevice, m_memoryManager.get(),
	                            m_graphicsCommandPool.get(), m_graphicsQueue)) {
		mprintf(("Failed to initialize Vulkan texture manager!\n"));
		return false;
	}
	setTextureManager(m_textureManager.get());

	// Initialize shader manager
	m_shaderManager = std::unique_ptr<VulkanShaderManager>(new VulkanShaderManager());
	if (!m_shaderManager->init(m_device.get())) {
		mprintf(("Failed to initialize Vulkan shader manager!\n"));
		return false;
	}
	setShaderManager(m_shaderManager.get());

	// Initialize descriptor manager
	m_descriptorManager = std::unique_ptr<VulkanDescriptorManager>(new VulkanDescriptorManager());
	if (!m_descriptorManager->init(m_device.get())) {
		mprintf(("Failed to initialize Vulkan descriptor manager!\n"));
		return false;
	}
	setDescriptorManager(m_descriptorManager.get());

	// Initialize pipeline manager
	m_pipelineManager = std::unique_ptr<VulkanPipelineManager>(new VulkanPipelineManager());
	if (!m_pipelineManager->init(m_device.get(), m_shaderManager.get(), m_descriptorManager.get())) {
		mprintf(("Failed to initialize Vulkan pipeline manager!\n"));
		return false;
	}
	setPipelineManager(m_pipelineManager.get());
	m_pipelineManager->loadPipelineCache("vulkan_pipeline.cache");

	// Initialize state tracker
	m_stateTracker = std::unique_ptr<VulkanStateTracker>(new VulkanStateTracker());
	if (!m_stateTracker->init(m_device.get())) {
		mprintf(("Failed to initialize Vulkan state tracker!\n"));
		return false;
	}
	setStateTracker(m_stateTracker.get());

	// Initialize draw manager
	m_drawManager = std::unique_ptr<VulkanDrawManager>(new VulkanDrawManager());
	if (!m_drawManager->init(m_device.get())) {
		mprintf(("Failed to initialize Vulkan draw manager!\n"));
		return false;
	}
	setDrawManager(m_drawManager.get());

	// Initialize post-processing
	m_postProcessor = std::unique_ptr<VulkanPostProcessor>(new VulkanPostProcessor());
	if (!m_postProcessor->init(m_device.get(), m_physicalDevice, m_memoryManager.get(),
	                           m_swapChainExtent, m_depthFormat)) {
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
			mprintf(("Warning: Unable to read post-processing table\n"));
		}
	}

	// Initialize query manager for GPU timestamp profiling
	m_queryManager = std::unique_ptr<VulkanQueryManager>(new VulkanQueryManager());
	if (!m_queryManager->init(m_device.get(), m_physicalDevice.getProperties().limits.timestampPeriod,
	                          m_graphicsCommandPool.get(), m_graphicsQueue)) {
		mprintf(("Warning: Failed to initialize Vulkan query manager, GPU profiling will be disabled\n"));
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
		mprintf(("Error in first SDL_Vulkan_GetInstanceExtensions: %s\n", SDL_GetError()));
		return false;
	}

	SCP_vector<const char*> extensions;
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
			if (!stricmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				m_debugUtilsEnabled = true;
			}
		}
	}

	SCP_vector<const char*> layers;
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
			if (!stricmp(layer.layerName, "VK_LAYER_KHRONOS_validation")) {
				layers.push_back("VK_LAYER_KHRONOS_validation");
			} else if (!stricmp(layer.layerName, "VK_LAYER_LUNARG_core_validation")) {
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
		auto qprops = dev.getQueueFamilyProperties();
		vals.queueProperties.assign(qprops.begin(), qprops.end());
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
	for (const auto& ext : deviceValues.extensions) {
		if (strcmp(ext.extensionName, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0) {
			m_supportsShaderViewportLayerOutput = true;
			enabledExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
			mprintf(("Vulkan: Enabling %s (shadow cascade support)\n", VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME));
			break;
		}
	}

	vk::DeviceCreateInfo deviceCreate;
	deviceCreate.pQueueCreateInfos = queueInfos.data();
	deviceCreate.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
	deviceCreate.pEnabledFeatures = &deviceValues.features;

	deviceCreate.ppEnabledExtensionNames = enabledExtensions.data();
	deviceCreate.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());

	m_device = deviceValues.device.createDeviceUnique(deviceCreate);

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
	if (!m_memoryManager->init(m_physicalDevice, m_device.get())) {
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
		mprintf(("Failed to initialize Vulkan buffer manager!\n"));
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

	return true;
}
void VulkanRenderer::createFrameBuffers()
{
	m_swapChainFramebuffers.reserve(m_swapChainImageViews.size());
	for (const auto& imageView : m_swapChainImageViews) {
		// Attachment 0: color, Attachment 1: depth (shared across all framebuffers)
		const vk::ImageView attachments[] = {
			imageView.get(),
			m_depthImageView.get(),
		};

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo.renderPass = m_renderPass.get();
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;

		m_swapChainFramebuffers.push_back(m_device->createFramebufferUnique(framebufferInfo));
	}
}
vk::Format VulkanRenderer::findDepthFormat()
{
	// Prefer D32_SFLOAT for best precision, fall back to D32_SFLOAT_S8 or D24_UNORM_S8
	const vk::Format candidates[] = {
		vk::Format::eD32Sfloat,
		vk::Format::eD32SfloatS8Uint,
		vk::Format::eD24UnormS8Uint,
	};

	for (auto format : candidates) {
		auto props = m_physicalDevice.getFormatProperties(format);
		if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
			return format;
		}
	}

	// Should never happen on any real GPU
	Error(LOCATION, "Failed to find supported depth format!");
	return vk::Format::eD32Sfloat;
}
void VulkanRenderer::createDepthResources()
{
	m_depthFormat = findDepthFormat();

	// Create depth image
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = m_depthFormat;
	imageInfo.extent.width = m_swapChainExtent.width;
	imageInfo.extent.height = m_swapChainExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	m_depthImage = m_device->createImageUnique(imageInfo);

	// Allocate GPU memory for the depth image
	m_memoryManager->allocateImageMemory(m_depthImage.get(), MemoryUsage::GpuOnly, m_depthImageMemory);

	// Create depth image view
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = m_depthImage.get();
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = m_depthFormat;
	viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	m_depthImageView = m_device->createImageViewUnique(viewInfo);

	mprintf(("Vulkan: Created depth buffer (%dx%d, format %d)\n",
		m_swapChainExtent.width, m_swapChainExtent.height, static_cast<int>(m_depthFormat)));
}
void VulkanRenderer::createRenderPass()
{
	// Attachment 0: Color - clear each frame
	// UI screens draw their own full-screen backgrounds; 3D clears via scene_texture_begin.
	// Popups that need previous frame content use gr_save_screen/gr_restore_screen.
	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = m_swapChainImageFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	// Attachment 1: Depth
	vk::AttachmentDescription depthAttachment;
	depthAttachment.format = m_depthFormat;
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference colorAttachRef;
	colorAttachRef.attachment = 0;
	colorAttachRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthAttachRef;
	depthAttachRef.attachment = 1;
	depthAttachRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachRef;
	subpass.pDepthStencilAttachment = &depthAttachRef;

	vk::SubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
	                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
	                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
	                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	m_renderPass = m_device->createRenderPassUnique(renderPassInfo);

	// Create a second render pass with loadOp=eLoad for resuming the swap chain
	// after post-processing. Same formats/samples = render-pass-compatible with m_renderPass.
	colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
	colorAttachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;

	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	attachments = {colorAttachment, depthAttachment};

	m_renderPassLoad = m_device->createRenderPassUnique(renderPassInfo);
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
		m_frames[i].reset(new VulkanRenderFrame(m_device.get(), m_swapChain.get(), m_graphicsQueue, m_presentQueue));
	}

	m_swapChainImageRenderImage.resize(m_swapChainImages.size(), nullptr);
}
void VulkanRenderer::acquireNextSwapChainImage()
{
	m_frames[m_currentFrame]->waitForFinish();

	// Recreate swap chain if flagged from a previous frame
	if (m_swapChainNeedsRecreation) {
		// Wait for minimized window (0x0 extent) before recreating
		while (true) {
			if (recreateSwapChain()) {
				break;
			}
			// Window is minimized — wait and pump events until surface is valid again
			os_sleep(100);
			SDL_PumpEvents();
		}
	}

	uint32_t imageIndex = 0;
	auto status = m_frames[m_currentFrame]->acquireSwapchainImage(imageIndex);

	if (status == SwapChainStatus::eOutOfDate) {
		// Must recreate immediately and retry
		while (true) {
			if (recreateSwapChain()) {
				break;
			}
			os_sleep(100);
			SDL_PumpEvents();
		}
		status = m_frames[m_currentFrame]->acquireSwapchainImage(imageIndex);
		if (status == SwapChainStatus::eOutOfDate) {
			// If still failing after recreation, flag for next frame
			m_swapChainNeedsRecreation = true;
		}
	}

	if (status == SwapChainStatus::eSuboptimal) {
		m_swapChainNeedsRecreation = true;
	}

	m_currentSwapChainImage = imageIndex;

	// Ensure that this image is no longer in use
	if (m_swapChainImageRenderImage[m_currentSwapChainImage]) {
		m_swapChainImageRenderImage[m_currentSwapChainImage]->waitForFinish();
	}
	// Reserve the image as in use
	m_swapChainImageRenderImage[m_currentSwapChainImage] = m_frames[m_currentFrame].get();
}
void VulkanRenderer::setupFrame()
{
	if (m_frameInProgress) {
		Warning(LOCATION, "VulkanRenderer::setupFrame called while frame already in progress!");
		return;
	}

	// Free completed texture upload command buffers
	Assertion(m_textureManager, "Vulkan TextureManager not initialized in setupFrame!");
	m_textureManager->frameStart();

	// Allocate command buffer for this frame
	vk::CommandBufferAllocateInfo cmdBufferAlloc;
	cmdBufferAlloc.commandPool = m_graphicsCommandPool.get();
	cmdBufferAlloc.level = vk::CommandBufferLevel::ePrimary;
	cmdBufferAlloc.commandBufferCount = 1;

	auto cmdBufs = m_device->allocateCommandBuffers(cmdBufferAlloc);
	m_currentCommandBuffers.assign(cmdBufs.begin(), cmdBufs.end());
	m_currentCommandBuffer = m_currentCommandBuffers.front();

	// Begin command buffer
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	m_currentCommandBuffer.begin(beginInfo);

	Assertion(m_descriptorManager, "Vulkan DescriptorManager not initialized in setupFrame!");
	m_descriptorManager->beginFrame();

	Assertion(m_stateTracker, "Vulkan StateTracker not initialized in setupFrame!");
	m_stateTracker->beginFrame(m_currentCommandBuffer);

	// Reset timestamp queries that were written last frame (must be outside render pass)
	if (m_queryManager) {
		m_queryManager->beginFrame(m_currentCommandBuffer);
	}

	// Reset per-frame flags
	m_sceneDepthCopiedThisFrame = false;

	// Reset per-frame draw statistics
	Assertion(m_drawManager, "Vulkan DrawManager not initialized in setupFrame!");
	m_drawManager->resetFrameStats();

	// Begin render pass
	vk::RenderPassBeginInfo renderPassBegin;
	renderPassBegin.renderPass = m_renderPass.get();
	renderPassBegin.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	renderPassBegin.renderArea.offset.x = 0;
	renderPassBegin.renderArea.offset.y = 0;
	renderPassBegin.renderArea.extent = m_swapChainExtent;

	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});  // Clear to black each frame
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);  // Clear depth to far plane

	renderPassBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBegin.pClearValues = clearValues.data();

	m_currentCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

	// Set up state tracker for FSO draws
	m_stateTracker->setRenderPass(m_renderPass.get(), 0);
	// Negative viewport height for OpenGL-compatible Y-up NDC (VK_KHR_maintenance1)
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(m_swapChainExtent.height),
		static_cast<float>(m_swapChainExtent.width),
		-static_cast<float>(m_swapChainExtent.height));

	m_frameInProgress = true;
}

void VulkanRenderer::flip()
{
	if (!m_frameInProgress) {
		nprintf(("Vulkan", "VulkanRenderer::flip called without frame in progress, skipping\n"));
		return;
	}

	// Print per-frame diagnostic summary before ending
	Assertion(m_drawManager, "Vulkan DrawManager not initialized in flip!");
	m_drawManager->printFrameStats();

	// End render pass
	m_currentCommandBuffer.endRenderPass();
	m_stateTracker->endFrame();
	m_descriptorManager->endFrame();

	// End command buffer
	m_currentCommandBuffer.end();

	// Set up cleanup callback for command buffers
	auto buffersToFree = m_currentCommandBuffers;
	m_frames[m_currentFrame]->onFrameFinished([this, buffersToFree]() mutable {
		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), buffersToFree);
	});

	// Submit and present
	auto presentStatus = m_frames[m_currentFrame]->submitAndPresent(m_currentCommandBuffers);

	if (presentStatus == SwapChainStatus::eSuboptimal || presentStatus == SwapChainStatus::eOutOfDate) {
		m_swapChainNeedsRecreation = true;
	}

	// Notify query manager that this frame's command buffer was submitted
	if (m_queryManager) {
		m_queryManager->notifySubmission();
	}

	// Track which swap chain image was just presented so saveScreen() can read it
	m_previousSwapChainImage = m_currentSwapChainImage;

	// Clear current command buffer reference
	m_currentCommandBuffer = nullptr;
	m_currentCommandBuffers.clear();
	m_frameInProgress = false;

	// Advance counters to prepare for the next frame
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	++m_frameNumber;

	// Set the frame index for the buffer manager immediately after incrementing
	// This ensures any buffer operations that happen before setupFrame() use the correct frame
	m_bufferManager->setCurrentFrame(m_currentFrame);

	acquireNextSwapChainImage();

	// Process deferred resource deletions AFTER the fence wait in
	// acquireNextSwapChainImage, so we know the previous frame's commands
	// (including async upload CBs) have completed before destroying resources.
	m_deletionQueue->processDestructions();
}

bool VulkanRenderer::readbackFramebuffer(ubyte** outPixels, uint32_t* outWidth, uint32_t* outHeight)
{
	*outPixels = nullptr;
	*outWidth = 0;
	*outHeight = 0;

	if (m_previousSwapChainImage == UINT32_MAX) {
		mprintf(("VulkanRenderer::readbackFramebuffer - no previous frame available\n"));
		return false;
	}

	if (!m_frameInProgress) {
		mprintf(("VulkanRenderer::readbackFramebuffer - no frame in progress\n"));
		return false;
	}

	auto prevImage = m_swapChainImages[m_previousSwapChainImage];
	uint32_t w = m_swapChainExtent.width;
	uint32_t h = m_swapChainExtent.height;
	vk::DeviceSize bufferSize = static_cast<vk::DeviceSize>(w) * h * 4;

	// End the current render pass so we can record transfer commands
	m_currentCommandBuffer.endRenderPass();

	// --- One-shot command buffer to copy previous frame to staging buffer ---

	vk::CommandBufferAllocateInfo cmdAlloc;
	cmdAlloc.commandPool = m_graphicsCommandPool.get();
	cmdAlloc.level = vk::CommandBufferLevel::ePrimary;
	cmdAlloc.commandBufferCount = 1;

	auto cmdBuffers = m_device->allocateCommandBuffers(cmdAlloc);
	auto cmd = cmdBuffers.front();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	cmd.begin(beginInfo);

	// Transition previous swap chain image for transfer read
	vk::ImageMemoryBarrier preBarrier;
	preBarrier.oldLayout = vk::ImageLayout::ePresentSrcKHR;
	preBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	preBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	preBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	preBarrier.image = prevImage;
	preBarrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
	preBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	preBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eTransfer,
		{}, nullptr, nullptr, preBarrier);

	// Create staging buffer for readback
	vk::BufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

	auto stagingBuffer = m_device->createBuffer(bufferCreateInfo);

	VulkanAllocation stagingAlloc{};
	if (!m_memoryManager->allocateBufferMemory(stagingBuffer, MemoryUsage::GpuToCpu, stagingAlloc)) {
		mprintf(("VulkanRenderer::readbackFramebuffer - failed to allocate staging buffer\n"));
		m_device->destroyBuffer(stagingBuffer);
		cmd.end();
		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), cmdBuffers);

		// Re-begin render pass so the frame can continue
		vk::RenderPassBeginInfo renderPassBegin;
		renderPassBegin.renderPass = m_renderPass.get();
		renderPassBegin.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
		renderPassBegin.renderArea.offset.x = 0;
		renderPassBegin.renderArea.offset.y = 0;
		renderPassBegin.renderArea.extent = m_swapChainExtent;
		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		renderPassBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_renderPass.get(), 0);
		m_stateTracker->setViewport(0.0f,
			static_cast<float>(m_swapChainExtent.height),
			static_cast<float>(m_swapChainExtent.width),
			-static_cast<float>(m_swapChainExtent.height));
		return false;
	}

	// Copy image to staging buffer
	vk::BufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;   // tightly packed
	region.bufferImageHeight = 0; // tightly packed
	region.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
	region.imageOffset = vk::Offset3D(0, 0, 0);
	region.imageExtent = vk::Extent3D(w, h, 1);

	cmd.copyImageToBuffer(prevImage, vk::ImageLayout::eTransferSrcOptimal, stagingBuffer, region);

	// Transition previous swap chain image back
	vk::ImageMemoryBarrier postBarrier;
	postBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
	postBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
	postBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	postBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	postBarrier.image = prevImage;
	postBarrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
	postBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	postBarrier.dstAccessMask = {};

	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eBottomOfPipe,
		{}, nullptr, nullptr, postBarrier);

	cmd.end();

	// Submit one-shot command buffer and wait
	auto fence = m_device->createFence({});

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	m_graphicsQueue.submit(submitInfo, fence);

	auto waitResult = m_device->waitForFences(fence, VK_TRUE, UINT64_MAX);
	if (waitResult != vk::Result::eSuccess) {
		mprintf(("VulkanRenderer::readbackFramebuffer - fence wait failed\n"));
	}

	m_device->destroyFence(fence);
	m_device->freeCommandBuffers(m_graphicsCommandPool.get(), cmdBuffers);

	// Read back pixels from staging buffer (raw BGRA matching swap chain format)
	bool success = false;
	auto* mappedPtr = static_cast<ubyte*>(m_memoryManager->mapMemory(stagingAlloc));

	if (mappedPtr) {
		auto* pixels = static_cast<ubyte*>(vm_malloc(static_cast<int>(bufferSize)));
		if (pixels) {
			memcpy(pixels, mappedPtr, bufferSize);
			*outPixels = pixels;
			*outWidth = w;
			*outHeight = h;
			success = true;
		}
		m_memoryManager->unmapMemory(stagingAlloc);
	}

	// Free staging buffer
	m_device->destroyBuffer(stagingBuffer);
	m_memoryManager->freeAllocation(stagingAlloc);

	// Re-begin render pass on main command buffer
	vk::RenderPassBeginInfo renderPassBegin;
	renderPassBegin.renderPass = m_renderPass.get();
	renderPassBegin.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	renderPassBegin.renderArea.offset.x = 0;
	renderPassBegin.renderArea.offset.y = 0;
	renderPassBegin.renderArea.extent = m_swapChainExtent;

	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	renderPassBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBegin.pClearValues = clearValues.data();

	m_currentCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

	m_stateTracker->setRenderPass(m_renderPass.get(), 0);
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(m_swapChainExtent.height),
		static_cast<float>(m_swapChainExtent.width),
		-static_cast<float>(m_swapChainExtent.height));

	return success;
}

uint32_t VulkanRenderer::getMinUniformBufferOffsetAlignment() const
{
	if (!m_physicalDevice) {
		// Fallback to common value if device not initialized
		return 256;
	}

	auto properties = m_physicalDevice.getProperties();
	return static_cast<uint32_t>(properties.limits.minUniformBufferOffsetAlignment);
}

uint32_t VulkanRenderer::getMaxUniformBufferSize() const
{
	if (!m_physicalDevice) {
		return 65536;
	}

	auto properties = m_physicalDevice.getProperties();
	return properties.limits.maxUniformBufferRange;
}

float VulkanRenderer::getMaxAnisotropy() const
{
	if (!m_physicalDevice) {
		return 1.0f;
	}

	auto properties = m_physicalDevice.getProperties();
	return properties.limits.maxSamplerAnisotropy;
}

bool VulkanRenderer::isTextureCompressionBCSupported() const
{
	if (!m_physicalDevice) {
		return false;
	}

	auto features = m_physicalDevice.getFeatures();
	return features.textureCompressionBC == VK_TRUE;
}

void VulkanRenderer::waitIdle()
{
	if (m_device) {
		m_device->waitIdle();
	}
}

void VulkanRenderer::waitForFrame(uint64_t frameNumber)
{
	// Fast path: if enough frames have elapsed, the work is definitely done
	if (m_frameNumber >= frameNumber + MAX_FRAMES_IN_FLIGHT) {
		return;
	}

	// Wait on the specific frame's fence
	uint32_t frameIndex = static_cast<uint32_t>(frameNumber % MAX_FRAMES_IN_FLIGHT);
	m_frames[frameIndex]->waitForFinish();
}

VkCommandBuffer VulkanRenderer::getVkCurrentCommandBuffer() const
{
	return static_cast<VkCommandBuffer>(m_currentCommandBuffer);
}

void VulkanRenderer::createImGuiDescriptorPool()
{
	vk::DescriptorPoolSize poolSize;
	poolSize.type = vk::DescriptorType::eCombinedImageSampler;
	poolSize.descriptorCount = 100;

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	poolInfo.maxSets = 100;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	m_imguiDescriptorPool = m_device->createDescriptorPoolUnique(poolInfo);
}

void VulkanRenderer::initImGui()
{
	createImGuiDescriptorPool();

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = static_cast<VkInstance>(*m_vkInstance);
	initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(m_physicalDevice);
	initInfo.Device = static_cast<VkDevice>(*m_device);
	initInfo.QueueFamily = m_graphicsQueueFamilyIndex;
	initInfo.Queue = static_cast<VkQueue>(m_graphicsQueue);
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = static_cast<VkDescriptorPool>(*m_imguiDescriptorPool);
	initInfo.Subpass = 0;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = static_cast<uint32_t>(m_swapChainImages.size());
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&initInfo, static_cast<VkRenderPass>(*m_renderPass));

	// Upload font textures via one-time command buffer
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

		ImGui_ImplVulkan_CreateFontsTexture(static_cast<VkCommandBuffer>(cmd));

		cmd.end();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		m_graphicsQueue.submit(submitInfo, nullptr);
		m_graphicsQueue.waitIdle();

		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), cmdBuffers);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	mprintf(("Vulkan: ImGui backend initialized successfully\n"));
}

void VulkanRenderer::shutdownImGui()
{
	ImGui_ImplVulkan_Shutdown();
	m_imguiDescriptorPool.reset();
	mprintf(("Vulkan: ImGui backend shut down\n"));
}

void VulkanRenderer::beginSceneRendering()
{
	if (!m_postProcessor || !m_postProcessor->isInitialized()) {
		return;
	}
	if (m_sceneRendering) {
		return;
	}

	// End the current swap chain render pass
	m_currentCommandBuffer.endRenderPass();

	// Use G-buffer render pass when deferred lighting is enabled and G-buffer is ready
	m_useGbufRenderPass = m_postProcessor->isGbufInitialized() && light_deferred_enabled();

	// Begin the HDR scene render pass (or G-buffer render pass for deferred)
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_postProcessor->getSceneExtent();

	if (m_useGbufRenderPass) {
		rpBegin.renderPass = m_postProcessor->getGbufRenderPass();
		rpBegin.framebuffer = m_postProcessor->getGbufFramebuffer();

		// 7 clear values: 6 color + depth
		std::array<vk::ClearValue, 7> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f}); // color
		clearValues[1].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f}); // position
		clearValues[2].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f}); // normal
		clearValues[3].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f}); // specular
		clearValues[4].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f}); // emissive
		clearValues[5].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f}); // composite
		clearValues[6].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();

		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->getGbufRenderPass(), 0);
		m_stateTracker->setColorAttachmentCount(VulkanPostProcessor::GBUF_COLOR_ATTACHMENT_COUNT);
	} else {
		rpBegin.renderPass = m_postProcessor->getSceneRenderPass();
		rpBegin.framebuffer = m_postProcessor->getSceneFramebuffer();

		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();

		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->getSceneRenderPass(), 0);
		m_stateTracker->setColorAttachmentCount(1);
	}

	// Negative viewport height for Y-flip (same as swap chain pass)
	auto extent = m_postProcessor->getSceneExtent();
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(extent.height),
		static_cast<float>(extent.width),
		-static_cast<float>(extent.height));

	m_sceneRendering = true;
}

void VulkanRenderer::endSceneRendering()
{
	if (!m_postProcessor || !m_postProcessor->isInitialized()) {
		return;
	}
	if (!m_sceneRendering) {
		return;
	}

	// End HDR scene render pass (transitions scene color to eShaderReadOnlyOptimal)
	m_currentCommandBuffer.endRenderPass();

	// Update distortion ping-pong textures (every ~30ms, matching OpenGL)
	if (Gr_framebuffer_effects.any_set()) {
		m_postProcessor->updateDistortion(m_currentCommandBuffer, flFrametime);
	}

	// Execute post-processing passes (all between HDR scene pass and swap chain pass)
	m_postProcessor->executeBloom(m_currentCommandBuffer);
	m_postProcessor->executeTonemap(m_currentCommandBuffer);
	m_postProcessor->executeFXAA(m_currentCommandBuffer);
	m_postProcessor->executeLightshafts(m_currentCommandBuffer);
	m_postProcessor->executePostEffects(m_currentCommandBuffer);

	// Begin the resumed swap chain render pass (loadOp=eLoad to preserve pre-scene content)
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = m_renderPassLoad.get();
	rpBegin.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_swapChainExtent;

	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
	rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
	rpBegin.pClearValues = clearValues.data();

	m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);

	// Update state tracker for the resumed swap chain pass
	m_stateTracker->setRenderPass(m_renderPassLoad.get(), 0);
	m_stateTracker->setColorAttachmentCount(1);
	// Non-flipped viewport for post-processing blit (HDR texture is already correct orientation)
	m_stateTracker->setViewport(0.0f, 0.0f,
		static_cast<float>(m_swapChainExtent.width),
		static_cast<float>(m_swapChainExtent.height));

	// Blit the HDR scene to swap chain through post-processing
	m_postProcessor->blitToSwapChain(m_currentCommandBuffer);

	// Restore Y-flipped viewport for HUD rendering
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(m_swapChainExtent.height),
		static_cast<float>(m_swapChainExtent.width),
		-static_cast<float>(m_swapChainExtent.height));

	m_sceneRendering = false;
	m_useGbufRenderPass = false;
}

void VulkanRenderer::copyEffectTexture()
{
	if (!m_sceneRendering || !m_postProcessor || !m_postProcessor->isInitialized()) {
		return;
	}

	// End the current scene render pass
	// This transitions scene color to eShaderReadOnlyOptimal (the render pass's finalLayout)
	// For G-buffer: all 6 color attachments transition to eShaderReadOnlyOptimal
	m_currentCommandBuffer.endRenderPass();

	// Copy scene color → effect texture (handles scene color transitions)
	m_postProcessor->copyEffectTexture(m_currentCommandBuffer);

	// If G-buffer is active, transition attachments 1-5 for render pass resume
	if (m_useGbufRenderPass) {
		m_postProcessor->transitionGbufForResume(m_currentCommandBuffer);
	}

	// Resume the scene render pass with loadOp=eLoad to preserve existing content
	// Scene color is now in eColorAttachmentOptimal (copyEffectTexture transitions it back)
	// Depth is still in eDepthStencilAttachmentOptimal (untouched by the copy)
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_postProcessor->getSceneExtent();

	if (m_useGbufRenderPass) {
		rpBegin.renderPass = m_postProcessor->getGbufRenderPassLoad();
		rpBegin.framebuffer = m_postProcessor->getGbufFramebuffer();
		// Clear values ignored for eLoad but array must cover all attachments
		std::array<vk::ClearValue, 7> clearValues{};
		clearValues[6].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->getGbufRenderPassLoad(), 0);
	} else {
		rpBegin.renderPass = m_postProcessor->getSceneRenderPassLoad();
		rpBegin.framebuffer = m_postProcessor->getSceneFramebuffer();
		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->getSceneRenderPassLoad(), 0);
	}

	// Restore Y-flipped viewport for scene rendering
	auto extent = m_postProcessor->getSceneExtent();
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(extent.height),
		static_cast<float>(extent.width),
		-static_cast<float>(extent.height));
}

void VulkanRenderer::copySceneDepthForParticles()
{
	if (m_sceneDepthCopiedThisFrame || !m_sceneRendering || !m_postProcessor || !m_postProcessor->isInitialized()) {
		return;
	}

	// End the current scene render pass
	// This transitions: color → eShaderReadOnlyOptimal, depth → eDepthStencilAttachmentOptimal
	// For G-buffer: all 6 color attachments → eShaderReadOnlyOptimal
	m_currentCommandBuffer.endRenderPass();

	// Copy scene depth → samplable depth copy (handles all depth image transitions)
	m_postProcessor->copySceneDepth(m_currentCommandBuffer);

	// Transition scene color: eShaderReadOnlyOptimal → eColorAttachmentOptimal
	// (needed for the resumed render pass with loadOp=eLoad, which expects
	// initialLayout=eColorAttachmentOptimal; copySceneDepth only touches depth)
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_postProcessor->getSceneColorImage();
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		m_currentCommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, {}, {}, barrier);
	}

	// If G-buffer is active, transition attachments 1-5 for render pass resume
	if (m_useGbufRenderPass) {
		m_postProcessor->transitionGbufForResume(m_currentCommandBuffer);
	}

	// Resume the scene render pass with loadOp=eLoad
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_postProcessor->getSceneExtent();

	if (m_useGbufRenderPass) {
		rpBegin.renderPass = m_postProcessor->getGbufRenderPassLoad();
		rpBegin.framebuffer = m_postProcessor->getGbufFramebuffer();
		std::array<vk::ClearValue, 7> clearValues{};
		clearValues[6].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->getGbufRenderPassLoad(), 0);
	} else {
		rpBegin.renderPass = m_postProcessor->getSceneRenderPassLoad();
		rpBegin.framebuffer = m_postProcessor->getSceneFramebuffer();
		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->getSceneRenderPassLoad(), 0);
	}

	// Restore Y-flipped viewport for scene rendering
	auto extent = m_postProcessor->getSceneExtent();
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(extent.height),
		static_cast<float>(extent.width),
		-static_cast<float>(extent.height));

	m_sceneDepthCopiedThisFrame = true;
}

bool VulkanRenderer::recreateSwapChain()
{
	mprintf(("Vulkan: Recreating swap chain...\n"));

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
		mprintf(("Vulkan: Surface extent is 0x0 (minimized), deferring swap chain recreation\n"));
		return false;
	}

	// Recreate swap chain, image views, and framebuffers
	// (createSwapChain clears old resources and transitions new images internally)
	createSwapChain(freshValues, m_swapChain.get());
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

	mprintf(("Vulkan: Swap chain recreated successfully (%ux%u, %zu images)\n",
		m_swapChainExtent.width, m_swapChainExtent.height, m_swapChainImages.size()));

	return true;
}

void VulkanRenderer::shutdown()
{
	// Wait for all frames to complete to ensure no drawing is in progress when we destroy the device
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		m_frames[i]->waitForFinish();
	}
	// For good measure, also wait until the device is idle
	m_device->waitIdle();

	// Shutdown ImGui Vulkan backend before destroying any Vulkan objects
	shutdownImGui();

	// Shutdown managers in reverse order of initialization
	if (m_queryManager) {
		setQueryManager(nullptr);
		m_queryManager->shutdown();
		m_queryManager.reset();
	}

	if (m_postProcessor) {
		setPostProcessor(nullptr);
		m_postProcessor->shutdown();
		m_postProcessor.reset();
	}

	// Clean up shared post-processing manager
	if (graphics::Post_processing_manager) {
		graphics::Post_processing_manager->clear();
		graphics::Post_processing_manager = nullptr;
	}

	if (m_drawManager) {
		setDrawManager(nullptr);
		m_drawManager->shutdown();
		m_drawManager.reset();
	}

	if (m_stateTracker) {
		setStateTracker(nullptr);
		m_stateTracker->shutdown();
		m_stateTracker.reset();
	}

	if (m_pipelineManager) {
		m_pipelineManager->savePipelineCache("vulkan_pipeline.cache");
		setPipelineManager(nullptr);
		m_pipelineManager->shutdown();
		m_pipelineManager.reset();
	}

	if (m_descriptorManager) {
		setDescriptorManager(nullptr);
		m_descriptorManager->shutdown();
		m_descriptorManager.reset();
	}

	if (m_shaderManager) {
		setShaderManager(nullptr);
		m_shaderManager->shutdown();
		m_shaderManager.reset();
	}

	if (m_textureManager) {
		setTextureManager(nullptr);
		m_textureManager->shutdown();
		m_textureManager.reset();
	}

	if (m_bufferManager) {
		setBufferManager(nullptr);
		m_bufferManager->shutdown();
		m_bufferManager.reset();
	}

	// Destroy depth resources before memory manager
	m_depthImageView.reset();
	m_depthImage.reset();
	if (m_memoryManager && m_depthImageMemory.memory) {
		m_memoryManager->freeAllocation(m_depthImageMemory);
	}

	// Deletion queue must be flushed before memory manager shutdown
	if (m_deletionQueue) {
		setDeletionQueue(nullptr);
		m_deletionQueue->shutdown();
		m_deletionQueue.reset();
	}

	if (m_memoryManager) {
		setMemoryManager(nullptr);
		m_memoryManager->shutdown();
		m_memoryManager.reset();
	}
}

} // namespace vulkan
} // namespace graphics
