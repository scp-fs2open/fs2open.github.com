#pragma once
#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

// ETC2 types
enum KTX_Format {
	KTX_ETC2_RGB = 100,
	KTX_ETC2_RGBA_EAC,
	KTX_ETC2_SRGB,
	KTX_ETC2_SRGBA_EAC,
	KTX_ETC2_RGB_A1,
	KTX_ETC2_SRGB_A1
};

enum KTX1_Error {
	KTX1_ERROR_NONE = 0,
	KTX1_ERROR_INVALID_FILENAME,
	KTX1_ERROR_BAD_HEADER,
	KTX1_ERROR_UNSUPPORTED,
	KTX1_ERROR_INVALID_FORMAT
};

struct KTX1_Header {
	uint8_t identifier[12];
	uint32_t endianness; // 0x04030201 little
	uint32_t glType;
	uint32_t glTypeSize;
	uint32_t glFormat;
	uint32_t glInternalFormat;
	uint32_t glBaseInternalFormat;
	uint32_t pixelWidth, pixelHeight, pixelDepth;
	uint32_t numberOfArrayElements;
	uint32_t numberOfFaces;
	uint32_t numberOfMipmapLevels;
	uint32_t bytesOfKeyValueData;
};

//reads a dds header and returns a KTX1_Error
int ktx1_read_header(const char* filename, CFILE* img_cfp, int* w, int* h, int* bpp, int* c_type, int* mip_levels, size_t* total_size);

//reads bitmap and returns a KTX1_Error
int ktx1_read_bitmap(const char* filename, ubyte* dst, ubyte* out_bpp);

//Get the GL enum type for this KTX format type, returns 0 if type is invalid.
int ktx_map_ktx_format_to_gl_internal(const int ktx_format);

//Get ktx block size (bytes per block) from GL internat format enum, returns 0 if internal format is invalid
uint32_t ktx_etc_block_size(const int internal_format);