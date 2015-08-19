/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _OGL_BMPMAN_H
#define _OGL_BMPMAN_H

#include "bmpman/bmpman.h"
#include "globalincs/pstypes.h"


// anything API specific to freeing bm data
void gr_opengl_bm_free_data(int n, bool release);

// API specifics for creating a user bitmap
void gr_opengl_bm_create(int n);

// Load an image and validate it while retrieving information for later use
// Input:	type		= current BM_TYPE_*
//			n			= location in bm_bitmaps[]
//			filename	= name of the current file
//			img_cfp		= already open CFILE handle, if available
//
// Output:	w			= bmp width
//			h			= bmp height
//			bpp			= bmp bits per pixel
//			c_type		= output for an updated BM_TYPE_*
//			mm_lvl		= number of mipmap levels for the image
//			size		= size of the data contained in the image
int gr_opengl_bm_load(BM_TYPE type, int n, const char *filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, BM_TYPE *c_type = 0, int *mm_lvl = 0, int *size = 0);

// API specific init instructions
void gr_opengl_bm_init(int n);

// specific instructions for setting up the start of a page-in session
void gr_opengl_bm_page_in_start();

// Lock an image files data into memory
int gr_opengl_bm_lock(const char *filename, int handle, int bitmapnum, ubyte bpp, ubyte flags, bool nodebug);

void gr_opengl_bm_save_render_target(int slot);
int gr_opengl_bm_make_render_target(int n, int *width, int *height, ubyte *bpp, int *mm_lvl, int flags);
int gr_opengl_bm_set_render_target(int n, int face);

#endif // _OGL_BMPMAN_H
