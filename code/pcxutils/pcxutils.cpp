/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/PcxUtils/pcxutils.cpp $
 * $Revision: 2.11 $
 * $Date: 2006-06-27 05:04:43 $
 * $Author: taylor $
 *
 * code to deal with pcx files
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.10  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 2.9  2005/09/06 02:40:24  taylor
 * fix for -pcx32 on big-endian
 *
 * Revision 2.8  2005/02/04 10:12:32  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.7  2004/10/31 22:00:57  taylor
 * new bmpman merge support, add PreProcDefines.h a few new places
 *
 * Revision 2.6  2004/07/26 20:47:48  Kazan
 * remove MCD complete
 *
 * Revision 2.5  2004/07/12 16:33:02  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.4  2004/03/05 09:02:09  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2003/11/19 20:37:24  randomtiger
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
 * Revision 2.2  2003/03/18 10:07:05  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.2  2002/10/02 11:40:19  randomtiger
 * Bmpmap has been reverted to an old non d3d8 version.
 * All d3d8 code is now in the proper place.
 * PCX code is now working to an extent. Problems with alpha though.
 * Ani's work slowly with alpha problems.
 * Also I have done a bit of tidying - RT
 *
 * Revision 2.1.2.1  2002/09/24 18:56:44  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
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
 * 11    8/10/99 6:54p Dave
 * Mad optimizations. Added paging to the nebula effect.
 * 
 * 10    7/13/99 1:16p Dave
 * 32 bit support. Whee!
 * 
 * 9     2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 8     2/03/99 6:06p Dave
 * Groundwork for FS2 PXO usertracker support.  Gametracker support next.
 * 
 * 7     2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 6     12/01/98 5:54p Dave
 * Simplified the way pixel data is swizzled. Fixed tga bitmaps to work
 * properly in D3D and Glide.
 * 
 * 5     12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 * 
 * 4     12/01/98 8:06a Dave
 * Temporary checkin to fix some texture transparency problems in d3d.
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 16    1/19/98 11:37p Lawrance
 * Fixing Optimization build warnings
 * 
 * 15    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 14    9/03/97 4:32p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 13    7/20/97 6:59p Lawrance
 * fixed bug that was writing out PCX files with an extra bogus line
 * 
 * 12    6/05/97 6:07p John
 * fixed warning
 * 
 * 11    6/05/97 3:59p John
 * Fixed a bug in PCX reader
 * 
 * 10    2/25/97 12:06p John
 * fixed a pcx potential bug.
 * 
 * 9     2/25/97 12:03p John
 * fixed a pcx reading bug.
 * 
 * 8     2/20/97 4:18p John
 * fixed bug reading odd-width pcx files
 * 
 * 7     11/26/96 9:28a Allender
 * get palette info when getting pcx info
 * 
 * 6     11/18/96 12:36p John
 * Added code to dump screen to a PCX file.
 * 
 * 5     11/13/96 4:51p Allender
 * started overhaul of bitmap manager.  bm_load no longer actually load
 * the data, only the info for the bitmap.  Locking the bitmap now forces
 * load when no data present (or will if bpp changes)
 *
 * $NoKeywords: $
 */

#include "cfile/cfile.h"
#include "pcxutils/pcxutils.h"
#include "palman/palman.h"
#include "bmpman/bmpman.h"




/* PCX Header data type */
typedef struct	{
	ubyte		Manufacturer;
	ubyte		Version;
	ubyte		Encoding;
	ubyte		BitsPerPixel;
	short		Xmin;
	short		Ymin;
	short		Xmax;
	short		Ymax;
	short		Hdpi;
	short		Vdpi;
	ubyte		ColorMap[16][3];
	ubyte		Reserved;
	ubyte		Nplanes;
	short		BytesPerLine;
	ubyte		filler[60];
} PCXHeader;

