#include "multi_objects.h"
#include "enums.h"

#include "network/multi_dogfight.h"
#include "network/multi_pxo.h"
#include "network/multiui.h"
#include "network/multi.h"
#include "network/multi_kick.h"
#include "network/multi_team.h"
#include "network/multimsgs.h"
#include "network/multi_ingame.h"
#include "network/multiteamselect.h"
#include "mission/missionparse.h"
#include "pilotfile/pilotfile.h"
#include "ship/ship.h"
#include "object/objectshield.h"
#include "weapon/weapon.h"
#include "scripting/lua/LuaTable.h"

#include "scripting/api/objs/player.h"

namespace scripting {
namespace api {

channel_h::channel_h() : channel(-1) {}
channel_h::channel_h(int l_channel) : channel(l_channel) {}

pxo_channel* channel_h::getChannel() const
{
	if (!isValid())
		return nullptr;

	return &Multi_pxo_channels[channel];
}

bool channel_h::isCurrent() const
{
	return !stricmp(Multi_pxo_channels[channel].name, Multi_pxo_channel_current.name);
}

bool channel_h::isValid() const
{
	return channel >= 0 && channel < static_cast<int>(Multi_pxo_channels.size());
}

net_player_h::net_player_h() : player(-1) {}
net_player_h::net_player_h(int l_player) : player(l_player) {}

net_player* net_player_h::getPlayer() const
{
	if (!isValid())
		return nullptr;

	return &Net_players[player];
}

int net_player_h::getIndex() const
{
	return player;
}

bool net_player_h::isValid() const
{
	//If we're not in multiplayer mode then there will be no players!
	if (!(Game_mode & GM_MULTIPLAYER)) {
		return false;
	}
	
	if (player < 0 || player >= MAX_PLAYERS) {
		return false;
	}
	//disconnected players and servers are not valid for the UI!
	if (!MULTI_CONNECTED(Net_players[player]) || MULTI_STANDALONE(Net_players[player])) {
		return false;
	}
	return true;
}

net_mission_h::net_mission_h() : mission(-1) {}
net_mission_h::net_mission_h(int l_mission) : mission(l_mission) {}

multi_create_info* net_mission_h::getMission() const
{
	if (!isValid())
		return nullptr;

	return &Multi_create_mission_list[mission];
}

int net_mission_h::getIndex() const
{
	return mission;
}

bool net_mission_h::isValid() const
{
	return SCP_vector_inbounds(Multi_create_mission_list, mission);
}

net_campaign_h::net_campaign_h() : campaign(-1) {}
net_campaign_h::net_campaign_h(int l_campaign) : campaign(l_campaign) {}

multi_create_info* net_campaign_h::getCampaign() const
{
	if (!isValid())
		return nullptr;

	return &Multi_create_campaign_list[campaign];
}

int net_campaign_h::getIndex() const
{
	return campaign;
}

bool net_campaign_h::isValid() const
{
	return SCP_vector_inbounds(Multi_create_campaign_list, campaign);
}

net_game_h::net_game_h() {}

netgame_info* net_game_h::getNetgame() const
{
	return &Netgame;
}

bool net_game_h::isValid() const
{
	if ((Game_mode & GM_MULTIPLAYER) && (Netgame.name[0] != '\0')) {
		return true;
	} else {
		return false;
	}
}

active_game_h::active_game_h() : game(-1) {}
active_game_h::active_game_h(int l_game) : game(l_game) {}

active_game* active_game_h::getGame() const
{
	if (!isValid())
		return nullptr;

	auto it = Active_games.begin();
	std::advance(it, game);

	return &(*it);
}

bool active_game_h::isValid() const
{
	return game >= 0 && game < static_cast<int>(Active_games.size());
}

dogfight_scores_h::dogfight_scores_h() : scores(-1) {}
dogfight_scores_h::dogfight_scores_h(int l_scores) : scores(l_scores) {}

multi_df_score* dogfight_scores_h::getScores() const
{
	if (!isValid())
		return nullptr;


	return &Multi_df_score[scores];
}

bool dogfight_scores_h::isValid() const
{
	return scores >= 0 && scores < Multi_df_score_count;
}

join_ship_choices_h::join_ship_choices_h() : choice(-1) {}
join_ship_choices_h::join_ship_choices_h(int l_choice) : choice(l_choice) {}

object* join_ship_choices_h::getObject() const
{
	if (!isValid())
		return nullptr;
	//This is kind of a roundabout way of doing this but it ensures we get
	//read-only access to just the information we need for this UI rather
	//than exposing the entire object itself
	return &Objects[Ingame_ship_choices[choice]];
}

int join_ship_choices_h::getIndex() const
{
	return choice;
}

bool join_ship_choices_h::isValid() const
{
	return SCP_vector_inbounds(Ingame_ship_choices, choice);
}

//**********HANDLE: channel section
ADE_OBJ(l_Channel, channel_h, "pxo_channel", "Channel Section handle");

ADE_FUNC(isValid,
	l_Channel,
	nullptr,
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	channel_h current;
	if (!ade_get_args(L, "o", l_Channel.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Name, l_Channel, nullptr, "The name of the channel", "string", "The name")
{
	channel_h current;
	if (!ade_get_args(L, "o", l_Channel.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getChannel()->name);
}

ADE_VIRTVAR(Description, l_Channel, nullptr, "The description of the channel", "string", "The description")
{
	channel_h current;
	if (!ade_get_args(L, "o", l_Channel.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getChannel()->desc);
}

ADE_VIRTVAR(NumPlayers, l_Channel, nullptr, "The number of players in the channel", "number", "The number of players")
{
	channel_h current;
	if (!ade_get_args(L, "o", l_Channel.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", static_cast<int>(current.getChannel()->num_users));
}

ADE_VIRTVAR(NumGames, l_Channel, nullptr, "The number of games the channel", "number", "The number of games")
{
	channel_h current;
	if (!ade_get_args(L, "o", l_Channel.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", static_cast<int>(current.getChannel()->num_servers));
}

ADE_FUNC(isCurrent, l_Channel, nullptr, "Returns whether this is the current channel", "boolean", "true for current, false otherwise. Nil if invalid.")
{
	channel_h current;
	if (!ade_get_args(L, "o", l_Channel.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isCurrent());
}

ADE_FUNC(joinChannel, l_Channel, nullptr, "Joins the specified channel", nullptr, nullptr)
{
	channel_h current;
	if (!ade_get_args(L, "o", l_Channel.Get(&current)))
		return ADE_RETURN_NIL;

	multi_pxo_maybe_join_channel(current.getChannel());

	return ADE_RETURN_NIL;
}

//**********HANDLE: net player section
ADE_OBJ(l_NetPlayer, net_player_h, "net_player", "Net Player handle");

ADE_FUNC(isValid,
	l_NetPlayer,
	nullptr,
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Name, l_NetPlayer, nullptr, "The player's callsign", "string", "The player callsign")
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getPlayer()->m_player->callsign);
}

ADE_VIRTVAR(Team, l_NetPlayer, "number Team", "The player's team as an integer", "number", "The team")
{
	net_player_h current;
	int team;
	if (!ade_get_args(L, "o|i", l_NetPlayer.Get(&current), &team)) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		if (team < 0 || team > 1) {
			LuaError(L, "Team must be either 0 or 1!");
		}
		multi_team_set_team(current.getPlayer(), team);
	}

	return ade_set_args(L, "i", current.getPlayer()->p_info.team);
}

ADE_VIRTVAR(State, l_NetPlayer, nullptr, "The player's current state string", "string", "The state")
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", multi_sync_get_state_string(current.getPlayer()).c_str());
}

ADE_FUNC(isSelf, l_NetPlayer, nullptr, "Whether or not the player is the current game instance's player", "boolean", "The self value")
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (current.getPlayer()->player_id == Net_player->player_id) {
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Master, l_NetPlayer, nullptr, "Whether or not the player is the game master", "boolean", "The master value")
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getPlayer()->flags & NETINFO_FLAG_AM_MASTER) {
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Host, l_NetPlayer, nullptr, "Whether or not the player is the game host", "boolean", "The host value")
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getPlayer()->flags & NETINFO_FLAG_GAME_HOST) {
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Observer, l_NetPlayer, nullptr, "Whether or not the player is an observer", "boolean", "The observer value")
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getPlayer()->flags & NETINFO_FLAG_OBSERVER) {
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Captain, l_NetPlayer, nullptr, "Whether or not the player is the team captain", "boolean", "The captain value")
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getPlayer()->flags & NETINFO_FLAG_TEAM_CAPTAIN) {
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_FUNC(getStats,
	l_NetPlayer,
	nullptr,
	"Gets a handle of the player stats by player name or invalid handle if the name is invalid",
	"scoring_stats",
	"Player stats handle")
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "o", l_ScoringStats.Set(scoring_stats_h(current.getPlayer()->m_player->stats, current.getPlayer()->m_player)));
}

ADE_FUNC(kickPlayer, l_NetPlayer, nullptr, "Kicks the player from the game", nullptr, nullptr)
{
	net_player_h current;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	multi_kick_player(current.getIndex(), 0);

	return ADE_RETURN_NIL;
}

//**********HANDLE: mission section
ADE_OBJ(l_NetMission, net_mission_h, "net_mission", "Net Mission handle");

ADE_FUNC(isValid,
	l_NetMission,
	nullptr,
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	net_mission_h current;
	if (!ade_get_args(L, "o", l_NetMission.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Name, l_NetMission, nullptr, "The name of the mission", "string", "The name")
{
	net_mission_h current;
	if (!ade_get_args(L, "o", l_NetMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getMission()->name);
}

ADE_VIRTVAR(Filename, l_NetMission, nullptr, "The filename of the mission", "string", "The filename")
{
	net_mission_h current;
	if (!ade_get_args(L, "o", l_NetMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getMission()->filename);
}

ADE_VIRTVAR(Players, l_NetMission, nullptr, "The max players for the mission", "number", "The max number of players")
{
	net_mission_h current;
	if (!ade_get_args(L, "o", l_NetMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "i", 0);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getMission()->max_players);
}

ADE_VIRTVAR(Respawn, l_NetMission, nullptr, "The mission specified respawn count", "number", "The respawn count")
{
	net_mission_h current;
	if (!ade_get_args(L, "o", l_NetMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getMission()->respawn);
}

ADE_VIRTVAR(Tracker,
	l_NetMission,
	nullptr,
	"The validity status of the mission tracker",
	"boolean",
	"true if valid, false if invalid, nil if unknown or handle is invalid")
{
	net_mission_h current;
	if (!ade_get_args(L, "o", l_NetMission.Get(&current)))
		return ADE_RETURN_NIL;
	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getMission()->valid_status == MVALID_STATUS_UNKNOWN) {
		return ADE_RETURN_NIL;
	}

	if (current.getMission()->valid_status == MVALID_STATUS_VALID) {
		return ADE_RETURN_TRUE;
	}

	if (current.getMission()->valid_status == MVALID_STATUS_INVALID) {
		return ADE_RETURN_FALSE;
	}

	//catch all
	return ADE_RETURN_NIL;
}

ADE_VIRTVAR(Type,
	l_NetMission,
	nullptr,
	"The type of mission. Can be MULTI_TYPE_COOP, MULTI_TYPE_TEAM, or MULTI_TYPE_DOGFIGHT",
	"enumeration",
	"the type")
{
	net_mission_h current;
	lua_enum eh_idx = ENUM_INVALID;

	if (!ade_get_args(L, "o", l_NetMission.Get(&current)))
		return ADE_RETURN_NIL;
	if (!current.isValid()) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(eh_idx)));
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getMission()->flags & MISSION_TYPE_MULTI_COOP) {
		eh_idx = LE_MULTI_TYPE_COOP;
	}

	if (current.getMission()->flags & MISSION_TYPE_MULTI_TEAMS) {
		eh_idx = LE_MULTI_TYPE_TEAM;
	}

	if (current.getMission()->flags & MISSION_TYPE_MULTI_DOGFIGHT) {
		eh_idx = LE_MULTI_TYPE_DOGFIGHT;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(eh_idx)));
}

ADE_VIRTVAR(Builtin,
	l_NetMission,
	nullptr,
	"Is true if the mission is a built-in Volition mission. False otherwise",
	"boolean",
	"builtin")
{
	net_mission_h current;

	if (!ade_get_args(L, "o", l_NetMission.Get(&current)))
		return ADE_RETURN_NIL;
	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", multi_is_item_builtin_volition(current.getMission()->filename));
}

//**********HANDLE: campaign section
ADE_OBJ(l_NetCampaign, net_campaign_h, "net_campaign", "Net Campaign handle");

ADE_FUNC(isValid,
	l_NetCampaign,
	nullptr,
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	net_campaign_h current;
	if (!ade_get_args(L, "o", l_NetCampaign.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Name, l_NetCampaign, nullptr, "The name of the mission", "string", "The name")
{
	net_campaign_h current;
	if (!ade_get_args(L, "o", l_NetCampaign.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getCampaign()->name);
}

ADE_VIRTVAR(Filename, l_NetCampaign, nullptr, "The filename of the mission", "string", "The filename")
{
	net_campaign_h current;
	if (!ade_get_args(L, "o", l_NetCampaign.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getCampaign()->filename);
}

ADE_VIRTVAR(Players, l_NetCampaign, nullptr, "The max players for the mission", "number", "The max number of players")
{
	net_campaign_h current;
	if (!ade_get_args(L, "o", l_NetCampaign.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "i", 0);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getCampaign()->max_players);
}

ADE_VIRTVAR(Respawn, l_NetCampaign, nullptr, "The mission specified respawn count", "number", "The respawn count")
{
	net_campaign_h current;
	if (!ade_get_args(L, "o", l_NetCampaign.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getCampaign()->respawn);
}

ADE_VIRTVAR(Tracker,
	l_NetCampaign,
	nullptr,
	"The validity status of the mission tracker",
	"boolean",
	"true if valid, false if invalid, nil if unknown or handle is invalid")
{
	net_campaign_h current;
	if (!ade_get_args(L, "o", l_NetCampaign.Get(&current)))
		return ADE_RETURN_NIL;
	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getCampaign()->valid_status == MVALID_STATUS_UNKNOWN) {
		return ADE_RETURN_NIL;
	}

	if (current.getCampaign()->valid_status == MVALID_STATUS_VALID) {
		return ADE_RETURN_TRUE;
	}

	if (current.getCampaign()->valid_status == MVALID_STATUS_INVALID) {
		return ADE_RETURN_FALSE;
	}

	// catch all
	return ADE_RETURN_NIL;
}

ADE_VIRTVAR(Type,
	l_NetCampaign,
	nullptr,
	"The type of mission. Can be MULTI_TYPE_COOP, MULTI_TYPE_TEAM, or MULTI_TYPE_DOGFIGHT",
	"enumeration",
	"the type")
{
	net_campaign_h current;
	lua_enum eh_idx = ENUM_INVALID;

	if (!ade_get_args(L, "o", l_NetCampaign.Get(&current)))
		return ADE_RETURN_NIL;
	if (!current.isValid()) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(eh_idx)));
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getCampaign()->flags & MISSION_TYPE_MULTI_COOP) {
		eh_idx = LE_MULTI_TYPE_COOP;
	}

	if (current.getCampaign()->flags & MISSION_TYPE_MULTI_TEAMS) {
		eh_idx = LE_MULTI_TYPE_TEAM;
	}

	if (current.getCampaign()->flags & MISSION_TYPE_MULTI_DOGFIGHT) {
		eh_idx = LE_MULTI_TYPE_DOGFIGHT;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(eh_idx)));
}

ADE_VIRTVAR(Builtin,
	l_NetCampaign,
	nullptr,
	"Is true if the mission is a built-in Volition mission. False otherwise",
	"boolean",
	"builtin")
{
	net_campaign_h current;

	if (!ade_get_args(L, "o", l_NetCampaign.Get(&current)))
		return ADE_RETURN_NIL;
	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", multi_is_item_builtin_volition(current.getCampaign()->filename));
}

//**********HANDLE: netgame section
ADE_OBJ(l_NetGame, net_game_h, "netgame", "Netgame handle");

ADE_FUNC(isValid,
	l_NetGame,
	nullptr,
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	net_game_h current;
	if (!ade_get_args(L, "o", l_NetGame.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Name,
	l_NetGame,
	nullptr,
	"The name of the game",
	"string",
	"the name")
{
	net_game_h current;
	if (!ade_get_args(L, "o", l_NetGame.Get(&current)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getNetgame()->name);
}

ADE_VIRTVAR(MissionFilename, l_NetGame, nullptr, "The filename of the currently selected mission", "string", "the mission filename")
{
	net_game_h current;
	if (!ade_get_args(L, "o", l_NetGame.Get(&current)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getNetgame()->mission_name);
}

ADE_VIRTVAR(MissionTitle, l_NetGame, nullptr, "The title of the currently selected mission", "string", "the mission title")
{
	net_game_h current;
	if (!ade_get_args(L, "o", l_NetGame.Get(&current)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getNetgame()->title);
}

ADE_VIRTVAR(CampaignName, l_NetGame, nullptr, "The name of the currently selected campaign", "string", "the campaign name")
{
	net_game_h current;
	if (!ade_get_args(L, "o", l_NetGame.Get(&current)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getNetgame()->campaign_name);
}

ADE_VIRTVAR(Password, l_NetGame, nullptr, "The current password for the game", "string", "the password")
{
	net_game_h current;
	if (!ade_get_args(L, "o", l_NetGame.Get(&current)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getNetgame()->passwd);
}

ADE_VIRTVAR(Closed, l_NetGame, "boolean Closed", "Whether or not the game is closed", "boolean", "true for closed, false otherwise")
{
	net_game_h current;
	bool closed;
	if (!ade_get_args(L, "o|b", l_NetGame.Get(&current), &closed))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (Net_player->flags & NETINFO_FLAG_AM_MASTER) {
			if (closed) {
				current.getNetgame()->flags |= NG_FLAG_TEMP_CLOSED;
			} else {
				current.getNetgame()->flags &= ~NG_FLAG_TEMP_CLOSED;
			}
		} else if (Net_player->flags & NETINFO_FLAG_GAME_HOST) {
			if (closed) {
				current.getNetgame()->options.flags |= MLO_FLAG_TEMP_CLOSED;
			} else {
				current.getNetgame()->options.flags &= ~MLO_FLAG_TEMP_CLOSED;
			}
			multi_options_update_netgame();
		}
		// It can take the server time to process the change, so return the input choice for now.
		return (ade_set_args(L, "b", closed));
	}

	if (current.getNetgame()->flags & NG_FLAG_TEMP_CLOSED) {
		return ADE_RETURN_TRUE;
	} else {
		return ADE_RETURN_FALSE;
	}
}

ADE_VIRTVAR(HostModifiesShips, l_NetGame, "boolean HostModifies", "Whether or not the only the host can modify ships", "boolean", "true if enabled, false otherwise")
{
	net_game_h current;
	bool enabled;
	if (!ade_get_args(L, "o|b", l_NetGame.Get(&current), &enabled))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (enabled) {
			current.getNetgame()->options.flags |= MSO_FLAG_SS_LEADERS;
		} else {
			current.getNetgame()->options.flags &= ~MSO_FLAG_SS_LEADERS;
		}
		multi_options_update_netgame();
	}

	if (current.getNetgame()->options.flags & MSO_FLAG_SS_LEADERS) {
		return ADE_RETURN_TRUE;
	} else {
		return ADE_RETURN_FALSE;
	}
}

ADE_VIRTVAR(Orders,
	l_NetGame,
	"enumeration Type",
	"Who can give orders during the game. Will be one of the MULTI_OPTION enums. Returns nil if there's an error.",
	"enumeration",
	"the option type")
{
	net_game_h current;
	enum_h* eh_idx = nullptr;
	if (!ade_get_args(L, "o|o", l_NetGame.Get(&current), l_Enum.GetPtr(&eh_idx)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (eh_idx != nullptr) {
			if (eh_idx->index == LE_MULTI_OPTION_RANK) {
				current.getNetgame()->options.squad_set = MSO_SQUAD_RANK;
			} else if (eh_idx->index == LE_MULTI_OPTION_LEAD) {
				current.getNetgame()->options.squad_set = MSO_SQUAD_LEADER;
			} else if (eh_idx->index == LE_MULTI_OPTION_ANY) {
				current.getNetgame()->options.squad_set = MSO_SQUAD_ANY;
			} else if (eh_idx->index == LE_MULTI_OPTION_HOST) {
				current.getNetgame()->options.squad_set = MSO_SQUAD_HOST;
			}
		}
		send_netgame_update_packet();
		multi_options_update_netgame();
	}

	if (current.getNetgame()->options.squad_set == MSO_SQUAD_RANK) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_OPTION_RANK)));
	} else if (current.getNetgame()->options.squad_set == MSO_SQUAD_LEADER) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_OPTION_LEAD)));
	} else if (current.getNetgame()->options.squad_set == MSO_SQUAD_ANY) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_OPTION_ANY)));
	} else if (current.getNetgame()->options.squad_set == MSO_SQUAD_HOST) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_OPTION_HOST)));
	}

	// catch all
	return ADE_RETURN_NIL;
}

