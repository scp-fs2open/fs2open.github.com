
#ifndef _WAVEFILE_H
#define _WAVEFILE_H
#pragma once

#include "globalincs/pstypes.h"
#include "sound/audiostr.h"
#include "sound/openal.h"

#include "libs/ffmpeg/FFmpegContext.h"

#include "osapi/osapi.h"

namespace ffmpeg {

/**
 * @brief Properties of an audio stream
 */
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
class WaveFile
{
	std::unique_ptr<libs::ffmpeg::FFmpegContext> m_ctx;
	
	AudioProperties m_baseAudioProps;
	AudioProperties m_audioProps;

	SwrContext* m_resampleCtx = nullptr;

	int m_audioStreamIndex = -1;
	AVStream* m_audioStream = nullptr;

	AVFrame* m_decodeFrame = nullptr;

	AVCodecContext* m_audioCodecCtx = nullptr;

	std::unique_ptr<FFmpegAudioReader> m_frameReader;

	size_t getBufferedData(uint8_t* buffer, size_t buffer_size);
 public:
	 WaveFile();
	 ~WaveFile();

	// Disallow copying
	WaveFile(const WaveFile&) = delete;
	WaveFile& operator=(const WaveFile&) = delete;

	/**
	 * @brief Opens the specified file for playback
	 *
	 * After this call you can start reading from this file
	 *
	 * @param pszFilename The filename
	 * @param keep_ext @c true if the extension of the specified file name should be kept
	 * @return @c true if the file was succesfully loaded, @c false otherwise
	 */
	bool Open (const char *pszFilename, bool keep_ext = true);

	/**
	 * @brief Prepare file for audio reading
	 *
	 * This reset the file poiner to the start of the stream and reset internal data. Future calls to Read start reading
	 * from the beginning of the file again
	 *
	 * @return @c true if succesfull, @c false otherwise
	 */
	bool Cue ();

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
	int	Read (uint8_t *pbDest, size_t cbSize);

	/**
	 * @brief Gets the OpenAL format of the audio.
	 * @return The OpenAL format.
	 */
	ALenum getALFormat() const;

	/**
	 * @brief Gets the size in bytes of one audio sample
	 * @return The sample byte size
	 */
	int getSampleByteSize() const;

	/**
	 * @brief Gets the amount of samples per second
	 * @return The sample rate
	 */
	int getSampleRate() const;

	/**
	 * @brief Determines length of the file in seconds
	 * @return The length of the audio in the file in seconds
	 */
	double getDuration() const;

	/**
	 * @brief Get total number of samples in the file
	 * @return The total number of samples in the file
	 */
	int getTotalSamples() const;

	/**
	 * @brief Gets number of channels of the audio in the file
	 * @return The number of channels
	 */
	int getNumChannels() const;

	/**
	 * @brief Gets the properties of the audio in the file
	 * @return The audio properties
	 */
	const AudioProperties& getAudioProperties() const { return m_audioProps; }

	/**
	 * @brief Changes the audio format the read functions return
	 *
	 * This will enable resampling of the audio read from the file. This can be used to reduce the number of channels the audio has if it's not supported.
	 *
	 * @param props The desired properties of the output
	 */
	void setAdjustedAudioProperties(const AudioProperties& props);
protected:
	size_t handleDecodedFrame(AVFrame* av_frame, uint8_t* out_buffer, size_t buffer_size);
};

}


#endif // _WAVEFILE_H
