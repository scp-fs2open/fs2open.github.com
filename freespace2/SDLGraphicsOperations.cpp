//
//

#include "SDLGraphicsOperations.h"

#if SDL_VERSION_ATLEAST(2, 0, 6)
#include <SDL_vulkan.h>
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"
#endif

namespace {
void setOGLProperties(const os::ViewPortProperties& props)
{
	SDL_GL_ResetAttributes();

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, props.pixel_format.red_size);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, props.pixel_format.green_size);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, props.pixel_format.blue_size);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, props.pixel_format.depth_size);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, props.pixel_format.stencil_size);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// disabled due to issues with implementation; may be re-enabled in future
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

	mprintf(("  Requested SDL Pixel values = R: %d, G: %d, B: %d, depth: %d, stencil: %d, double-buffer: %d, FSAA: %d\n",
		props.pixel_format.red_size, props.pixel_format.green_size, props.pixel_format.blue_size,
		props.pixel_format.depth_size, props.pixel_format.stencil_size, 1, props.pixel_format.multi_samples));

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, props.gl_attributes.major_version);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, props.gl_attributes.minor_version);

	int profile;
	switch (props.gl_attributes.profile) {
		case os::OpenGLProfile::Core:
			profile = SDL_GL_CONTEXT_PROFILE_CORE;
			break;
		case os::OpenGLProfile::Compatibility:
			profile = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
			break;
		default:
			UNREACHABLE("Unhandled profile value!");
			return;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile);

	int flags = 0;
	if (props.gl_attributes.flags[os::OpenGLContextFlags::Debug]) {
		flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
	}
	if (props.gl_attributes.flags[os::OpenGLContextFlags::ForwardCompatible]) {
		flags |= SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
}
}

class SDLOpenGLContext: public os::OpenGLContext {
	SDL_GLContext _glCtx;
 public:
	SDLOpenGLContext(SDL_GLContext sdl_gl_context) : _glCtx(sdl_gl_context) {
	}

	~SDLOpenGLContext() override {
		SDL_GL_DeleteContext(_glCtx);
	}

	os::OpenGLLoadProc getLoaderFunction() override {
		return SDL_GL_GetProcAddress;
	}

	void makeCurrent(SDL_Window* window) {
		SDL_GL_MakeCurrent(window, _glCtx);
	}

	bool setSwapInterval(int status) override {
		return SDL_GL_SetSwapInterval(status) == 0;
	}
};
class SDLWindowViewPort: public os::Viewport {
	SDL_Window* _window;
	os::ViewPortProperties _props;
 public:
	SDLWindowViewPort(SDL_Window* window, const os::ViewPortProperties& props) : _window(window), _props(props) {
		Assertion(window != nullptr, "Invalid window specified");
	}
	~SDLWindowViewPort() override {
		SDL_DestroyWindow(_window);
		_window = nullptr;
	}

	const os::ViewPortProperties& getProps() const {
		return _props;
	}
	SDL_Window* toSDLWindow() override {
		return _window;
	}
	std::pair<uint32_t, uint32_t> getSize() override {
		int width, height;
		SDL_GetWindowSize(_window, &width, &height);

		return std::make_pair(width, height);
	}
	void swapBuffers() override {
		SDL_GL_SwapWindow(_window);
	}
	void setState(os::ViewportState state) override {
		switch (state) {
			case os::ViewportState::Windowed:
				SDL_SetWindowFullscreen(_window, 0);
				SDL_SetWindowBordered(_window, SDL_TRUE);
				break;
			case os::ViewportState::Borderless:
				SDL_SetWindowFullscreen(_window, 0);
				SDL_SetWindowBordered(_window, SDL_FALSE);
				break;
			case os::ViewportState::Fullscreen:
				SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN);
				break;
			default:
				UNREACHABLE("Invalid window state!");
				break;
		}
	}
	void minimize() override {
		// lets not minimize if we are in windowed mode
		if (!(SDL_GetWindowFlags(_window) & SDL_WINDOW_FULLSCREEN)) {
			return;
		}

		SDL_MinimizeWindow(_window);
	}

	void restore() override {
		SDL_RestoreWindow(_window);
	}
};

