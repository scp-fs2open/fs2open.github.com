
#include "cutscene/ffmpeg/internal.h"


namespace cutscene {
namespace ffmpeg {
DecoderStatus::DecoderStatus() {}

DecoderStatus::~DecoderStatus() {
	videoStreamIndex = -1;
	videoStream = nullptr;
	videoCodec = nullptr;

	if (videoCodecCtx != nullptr) {
		avcodec_close(videoCodecCtx);
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
		avcodec_free_context(&videoCodecCtx);
#endif
		videoCodecCtx = nullptr;
	}

	audioStreamIndex = -1;
	audioStream = nullptr;
	audioCodec = nullptr;

	if (audioCodecCtx != nullptr) {
		avcodec_close(audioCodecCtx);
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
		avcodec_free_context(&audioCodecCtx);
#endif
		audioCodecCtx = nullptr;
	}
}

} // namespace ffmpeg
} // namespace cutscene
