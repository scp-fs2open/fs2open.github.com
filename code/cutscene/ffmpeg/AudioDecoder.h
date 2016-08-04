#pragma once

#include "cutscene/ffmpeg/internal.h"
#include "cutscene/ffmpeg/FFMPEGDecoder.h"

namespace cutscene {
namespace ffmpeg {

class AudioDecoder: public FFMPEGStreamDecoder<AudioFrame> {
 private:
	SwrContext* m_resampleCtx;

	uint8_t** m_outData;
	int m_outLinesize;

	int m_maxOutNumSamples;
	int m_outNumSamples;

	SCP_vector<short> m_audioBuffer;

	void handleDecodedFrame(AVFrame* frame);

	void flushAudioBuffer();

 public:
	explicit AudioDecoder(DecoderStatus* status);

	virtual ~AudioDecoder();

	virtual void decodePacket(AVPacket* packet) SCP_OVERRIDE;

	virtual void finishDecoding() SCP_OVERRIDE;
};
}
}
