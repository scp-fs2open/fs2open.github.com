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

#include <climits>
#include <algorithm>

#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "gamesequence/gamesequence.h"	//WMC - for scripting hooks in gr_flip()
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "graphics/light.h"
#include "graphics/font.h"
#include "graphics/grbatch.h"
#include "graphics/grinternal.h"
#include "graphics/opengl/gropengl.h"
#include "graphics/opengl/gropengldraw.h"
#include "graphics/grstub.h"
#include "graphics/paths/PathRenderer.h"
#include "graphics/util/GPUMemoryHeap.h"
#include "graphics/util/UniformBuffer.h"
#include "graphics/util/UniformBufferManager.h"
#include "io/keycontrol.h" // m!m
#include "io/timer.h"
#include "osapi/osapi.h"
#include "scripting/scripting.h"
#include "parse/parselo.h"
#include "render/3d.h"
#include "tracing/tracing.h"
#include "utils/boost/hash_combine.h"

#if ( SDL_VERSION_ATLEAST(1, 2, 7) )
#include "SDL_cpuinfo.h"
#endif

#include "SDL_surface.h"

const char *Resolution_prefixes[GR_NUM_RESOLUTIONS] = { "", "2_" };

screen gr_screen;

color_gun Gr_red, Gr_green, Gr_blue, Gr_alpha;
color_gun Gr_t_red, Gr_t_green, Gr_t_blue, Gr_t_alpha;
color_gun Gr_ta_red, Gr_ta_green, Gr_ta_blue, Gr_ta_alpha;
color_gun *Gr_current_red, *Gr_current_green, *Gr_current_blue, *Gr_current_alpha;


ubyte Gr_original_palette[768];		// The palette 
ubyte Gr_current_palette[768];
char Gr_current_palette_name[128] = NOX("none");

// cursor stuff
io::mouse::Cursor* Web_cursor = NULL;

int Gr_inited = 0;

uint Gr_signature = 0;

float Gr_gamma = 1.8f;
int Gr_gamma_int = 180;

// z-buffer stuff
int gr_zbuffering = 0;
int gr_zbuffering_mode = 0;
int gr_global_zbuffering = 0;

// stencil buffer stuff
int gr_stencil_mode = 0;

// Default clipping distances
const float Default_min_draw_distance = 1.0f;
const float Default_max_draw_distance = 1e10;
float Min_draw_distance = Default_min_draw_distance;
float Max_draw_distance = Default_max_draw_distance;

// Pre-computed screen resize vars
static float Gr_full_resize_X = 1.0f, Gr_full_resize_Y = 1.0f;
static float Gr_full_center_resize_X = 1.0f, Gr_full_center_resize_Y = 1.0f;
static float Gr_resize_X = 1.0f, Gr_resize_Y = 1.0f;
static float Gr_menu_offset_X = 0.0f, Gr_menu_offset_Y = 0.0f;
static float Gr_menu_zoomed_offset_X = 0.0f, Gr_menu_zoomed_offset_Y = 0.0f;

float Gr_save_full_resize_X = 1.0f, Gr_save_full_resize_Y = 1.0f;
float Gr_save_full_center_resize_X = 1.0f, Gr_save_full_center_resize_Y = 1.0f;
float Gr_save_resize_X = 1.0f, Gr_save_resize_Y = 1.0f;
float Gr_save_menu_offset_X = 0.0f, Gr_save_menu_offset_Y = 0.0f;
float Gr_save_menu_zoomed_offset_X = 0.0f, Gr_save_menu_zoomed_offset_Y = 0.0f;

bool Save_custom_screen_size;

// Forward definitions
static void uniform_buffer_managers_init();
static void uniform_buffer_managers_deinit();
static void uniform_buffer_managers_retire_buffers();

static void gpu_heap_init();
static void gpu_heap_deinit();

void gr_set_screen_scale(int w, int h, int zoom_w, int zoom_h, int max_w, int max_h, int center_w, int center_h, bool force_stretch)
{
	bool do_zoom = zoom_w > 0 && zoom_h > 0 && (zoom_w != w || zoom_h != h);

	Gr_full_resize_X = (float)max_w / (float)w;
	Gr_full_resize_Y = (float)max_h / (float)h;

	Gr_full_center_resize_X = (float)center_w / (float)w;
	Gr_full_center_resize_Y = (float)center_h / (float)h;

	if (do_zoom) {
		float aspect_quotient = ((float)center_w / (float)center_h) / ((float)zoom_w / (float)zoom_h);

		Gr_resize_X = (float)center_w / (float)zoom_w / ((aspect_quotient > 1.0f) ? aspect_quotient : 1.0f);
		Gr_resize_Y = (float)center_h / (float)zoom_h * ((aspect_quotient < 1.0f) ? aspect_quotient : 1.0f);

		Gr_menu_offset_X = ((center_w - w * Gr_resize_X) / 2.0f) + gr_screen.center_offset_x;
		Gr_menu_offset_Y = ((center_h - h * Gr_resize_Y) / 2.0f) + gr_screen.center_offset_y;

		Gr_menu_zoomed_offset_X = (Gr_menu_offset_X >= 0.0f) ? Gr_menu_offset_X : gr_screen.center_offset_x;
		Gr_menu_zoomed_offset_Y = (Gr_menu_offset_Y >= 0.0f) ? Gr_menu_offset_Y : gr_screen.center_offset_y;

		if (force_stretch || Cmdline_stretch_menu) {
			if (Gr_menu_offset_X > (float)gr_screen.center_offset_x) {
				Gr_resize_X = Gr_full_center_resize_X;
				Gr_menu_offset_X = Gr_menu_zoomed_offset_X = (float)gr_screen.center_offset_x;
			}
			if (Gr_menu_offset_Y > (float)gr_screen.center_offset_y) {
				Gr_resize_Y = Gr_full_center_resize_Y;
				Gr_menu_offset_Y = Gr_menu_zoomed_offset_Y = (float)gr_screen.center_offset_y;
			}
		}
	} else {
		if (force_stretch || Cmdline_stretch_menu) {
			Gr_resize_X = Gr_full_center_resize_X;
			Gr_resize_Y = Gr_full_center_resize_Y;

			Gr_menu_offset_X = Gr_menu_zoomed_offset_X = (float)gr_screen.center_offset_x;
			Gr_menu_offset_Y = Gr_menu_zoomed_offset_Y = (float)gr_screen.center_offset_y;
		} else {
			float aspect_quotient = ((float)center_w / (float)center_h) / ((float)w / (float)h);

			Gr_resize_X = Gr_full_center_resize_X / ((aspect_quotient > 1.0f) ? aspect_quotient : 1.0f);
			Gr_resize_Y = Gr_full_center_resize_Y * ((aspect_quotient < 1.0f) ? aspect_quotient : 1.0f);

			Gr_menu_offset_X = Gr_menu_zoomed_offset_X = ((aspect_quotient > 1.0f) ? ((center_w - w * Gr_resize_X) / 2.0f) : 0.0f) + gr_screen.center_offset_x;
			Gr_menu_offset_Y = Gr_menu_zoomed_offset_Y = ((aspect_quotient < 1.0f) ? ((center_h - h * Gr_resize_Y) / 2.0f) : 0.0f) + gr_screen.center_offset_y;
		}
	}

	gr_screen.custom_size = (w != max_w || w != center_w || h != max_h || h != center_h);

	if (gr_screen.rendering_to_texture == -1) {
		gr_screen.max_w_unscaled = w;
		gr_screen.max_h_unscaled = h;

		if (do_zoom) {
			gr_screen.max_w_unscaled_zoomed = gr_screen.max_w_unscaled + fl2i(Gr_menu_offset_X * 2.0f / Gr_resize_X);
			gr_screen.max_h_unscaled_zoomed = gr_screen.max_h_unscaled + fl2i(Gr_menu_offset_Y * 2.0f / Gr_resize_Y);
			if (gr_screen.max_w_unscaled_zoomed > gr_screen.max_w_unscaled) {
				gr_screen.max_w_unscaled_zoomed = gr_screen.max_w_unscaled;
			}
			if (gr_screen.max_h_unscaled_zoomed > gr_screen.max_h_unscaled) {
				gr_screen.max_h_unscaled_zoomed = gr_screen.max_h_unscaled;
			}
		} else {
			gr_screen.max_w_unscaled_zoomed = gr_screen.max_w_unscaled;
			gr_screen.max_h_unscaled_zoomed = gr_screen.max_h_unscaled;
		}
	}
}

