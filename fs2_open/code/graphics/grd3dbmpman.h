/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "PreProcDefines.h"
#ifndef _GRD3DBMPMAN_H
#define _GRD3DBMPMAN_H

#ifndef NDEBUG
#define BMPMAN_NDEBUG
#endif


struct CFILE;


typedef struct {
	IDirect3DBaseTexture8 *tinterface;
	float uscale, vscale;
} D3DBitmapData;



void gr_d3d_bm_free_data(int n);
void gr_d3d_bm_create(int n);
int gr_d3d_bm_load(ubyte type, int n, char *filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *c_type = 0, int *mm_lvl = 0, int *size = 0);
void gr_d3d_bm_init(int n);
void gr_d3d_bm_page_in_start();
int gr_d3d_bm_lock(char *filename, int handle, int bitmapnum, ubyte bpp, ubyte flags);


#endif
