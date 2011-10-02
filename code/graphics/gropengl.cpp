




#include "globalincs/pstypes.h"
#include "cmdline/cmdline.h"
#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "bmpman/bmpman.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "globalincs/systemvars.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/line.h"
#include "nebula/neb.h"
#include "io/mouse.h"
#include "osapi/osregistry.h"
#include "cfile/cfile.h"
#include "io/timer.h"
#include "ddsutils/ddsutils.h"
#include "model/model.h"
#include "debugconsole/timerbar.h"
#include "graphics/gropenglbmpman.h"
#include "graphics/gropengllight.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltnl.h"
#include "graphics/gropenglbmpman.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglstate.h"
#include "graphics/gropenglpostprocessing.h"


#if defined(_WIN32)
#include <windows.h>
#include <windowsx.h>
#include <direct.h>
#elif defined(__APPLE__)
#include "OpenGL.h"
#else
//#include <GL/glx.h>
typedef int ( * PFNGLXSWAPINTERVALSGIPROC) (int interval);
#endif


#if defined(_WIN32) && !defined(__GNUC__)
#pragma comment (lib, "opengl32")
#pragma comment (lib, "glu32")
#endif

// minimum GL version we can reliably support is 1.2
static const int MIN_REQUIRED_GL_VERSION = 12;

int GL_version = 0;

bool GL_initted = 0;

//0==no fog
//1==linear
//2==fog coord EXT
//3==NV Radial
int OGL_fogmode = 0;

#ifdef _WIN32
static HDC GL_device_context = NULL;
static HGLRC GL_render_context = NULL;
static PIXELFORMATDESCRIPTOR GL_pfd;
#endif

static ushort *GL_original_gamma_ramp = NULL;

int Use_VBOs = 0;
int Use_PBOs = 0;
int Use_GLSL = 0;

static int GL_dump_frames = 0;
static ubyte *GL_dump_buffer = NULL;
static int GL_dump_frame_number = 0;
static int GL_dump_frame_count = 0;
static int GL_dump_frame_count_max = 0;
static int GL_dump_frame_size = 0;

static ubyte *GL_saved_screen = NULL;
static ubyte *GL_saved_mouse_data = NULL;
static int GL_saved_screen_id = -1;
static GLuint GL_cursor_pbo = 0;
static GLuint GL_screen_pbo = 0;

static int GL_mouse_saved = 0;
static int GL_mouse_saved_x1 = 0;
static int GL_mouse_saved_y1 = 0;
static int GL_mouse_saved_x2 = 0;
static int GL_mouse_saved_y2 = 0;

void opengl_save_mouse_area(int x, int y, int w, int h);

extern char *Osreg_title;

extern GLfloat GL_anisotropy;

extern float FreeSpace_gamma;
void gr_opengl_set_gamma(float gamma);

extern float FreeSpace_gamma;
void gr_opengl_set_gamma(float gamma);

static int GL_fullscreen = 0;
static int GL_windowed = 0;
static int GL_minimized = 0;

static GLenum GL_read_format = GL_BGRA;


