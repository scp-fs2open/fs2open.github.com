/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef NO_DIRECT3D

// D3D8 includes
#include <d3d8.h>
#include <d3dx8.h>
#include <D3dx8tex.h>

#include <ctype.h>
#include "globalincs/pstypes.h"
#include "pcxutils/pcxutils.h"
#include "bmpman/bmpman.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/grd3dinternal.h"
#include "debugconsole/dbugfile.h"
#include "cmdline/cmdline.h"
#include "cfile/cfile.h"
#include "ddsutils/ddsutils.h"
#include "tgautils/tgautils.h"
#include "graphics/grd3dbmpman.h"
#include "globalincs/systemvars.h"
#include "jpgutils/jpgutils.h"
#include "anim/animplay.h"
#include "anim/packunpack.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"


D3DBitmapData d3d_bitmap_entry[MAX_BITMAPS];

int d3d_get_valid_texture_size(int value, bool width);



static inline int is_power_of_two(int w, int h)
{
	return ( ((w == 32) || (w == 64) || (w == 128) || (w == 256) || (w == 512) || (w == 1024) || (w == 2048) || (w == 4096)) &&
			 ((h == 32) || (h == 64) || (h == 128) || (h == 256) || (h == 512) || (h == 1024) || (h == 2048) || (h == 4096)) );
}

// anything API specific to freeing bm data
void gr_d3d_bm_free_data(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	int rc = 0;

	if (d3d_bitmap_entry[n].tinterface != NULL) {
		do { 
			rc = d3d_bitmap_entry[n].tinterface->Release(); 
			if(rc > 1)Int3();
		} while ( rc > 0 );
	//	} while ( rc > 1 );
	//	d3d_bitmap_entry[n].tinterface->Release();

		d3d_bitmap_entry[n].tinterface = NULL;
	}
}

// API specifics for creating a user bitmap
void gr_d3d_bm_create(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	Assert(d3d_bitmap_entry[n].tinterface == NULL);

	d3d_bitmap_entry[n].tinterface = NULL;
}

IDirect3DTexture8 *d3d_make_compressed_texture( ubyte *in_data, int width, int height, int alpha = 1 )
{
	IDirect3DTexture8 *thandle = NULL;
	IDirect3DSurface8 *dds_surface = NULL;
	RECT source_rect;
	HRESULT hr = D3D_OK;
//	int i;

	if ( FAILED( GlobalD3DVars::lpD3DDevice->CreateTexture( width, height, 1, 0,
															(alpha) ? D3DFMT_DXT5 : D3DFMT_DXT1,
															D3DPOOL_MANAGED, &thandle ) ) )
	{
		Int3();
		return NULL;
	}

	thandle->GetSurfaceLevel( 0, &dds_surface );

	source_rect.left = source_rect.top = 0;
	source_rect.right = width;
	source_rect.bottom = height;

	hr = D3DXLoadSurfaceFromMemory( dds_surface, NULL, NULL, in_data, (alpha) ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8,
							   (width * (3+alpha)), NULL, &source_rect, D3DX_FILTER_NONE, 0 );

	//D3D_RELEASE( dds_surface, i );
	dds_surface->Release();

	if ( hr != D3D_OK ) {
		Int3();
		return NULL;
	}

	return thandle;
}

extern void bm_clean_slot(int n);