ADE_VIRTVAR(EndMission,
	l_NetGame,
	"enumeration Type",
	"Who can end the game. Will be one of the MULTI_OPTION enums. Returns nil if there's an error.",
	"enumeration",
	"the option type")
{
	net_game_h current;
	enum_h* eh_idx = nullptr;
	if (!ade_get_args(L, "o|o", l_NetGame.Get(&current), l_Enum.GetPtr(&eh_idx)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (eh_idx != nullptr) {
			if (eh_idx->index == LE_MULTI_OPTION_RANK) {
				current.getNetgame()->options.endgame_set = MSO_END_RANK;
			} else if (eh_idx->index == LE_MULTI_OPTION_LEAD) {
				current.getNetgame()->options.endgame_set = MSO_END_LEADER;
			} else if (eh_idx->index == LE_MULTI_OPTION_ANY) {
				current.getNetgame()->options.endgame_set = MSO_END_ANY;
			} else if (eh_idx->index == LE_MULTI_OPTION_HOST) {
				current.getNetgame()->options.endgame_set = MSO_END_HOST;
			}
		}
		send_netgame_update_packet();
		multi_options_update_netgame();
	}

	if (current.getNetgame()->options.endgame_set == MSO_END_RANK) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_OPTION_RANK)));
	} else if (current.getNetgame()->options.endgame_set == MSO_END_LEADER) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_OPTION_LEAD)));
	} else if (current.getNetgame()->options.endgame_set == MSO_END_ANY) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_OPTION_ANY)));
	} else if (current.getNetgame()->options.endgame_set == MSO_END_HOST) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_OPTION_HOST)));
	}

	// catch all
	return ADE_RETURN_NIL;
}

