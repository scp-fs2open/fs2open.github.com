/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/PcxUtils/pcxutils.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:27 $
 * $Author: penguin $
 *
 * header file for PCX utilities
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     8/10/99 6:54p Dave
 * Mad optimizations. Added paging to the nebula effect.
 * 
 * 5     2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 4     12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 7     9/03/97 4:32p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 6     11/26/96 9:28a Allender
 * get palette info when getting pcx info
 * 
 * 5     11/18/96 12:36p John
 * Added code to dump screen to a PCX file.
 * 
 * 4     11/13/96 4:51p Allender
 * started overhaul of bitmap manager.  bm_load no longer actually load
 * the data, only the info for the bitmap.  Locking the bitmap now forces
 * load when no data present (or will if bpp changes)
 *
 * $NoKeywords: $
 */

#ifndef _PCXUTILS_H
#define _PCXUTILS_H

#include "pstypes.h"

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

#define PCX_ERROR_NONE 				0
#define PCX_ERROR_OPENING			1
#define PCX_ERROR_NO_HEADER		2
#define PCX_ERROR_WRONG_VERSION	3
#define PCX_ERROR_READING			4
#define PCX_ERROR_NO_PALETTE		5
#define PCX_ERROR_WRITING			6
#define PCX_ERROR_MEMORY			7

extern int pcx_read_header(char *filename, int *w, int *h, ubyte *pal );
extern int pcx_read_bitmap_8bpp( char * filename, ubyte *org_data, ubyte *palette );
extern int pcx_read_bitmap_16bpp( char * filename, ubyte *org_data );
extern int pcx_read_bitmap_16bpp_aabitmap( char *filename, ubyte *org_data );
extern int pcx_read_bitmap_16bpp_nondark( char *filename, ubyte *org_data );


// Dumps a 8bpp bitmap to a file.
// Set rowoff to -w for upside down bitmaps.
extern int pcx_write_bitmap( char * filename, int w, int h, ubyte ** row_ptrs, ubyte * palette );


/*
#ifdef __cplusplus
}
#endif
*/

#endif