static int d3d_bm_lock_ani_compress( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	anim			*the_anim;
	anim_instance	*the_anim_instance;
	bitmap			*bm;
	ubyte			*frame_data;
	int				size, i, alpha;
	int				first_frame, nframes;


	first_frame = be->info.ani.first_frame;
	nframes = bm_bitmaps[first_frame].info.ani.num_frames;

	// bpp can always be 24-bit since we don't do images with alpha here
//	bpp = (bpp == 32) ? bpp : 24;
	bpp = 32;

	alpha = (bpp == 32);

	if ( (the_anim = anim_load(bm_bitmaps[first_frame].filename)) == NULL ) {
		return 1;
	}

	if ( (the_anim_instance = init_anim_instance(the_anim, bpp)) == NULL ) {
		anim_free(the_anim);
		return 1;
	}

	int can_drop_frames = 0;

	if ( the_anim->total_frames != bm_bitmaps[first_frame].info.ani.num_frames )	{
		can_drop_frames = 1;
	}

	bm = &bm_bitmaps[first_frame].bm;
	size = bm->w * bm->h * (bpp >> 3);

	for ( i=0; i<nframes; i++ )	{
		be = &bm_bitmaps[first_frame+i];
		bmp = &bm_bitmaps[first_frame+i].bm;

		// Unload any existing data
		bm_clean_slot( first_frame+i );

		bmp->flags = 0;

		bmp->bpp = bpp;

		frame_data = anim_get_next_raw_buffer(the_anim_instance, 0, 0, bmp->bpp);

		Assert( frame_data != NULL );

		nprintf(("BMPMAN", "Attempting to compress '%s', original size %.3fM ... ", be->filename, ((float)size/1024.0f)/1024.0f));

		d3d_bitmap_entry[first_frame+i].tinterface = (IDirect3DBaseTexture8*) d3d_make_compressed_texture( frame_data, bmp->w, bmp->h, alpha );
		
		if ( d3d_bitmap_entry[first_frame+i].tinterface == NULL ) {
			free_anim_instance(the_anim_instance);
			anim_free(the_anim);

			nprintf(("BMPMAN", "compression failed!!\n"));

			return 1;
		}

		bmp->data = 0;
		bmp->palette = NULL;
		be->comp_type = (alpha) ? BM_TYPE_DXT5 : BM_TYPE_DXT1;
		// not sure how to get actual size on D3D, might as well best-guess it...
		be->mem_taken = (size / (alpha) ? 4 : 6); // should be DXT1 which should give a 6:1 ratio

		bm_update_memory_used( first_frame + i, be->mem_taken );

		nprintf(("BMPMAN", "new size is %.3fM.\n", ((float)be->mem_taken/1024.0f)/1024.0f));

		// Skip a frame
		if ( (i < nframes-1)  && can_drop_frames )	{
			frame_data = anim_get_next_raw_buffer(the_anim_instance, 0, 0, bm->bpp);
		}
	}

	free_anim_instance(the_anim_instance);
	anim_free(the_anim);
	
	return 0;
}

static int d3d_bm_lock_compress( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, int bpp, int flags )
{
	ubyte *data = NULL;
	int error_code = 1;
	int byte_size = 0;
	int alpha = 1;


	// don't use for EFFs, if they were wanting to be DDS then they should already be that way
	if ( be->type == BM_TYPE_EFF ) {
		return 1;
	}

	Assert( (be->type == BM_TYPE_PCX) ||
			(be->type == BM_TYPE_TGA) ||
			(be->type == BM_TYPE_JPG) );

	Assert( !(flags & BMP_AABITMAP) );

	// PCX need to be loaded as 32-bit
	if ( (be->type == BM_TYPE_PCX) && (bpp == 16) )
		bpp = 32;

	bm_clean_slot( bitmapnum );

	byte_size = (bpp >> 3);
	Assert( (byte_size == 3) || (byte_size == 4) );
	alpha = (byte_size != 3);

	// stupid D3D, we have to read everything at 32-bit apparently
	Assert( byte_size == 4 );

	data = (ubyte*)vm_malloc(bmp->w * bmp->h * byte_size);

	Assert( data != NULL );

	if ( data == NULL )
		return 1;

	// read in bitmap data
	if (be->type == BM_TYPE_PCX) {
		Assert( bpp == 32 );
		error_code = pcx_read_bitmap( be->filename, data, NULL, byte_size );
	} else if (be->type == BM_TYPE_TGA) {
	//	error_code = targa_read_bitmap( be->filename, data, NULL, byte_size );
		error_code = 1;
	} else if (be->type == BM_TYPE_JPG) {
	//	error_code = jpeg_read_bitmap( be->filename, data, NULL, byte_size );
		error_code = 1;
	} else {
		Assert( 0 );
	}

	nprintf(("BMPMAN", "Attempting to compress '%s', original size %.3fM ... ", be->filename, ((float)(bmp->w * bmp->h * byte_size)/1024.0f)/1024.0f));

	// NOTE: this assumes that the *_ERROR_NONE #define's are going to be 0
	if (error_code) {
		if (data != NULL) {
			vm_free(data);
			data = NULL;
		}

		nprintf(("BMPMAN", "initial read failed!!\n"));

		return 1;
	}

	// now for the attempt to compress the data
	d3d_bitmap_entry[bitmapnum].tinterface = (IDirect3DBaseTexture8*) d3d_make_compressed_texture( data, bmp->w, bmp->h, alpha );

	if (d3d_bitmap_entry[bitmapnum].tinterface == NULL) {
		if (data != NULL) {
			vm_free(data);
			data = NULL;
		}

		nprintf(("BMPMAN", "compression failed!!\n"));

		return 1;
	}

	bmp->data = 0;
	bmp->bpp = (alpha) ? (ubyte)32 : (ubyte)24;
	bmp->palette = NULL;
	be->comp_type = (alpha) ? BM_TYPE_DXT5 : BM_TYPE_DXT1;
	// not sure how to get actual size on D3D, might as well best-guess it...
	be->mem_taken = (bmp->w * bmp->h * byte_size / ((alpha) ? 4 : 6)); // in best case we should have 4:1 for DXT5, 6:1 for DXT1

	bm_update_memory_used( bitmapnum, be->mem_taken );

	nprintf(("BMPMAN", "new size is %.3fM.\n", ((float)be->mem_taken/1024.0f)/1024.0f));

	vm_free(data);

	return 0;
}

