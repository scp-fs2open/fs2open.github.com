/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrD3DTexture.cpp $
 * $Revision: 2.2 $
 * $Date: 2003-01-19 01:07:41 $
 * $Author: bobboau $
 *
 * Code to manage loading textures into VRAM for Direct3D
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 22    9/09/99 8:53p Dave
 * Fixed multiplayer degenerate orientation case problem. Make sure warp
 * effect never goes lower than LOD 1. 
 * 
 * 21    9/05/99 11:19p Dave
 * Made d3d texture cache much more safe. Fixed training scoring bug where
 * it would backout scores without ever having applied them in the first
 * place.
 * 
 * 20    8/16/99 4:04p Dave
 * Big honking checkin.
 * 
 * 19    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 18    7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 17    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 16    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 15    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 14    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 13    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 12    2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 11    2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 10    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 9     1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 8     1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 7     1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 6     12/01/98 6:12p Johnson
 * Make sure to page in weapon impact animations as xparent textures.
 * 
 * 5     12/01/98 10:32a Johnson
 * Fixed direct3d font problems. Fixed sun bitmap problem. Fixed direct3d
 * starfield problem.
 * 
 * 4     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 3     10/09/98 2:57p Dave
 * Starting splitting up OS stuff.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 37    6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 36    5/23/98 6:12p John
 * added code to use registry to force preloading of textures or not.
 * 
 * 35    5/23/98 5:17p John
 * added reg key to set texture divider
 * 
 * 34    5/23/98 5:01p John
 * made agp preloading happen if >= 6 MB of VRAM.
 * 
 * 33    5/23/98 4:14p John
 * Added code to preload textures to video card for AGP.   Added in code
 * to page in some bitmaps that weren't getting paged in at level start.
 * 
 * 32    5/22/98 10:29p John
 * made direct3d textures scale as little as glide.
 * 
 * 31    5/22/98 12:54p John
 * forced all cards to use a max of 256 pixel wide textures, but added a
 * registry setting to disable it.
 * 
 * 30    5/21/98 9:56p John
 * Made Direct3D work with classic alpha-blending only devices, like the
 * Virge.  Added a texture type XPARENT that fills the alpha in in the
 * bitmap for Virge.   Added support for Permedia by making making
 * additive alphablending be one/one instead of alpha/one, which didn't
 * work, and there is no way to tell this from caps.
 * 
 * 29    5/20/98 10:23a John
 * put in code to fix an optimized build problem.
 * 
 * 28    5/18/98 8:26p John
 * Made cards with only 1bpp alpha fonts work.
 * 
 * 27    5/12/98 7:53p John
 * Fixed some 3dfx d3d bugs on allenders, jasen and johnson's computers
 * caused by 8:3:3:2 format being used, but not liked by the card.
 * 
 * 26    5/12/98 8:18a John
 * Put in code to use a different texture format for alpha textures and
 * normal textures.   Turned off filtering for aabitmaps.  Took out
 * destblend=invsrccolor alpha mode that doesn't work on riva128. 
 * 
 * 25    5/11/98 10:58a John
 * Fixed pilot name cursor bug.  Started adding in code for alphachannel
 * textures.
 * 
 * 24    5/09/98 12:37p John
 * More texture caching
 * 
 * 23    5/09/98 12:16p John
 * Even better texture caching.
 * 
 * 22    5/09/98 11:07a John
 * Better Direct3D texture caching
 * 
 * 21    5/08/98 5:41p John
 * 
 * 20    5/08/98 5:36p John
 * MAde texturing blink white but not crash
 * 
 * 19    5/07/98 3:02p John
 * Mpre texture cleanup.   You can now reinit d3d without a crash.
 * 
 * 18    5/07/98 11:31a John
 * Removed DEMO defines
 * 
 * 17    5/07/98 10:28a John
 * Made texture format use 4444.   Made fonts use alpha to render.
 * 
 * 16    5/07/98 9:40a John
 * Fixed some bitmap transparency issues with Direct3D.
 * 
 * 15    5/06/98 11:21p John
 * Fixed a bitmap bug with Direct3D.  Started adding new caching code into
 * D3D.
 * 
 * 14    5/06/98 8:41p John
 * Fixed some font clipping bugs.   Moved texture handle set code for d3d
 * into the texture module.
 * 
 * 13    5/03/98 10:52a John
 * Made D3D sort of work on 3dfx.
 * 
 * 12    5/03/98 10:43a John
 * Working on Direct3D.
 * 
 * 11    4/09/98 11:05a John
 * Removed all traces of Direct3D out of the demo version of Freespace and
 * the launcher.
 * 
 * 10    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 9     3/08/98 10:25a John
 * Made textures in VRAM reload if they changed
 * 
 * 8     3/07/98 8:29p John
 * Put in some Direct3D features.  Transparency on bitmaps.  Made fonts &
 * aabitmaps render nice.
 * 
 * 7     3/06/98 5:39p John
 * Started adding in aabitmaps
 * 
 * 6     3/02/98 6:00p John
 * Moved MAX_BITMAPS into BmpMan.h so the stuff in the graphics code that
 * is dependent on it won't break if it changes.   Made ModelCache slots
 * be equal to MAX_OBJECTS which is what it is.
 * 
 * 5     2/17/98 7:46p John
 * Took out debug code
 * 
 * 4     2/17/98 7:28p John
 * Got fonts and texturing working in Direct3D
 * 
 * 3     2/06/98 4:56p John
 * Turned off texturing
 * 
 * 2     2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 1     2/03/98 9:24p John
 *
 * $NoKeywords: $
 */

