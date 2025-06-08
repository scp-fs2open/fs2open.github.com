
#include "comm_order.h"
#include "hud/hudsquadmsg.h"

namespace scripting::api {

//**********HANDLE: mission goals
ADE_OBJ_NO_MULTI(l_Comm_Item, int, "comm_item", "Comm Item handle");

ADE_VIRTVAR(Name, l_Comm_Item, nullptr, "The name of the comm item", "string", "The comm item name")
{
	int current;
	if (!ade_get_args(L, "o", l_Comm_Item.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", MsgItems[current].text.c_str());
}

ADE_VIRTVAR(Active, l_Comm_Item, nullptr, "Whether or not the item is active", "boolean", "The active status")
{
	int current;
	if (!ade_get_args(L, "o", l_Comm_Item.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (MsgItems[current].active > 0) {
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

// Yes, it's a little silly to get this this way. Adding a getter to the mission library was an option but for that I'd
// have to be sure the title is always set which is fine but mostly it just felt out of place. Having this here keeps all
// comm menu code neat and tidy right here. I also thought about adding it as a parameter of the On HUD Comm Menu Open hook
// but that method wouldn't update as the player moves through the menu since the hook only runs once.
ADE_FUNC(getMenuTitle, l_Comm_Item, nullptr, "Gets the current title of the menu. Will be the same for all currently valid comm items.", "string", "The menu title")
{
	int current = -1;

	if (!ade_get_args(L, "o", l_Comm_Item.Get(&current)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "s", Squad_msg_title);
}

ADE_FUNC(selectItem, l_Comm_Item, nullptr, "Selects the item and either proceeds to the next menu or issues the order", "boolean", "Returns true if successful, false otherwise")
{
	int current = -1;

	if (!ade_get_args(L, "o", l_Comm_Item.Get(&current)))
		return ADE_RETURN_FALSE;

	if (current < 0 || current >= Num_menu_items) {
		LuaError(L, "Lua tried to select squad message that is not valid!");
		return ADE_RETURN_FALSE;
	}

	Hud_set_lua_key(current);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isValid, l_Comm_Item, nullptr, "Detect if the handle is valid", "boolean", "true if valid, false otherwise")
{
	int current = -1;

	if (!ade_get_args(L, "o", l_Comm_Item.Get(&current)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", (current >= 0) && (current < Num_menu_items));
}

} // namespace scripting::api
