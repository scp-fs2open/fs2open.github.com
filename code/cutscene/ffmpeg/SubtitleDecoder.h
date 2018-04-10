#pragma once

#include "cutscene/ffmpeg/internal.h"
#include "cutscene/ffmpeg/FFMPEGDecoder.h"

namespace cutscene {
namespace ffmpeg {

class SubtitleDecoder: public FFMPEGStreamDecoder<SubtitleFrame> {
 public:
    explicit SubtitleDecoder(DecoderStatus* status);

	~SubtitleDecoder() override;

	void decodePacket(AVPacket* packet) override;

	void finishDecoding() override;
    void pushSubtitleFrame(AVPacket* subtitle, AVSubtitle* pSubtitle);
};

}
}