ADE_VIRTVAR(SkillLevel, l_NetGame, "number", "The current skill level the game, 0-4", "number", "the skill level")
{
	net_game_h current;
	int skill = 0;
	if (!ade_get_args(L, "o|i", l_NetGame.Get(&current), &skill))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		multi_ho_set_skill_level(skill);
		current.getNetgame()->options.skill_level = static_cast<ubyte>(multi_ho_get_skill_level());
	}

	return ade_set_args(L, "i", multi_ho_get_skill_level());
}

ADE_VIRTVAR(RespawnLimit, l_NetGame, "number", "The current respawn limit", "number", "the respawn limit")
{
	net_game_h current;
	int respawn = 0;
	if (!ade_get_args(L, "o|i", l_NetGame.Get(&current), &respawn))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (respawn < 0) {
			respawn = 0;
		}
		current.getNetgame()->options.respawn = respawn;
		current.getNetgame()->respawn = respawn;
	}

	return ade_set_args(L, "i", current.getNetgame()->options.respawn);
}

ADE_VIRTVAR(TimeLimit, l_NetGame, "number", "The current time limit in minutes. -1 means no limit.", "number", "the time limit")
{
	net_game_h current;
	int time = 0;
	if (!ade_get_args(L, "o|i", l_NetGame.Get(&current), &time))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (time < 0) {
			current.getNetgame()->options.mission_time_limit = fl2f(-1.0f);
		} else if (time >= MULTI_HO_MAX_TIME_LIMIT) {
			current.getNetgame()->options.mission_time_limit = MULTI_HO_MAX_TIME_LIMIT;
		} else {
			current.getNetgame()->options.mission_time_limit = fl2f(60.0f * (float)time);
		}
	}

	return ade_set_args(L, "i", current.getNetgame()->options.mission_time_limit);
}

