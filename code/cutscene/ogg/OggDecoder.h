#pragma once

#include "cfile/cfile.h"
#include "cutscene/Decoder.h"

#include "theora/theora.h"
#include "vorbis/codec.h"

namespace cutscene {
namespace ogg {
// structure for maintaining info on a THEORAFILE stream
struct MovieInfo {
	ubyte theora_p;
	ubyte vorbis_p;

	ogg_sync_state osyncstate;
	ogg_page opage;
	ogg_packet opacket;
	ogg_stream_state v_osstate;
	ogg_stream_state t_osstate;

	theora_info tinfo;
	theora_comment tcomment;
	theora_state tstate;

	vorbis_info vinfo;
	vorbis_dsp_state vstate;
	vorbis_block vblock;
	vorbis_comment vcomment;
};

class OggVideoFrame: public VideoFrame {
 public:
	OggVideoFrame() {}

	virtual ~OggVideoFrame() {}

	virtual DataPointers getDataPointers() {
		DataPointers ptrs;
		ptrs.y = yData.get();
		ptrs.u = uData.get();
		ptrs.v = vData.get();

		return ptrs;
	}

	std::unique_ptr<ubyte[]> yData;
	std::unique_ptr<ubyte[]> uData;
	std::unique_ptr<ubyte[]> vData;
};

class OggDecoder: public Decoder {
 private:
	CFILE* filePtr;

	MovieInfo movie;

	int buffer_data();

	void queue_page();

 public:
	OggDecoder();

	virtual ~OggDecoder();

	virtual bool initialize(const SCP_string& fileName) SCP_OVERRIDE;

	virtual MovieProperties getProperties() SCP_OVERRIDE;

	virtual void startDecoding() SCP_OVERRIDE;

	virtual bool hasAudio() SCP_OVERRIDE;

	virtual void close() SCP_OVERRIDE;
};
}
}
