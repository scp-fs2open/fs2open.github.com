#ifndef LUAUTIL_H
#define LUAUTIL_H
#pragma once

#include "LuaHeaders.h"
#include "LuaException.h"
#include "LuaConvert.h"
#include "LuaReference.h"

#include <cstdlib>

namespace luacpp {
/**
 * @brief Contains functions to convert C++ values to and from lua values.
 *
 * Currently the following types are supported:
 *   - `double`
 *   - `float`
 *   - `int`
 *   - `std::string`
 *   - `const char*` (only for pushing as using the pointer after it was removed from the stack is dangerous)
 *   - `bool`
 *   - `lua_CFunction`
 *   - `LuaTable` (only pop, use LuaValue version for pushing)
 *   - `LuaFunction` (only pop, use LuaValue version for pushing)
 *   - `LuaValue` (this will reference any value at the specified poition)
 */
namespace convert {

namespace {

bool isValidIndex(lua_State* state, int index) {
	if (1 <= std::abs(index) && std::abs(index) <= lua_gettop(state)) {
		return true;
	} else {
		return false;
	}
}

}
/**
 * @brief Pushes the lua value of the given @c value onto the lua stack.
 * @param luaState The lua_State to push the values to
 * @param value The value which should be pushed.
 *
 * @tparam ValueType The type of the value being pushed, for a custom type you
 * 	need to specialize this template.
 */
template<class ValueType>
void pushValue(lua_State* luaState, const ValueType& value);


/**
 * @brief Convinient function for string literals
 *
 * @param luaState The lua_State to push the values to
 * @param value The value which should be pushed.
 * @return void
 */
template<size_t N>
inline void pushValue(lua_State* luaState, const char(& value)[N]) {
	pushValue<const char*>(luaState, value);
}

template<>
inline void pushValue<double>(lua_State* luaState, const double& value) {
	lua_pushnumber(luaState, value);
}

template<>
inline void pushValue<float>(lua_State* luaState, const float& value) {
	lua_pushnumber(luaState, value);
}

template<>
inline void pushValue<int>(lua_State* luaState, const int& value) {
	lua_pushnumber(luaState, value);
}

template<>
inline void pushValue<size_t>(lua_State* luaState, const size_t& value) {
	lua_pushnumber(luaState, (lua_Number)value);
}

template<>
inline void pushValue<std::string>(lua_State* luaState, const std::string& value) {
	lua_pushlstring(luaState, value.c_str(), value.size());
}

template<>
inline void pushValue<const char*>(lua_State* luaState, const char* const& value) {
	lua_pushstring(luaState, value);
}

template<>
inline void pushValue<bool>(lua_State* luaState, const bool& value) {
	lua_pushboolean(luaState, value);
}

template<>
inline void pushValue<lua_CFunction>(lua_State* luaState,
							  const lua_CFunction& value) {
	lua_pushcfunction(luaState, value);
}


/**
* @brief Pops a value or throws an exception.
* If the conversion of the lua value fails, this function throws an exception.
*
* @param luaState The lua state
* @param stackPos The stack position of the value. Defaults to -1.
* @param remove Specifies if the value should be removed. Defaults to true.
* @return ValueType The type of value to be poped from the stack, must have a default and copy constructor.
*/
template<class ValueType>
ValueType popValue(lua_State* luaState, int stackPos = -1, bool remove = true);


template<>
inline double popValue<double>(lua_State* luaState, int stackposition, bool remove) {
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
inline float popValue<float>(lua_State* luaState, int stackposition, bool remove) {
	return static_cast<float>(popValue<double>(luaState, stackposition, remove));
}

template<>
inline int popValue<int>(lua_State* luaState, int stackposition, bool remove) {
	return static_cast<int>(popValue<double>(luaState, stackposition, remove));
}

template<>
inline size_t popValue<size_t>(lua_State* luaState, int stackposition, bool remove) {
	return static_cast<size_t>(popValue<double>(luaState, stackposition, remove));
}

template<>
inline std::string popValue<std::string>(lua_State* luaState, int stackposition, bool remove) {
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
inline bool popValue<bool>(lua_State* luaState, int stackposition, bool remove) {
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
inline lua_CFunction popValue<lua_CFunction>(lua_State* luaState, int stackposition, bool remove) {
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

/**
 * @brief Pops a value from the lua stack and stores it.
 *
 * This function checks if the topmost value on the lua stack is of the right type and if
 * that is the case pops this value from the stack and stors the value inside @c target.
 *
 * @param luaState The lua_State which should be checked
 * @param target The location where the value should be stored
 * @param stackPos The position of the value that should be used. Defaults to -1
 * @param remove @c true to remove the value from the stack, @c false to leave it on the stack
 * @return @c true when the value could successfully be converted,
 * 		@c false if the topmost value is not of the right type.
 */
template<class ValueType>
inline bool popValue(lua_State* L, ValueType& target, int stackPos = -1, bool remove = true) {
	try {
		target = popValue<ValueType>(L, stackPos, remove);
		return true;
	}
	catch (LuaException&) {
		return false;
	}
}
}
}


#endif