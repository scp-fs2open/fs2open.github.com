#include "openxr_internal.h"

#include "camera/camera.h"
#include "graphics/2d.h"
#include "globalincs/version.h"
#include "mod_table/mod_table.h"
#include "render/3d.h"
#include "starfield/starfield.h"

std::unique_ptr<star[]> Stars_XRBuffer;

#ifdef FS_OPENXR

#define XR_MAKE_VERSION_SHORT(major, minor, patch) \
    ((((major) & 0x3ffU) << 20) | (((minor) & 0x3ffU) << 10) | ((patch) & 0x3ffU))

bool openxr_req = false;
bool openxr_initialized = false;
bool openxr_recieve = false;

XrInstance xr_instance;
XrSystemId xr_system;
XrSession xr_session;
XrSpace xr_space;
XrTime xr_time;
XrDebugUtilsMessengerEXT xr_debugMessenger;
std::array<std::unique_ptr<XrSwapchainHandler>, 2> xr_swapchains;
std::array<XrView, 2> xr_views;
std::array<XrViewConfigurationView, 2> xr_configurationviews;
vec3d xr_offset = ZERO_VECTOR;
XrFrameState xr_state;
OpenXRFBStage xr_stage = OpenXRFBStage::NONE;
float xr_scale = 1.0f;

