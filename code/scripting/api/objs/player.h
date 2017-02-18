#pragma once

#include "playerman/player.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

class player_h {
	player *_plr = nullptr;
  public:
	player_h();
	player_h(const player& plr);

	bool isValid() const;

	player* get();

	void cleanup();
};

DECLARE_ADE_OBJ(l_Player, player_h);

}
}


