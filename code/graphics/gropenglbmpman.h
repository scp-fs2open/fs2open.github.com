/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Graphics/gropenglbmpman.h $
 * $Revision: 1.1 $
 * $Date: 2004-10-31 21:21:11 $
 * $Author: taylor $
 *
 * OpenGL specific bmpman routines
 *
 * $Log: not supported by cvs2svn $
 *
 * $NoKeywords: $
 */

#ifndef _OGL_BMPMAN_H
#define _OGL_BMPMAN_H

#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"


// anything API specific to freeing bm data
void gr_opengl_bm_free_data(int n);

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
int gr_opengl_bm_load(ubyte type, int n, char *filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *c_type = 0, int *mm_lvl = 0, int *size = 0);

// API specific init instructions
void gr_opengl_bm_init(int n);

// specific instructions for setting up the start of a page-in session
void gr_opengl_bm_page_in_start();

// Lock an image files data into memory
int gr_opengl_bm_lock(char *filename, int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags);


#endif // _OGL_BMPMAN_H
