/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef NO_DIRECT3D

#include <D3dx8tex.h>

#include "graphics/grd3d.h"
#include "graphics/grd3dinternal.h"
#include "graphics/2d.h"
#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"
#include "globalincs/systemvars.h"
#include "osapi/osregistry.h"
#include "debugconsole/dbugfile.h"
#include "graphics/grd3dbmpman.h"
#include "ddsutils/ddsutils.h"


extern int Cmdline_mipmap;


static tcache_slot_d3d *Textures = NULL;

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

/**
 *
 * @return bool 
 */
bool d3d_free_texture( tcache_slot_d3d *t )
{
	// Bitmap changed!!	
	if ( t->bitmap_id > -1 )	{				
		// if I have been used this frame, bail		
		if(t->used_this_frame){			
			return false;
		}		

		if(t->d3d8_thandle)
		{
			DBUGFILE_DEC_COUNTER(0);
			int m = 1;

			while ( m > 0 ) {
				m = t->d3d8_thandle->Release();
			}

			t->d3d8_thandle = NULL;
		}

		if ( D3D_last_bitmap_id == t->bitmap_id )	{
			D3D_last_bitmap_id = -1;
		}
			
		t->bitmap_id = -1;
		t->used_this_frame = 0;
		D3D_textures_in -= t->size;
		t->size = 0;
	}

	return true;
}

#ifndef NDEBUG
int Show_uploads = 0;
DCF_BOOL( show_uploads, Show_uploads )
#endif

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
		new_size = MAX(tex_w, tex_h);
		tex_w = new_size;
		tex_h = new_size;
	}	

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

