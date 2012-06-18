/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include <limits.h>

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "graphics/grstub.h"
#include "render/3d.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/font.h"
#include "graphics/grinternal.h"
#include "globalincs/systemvars.h"
#include "cmdline/cmdline.h"
#include "graphics/grbatch.h"
#include "parse/scripting.h"
#include "gamesequence/gamesequence.h"	//WMC - for scripting hooks in gr_flip()
#include "io/keycontrol.h" // m!m


#if defined(SCP_UNIX) && !defined(__APPLE__)
#if ( SDL_VERSION_ATLEAST(1, 2, 7) )
#include "SDL_cpuinfo.h"
#endif
#endif // SCP_UNIX && !__APPLE__

// Includes for different rendering systems
#include "graphics/gropengl.h"

screen gr_screen;

color_gun Gr_red, Gr_green, Gr_blue, Gr_alpha;
color_gun Gr_t_red, Gr_t_green, Gr_t_blue, Gr_t_alpha;
color_gun Gr_ta_red, Gr_ta_green, Gr_ta_blue, Gr_ta_alpha;
color_gun *Gr_current_red, *Gr_current_green, *Gr_current_blue, *Gr_current_alpha;


ubyte Gr_original_palette[768];		// The palette 
ubyte Gr_current_palette[768];
char Gr_current_palette_name[128] = NOX("none");

// cursor stuff
int Gr_cursor = -1;
int Web_cursor_bitmap = -1;
int Gr_cursor_size = 32;	// default w/h

int Gr_inited = 0;

uint Gr_signature = 0;

float Gr_gamma = 1.8f;
int Gr_gamma_int = 180;

// z-buffer stuff
int gr_zbuffering = 0;
int gr_zbuffering_mode = 0;
int gr_global_zbuffering = 0;

// Default clipping distances
const float Default_min_draw_distance = 1.0f;
const float Default_max_draw_distance = 1e10;
float Min_draw_distance = Default_min_draw_distance;
float Max_draw_distance = Default_max_draw_distance;

static int GL_cursor_nframes = 0;

// Pre-computed screen resize vars
static float Gr_resize_X = 1.0f, Gr_resize_Y = 1.0f;
static float Gr_unsize_X = 1.0f, Gr_unsize_Y = 1.0f;

float Gr_save_resize_X = 1.0f, Gr_save_resize_Y = 1.0f;
float Gr_save_unsize_X = 1.0f, Gr_save_unsize_Y = 1.0f;

void gr_set_screen_scale(int w, int h)
{
	Gr_resize_X = (float)gr_screen.max_w / (float)w;
	Gr_resize_Y = (float)gr_screen.max_h / (float)h;

	Gr_unsize_X = (float)w / (float)gr_screen.max_w;
	Gr_unsize_Y = (float)h / (float)gr_screen.max_h;
}

void gr_set_screen_scale(int w, int h, int max_w, int max_h)
{
	Gr_resize_X = (float)max_w / (float)w;
	Gr_resize_Y = (float)max_h / (float)h;

	Gr_unsize_X = (float)w / (float)max_w;
	Gr_unsize_Y = (float)h / (float)max_h;
}

void gr_reset_screen_scale()
{
	Gr_resize_X = Gr_save_resize_X;
	Gr_resize_Y = Gr_save_resize_Y;

	Gr_unsize_X = Gr_save_unsize_X;
	Gr_unsize_Y = Gr_save_unsize_Y;
}

/**
 * This function is to be called if you wish to scale GR_1024 or GR_640 x and y positions or
 * lengths in order to keep the correctly scaled to nonstandard resolutions
 *
 * @param x X value (width to be scaled), can be NULL
 * @param y Y value (height to be scaled), can be NULL
 * @return always true unless error
 */
bool gr_resize_screen_pos(int *x, int *y)
{
	if ( !gr_screen.custom_size && (gr_screen.rendering_to_texture == -1) ) {
		return false;
	}

	float xy_tmp = 0.0f;

	if ( x && (*x != 0) ) {
		xy_tmp = (*x) * Gr_resize_X;
		(*x) = fl2i(xy_tmp);
	}

	if ( y && (*y != 0) ) {
		xy_tmp = (*y) * Gr_resize_Y;
		(*y) = fl2i(xy_tmp);
	}

	return true;
}

/**
 *
 * @param x X value (width to be unscaled), can be NULL
 * @param y Y value (height to be unscaled), can be NULL
 * @return always true unless error
 */
bool gr_unsize_screen_pos(int *x, int *y)
{
	if ( !gr_screen.custom_size && (gr_screen.rendering_to_texture == -1) ) {
		return false;
	}

	float xy_tmp = 0.0f;

	if ( x && (*x != 0) ) {
		xy_tmp = (*x) * Gr_unsize_X;
		(*x) = fl2i(xy_tmp);
	}

	if ( y && (*y != 0) ) {
		xy_tmp = (*y) * Gr_unsize_Y;
		(*y) = fl2i(xy_tmp);
	}

	return true;
}

/**
 * This function is to be called if you wish to scale GR_1024 or GR_640 x and y positions or
 * lengths in order to keep the correctly scaled to nonstandard resolutions
 *
 * @param x X value (width to be scaled), can be NULL
 * @param y Y value (height to be scaled), can be NULL
 * @return always true unless error
 */
bool gr_resize_screen_posf(float *x, float *y)
{
	if ( !gr_screen.custom_size && (gr_screen.rendering_to_texture == -1) ) {
		return false;
	}

	if ( x && (*x != 0) )
		(*x) *= Gr_resize_X;

	if ( y && (*y != 0) )
		(*y) *= Gr_resize_Y;

	return true;
}

/**
 *
 * @param x X value (width to be unscaled), can be NULL
 * @param y Y value (height to be unscaled), can be NULL
 * @return always true unless error
 */
