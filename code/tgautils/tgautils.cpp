/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/TgaUtils/TgaUtils.cpp $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:02 $
 * $Author: penguin $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     7/13/99 1:16p Dave
 * 32 bit support. Whee!
 * 
 * 6     3/20/99 3:46p Dave
 * Added support for model-based background nebulae. Added 3 new
 * sexpressions.
 * 
 * 5     12/03/98 9:39a Dave
 * Removed bogus tga code assert?
 * 
 * 4     12/02/98 5:47p Dave
 * Put in interface xstr code. Converted barracks screen to new format.
 * 
 * 3     12/01/98 5:54p Dave
 * Simplified the way pixel data is swizzled. Fixed tga bitmaps to work
 * properly in D3D and Glide.
 * 
 * 2     12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 *  
 * $NoKeywords: $
 */

#include <stdio.h>
#include <string.h>

#include "tgautils.h"
#include "cfile.h"
#include "bmpman.h"
#include "palman.h"

// -----------------
//
// Defines
//
// -----------------

#define MAX_TARGA_RUN_LENGTH_PACKET 128
#define TARGA_HEADER_LENGTH 18
#define ULORIGIN		(header.image_descriptor & 0x20)

// -----------------
//
// Structures
//
// -----------------

typedef struct targa_header {
	ubyte id_length;
	ubyte color_map_type;
	ubyte image_type;
	short cmap_start;
	short cmap_length;
	ubyte cmap_depth;
	short xoffset;
	short yoffset;
	short width;
	short height;
	ubyte pixel_depth;
	ubyte image_descriptor;
} targa_header;

// -----------------
//
// Internal Functions
//
// -----------------


// copy from one pixel buffer to another
//
// to - pointer to dest. buffet
// from - pointer to source buffer
// pixels - number of pixels to copy
// fromsize - source pixel size
// tosize - dest. pixel size
//
// returns - number of pixels copied to destination
//
static int targa_copy_data(char *to, char *from, int pixels, int fromsize, int tosize)
{
	if ( (fromsize == 2) && (tosize==2) )	{
		// Flip the alpha bit on 1555 format
		ushort *src, *dst;

		src = (ushort *)from;
		dst = (ushort *)from;
		for (int i=0; i<pixels; i++ )	{
			*dst++ = (ushort)((*src++) ^ 0x8000);		// Flip the transparency bit
		}
		return tosize*pixels;
	} else if ( (fromsize == 2) && (tosize == 3) )	{
		ushort *src;

		src = (ushort *)from;
		for (int i=0; i<pixels; i++ )	{			
			ushort pixel = *src++;

			*to++ = (ubyte)((pixel & 0x1f) * 8);
			*to++ = (ubyte)(((pixel >> 5) & 63) * 4);
			*to++ = (ubyte)(((pixel >> 11) & 0x1f) * 8);
		}
		return tosize*pixels;
	} else {
		Assert(fromsize == tosize);
		memcpy(to, from, pixels * fromsize);
		return tosize*pixels;
	}
}

//	targa_pixels_equal -- Test if two pixels are identical
//
// pix1 - first pixel data
// pix2 - second pixel data
// pixbytes - number of bytes per pixel
//
// returns - 0 if No Match, else 1 if Match
static int targa_pixels_equal(char *pix1, char *pix2, int pixbytes)
{
	do	{
		if ( *pix1++ != *pix2++ ) {
			return 0;
		}
	} while ( --pixbytes > 0 );

	return 1;
}

