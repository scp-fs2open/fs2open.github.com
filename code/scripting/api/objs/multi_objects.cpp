#include "multi_objects.h"

#include "network/multi_pxo.h"
#include "network/multiui.h"

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

ADE_VIRTVAR(NumPlayers, l_Channel, nullptr, "The number of players in the channel", "string", "The number of players")
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

ADE_VIRTVAR(NumGames, l_Channel, nullptr, "The number of games the channel", "string", "The number of games")
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

} // namespace api
} // namespace scripting