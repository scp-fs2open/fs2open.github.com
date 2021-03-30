#include <limits>

#include "cutscene/ffmpeg/FFMPEGDecoder.h"

#include "cfile/cfile.h"
#include "libs/ffmpeg/FFmpegContext.h"

#include "cutscene/ffmpeg/internal.h"

#include "cutscene/ffmpeg/AudioDecoder.h"
#include "cutscene/ffmpeg/VideoDecoder.h"
#include "cutscene/ffmpeg/SubtitleDecoder.h"

#include "FFMPEGDecoder.h"
#include "localization/localize.h"

using namespace libs::ffmpeg;

namespace {
using namespace cutscene;
using namespace cutscene::ffmpeg;

class AVPacketScope
{
	AVPacket* _packet;
 public:
	explicit AVPacketScope(AVPacket* av_packet)
		: _packet(av_packet) {
	}

	AVPacketScope(const AVPacketScope&) = delete;
	AVPacketScope& operator=(const AVPacketScope&) = delete;

	~AVPacketScope() {
		av_packet_unref(_packet);
	}
};

const char* CHECKED_EXTENSIONS[] = {"webm", "mp4", "ogg",
                                    "png", // This is designed to be used with APNG animations
                                    "mve"};

const char* CHECKED_SUBT_EXTENSIONS[] = {"srt"};

double getFrameRate(AVStream* stream, AVCodecContext* codecCtx) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(58, 3, 102)
	auto fps = av_q2d(stream->r_frame_rate);
#else
	auto fps = av_q2d(av_stream_get_r_frame_rate(stream));
#endif

	if (fps < 0.000001)
	{
		fps = av_q2d(stream->avg_frame_rate);
	}

	if (fps < 0.000001)
	{
		fps = 1.0 / av_q2d(codecCtx->time_base);
	}

	return fps;
}

CodecContextParameters getCodecParameters(AVStream* stream) {
	CodecContextParameters paras;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
	paras.width        = stream->codecpar->width;
	paras.height       = stream->codecpar->height;
	paras.pixel_format = (AVPixelFormat)stream->codecpar->format;

	paras.channel_layout = stream->codecpar->channel_layout;
	paras.sample_rate    = stream->codecpar->sample_rate;
	paras.audio_format   = (AVSampleFormat)stream->codecpar->format;

	paras.codec_id = stream->codecpar->codec_id;
#else
	paras.width        = stream->codec->width;
	paras.height       = stream->codec->height;
	paras.pixel_format = stream->codec->pix_fmt;

	paras.channel_layout = stream->codec->channel_layout;
	paras.sample_rate    = stream->codec->sample_rate;
	paras.audio_format   = stream->codec->sample_fmt;

	paras.codec_id = stream->codec->codec_id;
#endif
	return paras;
}

SCP_string normalizeLanguage(const char* langauge_name) {
	if (!stricmp(langauge_name, "eng")) {
		return "English";
	}
	if (!stricmp(langauge_name, "ger")) {
		return "German";
	}
	if (!stricmp(langauge_name, "spa")) {
		return "Spanish";
	}

	// Print to log so that we can find the actual value more easily later
	mprintf(("FFmpeg log: Found unknown language value '%s'!\n", langauge_name));

	// Default to english for everything else
	return "English";
}

/**
 * @brief Given a FFmpeg pixel format returns the format it should be converted to
 * @param in_fmt The input format
 * @return The output format
 */