bool gr_unsize_screen_posf(float *x, float *y)
{
	if ( !gr_screen.custom_size && (gr_screen.rendering_to_texture == -1) ) {
		return false;
	}

	if ( x && (*x != 0) )
		(*x) *= Gr_unsize_X;

	if ( y && (*y != 0) )
		(*y) *= Gr_unsize_Y;

	return true;
}

void gr_close()
{
	if ( !Gr_inited ) {
		return;
	}

	palette_flush();

	switch (gr_screen.mode) {
		case GR_OPENGL:
			gr_opengl_cleanup();
			break;
	
		case GR_STUB:
			break;
	
		default:
			Int3();		// Invalid graphics mode
	}

	gr_font_close();

	Gr_inited = 0;
}

//XSTR:OFF
DCF(gr,"Changes graphics mode")
{
	int mode = gr_screen.mode;

	if ( Dc_command ) {
		dc_get_arg(ARG_STRING);
		
		if ( !strcmp( Dc_arg, "o")) {
			mode = GR_OPENGL;
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help ) {
		dc_printf( "Usage: gr mode\n" );
		dc_printf( "The options can be:\n" );
		dc_printf( "Macros:  A=software win32 window (obsolete)\n" );
		dc_printf( "         B=software directdraw fullscreen (obsolete)\n" );
		dc_printf( "         G=Glide (obsolete)\n" );
		dc_printf( "         O=OpenGL\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status ) {
		switch( gr_screen.mode ) {
		case GR_OPENGL:
			dc_printf( "OpenGl\n" );
			break;
		default:
			Int3();		// Invalid graphics mode
		}
	}
}
//XSTR:ON

/**
 * Set screen clear color
 */
DCF(clear_color, "set clear color r, g, b")
{
	ubyte r, g, b;

	dc_get_arg(ARG_UBYTE);
	r = Dc_arg_ubyte;
	dc_get_arg(ARG_UBYTE);
	g = Dc_arg_ubyte;
	dc_get_arg(ARG_UBYTE);
	b = Dc_arg_ubyte;

	// set the color
	gr_set_clear_color(r, g, b);
}

void gr_set_palette_internal( char *name, ubyte * palette, int restrict_font_to_128 )
{
	if ( palette == NULL ) {
		// Create a default palette
		int r,g,b,i;
		i = 0;

		for (r=0; r<6; r++ )
			for (g=0; g<6; g++ )
				for (b=0; b<6; b++ ) {
					Gr_current_palette[i*3+0] = (unsigned char)(r*51);
					Gr_current_palette[i*3+1] = (unsigned char)(g*51);
					Gr_current_palette[i*3+2] = (unsigned char)(b*51);
					i++;
				}
		for ( i=216;i<256; i++ ) {
			Gr_current_palette[i*3+0] = (unsigned char)((i-216)*6);
			Gr_current_palette[i*3+1] = (unsigned char)((i-216)*6);
			Gr_current_palette[i*3+2] = (unsigned char)((i-216)*6);
		}
		memmove( Gr_original_palette, Gr_current_palette, 768 );
	} else {
		memmove( Gr_original_palette, palette, 768 );
		memmove( Gr_current_palette, palette, 768 );
	}

	if ( Gr_inited ) {
		if (gr_screen.gf_set_palette) {
			(*gr_screen.gf_set_palette)(Gr_current_palette, restrict_font_to_128 );

			// Since the palette set code might shuffle the palette,
			// reload it into the source palette
			if ( palette ) {
				memmove( palette, Gr_current_palette, 768 );
			}
		}

		// Update Palette Manager tables
		memmove( gr_palette, Gr_current_palette, 768 );
		palette_update(name, restrict_font_to_128);
	}
}


void gr_set_palette( char *name, ubyte * palette, int restrict_font_to_128 )
{
	char *p;
	palette_flush();
	strcpy_s( Gr_current_palette_name, name );
	p = strchr( Gr_current_palette_name, '.' );
	if ( p ) *p = 0;
	gr_screen.signature = Gr_signature++;
	gr_set_palette_internal( name, palette, restrict_font_to_128 );
}

void gr_screen_resize(int width, int height)
{
	// this should only be called from FRED!!
	if ( !Fred_running ) {
		Int3();
		return;
	}

	gr_screen.save_max_w = gr_screen.max_w = gr_screen.max_w_unscaled = width;
	gr_screen.save_max_h = gr_screen.max_h = gr_screen.max_h_unscaled = height;

	gr_screen.offset_x = gr_screen.offset_x_unscaled = 0;
	gr_screen.offset_y = gr_screen.offset_y_unscaled = 0;

	gr_screen.clip_left = gr_screen.clip_left_unscaled = 0;
	gr_screen.clip_top = gr_screen.clip_top_unscaled = 0;
	gr_screen.clip_right = gr_screen.clip_right_unscaled = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.clip_bottom_unscaled = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.clip_width_unscaled = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.clip_height_unscaled = gr_screen.max_h;
	gr_screen.clip_aspect = i2fl(gr_screen.clip_width) / i2fl(gr_screen.clip_height);

	if (gr_screen.custom_size) {
		gr_unsize_screen_pos( &gr_screen.max_w_unscaled, &gr_screen.max_h_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled );
	}

	if (gr_screen.mode == GR_OPENGL) {
		extern void opengl_setup_viewport();
		opengl_setup_viewport();
	}
}

static bool gr_init_sub(int mode, int width, int height, int depth)
{
	int res = GR_1024;
	bool rc = false;

	memset( &gr_screen, 0, sizeof(screen) );

	if ( ((width == 640) && (height == 480)) || ((width == 1024) && (height == 768)) ) {
		gr_screen.custom_size = false;
	} else {
		gr_screen.custom_size = true;
	}

	if ( (width >= 1024) && (height >= 600) ) {
		res = GR_1024;
	} else {
		res = GR_640;
	}

	if (Fred_running) {
		gr_screen.custom_size = false;
		res = GR_640;
		mode = GR_OPENGL;
	}

	Gr_save_resize_X = Gr_resize_X = (float)width / ((res == GR_1024) ? 1024.0f : 640.0f);
	Gr_save_resize_Y = Gr_resize_Y = (float)height / ((res == GR_1024) ?  768.0f : 480.0f);

	Gr_save_unsize_X = Gr_unsize_X = ((res == GR_1024) ? 1024.0f : 640.0f) / (float)width;
	Gr_save_unsize_Y = Gr_unsize_Y = ((res == GR_1024) ?  768.0f : 480.0f) / (float)height;


	gr_screen.signature = Gr_signature++;
	gr_screen.bits_per_pixel = depth;
	gr_screen.bytes_per_pixel= depth / 8;
	gr_screen.rendering_to_texture = -1;
	gr_screen.recording_state_block = false;
	gr_screen.envmap_render_target = -1;
	gr_screen.mode = mode;
	gr_screen.res = res;
	gr_screen.aspect = 1.0f;			// Normal PC screen

	gr_screen.save_max_w = gr_screen.max_w = gr_screen.max_w_unscaled = width;
	gr_screen.save_max_h = gr_screen.max_h = gr_screen.max_h_unscaled = height;

	gr_screen.offset_x = gr_screen.offset_x_unscaled = 0;
	gr_screen.offset_y = gr_screen.offset_y_unscaled = 0;

	gr_screen.clip_left = gr_screen.clip_left_unscaled = 0;
	gr_screen.clip_top = gr_screen.clip_top_unscaled = 0;
	gr_screen.clip_right = gr_screen.clip_right_unscaled = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.clip_bottom_unscaled = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.clip_width_unscaled = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.clip_height_unscaled = gr_screen.max_h;
	gr_screen.clip_aspect = i2fl(gr_screen.clip_width) / i2fl(gr_screen.clip_height);
	gr_screen.clip_center_x = (gr_screen.clip_left + gr_screen.clip_right) * 0.5f;
	gr_screen.clip_center_y = (gr_screen.clip_top + gr_screen.clip_bottom) * 0.5f;

	if (gr_screen.custom_size) {
		gr_unsize_screen_pos( &gr_screen.max_w_unscaled, &gr_screen.max_h_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled );
	}

#ifdef WIN32
	// FRED doesn't need this
	if ( !Fred_running && !Is_standalone ) {
		// for Windows, we need to do this just before the *_init() calls
		extern void win32_create_window(int width, int height);
		win32_create_window( width, height );
	}
#endif

	switch (mode) {
		case GR_OPENGL:
			rc = gr_opengl_init();
			break;
		case GR_STUB: 
			rc = gr_stub_init();
			break;
		default:
			Int3();		// Invalid graphics mode
	}

	if ( !rc ) {
		return false;
	}

	return true;
}

bool gr_init(int d_mode, int d_width, int d_height, int d_depth)
{
	int width = 1024, height = 768, depth = 16, mode = GR_OPENGL;
	char *ptr = NULL;
	char *Default_video_settings = "OGL -(1024x768)x16 bit";

	if ( !Gr_inited ) {
		atexit(gr_close);
	}

	// If already inited, shutdown the previous graphics
	if (Gr_inited) {
		switch (gr_screen.mode) {
			case GR_OPENGL:
				gr_opengl_cleanup();
				break;
			
			case GR_STUB:
				break;
	
			default:
				Int3();		// Invalid graphics mode
		}
	}

	// We cannot continue without this, quit, but try to help the user out first
	ptr = os_config_read_string(NULL, NOX("VideocardFs2open"), NULL);

	// if we don't have a config string then construct one, using OpenGL 1024x768 16-bit as the default
	if (ptr == NULL) {
		ptr = Default_video_settings;
	}

	Assert( ptr != NULL );

	// NOTE: The "ptr+5" is to skip over the initial "????-" in the video string.
	//       If the format of that string changes you'll have to change this too!!!
	if ( sscanf(ptr+5, "(%dx%d)x%d ", &width, &height, &depth) != 3 ) {
		Error(LOCATION, "Can't understand 'VideocardFs2open' config entry!");
	}

	if (Cmdline_res != NULL) {
		int tmp_width = 0;
		int tmp_height = 0;

		if ( sscanf(Cmdline_res, "%dx%d", &tmp_width, &tmp_height) == 2 ) {
			width = tmp_width;
			height = tmp_height;
		}
	}

	if (d_mode == GR_DEFAULT) {
		// OpenGL should be default
		mode = GR_OPENGL;
	} else {
		mode = d_mode;
	}

	// see if we passed good values, and use those instead of the config settings
	if ( (d_width != GR_DEFAULT) && (d_height != GR_DEFAULT) ) {
		width = d_width;
		height = d_height;
	}

	if (d_depth != GR_DEFAULT) {
		depth = d_depth;
	}

	// check for hi-res interface files so that we can verify our width/height is correct
	bool has_sparky_hi = (cf_exists_full("2_ChoosePilot-m.pcx", CF_TYPE_ANY) && cf_exists_full("2_TechShipData-m.pcx", CF_TYPE_ANY));

	// if we don't have it then fall back to 640x480 mode instead
	if ( !has_sparky_hi ) {
		if ( (width == 1024) && (height == 768) ) {
			width = 640;
			height = 480;
		} else {
			width = 800;
			height = 600;
		}
	}

	// if we are in standalone mode then just use special defaults
	if (Is_standalone) {
		mode = GR_STUB;
		width = 640;
		height = 480;
		depth = 16;
	}

// These compiler macros will force windowed mode at the specified resolution if
// built in debug mode.  This helps if you run with the debugger active as the
// game won't be switching from fullscreen to minimized every time you hit a breakpoint or
// warning message.
#ifdef _DEBUG
#ifdef _FORCE_DEBUG_WIDESCREEN
	width = 1280;
	height = 800;
	depth = 32;
	Cmdline_window = 1;
#elif defined(_FORCE_DEBUG_1024)
	width = 1024;
	height = 768;
	depth = 32;
	Cmdline_window = 1;
#elif defined(_FORCE_DEBUG_640)
	width = 640;
	height = 480;
	depth = 32;
	Cmdline_window = 1;
#endif
#endif

	// now try to actually init everything...
	if ( gr_init_sub(mode, width, height, depth) == false ) {
		return false;
	}

	gr_set_palette_internal(Gr_current_palette_name, NULL, 0);

	bm_init();

	if (Gr_cursor < 0) {
		int w, h;

		Gr_cursor = bm_load( "cursor" );

		if (Gr_cursor >= 0) {
			// get cursor size, so that we can be sure to account for the full thing
			// in later cursor hiding code
			bm_get_info(Gr_cursor, &w, &h);
			Gr_cursor_size = MAX(w, h);

			if (Gr_cursor_size <= 0) {
				Int3();
				Gr_cursor_size = 32;
			}
		}
	}

	// load the web pointer cursor bitmap
	if (Web_cursor_bitmap < 0) {
		//if it still hasn't loaded then this usually means that the executable isn't in the same directory as the main fs2 install
		if ( (Web_cursor_bitmap = bm_load_animation("cursorweb")) < 0 ) {
			Error(LOCATION, "\nWeb cursor bitmap not found.  This is most likely due to one of three reasons:\n"
				"\t1) You're running FreeSpace Open from somewhere other than your FreeSpace 2 folder;\n"
				"\t2) You've somehow corrupted your FreeSpace 2 installation, e.g. by modifying or removing the retail VP files;\n"
				"\t3) You haven't installed FreeSpace 2 at all.  (Note that installing FreeSpace Open does NOT remove the need for a FreeSpace 2 installation.)\n"
				"Number 1 can be fixed by simply moving the FreeSpace Open executable file to the FreeSpace 2 folder.  Numbers 2 and 3 can be fixed by installing or reinstalling FreeSpace 2.\n");
		}
	}

	mprintf(("GRAPHICS: Initializing default colors...\n"));

	gr_set_color(0,0,0);
	gr_set_clear_color(0, 0, 0);

	gr_set_shader(NULL);

	os_set_title(Osreg_title);

	Gr_inited = 1;

	return true;
}

void gr_force_windowed()
{
	if ( !Gr_inited ) {
		return;
	}

	switch( gr_screen.mode ) {
		case GR_OPENGL:
			break;
		case GR_STUB: 
			break;
		default:
			Int3();		// Invalid graphics mode
	}

	if ( Os_debugger_running ) {
		Sleep(1000);
	}
}

int gr_activated = 0;
void gr_activate(int active)
{

	if (gr_activated == active) {
		return;
	}
	gr_activated = active;

	if ( !Gr_inited ) { 
		return;
	}

	switch( gr_screen.mode ) {
		case GR_OPENGL:
			extern void gr_opengl_activate(int active);
			gr_opengl_activate(active);
			break;
		case GR_STUB: 
			break;
		default:
			Int3();		// Invalid graphics mode
	}

}

// color stuff
void gr_get_color( int *r, int *g, int *b )
{
	if (r) *r = gr_screen.current_color.red;
	if (g) *g = gr_screen.current_color.green;
	if (b) *b = gr_screen.current_color.blue;
}

void gr_init_color(color *c, int r, int g, int b)
{
	CAP(r, 0, 255);
	CAP(g, 0, 255);
	CAP(b, 0, 255);

	c->screen_sig = gr_screen.signature;
	c->red = (ubyte)r;
	c->green = (ubyte)g;
	c->blue = (ubyte)b;
	c->alpha = 255;
	c->ac_type = AC_TYPE_NONE;
	c->alphacolor = -1;
	c->is_alphacolor = 0;
	c->magic = 0xAC01;
}

void gr_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	CAP(r, 0, 255);
	CAP(g, 0, 255);
	CAP(b, 0, 255);
	CAP(alpha, 0, 255);

	gr_init_color( clr, r, g, b );

	clr->alpha = (ubyte)alpha;
	clr->ac_type = (ubyte)type;
	clr->alphacolor = -1;
	clr->is_alphacolor = 1;
}

void gr_set_color( int r, int g, int b )
{
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

	gr_init_color( &gr_screen.current_color, r, g, b );	
}

void gr_set_color_fast(color *dst)
{
	if ( dst->screen_sig != gr_screen.signature ) {
		if (dst->is_alphacolor) {
			gr_init_alphacolor( dst, dst->red, dst->green, dst->blue, dst->alpha, dst->ac_type );
		} else {
			gr_init_color( dst, dst->red, dst->green, dst->blue );
		}
	}

	gr_screen.current_color = *dst;
}

// shader functions
void gr_create_shader(shader *shade, ubyte r, ubyte g, ubyte b, ubyte c )
{
	shade->screen_sig = gr_screen.signature;
	shade->r = r;
	shade->g = g;
	shade->b = b;
	shade->c = c;
}

void gr_set_shader(shader *shade)
{
	if (shade) {
		if (shade->screen_sig != gr_screen.signature) {
			gr_create_shader( shade, shade->r, shade->g, shade->b, shade->c );
		}
		gr_screen.current_shader = *shade;
	} else {
		gr_create_shader( &gr_screen.current_shader, 0, 0, 0, 0 );
	}
}

/**
 * Set the bitmap for the mouse pointer.  This is called by the animating mouse
 * pointer code.
 *
 * The lock parameter just locks basically disables the next call of this function that doesn't
 * have an unlock feature.  If adding in more cursor-changing situations, be aware of
 * unexpected results. You have been warned.
 *
 * @todo investigate memory leak of original Gr_cursor bitmap when this is called
 */
void gr_set_cursor_bitmap(int n, int lock)
{
	int w, h;
	static int locked = 0;

	if ( !locked || (lock == GR_CURSOR_UNLOCK) ) {
		// if we are changing the cursor to something different
		// then unload the previous cursor's data - taylor
		if ( (Gr_cursor >= 0) && (Gr_cursor != n) ) {
			// be sure to avoid changing a cursor which is simply another frame
			if ( (GL_cursor_nframes < 2) || ((n - Gr_cursor) >= GL_cursor_nframes) ) {
				gr_unset_cursor_bitmap(Gr_cursor);
			}
		}

		if (n != Gr_cursor) {
			// get cursor size, so that we can be sure to account for the full thing
			// in later cursor hiding code
			bm_get_info(n, &w, &h, NULL, &GL_cursor_nframes);
			Assert( GL_cursor_nframes > 0 );

			Gr_cursor_size = MAX(w, h);

			if (Gr_cursor_size <= 0) {
				Int3();
				Gr_cursor_size = 32;
			}
		}

		Gr_cursor = n;
	} else {
		locked = 0;
	}

	if (lock == GR_CURSOR_LOCK) {
		locked = 1;
	}
}

void gr_unset_cursor_bitmap(int n)
{
	if (n < 0) {
		return;
	}

	if (Gr_cursor == n) {
		bm_unload(Gr_cursor);
		Gr_cursor = -1;
	}
}

/**
 * Retrieves the current bitmap
 * Used in UI_GADGET to save/restore current cursor state
 */
int gr_get_cursor_bitmap()
{
	return Gr_cursor;
}

// new bitmap functions
void gr_bitmap(int _x, int _y, bool allow_scaling)
{
	int _w, _h;
	float x, y, w, h;
	vertex verts[4];

	if (gr_screen.mode == GR_STUB) {
		return;
	}

	bm_get_info(gr_screen.current_bitmap, &_w, &_h, NULL, NULL, NULL);

	x = i2fl(_x);
	y = i2fl(_y);
	w = i2fl(_w);
	h = i2fl(_h);

	// I will tidy this up later - RT
	if ( allow_scaling && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_posf(&x, &y);
		gr_resize_screen_posf(&w, &h);
	}

	memset(verts, 0, sizeof(verts));

	verts[0].screen.xyw.x = x;
	verts[0].screen.xyw.y = y;
	verts[0].texture_position.u = 0.0f;
	verts[0].texture_position.v = 0.0f;

	verts[1].screen.xyw.x = x + w;
	verts[1].screen.xyw.y = y;
	verts[1].texture_position.u = 1.0f;
	verts[1].texture_position.v = 0.0f;

	verts[2].screen.xyw.x = x + w;
	verts[2].screen.xyw.y = y + h;
	verts[2].texture_position.u = 1.0f;
	verts[2].texture_position.v = 1.0f;

	verts[3].screen.xyw.x = x;
	verts[3].screen.xyw.y = y + h;
	verts[3].texture_position.u = 0.0f;
	verts[3].texture_position.v = 1.0f;

	// turn off zbuffering
	int saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);

	gr_render(4, verts, TMAP_FLAG_TEXTURED | TMAP_FLAG_INTERFACE);

	gr_zbuffer_set(saved_zbuffer_mode);
}

