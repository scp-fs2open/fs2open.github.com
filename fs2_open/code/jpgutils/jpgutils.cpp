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
 * $Revision: 1.17 $
 * $Date: 2005-03-11 01:37:35 $
 * $Author: wmcoolmon $
 * 
 * source for handling jpeg stuff
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.16  2005/03/11 01:34:19  wmcoolmon
 * Not really. I've got lib files in my main VC.NET libs directory, and include files in grouped subdirectories under my VC.NET include directory. it keeps my HD free of scattered SDK directories and helps keep conflicts down.
 *
 * Revision 1.15  2005/03/10 16:16:52  taylor
 * maybe WMC can leave the include alone now ;)
 *
 * Revision 1.14  2005/03/06 11:32:40  wmcoolmon
 * Whoops!
 *
 * Revision 1.13  2005/03/06 11:23:45  wmcoolmon
 * RE-fixed stuff. Ogg support. Briefings.
 *
 * Revision 1.12  2005/03/03 07:32:55  wmcoolmon
 * Oops
 *
 * Revision 1.11  2005/03/03 06:05:28  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 1.10  2005/02/12 10:44:11  taylor
 * fix possible crash in bm_get_section_size()
 * get jpeg_read_header() working properly
 * VBO fixes and minor optimizations
 *
 * Revision 1.9  2005/02/07 08:33:14  taylor
 * should fix linker error in libjpeg
 *
 * Revision 1.8  2005/02/05 04:15:35  taylor
 * more post merge happiness
 *
 * Revision 1.7  2005/02/04 10:12:30  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 1.6  2004/10/31 22:00:56  taylor
 * new bmpman merge support, add PreProcDefines.h a few new places
 *
 * Revision 1.5  2004/10/10 16:59:37  taylor
 * holy crap at the copy-paste error
 *
 * Revision 1.4  2004/07/26 20:47:35  Kazan
 * remove MCD complete
 *
 * Revision 1.3  2004/07/12 16:32:51  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.2  2004/04/26 12:39:49  taylor
 * correct header to keep cvs log, minor changes
 *
 * 
 * $NoKeywords: $
 */

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#ifndef WMC
#include "jpeglib.h"
#else
extern "C" {
#include <libjpeg/jpeglib.h>
}
#endif

#undef LOCAL // fix from a jpeg header, pstypes.h will define it again

#include "globalincs/pstypes.h"
#include "jpgutils/jpgutils.h"
#include "cfile/cfile.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/2d.h"


// forward declarations
void jpeg_cfile_src(j_decompress_ptr cinfo, CFILE *cfp);

// structs
typedef struct {
  struct jpeg_source_mgr pub;	// public fields

  CFILE *infile;		// source stream
  JOCTET *buffer;		// start of buffer
  boolean start_of_file;	// have we gotten any data yet?
} cfile_source_mgr;

typedef cfile_source_mgr *cfile_src_ptr;
struct jpeg_decompress_struct jpeg_info;
struct jpeg_error_mgr jpeg_err;

#define INPUT_BUF_SIZE  4096	// choose an efficiently read'able size

static int jpeg_error_code;

// set current error
#define Jpeg_Set_Error(x)	{ jpeg_error_code = x; }

// error handler stuff, rather than the default, which will screw us
//
jmp_buf FSJpegError;

// error (exit) handler
void jpg_error_exit(j_common_ptr cinfo)
{
	// give an error message that isn't just generic
	(*cinfo->err->output_message) (cinfo);

	// do cleanup since we won't get the chance otherwise
	jpeg_destroy(cinfo);

	// set the error code, pretty much always going to be a read error
	Jpeg_Set_Error(JPEG_ERROR_READING);

	// bail
	longjmp(FSJpegError, 1);
}

// message handler for errors
void jpg_output_message(j_common_ptr cinfo)
{
	// JMSG_LENGTH_MAX should be 200, DO NOT change from this define so
	// that we don't risk overruns from the jpeg lib
	char buffer[JMSG_LENGTH_MAX];

	// Create the message 
	(*cinfo->err->format_message) (cinfo, buffer);

	// don't actually output anything unless we are a debug build, let bmpman
	// give any errors instead for release builds
#ifndef NDEBUG
	Warning(LOCATION, "%s %s", "JPEG Error:", buffer);
#endif
}



// Reads header information from the JPEG file into the bitmap pointer
// 
// filename - name of the JPEG bitmap file
// w - (output) width of the bitmap
// h - (output) height of the bitmap
// bpp - (output) bits per pixel of the bitmap
//
// returns - JPEG_ERROR_NONE if successful, otherwise error code
//
int jpeg_read_header(char *real_filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *palette)
{
	CFILE *jpeg_file = NULL;
	char filename[MAX_FILENAME_LEN];

	if (img_cfp == NULL) {
		strcpy( filename, real_filename );

		char *p = strchr( filename, '.' );

		if ( p )
			*p = 0;

		strcat( filename, ".jpg" );

		jpeg_file = cfopen( filename , "rb" );

		if ( !jpeg_file ) {
			return JPEG_ERROR_READING;
		}
	} else {
		jpeg_file = img_cfp;
	}

	Assert( jpeg_file != NULL );

	if (jpeg_file == NULL)
		return JPEG_ERROR_READING;

	// set the basic/default error code
	Jpeg_Set_Error(JPEG_ERROR_NONE);

	// initialize error message handler and decompression struct
	jpeg_info.err = jpeg_std_error(&jpeg_err);
	jpeg_err.error_exit = jpg_error_exit;
	jpeg_err.output_message = jpg_output_message;

	jpeg_create_decompress(&jpeg_info);

	// setup to read data via CFILE
	jpeg_cfile_src(&jpeg_info, jpeg_file);

	jpeg_read_header(&jpeg_info, TRUE);

	// send the info back out
	if (w) *w = jpeg_info.image_width;
	if (h) *h = jpeg_info.image_height;
	if (bpp) *bpp = (jpeg_info.num_components * 8);

	// cleanup
	jpeg_destroy_decompress(&jpeg_info);

	if (img_cfp == NULL) {
		cfclose(jpeg_file);
		jpeg_file = NULL;
	}

	return jpeg_error_code;
}


