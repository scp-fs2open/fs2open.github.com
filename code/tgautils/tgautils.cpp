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
 * $Revision: 2.20 $
 * $Date: 2006-04-20 06:32:30 $
 * $Author: Goober5000 $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.19  2006/04/05 13:47:01  taylor
 * remove -tga16, it's obsolete now
 * add a temporary -no_emissive_light option to not use emission type light in OGL
 *
 * Revision 2.18  2005/12/28 22:32:37  taylor
 * properly deal with post-89 TGA images which have extra extension and developer info in the file
 *
 * Revision 2.17  2005/11/13 06:49:32  taylor
 * small big-endian fix
 *
 * Revision 2.16  2005/09/05 09:38:19  taylor
 * merge of OSX tree
 * a lot of byte swaps were still missing, will hopefully be fully network compatible now
 *
 * Revision 2.15  2005/05/12 17:49:17  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.14  2005/03/15 17:10:58  taylor
 * make sure w,h,bpp are useable before trying to assign values to them
 *
 * Revision 2.13  2005/03/13 23:07:36  taylor
 * enable 32-bit to 16-bit TGA conversion with -tga16 cmdline option (experimental)
 * fix crash when upgrading from original campaign stats file to current
 *
 * Revision 2.12  2005/02/23 05:05:38  taylor
 * compiler warning fixes (for MSVC++ 6)
 * have the warp effect only load as many LODs as will get used
 * head off strange bug in release when corrupt soundtrack number gets used
 *    (will still Assert in debug)
 * don't ever try and save a campaign savefile in multi or standalone modes
 * first try at 32bit->16bit color conversion for TGA code (for TGA only ship textures)
 *
 * Revision 2.11  2005/02/04 10:12:33  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.10  2004/10/31 22:00:57  taylor
 * new bmpman merge support, add PreProcDefines.h a few new places
 *
 * Revision 2.9  2004/10/09 17:44:10  taylor
 * don't use DevIL to read the header, slightly more sane error checking
 *
 * Revision 2.8  2004/09/28 19:54:32  Kazan
 * | is binary or, || is boolean or - please use the right one
 * autopilot velocity ramping biasing
 * made debugged+k kill guardianed ships
 *
 * Revision 2.7  2004/09/22 03:50:13  tbird
 * Modified the image loading to accept 24 and 32 bit images
 *
 * Revision 2.6  2004/07/26 20:47:53  Kazan
 * remove MCD complete
 *
 * Revision 2.5  2004/07/12 16:33:08  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.4  2004/04/26 02:14:38  taylor
 * 32-bit support with DevIL
 *
 * Revision 2.3  2004/02/28 14:14:57  randomtiger
 * Removed a few uneeded if DIRECT3D's.
 * Set laser function to only render the effect one sided.
 * Added some stuff to the credits.
 * Set D3D fogging to fall back to vertex fog if table fog not supported.
 *
 * Revision 2.2  2003/03/18 10:07:06  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.1  2002/11/04 03:02:29  randomtiger
 *
 * I have made some fairly drastic changes to the bumpman system. Now functionality can be engine dependant.
 * This is so D3D8 can call its own loading code that will allow good efficient loading and use of textures that it desparately needs without
 * turning bumpman.cpp into a total hook infested nightmare. Note the new bumpman code is still relying on a few of the of the old functions and all of the old bumpman arrays.
 *
 * I have done this by adding to the gr_screen list of function pointers that are set up by the engines init functions.
 * I have named the define calls the same name as the original 'bm_' functions so that I havent had to change names all through the code.
 *
 * Rolled back to an old version of bumpman and made a few changes.
 * Added new files: grd3dbumpman.cpp and .h
 * Moved the bitmap init function to after the 3D engine is initialised
 * Added includes where needed
 * Disabled (for now) the D3D8 TGA loading - RT
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
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

#include <string.h>

#include "globalincs/pstypes.h"
#include "tgautils/tgautils.h"
#include "cfile/cfile.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/2d.h"
#include "cmdline/cmdline.h"

extern int Cmdline_jpgtga;

// -----------------
//
// Defines
//
// -----------------

const char *Xfile_ID = "TRUEVISION-XFILE.";

#define TARGA_FOOTER_SIZE					26  	// using sizeof(targa_footer) doesn't work unless we
												// pack the struct and I don't want to do that :)

#define MAX_TARGA_RUN_LENGTH_PACKET 	128
#define TARGA_HEADER_LENGTH 			18
#define ULORIGIN						(header.image_descriptor & 0x20)

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


