//
//

#include "FFmpegWaveFile.h"

#include "FFmpegAudioReader.h"

#include "sound/ds.h"

// -- from parselo.cpp --
extern const char* stristr(const char* str, const char* substr);

namespace {
using namespace sound;
using namespace sound::ffmpeg;

AudioProperties getAudioProps(AVStream* stream)
{
	AudioProperties props;

	int channels;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
	channels             = stream->codecpar->channels;
	props.channel_layout = stream->codecpar->channel_layout;
	props.sample_rate    = stream->codecpar->sample_rate;
	props.format         = (AVSampleFormat)stream->codecpar->format;
#else
	channels             = stream->codec->channels;
	props.channel_layout = stream->codec->channel_layout;
	props.sample_rate    = stream->codec->sample_rate;
	props.format         = stream->codec->sample_fmt;
#endif

	if (props.channel_layout == 0) {
		// Use a default channel layout value
		props.channel_layout = av_get_default_channel_layout(channels);
	}

	return props;
}

AudioProperties getAdjustedAudioProps(const AudioProperties& baseProps)
{
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

	// Determine the right format. Downsample if sound quality is low but don't increase size of samples if the source
	// doesn't support it
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
		UNREACHABLE("Unhandled switch value!");
		adjusted.format = AV_SAMPLE_FMT_NONE;
		break;
	}

	return adjusted;
}

SwrContext* getSWRContext(const AudioProperties& base, const AudioProperties& adjusted)
{
	SwrContext* swr = nullptr;
	swr = swr_alloc_set_opts(swr, adjusted.channel_layout, adjusted.format, adjusted.sample_rate, base.channel_layout,
							 base.format, base.sample_rate, 0, nullptr);

	if (swr_init(swr) < 0) {
		return nullptr;
	}

	return swr;
}
} // namespace

