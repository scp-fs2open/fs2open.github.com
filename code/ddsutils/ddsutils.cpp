
#include "ddsutils/ddsutils.h"
#include "cfile/cfile.h"
#include "osapi/osregistry.h"
#ifdef USE_OPENGL_ES
#include "graphics/opengl/es_compatibility.h"
#endif

#ifdef WITH_OPENGL
#include <glad/glad.h>
#else
static constexpr int GLAD_GL_EXT_texture_compression_s3tc = 0;
static constexpr int GLAD_GL_ARB_texture_compression_bptc = 0;
#endif

#define BCDEC_IMPLEMENTATION 1
PUSH_SUPPRESS_WARNINGS
#include "ddsutils/bcdec.h"
POP_SUPPRESS_WARNINGS

/*	Currently supported formats:
 *		DXT1a	(compressed)
 *		DXT1c	(compressed)
 *		DXT3	(compressed)
 *		DXT5	(compressed)
 *		uncompressed 1555	(16-bit, 1-bit being alpha)
 *		uncompressed 8888	(32-bit)
 *		uncompressed 888	(24-bit, no alpha)
 *		uncompressed R8		(8-bit, Red channel as luminance)
 */


// power of two check, to verify that a given DDS will always be usable since
// some may not be power-of-2
static inline int is_power_of_two(int w, int h)
{
	return ( (w && !(w & (w-1))) && (h && !(h & (h-1))) );
}

static bool valid_dx10_format(const DDS_HEADER_DXT10 &header)
{
	switch (header.dxgiFormat) {
		case DXGI_FORMAT::DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB:
			return true;

		default:
			return false;
	}
}

static bool conversion_needed(const DDS_HEADER &dds_header)
{
	if ((dds_header.ddspf.dwFlags & DDPF_FOURCC) != DDPF_FOURCC) {
		return false;
	}

	switch (dds_header.ddspf.dwFourCC) {
		case FOURCC_DXT1:
		case FOURCC_DXT3:
		case FOURCC_DXT5:
			return !GLAD_GL_EXT_texture_compression_s3tc;

		case FOURCC_DX10:
			// anything other than BC7 will end up invalid
			return !GLAD_GL_ARB_texture_compression_bptc;

		default:
			break;
	}

	// anything not specifically listed should end up invalid
	return false;
}

// Auto-determine max texture size based on avail device ram for software decompression
// 128 for <4GB, 256 for < 8GB, 512 for < 12GB and finally 1024
static size_t calculate_resize_max_size()
{
	size_t size = 1024;
	size_t ram_mib = SDL_GetSystemRAM();

	if (ram_mib > 0)
	{
		if (ram_mib < 4 * 1024) // < 4GB
			return 128;
		if (ram_mib < 8 * 1024) // < 8GB
			return 256;
		if (ram_mib < 12 * 1024) // < 12GB
			return 512;
	}

	return size;
}

// Memory usage for uncompressed textures is quite high. Some MVP assets can
// require well over a GB of VRAM for a single ship after conversion. To help
// alleviate this we need to resize those textures where possible. At 1024x1024
// the memory requirement is reduced to about 36 MB per ship with a full set
// of textures.
//
// NOTE: modifies header!!
//
// returns: number of mipmap levels to skip
static uint conversion_resize(DDS_HEADER &dds_header)
{
	const size_t MAX_SIZE = calculate_resize_max_size();
	uint width, height, depth, offset = 0;

	if (dds_header.dwMipMapCount <= 1) {
		return 0;
	}

	width = dds_header.dwWidth;
	height = dds_header.dwHeight;
	depth = dds_header.dwDepth;

	// drop levels until we get to an appropriate size, but make sure we have
	// at least 1 mipmap level remaining at the end (in case there's not a full chain)
	while (((width > MAX_SIZE) || (height > MAX_SIZE)) && (offset < dds_header.dwMipMapCount-1)) {
		// this shouldn't happen, but catch the obscure case (like 8192x4)
		if ((width <= 4) || (height <= 4)) {
			break;
		}

		width >>= 1;
		height >>= 1;
		depth >>= 1;

		++offset;
	}

	// no change needed
	if ( !offset ) {
		return offset;
	}

	// update header with new values
	dds_header.dwWidth = std::max(1U, width);
	dds_header.dwHeight = std::max(1U, height);
	dds_header.dwDepth = std::max(1U, depth);

	return offset;
}

