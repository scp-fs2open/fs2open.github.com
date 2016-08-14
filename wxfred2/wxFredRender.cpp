/*
 * Created for the FreeSpace2 Source Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "wxFredRender.h"

#include <globalincs/alphacolors.h>
#include <globalincs/pstypes.h>
#include <globalincs/vmallocator.h>
#include <graphics/2d.h>
#include <graphics/font.h>
#include <lighting/lighting.h>
#include <math/vecmat.h>
#include <osapi/osapi.h>
#include <parse/parselo.h>
#include <physics/physics.h>
#include <render/3d.h>

#include <SDL_loadso.h>

#include <wx/glcanvas.h>
#include <wx/window.h>

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

		canvas_p = win;
		context_p = new wxGLContext(win);

		Assertion(context_p != nullptr, "Failed to create wxGLContext!");
	};

	/**
	 * @brief The context deconstructor.
	 */
	~wxOpenGLContext() {};

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

const float FRED_DEFAULT_HTL_FOV = 0.485f;

static std::unique_ptr<wxGraphicsOperations> graphicsOperations;

static bool fred_inited = false;

void* wxOpenGLContext::_oglDllHandle = nullptr;
size_t wxOpenGLContext::_oglDllReferenceCount = 0;

// Colors used for 2d rendering
color colour_black;
color colour_green;
color colour_white;
color colour_yellow;

/**
 * @brief Renders the asteroid field
 */
void render_asteroid_field();

/**
 * @brief Renders the compass widget
 */
void render_compass();

/**
 * @brief Renders the grid widget
 */
void render_grid(grid* grid_p);

/**
 * @brief Renders all modeled objects within the mission
 */
void render_models();

/**
* @brief Displays/Renders the distances between selected objects
*/
void display_distances();

/**
 * @brief Highlights the currently selected skybox bitmap
 */
void hilight_bitmap();


void render_asteroid_field() {}

void render_compass() {}

void render_grid(grid* grid_p) {}

void render_models() {}

void display_distances() {}

void hilight_bitmap() {
	// Currently disabled for now.
	/*
	int i;
	vertex p[4];

	if (Starfield_bitmaps[Cur_bitmap].bitmap_index == -1)  // can't draw if no bitmap
	return;

	for (i=0; i<4; i++)
	{
	g3_rotate_faraway_vertex(&p[i], &Starfield_bitmaps[Cur_bitmap].points[i]);
	if (p[i].codes & CC_BEHIND)
	return;

	g3_project_vertex(&p[i]);
	if (p[i].flags & PF_OVERFLOW)
	return;
	}

	gr_set_color(255, 255, 255);
	g3_draw_line(&p[0], &p[1]);
	g3_draw_line(&p[1], &p[2]);
	g3_draw_line(&p[2], &p[3]);
	g3_draw_line(&p[3], &p[0]);
	*/
}


void wxfred::render_init(glcViewport* win) {
	if (!fred_inited) {
		// Create the GraphicsOperations helper object with the target window
		graphicsOperations.reset(new wxGraphicsOperations(win));

		// Do graphics init
		gr_init(graphicsOperations.get(), GR_OPENGL, 640, 480, 32);

		font::init();

		gr_set_gamma(3.0f);

		// sprintf(palette_filename, "gamepalette%d-%02d", 1, 1);
		// mprintf(("Loading palette %s\n", palette_filename));
		// palette_load_table(palette_filename);

		// particle::ParticleManager::init();

		alpha_colors_init();
		// glowpoint_init();
		// neb2_init();
		// stars_init();
		// brief_init_colors();
		// stars_post_level_init();
		// nebl_init();

		gr_init_alphacolor(&colour_white, 255, 255, 255, 255);
		gr_init_alphacolor(&colour_green, 0, 200, 0, 255);
		gr_init_alphacolor(&colour_yellow, 200, 255, 0, 255);
		gr_init_alphacolor(&colour_black, 0, 0, 0, 255);

		gr_reset_clip();
		g3_start_frame(0);
		g3_set_view_matrix(&win->vset.c_pos, &win->vset.c_orient, 0.5f);

		fred_inited = true;
	} else {
		// Set render target to window?
	}
}

void wxfred::render_frame(glcViewport *win) {
	Assertion(fred_inited, "Call to render_frame was made before render_init!\n");

	// Caller viewport must set context as current before entering!!
	// Caller viewport must swap buffers after we've rendered!

	// @note The following is essentially copypasty from fredrender::render_frame()
	g3_end_frame();	 // ** Accounted for

	gr_reset_clip();
	gr_clear();

	// Briefing dialog stuff goes here, in FRED.

	g3_start_frame(1);
	font::set_font(font::FONT1);
	light_reset();

	g3_set_view_matrix(&win->vset.c_pos, &win->vset.c_orient, FRED_DEFAULT_HTL_FOV);

	// fred_enable_htl();
	// Bg_bitmap_dialog?
	// fred_disable_htl();

	if (win->vset.show_horizon) {
		gr_set_color(128, 128, 64);
		g3_draw_horizon_line();
	}

	if (win->vset.show_asteroids) {
		gr_set_color(192, 96, 16);
		render_asteroid_field();
	}

	if (win->vset.show_grid) {
		render_grid(win->grid);
	}

	//	if (Bg_bitmap_dialog) {
	if (false) {
		hilight_bitmap();
	}

	gr_set_color(0, 0, 64);
	render_models();

	if (win->vset.show_distances) {
		display_distances();
	}

	/*
	display_ship_info();
	display_active_ship_subsystem();
	render_active_rect();
	*/

	// Fred tooltip stuff here

	gr_set_color(0, 160, 0);
	/*
	fred_enable_htl();
	jumpnode_render_all();
	fred_disable_htl();
	*/

	auto c_pos = win->vset.c_pos;
	auto c_orient = win->vset.c_orient;
	SCP_string buf;
	int w, h;	// Used for displaying current position text

	sprintf(buf, "(%.1f,%.1f,%.1f)", c_pos.xyz.x, c_pos.xyz.y, c_pos.xyz.z);
	gr_get_string_size(&w, &h, buf.c_str());
	gr_set_color_fast(&colour_white);
	gr_string(gr_screen.max_w - w - 2, 2, buf.c_str());

	g3_end_frame();	 // ** Accounted for
	render_compass();

	gr_flip();

	gr_reset_clip();

	g3_start_frame(0);	 // ** Accounted for
	g3_set_view_matrix(&c_pos, &c_orient, FRED_DEFAULT_HTL_FOV);
}