// reads header information from the PCX file into the bitmap pointer
int pcx_read_header(char *real_filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *pal )
{
	PCXHeader header;
	CFILE * PCXfile;
	char filename[MAX_FILENAME_LEN];
	int i, j;

	if (img_cfp == NULL) {
		strcpy( filename, real_filename );
		char *p = strchr( filename, '.' );
		if ( p ) *p = 0;
		strcat( filename, ".pcx" );

		PCXfile = cfopen( filename , "rb" );
		if ( !PCXfile )
			return PCX_ERROR_OPENING;
	} else {
		PCXfile = img_cfp;
	}

	// read 128 char PCX header
	if (cfread( &header, sizeof(PCXHeader), 1, PCXfile )!=1)	{
		if (img_cfp == NULL)
			cfclose( PCXfile );
		return PCX_ERROR_NO_HEADER;
	}

	header.Xmin = INTEL_SHORT( header.Xmin );
	header.Ymin = INTEL_SHORT( header.Ymin );
	header.Xmax = INTEL_SHORT( header.Xmax );
	header.Ymax = INTEL_SHORT( header.Ymax );
	header.Hdpi = INTEL_SHORT( header.Hdpi );
	header.Vdpi = INTEL_SHORT( header.Vdpi );

	for (i=0; i<16; i++ ){
		for (j=0; j<3; j++){
			header.ColorMap[i][j] = INTEL_INT( header.ColorMap[i][j] );
		}
	}

	header.BytesPerLine = INTEL_SHORT( header.BytesPerLine );

	for (i=0; i<60; i++ ){
		header.filler[i] = INTEL_INT( header.filler[i] );
	}

	// Is it a 256 color PCX file?
	if ((header.Manufacturer != 10)||(header.Encoding != 1)||(header.Nplanes != 1)||(header.BitsPerPixel != 8)||(header.Version != 5))	{
		if (img_cfp == NULL)
			cfclose( PCXfile );
		return PCX_ERROR_WRONG_VERSION;
	}

	if (w) *w = header.Xmax - header.Xmin + 1;
	if (h) *h = header.Ymax - header.Ymin + 1;
	if (bpp) *bpp = header.BitsPerPixel;
	
	if ( pal ) {
		cfseek( PCXfile, -768, CF_SEEK_END );
		cfread( pal, 3, 256, PCXfile );
	}

	if (img_cfp == NULL)
		cfclose(PCXfile);

	return PCX_ERROR_NONE;
}

// static ubyte Pcx_load[1024*768 + 768 + sizeof(PCXHeader)];
// int Pcx_load_offset = 0;
// int Pcx_load_size = 0;
/*
// #define GET_BUF()			do { buffer = &Pcx_load[Pcx_load_offset]; if(Pcx_load_offset + buffer_size > Pcx_load_size) { buffer_size = Pcx_load_size - Pcx_load_offset; } } while(0);
int pcx_read_bitmap_8bpp( char * real_filename, ubyte *org_data, ubyte *palette )
{
	PCXHeader header;
	CFILE * PCXfile;
	int row, col, count, xsize, ysize;
	ubyte data=0;
	int buffer_size, buffer_pos;
	ubyte buffer[1024];
	ubyte *pixdata;
	char filename[MAX_FILENAME_LEN];
		
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".pcx" );

	PCXfile = cfopen( filename , "rb" );
	if ( !PCXfile )
		return PCX_ERROR_OPENING;

	// read 128 char PCX header
	if (cfread( &header, sizeof(PCXHeader), 1, PCXfile )!=1)	{
		cfclose( PCXfile );
		return PCX_ERROR_NO_HEADER;
	}

	header.Xmin = INTEL_SHORT( header.Xmin );
	header.Ymin = INTEL_SHORT( header.Ymin );
	header.Xmax = INTEL_SHORT( header.Xmax );
	header.Ymax = INTEL_SHORT( header.Ymax );
	header.Hdpi = INTEL_SHORT( header.Hdpi );
	header.Vdpi = INTEL_SHORT( header.Vdpi );
	header.BytesPerLine = INTEL_SHORT( header.BytesPerLine );

	// Is it a 256 color PCX file?
	if ((header.Manufacturer != 10)||(header.Encoding != 1)||(header.Nplanes != 1)||(header.BitsPerPixel != 8)||(header.Version != 5))	{
		cfclose( PCXfile );
		return PCX_ERROR_WRONG_VERSION;
	}

	// Find the size of the image
	xsize = header.Xmax - header.Xmin + 1;
	ysize = header.Ymax - header.Ymin + 1;

	// Read the extended palette at the end of PCX file
	// Read in a character which should be 12 to be extended palette file

	cfseek( PCXfile, -768, CF_SEEK_END );
	cfread( palette, 3, 256, PCXfile );

	for ( int i=0; i<256; i++ ){				//tigital
		palette[i] = INTEL_INT( palette[i] );
	}

	cfseek( PCXfile, sizeof(PCXHeader), CF_SEEK_SET );
	
	buffer_size = 1024;
	buffer_pos = 0;
	
	buffer_size = cfread( buffer, 1, buffer_size, PCXfile );

	count = 0;

	for (row=0; row<ysize;row++)      {
		pixdata = org_data;
		for (col=0; col<header.BytesPerLine;col++)     {
			if ( count == 0 )	{
				data = buffer[buffer_pos++];
				if ( buffer_pos == buffer_size )	{
					buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
					Assert( buffer_size > 0 );
					buffer_pos = 0;
				}
				if ((data & 0xC0) == 0xC0)     {
					count = data & 0x3F;
					data = buffer[buffer_pos++];
					if ( buffer_pos == buffer_size )	{
						buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
						Assert( buffer_size > 0 );
						buffer_pos = 0;
					}
				} else {
					count = 1;
				}
			}
			if ( col < xsize )
				*pixdata++ = data;
			count--;
		}
		org_data += xsize;
	}
	cfclose(PCXfile);

	return PCX_ERROR_NONE;
}
*/