void opengl_go_fullscreen()
{
	if (Cmdline_fullscreen_window || Cmdline_window || GL_fullscreen || Fred_running)
		return;

#ifdef _WIN32
	DEVMODE dm;
	RECT cursor_clip;
	HWND wnd = (HWND)os_get_window();

	Assert( wnd );

	os_suspend();

	memset((void*)&dm, 0, sizeof(DEVMODE));

	dm.dmSize = sizeof(DEVMODE);
	dm.dmPelsHeight = gr_screen.max_h;
	dm.dmPelsWidth = gr_screen.max_w;
	dm.dmBitsPerPel = gr_screen.bits_per_pixel;
	dm.dmDisplayFrequency = os_config_read_uint( NULL, NOX("OGL_RefreshRate"), 0 );
	dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	if (dm.dmDisplayFrequency)
		dm.dmFields |= DM_DISPLAYFREQUENCY;

	if ( (ChangeDisplaySettings(&dm, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL ) {
		if (dm.dmDisplayFrequency) {
			// failed to switch with freq change so try without it just in case
			dm.dmDisplayFrequency = 0;
			dm.dmFields &= ~DM_DISPLAYFREQUENCY;

			if ( (ChangeDisplaySettings(&dm, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL ) {
				Warning( LOCATION, "Unable to go fullscreen on second attempt!" );
			}
		} else {
			Warning( LOCATION, "Unable to go fullscreen!" );
		}
	}

	ShowWindow( wnd, SW_SHOWNORMAL );
	UpdateWindow( wnd );

	SetForegroundWindow( wnd );
	SetActiveWindow( wnd );
	SetFocus( wnd );

	GetWindowRect((HWND)os_get_window(), &cursor_clip);
	ClipCursor(&cursor_clip);
	ShowCursor(FALSE);

	os_resume();  
#else
	if ( (os_config_read_uint(NULL, NOX("Fullscreen"), 1) == 1) && !(SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) ) {
		os_suspend();
	//	SDL_WM_ToggleFullScreen( SDL_GetVideoSurface() );
		if ( (SDL_SetVideoMode(gr_screen.max_w, gr_screen.max_h, 0, SDL_OPENGL | SDL_FULLSCREEN)) == NULL ) {
			mprintf(("Couldn't go fullscreen!\n"));
			if ( (SDL_SetVideoMode(gr_screen.max_w, gr_screen.max_h, 0, SDL_OPENGL)) == NULL ) {
				mprintf(("Couldn't drop back to windowed mode either!\n"));
				exit(1);
			}
		}
		os_resume();
	}
#endif

	gr_opengl_set_gamma(FreeSpace_gamma);

	GL_fullscreen = 1;
	GL_minimized = 0;
	GL_windowed = 0;
}

void opengl_go_windowed()
{
	if ( ( !Cmdline_fullscreen_window && !Cmdline_window ) /*|| GL_windowed*/ || Fred_running )
		return;

#ifdef _WIN32
	HWND wnd = (HWND)os_get_window();
	Assert( wnd );

	// if we are already in a windowed state, then just make sure that we are sane and bail
	if (GL_windowed) {
		SetForegroundWindow( wnd );
		SetActiveWindow( wnd );
		SetFocus( wnd );

		ClipCursor(NULL);
		ShowCursor(FALSE);
		return;
	}

	os_suspend();

	ShowWindow( wnd, SW_SHOWNORMAL );
	UpdateWindow( wnd );

	SetForegroundWindow( wnd );
	SetActiveWindow( wnd );
	SetFocus( wnd );

	ClipCursor(NULL);
	ShowCursor(FALSE);

	os_resume();  

#else
	if (SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) {
		os_suspend();

	//	SDL_WM_ToggleFullScreen( SDL_GetVideoSurface() );
		if ( (SDL_SetVideoMode(gr_screen.max_w, gr_screen.max_h, 0, SDL_OPENGL)) == NULL ) {
			Warning( LOCATION, "Unable to enter windowed mode!" );
		}

		os_resume();
	}
#endif

	GL_windowed = 1;
	GL_minimized = 0;
	GL_fullscreen = 0;
}

void opengl_minimize()
{
	// don't attempt to minimize if we are already in a window, or already minimized, or when playing a movie
	if (GL_minimized /*|| GL_windowed || Cmdline_window*/ || Fred_running)
		return;

#ifdef _WIN32
	HWND wnd = (HWND)os_get_window();
	Assert( wnd );

	// if we are a window then just show the cursor and bail
	if ( Cmdline_fullscreen_window || Cmdline_window || GL_windowed) {
		ClipCursor(NULL);
		ShowCursor(TRUE);
		return;
	}

	os_suspend();

	// restore original gamma settings
	if (GL_original_gamma_ramp != NULL) {
		SetDeviceGammaRamp( GL_device_context, GL_original_gamma_ramp );
	}

	ShowWindow(wnd, SW_MINIMIZE);
	ChangeDisplaySettings(NULL, 0);

	ClipCursor(NULL);
	ShowCursor(TRUE);

	os_resume();
#else
	// lets not minimize if we are in windowed mode
	if ( !(SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) )
		return;

	os_suspend();

	if (GL_original_gamma_ramp != NULL) {
		SDL_SetGammaRamp( GL_original_gamma_ramp, (GL_original_gamma_ramp+256), (GL_original_gamma_ramp+512) );
	}

	SDL_WM_IconifyWindow();
	os_resume();
#endif

	GL_minimized = 1;
	GL_windowed = 0;
	GL_fullscreen = 0;
}

void gr_opengl_activate(int active)
{
	if (active) {
		if (Cmdline_fullscreen_window||Cmdline_window)
			opengl_go_windowed();
		else
			opengl_go_fullscreen();

#ifdef SCP_UNIX
		// Check again and if we didn't go fullscreen turn on grabbing if possible
		if(!Cmdline_no_grab && !(SDL_GetVideoSurface()->flags & SDL_FULLSCREEN)) {
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}
#endif
	} else {
		opengl_minimize();

#ifdef SCP_UNIX
		// let go of mouse/keyboard
		if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON)
			SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
	}
}

void gr_opengl_clear()
{
	glClearColor(gr_screen.current_clear_color.red / 255.0f, 
		gr_screen.current_clear_color.green / 255.0f, 
		gr_screen.current_clear_color.blue / 255.0f, gr_screen.current_clear_color.alpha / 255.0f);

	glClear ( GL_COLOR_BUFFER_BIT );
}

void gr_opengl_flip()
{
	if ( !GL_initted )
		return;

	gr_reset_clip();

	mouse_eval_deltas();
	
	GL_mouse_saved = 0;
	
	if ( mouse_is_visible() ) {
		int mx, my;

		gr_reset_clip();
		mouse_get_pos( &mx, &my );

	//	opengl_save_mouse_area(mx, my, Gr_cursor_size, Gr_cursor_size);

		if (Gr_cursor != -1) {
			gr_set_bitmap(Gr_cursor);
			gr_bitmap( mx, my, false);
		}
	}

#ifdef _WIN32
	SwapBuffers(GL_device_context);
#else
	SDL_GL_SwapBuffers();
#endif

	opengl_tcache_frame();

#ifndef NDEBUG
	int ic = opengl_check_for_errors();

	if (ic) {
		mprintf(("!!DEBUG!! OpenGL Errors this frame: %i\n", ic));
	}
#endif
}

void gr_opengl_set_clip(int x, int y, int w, int h, bool resize)
{
	// check for sanity of parameters
	if (x < 0) {
		x = 0;
	}

	if (y < 0) {
		y = 0;
	}

	int to_resize = (resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)));

	int max_w = ((to_resize) ? gr_screen.max_w_unscaled : gr_screen.max_w);
	int max_h = ((to_resize) ? gr_screen.max_h_unscaled : gr_screen.max_h);

	if (x >= max_w) {
		x = max_w - 1;
	}

	if (y >= max_h) {
		y = max_h - 1;
	}

	if (x + w > max_w) {
		w = max_w - x;
	}

	if (y + h > max_h) {
		h = max_h - y;
	}
	
	if (w > max_w) {
		w = max_w;
	}

	if (h > max_h) {
		h = max_h;
	}

	gr_screen.offset_x_unscaled = x;
	gr_screen.offset_y_unscaled = y;
	gr_screen.clip_left_unscaled = 0;
	gr_screen.clip_right_unscaled = w-1;
	gr_screen.clip_top_unscaled = 0;
	gr_screen.clip_bottom_unscaled = h-1;
	gr_screen.clip_width_unscaled = w;
	gr_screen.clip_height_unscaled = h;

	if (to_resize) {
		gr_resize_screen_pos(&x, &y);
		gr_resize_screen_pos(&w, &h);
	} else {
		gr_unsize_screen_pos( &gr_screen.offset_x_unscaled, &gr_screen.offset_y_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled );
	}

	gr_screen.offset_x = x;
	gr_screen.offset_y = y;
	gr_screen.clip_left = 0;
	gr_screen.clip_right = w-1;
	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h-1;
	gr_screen.clip_width = w;
	gr_screen.clip_height = h;

	gr_screen.clip_aspect = i2fl(w) / i2fl(h);
	gr_screen.clip_center_x = (gr_screen.clip_left + gr_screen.clip_right) * 0.5f;
	gr_screen.clip_center_y = (gr_screen.clip_top + gr_screen.clip_bottom) * 0.5f;

	// just return early if we aren't actually going to need the scissor test
	if ( (x == 0) && (y == 0) && (w == max_w) && (h == max_h) ) {
		GL_state.ScissorTest(GL_FALSE);
		return;
	}

	GL_state.ScissorTest(GL_TRUE);
	if(GL_rendering_to_framebuffer) {
		glScissor(x, y, w, h);
	} else {
		glScissor(x, gr_screen.max_h-y-h, w, h);
	}
}

void gr_opengl_reset_clip()
{
	gr_screen.offset_x = gr_screen.offset_x_unscaled = 0;
	gr_screen.offset_y = gr_screen.offset_y_unscaled = 0;
	gr_screen.clip_left = gr_screen.clip_left_unscaled = 0;
	gr_screen.clip_top = gr_screen.clip_top_unscaled = 0;
	gr_screen.clip_right = gr_screen.clip_right_unscaled = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.clip_bottom_unscaled = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.clip_width_unscaled = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.clip_height_unscaled = gr_screen.max_h;

	if (gr_screen.custom_size) {
		gr_unsize_screen_pos( &gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled );
	}

	gr_screen.clip_aspect = i2fl(gr_screen.clip_width) / i2fl(gr_screen.clip_height);
	gr_screen.clip_center_x = (gr_screen.clip_left + gr_screen.clip_right) * 0.5f;
	gr_screen.clip_center_y = (gr_screen.clip_top + gr_screen.clip_bottom) * 0.5f;

	GL_state.ScissorTest(GL_FALSE);
}

void gr_opengl_set_palette(ubyte *new_palette, int is_alphacolor)
{
}

void gr_opengl_print_screen(char *filename)
{
	char tmp[MAX_PATH_LEN];
	ubyte tga_hdr[18];
	int i;
	ushort width, height;
	GLubyte *pixels = NULL;
	GLuint pbo = 0;

	// save to a "screenshots" directory and tack on the filename
#ifdef SCP_UNIX
	snprintf( tmp, MAX_PATH_LEN-1, "%s/%s/screenshots/%s.tga", detect_home(), Osreg_user_dir, filename);
	_mkdir( tmp );
#else
	_getcwd( tmp, MAX_PATH_LEN-1 );
	strcat_s( tmp, "\\screenshots\\" );
	_mkdir( tmp );

	strcat_s( tmp, filename );
	strcat_s( tmp, ".tga" );
#endif

	FILE *fout = fopen(tmp, "wb");

	if (fout == NULL) {
		return;
	}

//	glReadBuffer(GL_FRONT);

	// now for the data
	if (Use_PBOs) {
		Assert( !pbo );
		vglGenBuffersARB(1, &pbo);

		if ( !pbo ) {
			if (fout != NULL)
				fclose(fout);

			return;
		}

		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbo);
		vglBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, (gr_screen.max_w * gr_screen.max_h * 4), NULL, GL_STATIC_READ);

		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		// map the image data so that we can save it to file
		pixels = (GLubyte*) vglMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);
	} else {
		pixels = (GLubyte*) vm_malloc_q(gr_screen.max_w * gr_screen.max_h * 4);

		if (pixels == NULL) {
			if (fout != NULL) {
				fclose(fout);
			}

			return;
		}

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
		glFlush();
	}

	// Write the TGA header
	width = INTEL_SHORT((ushort)gr_screen.max_w);
	height = INTEL_SHORT((ushort)gr_screen.max_h);

	memset( tga_hdr, 0, sizeof(tga_hdr) );

	tga_hdr[2] = 2;		// ImageType    2 = 24bpp, uncompressed
	memcpy( tga_hdr + 12, &width, sizeof(ushort) );		// Width
	memcpy( tga_hdr + 14, &height, sizeof(ushort) );	// Height
	tga_hdr[16] = 24;	// PixelDepth

	fwrite( tga_hdr, sizeof(tga_hdr), 1, fout );

	// now for the data, we convert it from 32-bit to 24-bit
	for (i = 0; i < (gr_screen.max_w * gr_screen.max_h * 4); i += 4) {
#if BYTE_ORDER == BIG_ENDIAN
		int pix, *pix_tmp;

		pix_tmp = (int*)(pixels + i);
		pix = INTEL_INT(*pix_tmp);

		fwrite( &pix, 1, 3, fout );
#else
		fwrite( pixels + i, 1, 3, fout );
#endif
	}

	if (pbo) {
		vglUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
		pixels = NULL;
		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
		vglDeleteBuffersARB(1, &pbo);
	}

	// done!
	fclose(fout);

	if (pixels != NULL) {
		vm_free(pixels);
	}
}