void gr_bitmap_uv(int _x, int _y, int _w, int _h, float _u0, float _v0, float _u1, float _v1, bool allow_scaling)
{
	float x, y, w, h;
	vertex verts[4];

	if (gr_screen.mode == GR_STUB) {
		return;
	}

	x = i2fl(_x);
	y = i2fl(_y);
	w = i2fl(_w);
	h = i2fl(_h);

	// I will tidy this up later - RT
	if ( allow_scaling && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_posf(&x, &y);
		gr_resize_screen_posf(&w, &h);
	}

	memset(verts, 0, sizeof(verts));

	verts[0].screen.xyw.x = x;
	verts[0].screen.xyw.y = y;
	verts[0].texture_position.u = _u0;
	verts[0].texture_position.v = _v0;

	verts[1].screen.xyw.x = x + w;
	verts[1].screen.xyw.y = y;
	verts[1].texture_position.u = _u1;
	verts[1].texture_position.v = _v0;

	verts[2].screen.xyw.x = x + w;
	verts[2].screen.xyw.y = y + h;
	verts[2].texture_position.u = _u1;
	verts[2].texture_position.v = _v1;

	verts[3].screen.xyw.x = x;
	verts[3].screen.xyw.y = y + h;
	verts[3].texture_position.u = _u0;
	verts[3].texture_position.v = _v1;

	// turn off zbuffering
	int saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);

	gr_render(4, verts, TMAP_FLAG_TEXTURED | TMAP_FLAG_INTERFACE);

	gr_zbuffer_set(saved_zbuffer_mode);
}