ADE_VIRTVAR(KillLimit, l_NetGame, "number", "The current kill limit", "number", "the kill limit")
{
	net_game_h current;
	int kill = 0;
	if (!ade_get_args(L, "o|i", l_NetGame.Get(&current), &kill))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (kill < 0) {
			current.getNetgame()->options.kill_limit = 0;
		} else if (kill >= MULTI_HO_MAX_KILL_LIMIT) {
			current.getNetgame()->options.kill_limit = MULTI_HO_MAX_KILL_LIMIT;
		} else {
			current.getNetgame()->options.kill_limit = kill;
		}
	}

	return ade_set_args(L, "i", current.getNetgame()->options.kill_limit);
}

ADE_VIRTVAR(ObserverLimit, l_NetGame, "number", "The current observer limit", "number", "the observer limit")
{
	net_game_h current;
	int observer = 0;
	if (!ade_get_args(L, "o|i", l_NetGame.Get(&current), &observer))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (observer < 0) {
			current.getNetgame()->options.max_observers = 0;
		} else if (observer >= MULTI_HO_MAX_OBS) {
			current.getNetgame()->options.max_observers = MULTI_HO_MAX_OBS;
		} else {
			current.getNetgame()->options.max_observers = static_cast<ubyte>(observer);
		}
	}

	return ade_set_args(L, "i", current.getNetgame()->options.max_observers);
}

