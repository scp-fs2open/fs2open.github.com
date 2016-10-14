#include "cutscene/player.h"

#include "cutscene/Decoder.h"
#include "cutscene/player/VideoPresenter.h"
#include "cutscene/ffmpeg/FFMPEGDecoder.h"

#include "graphics/2d.h"

#include "globalincs/alphacolors.h"

#include "osapi/osapi.h"

#include "sound/openal.h"

#include "io/key.h"
#include "io/timer.h"

#include "parse/parselo.h"

#include "cutscene/player/OpenGLVideoPresenter.h"

using namespace cutscene::player;

namespace cutscene {
struct PlayerState {
	MovieProperties props;
	Decoder* decoder = nullptr;

	bool playing = true;

	bool playbackHasBegun = false;

	// Timing state
	std::uint64_t playback_time = 0;

	// Audio state
	bool audioInited = false;
	bool hasAudio = false;
	ALuint audioSid = 0;
	SCP_vector<ALuint> audioBuffers;
	SCP_queue<ALuint> unqueuedAudioBuffers;

	// Graphics state following
	bool videoInited = false;

	VideoFramePtr currentFrame;
	VideoFramePtr nextFrame;
	bool newFrameAdded = false;

	std::unique_ptr<VideoPresenter> videoPresenter;

	PlayerState() : decoder(nullptr), playing(true), playbackHasBegun(false),
					playback_time(0),
					audioInited(0), hasAudio(false), audioSid(0),
					videoInited(false), newFrameAdded(false) {}

 private:
	PlayerState(const PlayerState&) SCP_DELETED_FUNCTION;

	PlayerState& operator=(const PlayerState&) SCP_DELETED_FUNCTION;
};
}