// NEW new bitmap functions -Bobboau
void gr_bitmap_list(bitmap_2d_list* list, int n_bm, bool allow_scaling)
{
	for (int i = 0; i < n_bm; i++) {
		bitmap_2d_list *l = &list[i];

		bm_get_info(gr_screen.current_bitmap, &l->w, &l->h, NULL, NULL, NULL);

		if ( allow_scaling && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
			gr_resize_screen_pos(&l->x, &l->y);
			gr_resize_screen_pos(&l->w, &l->h);
		}
	}

	g3_draw_2d_poly_bitmap_list(list, n_bm, TMAP_FLAG_INTERFACE);
}

// _->NEW<-_ NEW new bitmap functions -Bobboau
//takes a list of rectangles that have assosiated rectangles in a texture
void gr_bitmap_list(bitmap_rect_list* list, int n_bm, bool allow_scaling)
{
	for(int i = 0; i < n_bm; i++) {
		bitmap_2d_list *l = &list[i].screen_rect;

		// if no valid hight or width values were given get some from the bitmap
		if ( (l->w <= 0) || (l->h <= 0) ) {
			bm_get_info(gr_screen.current_bitmap, &l->w, &l->h, NULL, NULL, NULL);
		}

		if ( allow_scaling && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
			gr_resize_screen_pos(&l->x, &l->y);
			gr_resize_screen_pos(&l->w, &l->h);
		}
	}

	g3_draw_2d_poly_bitmap_rect_list(list, n_bm, TMAP_FLAG_INTERFACE);
}