#if !defined(NDEBUG) && defined(FS_OPENXR_DEBUG)
static XrBool32 handleXRError(XrDebugUtilsMessageSeverityFlagsEXT severity, XrDebugUtilsMessageTypeFlagsEXT type, const XrDebugUtilsMessengerCallbackDataEXT* callbackData, void* /*userData*/) {
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
#endif

void openxr_prepare(float hudscale) {
	Hud_global_scale = hudscale;
	openxr_req = true;
}

static bool openxr_init_instance() {
	auto extensions = gr_openxr_get_extensions();

	const gameversion::version& fso_version = gameversion::get_executable_version();
	const gameversion::version& mod_version(Mod_version);

#if !defined(NDEBUG) && defined(FS_OPENXR_DEBUG)
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

	XrResult create = xrCreateInstance(&instanceCreateInfo, &xr_instance);
	if (create != XR_SUCCESS) {
		if (create == XR_ERROR_EXTENSION_NOT_PRESENT) {
			//This is to be expected to be one of THE common issue cases, since not all OpenXR runtimes, notably WMR, don't supply the OpenGL extensions
			ReleaseWarning(LOCATION, "The OpenXR runtime does not support the required backend! Try to use a different OpenXR runtime.");
		}

		mprintf(("Failed to create OpenXR instance with code %d\n", static_cast<int>(create)));

		return false;
	}

#if !defined(NDEBUG) && defined(FS_OPENXR_DEBUG)
	XrDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo {
		XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		nullptr,
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT,
		(PFN_xrDebugUtilsMessengerCallbackEXT)(&handleXRError),
		nullptr
	};

	openxr_callExtensionFunction<PFN_xrCreateDebugUtilsMessengerEXT>("xrCreateDebugUtilsMessengerEXT", xr_instance, &debugMessengerCreateInfo, &xr_debugMessenger);
#endif

	return true;
}

static bool openxr_init_system() {
	XrSystemGetInfo systemGetInfo {
	XR_TYPE_SYSTEM_GET_INFO,
	nullptr,
	XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY
	};

	XrResult systemInit = xrGetSystem(xr_instance, &systemGetInfo, &xr_system);
	if (systemInit != XR_SUCCESS) {
		mprintf(("Failed to create OpenXR system with code %d\n", static_cast<int>(systemInit)));
		xr_system = XR_NULL_SYSTEM_ID;
		return false;
	}

	return true;
}

static bool openxr_init_configuration_views() {
	uint32_t configurationViewsCount = static_cast<uint32_t>(xr_configurationviews.size());
	for (auto& configView : xr_configurationviews)
		configView = { XR_TYPE_VIEW_CONFIGURATION_VIEW, nullptr, 0, 0, 0, 0, 0, 0 };

	if (xrEnumerateViewConfigurationViews(xr_instance, xr_system, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, configurationViewsCount, &configurationViewsCount, xr_configurationviews.data()) != XR_SUCCESS) {
		mprintf(("Failed to find OpenXR configuration views for stereo displays!\n"));
		return false;
	}

	return true;
}

static bool openxr_init_swapchains() {
	uint32_t formatCount = 0;
	if (xrEnumerateSwapchainFormats(xr_session, 0, &formatCount, nullptr) != XR_SUCCESS) {
		return false;
	}

	SCP_vector<int64_t> formats(formatCount);
	if (xrEnumerateSwapchainFormats(xr_session, formatCount, &formatCount, formats.data()) != XR_SUCCESS) {
		return false;
	}

	int64_t chosenFormat = gr_openxr_get_swapchain_format(formats);

	std::array<XrSwapchain, 2> swapchains;
	for (uint32_t i = 0; i < 2; i++)
	{
		XrSwapchainCreateInfo swapchainCreateInfo {
			XR_TYPE_SWAPCHAIN_CREATE_INFO,
			nullptr,
			0,
			XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT,
			chosenFormat,
			1,
			xr_configurationviews[i].recommendedImageRectWidth,
			xr_configurationviews[i].recommendedImageRectHeight,
			1,
			1,
			1
		};

		XrResult swapchainInit = xrCreateSwapchain(xr_session, &swapchainCreateInfo, &swapchains[i]);
		if (swapchainInit != XR_SUCCESS) {
			mprintf(("Failed to create OpenXR swapchain %d with code %d\n", i, static_cast<int>(swapchainInit)));
			return false;
		}

		xr_swapchains[i].reset(new XrSwapchainHandler {
			swapchains[i],
			chosenFormat,
			xr_configurationviews[i].recommendedImageRectWidth,
			xr_configurationviews[i].recommendedImageRectHeight
		});
	}

	if (!gr_openxr_acquire_swapchain_buffers()) {
		return false;
	}

	return true;
}

static bool openxr_init_space() {
	XrReferenceSpaceCreateInfo spaceCreateInfo {
		XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
		nullptr,
		XR_REFERENCE_SPACE_TYPE_LOCAL,
		XrPosef{ XrQuaternionf{ 0, 0, 0, 1 }, XrVector3f{ 0, 0, 0 } },
	};

	if (xrCreateReferenceSpace(xr_session, &spaceCreateInfo, &xr_space) != XR_SUCCESS) {
		return false;
	}

	return true;
}

static void openxr_init_post() {
	XrViewLocateInfo viewLocateInfo {
		XR_TYPE_VIEW_LOCATE_INFO,
		nullptr,
		XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
		xr_time,
		xr_space
	};

	XrViewState viewState {
		XR_TYPE_VIEW_STATE,
		nullptr,
		0
	};

	xr_views.fill({ XR_TYPE_VIEW, nullptr, XrPosef{}, XrFovf{} });
	uint32_t views;
	xrLocateViews(
		xr_session,
		&viewLocateInfo,
		&viewState,
		2,
		&views,
		xr_views.data()
	);

	openxr_reset_offset();

	vec3d xr_offset_debug {{ {
		(xr_views[0].pose.position.x - xr_views[1].pose.position.x) * xr_scale,
		(xr_views[0].pose.position.y - xr_views[1].pose.position.y) * xr_scale,
		(xr_views[0].pose.position.z - xr_views[1].pose.position.z) * xr_scale} }};

	mprintf(("OpenXR HMD data:\n\tLeft Res: %dx%d\n\tLeft FoV (l|r|t|b): %f, %f, %f, %f\n\tRight Res: %dx%d\n\tRight FoV (l|r|t|b): %f, %f, %f, %f\n\tIPD: %f\n",
		xr_swapchains[0]->width, xr_swapchains[0]->height,
		xr_views[0].fov.angleLeft, xr_views[0].fov.angleRight, xr_views[0].fov.angleUp, xr_views[0].fov.angleDown,
		xr_swapchains[1]->width, xr_swapchains[1]->height,
		xr_views[1].fov.angleLeft, xr_views[1].fov.angleRight, xr_views[1].fov.angleUp, xr_views[1].fov.angleDown,
		vm_vec_mag(&xr_offset_debug)));

	VIEWER_ZOOM_DEFAULT = COCKPIT_ZOOM_DEFAULT = asymmetric_fov { 
		xr_views[0].fov.angleLeft + xr_views[1].fov.angleLeft,
		xr_views[0].fov.angleRight + xr_views[1].fov.angleRight,
		xr_views[0].fov.angleUp + xr_views[1].fov.angleUp,
		xr_views[0].fov.angleDown + xr_views[1].fov.angleDown
	} * (1.0f / (2.0f * PROJ_FOV_FACTOR));
}

float openxr_preinit(float req_ar, float scale) {
	xr_scale = scale;

	mprintf(("Attempting to pre-initialize OpenXR...\n"));

	if (!openxr_init_instance()) {
		openxr_req = false;
		return req_ar;
	}

	if (!openxr_init_system()) {
		openxr_req = false;
		return req_ar;
	}

	if (!openxr_init_configuration_views()) {
		openxr_req = false;
		return req_ar;
	}

	return  i2fl(xr_configurationviews[0].recommendedImageRectWidth) / i2fl(xr_configurationviews[0].recommendedImageRectHeight);
}

void openxr_init() {
	if (!openxr_req)
		return;

	mprintf(("Attempting to initialize OpenXR and get a session...\n"));

	if (!gr_openxr_test_capabilities())
		return;

	//Non OpenGL backends, like vulkan, may need to query certain extensions to be enabled here as well

	if (!gr_openxr_create_session())
		return;
	
	if (!openxr_init_swapchains())
		return;

	if (!openxr_init_space())
		return;

	openxr_initialized = true;

	mprintf(("OpenXR initialized!\n"));

	while (openxr_initialized && !openxr_recieve) {
		os_poll();
	}

	mprintf(("OpenXR recieving!\n"));

	openxr_init_post();
}

void openxr_close() {
	xrDestroySpace(xr_space);
	for (auto& sc : xr_swapchains)
		sc.release();
	xrDestroySession(xr_session);

#if !defined(NDEBUG) && defined(FS_OPENXR_DEBUG)
	openxr_callExtensionFunction<PFN_xrDestroyDebugUtilsMessengerEXT>("xrDestroyDebugUtilsMessengerEXT", xr_debugMessenger);
#endif

	xrDestroyInstance(xr_instance);
}

bool openxr_enabled() {
	return openxr_initialized && openxr_recieve;
}

bool openxr_requested() {
	return openxr_req;
}

void openxr_reset_offset() {
	xr_offset = ZERO_VECTOR;
	for (uint32_t i = 0; i < 2; i++) {
		const auto& pos = xr_views[i].pose.position;
		xr_offset += vec3d{ { {pos.x * xr_scale, pos.y * xr_scale, pos.z * -xr_scale} } };
	}
	xr_offset /= 2;
}

void openxr_start_mission() {
	if (!openxr_initialized)
		return;

	openxr_reset_offset();

	if (!static_cast<bool>(Stars_XRBuffer))
		Stars_XRBuffer = make_unique<star[]>(MAX_STARS);

	extern std::unique_ptr<star[]> Stars;
	std::copy(Stars.get(), Stars.get() + MAX_STARS, Stars_XRBuffer.get());
}

void openxr_poll() {
	if (!openxr_initialized)
		return;

	XrEventDataBuffer eventData{
		XR_TYPE_EVENT_DATA_BUFFER,
		nullptr,
		{0}
	};

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

			xr_time = event->time;

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
			default:
				break;
			}
			break;
		}
		default:
			break;
		}
	}
}

