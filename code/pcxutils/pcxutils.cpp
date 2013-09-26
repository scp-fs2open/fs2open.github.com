/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
int pcx_read_header(const char *real_filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *pal )
{
	PCXHeader header;
	CFILE * PCXfile;
	char filename[MAX_FILENAME_LEN];
	int i, j;

	if (img_cfp == NULL) {
		strcpy_s( filename, real_filename );
		char *p = strchr( filename, '.' );
		if ( p ) *p = 0;
		strcat_s( filename, ".pcx" );

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

	header.Xmin = INTEL_SHORT( header.Xmin ); //-V570
	header.Ymin = INTEL_SHORT( header.Ymin ); //-V570
	header.Xmax = INTEL_SHORT( header.Xmax ); //-V570
	header.Ymax = INTEL_SHORT( header.Ymax ); //-V570
	header.Hdpi = INTEL_SHORT( header.Hdpi ); //-V570
	header.Vdpi = INTEL_SHORT( header.Vdpi ); //-V570

	for (i=0; i<16; i++ ){
		for (j=0; j<3; j++){
			header.ColorMap[i][j] = INTEL_INT( header.ColorMap[i][j] ); //-V570
		}
	}

	header.BytesPerLine = INTEL_SHORT( header.BytesPerLine ); //-V570

	for (i=0; i<60; i++ ){
		header.filler[i] = INTEL_INT( header.filler[i] ); //-V570
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
		
	strcpy_s( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat_s( filename, ".pcx" );

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
int pcx_read_bitmap( const char * real_filename, ubyte *org_data, ubyte *pal, int byte_size, int aabitmap, int nondark, int cf_type )
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
	
	strcpy_s( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat_s( filename, ".pcx" );

	
	PCXfile = cfopen( filename , "rb", CFILE_NORMAL, cf_type );
	if ( !PCXfile ){
	
		return PCX_ERROR_OPENING;
	}

	// read 128 char PCX header
	if (cfread( &header, sizeof(PCXHeader), 1, PCXfile )!=1)	{
		cfclose( PCXfile );
	
		return PCX_ERROR_NO_HEADER;
	}

	header.Xmin = INTEL_SHORT( header.Xmin ); //-V570
	header.Ymin = INTEL_SHORT( header.Ymin ); //-V570
	header.Xmax = INTEL_SHORT( header.Xmax ); //-V570
	header.Ymax = INTEL_SHORT( header.Ymax ); //-V570
	header.Hdpi = INTEL_SHORT( header.Hdpi ); //-V570
	header.Vdpi = INTEL_SHORT( header.Vdpi ); //-V570
	header.BytesPerLine = INTEL_SHORT( header.BytesPerLine ); //-V570

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


int pcx_write_bitmap( const char * real_filename, int w, int h, ubyte ** row_ptrs, ubyte * palette )
{
	int retval;
	int i;
	ubyte data;
	PCXHeader header;
	FILE * PCXfile;
	char filename[MAX_FILENAME_LEN];
		
	strcpy_s( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat_s( filename, ".pcx" );

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
