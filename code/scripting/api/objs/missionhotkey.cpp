#include "missionhotkey.h"

#include "mission/missionhotkey.h"

namespace scripting {
namespace api {

hotkey_h::hotkey_h() : line(-1) {}
hotkey_h::hotkey_h(int l_line) : line(l_line) {}

hotkey_line* hotkey_h::getLine() const
{
	return &Hotkey_lines[line];
}

int hotkey_h::getIndex() const
{
	return line;
}

//**********HANDLE: help section
ADE_OBJ(l_Hotkey, hotkey_h, "hotkey_ship", "Help Section handle");

ADE_VIRTVAR(Text, l_Hotkey, nullptr, "The text of this hotkey line", "string", "The text")
{
	hotkey_h current;
	if (!ade_get_args(L, "o", l_Hotkey.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getLine()->label);
}

ADE_VIRTVAR(Type,
	l_Hotkey,
	nullptr,
	"The type of this hotkey line. 0 for nothing, 1 for heading, 2 for wing, 3 for ship, 4 for ship in a wing",
	"number",
	"The type")
{
	hotkey_h current;
	if (!ade_get_args(L, "o", l_Hotkey.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getLine()->type);
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

	int hotkeys;

	if (current.getLine()->type == HOTKEY_LINE_WING)
		hotkeys = get_wing_hotkeys(Hotkey_lines[current.getIndex()].index); // for wings
	else
		hotkeys = get_ship_hotkeys(Hotkey_lines[current.getIndex()].index); // for everything else (there's mastercard)

	auto table = luacpp::LuaTable::create(L);

	for (int i = 0; i < MAX_KEYED_TARGETS; i++) {
		bool key_active = false;
		if (hotkeys & (1 << i)) {
			key_active = true;
		}
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
	key--;

	int hotkey = Key_sets[key];

	add_hotkey(current.getIndex(), hotkey);

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
	key--;

	int hotkey = Key_sets[key];

	remove_hotkey(current.getIndex(), hotkey);

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

	clear_hotkeys(current.getIndex());

	return ADE_RETURN_NIL;
}

} // namespace api
} // namespace scripting