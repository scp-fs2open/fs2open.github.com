#pragma once

#include "playerman/player.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

class player_h {
	bool _owned = false;
	player *_plr = nullptr;
  public:
	player_h();
	player_h(player* plr);
	player_h(const player& plr);

	player_h(const player_h&) = delete;
	player_h& operator=(const player_h&) = delete;

	player_h(player_h&&) noexcept;
	player_h& operator=(player_h&&) noexcept;

	~player_h();

	bool isValid() const;

	player* get();
};

DECLARE_ADE_OBJ(l_Player, player_h);

class scoring_stats_h {
	scoring_struct _score;
	bool _valid = false;
  public:
	scoring_stats_h();
	scoring_stats_h(const scoring_struct& stats);

	bool isValid() const;

	scoring_struct* get();
};

DECLARE_ADE_OBJ(l_ScoringStats, scoring_stats_h);

}
}


