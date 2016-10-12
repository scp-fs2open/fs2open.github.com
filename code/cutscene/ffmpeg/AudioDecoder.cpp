#include "cutscene/ffmpeg/AudioDecoder.h"

namespace {
const int OUT_CH_LAYOUT = AV_CH_LAYOUT_STEREO;
const int OUT_SAMPLE_RATE = 48000;
const AVSampleFormat OUT_SAMPLE_FORMAT = AV_SAMPLE_FMT_S16;
const int OUT_NUM_CHANNELS = av_get_channel_layout_nb_channels(OUT_CH_LAYOUT);

const int DEFAULT_SRC_NUM_SAMPLES = 1024;

SwrContext* getSWRContext(uint64_t layout, int rate, AVSampleFormat inFmt) {
	auto swr = swr_alloc();

	av_opt_set_int(swr, "in_channel_layout", layout, 0);
	av_opt_set_int(swr, "in_sample_rate", rate, 0);
	av_opt_set_int(swr, "in_sample_fmt", inFmt, 0);

	av_opt_set_int(swr, "out_channel_layout", OUT_CH_LAYOUT, 0);
	av_opt_set_int(swr, "out_sample_rate", OUT_SAMPLE_RATE, 0);
	av_opt_set_int(swr, "out_sample_fmt", OUT_SAMPLE_FORMAT, 0);

	swr_init(swr);

	return swr;
}

int alloc_array_and_samples(uint8_t*** outData, int* linesize, int channels, int samples, AVSampleFormat format) {
	auto planes = av_sample_fmt_is_planar(format) ? channels : 1;

	*outData = reinterpret_cast<uint8_t**>(av_mallocz(planes * sizeof(**outData)));


	auto ret = av_samples_alloc(*outData, linesize, channels, samples, format, 0);

	if (ret < 0) {
		av_freep(outData);
	}

	return ret;
}

int64_t getDelay(SwrContext* ctx, int64_t sampleRate) {
#ifdef WITH_LIBAV
	return avresample_get_delay(ctx);
#else
	return swr_get_delay(ctx, sampleRate);
#endif
}

int resample_convert(SwrContext* ctx, uint8_t** output,
					 int out_plane_size, int out_samples, uint8_t** input,
					 int in_plane_size, int in_samples) {
#ifdef WITH_LIBAV
	return avresample_convert(ctx, output, out_plane_size, out_samples, input, in_plane_size, in_samples);
#else
	return swr_convert(ctx, output, out_samples, (const uint8_t**) input, in_samples);
#endif
}
}

