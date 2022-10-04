#include <cstdio>
#include <cstring>

#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "pngutils/pngutils.h"
#include "utils/base64.h"

struct png_status {
	CFILE* cfp = nullptr;
	const char* filename = nullptr;
	bool reading_header = false;
	bool writing = false;
};

/*
 * @brief copy/pasted from libpng
 */
static void png_scp_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;

	if (png_ptr == NULL)
		return;

	png_status* status = reinterpret_cast<png_status*>(png_get_io_ptr(png_ptr));

	/* fread() returns 0 on error, so it is OK to store this in a png_size_t
	* instead of an int, which is what fread() actually returns.
	*/
	check = (png_size_t)cfread(data, (png_size_t)1, (int)length, status->cfp);
	if (check != length)
		png_error(png_ptr, "Read Error");
}

static void png_scp_write_data(png_structp  png_ptr, png_bytep data, png_size_t length) {
	auto status = static_cast<png_status*>(png_get_io_ptr(png_ptr));

	Assertion(status != nullptr, "Invalid file pointer in PNG writing function.");

	auto check = (png_size_t)cfwrite(data, (png_size_t)1, (int)length, status->cfp);
	if (check != length)
		png_error(png_ptr, "Write Error");
}
static void png_scp_flush(png_structp png_ptr) {
	auto status = static_cast<png_status*>(png_get_io_ptr(png_ptr));

	Assertion(status != nullptr, "Invalid file pointer in PNG writing function.");

	cflush(status->cfp);
}

static png_voidp png_malloc_fn(png_structp, png_size_t size)
{
	return vm_malloc(size);
}

static void png_free_fn(png_structp, png_voidp ptr)
{
	vm_free(ptr);
}

static void png_error_fn(png_structp png_ptr, png_const_charp message)
{
	png_status* status = reinterpret_cast<png_status*>(png_get_error_ptr(png_ptr));

	if (status->writing) {
		mprintf(("PNG error while writing %s: %s\n", status->filename, message));
	} else {
		mprintf(("PNG error while reading %s of %s: %s\n", status->reading_header ? "header" : "pixel data", status->filename, message));
	}

	longjmp(png_jmpbuf(png_ptr), 1);
}

static void png_warning_fn(png_structp png_ptr, png_const_charp message)
{
	png_status* status = reinterpret_cast<png_status*>(png_get_error_ptr(png_ptr));

	if (status->writing) {
		nprintf(("PNG warning", "PNG warning while writing %s: %s\n", status->filename, message));
	} else {
		nprintf(("PNG warning", "PNG warning while reading %s of %s: %s\n", status->reading_header ? "header" : "pixel data", status->filename, message));
	}
}


static int png_read_header_data(int* w, int* h, int* bpp, png_voidp status, png_error_ptr error, png_error_ptr warning, png_rw_ptr readFunc, std::function<void()> onClose)
{
	png_infop info_ptr;
	png_structp png_ptr;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also supply the
	* the compiler header file version, so that we know if the application
	* was compiled with a compatible version of the library.  REQUIRED
	*/
	png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, status, error, warning, status, png_malloc_fn, png_free_fn);

	if (png_ptr == nullptr)
	{
		mprintf(("png_read_header: error creating read struct\n"));
		onClose();
		return PNG_ERROR_READING;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		mprintf(("png_read_header: error creating info struct\n"));
		onClose();
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		return PNG_ERROR_READING;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		mprintf(("png_read_header: something went wrong\n"));
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		onClose();
		/* If we get here, we had a problem reading the file */
		return PNG_ERROR_READING;
	}

	png_set_read_fn(png_ptr, status, readFunc);

	png_read_info(png_ptr, info_ptr);

	if (w) *w = png_get_image_width(png_ptr, info_ptr);
	if (h) *h = png_get_image_height(png_ptr, info_ptr);
	// this turns out to be near useless, but meh
	if (bpp) {
		// bit depth can also be 16 bit we tell libpng to reduce that to 8 bits so we also need to tell our caller about that
		auto bits = std::min(8, (int)png_get_bit_depth(png_ptr, info_ptr));
		*bpp = (png_get_channels(png_ptr, info_ptr) * bits);
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

	return PNG_ERROR_NONE;
}

