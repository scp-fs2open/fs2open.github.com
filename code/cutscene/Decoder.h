#pragma once

#include <memory>

#include "globalincs/pstypes.h"
#include "utils/boost/syncboundedqueue.h"

namespace cutscene {
struct FrameSize {
	size_t width = 0;
	size_t height = 0;
	size_t stride = 0;
};

class VideoFrame {
 protected:
	VideoFrame() {}

 public:
	virtual ~VideoFrame() {}

	double frameTime = -1.0;
	int id = -1;

	FrameSize ySize;
	FrameSize uvSize;

	struct DataPointers {
		ubyte* y;
		ubyte* u;
		ubyte* v;
	};

	virtual DataPointers getDataPointers() = 0;
};

/**
 * @brief Pointer type for passing frame data between threads
 *
 */
template<typename T>
using FramePtr = std::unique_ptr<T>;

typedef std::unique_ptr<VideoFrame> VideoFramePtr;

struct MovieProperties {
	FrameSize size;

	float fps = -1.0f;
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

	Decoder(const Decoder&) SCP_DELETED_FUNCTION;

	Decoder& operator=(const Decoder&) SCP_DELETED_FUNCTION;
 protected:
	Decoder();

 public:

	virtual ~Decoder();

	/**
	 * @brief Initializes the decoder
	 *
	 * @note A implementation should initialize the datastructures required for decoding here.
	 *
	 * @param fileName The name of the file that should be opened
	 * @return @c true if the initialization was successfull, @c false otherwise
	 */
	virtual bool initialize(const SCP_string& fileName) = 0;

	/**
	 * @brief Returns the properties of the video
	 * @return The properties
	 */
	virtual MovieProperties getProperties() = 0;

	/**
	 * @brief Starts decoding the video
	 */
	virtual void startDecoding() = 0;

	/**
	 * @brief Determines if the video has audio
	 * @return @c true if audio is included in the video
	 */
	virtual bool hasAudio() = 0;

	virtual bool hasSubtitles() = 0;

	/**
	 * @brief Ends decoding of the video file
	 */
	virtual void close() = 0;

	bool isAudioQueueFull() { return m_audioQueue->size() == m_queueSize; }

	bool isAudioFrameAvailable() { return !m_audioQueue->empty(); }

	size_t getAudioQueueSize() { return m_audioQueue->size(); }

	bool tryPopAudioData(AudioFramePtr&);

	bool isSubtitleQueueFull() { return m_subtitleQueue->size() == m_queueSize; }

	bool isSubtitleFrameAvailable() { return !m_subtitleQueue->empty(); }

	size_t getSubtitleQueueSize() { return m_subtitleQueue->size(); }

	bool tryPopSubtitleData(SubtitleFramePtr&);

	bool isVideoQueueFull() { return m_videoQueue->size() == m_queueSize; }

	bool isVideoFrameAvailable() { return !m_videoQueue->empty(); }

	size_t getVideoQueueSize() { return m_videoQueue->size(); }

	bool tryPopVideoFrame(VideoFramePtr&);

	void stopDecoder();

	bool isDecoding() { return m_decoding; }

 protected:
	void initializeQueues(size_t queueSize);

	bool canPushAudioData();

	void pushAudioData(AudioFramePtr&& data);

	bool canPushSubtitleData();

	void pushSubtitleData(SubtitleFramePtr&& data);

	bool canPushVideoData();

	void pushFrameData(VideoFramePtr&& frame);
};
}