void gr_opengl_cleanup(int minimize)
{	
	if ( !GL_initted ) {
		return;
	}

	if ( !Fred_running ) {
		gr_reset_clip();
		gr_clear();
		gr_flip();
		gr_clear();
		gr_flip();
		gr_clear();
	}

	GL_initted = false;

	opengl_tcache_flush();

#ifdef _WIN32
	HWND wnd = (HWND)os_get_window();

	if (GL_render_context) {
		if ( !wglMakeCurrent(NULL, NULL) ) {
			MessageBox(wnd, "SHUTDOWN ERROR", "error", MB_OK);
		}

		if ( !wglDeleteContext(GL_render_context) ) {
			MessageBox(wnd, "Unable to delete rendering context", "error", MB_OK);
		}

		GL_render_context = NULL;
	}
#endif

	opengl_minimize();

	if (minimize) {
#ifdef _WIN32
		if ( !Cmdline_fullscreen_window && !Cmdline_window ) {
			ChangeDisplaySettings(NULL, 0);
		}
#endif
	}
}

void gr_opengl_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{
//	mprintf(("gr_opengl_fog_set(%d,%d,%d,%d,%f,%f)\n",fog_mode,r,g,b,fog_near,fog_far));

	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));
	
	if (fog_mode == GR_FOGMODE_NONE) {
		if ( GL_state.Fog() ) {
			GL_state.Fog(GL_FALSE);
		}

		gr_screen.current_fog_mode = fog_mode;
		
		return;
	}
	
  	if (gr_screen.current_fog_mode != fog_mode) {
	  	if (OGL_fogmode == 3) {
			glFogf(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
			glFogf(GL_FOG_COORDINATE_SOURCE, GL_FRAGMENT_DEPTH);
		}
		// Um.. this is not the correct way to fog in software, probably doesn't matter though
		else if ( (OGL_fogmode == 2) && Cmdline_nohtl ) {
			glFogf(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
			fog_near *= fog_near;		// it's faster this way
			fog_far *= fog_far;		
		} else {
			glFogf(GL_FOG_COORDINATE_SOURCE, GL_FRAGMENT_DEPTH);
		}

		GL_state.Fog(GL_TRUE); 
		glFogf(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, fog_near);
		glFogf(GL_FOG_END, fog_far);

		gr_screen.current_fog_mode = fog_mode;
	}
	
	if ( (gr_screen.current_fog_color.red != r) ||
			(gr_screen.current_fog_color.green != g) ||
			(gr_screen.current_fog_color.blue != b) )
	{
		GLfloat fc[4];
		
		gr_init_color( &gr_screen.current_fog_color, r, g, b );
	
		fc[0] = (float)r/255.0f;
		fc[1] = (float)g/255.0f;
		fc[2] = (float)b/255.0f;
		fc[3] = 1.0f;
		
		glFogfv(GL_FOG_COLOR, fc);
	}

}

int gr_opengl_set_cull(int cull)
{
	GLboolean enabled = GL_FALSE;

	if (cull) {
		enabled = GL_state.CullFace(GL_TRUE);
		GL_state.FrontFaceValue(GL_CCW);
		GL_state.CullFaceValue(GL_BACK);
	} else {
		enabled = GL_state.CullFace(GL_FALSE);
	}

	return (enabled) ? 1 : 0;
}

void gr_opengl_set_clear_color(int r, int g, int b)
{
	gr_init_color(&gr_screen.current_clear_color, r, g, b);
}

int gr_opengl_zbuffer_get()
{
	if ( !gr_global_zbuffering ) {
		return GR_ZBUFF_NONE;
	}

	return gr_zbuffering_mode;
}

int gr_opengl_zbuffer_set(int mode)
{
	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if (gr_zbuffering_mode == GR_ZBUFF_NONE) {
		gr_zbuffering = 0;
		GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);
	} else if ( gr_zbuffering_mode == GR_ZBUFF_READ ) {
		gr_zbuffering = 1;
		GL_state.SetZbufferType(ZBUFFER_TYPE_READ);
	} else {
		gr_zbuffering = 1;
		GL_state.SetZbufferType(ZBUFFER_TYPE_FULL);
	}

	return tmp;
}

void gr_opengl_zbuffer_clear(int mode)
{
	if (mode) {
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;

		GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
		GL_state.SetZbufferType(ZBUFFER_TYPE_FULL);

		glClear(GL_DEPTH_BUFFER_BIT);
	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;

		GL_state.DepthTest(GL_FALSE);
	}
}

// I feel dirty...
static void opengl_make_gamma_ramp(float gamma, ushort *ramp)
{
	ushort x, y;
	ushort base_ramp[256];

	Assert( ramp != NULL );

	// generate the base ramp values first off

	// if no gamma set then just do this quickly
	if (gamma <= 0.0f) {
		memset( ramp, 0, 3 * 256 * sizeof(ushort) );
		return;
	}
	// identity gamma, avoid all of the math
	else if ( (gamma == 1.0f) || (GL_original_gamma_ramp == NULL) ) {
		if (GL_original_gamma_ramp != NULL) {
			memcpy( ramp, GL_original_gamma_ramp, 3 * 256 * sizeof(ushort) );
		}
		// set identity if no original ramp
		else {
			for (x = 0; x < 256; x++) {
				ramp[x]	= (x << 8) | x;
				ramp[x + 256] = (x << 8) | x;
				ramp[x + 512] = (x << 8) | x;
			}
		}

		return;
	}
	// for everything else we need to actually figure it up
	else {
		double g = 1.0 / (double)gamma;
		double val;

		Assert( GL_original_gamma_ramp != NULL );

		for (x = 0; x < 256; x++) {
			val = (pow(x/255.0, g) * 65535.0 + 0.5);
			CLAMP( val, 0, 65535 );

			base_ramp[x] = (ushort)val;
		}

		for (y = 0; y < 3; y++) {
			for (x = 0; x < 256; x++) {
				val = (base_ramp[x] * 2) - GL_original_gamma_ramp[x + y * 256];
				CLAMP( val, 0, 65535 );

				ramp[x + y * 256] = (ushort)val;
			}
		}
	}
}

void gr_opengl_set_gamma(float gamma)
{
	ushort *gamma_ramp = NULL;

	Gr_gamma = gamma;
	Gr_gamma_int = int (Gr_gamma*10);

	// new way - but not while running FRED
	if (!Fred_running && !Cmdline_no_set_gamma) {
		gamma_ramp = (ushort*) vm_malloc_q( 3 * 256 * sizeof(ushort) );

		if (gamma_ramp == NULL) {
			Int3();
			return;
		}

		memset( gamma_ramp, 0, 3 * 256 * sizeof(ushort) );

		// Create the Gamma lookup table
		opengl_make_gamma_ramp(gamma, gamma_ramp);

#ifdef _WIN32
		SetDeviceGammaRamp( GL_device_context, gamma_ramp );
#else
		SDL_SetGammaRamp( gamma_ramp, (gamma_ramp+256), (gamma_ramp+512) );
#endif

		vm_free(gamma_ramp);
	}
}

void gr_opengl_get_region(int front, int w, int h, ubyte *data)
{

//	if (front) {
//		glReadBuffer(GL_FRONT);
//	} else {
		glReadBuffer(GL_BACK);
//	}

	GL_state.SetTextureSource(TEXTURE_SOURCE_NO_FILTERING);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);
	
	if (gr_screen.bits_per_pixel == 16) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, data);
	} else if (gr_screen.bits_per_pixel == 32) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	}


}

