/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/TgaUtils/TgaUtils.h $
 * $Revision: 2.4 $
 * $Date: 2005-02-04 10:12:33 $
 * $Author: taylor $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/10/31 22:00:57  taylor
 * new bmpman merge support, add PreProcDefines.h a few new places
 *
 * Revision 2.2  2004/08/11 05:06:35  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/04/26 02:14:38  taylor
 * 32-bit support with DevIL
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     3/20/99 3:46p Dave
 * Added support for model-based background nebulae. Added 3 new
 * sexpressions.
 * 
 * 2     12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 *  
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef __TARGA_H
#define __TARGA_H

struct CFILE;

// --------------------
//
// Defines
//
// --------------------

#define TARGA_ERROR_NONE			0
#define TARGA_ERROR_READING		1
#define TARGA_ERROR_WRITING		2

// --------------------
//
// Prototypes
//
// --------------------

int targa_read_header(char *filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *palette=NULL );
int targa_read_bitmap(char *filename, ubyte *data, ubyte *palette, int dest_size );
int targa_write_bitmap(char *filename, ubyte *data, ubyte *palette, int w, int h, int bpp);

// The following are used by the tools\vani code.
int targa_compress(char *out, char *in, int outsize, int pixsize, int bytecount);
int targa_uncompress( ubyte *dst, ubyte *src, int bitmap_width, int bytes_per_pixel );

#endif // __TARGA_H
