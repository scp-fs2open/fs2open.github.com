
#ifndef _MVC_GRAPHICS_OPERATIONS
#define _MVC_GRAPHICS_OPERATIONS
#pragma once

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"

#include <glad/glad_wgl.h>

class MFCViewport : public os::Viewport
{
	HWND _windowHandle = nullptr;
	HDC _device_context = nullptr;
public:
	explicit MFCViewport(HWND hwnd, HDC dc);

	~MFCViewport() override;

	SDL_Window* toSDLWindow() override;

	std::pair<uint32_t, uint32_t> getSize() override;

	void swapBuffers() override;

	void setState(os::ViewportState state) override;

	void minimize() override;

	void restore() override;

	HDC getHDC();
};

class MFCOpenGLContext : public os::OpenGLContext
{
	// HACK: Since OpenGL apparently likes global state we also have to make this global...
	static void* _oglDllHandle;
	static size_t _oglDllReferenceCount;

	HGLRC _render_context = nullptr;
public:
	explicit MFCOpenGLContext(HGLRC hglrc);

	~MFCOpenGLContext() override;

	static void* wglLoader(const char* name);

	os::OpenGLLoadProc getLoaderFunction() override;

	void setSwapInterval(int status) override;

	HGLRC getHandle();
};

class MFCGraphicsOperations : public os::GraphicsOperations
{
	HWND _windowHandle = nullptr;
public:

	explicit MFCGraphicsOperations(HWND hwnd);

	~MFCGraphicsOperations() override;

	std::unique_ptr<os::Viewport> createViewport(const os::ViewPortProperties& props) override;

	std::unique_ptr<os::OpenGLContext> createOpenGLContext(os::Viewport* port, const os::OpenGLContextAttributes& ctx) override;

	void makeOpenGLContextCurrent(os::Viewport* view, os::OpenGLContext* ctx) override;
};

#endif // _MVC_GRAPHICS_OPERATIONS
