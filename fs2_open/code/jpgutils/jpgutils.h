/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/jpgutils/jpgutils.h $
 * $Revision: 1.3 $
 * $Date: 2004-08-11 05:06:26 $
 * $Author: Kazan $
 *
 * header for handling jpeg stuff
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/04/26 12:39:49  taylor
 * correct header to keep cvs log, minor changes
 *
 * 
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _JPEGUTILS_H
#define _JPEGUTILS_H

#include "globalincs/pstypes.h"


#define JPEG_ERROR_INVALID			-1
#define JPEG_ERROR_NONE 			0
#define JPEG_ERROR_READING			1

// reading
extern int jpeg_read_header(char *real_filename, int *w, int *h, int *bpp, ubyte *palette);
extern int jpeg_read_bitmap_32(char *real_filename, ubyte *image_data, ubyte *palette, int dest_size);


#endif // _JPEGUTILS_H
