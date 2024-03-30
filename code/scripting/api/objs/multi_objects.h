#pragma once

#include "network/multi.h"
#include "network/multiui.h"
#include "network/multi_pxo.h"
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

struct net_player_h {
	int player;
	net_player_h();
	explicit net_player_h(int l_player);
	net_player* getPlayer() const;
	int getIndex() const;
	bool isValid() const;
};

struct net_mission_h {
	int mission;
	net_mission_h();
	explicit net_mission_h(int l_mission);
	multi_create_info* getMission() const;
	int getIndex() const;
	bool isValid() const;
};

struct net_campaign_h {
	int campaign;
	net_campaign_h();
	explicit net_campaign_h(int l_campaign);
	multi_create_info* getCampaign() const;
	int getIndex() const;
	bool isValid() const;
};

struct net_game_h {
	net_game_h();
	netgame_info* getNetgame() const;
};

DECLARE_ADE_OBJ(l_Channel, channel_h);
DECLARE_ADE_OBJ(l_NetPlayer, net_player_h);
DECLARE_ADE_OBJ(l_NetMission, net_mission_h);
DECLARE_ADE_OBJ(l_NetCampaign, net_campaign_h);
DECLARE_ADE_OBJ(l_NetGame, net_game_h);

} // namespace api
} // namespace scripting