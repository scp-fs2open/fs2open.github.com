#include "ddsutils/ddsutils.h"
#include "cfile/cfile.h"



int Texture_compression_enabled=0;


int dds_read_header(char *filename, CFILE *img_cfp, int *width, int *height, int *bpp, int *compression_type, int *levels, int *size)
{
	DDSURFACEDESC2 dds_header;
	char code[5];
	CFILE *ddsfile;
	char real_name[MAX_FILENAME_LEN];
	char *p;
	int retval = DDS_ERROR_NONE;
	int ct;
	int bits = 0;

	if (img_cfp == NULL) {
		// this better not happen.. ever
		Assert(filename != NULL);

		// make sure there is an extension
		strcpy(real_name, filename);
		p = strchr(real_name, '.');
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

	memset(code, 0, sizeof(code));


	// read the code
	cfread(code, 1, 4, ddsfile);
	code[4] = 0;

	// check it
	if (strncmp(code, DDS_FILECODE, 4) != 0)
		return DDS_ERROR_BAD_HEADER;

	// read the header
	cfread(&dds_header, sizeof(DDSURFACEDESC2), 1, ddsfile);

	// stuff important info
	if (width)
		*width = dds_header.dwWidth;

	if (height)
		*height = dds_header.dwHeight;

	if (levels)
		*levels = dds_header.dwMipMapCount;

	// calculate the size of the data
	if (dds_header.dwFlags & DDSD_LINEARSIZE) {
		*size = dds_header.dwMipMapCount > 1 ? dds_header.dwPitchOrLinearSize * 2 : dds_header.dwPitchOrLinearSize;
	} else {
		// it's not a compressed format
		retval = DDS_ERROR_UNCOMPRESSED;
	}

	switch (dds_header.ddpfPixelFormat.dwFourCC) {
		case FOURCC_DXT1:
			bits=24;
			ct=DDS_DXT1;
			break;

		case FOURCC_DXT3:
			bits=32;
			ct=DDS_DXT3;
			break;

		case FOURCC_DXT5:
			bits=32;
			ct=DDS_DXT5;
			break;

		// dxt2 and dxt4 aren't supported
		case FOURCC_DXT2:
		case FOURCC_DXT4:
			retval = DDS_ERROR_INVALID_FORMAT;
			ct = DDS_DXT_INVALID;
			break;

		// none of the above
		default:
			retval = DDS_ERROR_UNCOMPRESSED;
			ct = DDS_DXT_INVALID;
			break;
	}

	if (bpp)
		*bpp = bits;

	if (compression_type)
		*compression_type = ct;

	if (img_cfp == NULL) {
		// close file and return
		cfclose(ddsfile);
		ddsfile = NULL;
	}

	return retval;
}

//reads pixel info from a dds file
int dds_read_bitmap(char *filename, ubyte **data, ubyte *bpp)
{
	int retval;
	int w,h,ct,lvl;
	int size = 0, bits = 0;
	CFILE *cfp;

	// open it up and go to the data section
	cfp = cfopen(filename, "rb");

	// just in case
	if (cfp == NULL)
		return DDS_ERROR_INVALID_FILENAME;

	// read the header -- if its at this stage, it should be legal.
	retval = dds_read_header(filename, cfp, &w, &h, &bits, &ct, &lvl, &size);

	Assert(retval == DDS_ERROR_NONE);

	// this really shouldn't be needed but better safe than sorry
	if (retval != DDS_ERROR_NONE) {
		return retval;
	}

	cfseek(cfp, DDS_OFFSET, CF_SEEK_SET);

	// read in the data
	cfread(*data, 1, size, cfp);

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

		case DDS_ERROR_NO_MEM:
			return "Insufficient memory";

		case DDS_ERROR_INVALID_FILENAME:
			return "File not found";

		case DDS_ERROR_BAD_HEADER:
			return "Filecode did not equal \"DDS \"";

		case DDS_ERROR_INVALID_FORMAT:
			return "File was compressed, but it was DXT2 or DXT4";
			
		case DDS_ERROR_UNCOMPRESSED:
			return "*.DDS files must be compressed and use DXT1, DXT3, or DXT5 compression";

		default:
			return "Abort, retry, fail?";
	}

	//get a warning otherwise
	return "Abort, retry, fail?";
}
