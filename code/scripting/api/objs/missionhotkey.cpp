#include "missionhotkey.h"
#include "enums.h"

#include "mission/missionhotkey.h"
#include "playerman/player.h"

namespace scripting {
namespace api {

hotkey_h::hotkey_h() : line(-1) {}
hotkey_h::hotkey_h(int l_line) : line(l_line) {}

hotkey_line* hotkey_h::getLine() const
{
	if (!isValid())
		return nullptr;

	return &Hotkey_lines[line];
}

int hotkey_h::getIndex() const
{
	return line;
}

bool hotkey_h::isValid() const
{
	return line >= 0 && line < MAX_LINES;
}

//**********HANDLE: help section
ADE_OBJ(l_Hotkey, hotkey_h, "hotkey_ship", "Hotkey handle");

ADE_FUNC(isValid, l_Hotkey, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	hotkey_h current;
	if (!ade_get_args(L, "o", l_Hotkey.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Text, l_Hotkey, nullptr, "The text of this hotkey line", "string", "The text")
{
	hotkey_h current;
	if (!ade_get_args(L, "o", l_Hotkey.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getLine()->label);
}

ADE_VIRTVAR(Type,
	l_Hotkey,
	nullptr,
	"The type of this hotkey line: HOTKEY_LINE_NONE, HOTKEY_LINE_HEADING, HOTKEY_LINE_WING, HOTKEY_LINE_SHIP, or HOTKEY_LINE_SUBSHIP.",
	"enumeration",
	"The type")
{
	hotkey_h current;
	lua_enum eh_idx = ENUM_INVALID;

	if (!ade_get_args(L, "o", l_Hotkey.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "o", l_Enum.Set(enum_h(eh_idx)));
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	switch (current.getLine()->type) {
		case HotkeyLineType::HEADING:
			eh_idx = LE_HOTKEY_LINE_HEADING;
			break;
		case HotkeyLineType::WING:
			eh_idx = LE_HOTKEY_LINE_WING;
			break;
		case HotkeyLineType::SHIP:
			eh_idx = LE_HOTKEY_LINE_SHIP;
			break;
		case HotkeyLineType::SUBSHIP:
			eh_idx = LE_HOTKEY_LINE_SUBSHIP;
			break;
		case HotkeyLineType::NONE:
		default:
			eh_idx = LE_HOTKEY_LINE_NONE;
			break;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(eh_idx)));
}

ADE_VIRTVAR(Keys,
	l_Hotkey,
	nullptr,
	"Gets a table of hotkeys set to the ship in the order from F5 - F12",
	"{ number => boolean ... }",
	"The hotkeys table")
{
	hotkey_h current;
	if (!ade_get_args(L, "o", l_Hotkey.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	auto table = luacpp::LuaTable::create(L);
	if (!current.isValid()) {
		return ade_set_error(L, "t", &table);
	}

	int hotkeys;
	if (current.getLine()->type == HotkeyLineType::WING)
		hotkeys = get_wing_hotkeys(current.getIndex()); // for wings
	else
		hotkeys = get_ship_hotkeys(current.getIndex()); // for everything else (there's mastercard)

	for (int i = 0; i < MAX_KEYED_TARGETS; i++) {
		bool key_active = hotkeys & (1 << i);
		table.addValue(i + 1, key_active); // translate to Lua index
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(addHotkey,
	l_Hotkey,
	"number Key",
	"Adds a hotkey to the to the ship in the list. 1-8 correspond to F5-F12. Returns nothing.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	hotkey_h current;
	int key;
	if (!ade_get_args(L, "oi", l_Hotkey.Get(&current), &key)) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}
	key--;

	add_hotkey(key, current.getIndex());

	return ADE_RETURN_NIL;
}

ADE_FUNC(removeHotkey,
	l_Hotkey,
	"number Key",
	"Removes a hotkey from the ship in the list. 1-8 correspond to F5-F12. Returns nothing.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	hotkey_h current;
	int key;
	if (!ade_get_args(L, "oi", l_Hotkey.Get(&current), &key)) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}
	key--;

	remove_hotkey(key, current.getIndex());

	return ADE_RETURN_NIL;
}

ADE_FUNC(clearHotkeys,
	l_Hotkey,
	nullptr,
	"Clears all hotkeys from the ship in the list. Returns nothing.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	hotkey_h current;
	if (!ade_get_args(L, "o", l_Hotkey.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	clear_hotkeys(current.getIndex());

	return ADE_RETURN_NIL;
}

} // namespace api
} // namespace scripting