#include "openxr_internal.h"

#include "graphics/2d.h"
#include "globalincs/version.h"
#include "mod_table/mod_table.h"

#define XR_MAKE_VERSION_SHORT(major, minor, patch) \
    ((((major) & 0x3ffU) << 20) | (((minor) & 0x3ffU) << 10) | ((patch) & 0x3ffU))

bool openxr_initialized = false;
bool openxr_recieve = false;

XrInstance xr_instance;
XrSystemId xr_system;
XrSession xr_session;
XrSpace xr_space;
XrDebugUtilsMessengerEXT xr_debugMessenger;
std::array<std::unique_ptr<XrSwapchainHandler>, 2> xr_swapchains;
std::array<XrView, 2> xr_views;
XrFrameState xr_state;
OpenXRFBStage xr_stage = OpenXRFBStage::NONE;

static XrBool32 handleXRError(XrDebugUtilsMessageSeverityFlagsEXT severity, XrDebugUtilsMessageTypeFlagsEXT type, const XrDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) {
	SCP_string message;
	switch (type)
	{
	case XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		message = "OpenXR: general\n%s";
		break;
	case XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		message = "OpenXR: validation\n%s";
		break;
	case XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		message = "OpenXR: performance\n%s";
		break;
	case XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT:
		message = "OpenXR: conformance\n%s";
		break;
	}

	switch (severity)
	{
	case XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		Information(LOCATION, message.c_str(), callbackData->message);
		break;
	case XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		Warning(LOCATION, message.c_str(), callbackData->message);
		break;
	case XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		Error(LOCATION, message.c_str(), callbackData->message);
		break;
	}

	return XR_FALSE;
}

void openxr_init() {
	auto extensions = gr_openxr_get_extensions();

	const gameversion::version& fso_version = gameversion::get_executable_version();
	const gameversion::version& mod_version(Mod_version);

#if !defined(NDEBUG)
	extensions.emplace_back("XR_EXT_debug_utils");
#endif

	XrInstanceCreateInfo instanceCreateInfo {
		XR_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		XrApplicationInfo {
			"", //Modname
			//Mod version. Should not contain negative numbers or only 0's.
			XR_MAKE_VERSION_SHORT(mod_version.major < 1 ? 1U : static_cast<uint32_t>(mod_version.major),
				mod_version.minor < 0 ? 0U : static_cast<uint32_t>(mod_version.minor),
				mod_version.build < 0 ? 0U : static_cast<uint32_t>(mod_version.build)),
			"FreeSpace Open",
			XR_MAKE_VERSION_SHORT(static_cast<uint32_t>(fso_version.major), static_cast<uint32_t>(fso_version.minor), static_cast<uint32_t>(fso_version.build)),
			XR_CURRENT_API_VERSION
		},
		0,
		nullptr,
		static_cast<uint32_t>(extensions.size()),
		extensions.data()
	};

	strncpy_s(instanceCreateInfo.applicationInfo.applicationName, Mod_title.empty() ? "FreeSpace Open" : Mod_title.c_str(), XR_MAX_APPLICATION_NAME_SIZE);

	if (xrCreateInstance(&instanceCreateInfo, &xr_instance) != XR_SUCCESS) {
		return;
	}

#if !defined(NDEBUG)
	XrDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo {
		XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		nullptr,
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT,
		handleXRError,
		nullptr
	};

	auto xrCreateDebugUtilsMessengerEXT = (PFN_xrCreateDebugUtilsMessengerEXT)openxr_getExtensionFunction("xrCreateDebugUtilsMessengerEXT");
	xrCreateDebugUtilsMessengerEXT(xr_instance, &debugMessengerCreateInfo, &xr_debugMessenger);
#endif

	XrSystemGetInfo systemGetInfo {
		XR_TYPE_SYSTEM_GET_INFO,
		nullptr,
		XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY
	};

	if (xrGetSystem(xr_instance, &systemGetInfo, &xr_system) != XR_SUCCESS) {
		xr_system = XR_NULL_SYSTEM_ID;
		return;
	}

	if (!gr_openxr_test_capabilities()) {
		return;
	}

	//Non OpenGL backends, like vulkan, may need to query certain extensions to be enabled here as well

	if (!gr_openxr_create_session()) {
		return;
	}

	uint32_t configurationViewsCount = 2;
	SCP_vector<XrViewConfigurationView> configurationViews(configurationViewsCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });

	if (xrEnumerateViewConfigurationViews(xr_instance, xr_system, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, configurationViewsCount, &configurationViewsCount, configurationViews.data()) != XR_SUCCESS) {
		return;
	}

	uint32_t formatCount = 0;
	if (xrEnumerateSwapchainFormats(xr_session, 0, &formatCount, nullptr) != XR_SUCCESS) {
		return;
	}

	SCP_vector<int64_t> formats(formatCount);
	if (xrEnumerateSwapchainFormats(xr_session, formatCount, &formatCount, formats.data()) != XR_SUCCESS) {
		return;
	}

	int64_t chosenFormat = gr_openxr_get_swapchain_format(formats);

	XrSwapchain swapchains[2];
	for (uint32_t i = 0; i < 2; i++)
	{
		XrSwapchainCreateInfo swapchainCreateInfo {
			XR_TYPE_SWAPCHAIN_CREATE_INFO,
			nullptr,
			0,
			XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT,
			chosenFormat,
			1,
			configurationViews[i].recommendedImageRectWidth,
			configurationViews[i].recommendedImageRectHeight,
			1,
			1,
			1
		};

		if(xrCreateSwapchain(xr_session, &swapchainCreateInfo, &swapchains[i]) != XR_SUCCESS) {
			return;
		}

		xr_swapchains[i].reset(new XrSwapchainHandler{
			swapchains[i],
			chosenFormat,
			configurationViews[i].recommendedImageRectWidth,
			configurationViews[i].recommendedImageRectHeight 
		});
	}

	if (!gr_openxr_acquire_swapchain_buffers()) {
		return;
	}

	XrReferenceSpaceCreateInfo spaceCreateInfo {
		XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
		nullptr,
		XR_REFERENCE_SPACE_TYPE_STAGE,
		XrPosef{ XrQuaternionf{ 0, 0, 0, 1 }, XrVector3f{ 0, 0, 0 } },
	};

	if (xrCreateReferenceSpace(xr_session, &spaceCreateInfo, &xr_space) != XR_SUCCESS) {
		return;
	}

	openxr_initialized = true;

	while (openxr_initialized && !openxr_recieve) {
		os_poll();
	}
}