namespace sound {
namespace ffmpeg {
FFmpegWaveFile::FFmpegWaveFile()
{
	// Init data members
	m_audioProps = AudioProperties();
	m_ctx.reset();

	m_decodeFrame = av_frame_alloc();
}

FFmpegWaveFile::~FFmpegWaveFile()
{
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

const AVCodec* sound::ffmpeg::FFmpegWaveFile::prepareOpened()
{
    using namespace libs::ffmpeg;
    auto ctx = m_ctx->ctx();

    const AVCodec* audio_codec = nullptr;

    m_audioStreamIndex   = av_find_best_stream(ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (m_audioStreamIndex < 0) {
        throw FFmpegException("Failed to find audio stream in file.");
    }
    m_audioStream = ctx->streams[m_audioStreamIndex];

	audio_codec = avcodec_find_decoder(m_audioStream->codecpar->codec_id);

	if ( !audio_codec ) {
		throw FFmpegException("Failed to find decoder for audio stream in file.");
	}

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

    return audio_codec;
}

// Open
bool FFmpegWaveFile::Open(const char* pszFilename, bool keep_ext)
{
	using namespace libs::ffmpeg;

	char filename[MAX_FILENAME_LEN];

	// NOTE: the extension will be removed and set to the actual extension of keep_ext is false, otherwise it's not
	// changed
	strcpy_s(filename, pszFilename);

	try {
		CFILE* cfp = nullptr;
		// if we are supposed to load the file as passed...
		if (keep_ext) {
			auto known_format = false;
			for (int i = 0; i < NUM_AUDIO_EXT; i++) {
				if (stristr(pszFilename, audio_ext_list[i])) {
					known_format = true;
					break;
				}
			}

			// not a supported extension format ... somebody screwed up their tbls :)
			if (!known_format) {
				throw FFmpegException("Unknown file extension.");
			}

			auto res = cf_find_file_location(pszFilename, CF_TYPE_ANY);

			if (!res.found) {
				throw FFmpegException("File not found.");
			}

			cfp = cfopen_special(res, "rb", CF_TYPE_ANY);
		} else {
			// ... otherwise we just find the best match
			auto res = cf_find_file_location_ext(filename, NUM_AUDIO_EXT, audio_ext_list, CF_TYPE_ANY);

			if (!res.found) {
				throw FFmpegException("File not found with any known extension.");
			}

			// set proper filename for later use
			strcpy_s(filename, res.name_ext.c_str());

			cfp = cfopen_special(res, "rb", CF_TYPE_ANY);
		}

		if (cfp == NULL) {
			throw FFmpegException("Failed to open file.");
		}

		m_ctx    = FFmpegContext::createContext(cfp);

        const AVCodec* audio_codec = prepareOpened();

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

// OpenMem
bool FFmpegWaveFile::OpenMem(const uint8_t* snddata, size_t snd_len)
{
    using namespace libs::ffmpeg;

	try {
		m_ctx    = FFmpegContext::createContextMem(snddata, snd_len);

        const AVCodec* audio_codec = prepareOpened();

		nprintf(("SOUND", "SOUND => %s => Using codec %s (%s)\n", "soundfile-in-memory", audio_codec->long_name, audio_codec->name));
	} catch (const FFmpegException& e) {
		mprintf(("SOUND ==> Could not open wave file %s for streaming. Reason: %s\n", "soundfile-in-memory", e.what()));
		return false;
	}

	// Cue for streaming
	Cue();
	m_frameReader.reset(new FFmpegAudioReader(m_ctx->ctx(), m_audioCodecCtx, m_audioStreamIndex));

	nprintf(("SOUND", "SOUND => Successfully opened: %s\n", "soundfile-in-memory"));

	// If we are here it means that everything went fine
	return true;
}

bool FFmpegWaveFile::Cue()
{
	auto err = av_seek_frame(m_ctx->ctx(), m_audioStreamIndex, 0, AVSEEK_FLAG_BYTE);

	if (err >= 0) {
		avcodec_flush_buffers(m_audioCodecCtx);
		avformat_flush(m_ctx->ctx());
	}

	return err >= 0;
}

void FFmpegWaveFile::setAdjustedAudioProperties(const AudioProperties& props)
{
	if (m_resampleCtx) {
		swr_free(&m_resampleCtx);
	}
	m_audioProps  = props;
	m_resampleCtx = getSWRContext(m_baseAudioProps, m_audioProps);

	Assertion(m_resampleCtx != nullptr, "Resample context creation failed! This should not happen!");
}

size_t FFmpegWaveFile::handleDecodedFrame(AVFrame* av_frame, uint8_t* out_buffer, size_t buffer_size)
{
	const auto sample_size = (av_get_bytes_per_sample(m_audioProps.format) * getNumChannels());

	int dest_num_samples = static_cast<int>(buffer_size / sample_size);
	auto written = swr_convert(m_resampleCtx, &out_buffer, dest_num_samples, (const uint8_t**)av_frame->extended_data,
							   av_frame->nb_samples);

	return (size_t)(written * sample_size);
}

size_t FFmpegWaveFile::getBufferedData(uint8_t* buffer, size_t buffer_size)
{
	// First determine if there is buffered data
	auto buffered = swr_get_out_samples(m_resampleCtx, 0);

	if (buffered > 0) {
		const auto sample_size = (av_get_bytes_per_sample(m_audioProps.format) * getNumChannels());
		int dest_num_samples   = static_cast<int>(buffer_size / sample_size);
		auto written           = swr_convert(m_resampleCtx, &buffer, dest_num_samples, nullptr, 0);

		auto advance = (size_t)(written * sample_size);
		Assertion(advance <= buffer_size,
				  "Buffer overrun!!! Decoding has written more data into the buffer than available!");

		return advance;
	}
	return 0;
}

int FFmpegWaveFile::Read(uint8_t* pbDest, size_t cbSize)
{
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

int FFmpegWaveFile::getTotalSamples() const
{
	auto duration  = av_mul_q(av_make_q(static_cast<int>(m_audioStream->duration), 1), m_audioStream->time_base);
	auto samples_r = av_mul_q(av_make_q(m_audioProps.sample_rate, 1), duration);

	// Compute the actual sample number and round the result up
	auto samples = 1 + ((samples_r.num - 1) / samples_r.den);

	return samples;
}

int FFmpegWaveFile::getNumChannels() const { return av_get_channel_layout_nb_channels(m_audioProps.channel_layout); }

AudioFileProperties FFmpegWaveFile::getFileProperties()
{
	AudioFileProperties props{};
	props.num_channels = getNumChannels();
	props.duration =
		av_q2d(av_mul_q(av_make_q(static_cast<int>(m_audioStream->duration), 1), m_audioStream->time_base));
	props.total_samples    = getTotalSamples();
	props.sample_rate      = m_audioProps.sample_rate;
	props.bytes_per_sample = av_get_bytes_per_sample(m_audioProps.format);
	return props;
}
void FFmpegWaveFile::setResamplingProperties(const ResampleProperties& resampleProps)
{
	using namespace libs::ffmpeg;

	auto current = m_audioProps;

	if (getNumChannels() == resampleProps.num_channels) {
		// No need to resample
		return;
	}

	if (resampleProps.num_channels == 1) {
		current.channel_layout = AV_CH_LAYOUT_MONO;

		setAdjustedAudioProperties(current);
	} else {
		throw FFmpegException("Number of channels to resample to is not supported!");
	}
}

} // namespace ffmpeg
} // namespace sound