void gr_reset_screen_scale()
{
	Gr_full_resize_X = Gr_save_full_resize_X;
	Gr_full_resize_Y = Gr_save_full_resize_Y;

	Gr_full_center_resize_X = Gr_save_full_center_resize_X;
	Gr_full_center_resize_Y = Gr_save_full_center_resize_Y;

	Gr_resize_X = Gr_save_resize_X;
	Gr_resize_Y = Gr_save_resize_Y;

	Gr_menu_offset_X = Gr_save_menu_offset_X;
	Gr_menu_offset_Y = Gr_save_menu_offset_Y;

	Gr_menu_zoomed_offset_X = Gr_save_menu_zoomed_offset_X;
	Gr_menu_zoomed_offset_Y = Gr_save_menu_zoomed_offset_Y;

	gr_screen.custom_size = Save_custom_screen_size;

	if (gr_screen.rendering_to_texture == -1) {
		gr_screen.max_w_unscaled = gr_screen.max_w_unscaled_zoomed = (gr_screen.res == GR_1024) ? 1024 : 640;
		gr_screen.max_h_unscaled = gr_screen.max_h_unscaled_zoomed = (gr_screen.res == GR_1024) ?  768 : 480;
	}
}

/**
 * This function is to be called if you wish to scale GR_1024 or GR_640 x and y positions or
 * lengths in order to keep the correctly scaled to nonstandard resolutions
 *
 * @param x X value, can be NULL
 * @param y Y value, can be NULL
 * @param w width, can be NULL
 * @param h height, can be NULL
 * @param resize_mode
 * @return always true unless error
 */
bool gr_resize_screen_pos(int *x, int *y, int *w, int *h, int resize_mode)
{
	if ( resize_mode == GR_RESIZE_NONE || (!gr_screen.custom_size && (gr_screen.rendering_to_texture == -1)) ) {
		return false;
	}

	float xy_tmp = 0.0f;

	if ( x ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*x) * Gr_full_resize_X;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*x) * Gr_full_center_resize_X + (float)gr_screen.center_offset_x;
			break;

		case GR_RESIZE_MENU:
			xy_tmp = (*x) * Gr_resize_X + Gr_menu_offset_X;
			break;

		case GR_RESIZE_MENU_ZOOMED:
			xy_tmp = (*x) * Gr_resize_X + Gr_menu_zoomed_offset_X;
			break;

		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*x) * Gr_resize_X;
			break;
	}
		(*x) = fl2ir(xy_tmp);
	}

	if ( y ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*y) * Gr_full_resize_Y;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*y) * Gr_full_center_resize_Y + (float)gr_screen.center_offset_y;
			break;

		case GR_RESIZE_MENU:
			xy_tmp = (*y) * Gr_resize_Y + Gr_menu_offset_Y;
			break;

		case GR_RESIZE_MENU_ZOOMED:
			xy_tmp = (*y) * Gr_resize_Y + Gr_menu_zoomed_offset_Y;
			break;

		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*y) * Gr_resize_Y;
			break;
	}
		(*y) = fl2ir(xy_tmp);
	}

	if ( w ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*w) * Gr_full_resize_X;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*w) * Gr_full_center_resize_X;
			break;

		case GR_RESIZE_MENU:
		case GR_RESIZE_MENU_ZOOMED:
		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*w) * Gr_resize_X;
			break;
		}
		(*w) = fl2ir(xy_tmp);
	}

	if ( h ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*h) * Gr_full_resize_Y;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*h) * Gr_full_center_resize_Y;
			break;

		case GR_RESIZE_MENU:
		case GR_RESIZE_MENU_ZOOMED:
		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*h) * Gr_resize_Y;
			break;
		}
		(*h) = fl2ir(xy_tmp);
	}

	return true;
}

/**
 *
 * @param x X value, can be NULL
 * @param y Y value, can be NULL
 * @param w width, can be NULL
 * @param h height, can be NULL
 * @param resize_mode
 * @return always true unless error
 */
bool gr_unsize_screen_pos(int *x, int *y, int *w, int *h, int resize_mode)
{
	if ( resize_mode == GR_RESIZE_NONE || (!gr_screen.custom_size && (gr_screen.rendering_to_texture == -1)) ) {
		return false;
	}

	float xy_tmp = 0.0f;

	if ( x ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*x) / Gr_full_resize_X;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = ((*x) - (float)gr_screen.center_offset_x) / Gr_full_center_resize_X;
			break;

		case GR_RESIZE_MENU:
			xy_tmp = ((*x) - Gr_menu_offset_X) / Gr_resize_X;
			break;

		case GR_RESIZE_MENU_ZOOMED:
			xy_tmp = ((*x) - Gr_menu_zoomed_offset_X) / Gr_resize_X;
			break;

		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*x) / Gr_resize_X;
			break;
	}
		(*x) = fl2ir(xy_tmp);
	}

	if ( y ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*y) / Gr_full_resize_Y;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = ((*y) - (float)gr_screen.center_offset_y) / Gr_full_center_resize_Y;
			break;

		case GR_RESIZE_MENU:
			xy_tmp = ((*y) - Gr_menu_offset_Y) / Gr_resize_Y;
			break;

		case GR_RESIZE_MENU_ZOOMED:
			xy_tmp = ((*y) - Gr_menu_zoomed_offset_Y) / Gr_resize_Y;
			break;

		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*y) / Gr_resize_Y;
			break;
	}
		(*y) = fl2ir(xy_tmp);
	}

	if ( w ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*w) / Gr_full_resize_X;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*w) / Gr_full_center_resize_X;
			break;

		case GR_RESIZE_MENU:
		case GR_RESIZE_MENU_ZOOMED:
		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*w) / Gr_resize_X;
			break;
		}
		(*w) = fl2ir(xy_tmp);
	}

	if ( h ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*h) / Gr_full_resize_Y;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*h) / Gr_full_center_resize_Y;
			break;

		case GR_RESIZE_MENU:
		case GR_RESIZE_MENU_ZOOMED:
		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*h) / Gr_resize_Y;
			break;
		}
		(*h) = fl2ir(xy_tmp);
	}

	return true;
}

/**
 * This function is to be called if you wish to scale GR_1024 or GR_640 x and y positions or
 * lengths in order to keep the correctly scaled to nonstandard resolutions
 *
 * @param x X value, can be NULL
 * @param y Y value, can be NULL
 * @param w width, can be NULL
 * @param h height, can be NULL
 * @param resize_mode
 * @return always true unless error
 */
