#include "cutscene/ffmpeg/VideoDecoder.h"

#include "tracing/tracing.h"

namespace {
const AVPixelFormat DESTINATION_FORMAT = AV_PIX_FMT_YUV420P;

SwsContext* getSWSContext(int width, int height, AVPixelFormat fmt) {
	return sws_getContext(width, height, fmt, width, height, DESTINATION_FORMAT,
						  SWS_BILINEAR, nullptr, nullptr, nullptr);
}

double getFrameTime(int64_t pts, AVRational time_base) {
	return pts * av_q2d(time_base);
}
}

namespace cutscene {
namespace ffmpeg {
class FFMPEGVideoFrame: public VideoFrame {
 public:
	FFMPEGVideoFrame() : frame(nullptr) {
	}

	virtual ~FFMPEGVideoFrame() {
		if (frame != nullptr) {
			av_freep(&frame->data[0]);
			av_frame_free(&frame);
		}
	}

	virtual DataPointers getDataPointers() {
		DataPointers ptrs;
		ptrs.y = frame->data[0];
		ptrs.u = frame->data[1];
		ptrs.v = frame->data[2];

		return ptrs;
	}

	AVFrame* frame;
};

VideoDecoder::VideoDecoder(DecoderStatus* status)
	: FFMPEGStreamDecoder(status),
	  m_frameId(0) {
	m_swsCtx = getSWSContext(m_status->videoCodecPars.width, m_status->videoCodecPars.height,
							 m_status->videoCodecPars.pixel_format);
}

VideoDecoder::~VideoDecoder() {
	sws_freeContext(m_swsCtx);
}

void VideoDecoder::convertAndPushPicture(const AVFrame* frame) {
	// Allocate a picture to hold the YUV data
	AVFrame* yuvFrame = av_frame_alloc();
	av_frame_copy(yuvFrame, frame);
	yuvFrame->format = DESTINATION_FORMAT;

	av_image_alloc(yuvFrame->data,
				   yuvFrame->linesize,
				   m_status->videoCodecPars.width,
				   m_status->videoCodecPars.height,
				   DESTINATION_FORMAT,
				   1);

	std::unique_ptr<FFMPEGVideoFrame> videoFramePtr(new FFMPEGVideoFrame());

	if (m_status->videoCodecPars.pixel_format == DESTINATION_FORMAT) {
		av_image_copy(yuvFrame->data,
					  yuvFrame->linesize,
					  (const uint8_t**) (frame->data),
					  frame->linesize,
					  DESTINATION_FORMAT,
					  m_status->videoCodecPars.width,
					  m_status->videoCodecPars.height);
	} else {
		// Convert frame to YUV
		sws_scale(
			m_swsCtx,
			(uint8_t const* const*) frame->data,
			frame->linesize,
			0,
			m_status->videoCodecPars.height,
			yuvFrame->data,
			yuvFrame->linesize
		);
	}

	videoFramePtr->id = ++m_frameId;
	videoFramePtr->frameTime = getFrameTime(av_frame_get_best_effort_timestamp(frame), m_status->videoStream->time_base);
	videoFramePtr->frame = yuvFrame;

	videoFramePtr->ySize.height = static_cast<size_t>(m_status->videoCodecPars.height);
	videoFramePtr->ySize.width = static_cast<size_t>(m_status->videoCodecPars.width);
	videoFramePtr->ySize.stride = static_cast<size_t>(yuvFrame->linesize[0]);

	// 420P means that the UV channels have half the width and height
	videoFramePtr->uvSize.height = static_cast<size_t>(m_status->videoCodecPars.height / 2);
	videoFramePtr->uvSize.width = static_cast<size_t>(m_status->videoCodecPars.width / 2);
	videoFramePtr->uvSize.stride = static_cast<size_t>(yuvFrame->linesize[1]);

	pushFrame(std::move(VideoFramePtr(videoFramePtr.release())));
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
