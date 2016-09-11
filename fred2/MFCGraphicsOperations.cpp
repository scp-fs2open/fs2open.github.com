
#include "MFCGraphicsOperations.h"

MFCViewport::MFCViewport(HWND hwnd, HDC dc): _windowHandle(hwnd), _device_context(dc)
{
	Assertion(hwnd != nullptr, "Invalid window handle!");
	Assertion(dc != nullptr, "Invalid device context handle!");

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		Error(LOCATION, "Couldn't init SDL video: %s", SDL_GetError());
		return;
	}
}

MFCViewport::~MFCViewport()
{
	ReleaseDC(_windowHandle, _device_context);

	_windowHandle = nullptr;
	_device_context = nullptr;
}

SDL_Window* MFCViewport::toSDLWindow()
{
	return nullptr;
}

std::pair<uint32_t, uint32_t> MFCViewport::getSize()
{
	RECT size;
	if (!GetWindowRect(_windowHandle, &size))
	{
		return std::make_pair(0, 0);
	}

	return std::make_pair(size.right - size.left, size.bottom - size.top);
}

void MFCViewport::swapBuffers()
{
	SwapBuffers(_device_context);
}

void MFCViewport::setState(os::ViewportState state)
{
	// Not implemented
}

void MFCViewport::minimize()
{
	// Not implemented
}

void MFCViewport::restore()
{
	// Not implemented
}

HDC MFCViewport::getHDC()
{
	return _device_context;
}

void* MFCOpenGLContext::_oglDllHandle = nullptr;
size_t MFCOpenGLContext::_oglDllReferenceCount = 0;

MFCOpenGLContext::MFCOpenGLContext(HGLRC hglrc): _render_context(hglrc)
{
	if (_oglDllHandle == nullptr)
	{
		_oglDllHandle = SDL_LoadObject("OPENGL32.DLL");
	}
	++_oglDllReferenceCount;
}

MFCOpenGLContext::~MFCOpenGLContext()
{
	if (!wglDeleteContext(_render_context))
	{
		mprintf(("Failed to delete render context!"));
	}

	--_oglDllReferenceCount;
	if (_oglDllReferenceCount == 0)
	{
		SDL_UnloadObject(_oglDllHandle);
		_oglDllHandle = nullptr;
	}
}

void* MFCOpenGLContext::wglLoader(const char* name)
{
	auto wglAddr = reinterpret_cast<void*>(wglGetProcAddress(name));
	if (wglAddr == nullptr && _oglDllHandle != nullptr)
	{
		wglAddr = SDL_LoadFunction(_oglDllHandle, name);
	}

	return wglAddr;
}

os::OpenGLLoadProc MFCOpenGLContext::getLoaderFunction()
{
	return wglLoader;
}

void MFCOpenGLContext::setSwapInterval(int status)
{
	if (GLAD_WGL_EXT_swap_control)
	{
		wglSwapIntervalEXT(status);
	}
}

HGLRC MFCOpenGLContext::getHandle()
{
	return _render_context;
}

MFCGraphicsOperations::MFCGraphicsOperations(HWND hwnd): _windowHandle(hwnd)
{
}

MFCGraphicsOperations::~MFCGraphicsOperations()
{
}