AVPixelFormat getConversionFormat(AVPixelFormat in_fmt)
{
	switch (in_fmt) {
	case AV_PIX_FMT_RGB24:
	case AV_PIX_FMT_BGR24:
	case AV_PIX_FMT_BGR8:
	case AV_PIX_FMT_BGR4:
	case AV_PIX_FMT_BGR4_BYTE:
	case AV_PIX_FMT_RGB8:
	case AV_PIX_FMT_RGB4:
	case AV_PIX_FMT_RGB4_BYTE:
	case AV_PIX_FMT_RGB48BE:
	case AV_PIX_FMT_RGB48LE:
	case AV_PIX_FMT_RGB565BE:
	case AV_PIX_FMT_RGB565LE:
	case AV_PIX_FMT_RGB555BE:
	case AV_PIX_FMT_RGB555LE:
	case AV_PIX_FMT_BGR565BE:
	case AV_PIX_FMT_BGR565LE:
	case AV_PIX_FMT_BGR555BE:
	case AV_PIX_FMT_BGR555LE:
	case AV_PIX_FMT_RGB444LE:
	case AV_PIX_FMT_RGB444BE:
	case AV_PIX_FMT_BGR444LE:
	case AV_PIX_FMT_BGR444BE:
	case AV_PIX_FMT_BGR48BE:
	case AV_PIX_FMT_BGR48LE:
	case AV_PIX_FMT_0RGB:
	case AV_PIX_FMT_RGB0:
	case AV_PIX_FMT_0BGR:
	case AV_PIX_FMT_BGR0:
		return AV_PIX_FMT_BGR24;

	case AV_PIX_FMT_RGBA64BE:
	case AV_PIX_FMT_RGBA64LE:
	case AV_PIX_FMT_BGRA64BE:
	case AV_PIX_FMT_BGRA64LE:
	case AV_PIX_FMT_ARGB:
	case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_ABGR:
	case AV_PIX_FMT_BGRA:
		return AV_PIX_FMT_BGRA;

	default:
		// Assume that everything else is converted to YUV 420
		return AV_PIX_FMT_YUV420P;
	}
}

FramePixelFormat getPixelFormat(AVPixelFormat fmt)
{
	switch (fmt) {
	case AV_PIX_FMT_BGR24:
		return FramePixelFormat::BGR;
	case AV_PIX_FMT_BGRA:
		return FramePixelFormat::BGRA;
	default:
		return FramePixelFormat::YUV420;
	}
}
}

