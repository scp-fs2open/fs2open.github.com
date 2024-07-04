#pragma once

#include "cutscene/ffmpeg/internal.h"
#include "cutscene/ffmpeg/FFMPEGDecoder.h"

namespace cutscene {
namespace ffmpeg {
class VideoDecoder: public FFMPEGStreamDecoder<VideoFrame> {
 private:
	int m_frameId;
	SwsContext* m_swsCtx;
	AVPixelFormat m_destinationFormat;

	void convertAndPushPicture(const AVFrame* frame);

 public:
	explicit VideoDecoder(DecoderStatus* status, AVPixelFormat destination_fmt);

	~VideoDecoder() override;

	void decodePacket(AVPacket* packet) override;

	void finishDecoding() override;

	void flushBuffers() override;
};
}
}