/*
 * @brief Reads header information from the PNG file into the bitmap pointer
 *
 * @param [in]  filename  name of the PNG bitmap file
 * @param [out] w         width of the bitmap
 * @param [out] h         height of the bitmap
 * @param [out] bpp       bits per pixel of the bitmap
 * @param [out] palette
 *
 * @retval PNG_ERROR_NONE if successful, otherwise error code
 */
int png_read_header(const char *real_filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte * /*palette*/)
{
	char filename[MAX_FILENAME_LEN];

	png_status status;
	status.reading_header = true;
	status.filename = real_filename;

	if (img_cfp == nullptr) {
		strcpy_s( filename, real_filename );

		char *p = strchr( filename, '.' );

		if ( p )
			*p = 0;
		strcat_s( filename, ".png" );

		status.cfp = cfopen( filename , "rb" );

		if ( !status.cfp ) {
			return PNG_ERROR_READING;
		}
	} else {
		status.cfp = img_cfp;
	}

	Assert( status.cfp != nullptr);

	if (status.cfp == nullptr)
		return PNG_ERROR_READING;

	int result = png_read_header_data(w, h, bpp, &status, png_error_fn, png_warning_fn, png_scp_read_data, [&status]() {cfclose(status.cfp); });

	if (img_cfp == nullptr) {
		cfclose(status.cfp);
		status.cfp = nullptr;
	}

	return result;
}

int png_read_header(const SCP_string& b64, int* w, int* h, int* bpp, ubyte* /*palette*/)
{
	struct b64_dec_buffer {
		SCP_string decoded;
		size_t pos;
	} buffer{ base64_decode(b64), 0 };

	return png_read_header_data(w, h, bpp, &buffer,
		[](png_structp png_ptr, png_const_charp msg) {
			mprintf(("PNG error while parsing base64 PNG: %s\n", msg));
			longjmp(png_jmpbuf(png_ptr), 1);
		},
		[](png_structp, png_const_charp msg) {
			mprintf(("PNG warning while parsing base64 PNG: %s\n", msg));
		},
			[](png_structp png_ptr, png_bytep datap, png_size_t length) {
			b64_dec_buffer& buf = *static_cast<b64_dec_buffer*>(png_get_io_ptr(png_ptr));

			for (size_t finalPos = buf.pos + length; buf.pos < finalPos; buf.pos++) {
				*(datap++) = buf.decoded.at(buf.pos);
			}
		}, []() {});
}

/*
 * Loads a PNG image
 *
 * @param [out] image_data     allocated storage for the bitmap
 * @param [in]  bpp
 * @param [in]  dest_size
 *
 * @retval true if succesful, false otherwise
 */
static int png_read_bitmap_data(ubyte *image_data, int *bpp, png_voidp status, png_error_ptr error, png_error_ptr warning, png_rw_ptr readFunc, std::function<void()> onClose)
{
	png_infop info_ptr;
	png_structp png_ptr;
	png_bytepp row_pointers;
	unsigned int i;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also supply the
	* the compiler header file version, so that we know if the application
	* was compiled with a compatible version of the library.  REQUIRED
	*/
	png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, status, error, warning, nullptr, nullptr, nullptr);

	if (png_ptr == nullptr)
	{
		mprintf(("png_read_bitmap: png_ptr went wrong\n"));
		onClose();
		return PNG_ERROR_READING;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == nullptr)
	{
		mprintf(("png_read_bitmap: info_ptr went wrong\n"));
		onClose();
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		return PNG_ERROR_READING;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		mprintf(("png_read_bitmap: something went wrong\n"));
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		onClose();
		/* If we get here, we had a problem reading the file */
		return PNG_ERROR_READING;
	}

	png_set_read_fn(png_ptr, status, readFunc);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16, NULL);
	auto len = png_get_rowbytes(png_ptr, info_ptr);

	row_pointers = png_get_rows(png_ptr, info_ptr);

	if(bpp)
		*bpp = (ubyte)(len / png_get_image_width(png_ptr, info_ptr)) << 3;

	//copy row data to image
	unsigned int height = png_get_image_height(png_ptr, info_ptr);
	for (i = 0; i < height; i++) {
		memcpy(&image_data[i * len], row_pointers[i], len);
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	onClose();

	return PNG_ERROR_NONE;
}

