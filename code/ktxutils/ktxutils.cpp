#include "ktxutils.h"
#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
//KTX1 Spec definition https://registry.khronos.org/KTX/specs/1.0/ktxspec.v1.html
//ETC2 formats spec https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.inline.html#ETC2

static const uint8_t KTX_ID[12] = { 0xAB, 'K', 'T', 'X', ' ', '1', '1', 0xBB, '\r', '\n', 0x1A, '\n' };

// BPB ETC/EAC
uint32_t ktx_etc_block_bytes(const int internal_format)
{
	switch (internal_format) 
	{
		case GL_COMPRESSED_RGB8_ETC2:
		case GL_COMPRESSED_SRGB8_ETC2:
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_R11_EAC:
		case GL_COMPRESSED_SIGNED_R11_EAC:
			return 8;

		case GL_COMPRESSED_RGBA8_ETC2_EAC:
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		case GL_COMPRESSED_RG11_EAC:
		case GL_COMPRESSED_SIGNED_RG11_EAC:
			return 16;

		default:
			return 0;
	}
}


int ktx_map_ktx_format_to_gl_internal(const int ktx_format)
{
	switch (ktx_format) 
	{
		case KTX_ETC2_RGB:
			return GL_COMPRESSED_RGB8_ETC2;
		case KTX_ETC2_RGBA_EAC:
			return GL_COMPRESSED_RGBA8_ETC2_EAC;
		case KTX_EAC_R11:
			return GL_COMPRESSED_R11_EAC;
		case KTX_EAC_RG11:
			return GL_COMPRESSED_RG11_EAC;
		case KTX_ETC2_SRGB:
			return GL_COMPRESSED_SRGB8_ETC2;
		case KTX_ETC2_SRGBA_EAC:
			return GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
		case KTX_ETC2_RGB_A1:
			return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		case KTX_ETC2_SRGB_A1:
			return GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		case KTX_EAC_R11_SNORM:
			return GL_COMPRESSED_SIGNED_R11_EAC;
		case KTX_EAC_RG11_SNORM:
			return GL_COMPRESSED_SIGNED_RG11_EAC;

		default: return 0;
	}
}

int ktx_map_gl_internal_to_bm(const int internal_format)
{
	switch (internal_format) 
	{
		case GL_COMPRESSED_RGB8_ETC2:
		case GL_COMPRESSED_SRGB8_ETC2:
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
			return BM_TYPE_ETC2_RGB;

		case GL_COMPRESSED_RGBA8_ETC2_EAC:
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
			return BM_TYPE_ETC2_RGBA_EAC;

		case GL_COMPRESSED_R11_EAC:
		case GL_COMPRESSED_SIGNED_R11_EAC:
			return BM_TYPE_EAC_R11;

		case GL_COMPRESSED_RG11_EAC:
		case GL_COMPRESSED_SIGNED_RG11_EAC:
			return BM_TYPE_EAC_RG11;

		default:
			return BM_TYPE_NONE;
	}
}