// create a texture from data stored in memory
IDirect3DTexture8 *d3d_make_texture(void *data, int bitmapnum, int size, int type, int flags, char *filename) 
{
	D3DXIMAGE_INFO source_desc;
	IDirect3DTexture8 *ptexture = NULL;
	D3DFORMAT use_format  = D3DFMT_UNKNOWN;
	ubyte comp_type = BM_TYPE_NONE;

	if ( FAILED(D3DXGetImageInfoFromFileInMemory(data, size, &source_desc)) ) {
		return NULL;
	} 

	// OGL doesn't support DXT2 or DXT4 so don't support it here either
	Assert( (source_desc.Format != D3DFMT_DXT2) && (source_desc.Format != D3DFMT_DXT4) );

	// Determine the destination (texture) format to hold the image
	switch(source_desc.Format)
	{
		case D3DFMT_DXT1:
		case D3DFMT_DXT3:
		case D3DFMT_DXT5:
			break;

		default:
		{
			bool use_alpha_format = false;	// initialization added by Goober5000

			// Determine if the destination format needs to store alpha details
			switch (type)
			{
				case BM_TYPE_TGA:
					use_alpha_format = (source_desc.Format == D3DFMT_A8R8G8B8);
					break;

				case BM_TYPE_JPG:
					use_alpha_format = false;
					break;

				case BM_TYPE_DDS:
					use_alpha_format = (source_desc.Format == D3DFMT_A8R8G8B8);
					break;

				default:
					Assert(0);	// just in case -- Goober5000
			}

			if (gr_screen.bits_per_pixel == 32) {
				use_format = use_alpha_format ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
			} else {
				use_format = use_alpha_format ? default_alpha_tformat : default_non_alpha_tformat;
			}

			if ( Cmdline_img2dds && (type != BM_TYPE_DDS) ) {
				use_format = (use_alpha_format) ? D3DFMT_DXT5 : D3DFMT_DXT1;
				comp_type = (use_alpha_format) ? BM_TYPE_DXT5 : BM_TYPE_DXT1;
			}
		}
	}

	float *uscale = &(d3d_bitmap_entry[bitmapnum].uscale);
	float *vscale = &(d3d_bitmap_entry[bitmapnum].vscale);
	  
	bool use_mipmapping = (Cmdline_mipmap > 0);

	DWORD filter = D3DX_FILTER_LINEAR; // Linear, enough to smooth rescales but not too much blur

	*uscale = *vscale = 1.0;

	if(flags == TCACHE_TYPE_INTERFACE) {
		use_mipmapping = 0;
	  	filter = D3DX_FILTER_NONE; 
		*uscale = ((float) source_desc.Width)  / ((float) d3d_get_valid_texture_size(source_desc.Width, true));
		*vscale = ((float) source_desc.Height) / ((float) d3d_get_valid_texture_size(source_desc.Height, false));
	} else if(gr_screen.bits_per_pixel == 16 && ((type == BM_TYPE_TGA) || (type == BM_TYPE_JPG))) {
	  	filter |=D3DX_FILTER_DITHER;
	}

	HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
		GlobalD3DVars::lpD3DDevice,
		data, size,
	  	// Opertunity to control sizes here.
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		use_mipmapping ? 0 : 1, 
		0, 
		use_format,
		D3DPOOL_MANAGED, 
		filter,
	 	D3DX_DEFAULT,
		0, 
		&source_desc, 
		NULL, &ptexture);

	if ( SUCCEEDED(hr) && (comp_type != BM_TYPE_NONE) ) {
		bm_bitmaps[bitmapnum].comp_type = comp_type;
	}

	return SUCCEEDED(hr) ? ptexture : NULL;
}