void opengl_save_mouse_area(int x, int y, int w, int h)
{
	int cursor_size;

	GL_CHECK_FOR_ERRORS("start of save_mouse_area()");

	// lazy - taylor
	cursor_size = (Gr_cursor_size * Gr_cursor_size);

	// no reason to be bigger than the cursor, should never be smaller
	if (w != Gr_cursor_size)
		w = Gr_cursor_size;
	if (h != Gr_cursor_size)
		h = Gr_cursor_size;

	GL_mouse_saved_x1 = x;
	GL_mouse_saved_y1 = y;
	GL_mouse_saved_x2 = x+w-1;
	GL_mouse_saved_y2 = y+h-1;

	CLAMP(GL_mouse_saved_x1, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(GL_mouse_saved_x2, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(GL_mouse_saved_y1, gr_screen.clip_top, gr_screen.clip_bottom );
	CLAMP(GL_mouse_saved_y2, gr_screen.clip_top, gr_screen.clip_bottom );

	GL_state.SetTextureSource(TEXTURE_SOURCE_NO_FILTERING);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	if ( Use_PBOs ) {
		// since this is used a lot, and is pretty small in size, we just create it once and leave it until exit
		if (!GL_cursor_pbo) {
			vglGenBuffersARB(1, &GL_cursor_pbo);
			vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_cursor_pbo);
			vglBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, cursor_size * 4, NULL, GL_STATIC_READ);
		}

		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_cursor_pbo);
		glReadBuffer(GL_BACK);
		glReadPixels(x, gr_screen.max_h-y-1-h, w, h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
	} else {
		// this should really only have to be malloc'd once
		if (GL_saved_mouse_data == NULL)
			GL_saved_mouse_data = (ubyte*)vm_malloc_q(cursor_size * 4);

		if (GL_saved_mouse_data == NULL)
			return;

		glReadBuffer(GL_BACK);
		glReadPixels(x, gr_screen.max_h-y-1-h, w, h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, GL_saved_mouse_data);
	}

	GL_CHECK_FOR_ERRORS("end of save_mouse_area()");

	GL_mouse_saved = 1;
}

int gr_opengl_save_screen()
{
	int i;
	ubyte *sptr = NULL, *dptr = NULL;
	ubyte *opengl_screen_tmp = NULL;
	int width_times_pixel, mouse_times_pixel;

	gr_opengl_reset_clip();

	if (GL_saved_screen || GL_screen_pbo) {
		// already have a screen saved so just bail...
		return -1;
	}

	GL_saved_screen = (ubyte*)vm_malloc_q( gr_screen.max_w * gr_screen.max_h * 4 );

	if (!GL_saved_screen) {
		mprintf(( "Couldn't get memory for saved screen!\n" ));
 		return -1;
	}

	GLboolean save_state = GL_state.DepthTest(GL_FALSE);
	glReadBuffer(GL_FRONT_LEFT);

	if ( Use_PBOs ) {
		GLubyte *pixels = NULL;

		vglGenBuffersARB(1, &GL_screen_pbo);

		if (!GL_screen_pbo) {
			if (GL_saved_screen) {
				vm_free(GL_saved_screen);
				GL_saved_screen = NULL;
			}

			return -1;
		}

		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_screen_pbo);
		vglBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, gr_screen.max_w * gr_screen.max_h * 4, NULL, GL_STATIC_READ);

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		pixels = (GLubyte*)vglMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);

		width_times_pixel = (gr_screen.max_w * 4);
		mouse_times_pixel = (Gr_cursor_size * 4);

		sptr = (ubyte *)pixels;
		dptr = (ubyte *)&GL_saved_screen[gr_screen.max_w * gr_screen.max_h * 4];

		for (i = 0; i < gr_screen.max_h; i++) {
			dptr -= width_times_pixel;
			memcpy(dptr, sptr, width_times_pixel);
			sptr += width_times_pixel;
		}

		vglUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

		if (GL_mouse_saved && GL_cursor_pbo) {
			vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_cursor_pbo);

			pixels = (GLubyte*)vglMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);

			sptr = (ubyte *)pixels;
			dptr = (ubyte *)&GL_saved_screen[(GL_mouse_saved_x1 + GL_mouse_saved_y2 * gr_screen.max_w) * 4];

			for (i = 0; i < Gr_cursor_size; i++) {
				memcpy(dptr, sptr, mouse_times_pixel);
				sptr += mouse_times_pixel;
				dptr -= width_times_pixel;
			}

			vglUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
			vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
		}

		vglDeleteBuffersARB(1, &GL_screen_pbo);
		GL_screen_pbo = 0;

		GL_saved_screen_id = bm_create(32, gr_screen.max_w, gr_screen.max_h, GL_saved_screen, 0);
	} else {
		opengl_screen_tmp = (ubyte*)vm_malloc_q( gr_screen.max_w * gr_screen.max_h * 4 );

		if (!opengl_screen_tmp) {
			if (GL_saved_screen) {
				vm_free(GL_saved_screen);
				GL_saved_screen = NULL;
			}

			mprintf(( "Couldn't get memory for temporary saved screen!\n" ));
			GL_state.DepthTest(save_state);
	 		return -1;
	 	}

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, opengl_screen_tmp);

		sptr = (ubyte *)&opengl_screen_tmp[gr_screen.max_w * gr_screen.max_h * 4];
		dptr = (ubyte *)GL_saved_screen;

		width_times_pixel = (gr_screen.max_w * 4);
		mouse_times_pixel = (Gr_cursor_size * 4);

		for (i = 0; i < gr_screen.max_h; i++) {
			sptr -= width_times_pixel;
			memcpy(dptr, sptr, width_times_pixel);
			dptr += width_times_pixel;
		}

		vm_free(opengl_screen_tmp);

		if (GL_mouse_saved && GL_saved_mouse_data) {
			sptr = (ubyte *)GL_saved_mouse_data;
			dptr = (ubyte *)&GL_saved_screen[(GL_mouse_saved_x1 + GL_mouse_saved_y2 * gr_screen.max_w) * 4];

			for (i = 0; i < Gr_cursor_size; i++) {
				memcpy(dptr, sptr, mouse_times_pixel);
				sptr += mouse_times_pixel;
				dptr -= width_times_pixel;
			}
		}

		GL_saved_screen_id = bm_create(32, gr_screen.max_w, gr_screen.max_h, GL_saved_screen, 0);
	}

	GL_state.DepthTest(save_state);

	return GL_saved_screen_id;
}

void gr_opengl_restore_screen(int bmp_id)
{
	gr_reset_clip();

	if ( !GL_saved_screen ) {
		gr_clear();
		return;
	}

	Assert( (bmp_id < 0) || (bmp_id == GL_saved_screen_id) );

	if (GL_saved_screen_id < 0)
		return;

	gr_set_bitmap(GL_saved_screen_id);
	gr_bitmap(0, 0, false);	// don't scale here since we already have real screen size
}

void gr_opengl_free_screen(int bmp_id)
{
	if (!GL_saved_screen)
		return;

	vm_free(GL_saved_screen);
	GL_saved_screen = NULL;

	Assert( (bmp_id < 0) || (bmp_id == GL_saved_screen_id) );

	if (GL_saved_screen_id < 0)
		return;

	bm_release(GL_saved_screen_id);
	GL_saved_screen_id = -1;
}

