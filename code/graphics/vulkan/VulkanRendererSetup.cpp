
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

#include <cstdint>


extern float flFrametime;

namespace graphics::vulkan {

namespace {

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

const SCP_vector<const char*> RequiredDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	// Barrier submission across this renderer is written entirely against
	// VK_KHR_synchronization2 (see VulkanBarrier.h) -- there is no legacy
	// vkCmdPipelineBarrier fallback, so the extension is required rather than
	// optionally negotiated.
	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
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

bool isDeviceUnsuitable(PhysicalDeviceValues& values, vk::SurfaceKHR surface)
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
		}
		// No dedicated transfer queue is selected: all uploads run on the graphics
		// queue (which implicitly supports transfer). Async transfer on a separate
		// queue is future work and must be reintroduced end-to-end, including
		// queue-family ownership transfers -- not half-wired.
		if (!values.presentQueueIndex.initialized && values.device.getSurfaceSupportKHR(i, surface)) {
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

	// VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME is in RequiredDeviceExtensions
	// above, but its feature bit still has to be checked independently -- a
	// device could theoretically expose the extension without enabling the
	// feature (same reasoning as bufferDeviceAddress's separate feature check).
	if (!values.synchronization2FeatureSupported) {
		nprintf(("vulkan", "Rejecting %s (%d) because the device does not support the synchronization2 feature.\n",
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
	mprintf(("  Found %s (%d) of type %s. API version %d.%d.%d, Driver version %d.%d.%d. Scored as %" PRIu64 "\n",
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

} // namespace
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

	// Everything from the surface down hangs off a target, so the main one has to exist before
	// createTargetSurface() has anywhere to put its handle. The game only ever has this one; qtFRED
	// adds a second when the briefing map first asks to be rendered into.
	auto mainTarget = std::make_unique<VulkanPresentTarget>();
	mainTarget->viewport = os::getMainViewport();
	m_mainTarget = mainTarget.get();
	m_current = m_mainTarget;
	m_targets.emplace(mainTarget->viewport, std::move(mainTarget));

	try {
		if (!initializeInstance()) {
			mprintf(("Failed to create Vulkan instance!\n"));
			return false;
		}
	} catch(const vk::SystemError &err) {
		mprintf(("Vulkan ERROR: %s\n", err.what()));
		return false;
	}

	if (!createTargetSurface(*m_mainTarget)) {
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

	if (!createSwapChain(*m_mainTarget, deviceValues)) {
		nprintf(("vulkan", "Failed to create swap chain.\n"));
		return false;
	}

	createDepthResources(*m_mainTarget);
	createCompositionResources(*m_mainTarget);
	// Shared by every target, and built here from the main target's negotiated format. Any later
	// target has to match it -- see createTargetResources().
	createEncodeRenderPass(m_mainTarget->imageFormat);
	createRenderPass();
	createFrameBuffers(*m_mainTarget);

	createPresentSyncObjects(*m_mainTarget);

	// Initialize texture manager (needs command pool for uploads)
	m_textureManager = std::make_unique<VulkanTextureManager>();
	if (!m_textureManager->init(m_device.get(), m_physicalDevice, m_memoryManager.get(),
	                            m_graphicsCommandPool.get(), m_graphicsQueue)) {
		nprintf(("vulkan", "Failed to initialize Vulkan texture manager!\n"));
		return false;
	}
	setTextureManager(m_textureManager.get());

	// Initialize shader manager
	m_shaderManager = std::make_unique<VulkanShaderManager>();
	if (!m_shaderManager->init(m_device.get())) {
		nprintf(("vulkan", "Failed to initialize Vulkan shader manager!\n"));
		return false;
	}
	setShaderManager(m_shaderManager.get());

	// Initialize raytraced shadow BLAS/TLAS manager (no-op internally if unsupported).
	// Must happen before the descriptor manager below, since the Global set's
	// layout needs to know whether to include the TLAS binding, and the
	// descriptor fallbacks need the raytracing manager's fallback TLAS.
	m_raytracingManager = std::make_unique<VulkanRaytracingManager>();
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
	m_descriptorManager = std::make_unique<VulkanDescriptorManager>();
	if (!m_descriptorManager->init(m_device.get(), rtDescriptorSupport)) {
		mprintf(("Failed to initialize Vulkan descriptor manager!\n"));
		return false;
	}
	setDescriptorManager(m_descriptorManager.get());
	m_descriptorManager->buildFallbacks(m_bufferManager.get(), m_textureManager.get(), m_raytracingManager.get());

	// Initialize pipeline manager
	m_pipelineManager = std::make_unique<VulkanPipelineManager>();
	if (!m_pipelineManager->init(m_device.get(), m_shaderManager.get(), m_descriptorManager.get())) {
		nprintf(("vulkan", "Failed to initialize Vulkan pipeline manager!\n"));
		return false;
	}
	setPipelineManager(m_pipelineManager.get());
	m_pipelineManager->loadPipelineCache("vulkan_pipeline.cache");

	// Initialize state tracker
	m_stateTracker = std::make_unique<VulkanStateTracker>();
	if (!m_stateTracker->init(m_device.get())) {
		nprintf(("vulkan", "Failed to initialize Vulkan state tracker!\n"));
		return false;
	}
	setStateTracker(m_stateTracker.get());

	// Initialize draw manager
	m_drawManager = std::make_unique<VulkanDrawManager>();
	if (!m_drawManager->init(m_device.get())) {
		nprintf(("vulkan", "Failed to initialize Vulkan draw manager!\n"));
		return false;
	}
	setDrawManager(m_drawManager.get());

	// Initialize post-processing
	m_postProcessor = std::make_unique<VulkanPostProcessor>();
	if (!m_postProcessor->init(m_device.get(), m_physicalDevice, m_memoryManager.get(),
	                           m_mainTarget->extent, m_depthFormat, m_mainTarget->hdrActive)) {
		mprintf(("Warning: Failed to initialize Vulkan post-processor, post-processing will be disabled\n"));
		m_postProcessor.reset();
	} else {
		setPostProcessor(m_postProcessor.get());
	}

	// Initialize shared post-processing manager (bloom/lightshaft settings, post-effect table)
	// This is renderer-agnostic; OpenGL creates it in opengl_post_process_init().
	if (!graphics::Post_processing_manager) {
		graphics::Post_processing_manager = std::make_unique<graphics::PostProcessingManager>();
		if (!graphics::Post_processing_manager->parse_table()) {
			nprintf(("vulkan", "Warning: Unable to read post-processing table\n"));
		}
	}

	// Initialize shared uniform buffer managers; this is renderer-agnostic and OpenGL
	// creates it in gr_opengl_init().
	gr_uniform_buffer_managers_init();

	// Initialize query manager for GPU timestamp profiling
	m_queryManager = std::make_unique<VulkanQueryManager>();
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

	attrs.display = gr_get_preferred_display();
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
	auto* vulkanSupport = m_graphicsOps->getVulkanSupport();
	if (vulkanSupport == nullptr) {
		mprintf(("Vulkan: The windowing implementation in use cannot present through Vulkan!\n"));
		return false;
	}

	const auto vkGetInstanceProcAddr =
		reinterpret_cast<PFN_vkGetInstanceProcAddr>(vulkanSupport->getVulkanProcAddr());
	if (vkGetInstanceProcAddr == nullptr) {
		mprintf(("Vulkan: Could not get vkGetInstanceProcAddr from the windowing system!\n"));
		return false;
	}

	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	VkInstanceCreateFlags createFlags = 0;

	// The windowing system only knows about the extensions its own surfaces need; everything else
	// (debug utils, swap chain color space, portability) is decided below against what the driver
	// actually supports. This must outlive `extensions`, which only holds views into it.
	SCP_vector<SCP_string> windowExtensions;
	if (!vulkanSupport->getVulkanInstanceExtensions(windowExtensions)) {
		mprintf(("Vulkan: Could not determine the instance extensions required by the window system!\n"));
		return false;
	}

	SCP_vector<const char*> extensions;
	extensions.reserve(windowExtensions.size());

	for (const auto& windowExtension : windowExtensions) {
		// SDL 3.2 will include portability enueration extension even if it's not
		// supported, so make sure not to add it blindly, and check for it later
		if (stricmp(windowExtension.c_str(), VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) != 0) {
			extensions.push_back(windowExtension.c_str());
		}
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

	// Enable the Vulkan validation layer + debug callbacks for a debug build,
	// the engine graphics-debug switch (-gr_debug), or the standalone GPU
	// synchronization-validation switch (-gr_sync_validation). The latter
	// deliberately does NOT set Cmdline_graphics_debug_output, so it must be
	// checked here explicitly.
	const bool enableVulkanValidation = FSO_DEBUG || Cmdline_graphics_debug_output || Cmdline_gr_sync_validation;

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
		if (enableVulkanValidation) {
			if (!stricmp(ext.extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				m_debugReportEnabled = true;
			}
			if (!stricmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				m_debugUtilsEnabled = true;
			}
		}

		// if portability enumeration extension is present then we need to add it
		if ( !stricmp(ext.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) ) {
			extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
			createFlags += VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
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
		if (enableVulkanValidation) {
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
	vk::InstanceCreateInfo createInfo(vk::Flags<vk::InstanceCreateFlagBits>(createFlags), &appInfo);
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
			vk::PhysicalDeviceBufferDeviceAddressFeatures,
			vk::PhysicalDeviceSynchronization2Features>();
		vals.features = featureChain.get<vk::PhysicalDeviceFeatures2>().features;
		vals.accelerationStructureFeatureSupported =
			featureChain.get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>().accelerationStructure != VK_FALSE;
		vals.rayQueryFeatureSupported =
			featureChain.get<vk::PhysicalDeviceRayQueryFeaturesKHR>().rayQuery != VK_FALSE;
		vals.bufferDeviceAddressFeatureSupported =
			featureChain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>().bufferDeviceAddress != VK_FALSE;
		vals.synchronization2FeatureSupported =
			featureChain.get<vk::PhysicalDeviceSynchronization2Features>().synchronization2 != VK_FALSE;

		auto qprops = dev.getQueueFamilyProperties();
		vals.queueProperties.assign(qprops.begin(), qprops.end());
		return vals;
	});

	mprintf(("Physical Vulkan devices:\n"));
	std::for_each(values.cbegin(), values.cend(), printPhysicalDevice);

	// Remove devices that do not have the features we need
	values.erase(std::remove_if(values.begin(),
					 values.end(),
					 [this](PhysicalDeviceValues& value) { return isDeviceUnsuitable(value, m_mainTarget->surface.get()); }),
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
		// Required by the Vulkan Portability subset whenever the physical device
		// advertises it (MoltenVK / macOS). Creating a device without it is
		// undefined behavior and trips VUID-VkDeviceCreateInfo-pProperties-04451.
		// The extension-name macro lives in vulkan_beta.h (VK_ENABLE_BETA_EXTENSIONS);
		// our headers do not enable that, so use the literal string instead.
		if (strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0) {
			enabledExtensions.push_back("VK_KHR_portability_subset");
			mprintf(("Vulkan: Enabling VK_KHR_portability_subset\n"));
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
	// Enable ONLY the core device features the renderer actually uses, each gated
	// on the device's support bit (deviceValues.features holds the queried support;
	// an unsupported feature stays false and is therefore never requested). Enabling
	// every supported feature would needlessly pull in ones this renderer never uses
	// -- notably robustBufferAccess (per-access bounds-checking cost), plus
	// wideLines, imageCubeArray, geometryShader, etc.
	//
	// Usage has been audited against the pipeline/sampler creation paths and the
	// compiled shaders. Because this only ever narrows the enabled feature set,
	// the failure mode of an audit gap is a used feature not being enabled; the
	// reliable exit criterion is a -gr_sync_validation soak across the test matrix
	// (deferred, MSAA, HDR, RT shadows, wireframe, RTT/env-map, decals) reporting
	// zero "feature not enabled" messages -- static analysis can't prove
	// completeness for shader-driven features.
	vk::PhysicalDeviceFeatures2 deviceFeatures2;
	const auto& supportedFeatures = deviceValues.features;
	auto& enabledFeatures = deviceFeatures2.features;
	enabledFeatures.samplerAnisotropy    = supportedFeatures.samplerAnisotropy;    // anisotropic texture filtering (VulkanTextureManager samplers)
	enabledFeatures.independentBlend     = supportedFeatures.independentBlend;     // per-attachment blend states (deferred decal G-buffer write masks)
	enabledFeatures.fillModeNonSolid     = supportedFeatures.fillModeNonSolid;     // wireframe (GR_FILL_MODE_WIRE -> vk::PolygonMode::eLine)
	enabledFeatures.shaderClipDistance   = supportedFeatures.shaderClipDistance;   // gl_ClipDistance[] in main / shadow-map / default-material vertex shaders
	enabledFeatures.textureCompressionBC = supportedFeatures.textureCompressionBC; // BC/DXT texture formats (optional; usage gated by isTextureCompressionBCSupported)
	enabledFeatures.depthClamp           = supportedFeatures.depthClamp;           // clamp depth in the shadow pass (optional; when absent, CAPABILITY_SHADOWS reports false and shadows are disabled)

	vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures;
	accelStructFeatures.accelerationStructure = VK_TRUE;

	vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures;
	rayQueryFeatures.rayQuery = VK_TRUE;

	vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures;
	bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

	// VK_KHR_synchronization2 is required (RequiredDeviceExtensions,
	// isDeviceUnsuitable() already rejected devices without the feature bit),
	// so this is always linked -- unlike the RT trio below, it's never unlinked.
	vk::PhysicalDeviceSynchronization2Features sync2Features;
	sync2Features.synchronization2 = VK_TRUE;

	vk::StructureChain<vk::DeviceCreateInfo,
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
		vk::PhysicalDeviceRayQueryFeaturesKHR,
		vk::PhysicalDeviceBufferDeviceAddressFeatures,
		vk::PhysicalDeviceSynchronization2Features>
		deviceCreateChain(deviceCreate, deviceFeatures2, accelStructFeatures, rayQueryFeatures,
			bufferDeviceAddressFeatures, sync2Features);

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
	m_presentQueue = m_device->getQueue(deviceValues.presentQueueIndex.index, 0);

	// Store physical device and queue family indices for later use
	m_physicalDevice = deviceValues.device;
	// Cache properties + features once (the limit/feature getters read these).
	m_deviceProperties = m_physicalDevice.getProperties();
	m_deviceFeatures = m_physicalDevice.getFeatures();
	m_graphicsQueueFamilyIndex = deviceValues.graphicsQueueIndex.index;
	m_presentQueueFamilyIndex = deviceValues.presentQueueIndex.index;

	// Initialize memory manager
	m_memoryManager = std::make_unique<VulkanMemoryManager>();
	if (!m_memoryManager->init(m_vkInstance.get(), m_physicalDevice, m_device.get(), m_supportsRaytracedShadows)) {
		mprintf(("Failed to initialize Vulkan memory manager!\n"));
		return false;
	}
	setMemoryManager(m_memoryManager.get());

	// Initialize deletion queue for deferred resource destruction
	m_deletionQueue = std::make_unique<VulkanDeletionQueue>();
	m_deletionQueue->init(m_device.get(), m_memoryManager.get());
	setDeletionQueue(m_deletionQueue.get());

	// Initialize buffer manager
	m_bufferManager = std::make_unique<VulkanBufferManager>();
	if (!m_bufferManager->init(m_device.get(), m_memoryManager.get(),
	                           m_graphicsQueueFamilyIndex,
	                           getMinUniformBufferOffsetAlignment())) {
		nprintf(("vulkan", "Failed to initialize Vulkan buffer manager!\n"));
		return false;
	}
	setBufferManager(m_bufferManager.get());
	// Set initial frame index for buffer manager
	m_bufferManager->setCurrentFrame(m_currentFrame, m_frameNumber);

	return true;
}
} // namespace graphics::vulkan
