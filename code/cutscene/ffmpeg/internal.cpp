
#include "cutscene/ffmpeg/internal.h"


namespace cutscene {
namespace ffmpeg {
DecoderStatus::DecoderStatus() {}

DecoderStatus::~DecoderStatus() {
	videoStreamIndex = -1;
	videoStream = nullptr;
	videoCodec = nullptr;

	if (videoCodecCtx != nullptr) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
		avcodec_free_context(&videoCodecCtx);
#else
		avcodec_close(videoCodecCtx);
#endif
		videoCodecCtx = nullptr;
	}

	audioStreamIndex = -1;
	audioStream = nullptr;
	audioCodec = nullptr;

	if (audioCodecCtx != nullptr) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
		avcodec_free_context(&audioCodecCtx);
#else
		avcodec_close(audioCodecCtx);
#endif
		audioCodecCtx = nullptr;
	}

	subtitleStreamIndex = -1;
	subtitleStream = nullptr;
	subtitleCodec = nullptr;

	if (subtitleCodecCtx != nullptr) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
		avcodec_free_context(&subtitleCodecCtx);
#else
		avcodec_close(subtitleCodecCtx);
#endif
		subtitleCodecCtx = nullptr;
	}
}

} // namespace ffmpeg
} // namespace cutscene
