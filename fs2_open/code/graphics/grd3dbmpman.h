/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef _GRD3DBMPMAN_H
#define _GRD3DBMPMAN_H

#ifndef NDEBUG
#define BMPMAN_NDEBUG
#endif


struct CFILE;


typedef struct {
	IDirect3DBaseTexture8 *tinterface;
	IDirect3DBaseTexture8 *backup_tinterface;
	float uscale, vscale;
	int flags;
	int x;	//these are only used for rebuilding default mem pool textures
	int y;
} D3DBitmapData;

#define DXT_DEFAULT_MEM_POOL	(1<<0)
#define DXT_DYNAMIC				(1<<1)
#define DXT_STATIC				(1<<2)
#define DXT_CUBEMAP				(1<<3)

void gr_d3d_bm_free_data(int n);
void gr_d3d_bm_create(int n);
int gr_d3d_bm_load(ubyte type, int n, char *filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *c_type = 0, int *mm_lvl = 0, int *size = 0);
void gr_d3d_bm_init(int n);
void gr_d3d_bm_page_in_start();
int gr_d3d_bm_lock(char *filename, int handle, int bitmapnum, ubyte bpp, ubyte flags);

bool gr_d3d_bm_make_render_target(int n, int &x, int &y, int flags);
bool gr_d3d_bm_set_render_target(int n, int face);

IDirect3DBaseTexture8* get_render_target_texture(int);

#endif
