#include "gropenglopenxr.h"

#define XR_USE_GRAPHICS_API_OPENGL

#include "graphics/openxr_internal.h"

SCP_vector<const char*> gr_opengl_openxr_get_extensions() {
	return { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
}

bool gr_opengl_openxr_test_capabilities() {
	auto xrGetOpenGLGraphicsRequirementsKHR = (PFN_xrGetOpenGLGraphicsRequirementsKHR)openxr_getExtensionFunction("xrGetOpenGLGraphicsRequirementsKHR");

	XrGraphicsRequirementsOpenGLKHR requirements;
	requirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;

	if (xrGetOpenGLGraphicsRequirementsKHR(xr_instance, xr_system, &requirements) != XR_SUCCESS) {
		return false;
	}

	return true;
}