#if BYTE_ORDER == BIG_ENDIAN
typedef struct { ubyte a, r, g, b; } COLOR32;
#else
typedef struct { ubyte b, g, r, a; } COLOR32;
#endif

//int pcx_read_bitmap_16bpp( char * real_filename, ubyte *org_data, ubyte bpp, int aabitmap, int nondark )
int pcx_read_bitmap( char * real_filename, ubyte *org_data, ubyte *pal, int byte_size, int aabitmap, int nondark )
{
	PCXHeader header;
	CFILE * PCXfile;
	int row, col, count, xsize, ysize;
	ubyte data=0;
	int buffer_size, buffer_pos;
	ubyte buffer[1024];
	ubyte *pixdata;
	char filename[MAX_FILENAME_LEN];
	ubyte palette[768];	
	ushort bit_16;
	COLOR32 bit_32;
	ubyte r, g, b, al;
	
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".pcx" );

	
	PCXfile = cfopen( filename , "rb" );
	if ( !PCXfile ){
	
		return PCX_ERROR_OPENING;
	}

	// read 128 char PCX header
	if (cfread( &header, sizeof(PCXHeader), 1, PCXfile )!=1)	{
		cfclose( PCXfile );
	
		return PCX_ERROR_NO_HEADER;
	}

	header.Xmin = INTEL_SHORT( header.Xmin );
	header.Ymin = INTEL_SHORT( header.Ymin );
	header.Xmax = INTEL_SHORT( header.Xmax );
	header.Ymax = INTEL_SHORT( header.Ymax );
	header.Hdpi = INTEL_SHORT( header.Hdpi );
	header.Vdpi = INTEL_SHORT( header.Vdpi );
	header.BytesPerLine = INTEL_SHORT( header.BytesPerLine );

	// Is it a 256 color PCX file?
	if ((header.Manufacturer != 10)||(header.Encoding != 1)||(header.Nplanes != 1)||(header.BitsPerPixel != 8)||(header.Version != 5))	{
		cfclose( PCXfile );
	
		return PCX_ERROR_WRONG_VERSION;
	}

	
	// Find the size of the image
	xsize = header.Xmax - header.Xmin + 1;
	ysize = header.Ymax - header.Ymin + 1;

	// Read the extended palette at the end of PCX file
	// Read in a character which should be 12 to be extended palette file

	cfseek( PCXfile, -768, CF_SEEK_END );
	cfread( palette, 1, (3 * 256), PCXfile );
	cfseek( PCXfile, sizeof(PCXHeader), CF_SEEK_SET );
	
	buffer_size = 1024;
	buffer_pos = 0;
	
