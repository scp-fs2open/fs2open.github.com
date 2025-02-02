#pragma once

//Don't include this file unless you know what you are doing and need to _directly_ interface with OpenXR

#include "openxr.h"

#include <type_traits>
#include <optional>

#ifdef FS_OPENXR

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

struct XrSwapchainHandler {
	XrSwapchain swapchain;
	int64_t format;
	uint32_t width;
	uint32_t height;

	~XrSwapchainHandler() {
		xrDestroySwapchain(swapchain);
	}
};

extern bool openxr_initialized; //is true if initialization was successfull. Does NOT imply that we can render right now!
extern XrInstance xr_instance;
extern XrSystemId xr_system;
extern XrSession xr_session;
extern XrSpace xr_space;
extern XrTime xr_time; //The last known time something happened. May be the last state change or the (predicted) time of the last frame
extern std::array<std::unique_ptr<XrSwapchainHandler>, 2> xr_swapchains;
extern std::array<XrView, 2> xr_views;
extern XrFrameState xr_state;
extern vec3d xr_offset;

enum class OpenXRFBStage { NONE, FIRST, SECOND };
extern OpenXRFBStage xr_stage; //State machine tracker for rendering. Needed since OpenXR needs to be able to tell whether this is a stereoscopic frame or not whenever the code flips

void openxr_start_frame();
void openxr_reset_offset();

template<typename openxr_fnc, typename... arg_t>
std::optional<std::invoke_result_t<openxr_fnc,arg_t...>> openxr_callExtensionFunction(const char* const name, arg_t&&... args) {
	openxr_fnc func;

	if (xrGetInstanceProcAddr(xr_instance, name, (PFN_xrVoidFunction*)&func) != XR_SUCCESS) {
		return std::nullopt;
	}

	return func(std::forward<arg_t>(args)...);
}

#endif
