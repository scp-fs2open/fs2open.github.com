#ifndef LUAUTIL_H
#define LUAUTIL_H
#pragma once

#include "LuaHeaders.h"
#include "LuaException.h"

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
void pushValue(lua_State* luaState, const char(& value)[N]) {
	pushValue<const char*>(luaState, value);
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

/**
 * @brief Pops a value from the lua stack and stors it.
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
bool popValue(lua_State* L, ValueType& target, int stackPos = -1, bool remove = true) {
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