/*
 * Loads a PNG image
 *
 * @param [in]  real_filename  name of the png file to load
 * @param [out] image_data     allocated storage for the bitmap
 * @param [in]  bpp
 * @param [in]  dest_size
 * @param [in]  cf_type
 *
 * @retval true if succesful, false otherwise
 */
int png_read_bitmap(const char* real_filename, ubyte* image_data, int* bpp, int  /*dest_size*/, int cf_type)
{
	char filename[MAX_FILENAME_LEN];

	png_status status;
	status.reading_header = false;
	status.filename = real_filename;

	strcpy_s(filename, real_filename);
	char* p = strchr(filename, '.');
	if (p) *p = 0;
	strcat_s(filename, ".png");

	status.cfp = cfopen(filename, "rb", CFILE_NORMAL, cf_type);

	if (status.cfp == NULL)
		return PNG_ERROR_READING;

	return png_read_bitmap_data(image_data, bpp, &status, png_error_fn, png_warning_fn, png_scp_read_data, [&status]() {cfclose(status.cfp); });
}

int png_read_bitmap(const SCP_string& b64, ubyte* image_data, int* bpp)
{
	struct b64_dec_buffer {
		SCP_string decoded;
		size_t pos;
	} buffer{ base64_decode(b64), 0 };

	return png_read_bitmap_data(image_data, bpp, &buffer,
		[](png_structp png_ptr, png_const_charp msg) {
			mprintf(("PNG error while parsing base64 PNG: %s\n", msg));
			longjmp(png_jmpbuf(png_ptr), 1);
		},
		[](png_structp, png_const_charp msg) {
			mprintf(("PNG warning while parsing base64 PNG: %s\n", msg));
		}, 
		[](png_structp png_ptr, png_bytep datap, png_size_t length) {
			b64_dec_buffer& buf = *static_cast<b64_dec_buffer*>(png_get_io_ptr(png_ptr));

			for (size_t finalPos = buf.pos + length; buf.pos < finalPos; buf.pos++) {
				*(datap++) = buf.decoded.at(buf.pos);
			}
		}, []() {});
}

static bool png_write_bitmap_data(size_t width, size_t height, bool y_flip, const uint8_t* data, png_voidp status, png_error_ptr error, png_error_ptr warning, png_rw_ptr writeFunc, png_flush_ptr flushFnc) {
	auto png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING, status, error, warning, status, png_malloc_fn, png_free_fn);

	auto info_ptr = png_create_info_struct(png_ptr);
	png_set_IHDR(png_ptr, info_ptr, (png_uint_32)width, (png_uint_32)height, 8, PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	std::vector<uint8_t*> rows(height);
	for (size_t y = 0; y < height; ++y) {
		auto index = y_flip ? height - y - 1 : y;

		rows[index] = (uint8_t*)data + y * width * 4;
	}

	png_set_rows(png_ptr, info_ptr, rows.data());
#ifdef PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
	// According to the documentation level 6 should perform reasonably well for us
	png_set_compression_level(png_ptr, 6);
#endif
	png_set_write_fn(png_ptr, status, writeFunc, flushFnc);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	return true;
}

bool png_write_bitmap(const char* filename, size_t width, size_t height, bool y_flip, const uint8_t* data) {
	png_status status;

	status.writing = true;
	status.filename = filename;
	status.cfp = cfopen(filename, "wb");

	if (!status.cfp) {
		return false;
	}

	png_write_bitmap_data(width, height, y_flip, data, &status, png_error_fn, png_warning_fn, png_scp_write_data, png_scp_flush);

	cfclose(status.cfp);

	return true;
}

