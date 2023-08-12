#pragma once

#include "openxr.h"

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

extern bool openxr_initialized;

extern XrInstance xr_instance;
extern XrSystemId xr_system;
extern XrSession xr_session;
extern XrSpace xr_space;
extern std::array<std::unique_ptr<XrSwapchainHandler>, 2> xr_swapchains;
extern std::array<XrView, 2> xr_views;
extern XrFrameState xr_state;

enum class OpenXRFBStage { NONE, LEFT, RIGHT };
extern OpenXRFBStage xr_stage;

PFN_xrVoidFunction openxr_getExtensionFunction(const char* const name);
void openxr_update_view();