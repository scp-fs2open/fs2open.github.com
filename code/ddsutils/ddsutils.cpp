
#include "ddsutils/ddsutils.h"
#include "cfile/cfile.h"
#include "osapi/osregistry.h"


/*	Currently supported formats:
 *		DXT1a	(compressed)
 *		DXT1c	(compressed)
 *		DXT3	(compressed)
 *		DXT5	(compressed)
 *		uncompressed 1555	(16-bit, 1-bit being alpha)
 *		uncompressed 8888	(32-bit)
 *		uncompressed 888	(24-bit, no alpha)
 *		paletted 8-bit		(256 colors)
 */
 

int Texture_compression_available = 0;
int Use_compressed_textures = 0;

// power of two check, to verify that a given DDS will always be usable since
// some may not be power-of-2
static inline int is_power_of_two(int w, int h)
{
	return ( (w && !(w & (w-1))) && (h && !(h & (h-1))) );
}


int dds_read_header(char *filename, CFILE *img_cfp, int *width, int *height, int *bpp, int *compression_type, int *levels, int *size, ubyte *palette)
{
	DDSURFACEDESC2 dds_header;
	int code = 0;
	CFILE *ddsfile;
	char real_name[MAX_FILENAME_LEN];
	int retval = DDS_ERROR_NONE;
	int ct = DDS_UNCOMPRESSED;
	int bits = 0;
	int i, trash, is_cubemap = 0;
	int d_width = 0, d_height = 0, d_depth = 0, d_size = 0;


	if (img_cfp == NULL) {
		// this better not happen.. ever
		Assert(filename != NULL);

		// make sure there is an extension
		strcpy_s(real_name, filename);
		char *p = strchr(real_name, '.');
		if (p) { *p=0; }
		strcat_s(real_name, ".dds");

		// try to open the file
		ddsfile = cfopen(real_name, "rb");

		// file not found
		if (ddsfile == NULL)
			return DDS_ERROR_INVALID_FILENAME;
	} else {
		ddsfile = img_cfp;
	}

	// read the code
	code = cfread_int(ddsfile);

	// check it
	if (code != DDS_FILECODE) {
		retval = DDS_ERROR_BAD_HEADER;
		goto Done;
	}

	// read header variables
	dds_header.dwSize				= cfread_uint(ddsfile);
	dds_header.dwFlags				= cfread_uint(ddsfile);
	dds_header.dwHeight				= cfread_uint(ddsfile);
	dds_header.dwWidth				= cfread_uint(ddsfile);
	dds_header.dwPitchOrLinearSize	= cfread_uint(ddsfile);
	dds_header.dwDepth				= cfread_uint(ddsfile);
	dds_header.dwMipMapCount		= cfread_uint(ddsfile);

	// skip over the crap we don't care about
	for (i = 0; i < 11; i++)
		trash = cfread_uint(ddsfile);

	dds_header.ddpfPixelFormat.dwSize				= cfread_uint(ddsfile);
	dds_header.ddpfPixelFormat.dwFlags				= cfread_uint(ddsfile);
	dds_header.ddpfPixelFormat.dwFourCC				= cfread_uint(ddsfile);
	dds_header.ddpfPixelFormat.dwRGBBitCount		= cfread_uint(ddsfile);
	dds_header.ddpfPixelFormat.dwRBitMask			= cfread_uint(ddsfile);
	dds_header.ddpfPixelFormat.dwGBitMask			= cfread_uint(ddsfile);
	dds_header.ddpfPixelFormat.dwBBitMask			= cfread_uint(ddsfile);
	dds_header.ddpfPixelFormat.dwRGBAlphaBitMask	= cfread_uint(ddsfile);

	dds_header.ddsCaps.dwCaps1		= cfread_uint(ddsfile);
	dds_header.ddsCaps.dwCaps2		= cfread_uint(ddsfile);

	// sanity
	if (dds_header.dwDepth == 0)
		dds_header.dwDepth = 1;

	if (dds_header.dwMipMapCount < 1)
		dds_header.dwMipMapCount = 1;

	d_width = dds_header.dwWidth;
	d_height = dds_header.dwHeight;
	d_depth = dds_header.dwDepth;

	// calculate the type and size of the data
	if (dds_header.ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
		// did I mention lately that I hate Microsoft?
		// this is here to fix the situation where Microsoft doesn't follow their own docs
		if ( dds_header.dwPitchOrLinearSize <= 0 ) {
			// calculate the block size, mult by 8 for DXT1, 16 for DXT3 & 5
			d_size = ((dds_header.dwWidth + 3)/4) * ((dds_header.dwHeight + 3)/4) * ((dds_header.dwDepth + 3)/4) * ((dds_header.ddpfPixelFormat.dwFourCC == FOURCC_DXT1) ? 8 : 16);
		} else {
			d_size = dds_header.dwPitchOrLinearSize;
		}

		// if we have mipmaps then compute them into the final size too
		for (i = 1; i < (int)dds_header.dwMipMapCount; i++) {
			// reduce size by half for the next pass
			d_width /= 2;
			d_height /= 2;

			if (d_width <= 0)
				d_width = 1;

			if (d_height <= 0)
				d_height = 1;

			// size of data block (4x4)
			d_size += ((d_width + 3) / 4) * ((d_height + 3) / 4) * ((dds_header.ddpfPixelFormat.dwFourCC == FOURCC_DXT1) ? 8 : 16);
		}

		if ( dds_header.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP ) {
			if ( !(dds_header.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES) ) {
				retval = DDS_ERROR_CUBEMAP_FACES;
				ct = DDS_DXT_INVALID;
				goto Done;
			}

			// it's a valid cubemap, probably
			is_cubemap = 1;

			// the previously computed size is just per face, mult by 6 for full size
			d_size *= 6;
		}

		Assert( d_size > 0 );
		*size = d_size;

		switch (dds_header.ddpfPixelFormat.dwFourCC) {
			case FOURCC_DXT1:
				bits = 24;
				ct = (is_cubemap) ? DDS_CUBEMAP_DXT1 : DDS_DXT1;
				break;

			case FOURCC_DXT3:
				bits = 32;
				ct = (is_cubemap) ? DDS_CUBEMAP_DXT3 : DDS_DXT3;
				break;

			case FOURCC_DXT5:
				bits = 32;
				ct = (is_cubemap) ? DDS_CUBEMAP_DXT5 : DDS_DXT5;
				break;

			// dxt2 and dxt4 aren't supported
			case FOURCC_DXT2:
			case FOURCC_DXT4:
				retval = DDS_ERROR_UNSUPPORTED;
				ct = DDS_DXT_INVALID;
				goto Done;
				break;

			// none of the above
			default:
				retval = DDS_ERROR_INVALID_FORMAT;
				ct = DDS_DXT_INVALID;
				goto Done;
				break;
		}
	} else if ( dds_header.ddpfPixelFormat.dwFlags & (DDPF_RGB | DDPF_PALETTEINDEXED8) ) {
		d_size += d_width * d_height * d_depth * (dds_header.ddpfPixelFormat.dwRGBBitCount / 8);

		// calculate full data size for all mipmap levels
		for (i = 1; i < (int)dds_header.dwMipMapCount; i++) {
			d_width /= 2;
			d_height /= 2;
			d_depth /= 2;

			if (d_width < 1)
				d_width = 1;

			if (d_height < 1)
				d_height = 1;

			if (d_depth < 1)
				d_depth = 1;

			d_size += d_width * d_height * d_depth * (dds_header.ddpfPixelFormat.dwRGBBitCount / 8);
		}

		Assert( d_size > 0 );

		if ( dds_header.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP ) {
			if ( !(dds_header.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES) ) {
				retval = DDS_ERROR_CUBEMAP_FACES;
				ct = DDS_DXT_INVALID;
				goto Done;
			}

			// it's a valid cubemap, probably
			is_cubemap = 1;

			// the previously computed size is just per face, mult by 6 for full size
			d_size *= 6;
		}

		*size = d_size;
		
		bits = dds_header.ddpfPixelFormat.dwRGBBitCount;
		ct = (is_cubemap) ? DDS_CUBEMAP_UNCOMPRESSED : DDS_UNCOMPRESSED;
	} else {
		// it's not a readable format
		retval = DDS_ERROR_INVALID_FORMAT;
		goto Done;
	}

	// we don't support compressed, non-power-of-2 images
	if ( (ct != DDS_UNCOMPRESSED) && (!is_power_of_two(dds_header.dwWidth, dds_header.dwHeight)) ) {
		retval = DDS_ERROR_NON_POWER_OF_2;
		goto Done;
	}

	// make sure that the video card can handle compressed textures before using them
	if ( !Use_compressed_textures && (ct != DDS_UNCOMPRESSED) && (ct != DDS_DXT_INVALID) ) {
		retval = DDS_ERROR_NO_COMPRESSION;
		goto Done;
	}

	// stuff important info
	if (bpp)
		*bpp = bits;

	if (compression_type)
		*compression_type = ct;

	if (width)
		*width = dds_header.dwWidth;

	if (height)
		*height = dds_header.dwHeight;

	if (levels)
		*levels = dds_header.dwMipMapCount;

	if (palette && (bits == 8)) {
		cfseek(ddsfile, DDS_OFFSET, CF_SEEK_SET);
		cfread(palette, 1, 1024, ddsfile);
	}


Done:
	if (img_cfp == NULL) {
		// close file and return
		cfclose(ddsfile);
		ddsfile = NULL;
	}

	return retval;
}