SCP_string png_b64_bitmap(size_t width, size_t height, bool y_flip, const uint8_t* data) {
	struct b64_enc_buffer {
		size_t i = 0;
		std::array<png_byte, 3> buffer;
		std::stringstream b64;

	} buffer;

	png_write_bitmap_data(width, height, y_flip, data, &buffer,
		[](png_structp png_ptr, png_const_charp msg) {
		mprintf(("PNG error while generating base64: %s\n", msg));
		longjmp(png_jmpbuf(png_ptr), 1);
		},
		[](png_structp, png_const_charp msg) {
			mprintf(("PNG warning while generating base64: %s\n", msg));
		}, 
		[](png_structp png_ptr, png_bytep datap, png_size_t length) {
			auto& buf = *static_cast<b64_enc_buffer*>(png_get_io_ptr(png_ptr));

			while (buf.i != 0 && length != 0) {
				length--;
				buf.buffer[buf.i] = *(datap++);

				if (++buf.i >= 3) {
					buf.i = 0;
					base64_encode(buf.b64, buf.buffer.data(), 3);
				}
			}

			if (length == 0)
				return;

			size_t rem = length % 3;

			base64_encode(buf.b64, datap, (unsigned int) length - (unsigned int) rem);
			datap = &datap[length - rem];

			for (; buf.i < rem; buf.i++) {
				buf.buffer[buf.i] = *(datap++);
			}
		},
		[](png_structp png_ptr) {
			auto& buf = *static_cast<b64_enc_buffer*>(png_get_io_ptr(png_ptr));

			if (buf.i != 0) {
				base64_encode(buf.b64, buf.buffer.data(), (unsigned int) buf.i);
			}
		});

	return buffer.b64.str();
}

/*
 * APNG related code
 * Refer https://wiki.mozilla.org/APNG_Specification
 *
 * Original Copyright (c) 2014 Max Stepin
 * maxst at users.sourceforge.net
 * http://sourceforge.net/projects/apng/files/libpng/examples/
 * and
 * https://github.com/apngasm/apngasm/commit/92e409b8b9182b4442b0d9ab6dac0ed90c4e3ffd
 * with hints gleaned from
 * https://bugs.webkit.org/attachment.cgi?id=248547&action=diff
 *
 * zlib license
 * ------------
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

namespace apng {

const uint id_IHDR = 0x52444849; // PNG header
const uint id_acTL = 0x4C546361; // Animation control chunk
const uint id_fcTL = 0x4C546366; // Frame control chunk
const uint id_IDAT = 0x54414449; // first frame and/or default image
const uint id_fdAT = 0x54416466; // Frame data chunk
const uint id_IEND = 0x444E4549; // end/footer chunk

// Protect against large PNGs. See Mozilla's bug #251381 for more info.
const unsigned long cMaxPNGSize = 1000000UL;

/*
 * @brief test if character is a valid png chunk type character
 * @note refer to http://www.w3.org/TR/PNG/#5Chunk-layout
 */
static inline bool not_chunk(ubyte c)
{
	return c < 65 || c > 122 || (c > 90 && c < 97);
}

/*
 * @brief shim for libpng info/IHDR chunk callback
 */
static inline void info_callback(png_structp png_ptr, png_infop  /*info_ptr*/)
{
	static_cast<apng_ani*>(png_get_progressive_ptr(png_ptr))->info_callback();
}

/*
 * @brief shim for libpng row callback
 */
static inline void row_callback(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int  /*pass*/)
{
	static_cast<apng_ani*>(png_get_progressive_ptr(png_ptr))->row_callback(new_row, row_num);
}

/*
 * @brief compose new frame from previous frame & currently-processing frames raw data
 * @note assumes png data has 4 bytes
 */
void apng_ani::_compose_frame()
{
	int u, v, al;

	// our libpng transformations should ensure 4 bytes per pixel by this point
	Assertion(bpp == 32, "apng frame composition assumes 4 bytes of data per pixel, get a coder!");
	for (uint j = 0; j < _frameh; j++) {
		ubyte* sp = _frame_raw.rows[j];
		ubyte* dp = frame.rows[j + _y_offset] + _x_offset * 4;

		if (_blend_op == 0) {
			// blend operation 0 (APNG_BLEND_OP_SOURCE) means ignore destination
			memcpy(dp, sp, _framew * 4);
		}
		else {
			for (uint i = 0; i < _framew; i++, sp += 4, dp += 4) {
				if (sp[3] == 255) {
					// if source is not transparent at all, just copy it
					memcpy(dp, sp, 4);
				}
				else {
					if (sp[3] != 0) {
						if (dp[3] != 0) {
							u = sp[3] * 255;
							v = (255 - sp[3]) * dp[3];
							al = u + v;
							dp[0] = static_cast<ubyte>((sp[0] * u + dp[0] * v) / al);
							dp[1] = static_cast<ubyte>((sp[1] * u + dp[1] * v) / al);
							dp[2] = static_cast<ubyte>((sp[2] * u + dp[2] * v) / al);
							dp[3] = static_cast<ubyte>(al / 255);
						}
						else {
							// destination is transparent, overwrite it
							memcpy(dp, sp, 4);
						}
					}
					// if source is transparent, ignore it
				}
			}
		}
	}
}