ADE_VIRTVAR(Locked, l_NetGame, "boolean", "Whether or not the loadouts have been locked for the current team. Can be set only by the host or team captain.", "boolean", "the locked status")
{
	net_game_h current;
	bool locked = false;
	if (!ade_get_args(L, "o|b", l_NetGame.Get(&current), &locked))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (locked) {
			multi_ts_lock_pressed();
		}
		return locked;
	}

	return ade_set_args(L, "b", static_cast<bool>(multi_ts_is_locked()));
}

ADE_VIRTVAR(Type, l_NetGame, "enumeration Type", "The current game type. Will be one of the MULTI_TYPE enums. Returns nil if there's an error.", "enumeration", "the game type")
{
	net_game_h current;
	enum_h *eh_idx = nullptr;
	if (!ade_get_args(L, "o|o", l_NetGame.Get(&current), l_Enum.GetPtr(&eh_idx)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (Net_player->flags & NETINFO_FLAG_AM_MASTER) {
			if (eh_idx != nullptr) {
				if (eh_idx->index == LE_MULTI_TYPE_COOP) {
					current.getNetgame()->type_flags = NG_TYPE_COOP;
				} else if (eh_idx->index == LE_MULTI_TYPE_TEAM) {
					current.getNetgame()->type_flags = NG_TYPE_TVT;
				} else if (eh_idx->index == LE_MULTI_TYPE_DOGFIGHT) {
					current.getNetgame()->type_flags = NG_TYPE_DOGFIGHT;
				} else if (eh_idx->index == LE_MULTI_TYPE_SQUADWAR) {
					current.getNetgame()->type_flags = NG_TYPE_SW;
				}
			}
			send_netgame_update_packet();
			multi_options_update_netgame();
		}
	}

	if (current.getNetgame()->type_flags == NG_TYPE_COOP) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_TYPE_COOP)));
	} else if (current.getNetgame()->type_flags == NG_TYPE_TVT) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_TYPE_TEAM)));
	} else if (current.getNetgame()->type_flags == NG_TYPE_DOGFIGHT) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_TYPE_DOGFIGHT)));
	} else if (current.getNetgame()->type_flags == NG_TYPE_SW) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_MULTI_TYPE_SQUADWAR)));
	}

	//catch all
	return ADE_RETURN_NIL;
}

