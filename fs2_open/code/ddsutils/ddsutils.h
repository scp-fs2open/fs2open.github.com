//header file to load .dds files
//these use dxtc texture compression

#ifndef __DDS_H
#define __DDS_H

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"


#define DDS_ERROR_NONE					0		// everything went fine
#define DDS_ERROR_INVALID_FILENAME		1		// bad or missing file
#define DDS_ERROR_UNSUPPORTED			2		// a known format but one we don't support
#define DDS_ERROR_INVALID_FORMAT		3		// format that's not supported
#define DDS_ERROR_BAD_HEADER			4		// header was not "DDS "
#define DDS_ERROR_NO_COMPRESSION		5		// file is compressed, compression isn't supported
#define DDS_ERROR_NON_POWER_OF_2		6		// file is not a power of 2 in dimensions
#define DDS_ERROR_CUBEMAP_FACES			7		// file is a cubemap, but doesn't have all six faces


#define DDS_DXT_INVALID				-1
#define DDS_UNCOMPRESSED				0
#define DDS_DXT1						1
#define DDS_DXT3						3
#define DDS_DXT5						5
#define DDS_CUBEMAP_UNCOMPRESSED		10
#define DDS_CUBEMAP_DXT1				11
#define DDS_CUBEMAP_DXT3				13
#define DDS_CUBEMAP_DXT5				15

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint)(ubyte)(ch0) | ((uint)(ubyte)(ch1) << 8) |   \
                ((uint)(ubyte)(ch2) << 16) | ((uint)(ubyte)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

// FOURCC codes for DX compressed-texture pixel formats
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

#define DDS_FILECODE	0x20534444	// "DDS " in file

// DDS format stuff ...
#define DDSD_LINEARSIZE			0x00080000
#define DDSD_PITCH				0x00000008
#define DDPF_ALPHAPIXELS		0x00000001
#define DDPF_FOURCC				0x00000004
#define DDPF_PALETTEINDEXED4	0x00000008
#define DDPF_PALETTEINDEXEDTO8	0x00000010
#define DDPF_PALETTEINDEXED8	0x00000020
#define DDPF_RGB				0x00000040
#define DDSD_PIXELFORMAT		0x00001000
#define DDSD_WIDTH				0x00000004
#define DDSD_HEIGHT				0x00000002
#define DDSD_CAPS				0x00000001
#define DDSD_MIPMAPCOUNT		0x00020000

#define DDSCAPS_COMPLEX			0x00000008
#define DDSCAPS_PRIMARYSURFACE	0x00000200
#define DDSCAPS_MIPMAP			0x00400000
#define DDSCAPS_TEXTURE			0x00001000

#define DDSCAPS2_CUBEMAP				0x00000200
#define DDSCAPS2_VOLUME					0x00200000
#define DDSCAPS2_CUBEMAP_POSITIVEX		0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX		0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY		0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY		0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ		0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ		0x00008000

#define DDSCAPS2_CUBEMAP_ALLFACES	( DDSCAPS2_CUBEMAP_POSITIVEX |	\
									  DDSCAPS2_CUBEMAP_NEGATIVEX |	\
									  DDSCAPS2_CUBEMAP_POSITIVEY |	\
									  DDSCAPS2_CUBEMAP_NEGATIVEY |	\
									  DDSCAPS2_CUBEMAP_POSITIVEZ |	\
									  DDSCAPS2_CUBEMAP_NEGATIVEZ )

#pragma pack(1)
// these structures are the headers for a dds file
/*typedef struct _DDPIXELFORMAT {
	uint	dwSize;
	uint	dwFlags;
	uint	dwFourCC;
	uint	dwRGBBitCount;
	uint	dwRBitMask;
	uint 	dwGBitMask;
	uint 	dwBBitMask;
	uint	dwRGBAlphaBitMask;
} DDPIXELFORMAT;

typedef struct _DDSCAPS2
{
	uint		dwCaps1;
	uint		dwCaps2;
	uint		Reserved[2];
} DDSCAPS2;*/

typedef struct _DDSURFACEDESC2
{
	uint			dwSize;				// size of the DDSURFACEDESC structure
	uint			dwFlags;			// determines what fields are valid
	uint			dwHeight;			// height of surface to be created
	uint			dwWidth;			// width of input surface
	uint			dwPitchOrLinearSize;
	uint			dwDepth;
	uint			dwMipMapCount;
	uint			dwReserved1[11];

	struct {
		uint	dwSize;
		uint	dwFlags;
		uint	dwFourCC;
		uint	dwRGBBitCount;
		uint	dwRBitMask;
		uint 	dwGBitMask;
		uint 	dwBBitMask;
		uint	dwRGBAlphaBitMask;
	} ddpfPixelFormat;

	struct {
		uint		dwCaps1;
		uint		dwCaps2;
		uint		Reserved[2];
	} ddsCaps;

//	DDPIXELFORMAT	ddpfPixelFormat;
//	DDSCAPS2		ddsCaps;			// direct draw surface capabilities
	uint			dwReserved2;
} DDSURFACEDESC2;
#pragma pack()

#define DDS_OFFSET						4+sizeof(DDSURFACEDESC2)		//place where the data starts -- should be 128

//reads a dds header
//returns one of the error values
//'compression_type' comes back as one of the DDS_DXTC* defines
int dds_read_header(char *filename, CFILE *img_cfp = NULL, int *width = 0, int *height = 0, int *bpp = 0, int *compression_type = 0, int *levels = 0, int *size = 0, ubyte *palette = NULL);

//reads bitmap
//size of the data it stored in size
int dds_read_bitmap(char *filename, ubyte *data, ubyte *bpp = NULL, int cf_type = CF_TYPE_ANY);

// writes a DDS file using given data
void dds_save_image(int width, int height, int bpp, int num_mipmaps, ubyte *data = NULL, int cubemap = 0, char *filename = NULL);

//returns a string from a DDS error code
const char *dds_error_string(int code);

extern int Texture_compression_available;
extern int Use_compressed_textures;

#endif //__DDS_H
