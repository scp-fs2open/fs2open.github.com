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
 * $Revision: 2.7 $
 * $Date: 2005-11-13 06:44:18 $
 * $Author: taylor $
 *
 * header file for PCX utilities
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.6  2005/07/13 03:35:34  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.5  2005/02/04 10:12:32  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.4  2004/10/31 22:00:57  taylor
 * new bmpman merge support, add PreProcDefines.h a few new places
 *
 * Revision 2.3  2004/08/11 05:06:32  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2003/11/19 20:37:24  randomtiger
 * Almost fully working 32 bit pcx, use -pcx32 flag to activate.
 * Made some commandline variables fit the naming standard.
 * Changed timerbar system not to run pushes and pops if its not in use.
 * Put in a note about not uncommenting asserts.
 * Fixed up a lot of missing POP's on early returns?
 * Perhaps the motivation for Assert functionality getting commented out?
 * Fixed up some bad asserts.
 * Changed nebula poofs to render in 2D in htl, it makes it look how it used to in non htl. (neb.cpp,1248)
 * Before the poofs were creating a nasty stripe effect where they intersected with ships hulls.
 * Put in a special check for the signs of that D3D init bug I need to lock down.
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
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

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"


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

extern int pcx_read_header(char *filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *pal = NULL );
//extern int pcx_read_bitmap_8bpp( char * filename, ubyte *org_data, ubyte *palette );
//extern int pcx_read_bitmap_16bpp( char * filename, ubyte *org_data );
//extern int pcx_read_bitmap_16bpp_aabitmap( char *filename, ubyte *org_data );
//extern int pcx_read_bitmap_16bpp_nondark( char *filename, ubyte *org_data );
//extern int pcx_read_bitmap_32(char *real_filename, ubyte *data );
extern int pcx_read_bitmap(char *filename, ubyte *org_data, ubyte *pal, int byte_size, int aabitmap = 0, int nondark = 0, int cf_type = CF_TYPE_ANY);

// Dumps a 8bpp bitmap to a file.
// Set rowoff to -w for upside down bitmaps.
extern int pcx_write_bitmap( char * filename, int w, int h, ubyte ** row_ptrs, ubyte * palette );


/*
#ifdef __cplusplus
}
#endif
*/

#endif