ADE_FUNC(acceptOptions,
	l_NetGame,
	nullptr,
	"Accepts the current game options and pushes them to the the network.",
	"boolean",
	"returns true if successful, false otherwise")
{
	net_game_h current;
	if (!ade_get_args(L, "o", l_NetGame.Get(&current)))
		return ADE_RETURN_FALSE;

	// set default options
	current.getNetgame()->options.flags = (MSO_FLAG_INGAME_XFER | MSO_FLAG_ACCEPT_PIX);

	// store these values locally
	memcpy(&Player->m_local_options, &Net_player->p_info.options, sizeof(multi_local_options));
	memcpy(&Player->m_server_options, &Netgame.options, sizeof(multi_server_options));
	Pilot.save_player(Player);

	// apply any changes in settings (notify everyone of voice qos changes, etc)
	multi_ho_apply_options();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setMission,
	l_NetGame,
	"[net_mission | net_campaign]",
	"Sets the mission or campaign for the Netgame. Handles changing all netgame values and updating the server.",
	"boolean",
	"returns true if successful, false otherwise")
{
	net_game_h current;
	net_mission_h mission;
	net_campaign_h campaign;

	int abs_index = -1;
	int mode = -1;
	if (luacpp::convert::ade_odata_is_userdata_type(L, 2, l_NetMission)) {
		if (!ade_get_args(L, "oo", l_NetGame.Get(&current), l_NetMission.Get(&mission))) {
			return ADE_RETURN_FALSE;
		}
		mode = MULTI_CREATE_SHOW_MISSIONS;
		abs_index = mission.getIndex();
		current.getNetgame()->campaign_mode = MP_SINGLE_MISSION;
	} else {
		if (!ade_get_args(L, "oo", l_NetGame.Get(&current), l_NetCampaign.Get(&campaign))) {
			return ADE_RETURN_FALSE;
		}
		mode = MULTI_CREATE_SHOW_CAMPAIGNS;
		abs_index = campaign.getIndex();
		current.getNetgame()->campaign_mode = MP_CAMPAIGN;
	}

	multi_create_list_set_item(abs_index, mode);

	return ADE_RETURN_TRUE;
}