/*
 * @brief start processing apng frame
 * @note uses libpng calls, treats apng data as discrete png images
 */
int apng_ani::_processing_start()
{
	static ubyte png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};

	_pngp = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	_infop = png_create_info_struct(_pngp);
	if (_pngp == nullptr || _infop == nullptr) {
		return 1;
	}

	if (setjmp(png_jmpbuf(_pngp))) {
		png_destroy_read_struct(&_pngp, &_infop, 0);
		return 1;
	}

	png_set_crc_action(_pngp, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
	png_set_progressive_read_fn(_pngp, this, apng::info_callback, apng::row_callback, nullptr);

	png_process_data(_pngp, _infop, png_sig, 8);
	memcpy(&_chunk_IHDR.data[8], &_chunk.data[12], 8); // use frame width & height from fcTL
	png_process_data(_pngp, _infop, &_chunk_IHDR.data[0], _chunk_IHDR.size);

	for (size_t i = 0; i < _info_chunks.size(); ++i) {
		// this processes chunks like tRNS / PLTE / etc
		png_process_data(_pngp, _infop, &_info_chunks.at(i).data[0], _info_chunks.at(i).size);
	}

	return 0;
}

/*
 * @brief process apng data chunks
 * @note uses libpng calls, treats apng data as discrete png images
 */
void apng_ani::_processing_data(ubyte* data, uint size)
{
	png_process_data(_pngp, _infop, data, size);
}

/*
 * @brief finish processing apng frame
 * @note uses libpng calls, treats apng data as discrete png images
 */
int apng_ani::_processing_finish()
{
	static ubyte end_sig[12] = {0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130};

	if (_pngp == nullptr || _infop == nullptr) {
		return 1;
	}

	if (setjmp(png_jmpbuf(_pngp))) {
		png_destroy_read_struct(&_pngp, &_infop, 0);
		return 1;
	}

	png_process_data(_pngp, _infop, end_sig, 12);
	png_destroy_read_struct(&_pngp, &_infop, 0);

	return 0;
}

/*
 * @brief process IHDR data
 * @note not called until the IDAT chunk starts processing
 */
void apng_ani::info_callback()
{
	png_set_expand(_pngp);
	png_set_strip_16(_pngp);
	png_set_gray_to_rgb(_pngp); // what about 'non-colour' headanis?
	png_set_add_alpha(_pngp, 0xff, PNG_FILLER_AFTER); // doesn't affect images that already have alpha channel
	png_set_interlace_handling(_pngp);
	png_set_bgr(_pngp);

	png_read_update_info(_pngp, _infop);
}

/*
 * @brief process each row of data provided by libpng
 * @note callback from libpng
 *
 * @param [in] new_row  pointer to start of next row of data
 * @param [in] row_num  number of row in image
 */
void apng_ani::row_callback(png_bytep new_row, png_uint_32 row_num)
{
	png_progressive_combine_row(_pngp, _frame_raw.rows.at(row_num), new_row);
}


/*
 * @brief manually process a single chunk of apng data
 */
