
#include "LuaConvert.h"

namespace luacpp {
namespace convert {
namespace internal {

bool isValidIndex(lua_State* state, int index) {
	if (1 <= std::abs(index) && std::abs(index) <= lua_gettop(state)) {
		return true;
	} else {
		return false;
	}
}

bool ade_odata_helper(lua_State* L, int stackposition, size_t idx) {
	if (!ade_odata_is_userdata_type(L, stackposition, idx)) {
		// Issue the LuaError here since this is the only place where we have all relevant information
		LuaError(L, "Argument %d is the wrong type of userdata; '%s' given, but '%s' expected", stackposition,
			::scripting::internal::getTableEntry((size_t)lua_tointeger(L, -2)).Name,
			::scripting::internal::getTableEntry(idx).GetName());
		return false;
	}
	else {
		return true;
	}
}

}

bool ade_odata_is_userdata_type(lua_State* L, int stackposition, size_t typeIdx) {
	// WMC - Get metatable
	lua_getmetatable(L, stackposition);
	int mtb_ldx = lua_gettop(L);

	if (lua_isnil(L, -1)) {
		return false;
	}

	// make sure the argument actually exists
	if (mtb_ldx < stackposition) {
		return false;
	}

	// Get ID
	lua_pushstring(L, "__adeid");
	lua_rawget(L, mtb_ldx);

	if (lua_tonumber(L, -1) != typeIdx) {
		lua_pushstring(L, "__adederivid");
		lua_rawget(L, mtb_ldx);
		if ((size_t)lua_tointeger(L, -1) != typeIdx) {
			return false;
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 2);
	return true;
}

void pushValue(lua_State* luaState, const double& value) {
	lua_pushnumber(luaState, value);
}
void pushValue(lua_State* luaState, const float& value) {
	lua_pushnumber(luaState, value);
}
void pushValue(lua_State* luaState, const int& value) {
	lua_pushnumber(luaState, value);
}
void pushValue(lua_State* luaState, const size_t& value) {
	lua_pushnumber(luaState, (lua_Number)value);
}
void pushValue(lua_State* luaState, const std::string& value) {
	lua_pushlstring(luaState, value.c_str(), value.size());
}
void pushValue(lua_State* luaState, const char* value) {
	lua_pushstring(luaState, value);
}
void pushValue(lua_State* luaState, const bool& value) {
	lua_pushboolean(luaState, value);
}
void pushValue(lua_State* luaState, const lua_CFunction& value) {
	lua_pushcfunction(luaState, value);
}

bool popValue(lua_State* luaState, int& target, int stackposition, bool remove) {
	double temp;

	auto ret = popValue(luaState, temp, stackposition, remove);
	if (ret) {
		target = (int)temp;
	}
	return ret;
}
bool popValue(lua_State* luaState, size_t& target, int stackposition, bool remove) {
	double temp;

	auto ret = popValue(luaState, temp, stackposition, remove);
	if (ret) {
		target = (size_t)temp;
	}
	return ret;
}
bool popValue(lua_State* luaState, float& target, int stackposition, bool remove) {
	double temp;

	auto ret = popValue(luaState, temp, stackposition, remove);
	if (ret) {
		target = (float)temp;
	}
	return ret;
}
bool popValue(lua_State* luaState, double& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_isnumber(luaState, stackposition)) {
		return false;
	} else {
		target = lua_tonumber(luaState, stackposition);

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return true;
	}
}
bool popValue(lua_State* luaState, std::string& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_isstring(luaState, stackposition)) {
		return false;
	} else {
		size_t size;
		const char* string = lua_tolstring(luaState, stackposition, &size);
		target.assign(string, size);

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return true;
	}
}

bool popValue(lua_State* luaState, bool& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_isboolean(luaState, stackposition)) {
		return false;
	} else {
		target = lua_toboolean(luaState, stackposition) != 0;

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return true;
	}
}

bool popValue(lua_State* luaState, lua_CFunction& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_iscfunction(luaState, stackposition)) {
		return false;
	} else {
		target = lua_tocfunction(luaState, stackposition);

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return true;
	}
}

}
}
