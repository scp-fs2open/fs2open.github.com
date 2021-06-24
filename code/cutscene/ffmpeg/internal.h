#pragma once

#include <functional>

#include "libs/ffmpeg/FFmpegHeaders.h"

#include "cutscene/Decoder.h"

namespace cutscene {
namespace ffmpeg {
struct CodecContextParameters {
	int width = -1;
	int height = -1;
	AVPixelFormat pixel_format = AV_PIX_FMT_NONE;

	uint64_t channel_layout = 0;
	int sample_rate = -1;
	AVSampleFormat audio_format = AV_SAMPLE_FMT_NONE;

	AVCodecID codec_id = AV_CODEC_ID_NONE;
};

struct DecoderStatus {
	int videoStreamIndex = -1;
	AVStream* videoStream = nullptr;
	CodecContextParameters videoCodecPars;
	AVCodec* videoCodec = nullptr;
	AVCodecContext* videoCodecCtx = nullptr;

	int audioStreamIndex = -1;
	AVStream* audioStream = nullptr;
	CodecContextParameters audioCodecPars;
	AVCodec* audioCodec = nullptr;
	AVCodecContext* audioCodecCtx = nullptr;

	// Subtitles are a bit different since they max come from a different file
	bool externalSubtitles   = false;
	int subtitleStreamIndex = -1;
	AVStream* subtitleStream = nullptr;
	CodecContextParameters subtitleCodecPars;
	AVCodec* subtitleCodec = nullptr;
	AVCodecContext* subtitleCodecCtx = nullptr;

	DecoderStatus();

	~DecoderStatus();

	DecoderStatus(const DecoderStatus&) = delete;
	DecoderStatus& operator=(const DecoderStatus&) = delete;
};

template<typename Frame>
class FFMPEGStreamDecoder {
 protected:
	typedef Frame frame_type;

	DecoderStatus* m_status = nullptr;

	AVFrame* m_decodeFrame = nullptr;

	SCP_queue<FramePtr<frame_type>> m_decodedFrames;

	void pushFrame(FramePtr<frame_type>&& frame);
 public:
	explicit FFMPEGStreamDecoder(DecoderStatus* status);
	virtual ~FFMPEGStreamDecoder();

	FFMPEGStreamDecoder(const FFMPEGStreamDecoder&) = delete;
	FFMPEGStreamDecoder& operator=(const FFMPEGStreamDecoder&) = delete;

	virtual void decodePacket(AVPacket* packet) = 0;

	virtual void finishDecoding() = 0;

	virtual void flushBuffers() = 0;

	virtual FramePtr<frame_type> getFrame();
};

template <typename Frame>
void FFMPEGStreamDecoder<Frame>::pushFrame(FramePtr<frame_type>&& frame)
{
	m_decodedFrames.push(std::move(frame));
}

	template <typename Frame>
FFMPEGStreamDecoder<Frame>::FFMPEGStreamDecoder(DecoderStatus* status):m_status(status)
{
	m_decodeFrame = av_frame_alloc();
}

template <typename Frame>
FFMPEGStreamDecoder<Frame>::~FFMPEGStreamDecoder()
{
	av_frame_free(&m_decodeFrame);
}

template <typename Frame>
FramePtr<typename FFMPEGStreamDecoder<Frame>::frame_type> FFMPEGStreamDecoder<Frame>::getFrame()
{
	if (m_decodedFrames.empty()) {
		return nullptr;
	}

	auto frame = std::move(m_decodedFrames.front());
	m_decodedFrames.pop();

	return frame;
}
}
}