typedef struct targa_footer {
	uint ext_offset;		// file offset to extension area (we ignore it)
	uint dev_offset;		// file offset to developer area (we also ignore this)
	char sig_string[18];	// check to see if this is new TGA format, string should be value of Xfile_ID
} targa_footer;

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
// NOTE : for FreeSpace2, this also swizzles data into the proper screen (NOT texture) format - just like
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
	int pixel32;
	ubyte pal_index;
	ubyte r, g, b;
	ubyte al = 0;

	for(idx=0; idx<num_pixels; idx++){
		// 24 or 32 bit
		if ( (bytes_per_pixel == 3) || (bytes_per_pixel == 4) ) {
			memset( &pixel32, 0xff, sizeof(int) );
			memcpy( &pixel32, *src, bytes_per_pixel );

#if BYTE_ORDER == BIG_ENDIAN
			// on big-endian it will be used as ARGB so switch it back to BGRA
			if ( dest_size == 4 ) {
				pixel32 = INTEL_INT(pixel32);
			}
#endif

			// should have it's own alpha settings so just copy it out as is
			memcpy( *dst, &pixel32, dest_size );
		}
		// 8 or 16 bit
		else {
			// stuff the 16 bit pixel
			memcpy(&pixel, *src, bytes_per_pixel);

			pixel = INTEL_SHORT(pixel);
						
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
int targa_read_header(char *real_filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *palette )
{	
	targa_header header;
	CFILE *targa_file = NULL;
	char filename[MAX_FILENAME_LEN];

	if (img_cfp == NULL) {
		strcpy( filename, real_filename );

		char *p = strchr( filename, '.' );

		if ( p )
			*p = 0;

		strcat( filename, ".tga" );

		targa_file = cfopen( filename , "rb" );

		if ( !targa_file ) {
			return TARGA_ERROR_READING;
		}
	} else {
		targa_file = img_cfp;
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

	if (img_cfp == NULL) {
		cfclose(targa_file);
		targa_file = NULL;
	}

	Assert( (header.pixel_depth == 16) || (header.pixel_depth == 24) || (header.pixel_depth == 32) );

	// If we aren't using the -jpgtga option then don't even try to use anything other
	// than 16-bit TARGAs.  Otherwise DevIL should be available to deal with heigher bits.
	if ( !Cmdline_jpgtga && (header.pixel_depth != 16) )
		return TARGA_ERROR_READING;

	if ( Cmdline_jpgtga && (header.pixel_depth != 16) && (header.pixel_depth != 24) && (header.pixel_depth != 32) )
		return TARGA_ERROR_READING;

	if (w) *w = header.width;
	if (h) *h = header.height;
	if (bpp) *bpp = header.pixel_depth;

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
	targa_footer footer;
	CFILE *targa_file;
	char filename[MAX_FILENAME_LEN];
	ubyte r, g, b;
	int xfile_offset = 0;
		
	// open the file
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat( filename, ".tga" );

	targa_file = cfopen( filename , "rb" );
	if ( !targa_file ){
		return TARGA_ERROR_READING;
	}		

	// read the footer info first
	cfseek( targa_file, cfilelength(targa_file) - TARGA_FOOTER_SIZE, CF_SEEK_SET );

	memset( &footer, 0, sizeof(targa_footer) );

	footer.ext_offset = cfread_uint(targa_file);
	footer.dev_offset = cfread_uint(targa_file);

	cfread_string(footer.sig_string, sizeof(footer.sig_string), targa_file);

	if ( !strcmp(footer.sig_string, Xfile_ID) ) {
		// it's an extended file to lets be sure to skip the extra crap which comes after the
		// image data section, dev section comes first in the file and we only need one offset
		if (footer.dev_offset || footer.ext_offset) {
			xfile_offset = cfilelength(targa_file) - ((footer.dev_offset) ? footer.dev_offset : footer.ext_offset);
		}
	}

	// done with the footer so jump back and do normal reading
	cfseek( targa_file, 0, CF_SEEK_SET );

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

	// we're only allowing 2 bytes per pixel (16 bit compressed), unless Cmdline_jpgtga is used
	Assert( (bytes_per_pixel == 2) || (bytes_per_pixel == 3) || (bytes_per_pixel == 4) );
	if(!Cmdline_jpgtga && (bytes_per_pixel != 2)){
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
		if ( cfseek(targa_file, header.id_length, CF_SEEK_CUR) ) {
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

	int bytes_remaining = cfilelength(targa_file) - cftell(targa_file) - xfile_offset;

	Assert(bytes_remaining > 0);

	ubyte *fileptr = (ubyte*)vm_malloc(bytes_remaining);
	Assert(fileptr);
	if(fileptr == NULL){
		return TARGA_ERROR_READING;
	}

	ubyte *src_pixels = fileptr;

	cfread(fileptr, bytes_remaining, 1, targa_file);	
	
	int rowsize = header.width * dest_size;

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

	vm_free(fileptr);
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
	compressed_data = (ubyte*)vm_malloc(w * h * bytes_per_pixel);
	Assert(compressed_data);
	if(compressed_data == NULL){
		cfclose(f);
		return -1;
	}

	int compressed_data_len;
	compressed_data_len = targa_compress((char*)compressed_data, (char*)data, 3, bytes_per_pixel, w * h * bytes_per_pixel);
	if (compressed_data_len < 0) {
		vm_free(compressed_data);
		cfclose(f);		
		return -1;
	}

	cfwrite(compressed_data, compressed_data_len, 1, f);
	cfclose(f);
	f = NULL;

	return 0;
}
