#pragma once

#include <memory>

#include "globalincs/pstypes.h"
#include "utils/boost/syncboundedqueue.h"

namespace cutscene {
struct FrameSize {
	size_t width = 0;
	size_t height = 0;
	size_t stride = 0;

	FrameSize();
	FrameSize(size_t width, size_t in_height, size_t in_stride);
};

class VideoFrame {
 protected:
	VideoFrame() = default;

  public:
	virtual ~VideoFrame() = default;

	double frameTime = -1.0;
	int id = -1;

	virtual size_t getPlaneNumber() = 0;

	virtual FrameSize getPlaneSize(size_t plane) = 0;

	virtual void* getPlaneData(size_t plane) = 0;
};

/**
 * @brief Pointer type for passing frame data between threads
 *
 */
template<typename T>
using FramePtr = std::unique_ptr<T>;

typedef std::unique_ptr<VideoFrame> VideoFramePtr;

enum class FramePixelFormat {
	Invalid,
	YUV420,
	BGR,
	BGRA
};

struct MovieProperties {
	FrameSize size;

	float fps = -1.0f;

	FramePixelFormat pixelFormat = FramePixelFormat::Invalid;
};

struct AudioFrame {
	SCP_vector<short> audioData;

	int channels = -1;

	int rate = -1;
};
typedef std::unique_ptr<AudioFrame> AudioFramePtr;

struct SubtitleFrame {
    double displayStartTime = -1.0;
    double displayEndTime = -1.0;

	SCP_string text;
};
typedef std::unique_ptr<SubtitleFrame> SubtitleFramePtr;

struct PlaybackProperties {
	bool with_audio = true;
	bool looping = false;
};

/**
 * @brief Abstract class for decoding a video or audio stream
 *
 * A decoder maintaines two queues of decoded audio and video frames which are filled from a background thread and
 * retrieved by the main thread.
 */
class Decoder {
 private:
	std::unique_ptr<sync_bounded_queue<VideoFramePtr>> m_videoQueue;
	std::unique_ptr<sync_bounded_queue<AudioFramePtr>> m_audioQueue;
	std::unique_ptr<sync_bounded_queue<SubtitleFramePtr>> m_subtitleQueue;

	bool m_decoding;
	size_t m_queueSize = 0;

 protected:
	Decoder();

 public:
	Decoder(const Decoder&) = delete;
	Decoder& operator=(const Decoder&) = delete;

	virtual ~Decoder();

	/**
	 * @brief Initializes the decoder
	 *
	 * @note A implementation should initialize the datastructures required for decoding here.
	 *
	 * @param fileName The name of the file that should be opened
	 * @return @c true if the initialization was successfull, @c false otherwise
	 */
	virtual bool initialize(const SCP_string& fileName, const PlaybackProperties& properties) = 0;

	/**
	 * @brief Returns the properties of the video
	 * @return The properties
	 */
	virtual MovieProperties getProperties() const = 0;

	/**
	 * @brief Starts decoding the video
	 */
	virtual void startDecoding() = 0;

	/**
	 * @brief Determines if the video has audio
	 * @return @c true if audio is included in the video
	 */
	virtual bool hasAudio() const = 0;

	virtual bool hasSubtitles() const = 0;

	/**
	 * @brief Ends decoding of the video file
	 */
	virtual void close() = 0;

	bool isAudioQueueFull() { return m_audioQueue->size() == m_queueSize; }

	bool isAudioFrameAvailable() { return !m_audioQueue->empty(); }

	size_t getAudioQueueSize() { return m_audioQueue->size(); }

	bool tryPopAudioData(AudioFramePtr&);

	bool isSubtitleQueueFull() { return m_subtitleQueue->size() == m_queueSize; }

	bool tryPopSubtitleData(SubtitleFramePtr&);

	bool isVideoQueueFull() { return m_videoQueue->size() == m_queueSize; }

	bool isVideoFrameAvailable() { return !m_videoQueue->empty(); }

	size_t getVideoQueueSize() { return m_videoQueue->size(); }

	bool tryPopVideoFrame(VideoFramePtr&);

	void stopDecoder();

	bool isDecoding() { return m_decoding; }

 protected:
	void initializeQueues(size_t queueSize);

	void pushAudioData(AudioFramePtr&& data);

	void pushSubtitleData(SubtitleFramePtr&& data);

	void pushFrameData(VideoFramePtr&& frame);
};
}