int ktx1_read_header(const char* filename, CFILE* img_cfp, int* w, int* h, int* bpp, int* c_type, int* mip_levels, size_t* total_size)
{
	CFILE* cf;
	char real_name[MAX_FILENAME_LEN];
	if (img_cfp == nullptr) {
		// this better not happen.. ever
		Assert(filename != nullptr);

		// make sure there is an extension
		strcpy_s(real_name, filename);
		char* p = strchr(real_name, '.');
		if (p) {
			*p = 0;
		}
		strcat_s(real_name, ".ktx");

		// try to open the file
		cf = cfopen(real_name, "rb");

		// file not found
		if (cf == nullptr)
			return KTX1_ERROR_INVALID_FILENAME;
	} else {
		cf = img_cfp;
	}

	KTX1_Header hdr{};
	if (cfread(&hdr, 1, sizeof(hdr), cf) != sizeof(hdr)) {
		if (!img_cfp)
			cfclose(cf);
		mprintf(("ktx: read header error, parse header error. File: file='%s'\n", filename));
		return KTX1_ERROR_BAD_HEADER;
	}

	if (memcmp(hdr.identifier, KTX_ID, 12) != 0 || hdr.endianness != 0x04030201) {
		if (!img_cfp)
			cfclose(cf);
		mprintf(("ktx: read header error, parse header error. File: file='%s'\n", filename));
		return KTX1_ERROR_BAD_HEADER;
	}

	// Only 2D no arrays or cubemaps
	if (hdr.pixelHeight == 0 || hdr.pixelWidth == 0 || hdr.numberOfFaces > 1 || hdr.numberOfArrayElements > 0 ||
		(hdr.pixelDepth > 1)) {
		if (!img_cfp)
			cfclose(cf);
		mprintf(("ktx: read header error, only 2D textures are supported. File: file='%s'\n", filename));
		return KTX1_ERROR_UNSUPPORTED;
	}

	const bool isCompressed = (hdr.glType == 0 && hdr.glFormat == 0);
	if (!isCompressed) {
		//uncompressed
		if (!img_cfp)
			cfclose(cf);
		mprintf(("ktx: read header error, unsupported texture format on file: file='%s'\n", filename));
		return KTX1_ERROR_INVALID_FORMAT;
	}

	const uint32_t fmt = hdr.glInternalFormat;
	const uint32_t blockBytes = ktx_etc_block_bytes(fmt);
	const int bm_ct = ktx_map_gl_internal_to_bm(fmt);
	if (blockBytes == 0 || bm_ct == BM_TYPE_NONE) {
		if (!img_cfp)
			cfclose(cf);
		mprintf(("ktx: read header error, unsupported texture format on file: file='%s'\n", filename));
		return KTX1_ERROR_INVALID_FORMAT;
	}

	// skip metadata
	cfseek(cf, (int)hdr.bytesOfKeyValueData, CF_SEEK_CUR);

	// calculate max size
	uint32_t W = hdr.pixelWidth, H = hdr.pixelHeight;
	const uint32_t mips = std::max(1u, hdr.numberOfMipmapLevels);
	size_t total = 0;
	for (uint32_t level = 0; level < mips; ++level) {
		uint32_t imageSize = 0;
		cfread(&imageSize, 1, sizeof(imageSize), cf);
		total += imageSize;
		cfseek(cf, (int)imageSize, CF_SEEK_CUR);
		uint32_t pad = ((-(int)imageSize) & 3);
		if (pad)
			cfseek(cf, (int)pad, CF_SEEK_CUR);

		W = std::max(1u, W >> 1);
		H = std::max(1u, H >> 1);
	}

	if (w)
		*w = (int)hdr.pixelWidth;
	if (h)
		*h = (int)hdr.pixelHeight;
	if (c_type)
		*c_type = bm_ct;
	if (bpp)
	{
		switch (bm_ct) 
		{
			case BM_TYPE_ETC2_RGB:
				*bpp = 24;
				break;
			case BM_TYPE_ETC2_RGBA_EAC:
				*bpp = 32;
				break;
			case BM_TYPE_EAC_R11:
				*bpp = 8;
				break;
			case BM_TYPE_EAC_RG11:
				*bpp = 16;
				break;
			default:
				*bpp = 0;
				break;
		}
	}
	if (mip_levels)
		*mip_levels = (int)mips;
	if (total_size)
		*total_size = total;

	if (!img_cfp)
		cfclose(cf);
	return KTX1_ERROR_NONE;
}

int ktx1_read_bitmap(const char* filename, ubyte* dst, ubyte* out_bpp)
{
	CFILE* cf;
	char real_name[MAX_FILENAME_LEN];
	// this better not happen.. ever
	Assert(filename != nullptr);

	// make sure there is an extension
	strcpy_s(real_name, filename);
	char* p = strchr(real_name, '.');
	if (p) {
		*p = 0;
	}
	strcat_s(real_name, ".ktx");

	// try to open the file
	cf = cfopen(real_name, "rb");

	// file not found
	if (cf == nullptr)
		return KTX1_ERROR_INVALID_FILENAME;

	KTX1_Header hdr{};
	int rc = KTX1_ERROR_NONE;
	if (cfread(&hdr, 1, sizeof(hdr), cf) != sizeof(hdr) || memcmp(hdr.identifier, KTX_ID, 12) != 0) 
	{
		rc = KTX1_ERROR_BAD_HEADER;
	}
	if (hdr.glType != 0 || hdr.glFormat != 0) 
	{
		rc = KTX1_ERROR_INVALID_FORMAT;
	}

	if (rc == KTX1_ERROR_NONE) 
	{
		cfseek(cf, (int)hdr.bytesOfKeyValueData, CF_SEEK_CUR);

		const uint32_t mips = std::max(1u, hdr.numberOfMipmapLevels);
		size_t offset = 0;
		for (uint32_t level = 0; level < mips; ++level) 
		{
			uint32_t image_size = 0;
			cfread(&image_size, 1, sizeof(image_size), cf);
			cfread(dst + offset, 1, image_size, cf);
			offset += image_size;
			uint32_t pad = ((-(int)image_size) & 3); // 4 byte padding
			if (pad)
				cfseek(cf, (int)pad, CF_SEEK_CUR);
		}

		if (out_bpp)
			*out_bpp = 0;
	}

	cfclose(cf);
	return rc;
}