//	Perform targa RLE on the input data
//
// out - Buffer to write it out to
// in - Buffer to compress
// outsize - Number of bytes in output buffer
// pixsize - Number of bytes in input pixel
// bytecount - Number of bytes input
//
// returns -  size of compressed data
//
int targa_compress(char *out, char *in, int outsize, int pixsize, int bytecount)
{
	int pixcount;		// number of pixels in the current packet
	char *inputpixel=NULL;	// current input pixel position
	char *matchpixel=NULL;	// pixel value to match for a run
	char *flagbyte=NULL;		// location of last flag byte to set
	int rlcount;		// current count in r.l. string 
	int rlthresh;		// minimum valid run length
	char *copyloc;		// location to begin copying at

	// set the threshold -- the minimum valid run length

	if (outsize == 1) {
		rlthresh = 2;					// for 8bpp, require a 2 pixel span before rle'ing
	} else {
		rlthresh = 1;			
	}

	// set the first pixel up

	flagbyte = out;	// place to put next flag if run
	inputpixel = in;
	pixcount = 1;
	rlcount = 0;
	copyloc = (char *)0;

	// loop till data processing complete
	do	{

		// if we have accumulated a 128-byte packet, process it
		if ( pixcount == 129 )	{
			*flagbyte = 127;

			// set the run flag if this is a run

			if ( rlcount >= rlthresh )	{
					*flagbyte |= 0x80;
					pixcount = 2;
			}

			// copy the data into place
			++flagbyte;
			flagbyte += targa_copy_data(flagbyte, copyloc, pixcount-1, pixsize, outsize);
			pixcount = 1;

			// set up for next packet
			continue;
		}

		// if zeroth byte, handle as special case
		if ( pixcount == 1 )	{
			rlcount = 0;
			copyloc = inputpixel;		/* point to 1st guy in packet */
			matchpixel = inputpixel;	/* set pointer to pix to match */
			pixcount = 2;
			inputpixel += pixsize;
			continue;
		}

		// assembling a packet -- look at next pixel

		// current pixel == match pixel?
		if ( targa_pixels_equal(inputpixel, matchpixel, outsize) )	{

			//	establishing a run of enough length to
			//	save space by doing it
			//		-- write the non-run length packet
			//		-- start run-length packet

			if ( ++rlcount == rlthresh )	{
				
				//	close a non-run packet
				
				if ( pixcount > (rlcount+1) )	{
					// write out length and do not set run flag

					*flagbyte++ = (char)(pixcount - 2 - rlthresh);

					flagbyte += targa_copy_data(flagbyte, copyloc, (pixcount-1-rlcount), pixsize, outsize);

					copyloc = inputpixel;
					pixcount = rlcount + 1;
				}
			}
		} else {

			// no match -- either break a run or continue without one
			//	if a run exists break it:
			//		write the bytes in the string (outsize+1)
			//		start the next string

			if ( rlcount >= rlthresh )	{

				*flagbyte++ = (char)(0x80 | rlcount);
				flagbyte += targa_copy_data(flagbyte, copyloc, 1, pixsize, outsize);
				pixcount = 1;
				continue;
			} else {

				//	not a match and currently not a run
				//		- save the current pixel
				//		- reset the run-length flag
				rlcount = 0;
				matchpixel = inputpixel;
			}
		}
		pixcount++;
		inputpixel += pixsize;
	} while ( inputpixel < (in + bytecount));

	// quit this buffer without loosing any data

	if ( --pixcount >= 1 )	{
		*flagbyte = (char)(pixcount - 1);
		if ( rlcount >= rlthresh )	{
			*flagbyte |= 0x80;
			pixcount = 1;
		}

		// copy the data into place
		++flagbyte;
		flagbyte += targa_copy_data(flagbyte, copyloc, pixcount, pixsize, outsize);
	}
	return(flagbyte-out);
}

// Reads a pixel of the specified bytes_per_pixel into memory and
// returns the number of bytes read into memory.
// NOTE : for Freespace2, this also swizzles data into the proper screen (NOT texture) format - just like
//        the pcxutils do.
// 
// dst - A pointer to the destination.  Must be at least 4 bytes long.
// targa_file - The file to read from.
// bytes_per_pixel - The bytes per pixel of the file.
// dest_size - bytes per pixel in destination (1 or 2. 1 == 8 bit paletted)
//
// returns - Number of byte read into memory
//
static void targa_read_pixel( int num_pixels, ubyte **dst, ubyte **src, int bytes_per_pixel, int dest_size )
{
	int idx;
	ushort pixel;
	ubyte pal_index;
	ubyte r, g, b;
	ubyte al = 0;

	for(idx=0; idx<num_pixels; idx++){
		// stuff the 16 bit pixel
		memcpy(&pixel, *src, bytes_per_pixel);
						
		// if the pixel is transparent, make it so...	
		if(((pixel & 0x7c00) == 0) && ((pixel & 0x03e0) == 0x03e0) && ((pixel & 0x001f) == 0)){
			r = b = 0;
			g = 255;
			al = 0;
			bm_set_components((ubyte*)&pixel, &r, &g, &b, &al);
		} else {
			// get the 8 bit r, g, and b values
			r = (ubyte)(((pixel & 0x7c00) >> 10) * 8);
			g = (ubyte)(((pixel & 0x03e0) >> 5) * 8);
			b = (ubyte)((pixel & 0x001f) * 8);
			al = 1;

			// now stuff these back, swizzling properly
			pixel = 0;
			bm_set_components((ubyte*)&pixel, &r, &g, &b, &al);
		}

		// 16 bit destination
		if(dest_size == 2){
			// stuff the final pixel		
			memcpy( *dst, &pixel, bytes_per_pixel );			
		}
		// 8 bit destination 
		else {
			pal_index = (ubyte)palette_find((int)r, (int)g, (int)b);
			**dst = pal_index;			
		}

		// next pixel
		(*dst) += dest_size;
		(*src) += bytes_per_pixel;		
	}
}