void apng_ani::_process_chunk()
{
	_id = _read_chunk(_chunk);

	if (_id == id_acTL && !_got_IDAT && !_got_acTL) {
		// animation control chunk
		if (!_reading) {
			return;
		}
		nframes = png_get_uint_32(&_chunk.data[8]);
		plays = png_get_uint_32(&_chunk.data[12]);

		if (!nframes || nframes > PNG_UINT_31_MAX || plays > PNG_UINT_31_MAX) {
			_apng_failed("invalid apng acTL data");
		}

		_got_acTL = true;
		_frames.reserve(nframes);
		_frame_offsets.reserve(nframes+1); // extra 1 is for EOF offset
	}
	else if (_id == id_fcTL && (!_got_IDAT || _got_acTL)) {
		// frame control chunk
		if (_reading) {
			uint sequence_num = png_get_uint_32(&_chunk.data[8]);
			if (sequence_num != _sequence_num++) {
				_apng_failed("invalid apng fcTL sequence number");
			}
		}

		// handle frame finish in next_frame
		_framew = png_get_uint_32(&_chunk.data[12]);
		_frameh = png_get_uint_32(&_chunk.data[16]);
		_x_offset = png_get_uint_32(&_chunk.data[20]);
		_y_offset = png_get_uint_32(&_chunk.data[24]);
		_delay_num = png_get_uint_16(&_chunk.data[28]);
		_delay_den = png_get_uint_16(&_chunk.data[30]);
		_dispose_op = _chunk.data[32];
		_blend_op = _chunk.data[33];

		if (_reading &&
				(_framew > cMaxPNGSize || _frameh > cMaxPNGSize
				|| _x_offset > cMaxPNGSize || _y_offset > cMaxPNGSize
				|| _x_offset + _framew > w || _y_offset + _frameh > h
				|| _dispose_op > 2 || _blend_op > 1)) {
			_apng_failed("invalid apng fcTL data");
		}

		// according to spec...
		if (current_frame == 0 && _dispose_op == 2) {
			_dispose_op = 1;
		}

		if (_delay_den == 0) _delay_den = 100; // APNG spec
		if (_delay_num == 0) _delay_num = 1;   // arbitrary lower bound
		float frame_delay = static_cast<float>(_delay_num)/static_cast<float>(_delay_den);
		frame.delay = frame_delay;

		if (_reading) {
			anim_time+= frame_delay;
			_frame_offsets.push_back((int)_offset);
		}
		else {
			if (_got_IDAT && _processing_start()) {
				_apng_failed("couldn't start fdat apng frame");
			}
		}
	}
	else if (_id == id_IDAT) {
		_got_IDAT = true;
		_processing_data(&_chunk.data[0], _chunk.size);
	}
	else if (_id == id_fdAT && _got_acTL) {
		if (_reading) {
			uint sequence_num = png_get_uint_32(&_chunk.data[8]);
			if (sequence_num != _sequence_num++) {
				_apng_failed("invalid apng fdAT sequence number");
			}
		}

		if (_reading) {
			return;
		}

		png_save_uint_32(&_chunk.data[4], _chunk.size - 16);
		memcpy(&_chunk.data[8], "IDAT", 4);
		_processing_data(&_chunk.data[4], _chunk.size - 4);
	}
	else if (_id == id_IEND) {
		return;
	}
	else if (not_chunk(_chunk.data[4]) || not_chunk(_chunk.data[5]) ||
			not_chunk(_chunk.data[6]) || not_chunk(_chunk.data[7])) {
		_apng_failed("unknown chunk ID found");
	}
	else if (!_got_IDAT) {
		_processing_data(&_chunk.data[0], _chunk.size);
		_info_chunks.push_back(_chunk);
	}
}

unsigned int apng_ani::_read_chunk(_chunk_s& chunk)
{
	_offset = cftell(_cfp);
	if (cfread(&chunk.data[0], 4, 1, _cfp) == 1) {
		chunk.size = png_get_uint_32(&chunk.data[0]) + 12;
		// reduce the amount of vector resizing
		if (chunk.size > _max_chunk_size) {
			_max_chunk_size = chunk.size;
			chunk.data.resize(chunk.size);
		}
		if (cfread(&chunk.data[4], chunk.size-4, 1, _cfp) == 1) {
			return *(uint*)(&chunk.data[4]);
		}
	}
	return 0;
}

/*
 * @brief preload/cache the anim frames
 * @note useful for preloading without having to load the apng into bmpman slots
 */