#include "graphics/grd3d.h"
#include "graphics/grd3dinternal.h"
#include "graphics/2d.h"
#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"
#include "io/key.h"
#include "globalincs/systemvars.h"
#include "osapi/osregistry.h"

#include "network/multi_log.h"

typedef struct tcache_slot_d3d {
	LPDIRECTDRAWSURFACE		vram_texture_surface;
	LPDIRECT3DTEXTURE2		vram_texture;
	D3DTEXTUREHANDLE			texture_handle;
	float							u_scale, v_scale;
	int							bitmap_id;
	int							size;
	char							used_this_frame;
	int							time_created;
	ushort						w, h;

	// sections
	tcache_slot_d3d			*data_sections[MAX_BMAP_SECTIONS_X][MAX_BMAP_SECTIONS_Y];
	tcache_slot_d3d			*parent;
} tcache_slot_d3d;

static void *Texture_sections = NULL;
tcache_slot_d3d *Textures = NULL;


int D3D_texture_sections = 0;
int D3D_texture_ram = 0;
int D3D_frame_count = 0;
int D3D_min_texture_width = 0;
int D3D_max_texture_width = 0;
int D3D_min_texture_height = 0;
int D3D_max_texture_height = 0;
int D3D_square_textures = 0;
int D3D_pow2_textures = 0;
int D3D_textures_in = 0;
int D3D_textures_in_frame = 0;
int D3D_last_bitmap_id = -1;
int D3D_last_detail = -1;
int D3D_last_bitmap_type = -1;
int D3D_last_section_x = -1;
int D3D_last_section_y = -1;


int vram_full = 0;

int d3d_free_texture( tcache_slot_d3d *t )
{
	int idx, s_idx;

	// Bitmap changed!!	
	if ( t->bitmap_id > -1 )	{				
		// if I, or any of my children have been used this frame, bail		
		if(t->used_this_frame){			
			return 0;
		}		
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				if((t->data_sections[idx][s_idx] != NULL) && (t->data_sections[idx][s_idx]->used_this_frame)){
					return 0;
				}
			}
		}

		// ok, now we know its legal to free everything safely
		if ( t->vram_texture )	{
			t->vram_texture->Release();
			t->vram_texture = NULL;
		}

		if ( t->vram_texture_surface )	{
			t->vram_texture_surface->Release();
			t->vram_texture_surface = NULL;
		}

		t->texture_handle = NULL;

		if ( D3D_last_bitmap_id == t->bitmap_id )	{
			D3D_last_bitmap_id = -1;
		}
			
		// if this guy has children, free them too, since the children
		// actually make up his size
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				if(t->data_sections[idx][s_idx] != NULL){
					d3d_free_texture(t->data_sections[idx][s_idx]);						
				}
			}
		}			

		t->bitmap_id = -1;
		t->used_this_frame = 0;
		D3D_textures_in -= t->size;		
	}

	return 1;
}

// we must make sure we never free my parent or any of my siblings!!!!!
int d3d_older_test(tcache_slot_d3d *new_slot, tcache_slot_d3d *test, tcache_slot_d3d *oldest)
{
	if ( (test != new_slot) && (test != new_slot->parent) && (test->bitmap_id > -1) && (!test->used_this_frame))	{
		if ( (oldest == NULL) || (test->time_created < oldest->time_created))	{
			return 1;
		}
	}

	// not older
	return 0;
}

int d3d_free_some_texture_ram(tcache_slot_d3d *t, int size)
{	
	tcache_slot_d3d *oldest = NULL;
	
	// Go through all the textures... find the oldest one 
	// that was not used this frame yet.
	int i;

	int goal_size = D3D_textures_in - size*2;
	if ( goal_size < 0 )	{
		goal_size = 0;
	} else if ( goal_size > D3D_texture_ram*3/4 )	{
		goal_size = D3D_texture_ram*3/4;
	}

	while( D3D_textures_in > goal_size )	{
		oldest = NULL;
		for( i=0; i<MAX_BITMAPS; i++ )	{			
			// maybe pick this one
			if(d3d_older_test(t, &Textures[i], oldest)){
				oldest = &Textures[i];
			}							
		}

		if ( oldest == NULL )	{
			mprintf(( "Couldn't free enough VRAM this frame... you might see some ugliness!\n" ));
			return 0;
		}

		d3d_free_texture(oldest);
	}

	mprintf(( "Freed 1/4 of the VRAM\n" ));
	return 1;
}

#ifndef NDEBUG
int Show_uploads = 0;
DCF_BOOL( show_uploads, Show_uploads )
#endif

// is the given existing texture handle the same dimensions as the passed in bitmap?
int d3d_tcache_same_dimension(int bitmap_id, int bitmap_type, tcache_slot_d3d *t)
{
	int idx, s_idx;
	int w, h, nframes, fps;
	ubyte flags;
	bitmap_section_info *s = NULL;

	// get bitmap info
	bm_get_info(bitmap_id, &w, &h, &flags, &nframes, &fps, &s);

	// bogus
	if(s == NULL){
		return 0;
	}

	// sectioned bitmap
	if(bitmap_type == TCACHE_TYPE_BITMAP_SECTION){
		for(idx=0; idx<s->num_x; idx++){
			for(s_idx=0; s_idx<s->num_y; s_idx++){				
				// bogus
				if(t->data_sections[idx][s_idx] == NULL){
					return 0;
				}

				// given a bitmap and a section, return the size (w, h)
				bm_get_section_size(bitmap_id, idx, s_idx, &w, &h);

				// same ?
				if((t->data_sections[idx][s_idx]->w != w) || (t->data_sections[idx][s_idx]->h != h)){
					return 0;
				}
			}
		}
	}
	// non-sectioned bitmap
	else {
		if((t->w != w) || (t->h != h)){
			return 0;
		}
	}

	// all good
	return 1;
}

