#include "ddsutils/ddsutils.h"
#include "cfile/cfile.h"

int Texture_compression_enabled=0;

static int dds_read_header_internal(LPDDSURFACEDESC2 lpddsd, char *filename, int *width, int *height, int *bpp, int *compression_type, int *levels)
{
	char code[5];
	CFILE *ddsfile;
	char real_name[MAX_FILENAME_LEN];
	char *p;
	int retval=DDS_ERROR_NONE;
	int ct;
	int bits=0;

	//this better not happen.. ever
	Assert(filename!=NULL);

	//make sure there is an extension
	strcpy(real_name, filename);
	p=strchr(real_name, '.');
	if (p) *p=0;
	strcat(real_name, ".dds");

	memset(code,0,sizeof(code));
	//try to open the file
	ddsfile = cfopen(real_name, "rb");

	//file not found
	if (!ddsfile)
	{
		return DDS_ERROR_INVALID_FILENAME;
	}

	//read the code
	cfread(code,1,4,ddsfile);
	code[4]=0;

	//check it
	if (strncmp(code,DDS_FILECODE,4)!=0)
	{
		return DDS_ERROR_BAD_HEADER;
	}

	//read the header
	cfread(lpddsd,sizeof(DDSURFACEDESC2),1,ddsfile);

	//stuff important info
	if (width)
		*width = lpddsd->dwWidth;

	if (height)
		*height = lpddsd->dwHeight;

	if (levels)
		*levels = lpddsd->dwMipMapCount;

	switch(lpddsd->ddpfPixelFormat.dwFourCC)
	{
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

		//dxt2 and dxt4 aren't supported
		case FOURCC_DXT2:
		case FOURCC_DXT4:
			retval=DDS_ERROR_INVALID_FORMAT;
			ct=DDS_DXT_INVALID;
			break;

		//none of the above
		default:
			retval=DDS_ERROR_UNCOMPRESSED;
			ct=DDS_DXT_INVALID;
			break;
	}

	if (bpp) *bpp=bits;
	if (compression_type) *compression_type=ct;

	//close file and return
	cfclose(ddsfile);
	return retval;
}

int dds_read_header(char *filename, int *width, int *height, int *bpp, int *compression_type, int *levels)
{
	DDSURFACEDESC2 ddsd;
	return dds_read_header_internal(&ddsd, filename, width, height, bpp, compression_type, levels);
}

//reads pixel info from a dds file
int dds_read_bitmap(char *filename, int *size, uint* data)
{
	int retval;
	DDSURFACEDESC2 ddsd;
	int w,h,ct,lvl,bpp;
	CFILE *cfp;
	ubyte* tmp;

	//this must be set
	Assert(size!=NULL);
	
	//read the header -- if its at this stage, it should be legal.
	retval=dds_read_header_internal(&ddsd, filename, &w, &h, &bpp, &ct, &lvl);
	Assert(retval==DDS_ERROR_NONE);

	//get some more info from the surface
	*size=ddsd.dwMipMapCount > 1 ? ddsd.dwLinearSize * 2 : ddsd.dwLinearSize;

	//open it up and go to the data section
	cfp=cfopen(filename,"rb");
	cfseek(cfp,DDS_OFFSET, CF_SEEK_SET);

	//get the data
	tmp=(ubyte*)malloc(*size);
	
	if (!tmp)
	{
		cfclose(cfp);
		return DDS_ERROR_NO_MEM;		//barf
	}

	cfread(tmp,1,*size,cfp);

	*data=(uint)tmp;
	//we look done here
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


	






