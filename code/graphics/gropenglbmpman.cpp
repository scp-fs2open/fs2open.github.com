/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "cmdline/cmdline.h"
#include "ddsutils/ddsutils.h"
#include "globalincs/systemvars.h"
#include "graphics/gropenglbmpman.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropenglstate.h"
#include "graphics/gropengltexture.h"
#include "jpgutils/jpgutils.h"
#include "pcxutils/pcxutils.h"
#include "pngutils/pngutils.h"
#include "tgautils/tgautils.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"



static inline int is_power_of_two(int w, int h)
{
	// NOTE: OpenGL texture code has a min tex size of 16 (currently), so we need to be at least
	//       the min size here to qualify as power-of-2 and not get resized later on
	return ( ((w >= GL_min_texture_width) && !(w & (w-1))) && ((h >= GL_min_texture_height) && !(h & (h-1))) );
}

int get_num_mipmap_levels(int w, int h)
{
	int size, levels = 0;

	// make sure we can and should generate mipmaps before trying to use them
	if ( !Cmdline_mipmap || !Is_Extension_Enabled(OGL_SGIS_GENERATE_MIPMAP) )
		return 1;

	size = MAX(w, h);

	while (size > 0) {
		size >>= 1;
		levels++;
	}

	return (levels > 1) ? levels : 1;
}

/**
 * Anything API specific to freeing bm data
 */
void gr_opengl_bm_free_data(int n, bool release)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	// might as well free up the on card texture data too in order to get rid
	// of old interface stuff but don't free USER types, unless it's a total release,
	// since we can reuse ANI slots for faster and less resource intensive rendering
	if ( release || (bm_bitmaps[n].type != BM_TYPE_USER) )
		opengl_free_texture_slot( n );

	if ( (bm_bitmaps[n].type == BM_TYPE_RENDER_TARGET_STATIC) || (bm_bitmaps[n].type == BM_TYPE_RENDER_TARGET_DYNAMIC) )
		opengl_kill_render_target( n );
}

/**
 * API specifics for creating a user bitmap
 */
void gr_opengl_bm_create(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );
}

/**
 * API specific init instructions
 */
void gr_opengl_bm_init(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );
}

/**
 * Specific instructions for setting up the start of a page-in session
 */
void gr_opengl_bm_page_in_start()
{
	opengl_preload_init();
}

extern void bm_clean_slot(int n);

extern bool opengl_texture_slot_valid(int n, int handle);

void gr_opengl_bm_save_render_target(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	if ( !Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) || Cmdline_no_fbo ) {
		return;
	}

	bitmap_entry *be = &bm_bitmaps[n];
	bitmap *bmp = &be->bm;

	size_t rc = opengl_export_render_target( n, bmp->w, bmp->h, (bmp->true_bpp == 32), be->num_mipmaps, (ubyte*)bmp->data );

	if (rc != be->mem_taken) {
		Int3();
		return;
	}

	if (Cmdline_save_render_targets) {
		dds_save_image(bmp->w, bmp->h, bmp->true_bpp, be->num_mipmaps, (ubyte*)bmp->data, (bmp->flags & BMP_FLAG_CUBEMAP));
	}
}

int gr_opengl_bm_make_render_target(int n, int *width, int *height, ubyte *bpp, int *mm_lvl, int flags)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	if ( !Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) || Cmdline_no_fbo ) {
		return 0;
	}

	if ( (flags & BMP_FLAG_CUBEMAP) && !Is_Extension_Enabled(OGL_ARB_TEXTURE_CUBE_MAP) ) {
		return 0;
	}

	if ( (flags & BMP_FLAG_CUBEMAP) && (*width != *height) ) {
		MIN(*width, *height) = MAX(*width, *height);
	}

	// Only enforce power of two size if not supported
	if (!(Is_Extension_Enabled(OGL_ARB_TEXTURE_NON_POWER_OF_TWO)))
	{
		Assert( is_power_of_two(*width, *height) );
	}

	if ( opengl_make_render_target(bm_bitmaps[n].handle, n, width, height, bpp, mm_lvl, flags) ) {
		return 1;
	}

	return 0;
}

int gr_opengl_bm_set_render_target(int n, int face)
{
	if ( !Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) || Cmdline_no_fbo ) {
		return 0;
	}

	if (n == -1) {
		opengl_set_render_target(-1);
		return 1;
	}

	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	int is_static = (bm_bitmaps[n].type == BM_TYPE_RENDER_TARGET_STATIC);

	if ( opengl_set_render_target(n, face, is_static) ) {
		return 1;
	}

	return 0;
}

bool gr_opengl_bm_data(int n, bitmap* bm)
{
	// Do nothing here
	return true;
}
