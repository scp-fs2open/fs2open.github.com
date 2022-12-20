//
//

#include "LuaEnum.h"

namespace scripting {
namespace api {

lua_enum_h::lua_enum_h() : lua_enum(-1) {}

lua_enum_h::lua_enum_h(int idx) : lua_enum(idx) {}

dynamic_sexp_enum_list* lua_enum_h::getEnum() const
{
	return &Dynamic_enums[lua_enum];
}

ADE_OBJ(l_LuaEnum, lua_enum_h, "LuaEnum", "Lua Enum handle");

ADE_VIRTVAR(Name, l_LuaEnum, "string", "The enum name", "string", "The enum name")
{
	lua_enum_h lua_enum;
	const char* enum_name;
	if (!ade_get_args(L, "o|s", l_LuaEnum.Get(&lua_enum), &enum_name))
		return ade_set_error(L, "o", l_LuaEnum.Set(lua_enum_h()));

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", lua_enum.getEnum()->name.c_str());
}

ADE_INDEXER(l_LuaEnum, "number Index", "Array of enum items", "string", "enum item string, or nil if index or enum handle is invalid")
{
	lua_enum_h lua_enum;
	int idx = -1;
	if (!ade_get_args(L, "o|i", l_LuaEnum.Get(&lua_enum), &idx))
		return ADE_RETURN_NIL;

	if ((idx < 0) || (idx >= (int)lua_enum.getEnum()->list.size()))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "s", lua_enum.getEnum()->list[idx].c_str());
}

ADE_FUNC(__len, l_LuaEnum, nullptr, "The number of Lua enum items", "number", "The number of Lua enums item.")
{
	lua_enum_h lua_enum;
	if (!ade_get_args(L, "o", l_LuaEnum.Get(&lua_enum)))
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", (int)lua_enum.getEnum()->list.size());
}

ADE_FUNC(addEnumItem,
	l_LuaEnum,
	"string itemname",
	"Adds an enum item with the given string.",
	nullptr,
	"Returns nothing")
{
	lua_enum_h lua_enum;
	const char* item_name;
	if (!ade_get_args(L, "os", l_LuaEnum.Get(&lua_enum) , &item_name))
		return ade_set_error(L, "o", l_LuaEnum.Set(lua_enum_h()));

	lua_enum.getEnum()->list.push_back(item_name);

	return ADE_RETURN_NIL;
}

} // namespace api
} // namespace scripting
