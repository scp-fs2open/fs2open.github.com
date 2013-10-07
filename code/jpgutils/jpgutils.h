/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _JPEGUTILS_H
#define _JPEGUTILS_H

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"


#define JPEG_ERROR_INVALID			-1
#define JPEG_ERROR_NONE 			0
#define JPEG_ERROR_READING			1

// reading
extern int jpeg_read_header(const char *real_filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *palette = NULL);
extern int jpeg_read_bitmap(const char *real_filename, ubyte *image_data, ubyte *palette, int dest_size, int cf_type = CF_TYPE_ANY);


#endif // _JPEGUTILS_H
