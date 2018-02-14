//
//

#include "WaveFile.h"
#include "FFmpegAudioReader.h"
#include "sound/ds.h"

// -- from parselo.cpp --
extern const char* stristr(const char* str, const char* substr);

namespace {
using namespace ffmpeg;

AudioProperties getAudioProps(AVStream* stream) {
	AudioProperties props;

	int channels;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
    channels = stream->codecpar->channels;
    props.channel_layout = stream->codecpar->channel_layout;
    props.sample_rate = stream->codecpar->sample_rate;
    props.format = (AVSampleFormat)stream->codecpar->format;
#else
	channels = stream->codec->channels;
	props.channel_layout = stream->codec->channel_layout;
	props.sample_rate = stream->codec->sample_rate;
	props.format = stream->codec->sample_fmt;
#endif

	if (props.channel_layout == 0) {
		// Use a default channel layout value
		props.channel_layout = av_get_default_channel_layout(channels);
	}

	return props;
}

AudioProperties getAdjustedAudioProps(const AudioProperties& baseProps) {
	AudioProperties adjusted;
	adjusted.sample_rate = baseProps.sample_rate; // Don't adjust sample rate

	adjusted.channel_layout = baseProps.channel_layout;
	if (av_get_channel_layout_nb_channels(baseProps.channel_layout) > 2) {
		adjusted.channel_layout = AV_CH_LAYOUT_STEREO;
	}

	int max_bytes_per_sample;
	switch (Ds_sound_quality) {
		case DS_SQ_HIGH:
			max_bytes_per_sample = Ds_float_supported ? 4 : 2;
			break;

		case DS_SQ_MEDIUM:
			max_bytes_per_sample = 2;
			break;

		default:
			max_bytes_per_sample = 1;
			break;
	}

	// Determine the right format. Downsample if sound quality is low but don't increase size of samples if the source doesn't support it
	auto bytes_per_sample = std::min(av_get_bytes_per_sample(baseProps.format), max_bytes_per_sample);

	switch (bytes_per_sample) {
		case 1:
			adjusted.format = AV_SAMPLE_FMT_U8;
			break;
		case 2:
			adjusted.format = AV_SAMPLE_FMT_S16;
			break;
		case 4:
			adjusted.format = AV_SAMPLE_FMT_FLT;
			break;
		default:
			Assertion(false, "Unhandled switch value!");
			adjusted.format = AV_SAMPLE_FMT_NONE;
			break;
	}

	return adjusted;
}

SwrContext* getSWRContext(const AudioProperties& base, const AudioProperties& adjusted) {
	SwrContext* swr = nullptr;
	swr = swr_alloc_set_opts(swr, adjusted.channel_layout, adjusted.format, adjusted.sample_rate,
							 base.channel_layout, base.format, base.sample_rate, 0, nullptr);

	if (swr_init(swr) < 0) {
		return nullptr;
	}

	return swr;
}
}

