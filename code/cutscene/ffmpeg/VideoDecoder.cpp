#include "cutscene/ffmpeg/VideoDecoder.h"

#include "tracing/tracing.h"

namespace {
SwsContext* getSWSContext(int width, int height, AVPixelFormat fmt, AVPixelFormat destination_fmt)
{
	return sws_getContext(width, height, fmt, width, height, destination_fmt, SWS_BILINEAR, nullptr, nullptr, nullptr);
}

double getFrameTime(int64_t pts, AVRational time_base) {
	return pts * av_q2d(time_base);
}
}

namespace cutscene {
namespace ffmpeg {
class FFMPEGVideoFrame: public VideoFrame {
	size_t _width;
	size_t _height;
	AVFrame* _frame;

  public:
	FFMPEGVideoFrame(size_t width, size_t height, AVFrame* frame) : _width(width), _height(height), _frame(frame) {}

	~FFMPEGVideoFrame() override {
		if (_frame != nullptr) {
			av_freep(&_frame->data[0]);
			av_frame_free(&_frame);
		}
	}
	size_t getPlaneNumber() override
	{
		switch (_frame->format) {
		case AV_PIX_FMT_YUV420P:
			// YUV data is planar
			return 3;
		default:
			// Everything else is packed
			return 1;
		}
	}
	FrameSize getPlaneSize(size_t plane) override
	{
		switch (_frame->format) {
		case AV_PIX_FMT_YUV420P: {
			// YUV data is planar
			auto width  = plane > 0 ? _width / 2 : _width;
			auto height = plane > 0 ? _height / 2 : _height;

			return {width, height, (size_t)_frame->linesize[plane]};
		}
		default:
			// Everything else is packed
			return {_width, _height, (size_t)_frame->linesize[plane]};
		}
	}
	void* getPlaneData(size_t plane) override { return _frame->data[plane]; }
};

VideoDecoder::VideoDecoder(DecoderStatus* status, AVPixelFormat destination_fmt)
    : FFMPEGStreamDecoder(status), m_frameId(0), m_destinationFormat(destination_fmt)
{
	m_swsCtx = getSWSContext(m_status->videoCodecPars.width, m_status->videoCodecPars.height,
	                         m_status->videoCodecPars.pixel_format, destination_fmt);
}

VideoDecoder::~VideoDecoder() {
	sws_freeContext(m_swsCtx);
}

void VideoDecoder::convertAndPushPicture(const AVFrame* frame) {
	// Allocate a picture to hold the destination data
	AVFrame* yuvFrame = av_frame_alloc();
	av_frame_copy(yuvFrame, frame);
	yuvFrame->format = m_destinationFormat;

	av_image_alloc(yuvFrame->data, yuvFrame->linesize, m_status->videoCodecPars.width, m_status->videoCodecPars.height,
	               m_destinationFormat, 1);

	if (m_status->videoCodecPars.pixel_format == m_destinationFormat) {
		av_image_copy(yuvFrame->data, yuvFrame->linesize, (const uint8_t**)(frame->data), frame->linesize,
		              m_destinationFormat, m_status->videoCodecPars.width, m_status->videoCodecPars.height);
	} else {
		// Convert frame to destination format
		sws_scale(m_swsCtx, (uint8_t const* const*)frame->data, frame->linesize, 0, m_status->videoCodecPars.height,
		          yuvFrame->data, yuvFrame->linesize);
	}

	std::unique_ptr<FFMPEGVideoFrame> videoFramePtr(
	    new FFMPEGVideoFrame(static_cast<size_t>(m_status->videoCodecPars.width),
	                         static_cast<size_t>(m_status->videoCodecPars.height), yuvFrame));
	videoFramePtr->id = ++m_frameId;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(58, 3, 102)
	videoFramePtr->frameTime = getFrameTime(frame->best_effort_timestamp, m_status->videoStream->time_base);
#else
	videoFramePtr->frameTime = getFrameTime(av_frame_get_best_effort_timestamp(frame), m_status->videoStream->time_base);
#endif

	pushFrame(VideoFramePtr(videoFramePtr.release()));
}

void VideoDecoder::decodePacket(AVPacket* packet) {
	TRACE_SCOPE(tracing::CutsceneFFmpegVideoDecoder);
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
	int send_result;
	do {
		send_result = avcodec_send_packet(m_status->videoCodecCtx, packet);

		while(avcodec_receive_frame(m_status->videoCodecCtx, m_decodeFrame) == 0) {
			convertAndPushPicture(m_decodeFrame);
		}
	} while (send_result == AVERROR(EAGAIN));
#else
	int finishedFrame = 0;
	auto result = avcodec_decode_video2(m_status->videoCodecCtx, m_decodeFrame, &finishedFrame, packet);

	if (result >= 0 && finishedFrame) {
		convertAndPushPicture(m_decodeFrame);
	}
#endif
}

void VideoDecoder::finishDecoding() {
	TRACE_SCOPE(tracing::CutsceneFFmpegVideoDecoder);

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
	// Send flush packet
	avcodec_send_packet(m_status->videoCodecCtx, nullptr);

	// Handle those decoders that have a delay
	while (true) {
		auto ret = avcodec_receive_frame(m_status->videoCodecCtx, m_decodeFrame);

		if (ret == 0) {
			convertAndPushPicture(m_decodeFrame);
		} else {
			// Everything consumed or error
			break;
		}
	}
#else
	// Handle those decoders that have a delay
	AVPacket nullPacket;
	memset(&nullPacket, 0, sizeof(nullPacket));
	nullPacket.data = nullptr;
	nullPacket.size = 0;

	while (true) {
		int finishedFrame = 1;
		auto err = avcodec_decode_video2(m_status->videoCodecCtx, m_decodeFrame, &finishedFrame, &nullPacket);

		if (err < 0 || !finishedFrame) {
			break;
		}

		convertAndPushPicture(m_decodeFrame);
	}
#endif
}
}
}
