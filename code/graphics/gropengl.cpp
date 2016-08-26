




#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "ddsutils/ddsutils.h"
#include "debugconsole/console.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "graphics/paths/PathRenderer.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglbmpman.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropengllight.h"
#include "graphics/gropenglpostprocessing.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglstate.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengltnl.h"
#include "graphics/line.h"
#include "io/mouse.h"
#include "io/timer.h"
#include "math/floating.h"
#include "model/model.h"
#include "nebula/neb.h"
#include "osapi/osapi.h"
#include "osapi/osregistry.h"
#include "palman/palman.h"
#include "render/3d.h"
#include "popup/popup.h"

#ifdef HAVE_GLU_H
#include <glu.h>
#else
#include "gl/glu.h"
#endif

#if defined(_WIN32)
#include <windows.h>
#include <windowsx.h>
#include <direct.h>
#endif

#include <glad/glad.h>

// minimum GL version we can reliably support is 3.2
static const int MIN_REQUIRED_GL_VERSION = 32;

// minimum GLSL version we can reliably support is 110
static const int MIN_REQUIRED_GLSL_VERSION = 150;

int GL_version = 0;
int GLSL_version = 0;

bool GL_initted = 0;

//0==no fog
//1==linear
//2==fog coord EXT
//3==NV Radial
int OGL_fogmode = 0;

static ushort *GL_original_gamma_ramp = NULL;

int Use_PBOs = 0;

float GL_line_width = 1.0f;

static ubyte *GL_saved_screen = NULL;
static int GL_saved_screen_id = -1;
static GLuint GL_screen_pbo = 0;

float GL_alpha_threshold = 0.0f;

extern const char *Osreg_title;

extern GLfloat GL_anisotropy;

extern float FreeSpace_gamma;
void gr_opengl_set_gamma(float gamma);

extern float FreeSpace_gamma;
void gr_opengl_set_gamma(float gamma);

static int GL_fullscreen = 0;
static int GL_windowed = 0;
static int GL_minimized = 0;

static GLenum GL_read_format = GL_BGRA;

GLuint GL_vao = 0;

static std::unique_ptr<os::OpenGLContext> GL_context = nullptr;

void opengl_go_fullscreen()
{
	if (Cmdline_fullscreen_window || Cmdline_window || GL_fullscreen || Fred_running)
		return;

	gr_opengl_set_gamma(FreeSpace_gamma);

	GL_fullscreen = 1;
	GL_minimized = 0;
	GL_windowed = 0;
}

void opengl_go_windowed()
{
	if ( ( !Cmdline_fullscreen_window && !Cmdline_window ) /*|| GL_windowed*/ || Fred_running )
		return;

	GL_windowed = 1;
	GL_minimized = 0;
	GL_fullscreen = 0;
}

void opengl_minimize()
{
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
	} else {
		opengl_minimize();
	}
}

void gr_opengl_clear()
{
	float red = gr_screen.current_clear_color.red / 255.0f;
	float green = gr_screen.current_clear_color.green / 255.0f;
	float blue = gr_screen.current_clear_color.blue / 255.0f;
	float alpha = gr_screen.current_clear_color.alpha / 255.0f;

	if ( High_dynamic_range ) {
		const float SRGB_GAMMA = 2.2f;

		red = pow(red, SRGB_GAMMA);
		green = pow(green, SRGB_GAMMA);
		blue = pow(blue, SRGB_GAMMA);
	}

	glClearColor(red, green, blue, alpha);

	glClear ( GL_COLOR_BUFFER_BIT );
}

void gr_opengl_flip()
{
	if ( !GL_initted )
		return;

	gr_reset_clip();

	mouse_reset_deltas();

	if (Cmdline_gl_finish)
		glFinish();

	os::getMainViewport()->swapBuffers();

	opengl_tcache_frame();
	opengl_reset_immediate_buffer();

#ifndef NDEBUG
	int ic = opengl_check_for_errors();

	if (ic) {
		mprintf(("!!DEBUG!! OpenGL Errors this frame: %i\n", ic));
	}
#endif
}

