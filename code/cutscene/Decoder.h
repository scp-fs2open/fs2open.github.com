#pragma once

#include <memory>

#include "globalincs/pstypes.h"

#include <boost/thread/sync_bounded_queue.hpp>

namespace cutscene {
struct FrameSize {
	size_t width;
	size_t height;
	size_t stride;
};

class VideoFrame {
 protected:
	VideoFrame() {}

 public:
	virtual ~VideoFrame() {}

	double frameTime;
	int id;

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
 * This needs to be a shared pointer for older boost versions since the sync_bounded_queue didn't support
 * move-only types yet.
 */
template<typename T>
#if BOOST_VERSION < 105700
using FramePtr = std::shared_ptr<T>;
#else
using FramePtr = std::unique_ptr<T>;
#endif

typedef FramePtr<VideoFrame> VideoFramePtr;

struct MovieProperties {
	FrameSize size;

	float fps;
};

struct AudioFrame {
	SCP_vector<short> audioData;

	int channels;

	long rate;
};
typedef FramePtr<AudioFrame> AudioFramePtr;

/**
 * @brief Abstract class for decoding a video or audio stream
 *
 * A decoder maintaines two queues of decoded audio and video frames which are filled from a background thread and
 * retrieved by the main thread.
 */
class Decoder {
 private:
	std::unique_ptr<boost::sync_bounded_queue<VideoFramePtr>> m_videoQueue;
	std::unique_ptr<boost::sync_bounded_queue<AudioFramePtr>> m_audioQueue;

	bool m_decoding;
	size_t m_queueSize;

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

	/**
	 * @brief Ends decoding of the video file
	 */
	virtual void close() = 0;

	bool isAudioQueueFull() { return m_audioQueue->size() == m_queueSize; }

	bool isAudioFrameAvailable() { return !m_audioQueue->empty(); }

	bool tryPopAudioData(AudioFramePtr&);

	bool isVideoQueueFull() { return m_videoQueue->size() == m_queueSize; }

	bool isVideoFrameAvailable() { return !m_videoQueue->empty(); }

	bool tryPopVideoFrame(VideoFramePtr&);

	void stopDecoder();

	bool isDecoding() { return m_decoding; }

 protected:
	void initializeQueues(size_t queueSize);

	bool canPushAudioData();

	void pushAudioData(AudioFramePtr&& data);

	bool canPushVideoData();

	void pushFrameData(VideoFramePtr&& frame);
};
}
