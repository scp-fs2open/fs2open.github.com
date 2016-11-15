
#include "LuaException.h"
#include "LuaConvert.h"
#include "LuaTable.h"
#include "LuaFunction.h"
#include "LuaReference.h"

#include "LuaHeaders.h"

namespace {
bool isValidIndex(lua_State* state, int index) {
	if (1 <= abs(index) && abs(index) <= lua_gettop(state)) {
		return true;
	} else {
		return false;
	}
}
}

namespace luacpp {
namespace convert {
template<>
void pushValue<double>(lua_State* luaState, const double& value) {
	lua_pushnumber(luaState, value);
}

template<>
void pushValue<float>(lua_State* luaState, const float& value) {
	lua_pushnumber(luaState, value);
}

template<>
void pushValue<int>(lua_State* luaState, const int& value) {
	lua_pushnumber(luaState, value);
}

template<>
void pushValue<size_t>(lua_State* luaState, const size_t& value) {
	lua_pushnumber(luaState, value);
}

template<>
void pushValue<std::string>(lua_State* luaState, const std::string& value) {
	lua_pushlstring(luaState, value.c_str(), value.size());
}

template<>
void pushValue<const char*>(lua_State* luaState, const char* const& value) {
	lua_pushstring(luaState, value);
}

template<>
void pushValue<bool>(lua_State* luaState, const bool& value) {
	lua_pushboolean(luaState, value);
}

template<>
void pushValue<lua_CFunction>(lua_State* luaState,
							  const lua_CFunction& value) {
	lua_pushcfunction(luaState, value);
}

template<>
void pushValue<LuaValue>(lua_State* luaState, const LuaValue& value) {
	if (luaState != value.getLuaState()) {
		throw LuaException("Lua state mismatch!");
	}

	value.pushValue();
}

template<>
double popValue<double>(lua_State* luaState, int stackposition, bool remove) {
	if (!isValidIndex(luaState, stackposition)) {
		throw LuaException("Specified stack position is not valid!");
	}

	if (!lua_isnumber(luaState, stackposition)) {
		throw LuaException("Specified position is no number!");
	} else {
		double number = lua_tonumber(luaState, stackposition);

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return number;
	}
}

template<>
float popValue<float>(lua_State* luaState, int stackposition, bool remove) {
	return static_cast<float>(popValue<double>(luaState, stackposition, remove));
}

template<>
int popValue<int>(lua_State* luaState, int stackposition, bool remove) {
	return static_cast<int>(popValue<double>(luaState, stackposition, remove));
}

template<>
size_t popValue<size_t>(lua_State* luaState, int stackposition, bool remove) {
	return static_cast<size_t>(popValue<double>(luaState, stackposition, remove));
}

template<>
std::string popValue<std::string>(lua_State* luaState, int stackposition, bool remove) {
	if (!isValidIndex(luaState, stackposition)) {
		throw LuaException("Specified stack position is not valid!");
	}

	if (!lua_isstring(luaState, stackposition)) {
		throw LuaException("Specified index is no string!");
	} else {
		std::string target;

		size_t size;
		const char* string = lua_tolstring(luaState, stackposition, &size);
		target.assign(string, size);

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return target;
	}
}

template<>
bool popValue<bool>(lua_State* luaState, int stackposition, bool remove) {
	if (!isValidIndex(luaState, stackposition)) {
		throw LuaException("Specified stack position is not valid!");
	}

	if (!lua_isboolean(luaState, stackposition)) {
		throw LuaException("Specified index is no boolean value!");
	} else {
		bool target = lua_toboolean(luaState, stackposition) != 0;

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return target;
	}
}

template<>
lua_CFunction popValue<lua_CFunction>(lua_State* luaState, int stackposition, bool remove) {
	if (!isValidIndex(luaState, stackposition)) {
		throw LuaException("Specified stack position is not valid!");
	}

	if (!lua_iscfunction(luaState, stackposition)) {
		throw LuaException("Specified index is no C-function!");
	} else {
		lua_CFunction target = lua_tocfunction(luaState, stackposition);

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return target;
	}
}

template<>
LuaTable popValue<LuaTable>(lua_State* luaState, int stackposition, bool remove) {
	if (!isValidIndex(luaState, stackposition)) {
		throw LuaException("Specified stack position is not valid!");
	}

	if (!lua_istable(luaState, stackposition)) {
		throw LuaException("Specified index is no table!");
	} else {
		LuaTable target;
		target.setReference(UniqueLuaReference::create(luaState, stackposition));

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return target;
	}
}

template<>
LuaFunction popValue<LuaFunction>(lua_State* luaState, int stackposition, bool remove) {
	if (!isValidIndex(luaState, stackposition)) {
		throw LuaException("Specified stack position is not valid!");
	}

	if (!lua_isfunction(luaState, stackposition)) {
		throw LuaException("Specified index is no function!");
	} else {
		LuaFunction target;
		target.setReference(UniqueLuaReference::create(luaState, stackposition));

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return target;
	}
}

template<>
LuaValue popValue<LuaValue>(lua_State* luaState, int stackposition, bool remove) {
	if (!isValidIndex(luaState, stackposition)) {
		throw LuaException("Specified stack position is not valid!");
	}

	LuaValue target;
	target.setReference(UniqueLuaReference::create(luaState, stackposition));

	if (remove) {
		lua_remove(luaState, stackposition);
	}

	return target;
}
}
}