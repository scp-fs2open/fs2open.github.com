#pragma once

#include "globalincs/pstypes.h"
#include "libs/ffmpeg/FFmpegContext.h"
#include "osapi/osapi.h"
#include "sound/IAudioFile.h"
#include "sound/audiostr.h"
#include "sound/openal.h"

namespace sound {
namespace ffmpeg {

struct AudioProperties
{
	int sample_rate = -1;
	int64_t channel_layout = -1;
	AVSampleFormat format = AV_SAMPLE_FMT_NONE;
};

// Forward declaration
class FFmpegAudioReader;

/**
 * @brief A class responsible for reading auto data from media files
 *
 * Contrary to its name it doesn't only support Wave files but can also play Ogg files.
 *
 * @details This uses FFmpeg to decode the audio files.
 */
class FFmpegWaveFile : public IAudioFile {
	std::unique_ptr<libs::ffmpeg::FFmpegContext> m_ctx;

	AudioProperties m_baseAudioProps;
	AudioProperties m_audioProps;

	SwrContext* m_resampleCtx = nullptr;

	int m_audioStreamIndex  = -1;
	AVStream* m_audioStream = nullptr;

	AVFrame* m_decodeFrame = nullptr;

	AVCodecContext* m_audioCodecCtx = nullptr;

	std::unique_ptr<FFmpegAudioReader> m_frameReader;

	size_t getBufferedData(uint8_t* buffer, size_t buffer_size);

  public:
	FFmpegWaveFile();
	~FFmpegWaveFile() override;

	// Disallow copying
	FFmpegWaveFile(const FFmpegWaveFile&) = delete;
	FFmpegWaveFile& operator=(const FFmpegWaveFile&) = delete;

	/**
	 * @brief Opens the specified file for playback
	 *
	 * After this call you can start reading from this file
	 *
	 * @param pszFilename The filename
	 * @param keep_ext @c true if the extension of the specified file name should be kept
	 * @return @c true if the file was succesfully loaded, @c false otherwise
	 */
	bool Open(const char* pszFilename, bool keep_ext = true) override;
	
	/**
	 * @brief Sets up the given in-memory "soundfile"
	 *
	 * @param snddata The sound
 	 * @param snd_len The sound's length
	 * @return @c true if the sound was succesfully loaded, @c false otherwise
	 */
	bool OpenMem(const uint8_t* snddata, size_t snd_len) override;

	/**
	 * @brief Prepare file for audio reading
	 *
	 * This reset the file poiner to the start of the stream and reset internal data. Future calls to Read start reading
	 * from the beginning of the file again
	 *
	 * @return @c true if succesfull, @c false otherwise
	 */
	bool Cue() override;

	/**
	 * @brief Read audio data into a buffer
	 *
	 * Reads up to cbSize bytes of audio data into the buffer. cbSize must be a multiple of the size of one sample
	 *
	 * @param pbDest The buffer to write data to
	 * @param cbSize The size of the buffer
	 * @return The number of bytes written into the buffer. Can be 0 if no data is available, try a bigger buffer.
	 * Returns -1 when the end of the stream or an error has been encountered
	 */
	int Read(uint8_t* pbDest, size_t cbSize) override;

	AudioFileProperties getFileProperties() override;

	void setResamplingProperties(const ResampleProperties& resampleProps) override;

  private:
    const AVCodec* prepareOpened();

	size_t handleDecodedFrame(AVFrame* av_frame, uint8_t* out_buffer, size_t buffer_size);

	int getTotalSamples() const;

	int getNumChannels() const;

	/**
	 * @brief Changes the audio format the read functions return
	 *
	 * This will enable resampling of the audio read from the file. This can be used to reduce the number of channels the audio has if it's not supported.
	 *
	 * @param props The desired properties of the output
	 */
	void setAdjustedAudioProperties(const AudioProperties& props);

};

} // namespace ffmpeg
} // namespace sound
