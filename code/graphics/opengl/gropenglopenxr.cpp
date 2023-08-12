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
#include "graphics/opengl/gropenglstate.h"
#include "osapi/osapi.h"

#include <SDL.h>
#include <SDL_syswm.h>

//SETUP FUNCTIONS OGL
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

	if (requirements.minApiVersionSupported > XR_MAKE_VERSION(GLVersion.major, GLVersion.minor, 0)) {
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

int64_t gr_opengl_openxr_get_swapchain_format(const SCP_vector<int64_t>& allowed) {
	//We prefer whatever our backbuffer is
	if (std::find(allowed.cbegin(), allowed.cend(), GL_RGBA16F) != allowed.cend())
		return GL_RGBA16F;

	//Otherwise, Fallback
	return allowed.front();
}

//PIPELINE FUNCTIONS OGL
std::array<SCP_vector<XrSwapchainImageOpenGLKHR>, 2> swapchainImages;

GLuint xr_fbo;

bool gr_opengl_openxr_acquire_swapchain_buffers() {
	// create framebuffer
	glGenFramebuffers(1, &xr_fbo);
	GL_state.BindFrameBuffer(xr_fbo);
	opengl_set_object_label(GL_FRAMEBUFFER, xr_fbo, "XR Swapchain");

	for (uint32_t i = 0; i < 2; i++) {
		uint32_t imageCount = 0;

		if (xrEnumerateSwapchainImages(xr_swapchains[i]->swapchain, 0, &imageCount, nullptr) != XR_SUCCESS) {
			return false;
		}

		swapchainImages[i] = SCP_vector<XrSwapchainImageOpenGLKHR>(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR, nullptr, 0 });

		if (xrEnumerateSwapchainImages(xr_swapchains[i]->swapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)swapchainImages[i].data()) != XR_SUCCESS) {
			return false;
		}
	}

	return true;
}

void gr_opengl_openxr_flip() {
	if (!openxr_initialized)
		return;

	//If a stereoscopic frame has started, we're good. Otherwise, this is a normal frame that still needs to be OpenXR-Initialized
	if (xr_stage == OpenXRFBStage::NONE)
		openxr_update_view();

	XrFrameBeginInfo beginFrameInfo { XR_TYPE_FRAME_BEGIN_INFO };
	XrResult result = xrBeginFrame(xr_session, &beginFrameInfo);

	for (uint32_t i = 0; i < 2; i++) {
		XrSwapchain swapchain = xr_swapchains[i]->swapchain;

		XrSwapchainImageAcquireInfo acquireImageInfo {
			XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO,
			nullptr
		};
		uint32_t activeIndex;
		xrAcquireSwapchainImage(swapchain, &acquireImageInfo, &activeIndex);

		XrSwapchainImageWaitInfo waitImageInfo{
			waitImageInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
			nullptr,
			waitImageInfo.timeout = std::numeric_limits<int64_t>::max()
		};
		xrWaitSwapchainImage(swapchain, &waitImageInfo);

		const XrSwapchainImageOpenGLKHR& image = swapchainImages[i][activeIndex];

		//Blit
		GR_DEBUG_SCOPE("VR Blit");
		GL_state.BindFrameBuffer(0, GL_READ_FRAMEBUFFER);
		GL_state.BindFrameBuffer(xr_fbo, GL_DRAW_FRAMEBUFFER);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, image.image, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glReadBuffer(GL_BACK);
		glBlitFramebuffer(0, 0, gr_screen.max_w, gr_screen.max_h, 0, 0, xr_swapchains[i]->width, xr_swapchains[i]->height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);

		glDrawBuffer(GL_BACK);
		GL_state.BindFrameBuffer(0);

		XrSwapchainImageReleaseInfo releaseImageInfo {
			XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO,
			nullptr
		};

		xrReleaseSwapchainImage(swapchain, &releaseImageInfo);
	}

	XrCompositionLayerProjectionView projectedViews[2];

	for (uint32_t i = 0; i < 2; i++)
	{
		projectedViews[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
		projectedViews[i].next = nullptr;
		projectedViews[i].pose = xr_views[i].pose;
		projectedViews[i].fov = xr_views[i].fov;
		projectedViews[i].subImage = {
			xr_swapchains[i]->swapchain,
			{
				{ 0, 0 },
				{ (int32_t)xr_swapchains[i]->width, (int32_t)xr_swapchains[i]->height }
			},
			0
		};
	}

	XrCompositionLayerProjection layer {
		XR_TYPE_COMPOSITION_LAYER_PROJECTION,
		nullptr,
		0,
		xr_space,
		2,
		projectedViews
	};
	auto pLayer = (const XrCompositionLayerBaseHeader*)&layer;

	XrFrameEndInfo frameEndInfo {
		XR_TYPE_FRAME_END_INFO,
		nullptr,
		xr_state.predictedDisplayTime,
		XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
	};

	if (xr_state.shouldRender) {
		frameEndInfo.layerCount = 1;
		frameEndInfo.layers = &pLayer;
	}

	xrEndFrame(xr_session, &frameEndInfo);
}