//	Assert( buffer_size == 1024 );	// AL: removed to avoid optimized warning 'unreachable code'
	
	buffer_size = cfread( buffer, 1, buffer_size, PCXfile );

	count = 0;

	for (row=0; row<ysize;row++)      {
	
		pixdata = org_data;
		for (col=0; col<header.BytesPerLine;col++)     {
			if ( count == 0 )	{
				data = buffer[buffer_pos++];
				if ( buffer_pos == buffer_size )	{
					buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
					Assert( buffer_size > 0 );
					buffer_pos = 0;
				}
				if ((data & 0xC0) == 0xC0)     {
					count = data & 0x3F;
					data = buffer[buffer_pos++];
					if ( buffer_pos == buffer_size )	{
						buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
						Assert( buffer_size > 0 );
						buffer_pos = 0;
					}
				} else {
					count = 1;
				}
			}
			// stuff the pixel
			if ( col < xsize ) {
				// 8-bit PCX reads
				if ( byte_size == 1 ) {
					*pixdata++ = data;
				} else {
					// 16-bit AABITMAP reads
					if ( (byte_size == 2) && aabitmap ) {
						// stuff the pixel
						// memcpy(pixdata, &data, 2);
						*((ushort*)pixdata) = (ushort)data;
					} else {
						// stuff the 24 bit value				
						r = palette[data*3];
						g = palette[data*3 + 1];
						b = palette[data*3 + 2];

						// clear the pixel
						bit_16 = 0;
						memset(&bit_32, 0, sizeof(COLOR32));

						// 16-bit non-darkening reads
						if ( (byte_size == 2) && nondark ) {
							al = 0;
							if (palman_is_nondarkening(r, g, b)) {
								al = 255;
							}
						} else {
							// if the color matches the transparent color, make it so
							al = 255;

							if ( (0 == (int)palette[data*3]) && (255 == (int)palette[data*3+1]) && (0 == (int)palette[data*3+2]) ) {
								r = b = 0;
								g = (byte_size == 4) ? 0 : 255;
								al = 0;					
							}
						}

						// normal 16-bit reads
						if ( byte_size == 2 ) {
							// stuff the color
							bm_set_components((ubyte*)&bit_16, &r, &g, &b, &al);				

							// stuff the pixel
							*((ushort*)pixdata) = bit_16;
						}
						// normal 32-bit reads
						else if ( byte_size == 4 ) {
							if ( /*(r == 0) && (b == 0) && (g == 255) && (al == 0)*/ 0 ) {
								memset(&bit_32, 0, sizeof(COLOR32));
							} else {
								bit_32.r = r;
								bit_32.g = g;
								bit_32.b = b;
								bit_32.a = al;
							}

							// stuff the pixel
							*((COLOR32*)pixdata) = bit_32;
						}
					}

					pixdata += byte_size;
				}
			}

			count--;
		}

		org_data += (xsize * byte_size);
	}
	
	cfclose(PCXfile);
	
	return PCX_ERROR_NONE;
}
/*
int pcx_read_bitmap_16bpp_aabitmap( char * real_filename, ubyte *org_data )
{
	PCXHeader header;
	CFILE * PCXfile;
	int row, col, count, xsize, ysize;
	ubyte data=0;
	int buffer_size, buffer_pos;
	ubyte buffer[1024];
	ubyte *pixdata;
	char filename[MAX_FILENAME_LEN];
	ubyte palette[768];		
		
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;

	strcat( filename, ".pcx" );

	PCXfile = cfopen( filename , "rb" );
	if ( !PCXfile ){
		return PCX_ERROR_OPENING;
	}

	// read 128 char PCX header
	if (cfread( &header, sizeof(PCXHeader), 1, PCXfile )!=1)	{
		cfclose( PCXfile );
		return PCX_ERROR_NO_HEADER;
	}

	header.Xmin = INTEL_SHORT( header.Xmin );
	header.Ymin = INTEL_SHORT( header.Ymin );
	header.Xmax = INTEL_SHORT( header.Xmax );
	header.Ymax = INTEL_SHORT( header.Ymax );
	header.Hdpi = INTEL_SHORT( header.Hdpi );
	header.Vdpi = INTEL_SHORT( header.Vdpi );
	header.BytesPerLine = INTEL_SHORT( header.BytesPerLine );

	// Is it a 256 color PCX file?
	if ((header.Manufacturer != 10)||(header.Encoding != 1)||(header.Nplanes != 1)||(header.BitsPerPixel != 8)||(header.Version != 5))	{
		cfclose( PCXfile );
		return PCX_ERROR_WRONG_VERSION;
	}

	// Find the size of the image
	xsize = header.Xmax - header.Xmin + 1;
	ysize = header.Ymax - header.Ymin + 1;

	// Read the extended palette at the end of PCX file
	// Read in a character which should be 12 to be extended palette file

	cfseek( PCXfile, -768, CF_SEEK_END );
	cfread( palette, 3, 256, PCXfile );
	cfseek( PCXfile, sizeof(PCXHeader), CF_SEEK_SET );
	
	buffer_size = 1024;
	buffer_pos = 0;
	
//	Assert( buffer_size == 1024 );	// AL: removed to avoid optimized warning 'unreachable code'
	buffer_size = cfread( buffer, 1, buffer_size, PCXfile );

	count = 0;

	for (row=0; row<ysize;row++)      {
		pixdata = org_data;
		for (col=0; col<header.BytesPerLine;col++)     {
			if ( count == 0 )	{
				data = buffer[buffer_pos++];
				if ( buffer_pos == buffer_size )	{
					buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
					Assert( buffer_size > 0 );
					buffer_pos = 0;
				}
				if ((data & 0xC0) == 0xC0)     {
					count = data & 0x3F;
					data = buffer[buffer_pos++];
					if ( buffer_pos == buffer_size )	{
						buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
						Assert( buffer_size > 0 );
						buffer_pos = 0;
					}
				} else {
					count = 1;
				}
			}
			// stuff the pixel
			if ( col < xsize ){				
				// stuff the pixel
				// memcpy(pixdata, &data, 2);
				*((ushort*)pixdata) = (ushort)data;

				pixdata += 2;
			}
			count--;
		}

		org_data += (xsize * 2);
	}
	cfclose(PCXfile);

	return PCX_ERROR_NONE;
}

int pcx_read_bitmap_16bpp_nondark( char * real_filename, ubyte *org_data )
{
	PCXHeader header;
	CFILE * PCXfile;
	int row, col, count, xsize, ysize;
	ubyte data=0;
	int buffer_size, buffer_pos;
	ubyte buffer[1024];
	ubyte *pixdata;
	char filename[MAX_FILENAME_LEN];
	ubyte palette[768];	
	ushort bit_16;	
	ubyte r, g, b, al;
		
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".pcx" );

	PCXfile = cfopen( filename , "rb" );
	if ( !PCXfile ){
		return PCX_ERROR_OPENING;
	}

	// read 128 char PCX header
	if (cfread( &header, sizeof(PCXHeader), 1, PCXfile )!=1)	{
		cfclose( PCXfile );
		return PCX_ERROR_NO_HEADER;
	}

	header.Xmin = INTEL_SHORT( header.Xmin );
	header.Ymin = INTEL_SHORT( header.Ymin );
	header.Xmax = INTEL_SHORT( header.Xmax );
	header.Ymax = INTEL_SHORT( header.Ymax );
	header.Hdpi = INTEL_SHORT( header.Hdpi );
	header.Vdpi = INTEL_SHORT( header.Vdpi );
	header.BytesPerLine = INTEL_SHORT( header.BytesPerLine );

	// Is it a 256 color PCX file?
	if ((header.Manufacturer != 10)||(header.Encoding != 1)||(header.Nplanes != 1)||(header.BitsPerPixel != 8)||(header.Version != 5))	{
		cfclose( PCXfile );
		return PCX_ERROR_WRONG_VERSION;
	}

	// Find the size of the image
	xsize = header.Xmax - header.Xmin + 1;
	ysize = header.Ymax - header.Ymin + 1;

	// Read the extended palette at the end of PCX file
	// Read in a character which should be 12 to be extended palette file

	cfseek( PCXfile, -768, CF_SEEK_END );
	cfread( palette, 3, 256, PCXfile );
	cfseek( PCXfile, sizeof(PCXHeader), CF_SEEK_SET );
	
	buffer_size = 1024;
	buffer_pos = 0;
	
//	Assert( buffer_size == 1024 );	// AL: removed to avoid optimized warning 'unreachable code'
	buffer_size = cfread( buffer, 1, buffer_size, PCXfile );

	count = 0;

	for (row=0; row<ysize;row++)      {
		pixdata = org_data;
		for (col=0; col<header.BytesPerLine;col++)     {
			if ( count == 0 )	{
				data = buffer[buffer_pos++];
				if ( buffer_pos == buffer_size )	{
					buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
					Assert( buffer_size > 0 );
					buffer_pos = 0;
				}
				if ((data & 0xC0) == 0xC0)     {
					count = data & 0x3F;
					data = buffer[buffer_pos++];
					if ( buffer_pos == buffer_size )	{
						buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
						Assert( buffer_size > 0 );
						buffer_pos = 0;
					}
				} else {
					count = 1;
				}
			}
			// stuff the pixel
			if ( col < xsize ){				
				// stuff the 24 bit value				
				r = palette[data*3];
				g = palette[data*3 + 1];
				b = palette[data*3 + 2];							

				// if this is a nondarkening texture				
				// if this color matches a nondarkening pixel color, set the alpha to high
				al = 0;
				if(palman_is_nondarkening(r, g, b)){
					al = 255;
				}

				// set the pixel
				bit_16 = 0;

				bm_set_components((ubyte*)&bit_16, &r, &g, &b, &al);					
				
				// stuff the pixel
				*((ushort*)pixdata) = bit_16;				
				pixdata += 2;
			}
			count--;
		}

		org_data += (xsize * 2);
	}
	cfclose(PCXfile);

	return PCX_ERROR_NONE;
}


int pcx_read_bitmap_32(char *real_filename, ubyte *org_data )
{
	PCXHeader header;
	CFILE * PCXfile;
	ubyte data=0;
	int row, col, count;
	int buffer_pos;
	char filename[MAX_FILENAME_LEN];
	ubyte palette[768];	
	
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".pcx" );

	
	PCXfile = cfopen( filename , "rb" );
	if ( !PCXfile ){
	
		return PCX_ERROR_OPENING;
	}

	// read 128 char PCX header
	if (cfread( &header, sizeof(PCXHeader), 1, PCXfile )!=1)	{
		cfclose( PCXfile );
	
		return PCX_ERROR_OPENING;
	}

	header.Xmin = INTEL_SHORT( header.Xmin );
	header.Ymin = INTEL_SHORT( header.Ymin );
	header.Xmax = INTEL_SHORT( header.Xmax );
	header.Ymax = INTEL_SHORT( header.Ymax );
	header.Hdpi = INTEL_SHORT( header.Hdpi );
	header.Vdpi = INTEL_SHORT( header.Vdpi );
	header.BytesPerLine = INTEL_SHORT( header.BytesPerLine );

	// Is it a 256 color PCX file?
	if ((header.Manufacturer != 10)||(header.Encoding != 1)||(header.Nplanes != 1)||(header.BitsPerPixel != 8)||(header.Version != 5))	{
		cfclose( PCXfile );
		return PCX_ERROR_OPENING;
	}

	// Find the size of the image
	int src_xsize = header.Xmax - header.Xmin + 1;
	int src_ysize = header.Ymax - header.Ymin + 1;

	// Read the extended palette at the end of PCX file
	// Read in a character which should be 12 to be extended palette file
	cfseek( PCXfile, -768, CF_SEEK_END );
	cfread( palette, 3, 256, PCXfile );
	cfseek( PCXfile, sizeof(PCXHeader), CF_SEEK_SET );
	
	int buffer_size = 1024;
	ubyte buffer[1024];
	buffer_pos = 0;
	
	buffer_size = cfread( buffer, 1, buffer_size, PCXfile );

	count = 0; 

#if BYTE_ORDER == BIG_ENDIAN
	typedef struct { ubyte a, r, g, b; } COLOR32;
#else
	typedef struct { ubyte b, g, r, a; } COLOR32;
#endif

	for (row=0; row < src_ysize;row++)      {
	
		COLOR32 *pixdata = (COLOR32 *) org_data;

		for (col=0; col < header.BytesPerLine;col++)     {
			if ( count == 0 )	{
				data = buffer[buffer_pos++];
				if ( buffer_pos == buffer_size )	{
					buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
					Assert( buffer_size > 0 );
					buffer_pos = 0;
				}
				if ((data & 0xC0) == 0xC0)     {
					count = data & 0x3F;
					data = buffer[buffer_pos++];
					if ( buffer_pos == buffer_size )	{
						buffer_size = cfread( buffer, 1, buffer_size, PCXfile );
						Assert( buffer_size > 0 );
						buffer_pos = 0;
					}
				} else {
					count = 1;
				}
			}
			// stuff the pixel
			if ( col < src_xsize ){	  
				
				if((0 == (int)palette[data*3]) && (255 == (int)palette[data*3+1]) && (0 == (int)palette[data*3+2])){
					pixdata->r = pixdata->b = pixdata->g = pixdata->a = 0;					
				} else {

					// stuff the 24 bit value				
					pixdata->r = palette[data*3];
					pixdata->g = palette[data*3 + 1];
					pixdata->b = palette[data*3 + 2];
	
					pixdata->a = 255;
				}

				pixdata++;
			}
			count--;
		}

		org_data += (src_xsize * 4);
	}
	
	cfclose(PCXfile);

	return PCX_ERROR_NONE;
}
*/

