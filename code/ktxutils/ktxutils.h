#pragma once
#include "globalincs/pstypes.h"
#include "cfile/cfile.h"
#ifdef WITH_OPENGL
#include <glad/glad.h>
#endif

// ETC2 types
#define KTX_ETC2_RGB		101
#define KTX_ETC2_RGBA_EAC	102
#define KTX_EAC_R11			103
#define KTX_EAC_RG11		104
#define KTX_ETC2_SRGB		105
#define KTX_ETC2_SRGBA_EAC	106
#define KTX_ETC2_RGB_A1		107
#define KTX_ETC2_SRGB_A1	108
#define KTX_EAC_R11_SNORM	109
#define KTX_EAC_RG11_SNORM	110

// GLenum definitions, ETC2 is guarranted on ES 3.2, but in desktop GL it is supported from 4.3
// glad loader is GL 3.2 and these definitions are not there. But these will work if the GPU is GL 4.3
// https://www.sidefx.com/docs/hdk/glcorearb_8h_source.html
#ifndef GL_COMPRESSED_RGB8_ETC2
#define GL_COMPRESSED_RGB8_ETC2 0x9274
#endif
#ifndef GL_COMPRESSED_SRGB8_ETC2
#define GL_COMPRESSED_SRGB8_ETC2 0x9275
#endif
#ifndef GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#endif
#ifndef GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
#define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#endif
#ifndef GL_COMPRESSED_R11_EAC
#define GL_COMPRESSED_R11_EAC 0x9270
#endif
#ifndef GL_COMPRESSED_SIGNED_R11_EAC
#define GL_COMPRESSED_SIGNED_R11_EAC 0x9271
#endif
#ifndef GL_COMPRESSED_RG11_EAC
#define GL_COMPRESSED_RG11_EAC 0x9272
#endif
#ifndef GL_COMPRESSED_SIGNED_RG11_EAC
#define GL_COMPRESSED_SIGNED_RG11_EAC 0x9273
#endif

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

int ktx1_read_header(const char* filename, CFILE* img_cfp, int* w, int* h, int* bpp, int* c_type, int* mip_levels, size_t* total_size);

int ktx1_read_bitmap(const char* filename, ubyte* dst, ubyte* out_bpp, int cf_type);

int ktx_map_ktx_type_to_gl_internal(unsigned int ktx_format);

uint32_t ktx_etc_block_bytes(GLenum internal_format);