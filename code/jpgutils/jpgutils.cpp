/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/jpgutils/jpgutils.cpp $
 * $Revision: 1.2 $
 * $Date: 2004-04-26 12:39:49 $
 * $Author: taylor $
 * 
 * source for handling jpeg stuff
 * 
 * $Log: not supported by cvs2svn $
 * 
 * $NoKeywords: $
 */

#include <stdio.h>
#include <string.h>

#include "globalincs/pstypes.h"
#include "jpgutils/jpgutils.h"
#include "cfile/cfile.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/2d.h"
#include "openil/il_func.h"


// a placeholder for the future
#define jpeg_read_bitmap(a, b, c ,d)	jpeg_read_bitmap_32(a, b, c, d)



// Reads header information from the JPEG file into the bitmap pointer
// 
// filename - name of the JPEG bitmap file
// w - (output) width of the bitmap
// h - (output) height of the bitmap
// bpp - (output) bits per pixel of the bitmap
//
// returns - JPEG_ERROR_NONE if successful, otherwise error code
//
int jpeg_read_header(char *real_filename, int *w, int *h, int *bpp, ubyte *palette)
{
#ifndef USE_DEVIL_JPG
	// non-DevIL stuff...
#else
	char filename[MAX_FILENAME_LEN];
	ILuint jpgimage;
	ILint ilw, ilh;
		
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".jpg" );

	ilGenImages(1,&jpgimage);
	ilBindImage(jpgimage);

	if (ilLoadImage(filename) == IL_FALSE) {
		mprintf(( "Couldn't open '%s' -- some kind of devIL-error: %d\n", filename, iluErrorString(ilGetError()) ));
		ilDeleteImages(1,&jpgimage);
		return JPEG_ERROR_READING;
	}

	ilGetIntegerv(IL_IMAGE_WIDTH,&ilw);
	ilGetIntegerv(IL_IMAGE_HEIGHT,&ilh);

	*w = ilw;
	*h = ilh;

	ilDeleteImages(1,&jpgimage);
#endif

	return JPEG_ERROR_NONE;
}


// Loads a JPEG image, produces 32-bit data
// 
// filename - name of the targa file to load
// image_data - allocated storage for the bitmap
//
// returns - true if succesful, false otherwise
//
int jpeg_read_bitmap_32(char *real_filename, ubyte *image_data, ubyte *palette, int dest_size)
{
#ifndef USE_DEVIL_JPG
	// non-DevIL stuff...
#else
	char filename[MAX_FILENAME_LEN];
	ILuint jpgimage;
	ILint ilsize;
		
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".tga" );

	ilGenImages(1,&jpgimage);
	ilBindImage(jpgimage);

	if (ilLoadImage(filename) == IL_FALSE) {
		mprintf(("Couldn't open '%s -- some kind of devIL-error: %d\n", filename, iluErrorString(ilGetError())));
		ilDeleteImages(1,&jpgimage);
		return JPEG_ERROR_READING;
	}

	// convert to BGRA if not already there
	ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);

	ilGetIntegerv(IL_IMAGE_SIZE_OF_DATA,&ilsize);

	memcpy(image_data, ilGetData(), ilsize);

	ilDeleteImages(1,&jpgimage);
#endif

	return JPEG_ERROR_NONE;
}