// create D3D texture from an image file
int d3d_lock_d3dx_types(char *file, int type, ubyte flags, int bitmapnum)
{
	char filename[MAX_FILENAME_LEN];

	strcpy( filename, file);
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;

	switch (type) {
		case BM_TYPE_DXT1:
		case BM_TYPE_DXT3:
		case BM_TYPE_DXT5:
		case BM_TYPE_DDS:
			strcat( filename, ".dds" );
			break;

		case BM_TYPE_TGA:
			strcat( filename, ".tga" );
			break;

		case BM_TYPE_JPG:
			strcat( filename, ".jpg" );
			break;

		default:
			return 0;
	}

	CFILE *img_file = cfopen( filename , "rb" );

	if (img_file == NULL) {
		DBUGFILE_OUTPUT_1("Failed to open through cfopen '%s'", filename);
		return 0;
	}

	int size = cfilelength(img_file);
	void *img_data = vm_malloc(size);

	if  (img_data == NULL)
		return 0;

	cfread(img_data, size, 1, img_file);	

	cfclose(img_file);
	img_file = NULL;

	d3d_bitmap_entry[bitmapnum].tinterface = (IDirect3DBaseTexture8*) d3d_make_texture(img_data, bitmapnum, size, type, flags, filename);

	vm_free(img_data);

	return 1;
}

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
int gr_d3d_bm_load(ubyte type, int n, char *filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *c_type, int *mm_lvl, int *bm_size)
{
	int dds_ct;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	Assert(d3d_bitmap_entry[n].tinterface == NULL);

	if ( d3d_bitmap_entry[n].tinterface != NULL )
		return -1;

	// DDS file
	if (type == BM_TYPE_DDS) {
		int dds_error = dds_read_header( filename, img_cfp, w, h, bpp, &dds_ct, mm_lvl, bm_size );
		if (dds_error != DDS_ERROR_NONE) {
			mprintf(("dds: Couldn't open '%s' -- error description %s\n", filename, dds_error_string(dds_error)));
			return -1;
		}
		
		switch (dds_ct) {
			case DDS_DXT1:
				*c_type = BM_TYPE_DXT1;
				break;
				
			case DDS_DXT3:
				*c_type = BM_TYPE_DXT3;
				break;
				
			case DDS_DXT5:
				*c_type = BM_TYPE_DXT5;
				break;
				
			case DDS_UNCOMPRESSED:
				*c_type = BM_TYPE_DDS;
				break;

			// not sure how to properly handle existing cubemaps from a file
			// in D3D, so just don't use them for now - taylor
			case DDS_CUBEMAP_DXT1:
			case DDS_CUBEMAP_DXT3:
			case DDS_CUBEMAP_DXT5:
			case DDS_CUBEMAP_UNCOMPRESSED:
				mprintf(("D3D-STUB:  Don't currently support cubemaps from file!\n"));
				return -1;

			default:
				Error(LOCATION, "bad DDS file compression.  Not using DXT1,3,5 %s", filename);
				return -1;
		}
	}
	// if its a tga file
	else if (type == BM_TYPE_TGA) {
		int tga_error = targa_read_header( filename, img_cfp, w, h, bpp, NULL );
		if ( tga_error != TARGA_ERROR_NONE )	{
			mprintf(( "tga: Couldn't open '%s'\n", filename ));
			return -1;
		}
	}
	// maybe it's a PCX
	else if (type == BM_TYPE_PCX) {
		int pcx_error = pcx_read_header( filename, img_cfp, w, h, bpp, NULL );		
		if ( pcx_error != PCX_ERROR_NONE )	{
			mprintf(( "bm_pcx: Couldn't open '%s'\n", filename ));
			return -1;
		}
	}
	// maybe it's a JPG
	else if (type == BM_TYPE_JPG) {
		int jpg_error = jpeg_read_header( filename, img_cfp, w, h, bpp, NULL );
		if ( jpg_error != JPEG_ERROR_NONE ) {
			mprintf(( "jpg: Couldn't open '%s'\n", filename ));
			return -1;
		}
	} else {
		Assert( 0 );
		
		return -1;
	}

	return 0;
}

// API specific init instructions
void gr_d3d_bm_init(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	memset( &d3d_bitmap_entry[n], 0, sizeof(D3DBitmapData) );
}

extern void gr_d3d_preload_init();
// specifics for setting up the start of a page-in session
void gr_d3d_bm_page_in_start()
{
	gr_d3d_preload_init();

	if (GlobalD3DVars::lpD3DDevice != NULL)
		GlobalD3DVars::lpD3DDevice->ResourceManagerDiscardBytes(0);
}