/**
 * Given endpoints, and thickness, calculate coords of the endpoint
 */
void gr_pline_helper(vec3d *out, vec3d *in1, vec3d *in2, int thickness)
{
	vec3d slope;

	// slope of the line
	if(vm_vec_same(in1, in2)) {
		slope = vmd_zero_vector;
	} else {
		vm_vec_sub(&slope, in2, in1);
		float temp = -slope.xyz.x;
		slope.xyz.x = slope.xyz.y;
		slope.xyz.y = temp;
		vm_vec_normalize(&slope);
	}
	// get the points
	vm_vec_scale_add(out, in1, &slope, (float)thickness);
}

/**
 * Special function for drawing polylines.
 *
 * This function is specifically intended for polylines where each section 
 * is no more than 90 degrees away from a previous section.
 * Moreover, it is _really_ intended for use with 45 degree angles. 
 */
void gr_pline_special(vec3d **pts, int num_pts, int thickness,bool resize)
{
	vec3d s1, s2, e1, e2, dir;
	vec3d last_e1, last_e2;
	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};
	int saved_zbuffer_mode, idx;
	int started_frame = 0;

	// if we have less than 2 pts, bail
	if(num_pts < 2) {
		return;
	}

	extern int G3_count;
	if(G3_count == 0) {
		g3_start_frame(1);
		started_frame = 1;
	}

	// turn off zbuffering
	saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);

	// turn off culling
	int cull = gr_set_cull(0);

	// draw each section
	last_e1 = vmd_zero_vector;
	last_e2 = vmd_zero_vector;
	int j;
	for(idx=0; idx<num_pts-1; idx++) {
		// get the start and endpoints
		s1 = *pts[idx];											// start 1 (on the line)
		gr_pline_helper(&s2, pts[idx], pts[idx+1], thickness);	// start 2
		e1 = *pts[idx+1];										// end 1 (on the line)
		vm_vec_sub(&dir, pts[idx+1], pts[idx]);
		vm_vec_add(&e2, &s2, &dir);								// end 2
		
		// stuff coords
		v[0].screen.xyw.x = (float)ceil(s1.xyz.x);
		v[0].screen.xyw.y = (float)ceil(s1.xyz.y);
		v[0].screen.xyw.w = 0.0f;
		v[0].texture_position.u = 0.5f;
		v[0].texture_position.v = 0.5f;
		v[0].flags = PF_PROJECTED;
		v[0].codes = 0;
		v[0].r = gr_screen.current_color.red;
		v[0].g = gr_screen.current_color.green;
		v[0].b = gr_screen.current_color.blue;

		v[1].screen.xyw.x = (float)ceil(s2.xyz.x);
		v[1].screen.xyw.y = (float)ceil(s2.xyz.y);
		v[1].screen.xyw.w = 0.0f;
		v[1].texture_position.u = 0.5f;
		v[1].texture_position.v = 0.5f;
		v[1].flags = PF_PROJECTED;
		v[1].codes = 0;
		v[1].r = gr_screen.current_color.red;
		v[1].g = gr_screen.current_color.green;
		v[1].b = gr_screen.current_color.blue;

		v[2].screen.xyw.x = (float)ceil(e2.xyz.x);
		v[2].screen.xyw.y = (float)ceil(e2.xyz.y);
		v[2].screen.xyw.w = 0.0f;
		v[2].texture_position.u = 0.5f;
		v[2].texture_position.v = 0.5f;
		v[2].flags = PF_PROJECTED;
		v[2].codes = 0;
		v[2].r = gr_screen.current_color.red;
		v[2].g = gr_screen.current_color.green;
		v[2].b = gr_screen.current_color.blue;

		v[3].screen.xyw.x = (float)ceil(e1.xyz.x);
		v[3].screen.xyw.y = (float)ceil(e1.xyz.y);
		v[3].screen.xyw.w = 0.0f;
		v[3].texture_position.u = 0.5f;
		v[3].texture_position.v = 0.5f;
		v[3].flags = PF_PROJECTED;
		v[3].codes = 0;
		v[3].r = gr_screen.current_color.red;
		v[3].g = gr_screen.current_color.green;
		v[3].b = gr_screen.current_color.blue;

		//We could really do this better...but oh well. _WMC
		if(resize) {
			for(j=0;j<4;j++) {
				gr_resize_screen_posf(&v[j].screen.xyw.x,&v[j].screen.xyw.y);
			}
		}

		// draw the polys
		g3_draw_poly_constant_sw(4, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB, 0.1f);

		// if we're past the first section, draw a "patch" triangle to fill any gaps
		if(idx > 0) {
			// stuff coords
			v[0].screen.xyw.x = (float)ceil(s1.xyz.x);
			v[0].screen.xyw.y = (float)ceil(s1.xyz.y);
			v[0].screen.xyw.w = 0.0f;
			v[0].texture_position.u = 0.5f;
			v[0].texture_position.v = 0.5f;
			v[0].flags = PF_PROJECTED;
			v[0].codes = 0;
			v[0].r = gr_screen.current_color.red;
			v[0].g = gr_screen.current_color.green;
			v[0].b = gr_screen.current_color.blue;

			v[1].screen.xyw.x = (float)ceil(s2.xyz.x);
			v[1].screen.xyw.y = (float)ceil(s2.xyz.y);
			v[1].screen.xyw.w = 0.0f;
			v[1].texture_position.u = 0.5f;
			v[1].texture_position.v = 0.5f;
			v[1].flags = PF_PROJECTED;
			v[1].codes = 0;
			v[1].r = gr_screen.current_color.red;
			v[1].g = gr_screen.current_color.green;
			v[1].b = gr_screen.current_color.blue;


			v[2].screen.xyw.x = (float)ceil(last_e2.xyz.x);
			v[2].screen.xyw.y = (float)ceil(last_e2.xyz.y);
			v[2].screen.xyw.w = 0.0f;
			v[2].texture_position.u = 0.5f;
			v[2].texture_position.v = 0.5f;
			v[2].flags = PF_PROJECTED;
			v[2].codes = 0;
			v[2].r = gr_screen.current_color.red;
			v[2].g = gr_screen.current_color.green;
			v[2].b = gr_screen.current_color.blue;

			//Inefficiency or flexibility? you be the judge -WMC
			if(resize) {
				for(j=0;j<3;j++) {
					gr_resize_screen_posf(&v[j].screen.xyw.x,&v[j].screen.xyw.y);
				}
			}

			g3_draw_poly_constant_sw(3, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB, 0.1f);
		}

		// store our endpoints
		last_e1 = e1;
		last_e2 = e2;
	}

	if(started_frame) {
		g3_end_frame();
	}

	// restore zbuffer mode
	gr_zbuffer_set(saved_zbuffer_mode);

	// restore culling
	gr_set_cull(cull);
}

