#include "gropenglopenxr.h"

#ifdef SCP_UNIX

#define XR_USE_PLATFORM_XLIB

#elif defined WIN32

#define XR_USE_PLATFORM_WIN32
#include <unknwn.h>

#elif defined __APPLE_CC__

//Not supported

#endif

#define XR_USE_GRAPHICS_API_OPENGL
#include "graphics/openxr_internal.h"

#include "graphics/opengl/gropengl.h"
#include "osapi/osapi.h"

#include <SDL.h>
#include <SDL_syswm.h>

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

	if (requirements.minApiVersionSupported < XR_MAKE_VERSION(GLVersion.major, GLVersion.minor, 0)) {
		return false;
	}

	return true;
}

#ifdef SCP_UNIX
bool gr_opengl_openxr_create_session() {
	/*SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(os::getSDLMainWindow(), &wmInfo);

	XrGraphicsBindingOpenGLXlibKHR graphicsBinding{
		graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR,
		nullptr,
		graphicsBinding.hDC = wmInfo.info.x11.display,
		0,
		...,
		glXGetCurrentDrawable(),
		(GLXContext) SDL_GL_GetCurrentContext() //uuuuuugly, and not technically allowed by the standard, but this "opaque" SDL_GLContext type is just the GLXContext on X11
	};

	XrSessionCreateInfo sessionCreateInfo{
		sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO,
		sessionCreateInfo.next = &graphicsBinding,
		sessionCreateInfo.createFlags = 0,
		sessionCreateInfo.systemId = xr_system
	};

	if (xrCreateSession(xr_instance, &sessionCreateInfo, &xr_session) != XR_SUCCESS) {
		return false;
	}

	return true;*/

	return false;
}
#elif defined WIN32
bool gr_opengl_openxr_create_session() {
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(os::getSDLMainWindow(), &wmInfo);

	XrGraphicsBindingOpenGLWin32KHR graphicsBinding{
		XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR,
		nullptr,
		wmInfo.info.win.hdc,
		(HGLRC) SDL_GL_GetCurrentContext() //uuuuuugly, and not technically allowed by the standard, but this "opaque" SDL_GLContext type is just the hGLRC on Win32
	};
	
	XrSessionCreateInfo sessionCreateInfo{
		sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO,
		sessionCreateInfo.next = &graphicsBinding,
		sessionCreateInfo.createFlags = 0,
		sessionCreateInfo.systemId = xr_system
	};

	if (xrCreateSession(xr_instance, &sessionCreateInfo, &xr_session) != XR_SUCCESS) {
		return false;
	}

	return true;
}
#elif defined __APPLE_CC__
bool gr_opengl_openxr_create_session() {
	return false;
}
#endif

