#ifndef _PNGUTILS_H
#define _PNGUTILS_H

// see comments in (1.2) pngconf.h on Linux
// libpng wants to check that the version of setjmp it uses is the same as the
// one that FSO uses; and it is
#define PNG_SKIP_SETJMP_CHECK

#include "cfile/cfile.h"
#include "globalincs/pstypes.h"
#include "png.h"

#define PNG_ERROR_INVALID		-1
#define PNG_ERROR_NONE 			0
#define PNG_ERROR_READING		1

// reading
extern int png_read_header(const char *real_filename, CFILE *img_cfp = NULL, int *w = nullptr, int *h = nullptr, int *bpp = nullptr, ubyte *palette = nullptr);
extern int png_read_header(const SCP_string& b64, int* w, int* h, int* bpp, ubyte *palette = nullptr);
extern int png_read_bitmap(const char *real_filename, ubyte *image_data, int *bpp, int dest_size, int cf_type = CF_TYPE_ANY);
extern int png_read_bitmap(const SCP_string& b64, ubyte* image_data, int* bpp);

extern bool png_write_bitmap(const char* filename, size_t width, size_t height, bool y_flip, const uint8_t* data);
extern SCP_string png_b64_bitmap(size_t width, size_t height, bool y_flip, const uint8_t* data);

namespace apng {

/*
 * @brief read apngs and provide access to their frames
 *
 * @note would prefer to use C++11 "Non-static data member initializers" however MSVC 2013 is the 1st version that supports them and we currently need to support 2010
 */
class apng_ani {
public:
	struct apng_frame {
		SCP_vector<ubyte>  data;
		SCP_vector<ubyte*> rows;
		float              delay;
	};

	apng_frame frame;
	uint       w;
	uint       h;
	int       bpp;
	uint       nframes;
	uint       current_frame;
	uint       plays;
	float      anim_time;

	apng_ani(const char* filenamen, bool cache = true);
	~apng_ani();

	int    load_header();
	void   goto_start();
	void   next_frame();
	void   prev_frame();
	void   preload();
	void   info_callback();
	void   row_callback(png_bytep new_row, png_uint_32 row_num);
	size_t imgsize();

private:
	struct _chunk_s {
		uint              size;
		SCP_vector<ubyte> data;
	};
	SCP_vector<apng_frame>  _frames;
	SCP_vector<int>         _frame_offsets;
	SCP_vector<_chunk_s>    _info_chunks;
	SCP_string              _filename;
	apng_frame              _frame_next, _frame_raw;
	_chunk_s                _chunk_IHDR, _chunk;
	png_structp             _pngp;
	png_infop               _infop;
	CFILE*                  _cfp;
	size_t                  _offset;
	uint                    _sequence_num, _id;
	uint                    _row_len, _image_size;
	uint                    _framew, _frameh;
	uint                    _x_offset, _y_offset;
	uint                    _max_chunk_size;
	ushort                  _delay_num, _delay_den;
	ubyte                   _dispose_op, _blend_op;
	bool                    _reading, _got_acTL, _got_IDAT, _cache;

	uint _read_chunk(_chunk_s& chunk);
	void _process_chunk();
	int  _processing_start();
	void _processing_data(ubyte* data, uint size);
	int  _processing_finish();
	void _apng_failed(const char* msg);
	void _compose_frame();
	void _cleanup_resources();
};

class ApngException : public std::runtime_error
{
 public:
	explicit ApngException(const std::string& msg) : std::runtime_error(msg) {}
	~ApngException() noexcept override = default;
};

}

#endif // _PNGUTILS_H
