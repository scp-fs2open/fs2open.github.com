/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _PCXUTILS_H
#define _PCXUTILS_H

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"


#define PCX_ERROR_NONE 				0
#define PCX_ERROR_OPENING			1
#define PCX_ERROR_NO_HEADER		2
#define PCX_ERROR_WRONG_VERSION	3
#define PCX_ERROR_READING			4
#define PCX_ERROR_NO_PALETTE		5
#define PCX_ERROR_WRITING			6
#define PCX_ERROR_MEMORY			7

extern int pcx_read_header(const char *filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *pal = NULL );
extern int pcx_read_bitmap(const char *filename, ubyte *org_data, ubyte *pal, int byte_size, int aabitmap = 0, bool mask_bitmap = false, int cf_type = CF_TYPE_ANY);

// Dumps a 8bpp bitmap to a file.
// Set rowoff to -w for upside down bitmaps.
extern int pcx_write_bitmap( const char * filename, int w, int h, ubyte ** row_ptrs, ubyte * palette );

#endif
