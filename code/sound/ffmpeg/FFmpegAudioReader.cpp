#include "FFmpegAudioReader.h"

namespace
{
class AVPacketScope
{
	AVPacket* _packet;
public:
	explicit AVPacketScope(AVPacket* av_packet)
		: _packet(av_packet) {
	}

	~AVPacketScope() {
		av_packet_unref(_packet);
	}
};
}

namespace ffmpeg
{
FFmpegAudioReader::FFmpegAudioReader(AVFormatContext* av_format_context, AVCodecContext* codec_ctx,
                                     int stream_idx) : _stream_idx(stream_idx),
                                                       _format_ctx(av_format_context),
                                                       _codec_ctx(codec_ctx) {
}

bool FFmpegAudioReader::readFrame(AVFrame* decode_frame) {
	// FFmpeg 3.1 and onwards has a new, better packet handling API but that version is relatively new so we still need
	// to support the earlier API. The compatibility code can be removed once the new version is supported by most platforms

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
	// Process pending frames
	auto pending_res = avcodec_receive_frame(_codec_ctx, decode_frame);

	if (pending_res == 0) {
		// Got a frame
		return true;
	}
	if (pending_res == AVERROR_EOF) {
		// No more frames available
		return false;
	}

	if (pending_res != AVERROR(EAGAIN)) {
		// Unknown error.
		return false;
	}
#else
	// Assume that everything was handled before for this version
	// That might not be true in all cases but supporting those rare cases for an outdated version is not worth the trouble
#endif

	AVPacket packet;
	while (av_read_frame(_format_ctx, &packet) >= 0) {
		AVPacketScope packet_scope(&packet);
		if (packet.stream_index == _stream_idx) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
			auto res = avcodec_send_packet(_codec_ctx, &packet);

			if (res != 0) {
				// Error or EOF
				return false;
			}

			res = avcodec_receive_frame(_codec_ctx, decode_frame);

			if (res == 0) {
				// Got a frame
				return true;
			}
			if (res == AVERROR_EOF) {
				// No more frames available
				return false;
			}

			if (res != AVERROR(EAGAIN)) {
				// Unknown error.
				return false;
			}
			// EGAIN was returned, send new input
#else
			int finishedFrame = 0;
			auto err = avcodec_decode_audio4(_codec_ctx, decode_frame, &finishedFrame, &packet);

			if (finishedFrame) {
				return true;
			}

			if (err < 0) {
				// Error
				return false;
			}
#endif
		}
	}

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
	// Flush decoder
	if (avcodec_send_packet(_codec_ctx, nullptr) != 0) {
		return false;
	}

	auto flush_res = avcodec_receive_frame(_codec_ctx, decode_frame);

	if (flush_res == 0) {
		// Got a frame
		return true;
	}
#else
	AVPacket nullPacket;
	memset(&nullPacket, 0, sizeof(nullPacket));
	nullPacket.data = nullptr;
	nullPacket.size = 0;

	int finishedFrame = 1;
	auto err = avcodec_decode_audio4(_codec_ctx, decode_frame, &finishedFrame, &nullPacket);

	if (finishedFrame && err >= 0) {
		return true;
	}
#endif

	// If we are here then read_frame reached the end or returned an error
	return false;
}
}
