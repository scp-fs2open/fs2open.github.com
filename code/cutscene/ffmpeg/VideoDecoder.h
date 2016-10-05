#pragma once

#include "cutscene/ffmpeg/internal.h"
#include "cutscene/ffmpeg/FFMPEGDecoder.h"

namespace cutscene {
namespace ffmpeg {
class VideoDecoder: public FFMPEGStreamDecoder<VideoFrame> {
 private:
	int m_frameId;
	SwsContext* m_swsCtx;

	void convertAndPushPicture(const AVFrame* frame);

 public:
	explicit VideoDecoder(DecoderStatus* status);

	virtual ~VideoDecoder();

	virtual void decodePacket(AVPacket* packet) SCP_OVERRIDE;

	virtual void finishDecoding() SCP_OVERRIDE;
};
}
}
