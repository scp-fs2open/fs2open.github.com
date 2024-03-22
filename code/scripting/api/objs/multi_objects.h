#pragma once

#include "network/multi_pxo.h"
#include "network/multiui.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct channel_h {
	int channel;
	channel_h();
	explicit channel_h(int l_channel);
	pxo_channel* getChannel() const;
	bool isCurrent() const;
	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Channel, channel_h);

struct active_game_h {
	int game;
	active_game_h();
	explicit active_game_h(int l_game);
	active_game* getGame() const;
	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Active_Game, active_game_h);

} // namespace api
} // namespace scripting