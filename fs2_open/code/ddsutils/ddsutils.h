//header file to load .dds files
//these use dxtc texture compression

#ifndef __DDS_H
#define __DDS_H

#include "PreProcDefines.h"
#include "globalincs/pstypes.h"
#ifdef _WIN32
#include "directx/vddraw.h"
#endif

struct CFILE;

#define DDS_ERROR_NONE					0		//everything went fine
#define DDS_ERROR_INVALID_FILENAME		1		//bad filename
#define DDS_ERROR_NO_MEM				2		//not enough mem to load
#define DDS_ERROR_UNSUPPORTED			3		//not a readable format		
#define DDS_ERROR_INVALID_FORMAT		4		//can only use dxtc1, dxtc3, dxtc5
#define DDS_ERROR_BAD_HEADER			5		//header was not "DDS "


#define DDS_DXT_INVALID				-1
#define DDS_UNCOMPRESSED				0
#define DDS_DXT1						1
#define DDS_DXT3						3
#define DDS_DXT5						5

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

// FOURCC codes for DX compressed-texture pixel formats
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

#define DDS_FILECODE	0x20534444	// "DDS " in file

#ifndef DUMMYUNIONNAMEN
#if defined(__cplusplus) || !defined(NONAMELESSUNION)
#define DUMMYUNIONNAMEN(n)
#else
#define DUMMYUNIONNAMEN(n)      u##n
#endif
#endif

//these structures are the headers for a dds file
typedef struct _DDSCAPS2
{
	DWORD		dwCaps1;
	DWORD		dwCaps2;
	DWORD		Reserved[2];
} DDSCAPS2;

typedef struct _DDSURFACEDESC2
{
    DWORD               dwSize;                 // size of the DDSURFACEDESC structure
    DWORD               dwFlags;                // determines what fields are valid
    DWORD               dwHeight;               // height of surface to be created
    DWORD               dwWidth;                // width of input surface
	DWORD				dwPitchOrLinearSize;
	DWORD				dwDepth;
	DWORD				dwMipMapCount;
	DWORD				dwReserved1[11];
	DDPIXELFORMAT		ddpfPixelFormat;
    DDSCAPS2            ddsCaps;                // direct draw surface capabilities
	DWORD				dwReserved2;
} DDSURFACEDESC2, FAR* LPDDSURFACEDESC2;

#define DDS_OFFSET						4+sizeof(DDSURFACEDESC2)		//place where the data starts -- should be 128

//reads a dds header
//returns one of the error values
//'compression_type' comes back as one of the DDS_DXTC* defines
int dds_read_header(char *filename, CFILE *img_cfp = NULL, int *width = 0, int *height = 0, int *bpp = 0, int *compression_type = 0, int *levels = 0, int *size = 0);

//reads bitmap
//size of the data it stored in size
int dds_read_bitmap(char *filename, ubyte *data, ubyte *bpp = NULL);

//returns a string from a DDS error code
const char* dds_error_string(int code);

extern int Texture_compression_enabled;

#endif //__DDS_H
