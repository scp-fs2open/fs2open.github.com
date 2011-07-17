#include <stdio.h>
#include <string.h>

#include "png.h"

#include "globalincs/pstypes.h"
#include "pngutils/pngutils.h"
#include "cfile/cfile.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/2d.h"

CFILE *png_file = NULL;

//copy/pasted from libpng
void png_scp_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;

	if (png_ptr == NULL)
		return;
	/* fread() returns 0 on error, so it is OK to store this in a png_size_t
	* instead of an int, which is what fread() actually returns.
	*/
	check = (png_size_t)cfread(data, (png_size_t)1, length, png_file);
	if (check != length)
		png_error(png_ptr, "Read Error");
}

// Reads header information from the PNG file into the bitmap pointer
//
// filename - name of the PNG bitmap file
// w - (output) width of the bitmap
// h - (output) height of the bitmap
// bpp - (output) bits per pixel of the bitmap
//
// returns - PNG_ERROR_NONE if successful, otherwise error code
//
int png_read_header(char *real_filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *palette)
{
	char filename[MAX_FILENAME_LEN];
	png_infop info_ptr;
	png_structp png_ptr;

	png_file = NULL;

	//mprintf(("png_read_header: %s\n", real_filename));

	if (img_cfp == NULL) {
		strcpy_s( filename, real_filename );

		char *p = strchr( filename, '.' );

		if ( p )
			*p = 0;

		strcat_s( filename, ".png" );

		png_file = cfopen( filename , "rb" );

		if ( !png_file ) {
			return PNG_ERROR_READING;
		}
	} else {
		png_file = img_cfp;
	}

	Assert( png_file != NULL );

	if (png_file == NULL)
		return PNG_ERROR_READING;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also supply the
	* the compiler header file version, so that we know if the application
	* was compiled with a compatible version of the library.  REQUIRED
	*/
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		mprintf(("png_read_header: error creating read struct\n"));
		cfclose(png_file);
		return PNG_ERROR_READING;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		mprintf(("png_read_header: error creating info struct\n"));
		cfclose(png_file);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return PNG_ERROR_READING;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		mprintf(("png_read_header: something went wrong\n"));
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		cfclose(png_file);
		/* If we get here, we had a problem reading the file */
		return PNG_ERROR_READING;
	}

	png_set_read_fn(png_ptr, &png_file, png_scp_read_data);

	png_read_info(png_ptr, info_ptr);

	if (w) *w = png_get_image_width(png_ptr, info_ptr);
	if (h) *h = png_get_image_height(png_ptr, info_ptr);
	// this turns out to be near useless, but meh
	if (bpp) *bpp = (png_get_channels(png_ptr, info_ptr) * png_get_bit_depth(png_ptr, info_ptr));

	if (img_cfp == NULL) {
		cfclose(png_file);
		png_file = NULL;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return PNG_ERROR_NONE;
}

// Loads a PNG image
//
// filename - name of the targa file to load
// image_data - allocated storage for the bitmap
//
// returns - true if succesful, false otherwise
//
int png_read_bitmap(char *real_filename, ubyte *image_data, ubyte *bpp, int dest_size, int cf_type)
{
	char filename[MAX_FILENAME_LEN];
	png_infop info_ptr;
	png_structp png_ptr;
	png_bytepp row_pointers;
	unsigned int i, len;

	png_file = NULL;

	strcpy_s( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;
	strcat_s( filename, ".png" );

	png_file = cfopen(filename, "rb", CFILE_NORMAL, cf_type);

	if (png_file == NULL)
		return PNG_ERROR_READING;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also supply the
	* the compiler header file version, so that we know if the application
	* was compiled with a compatible version of the library.  REQUIRED
	*/
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		mprintf(("png_read_bitmap: png_ptr went wrong\n"));
		cfclose(png_file);
		return PNG_ERROR_READING;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		mprintf(("png_read_bitmap: info_ptr went wrong\n"));
		cfclose(png_file);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return PNG_ERROR_READING;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		mprintf(("png_read_bitmap: something went wrong\n"));
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		cfclose(png_file);
		/* If we get here, we had a problem reading the file */
		return PNG_ERROR_READING;
	}

	png_set_read_fn(png_ptr, &png_file, png_scp_read_data);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR | PNG_TRANSFORM_EXPAND, NULL);
	len = png_get_rowbytes(png_ptr, info_ptr);

	row_pointers = png_get_rows(png_ptr, info_ptr);

	if(bpp)
		*bpp = (ubyte)(len / png_get_image_width(png_ptr, info_ptr)) << 3;

	//copy row data to image
	unsigned int height = png_get_image_height(png_ptr, info_ptr);
	for (i = 0; i < height; i++) {
		memcpy(&image_data[i * len], row_pointers[i], len);
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	cfclose(png_file);

	return PNG_ERROR_NONE;
}