// get the final texture size (the one which will get allocated as a surface)
void d3d_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out)
{
	int tex_w, tex_h;

	// bogus
	if((w_out == NULL) ||  (h_out == NULL)){
		return;
	}

	// starting size
	tex_w = w_in;
	tex_h = h_in;

	if ( D3D_pow2_textures )	{
		int i;
		for (i=0; i<16; i++ )	{
			if ( (tex_w > (1<<i)) && (tex_w <= (1<<(i+1))) )	{
				tex_w = 1 << (i+1);
				break;
			}
		}

		for (i=0; i<16; i++ )	{
			if ( (tex_h > (1<<i)) && (tex_h <= (1<<(i+1))) )	{
				tex_h = 1 << (i+1);
				break;
			}
		}
	}

	if ( tex_w < D3D_min_texture_width ) {
		tex_w = D3D_min_texture_width;
	} else if ( tex_w > D3D_max_texture_width )	{
		tex_w = D3D_max_texture_width;
	}

	if ( tex_h < D3D_min_texture_height ) {
		tex_h = D3D_min_texture_height;
	} else if ( tex_h > D3D_max_texture_height )	{
		tex_h = D3D_max_texture_height;
	}

	if ( D3D_square_textures )	{
		int new_size;
		// Make the both be equal to larger of the two
		new_size = max(tex_w, tex_h);
		tex_w = new_size;
		tex_h = new_size;
	}	

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

// data == start of bitmap data
// sx == x offset into bitmap
// sy == y offset into bitmap
// src_w == absolute width of section on source bitmap
// src_h == absolute height of section on source bitmap
// bmap_w == width of source bitmap
// bmap_h == height of source bitmap
// tex_w == width of final texture
// tex_h == height of final texture
int d3d_create_texture_sub(int bitmap_type, int texture_handle, ushort *data, int sx, int sy, int src_w, int src_h, int bmap_w, int bmap_h, int tex_w, int tex_h, tcache_slot_d3d *t, int reload, int fail_on_full)
{
	LPDIRECTDRAWSURFACE		sys_texture_surface = NULL;
	LPDIRECT3DTEXTURE2		sys_texture = NULL;
	int ret_val = 1;

	#ifndef NDEBUG
		if ( Show_uploads )	{
			if ( reload )	{
				mprintf(( "Reloading '%s'\n", bm_get_filename(texture_handle) ));
			} else {
				mprintf(( "Uploading '%s'\n", bm_get_filename(texture_handle) ));
			}
		}
	#endif

	// bogus
	if(t == NULL){
		return 0;
	}
		
	if ( t->used_this_frame )	{
		mprintf(( "ARGHH!!! Texture already used this frame!  Cannot free it!\n" ));
		return 0;
	}	
	if ( !reload )	{
		// gah
		if(!d3d_free_texture(t)){
			return 0;
		}
	}

	DDSURFACEDESC ddsd;
	HRESULT ddrval;
	DWORD dwHeight, dwWidth;
	int i,j;	
	ushort *bmp_data;

	DDPIXELFORMAT *surface_desc;

	switch( bitmap_type )	{
		case TCACHE_TYPE_AABITMAP:		
			surface_desc = &AlphaTextureFormat;
			break;

		case TCACHE_TYPE_XPARENT:
			Int3();

		default:
			surface_desc = &NonAlphaTextureFormat;
	}	

	// get final texture size
	d3d_tcache_get_adjusted_texture_size(tex_w, tex_h, &tex_w, &tex_h);

	if ( (tex_w < 1) || (tex_h < 1) )	{
		mprintf(("Bitmap is to small at %dx%d.\n", tex_w, tex_h ));		
		return 0;
	}

	if ( bitmap_type == TCACHE_TYPE_AABITMAP )	{
		t->u_scale = (float)bmap_w / (float)tex_w;
		t->v_scale = (float)bmap_h / (float)tex_h;
	} else if(bitmap_type == TCACHE_TYPE_BITMAP_SECTION){
		t->u_scale = (float)src_w / (float)tex_w;
		t->v_scale = (float)src_h / (float)tex_h;
	} else {
		t->u_scale = 1.0f;
		t->v_scale = 1.0f;
	}
		
	dwHeight = tex_h;
	dwWidth = tex_w;
	bmp_data = (ushort *)data;
	ubyte *bmp_data_byte = (ubyte*)data;

	// Create a surface in system memory and load texture into it.

	// Create a surface of the given format using the dimensions of the bitmap
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));

	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwHeight = dwHeight;
	ddsd.dwWidth = dwWidth;
	ddsd.ddpfPixelFormat = *surface_desc;

	sys_texture_surface = NULL;
	ddrval = lpDD->CreateSurface(&ddsd, &sys_texture_surface, NULL);
	if ( (ddrval != DD_OK) || (sys_texture_surface == NULL) ) {
		mprintf(("CreateSurface for texture failed (loadtex), w=%d, h=%d, %s\n", tex_w, tex_h, d3d_error_string(ddrval) ));
		mprintf(( "Texture RAM = %d KB\n", D3D_textures_in / 1024 ));
		// bm_unlock(bitmap_handle);
		return 0;
	}
	
	// Lock the surface so it can be filled with the bitmap data
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddrval = sys_texture_surface->Lock(NULL, &ddsd, 0, NULL);
	if (ddrval != DD_OK) {
		sys_texture_surface->Release();
		mprintf(("Lock failed while loading surface (loadtex).\n" ));
		return 0;
	}

	Assert( surface_desc->dwRGBBitCount == 16 );

	// Each RGB bit count requires different pointers
	ushort *lpSP;	
	ushort xlat[256];
	int r, g, b, a;

	switch( bitmap_type )	{		
		case TCACHE_TYPE_AABITMAP:			
			// setup convenient translation table
			for (i=0; i<16; i++ )	{
				r = 255;
				g = 255;
				b = 255;
				a = Gr_gamma_lookup[(i*255)/15];
				r /= Gr_ta_red.scale;
				g /= Gr_ta_green.scale;
				b /= Gr_ta_blue.scale;
				a /= Gr_ta_alpha.scale;
				xlat[i] = unsigned short(((a<<Gr_ta_alpha.shift) | (r << Gr_ta_red.shift) | (g << Gr_ta_green.shift) | (b << Gr_ta_blue.shift)));
			}			
			
			xlat[15] = xlat[1];			
			for ( ; i<256; i++ )	{
				xlat[i] = xlat[0];						
			}			
			
			for (j = 0; j < tex_h; j++) {				
				lpSP = (unsigned short*)(((char*)ddsd.lpSurface) +	ddsd.lPitch * j);

				for (i = 0; i < tex_w; i++) {
					if ( (i < bmap_w) && (j<bmap_h) )	{						
						*lpSP++ = xlat[(ubyte)bmp_data_byte[j*bmap_w+i]];
					} else {
						*lpSP++ = 0;
					}
				}
			}
		break;
	
	case TCACHE_TYPE_BITMAP_SECTION:	
		for (j = 0; j < src_h; j++) {
			// the proper line in the temp ram
			lpSP = (unsigned short*)(((char*)ddsd.lpSurface) +	ddsd.lPitch * j);

			// nice and clean
			for (i = 0; i < src_w; i++) {												
				// stuff the texture into vram
				*lpSP++ = bmp_data[((j+sy) * bmap_w) + sx + i];
			}			
		}
		break;

	default:		{	// normal:		
			fix u, utmp, v, du, dv;

			u = v = 0;

			du = ( (bmap_w-1)*F1_0 ) / tex_w;
			dv = ( (bmap_h-1)*F1_0 ) / tex_h;
											
			for (j = 0; j < tex_h; j++) {
				lpSP = (unsigned short*)(((char*)ddsd.lpSurface) +	ddsd.lPitch * j);

				utmp = u;				
				
				for (i = 0; i < tex_w; i++) {
					*lpSP++ = bmp_data[f2i(v)*bmap_w+f2i(utmp)];
					utmp += du;
				}
				v += dv;
			}
		}
		break;
	}

	// bm_unlock(bitmap_handle);

	// Unlock the texture 
	sys_texture_surface->Unlock(NULL);

	sys_texture = NULL;
	ddrval = sys_texture_surface->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&sys_texture);
	if ( (ddrval != DD_OK) || (sys_texture == NULL) ) {
		mprintf(( "Getting sys surface's texture failed!\n" ));

		// bad return value
		ret_val = 0;

		goto FreeSurfacesAndExit;
	}

