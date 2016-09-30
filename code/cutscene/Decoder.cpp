#include "cutscene/Decoder.h"

#include <memory>

namespace cutscene {
Decoder::Decoder() : m_decoding(true) {
}

Decoder::~Decoder() {
}

bool Decoder::tryPopAudioData(AudioFramePtr& out) {
#if BOOST_VERSION < 105700
    try
    {
        return m_audioQueue->try_pull(out);
    }
    catch (const boost::sync_queue_is_closed&)
    {
        return false;
    }
#else
	auto res = m_audioQueue->try_pull_front(out);
	return res == boost::queue_op_status::success;
#endif
}

bool Decoder::tryPopVideoFrame(VideoFramePtr& out) {
#if BOOST_VERSION < 105700
    try
    {
        return m_videoQueue->try_pull(out);
    }
    catch (const boost::sync_queue_is_closed&)
    {
        return false;
    }
#else
	auto res = m_videoQueue->try_pull_front(out);
	return res == boost::queue_op_status::success;
#endif
}

void Decoder::initializeQueues(size_t queueSize) {
	m_queueSize = queueSize;

	m_videoQueue.reset(new boost::sync_bounded_queue<VideoFramePtr>(m_queueSize));
	m_audioQueue.reset(new boost::sync_bounded_queue<AudioFramePtr>(m_queueSize));
}

void Decoder::stopDecoder() {
	m_decoding = false;

	m_videoQueue->close();
	m_audioQueue->close();
}

bool Decoder::canPushAudioData() {
	return !isAudioQueueFull();
}

void Decoder::pushAudioData(AudioFramePtr&& data) {
	Assertion(data, "Invalid audio data passed!");

	try {
#if BOOST_VERSION < 105700
		m_audioQueue->push(std::move(data));
#else
		m_audioQueue->push_back(std::move(data));
#endif
	}
	catch (boost::sync_queue_is_closed&) {
		// Ignore
	}
}

bool Decoder::canPushVideoData() {
	return !isVideoQueueFull();
}

void Decoder::pushFrameData(VideoFramePtr&& frame) {
	Assertion(frame, "Invalid video data passed!");

	try {
#if BOOST_VERSION < 105700
		m_videoQueue->push(std::move(frame));
#else
		m_videoQueue->push_back(std::move(frame));
#endif
	}
	catch (boost::sync_queue_is_closed&) {
		// Ignore
	}
}
}