int poly_list::find_first_vertex(int idx)
{
	vec3d *o_norm = &norm[idx];
	vertex *o_vert = &vert[idx];
	vec3d *p_norm = &norm[0];
	vertex *p_vert = &vert[0];

	// we should always equal ourselves, so just use that as the stopping point
	for (int i = 0; i < idx; i++) {
		if ( (*p_norm == *o_norm)
			&& (p_vert->world == o_vert->world)
			&& (p_vert->texture_position == o_vert->texture_position) )
		{
			return i;
		}

		++p_norm;
		++p_vert;
	}

	return idx;
}

/**
 * Given a list (plist) find the index within the indexed list that the vert at position idx within list is at
 */
int poly_list::find_index(poly_list *plist, int idx)
{
	vec3d *o_norm = &plist->norm[idx];
	vertex *o_vert = &plist->vert[idx];
	vec3d *p_norm = &norm[0];
	vertex *p_vert = &vert[0];

	for (int i = 0; i < n_verts; i++) {
		if ( (*p_norm == *o_norm)
			&& (p_vert->world == o_vert->world)
			&& (p_vert->texture_position == o_vert->texture_position))
		{
			return i;
		}

		++p_vert;
		++p_norm;
	}

	return -1;
}


void poly_list::allocate(int _verts)
{
	if (_verts <= currently_allocated)
		return;

	if (vert != NULL) {
		vm_free(vert);
		vert = NULL;
	}

	if (norm != NULL) {
		vm_free(norm);
		norm = NULL;
	}

	if (tsb != NULL) {
		vm_free(tsb);
		tsb = NULL;
	}

	if (_verts) {
		vert = (vertex*)vm_malloc(sizeof(vertex) * _verts);
		norm = (vec3d*)vm_malloc(sizeof(vec3d) * _verts);

		if (Cmdline_normal) {
			tsb = (tsb_t*)vm_malloc(sizeof(tsb_t) * _verts);
		}
	}

	n_verts = 0;
	currently_allocated = _verts;
}