RetryLoad:

	if ( !reload )	{	
		// Create a surface of the given format using the dimensions of the bitmap
		memset(&ddsd, 0, sizeof(DDSURFACEDESC));

		ddsd.dwSize = sizeof(DDSURFACEDESC);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD | DDSCAPS_VIDEOMEMORY;
		//| DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_ALLOCONLOAD;
		ddsd.dwHeight = dwHeight;
		ddsd.dwWidth = dwWidth;
		ddsd.ddpfPixelFormat = *surface_desc;

		t->vram_texture_surface = NULL;
		ddrval = lpDD->CreateSurface(&ddsd, &t->vram_texture_surface, NULL);
		if ( (ddrval != DD_OK) || (t->vram_texture_surface == NULL) ) {
			t->vram_texture = NULL;
			t->vram_texture_surface = NULL;
			t->texture_handle = NULL;

			if ( ddrval==DDERR_OUTOFVIDEOMEMORY )	{
				mprintf(("Out of VRAM (w=%d, h=%d, used=%d KB)\n", tex_w, tex_h, D3D_textures_in / 1024 ));
				if ( fail_on_full )	{
					// bad return value
					ret_val = 0;

					goto FreeSurfacesAndExit;
				}
				if ( d3d_free_some_texture_ram(t,dwHeight*dwWidth*2))	{
					goto RetryLoad;
				}
			} else {
				mprintf(("CreateSurface for VRAM texture failed, w=%d, h=%d\n%s\n", tex_w, tex_h, d3d_error_string(ddrval) ));
				mprintf(( "Texture RAM = %d KB\n", D3D_textures_in / 1024 ));
			}
			vram_full = 1;

			// bad return value
			ret_val = 0;

			goto FreeSurfacesAndExit;
			//goto RetryLoad;
		}

		t->vram_texture = NULL;
		ddrval = t->vram_texture_surface->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&t->vram_texture);
		if ( (ddrval != DD_OK) || (t->vram_texture == NULL) )	{
			mprintf(( "GR_D3D_INIT: TextureSurface->QueryInterface failed.\n" ));
			vram_full = 1;

			// bad return value
			ret_val = 0;

			goto FreeSurfacesAndExit;
		}

		//	char *name = bm_get_filename(bitmap_handle);
		//	mprintf(( "Uploading '%s'\n", name ));
		t->texture_handle = NULL;
		ddrval = t->vram_texture->GetHandle(lpD3DDevice, &t->texture_handle );
		if ( (ddrval != DD_OK) || (t->texture_handle == NULL) )	{
			mprintf(( "GR_D3D_INIT: Texture->GetHandle failed.\n" ));
			t->texture_handle = NULL;
			vram_full = 1;

			// bad return value
			ret_val = 0;

			goto FreeSurfacesAndExit;
		}
	}

	// argh. this texture appears to be bogus. lets free it
	if(t->vram_texture == NULL){
		d3d_free_texture(t);
			
		// bad
		ret_val = 0;
		
		goto FreeSurfacesAndExit;
	}

	ddrval = t->vram_texture->Load( sys_texture );
	if ( ddrval != DD_OK ) {
		mprintf(("VRAM Load failed, w=%d, h=%d, %s\n", tex_w, tex_h, d3d_error_string(ddrval) ));
		mprintf(( "Texture RAM = %d KB\n", D3D_textures_in / 1024 ));
		vram_full = 1;

		// bad return value
		ret_val = 0;

		goto FreeSurfacesAndExit;
	}

	t->bitmap_id = texture_handle;
	t->time_created = D3D_frame_count;
	t->used_this_frame = 0;	
	t->size = tex_w * tex_h * 2;	
	t->w = (ushort)tex_w;
	t->h = (ushort)tex_h;
	D3D_textures_in_frame += t->size;
	if ( !reload )	{	
		D3D_textures_in += t->size;
	}