void apng_ani::preload()
{
	_reading = false;
	_cache = true;  // implied, otherwise preload is almost pointless
	while (current_frame < nframes) {
		next_frame();
	}
	goto_start();
}

/*
 * @brief get previous apng frame
 * @note due to apng format, this can only play backwards from the furthest forward frame reached
 */
void apng_ani::prev_frame()
{
	if (_cache != true) {
		_apng_failed("no caching, therefore can't retrieve previous frame");
	}
	_reading = false;
	if (current_frame > 0) {
		frame = _frames.at(--current_frame);
		nprintf(("apng", "apng prev_frame; (%03i/%03u)\n", current_frame, static_cast<uint>(_frames.size())));
	}
}


/*
 * @brief get next apng frame
 */
void apng_ani::next_frame()
{
	_reading = false;
	// setup new frame (if frame doesn't already exist)
	// always do if not caching
	// don't do if caching & already have frame
	if (_cache == false || (_frames.size() <= current_frame)) {
		if (current_frame > 0) {
			if (_dispose_op == 1) {
				// clear previous frame region of output buffer to fully transparent black
				for (uint row = 0; row < _frameh; ++row) {
					memset(&frame.data.at((_y_offset+row) * _row_len + _x_offset * 4), 0, _framew * 4);
				}
			}
			else if (_dispose_op == 2) {
				// revert previous frames region of output buffer to previous contents
				for (uint row = 0; row < _frameh; ++row) {
					uint pos = (_y_offset+row) * _row_len + _x_offset * 4;
					memcpy(&frame.data.at(pos), &_frame_next.data.at(pos), _framew * 4);
				}
			}
			// default is to do nothing with output buffer
		}

		while (cftell(_cfp) < _frame_offsets.at(current_frame+1)) {
			_process_chunk();
		}

		nprintf(("apng", "apng next_frame; new (%03i/%03u/%03i) (%u) (%u) %03u|%03u %03u|%03u (%02u) (%04f)\n",
				current_frame, static_cast<uint>(_frames.size()), nframes, _dispose_op, _blend_op,
				_framew, _x_offset, _frameh, _y_offset,
				static_cast<uint>(_frame_offsets.size()), frame.delay));

		if (_got_IDAT && _processing_finish()) {
			_apng_failed("couldn't finish fdat apng frame");
		}

		if (_dispose_op == 2) {
			// revert to previous; so save the current frame region for later
			for (uint row = 0; row < _frameh; ++row) {
				uint pos = (_y_offset+row) * _row_len + _x_offset * 4;
				memcpy(&_frame_next.data.at(pos), &frame.data.at(pos), _framew * 4);
			}
		}

		_compose_frame();
		if (_cache == true) _frames.push_back(frame);
	}
	else {
		if (current_frame < nframes) {
			nprintf(("apng", "apng next_frame; used old (%03i/%03u)\n", current_frame, static_cast<uint>(_frames.size())));
			frame = _frames.at(current_frame);
		}
	}
	++current_frame;
}

/*
 * @brief return image size in bytes
 */
size_t apng_ani::imgsize()
{
	return _image_size;
}


/*
 * @brief send animation to its 1st frame
 *
 * @note headers must be read before this function can be called
 */
void apng_ani::goto_start()
{
	current_frame = 0;
	if (_cache == false ) {  // only required if not caching frames
		if (cfseek(_cfp, _frame_offsets.at(0), CF_SEEK_SET) != 0) {
			_apng_failed("couldn't seek to 1st fcTL offset");
		}
	}
}


/*
 * @brief Get info about the apng
 * @note Also validates the apng & sets it up to have frames read
 *
 * @retval PNG_ERROR_NONE (0), otherwise will raise exception
 */