std::unique_ptr<os::Viewport> MFCGraphicsOperations::createViewport(const os::ViewPortProperties& props)
{
	int PixelFormat;
	PIXELFORMATDESCRIPTOR pfd_test;
	PIXELFORMATDESCRIPTOR GL_pfd;

	mprintf(("  Initializing WGL...\n"));

	memset(&GL_pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	memset(&pfd_test, 0, sizeof(PIXELFORMATDESCRIPTOR));

	GL_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	GL_pfd.nVersion = 1;
	GL_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	if (props.enable_opengl)
	{
		GL_pfd.dwFlags |= PFD_SUPPORT_OPENGL;
	}

	GL_pfd.iPixelType = PFD_TYPE_RGBA;
	GL_pfd.cColorBits = (BYTE)(props.pixel_format.red_size + props.pixel_format.green_size + props.pixel_format.blue_size + props.pixel_format.alpha_size);
	GL_pfd.cRedBits = (BYTE)(props.pixel_format.red_size);
	GL_pfd.cGreenBits = (BYTE)(props.pixel_format.green_size);
	GL_pfd.cBlueBits = (BYTE)(props.pixel_format.blue_size);
	GL_pfd.cAlphaBits = (BYTE)(props.pixel_format.alpha_size);
	GL_pfd.cDepthBits = (BYTE)(props.pixel_format.depth_size);
	GL_pfd.cStencilBits = (BYTE)(props.pixel_format.stencil_size);

	Assert(_windowHandle != NULL);

	auto device_context = GetDC(_windowHandle);

	if (!device_context)
	{
		mprintf(("Unable to get device context for OpenGL W32!\n"));
		return nullptr;
	}

	PixelFormat = ChoosePixelFormat(device_context, &GL_pfd);

	if (!PixelFormat)
	{
		mprintf(("Unable to choose pixel format for OpenGL W32!\n"));
		ReleaseDC(_windowHandle, device_context);
		return nullptr;
	}
	else
	{
		DescribePixelFormat(device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd_test);
	}

	if (!SetPixelFormat(device_context, PixelFormat, &GL_pfd))
	{
		mprintf(("Unable to set pixel format for OpenGL W32!\n"));
		ReleaseDC(_windowHandle, device_context);
		return nullptr;
	}

	mprintf(("  Requested SDL Video values = R: %d, G: %d, B: %d, depth: %d, stencil: %d\n",
		props.pixel_format.red_size, props.pixel_format.green_size, props.pixel_format.blue_size,
		props.pixel_format.depth_size, props.pixel_format.stencil_size));

	return std::unique_ptr<os::Viewport>(new MFCViewport(_windowHandle, device_context));
}

static void* gladWGLLoader(const char* name)
{
	return wglGetProcAddress(name);
}

std::unique_ptr<os::OpenGLContext> MFCGraphicsOperations::createOpenGLContext(os::Viewport* port, const os::OpenGLContextAttributes& ctx)
{
	auto mfcView = reinterpret_cast<MFCViewport*>(port);

	auto temp_ctx = wglCreateContext(mfcView->getHDC());
	if (!temp_ctx)
	{
		mprintf(("Unable to create fake context for OpenGL W32!\n"));
		return nullptr;
	}

	if (!wglMakeCurrent(mfcView->getHDC(), temp_ctx))
	{
		mprintf(("Unable to make current thread for OpenGL W32!\n"));
		if (!wglDeleteContext(temp_ctx))
		{
			mprintf(("Failed to delete render context!"));
		}
		return nullptr;
	}

	if (!gladLoadWGLLoader(gladWGLLoader, mfcView->getHDC()))
	{
		mprintf(("Failed to load WGL functions!\n"));
		wglMakeCurrent(nullptr, nullptr);
		if (!wglDeleteContext(temp_ctx))
		{
			mprintf(("Failed to delete render context!"));
		}
		return nullptr;
	}

	if (!GLAD_WGL_ARB_create_context)
	{
		// No support for 3.x
		if (ctx.major_version >= 3)
		{
			mprintf(("Can't create requested OpenGL context!\n"));
			wglMakeCurrent(nullptr, nullptr);
			if (!wglDeleteContext(temp_ctx))
			{
				mprintf(("Failed to delete fake context!"));
			}
			return nullptr;
		}
		// Major version is low enough -> just use the fake context
		return std::unique_ptr<os::OpenGLContext>(new MFCOpenGLContext(temp_ctx));
	}
	
	SCP_vector<int> attrib_list;
	attrib_list.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
	attrib_list.push_back(ctx.major_version);

	attrib_list.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
	attrib_list.push_back(ctx.minor_version);

	if (GLAD_WGL_ARB_create_context_profile)
	{
		attrib_list.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
		attrib_list.push_back(ctx.profile == os::OpenGLProfile::Core ?
			WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB);
	}

	int flags = 0;
	if (ctx.flags[os::OpenGLContextFlags::Debug])
	{
		flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
	}
	if (ctx.flags[os::OpenGLContextFlags::ForwardCompatible])
	{
		flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
	}
	if (flags != 0)
	{
		attrib_list.push_back(WGL_CONTEXT_FLAGS_ARB);
		attrib_list.push_back(flags);
	}

	attrib_list.push_back(0);

	auto attrib_ctx = wglCreateContextAttribsARB(mfcView->getHDC(), nullptr, attrib_list.data());

	// We don't need the fake context anymore
	wglMakeCurrent(nullptr, nullptr);
	if (!wglDeleteContext(temp_ctx))
	{
		mprintf(("Failed to delete fakse context!"));
	}
	
	if (attrib_ctx == nullptr)
	{
		mprintf(("Failed to create OpenGL context!\n"));
		return nullptr;
	}
	if (!wglMakeCurrent(mfcView->getHDC(), attrib_ctx))
	{
		mprintf(("Unable to make current thread for OpenGL W32!\n"));
		if (!wglDeleteContext(attrib_ctx))
		{
			mprintf(("Failed to delete render context!"));
		}
		return nullptr;
	}

	if (!gladLoadWGLLoader(gladWGLLoader, mfcView->getHDC()))
	{
		mprintf(("Failed to load WGL functions!\n"));
		wglMakeCurrent(nullptr, nullptr);
		if (!wglDeleteContext(temp_ctx))
		{
			mprintf(("Failed to delete render context!"));
		}
		return nullptr;
	}

	return std::unique_ptr<os::OpenGLContext>(new MFCOpenGLContext(attrib_ctx));
}

void MFCGraphicsOperations::makeOpenGLContextCurrent(os::Viewport* view, os::OpenGLContext* ctx)
{
	if (view == nullptr && ctx == nullptr)
	{
		if (!wglMakeCurrent(nullptr, nullptr))
		{
			mprintf(("Failed to make OpenGL context current!\n"));
		}
		return;
	}

	Assertion(view != nullptr, "Both viewport of context must be valid at this point!");
	Assertion(ctx != nullptr, "Both viewport of context must be valid at this point!");

	auto mfcCtx = reinterpret_cast<MFCOpenGLContext*>(ctx);
	auto mfcView = reinterpret_cast<MFCViewport*>(view);

	if (!wglMakeCurrent(mfcView->getHDC(), mfcCtx->getHandle()))
	{
		mprintf(("Failed to make OpenGL context current!\n"));
	}
}