poly_list::~poly_list()
{
	if (vert != NULL) {
		vm_free(vert);
		vert = NULL;
	}

	if (norm != NULL) {
		vm_free(norm);
		norm = NULL;
	}

	if (tsb != NULL) {
		vm_free(tsb);
		tsb = NULL;
	}
}

void poly_list::calculate_tangent()
{
	vertex *v0, *v1, *v2;
	vec3d *t0, *t1, *t2;
	vec3d side0, side1;
	vec3d vt0, vt1;
	float deltaU0, deltaV0, deltaU1, deltaV1;
	vec3d tangent, binormal, cross;
	float magg, scale;

	if ( !Cmdline_normal ) {
		return;
	}

	Assert( !(n_verts % 3) );

	for (int i = 0; i < n_verts; i += 3) {
		// vertex (reading)
		v0 = &vert[i];
		v1 = &vert[i+1];
		v2 = &vert[i+2];
		// tangents (writing)
		t0 = &tsb[i].tangent;
		t1 = &tsb[i+1].tangent;
		t2 = &tsb[i+2].tangent;


		deltaU0 = v1->texture_position.u - v0->texture_position.u;
		deltaV0 = v1->texture_position.v - v0->texture_position.v;

		deltaU1 = v2->texture_position.u - v0->texture_position.u;
		deltaV1 = v2->texture_position.v - v0->texture_position.v;

		// quick short circuit for NULL case
		float n = (deltaU0 * deltaV1) - (deltaU1 * deltaV0);

		if (n == 0.0f) {
			// hit NULL, so just set identity
			tangent  = vmd_x_vector;
			binormal = vmd_y_vector;
		} else {
			float blah = 1.0f / n;

			vm_vec_sub(&side0, &v1->world, &v0->world);
			vm_vec_sub(&side1, &v2->world, &v0->world);

			// tangent
			vm_vec_copy_scale(&vt0, &side0, deltaV1);
			vm_vec_copy_scale(&vt1, &side1, deltaV0);
			vm_vec_sub(&tangent, &vt0, &vt1);
			vm_vec_scale(&tangent, blah);

			// binormal
			vm_vec_copy_scale(&vt0, &side0, deltaU1);
			vm_vec_copy_scale(&vt1, &side1, deltaU0);
			vm_vec_sub(&binormal, &vt0, &vt1);
			vm_vec_scale(&binormal, blah);
		}

		// orthogonalize tangent (for all 3 verts)
		magg = vm_vec_dot(&norm[i], &tangent);
		vm_vec_scale_sub(t0, &tangent, &norm[i], magg);
		vm_vec_normalize_safe(t0);

		magg = vm_vec_dot(&norm[i+1], &tangent);
		vm_vec_scale_sub(t1, &tangent, &norm[i+1], magg);
		vm_vec_normalize_safe(t1);

		magg = vm_vec_dot(&norm[i+2], &tangent);
		vm_vec_scale_sub(t2, &tangent, &norm[i+2], magg);
		vm_vec_normalize_safe(t2);

		// compute handedness (for all 3 verts)
		vm_vec_crossprod(&cross, &norm[i], &tangent);
		scale = vm_vec_dot(&cross, &binormal);
		tsb[i].scaler = (scale < 0.0f) ? -1.0f : 1.0f;

		vm_vec_crossprod(&cross, &norm[i+1], &tangent);
		scale = vm_vec_dot(&cross, &binormal);
		tsb[i+1].scaler = (scale < 0.0f) ? -1.0f : 1.0f;

		vm_vec_crossprod(&cross, &norm[i+2], &tangent);
		scale = vm_vec_dot(&cross, &binormal);
		tsb[i+2].scaler = (scale < 0.0f) ? -1.0f : 1.0f;
	}
}