void openxr_close() {
	xrDestroySpace(xr_space);
	for (auto& sc : xr_swapchains)
		sc.release();
	xrDestroySession(xr_session);

#if !defined(NDEBUG)
	((PFN_xrDestroyDebugUtilsMessengerEXT)openxr_getExtensionFunction("xrDestroyDebugUtilsMessengerEXT"))(xr_debugMessenger);
#endif

	xrDestroyInstance(xr_instance);
}

void openxr_poll() {
	if (!openxr_initialized)
		return;

	XrEventDataBuffer eventData;
	eventData.type = XR_TYPE_EVENT_DATA_BUFFER;

	XrResult result = xrPollEvent(xr_instance, &eventData);
	if (result == XR_EVENT_UNAVAILABLE) {
		//No event
	}
	else if (result != XR_SUCCESS) {
		openxr_initialized = false;
		openxr_close();
	}
	else {
		switch (eventData.type)
		{
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
			openxr_initialized = false;
			openxr_close();
			break;
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
		{
			auto event = (XrEventDataSessionStateChanged*)&eventData;

			switch (event->state)
			{
			case XR_SESSION_STATE_IDLE:
				openxr_recieve = false;
				break;
			case XR_SESSION_STATE_READY:
			{
				XrSessionBeginInfo sessionBeginInfo{
					XR_TYPE_SESSION_BEGIN_INFO,
					nullptr,
					XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO
				};

				if (xrBeginSession(xr_session, &sessionBeginInfo) != XR_SUCCESS) {
					openxr_initialized = false;
					openxr_close();
				}
			}
			/*fallthrough*/
			case XR_SESSION_STATE_SYNCHRONIZED:
			case XR_SESSION_STATE_VISIBLE:
			case XR_SESSION_STATE_FOCUSED:
				openxr_recieve = true;
				break;
			case XR_SESSION_STATE_STOPPING:
				xrEndSession(xr_session);
			/*fallthrough*/
			case XR_SESSION_STATE_LOSS_PENDING:
			case XR_SESSION_STATE_EXITING:
				openxr_initialized = false;
				openxr_close();
				break;
			}
			break;
		}
		default:
			break;
		}
	}
}

PFN_xrVoidFunction openxr_getExtensionFunction(const char* const name) {
	PFN_xrVoidFunction func;

	if (xrGetInstanceProcAddr(xr_instance, name, &func) != XR_SUCCESS) {
		return nullptr;
	}

	return func;
}

void openxr_update_view() {
	XrFrameWaitInfo frameWaitInfo {
		XR_TYPE_FRAME_WAIT_INFO
	};
	xr_state = XrFrameState {
		XR_TYPE_FRAME_STATE
	};

	xrWaitFrame(xr_session, &frameWaitInfo, &xr_state);

	XrViewLocateInfo viewLocateInfo {
		XR_TYPE_VIEW_LOCATE_INFO,
		nullptr,
		XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
		xr_state.predictedDisplayTime,
		xr_space
	};

	XrViewState viewState {
		XR_TYPE_VIEW_STATE
	};

	xr_views.fill({ XR_TYPE_VIEW });

	uint32_t views;
	xrLocateViews(
		xr_session,
		&viewLocateInfo,
		&viewState,
		2,
		&views,
		xr_views.data()
	);
}

void openxr_start_stereo_frame() {
	xr_stage = OpenXRFBStage::LEFT;

	openxr_update_view();
}