//reads pixel info from a dds file
int dds_read_bitmap(char *filename, ubyte *data, ubyte *bpp, int cf_type)
{
	int retval;
	int w,h,ct,lvl;
	int size = 0, bits = 0;
	CFILE *cfp;
	char real_name[MAX_FILENAME_LEN];

	// this better not happen.. ever
	Assert(filename != NULL);

	// make sure there is an extension
	strcpy_s(real_name, filename);
	char *p = strchr(real_name, '.');
	if (p) { *p = 0; }
	strcat_s(real_name, ".dds");

	// open it up and go to the data section
	cfp = cfopen(real_name, "rb", CFILE_NORMAL, cf_type);

	// just in case
	if (cfp == NULL)
		return DDS_ERROR_INVALID_FILENAME;

	// read the header -- if its at this stage, it should be legal.
	retval = dds_read_header(real_name, cfp, &w, &h, &bits, &ct, &lvl, &size);
	Assert(retval == DDS_ERROR_NONE);

	// this really shouldn't be needed but better safe than sorry
	if (retval != DDS_ERROR_NONE) {
		return retval;
	}

	cfseek(cfp, DDS_OFFSET, CF_SEEK_SET);

	// read in the data
	cfread(data, 1, size, cfp);

	if (bpp)
		*bpp = (ubyte)bits;

	// we look done here
	cfclose(cfp);

	return DDS_ERROR_NONE;
}