bool gr_resize_screen_posf(float *x, float *y, float *w, float *h, int resize_mode)
{
	if ( resize_mode == GR_RESIZE_NONE || (!gr_screen.custom_size && (gr_screen.rendering_to_texture == -1)) ) {
		return false;
	}

	float xy_tmp = 0.0f;

	if ( x ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*x) * Gr_full_resize_X;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*x) * Gr_full_center_resize_X + (float)gr_screen.center_offset_x;
			break;

		case GR_RESIZE_MENU:
			xy_tmp = (*x) * Gr_resize_X + Gr_menu_offset_X;
			break;

		case GR_RESIZE_MENU_ZOOMED:
			xy_tmp = (*x) * Gr_resize_X + Gr_menu_zoomed_offset_X;
			break;

		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*x) * Gr_resize_X;
			break;
		}
		(*x) = xy_tmp;
	}

	if ( y ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*y) * Gr_full_resize_Y;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*y) * Gr_full_center_resize_Y + (float)gr_screen.center_offset_y;
			break;

		case GR_RESIZE_MENU:
			xy_tmp = (*y) * Gr_resize_Y + Gr_menu_offset_Y;
			break;

		case GR_RESIZE_MENU_ZOOMED:
			xy_tmp = (*y) * Gr_resize_Y + Gr_menu_zoomed_offset_Y;
			break;

		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*y) * Gr_resize_Y;
			break;
		}
		(*y) = xy_tmp;
	}

	if ( w ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*w) * Gr_full_resize_X;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*w) * Gr_full_center_resize_X;
			break;

		case GR_RESIZE_MENU:
		case GR_RESIZE_MENU_ZOOMED:
		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*w) * Gr_resize_X;
			break;
		}
		(*w) = xy_tmp;
	}

	if ( h ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*h) * Gr_full_resize_Y;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*h) * Gr_full_center_resize_Y;
			break;

		case GR_RESIZE_MENU:
		case GR_RESIZE_MENU_ZOOMED:
		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*h) * Gr_resize_Y;
			break;
		}
		(*h) = xy_tmp;
	}

	return true;
}

/**
 *
 * @param x X value, can be NULL
 * @param y Y value, can be NULL
 * @param w width, can be NULL
 * @param h height, can be NULL
 * @param resize_mode
 * @return always true unless error
 */
bool gr_unsize_screen_posf(float *x, float *y, float *w, float *h, int resize_mode)
{
	if ( resize_mode == GR_RESIZE_NONE || (!gr_screen.custom_size && (gr_screen.rendering_to_texture == -1)) ) {
		return false;
	}

	float xy_tmp = 0.0f;

	if ( x ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*x) / Gr_full_resize_X;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = ((*x) - (float)gr_screen.center_offset_x) / Gr_full_center_resize_X;
			break;

		case GR_RESIZE_MENU:
			xy_tmp = ((*x) - Gr_menu_offset_X) / Gr_resize_X;
			break;

		case GR_RESIZE_MENU_ZOOMED:
			xy_tmp = ((*x) - Gr_menu_zoomed_offset_X) / Gr_resize_X;
			break;

		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*x) / Gr_resize_X;
			break;
		}
		(*x) = xy_tmp;
	}

	if ( y ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*y) / Gr_full_resize_Y;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = ((*y) - (float)gr_screen.center_offset_y) / Gr_full_center_resize_Y;
			break;

		case GR_RESIZE_MENU:
			xy_tmp = ((*y) - Gr_menu_offset_Y) / Gr_resize_Y;
			break;

		case GR_RESIZE_MENU_ZOOMED:
			xy_tmp = ((*y) - Gr_menu_zoomed_offset_Y) / Gr_resize_Y;
			break;

		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*y) / Gr_resize_Y;
			break;
		}
		(*y) = xy_tmp;
	}

	if ( w ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*w) / Gr_full_resize_X;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*w) / Gr_full_center_resize_X;
			break;

		case GR_RESIZE_MENU:
		case GR_RESIZE_MENU_ZOOMED:
		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*w) / Gr_resize_X;
			break;
		}
		(*w) = xy_tmp;
	}

	if ( h ) {
		switch (resize_mode) {
		case GR_RESIZE_FULL:
			xy_tmp = (*h) / Gr_full_resize_Y;
			break;

		case GR_RESIZE_FULL_CENTER:
			xy_tmp = (*h) / Gr_full_center_resize_Y;
			break;

		case GR_RESIZE_MENU:
		case GR_RESIZE_MENU_ZOOMED:
		case GR_RESIZE_MENU_NO_OFFSET:
			xy_tmp = (*h) / Gr_resize_Y;
			break;
		}
		(*h) = xy_tmp;
	}

	return true;
}

void gr_close()
{
	if ( !Gr_inited ) {
		return;
	}

	gpu_heap_deinit();

	// Cleanup uniform buffer managers
	uniform_buffer_managers_deinit();
	
	font::close();

	gr_light_shutdown();

	graphics::paths::PathRenderer::shutdown();

	switch (gr_screen.mode) {
		case GR_OPENGL:
			gr_opengl_cleanup(true);
			break;
	
		case GR_STUB:
			break;
	
		default:
			Int3();		// Invalid graphics mode
	}

	bm_close();

	Gr_inited = 0;
}


/**
 * Set screen clear color
 */
DCF(clear_color, "set clear color r, g, b")
{
	ubyte r, g, b;

	dc_stuff_ubyte(&r);
	dc_stuff_ubyte(&g);
	dc_stuff_ubyte(&b);

	// set the color
	gr_set_clear_color(r, g, b);
}

void gr_set_palette_internal( const char * /*name*/, ubyte * palette, int  /*restrict_font_to_128*/ )
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
		// Since the palette set code might shuffle the palette,		
		// reload it into the source palette		
		if (palette) {
			memmove(palette, Gr_current_palette, 768);
		}
	}
}


void gr_set_palette( const char *name, ubyte * palette, int restrict_font_to_128 )
{
	char *p;
	strcpy_s( Gr_current_palette_name, name );
	p = strchr( Gr_current_palette_name, '.' );
	if ( p ) *p = 0;
	gr_screen.signature = Gr_signature++;
	gr_set_palette_internal( name, palette, restrict_font_to_128 );
}

void gr_screen_resize(int width, int height)
{
	gr_screen.save_center_w = gr_screen.center_w = gr_screen.save_max_w = gr_screen.max_w = gr_screen.max_w_unscaled = gr_screen.max_w_unscaled_zoomed = width;
	gr_screen.save_center_h = gr_screen.center_h = gr_screen.save_max_h = gr_screen.max_h = gr_screen.max_h_unscaled = gr_screen.max_h_unscaled_zoomed = height;

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
		gr_unsize_screen_pos( &gr_screen.max_w_unscaled_zoomed, &gr_screen.max_h_unscaled_zoomed );
		gr_unsize_screen_pos( &gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled );
	}

	gr_screen.save_max_w_unscaled = gr_screen.max_w_unscaled;
	gr_screen.save_max_h_unscaled = gr_screen.max_h_unscaled;
	gr_screen.save_max_w_unscaled_zoomed = gr_screen.max_w_unscaled_zoomed;
	gr_screen.save_max_h_unscaled_zoomed = gr_screen.max_h_unscaled_zoomed;

	gr_setup_viewport();
}

int gr_get_resolution_class(int width, int height)
{
	if ((width >= GR_1024_THRESHOLD_WIDTH) && (height >= GR_1024_THRESHOLD_HEIGHT)) {
		return GR_1024;
	} else {
		return GR_640;
	}
}