namespace ffmpeg {
WaveFile::WaveFile() {
	// Init data members
	m_audioProps = AudioProperties();
	m_ctx.reset();

	m_decodeFrame = av_frame_alloc();
}

WaveFile::~WaveFile() {
	av_frame_free(&m_decodeFrame);

	if (m_audioCodecCtx) {
		avcodec_close(m_audioCodecCtx);
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
		avcodec_free_context(&m_audioCodecCtx);
#endif
		m_audioCodecCtx = nullptr;
	}

	if (m_resampleCtx) {
		swr_free(&m_resampleCtx);
	}

	// Free memory
	m_ctx.reset();

}

// Open
bool WaveFile::Open(const char* pszFilename, bool keep_ext) {
	using namespace libs::ffmpeg;

	size_t FileSize, FileOffset;
	const void* file_data = nullptr;
	char fullpath[MAX_PATH_LEN];
	char filename[MAX_FILENAME_LEN];

	// NOTE: the extension will be removed and set to the actual extension of keep_ext is false, otherwise it's not changed
	strcpy_s(filename, pszFilename);

	try {
		int rc = -1;
		// if we are supposed to load the file as passed...
		if (keep_ext) {
			for (int i = 0; i < NUM_AUDIO_EXT; i++) {
				if (stristr(pszFilename, audio_ext_list[i])) {
					rc = i;
					break;
				}
			}

			// not a supported extension format ... somebody screwed up their tbls :)
			if (rc < 0) {
				throw FFmpegException("Unknown file extension.");
			}

			rc = cf_find_file_location(pszFilename,
									   CF_TYPE_ANY,
									   sizeof(fullpath) - 1,
									   fullpath,
									   &FileSize,
									   &FileOffset,
									   false,
									   &file_data);

			if(rc == 0) {
				throw FFmpegException("File not found.");
			}
		}
		else {
			// ... otherwise we just find the best match
			rc = cf_find_file_location_ext(filename,
										   NUM_AUDIO_EXT,
										   audio_ext_list,
										   CF_TYPE_ANY,
										   sizeof(fullpath) - 1,
										   fullpath,
										   &FileSize,
										   &FileOffset,
										   false,
										   &file_data);

			if (rc < 0) {
				throw FFmpegException("File not found with any known extension.");
			}

			// cf_find_file_location_ext already handles file names with extensions so we also need to handle that here
			auto dot = strrchr(filename, '.');
			if (dot && (strlen(dot) > 2)) {
				(*dot) = 0;
			}

			// set proper filename for later use
			strcat_s(filename, audio_ext_list[rc]);
		}

		auto cfp = cfopen_special(fullpath, "rb", FileSize, FileOffset, file_data, CF_TYPE_ANY);

		if (cfp == NULL) {
			throw FFmpegException("Failed to open file.");
		}

		m_ctx = FFmpegContext::createContext(cfp);
		auto ctx = m_ctx->ctx();

		AVCodec* audio_codec = nullptr;
		m_audioStreamIndex = av_find_best_stream(ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &audio_codec, 0);
		if (m_audioStreamIndex < 0) {
			throw FFmpegException("Failed to find audio stream in file.");
		}
		m_audioStream = ctx->streams[m_audioStreamIndex];

		int err;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
        m_audioCodecCtx = avcodec_alloc_context3(audio_codec);

        // Copy codec parameters from input stream to output codec context
        err = avcodec_parameters_to_context(m_audioCodecCtx, m_audioStream->codecpar);
        if (err < 0) {
            char errorStr[512];
            av_strerror(err, errorStr, sizeof(errorStr));
            throw FFmpegException(errorStr);
        }
#else
		m_audioCodecCtx = m_audioStream->codec;
#endif

		err = avcodec_open2(m_audioCodecCtx, audio_codec, nullptr);
		if (err < 0) {
			char errorStr[512];
			av_strerror(err, errorStr, sizeof(errorStr));
			throw FFmpegException(errorStr);
		}

		m_baseAudioProps = getAudioProps(m_audioStream);

		setAdjustedAudioProperties(getAdjustedAudioProps(m_baseAudioProps));

		if (getALFormat() == AL_INVALID_VALUE) {
			throw FFmpegException("Invalid audio format.");
		}

		nprintf(("SOUND", "SOUND => %s => Using codec %s (%s)\n", filename, audio_codec->long_name, audio_codec->name));
	} catch (const FFmpegException& e) {
		mprintf(("SOUND ==> Could not open wave file %s for streaming. Reason: %s\n", filename, e.what()));
		return false;
	}

	// Cue for streaming
	Cue();
	m_frameReader.reset(new FFmpegAudioReader(m_ctx->ctx(), m_audioCodecCtx, m_audioStreamIndex));

	nprintf(("SOUND", "SOUND => Successfully opened: %s\n", filename));

	// If we are here it means that everything went fine
	return true;
}


bool WaveFile::Cue() {
	auto err = av_seek_frame(m_ctx->ctx(), m_audioStreamIndex, 0, AVSEEK_FLAG_BYTE);

	if (err >= 0) {
		avcodec_flush_buffers(m_audioCodecCtx);
		avformat_flush(m_ctx->ctx());
	}

	return err >= 0;
}

void WaveFile::setAdjustedAudioProperties(const AudioProperties& props) {
	if (m_resampleCtx) {
		swr_free(&m_resampleCtx);
	}
	m_audioProps = props;
	m_resampleCtx = getSWRContext(m_baseAudioProps, m_audioProps);

	Assertion(m_resampleCtx != nullptr, "Resample context creation failed! This should not happen!");
}

size_t WaveFile::handleDecodedFrame(AVFrame* av_frame, uint8_t* out_buffer, size_t buffer_size) {
	const auto sample_size = (av_get_bytes_per_sample(m_audioProps.format) * getNumChannels());

	int dest_num_samples = static_cast<int>(buffer_size / sample_size);
	auto written = swr_convert(m_resampleCtx,
							   &out_buffer,
							   dest_num_samples,
							   (const uint8_t**) av_frame->extended_data,
							   av_frame->nb_samples);


	return (size_t) (written * sample_size);
}

size_t WaveFile::getBufferedData(uint8_t* buffer, size_t buffer_size) {
	// First determine if there is buffered data
	auto buffered = swr_get_out_samples(m_resampleCtx, 0);

	if (buffered > 0) {
		const auto sample_size = (av_get_bytes_per_sample(m_audioProps.format) * getNumChannels());
		int dest_num_samples = static_cast<int>(buffer_size / sample_size);
		auto written = swr_convert(m_resampleCtx, &buffer, dest_num_samples, nullptr, 0);

		auto advance = (size_t) (written * sample_size);
		Assertion(advance <= buffer_size,
				  "Buffer overrun!!! Decoding has written more data into the buffer than available!");

		return advance;
	}
	return 0;
}

int WaveFile::Read(uint8_t* pbDest, size_t cbSize) {
	size_t buffer_pos = 0;
	buffer_pos += getBufferedData(pbDest, cbSize);
	if (buffer_pos == cbSize) {
		return static_cast<int>(buffer_pos);
	}

	while (m_frameReader->readFrame(m_decodeFrame)) {
		// Got a new frame
		auto advance = handleDecodedFrame(m_decodeFrame, pbDest + buffer_pos, cbSize - buffer_pos);

		buffer_pos += advance;
		Assertion(buffer_pos <= cbSize,
				  "Buffer overrun!!! Decoding has written more data into the buffer than available!");

		if (buffer_pos == cbSize) {
			return static_cast<int>(buffer_pos);
		}
	}

	// If we are here then the audio stream is finished, write any buffered data
	buffer_pos += getBufferedData(pbDest + buffer_pos, cbSize - buffer_pos);

	if (buffer_pos == 0) {
		// End of stream has been reached
		return -1;
	}

	return static_cast<int>(buffer_pos);
}

int WaveFile::getSampleByteSize() const {
	return av_get_bytes_per_sample(m_audioProps.format) * getNumChannels();
}

int WaveFile::getSampleRate() const {
	return m_audioProps.sample_rate;
}

double WaveFile::getDuration() const {
	return av_q2d(av_mul_q(av_make_q(static_cast<int>(m_audioStream->duration), 1), m_audioStream->time_base));
}

int WaveFile::getTotalSamples() const {
	auto duration = av_mul_q(av_make_q(static_cast<int>(m_audioStream->duration), 1), m_audioStream->time_base);
	auto samples_r = av_mul_q(av_make_q(m_audioProps.sample_rate, 1), duration);

	// Compute the actual sample number and round the result up
	auto samples = 1 + ((samples_r.num - 1) / samples_r.den);

	return samples;
}

int WaveFile::getNumChannels() const {
	return av_get_channel_layout_nb_channels(m_audioProps.channel_layout);
}
ALenum WaveFile::getALFormat() const {
	return openal_get_format(av_get_bytes_per_sample(m_audioProps.format) * 8, getNumChannels());;
}
}