static void opengl_flush_frame_dump()
{
	char filename[MAX_FILENAME_LEN];

	Assert( GL_dump_buffer != NULL);

	for (int i = 0; i < GL_dump_frame_count; i++) {
		sprintf(filename, NOX("frm%04d.tga"), GL_dump_frame_number );
		GL_dump_frame_number++;

		CFILE *f = cfopen(filename, "wb", CFILE_NORMAL, CF_TYPE_DATA);

		// Write the TGA header
		cfwrite_ubyte( 0, f );	//	IDLength;
		cfwrite_ubyte( 0, f );	//	ColorMapType;
		cfwrite_ubyte( 2, f );	//	ImageType;		// 2 = 24bpp, uncompressed, 10=24bpp rle compressed
		cfwrite_ushort( 0, f );	// CMapStart;
		cfwrite_ushort( 0, f );	//	CMapLength;
		cfwrite_ubyte( 0, f );	// CMapDepth;
		cfwrite_ushort( 0, f );	//	XOffset;
		cfwrite_ushort( 0, f );	//	YOffset;
		cfwrite_ushort( (ushort)gr_screen.max_w, f );	//	Width;
		cfwrite_ushort( (ushort)gr_screen.max_h, f );	//	Height;
		cfwrite_ubyte( 24, f );	//PixelDepth;
		cfwrite_ubyte( 0, f );	//ImageDesc;

		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_BGR_EXT, GL_UNSIGNED_BYTE, GL_dump_buffer);

		// save the data out
		cfwrite( GL_dump_buffer, GL_dump_frame_size, 1, f );

		cfclose(f);

	}

	GL_dump_frame_count = 0;
}

void gr_opengl_dump_frame_start(int first_frame, int frames_between_dumps)
{
	if ( GL_dump_frames )	{
		Int3();		//  We're already dumping frames.  See John.
		return;
	}

	GL_dump_frames = 1;
	GL_dump_frame_number = first_frame;
	GL_dump_frame_count = 0;
	GL_dump_frame_count_max = frames_between_dumps; // only works if it's 1
	GL_dump_frame_size = gr_screen.max_w * gr_screen.max_h * 3;
	
	if ( !GL_dump_buffer ) {
		int size = GL_dump_frame_count_max * GL_dump_frame_size;

		GL_dump_buffer = (ubyte *)vm_malloc(size);

		if ( !GL_dump_buffer )	{
			Error(LOCATION, "Unable to malloc %d bytes for dump buffer", size );
		}
	}
}

void gr_opengl_dump_frame_stop()
{
	if ( !GL_dump_frames )	{
		Int3();		//  We're not dumping frames.  See John.
		return;
	}	

	// dump any remaining frames
	opengl_flush_frame_dump();
	
	GL_dump_frames = 0;

	if ( GL_dump_buffer )	{
		vm_free(GL_dump_buffer);
		GL_dump_buffer = NULL;
	}
}

void gr_opengl_dump_frame()
{
	GL_dump_frame_count++;

	if ( GL_dump_frame_count == GL_dump_frame_count_max ) {
		opengl_flush_frame_dump();
	}
}

//fill mode, solid/wire frame
void gr_opengl_set_fill_mode(int mode)
{
	if (mode == GR_FILL_MODE_SOLID) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		return;
	}

	if (mode == GR_FILL_MODE_WIRE) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		return;
	}

	// default setting
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void gr_opengl_zbias(int bias)
{
	if (bias) {
		GL_state.PolygonOffsetFill(GL_TRUE);
		glPolygonOffset(0.0, -i2fl(bias));
	} else {
		GL_state.PolygonOffsetFill(GL_FALSE);
	}
}

void gr_opengl_push_texture_matrix(int unit)
{
	GLint current_matrix;

	if (unit > GL_supported_texture_units)
		return;

	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	vglActiveTextureARB(GL_TEXTURE0_ARB+unit);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	glMatrixMode(current_matrix);
}

void gr_opengl_pop_texture_matrix(int unit)
{
	GLint current_matrix;

	if (unit > GL_supported_texture_units)
		return;

	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	vglActiveTextureARB(GL_TEXTURE0_ARB+unit);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(current_matrix);
}

void gr_opengl_translate_texture_matrix(int unit, vec3d *shift)
{
	GLint current_matrix;

	if (unit > GL_supported_texture_units) {
		/*tex_shift=*shift;*/
		return;
	}

	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	vglActiveTextureARB(GL_TEXTURE0_ARB+unit);

	glMatrixMode(GL_TEXTURE);
	glTranslated(shift->xyz.x, shift->xyz.y, shift->xyz.z);

	glMatrixMode(current_matrix);

//	tex_shift=vmd_zero_vector;
}

void gr_opengl_setup_background_fog(bool set)
{
	if (Cmdline_nohtl) {
		return;
	}
}

void gr_opengl_set_line_width(float width)
{
	glLineWidth(width);
}

// Returns the human readable error string if there is an error or NULL if not
const char *opengl_error_string()
{
	GLenum error = GL_NO_ERROR;

	error = glGetError();

	if ( error != GL_NO_ERROR ) {
		return (const char *)gluErrorString(error);
	}

	return NULL;
}

int opengl_check_for_errors(char *err_at)
{
	const char *error_str = NULL;
	int num_errors = 0;

	error_str = opengl_error_string();

	if (error_str) {
		if (err_at != NULL) {
			nprintf(("OpenGL", "OpenGL Error from %s: %s\n", err_at, error_str));
		} else {
			nprintf(("OpenGL", "OpenGL Error: %s\n", error_str));
		}

		num_errors++;
	}

	return num_errors;
}

void opengl_set_vsync(int status)
{
	if ( (status < 0) || (status > 1) ) {
		Int3();
		return;
	}

#if defined(__APPLE__)
	// GLInt on 10.6 is an actual int now, instead of a long
	// This will need further testing once Snow Leopard 10.6 goes RTM
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, (GLint*)&status);
#elif defined(_WIN32)
	vwglSwapIntervalEXT(status);
#else
	// NOTE: this may not work well with the closed NVIDIA drivers since those use the
	//       special "__GL_SYNC_TO_VBLANK" environment variable to manage sync
	vglXSwapIntervalSGI(status);
#endif

	GL_CHECK_FOR_ERRORS("end of set_vsync()");
}