FreeSurfacesAndExit:
	if ( sys_texture )	{
		sys_texture->Release();
		sys_texture = NULL;
	}

	if ( sys_texture_surface )	{
		sys_texture_surface->Release();
		sys_texture_surface = NULL;
	}

	// hopefully this is 1  :)
	return ret_val;
}

int d3d_create_texture(int bitmap_handle, int bitmap_type, tcache_slot_d3d *tslot, int fail_on_full)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	ubyte bpp = 16;
	int reload = 0;

	// setup texture/bitmap flags
	flags = 0;
	switch(bitmap_type){
	case TCACHE_TYPE_AABITMAP:
		flags |= BMP_AABITMAP;
		bpp = 8;
		break;
	case TCACHE_TYPE_NORMAL:
		flags |= BMP_TEX_OTHER;
	case TCACHE_TYPE_XPARENT:
		flags |= BMP_TEX_XPARENT;				
		break;
	case TCACHE_TYPE_NONDARKENING:		
		Int3();
		flags |= BMP_TEX_NONDARK;
		break;
	}
	
	// lock the bitmap into the proper format
	bmp = bm_lock(bitmap_handle, bpp, flags);
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));
		return 0;
	}

	int max_w = bmp->w;
	int max_h = bmp->h; 
		
	if ( bitmap_type != TCACHE_TYPE_AABITMAP )	{
		max_w /= D3D_texture_divider;
		max_h /= D3D_texture_divider;

		// Detail.debris_culling goes from 0 to 4.
		max_w /= 16 >> Detail.hardware_textures;
		max_h /= 16 >> Detail.hardware_textures;
	}

	// get final texture size as it will be allocated as a DD surface
	d3d_tcache_get_adjusted_texture_size(max_w, max_h, &final_w, &final_h);

	// if this tcache slot has no bitmap
	if ( tslot->bitmap_id < 0) {			
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->bitmap_id != bitmap_handle)	{
		if((final_w == tslot->w) && (final_h == tslot->h)){
			reload = 1;

			ml_printf("Reloading texture %d\n", bitmap_handle);
		} else {
			reload = 0;
		}
	}

	// call the helper
	int ret_val = d3d_create_texture_sub(bitmap_type, bitmap_handle, (ushort*)bmp->data, 0, 0, bmp->w, bmp->h, bmp->w, bmp->h, max_w, max_h, tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}

int d3d_create_texture_sectioned(int bitmap_handle, int bitmap_type, tcache_slot_d3d *tslot, int sx, int sy, int fail_on_full)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	int section_x, section_y;
	int reload = 0;

	// setup texture/bitmap flags
	Assert(bitmap_type == TCACHE_TYPE_BITMAP_SECTION);
	if(bitmap_type != TCACHE_TYPE_BITMAP_SECTION){
		bitmap_type = TCACHE_TYPE_BITMAP_SECTION;
	}
	flags = BMP_TEX_XPARENT;	
		
	// lock the bitmap in the proper format
	bmp = bm_lock(bitmap_handle, 16, flags);	
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));
		return 0;
	}	

	// determine the width and height of this section
	bm_get_section_size(bitmap_handle, sx, sy, &section_x, &section_y);	

	// get final texture size as it will be allocated as a DD surface
	d3d_tcache_get_adjusted_texture_size(section_x, section_y, &final_w, &final_h);

	// if this tcache slot has no bitmap
	if ( tslot->bitmap_id < 0) {					
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->bitmap_id != bitmap_handle)	{		
		if((final_w == tslot->w) && (final_h == tslot->h)){			
			reload = 1;			
		} else {			
			reload = 0;
		}
	}

	// call the helper
	int ret_val = d3d_create_texture_sub(bitmap_type, bitmap_handle, (ushort*)bmp->data, bmp->sections.sx[sx], bmp->sections.sy[sy], section_x, section_y, bmp->w, bmp->h, section_x, section_y, tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}

