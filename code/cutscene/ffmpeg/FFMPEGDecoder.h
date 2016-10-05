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

	virtual ~FFMPEGDecoder();

	bool initialize(const SCP_string& fileName) SCP_OVERRIDE;

	MovieProperties getProperties() SCP_OVERRIDE;

	void startDecoding() SCP_OVERRIDE;

	bool hasAudio() SCP_OVERRIDE;

	void close() SCP_OVERRIDE;
};
}
}
