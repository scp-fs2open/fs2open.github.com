#include "multi_objects.h"

#include "network/multi_pxo.h"

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

} // namespace api
} // namespace scripting