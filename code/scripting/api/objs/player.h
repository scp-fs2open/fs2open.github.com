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

class scoring_stats_h {
	scoring_struct *_score = nullptr;
  public:
	scoring_stats_h();
	scoring_stats_h(const scoring_struct& stats);

	bool isValid() const;

	scoring_struct* get();

	void cleanup();
};

DECLARE_ADE_OBJ(l_ScoringStats, scoring_stats_h);

}
}