extern int bm_get_cache_slot( int bitmap_id, int separate_ani_frames );
int d3d_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, int sx, int sy, int force )
{
	bitmap *bmp = NULL;
	int idx, s_idx;	
	int ret_val = 1;

	if ( bitmap_id < 0 )	{
		D3D_last_bitmap_id  = -1;
		return 0;
	}

	if ( D3D_last_detail != Detail.hardware_textures )	{
		D3D_last_detail = Detail.hardware_textures;
		d3d_tcache_flush();
	}

	//mprintf(( "Setting texture %d\n", bitmap_handle ));
	if ( vram_full ) {
		return 0;
	}

	int n = bm_get_cache_slot( bitmap_id, 1 );
//	if(n == -1)return -1;//for a bad handel, this was triping some assertion errors in the glowmap code
	tcache_slot_d3d * t = &Textures[n];		
	
	if ( (D3D_last_bitmap_id == bitmap_id) && (D3D_last_bitmap_type==bitmap_type) && (t->bitmap_id == bitmap_id) && (D3D_last_section_x == sx) && (D3D_last_section_y == sy))	{
		t->used_this_frame++;
		
		// mark all children as used
		if(D3D_texture_sections){
			for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
				for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
					if(t->data_sections[idx][s_idx] != NULL){
						t->data_sections[idx][s_idx]->used_this_frame++;
					}
				}
			}
		}

		*u_scale = t->u_scale;
		*v_scale = t->v_scale;
		return 1;
	}	

	// if this is a sectioned bitmap
	if(bitmap_type == TCACHE_TYPE_BITMAP_SECTION){		
		Assert((sx >= 0) && (sy >= 0) && (sx < MAX_BMAP_SECTIONS_X) && (sy < MAX_BMAP_SECTIONS_Y));
		if(!((sx >= 0) && (sy >= 0) && (sx < MAX_BMAP_SECTIONS_X) && (sy < MAX_BMAP_SECTIONS_Y))){
			return 0;
		}		

		ret_val = 1;

		// if the texture sections haven't been created yet
		if((t->bitmap_id < 0) || (t->bitmap_id != bitmap_id)){						
			// lock the bitmap in the proper format
			bmp = bm_lock(bitmap_id, 16, BMP_TEX_XPARENT);	
			bm_unlock(bitmap_id);			
			
			// now lets do something for each texture			
			for(idx=0; idx<bmp->sections.num_x; idx++){
				for(s_idx=0; s_idx<bmp->sections.num_y; s_idx++){																				
					// hmm. i'd rather we didn't have to do it this way...
					if(!d3d_create_texture_sectioned(bitmap_id, bitmap_type, t->data_sections[idx][s_idx], idx, s_idx, fail_on_full)){
						ret_val = 0;
					}

					// not used this frame
					t->data_sections[idx][s_idx]->used_this_frame = 0;
				}
			}

			// zero out pretty much everything in the parent struct since he's just the root
			t->bitmap_id = bitmap_id;			
			t->texture_handle = NULL;
			t->time_created = t->data_sections[sx][sy]->time_created;
			t->used_this_frame = 0;
			t->vram_texture = NULL;
			t->vram_texture_surface = NULL;
		}		

		// argh. we failed to upload. free anything we can
		if(!ret_val){
			d3d_free_texture(t);
		}
		// swap in the texture we want				
		else {			
			t = t->data_sections[sx][sy];
		}
	}
	// all other "normal" textures
	else if((bitmap_id < 0) || (bitmap_id != t->bitmap_id)){		
		ret_val = d3d_create_texture( bitmap_id, bitmap_type, t, fail_on_full );		
	}			

	// everything went ok
	if(ret_val && (t->texture_handle != NULL) && !vram_full){
		*u_scale = t->u_scale;
		*v_scale = t->v_scale;

		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, t->texture_handle );
		
		D3D_last_bitmap_id = t->bitmap_id;
		D3D_last_bitmap_type = bitmap_type;
		D3D_last_section_x = sx;
		D3D_last_section_y = sy;

		t->used_this_frame++;		
	}
	// gah
	else {			
		return 0;
	}	
	
	return 1;
}

int D3D_should_preload = 0;