void opengl_setup_viewport()
{
	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (GL_rendering_to_framebuffer) {
		glOrtho(0, gr_screen.max_w, 0, gr_screen.max_h, -1.0, 1.0);
	} else {
		glOrtho(0, gr_screen.max_w, gr_screen.max_h, 0, -1.0, 1.0);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// NOTE: This should only ever be called through os_cleanup(), or when switching video APIs
void gr_opengl_shutdown()
{
	if (GL_cursor_pbo) {
		vglDeleteBuffersARB(1, &GL_cursor_pbo);
		GL_cursor_pbo = 0;
	}

	if (GL_saved_mouse_data != NULL) {
		vm_free(GL_saved_mouse_data);
		GL_saved_mouse_data = NULL;
	}

	opengl_tcache_shutdown();
	opengl_light_shutdown();
	opengl_tnl_shutdown();
	opengl_scene_texture_shutdown();
	opengl_post_process_shutdown();
	opengl_shader_shutdown();

	GL_initted = false;

#ifdef _WIN32
	// restore original gamma settings
	if (GL_original_gamma_ramp != NULL) {
		SetDeviceGammaRamp( GL_device_context, GL_original_gamma_ramp );
	}

	// swap out our window mode and un-jail the cursor
	ShowWindow((HWND)os_get_window(), SW_HIDE);
	ClipCursor(NULL);
	ChangeDisplaySettings( NULL, 0 );
#else
	if (GL_original_gamma_ramp != NULL) {
		SDL_SetGammaRamp( GL_original_gamma_ramp, (GL_original_gamma_ramp+256), (GL_original_gamma_ramp+512) );
	}
#endif

	if (GL_original_gamma_ramp != NULL) {
		vm_free(GL_original_gamma_ramp);
		GL_original_gamma_ramp = NULL;
	}

#ifdef _WIN32
	wglMakeCurrent(NULL, NULL);

	if (GL_render_context) {
		wglDeleteContext(GL_render_context);
		GL_render_context = NULL;
	}

	GL_device_context = NULL;
#endif
}

// NOTE: This should only ever be called through atexit()!!!
void opengl_close()
{
//	if ( !GL_initted )
//		return;
}

int opengl_init_display_device()
{
	int bpp = gr_screen.bits_per_pixel;

	if ( (bpp != 16) && (bpp != 32) ) {
		Int3();
		return 1;
	}


	// screen format
	switch (bpp) {
		case 16: {
			Gr_red.bits = 5;
			Gr_red.shift = 11;
			Gr_red.scale = 8;
			Gr_red.mask = 0xF800;

			Gr_green.bits = 6;
			Gr_green.shift = 5;
			Gr_green.scale = 4;
			Gr_green.mask = 0x7E0;

			Gr_blue.bits = 5;
			Gr_blue.shift = 0;
			Gr_blue.scale = 8;
			Gr_blue.mask = 0x1F;		

			break;
		}

		case 32: {
			Gr_red.bits = 8;
			Gr_red.shift = 16;
			Gr_red.scale = 1;
			Gr_red.mask = 0xff0000;

			Gr_green.bits = 8;
			Gr_green.shift = 8;
			Gr_green.scale = 1;
			Gr_green.mask = 0x00ff00;

			Gr_blue.bits = 8;
			Gr_blue.shift = 0;
			Gr_blue.scale = 1;
			Gr_blue.mask = 0x0000ff;

			Gr_alpha.bits = 8;
			Gr_alpha.shift = 24;
			Gr_alpha.mask = 0xff000000;
			Gr_alpha.scale = 1;

			break;
		}
	}

	// texture format
	Gr_t_red.bits = 5;
	Gr_t_red.mask = 0x7c00;
	Gr_t_red.shift = 10;
	Gr_t_red.scale = 8;
	
	Gr_t_green.bits = 5;
	Gr_t_green.mask = 0x03e0;
	Gr_t_green.shift = 5;
	Gr_t_green.scale = 8;
	
	Gr_t_blue.bits = 5;
	Gr_t_blue.mask = 0x001f;
	Gr_t_blue.shift = 0;
	Gr_t_blue.scale = 8;
	
	Gr_t_alpha.bits = 1;
	Gr_t_alpha.mask = 0x8000;
	Gr_t_alpha.scale = 255;
	Gr_t_alpha.shift = 15;

	// alpha-texture format
	Gr_ta_red.bits = 4;
	Gr_ta_red.mask = 0x0f00;
	Gr_ta_red.shift = 8;
	Gr_ta_red.scale = 17;
	
	Gr_ta_green.bits = 4;
	Gr_ta_green.mask = 0x00f0;
	Gr_ta_green.shift = 4;
	Gr_ta_green.scale = 17;
	
	Gr_ta_blue.bits = 4;
	Gr_ta_blue.mask = 0x000f;
	Gr_ta_blue.shift = 0;
	Gr_ta_blue.scale = 17;
	
	Gr_ta_alpha.bits = 4;
	Gr_ta_alpha.mask = 0xf000;
	Gr_ta_alpha.shift = 12;
	Gr_ta_alpha.scale = 17;

	// allocate storage for original gamma settings
	if ( !Cmdline_no_set_gamma && (GL_original_gamma_ramp == NULL) ) {
		GL_original_gamma_ramp = (ushort*) vm_malloc_q( 3 * 256 * sizeof(ushort) );

		if (GL_original_gamma_ramp == NULL) {
			mprintf(("  Unable to allocate memory for gamma ramp!  Disabling...\n"));
			Cmdline_no_set_gamma = 1;
		} else {
			// assume identity ramp by default, to be overwritten by true ramp later
			for (ushort x = 0; x < 256; x++) {
				GL_original_gamma_ramp[x] = GL_original_gamma_ramp[x + 256] = GL_original_gamma_ramp[x + 512] = (x << 8) | x;
			}
		}
	}


	// now init the display device
#ifdef _WIN32
	int PixelFormat;
	HWND wnd = 0;
	PIXELFORMATDESCRIPTOR pfd_test;

	mprintf(("  Initializing WGL...\n"));

	memset(&GL_pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	memset(&pfd_test, 0, sizeof(PIXELFORMATDESCRIPTOR));

	GL_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	GL_pfd.nVersion = 1;
	GL_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	GL_pfd.iPixelType = PFD_TYPE_RGBA;
	GL_pfd.cColorBits = (ubyte)bpp;
	GL_pfd.cRedBits = (ubyte)Gr_red.bits;
	GL_pfd.cGreenBits = (ubyte)Gr_green.bits;
	GL_pfd.cBlueBits = (ubyte)Gr_blue.bits;
	GL_pfd.cAlphaBits = (bpp == 32) ? (ubyte)Gr_alpha.bits : 0;
	GL_pfd.cDepthBits = (bpp == 32) ? 24 : 16;


	wnd = (HWND)os_get_window();

	Assert( wnd != NULL );

	extern uint os_get_dc();
	GL_device_context = (HDC)os_get_dc();

	if ( !GL_device_context ) {
		MessageBox(wnd, "Unable to get device context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	PixelFormat = ChoosePixelFormat(GL_device_context, &GL_pfd);

	if ( !PixelFormat ) {
		MessageBox(wnd, "Unable to choose pixel format for OpenGL W32!","error", MB_ICONERROR | MB_OK);
		return 1;
	} else {
		DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd_test);

		// make sure that we are hardware accelerated and not using the generic implementation
		if ( !Fred_running && (pfd_test.dwFlags & PFD_GENERIC_FORMAT) && !(pfd_test.dwFlags & PFD_GENERIC_ACCELERATED) ) {
			Assert( bpp == 32 );

			// if we failed at 32-bit then we are probably a 16-bit desktop, so try and init a 16-bit visual instead
			GL_pfd.cAlphaBits = 0;
			GL_pfd.cDepthBits = 16;
			// NOTE: the bit values for colors should get updated automatically by ChoosePixelFormat()

			PixelFormat = ChoosePixelFormat(GL_device_context, &GL_pfd);

			if (!PixelFormat) {
				MessageBox(wnd, "Unable to choose pixel format for OpenGL W32!","error", MB_ICONERROR | MB_OK);
				return 1;
			}

			// double-check that we are correct now
			DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd_test);

			if ( (pfd_test.dwFlags & PFD_GENERIC_FORMAT) && !(pfd_test.dwFlags & PFD_GENERIC_ACCELERATED) ) {
				MessageBox(wnd, "Unable to get proper pixel format for OpenGL W32!", "Error", MB_ICONERROR | MB_OK);
				return 1;
			}
		}
	}

	if ( !SetPixelFormat(GL_device_context, PixelFormat, &GL_pfd) ) {
		MessageBox(wnd, "Unable to set pixel format for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	GL_render_context = wglCreateContext(GL_device_context);
	if ( !GL_render_context ) {
		MessageBox(wnd, "Unable to create rendering context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	if ( !wglMakeCurrent(GL_device_context, GL_render_context) ) {
		MessageBox(wnd, "Unable to make current thread for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	mprintf(("  Requested WGL Video values = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d\n", Gr_red.bits, Gr_green.bits, Gr_blue.bits, GL_pfd.cColorBits, (GL_pfd.dwFlags & PFD_DOUBLEBUFFER) > 0));

	// now report back as to what we ended up getting

	DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &GL_pfd);

	int r = GL_pfd.cRedBits;
	int g = GL_pfd.cGreenBits;
	int b = GL_pfd.cBlueBits;
	int depth = GL_pfd.cColorBits;
	int db = ((GL_pfd.dwFlags & PFD_DOUBLEBUFFER) > 0);

	mprintf(("  Actual WGL Video values    = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d\n", r, g, b, depth, db));

	// get the default gamma ramp so that we can restore it on close
	if (GL_original_gamma_ramp != NULL) {
		GetDeviceGammaRamp( GL_device_context, GL_original_gamma_ramp );
	}

#else

	int flags = SDL_OPENGL;
	int r = 0, g = 0, b = 0, depth = 0, db = 1;

	mprintf(("  Initializing SDL...\n"));

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		fprintf (stderr, "Couldn't init SDL: %s", SDL_GetError());
		return 1;
	}

	// grab mouse/key unless told otherwise, ignore when we are going fullscreen
	if ( (Cmdline_fullscreen_window|| Cmdline_window || os_config_read_uint(NULL, "Fullscreen", 1) == 0) && !Cmdline_no_grab ) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, Gr_red.bits);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, Gr_green.bits);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, Gr_blue.bits);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, (bpp == 32) ? 24 : 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, db);
	
	int fsaa_samples = os_config_read_uint(NULL, "OGL_AntiAliasSamples", 0);
	
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, (fsaa_samples == 0) ? 0 : 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, fsaa_samples);

	// Slight hack to make Mesa advertise S3TC support without libtxc_dxtn
	setenv("force_s3tc_enable", "true", 1);

	mprintf(("  Requested SDL Video values = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d, FSAA: %d\n", Gr_red.bits, Gr_green.bits, Gr_blue.bits, (bpp == 32) ? 24 : 16, db, fsaa_samples));

	if (SDL_SetVideoMode(gr_screen.max_w, gr_screen.max_h, bpp, flags) == NULL) {
		fprintf (stderr, "Couldn't set video mode: %s", SDL_GetError());
		return 1;
	}

	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &r);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &b);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &db);
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &fsaa_samples);

	mprintf(("  Actual SDL Video values    = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d, FSAA: %d\n", r, g, b, depth, db, fsaa_samples));

	SDL_ShowCursor(0);

	/* might as well put this here */
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	if (GL_original_gamma_ramp != NULL) {
		SDL_GetGammaRamp( GL_original_gamma_ramp, (GL_original_gamma_ramp+256), (GL_original_gamma_ramp+512) );
	}
