#pragma once

#include "cutscene/ffmpeg/internal.h"
#include "cutscene/ffmpeg/FFMPEGDecoder.h"

namespace cutscene {
namespace ffmpeg {

class SubtitleDecoder: public FFMPEGStreamDecoder<SubtitleFrame> {
 public:
    explicit SubtitleDecoder(DecoderStatus* status);

    virtual ~SubtitleDecoder();

    virtual void decodePacket(AVPacket* packet) SCP_OVERRIDE;

    virtual void finishDecoding() SCP_OVERRIDE;
    void pushSubtitleFrame(AVPacket* subtitle, AVSubtitle* pSubtitle);
};

}
}


