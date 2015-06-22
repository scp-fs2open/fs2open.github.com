#ifndef _PNGUTILS_H
#define _PNGUTILS_H

#include "cfile/cfile.h"
#include "globalincs/pstypes.h"

#define PNG_ERROR_INVALID		-1
#define PNG_ERROR_NONE 			0
#define PNG_ERROR_READING		1

// reading
extern int png_read_header(const char *real_filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *palette = NULL);
extern int png_read_bitmap(const char *real_filename, ubyte *image_data, ubyte *bpp, int dest_size, int cf_type = CF_TYPE_ANY);

#endif // _PNGUTILS_H
