#include "ddsutils/ddsutils.h"
#include "cfile/cfile.h"


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
inline int is_power_of_2(int n)
{
	return ( n && !(n & (n-1)) );
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
	int i, trash;

	if (img_cfp == NULL) {
		// this better not happen.. ever
		Assert(filename != NULL);

		// make sure there is an extension
		strcpy(real_name, filename);
		char *p = strchr(real_name, '.');
		if (p) { *p=0; }
		strcat(real_name, ".dds");

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

	// calculate the type and size of the data
	if (dds_header.ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
		// did I mention lately that I hate Microsoft?
		// this is here to fix the situation where Microsoft doesn't follow their own docs
		if ( dds_header.dwPitchOrLinearSize <= 0 ) {
			if (dds_header.dwDepth == 0)
				dds_header.dwDepth = 1;

			// calculate the block size, mult by 8 for DXT1, 16 for DXT3 & 5
			*size = ((dds_header.dwWidth + 3)/4) * ((dds_header.dwHeight + 3)/4) * ((dds_header.dwDepth + 3)/4) * ((dds_header.ddpfPixelFormat.dwFourCC == FOURCC_DXT1) ? 8 : 16);
		} else {
			*size = dds_header.dwMipMapCount > 1 ? dds_header.dwPitchOrLinearSize * 2 : dds_header.dwPitchOrLinearSize;
		}

		switch (dds_header.ddpfPixelFormat.dwFourCC) {
			case FOURCC_DXT1:
				bits = 24;
				ct = DDS_DXT1;
				break;

			case FOURCC_DXT3:
				bits = 32;
				ct = DDS_DXT3;
				break;

			case FOURCC_DXT5:
				bits = 32;
				ct = DDS_DXT5;
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
		if (dds_header.dwDepth == 0)
			dds_header.dwDepth = 1;

		if (dds_header.dwMipMapCount < 1)
			dds_header.dwMipMapCount = 1;

		int d_width = dds_header.dwWidth;
		int d_height = dds_header.dwHeight;
		int d_depth = dds_header.dwDepth;
		int d_size = 0;

		// calculate full data size for all mipmap levels
		for (i = 0; i < (int)dds_header.dwMipMapCount; i++) {
			d_size += d_width * d_height * d_depth * (dds_header.ddpfPixelFormat.dwRGBBitCount / 8);

			d_width /= 2;
			d_height /= 2;
			d_depth /= 2;

			if (d_width < 1)
				d_width = 1;

			if (d_height < 1)
				d_height = 1;

			if (d_depth < 1)
				d_depth = 1;
		}

		Assert( d_size > 0 );

		*size = d_size;
		
		bits = dds_header.ddpfPixelFormat.dwRGBBitCount;
		ct = DDS_UNCOMPRESSED;
	} else {
		// it's not a readable format
		retval = DDS_ERROR_INVALID_FORMAT;
		goto Done;
	}

	// we don't support compressed, non-power-of-2 images
	if ( (ct != DDS_UNCOMPRESSED) && (!is_power_of_2(dds_header.dwWidth) || !is_power_of_2(dds_header.dwHeight)) ) {
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
int dds_read_bitmap(char *filename, ubyte *data, ubyte *bpp)
{
	int retval;
	int w,h,ct,lvl;
	int size = 0, bits = 0;
	CFILE *cfp;
	char real_name[MAX_FILENAME_LEN];

	// this better not happen.. ever
	Assert(filename != NULL);

	// make sure there is an extension
	strcpy(real_name, filename);
	char *p = strchr(real_name, '.');
	if (p) { *p = 0; }
	strcat(real_name, ".dds");

	// open it up and go to the data section
	cfp = cfopen(real_name, "rb");

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

//returns string representation of DDS_** error code
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

		default:
			return "Abort, retry, fail?";
	}

	//get a warning otherwise
	return "Abort, retry, fail?";
}
