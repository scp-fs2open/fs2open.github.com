#include "openxr.h"

#include "globalincs/version.h"
#include "mod_table/mod_table.h"

#define XR_USE_GRAPHICS_API_OPENGL

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#define XR_MAKE_VERSION_SHORT(major, minor, patch) \
    ((((major) & 0x3ffU) << 20) | (((minor) & 0x3ffU) << 10) | ((patch) & 0x3ffU))

bool openxr_initialized = false;

XrInstance xr_instance;
XrSystemId xr_system;

void openxr_init() {
	if (gr_screen.mode != GR_OPENGL) {
		//Bail if we don't have OpenGL, Vulkan support is not yet implemented.
		return;
	}

	char* extensions[] = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
	uint32_t extension_count = sizeof(extensions) / sizeof(extensions[0]);

	const gameversion::version& fso_version = gameversion::get_executable_version();
	const gameversion::version& mod_version(Mod_version);

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
		extension_count,
		(const char* const*)extensions
	};

	strncpy_s(instanceCreateInfo.applicationInfo.applicationName, Mod_title.c_str(), XR_MAX_APPLICATION_NAME_SIZE);

	if (xrCreateInstance(&instanceCreateInfo, &xr_instance) != XR_SUCCESS) {
		return;
	}

	XrSystemGetInfo systemGetInfo {
		XR_TYPE_SYSTEM_GET_INFO,
		nullptr,
		XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY
	};

	if (xrGetSystem(xr_instance, &systemGetInfo, &xr_system) != XR_SUCCESS) {
		xr_system = XR_NULL_SYSTEM_ID;
		return;
	}

	openxr_initialized = true;
}

void openxr_close() {
	xrDestroyInstance(xr_instance);
}