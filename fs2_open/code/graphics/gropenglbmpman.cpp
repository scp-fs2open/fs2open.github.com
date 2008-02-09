/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Graphics/gropenglbmpman.cpp $
 * $Revision: 1.3 $
 * $Date: 2004-12-05 01:28:39 $
 * $Author: taylor $
 *
 * OpenGL specific bmpman routines
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/11/23 00:10:06  taylor
 * try and protect the bitmap_entry stuff a bit better
 * fix the transparent support ship, again, but correctly this time
 *
 * Revision 1.1  2004/10/31 21:21:11  taylor
 * initial import from bmpman merge
 *
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"
#include "ddsutils/ddsutils.h"
#include "tgautils/tgautils.h"
#include "jpgutils/jpgutils.h"
#include "pcxutils/pcxutils.h"
#include "graphics/gropengltexture.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"


// anything API specific to freeing bm data
void gr_opengl_bm_free_data(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );
}

// API specifics for creating a user bitmap
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
		int pcx_error = pcx_read_header( filename, img_cfp, w, h, NULL );
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

// API specific init instructions
void gr_opengl_bm_init(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );
}

// specific instructions for setting up the start of a page-in session
void gr_opengl_bm_page_in_start()
{
	gr_opengl_preload_init();
}

// Lock an image files data into memory
int gr_opengl_bm_lock( char *filename, int handle, int bitmapnum, ubyte bpp, ubyte flags )
{
	ubyte c_type = BM_TYPE_NONE;

	bitmap_entry *be = &bm_bitmaps[bitmapnum];
	bitmap *bmp = &be->bm;

	// don't do a bpp check here since it could be different in OGL - taylor
	if ( (bmp->data == 0) ) {
		Assert(be->ref_count == 1);

		if ( be->type != BM_TYPE_USER ) {
			if ( bmp->data == 0 ) {
				nprintf (("BmpMan","Loading %s for the first time.\n", be->filename));
			}
		}

		if ( !Bm_paging )	{
			if ( be->type != BM_TYPE_USER ) {
				char flag_text[64];
				strcpy( flag_text, "--" );							
				nprintf(( "Paging", "Loading %s (%dx%dx%dx%s)\n", be->filename, bmp->w, bmp->h, bpp, flag_text ));
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
			c_type = be->info.eff.type;
		} else {
			c_type = be->type;
		}

		switch ( c_type ) {
			case BM_TYPE_PCX:
				bm_lock_pcx( handle, bitmapnum, be, bmp, bpp, flags );
				break;

			case BM_TYPE_ANI:
				bm_lock_ani( handle, bitmapnum, be, bmp, bpp, flags );
				break;

			case BM_TYPE_USER:	
				bm_lock_user( handle, bitmapnum, be, bmp, bpp, flags );
				break;

			case BM_TYPE_TGA:
				bm_lock_tga( handle, bitmapnum, be, bmp, bpp, flags );
				break;

			case BM_TYPE_JPG:
				bm_lock_jpg( handle, bitmapnum, be, bmp, bpp, flags );
				break;

			case BM_TYPE_DDS:
			case BM_TYPE_DXT1:
			case BM_TYPE_DXT3:
			case BM_TYPE_DXT5:
				bm_lock_dds( handle, bitmapnum, be, bmp, bpp, flags );
				break;

			default:
				Warning(LOCATION, "Unsupported type in bm_lock -- %d\n", c_type );
				return -1;
		}		

		// always go back to screen format
		BM_SELECT_SCREEN_FORMAT();
	}

	// make sure we actually did something
	if ( !(bmp->data) ) {
		// crap, bail...
		return -1;
	}

	return 0;
}