void *d3d_vimage_to_texture(int bitmap_type, int bpp, void *thandle, ushort *data, int src_w, int src_h, int *tex_w, int *tex_h, float *u_scale, float *v_scale, int reload)
{
	// Don't prepare
	bool use_mipmapping = (bitmap_type != TCACHE_TYPE_INTERFACE);

	if(Cmdline_mipmap == 0) {
		use_mipmapping = 0;
	}

	int i,j;	

	PIXELFORMAT *surface_desc = NULL;
	D3DFORMAT d3d8_format;

	switch( bitmap_type ) {
		case TCACHE_TYPE_AABITMAP:		
			surface_desc = (bpp == 32) ? NULL : &AlphaTextureFormat;
			d3d8_format  = (bpp == 32) ? D3DFMT_A8R8G8B8 : default_alpha_tformat;
			break;

//		case TCACHE_TYPE_XPARENT:
//			Int3();

		default:
			surface_desc = (bpp == 32) ? NULL : &NonAlphaTextureFormat;
			d3d8_format  = (bpp == 32) ? D3DFMT_A8R8G8B8 : default_non_alpha_tformat;
			break;
	}	

	// get final texture size
	d3d_tcache_get_adjusted_texture_size(*tex_w, *tex_h, tex_w, tex_h);

	if ( (bitmap_type == TCACHE_TYPE_AABITMAP) || (bitmap_type == TCACHE_TYPE_INTERFACE) ) 
	{
		*u_scale = (float)src_w / (float)*tex_w;
		*v_scale = (float)src_h / (float)*tex_h;
	} else {
		*u_scale = 1.0f;
		*v_scale = 1.0f;
	}

	IDirect3DTexture8 *texture_handle = (IDirect3DTexture8 *) thandle; 
	if(!reload) {

		DBUGFILE_INC_COUNTER(0);
		if(FAILED(GlobalD3DVars::lpD3DDevice->CreateTexture(
			*tex_w, *tex_h,
			use_mipmapping ? 0 : 1, 
			0,
			d3d8_format, 
			D3DPOOL_MANAGED, 
			&texture_handle)))
		{
			Assert(0);
			return NULL;
		}
	}

	IDirect3DSurface8 *d3d_surface = NULL; 

	texture_handle->GetSurfaceLevel(0, &d3d_surface);   

	// lock texture here
	D3DLOCKED_RECT locked_rect;
	if(FAILED(d3d_surface->LockRect(&locked_rect, NULL, 0)))
	{
		Assert(0);
		return NULL;
	}

	ubyte *bmp_data_byte = (ubyte*)data;
	
	// If 16 bit 
	if( surface_desc)
	{
		ushort *bmp_data = (ushort *)data;
		ushort *lpSP;	

		int pitch = locked_rect.Pitch / sizeof(ushort); 
		ushort *dest_data_start = (ushort *) locked_rect.pBits;

		ushort xlat[256];
		int r, g, b, a;

		switch( bitmap_type ) {		
			case TCACHE_TYPE_AABITMAP:			
				// setup convenient translation table
				for (i=0; i<16; i++ ) {
					r = 255;
					g = 255;
					b = 255;
					a = (i*255)/15;
					r /= Gr_ta_red.scale;
					g /= Gr_ta_green.scale;
					b /= Gr_ta_blue.scale;
					a /= Gr_ta_alpha.scale;
					xlat[i] = unsigned short(((a<<Gr_ta_alpha.shift) | (r << Gr_ta_red.shift) | (g << Gr_ta_green.shift) | (b << Gr_ta_blue.shift)));
				}			
				
				xlat[15] = xlat[1];			
				for ( ; i<256; i++ ) {
					xlat[i] = xlat[0];						
				}			
				
				for (j = 0; j < *tex_h; j++) {				
				  	lpSP = dest_data_start + pitch * j;
		
					for (i = 0; i < *tex_w; i++) {
						if ( (i < src_w) && (j<src_h) ) {						
							*lpSP++ = xlat[(ubyte)bmp_data_byte[j*src_w+i]];
						} else {
							*lpSP++ = 0;
						}
					}
				}
			break;

		case TCACHE_TYPE_INTERFACE:  
			for (j = 0; j < src_h; j++) {
				// the proper line in the temp ram
  				lpSP = dest_data_start + (pitch * j);
		
				// nice and clean
				for (i = 0; i < src_w; i++) {												
					// stuff the texture into vram
				  	*lpSP++ = bmp_data[(j * src_w) + i];
				}			
			}
			break;

		// Stretches bitmaps to 2 power of n format
		default: {	// normal:		
				fix u, utmp, v, du, dv;
		
				u = v = 0;
		
				du = ( (src_w-1)*F1_0 ) / *tex_w;
				dv = ( (src_h-1)*F1_0 ) / *tex_h;
												
				for (j = 0; j < *tex_h; j++) {
					lpSP = dest_data_start + pitch * j;
		
					utmp = u;				
					
					for (i = 0; i < *tex_w; i++) {
				 		*lpSP++ = bmp_data[f2i(v)*src_w+f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}
			}
			break;
		}
	}
	// Its 32 bit
	else
	{
		D3DCOLOR *bmp_data = (D3DCOLOR *)data;
		D3DCOLOR *lpSP;	

		int pitch = locked_rect.Pitch / sizeof(D3DCOLOR); 
		D3DCOLOR *dest_data_start = (D3DCOLOR *) locked_rect.pBits;

		for (j = 0; j < *tex_h; j++)  
			for (i = 0; i < *tex_w; i++) 
				  	((D3DCOLOR*)locked_rect.pBits)[(pitch * j) + i]	= 0xff0000ff;

		switch( bitmap_type ) {

			case TCACHE_TYPE_AABITMAP: Assert(0); break; 

			case TCACHE_TYPE_INTERFACE: 
			{
				for (j = 0; j < src_h; j++) {
					// the proper line in the temp ram
  					lpSP = dest_data_start + (pitch * j);
				
					// nice and clean
					for (i = 0; i < src_w; i++) {												
						// stuff the texture into vram
					  	*lpSP++ = bmp_data[(j * src_w) + i];
					}			
				}
				break; 
			}

			default: {
				fix u, utmp, v, du, dv;
		
				u = v = 0;
		
				du = ( (src_w-1)*F1_0 ) / *tex_w;
				dv = ( (src_h-1)*F1_0 ) / *tex_h;
												
				for (j = 0; j < *tex_h; j++) {
					lpSP = dest_data_start + pitch * j;
		
					utmp = u;				
					
					for (i = 0; i < *tex_w; i++) {
				 		*lpSP++ = bmp_data[f2i(v)*src_w+f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}	
				break;
			}
		}
	}

	// Unlock the texture 
	if(FAILED(d3d_surface->UnlockRect()))
	{
		Assert(0);
		return NULL;
	}

	if( use_mipmapping ) {
		D3DXFilterTexture( texture_handle, NULL, 0, D3DX_DEFAULT);
	}

	return texture_handle;
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
int d3d_create_texture_sub(int bitmap_type, int texture_handle, int bpp, ushort *data, int src_w, int src_h, int tex_w, int tex_h, tcache_slot_d3d *t, int reload, int fail_on_full)
{
	Assert(*data != 0xdeadbeef);
	if(t == NULL)
	{
		return 0;
	}

	if(!reload) {
		d3d_free_texture(t);
	}

#if 0
	if ( reload )	{
  		mprintf( ("Reloading '%s'\n", bm_get_filename(texture_handle)) );
	} else {
		mprintf( ("Uploading '%s'\n", bm_get_filename(texture_handle)) );
	}
#endif

	t->d3d8_thandle = (IDirect3DTexture8 *) d3d_vimage_to_texture(
		bitmap_type, bpp, t->d3d8_thandle, data, 
		src_w, src_h, 
		&tex_w, &tex_h,
		&t->u_scale, &t->v_scale,
		reload);
	
	if(t->d3d8_thandle == NULL)
	{
		return 0;
	}

	t->bitmap_id = texture_handle;
	t->time_created = D3D_frame_count;
	t->used_this_frame = 0;	
	t->size = tex_w * tex_h * bpp / 8;	
	t->w = (ushort)tex_w;
	t->h = (ushort)tex_h;
	D3D_textures_in_frame += t->size;
	if (!reload) {
		D3D_textures_in += t->size;
	}

	return 1;
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
			break;
		case TCACHE_TYPE_INTERFACE:
		case TCACHE_TYPE_XPARENT:
			flags |= BMP_TEX_XPARENT;				
			break;
		case TCACHE_TYPE_COMPRESSED:
			switch (bm_is_compressed(bitmap_handle)) {
				case DDS_DXT1:				//dxt1
					bpp = 24;
					flags |= BMP_TEX_DXT1;
					break;
				case DDS_DXT3:				//dxt3
					bpp = 32;
					flags |= BMP_TEX_DXT3;
					break;
				case DDS_DXT5:				//dxt5
					bpp = 32;
					flags |= BMP_TEX_DXT5;
					break;
				default:
					Assert( 0 );
					break;
			}
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

	// if we ended up locking a texture that wasn't originally compressed then this should catch it
	if ( bm_is_compressed(bitmap_handle) ) {
		bitmap_type = TCACHE_TYPE_COMPRESSED;
	}

	if ( (bitmap_type != TCACHE_TYPE_AABITMAP) && (bitmap_type != TCACHE_TYPE_INTERFACE) && (bitmap_type != TCACHE_TYPE_COMPRESSED) )	{
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
	else if(tslot->bitmap_id != bitmap_handle) {
		reload = (final_w == tslot->w) && (final_h == tslot->h);
	}

 	int ret_val = d3d_create_texture_sub(bitmap_type, bitmap_handle, bmp->bpp, (ushort*)bmp->data, bmp->w, bmp->h, max_w, max_h, tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}

bool d3d_preload_texture_func(int bitmap_id)
{
	return 1;
}

bool d3d_lock_and_set_internal_texture(int stage, int handle, ubyte bpp, int bitmap_type, float *u_scale, float *v_scale );

int d3d_tcache_set_internal(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, int force, int stage )
{
	//bitmap *bmp = NULL;

	if ( bitmap_id < 0 )	{
		D3D_last_bitmap_id  = -1;
		return 0;
	}

	if(d3d_lock_and_set_internal_texture(stage, bitmap_id, (ubyte) 16, bitmap_type, u_scale, v_scale) == true) 
	{
	 	D3D_last_bitmap_id = -1;
	 	return 1;
	}

	int n = bm_get_cache_slot( bitmap_id, 1 );

	if ( D3D_last_detail != Detail.hardware_textures )	{
		D3D_last_detail = Detail.hardware_textures;
		d3d_tcache_flush();
	}
	
	tcache_slot_d3d * t = &Textures[n];	
	
	if(!bm_is_render_target(bitmap_id)){
		// If rendering exactly the same texture section as before
		if ( (D3D_last_bitmap_id == bitmap_id) && (D3D_last_bitmap_type==bitmap_type) && (t->bitmap_id == bitmap_id))	{
			t->used_this_frame++;
		
			*u_scale = t->u_scale;
			*v_scale = t->v_scale;
			return 1;
		}	

		// if the texture sections haven't been created yet
		if((t->bitmap_id < 0) || (t->bitmap_id != bitmap_id))
		{	
			if(d3d_create_texture( bitmap_id, bitmap_type, t, fail_on_full ) == 0) 
			{
	 			d3d_free_texture(t);
	 			return 0;
			}
		}
		
		*u_scale = t->u_scale;
		*v_scale = t->v_scale;

		d3d_SetTexture(stage, t->d3d8_thandle);
	}else{
		d3d_SetTexture(stage, get_render_target_texture(bitmap_id));
	}
	
	D3D_last_bitmap_id = t->bitmap_id;
	D3D_last_bitmap_type = bitmap_type;

	t->used_this_frame++;		
	return 1;
}

int d3d_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, int force, int stage)
{
	return d3d_tcache_set_internal(bitmap_id, bitmap_type, u_scale, v_scale, fail_on_full, force, stage );
}

void d3d_tcache_init()
{
	int i; 	
  	D3D_min_texture_width  = 16;
	D3D_min_texture_height = 16;
	D3D_max_texture_width  = GlobalD3DVars::d3d_caps.MaxTextureWidth;
	D3D_max_texture_height = GlobalD3DVars::d3d_caps.MaxTextureHeight;

	D3D_square_textures = (GlobalD3DVars::d3d_caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) ? 1 : 0; 
	D3D_pow2_textures   = (GlobalD3DVars::d3d_caps.TextureCaps & D3DPTEXTURECAPS_POW2)       ? 1 : 0; 


	// RT I don't think wide surfaces are supported in D3D8 so we need this code
	if(0)
	{
		if ( D3D_max_texture_width > gr_screen.max_w ) {
			D3D_max_texture_width = gr_screen.max_w;

			if ( D3D_pow2_textures ) {	
				for (i=0; i<16; i++ ) {	
					if ( (D3D_max_texture_width>= (1<<i)) && (D3D_max_texture_width < (1<<(i+1))) )	{
						D3D_max_texture_width = 1 << i;
						break;
					}
				}
			}

			if ( D3D_max_texture_height > D3D_max_texture_width ) {
				D3D_max_texture_height = D3D_max_texture_width;
			}

			mprintf(( "Doesn't support wide surfaces. Bashing max down to %d\n", D3D_max_texture_width ));
		}
	}

	{
		if(	!os_config_read_uint( NULL, NOX("D3DUseLargeTextures"), 0 )) {
			if ( D3D_max_texture_width > 1024 )	{
				D3D_max_texture_width = 1024;
			}

			if ( D3D_max_texture_height > 1024 )	{
				D3D_max_texture_height = 1024;
			}	
		} 
	}

	DBUGFILE_OUTPUT_2("Max textures: %d %d",D3D_max_texture_width,D3D_max_texture_height);
	
	Textures = (tcache_slot_d3d *) vm_malloc(MAX_BITMAPS * sizeof(tcache_slot_d3d));

	if ( !Textures ) {
		DBUGFILE_OUTPUT_0("exit");
		exit(1);
	}

	memset( Textures, 0, MAX_BITMAPS * sizeof(tcache_slot_d3d) );

	// Init the texture structures
	for( i=0; i<MAX_BITMAPS; i++ )	{
	//	Textures[i].d3d8_thandle = NULL;

		Textures[i].bitmap_id = -1;
	//	Textures[i].size = 0;
	//	Textures[i].used_this_frame = 0; 

	//	Textures[i].parent = NULL;
	}

	D3D_last_detail = Detail.hardware_textures;
	D3D_last_bitmap_id = -1;
	D3D_last_bitmap_type = -1;

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
}

void d3d_tcache_cleanup()
{
	d3d_tcache_flush();
	
	D3D_textures_in = 0;
	D3D_textures_in_frame = 0;

	if ( Textures )	{
		vm_free(Textures);
		Textures = NULL;
	}
}

void d3d_tcache_frame()
{
	D3D_last_bitmap_id = -1;
	D3D_textures_in_frame = 0;

	D3D_frame_count++;

	int i;
	for( i=0; i<MAX_BITMAPS; i++ )	{
		Textures[i].used_this_frame = 0; 
	}
}


void gr_d3d_preload_init()
{
	d3d_tcache_flush();
}

int gr_d3d_preload(int bitmap_num, int is_aabitmap)
{
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

int d3d_get_valid_texture_size(int value, bool width)
{
	int min = width ? D3D_min_texture_width : D3D_min_texture_height;

	if(value < min)
		return min;

	int max = width ? D3D_max_texture_width : D3D_max_texture_height;

	for(int v = min; v <= max; v <<= 1) {
		if(value <= v)
		{
			return v;
		}
	}

	// value is too big
	return -1;
}

//added this to fix AA lines, but then found I didn't need to, 
//leaving it as it will be very useful later, and a pain to remove
void gr_d3d_set_texture_addressing(int address){
	if(address == TMAP_ADDRESS_WRAP){
	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP  );
	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP  );
	}else if(address == TMAP_ADDRESS_MIRROR){
 	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_MIRROR  );
 	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_MIRROR  );
	}else if(address == TMAP_ADDRESS_CLAMP){
	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP  );
	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP  );
	}
}


void d3d_set_texture_panning(float u, float v, bool enable){

	if(enable){
		d3d_SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		D3DXMATRIX world(
			1, 0, 0, 0,
			0, 1, 0, 0,
			u, v, 0, 0,
			0, 0, 0, 1);

		GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_TEXTURE0, &world);
	}else{
		d3d_SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	}
}

#endif // !NO_DIRECT3D