// Loads a JPEG image
// 
// filename - name of the targa file to load
// image_data - allocated storage for the bitmap
//
// returns - true if succesful, false otherwise
//
int jpeg_read_bitmap(char *real_filename, ubyte *image_data, ubyte *palette, int dest_size)
{
	char filename[MAX_FILENAME_LEN];
	CFILE *img_cfp = NULL;
	JSAMPARRAY buffer = NULL;
	int rc = 0;

	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".jpg" );


	img_cfp = cfopen(filename, "rb");

	if (img_cfp == NULL)
		return JPEG_ERROR_READING;

	// set the basic error code
	Jpeg_Set_Error(JPEG_ERROR_NONE);

	// initialize error message handler
	jpeg_info.err = jpeg_std_error(&jpeg_err);
	jpeg_err.error_exit = jpg_error_exit;
	jpeg_err.output_message = jpg_output_message;

	// SPECIAL NOTE: we've already allocated memory on the basis of original height, width and bpp
	// of the image from the header.  DO NOT change any settings that affect output variables here
	// or we risk needed more memory than we have available in "image_data".  The only exception is
	// bpp since 'dest_size' should already indicate what we want to end up with.

	if ( (rc = setjmp( FSJpegError ) == 0) != FALSE ) {
		// initialize decompression struct
		jpeg_create_decompress(&jpeg_info);

		// setup to read data via CFILE
		jpeg_cfile_src(&jpeg_info, img_cfp);

		jpeg_read_header(&jpeg_info, TRUE);

		// memory for the storage of each scanline
		jpeg_calc_output_dimensions(&jpeg_info);

		// set the output components to be 'dest_size' (so we can support 16/24/32-bit images with this one function)
		jpeg_info.output_components = dest_size;
		jpeg_info.out_color_components = dest_size;	// may need/have to match above

		// multiplying by rec_outbuf_height isn't required but is more efficient
		int size = jpeg_info.output_width * jpeg_info.output_components * jpeg_info.rec_outbuf_height;
		// a standard malloc doesn't appear to work properly here (debug vm_malloc??), crashes in lib
		buffer = (*jpeg_info.mem->alloc_sarray)((j_common_ptr) &jpeg_info, JPOOL_IMAGE, size, 1); // DON'T free() THIS! jpeg lib does it

		// begin decompression process --
		jpeg_start_decompress(&jpeg_info);

		// read each scanline and output to previously allocated 'image_data'
		while (jpeg_info.output_scanline < jpeg_info.output_height) {
			jpeg_read_scanlines(&jpeg_info, buffer, 1);

			memcpy(image_data, buffer[0], size); // have to use [0] here or the output is wrong
			image_data += size;
		}

		// -- done with decompression process
		jpeg_finish_decompress(&jpeg_info);

		// cleanup
		jpeg_destroy_decompress(&jpeg_info);
	}

	cfclose(img_cfp);


	return jpeg_error_code;
}




// --------------------- handlers for reading of files -------------------------
// ---- basic copy of source from jdatasrc.c, originally:
//		Copyright (C) 1994-1996, Thomas G. Lane.

void jpeg_cf_init_source(j_decompress_ptr cinfo)
{
	cfile_src_ptr src = (cfile_src_ptr) cinfo->src;

	// We reset the empty-input-file flag for each image,
	// but we don't clear the input buffer.
	// This is correct behavior for reading a series of images from one source.
	src->start_of_file = TRUE;
}

boolean jpeg_cf_fill_input_buffer(j_decompress_ptr cinfo)
{
	cfile_src_ptr src = (cfile_src_ptr) cinfo->src;
	size_t nbytes;

	nbytes = cfread(src->buffer, 1, INPUT_BUF_SIZE, src->infile);

	if (nbytes <= 0) {
		if (src->start_of_file) {	// Treat empty input file as fatal error
			Jpeg_Set_Error(JPEG_ERROR_READING);
		}

		// Insert a fake EOI marker
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;

		return FALSE;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}

void jpeg_cf_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	cfile_src_ptr src = (cfile_src_ptr) cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
			num_bytes -= (long) src->pub.bytes_in_buffer;

			if (!jpeg_cf_fill_input_buffer(cinfo))
				return;
		}

		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}

void jpeg_cf_term_source(j_decompress_ptr cinfo)
{
	// no work necessary here
}

void jpeg_cfile_src(j_decompress_ptr cinfo, CFILE *cfp)
{
	cfile_src_ptr src;

	if (cinfo->src == NULL) {	// first time for this JPEG object?
		cinfo->src = (struct jpeg_source_mgr *) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(cfile_source_mgr));
		src = (cfile_src_ptr) cinfo->src;
		src->buffer = (JOCTET *) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
	}

	src = (cfile_src_ptr) cinfo->src;
	src->pub.init_source = jpeg_cf_init_source;
	src->pub.fill_input_buffer = jpeg_cf_fill_input_buffer;
	src->pub.skip_input_data = jpeg_cf_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method
	src->pub.term_source = jpeg_cf_term_source;
	src->infile = cfp;
	src->pub.bytes_in_buffer = 0; // forces fill_input_buffer on first read
	src->pub.next_input_byte = NULL; // until buffer loaded
}
