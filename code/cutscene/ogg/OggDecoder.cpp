#include <algorithm>
#include <string>

#include "cutscene/player.h"
#include "cutscene/ogg/OggDecoder.h"

#include "sound/sound.h"

namespace {
void copyYUVData(cutscene::ogg::OggVideoFrame* frame, const yuv_buffer& buffer) {
	frame->yData.reset(new ubyte[buffer.y_height * buffer.y_stride]);
	memcpy(frame->yData.get(), buffer.y, buffer.y_height * buffer.y_stride);

	frame->uData.reset(new ubyte[buffer.uv_height * buffer.uv_stride]);
	memcpy(frame->uData.get(), buffer.u, buffer.uv_height * buffer.uv_stride);

	frame->vData.reset(new ubyte[buffer.uv_height * buffer.uv_stride]);
	memcpy(frame->vData.get(), buffer.v, buffer.uv_height * buffer.uv_stride);

	frame->ySize.height = buffer.y_height;
	frame->ySize.stride = buffer.y_stride;
	frame->ySize.width = buffer.y_width;

	frame->uvSize.height = buffer.uv_height;
	frame->uvSize.stride = buffer.uv_stride;
	frame->uvSize.width = buffer.uv_width;
}
}

namespace cutscene {
namespace ogg {
OggDecoder::OggDecoder() : Decoder(), filePtr(nullptr) {
	memset(&movie, 0, sizeof(movie));
}

OggDecoder::~OggDecoder() {

}

int OggDecoder::buffer_data() {
	char* buffer = ogg_sync_buffer(&movie.osyncstate,
								   8192); // Doubled read size to fix choppy audio with high bitrate movies - Valathil
	int bytes = cfread(buffer, 1, 8192, filePtr);

	ogg_sync_wrote(&movie.osyncstate, bytes);

	return bytes;
}

void OggDecoder::queue_page() {
	Assert(movie.theora_p);

	ogg_stream_pagein(&movie.t_osstate, &movie.opage);

	if (movie.vorbis_p) {
		ogg_stream_pagein(&movie.v_osstate, &movie.opage);
	}
}

bool OggDecoder::initialize(const SCP_string& fileName) {
	float fps;
	SCP_string movieName = fileName;
	std::transform(movieName.begin(), movieName.end(), movieName.begin(), ::tolower);

	size_t dotPos = movieName.find('.');
	if (dotPos != SCP_string::npos) {
		movieName.resize(dotPos);
	}
	movieName.append(".ogg");

	// NOTE: Because the .ogg extension is used for both movies and sounds, we have to
	//       be a bit more specific about our search locations, so we look only in the
	//       two places that Theora movies might exist.
	filePtr = cfopen(movieName.c_str(), "rb", CFILE_NORMAL, CF_TYPE_ROOT);

	if (filePtr == nullptr) {
		filePtr = cfopen(movieName.c_str(), "rb", CFILE_NORMAL, CF_TYPE_MOVIES);
	}

	if (filePtr == nullptr) {
		mprintf(("Theora ERROR:  Unable to find and open movie file named '%s'\n", movieName.c_str()));
		return false;
	}
	// Initialize movie struct
	memset(&movie, 0, sizeof(movie));

	// start up Ogg stream synchronization layer
	ogg_sync_init(&movie.osyncstate);

	// init supporting Vorbis structures needed in header parsing
	vorbis_info_init(&movie.vinfo);
	vorbis_comment_init(&movie.vcomment);

	// init supporting Theora structures needed in header parsing
	theora_comment_init(&movie.tcomment);
	theora_info_init(&movie.tinfo);

	int stateflag = 0;
	// ogg file open so parse the headers
	// we are only interested in Vorbis/Theora streams
	while (!stateflag) {
		int ret = buffer_data();

		if (ret == 0) {
			break;
		}

		while (ogg_sync_pageout(&movie.osyncstate, &movie.opage) > 0) {
			ogg_stream_state test;

			// is this a mandated initial header? If not, stop parsing
			if (!ogg_page_bos(&movie.opage)) {
				// don't leak the page; get it into the appropriate stream
				queue_page();
				stateflag = 1;
				break;
			}

			ogg_stream_init(&test, ogg_page_serialno(&movie.opage));
			ogg_stream_pagein(&test, &movie.opage);
			ogg_stream_packetout(&test, &movie.opacket);

			// identify the codec: try theora
			if (!movie.theora_p && (theora_decode_header(&movie.tinfo, &movie.tcomment, &movie.opacket) >= 0)) {
				// it is theora
				memcpy(&movie.t_osstate, &test, sizeof(test));
				movie.theora_p = 1;
			} else if (!movie.vorbis_p &&
				(vorbis_synthesis_headerin(&movie.vinfo, &movie.vcomment, &movie.opacket) >= 0)) {
				// it is vorbis
				memcpy(&movie.v_osstate, &test, sizeof(test));
				movie.vorbis_p = 1;
			} else {
				// whatever it is, we don't care about it
				ogg_stream_clear(&test);
			}
		}
		// fall through to non-bos page parsing
	}

	// if we don't have usable video then bail out now
	if (!movie.theora_p) {
		mprintf(("Theora ERROR: Unable to find video data in '%s'\n", movieName.c_str()));
		goto Error;
	}

	// go ahead and do some audio stream error checking...
	if ((movie.vinfo.channels < 0) || (movie.vinfo.channels > 2)) {
		mprintf(("Theora ERROR:  Unsupported number of audio channels!\n"));
		movie.vorbis_p = 0;
	}

	// if we don't have usable audio then close out it's partial setup and just use the video
	if (!Sound_enabled || !movie.vorbis_p) {
		vorbis_info_clear(&movie.vinfo);
		vorbis_comment_clear(&movie.vcomment);
		movie.vorbis_p = 0;
	}

	int ret;

	// we're expecting more header packets.
	while ((movie.theora_p < 3) || (movie.vorbis_p && (movie.vorbis_p < 3))) {

		ret = ogg_stream_packetout(&movie.t_osstate, &movie.opacket);
		// look for further theora headers
		while ((movie.theora_p < 3) && (ret)) {
			if ((ret < 0) || theora_decode_header(&movie.tinfo, &movie.tcomment, &movie.opacket)) {
				mprintf(("Theora ERROR:  Error parsing Theora stream headers on '%s'!  Corrupt stream?\n", movieName.c_str()));
				goto Error;
			}

			if (++movie.theora_p == 3) {
				break;
			}

			ret = ogg_stream_packetout(&movie.t_osstate, &movie.opacket);
		}

		ret = ogg_stream_packetout(&movie.v_osstate, &movie.opacket);

		// look for more vorbis header packets
		while (movie.vorbis_p && (movie.vorbis_p < 3) && (ret)) {
			if ((ret < 0) || vorbis_synthesis_headerin(&movie.vinfo, &movie.vcomment, &movie.opacket)) {
				mprintf(("Theora ERROR:  Error parsing Vorbis stream headers on '%s'!  Corrupt stream?\n", movieName.c_str()));
				goto Error;
			}

			if (++movie.vorbis_p == 3) {
				break;
			}

			ret = ogg_stream_packetout(&movie.v_osstate, &movie.opacket);
		}

		// The header pages/packets will arrive before anything else we care about, or the stream is not obeying spec

		if (ogg_sync_pageout(&movie.osyncstate, &movie.opage) > 0) {
			queue_page(); // demux into the appropriate stream
		} else {
			ret = buffer_data(); // someone needs more data

			if (ret == 0) {
				mprintf(("Theora ERROR:  End of file while searching for codec headers in '%s'!\n", movieName.c_str()));
				goto Error;
			}
		}
	}

	// initialize video decoder
	theora_decode_init(&movie.tstate, &movie.tinfo);

	// and now for video stream error checking...
	if (movie.tinfo.pixelformat != OC_PF_420) {
		mprintf(("Theora ERROR:  Only the yuv420p chroma is supported!\n"));
		goto Error;
	}

	if (movie.tinfo.offset_x || movie.tinfo.offset_y) {
		mprintf(("Theora ERROR:  Player does not support frame offsets!\n"));
		goto Error;
	}

	// initialize audio decoder, if there is audio
	if (movie.vorbis_p) {
		vorbis_synthesis_init(&movie.vstate, &movie.vinfo);
		vorbis_block_init(&movie.vstate, &movie.vblock);
	}

	fps = static_cast<float>(movie.tinfo.fps_numerator) / movie.tinfo.fps_denominator;
	// Always buffer one second of video playback
	initializeQueues(static_cast<size_t>(fps));

	return true;

	Error:
	close();

	return false;
}

MovieProperties OggDecoder::getProperties() {
	MovieProperties props;

	props.size.height = movie.tinfo.frame_height;
	props.size.width = movie.tinfo.frame_width;

	props.fps = static_cast<float>(movie.tinfo.fps_numerator) / movie.tinfo.fps_denominator;

	return props;
}

void OggDecoder::startDecoding() {
	int i, j;
	bool videobuf_ready = false;
	bool audiobuf_ready = false;

	int audiobuf_fill = 0;
	int audiofd_fragsize = movie.vinfo.channels * movie.vinfo.rate;
	SCP_vector<short> audiobuf;
	audiobuf.resize(audiofd_fragsize);

	int frameId = 0;

	AudioFramePtr audioData;
	std::unique_ptr<OggVideoFrame> videoData;

	if (!movie.theora_p) {
		return;
	}

	// on to the main decode loop.	We assume in this example that audio
	// and video start roughly together, and don't begin playback until
	// we have a start frame for both.	This is not necessarily a valid
	// assumption in Ogg A/V streams! It will always be true of the
	// example_encoder (and most streams) though.

	while (isDecoding()) {
		// we want a video and audio frame ready to go at all times.	If
		// we have to buffer incoming, buffer the compressed data (ie, let
		// ogg do the buffering)
		if (canPushAudioData()) {
			while (movie.vorbis_p && !audiobuf_ready) {
				int ret;
				float** pcm;

				// if there's pending, decoded audio, grab it
				if ((ret = vorbis_synthesis_pcmout(&movie.vstate, &pcm)) > 0) {
					int count = audiobuf_fill;
					int maxsamples = (audiofd_fragsize - audiobuf_fill) / movie.vinfo.channels;

					for (i = 0; (i < ret) && (i < maxsamples); i++) {
						for (j = 0; j < movie.vinfo.channels; j++) {
							float val = pcm[j][i] * 32767.0f + 0.5f;

							if (val > 32767.0f) {
								val = 32767.0f;
							}

							if (val < -32768.0f) {
								val = -32768.0f;
							}

							audiobuf[count++] = (short) val;
						}
					}

					vorbis_synthesis_read(&movie.vstate, i);

					audiobuf_fill += i * movie.vinfo.channels;

					if (audiobuf_fill == audiofd_fragsize) {
						audiobuf_ready = true;
						audiobuf_fill = 0;

						audioData.reset(new AudioFrame());
						audioData->audioData = SCP_vector<short>();
						audioData->audioData.assign(audiobuf.begin(), audiobuf.end());

						audioData->channels = movie.vinfo.channels;
						audioData->rate = movie.vinfo.rate;
					}
				} else {
					// no pending audio; is there a pending packet to decode?
					if (ogg_stream_packetout(&movie.v_osstate, &movie.opacket) > 0) {
						if (vorbis_synthesis(&movie.vblock, &movie.opacket) == 0) {
							vorbis_synthesis_blockin(&movie.vstate, &movie.vblock);
						}
					}
						// we need more data; break out to suck in another page
					else {
						break;
					}
				}
			}
		}

		while (!videobuf_ready && canPushVideoData()) {
			// theora is one in, one out...
			if (ogg_stream_packetout(&movie.t_osstate, &movie.opacket) <= 0) {
				break;
			}

			theora_decode_packetin(&movie.tstate, &movie.opacket);

			auto videobuf_time = theora_granule_time(&movie.tstate, movie.tstate.granulepos);

			videobuf_ready = true;
			videoData.reset(new OggVideoFrame());
			videoData->frameTime = videobuf_time;
			videoData->id = ++frameId;

			yuv_buffer yuv;
			// Now decode to YUV
			theora_decode_YUVout(&movie.tstate, &yuv);

			// Copy the data to our frame
			copyYUVData(videoData.get(), yuv);
		}

		if (!videobuf_ready || (movie.vorbis_p && !audiobuf_ready)) {
			// no data yet for somebody.	Grab another page
			buffer_data();

			while (ogg_sync_pageout(&movie.osyncstate, &movie.opage) > 0) {
				queue_page();
			}
		}

		if (!videobuf_ready && (!movie.vorbis_p || !audiobuf_ready) && cfeof(filePtr)) {
			// Check if the streams are empty
			if (ogg_stream_packetpeek(&movie.t_osstate, nullptr) == 0 &&
				(!movie.vorbis_p || ogg_stream_packetpeek(&movie.v_osstate, nullptr) == 0)) {
				// The file pointer is at the end of the file
				break;
			}
		}

		if (canPushAudioData() && audiobuf_ready) {
			pushAudioData(std::move(audioData));;
			audioData = nullptr;
			audiobuf_ready = false;
		}

		// are we at or past time for this video frame?
		if (canPushVideoData() && videobuf_ready) {
			pushFrameData(VideoFramePtr(videoData.release()));
			videoData = nullptr;
			videobuf_ready = false;
		}
	}

	if (audiobuf_fill > 0) {
		// There is some audio data left
		audioData.reset(new AudioFrame());
		audioData->audioData = SCP_vector<short>();
		audioData->audioData.assign(audiobuf.begin(), audiobuf.begin() + audiobuf_fill);

		audioData->channels = movie.vinfo.channels;
		audioData->rate = movie.vinfo.rate;

		pushAudioData(std::move(audioData));
	}

	// Set the decoding flag to false so the player knows that we are finished
	stopDecoder();
}

bool OggDecoder::hasAudio() {
	return movie.vorbis_p != 0;
}

void OggDecoder::close() {
	if (movie.vorbis_p) {
		ogg_stream_clear(&movie.v_osstate);

		vorbis_block_clear(&movie.vblock);
		vorbis_dsp_clear(&movie.vstate);
		vorbis_comment_clear(&movie.vcomment);
		vorbis_info_clear(&movie.vinfo);
	}

	if (movie.theora_p) {
		ogg_stream_clear(&movie.t_osstate);

		theora_clear(&movie.tstate);
		theora_comment_clear(&movie.tcomment);
		theora_info_clear(&movie.tinfo);
	}

	ogg_sync_clear(&movie.osyncstate);

	// free the stream
	if (filePtr) {
		cfclose(filePtr);
	}
}
}
}
