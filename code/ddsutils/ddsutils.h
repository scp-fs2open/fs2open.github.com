//header file to load .dds files
//these use dxtc texture compression

#ifndef __DDS_H
#define __DDS_H

#include "PreProcDefines.h"
#include "globalincs/pstypes.h"
#include "directx/vddraw.h"

#define DDS_ERROR_NONE					0		//everything went fine
#define DDS_ERROR_INVALID_FILENAME		1		//bad filename
#define DDS_ERROR_NO_MEM				2		//not enough mem to load
#define DDS_ERROR_UNCOMPRESSED			3		//not using dxtc compression		
#define DDS_ERROR_INVALID_FORMAT		4		//can only use dxtc1, dxtc3, dxtc5
#define DDS_ERROR_BAD_HEADER			5		//header was not "DDS "


#define DDS_DXT_INVALID				-1
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

#define DDS_FILECODE					"DDS "

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
    DWORD       dwCaps;         // capabilities of surface wanted
    DWORD       dwCaps2;
    DWORD       dwCaps3;
    union
    {
        DWORD       dwCaps4;
        DWORD       dwVolumeDepth;
    } DUMMYUNIONNAMEN(1);
} DDSCAPS2;

typedef struct _DDSURFACEDESC2
{
    DWORD               dwSize;                 // size of the DDSURFACEDESC structure
    DWORD               dwFlags;                // determines what fields are valid
    DWORD               dwHeight;               // height of surface to be created
    DWORD               dwWidth;                // width of input surface
    union
    {
        LONG            lPitch;                 // distance to start of next line (return value only)
        DWORD           dwLinearSize;           // Formless late-allocated optimized surface size
    } DUMMYUNIONNAMEN(1);
    union
    {
        DWORD           dwBackBufferCount;      // number of back buffers requested
        DWORD           dwDepth;                // the depth if this is a volume texture 
    } DUMMYUNIONNAMEN(5);
    union
    {
        DWORD           dwMipMapCount;          // number of mip-map levels requestde
                                                // dwZBufferBitDepth removed, use ddpfPixelFormat one instead
        DWORD           dwRefreshRate;          // refresh rate (used when display mode is described)
        DWORD           dwSrcVBHandle;          // The source used in VB::Optimize
    } DUMMYUNIONNAMEN(2);
    DWORD               dwAlphaBitDepth;        // depth of alpha buffer requested
    DWORD               dwReserved;             // reserved
    LPVOID              lpSurface;              // pointer to the associated surface memory
    union
    {
        DDCOLORKEY      ddckCKDestOverlay;      // color key for destination overlay use
        DWORD           dwEmptyFaceColor;       // Physical color for empty cubemap faces
    } DUMMYUNIONNAMEN(3);
    DDCOLORKEY          ddckCKDestBlt;          // color key for destination blt use
    DDCOLORKEY          ddckCKSrcOverlay;       // color key for source overlay use
    DDCOLORKEY          ddckCKSrcBlt;           // color key for source blt use
    union
    {
        DDPIXELFORMAT   ddpfPixelFormat;        // pixel format description of the surface
        DWORD           dwFVF;                  // vertex format description of vertex buffers
    } DUMMYUNIONNAMEN(4);
    DDSCAPS2            ddsCaps;                // direct draw surface capabilities
    DWORD               dwTextureStage;         // stage in multitexture cascade
} DDSURFACEDESC2, FAR* LPDDSURFACEDESC2;

#define DDS_OFFSET						4+sizeof(DDSURFACEDESC2)		//place where the data starts -- should be 128

//reads a dds header
//returns one of the error values
//'compression_type' comes back as one of the DDS_DXTC* defines
int dds_read_header(char *filename, int *width, int *height, int *bpp, int *compression_type, int *levels);

//reads bitmap
//size of the data it stored in size
int dds_read_bitmap(char *filename, int *size, uint* data);

//returns a string from a DDS error code
const char* dds_error_string(int code);

extern int Texture_compression_enabled;

#endif //__DDS_H