int apng_ani::load_header()
{
	char filename[MAX_FILENAME_LEN];

	strcpy_s(filename, _filename.c_str());
	char *p = strchr( filename, '.' );
	if ( p != nullptr ) *p = 0;
	strcat_s( filename, ".png" );

	_cfp = cfopen( filename , "rb" );

	if ( _cfp == nullptr) {
		_apng_failed("couldn't open filename");
	}

	_reading = true;

	ubyte sig[8];
	if (cfread(sig, 8, 1, _cfp) != 1)  {
		_apng_failed("cfread of png signature failed");
	}
	if (png_sig_cmp(sig, 0, 8) != 0) {
		_apng_failed("file has invalid png signature");
	}

	// setup chunk sizes before use
	_chunk_IHDR.data.resize(25); // fixed IHDR chunk size
	_chunk.data.resize(25);      // match the other sizes, maybe waste up to 13 bytes (ooooh)

	_id = _read_chunk(_chunk_IHDR);

	if (_id != id_IHDR || _chunk_IHDR.size != 25) {
		_apng_failed("failed to read IHDR chunk");
	}

	w = png_get_uint_32(&_chunk_IHDR.data[8]);
	h = png_get_uint_32(&_chunk_IHDR.data[12]);
	_row_len = w * 4;

	// setup frames & keep bm_create happy
	_image_size = _row_len * h;
	frame.data.reserve(_image_size); // alloc only once
	frame.data.assign(_image_size, 0); // all transparent black per spec
	frame.rows.resize(h);
	_frame_raw.data.resize(_image_size);
	_frame_raw.rows.resize(h);
	_frame_next.data.resize(_image_size);
	_frame_next.rows.resize(h);
	for (uint i = 0; i < h; ++i) {
		// everything is correctly sized above; avoid .at() error checks
		frame.rows[i]       = &frame.data[i * _row_len];
		_frame_raw.rows[i]  = &_frame_raw.data[i * _row_len];
		_frame_next.rows[i] = &_frame_next.data[i * _row_len];
	}

	// read all data
	while (!cfeof(_cfp)) {
		_process_chunk();
	}

	// should be at EOF; attach to _frame_offsets to make next_frame code simpler
	Assertion(cfeof(_cfp) != 0, "apng not at EOF, get a coder!");
	_frame_offsets.push_back(cftell(_cfp));

	// sanity checks
	if (anim_time <= 0.0f) {
		_apng_failed("animation duration <= 0.0f, bad data?");
	}

	if (nframes < 1) {
		_apng_failed("animation didn't have any frames, is this a static png?");
	}

	// back to start, including reset of _cfp so it can be used for the 1st frame
	_reading = false;
	if (cfseek(_cfp, _frame_offsets.at(0), CF_SEEK_SET) != 0) {
		_apng_failed("couldn't seek to 1st fcTL offset");
	}

	return PNG_ERROR_NONE;
}

/*
 * @brief cleanup resources
 *
 */
void apng_ani::_cleanup_resources()
{
	png_destroy_read_struct(&_pngp, &_infop, nullptr);
	if (_cfp != nullptr) cfclose(_cfp);
}

/*
 * @brief something went badly wrong, throw an exception
 *
 * @param [in] msg  text to display about the error
 */
void apng_ani::_apng_failed(const char* msg)
{
	nframes = 0;
	current_frame = 0;
	plays = 0;
	anim_time = 0.0f;
	_cleanup_resources();

	SCP_string error_msg = "(file ";
	error_msg += _filename;
	error_msg += ") ";
	error_msg += msg;
	throw ApngException(error_msg.c_str());
}

/*
 * @brief ctor
 *
 * @note loads apng header data
 */
apng_ani::apng_ani(const char* filename, bool cache)
	: w(0)
	, h(0)
	, bpp(32)      // force all apngs to use this
	, nframes(0)
	, current_frame(0)
	, plays(0)
	, anim_time(0.0f)
	, _filename(filename)
	, _pngp(nullptr)
	, _infop(nullptr)
	, _cfp(nullptr)
	, _offset(0)
	, _sequence_num(0)
	, _id(0)
	, _row_len(0)
	, _image_size(0)
	, _framew(0)
	, _frameh(0)
	, _x_offset(0)
	, _y_offset(0)
	, _max_chunk_size(25) // IHDR must be 1st and it must be this big
	, _delay_num(1)
	, _delay_den(100)
	, _dispose_op(0)
	, _blend_op(0)
	, _reading(true)
	, _got_acTL(false)
	, _got_IDAT(false)
	, _cache(cache)
{
	load_header();
}

apng_ani::~apng_ani()
{
	_cleanup_resources();
}

} // end namespace apng