#endif

	return 0;
}


void opengl_setup_function_pointers()
{
	// *****************************************************************************
	// NOTE: All function pointers here should have a Cmdline_nohtl check at the top
	//       if they shouldn't be run in non-HTL mode, Don't keep separate entries.

	gr_screen.gf_flip				= gr_opengl_flip;
	gr_screen.gf_set_clip			= gr_opengl_set_clip;
	gr_screen.gf_reset_clip			= gr_opengl_reset_clip;
	
	gr_screen.gf_clear				= gr_opengl_clear;
//	gr_screen.gf_bitmap				= gr_opengl_bitmap;
	gr_screen.gf_bitmap_ex			= gr_opengl_bitmap_ex;
	gr_screen.gf_aabitmap			= gr_opengl_aabitmap;
	gr_screen.gf_aabitmap_ex		= gr_opengl_aabitmap_ex;
	
//	gr_screen.gf_rect				= gr_opengl_rect;
//	gr_screen.gf_shade				= gr_opengl_shade;
	gr_screen.gf_string				= gr_opengl_string;
	gr_screen.gf_circle				= gr_opengl_circle;
	gr_screen.gf_curve				= gr_opengl_curve;

	gr_screen.gf_line				= gr_opengl_line;
	gr_screen.gf_aaline				= gr_opengl_aaline;
	gr_screen.gf_pixel				= gr_opengl_pixel;
	gr_screen.gf_scaler				= gr_opengl_scaler;
	gr_screen.gf_tmapper			= gr_opengl_tmapper;
	gr_screen.gf_render				= gr_opengl_render;
	gr_screen.gf_render_effect		= gr_opengl_render_effect;

	gr_screen.gf_gradient			= gr_opengl_gradient;

	gr_screen.gf_set_palette		= gr_opengl_set_palette;
	gr_screen.gf_print_screen		= gr_opengl_print_screen;

	gr_screen.gf_fade_in			= gr_opengl_fade_in;
	gr_screen.gf_fade_out			= gr_opengl_fade_out;
	gr_screen.gf_flash				= gr_opengl_flash;
	gr_screen.gf_flash_alpha		= gr_opengl_flash_alpha;
	
	gr_screen.gf_zbuffer_get		= gr_opengl_zbuffer_get;
	gr_screen.gf_zbuffer_set		= gr_opengl_zbuffer_set;
	gr_screen.gf_zbuffer_clear		= gr_opengl_zbuffer_clear;
	
	gr_screen.gf_save_screen		= gr_opengl_save_screen;
	gr_screen.gf_restore_screen		= gr_opengl_restore_screen;
	gr_screen.gf_free_screen		= gr_opengl_free_screen;
	
	gr_screen.gf_dump_frame_start	= gr_opengl_dump_frame_start;
	gr_screen.gf_dump_frame_stop	= gr_opengl_dump_frame_stop;
	gr_screen.gf_dump_frame			= gr_opengl_dump_frame;
	
	gr_screen.gf_set_gamma			= gr_opengl_set_gamma;

	gr_screen.gf_fog_set			= gr_opengl_fog_set;	

	// UnknownPlayer : Don't recognize this - MAY NEED DEBUGGING
	gr_screen.gf_get_region			= gr_opengl_get_region;

	// now for the bitmap functions
	gr_screen.gf_bm_free_data			= gr_opengl_bm_free_data;
	gr_screen.gf_bm_create				= gr_opengl_bm_create;
	gr_screen.gf_bm_init				= gr_opengl_bm_init;
	gr_screen.gf_bm_load				= gr_opengl_bm_load;
	gr_screen.gf_bm_page_in_start		= gr_opengl_bm_page_in_start;
	gr_screen.gf_bm_lock				= gr_opengl_bm_lock;
	gr_screen.gf_bm_make_render_target	= gr_opengl_bm_make_render_target;
	gr_screen.gf_bm_set_render_target	= gr_opengl_bm_set_render_target;

	gr_screen.gf_set_cull			= gr_opengl_set_cull;

	gr_screen.gf_cross_fade			= gr_opengl_cross_fade;

	gr_screen.gf_tcache_set			= gr_opengl_tcache_set;

	gr_screen.gf_set_clear_color	= gr_opengl_set_clear_color;

	gr_screen.gf_preload			= gr_opengl_preload;

	gr_screen.gf_push_texture_matrix		= gr_opengl_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix			= gr_opengl_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix	= gr_opengl_translate_texture_matrix;

	gr_screen.gf_set_texture_addressing	= gr_opengl_set_texture_addressing;
	gr_screen.gf_zbias					= gr_opengl_zbias;
	gr_screen.gf_set_fill_mode			= gr_opengl_set_fill_mode;
	gr_screen.gf_set_texture_panning	= gr_opengl_set_texture_panning;

	gr_screen.gf_create_buffer		= gr_opengl_create_buffer;
	gr_screen.gf_config_buffer		= gr_opengl_config_buffer;
	gr_screen.gf_pack_buffer		= gr_opengl_pack_buffer;
	gr_screen.gf_destroy_buffer		= gr_opengl_destroy_buffer;
	gr_screen.gf_render_buffer		= gr_opengl_render_buffer;
	gr_screen.gf_set_buffer			= gr_opengl_set_buffer;

	gr_screen.gf_start_instance_matrix			= gr_opengl_start_instance_matrix;
	gr_screen.gf_end_instance_matrix			= gr_opengl_end_instance_matrix;
	gr_screen.gf_start_angles_instance_matrix	= gr_opengl_start_instance_angles;

	gr_screen.gf_make_light			= gr_opengl_make_light;
	gr_screen.gf_modify_light		= gr_opengl_modify_light;
	gr_screen.gf_destroy_light		= gr_opengl_destroy_light;
	gr_screen.gf_set_light			= gr_opengl_set_light;
	gr_screen.gf_reset_lighting		= gr_opengl_reset_lighting;
	gr_screen.gf_set_ambient_light	= gr_opengl_set_ambient_light;

	gr_screen.gf_post_process_set_effect	= gr_opengl_post_process_set_effect;
	gr_screen.gf_post_process_set_defaults	= gr_opengl_post_process_set_defaults;

	gr_screen.gf_post_process_begin		= gr_opengl_post_process_begin;
	gr_screen.gf_post_process_end		= gr_opengl_post_process_end;
	gr_screen.gf_post_process_save_zbuffer	= gr_opengl_post_process_save_zbuffer;

	gr_screen.gf_scene_texture_begin = gr_opengl_scene_texture_begin;
	gr_screen.gf_scene_texture_end = gr_opengl_scene_texture_end;

	gr_screen.gf_start_clip_plane	= gr_opengl_start_clip_plane;
	gr_screen.gf_end_clip_plane		= gr_opengl_end_clip_plane;

	gr_screen.gf_lighting			= gr_opengl_set_lighting;

	gr_screen.gf_set_proj_matrix	= gr_opengl_set_projection_matrix;
	gr_screen.gf_end_proj_matrix	= gr_opengl_end_projection_matrix;

	gr_screen.gf_set_view_matrix	= gr_opengl_set_view_matrix;
	gr_screen.gf_end_view_matrix	= gr_opengl_end_view_matrix;

	gr_screen.gf_push_scale_matrix	= gr_opengl_push_scale_matrix;
	gr_screen.gf_pop_scale_matrix	= gr_opengl_pop_scale_matrix;
	gr_screen.gf_center_alpha		= gr_opengl_center_alpha;

	gr_screen.gf_setup_background_fog	= gr_opengl_setup_background_fog;

	gr_screen.gf_start_state_block	= gr_opengl_start_state_block;
	gr_screen.gf_end_state_block	= gr_opengl_end_state_block;
	gr_screen.gf_set_state_block	= gr_opengl_set_state_block;

	gr_screen.gf_draw_line_list		= gr_opengl_draw_line_list;

	gr_screen.gf_set_line_width		= gr_opengl_set_line_width;

	gr_screen.gf_line_htl			= gr_opengl_line_htl;
	gr_screen.gf_sphere_htl			= gr_opengl_sphere_htl;

	// NOTE: All function pointers here should have a Cmdline_nohtl check at the top
	//       if they shouldn't be run in non-HTL mode, Don't keep separate entries.
	// *****************************************************************************
}