// save some image data as a DDS image
// NOTE: we only support, uncompressed, 24-bit RGB and 32-bit RGBA images here!!
void dds_save_image(int width, int height, int bpp, int num_mipmaps, ubyte *data, int cubemap, char *filename)
{
	DDSURFACEDESC2 dds_header;
	char real_filename[MAX_FILENAME_LEN];

	if (data == NULL) {
		Int3();
		return;
	}

	// header size check
	if (sizeof(DDSURFACEDESC2) != 124) {
		Int3();
		return;
	}


	// not that that's out of the way we can get down to business, constructing a sane filename
	memset( &real_filename, 0, sizeof(real_filename) );

	int count = os_config_read_uint(NULL, "ImageExportNum", 0);

	if (count > 999)
		count = 0;

	if (filename == NULL) {
		if (cubemap) {
			sprintf(real_filename, "cubemap%.3d.dds", count++);
		} else {
			sprintf(real_filename, "image%.3d.dds", count++);
		}
		os_config_write_uint(NULL, "ImageExportNum", count);
	} else {
		Assert( strlen(filename) < MAX_FILENAME_LEN-5 );

		strcpy_s(real_filename, filename);

		char *p = strchr(real_filename, '.');
		if (p) { *p = 0; }

		strcat_s(real_filename, ".dds");
	}

	CFILE *image = cfopen( real_filename, "wb", CFILE_NORMAL, CF_TYPE_CACHE );

	if (image == NULL) {
		mprintf(("Unable to open DDS image for saving!!\n"));
		return;
	}

	if (num_mipmaps < 1)
		num_mipmaps = 1;

	// we have a filename and file handle, so now lets create our DDS header...
	memset( &dds_header, 0, sizeof(DDSURFACEDESC2) );

	uint flags = (DDSD_CAPS | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT);
	uint pixel_flags = DDPF_RGB;
	uint caps1 = DDSCAPS_TEXTURE;
	uint caps2 = 0;

	if (bpp == 32) {
		pixel_flags |= DDPF_ALPHAPIXELS;
	}

	if (num_mipmaps > 1) {
		flags |= DDSD_MIPMAPCOUNT;
		caps1 |= (DDSCAPS_COMPLEX | DDSCAPS_MIPMAP);
	}

	if (cubemap) {
		caps1 |= DDSCAPS_COMPLEX;
		caps2 |= (DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_ALLFACES);
	}

	dds_header.dwSize				= 124;
	dds_header.dwFlags				= flags;
	dds_header.dwHeight				= height;
	dds_header.dwWidth				= width;
	dds_header.dwPitchOrLinearSize	= (width * (bpp >> 3));
	dds_header.dwDepth				= 0;
	dds_header.dwMipMapCount		= num_mipmaps;

	dds_header.ddpfPixelFormat.dwSize				= 32;
	dds_header.ddpfPixelFormat.dwFlags				= pixel_flags;
	dds_header.ddpfPixelFormat.dwFourCC				= 0;
	dds_header.ddpfPixelFormat.dwRGBBitCount		= bpp;
	dds_header.ddpfPixelFormat.dwRBitMask			= 0x00ff0000;
	dds_header.ddpfPixelFormat.dwGBitMask			= 0x0000ff00;
	dds_header.ddpfPixelFormat.dwBBitMask			= 0x000000ff;
	dds_header.ddpfPixelFormat.dwRGBAlphaBitMask	= (bpp == 32) ? 0xff000000 : 0x00000000;

	dds_header.ddsCaps.dwCaps1		= caps1;
	dds_header.ddsCaps.dwCaps2		= caps2;

	// now save it all...
	uint dds_id = DDS_FILECODE;
	cfwrite(&dds_id, 1, 4, image);
	cfwrite(&dds_header, 1, sizeof(DDSURFACEDESC2), image);

	int faces = (cubemap) ? 6 : 1;
	int f_width = width;
	int f_height = height;
	int f_size = 0, f_offset = 0;

	// cubemaps are written:
	//  face 1
	//    all face 1 mipmaps
	//  face 2
	//    all face 2 mipmaps
	// ... etc.

	for (int i = 0; i < faces; i++) {
		for (int j = 0; j < num_mipmaps; j++) {
			f_size = (f_width * f_height * (bpp >> 3));

			cfwrite(data + f_offset, 1, f_size, image);

			// increase offset for next mipmap level
			f_offset += f_size;

			// half width and height for next mipmap level
			f_width /= 2;
			f_height /= 2;

			if (f_width < 1)
				f_width = 1;

			if (f_height < 1)
				f_height = 1;
		}

		// reset width and height to original values for the next face
		f_width = width;
		f_height = height;
	}

	// done!  now lets go get something to eat...
	cfclose(image);
}


// returns string representation of DDS_** error code
const char *dds_error_string(int code)
{
	switch (code)
	{
		case DDS_ERROR_NONE:
			return "No error";

		case DDS_ERROR_INVALID_FILENAME:
			return "File not found";

		case DDS_ERROR_BAD_HEADER:
			return "Filecode did not equal \"DDS \"";

		case DDS_ERROR_INVALID_FORMAT:
			return "DDS was in an unsupported/unknown format";
			
		case DDS_ERROR_UNSUPPORTED:
			return "DDS format was known but is not supported (ie. DXT2/DXT4)";

		case DDS_ERROR_NO_COMPRESSION:
			return "DDS is compressed but compression support is not enabled";

		case DDS_ERROR_NON_POWER_OF_2:
			return "Cannot load DDS if not power-of-2";

		case DDS_ERROR_CUBEMAP_FACES:
			return "Cubemaps must have all 6 faces";

		default:
			return "Abort, retry, fail?";
	}
}