//**********HANDLE: channel section
ADE_OBJ(l_Active_Game, active_game_h, "active_game", "Active Game handle");

ADE_FUNC(isValid,
	l_Active_Game,
	nullptr,
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Status, l_Active_Game, nullptr, "The status of the game", "string", "The status")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	SCP_string status_text;
	multi_join_game_set_status_text(current.getGame(), status_text);

	return ade_set_args(L, "s", status_text.c_str());
}

ADE_VIRTVAR(Type, l_Active_Game, nullptr, "The type of the game", "string", "The type")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	SCP_string game_type;
	switch (current.getGame()->flags & AG_FLAG_TYPE_MASK) {
	case AG_FLAG_COOP:
		game_type = "coop";
		break;
	case AG_FLAG_TEAMS:
		game_type = "team";
		break;
	case AG_FLAG_DOGFIGHT:
		game_type = "dogfight";
		break;
	default:
		game_type = "";
		break;
	}

	return ade_set_args(L, "s", game_type.c_str());
}

ADE_VIRTVAR(Speed, l_Active_Game, nullptr, "The speed of the game", "string", "The speed")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	SCP_string speed_text;
	int con_type;
	multi_join_game_set_speed_text(current.getGame(), con_type, speed_text);

	return ade_set_args(L, "s", speed_text.c_str());
}

ADE_VIRTVAR(Standalone, l_Active_Game, nullptr, "Whether or not the game is standalone", "boolean", "True for standalone, false otherwise")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	bool standalone = false;
	if (current.getGame()->flags & AG_FLAG_STANDALONE) {
		standalone = true;
	}

	return ade_set_args(L, "b", standalone);
}

ADE_VIRTVAR(Campaign, l_Active_Game, nullptr, "Whether or not the game is campaign", "boolean", "True for campaign, false otherwise")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	bool campaign = false;
	if (current.getGame()->flags & AG_FLAG_CAMPAIGN) {
		campaign = true;
	}

	return ade_set_args(L, "b", campaign);
}

ADE_VIRTVAR(Server, l_Active_Game, nullptr, "The server name of the game", "string", "The server")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getGame()->name);
}

ADE_VIRTVAR(Mission, l_Active_Game, nullptr, "The mission name of the game", "string", "The mission")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getGame()->mission_name);
}

ADE_VIRTVAR(Ping, l_Active_Game, nullptr, "The ping average of the game", "number", "The ping")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getGame()->ping.ping_avg);
}

ADE_VIRTVAR(Players, l_Active_Game, nullptr, "The number of players in the game", "number", "The number of players")
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getGame()->num_players);
}

ADE_FUNC(setSelected, l_Active_Game, nullptr, "Sets the specified game as the selected game to possibly join. Must be used before sendJoinRequest will work.", nullptr, nullptr)
{
	active_game_h current;
	if (!ade_get_args(L, "o", l_Active_Game.Get(&current)))
		return ADE_RETURN_NIL;

	Multi_join_selected_item = current.getGame();

	return ADE_RETURN_NIL;
}

//**********HANDLE: channel section
ADE_OBJ(l_Dogfight_Scores, dogfight_scores_h, "dogfight_scores", "Dogfight scores handle");

