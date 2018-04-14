#pragma once

#include "cutscene/Decoder.h"

namespace cutscene {
namespace ffmpeg {
struct InputStream;

struct DecoderStatus;

class FFMPEGDecoder: public Decoder {
 private:
	std::unique_ptr<InputStream> m_input;

	std::unique_ptr<DecoderStatus> m_status;

 public:
	FFMPEGDecoder();

	~FFMPEGDecoder() override;

	bool initialize(const SCP_string& fileName) override;

	MovieProperties getProperties() override;

	void startDecoding() override;

	bool hasAudio() override;

	bool hasSubtitles() override;

	void close() override;
};
}
}
