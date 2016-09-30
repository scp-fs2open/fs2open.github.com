#pragma once

#include <memory>

#include <thread>

#include "globalincs/pstypes.h"

namespace cutscene {
class Decoder;

struct PlayerState;

/**
 * @brief A movie player
 */
class Player {
 private:
	std::unique_ptr<Decoder> m_decoder;

	std::unique_ptr<std::thread> m_decoderThread;

	void processDecoderData(PlayerState* state);

 private:
	Player(const Player&) = delete;

	void decoderThread();

 public:
	Player();

	~Player();

	/**
     * @brief Begin playing the previously selected movie
     */
	void startPlayback();

	/**
     * @brief Creates a player
     * The player is configured to play the movie with the specified name
     * @param name The movie to play
     * @return @c nullptr if the specified movie could not be opened, a valid Player pointer otherwise
     */
	static std::unique_ptr<Player> newPlayer(const SCP_string& name);
};
}