// -----------------
//
// External Functions
//
// -----------------

// Reads header information from the targa file into the bitmap pointer
// 
// filename - name of the targa bitmap file
// w - (output) width of the bitmap
// h - (output) height of the bitmap
// bpp - (output) bits per pixel of the bitmap
//
// returns - TARGA_ERROR_NONE if successful, otherwise error code
//
int targa_read_header(char *real_filename, int *w, int *h, int *bpp, ubyte *palette )
{	
	targa_header header;
	CFILE *targa_file;
	char filename[MAX_FILENAME_LEN];
		
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".tga" );

	targa_file = cfopen( filename , "rb" );
	if ( !targa_file ){
		return TARGA_ERROR_READING;
	}	

	header.id_length = cfread_ubyte(targa_file);
	// header.id_length=targa_file.read_char();

	header.color_map_type = cfread_ubyte(targa_file);
	// header.color_map_type=targa_file.read_char();

	header.image_type = cfread_ubyte(targa_file);
	// header.image_type=targa_file.read_char();

	header.cmap_start = cfread_short(targa_file);
	// header.cmap_start=targa_file.read_short();

	header.cmap_length = cfread_short(targa_file);
	// header.cmap_length=targa_file.read_short();

	header.cmap_depth = cfread_ubyte(targa_file);
	// header.cmap_depth=targa_file.read_char();

	header.xoffset = cfread_short(targa_file);
	// header.xoffset=targa_file.read_short();

	header.yoffset = cfread_short(targa_file);
	// header.yoffset=targa_file.read_short();

	header.width = cfread_short(targa_file);
	// header.width=targa_file.read_short();

	header.height = cfread_short(targa_file);
	// header.height=targa_file.read_short();

	header.pixel_depth = cfread_ubyte(targa_file);
	// header.pixel_depth=targa_file.read_char();

	header.image_descriptor = cfread_ubyte(targa_file);
	// header.image_descriptor=targa_file.read_char();

	cfclose(targa_file);
	targa_file = NULL;

	*w = header.width;
	*h = header.height;
	*bpp = header.pixel_depth;

	// only support 16 bit pixels
	Assert(*bpp == 16);
	if(*bpp != 16){
		return TARGA_ERROR_READING;
	}
	
	return TARGA_ERROR_NONE;
}

// Uncompresses some RLE'd TGA data
//
// dst: pointer uncompressed destination.
// src: pointer to source rle'd data.
// bitmap_width: how many pixels to uncompress.
// bytes_per_pixel: bytes per pixel of the data.
//
// returns: number of input bytes processed.
//
int targa_uncompress( ubyte *dst, ubyte *src, int bitmap_width, int bytes_per_pixel, int dest_size )
{
	ubyte *pixdata = dst;
	ubyte *src_pixels = src;

	int pixel_count = 0;         // Initialize pixel counter 

	// Main decoding loop 
	while (pixel_count < bitmap_width ) {

		// Get the pixel count 
		int run_count = *src_pixels++;

		// Make sure writing this next run will not overflow the buffer 
		Assert(pixel_count + (run_count & 0x7f) + 1 <= bitmap_width );
		
		// If the run is encoded... 
		if ( run_count & 0x80 ) {
			run_count &= ~0x80;              // Mask off the upper bit       

			// Update total pixel count 
			pixel_count += (run_count + 1);

			ubyte pixel_value[4];	// temporary
			ubyte *tmp = pixel_value;
			targa_read_pixel( 1, &tmp, &src_pixels, bytes_per_pixel, dest_size );

			// Write remainder of pixel run to buffer 'run_count' times 
			do {
				memcpy( pixdata, pixel_value, dest_size );
				pixdata += dest_size;
			} while (run_count--);

		} else {   // ...the run is unencoded (raw) 
			// Update total pixel count 
			pixel_count += (run_count + 1);

			// Read run_count pixels 
			targa_read_pixel(run_count+1, &pixdata, &src_pixels, bytes_per_pixel, dest_size );
		}
	}

	Assert( pixel_count == bitmap_width );

	return src_pixels - src;
}


