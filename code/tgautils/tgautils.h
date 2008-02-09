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
 * $Revision: 2.1 $
 * $Date: 2004-04-26 02:14:38 $
 * $Author: taylor $
 *
 *
 * $Log: not supported by cvs2svn $
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


#ifndef __TARGA_H
#define __TARGA_H

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

int targa_read_header(char *filename, int *w, int *h, int *bpp, ubyte *palette=NULL );
int targa_read_bitmap(char *filename, ubyte *data, ubyte *palette, int dest_size );
int targa_read_bitmap_32(char *filename, ubyte *data, ubyte *palette, int dest_size );
int targa_write_bitmap(char *filename, ubyte *data, ubyte *palette, int w, int h, int bpp);

// The following are used by the tools\vani code.
int targa_compress(char *out, char *in, int outsize, int pixsize, int bytecount);
int targa_uncompress( ubyte *dst, ubyte *src, int bitmap_width, int bytes_per_pixel );

#endif // __TARGA_H
