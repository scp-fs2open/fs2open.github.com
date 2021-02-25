#pragma once

#include "globalincs/vmallocator.h"

namespace sound {

/**
 * @brief Properties of an audio file
 */
struct AudioFileProperties {
	int bytes_per_sample = -1;

	double duration = -1.0;

	int total_samples = -1;

	int num_channels = -1;

	int sample_rate = -1;
};

struct ResampleProperties {
	int num_channels = -1;
};


/**
 * @brief An audio file from which decoded audio data can be read
 */
class IAudioFile {
  public:
	virtual ~IAudioFile() = default;

	/**
	 * @brief Opens the specified file for playback
	 *
	 * After this call you can start reading from this file
	 *
	 * @param pszFilename The filename
	 * @param keep_ext @c true if the extension of the specified file name should be kept
	 * @return @c true if the file was succesfully loaded, @c false otherwise
	 */
	virtual bool Open(const char* pszFilename, bool keep_ext = true) = 0;
	
	/**
	 * @brief Sets up the given in-memory "soundfile"
	 *
	 * @param snddata The sound
 	 * @param snd_len The sound's length
	 * @return @c true if the sound was succesfully loaded, @c false otherwise
	 */
	virtual bool OpenMem(const uint8_t* snddata, size_t snd_len) = 0;
	
	/**
	 * @brief Prepare file for audio reading
	 *
	 * This reset the file poiner to the start of the stream and reset internal data. Future calls to Read start reading
	 * from the beginning of the file again
	 *
	 * @return @c true if succesfull, @c false otherwise
	 */
	virtual bool Cue() = 0;

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
	virtual int Read(uint8_t* pbDest, size_t cbSize) = 0;

	/**
	 * @brief Gets properties related to the audio file
	 * @return A struct containing various file related members
	 */
	virtual AudioFileProperties getFileProperties() = 0;

	/**
	 * @brief Sets properties for resampling audio data before returning audio data
	 * @param resampleProps The properties for resampling
	 */
	virtual void setResamplingProperties(const ResampleProperties& resampleProps) = 0;
};

} // namespace sound
