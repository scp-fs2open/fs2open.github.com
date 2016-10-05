#pragma once

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
 * createContex can be used for opening a specific file for FFmpeg decoding
 */
class FFmpegContext {
 private:
	AVFormatContext* m_ctx;
	CFILE* m_file;

	FFmpegContext(CFILE* file);
 public:
	~FFmpegContext();

	FFmpegContext(const FFmpegContext&) = delete;
	FFmpegContext& operator=(const FFmpegContext&) = delete;

	inline AVFormatContext* ctx() { return m_ctx; }

	static std::unique_ptr<FFmpegContext> createContext(CFILE* mediaFile);

	static std::unique_ptr<FFmpegContext> createContext(const SCP_string& path, int dir_type);
};

}
}