#pragma once

#include "cutscene/Decoder.h"

namespace cutscene {
namespace ffmpeg {
struct InputStream;

struct DecoderStatus;

class SubtitleDecoder;

class FFMPEGDecoder: public Decoder {
 private:
	std::unique_ptr<InputStream> m_input;

	std::unique_ptr<InputStream> m_subtitleInput;

	std::unique_ptr<DecoderStatus> m_status;

	bool hasExternalSubtitle() const;

	void runSubtitleDecoder(SubtitleDecoder* decoder);

  public:
	FFMPEGDecoder();

	~FFMPEGDecoder() override;

	bool initialize(const SCP_string& fileName) override;

	MovieProperties getProperties() const override;

	void startDecoding() override;

	bool hasAudio() const override;

	bool hasSubtitles() const override;

	void close() override;
};
}
}
