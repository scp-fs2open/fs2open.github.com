#pragma once

#include "network/multi_dogfight.h"
#include "network/multi.h"
#include "network/multiui.h"
#include "network/multi_pxo.h"
#include "network/multiui.h"
#include "object/object.h"
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
	bool isValid() const;
};

struct active_game_h {
	int game;
	active_game_h();
	explicit active_game_h(int l_game);
	active_game* getGame() const;
	bool isValid() const;
};

struct dogfight_scores_h {
	int scores;
	dogfight_scores_h();
	explicit dogfight_scores_h(int l_scores);
	multi_df_score* getScores() const;
	bool isValid() const;
};

struct join_ship_choices_h {
	int choice;
	join_ship_choices_h();
	explicit join_ship_choices_h(int l_choice);
	object* getObject() const;
	int getIndex() const;
	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Channel, channel_h);
DECLARE_ADE_OBJ(l_NetPlayer, net_player_h);
DECLARE_ADE_OBJ(l_NetMission, net_mission_h);
DECLARE_ADE_OBJ(l_NetCampaign, net_campaign_h);
DECLARE_ADE_OBJ(l_NetGame, net_game_h);
DECLARE_ADE_OBJ(l_Active_Game, active_game_h);
DECLARE_ADE_OBJ(l_Dogfight_Scores, dogfight_scores_h);
DECLARE_ADE_OBJ(l_Join_Ship_Choice, join_ship_choices_h);

} // namespace api
} // namespace scripting