ADE_FUNC(isValid,
	l_Dogfight_Scores,
	nullptr,
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	dogfight_scores_h current;
	if (!ade_get_args(L, "o", l_Dogfight_Scores.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Callsign,
	l_Dogfight_Scores,
	nullptr,
	"Gets the callsign for the player who's scores these are",
	"string",
	"the callsign or nil if invalid")
{
	dogfight_scores_h current;
	if (!ade_get_args(L, "o", l_Dogfight_Scores.Get(&current)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getScores()->callsign);
}

ADE_FUNC(getKillsOnPlayer,
	l_Dogfight_Scores,
	"net_player",
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	dogfight_scores_h current;
	net_player_h player;
	if (!ade_get_args(L, "oo", l_Dogfight_Scores.Get(&current), l_NetPlayer.Get(&player)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", current.getScores()->stats.m_dogfight_kills[player.getIndex()]);
}

//**********HANDLE: join choice section
ADE_OBJ(l_Join_Ship_Choice, join_ship_choices_h, "net_join_choice", "Join Choice handle");

ADE_FUNC(isValid,
	l_Join_Ship_Choice,
	nullptr,
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	join_ship_choices_h current;
	if (!ade_get_args(L, "o", l_Join_Ship_Choice.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Name, l_Join_Ship_Choice,
	nullptr,
	"Gets the name of the ship",
	"string",
	"the name or nil if invalid")
{
	join_ship_choices_h current;
	if (!ade_get_args(L, "o", l_Join_Ship_Choice.Get(&current)))
		return ADE_RETURN_NIL;

	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}
	
	return ade_set_args(L, "s", Ships[current.getObject()->instance].ship_name);
}

ADE_VIRTVAR(ShipIndex,
	l_Join_Ship_Choice,
	nullptr,
	"Gets the index of the ship class",
	"string",
	"the index or nil if invalid")
{
	join_ship_choices_h current;
	if (!ade_get_args(L, "o", l_Join_Ship_Choice.Get(&current)))
		return ADE_RETURN_NIL;

	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", Ships[current.getObject()->instance].ship_info_index);
}

ADE_FUNC(getPrimaryWeaponsList,
	l_Join_Ship_Choice,
	nullptr,
	"Gets the table of primary weapon indexes on the ship",
	"table",
	"the table of indexes or nil if invalid")
{
	join_ship_choices_h current;
	if (!ade_get_args(L, "o", l_Join_Ship_Choice.Get(&current)))
		return ADE_RETURN_NIL;

	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	luacpp::LuaTable weapons = luacpp::LuaTable::create(L);

	auto wp = &Ships[current.getObject()->instance].weapons;

	for (int idx = 0; idx < wp->num_primary_banks; idx++) {
		weapons.addValue(idx, wp->primary_bank_weapons[idx]);
	}

	return ade_set_args(L, "t", weapons);
}

ADE_FUNC(getSecondaryWeaponsList,
	l_Join_Ship_Choice,
	nullptr,
	"Gets the table of secondary weapon indexes on the ship",
	"table",
	"the table of indexes or nil if invalid")
{
	join_ship_choices_h current;
	if (!ade_get_args(L, "o", l_Join_Ship_Choice.Get(&current)))
		return ADE_RETURN_NIL;

	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	luacpp::LuaTable weapons = luacpp::LuaTable::create(L);

	auto wp = &Ships[current.getObject()->instance].weapons;

	for (int idx = 0; idx < wp->num_secondary_banks; idx++) {
		weapons.addValue(idx, wp->secondary_bank_weapons[idx]);
	}

	return ade_set_args(L, "t", weapons);
}

ADE_FUNC(getStatus,
	l_Join_Ship_Choice,
	nullptr,
	"Gets the status of the ship's hull and shields",
	"number table",
	"The hull health and then a table of shield quadrant healths")
{
	join_ship_choices_h current;
	if (!ade_get_args(L, "o", l_Join_Ship_Choice.Get(&current)))
		return ADE_RETURN_NIL;

	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	float hull = get_hull_pct(current.getObject());
	float max_shield = shield_get_max_quad(current.getObject());

	luacpp::LuaTable shields = luacpp::LuaTable::create(L);
	int n_quadrants = static_cast<int>(current.getObject()->shield_quadrant.size());
	for (int i = 0; i < n_quadrants; i++) {
		float temp_float = current.getObject()->shield_quadrant[i] / max_shield;
		shields.addValue(i + 1, temp_float);
	}

	return ade_set_args(L, "ft", hull, shields);
}

ADE_FUNC(setChoice,
	l_Join_Ship_Choice,
	nullptr,
	"Sets the current ship as chosen when Accept is clicked", "boolean",
	"returns true if successful, Nil if there's handle error.")
{
	join_ship_choices_h current;
	if (!ade_get_args(L, "o", l_Join_Ship_Choice.Get(&current)))
		return ADE_RETURN_NIL;

	if (!current.isValid()) {
		return ADE_RETURN_FALSE;
	}

	multi_ingame_set_selected(current.getIndex());

	return ADE_RETURN_TRUE;
}

} // namespace api
} // namespace scripting