void gr_opengl_set_clip(int x, int y, int w, int h, int resize_mode)
{
	// check for sanity of parameters
	if (x < 0) {
		x = 0;
	}

	if (y < 0) {
		y = 0;
	}

	int to_resize = (resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)));

	int max_w = ((to_resize) ? gr_screen.max_w_unscaled : gr_screen.max_w);
	int max_h = ((to_resize) ? gr_screen.max_h_unscaled : gr_screen.max_h);

	if ((gr_screen.rendering_to_texture != -1) && to_resize) {
		gr_unsize_screen_pos(&max_w, &max_h);
	}

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
		gr_resize_screen_pos(&x, &y, &w, &h, resize_mode);
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
	if(GL_rendering_to_texture) {
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

void gr_opengl_print_screen(const char *filename)
{
	char tmp[MAX_PATH_LEN];
	ubyte tga_hdr[18];
	int i;
	ushort width, height;
	GLubyte *pixels = NULL;
	GLuint pbo = 0;

	// save to a "screenshots" directory and tack on the filename
	snprintf(tmp, MAX_PATH_LEN-1, "screenshots/%s.tga", filename);
    
    _mkdir(os_get_config_path("screenshots").c_str());

	FILE *fout = fopen(os_get_config_path(tmp).c_str(), "wb");

	if (fout == NULL) {
		return;
	}

//	glReadBuffer(GL_FRONT);

	// now for the data
	if (Use_PBOs) {
		Assert( !pbo );
		glGenBuffers(1, &pbo);

		if ( !pbo ) {
			if (fout != NULL)
				fclose(fout);

			return;
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_PACK_BUFFER, (gr_screen.max_w * gr_screen.max_h * 4), NULL, GL_STATIC_READ);

		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		// map the image data so that we can save it to file
		pixels = (GLubyte*) glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	} else {
		pixels = (GLubyte*) vm_malloc(gr_screen.max_w * gr_screen.max_h * 4, memory::quiet_alloc);

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
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		pixels = NULL;
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glDeleteBuffers(1, &pbo);
	}

	// done!
	fclose(fout);

	if (pixels != NULL) {
		vm_free(pixels);
	}
}

void gr_opengl_shutdown(os::GraphicsOperations* graphicsOps)
{
	graphics::paths::PathRenderer::shutdown();

	opengl_tcache_shutdown();
	opengl_light_shutdown();
	opengl_tnl_shutdown();
	opengl_scene_texture_shutdown();
	opengl_post_process_shutdown();
	opengl_shader_shutdown();

	GL_initted = false;

	if ( GL_version >= 30 ) {
		glDeleteVertexArrays(1, &GL_vao);
		GL_vao = 0;
	}
	
	if (GL_original_gamma_ramp != NULL && os::getSDLMainWindow() != nullptr) {
		SDL_SetWindowGammaRamp( os::getSDLMainWindow(), GL_original_gamma_ramp, (GL_original_gamma_ramp+256), (GL_original_gamma_ramp+512) );
	}

	if (GL_original_gamma_ramp != NULL) {
		vm_free(GL_original_gamma_ramp);
		GL_original_gamma_ramp = NULL;
	}

	graphicsOps->makeOpenGLContextCurrent(nullptr, nullptr);
	GL_context = nullptr;
}

void gr_opengl_cleanup(os::GraphicsOperations* graphicsOps, bool closing, int minimize)
{
	if ( !GL_initted ) {
		return;
	}

	if ( !closing && !Fred_running ) {
		gr_reset_clip();
		gr_clear();
		gr_flip();
		gr_clear();
		gr_flip();
		gr_clear();
	}

	GL_initted = false;

	opengl_tcache_flush();

	opengl_minimize();

	gr_opengl_shutdown(graphicsOps);
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

	if ( is_minimum_GLSL_version() && Current_shader != NULL && Current_shader->shader == SDR_TYPE_MODEL ) {
		return;
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

int gr_opengl_set_color_buffer(int mode)
{
	GLboolean enabled = GL_FALSE;

	if ( mode ) {
		enabled = GL_state.ColorMask(GL_TRUE);
	} else {
		enabled = GL_state.ColorMask(GL_FALSE);
	}

	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	return (enabled) ? 1 : 0;
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

int gr_opengl_stencil_set(int mode)
{
	int tmp = gr_stencil_mode;

	gr_stencil_mode = mode;

	if ( mode == GR_STENCIL_READ ) {
		GL_state.StencilTest(1);
		GL_state.SetStencilType(STENCIL_TYPE_READ);
	} else if ( mode == GR_STENCIL_WRITE ) {
		GL_state.StencilTest(1);
		GL_state.SetStencilType(STENCIL_TYPE_WRITE);
	} else {
		GL_state.StencilTest(0);
		GL_state.SetStencilType(STENCIL_TYPE_NONE);
	}

	return tmp;
}

void gr_opengl_stencil_clear()
{
	glClear(GL_STENCIL_BUFFER_BIT);
}

int gr_opengl_alpha_mask_set(int mode, float alpha)
{
	if ( mode ) {
		GL_alpha_threshold = alpha;
	} else {
		GL_alpha_threshold = 0.0f;
	}

	if ( is_minimum_GLSL_version() ) { // alpha masking is deprecated
		return mode;
	}

	if ( mode ) {
		GL_state.AlphaTest(GL_TRUE);
		GL_state.AlphaFunc(GL_GREATER, alpha);
	} else {
		GL_state.AlphaTest(GL_FALSE);
		GL_state.AlphaFunc(GL_ALWAYS, 1.0f);
	}

	return mode;
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
	if (!Fred_running && !Cmdline_no_set_gamma && os::getSDLMainWindow() != nullptr) {
		gamma_ramp = (ushort*) vm_malloc( 3 * 256 * sizeof(ushort), memory::quiet_alloc);

		if (gamma_ramp == NULL) {
			Int3();
			return;
		}

		memset( gamma_ramp, 0, 3 * 256 * sizeof(ushort) );

		// Create the Gamma lookup table
		opengl_make_gamma_ramp(gamma, gamma_ramp);

		SDL_SetWindowGammaRamp( os::getSDLMainWindow(), gamma_ramp, (gamma_ramp+256), (gamma_ramp+512) );

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

int gr_opengl_save_screen()
{
	int i;
	ubyte *sptr = NULL, *dptr = NULL;
	ubyte *opengl_screen_tmp = NULL;
	int width_times_pixel;

	gr_opengl_reset_clip();

	if (GL_saved_screen || GL_screen_pbo) {
		// already have a screen saved so just bail...
		return -1;
	}

	GL_saved_screen = (ubyte*)vm_malloc( gr_screen.max_w * gr_screen.max_h * 4, memory::quiet_alloc);

	if (!GL_saved_screen) {
		mprintf(( "Couldn't get memory for saved screen!\n" ));
 		return -1;
	}

	GLboolean save_state = GL_state.DepthTest(GL_FALSE);
	glReadBuffer(GL_FRONT_LEFT);

	if ( Use_PBOs ) {
		GLubyte *pixels = NULL;

		glGenBuffers(1, &GL_screen_pbo);

		if (!GL_screen_pbo) {
			if (GL_saved_screen) {
				vm_free(GL_saved_screen);
				GL_saved_screen = NULL;
			}

			return -1;
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_screen_pbo);
		glBufferData(GL_PIXEL_PACK_BUFFER, gr_screen.max_w * gr_screen.max_h * 4, NULL, GL_STATIC_READ);

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		pixels = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

		width_times_pixel = (gr_screen.max_w * 4);

		sptr = (ubyte *)pixels;
		dptr = (ubyte *)&GL_saved_screen[gr_screen.max_w * gr_screen.max_h * 4];

		for (i = 0; i < gr_screen.max_h; i++) {
			dptr -= width_times_pixel;
			memcpy(dptr, sptr, width_times_pixel);
			sptr += width_times_pixel;
		}

		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		glDeleteBuffers(1, &GL_screen_pbo);
		GL_screen_pbo = 0;

		GL_saved_screen_id = bm_create(32, gr_screen.max_w, gr_screen.max_h, GL_saved_screen, 0);
	} else {
		opengl_screen_tmp = (ubyte*)vm_malloc( gr_screen.max_w * gr_screen.max_h * 4, memory::quiet_alloc);

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

		for (i = 0; i < gr_screen.max_h; i++) {
			sptr -= width_times_pixel;
			memcpy(dptr, sptr, width_times_pixel);
			dptr += width_times_pixel;
		}

		vm_free(opengl_screen_tmp);

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
	gr_bitmap(0, 0, GR_RESIZE_NONE);	// don't scale here since we already have real screen size
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

//fill mode, solid/wire frame
void gr_opengl_set_fill_mode(int mode)
{
	if (mode == GR_FILL_MODE_SOLID) {
		GL_state.SetPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		return;
	}

	if (mode == GR_FILL_MODE_WIRE) {
		GL_state.SetPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		return;
	}

	// default setting
	GL_state.SetPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void gr_opengl_zbias(int bias)
{
	if (bias) {
		GL_state.PolygonOffsetFill(GL_TRUE);
		if(bias < 0) {
			GL_state.SetPolygonOffset(1.0, -i2fl(bias));
		}
		else {
			GL_state.SetPolygonOffset(0.0, -i2fl(bias));
		}
	} else {
		GL_state.PolygonOffsetFill(GL_FALSE);
	}
}

void gr_opengl_push_texture_matrix(int unit)
{

}

void gr_opengl_pop_texture_matrix(int unit)
{

}

void gr_opengl_translate_texture_matrix(int unit, const vec3d *shift)
{

}

void gr_opengl_set_line_width(float width)
{
	if (width <= 1.0f) {
		GL_state.SetLineWidth(width);
	}
	GL_line_width = width;
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
#ifdef NDEBUG
	return 0;
#endif
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

	GL_context->setSwapInterval(status);

	GL_CHECK_FOR_ERRORS("end of set_vsync()");
}

void opengl_setup_viewport_fixed_pipeline()
{

}

void opengl_setup_viewport()
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_setup_viewport_fixed_pipeline();
		return;
	}

	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	GL_last_projection_matrix = GL_projection_matrix;

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (GL_rendering_to_texture) {
		opengl_create_orthographic_projection_matrix(&GL_projection_matrix, 0, i2fl(gr_screen.max_w), 0, i2fl(gr_screen.max_h), -1.0, 1.0);
	} else {
		opengl_create_orthographic_projection_matrix(&GL_projection_matrix, 0, i2fl(gr_screen.max_w), i2fl(gr_screen.max_h), 0, -1.0, 1.0);
	}
}

int opengl_init_display_device(os::GraphicsOperations* graphicsOps)
{
	int bpp = gr_screen.bits_per_pixel;

	Assertion((bpp == 16) || (bpp == 32), "Invalid bits-per-pixel value %d!", bpp);

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
		GL_original_gamma_ramp = (ushort*) vm_malloc( 3 * 256 * sizeof(ushort), memory::quiet_alloc);

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

	os::ViewPortProperties attrs;
	attrs.pixel_format.red_size = Gr_red.bits;
	attrs.pixel_format.green_size = Gr_green.bits;
	attrs.pixel_format.blue_size = Gr_blue.bits;
	attrs.pixel_format.alpha_size = (bpp == 32) ? Gr_alpha.bits : 0;
	attrs.pixel_format.depth_size = (bpp == 32) ? 24 : 16;
	attrs.pixel_format.stencil_size = (bpp == 32) ? 8 : 1;

	attrs.pixel_format.multi_samples = os_config_read_uint(NULL, "OGL_AntiAliasSamples", 0);

	attrs.enable_opengl = true;

	attrs.gl_attributes.major_version = MIN_REQUIRED_GL_VERSION / 10;
	attrs.gl_attributes.minor_version = MIN_REQUIRED_GL_VERSION % 10;

#ifndef NDEBUG
	attrs.gl_attributes.flags.set(os::OpenGLContextFlags::Debug);
#endif

	attrs.gl_attributes.profile = os::OpenGLProfile::Core;

	attrs.display = os_config_read_uint("Video", "Display", 0);
	attrs.width = (uint32_t) gr_screen.max_w;
	attrs.height = (uint32_t) gr_screen.max_h;

	attrs.title = Osreg_title;

	if (!Cmdline_window && ! Cmdline_fullscreen_window) {
		attrs.flags.set(os::ViewPortFlags::Fullscreen);
	} else if (Cmdline_fullscreen_window) {
		attrs.flags.set(os::ViewPortFlags::Borderless);
	}

	auto viewport = graphicsOps->createViewport(attrs);
	if (!viewport) {
		return 1;
	}

	const int gl_versions[] = { 41, 40, 33, 32, 31, 30, 21, 20 };

	// find the latest and greatest OpenGL context
	for (auto ver : gl_versions)
	{
		auto gl_attrs = attrs.gl_attributes;
		gl_attrs.major_version = ver / 10;
		gl_attrs.minor_version = ver % 10;

		GL_context = graphicsOps->createOpenGLContext(viewport.get(), gl_attrs);

		if (GL_context != nullptr)
		{
			break;
		}
	}

	if (GL_context == nullptr) {
		return 1;
	}

	graphicsOps->makeOpenGLContextCurrent(viewport.get(), GL_context.get());

	auto port = os::addViewport(std::move(viewport));
	os::setMainViewPort(port);

	if (GL_original_gamma_ramp != NULL && os::getSDLMainWindow() != nullptr) {
		SDL_GetWindowGammaRamp( os::getSDLMainWindow(), GL_original_gamma_ramp, (GL_original_gamma_ramp+256),
								(GL_original_gamma_ramp+512) );
	}

	return 0;
}


void opengl_setup_function_pointers()
{
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
	gr_screen.gf_unfilled_circle	= gr_opengl_unfilled_circle;
	gr_screen.gf_arc				= gr_opengl_arc;
	gr_screen.gf_curve				= gr_opengl_curve;

	gr_screen.gf_line				= gr_opengl_line;
	gr_screen.gf_aaline				= gr_opengl_aaline;
	gr_screen.gf_pixel				= gr_opengl_pixel;
	gr_screen.gf_scaler				= gr_opengl_scaler;
	gr_screen.gf_tmapper			= gr_opengl_tmapper;
	gr_screen.gf_render				= gr_opengl_render;
	gr_screen.gf_render_effect		= gr_opengl_render_effect;

	gr_screen.gf_gradient			= gr_opengl_gradient;

	gr_screen.gf_print_screen		= gr_opengl_print_screen;

	gr_screen.gf_flash				= gr_opengl_flash;
	gr_screen.gf_flash_alpha		= gr_opengl_flash_alpha;

	gr_screen.gf_zbuffer_get		= gr_opengl_zbuffer_get;
	gr_screen.gf_zbuffer_set		= gr_opengl_zbuffer_set;
	gr_screen.gf_zbuffer_clear		= gr_opengl_zbuffer_clear;

	gr_screen.gf_stencil_set		= gr_opengl_stencil_set;
	gr_screen.gf_stencil_clear		= gr_opengl_stencil_clear;

	gr_screen.gf_alpha_mask_set		= gr_opengl_alpha_mask_set;

	gr_screen.gf_save_screen		= gr_opengl_save_screen;
	gr_screen.gf_restore_screen		= gr_opengl_restore_screen;
	gr_screen.gf_free_screen		= gr_opengl_free_screen;

	gr_screen.gf_set_gamma			= gr_opengl_set_gamma;

	gr_screen.gf_fog_set			= gr_opengl_fog_set;

	// UnknownPlayer : Don't recognize this - MAY NEED DEBUGGING
	gr_screen.gf_get_region			= gr_opengl_get_region;

	// now for the bitmap functions
	gr_screen.gf_bm_free_data			= gr_opengl_bm_free_data;
	gr_screen.gf_bm_create				= gr_opengl_bm_create;
	gr_screen.gf_bm_init				= gr_opengl_bm_init;
	gr_screen.gf_bm_page_in_start		= gr_opengl_bm_page_in_start;
	gr_screen.gf_bm_data				= gr_opengl_bm_data;
	gr_screen.gf_bm_make_render_target	= gr_opengl_bm_make_render_target;
	gr_screen.gf_bm_set_render_target	= gr_opengl_bm_set_render_target;

	gr_screen.gf_set_cull			= gr_opengl_set_cull;
	gr_screen.gf_set_color_buffer	= gr_opengl_set_color_buffer;

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

	gr_screen.gf_create_vertex_buffer	= gr_opengl_create_vertex_buffer;
	gr_screen.gf_create_index_buffer	= gr_opengl_create_index_buffer;
	gr_screen.gf_delete_buffer		= gr_opengl_delete_buffer;
	gr_screen.gf_create_buffer		= gr_opengl_create_buffer;
	gr_screen.gf_config_buffer		= gr_opengl_config_buffer;
	gr_screen.gf_pack_buffer		= gr_opengl_pack_buffer;
	gr_screen.gf_destroy_buffer		= gr_opengl_destroy_buffer;
	gr_screen.gf_render_buffer		= gr_opengl_render_buffer;
	gr_screen.gf_set_buffer			= gr_opengl_set_buffer;
	gr_screen.gf_update_buffer_data		= gr_opengl_update_buffer_data;

	gr_screen.gf_update_transform_buffer	= gr_opengl_update_transform_buffer;
	gr_screen.gf_set_transform_buffer_offset	= gr_opengl_set_transform_buffer_offset;

	gr_screen.gf_render_stream_buffer		= gr_opengl_render_stream_buffer;

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
	gr_screen.gf_copy_effect_texture = gr_opengl_copy_effect_texture;

	gr_screen.gf_deferred_lighting_begin = gr_opengl_deferred_lighting_begin;
	gr_screen.gf_deferred_lighting_end = gr_opengl_deferred_lighting_end;
	gr_screen.gf_deferred_lighting_finish = gr_opengl_deferred_lighting_finish;

	gr_screen.gf_start_clip_plane	= gr_opengl_start_clip_plane;
	gr_screen.gf_end_clip_plane		= gr_opengl_end_clip_plane;

	gr_screen.gf_lighting			= gr_opengl_set_lighting;
	gr_screen.gf_set_light_factor	= gr_opengl_set_light_factor;

	gr_screen.gf_set_proj_matrix	= gr_opengl_set_projection_matrix;
	gr_screen.gf_end_proj_matrix	= gr_opengl_end_projection_matrix;

	gr_screen.gf_set_view_matrix	= gr_opengl_set_view_matrix;
	gr_screen.gf_end_view_matrix	= gr_opengl_end_view_matrix;

	gr_screen.gf_push_scale_matrix	= gr_opengl_push_scale_matrix;
	gr_screen.gf_pop_scale_matrix	= gr_opengl_pop_scale_matrix;
	gr_screen.gf_center_alpha		= gr_opengl_center_alpha;
	gr_screen.gf_set_thrust_scale	= gr_opengl_set_thrust_scale;

	gr_screen.gf_draw_line_list		= gr_opengl_draw_line_list;

	gr_screen.gf_set_line_width		= gr_opengl_set_line_width;

	gr_screen.gf_line_htl			= gr_opengl_line_htl;
	gr_screen.gf_sphere				= gr_opengl_sphere;

	gr_screen.gf_set_animated_effect = gr_opengl_shader_set_animated_effect;

	gr_screen.gf_maybe_create_shader = gr_opengl_maybe_create_shader;
	gr_screen.gf_shadow_map_start	= gr_opengl_shadow_map_start;
	gr_screen.gf_shadow_map_end		= gr_opengl_shadow_map_end;

	gr_screen.gf_update_texture = gr_opengl_update_texture;
	gr_screen.gf_get_bitmap_from_texture = gr_opengl_get_bitmap_from_texture;

	gr_screen.gf_clear_states	= gr_opengl_clear_states;

	gr_screen.gf_set_team_color		= gr_opengl_set_team_color;

	gr_screen.gf_render_model = gr_opengl_render_model;
	gr_screen.gf_render_primitives= gr_opengl_render_primitives;
	gr_screen.gf_render_primitives_immediate = gr_opengl_render_primitives_immediate;
	gr_screen.gf_render_primitives_2d = gr_opengl_render_primitives_2d;
	gr_screen.gf_render_primitives_2d_immediate = gr_opengl_render_primitives_2d_immediate;
	gr_screen.gf_render_primitives_particle	= gr_opengl_render_primitives_particle;
	gr_screen.gf_render_primitives_distortion = gr_opengl_render_primitives_distortion;

	gr_screen.gf_is_capable = gr_opengl_is_capable;

	// NOTE: All function pointers here should have a Cmdline_nohtl check at the top
	//       if they shouldn't be run in non-HTL mode, Don't keep separate entries.
	// *****************************************************************************
}

#ifndef NDEBUG
static void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
						   GLsizei length, const GLchar *message, const void *userParam) {
	const char* sourceStr;
	const char* typeStr;
	const char* severityStr;

	switch(source) {
		case GL_DEBUG_SOURCE_API_ARB:
			sourceStr = "OpenGL";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
			sourceStr = "WindowSys";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
			sourceStr = "Shader Compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
			sourceStr = "Third Party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION_ARB:
			sourceStr = "Application";
			break;
		case GL_DEBUG_SOURCE_OTHER_ARB:
			sourceStr = "Other";
			break;
		default:
			sourceStr = "Unknown";
			break;
	}

	switch(type) {
		case GL_DEBUG_TYPE_ERROR_ARB:
			typeStr = "Error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
			typeStr = "Deprecated behavior";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
			typeStr = "Undefined behavior";
			break;
		case GL_DEBUG_TYPE_PORTABILITY_ARB:
			typeStr = "Portability";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE_ARB:
			typeStr = "Performance";
			break;
		case GL_DEBUG_TYPE_OTHER_ARB:
			typeStr = "Other";
			break;
		default:
			typeStr = "Unknown";
			break;
	}

	switch(severity) {
		case GL_DEBUG_SEVERITY_HIGH_ARB:
			severityStr = "High";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM_ARB:
			severityStr = "Medium";
			break;
		case GL_DEBUG_SEVERITY_LOW_ARB:
			severityStr = "Low";
			break;
		default:
			severityStr = "Unknown";
			break;
	}

	nprintf(("OpenGL Debug", "OpenGL Debug: Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n",
		sourceStr, typeStr, id, severityStr, message));
	printf("OpenGL Debug: Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n",
		   sourceStr, typeStr, id, severityStr, message);
}

static bool hasPendingDebugMessage() {
	GLint numMsgs = 0;
	glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES_ARB, &numMsgs);

	return numMsgs > 0;
}

static bool printNextDebugMessage() {
	if (!hasPendingDebugMessage()) {
		return false;
	}

	GLint msgLen = 0;
	glGetIntegerv(GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB, &msgLen);

	SCP_vector<GLchar> msg;
	msg.resize(msgLen + 1); // Includes null character, needs to be removed later

	GLenum source;
	GLenum type;
	GLenum severity;
	GLuint id;
	GLsizei length;

	GLuint numFound = glGetDebugMessageLogARB(1, static_cast<GLsizei>(msg.size()), &source, &type, &id, &severity, &length, &msg[0]);

	if (numFound < 1) {
		return false;
	}

	debug_callback(source, type, id, severity, length, msg.data(), nullptr);

	return true;
}
#endif

static void init_extensions() {
	// if S3TC compression is found, then "GL_ARB_texture_compression" must be an extension
	Use_compressed_textures = GLAD_GL_EXT_texture_compression_s3tc;
	Texture_compression_available = true;
	// Swifty put this in, but it's not doing anything. Once he uses it, he can uncomment it.
	//int use_base_vertex = Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX);
	
	if ( !Cmdline_no_pbo ) {
		Use_PBOs = 1;
	}
	
	int ver = 0, major = 0, minor = 0;
	const char *glsl_ver = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	sscanf(glsl_ver, "%d.%d", &major, &minor);
	ver = (major * 100) + minor;

	GLSL_version = ver;

	// we require a minimum GLSL version
	if (!is_minimum_GLSL_version()) {
		Error(LOCATION,  "Current GL Shading Langauge Version of %d is less than the required version of %d. Switch video modes or update your drivers.", GLSL_version, MIN_REQUIRED_GLSL_VERSION);
	}

	// can't have this stuff without GLSL support
	if ( !is_minimum_GLSL_version() ) {
		Cmdline_normal = 0;
		Cmdline_height = 0;
		Cmdline_postprocess = 0;
		Cmdline_shadow_quality = 0;
		Cmdline_no_deferred_lighting = 1;
	}

	if ( GLSL_version < 120 ) {
		mprintf(("  No hardware support for deferred lighting. Deferred lighting will be disabled. \n"));
		Cmdline_no_deferred_lighting = 1;
		Cmdline_no_batching = true;
	}

	if (is_minimum_GLSL_version()) {
		GLint max_texture_units;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);

		// we need enough texture slots for this stuff to work

		if (max_texture_units < 6) {
			mprintf(( "Not enough texture units for height map support. We need at least 6, we found %d.\n", max_texture_units ));
			Cmdline_height = 0;
		} else if (max_texture_units < 5) {
			mprintf(( "Not enough texture units for height and normal map support. We need at least 5, we found %d.\n", max_texture_units ));
			Cmdline_normal = 0;
			Cmdline_height = 0;
		} else if (max_texture_units < 4) {
			mprintf(( "Not enough texture units found for GLSL support. We need at least 4, we found %d.\n", max_texture_units ));
			GLSL_version = 0;
		}
	}
}

bool gr_opengl_init(os::GraphicsOperations* graphicsOps)
{
	if (GL_initted) {
		gr_opengl_cleanup(graphicsOps, false);
		GL_initted = false;
	}

	mprintf(( "Initializing OpenGL graphics device at %ix%i with %i-bit color...\n",
		  gr_screen.max_w,
		  gr_screen.max_h,
		  gr_screen.bits_per_pixel ));

	if ( opengl_init_display_device(graphicsOps) ) {
		Error(LOCATION, "Unable to initialize display device!\n");
	}

	// Initialize function pointers
	if (!gladLoadGLLoader(GL_context->getLoaderFunction())) {
		Error(LOCATION, "Failed to load OpenGL!");
	}

	// version check
	GL_version = (GLVersion.major * 10) + GLVersion.minor;

	if (GL_version < MIN_REQUIRED_GL_VERSION) {
		Error(LOCATION, "Current GL Version of %d.%d is less than the "
			"required version of %d.%d.\n"
			"Switch video modes or update your drivers.",
			GLVersion.major,
			GLVersion.minor,
			(MIN_REQUIRED_GL_VERSION / 10),
			(MIN_REQUIRED_GL_VERSION % 10));
	}

	GL_initted = true;

#ifndef NDEBUG
	// Set up the debug extension if present
	if (GLAD_GL_ARB_debug_output) {
		nprintf(("OpenGL Debug", "Using OpenGL debug extension\n"));
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		GLuint unusedIds = 0;
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);

		glDebugMessageCallbackARB(debug_callback, nullptr);

		// Now print all pending log messages
		while (hasPendingDebugMessage()) {
			printNextDebugMessage();
		}
	}
#endif


	// this MUST be done before any other gr_opengl_* or
	// opengl_* function calls!!
	opengl_setup_function_pointers();

	mprintf(( "  OpenGL Vendor    : %s\n", glGetString(GL_VENDOR) ));
	mprintf(( "  OpenGL Renderer  : %s\n", glGetString(GL_RENDERER) ));
	mprintf(( "  OpenGL Version   : %s\n", glGetString(GL_VERSION) ));
	mprintf(( "\n" ));

	if (Cmdline_fullscreen_window || Cmdline_window) {
		opengl_go_windowed();
	} else {
		opengl_go_fullscreen();
	}

	init_extensions();

	// setup the lighting stuff that will get used later
	opengl_light_init();

	// init state system (must come AFTER light is set up)
	GL_state.init();

	GLint max_texture_units = GL_supported_texture_units;
	GLint max_texture_coords = GL_supported_texture_units;

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
	max_texture_coords = 1;

	// create vertex array object to make OpenGL Core happy if we can
	if ( GL_version >= 30 ) {
		glGenVertexArrays(1, &GL_vao);
		glBindVertexArray(GL_vao);
	}

	GL_state.Texture.init(max_texture_units);
	GL_state.Array.init(max_texture_coords);

	opengl_set_texture_target();
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable();

	// ready the texture system
	opengl_tcache_init();

	opengl_tnl_init();

	// setup default shaders, and shader related items
	opengl_shader_init();

	// post processing effects, after shaders are initialized
	opengl_setup_scene_textures();
	opengl_post_process_init();

	// must be called after extensions are setup
	opengl_set_vsync( !Cmdline_no_vsync );

	opengl_setup_viewport();
	vm_matrix4_set_identity(&GL_view_matrix);
	vm_matrix4_set_identity(&GL_model_view_matrix);

	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_STENCIL_BUFFER_BIT);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glDepthRange(0.0, 1.0);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glFlush();

	Gr_current_red = &Gr_red;
	Gr_current_blue = &Gr_blue;
	Gr_current_green = &Gr_green;
	Gr_current_alpha = &Gr_alpha;

	gr_opengl_reset_clip();
	gr_opengl_clear();
	gr_opengl_flip();
	gr_opengl_clear();
	gr_opengl_flip();
	gr_opengl_clear();

	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &GL_max_elements_vertices);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &GL_max_elements_indices);

	mprintf(( "  Max texture units: %i (%i)\n", GL_supported_texture_units, max_texture_units ));
	mprintf(( "  Max client texture states: %i (%i)\n", GL_supported_texture_units, max_texture_coords ));
	mprintf(( "  Max elements vertices: %i\n", GL_max_elements_vertices ));
	mprintf(( "  Max elements indices: %i\n", GL_max_elements_indices ));
	mprintf(( "  Max texture size: %ix%i\n", GL_max_texture_width, GL_max_texture_height ));

	mprintf(( "  Max render buffer size: %ix%i\n",
		  GL_max_renderbuffer_size,
		  GL_max_renderbuffer_size ));

	mprintf(( "  Can use compressed textures: %s\n", Use_compressed_textures ? NOX("YES") : NOX("NO") ));
	mprintf(( "  Texture compression available: %s\n", Texture_compression_available ? NOX("YES") : NOX("NO") ));
	mprintf(( "  Post-processing enabled: %s\n", (Cmdline_postprocess) ? "YES" : "NO"));
	mprintf(( "  Using %s texture filter.\n", (GL_mipmap_filter) ? NOX("trilinear") : NOX("bilinear") ));

	if (is_minimum_GLSL_version()) {
		mprintf(( "  OpenGL Shader Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION) ));
	}
	
	// This stops fred crashing if no textures are set
	gr_screen.current_bitmap = -1;

	mprintf(("... OpenGL init is complete!\n"));

	if (Cmdline_ati_color_swap)
		GL_read_format = GL_RGBA;

	return true;
}

bool gr_opengl_is_capable(gr_capability capability)
{
	if ( !is_minimum_GLSL_version() ) {
		return false;
	}

	if ( GL_version < 20 ) {
		return false;
	}

	switch ( capability ) {
	case CAPABILITY_ENVIRONMENT_MAP:
		return true;
	case CAPABILITY_NORMAL_MAP:
		return Cmdline_normal ? true : false;
	case CAPABILITY_HEIGHT_MAP:
		return Cmdline_height ? true : false;
	case CAPABILITY_SOFT_PARTICLES:
	case CAPABILITY_DISTORTION:
		return Cmdline_softparticles && (GLSL_version >= 120) && !Cmdline_no_fbo;
	case CAPABILITY_POST_PROCESSING:
		return Cmdline_postprocess && (GLSL_version >= 120) && !Cmdline_no_fbo;
	case CAPABILITY_DEFERRED_LIGHTING:
		return !Cmdline_no_fbo && !Cmdline_no_deferred_lighting && (GLSL_version >= 120);
	case CAPABILITY_SHADOWS:
		return GL_version >= 32;
	case CAPABILITY_BATCHED_SUBMODELS:
		return (GLSL_version >= 150);
	case CAPABILITY_POINT_PARTICLES:
		return GL_version >= 32 && !Cmdline_no_geo_sdr_effects;
	}

	return false;
}

uint opengl_data_type_size(GLenum data_type)
{
	switch ( data_type ) {
	case GL_BYTE:
		return sizeof(GLbyte);
	case GL_UNSIGNED_BYTE:
		return sizeof(GLubyte);
	case GL_SHORT:
		return sizeof(GLshort);
	case GL_UNSIGNED_SHORT:
		return sizeof(GLushort);
	case GL_INT:
		return sizeof(GLint);
	case GL_UNSIGNED_INT:
		return sizeof(GLuint);
	case GL_FLOAT:
		return sizeof(GLfloat);
	case GL_DOUBLE:
		return sizeof(GLdouble);
	}

	return 0;
}

DCF(ogl_minimize, "Minimizes opengl")
{
	bool minimize_ogl = false;

	if ( gr_screen.mode != GR_OPENGL ) {
		dc_printf("Command only available in OpenGL mode.\n");
		return;
	}

	if (dc_optional_string_either("help", "--help")) {
		dc_printf("[bool] If true is passed, then the OpenGL window will minimize.\n");
		return;
	}
	dc_stuff_boolean(&minimize_ogl);

	if (minimize_ogl) {
		opengl_minimize();
	}
}

DCF(ogl_anisotropy, "toggles anisotropic filtering")
{
	bool process = true;
	int value;

	if ( gr_screen.mode != GR_OPENGL ) {
		dc_printf("Can only set anisotropic filter in OpenGL mode.\n");
		return;
	}

	if (dc_optional_string_either("help", "--help")) {
		dc_printf("Sets OpenGL anisotropic filtering level.\n");
		dc_printf("GL_anisotropy [int]  Valid values are 0 to %i. 0 turns off anisotropic filtering.\n", (int)opengl_get_max_anisotropy());
		process = false;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Current anisotropic filter value is %i\n", (int)GL_anisotropy);
		process = false;
	}

	if (!process) {
		return;
	}

	if ( !GLAD_GL_EXT_texture_filter_anisotropic ) {
		dc_printf("Error: Anisotropic filter is not settable!\n");
		return;
	}

	if (!dc_maybe_stuff_int(&value)) {
		// No arg passed, set to default
			GL_anisotropy = 1.0f;
		//	opengl_set_anisotropy();
			dc_printf("Anisotropic filter value reset to default level.\n");
	} else {
		GL_anisotropy = (GLfloat)value;
		//	opengl_set_anisotropy( (float)Dc_arg_float );
	}
}

/**
 * Helper function to enquire whether minimum GLSL version present.
 *
 * Compares global variable set by glGetString(GL_SHADING_LANGUAGE_VERSION)
 * against compile time MIN_REQUIRED_GLSL_VERSION.
 *
 * @return true if GLSL support present is above the minimum version.
 */
bool is_minimum_GLSL_version() {
	return GLSL_version >= MIN_REQUIRED_GLSL_VERSION ? true : false;
}
