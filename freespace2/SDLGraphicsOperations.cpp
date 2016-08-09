//
//

#include "SDLGraphicsOperations.h"

#include "cmdline/cmdline.h"

#include <stdlib.h>

class SDLOpenGLContext : public os::OpenGLContext
{
	SDL_GLContext _glCtx;
public:
	explicit SDLOpenGLContext(SDL_GLContext sdl_gl_context)
		: _glCtx(sdl_gl_context)
	{
	}

	~SDLOpenGLContext() override
	{
		SDL_GL_DeleteContext(_glCtx);
	}

	os::OpenGLLoadProc getLoaderFunction() override
	{
		return SDL_GL_GetProcAddress;
	}

	void makeCurrent()
	{
		SDL_GL_MakeCurrent(os_get_window(), _glCtx);
	}

	void swapBuffers() override
	{
		SDL_GL_SwapWindow(os_get_window());
	}

	void setSwapInterval(int status) override
	{
		SDL_GL_SetSwapInterval(status);
	}
};

SDLGraphicsOperations::~SDLGraphicsOperations() {
}

std::unique_ptr<os::OpenGLContext> SDLGraphicsOperations::createOpenGLContext(const os::OpenGLContextAttributes& attrs,
															 uint32_t width,
															 uint32_t height) {
	mprintf(("  Initializing SDL video...\n"));

#ifdef SCP_UNIX
	// Slight hack to make Mesa advertise S3TC support without libtxc_dxtn
	setenv("force_s3tc_enable", "true", 1);
#endif

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		Error(LOCATION, "Couldn't init SDL video: %s", SDL_GetError());
		return nullptr;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, attrs.red_size);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, attrs.green_size);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, attrs.blue_size);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, attrs.depth_size);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, attrs.stencil_size);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, (attrs.multi_samples == 0) ? 0 : 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, attrs.multi_samples);

	mprintf(("  Requested SDL Video values = R: %d, G: %d, B: %d, depth: %d, stencil: %d, double-buffer: %d, FSAA: %d\n",
		attrs.red_size, attrs.green_size, attrs.blue_size, attrs.depth_size, attrs.stencil_size, 1, attrs.multi_samples));

	uint32_t windowflags = SDL_WINDOW_OPENGL;
	if (Cmdline_fullscreen_window) {
		windowflags |= SDL_WINDOW_BORDERLESS;
	}

	uint display = os_config_read_uint("Video", "Display", 0);
	SDL_Window* window =
		SDL_CreateWindow(Osreg_title, SDL_WINDOWPOS_CENTERED_DISPLAY(display), SDL_WINDOWPOS_CENTERED_DISPLAY(display),
						 width, height, windowflags);
	if (window == nullptr) {
		Error(LOCATION, "Could not create window: %s\n", SDL_GetError());
		return nullptr;
	}

	os_set_window(window);

	auto ctx = SDL_GL_CreateContext(os_get_window());

	if (ctx == nullptr) {
		Error(LOCATION, "Could not create OpenGL Context: %s\n", SDL_GetError());
		return nullptr;
	}

	int r, g, b, depth, stencil, db, fsaa_samples;
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &r);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &b);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &db);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencil);
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &fsaa_samples);

	mprintf(("  Actual SDL Video values    = R: %d, G: %d, B: %d, depth: %d, stencil: %d, double-buffer: %d, FSAA: %d\n",
		r, g, b, depth, stencil, db, fsaa_samples));


	return std::unique_ptr<os::OpenGLContext>(new SDLOpenGLContext(ctx));
}

void SDLGraphicsOperations::makeOpenGLContextCurrent(os::OpenGLContext* ctx)
{
	if (ctx == nullptr)
	{
		SDL_GL_MakeCurrent(os_get_window(), nullptr);
	} else
	{
		reinterpret_cast<SDLOpenGLContext*>(ctx)->makeCurrent();
	}
}