void d3d_tcache_init(int use_sections)
{
	int i, idx, s_idx;

	D3D_should_preload = 0;

	{
		DDSCAPS ddsCaps;
		DWORD dwFree, dwTotal;

		memset(&ddsCaps,0,sizeof(ddsCaps) );
		ddsCaps.dwCaps = DDSCAPS_TEXTURE;
		HRESULT ddrval = lpDD->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree);
		if ( ddrval != DD_OK )	{
			mprintf(( "GR_D3D_INIT: GetAvailableVidMem failed.\n" ));
			dwFree = 0;
//			goto D3DError;
		}

		D3D_texture_ram = dwFree;

		uint tmp_pl = os_config_read_uint( NULL, NOX("D3DPreloadTextures"), 255 );

		if ( tmp_pl == 0 )	{
			D3D_should_preload = 0;
		} else if ( tmp_pl == 1 )	{
			D3D_should_preload = 1;
		} else {
			if ( D3D_texture_ram >= 6*1024*1024  )	{
				D3D_should_preload = 1;
			} else {
				D3D_should_preload = 0;
			}
		}

		/*
		int megs = dwFree / (1024*1024);		
		uint tmp_val = os_config_read_uint( NULL, NOX("D3DTextureDivider"), 255 );

		if ( tmp_val == 0 )	{
			// auto set
			if ( megs <= 4 )	{
				D3D_texture_divider = 4;
			} else if ( megs <=8 )	{
				D3D_texture_divider = 3;
			} else if ( megs <=16 )	{
				D3D_texture_divider = 2;
			} else {
				D3D_texture_divider = 1;
			}
		} else if ( tmp_val < 5 )	{
			D3D_texture_divider = tmp_val;
		} else {
			// force to 4
			D3D_texture_divider = 4;
		}
		*/

		// setup texture divider
		uint tmp_val = os_config_read_uint( NULL, NOX("D3DFast"), 1 );
		D3D_texture_divider = 1;
		if(tmp_val){
			D3D_texture_divider = 4;
		}			

#ifndef NDEBUG
		int megs = dwFree / (1024*1024);		
		mprintf(( "TEXTURE RAM: %d bytes (%d MB) available, size divisor = %d\n", dwFree, megs, D3D_texture_divider ));
#endif
	}

	
  	D3D_min_texture_width = (int)lpDevDesc->dwMinTextureWidth;
	D3D_max_texture_width = (int)lpDevDesc->dwMaxTextureWidth;
	D3D_min_texture_height = (int)lpDevDesc->dwMinTextureHeight;
	D3D_max_texture_height = (int)lpDevDesc->dwMaxTextureHeight;

	// The following checks are needed because the PowerVR reports
	// it's texture caps as 0,0 up to 0,0.
	if ( D3D_min_texture_width < 16 )	{
		D3D_min_texture_width = 16;
	}
	if ( D3D_min_texture_height < 16 )	{
		D3D_min_texture_height = 16;
	}
	if ( D3D_max_texture_width < 16 )	{
		mprintf(( "Driver claims to have a max texture width of %d.  Bashing to 256.\n(Is this a PowerVR?)\n", D3D_max_texture_width ));
		D3D_max_texture_width = 256;					// Can we assume 256?
	}

	if ( D3D_max_texture_height < 16 )	{
		mprintf(( "Driver claims to have a max texture height of %d.  Bashing to 256.\n(Is this a PowerVR?)\n", D3D_max_texture_height ));
		D3D_max_texture_height = 256;					// Can we assume 256?
	}

	if (lpDevDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)	{
		mprintf(( "Darn. Driver needs square textures.\n" ));
		D3D_square_textures = 1;
	} else {
		mprintf(( "Woohoo! Driver doesn't need square textures!\n" ));
		D3D_square_textures = 0;
	}

	if ( lpDevDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2 )	{
		mprintf(( "Textures must be power of 2.\n" ));
		D3D_pow2_textures = 1;
	} else {
		mprintf(( "Textures can be any size.\n" ));
		D3D_pow2_textures = 0;
	}


	if ( !(DD_driver_caps.dwCaps2 & DDCAPS2_WIDESURFACES) )	{
		if ( D3D_max_texture_width > gr_screen.max_w )	{
			D3D_max_texture_width = gr_screen.max_w;

			if ( D3D_pow2_textures )	{
				for (i=0; i<16; i++ )	{
					if ( (D3D_max_texture_width>= (1<<i)) && (D3D_max_texture_width < (1<<(i+1))) )	{
						D3D_max_texture_width = 1 << i;
						break;
					}
				}
			}

			if ( D3D_max_texture_height > D3D_max_texture_width )	{
				D3D_max_texture_height = D3D_max_texture_width;
			}

			mprintf(( "Doesn't support wide surfaces. Bashing max down to %d\n", D3D_max_texture_width ));
		}
	}

	if ( !os_config_read_uint( NULL, NOX("D3DUseLargeTextures"), 0 ))	{
		mprintf(( "No large texture flag specified, so using max of 256\n" ));
		if ( D3D_max_texture_width > 256 )	{
			D3D_max_texture_width = 256;
		}

		if ( D3D_max_texture_height > 256 )	{
			D3D_max_texture_height = 256;
		}
	} else {
		mprintf(( "Large textures enabled!\n" ));
	}
	
	Textures = (tcache_slot_d3d *)malloc(MAX_BITMAPS*sizeof(tcache_slot_d3d));
	if ( !Textures )	{
		exit(1);
	}

	if(use_sections){
		Texture_sections = (tcache_slot_d3d*)malloc(MAX_BITMAPS * MAX_BMAP_SECTIONS_X * MAX_BMAP_SECTIONS_Y * sizeof(tcache_slot_d3d));
		if(!Texture_sections){
			exit(1);
		}
		memset(Texture_sections, 0, MAX_BITMAPS * MAX_BMAP_SECTIONS_X * MAX_BMAP_SECTIONS_Y * sizeof(tcache_slot_d3d));
	}

	// Init the texture structures
	int section_count = 0;
	for( i=0; i<MAX_BITMAPS; i++ )	{
		Textures[i].vram_texture = NULL;
		Textures[i].vram_texture_surface = NULL;
		Textures[i].texture_handle = NULL;

		Textures[i].bitmap_id = -1;
		Textures[i].size = 0;
		Textures[i].used_this_frame = 0; 

		Textures[i].parent = NULL;

		// allocate sections
		if(use_sections){
			for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
				for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
					Textures[i].data_sections[idx][s_idx] = &((tcache_slot_d3d*)Texture_sections)[section_count++];
					Textures[i].data_sections[idx][s_idx]->parent = &Textures[i];
					
					Textures[i].data_sections[idx][s_idx]->vram_texture = NULL;
					Textures[i].data_sections[idx][s_idx]->vram_texture_surface = NULL;
					Textures[i].data_sections[idx][s_idx]->texture_handle = NULL;

					Textures[i].data_sections[idx][s_idx]->bitmap_id = -1;
					Textures[i].data_sections[idx][s_idx]->size = 0;
					Textures[i].data_sections[idx][s_idx]->used_this_frame = 0;
				}
			}
		} else {
			for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
				for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
					Textures[i].data_sections[idx][s_idx] = NULL;					
				}
			}
		}
	}

	D3D_texture_sections = use_sections;

	D3D_last_detail = Detail.hardware_textures;
	D3D_last_bitmap_id = -1;
	D3D_last_bitmap_type = -1;

	D3D_last_section_x = -1;
	D3D_last_section_y = -1;

	D3D_textures_in = 0;
	D3D_textures_in_frame = 0;

}