// subroutine for writing an encoded byte pair
// returns count of bytes written, 0 if error
int pcx_encode_byte(ubyte byt, ubyte cnt, FILE * fid)
{
	if (cnt) {
		if ( (cnt==1) && (0xc0 != (0xc0 & byt)) )	{
			if(EOF == putc((int)byt, fid))
				return 0; 	// disk write error (probably full)
			return 1;
		} else {
			if(EOF == putc((int)0xC0 | cnt, fid))
				return 0; 	// disk write error
			if(EOF == putc((int)byt, fid))
				return 0; 	// disk write error
			return 2;
		}
	}
	return 0;
}

// returns number of bytes written into outBuff, 0 if failed
int pcx_encode_line(ubyte *inBuff, int inLen, FILE * fp)
{
	ubyte this_ptr, last;

	int srcIndex, i;
	register int total;
	register ubyte runCount; 	// max single runlength is 63
	total = 0;
	last = *(inBuff);
	runCount = 1;

	for (srcIndex = 1; srcIndex < inLen; srcIndex++) {
		this_ptr = *(++inBuff);
 		if (this_ptr == last)	{
			runCount++;			// it encodes
			if (runCount == 63)	{
				i = pcx_encode_byte(last, runCount, fp);
				if(!i){
					return(0);
				}
				total += i;
				runCount = 0;
			}
		} else {   	// this_ptr != last
			if (runCount)	{
				i = pcx_encode_byte(last, runCount, fp);
				if (!i){
					return(0);
				}
				total += i;
			}
			last = this_ptr;
			runCount = 1;
		}
	}

	if (runCount)	{		// finish up
		i = pcx_encode_byte(last, runCount, fp);
		if (!i){
			return 0;
		}
		return total + i;
	}
	return total;
}


