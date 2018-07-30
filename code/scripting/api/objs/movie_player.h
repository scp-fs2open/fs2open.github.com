#pragma once

#include "scripting/ade_api.h"

#include "cutscene/player.h"

namespace scripting {
namespace api {

class movie_player_h {
	std::unique_ptr<cutscene::Player> _player;

  public:
	movie_player_h();
	explicit movie_player_h(std::unique_ptr<cutscene::Player>&& player);

	cutscene::Player* player();

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_MoviePlayer, movie_player_h);

} // namespace api
} // namespace scripting
