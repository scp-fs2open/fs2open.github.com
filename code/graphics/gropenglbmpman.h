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

// API specific init instructions
void gr_opengl_bm_init(int n);

// specific instructions for setting up the start of a page-in session
void gr_opengl_bm_page_in_start();

bool gr_opengl_bm_data(int n, bitmap* bm);

void gr_opengl_bm_save_render_target(int slot);
int gr_opengl_bm_make_render_target(int n, int *width, int *height, ubyte *bpp, int *mm_lvl, int flags);
int gr_opengl_bm_set_render_target(int n, int face);

#endif // _OGL_BMPMAN_H
