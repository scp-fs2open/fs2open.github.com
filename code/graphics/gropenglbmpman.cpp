/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"
#include "ddsutils/ddsutils.h"
#include "tgautils/tgautils.h"
#include "pngutils/pngutils.h"
#include "jpgutils/jpgutils.h"
#include "pcxutils/pcxutils.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropenglextension.h"
#include "globalincs/systemvars.h"
#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "cmdline/cmdline.h"

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
int gr_opengl_bm_load(ubyte type, int n, char *filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *c_type, int *mm_lvl, int *size)
{
	int dds_ct;

	if (type == BM_TYPE_DDS) {
		int dds_error = dds_read_header( filename, img_cfp, w, h, bpp, &dds_ct, mm_lvl, size );

		if (dds_error != DDS_ERROR_NONE) {
			mprintf(("DDS ERROR: Couldn't open '%s' -- %s\n", filename, dds_error_string(dds_error)));
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

			case DDS_CUBEMAP_DXT1:
				*c_type = BM_TYPE_CUBEMAP_DXT1;
				break;

			case DDS_CUBEMAP_DXT3:
				*c_type = BM_TYPE_CUBEMAP_DXT3;
				break;

			case DDS_CUBEMAP_DXT5:
				*c_type = BM_TYPE_CUBEMAP_DXT5;
				break;

			case DDS_CUBEMAP_UNCOMPRESSED:
				*c_type = BM_TYPE_CUBEMAP_DDS;
				break;

			default:
				Error(LOCATION, "Bad DDS file compression! Not using DXT1,3,5: %s", filename);
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
	// if its a png file
 	else if (type == BM_TYPE_PNG) {
 		int png_error=png_read_header( filename, img_cfp, w, h, bpp, NULL );
 		if ( png_error != PNG_ERROR_NONE ) {
 			mprintf(( "png: Couldn't open '%s'\n", filename ));
 			return -1;
 		}
 	}
	// if its a jpg file
	else if (type == BM_TYPE_JPG) {
		int jpg_error=jpeg_read_header( filename, img_cfp, w, h, bpp, NULL );
		if ( jpg_error != JPEG_ERROR_NONE ) {
			mprintf(( "jpg: Couldn't open '%s'\n", filename ));
			return -1;
		}
	}
	// if its a pcx file
	else if (type == BM_TYPE_PCX) {
		int pcx_error = pcx_read_header( filename, img_cfp, w, h, bpp, NULL );
		if ( pcx_error != PCX_ERROR_NONE )	{
			mprintf(( "pcx: Couldn't open '%s'\n", filename ));
			return -1;
		}
	} else {
		Assert( 0 );

		return -1;
	}

	return 0;
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
extern int opengl_compress_image( ubyte **out_data, ubyte *in_data, int width, int height, int alpha = 1, int num_mipmaps = 1 );

static int opengl_bm_lock_ani_compress( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	anim			*the_anim;
	anim_instance	*the_anim_instance;
	bitmap			*bm;
	ubyte			*frame_data;
	int				size, i;
	int				first_frame, nframes;
	ubyte *compressed_data = NULL;
	int out_size = 0;
	int alpha = 0;
	int num_mipmaps = 1;

	first_frame = be->info.ani.first_frame;
	nframes = bm_bitmaps[first_frame].info.ani.num_frames;

	// bpp can always be 24-bit since we don't do images with alpha here
	bpp = 24;

	alpha = (bpp == 32);

	if ( (the_anim = anim_load(bm_bitmaps[first_frame].filename, bm_bitmaps[first_frame].dir_type)) == NULL ) {
		nprintf(("BMPMAN", "Error opening %s in bm_lock\n", be->filename));
		return 1;
	}

	if ( (the_anim_instance = init_anim_instance(the_anim, bpp)) == NULL ) {
		nprintf(("BMPMAN", "Error opening %s in bm_lock\n", be->filename));
		anim_free(the_anim);
		return 1;
	}

	int can_drop_frames = 0;

	if ( the_anim->total_frames != bm_bitmaps[first_frame].info.ani.num_frames )	{
		can_drop_frames = 1;
	}

	bm = &bm_bitmaps[first_frame].bm;
	size = bm->w * bm->h * (bpp >> 3);

	num_mipmaps = get_num_mipmap_levels( bm->w, bm->h );
	Assert( num_mipmaps > 0 );

	nprintf(("BMPMAN", "Attempting to compress '%s' with %d frames, original size %.3fM ... ", bm_bitmaps[first_frame].filename, nframes, ((float)(size*nframes)/1024.0f)/1024.0f));

	for ( i=0; i<nframes; i++ )	{
		be = &bm_bitmaps[first_frame+i];
		bmp = &bm_bitmaps[first_frame+i].bm;

		// Unload any existing data
		bm_clean_slot( first_frame+i );

		bmp->flags = 0;

		bmp->bpp = bpp;

		frame_data = anim_get_next_raw_buffer(the_anim_instance, 0, 0, bmp->bpp);

		Assert( frame_data != NULL );

		out_size = opengl_compress_image(&compressed_data, frame_data, bmp->w, bmp->h, alpha, num_mipmaps);

		if (out_size == 0) {
			if (compressed_data != NULL) {
				vm_free(compressed_data);
				compressed_data = NULL;
			}

			nprintf(("BMPMAN", "compression failed!!\n"));

			return 1;
		}

		Assert( compressed_data != NULL );

		bmp->data = (ptr_u)compressed_data;
		bmp->bpp = (alpha) ? 32 : 24;
		bmp->palette = NULL;
		be->comp_type = (alpha) ? BM_TYPE_DXT5 : BM_TYPE_DXT1;
		be->mem_taken = out_size;
		be->num_mipmaps = num_mipmaps;

		bm_update_memory_used( first_frame + i, out_size );

		// Skip a frame
		if ( (i < nframes-1)  && can_drop_frames )	{
			frame_data = anim_get_next_raw_buffer(the_anim_instance, 0, 0, bm->bpp);
		}

		compressed_data = NULL;
	}

	if (num_mipmaps > 1) {
		nprintf(("BMPMAN", "new size is %.3fM with %d mipmap levels.\n", ((float)(out_size*nframes)/1024.0f)/1024.0f, num_mipmaps));
	} else {
		nprintf(("BMPMAN", "new size is %.3fM.\n", ((float)(out_size*nframes)/1024.0f)/1024.0f));
	}

	free_anim_instance(the_anim_instance);
	anim_free(the_anim);

	return 0;
}

static int opengl_bm_lock_compress( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, int bpp, int flags )
{
	ubyte *data = NULL;
	ubyte *compressed_data = NULL;
	int error_code = 1;
	int out_size = 0;
	int byte_size = 0;
	int alpha = 1;
	int num_mipmaps = 1;

	// don't use for EFFs, if they were wanting to be DDS then they should already be that way
	if ( be->type == BM_TYPE_EFF ) {
		return 1;
	}

	Assert( (be->type == BM_TYPE_PCX) ||
			(be->type == BM_TYPE_TGA) ||
			(be->type == BM_TYPE_PNG) ||
			(be->type == BM_TYPE_JPG) );

	Assert( !(flags & BMP_AABITMAP) );

	// PCX need to be loaded as 32-bit
	if ( (be->type == BM_TYPE_PCX) && (bpp == 16) )
		bpp = 32;

	bm_clean_slot( bitmapnum );

	byte_size = (bpp >> 3);
	Assert( (byte_size == 3) || (byte_size == 4) );
	alpha = (byte_size != 3);

	data = (ubyte*)vm_malloc(bmp->w * bmp->h * byte_size);

	Assert( data != NULL );

	if ( data == NULL )
		return 1;

	// read in bitmap data
	if (be->type == BM_TYPE_PCX) {
		Assert( bpp == 32 );
		error_code = pcx_read_bitmap( be->filename, data, NULL, byte_size );
	} else if (be->type == BM_TYPE_TGA) {
		error_code = targa_read_bitmap( be->filename, data, NULL, byte_size );
 	} else if (be->type == BM_TYPE_PNG) {
 		error_code = png_read_bitmap( be->filename, data, NULL, byte_size );
	} else if (be->type == BM_TYPE_JPG) {
		error_code = jpeg_read_bitmap( be->filename, data, NULL, byte_size );
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

	num_mipmaps = get_num_mipmap_levels( bmp->w, bmp->h );
	Assert( num_mipmaps > 0 );

	// now for the attempt to compress the data
	out_size = opengl_compress_image(&compressed_data, data, bmp->w, bmp->h, alpha, num_mipmaps);

	if (data != NULL) {
		vm_free(data);
		data = NULL;
	}

	if (out_size == 0) {
		if (compressed_data != NULL) {
			vm_free(compressed_data);
			compressed_data = NULL;
		}

		nprintf(("BMPMAN", "compression failed!!\n"));

		return 1;
	}

	Assert( compressed_data != NULL );

	if (num_mipmaps > 1) {
		nprintf(("BMPMAN", "new size is %.3fM with %d mipmap levels.\n", ((float)out_size/1024.0f)/1024.0f, num_mipmaps));
	} else {
		nprintf(("BMPMAN", "new size is %.3fM.\n", ((float)out_size/1024.0f)/1024.0f));
	}

	bmp->data = (ptr_u)compressed_data;
	bmp->bpp = (alpha) ? (ubyte)32 : (ubyte)24;
	bmp->palette = NULL;
	be->comp_type = (alpha) ? BM_TYPE_DXT5 : BM_TYPE_DXT1;
	be->mem_taken = out_size;
	be->num_mipmaps = num_mipmaps;

	bm_update_memory_used( bitmapnum, out_size );

	return 0;
}

extern bool opengl_texture_slot_valid(int n, int handle);

/**
 * Lock an image files data into memory
 */
int gr_opengl_bm_lock( char *filename, int handle, int bitmapnum, ubyte bpp, ubyte flags )
{
	ubyte c_type = BM_TYPE_NONE;
	ubyte true_bpp;
	int try_compress = 0;

	bitmap_entry *be = &bm_bitmaps[bitmapnum];
	bitmap *bmp = &be->bm;

	Assert( !Is_standalone );

	if (bmp->true_bpp > bpp)
		true_bpp = bmp->true_bpp;
	else
		true_bpp = bpp;

	// don't do a bpp check here since it could be different in OGL - taylor
	if ( (bmp->data == 0) && !opengl_texture_slot_valid(bitmapnum, handle) ) {
		Assert(be->ref_count == 1);

		if (be->type != BM_TYPE_USER) {
			if (bmp->data == 0)
				nprintf (("BmpMan", "Loading %s for the first time.\n", be->filename));
		}

		if ( !Bm_paging )	{
			if (be->type != BM_TYPE_USER)						
				nprintf(("Paging", "Loading %s (%dx%dx%d)\n", be->filename, bmp->w, bmp->h, true_bpp));
		}

		// select proper format
		if (flags & BMP_AABITMAP)
			BM_SELECT_ALPHA_TEX_FORMAT();
		else if (flags & BMP_TEX_ANY)
			BM_SELECT_TEX_FORMAT();					
		else
			BM_SELECT_SCREEN_FORMAT();

		// make sure we use the real graphic type for EFFs
		if ( be->type == BM_TYPE_EFF ) {
			c_type = be->info.ani.eff.type;
		} else {
			c_type = be->type;
		}

		try_compress = (Cmdline_img2dds && Texture_compression_available && is_power_of_two(bmp->w, bmp->h) && !(flags & BMP_AABITMAP) && !Is_standalone);

		switch ( c_type )
		{
			case BM_TYPE_PCX:
				if (try_compress && (true_bpp >= 16)) {
					if ( !opengl_bm_lock_compress(handle, bitmapnum, be, bmp, true_bpp, flags) ) {
						break;
					}
				}

				bm_lock_pcx( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;

			case BM_TYPE_ANI:
				if (try_compress && (true_bpp >= 16) && !(flags & BMP_TEX_XPARENT)) {
					if ( !opengl_bm_lock_ani_compress(handle, bitmapnum, be, bmp, true_bpp, flags) ) {
						break;
					}
				}

				bm_lock_ani( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;

			case BM_TYPE_TGA:
				if (try_compress && (true_bpp >= 24)) {
					if ( !opengl_bm_lock_compress(handle, bitmapnum, be, bmp, true_bpp, flags) ) {
						break;
					}
				}

				bm_lock_tga( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;

 			case BM_TYPE_PNG:
 				//libpng handles compression with zlib
 				bm_lock_png( handle, bitmapnum, be, bmp, true_bpp, flags );
 				break;

			case BM_TYPE_JPG:
				if (try_compress) {
					if ( !opengl_bm_lock_compress(handle, bitmapnum, be, bmp, true_bpp, flags) ) {
						break;
					}
				}

				bm_lock_jpg( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;

			case BM_TYPE_DDS:
			case BM_TYPE_DXT1:
			case BM_TYPE_DXT3:
			case BM_TYPE_DXT5:
			case BM_TYPE_CUBEMAP_DDS:
			case BM_TYPE_CUBEMAP_DXT1:
			case BM_TYPE_CUBEMAP_DXT3:
			case BM_TYPE_CUBEMAP_DXT5:
				bm_lock_dds( handle, bitmapnum, be, bmp, true_bpp, flags );
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

		// make sure we actually did something
		if ( !(bmp->data) )
			return -1;
	}

	return 0;
}

void gr_opengl_bm_save_render_target(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	if ( !Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) || Cmdline_no_fbo ) {
		return;
	}

	bitmap_entry *be = &bm_bitmaps[n];
	bitmap *bmp = &be->bm;

	int rc = opengl_export_image(n, bmp->w, bmp->h, (bmp->true_bpp == 32), be->num_mipmaps, (ubyte*)bmp->data);

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
