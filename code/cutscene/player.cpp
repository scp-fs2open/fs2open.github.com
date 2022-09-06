#include "cutscene/player.h"

#include "cutscene/Decoder.h"
#include "cutscene/VideoPresenter.h"
#include "cutscene/ffmpeg/FFMPEGDecoder.h"

#include "graphics/2d.h"

#include "globalincs/alphacolors.h"

#include "osapi/osapi.h"

#include "sound/openal.h"

#include "tracing/tracing.h"

#include "io/key.h"
#include "io/timer.h"

#include "player.h"
#include "parse/parselo.h"

using namespace cutscene::player;

namespace {
using namespace cutscene;

const int MAX_AUDIO_BUFFERS = 15;

std::unique_ptr<Decoder> findDecoder(__UNUSED const SCP_string& name, __UNUSED const PlaybackProperties& properties) {
#ifdef WITH_FFMPEG
	{
		std::unique_ptr<Decoder> ffmpeg(new ::cutscene::ffmpeg::FFMPEGDecoder());
		if (ffmpeg->initialize(name, properties)) {
			return ffmpeg;
		}
	}
#endif

	return nullptr;
}

// get relative time since beginning playback, compensating for A/V drift
double playbackGetTime(PlayerState* state) {
	return state->playback_time * 0.000001;
}

void videoPlaybackInit(PlayerState* state) {
	if (state->videoInited) {
		return;
	}

	Assert(state != NULL);

	if (gr_screen.mode != GR_STUB) {
		// The video presenter is independent of the underlying graphics API
		state->videoPresenter.reset(new VideoPresenter(state->props));
	}

	state->videoInited = true;
}

void audioPlaybackInit(PlayerState* state) {
	if (state->audioInited) {
		return;
	}

	if (!state->decoder->hasAudio() || Cmdline_freespace_no_sound) {
		state->hasAudio = false;
		return;
	}
	state->hasAudio = true;

	OpenAL_ErrorCheck(alGenSources(1, &state->audioSid), return);
	OpenAL_ErrorPrint(alSourcef(state->audioSid, AL_GAIN, 1.0f));

	state->audioBuffers.clear();
	state->audioBuffers.resize(MAX_AUDIO_BUFFERS, 0);

	OpenAL_ErrorCheck(alGenBuffers(static_cast<ALsizei>(state->audioBuffers.size()), state->audioBuffers.data()),
					  return);

	for (auto buffer : state->audioBuffers) {
		state->unqueuedAudioBuffers.push(buffer);
	}

	state->audioInited = true;
}

void processVideoData(PlayerState* state) {
	TRACE_SCOPE(tracing::CutsceneProcessVideoData);

	state->newFrameAdded = false;

	if (!state->decoder->isVideoFrameAvailable()) {
		// Nothing to do here...
		return;
	}

	if (state->currentFrame == nullptr && state->nextFrame == nullptr) {
		// Load the initial frame
		VideoFramePtr firstFrame;
		auto r = state->decoder->tryPopVideoFrame(firstFrame);

		// This shouldn't happen...
		Assertion(r, "Failed to pop frame!");

		// At this point the new frame is the next frame
		state->nextFrame = std::move(firstFrame);
	}

	if (!state->nextFrame) {
		// No next frame, try getting a new one

		if (!state->decoder->tryPopVideoFrame(state->nextFrame)) {
			// No new frame available :(
			return;
		}
	}

	auto currentTime = playbackGetTime(state);
	if (currentTime < state->nextFrame->frameTime) {
		// Old frame is still valid, nothing to do here
		return;
	}

	while (currentTime >= state->nextFrame->frameTime) {
		if (state->currentFrame) {
			if (state->nextFrame->frameTime < state->currentFrame->frameTime) {
				// We reached the end of the movie and just looped to the beginning. Now we need to reset the playback
				// time
				state->playback_time = 0;
				currentTime = playbackGetTime(state);
			}
		}

		// Move the next frame to the current frame slot
		state->currentFrame = std::move(state->nextFrame);

		// Get a new frame from the decoder
		auto success = state->decoder->tryPopVideoFrame(state->nextFrame);
		if (!success) {
			// Make sure the pointer is actually empty
			state->nextFrame = nullptr;
			// No more frames available
			break;
		}
	}

	// Now upload the new frame
	if (state->videoPresenter) {
		state->videoPresenter->uploadVideoFrame(state->currentFrame);
		state->newFrameAdded = true;
	}
}

void processSubtitleData(PlayerState* state) {
	SubtitleFramePtr ptr;
	while (state->decoder->tryPopSubtitleData(ptr)) {
		// Take all subtitle frames from our queue
		state->queued_subtitles.push(std::move(ptr));
	}

	if (state->currentSubtitle && playbackGetTime(state) <= state->currentSubtitle->displayEndTime) {
		// We currently have a frame and it it still being displayed so there is nothing for us to do here
		return;
	}

	// Subtitle has expired so we don't need it anymore
	state->currentSubtitle = nullptr;

	while(!state->queued_subtitles.empty()) {
		// Take a look at the first entry and check if it's time to display it yet
		auto& nextSubtitle = state->queued_subtitles.front();
		if (playbackGetTime(state) >= nextSubtitle->displayStartTime) {
			state->currentSubtitle = std::move(nextSubtitle);
			state->queued_subtitles.pop();
		} else {
			// The next subtitle should not be displayed yet so we are done here
			break;
		}
	}
}

bool processAudioData(PlayerState* state) {
	TRACE_SCOPE(tracing::CutsceneProcessAudioData);

	if (!state->hasAudio) {
		if (state->decoder->hasAudio()) {
			// Even if we don't play the sound we still need to remove it from the queue
			AudioFramePtr audioData;
			while (state->decoder->tryPopAudioData(audioData)) {
				// Intentionally left empty
			}
		}
		return false;
	}

	ALint processed = 0;
	OpenAL_ErrorCheck(alGetSourcei(state->audioSid, AL_BUFFERS_PROCESSED, &processed), return false);

	// First check for free buffers and push them int the audio queue
	for (int i = 0; i < processed; ++i) {
		ALuint buffer;
		OpenAL_ErrorPrint(alSourceUnqueueBuffers(state->audioSid, 1, &buffer));
		state->unqueuedAudioBuffers.push(buffer);
	}

	AudioFramePtr audioData;

	while (!state->unqueuedAudioBuffers.empty() && state->decoder->tryPopAudioData(audioData)) {
		auto buffer = state->unqueuedAudioBuffers.front();
		state->unqueuedAudioBuffers.pop();

		ALenum format = (audioData->channels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;

		OpenAL_ErrorCheck(alBufferData(buffer,
									   format,
									   audioData->audioData.data(),
									   static_cast<ALsizei>(audioData->audioData.size() * sizeof(short)),
									   static_cast<ALsizei>(audioData->rate)), return false);

		OpenAL_ErrorCheck(alSourceQueueBuffers(state->audioSid, 1, &buffer), return false);
	}

	ALint status = 0, queued = 0;
	OpenAL_ErrorCheck(alGetSourcei(state->audioSid, AL_SOURCE_STATE, &status), return false);

	OpenAL_ErrorCheck(alGetSourcei(state->audioSid, AL_BUFFERS_QUEUED, &queued), return false);

	if ((status != AL_PLAYING) && (queued > 0)) {
		OpenAL_ErrorPrint(alSourcePlay(state->audioSid));
	}

	// Get status again in cause we just started playback
	OpenAL_ErrorCheck(alGetSourcei(state->audioSid, AL_SOURCE_STATE, &status), return false);
	return status == AL_PLAYING;
}

void audioPlaybackClose(PlayerState* state) {
	if (!state->audioInited) {
		return;
	}

	ALint p = 0;

	OpenAL_ErrorPrint(alSourceStop(state->audioSid));
	OpenAL_ErrorPrint(alGetSourcei(state->audioSid, AL_BUFFERS_PROCESSED, &p));
	OpenAL_ErrorPrint(alSourceUnqueueBuffers(state->audioSid, p, state->audioBuffers.data()));
	OpenAL_ErrorPrint(alDeleteSources(1, &state->audioSid));

	for (auto buffer : state->audioBuffers) {
		// make sure that the buffer is real before trying to delete, it could crash for some otherwise
		if ((buffer != 0) && alIsBuffer(buffer)) {
			OpenAL_ErrorPrint(alDeleteBuffers(1, &buffer));
		}
	}

	state->audioInited = false;
}

void videoPlaybackClose(PlayerState* state) {
	if (!state->videoInited) {
		return;
	}

	state->videoPresenter.reset();

	state->videoInited = false;
}

bool shouldBeginPlayback(Decoder* decoder) {
	auto video = decoder->isVideoQueueFull();
	auto audio = decoder->isAudioQueueFull();
	auto subtitles = decoder->isSubtitleQueueFull();

	// Wait until one of the queues is full, that should make sure
	// that we have enough frames at the beginning of playback and that
	// we don't wait indefinitely if the cutscene has a weird amount of audio
	// or video at the beginning
	// Also only wait while the decoder is still working. Otherwise we will wait indefinitely if the video
	// is very short.
	return (audio || video || subtitles) || !decoder->isDecoding();
}
}

namespace cutscene {
Player::Player(std::unique_ptr<Decoder>&& decoder, const PlaybackProperties& properties)
    : m_decoder(std::move(decoder))
{
	Assertion(!m_state.videoInited,
	          "Internal State has been initialized before! Create a new player for replaying a movie.");

	m_decoderThread.reset(new std::thread(std::bind(&Player::decoderThread, this)));

	m_state.props   = m_decoder->getProperties();
	m_state.decoder = m_decoder.get();

	videoPlaybackInit(&m_state);

	if (properties.with_audio) {
		audioPlaybackInit(&m_state);
	}
}

Player::~Player() {
	// If video is initialized it means that stopPlayback has not been called yet
	if (m_state.videoInited) {
		stopPlayback();
	}

	if (m_decoder) {
		m_decoder->close();
	}
}

bool Player::processDecoderData() {
	if (!m_state.playbackHasBegun) {
		// Wait until video and audio are available
		// If we don't have audio, don't wait for it (obviously...)
		if (!shouldBeginPlayback(m_state.decoder)) {
			return true;
		}

		m_state.playbackHasBegun = true;
	}

	TRACE_SCOPE(tracing::CutsceneProcessDecoder);

	processVideoData(&m_state);

	processSubtitleData(&m_state);

	auto audioPlaying = processAudioData(&m_state);

	// Set the playing flag if the decoder is still active or there is still data available
	auto decoding = m_decoder->isDecoding();

	// Audio is pending if there is data left in the queue or if OpenAL is still playing audio
	auto pendingAudio = (m_decoder->isAudioFrameAvailable() && m_decoder->hasAudio()) || audioPlaying;
	auto pendingVideo = m_decoder->isVideoFrameAvailable();

	return decoding || pendingAudio || pendingVideo;
}
bool Player::update(uint64_t diff_time_micro) {
	if (m_state.playbackHasBegun) {
		m_state.playback_time += diff_time_micro;
	}

	return processDecoderData();
}
void Player::stopPlayback() {
	m_decoder->stopDecoder();

	audioPlaybackClose(&m_state);
	videoPlaybackClose(&m_state);

	if(m_decoderThread->joinable())
		m_decoderThread->join();
}

void Player::decoderThread() {
	try {
		m_decoder->startDecoding();
	} catch (const std::exception& e) {
		mprintf(("Video: An exception was thrown while decoding the video: %s\n", e.what()));
	} catch (...) {
		mprintf(("Video: An exception was thrown while decoding the video!\n"));
	}
}

std::unique_ptr<Player> Player::newPlayer(const SCP_string& name, const PlaybackProperties& properties)
{
	mprintf(("Creating player for movie '%s'.\n", name.c_str()));

	auto decoder = findDecoder(name, properties);

	if (decoder == nullptr) {
		return nullptr;
	}
	return std::unique_ptr<Player>(new Player(std::move(decoder), properties));
}
const PlayerState& Player::getInternalState() const {
	return m_state;
}
const MovieProperties& Player::getMovieProperties() const {
	return m_state.props;
}
void Player::draw(float x1, float y1, float x2, float y2) {
	if (!m_state.currentFrame) {
		return;
	}

	if (m_state.videoPresenter) {
		m_state.videoPresenter->displayFrame(x1, y1, x2, y2);
	}
}
SCP_string Player::getCurrentSubtitle() {
	if (!m_state.currentSubtitle) {
		return "";
	} else {
		return m_state.currentSubtitle->text;
	}
}
bool Player::isPlaybackReady() const { return m_state.playbackHasBegun || shouldBeginPlayback(m_decoder.get()); }
}
