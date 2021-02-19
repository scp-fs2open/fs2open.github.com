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
 * @brief Contains a FFmpeg context
 *
 * This should be used for making sure a FFmpeg context is always deallocated even if an exception is throws.
 * createContext can be used for opening a specific file for FFmpeg decoding
 */

struct MemSoundCursor {
    const SCP_vector<uint8_t> snddata;
    SCP_vector<uint8_t>::size_type cursor_pos;
    
    MemSoundCursor(const uint8_t* snddata, size_t snd_len);
    MemSoundCursor() = default;
};

class FFmpegContext {
 private:
	AVFormatContext* m_ctx;
	CFILE* m_file;
	
	MemSoundCursor m_memsound;

	FFmpegContext(CFILE* file);
	FFmpegContext(const uint8_t* snddata, size_t snd_len);
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