static bool gr_init_sub(std::unique_ptr<os::GraphicsOperations>&& graphicsOps, int mode, int width, int height, int depth, float center_aspect_ratio)
{
	int res = GR_1024;
	bool rc = false;

	memset( &gr_screen, 0, sizeof(screen) );

	float aspect_ratio = (float)width / (float)height;

	if ( (((width == 640) && (height == 480)) || ((width == 1024) && (height == 768))) && (aspect_ratio == center_aspect_ratio) ) {
		gr_screen.custom_size = false;
	} else {
		gr_screen.custom_size = true;
	}

	gr_screen.save_max_w = gr_screen.max_w = gr_screen.max_w_unscaled = gr_screen.max_w_unscaled_zoomed = width;
	gr_screen.save_max_h = gr_screen.max_h = gr_screen.max_h_unscaled = gr_screen.max_h_unscaled_zoomed = height;
	if (aspect_ratio > center_aspect_ratio) {
		gr_screen.save_center_w = gr_screen.center_w = fl2ir(height * center_aspect_ratio);
		gr_screen.save_center_h = gr_screen.center_h = height;
	} else if (aspect_ratio < center_aspect_ratio) {
		gr_screen.save_center_w = gr_screen.center_w = width;
		gr_screen.save_center_h = gr_screen.center_h = fl2ir(width / center_aspect_ratio);
	} else {
		gr_screen.save_center_w = gr_screen.center_w = width;
		gr_screen.save_center_h = gr_screen.center_h = height;
	}
	gr_screen.save_center_offset_x = gr_screen.center_offset_x = (width - gr_screen.center_w) / 2;
	gr_screen.save_center_offset_y = gr_screen.center_offset_y = (height - gr_screen.center_h) / 2;

	res = gr_get_resolution_class(gr_screen.center_w, gr_screen.center_h);

	if (Fred_running) {
		gr_screen.custom_size = false;
		res = GR_640;
		mode = GR_OPENGL;
	}

	Save_custom_screen_size = gr_screen.custom_size;

	Gr_save_full_resize_X = Gr_full_resize_X = (float)width / ((res == GR_1024) ? 1024.0f : 640.0f);
	Gr_save_full_resize_Y = Gr_full_resize_Y = (float)height / ((res == GR_1024) ?  768.0f : 480.0f);

	Gr_save_full_center_resize_X = Gr_full_center_resize_X = (float)gr_screen.center_w / ((res == GR_1024) ? 1024.0f : 640.0f);
	Gr_save_full_center_resize_Y = Gr_full_center_resize_Y = (float)gr_screen.center_h / ((res == GR_1024) ?  768.0f : 480.0f);

	if (gr_screen.custom_size && !Cmdline_stretch_menu) {
		float aspect_quotient = center_aspect_ratio / (4.0f / 3.0f);

		Gr_save_resize_X = Gr_resize_X = Gr_full_center_resize_X / ((aspect_quotient > 1.0f) ? aspect_quotient : 1.0f);
		Gr_save_resize_Y = Gr_resize_Y = Gr_full_center_resize_Y * ((aspect_quotient < 1.0f) ? aspect_quotient : 1.0f);

		Gr_save_menu_offset_X = Gr_menu_offset_X = ((aspect_quotient > 1.0f) ? ((gr_screen.center_w - gr_screen.center_w / aspect_quotient) / 2.0f) : 0.0f) + gr_screen.center_offset_x;
		Gr_save_menu_offset_Y = Gr_menu_offset_Y = ((aspect_quotient < 1.0f) ? ((gr_screen.center_h - gr_screen.center_h * aspect_quotient) / 2.0f) : 0.0f) + gr_screen.center_offset_y;
	} else {
		Gr_save_resize_X = Gr_resize_X = Gr_full_center_resize_X;
		Gr_save_resize_Y = Gr_resize_Y = Gr_full_center_resize_Y;

		Gr_save_menu_offset_X = Gr_menu_offset_X = (float)gr_screen.center_offset_x;
		Gr_save_menu_offset_Y = Gr_menu_offset_Y = (float)gr_screen.center_offset_y;
	}

	Gr_save_menu_zoomed_offset_X = Gr_menu_zoomed_offset_X = Gr_menu_offset_X;
	Gr_save_menu_zoomed_offset_Y = Gr_menu_zoomed_offset_Y = Gr_menu_offset_Y;
	

	gr_screen.signature = Gr_signature++;
	gr_screen.bits_per_pixel = depth;
	gr_screen.bytes_per_pixel= depth / 8;
	gr_screen.rendering_to_texture = -1;
	gr_screen.envmap_render_target = -1;
	gr_screen.line_width = 1.0f;
	gr_screen.mode = mode;
	gr_screen.res = res;
	gr_screen.aspect = 1.0f;			// Normal PC screen

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
		gr_unsize_screen_pos( &gr_screen.max_w_unscaled_zoomed, &gr_screen.max_h_unscaled_zoomed );
		gr_unsize_screen_pos( &gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled );
	}

	gr_screen.save_max_w_unscaled = gr_screen.max_w_unscaled;
	gr_screen.save_max_h_unscaled = gr_screen.max_h_unscaled;
	gr_screen.save_max_w_unscaled_zoomed = gr_screen.max_w_unscaled_zoomed;
	gr_screen.save_max_h_unscaled_zoomed = gr_screen.max_h_unscaled_zoomed;
	
	switch (mode) {
		case GR_OPENGL:
			rc = gr_opengl_init(std::move(graphicsOps));
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

static void init_window_icon() {
	auto view = os::getMainViewport();

	if (view == nullptr) {
		// Graphics backend has no viewport
		return;
	}

	auto sdl_wnd = view->toSDLWindow();

	if (sdl_wnd == nullptr) {
		// No support for changing the icon
		return;
	}

	auto icon_handle = bm_load(Window_icon_path);
	if (icon_handle < 0) {
		Warning(LOCATION, "Failed to load window icon '%s'!", Window_icon_path.c_str());
		return;
	}

	auto surface = bm_to_sdl_surface(icon_handle);
	if (surface == nullptr) {
		Warning(LOCATION, "Convert icon '%s' to a SDL surface!", Window_icon_path.c_str());
		bm_release(icon_handle);
		return;
	}

	SDL_SetWindowIcon(sdl_wnd, surface);

	SDL_FreeSurface(surface);
	bm_release(icon_handle);
}

bool gr_init(std::unique_ptr<os::GraphicsOperations>&& graphicsOps, int d_mode, int d_width, int d_height, int d_depth)
{
	int width = 1024, height = 768, depth = 32, mode = GR_OPENGL;
	float center_aspect_ratio = -1.0f;
	const char *ptr = NULL;
	// If already inited, shutdown the previous graphics
	if (Gr_inited) {
		switch (gr_screen.mode) {
			case GR_OPENGL:
				gr_opengl_cleanup(false);
				break;
			
			case GR_STUB:
				break;
	
			default:
				Int3();		// Invalid graphics mode
		}
	}

	// We cannot continue without this, quit, but try to help the user out first
	ptr = os_config_read_string(NULL, NOX("VideocardFs2open"), NULL);

	// if we don't have a config string then construct one, using OpenGL 1024x768 32-bit as the default
	if (ptr == NULL) {
		// If we don't have a display mode, use SDL to get default settings
		// We need to initialize SDL to do this

		if (SDL_InitSubSystem(SDL_INIT_VIDEO) == 0)
		{
			int display = static_cast<int>(os_config_read_uint("Video", "Display", 0));
			SDL_DisplayMode displayMode;
			if (SDL_GetDesktopDisplayMode(display, &displayMode) == 0)
			{
				width = displayMode.w;
				height = displayMode.h;
				int sdlBits = SDL_BITSPERPIXEL(displayMode.format);

				if (SDL_ISPIXELFORMAT_ALPHA(displayMode.format))
				{
					depth = sdlBits;
				}
				else
				{
					// Fix a few values
					if (sdlBits == 24)
					{
						depth = 32;
					}
					else if (sdlBits == 15)
					{
						depth = 16;
					}
					else
					{
						depth = sdlBits;
					}
				}

				SCP_string videomode;
				sprintf(videomode, "OGL -(%dx%d)x%d bit", width, height, depth);

				os_config_write_string(NULL, NOX("VideocardFs2open"), videomode.c_str());
			}
		}
	} else {
		Assert(ptr != NULL);

		// NOTE: The "ptr+5" is to skip over the initial "????-" in the video string.
		//       If the format of that string changes you'll have to change this too!!!
		if (sscanf(ptr + 5, "(%dx%d)x%d ", &width, &height, &depth) != 3) {
			Error(LOCATION, "Can't understand 'VideocardFs2open' config entry!");
		}
	}

	if (Cmdline_res != NULL) {
		int tmp_width = 0;
		int tmp_height = 0;

		if ( sscanf(Cmdline_res, "%dx%d", &tmp_width, &tmp_height) == 2 ) {
			width = tmp_width;
			height = tmp_height;
		}
	}
	if (Cmdline_center_res != NULL) {
		int tmp_center_width = 0;
		int tmp_center_height = 0;

		if ( (sscanf(Cmdline_center_res, "%dx%d", &tmp_center_width, &tmp_center_height) == 2) && (tmp_center_width > 0) && (tmp_center_height > 0) ) {
			center_aspect_ratio = (float)tmp_center_width / (float)tmp_center_height;
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

	if (gr_get_resolution_class(width, height) != GR_640) {
		// check for hi-res interface files so that we can verify our width/height is correct
		// if we don't have it then fall back to 640x480 mode instead
		if ( !cf_exists_full("2_ChoosePilot-m.pcx", CF_TYPE_ANY)) {
			if ( (width == 1024) && (height == 768) ) {
				width = 640;
				height = 480;
				center_aspect_ratio = -1.0f;
			} else {
				width = 800;
				height = 600;
				center_aspect_ratio = -1.0f;
			}
		}
	}

	// if we are in standalone mode then just use special defaults
	if (Is_standalone) {
		mode = GR_STUB;
		width = 640;
		height = 480;
		depth = 16;
		center_aspect_ratio = -1.0f;
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
	center_aspect_ratio = -1.0f;
	Cmdline_window = 1;
#elif defined(_FORCE_DEBUG_1024)
	width = 1024;
	height = 768;
	depth = 32;
	center_aspect_ratio = -1.0f;
	Cmdline_window = 1;
#elif defined(_FORCE_DEBUG_640)
	width = 640;
	height = 480;
	depth = 32;
	center_aspect_ratio = -1.0f;
	Cmdline_window = 1;
#endif
#endif

	if (center_aspect_ratio <= 0.0f) {
		float aspect_ratio = (float)width / (float)height;
		if (aspect_ratio > 3.5f) {
			center_aspect_ratio = aspect_ratio * 0.3f;
		} else {
			center_aspect_ratio = aspect_ratio;
		}
	}

	// now try to actually init everything...
	if ( gr_init_sub(std::move(graphicsOps), mode, width, height, depth, center_aspect_ratio) == false ) {
		return false;
	}

	gr_light_init();

	gr_set_palette_internal(Gr_current_palette_name, NULL, 0);

	bm_init();

	init_window_icon();

	io::mouse::CursorManager::init();

	mprintf(("Initializing path renderer...\n"));
	graphics::paths::PathRenderer::init();

	// Initialize uniform buffer managers
	uniform_buffer_managers_init();

	gpu_heap_init();

	bool missing_installation = false;
	if (!running_unittests && Web_cursor == nullptr) {
		if (Is_standalone) {
			// Cursors don't work in standalone mode, just check if the animation exists.
			auto handle = bm_load_animation("cursorweb");
			if (handle < 0) {
				missing_installation = true;
			} else {
				bm_release(handle);
			}
		} else {
			Web_cursor = io::mouse::CursorManager::get()->loadCursor("cursorweb", true);
			missing_installation = Web_cursor == nullptr;
		}
	}
	if (missing_installation) {
		Error(LOCATION, "\nWeb cursor bitmap not found.  This is most likely due to one of three reasons:\n"
			"    1) You're running FreeSpace Open from somewhere other than your FreeSpace 2 folder;\n"
			"    2) You've somehow corrupted your FreeSpace 2 installation, e.g. by modifying or removing the retail VP files;\n"
			"    3) You haven't installed FreeSpace 2 at all.  (Note that installing FreeSpace Open does NOT remove the need for a FreeSpace 2 installation.)\n"
			"Number 1 can be fixed by simply moving the FreeSpace Open executable file to the FreeSpace 2 folder.  Numbers 2 and 3 can be fixed by installing or reinstalling FreeSpace 2.\n");
	}

	mprintf(("GRAPHICS: Initializing default colors...\n"));

	gr_set_color(0,0,0);
	gr_set_clear_color(0, 0, 0);

	gr_set_shader(NULL);

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
		os_sleep(1000);
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

	if (active) {
		if (Cmdline_fullscreen_window||Cmdline_window) {
			os::getMainViewport()->restore();
		} else {
			os::getMainViewport()->setState(os::ViewportState::Fullscreen);
		}
	} else {
		os::getMainViewport()->minimize();
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
	c->raw8 = 0;
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

// new bitmap functions
void gr_bitmap(int _x, int _y, int resize_mode)
{
	GR_DEBUG_SCOPE("2D Bitmap");

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

	auto do_resize = gr_resize_screen_posf(&x, &y, &w, &h, resize_mode);

	x += ((do_resize) ? gr_screen.offset_x : gr_screen.offset_x_unscaled);
	y += ((do_resize) ? gr_screen.offset_y : gr_screen.offset_y_unscaled);

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
	//int saved_zbuffer_mode = gr_zbuffer_get();
	//gr_zbuffer_set(GR_ZBUFF_NONE);

	material mat_params;
	material_set_interface(
		&mat_params, 
		gr_screen.current_bitmap, 
		gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER ? true : false, 
		gr_screen.current_alpha
	);

	g3_render_primitives_textured(&mat_params, verts, 4, PRIM_TYPE_TRIFAN, true);

	//gr_zbuffer_set(saved_zbuffer_mode);
}

void gr_bitmap_uv(int _x, int _y, int _w, int _h, float _u0, float _v0, float _u1, float _v1, int resize_mode)
{
	GR_DEBUG_SCOPE("2D Bitmap UV");

	float x, y, w, h;
	vertex verts[4];

	x = i2fl(_x);
	y = i2fl(_y);
	w = i2fl(_w);
	h = i2fl(_h);

	// I will tidy this up later - RT
	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_posf(&x, &y, &w, &h, resize_mode);
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

	material material_params;
	material_set_interface(
		&material_params,
		gr_screen.current_bitmap,
		gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER ? true : false,
		gr_screen.current_alpha
	);
	g3_render_primitives_textured(&material_params, verts, 4, PRIM_TYPE_TRIFAN, true);
}

// NEW new bitmap functions -Bobboau
// void gr_bitmap_list(bitmap_2d_list* list, int n_bm, int resize_mode)
// {
// 	for (int i = 0; i < n_bm; i++) {
// 		bitmap_2d_list *l = &list[i];
// 
// 		bm_get_info(gr_screen.current_bitmap, &l->w, &l->h, NULL, NULL, NULL);
// 
// 		if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
// 			gr_resize_screen_pos(&l->x, &l->y, &l->w, &l->h, resize_mode);
// 		}
// 	}
// 
// 	g3_draw_2d_poly_bitmap_list(list, n_bm, TMAP_FLAG_INTERFACE);
// }

// _->NEW<-_ NEW new bitmap functions -Bobboau
//takes a list of rectangles that have assosiated rectangles in a texture
void gr_bitmap_list(bitmap_rect_list* list, int n_bm, int resize_mode)
{
	GR_DEBUG_SCOPE("2D Bitmap list");

	// adapted from g3_draw_2d_poly_bitmap_list

	for ( int i = 0; i < n_bm; i++ ) {
		bitmap_2d_list *l = &list[i].screen_rect;

		// if no valid hight or width values were given get some from the bitmap
		if ( (l->w <= 0) || (l->h <= 0) ) {
			bm_get_info(gr_screen.current_bitmap, &l->w, &l->h, NULL, NULL, NULL);
		}

		if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
			gr_resize_screen_pos(&l->x, &l->y, &l->w, &l->h, resize_mode);
		}
	}

	vertex* vert_list = new vertex[6 * n_bm];
	float sw = 0.1f;

	for ( int i = 0; i < n_bm; i++ ) {
		// stuff coords	

		bitmap_2d_list* b = &list[i].screen_rect;
		texture_rect_list* t = &list[i].texture_rect;
		//tri one
		vertex *V = &vert_list[i * 6];
		V->screen.xyw.x = (float)b->x;
		V->screen.xyw.y = (float)b->y;
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u0;
		V->texture_position.v = (float)t->v0;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)(b->x + b->w);
		V->screen.xyw.y = (float)b->y;
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u1;
		V->texture_position.v = (float)t->v0;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)(b->x + b->w);
		V->screen.xyw.y = (float)(b->y + b->h);
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u1;
		V->texture_position.v = (float)t->v1;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		//tri two
		V++;
		V->screen.xyw.x = (float)b->x;
		V->screen.xyw.y = (float)b->y;
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u0;
		V->texture_position.v = (float)t->v0;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)(b->x + b->w);
		V->screen.xyw.y = (float)(b->y + b->h);
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u1;
		V->texture_position.v = (float)t->v1;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)b->x;
		V->screen.xyw.y = (float)(b->y + b->h);
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u0;
		V->texture_position.v = (float)t->v1;
		V->flags = PF_PROJECTED;
		V->codes = 0;
	}

	material mat_params;
	material_set_interface(
		&mat_params,
		gr_screen.current_bitmap,
		gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER ? true : false,
		gr_screen.current_alpha
	);
	g3_render_primitives_textured(&mat_params, vert_list, 6 * n_bm, PRIM_TYPE_TRIS, true);

	delete[] vert_list;
}



/**
* Given endpoints, and thickness, calculate coords of the endpoint
* Adapted from gr_pline_helper()
*/
void gr_pline_helper(vec3d *out, vec3d *in1, vec3d *in2, int thickness)
{
	vec3d slope;

	// slope of the line
	if ( vm_vec_same(in1, in2) ) {
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
* Adapted from gr_pline_special()
*/
void gr_pline_special(SCP_vector<vec3d> *pts, int thickness, int resize_mode)
{
	vec3d s1, s2, e1, e2, dir;
	vec3d last_e1, last_e2;
	vertex v[4];
	int started_frame = 0;

	size_t num_pts = pts->size();

	// if we have less than 2 pts, bail
	if ( num_pts < 2 ) {
		return;
	}

	extern int G3_count;
	if ( G3_count == 0 ) {
		g3_start_frame(1);
		started_frame = 1;
	}

	float sw = 0.1f;

	color clr = gr_screen.current_color;

	material material_def;

	material_def.set_depth_mode(ZBUFFER_TYPE_NONE);
	material_def.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	material_def.set_cull_mode(false);

	// draw each section
	last_e1 = vmd_zero_vector;
	last_e2 = vmd_zero_vector;
	int j;
	for(size_t idx=0; idx<num_pts-1; idx++) {
		// get the start and endpoints
		s1 = pts->at(idx);													// start 1 (on the line)
		e1 = pts->at(idx + 1);												// end 1 (on the line)
		gr_pline_helper(&s2, &s1, &e1, thickness);	// start 2
		vm_vec_sub(&dir, &e1, &s1);
		vm_vec_add(&e2, &s2, &dir);											// end 2

																			// stuff coords
		v[0].screen.xyw.x = (float)ceil(s1.xyz.x);
		v[0].screen.xyw.y = (float)ceil(s1.xyz.y);
		v[0].screen.xyw.w = sw;
		v[0].texture_position.u = 0.5f;
		v[0].texture_position.v = 0.5f;
		v[0].flags = PF_PROJECTED;
		v[0].codes = 0;
		v[0].r = clr.red;
		v[0].g = clr.green;
		v[0].b = clr.blue;
		v[0].a = clr.alpha;

		v[1].screen.xyw.x = (float)ceil(s2.xyz.x);
		v[1].screen.xyw.y = (float)ceil(s2.xyz.y);
		v[1].screen.xyw.w = sw;
		v[1].texture_position.u = 0.5f;
		v[1].texture_position.v = 0.5f;
		v[1].flags = PF_PROJECTED;
		v[1].codes = 0;
		v[1].r = clr.red;
		v[1].g = clr.green;
		v[1].b = clr.blue;
		v[1].a = clr.alpha;

		v[2].screen.xyw.x = (float)ceil(e2.xyz.x);
		v[2].screen.xyw.y = (float)ceil(e2.xyz.y);
		v[2].screen.xyw.w = sw;
		v[2].texture_position.u = 0.5f;
		v[2].texture_position.v = 0.5f;
		v[2].flags = PF_PROJECTED;
		v[2].codes = 0;
		v[2].r = clr.red;
		v[2].g = clr.green;
		v[2].b = clr.blue;
		v[2].a = clr.alpha;

		v[3].screen.xyw.x = (float)ceil(e1.xyz.x);
		v[3].screen.xyw.y = (float)ceil(e1.xyz.y);
		v[3].screen.xyw.w = sw;
		v[3].texture_position.u = 0.5f;
		v[3].texture_position.v = 0.5f;
		v[3].flags = PF_PROJECTED;
		v[3].codes = 0;
		v[3].r = clr.red;
		v[3].g = clr.green;
		v[3].b = clr.blue;
		v[3].a = clr.alpha;

		//We could really do this better...but oh well. _WMC
		if ( resize_mode != GR_RESIZE_NONE ) {
			for ( j = 0; j<4; j++ ) {
				gr_resize_screen_posf(&v[j].screen.xyw.x, &v[j].screen.xyw.y, NULL, NULL, resize_mode);
			}
		}

		// draw the polys
		//g3_draw_poly_constant_sw(4, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB, 0.1f);
		g3_render_primitives_colored(&material_def, v, 4, PRIM_TYPE_TRIFAN, true);

		// if we're past the first section, draw a "patch" triangle to fill any gaps
		if ( idx > 0 ) {
			// stuff coords
			v[0].screen.xyw.x = (float)ceil(s1.xyz.x);
			v[0].screen.xyw.y = (float)ceil(s1.xyz.y);
			v[0].screen.xyw.w = sw;
			v[0].texture_position.u = 0.5f;
			v[0].texture_position.v = 0.5f;
			v[0].flags = PF_PROJECTED;
			v[0].codes = 0;
			v[0].r = clr.red;
			v[0].g = clr.green;
			v[0].b = clr.blue;
			v[0].a = clr.alpha;

			v[1].screen.xyw.x = (float)ceil(s2.xyz.x);
			v[1].screen.xyw.y = (float)ceil(s2.xyz.y);
			v[1].screen.xyw.w = sw;
			v[1].texture_position.u = 0.5f;
			v[1].texture_position.v = 0.5f;
			v[1].flags = PF_PROJECTED;
			v[1].codes = 0;
			v[1].r = clr.red;
			v[1].g = clr.green;
			v[1].b = clr.blue;
			v[1].a = clr.alpha;

			v[2].screen.xyw.x = (float)ceil(last_e2.xyz.x);
			v[2].screen.xyw.y = (float)ceil(last_e2.xyz.y);
			v[2].screen.xyw.w = sw;
			v[2].texture_position.u = 0.5f;
			v[2].texture_position.v = 0.5f;
			v[2].flags = PF_PROJECTED;
			v[2].codes = 0;
			v[2].r = clr.red;
			v[2].g = clr.green;
			v[2].b = clr.blue;
			v[2].a = clr.alpha;

			//Inefficiency or flexibility? you be the judge -WMC
			if ( resize_mode != GR_RESIZE_NONE ) {
				for ( j = 0; j<3; j++ ) {
					gr_resize_screen_posf(&v[j].screen.xyw.x, &v[j].screen.xyw.y, NULL, NULL, resize_mode);
				}
			}

			g3_render_primitives_colored(&material_def, v, 3, PRIM_TYPE_TRIFAN, true);
		}

		// store our endpoints
		last_e1 = e1;
		last_e2 = e2;
	}

	if ( started_frame ) {
		g3_end_frame();
	}
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

int poly_list::find_first_vertex_fast(int idx)
{
	uint* first_idx = std::lower_bound(sorted_indices, sorted_indices + n_verts, idx, finder(this, NULL, NULL));

	if ( first_idx == sorted_indices + n_verts ) {
		// if this happens then idx was never in the index list to begin with which is not good
		mprintf(("Sorted index list missing index %d!", idx));
		Int3();
		return idx;
	}

	return *first_idx;
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

int poly_list::find_index_fast(poly_list *plist, int idx)
{
	// searching for an out of bounds index using the finder means we're trying to find the vert and norm we're passing into the finder instance
	uint* first_idx = std::lower_bound(sorted_indices, sorted_indices + n_verts, n_verts, finder(this, &plist->vert[idx], &plist->norm[idx]));

	if (first_idx == sorted_indices + n_verts) {
		return -1;
	}

	return *first_idx;
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

	if ( submodels != NULL ) {
		vm_free(submodels);
		submodels = NULL;
	}

	if ( sorted_indices != NULL ) {
		vm_free(sorted_indices);
		sorted_indices = NULL;
	}

	if (_verts) {
		vert = (vertex*)vm_malloc(sizeof(vertex) * _verts);
		norm = (vec3d*)vm_malloc(sizeof(vec3d) * _verts);

		if (Cmdline_normal) {
			tsb = (tsb_t*)vm_malloc(sizeof(tsb_t) * _verts);
		}

		submodels = (int*)vm_malloc(sizeof(int) * _verts);

		sorted_indices = (uint*)vm_malloc(sizeof(uint) * _verts);
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

	if ( submodels != NULL ) {
		vm_free(submodels);
		submodels = NULL;
	}

	if (sorted_indices != NULL) {
		vm_free(sorted_indices);
		sorted_indices = NULL;
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
		vm_vec_cross(&cross, &norm[i], &tangent);
		scale = vm_vec_dot(&cross, &binormal);
		tsb[i].scaler = (scale < 0.0f) ? -1.0f : 1.0f;

		vm_vec_cross(&cross, &norm[i+1], &tangent);
		scale = vm_vec_dot(&cross, &binormal);
		tsb[i+1].scaler = (scale < 0.0f) ? -1.0f : 1.0f;

		vm_vec_cross(&cross, &norm[i+2], &tangent);
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
	if ( nverts_good == NULL )
		return;
    
	memset( nverts_good, 0, n_verts );

	vertex_list.reserve(n_verts);

	generate_sorted_index_list();

	for (j = 0; j < n_verts; j++) {
		if (find_first_vertex_fast(j) == j) {
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

		buffer_list_internal.submodels[z] = submodels[j];

		buffer_list_internal.n_verts++;
		z++;
	}

	Assert(nverts == buffer_list_internal.n_verts);

	if (nverts_good != NULL) {
		vm_free(nverts_good);
	}

	buffer_list_internal.generate_sorted_index_list();

	(*this) = buffer_list_internal;
}

poly_list& poly_list::operator = (const poly_list &other_list)
{
	allocate(other_list.n_verts);

	memcpy(norm, other_list.norm, sizeof(vec3d) * other_list.n_verts);
	memcpy(vert, other_list.vert, sizeof(vertex) * other_list.n_verts);

	if (Cmdline_normal) {
		memcpy(tsb, other_list.tsb, sizeof(tsb_t) * other_list.n_verts);
	}

	memcpy(submodels, other_list.submodels, sizeof(int) * other_list.n_verts);

	memcpy(sorted_indices, other_list.sorted_indices, sizeof(uint) * other_list.n_verts);

	n_verts = other_list.n_verts;

	return *this;
}

void poly_list::generate_sorted_index_list()
{
	for ( int j = 0; j < n_verts; ++j) {
		sorted_indices[j] = j;
	}

	std::sort(sorted_indices, sorted_indices + n_verts, finder(this));
}

bool poly_list::finder::operator()(const uint a, const uint b)
{
	vertex *vert_a; 
	vertex *vert_b;
	vec3d *norm_a;
	vec3d *norm_b;

	Assert(search_list != NULL);

	if ( a == (uint)search_list->n_verts ) {
		Assert(vert_to_find != NULL);
		Assert(norm_to_find != NULL);
		Assert(a != b);

		vert_a = vert_to_find;
		norm_a = norm_to_find;
	} else {
		vert_a = &search_list->vert[a];
		norm_a = &search_list->norm[a];
	}
	
	if ( b == (uint)search_list->n_verts ) {
		Assert(vert_to_find != NULL);
		Assert(norm_to_find != NULL);
		Assert(a != b);

		vert_b = vert_to_find;
		norm_b = norm_to_find;
	} else {
		vert_b = &search_list->vert[b];
		norm_b = &search_list->norm[b];
	}

	if (norm_a->xyz.x != norm_b->xyz.x) {
		return norm_a->xyz.x < norm_b->xyz.x;
	}

	if (norm_a->xyz.y != norm_b->xyz.y) {
		return norm_a->xyz.y < norm_b->xyz.y;
	}

	if (norm_a->xyz.z != norm_b->xyz.z) {
		return norm_a->xyz.z < norm_b->xyz.z;
	}

	if (vert_a->world.xyz.x != vert_b->world.xyz.x) {
		return vert_a->world.xyz.x < vert_b->world.xyz.x;
	}

	if (vert_a->world.xyz.y != vert_b->world.xyz.y) {
		return vert_a->world.xyz.y < vert_b->world.xyz.y;
	}

	if (vert_a->world.xyz.z != vert_b->world.xyz.z) {
		return vert_a->world.xyz.z < vert_b->world.xyz.z;
	}

	if (vert_a->texture_position.u != vert_b->texture_position.u) {
		return vert_a->texture_position.u < vert_b->texture_position.u;
	}

	if ( vert_a->texture_position.v != vert_b->texture_position.v ) {
		return vert_a->texture_position.v < vert_b->texture_position.v;
	}

	if ( !compare_indices ) {
		return vert_a->texture_position.v < vert_b->texture_position.v;
	} else {
		return a < b;
	}
}

void gr_shield_icon(coord2d coords[6], int resize_mode)
{
	if (gr_screen.mode == GR_STUB) {
		return;
	}
	
	g3_render_shield_icon(&gr_screen.current_color, coords, resize_mode);
}

void gr_set_bitmap(int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha)
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
}

void gr_flip(bool execute_scripting)
{
	// m!m avoid running CHA_ONFRAME when the "Quit mission" popup is shown. See mantis 2446 for reference
	if (execute_scripting && !quit_mission_popup_shown)
	{
		TRACE_SCOPE(tracing::LuaOnFrame);

		//WMC - Do conditional hooks. Yippee!
		Script_system.RunCondition(CHA_ONFRAME);
		//WMC - Do scripting reset stuff
		Script_system.EndFrame();
	}

	gr_reset_immediate_buffer();

	// Use this opportunity for retiring the uniform buffers
	uniform_buffer_managers_retire_buffers();

	gr_screen.gf_flip();
}

void gr_print_timestamp(int x, int y, fix timestamp, int resize_mode)
{
	int seconds = fl2i(f2fl(timestamp));

	// format the time information into strings
	SCP_string time;
	sprintf(time, "%.1d:%.2d:%.2d", (seconds / 3600) % 10, (seconds / 60) % 60, seconds % 60);

	gr_string(x, y, time.c_str(), resize_mode);
}

static std::unique_ptr<graphics::util::UniformBufferManager>
	uniform_buffer_managers[static_cast<size_t>(uniform_block_type::NUM_BLOCK_TYPES)];

static void uniform_buffer_managers_init() {
	for (size_t i = 0; i < static_cast<size_t>(uniform_block_type::NUM_BLOCK_TYPES); ++i) {
		auto enumVal = static_cast<uniform_block_type>(i);

		uniform_buffer_managers[i].reset(new graphics::util::UniformBufferManager(enumVal));
	}
}
static void uniform_buffer_managers_deinit() {
	for (auto& manager: uniform_buffer_managers) {
		manager.reset();
	}
}
static void uniform_buffer_managers_retire_buffers() {
	GR_DEBUG_SCOPE("Retiring unused uniform buffers");

	for (auto& manager: uniform_buffer_managers) {
		manager->retireBuffers();
	}
}

graphics::util::UniformBuffer* gr_get_uniform_buffer(uniform_block_type type) {
	return uniform_buffer_managers[static_cast<size_t>(type)]->getBuffer();
}

SCP_vector<DisplayData> gr_enumerate_displays()
{
	// It seems that linux cannot handle having the video subsystem inited
	// too late
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		return SCP_vector<DisplayData>();
	}

	SCP_vector<DisplayData> data;

	auto num_displays = SDL_GetNumVideoDisplays();
	for (auto i = 0; i < num_displays; ++i) {
		DisplayData display;
		display.index = i;

		SDL_Rect bounds;
		if (SDL_GetDisplayBounds(i, &bounds) == 0) {
			display.x = bounds.x;
			display.y = bounds.y;
			display.width = bounds.w;
			display.height = bounds.h;
		}

		auto name = SDL_GetDisplayName(i);
		if (name != nullptr) {
			display.name = name;
		}

		auto num_mods = SDL_GetNumDisplayModes(i);
		for (auto j = 0; j < num_mods; ++j) {
			SDL_DisplayMode mode;
			if (SDL_GetDisplayMode(i, j, &mode) != 0) {
				continue;
			}
			
			VideoModeData videoMode;
			videoMode.width = mode.w;
			videoMode.height = mode.h;

			int sdlBits = SDL_BITSPERPIXEL(mode.format);

			if (SDL_ISPIXELFORMAT_ALPHA(mode.format)) {
				videoMode.bit_depth = sdlBits;
			} else {
				// Fix a few values
				if (sdlBits == 24) {
					videoMode.bit_depth = 32;
				} else if (sdlBits == 15) {
					videoMode.bit_depth = 16;
				} else {
					videoMode.bit_depth = sdlBits;
				}
			}

			display.video_modes.push_back(videoMode);
		}

		data.push_back(display);
	}
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	return data;
}

namespace std {

size_t hash<vertex_format_data>::operator()(const vertex_format_data& data) const {
	size_t seed = 0;
	boost::hash_combine(seed, (size_t)data.format_type);
	boost::hash_combine(seed, data.offset);
	boost::hash_combine(seed, data.stride);
	return seed;
}
size_t hash<vertex_layout>::operator()(const vertex_layout& data) const {
	return data.hash();
}

}
bool vertex_layout::resident_vertex_format(vertex_format_data::vertex_format format_type) const {
	return ( Vertex_mask & vertex_format_data::mask(format_type) ) ? true : false;
}
void vertex_layout::add_vertex_component(vertex_format_data::vertex_format format_type, size_t stride, size_t offset) {
	// A stride value of 0 is not handled consistently by the graphics API so we must enforce that that does not happen
	Assertion(stride != 0, "The stride of a vertex component may not be zero!");

	if ( resident_vertex_format(format_type) ) {
		// we already have a vertex component of this format type
		return;
	}

	if (Vertex_mask == 0) {
		// This is the first element so we need to initialize the global stride here
		Vertex_stride = stride;
	}

	Assertion(Vertex_stride == stride, "The strides of all elements must be the same in a vertex layout!");

	Vertex_mask |= (1 << format_type);
	Vertex_components.push_back(vertex_format_data(format_type, stride, offset));
}
bool vertex_layout::operator==(const vertex_layout& other) const {
	if (Vertex_mask != other.Vertex_mask) {
		return false;
	}

	if (Vertex_components.size() != other.Vertex_components.size()) {
		return false;
	}

	return std::equal(Vertex_components.cbegin(),
					  Vertex_components.cend(),
					  other.Vertex_components.cbegin());
}
size_t vertex_layout::hash() const {
	size_t seed = 0;
	boost::hash_combine(seed, Vertex_mask);

	for (auto& comp : Vertex_components) {
		boost::hash_combine(seed, comp);
	}

	return seed;
}

static std::unique_ptr<graphics::util::GPUMemoryHeap> gpu_heaps [static_cast<size_t>(GpuHeap::NUM_VALUES)];

static void gpu_heap_init() {
	for (size_t i = 0; i < static_cast<size_t>(GpuHeap::NUM_VALUES); ++i) {
		auto enumVal = static_cast<GpuHeap>(i);

		gpu_heaps[i].reset(new graphics::util::GPUMemoryHeap(enumVal));
	}
}

static void gpu_heap_deinit() {
	for (auto& heap : gpu_heaps) {
		heap.reset();
	}
}

static graphics::util::GPUMemoryHeap* get_gpu_heap(GpuHeap heap_type) {
	Assertion(heap_type != GpuHeap::NUM_VALUES, "Invalid heap type value detected.");

	return gpu_heaps[static_cast<size_t>(heap_type)].get();
}

void gr_heap_allocate(GpuHeap heap_type, size_t size, void* data, size_t& offset_out, int& handle_out) {
	TRACE_SCOPE(tracing::GpuHeapAllocate);

	auto gpuHeap = get_gpu_heap(heap_type);

	offset_out = gpuHeap->allocateGpuData(size, data);
	handle_out = gpuHeap->bufferHandle();
}

void gr_heap_deallocate(GpuHeap heap_type, size_t data_offset) {
	TRACE_SCOPE(tracing::GpuHeapDeallocate);

	auto gpuHeap = get_gpu_heap(heap_type);

	gpuHeap->freeGpuData(data_offset);
}
