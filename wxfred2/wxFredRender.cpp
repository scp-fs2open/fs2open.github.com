/*
 * Created for the FreeSpace2 Source Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "wxFredRender.h"

#include <globalincs/pstypes.h>
#include <math/vecmat.h>
#include <osapi/osapi.h>
#include <physics/physics.h>

#include <SDL_loadso.h>
#include <wx/glcanvas.h>'

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif
using namespace wxfred;

class wxOpenGLContext : public os::OpenGLContext
{
public:
	/**
	 * @brief The context constructor. Must have a valid wxGLCanvas as input
	 */
	explicit wxOpenGLContext(wxGLCanvas* win) {
		Assertion(win != nullptr, "Could not create wxOpenGLContext! Must have a valid wxGLCanvas class (or derivative)\n");

		context_p = new wxGLContext(win);

		Assertion(context_p != nullptr, "Failed to create wxGLContext!");
	};

	/**
	 * @brief The context deconstructor.
	 */
	~wxOpenGLContext() {
	};

	/**
	 * @brief The OpenGL loader function. Loads the OPENGL32.DLL
	 */
	static void* wglLoader(const char* name) {
		return SDL_LoadFunction(_oglDllHandle, name);
	}

	/**
	 * @brief Gets the OpenGL loader function
	 */
	os::OpenGLLoadProc getLoaderFunction() override {
		return wglLoader;
	};

	/**
	 * @brief Swaps the buffers of this context
	 */
	void swapBuffers() override {
		canvas_p->SwapBuffers();
	}

	/**
	 * @brief Sets the swap interval, used in vsync modes
	 *
	 * @note Unused by wxFRED
	 * @note This might cause problems. wxWidgets doesn't provide a SwapInterval wrapper, so we'd have to use something
	 *  else to get vsync operational
	 */
	void setSwapInterval(int status) override {};

	void makeCurrent() {
		if (!canvas_p->SetCurrent(*context_p)) {
			mprintf(("Failed to make OpenGL conext current!\n"));
		}
	}

	void setTarget(wxGLCanvas* win) {
		Assertion(win != nullptr, "Cannot set render target to a nullptr!\n");

		// Check if target window has same properties as context (and existing canvas)
		canvas_p = win;
	}

private:
	// HACK: Since OpenGL apparently likes global state we also have to make this global...
	static void* _oglDllHandle;
	static size_t _oglDllReferenceCount;

	wxGLContext *context_p; //!< Reference to the wxGLContext, which must exist
	wxGLCanvas  *canvas_p;  //!< Reference to the current canvas, which must exist
};

class wxGraphicsOperations : public os::GraphicsOperations
{
public:
	explicit wxGraphicsOperations(wxWindow *win) {
		Assertion(win != nullptr, "Failed to create wxGraphicsOperations! An invalid reference to a wxWindow was passed to the constructor.\n");
		_parent = win;
	}

	~wxGraphicsOperations() {
		// Destroy context on death
	}

	std::unique_ptr<os::OpenGLContext> createOpenGLContext(const os::OpenGLContextAttributes& attrs, uint32_t width, uint32_t height) override {
		// wxWidgets 3.0's wierd-ass attribute list
		int attributeList[] = {
			WX_GL_RGBA,
			WX_GL_DOUBLEBUFFER,
			WX_GL_MIN_RED, attrs.red_size,
			WX_GL_MIN_GREEN, attrs.green_size,
			WX_GL_MIN_BLUE, attrs.blue_size,
			WX_GL_MIN_ALPHA, attrs.alpha_size,

			WX_GL_DEPTH_SIZE, attrs.depth_size,
			WX_GL_STENCIL_SIZE, attrs.stencil_size,

			WX_GL_SAMPLE_BUFFERS, ((attrs.multi_samples == 0) ? 0 : 1),
			WX_GL_SAMPLES, attrs.multi_samples,	// "4 for 2x2 antialiasing supersampling on most graphics cards "

			// Currently unused
//			0, attrs.major_version,
//			0, attrs.minor_version,

//			0, attrs.flags,
//			OpenGLProfile profile,

			0	// attributeList must be 0 terminated. :shrug:
		};

		mprintf(("  Requested GL Video values = R: %d, G: %d, B: %d, depth: %d, stencil: %d, double-buffer: %d, FSAA: %d\n",
			attrs.red_size, attrs.green_size, attrs.blue_size, attrs.depth_size, attrs.stencil_size, 1, attrs.multi_samples));

		// Create canvas with attributes
		auto canvas = new wxGLCanvas(_parent, wxID_ANY, attributeList, wxDefaultPosition, wxSize(width, height), NULL, "GLCanvas", wxNullPalette);
		if (canvas == nullptr) {
			Error(LOCATION, "Could not create initial wxGLCanvas!\n");
			return nullptr;
		}

		// wxWidgets doesn't supply a method to get the attributes used, because creation of the wxCanvas will fail on anything unsupported
		

		// Create context from the canvas
		return std::unique_ptr<os::OpenGLContext>(new wxOpenGLContext(canvas));
	};

	/**
	 * @brief Makes the given context current. i.e. subsequent gr calls will draw to this context
	 *
	 * @param[in] ctx The context to be made current, or nullptr to disconnect context from caller
	 */
	void makeOpenGLContextCurrent(os::OpenGLContext* ctx) override {
		if (ctx != nullptr) {
			reinterpret_cast<wxOpenGLContext*>(ctx)->makeCurrent();
		} // Else, do nothing. Context cleans up after itself on death
	}

private:
	wxWindow* _parent;	// The initial window we'll be making canvas/context for
};

void wxfred::render_init() {

}

void wxfred::render_frame(ViewSettings& vset) {
	// Caller viewport must set context as current before entering!!
	// Caller viewport must swap buffers after we've rendered!
}