namespace cutscene {
namespace ffmpeg {
struct InputStream {
	std::unique_ptr<FFmpegContext> m_ctx;
};

FFMPEGDecoder::FFMPEGDecoder() {
}

FFMPEGDecoder::~FFMPEGDecoder() {
}

namespace {
std::unique_ptr<InputStream> openStream(const SCP_string& name) {
	// Only check the root and movies folders
	int dirType;
	if (cf_exists_full(name.c_str(), CF_TYPE_ROOT)) {
		dirType = CF_TYPE_ROOT;
	} else if (cf_exists_full(name.c_str(), CF_TYPE_MOVIES)) {
		dirType = CF_TYPE_MOVIES;
	} else {
		return nullptr;
	}

	std::unique_ptr<InputStream> input(new InputStream());

	try {
		// Create the FFmpeg context
		input->m_ctx = FFmpegContext::createContext(name, dirType);

		// This may fail, in that case return null
		if (!input->m_ctx) {
			return nullptr;
		}
	}
	catch (const FFmpegException& e) {
		mprintf(("Error opening %s: %s\n", name.c_str(), e.what()));
		return nullptr;
	}

	return input;
}

std::unique_ptr<DecoderStatus> initializeStatus(std::unique_ptr<InputStream>& stream,
                                                std::unique_ptr<InputStream>& subt,
                                                const PlaybackProperties& properties)
{
	if (subt && properties.looping) {
		mprintf(("FFmpeg: External subtitles and looping movies are not supported!\n"));
		return nullptr;
	}

	std::unique_ptr<DecoderStatus> status(new DecoderStatus());

	auto ctx = stream->m_ctx->ctx();

	auto videoStream = av_find_best_stream(ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &status->videoCodec, 0);
	if (videoStream < 0) {
		if (videoStream == AVERROR_STREAM_NOT_FOUND) {
			mprintf(("FFmpeg: No video stream found in file!\n"));
		} else if (videoStream == AVERROR_DECODER_NOT_FOUND) {
			mprintf(("FFmpeg: Codec for video stream could not be found!\n"));
		} else {
			mprintf(("FFmpeg: Unknown error while finding video stream!\n"));
		}

		return nullptr;
	}

	int audioStream = -1;

	if (properties.with_audio) {
		audioStream = av_find_best_stream(ctx, AVMEDIA_TYPE_AUDIO, -1, videoStream, &status->audioCodec, 0);
		if (audioStream < 0) {
			if (audioStream == AVERROR_STREAM_NOT_FOUND) {
				mprintf(("FFmpeg: No audio stream found in file!\n"));
			} else if (audioStream == AVERROR_DECODER_NOT_FOUND) {
				mprintf(("FFmpeg: Codec for audio stream could not be found!\n"));
			} else {
				mprintf(("FFmpeg: Unknown error while finding audio stream!\n"));
			}
		}
	}

	status->videoStreamIndex = videoStream;
	status->videoStream = ctx->streams[videoStream];

	if (audioStream >= 0) {
		status->audioStreamIndex = audioStream;
		status->audioStream = ctx->streams[audioStream];
	}

	if (subt) {
		auto subtCtx = subt->m_ctx->ctx();

		auto subtStream = av_find_best_stream(subtCtx, AVMEDIA_TYPE_SUBTITLE, -1, -1, nullptr, 0);
		if (subtStream < 0) {
			if (subtStream == AVERROR_STREAM_NOT_FOUND) {
				mprintf(("FFmpeg: No subtitle stream found in subtitle file!\n"));
			} else if (subtStream == AVERROR_DECODER_NOT_FOUND) {
				mprintf(("FFmpeg: Codec for subtitle stream could not be found!\n"));
			} else {
				mprintf(("FFmpeg: Unknown error while finding subtitle stream!\n"));
			}
		} else {
			status->subtitleStream      = subtCtx->streams[subtStream];
			status->externalSubtitles   = true;
			status->subtitleStreamIndex = subtStream;
		}
	}

	if (status->subtitleStream == nullptr) {
		// We don't have an external subtitle stream or loading from that file has failed
		auto& current_language = Lcl_languages[lcl_get_current_lang_index()];
		for (uint32_t i = 0; i < ctx->nb_streams; ++i) {
			auto test_stream = ctx->streams[i];

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
			auto& pars = test_stream->codecpar;

			if (!(pars->codec_type == AVMEDIA_TYPE_SUBTITLE)) {
				continue;
			}
#endif
			AVDictionaryEntry* tag = nullptr;
			if ((tag = av_dict_get(test_stream->metadata, "language", nullptr, 0)) == nullptr) {
				continue;
			}

			auto normalized_language = normalizeLanguage(tag->value);

			if (!stricmp(normalized_language.c_str(), current_language.lang_name)) {
				status->subtitleStreamIndex = i;
				status->subtitleStream      = test_stream;
				break;
			}
		}
	}

    status->videoCodecPars = getCodecParameters(status->videoStream);

	if (properties.looping && status->videoCodecPars.codec_id == AV_CODEC_ID_INTERPLAY_VIDEO) {
		mprintf(("FFmpeg: Looping is not supported for inteplay (MVE) movies!\n"));
		return nullptr;
	}

	int err;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
	status->videoCodecCtx = avcodec_alloc_context3(status->videoCodec);

	// Copy codec parameters from input stream to output codec context
	err = avcodec_parameters_to_context(status->videoCodecCtx, status->videoStream->codecpar);
	if (err < 0) {
		char errorStr[512];
		av_strerror(err, errorStr, sizeof(errorStr));
		mprintf(("FFMPEG: Failed to copy context parameters! Error: %s\n", errorStr));
		return nullptr;
	}
#else
	status->videoCodecCtx = status->videoStream->codec;
#endif

	err = avcodec_open2(status->videoCodecCtx, status->videoCodec, nullptr);
	if (err < 0) {
		char errorStr[512];
		av_strerror(err, errorStr, sizeof(errorStr));
		mprintf(("FFMPEG: Failed to open video codec! Error: %s\n", errorStr));
		return nullptr;
	}

	mprintf(("FFmpeg: Using video codec %s (%s).\n", status->videoCodec->long_name ? status->videoCodec->long_name : "<Unknown>",
		status->videoCodec->name ? status->videoCodec->name : "<Unknown>"));

	// Now initialize audio, if this fails it's not a fatal error
	if (audioStream >= 0) {
		status->audioCodecPars = getCodecParameters(status->audioStream);

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
		status->audioCodecCtx = avcodec_alloc_context3(status->audioCodec);

		err = avcodec_parameters_to_context(status->audioCodecCtx, status->audioStream->codecpar);
		if (err < 0) {
			char errorStr[512];
			av_strerror(err, errorStr, sizeof(errorStr));
			mprintf(("FFMPEG: Failed to copy context parameters! Error: %s\n", errorStr));
			return nullptr;
		}
#else
		status->audioCodecCtx = status->audioStream->codec;
#endif

		err = avcodec_open2(status->audioCodecCtx, status->audioCodec, nullptr);
		if (err < 0) {
			char errorStr[512];
			av_strerror(err, errorStr, sizeof(errorStr));
			mprintf(("FFMPEG: Failed to open audio codec! Error: %s\n", errorStr));
		}

		mprintf(("FFmpeg: Using audio codec %s (%s).\n", status->audioCodec->long_name ? status->audioCodec->long_name : "<Unknown>",
			status->audioCodec->name ? status->audioCodec->name : "<Unknown>"));
	}

	if (status->subtitleStream != nullptr) {
		// Not really sure if this makes sense for subtitles but it shouldn't break anything...
		status->subtitleCodecPars = getCodecParameters(status->subtitleStream);

		status->subtitleCodec = avcodec_find_decoder(status->subtitleCodecPars.codec_id);

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
		status->subtitleCodecCtx = avcodec_alloc_context3(status->subtitleCodec);

		err = avcodec_parameters_to_context(status->subtitleCodecCtx, status->subtitleStream->codecpar);
		if (err < 0) {
			char errorStr[512];
			av_strerror(err, errorStr, sizeof(errorStr));
			mprintf(("FFMPEG: Failed to copy context parameters! Error: %s\n", errorStr));
			return nullptr;
		}
#else
		status->subtitleCodecCtx = status->subtitleStream->codec;
#endif

		err = avcodec_open2(status->subtitleCodecCtx, status->subtitleCodec, nullptr);
		if (err < 0) {
			char errorStr[512];
			av_strerror(err, errorStr, sizeof(errorStr));
			mprintf(("FFMPEG: Failed to open subtitle codec! Error: %s\n", errorStr));
		}

		mprintf(("FFmpeg: Using subtitle codec %s (%s).\n",
		         status->subtitleCodec->long_name ? status->subtitleCodec->long_name : "<Unknown>",
		         status->subtitleCodec->name ? status->subtitleCodec->name : "<Unknown>"));
	}

	return status;
}

std::unique_ptr<InputStream> openInputStream(const SCP_string& name) {
	// Check a list of extensions we might use
	// The actual format of the file may be whatever FFmpeg supports
	for (auto ext : CHECKED_EXTENSIONS) {
		auto fileName = name + "." + ext;

		auto input = openStream(fileName);

		if (input) {
			return input;
		}
	}

	return nullptr;
}

std::unique_ptr<InputStream> openSubtitleStream(const SCP_string& name)
{
	auto& current_language = Lcl_languages[lcl_get_current_lang_index()];

	for (auto ext : CHECKED_SUBT_EXTENSIONS) {
		SCP_string fileName;
		if (strlen(current_language.lang_ext) == 0) {
			fileName = name + "." + ext;
		} else {
			fileName = name + "-" + current_language.lang_ext + "." + ext;
		}

		auto input = openStream(fileName);

		if (input) {
			return input;
		}
	}

	return nullptr;
}
}

bool FFMPEGDecoder::initialize(const SCP_string& fileName, const PlaybackProperties& properties)
{
	SCP_string movieName = fileName;
	// First make the file name lower case
	SCP_tolower(movieName);

	// Then remove the extension
	size_t dotPos = movieName.find('.');
	if (dotPos != SCP_string::npos) {
		movieName.resize(dotPos);
	}

	// Try to open the input stream
	auto input = openInputStream(movieName);
	if (!input) {
		return false;
	}

	auto subt = openSubtitleStream(movieName);

	// We now have a valid input stream, try to find the correct streams
	auto status = initializeStatus(input, subt, properties);
	if (!status) {
		return false;
	}

	// Buffer ~ 2 seconds of video and audio
	initializeQueues(static_cast<size_t>(ceil(getFrameRate(status->videoStream, status->videoCodecCtx))) * 2);

	// We're done, now just put the pointer into this
	std::swap(m_input, input);
	std::swap(m_subtitleInput, subt);
	std::swap(m_status, status);
	m_properties = properties;
	return true;
}

MovieProperties FFMPEGDecoder::getProperties() const
{
	MovieProperties props;
	props.size.width = static_cast<size_t>(m_status->videoCodecPars.width);
	props.size.height = static_cast<size_t>(m_status->videoCodecPars.height);

	props.fps = static_cast<float>(getFrameRate(m_status->videoStream, m_status->videoCodecCtx));

	props.pixelFormat = getPixelFormat(getConversionFormat(m_status->videoCodecPars.pixel_format));

	return props;
}

void FFMPEGDecoder::startDecoding() {
	std::unique_ptr<VideoDecoder> videoDecoder(new VideoDecoder(m_status.get(), getConversionFormat(m_status->videoCodecPars.pixel_format)));

	std::unique_ptr<AudioDecoder> audioDecoder;
	std::unique_ptr<SubtitleDecoder> subtitleDecoder;

	if (hasAudio()) {
		audioDecoder.reset(new AudioDecoder(m_status.get()));
	}
	if (hasSubtitles()) {
		subtitleDecoder.reset(new SubtitleDecoder(m_status.get()));
	}

	std::unique_ptr<std::thread> subtitle_thread;
	if (hasExternalSubtitle()) {
		// Start a new thread for the subtitles since otherwise we would need to make sure that neither of the input
		// streames starves while the other waits for more space in the queues
		subtitle_thread.reset(
		    new std::thread([this, &subtitleDecoder]() { runSubtitleDecoder(subtitleDecoder.get()); }));
	}

	auto ctx = m_input->m_ctx->ctx();
	AVPacket packet;
	do {
		if (m_properties.looping) {
			// Only seek if we are looping the movie. The MVE code doesn't support seeking to this function may not be
			// called there
			auto seek_err = av_seek_frame(ctx, -1, 0, AVSEEK_FLAG_BACKWARD);
			if (seek_err < 0) {
				char errorStr[512];
				av_strerror(seek_err, errorStr, sizeof(errorStr));
				mprintf(("FFMPEG: Failed to seek to start of movie file! Error: %s\n", errorStr));
				break;
			}
		}

		videoDecoder->flushBuffers();
		if (audioDecoder) {
			audioDecoder->flushBuffers();
		}
		if (subtitleDecoder) {
			subtitleDecoder->flushBuffers();
		}

		while (isDecoding()) {
			auto read_err = av_read_frame(ctx, &packet);
			AVPacketScope scope(&packet);

			if (read_err < 0) {
				if (read_err == AVERROR_EOF) {
					// Finished reading -> break out of loop
					break;
				} else if (avio_feof(ctx->pb) != 0) {
					// Also EOF!
					break;
				} else {
					// Some kind of other error, try to continue reading
					char errorStr[512];
					av_strerror(read_err, errorStr, sizeof(errorStr));
					mprintf(("FFMPEG: Failed to read frame! Error: %s\n", errorStr));

					// Skip packet
					continue;
				}
			}

			if (packet.stream_index == m_status->videoStreamIndex) {
				videoDecoder->decodePacket(&packet);

				VideoFramePtr ptr;
				while ((ptr = videoDecoder->getFrame()) != nullptr) {
					pushFrameData(std::move(ptr));
				}
			} else if (audioDecoder && packet.stream_index == m_status->audioStreamIndex) {
				audioDecoder->decodePacket(&packet);

				AudioFramePtr ptr;
				while ((ptr = audioDecoder->getFrame()) != nullptr) {
					pushAudioData(std::move(ptr));
				}
			} else if (subtitleDecoder && packet.stream_index == m_status->subtitleStreamIndex) {
				subtitleDecoder->decodePacket(&packet);

				SubtitleFramePtr ptr;
				while ((ptr = subtitleDecoder->getFrame()) != nullptr) {
					pushSubtitleData(std::move(ptr));
				}
			}
		}

		if (isDecoding()) {
			// If we are still alive then read the last frames from the decoders
			videoDecoder->finishDecoding();
			VideoFramePtr video_ptr;
			while ((video_ptr = videoDecoder->getFrame()) != nullptr) {
				pushFrameData(std::move(video_ptr));
			}

			if (audioDecoder) {
				audioDecoder->finishDecoding();
				AudioFramePtr audio_frame;
				while ((audio_frame = audioDecoder->getFrame()) != nullptr) {
					pushAudioData(std::move(audio_frame));
				}
			}
		}
		// If the video is being looped then we start again from the beginning
	} while (m_properties.looping && isDecoding());

	// Stop the decoder first to let the subtitle thread know that it shouldn't continue
	stopDecoder();

	if (subtitle_thread) {
		subtitle_thread->join();
	}
}

bool FFMPEGDecoder::hasAudio() const { return m_status->audioStreamIndex >= 0; }

bool FFMPEGDecoder::hasSubtitles() const { return m_status->subtitleStream != nullptr; }

bool FFMPEGDecoder::hasExternalSubtitle() const { return m_status->externalSubtitles; }

void FFMPEGDecoder::close() {
	if (m_status) {
		m_status = nullptr;
	}

	if (m_input) {
		// This will delete the InputStream pointer and free all data
		m_input = nullptr;
	}

	if (m_subtitleInput) {
		m_subtitleInput = nullptr;
	}
}
void FFMPEGDecoder::runSubtitleDecoder(SubtitleDecoder* decoder)
{
	auto ctx = m_subtitleInput->m_ctx->ctx();
	AVPacket packet;

	while (isDecoding()) {
		auto read_err = av_read_frame(ctx, &packet);
		AVPacketScope scope(&packet);

		if (read_err < 0) {
			if (read_err == AVERROR_EOF) {
				// Finished reading -> break out of loop
				break;
			} else if (avio_feof(ctx->pb) != 0) {
				// Also EOF!
				break;
			} else {
				// Some kind of other error, try to continue reading
				char errorStr[512];
				av_strerror(read_err, errorStr, sizeof(errorStr));
				mprintf(("FFMPEG: Failed to read frame! Error: %s\n", errorStr));

				// Skip packet
				continue;
			}
		}

		if (packet.stream_index == m_status->subtitleStreamIndex) {
			decoder->decodePacket(&packet);

			SubtitleFramePtr ptr;
			while ((ptr = decoder->getFrame()) != nullptr) {
				pushSubtitleData(std::move(ptr));
			}
		}
	}
	if (isDecoding()) {
		// If we are still alive then read the last frames from the decoders
		decoder->finishDecoding();

		SubtitleFramePtr ptr;
		while ((ptr = decoder->getFrame()) != nullptr) {
			pushSubtitleData(std::move(ptr));
		}
	}
}
}
}