namespace cutscene {
namespace ffmpeg {
AudioDecoder::AudioDecoder(DecoderStatus* status)
	: FFMPEGStreamDecoder(status) {
	m_audioBuffer.reserve(static_cast<size_t>(OUT_SAMPLE_RATE * OUT_NUM_CHANNELS / 2));

	m_resampleCtx = getSWRContext(m_status->audioCodecPars.channel_layout, m_status->audioCodecPars.sample_rate,
								  m_status->audioCodecPars.audio_format);

	/*
    * compute the number of converted samples: buffering is avoided
    * ensuring that the output buffer will contain at least all the
    * converted input samples
    */
	m_maxOutNumSamples = m_outNumSamples = static_cast<int>(av_rescale_rnd(DEFAULT_SRC_NUM_SAMPLES,
																		   OUT_SAMPLE_RATE,
																		   m_status->audioCodecPars.sample_rate,
																		   AV_ROUND_UP));

	auto ret = alloc_array_and_samples(&m_outData, &m_outLinesize, OUT_NUM_CHANNELS,
									   m_outNumSamples, OUT_SAMPLE_FORMAT);

	if (ret < 0) {
		mprintf(("FFMPEG: Failed to allocate samples array!\n"));
	}
}

AudioDecoder::~AudioDecoder() {
	if (m_outData) {
		av_freep(&m_outData[0]);
	}
	av_freep(&m_outData);

	swr_free(&m_resampleCtx);
}

void AudioDecoder::flushAudioBuffer() {
	if (m_audioBuffer.empty()) {
		// Nothing to do here
		return;
	}

	AudioFramePtr audioFrame(new AudioFrame());
	audioFrame->channels = OUT_NUM_CHANNELS;
	audioFrame->rate = OUT_SAMPLE_RATE;

	audioFrame->audioData.assign(m_audioBuffer.begin(), m_audioBuffer.end());

	pushFrame(std::move(audioFrame));
	m_audioBuffer.clear();
}

void AudioDecoder::handleDecodedFrame(AVFrame* frame) {
	/* compute destination number of samples */
	m_outNumSamples = static_cast<int>(av_rescale_rnd(getDelay(m_resampleCtx, frame->sample_rate)
														  + frame->nb_samples, OUT_SAMPLE_RATE, frame->sample_rate,
													  AV_ROUND_UP));

	if (m_outNumSamples > m_maxOutNumSamples) {
		av_freep(&m_outData[0]);
		auto ret = av_samples_alloc(m_outData, &m_outLinesize, OUT_NUM_CHANNELS, m_outNumSamples,
									OUT_SAMPLE_FORMAT, 1);
		if (ret < 0) {
			mprintf(("FFMPEG: Failed to allocate samples!!!"));
			return;
		}

		m_maxOutNumSamples = m_outNumSamples;
	}

	/* convert to destination format */
	auto ret = resample_convert(m_resampleCtx, m_outData, 0, m_outNumSamples,
								(uint8_t**) frame->data, 0, frame->nb_samples);
	if (ret < 0) {
		mprintf(("FFMPEG: Error while converting audio!\n"));
		return;
	}

	auto outBufsize = av_samples_get_buffer_size(&m_outLinesize, OUT_NUM_CHANNELS, ret, OUT_SAMPLE_FORMAT, 1);
	if (outBufsize < 0) {
		mprintf(("FFMPEG: Could not get sample buffer size!\n"));
		return;
	}

	auto begin = reinterpret_cast<short*>(m_outData[0]);
	auto end = reinterpret_cast<short*>(m_outData[0] + outBufsize);

	auto size = std::distance(begin, end);
	auto newSize = m_audioBuffer.size() + size;

	if (newSize <= m_audioBuffer.capacity()) {
		// We haven't filled the buffer yet
		m_audioBuffer.insert(m_audioBuffer.end(), begin, end);
	} else {
		flushAudioBuffer();
		m_audioBuffer.assign(begin, end);
	}
}

void AudioDecoder::decodePacket(AVPacket* packet) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
	int send_result;
	do {
		send_result = avcodec_send_packet(m_status->audioCodecCtx, packet);

		while (avcodec_receive_frame(m_status->audioCodecCtx, m_decodeFrame) == 0) {
			handleDecodedFrame(m_decodeFrame);
		}
	} while (send_result == AVERROR(EAGAIN));
#else
	int finishedFrame = 0;
	auto err = avcodec_decode_audio4(m_status->audioCodecCtx, m_decodeFrame, &finishedFrame, packet);

	if (err >= 0 && finishedFrame) {
		handleDecodedFrame(m_decodeFrame);
	}
#endif
}

void AudioDecoder::finishDecoding() {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(57, 24, 255)
	// Send flush packet
	avcodec_send_packet(m_status->audioCodecCtx, nullptr);

	// Handle those decoders that have a delay
	while (true) {
		auto ret = avcodec_receive_frame(m_status->audioCodecCtx, m_decodeFrame);

		if (ret == 0) {
			handleDecodedFrame(m_decodeFrame);
		}
		else {
			// Everything consumed or error
			break;
		}
	}
#else
    // Handle those decoders that have a delay
	AVPacket nullPacket;
	memset(&nullPacket, 0, sizeof(nullPacket));
	nullPacket.data = nullptr;
	nullPacket.size = 0;

	while (true) {
		int finishedFrame = 1;
		auto err = avcodec_decode_audio4(m_status->audioCodecCtx, m_decodeFrame, &finishedFrame, &nullPacket);

		if (err < 0 || !finishedFrame) {
			break;
		}

		handleDecodedFrame(m_decodeFrame);
	}
#endif

	// Push the last bits of audio data into the queue
	flushAudioBuffer();
}
}
}