void d3d_tcache_flush()
{
	int i; 


	for( i=0; i<MAX_BITMAPS; i++ )	{
		d3d_free_texture( &Textures[i] );		
	}
	if ( D3D_textures_in != 0 )	{
		mprintf(( "WARNING: VRAM is at %d instead of zero after flushing!\n", D3D_textures_in ));
		D3D_textures_in = 0;
	}

	D3D_last_bitmap_id = -1;
	D3D_last_section_x = -1;
	D3D_last_section_y = -1;
}

void d3d_tcache_cleanup()
{
	d3d_tcache_flush();
	
	D3D_textures_in = 0;
	D3D_textures_in_frame = 0;

	if ( Textures )	{
		free(Textures);
		Textures = NULL;
	}

	if(Texture_sections != NULL){
		free(Texture_sections);
		Texture_sections = NULL;
	}
}

void d3d_tcache_frame()
{
	int idx, s_idx;

	D3D_last_bitmap_id = -1;
	D3D_textures_in_frame = 0;

	D3D_frame_count++;

	int i;
	for( i=0; i<MAX_BITMAPS; i++ )	{
		Textures[i].used_this_frame = 0; 

		// data sections
		if(Textures[i].data_sections[0][0] != NULL){
			Assert(D3D_texture_sections);
			if(D3D_texture_sections){				
				for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
					for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
						if(Textures[i].data_sections[idx][s_idx] != NULL){
							Textures[i].data_sections[idx][s_idx]->used_this_frame = 0;
						}
					}
				}
			}
		}
	}

	if ( vram_full )	{
		d3d_tcache_flush();
		vram_full = 0;
	}
}


void gr_d3d_preload_init()
{
	if ( gr_screen.mode != GR_DIRECT3D )	{
		return;
	}
	d3d_tcache_flush();
}

int gr_d3d_preload(int bitmap_num, int is_aabitmap)
{
	if ( gr_screen.mode != GR_DIRECT3D )	{
		return 0;
	}

	if ( !D3D_should_preload )	{
		return 0;
	}

	float u_scale, v_scale;

	int retval;
	
	if ( is_aabitmap )	{
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale, 1 );
	} else {
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 1 );
	}

	if ( !retval )	{
		mprintf(("Texture upload failed!\n" ));
	}

	return retval;
}

// call this to safely fill in the texture shift and scale values for the specified texture type (Gr_t_*)
void gr_d3d_get_tex_format(int alpha)
{
	/*
	DDPIXELFORMAT *surface_desc;
	int s;	
	// RGB decoder
	unsigned long m;		

	// get the proper texture format
	if(alpha){	
		surface_desc = &AlphaTextureFormat;
	} else {	
		surface_desc = &NonAlphaTextureFormat;
	}

	// Determine the red, green and blue masks' shift and scale.
	for (s = 0, m = surface_desc->dwRBitMask; !(m & 1); s++, m >>= 1);
	Gr_t_red_shift = s;
	Gr_t_red_scale = 255 / (surface_desc->dwRBitMask >> s);
	for (s = 0, m = surface_desc->dwGBitMask; !(m & 1); s++, m >>= 1);
	Gr_t_green_shift = s;
	Gr_t_green_scale = 255 / (surface_desc->dwGBitMask >> s);
	for (s = 0, m = surface_desc->dwBBitMask; !(m & 1); s++, m >>= 1);
	Gr_t_blue_shift = s;
	Gr_t_blue_scale = 255 / (surface_desc->dwBBitMask >> s);

	if ( surface_desc->dwFlags & DDPF_ALPHAPIXELS ) {
		for (s = 0, m = surface_desc->dwRGBAlphaBitMask; !(m & 1); s++, m >>= 1);
		Gr_t_alpha_shift = s;
		Gr_t_alpha_scale = 255 / (surface_desc->dwRGBAlphaBitMask >> s);
	} else {
		Gr_t_alpha_shift = 0;
		Gr_t_alpha_scale = 256;
	}
	*/
}