bool gr_opengl_init()
{
	char *ver;
	int major = 0, minor = 0;

	if ( !GL_initted )
		atexit(opengl_close);

	if (GL_initted) {
		gr_opengl_cleanup();
		GL_initted = false;
	}

	mprintf(( "Initializing OpenGL graphics device at %ix%i with %i-bit color...\n", gr_screen.max_w, gr_screen.max_h, gr_screen.bits_per_pixel ));

	if ( opengl_init_display_device() ) {
		Error(LOCATION, "Unable to initialize display device!\n");
	}

	// version check
	ver = (char *)glGetString(GL_VERSION);
	sscanf(ver, "%d.%d", &major, &minor);

	GL_version = (major * 10) + minor;

	if (GL_version < MIN_REQUIRED_GL_VERSION) {
		Error(LOCATION, "Current GL Version of %d.%d is less than the required version of %d.%d.\nSwitch video modes or update your drivers.", major, minor, (MIN_REQUIRED_GL_VERSION / 10), (MIN_REQUIRED_GL_VERSION % 10));
	}

	GL_initted = true;

	// this MUST be done before any other gr_opengl_* or opengl_* funcion calls!!
	opengl_setup_function_pointers();

	mprintf(( "  OpenGL Vendor    : %s\n", glGetString(GL_VENDOR) ));
	mprintf(( "  OpenGL Renderer  : %s\n", glGetString(GL_RENDERER) ));
	mprintf(( "  OpenGL Version   : %s\n", ver ));
	mprintf(( "\n" ));

	if (Cmdline_fullscreen_window || Cmdline_window) {
		opengl_go_windowed();
	} else {
		opengl_go_fullscreen();
	}

	// initialize the extensions and make sure we aren't missing something that we need
	opengl_extensions_init();

	// setup the lighting stuff that will get used later
	opengl_light_init();
	
	// init state system (must come AFTER light is set up)
	GL_state.init();

	GLint max_texture_units = GL_supported_texture_units;

	if (Use_GLSL) {
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &max_texture_units);
	}

	GL_state.Texture.init(max_texture_units);

	opengl_set_texture_target();
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable();

	// ready the texture system
	opengl_tcache_init();

	extern void opengl_tnl_init();
	opengl_tnl_init();

	// setup default shaders, and shader related items
	opengl_shader_init();

	// post processing effects, after shaders are initialized
	opengl_setup_scene_textures();
	opengl_post_process_init();

	// must be called after extensions are setup
	opengl_set_vsync( !Cmdline_no_vsync );


	opengl_setup_viewport();

	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_STENCIL_BUFFER_BIT);

	glShadeModel(GL_SMOOTH);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_FOG_HINT, GL_NICEST);

	glDepthRange(0.0, 1.0);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glFlush();

	Gr_current_red = &Gr_red;
	Gr_current_blue = &Gr_blue;
	Gr_current_green = &Gr_green;
	Gr_current_alpha = &Gr_alpha;

	Mouse_hidden++;
	gr_opengl_reset_clip();
	gr_opengl_clear();
	gr_opengl_flip();
	gr_opengl_clear();
	gr_opengl_flip();
	gr_opengl_clear();
	Mouse_hidden--;


	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &GL_max_elements_vertices);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &GL_max_elements_indices);

	mprintf(( "  Max texture units: %i (%i)\n", GL_supported_texture_units, max_texture_units ));
	mprintf(( "  Max elements vertices: %i\n", GL_max_elements_vertices ));
	mprintf(( "  Max elements indices: %i\n", GL_max_elements_indices ));
	mprintf(( "  Max texture size: %ix%i\n", GL_max_texture_width, GL_max_texture_height ));

	if ( Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) ) {
		mprintf(( "  Max render buffer size: %ix%i\n", GL_max_renderbuffer_size, GL_max_renderbuffer_size ));
	}

	mprintf(( "  Can use compressed textures: %s\n", Use_compressed_textures ? NOX("YES") : NOX("NO") ));
	mprintf(( "  Texture compression available: %s\n", Texture_compression_available ? NOX("YES") : NOX("NO") ));
	mprintf(( "  Post-processing enabled: %s\n", (Cmdline_postprocess) ? "YES" : "NO"));
	mprintf(( "  Using %s texture filter.\n", (GL_mipmap_filter) ? NOX("trilinear") : NOX("bilinear") ));

	if (Use_GLSL) {
		if (Use_GLSL > 1) {
			mprintf(( "  Using GLSL for model rendering.\n" ));
		}

		mprintf(( "  OpenGL Shader Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION_ARB) ));
	}

	// This stops fred crashing if no textures are set
	gr_screen.current_bitmap = -1;

	mprintf(("... OpenGL init is complete!\n"));

    if (Cmdline_ati_color_swap)
        GL_read_format = GL_RGBA;

	return true;
}

DCF(ogl_minimize, "Minimizes opengl")
{
	if ( gr_screen.mode != GR_OPENGL ) {
		dc_printf("Command only available in OpenGL mode.\n");
		return;
	}

	if (Dc_command) {
		dc_get_arg(ARG_TRUE);

		if ( Dc_arg_type & ARG_TRUE ) {
			opengl_minimize();
		}
	}

	if (Dc_help)
		dc_printf("If set to true then the OpenGL window will minimize.\n");
}

DCF(ogl_anisotropy, "toggles anisotropic filtering")
{
	if ( gr_screen.mode != GR_OPENGL ) {
		dc_printf("Can only set anisotropic filter in OpenGL mode.\n");
		return;
	}

	if ( Dc_command && !Is_Extension_Enabled(OGL_EXT_TEXTURE_FILTER_ANISOTROPIC) ) {
		dc_printf("Error: Anisotropic filter is not settable!\n");
		return;
	}

	if ( Dc_command ) {
		dc_get_arg(ARG_INT | ARG_NONE);

		if ( Dc_arg_type & ARG_NONE ) {
			GL_anisotropy = 1.0f;
		//	opengl_set_anisotropy();
			dc_printf("Anisotropic filter value reset to default level.\n");
		}

		if ( Dc_arg_type & ARG_INT ) {
			GL_anisotropy = (GLfloat)Dc_arg_float;
		//	opengl_set_anisotropy( (float)Dc_arg_float );
		}
	}

	if ( Dc_status ) {
		dc_printf("Current anisotropic filter value is %i\n", (int)GL_anisotropy);
	}

	if (Dc_help) {
		dc_printf("Sets OpenGL anisotropic filtering level.\n");
		dc_printf("Valid values are 1 to %i, or 0 to turn off.\n", (int)opengl_get_max_anisotropy());
	}
}