namespace {
using namespace cutscene;

const int MAX_AUDIO_BUFFERS = 15;

Decoder* findDecoder(const SCP_string& name) {
	{
		auto ffmpeg = new ffmpeg::FFMPEGDecoder();
		if (ffmpeg->initialize(name)) {
			return ffmpeg;
		}
		delete ffmpeg;
	}

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

	if (gr_screen.mode == GR_OPENGL) {
		state->videoPresenter.reset(new OpenGLVideoPresenter(state->props));
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

	while(currentTime >= state->nextFrame->frameTime) {
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

bool processAudioData(PlayerState* state) {
	if (!state->hasAudio) {
		if (state->decoder->hasAudio()) {
			// Even if we don't play the sound we still need to remove it from the queue
			AudioFramePtr audioData;
			while(state->decoder->tryPopAudioData(audioData)) {
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

		OpenAL_ErrorCheck(alBufferData(buffer, format, audioData->audioData.data(),
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

template<typename... Args>
float print_string(float x, float y, const char* fmt, Args... params) {
	SCP_string text;
	sprintf(text, fmt, params...);

	gr_string(x, y, text.c_str(), GR_RESIZE_NONE);

	return y + font::get_current_font()->getHeight();
}

void showVideoInfo(PlayerState* state) {
	gr_set_color_fast(&Color_white);

	float y = 200.f;
	float x = 100.f;
	y = print_string(x, y, "Movie FPS: %f", state->props.fps);
	y = print_string(x, y, "Size: %dx%d", state->props.size.width, state->props.size.height);

	y = gr_screen.max_h - 200.f;

	size_t audio_queue_size = state->decoder->getAudioQueueSize();
	if (state->hasAudio) {
		ALint queued;
		OpenAL_ErrorPrint(alGetSourcei(state->audioSid, AL_BUFFERS_QUEUED, &queued));
		audio_queue_size += queued;
	}

	y = print_string(x, y, "Audio Queue size: " SIZE_T_ARG, audio_queue_size);
	y = print_string(x, y, "Video Queue size: " SIZE_T_ARG, state->decoder->getVideoQueueSize());
	y += font::get_current_font()->getHeight();
	// Estimate the size of the video buffer
	// We use YUV420p frames so one pixel uses 1.5 bytes of storage
	size_t single_frame_size = (size_t) (state->props.size.width * state->props.size.height * 1.5);
	size_t total_size = single_frame_size * state->decoder->getVideoQueueSize();
	print_string(x, y, "Video buffer size: " SIZE_T_ARG "B", total_size);
}

void displayVideo(PlayerState* state) {
	if (!state->currentFrame) {
		return;
	}

	gr_clear();
	if (state->videoPresenter) {
		state->videoPresenter->displayFrame();
	}

	if (Cmdline_show_video_info) {
		showVideoInfo(state);
	}

	gr_flip();
}

void processEvents(PlayerState* state) {
	io::mouse::CursorManager::get()->showCursor(false);

	os_poll();

	int k = key_inkey();
	switch (k) {
		case KEY_ESC:
		case KEY_ENTER:
		case KEY_SPACEBAR:
			state->playing = false;
			break;
		default:
			break;
	}
}

bool shouldBeginPlayback(Decoder* decoder) {
	auto video = decoder->isVideoQueueFull();
	auto audio = decoder->isAudioQueueFull();

	// Wait until one of the queues is full, that should make sure
	// that we have enough frames at the beginning of playback and that
	// we don't wait indefinitely if the cutscene has a weird amount of audio
	// or video at the beginning
	// Also only wait while the decoder is still working. Otherwise we will wait indefinitely if the video
	// is very short.
	return (audio || video) || !decoder->isDecoding();
}
}

namespace cutscene {
Player::Player() {
}

Player::~Player() {
	if (m_decoder) {
		m_decoder->close();
	}
}

void Player::processDecoderData(PlayerState* state) {
	if (!state->playbackHasBegun) {
		// Wait until video and audio are available
		// If we don't have audio, don't wait for it (obviously...)
		if (!shouldBeginPlayback(state->decoder)) {
			return;
		}

		state->playbackHasBegun = true;
	}

	processVideoData(state);

	auto audioPlaying = processAudioData(state);

	// Set the playing flag if the decoder is still active or there is still data available
	auto decoding = m_decoder->isDecoding();

	// Audio is pending if there is data left in the queue or of OpenAL is still playing audio
	auto pendingAudio = (m_decoder->isAudioFrameAvailable() && m_decoder->hasAudio()) || audioPlaying;
	auto pendingVideo = m_decoder->isVideoFrameAvailable();

	state->playing = decoding || pendingAudio || pendingVideo;
}

void Player::startPlayback() {
	m_decoderThread.reset(new std::thread(std::bind(&Player::decoderThread, this)));

	PlayerState state;
	state.props = m_decoder->getProperties();
	state.decoder = m_decoder.get();

	// Compute the maximum time we will sleep to make sure we can still maintain the movie FPS
	// and not waste too much CPU time
	// We will sleep at most half the time a frame would be displayed
	auto sleepTime = static_cast<std::uint64_t>((1. / (4. * state.props.fps)) * 1000.);

	videoPlaybackInit(&state);

	audioPlaybackInit(&state);

	auto lastDisplayTimestamp = timer_get_microseconds();
	while (state.playing) {
		processDecoderData(&state);

		displayVideo(&state);

		processEvents(&state);

		auto timestamp = timer_get_microseconds();

		auto passed = timestamp - lastDisplayTimestamp;
		lastDisplayTimestamp = timestamp;

		if (state.playbackHasBegun) {
			state.playback_time += passed;
		}

		if (passed < sleepTime) {
			auto sleep = sleepTime - passed;

			os_sleep(static_cast<uint>(sleep));
		}
	}
	m_decoder->stopDecoder();

	audioPlaybackClose(&state);
	videoPlaybackClose(&state);

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

std::unique_ptr<Player> Player::newPlayer(const SCP_string& name) {
	mprintf(("Creating player for movie '%s'.\n", name.c_str()));

	auto decoder = findDecoder(name);

	if (decoder == nullptr) {
		return nullptr;
	}

	std::unique_ptr<Player> player(new Player());
	player->m_decoder.reset(decoder);

	return player;
}
}