static poly_list buffer_list_internal;

void poly_list::make_index_buffer(SCP_vector<int> &vertex_list)
{
	int nverts = 0;
	int j, z = 0;
	ubyte *nverts_good = NULL;

	// calculate tangent space data (must be done early)
	calculate_tangent();

	// using vm_malloc() here rather than 'new' so we get the extra out-of-memory check
	nverts_good = (ubyte *) vm_malloc(n_verts);

	Assert( nverts_good != NULL );
	memset( nverts_good, 0, n_verts );

	vertex_list.reserve(n_verts);

	for (j = 0; j < n_verts; j++) {
		if (find_first_vertex(j) == j) {
			nverts++;
			nverts_good[j] = 1;
			vertex_list.push_back(j);
		}
	}

	// if there is nothig to change then bail
	if (n_verts == nverts) {
		if (nverts_good != NULL) {
			vm_free(nverts_good);
		}

		return;
	}

	buffer_list_internal.n_verts = 0;
	buffer_list_internal.allocate(nverts);

	for (j = 0; j < n_verts; j++) {
		if ( !nverts_good[j] ) {
			continue;
		}

		buffer_list_internal.vert[z] = vert[j];
		buffer_list_internal.norm[z] = norm[j];

		if (Cmdline_normal) {
			buffer_list_internal.tsb[z] = tsb[j];
		}

		buffer_list_internal.n_verts++;
		z++;
	}

	Assert(nverts == buffer_list_internal.n_verts);

	if (nverts_good != NULL) {
		vm_free(nverts_good);
	}

	(*this) = buffer_list_internal;
}

poly_list& poly_list::operator = (poly_list &other_list)
{
	allocate(other_list.n_verts);

	memcpy(norm, other_list.norm, sizeof(vec3d) * other_list.n_verts);
	memcpy(vert, other_list.vert, sizeof(vertex) * other_list.n_verts);

	if (Cmdline_normal) {
		memcpy(tsb, other_list.tsb, sizeof(tsb_t) * other_list.n_verts);
	}

	n_verts = other_list.n_verts;

	return *this;
}

void gr_rect(int x, int y, int w, int h, bool resize)
{
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	if (resize) {
		gr_resize_screen_pos(&x, &y);
		gr_resize_screen_pos(&w, &h);
	}

	g3_draw_2d_rect(x, y, w, h,
		gr_screen.current_color.red,
		gr_screen.current_color.green,
		gr_screen.current_color.blue,
		gr_screen.current_color.alpha);
}

void gr_shade(int x, int y, int w, int h, bool resize)
{
	int r, g, b, a;

	if (gr_screen.mode == GR_STUB) {
		return;
	}

	if (resize) {
		gr_resize_screen_pos(&x, &y);
		gr_resize_screen_pos(&w, &h);
	}

	r = (int)gr_screen.current_shader.r;
	g = (int)gr_screen.current_shader.g;
	b = (int)gr_screen.current_shader.b;
	a = (int)gr_screen.current_shader.c;

	g3_draw_2d_rect(x, y, w, h, r, g, b, a);
}

void gr_set_bitmap(int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha)
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
}

void gr_flip()
{
	// m!m avoid running CHA_ONFRAME when the "Quit mission" popup is shown. See mantis 2446 for reference
	if (!quit_mission_popup_shown)
	{
		//WMC - Evaluate global hook if not override.
		Script_system.RunBytecode(Script_globalhook);
		//WMC - Do conditional hooks. Yippee!
		Script_system.RunCondition(CHA_ONFRAME);
		//WMC - Do scripting reset stuff
		Script_system.EndFrame();
	}

	gr_screen.gf_flip();
}