int pcx_write_bitmap( char * real_filename, int w, int h, ubyte ** row_ptrs, ubyte * palette )
{
	int retval;
	int i;
	ubyte data;
	PCXHeader header;
	FILE * PCXfile;
	char filename[MAX_FILENAME_LEN];
		
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".pcx" );

	memset( &header, 0, sizeof( PCXHeader ) );

	header.Manufacturer = 10;
	header.Encoding = 1;
	header.Nplanes = 1;
	header.BitsPerPixel = 8;
	header.Version = 5;
	header.Xmax = (short)(w-1);
	header.Ymax = (short)(h-1);
	header.Ymin = 0;
	header.Xmin = 0;
	header.BytesPerLine =(short)(w);

	PCXfile = fopen( filename , "wb" );
	if ( !PCXfile )
		return PCX_ERROR_OPENING;

	if ( fwrite( &header, sizeof( PCXHeader ), 1, PCXfile ) != 1 )	{
		fclose( PCXfile );
		return PCX_ERROR_WRITING;
	}

	for (i=0; i<h; i++ )	{
		if (!pcx_encode_line( row_ptrs[i], w, PCXfile ))	{
			fclose( PCXfile );
			return PCX_ERROR_WRITING;
		}
	}

	// Mark an extended palette
	data = 12;
	if (fwrite( &data, 1, 1, PCXfile )!=1)	{
		fclose( PCXfile );
		return PCX_ERROR_WRITING;
	}

	// Write the extended palette
//	for (i=0; i<768; i++ )
//		palette[i] <<= 2;

	retval = fwrite( palette, 768, 1, PCXfile );

//	for (i=0; i<768; i++ )
//		palette[i] >>= 2;

	if (retval !=1)	{
		fclose( PCXfile );
		return PCX_ERROR_WRITING;
	}

	fclose( PCXfile );
	return PCX_ERROR_NONE;

}

//text for error messges
char pcx_error_messages[] = {
	"No error.\0"
	"Error opening file.\0"
	"Couldn't read PCX header.\0"
	"Unsupported PCX version.\0"
	"Error reading data.\0"
	"Couldn't find palette information.\0"
	"Error writing data.\0"
};


//function to return pointer to error message
char *pcx_errormsg(int error_number)
{
	char *p = pcx_error_messages;

	while (error_number--) {

		if (!p) return NULL;

		p += strlen(p)+1;

	}

	return p;

}