// Loads a Targa bitmap
// 
// filename - name of the targa file to load
// image_data - allocated storage for the bitmap
//
// returns - true if succesful, false otherwise
//
int targa_read_bitmap(char *real_filename, ubyte *image_data, ubyte *palette, int dest_size)
{
	Assert(real_filename);
	targa_header header;
	CFILE *targa_file;
	char filename[MAX_FILENAME_LEN];
	ubyte r, g, b;
		
	// open the file
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".tga" );

	targa_file = cfopen( filename , "rb" );
	if ( !targa_file ){
		return TARGA_ERROR_READING;
	}		

	header.id_length = cfread_ubyte(targa_file);
	// header.id_length=targa_file.read_char();

	header.color_map_type = cfread_ubyte(targa_file);
	// header.color_map_type=targa_file.read_char();

	header.image_type = cfread_ubyte(targa_file);
	// header.image_type=targa_file.read_char();

	header.cmap_start = cfread_short(targa_file);
	// header.cmap_start=targa_file.read_short();

	header.cmap_length = cfread_short(targa_file);
	// header.cmap_length=targa_file.read_short();

	header.cmap_depth = cfread_ubyte(targa_file);
	// header.cmap_depth=targa_file.read_char();

	header.xoffset = cfread_short(targa_file);
	// header.xoffset=targa_file.read_short();

	header.yoffset = cfread_short(targa_file);
	// header.yoffset=targa_file.read_short();

	header.width = cfread_short(targa_file);
	// header.width=targa_file.read_short();

	header.height = cfread_short(targa_file);
	// header.height=targa_file.read_short();

	header.pixel_depth = cfread_ubyte(targa_file);
	// header.pixel_depth=targa_file.read_char();

	header.image_descriptor = cfread_ubyte(targa_file);
	// header.image_descriptor=targa_file.read_char();	

	int bytes_per_pixel = (header.pixel_depth>>3);
	// we're only allowing 2 bytes per pixel (16 bit compressed)
	Assert(bytes_per_pixel == 2);
	if(bytes_per_pixel != 2){
		cfclose(targa_file);
		return TARGA_ERROR_READING;
	}

	int xo, yo;
	if ( header.image_descriptor & 0x10 )	{
		xo = 1;
	} else {
		xo = 0;
	}

	if ( header.image_descriptor & 0x20 )	{
		yo = 1;
	} else {
		yo = 0;
	}		

	// only accept 16 bit, compressed
	if(header.pixel_depth!=16) {
		cfclose(targa_file);
		return TARGA_ERROR_READING;
	}

	/*
	char test=char(header.image_descriptor&0xF);
	if((test!=8)&&(test!=0)) {
		cfclose(targa_file);
		return TARGA_ERROR_READING;
	}
	*/

	if((header.image_type!=1)&&(header.image_type!=2)&&(header.image_type!=9)&&(header.image_type!=10)) {
		cfclose(targa_file);
		return TARGA_ERROR_READING;
	}

	// skip the Image ID field -- should not be needed
	if(header.id_length>0) {
		if(cfseek(targa_file, header.id_length, CF_SEEK_SET)) {
			cfclose(targa_file);
			return TARGA_ERROR_READING;
		}
	}

	// read palette if one present.

	if (header.color_map_type)	{		 // non-zero indicates palette in the file
		Int3();

		// Determine the size of the color map
		Assert(header.cmap_depth==24);
		Assert(header.cmap_length<=256);
		Assert(palette);

		// Read the color map data
		int i;
		for (i = 0; i < header.cmap_length; i++)	{
			r = cfread_ubyte(targa_file);
			g = cfread_ubyte(targa_file);
			b = cfread_ubyte(targa_file);

			if(palette != NULL){
				palette[i*3+2] = r;
				palette[i*3+1] = g;
				palette[i*3+0] = b;
			}
		} 
		// Fill out with black.
		if(palette != NULL){
			for (; i < 256; i++)	{
				palette[i*3+2] = 0;
				palette[i*3+1] = 0;
				palette[i*3+0] = 0;
			}
		}
	}
	
	int bytes_remaining = cfilelength(targa_file)-cftell(targa_file);

	Assert(bytes_remaining>0);

	ubyte *fileptr = (ubyte*)malloc(bytes_remaining);
	Assert(fileptr);
	if(fileptr == NULL){
		return TARGA_ERROR_READING;
	}

	ubyte *src_pixels = fileptr;

	cfread(fileptr, bytes_remaining, 1, targa_file);	
	
	int rowsize = header.width * bytes_per_pixel;	

	if ( (header.image_type == 1) || (header.image_type == 2) || (header.image_type == 3) ) {
		// Uncompressed read

		for (int i = 0; i < header.height; i++)	{
			ubyte * pixptr;

			if ( ULORIGIN )	{
				pixptr = image_data + i * rowsize;
			} else {
				pixptr = image_data + ((header.height - i - 1) * rowsize);
			}
			 
			targa_read_pixel(header.width, &pixptr, &src_pixels, bytes_per_pixel, dest_size );
		}

	} else if (header.image_type == 9 || header.image_type == 10 || header.image_type == 11) {

		// the following handles RLE'ed targa data. 

		// targas encoded by the scanline -- loop on the height
		for (int i = 0; i < header.height; i++) {
			ubyte *pixdata;

			if (ULORIGIN)	{
				pixdata = image_data + i * rowsize;
			} else {
				pixdata = image_data + ((header.height - i - 1) * rowsize);
			}

			src_pixels += targa_uncompress( pixdata, src_pixels, header.width, bytes_per_pixel, dest_size );
		}

	}

	free(fileptr);
	cfclose(targa_file);
	targa_file = NULL;

	return TARGA_ERROR_NONE;
}

