#include "missionlog.h"



#include "iff_defs/iff_defs.h"
#include "ship/ship.h"
#include "globalincs/alphacolors.h"
#include "parse/parselo.h"
#include "scripting/api/objs/color.h"

namespace scripting {
namespace api {

log_entry_h::log_entry_h() : section(-1) {}
log_entry_h::log_entry_h(int l_section) : section(l_section) {}

log_line_complete* log_entry_h::getSection() const
{
	return &Log_scrollback_vec[section];
}

message_entry_h::message_entry_h() : section(-1) {}
message_entry_h::message_entry_h(int l_section) : section(l_section) {}

line_node* message_entry_h::getSection() const
{
	return &Msg_scrollback_vec[section];
}

//**********HANDLE: log entry
ADE_OBJ(l_Log_Entry, log_entry_h, "log_entry", "Log Entry handle");

ADE_VIRTVAR(Timestamp, l_Log_Entry, nullptr, "The timestamp of the log entry", "string", "The timestamp")
{
	log_entry_h current;
	if (!ade_get_args(L, "o", l_Log_Entry.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	int seconds = fl2i(f2fl(current.getSection()->timestamp));

	// format the time information into strings
	SCP_string time;
	sprintf(time, "%.1d:%.2d:%.2d", (seconds / 3600) % 10, (seconds / 60) % 60, seconds % 60);

	return ade_set_args(L, "s", time.c_str());
}

ADE_VIRTVAR(Flags, l_Log_Entry, nullptr, "The flag of the log entry. 1 for Goal True, 2 for Goal Failed, 0 otherwise.", "string", "The flag")
{
	log_entry_h current;
	if (!ade_get_args(L, "o", l_Log_Entry.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	int this_flag = 0;

	if (current.getSection()->objective.flags & LOG_FLAG_GOAL_TRUE)
		this_flag = 1;

	if (current.getSection()->objective.flags & LOG_FLAG_GOAL_FAILED)
		this_flag = 2;

	return ade_set_args(L, "i", this_flag);
}

ADE_VIRTVAR(ObjectiveText, l_Log_Entry, nullptr, "The objective text of the log entry", "string", "The objective text")
{
	log_entry_h current;
	if (!ade_get_args(L, "o", l_Log_Entry.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getSection()->objective.text.get());
}

ADE_VIRTVAR(ObjectiveColor, l_Log_Entry, nullptr, "The objective color of the log entry.", "color", "The objective color")
{
	log_entry_h current;
	if (!ade_get_args(L, "o", l_Log_Entry.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	color ac = log_line_get_color(current.getSection()->objective.color);

	return ade_set_args(L, "o", l_Color.Set(ac));
}

ADE_VIRTVAR(ActionTexts,
	l_Log_Entry,
	nullptr,
	"Gets a table of action texts in the log entry",
	"{ number => string ... }",
	"The action texts table")
{
	log_entry_h current;
	if (!ade_get_args(L, "o", l_Log_Entry.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	auto table = luacpp::LuaTable::create(L);

	for (size_t i = 0; i < current.getSection()->actions.size(); i++) {
		table.addValue(i + 1, current.getSection()->actions[i].text.get()); // translate to Lua index
	}

	return ade_set_args(L, "t", &table);
}

ADE_VIRTVAR(ActionColors,
	l_Log_Entry,
	nullptr,
	"Gets a table of action colors in the log entry.",
	"{ number => color ... }",
	"The action colors table")
{
	log_entry_h current;
	if (!ade_get_args(L, "o", l_Log_Entry.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	auto table = luacpp::LuaTable::create(L);

	for (size_t i = 0; i < current.getSection()->actions.size(); i++) {

		color ac = log_line_get_color(current.getSection()->actions[i].color);
		table.addValue(i + 1, l_Color.Set(ac)); // translate to Lua index
	}

	return ade_set_args(L, "t", &table);
}

//**********HANDLE: message entry
ADE_OBJ(l_Message_Entry, message_entry_h, "message_entry", "Log Entry handle");

ADE_VIRTVAR(Timestamp, l_Message_Entry, nullptr, "The timestamp of the message entry", "string", "The timestamp")
{
	message_entry_h current;
	if (!ade_get_args(L, "o", l_Message_Entry.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	int seconds = fl2i(f2fl(current.getSection()->time));

	// format the time information into strings
	SCP_string time;
	sprintf(time, "%.1d:%.2d:%.2d", (seconds / 3600) % 10, (seconds / 60) % 60, seconds % 60);

	return ade_set_args(L, "s", time.c_str());
}

ADE_VIRTVAR(Color, l_Message_Entry, nullptr, "The color of the message entry.", "color", "The color")
{
	message_entry_h current;
	if (!ade_get_args(L, "o", l_Message_Entry.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}
	color this_color;

	int team = HUD_source_get_team(current.getSection()->source);

	if (team >= 0) {
		this_color = *iff_get_color_by_team(team, Player_ship->team, 0);
	} else {
		switch (current.getSection()->source) {
		case HUD_SOURCE_TRAINING:
			this_color = Color_bright_blue;
			break;

		case HUD_SOURCE_TERRAN_CMD:
			this_color = Color_bright_white;
			break;

		case HUD_SOURCE_IMPORTANT:
		case HUD_SOURCE_FAILED:
		case HUD_SOURCE_SATISFIED:
			this_color = Color_bright_white;
			break;

		default:
			this_color = Color_text_normal;
			break;
		}
	}

	return ade_set_args(L, "o", l_Color.Set(this_color));
}

ADE_VIRTVAR(Text, l_Message_Entry, nullptr, "The text of the message entry", "string", "The text")
{
	message_entry_h current;
	if (!ade_get_args(L, "o", l_Message_Entry.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getSection()->text.c_str());
}

} // namespace api
} // namespace scripting