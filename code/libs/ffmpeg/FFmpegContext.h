#pragma once

#include "globalincs/vmallocator.h"
#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

#include "FFmpeg.h"
#include "FFmpegHeaders.h"

#include <memory>

namespace libs {
namespace ffmpeg {

/**
 * @brief Stores a vector of bytes and an access position to treat it as file
 *
 *
 */

struct MemFileCursor {
    const SCP_vector<uint8_t> snddata;
    SCP_vector<uint8_t>::size_type cursor_pos;

    MemFileCursor(const uint8_t* snddata, size_t snd_len);
    MemFileCursor();
};


/**
 * @brief Contains a FFmpeg context
 *
 * This should be used for making sure a FFmpeg context is always deallocated even if an exception is throws.
 * createContext can be used for opening a specific file for FFmpeg decoding
 */


class FFmpegContext {
 private:
	AVFormatContext* m_ctx;
	CFILE* m_file;
	
    MemFileCursor m_memsound;

	FFmpegContext(CFILE* file);
	FFmpegContext(const uint8_t* snddata, size_t snd_len);

    void prepare(void *opaque,
                 int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                 int64_t (*seek)(void *opaque, int64_t offset, int whence));
 public:
	~FFmpegContext();

	FFmpegContext(const FFmpegContext&) = delete;
	FFmpegContext& operator=(const FFmpegContext&) = delete;

	inline AVFormatContext* ctx() { return m_ctx; }

	static std::unique_ptr<FFmpegContext> createContext(CFILE* mediaFile);
	
	static std::unique_ptr<FFmpegContext> createContextMem(const uint8_t* snddata, size_t snd_len);

	static std::unique_ptr<FFmpegContext> createContext(const SCP_string& path, int dir_type);
};

}
}