// Write out a Targa format bitmap.  Always writes out a top-up bitmap. 
// JAS: DOESN'T WORK WITH 8-BPP PALETTES
//
// filename: name of the Targa file, .tga extension added if not passed in
// data:     raw image data
// w:        width of the bitmap in pixels
// h:        height of the bitmap in pixels
// bpp:      bits per pixel of the bitmap
//
// returns:  0 if successful, otherwise -1
//
int targa_write_bitmap(char *real_filename, ubyte *data, ubyte *palette, int w, int h, int bpp)
{
	Assert(bpp == 24);
	char filename[MAX_FILENAME_LEN];
	CFILE *f;
	int bytes_per_pixel = BYTES_PER_PIXEL(bpp);		
		
	// open the file
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".tga" );

	f = cfopen( filename , "wb" );
	if ( !f ){
		return TARGA_ERROR_READING;
	}			

	// Write the TGA header
	cfwrite_ubyte(0, f);
	// f.write_ubyte(0);				//	IDLength

	cfwrite_ubyte(0, f);
	//f.write_ubyte(0);				//	ColorMapType

	cfwrite_ubyte(10, f);
	// f.write_ubyte(10);			//	image_type: 2 = 24bpp, uncompressed, 10=24bpp rle compressed

	cfwrite_ushort(0, f);
	// f.write_ushort(0);			// CMapStart

	cfwrite_ushort(0, f);
	// f.write_ushort(0);			//	CMapLength

	cfwrite_ubyte(0, f);
	// f.write_ubyte(0);				// CMapDepth

	cfwrite_ushort(0, f);
	// f.write_ushort(0);			//	XOffset

	cfwrite_ushort(0, f);
	// f.write_ushort(0);			//	YOffset

	cfwrite_ushort((ushort)w, f);
	// f.write_ushort((ushort)w);	//	Width

	cfwrite_ushort((ushort)h, f);
	// f.write_ushort((ushort)h);	//	Height

	cfwrite_ubyte(24, f);
	// f.write_ubyte(24);			// pixel_depth

	cfwrite_ubyte(0x20, f);
	// f.write_ubyte(0x20);				// ImageDesc  ( 0x20 = Origin at upper left )

	ubyte *compressed_data;
	compressed_data = (ubyte*)malloc(w * h * bytes_per_pixel);
	Assert(compressed_data);
	if(compressed_data == NULL){
		cfclose(f);
		return -1;
	}

	int compressed_data_len;
	compressed_data_len = targa_compress((char*)compressed_data, (char*)data, 3, bytes_per_pixel, w * h * bytes_per_pixel);
	if (compressed_data_len < 0) {
		free(compressed_data);
		cfclose(f);		
		return -1;
	}

	cfwrite(compressed_data, compressed_data_len, 1, f);
	cfclose(f);
	f = NULL;

	return 0;
}