OpenXRTrackingInfo openxr_start_stereo_frame() {
	if (!openxr_initialized)
		return OpenXRTrackingInfo{};

	xr_stage = OpenXRFBStage::FIRST;

	openxr_start_frame();

	OpenXRTrackingInfo info{};

	for (uint32_t i = 0; i < 2; i++) {
		const auto& pos = xr_views[i].pose.position;
		const auto& ori = xr_views[i].pose.orientation;
		info.eyes[i].offset = vec3d{{ {pos.x * xr_scale, pos.y * xr_scale, pos.z * -xr_scale} }} - xr_offset;
		vm_quaternion_to_matrix(&info.eyes[i].orientation, ori.x, ori.y, -ori.z, -ori.w);
		info.eyes[i].zoom = asymmetric_fov{ xr_views[i].fov.angleLeft, xr_views[i].fov.angleRight, xr_views[i].fov.angleUp, xr_views[i].fov.angleDown } * (1.0f / PROJ_FOV_FACTOR);
	}

	return info;
}

//Internal Helper functions

void openxr_start_frame() {
	XrFrameWaitInfo frameWaitInfo {
		XR_TYPE_FRAME_WAIT_INFO,
		nullptr
	};
	xr_state = XrFrameState {
		XR_TYPE_FRAME_STATE,
		nullptr,
		0,
		0,
		0
	};

	xrWaitFrame(xr_session, &frameWaitInfo, &xr_state);

	XrViewLocateInfo viewLocateInfo {
		XR_TYPE_VIEW_LOCATE_INFO,
		nullptr,
		XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
		xr_state.predictedDisplayTime,
		xr_space
	};

	xr_time = xr_state.predictedDisplayTime;

	XrViewState viewState {
		XR_TYPE_VIEW_STATE,
		nullptr,
		0
	};

	xr_views.fill({ XR_TYPE_VIEW, nullptr, XrPosef{}, XrFovf{} });

	uint32_t views;
	xrLocateViews(
		xr_session,
		&viewLocateInfo,
		&viewState,
		2,
		&views,
		xr_views.data()
	);

	XrFrameBeginInfo beginFrameInfo{ XR_TYPE_FRAME_BEGIN_INFO, nullptr };
	xrBeginFrame(xr_session, &beginFrameInfo);
}

#else
// Stubs for when building without OpenXR support.
// NOTE: macOS has issues linking with OpenXR.

void openxr_prepare(float hudscale) {}

float openxr_preinit(float req_ar, float scale) {
	mprintf(("Cannot create OpenXR session. Not built with OpenXR support.\n"));
	return 0.0f;
}

void openxr_init() {}

void openxr_close() {}

void openxr_poll() {}

void openxr_start_mission() {}

bool openxr_enabled() { return false; }

bool openxr_requested() { return false; }

OpenXRTrackingInfo openxr_start_stereo_frame() { return OpenXRTrackingInfo{}; }

#endif