// Lock an image files data into memory
int gr_d3d_bm_lock(char *filename, int handle, int bitmapnum, ubyte bpp, ubyte flags)
{
	ubyte c_type = BM_TYPE_NONE;
	char realname[MAX_FILENAME_LEN];
	ubyte true_bpp;
	int try_compress = 0;

	bitmap_entry *be = &bm_bitmaps[bitmapnum];
	bitmap *bmp = &be->bm;

	Assert( !Is_standalone );

	if (bmp->true_bpp > bpp)
		true_bpp = bmp->true_bpp;
	else
		true_bpp = bpp;

	if ( (bmp->data == 0) && (d3d_bitmap_entry[bitmapnum].tinterface == NULL) ) {
		Assert(be->ref_count == 1);

		if ( c_type != BM_TYPE_USER ) {
			if ( bmp->data == 0 ) {
				mprintf (("Loading %s for the first time.\n", filename));
			}
		}

		if ( !Bm_paging )	{
			if ( c_type != BM_TYPE_USER ) {
				char flag_text[64];
				strcpy( flag_text, "--" );							
				nprintf(( "Paging", "Loading %s (%dx%dx%dx%s)\n", filename, bmp->w, bmp->h, true_bpp, flag_text ));
			}
		}

		// select proper format
		if(flags & BMP_AABITMAP){
			BM_SELECT_ALPHA_TEX_FORMAT();
		} else if(flags & BMP_TEX_ANY){
			BM_SELECT_TEX_FORMAT();					
		} else {
		   	BM_SELECT_SCREEN_FORMAT();
		}

		// make sure we use the real graphic type for EFFs
		if ( be->type == BM_TYPE_EFF ) {
			c_type = be->info.ani.eff.type;
			strcpy(realname, be->info.ani.eff.filename);
		} else {
			c_type = be->type;
			strcpy(realname, be->filename);
		}

		try_compress = (Cmdline_img2dds && Texture_compression_available && is_power_of_two(bmp->w, bmp->h) && !(flags & BMP_AABITMAP) && !Is_standalone);

		switch ( c_type ) {
			case BM_TYPE_PCX:
				if (try_compress && (true_bpp >= 16)) {
					if ( !d3d_bm_lock_compress(handle, bitmapnum, be, bmp, true_bpp, flags) ) {
						break;
					}
				}

		  		bm_lock_pcx( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;

			case BM_TYPE_ANI:
				if (try_compress && (true_bpp >= 16) && !(flags & BMP_TEX_XPARENT)) {
					if ( !d3d_bm_lock_ani_compress(handle, bitmapnum, be, bmp, true_bpp, flags) ) {
						break;
					}
				}

				bm_lock_ani( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;

			case BM_TYPE_TGA:
				/*if (try_compress && (true_bpp >= 24)) {
					if ( !d3d_bm_lock_compress(handle, bitmapnum, be, bmp, true_bpp, flags) ) {
						break;
					}
				}*/
				
				d3d_lock_d3dx_types(realname, c_type, flags, bitmapnum);			
				break;
				
			case BM_TYPE_JPG:
				/*if (try_compress) {
					if ( !d3d_bm_lock_compress(handle, bitmapnum, be, bmp, true_bpp, flags) ) {
						break;
					}
				}*/
				
				d3d_lock_d3dx_types(realname, c_type, flags, bitmapnum);			
				break;

			case BM_TYPE_DDS:
			case BM_TYPE_DXT1:
			case BM_TYPE_DXT3:
			case BM_TYPE_DXT5:
				d3d_lock_d3dx_types(realname, c_type, flags, bitmapnum);			
				break;

			case BM_TYPE_USER:	
				bm_lock_user( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;

			default:
				Warning(LOCATION, "Unsupported type in bm_lock -- %d\n", c_type );
				return -1;
		}		

		// always go back to screen format
		BM_SELECT_SCREEN_FORMAT();
	}

	// make sure we actually did something
	if ( !(bmp->data) && (d3d_bitmap_entry[bitmapnum].tinterface == NULL) ) {
		// oops, this isn't good - nothing ever got loaded
		return -1;
	}

	return 0;
}

// Lock a image file's data into memory and set it as a texture
bool d3d_lock_and_set_internal_texture(int stage, int handle, ubyte bpp, int bitmap_type, float *u_scale, float *v_scale )
{
	ubyte true_bpp;
	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE

	ubyte c_type = BM_TYPE_NONE;

	if ( (bm_bitmaps[bitmapnum].type == BM_TYPE_RENDER_TARGET_DYNAMIC) || (bm_bitmaps[bitmapnum].type == BM_TYPE_RENDER_TARGET_STATIC) )
		return false;

	Assert( !Is_standalone );

	if (bm_bitmaps[bitmapnum].bm.true_bpp > bpp)
		true_bpp = bm_bitmaps[bitmapnum].bm.true_bpp;
	else
		true_bpp = bpp;

	ubyte bmp_flags = 0;
	switch (bitmap_type) {
		case TCACHE_TYPE_AABITMAP:
			bmp_flags |= BMP_AABITMAP;
			break;
		case TCACHE_TYPE_NORMAL:
			bmp_flags |= BMP_TEX_OTHER;
			break;
		case TCACHE_TYPE_INTERFACE:
		case TCACHE_TYPE_XPARENT:
			bmp_flags |= BMP_TEX_XPARENT;				
			break;
		case TCACHE_TYPE_COMPRESSED:
			switch (bm_is_compressed(handle)) {
				case DDS_DXT1:				//dxt1
					bmp_flags |= BMP_TEX_DXT1;
					break;
				case DDS_DXT3:				//dxt3
					bmp_flags |= BMP_TEX_DXT3;
					break;
				case DDS_DXT5:				//dxt5
					bmp_flags |= BMP_TEX_DXT5;
					break;
				default:
					Assert( 0 );
					break;
			}
			break;
	}

	if (bm_bitmaps[bitmapnum].type == BM_TYPE_EFF) {
		c_type = bm_bitmaps[bitmapnum].info.ani.eff.type;
	} else {
		c_type = bm_bitmaps[bitmapnum].type;
	}

	int valid_type = 0;

	int try_compress = (Cmdline_img2dds && Texture_compression_available && 
						is_power_of_two(bm_bitmaps[bitmapnum].bm.w, bm_bitmaps[bitmapnum].bm.h) && 
						!(bmp_flags & BMP_AABITMAP) && !Is_standalone);

	switch ( c_type ) {
		case BM_TYPE_DXT1:
		case BM_TYPE_DXT3:
		case BM_TYPE_DXT5:
		case BM_TYPE_DDS:
		case BM_TYPE_JPG:
		case BM_TYPE_TGA:
			valid_type = 1;
			break;

		case BM_TYPE_ANI:
		case BM_TYPE_PCX:
			if ( try_compress && (true_bpp >= 16) && !(bmp_flags & BMP_TEX_XPARENT) ) {
				valid_type = 1;
			} else {
				valid_type = 0;
			}
			break;
	
		default:
			valid_type = 0;
			break;
	}

	if (valid_type) {
		// There is no internal texture
		bm_lock(handle, true_bpp, bmp_flags );
		bm_unlock(handle); 
		
	 	if(u_scale) *u_scale = d3d_bitmap_entry[bitmapnum].uscale;
	 	if(v_scale) *v_scale = d3d_bitmap_entry[bitmapnum].vscale;

		d3d_SetTexture(stage, d3d_bitmap_entry[bitmapnum].tinterface);
		return true;
	}

	if(Cmdline_d3d_lesstmem && c_type == BM_TYPE_PCX)
	{
		if(d3d_bitmap_entry[bitmapnum].tinterface == NULL)
		{
			// Trick the normal functions into getting the info we need
			tcache_slot_d3d t;
			t.bitmap_id = -1;

			d3d_create_texture(handle, bitmap_type, &t, 0); 
			d3d_bitmap_entry[bitmapnum].uscale = t.u_scale;
	 		d3d_bitmap_entry[bitmapnum].vscale = t.v_scale;
		  	d3d_bitmap_entry[bitmapnum].tinterface = t.d3d8_thandle;

			vm_free((void *) bm_bitmaps[bitmapnum].bm.data);
			bm_bitmaps[bitmapnum].bm.data = NULL;
		}

		if(u_scale) *u_scale = d3d_bitmap_entry[bitmapnum].uscale;
	 	if(v_scale) *v_scale = d3d_bitmap_entry[bitmapnum].vscale;

		d3d_SetTexture(stage, d3d_bitmap_entry[bitmapnum].tinterface);
		return true;
	}

	return false;
}

	static IDirect3DSurface8 *surface = NULL;
	IDirect3DSurface8 *depth = NULL;
	static IDirect3DSurface8 *back_depth = NULL;	//the back_buffer's zbuffer
	static IDirect3DSurface8 *texture_depth = NULL;	//a zbuffer assosiated with rendeing to a texture

void bm_pre_lost(){
	int I;
	if(back_depth)I = back_depth->Release();
	back_depth = NULL;
	if(surface)I = surface->Release();
	surface = NULL;
	if(texture_depth)texture_depth->Release();
		texture_depth=NULL;
	for( int i = 0; i<MAX_BITMAPS; i++){
		if(d3d_bitmap_entry[i].flags & DXT_DEFAULT_MEM_POOL && d3d_bitmap_entry[i].tinterface){
			I = d3d_bitmap_entry[i].tinterface->Release();
			d3d_bitmap_entry[i].tinterface = NULL;
		}
	}
}

void bm_post_lost(){
	int I = 0;
	for(int i = 0; i<MAX_BITMAPS; i++){
		if(d3d_bitmap_entry[i].flags&DXT_DEFAULT_MEM_POOL){
			Assert(!d3d_bitmap_entry[i].tinterface);

			bool cube = (d3d_bitmap_entry[i].flags & DXT_CUBEMAP) != 0;

			if(cube)GlobalD3DVars::lpD3DDevice->CreateCubeTexture(max(d3d_bitmap_entry[i].x,d3d_bitmap_entry[i].y),1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, (IDirect3DCubeTexture8**)&d3d_bitmap_entry[i].tinterface);
			else GlobalD3DVars::lpD3DDevice->CreateTexture(d3d_bitmap_entry[i].x,d3d_bitmap_entry[i].y,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, (IDirect3DTexture8**)&d3d_bitmap_entry[i].tinterface);

			if(d3d_bitmap_entry[i].backup_tinterface){
				//if it has a backup copy that backup back into it
				//if it doesn't have a backup then it is a dynamic render target and doesn't need it (and can't get it)
				IDirect3DSurface8* texture;
				IDirect3DSurface8* backup;
				int nf = (cube)?6:1;
				for(int f = 0; f<nf; f++){
					if(cube){
						(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[i].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(f),0,&texture);
						(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[i].backup_tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(f),0,&backup);
					}else{
						(*((IDirect3DTexture8**)(&d3d_bitmap_entry[i].tinterface)))->GetSurfaceLevel(0,&texture);
						(*((IDirect3DTexture8**)(&d3d_bitmap_entry[i].backup_tinterface)))->GetSurfaceLevel(0,&backup);
					}
	
					D3DXLoadSurfaceFromSurface(texture,NULL,NULL,backup,NULL,NULL, D3DX_FILTER_POINT,0);
					I= backup->Release();
					I= texture->Release();
				}
			}
		}
	}
}


int gr_d3d_bm_make_render_target(int n, int *width, int *height, ubyte *bpp, int *mm_lvl, int flags)
{
	*mm_lvl = 1;
	int x = *width;
	int y = *height;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	Assert(d3d_bitmap_entry[n].tinterface == NULL);

#if 0
	return false;
#else
	//mark this surface as haveing a resource we are going to have to clean up dureing a lost device
		d3d_bitmap_entry[n].flags |= DXT_DEFAULT_MEM_POOL;

		//make the drimary drawing surface
		if(flags & BMP_FLAG_CUBEMAP){
			d3d_bitmap_entry[n].flags |= DXT_CUBEMAP;
			GlobalD3DVars::lpD3DDevice->CreateCubeTexture(max(x,y),1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, (IDirect3DCubeTexture8**)&d3d_bitmap_entry[n].tinterface);
		}else{
			GlobalD3DVars::lpD3DDevice->CreateTexture(x,y,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, (IDirect3DTexture8**)&d3d_bitmap_entry[n].tinterface);
		}

		if(flags & BMP_FLAG_RENDER_TARGET_STATIC){
			//if we are going to want to keep this
			//if this is a static render target 
			//then we are going to want to keep a copy of it in system memory
			d3d_bitmap_entry[n].flags	|= DXT_STATIC;
			if(flags & BMP_FLAG_CUBEMAP){
				GlobalD3DVars::lpD3DDevice->CreateCubeTexture	(max(x,y),	1,D3DUSAGE_DYNAMIC,D3DFMT_X8R8G8B8,D3DPOOL_SYSTEMMEM, (IDirect3DCubeTexture8**)	&d3d_bitmap_entry[n].backup_tinterface);
			}else{
				GlobalD3DVars::lpD3DDevice->CreateTexture		(x,y,		1,D3DUSAGE_DYNAMIC,D3DFMT_X8R8G8B8,D3DPOOL_SYSTEMMEM, (IDirect3DTexture8**)		&d3d_bitmap_entry[n].backup_tinterface);
			}
		}else{
			//if this is a dynamic render target then it will not live long enough for us to care
			d3d_bitmap_entry[n].flags	|= DXT_DYNAMIC;
			d3d_bitmap_entry[n].backup_tinterface = NULL;
		}

		//now see what we actualy got
	D3DSURFACE_DESC desc;
	if(flags & BMP_FLAG_CUBEMAP){
		((IDirect3DCubeTexture8*)(d3d_bitmap_entry[n].tinterface))->GetLevelDesc(0, &desc);
	}else{
		((IDirect3DTexture8*)(d3d_bitmap_entry[n].tinterface))->GetLevelDesc(0, &desc);
	}
	x = desc.Width;
	y = desc.Height;

		//we need to know how big it was so we can rebuild it if the deviece gets lost
	d3d_bitmap_entry[n].x = x;
	d3d_bitmap_entry[n].y = y;

	*width = x;
	*height = y;

	if(!(back_depth)){
		//get the backbuffer's surface so we can return it later
		GlobalD3DVars::lpD3DDevice->GetDepthStencilSurface(&back_depth);
	}

	
	if(surface)surface->Release();
	(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(0),0,&surface);
	GlobalD3DVars::lpD3DDevice->SetRenderTarget(surface, back_depth);
	gr_clear();
	if(surface)surface->Release();
	(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(1),0,&surface);
	GlobalD3DVars::lpD3DDevice->SetRenderTarget(surface, back_depth);
	gr_clear();
	if(surface)surface->Release();
	(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(2),0,&surface);
	GlobalD3DVars::lpD3DDevice->SetRenderTarget(surface, back_depth);
	gr_clear();
	if(surface)surface->Release();
	(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(3),0,&surface);
	GlobalD3DVars::lpD3DDevice->SetRenderTarget(surface, back_depth);
	gr_clear();
	if(surface)surface->Release();
	(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(4),0,&surface);
	GlobalD3DVars::lpD3DDevice->SetRenderTarget(surface, back_depth);
	gr_clear();
	if(surface)surface->Release();
	(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(5),0,&surface);
	GlobalD3DVars::lpD3DDevice->SetRenderTarget(surface, back_depth);
	gr_clear();
	if(surface)surface->Release();
	GlobalD3DVars::lpD3DDevice->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO, &surface);
	GlobalD3DVars::lpD3DDevice->SetRenderTarget(surface, back_depth);

	return true;
#endif
}


int gr_d3d_bm_set_render_target(int n, int face)
{
	int i = 0;
	static bool once = false;
	if(!once){atexit(bm_pre_lost);once = true;}
	//some cleanup code, these have to be frees before the program terminates

#if 0
	return false;
#else
	if(n != -1){
		//data validation
		if(d3d_bitmap_entry[n].flags & DXT_CUBEMAP)Assert(face!=-1);
		//if this is a cube map then the face parameter gets used and is important

		Assert(d3d_bitmap_entry[n].tinterface != NULL);	//make sure this texture has a surface

		if(bm_bitmaps[n].type != BM_TYPE_RENDER_TARGET_STATIC && bm_bitmaps[n].type != BM_TYPE_RENDER_TARGET_DYNAMIC){
			//odds are somone passed a normal texture created with bm_load
			Error( LOCATION, "trying to set invalid bitmap as render target" );
			return false;
		}

	}else n = -1;

	if(!(back_depth)){
		//get the backbuffer's surface so we can return it later
		GlobalD3DVars::lpD3DDevice->GetDepthStencilSurface(&back_depth);
	}

	if(!texture_depth && n!= -1){
		//make the texture zbuffer if it is missing for some reason
		GlobalD3DVars::lpD3DDevice->CreateDepthStencilSurface(d3d_bitmap_entry[n].x,d3d_bitmap_entry[n].y, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, &texture_depth);
	}

	if(gr_screen.rendering_to_texture != -1 && surface && d3d_bitmap_entry[gr_screen.rendering_to_texture].backup_tinterface){
		//if the current surface is alright and if this is a static texture 
		//(dynamic render targets' backup texture is always NULL) 
		//always copy the current surface to it's backup before we change it
		IDirect3DSurface8* backup;
		if(d3d_bitmap_entry[gr_screen.rendering_to_texture].flags&DXT_CUBEMAP){
			if(gr_screen.rendering_to_face != -1){
				(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[gr_screen.rendering_to_texture].backup_tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(gr_screen.rendering_to_face),0,&backup);
				D3DXLoadSurfaceFromSurface(backup,NULL,NULL,surface,NULL,NULL, D3DX_FILTER_POINT,0);
				i = backup->Release();
			}
		}else{
			(*((IDirect3DTexture8**)(&d3d_bitmap_entry[gr_screen.rendering_to_texture].backup_tinterface)))->GetSurfaceLevel(0,&backup);
			D3DXLoadSurfaceFromSurface(backup,NULL,NULL,surface,NULL,NULL, D3DX_FILTER_POINT,0);
			i = backup->Release();
		}
	}

	//ok get rid of the last rendering surface we were useing, it isn't needed anymore
	if(surface)i = surface->Release();

	if(n == -1){
		//we are not rendering to a texture
		GlobalD3DVars::lpD3DDevice->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO, &surface);
		depth = back_depth;
//		gr_screen.max_w = gr_screen.save_max_w;
//		gr_screen.max_h = gr_screen.save_max_h;
	}else{
		i = 0;
		//make sure we clear out what ever was in it before we get a new one

		if(d3d_bitmap_entry[n].flags&DXT_CUBEMAP){
			Assert(face > -1 &&face <6);
			(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(face),0,&surface);
		}else{
			(*((IDirect3DTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetSurfaceLevel(0,&surface);
		}
		depth = texture_depth;

//		gr_screen.max_w = d3d_bitmap_entry[n].x;
//		gr_screen.max_h = d3d_bitmap_entry[n].y;
	}

	if(D3D_OK != GlobalD3DVars::lpD3DDevice->SetRenderTarget(surface, depth))return false;

	return true;
#endif
}


IDirect3DBaseTexture8* get_render_target_texture(int handle){
	int n = handle % MAX_BITMAPS;
	return d3d_bitmap_entry[n].tinterface;
}

#endif // !NO_DIRECT3D