SDLGraphicsOperations::SDLGraphicsOperations() {
	mprintf(("  Initializing SDL video...\n"));

#ifdef SCP_UNIX
	// Slight hack to make Mesa advertise S3TC support without libtxc_dxtn
	setenv("force_s3tc_enable", "true", 1);
#endif

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		Error(LOCATION, "Couldn't init SDL video: %s", SDL_GetError());
		return;
	}
}
SDLGraphicsOperations::~SDLGraphicsOperations() {
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	ImGui_ImplSDL2_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
}
std::unique_ptr<os::Viewport> SDLGraphicsOperations::createViewport(const os::ViewPortProperties& props)
{
	uint32_t windowflags = SDL_WINDOW_SHOWN;
	if (props.enable_opengl) {
		windowflags |= SDL_WINDOW_OPENGL;
		setOGLProperties(props);
	}
#if SDL_VERSION_ATLEAST(2, 0, 6)
	if (props.enable_vulkan) {
		windowflags |= SDL_WINDOW_VULKAN;
	}
#endif
	if (props.flags[os::ViewPortFlags::Borderless]) {
		windowflags |= SDL_WINDOW_BORDERLESS;
	}
	if (props.flags[os::ViewPortFlags::Fullscreen]) {
		windowflags |= SDL_WINDOW_FULLSCREEN;
	}
	if (props.flags[os::ViewPortFlags::Resizeable]) {
		windowflags |= SDL_WINDOW_RESIZABLE;
	}

	SDL_Rect bounds;
	if (SDL_GetDisplayBounds(props.display, &bounds) != 0) {
		mprintf(("Failed to get display bounds: %s\n", SDL_GetError()));
		return nullptr;
	}

	int x;
	int y;

	if (bounds.w == (int)props.width && bounds.h == (int)props.height) {
		// If we have the same size as the desktop we explicitly specify 0,0 to make sure that the window borders aren't hidden
		mprintf(("SDL: Creating window at %d,%d because window has same size as desktop.\n", bounds.x, bounds.y));
		x = bounds.x;
		y = bounds.y;
	} else {
		x = SDL_WINDOWPOS_CENTERED_DISPLAY(props.display);
		y = SDL_WINDOWPOS_CENTERED_DISPLAY(props.display);
	}

	SDL_Window* window = SDL_CreateWindow(props.title.c_str(),
										  x,
										  y,
										  props.width,
										  props.height,
										  windowflags);
	if (window == nullptr) {
		mprintf(("Failed to create SDL Window: %s\n", SDL_GetError()));
		return nullptr;
	}

	return std::unique_ptr<os::Viewport>(new SDLWindowViewPort(window, props));
}
void SDLGraphicsOperations::makeOpenGLContextCurrent(os::Viewport* view, os::OpenGLContext* ctx) {
	if (view == nullptr && ctx == nullptr) {
		SDL_GL_MakeCurrent(nullptr, nullptr);
		return;
	}

	Assertion(view != nullptr, "Both viewport of context must be valid at this point!");
	Assertion(ctx != nullptr, "Both viewport of context must be valid at this point!");

	auto sdlCtx = reinterpret_cast<SDLOpenGLContext*>(ctx);
	sdlCtx->makeCurrent(view->toSDLWindow());
}
std::unique_ptr<os::OpenGLContext> SDLGraphicsOperations::createOpenGLContext(os::Viewport* viewport,
																			  const os::OpenGLContextAttributes& gl_attrs) {
	auto sdlViewport = reinterpret_cast<SDLWindowViewPort*>(viewport);

	auto props = sdlViewport->getProps();
	props.gl_attributes = gl_attrs;
	setOGLProperties(props);

	auto ctx = SDL_GL_CreateContext(viewport->toSDLWindow());

	if (ctx == nullptr) {
		mprintf(("Could not create OpenGL Context: %s\n", SDL_GetError()));
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


	ImGui_ImplSDL2_InitForOpenGL(viewport->toSDLWindow(), ctx);
	ImGui_ImplOpenGL3_Init();

	return std::unique_ptr<os::OpenGLContext>(new SDLOpenGLContext(ctx));
}