static int _dds_read_header(CFILE *ddsfile, DDS_HEADER &dds_header, DDS_HEADER_DXT10 *dds_header_dx10 = nullptr)
{
	DDS_HEADER_DXT10 dx10_header;

	// read the code
	int code = cfread_int(ddsfile);

	// check it
	if (code != DDS_FILECODE) {
		return DDS_ERROR_BAD_HEADER;
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
	cfseek(ddsfile, 11*4, CF_SEEK_CUR);

	dds_header.ddspf.dwSize			= cfread_uint(ddsfile);
	dds_header.ddspf.dwFlags		= cfread_uint(ddsfile);
	dds_header.ddspf.dwFourCC		= cfread_uint(ddsfile);
	dds_header.ddspf.dwRGBBitCount	= cfread_uint(ddsfile);
	dds_header.ddspf.dwRBitMask		= cfread_uint(ddsfile);
	dds_header.ddspf.dwGBitMask		= cfread_uint(ddsfile);
	dds_header.ddspf.dwBBitMask		= cfread_uint(ddsfile);
	dds_header.ddspf.dwABitMask		= cfread_uint(ddsfile);

	dds_header.dwCaps		= cfread_uint(ddsfile);
	dds_header.dwCaps2		= cfread_uint(ddsfile);

	// Unused but need them checked for later
	dds_header.dwCaps3		= cfread_uint(ddsfile);
	dds_header.dwCaps4		= cfread_uint(ddsfile);
	dds_header.dwReserved2	= cfread_uint(ddsfile);

	if (dds_header.ddspf.dwFourCC == FOURCC_DX10) {
		dx10_header.dxgiFormat			= static_cast<DXGI_FORMAT>(cfread_uint(ddsfile));
		dx10_header.resourceDimension	= static_cast<D3D10_RESOURCE_DIMENSION>(cfread_uint(ddsfile));
		dx10_header.miscFlag			= cfread_uint(ddsfile);
		dx10_header.arraySize			= cfread_uint(ddsfile);
		dx10_header.miscFlags2			= cfread_uint(ddsfile);

		if (dds_header_dx10) {
			memcpy(dds_header_dx10, &dx10_header, sizeof(DDS_HEADER_DXT10));
		}
	}

	// sanitize values
	dds_header.dwDepth = std::max(1U, dds_header.dwDepth);
	dds_header.dwMipMapCount = std::max(1U, dds_header.dwMipMapCount);

#ifndef NDEBUG
	if (dds_header.dwDepth > 1) {
		Assertion(dds_header.dwFlags & DDSD_DEPTH, "ERROR: Volume texture missing proper flags!!");
	}
#endif

	// set pitch manually (per MS docs)
	dds_header.dwFlags &= ~DDSD_LINEARSIZE;
	dds_header.dwFlags |= DDSD_PITCH;

	if (dds_header.ddspf.dwFlags & DDPF_FOURCC) {
		int block_size = (dds_header.ddspf.dwFourCC == FOURCC_DXT1) ? 8 : 16;
		dds_header.dwPitchOrLinearSize = std::max(1U, (dds_header.dwWidth+3)/4) * block_size;
	} else {
		// this isn't technically correct, but it's only wrong for formats we don't support
		dds_header.dwPitchOrLinearSize = (dds_header.dwWidth * dds_header.ddspf.dwRGBBitCount + 7) / 8;
	}

	return DDS_ERROR_NONE;
}

static size_t compute_dds_size(const DDS_HEADER &dds_header, bool converting = false)
{
	const uint block_sz = 4;
	uint d_width = 0, d_height = 0, d_depth = 0;
	size_t d_size = 0;

	for (uint i = 0; i < dds_header.dwMipMapCount; i++) {
		d_width = std::max(1U, dds_header.dwWidth >> i);
		d_height = std::max(1U, dds_header.dwHeight >> i);
		d_depth = std::max(1U, dds_header.dwDepth >> i);

		// When converting we need to pad a bit to compensate for the decompression
		// size on smaller mipmap levels. We need to ensure there is always enough
		// room to decode an entire 4x4 block in rgba space
		if (converting) {
			auto sz = std::min(d_width, d_height);

			if (sz < block_sz) {
				d_size += ((block_sz * block_sz) - (sz * sz)) * d_depth * 4;
			}
		}

		if (dds_header.ddspf.dwFlags & DDPF_FOURCC) {
			// size of data block (4x4)
			d_size += ((d_width + 3) / 4) * ((d_height + 3) / 4) * d_depth * ((dds_header.ddspf.dwFourCC == FOURCC_DXT1) ? 8 : 16);
		} else {
			d_size += d_width * d_height * d_depth * (dds_header.ddspf.dwRGBBitCount / 8);
		}
	}

	// assumes valid cubemap (should be checked elsewhere)
	if (dds_header.dwCaps2 & DDSCAPS2_CUBEMAP) {
		// the previously computed size is just per face, mult by 6 for full size
		d_size *= 6;
	}

	Assertion(d_size > 0, "ERROR: DDS size computed to 0!!");

	return d_size;
}

static int get_bit_count(const DDS_HEADER &dds_header)
{
	if (dds_header.ddspf.dwFlags & DDPF_RGB) {
		return dds_header.ddspf.dwRGBBitCount;
	} else if (dds_header.ddspf.dwFlags & DDPF_FOURCC) {
		if (dds_header.ddspf.dwFourCC == FOURCC_DXT1) {
			return 24;
		} else {
			return 32;
		}
	}

	return 0;
}

int dds_read_header(const char *filename, CFILE *img_cfp, int *width, int *height, int *bpp, int *compression_type, int *levels, size_t *size)
{
	DDS_HEADER dds_header;
	DDS_HEADER_DXT10 dds_header_dx10;
	CFILE *ddsfile;
	char real_name[MAX_FILENAME_LEN];
	int retval = DDS_ERROR_NONE;
	int ct = DDS_UNCOMPRESSED;
	int is_cubemap = 0;
	bool convert = false;


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

	retval = _dds_read_header(ddsfile, dds_header, &dds_header_dx10);

	if (retval != DDS_ERROR_NONE) {
		goto Done;
	}

	// check for valid cubemap (we require all 6 faces)
	if (dds_header.dwCaps2 & DDSCAPS2_CUBEMAP) {
		if ( !(dds_header.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES) ) {
			retval = DDS_ERROR_CUBEMAP_FACES;
			ct = DDS_DXT_INVALID;
			goto Done;
		}

		// it's a valid cubemap, probably
		is_cubemap = 1;
	}

	// determine and verify the type of data for internal use
	if (dds_header.ddspf.dwFlags & DDPF_FOURCC) {
		switch (dds_header.ddspf.dwFourCC) {
			case FOURCC_DXT1:
				ct = (is_cubemap) ? DDS_CUBEMAP_DXT1 : DDS_DXT1;
				break;

			case FOURCC_DXT3:
				ct = (is_cubemap) ? DDS_CUBEMAP_DXT3 : DDS_DXT3;
				break;

			case FOURCC_DXT5:
				ct = (is_cubemap) ? DDS_CUBEMAP_DXT5 : DDS_DXT5;
				break;

			// dxt2 and dxt4 aren't supported
			case FOURCC_DXT2:
			case FOURCC_DXT4:
				retval = DDS_ERROR_UNSUPPORTED;
				ct = DDS_DXT_INVALID;
				goto Done;
				break;

			case FOURCC_DX10:
				if (valid_dx10_format(dds_header_dx10)) {
					// Currently only BC7 is supported
					ct = DDS_BC7;
				} else {
					retval = DDS_ERROR_UNSUPPORTED;
					ct = DDS_DXT_INVALID;
					goto Done;
				}
				break;

			// none of the above
			default:
				retval = DDS_ERROR_INVALID_FORMAT;
				ct = DDS_DXT_INVALID;
				goto Done;
				break;
		}
	} else if (dds_header.ddspf.dwFlags & DDPF_RGB) {
		ct = (is_cubemap) ? DDS_CUBEMAP_UNCOMPRESSED : DDS_UNCOMPRESSED;
	} else {
		// it's not a readable format
		retval = DDS_ERROR_INVALID_FORMAT;
		goto Done;
	}

	// maybe do conversion if format not supported
	convert = conversion_needed(dds_header);

	if (convert) {
		// switch to uncompressed format and reset vars
		dds_header.ddspf.dwFlags &= ~DDPF_FOURCC;
		dds_header.ddspf.dwFlags |= DDPF_RGB;

		dds_header.ddspf.dwRGBBitCount = 32;

		// NOTE: modifies header and returns offset for mipmap count
		dds_header.dwMipMapCount -= conversion_resize(dds_header);
		dds_header.dwMipMapCount = std::max(1U, dds_header.dwMipMapCount);

		ct = (is_cubemap) ? DDS_CUBEMAP_UNCOMPRESSED : DDS_UNCOMPRESSED;
	}

	// we don't support compressed, non-power-of-2 images
	if ( (ct != DDS_UNCOMPRESSED) && (!is_power_of_two(dds_header.dwWidth, dds_header.dwHeight)) ) {
		retval = DDS_ERROR_NON_POWER_OF_2;
		goto Done;
	}

	// stuff important info
	if (size)
		*size = compute_dds_size(dds_header, convert);

	if (bpp)
		*bpp = get_bit_count(dds_header);

	if (compression_type)
		*compression_type = ct;

	if (width)
		*width = dds_header.dwWidth;

	if (height)
		*height = dds_header.dwHeight;

	if (levels)
		*levels = dds_header.dwMipMapCount;

Done:
	if (img_cfp == NULL) {
		// close file and return
		cfclose(ddsfile);
		ddsfile = NULL;
	}

	return retval;
}

static void (*decompress_dds)(const void *in, void *out, int pitch) = nullptr;
static uint32_t BLOCK_SIZE = 0;

//reads pixel info from a dds file
int dds_read_bitmap(const char *filename, ubyte *data, ubyte *bpp, int cf_type)
{
	int retval;
	size_t size = 0;
	CFILE *cfp;
	char real_name[MAX_FILENAME_LEN];
	DDS_HEADER dds_header;

	// this better not happen.. ever
	Assert(filename != NULL);

	// make sure there is an extension
	strcpy_s(real_name, filename);
	char *p = strchr(real_name, '.');
	if (p) { *p = 0; }
	strcat_s(real_name, ".dds");

	// open it up and go to the data section
	cfp = cfopen(real_name, "rb", cf_type);

	// just in case
	if (cfp == nullptr)
		return DDS_ERROR_INVALID_FILENAME;

	// read the header -- if its at this stage, it should be legal.
	retval = _dds_read_header(cfp, dds_header);
	Assert(retval == DDS_ERROR_NONE);

	// this really shouldn't be needed but better safe than sorry
	if (retval != DDS_ERROR_NONE) {
		cfclose(cfp);
		return retval;
	}

	cfseek(cfp, (dds_header.ddspf.dwFourCC == FOURCC_DX10) ? DX10_OFFSET : DDS_OFFSET, CF_SEEK_SET);

	size = compute_dds_size(dds_header);	// don't add padding on this one!!

	// read in the data
	if ( !conversion_needed(dds_header) ) {
		cfread(data, 1, (int)size, cfp);
	} else {
		// Compression format not supported, convert to BGRA
		ubyte *comp_data = (ubyte*)vm_malloc(size);

		cfread(comp_data, 1, (int)size, cfp);

		ubyte *src = comp_data;
		ubyte *dst = data;

		uint d_width, d_height, d_depth;
		size_t data_offset = 0;

		// NOTE: this alters the width, height, and depth values in the header,
		//       so we have to jump through some hoops to get the proper values
		const uint mipmap_offset = conversion_resize(dds_header);

		const int num_faces = (dds_header.dwCaps2 & DDSCAPS2_CUBEMAP) ? 6 : 1;
		const bool has_depth = (dds_header.dwFlags & DDSD_DEPTH) == DDSD_DEPTH;

		switch (dds_header.ddspf.dwFourCC) {
			case FOURCC_DX10:
				decompress_dds = bcdec_bc7;
				BLOCK_SIZE = BCDEC_BC7_BLOCK_SIZE;
				break;
			case FOURCC_DXT5:
				decompress_dds = bcdec_bc3;
				BLOCK_SIZE = BCDEC_BC3_BLOCK_SIZE;
				break;
			case FOURCC_DXT1:
				decompress_dds = bcdec_bc1;
				BLOCK_SIZE = BCDEC_BC1_BLOCK_SIZE;
				break;
			case FOURCC_DXT3:
				decompress_dds = bcdec_bc2;
				BLOCK_SIZE = BCDEC_BC2_BLOCK_SIZE;
				break;
			default:
				Error(LOCATION, "Invalid FourCC (%d) for DDS decompression!", dds_header.ddspf.dwFourCC);
				break;
		}

		for (int f = 0; f < num_faces; ++f) {
			// if we resized then skip over all of that data (altering values to match pre-resize)
			for (uint x = 0; x < mipmap_offset; ++x) {
				d_width = std::max(1U, dds_header.dwWidth << (mipmap_offset - x));
				d_height = std::max(1U, dds_header.dwHeight << (mipmap_offset - x));
				d_depth = has_depth ? std::max(1U, dds_header.dwDepth << (mipmap_offset - x)) : 1U;

				src += ((d_width + 3) / 4) * ((d_height + 3) / 4) * d_depth * BLOCK_SIZE;
			}

			for (uint m = mipmap_offset; m < dds_header.dwMipMapCount; ++m) {
				// width/height/depth at post-resize mipmap sizes, so compensate here
				d_width = std::max(1U, dds_header.dwWidth >> (m - mipmap_offset));
				d_height = std::max(1U, dds_header.dwHeight >> (m - mipmap_offset));
				d_depth = std::max(1U, dds_header.dwDepth >> (m - mipmap_offset));

				for (uint d = 0; d < d_depth; ++d) {
					auto depth_offset = d * d_width * d_height * 4;

					for (uint i = 0; i < d_height; i += 4) {
						for (uint j = 0; j < d_width; j += 4) {
							dst = data + data_offset + depth_offset + ((i * d_width + j) * 4);

							decompress_dds(src, dst, d_width * 4);
							src += BLOCK_SIZE;
						}
					}
				}

				// we need it in bgra, XOR to the rescue
				for (size_t x = data_offset; x < (data_offset + (d_width * d_height * d_depth * 4)); x += 4) {
					data[x] ^= data[x+2], data[x+2] ^= data[x], data[x] ^= data[x+2];
				}

				// bump data offset to next layer
				data_offset += (d_width * d_height * d_depth * 4);
			}
		}

		vm_free(comp_data);
		comp_data = nullptr;

		// switch to uncompressed format and reset vars (needed below to get correct bit count)
		dds_header.ddspf.dwFlags &= ~DDPF_FOURCC;
		dds_header.ddspf.dwFlags |= DDPF_RGB;
		dds_header.ddspf.dwRGBBitCount = 32;
	}

	if (bpp)
		*bpp = (ubyte)get_bit_count(dds_header);

	// we look done here
	cfclose(cfp);

	return DDS_ERROR_NONE;
}

// save some image data as a DDS image
// NOTE: we only support, uncompressed, 24-bit RGB and 32-bit RGBA images here!!
void dds_save_image(int width, int height, int bpp, int num_mipmaps, ubyte *data, int cubemap, const char *filename)
{
	DDS_HEADER dds_header;
	char real_filename[MAX_FILENAME_LEN];

	if (data == NULL) {
		Int3();
		return;
	}

	// header size check
	if (sizeof(DDS_HEADER) != 124) {
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

	CFILE *image = cfopen( real_filename, "wb", CF_TYPE_CACHE );

	if (image == NULL) {
		mprintf(("Unable to open DDS image for saving!!\n"));
		return;
	}

	if (num_mipmaps < 1)
		num_mipmaps = 1;

	// we have a filename and file handle, so now lets create our DDS header...
	memset( &dds_header, 0, sizeof(DDS_HEADER) );

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

	dds_header.ddspf.dwSize				= 32;
	dds_header.ddspf.dwFlags				= pixel_flags;
	dds_header.ddspf.dwFourCC				= 0;
	dds_header.ddspf.dwRGBBitCount		= bpp;
	dds_header.ddspf.dwRBitMask			= 0x00ff0000;
	dds_header.ddspf.dwGBitMask			= 0x0000ff00;
	dds_header.ddspf.dwBBitMask			= 0x000000ff;
	dds_header.ddspf.dwABitMask	= (bpp == 32) ? 0xff000000 : 0x00000000;

	dds_header.dwCaps		= caps1;
	dds_header.dwCaps2		= caps2;

	// now save it all...
	uint dds_id = DDS_FILECODE;
	cfwrite(&dds_id, 1, 4, image);
	cfwrite(&dds_header, 1, sizeof(DDS_HEADER), image);

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
