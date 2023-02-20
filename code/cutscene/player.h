#pragma once

#include <memory>

#include <thread>

#include "globalincs/pstypes.h"
#include "sound/openal.h"

#include "Decoder.h"
#include "VideoPresenter.h"

namespace cutscene {
class Decoder;

struct PlayerState {
	MovieProperties props;
	Decoder* decoder = nullptr;

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

	std::unique_ptr<player::VideoPresenter> videoPresenter;

	SCP_queue<SubtitleFramePtr> queued_subtitles;
	SubtitleFramePtr currentSubtitle;

	PlayerState() {
	}

	PlayerState(const PlayerState&) = delete;
	PlayerState& operator=(const PlayerState&) = delete;
};

/**
 * @brief A movie player
 */
class Player {
  private:
	std::unique_ptr<Decoder> m_decoder;

	std::unique_ptr<std::thread> m_decoderThread;

	PlayerState m_state;

	bool processDecoderData();

  private:
	void decoderThread();

	Player(std::unique_ptr<Decoder>&& decoder, const PlaybackProperties& properties);
  public:
	~Player();

	Player(const Player&) = delete;
	Player& operator=(const Player&) = delete;

	/**
	 * @brief Update the player and increase display timestamp by specified delta time
	 * @param diff_time_micro The time to increase the display timestamp in microseconds (not milliseconds!)
	 * @return @c true if there is more content to display. @c false if the player reached the end of the movie and has
	 * 			displayed the last frame.
	 */
	bool update(uint64_t diff_time_micro);

	/**
	 * @brief Gives access to the internal state of the player
	 *
	 * @warning This should only be used for debugging purposed!
	 *
	 * @return A reference to the internal state
	 */
	const PlayerState& getInternalState() const;

	/**
	 * @brief Gets the properties of the movie that this player is displaying
	 * @return A reference to the movie properties
	 */
	const MovieProperties& getMovieProperties() const;

	/**
	 * @brief Determines if this player can start displaying video.
	 * @return true if the cutscene is ready, false otherwise
	 */
	bool isPlaybackReady() const;

	/**
	 * @brief Draws the current movie frame at the specified coordinates.
	 *
	 * This may not draw anything if there is currently not a frame to display.
	 *
	 * @param x1 The X coordinate of the top left corner
	 * @param y1 The Y coordinate of the top left corner
	 * @param x2 The X coordinate of the bottom right corner
	 * @param y2 The Y coordinate of the bottom right corner
	 */
	void draw(float x1, float y1, float x2, float y2, float alpha = 1.0f);

	/**
	 * @brief Gets the subtitle text that should be displayed now
	 * @return The text, encoded in UTF-8
	 */
	SCP_string getCurrentSubtitle();

	/**
	 * @brief Stops playback
	 *
	 * This is also automatically called when this instance is destroyed.
	 */
	void stopPlayback();

	/**
	 * @brief Creates a player
	 * The player is configured to play the movie with the specified name
	 * @param name The movie to play
	 * @param properties A structure describing how the movie will be used
	 * @return @c nullptr if the specified movie could not be opened, a valid Player pointer otherwise
	 */
	static std::unique_ptr<Player> newPlayer(const SCP_string& name, const PlaybackProperties& properties = PlaybackProperties());
};
}
