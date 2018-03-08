#ifndef LUAUTIL_H
#define LUAUTIL_H
#pragma once

#include "LuaHeaders.h"
#include "LuaException.h"
#include "LuaConvert.h"
#include "LuaReference.h"

#include "scripting/ade.h"

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

namespace internal {
bool isValidIndex(lua_State* state, int index);
}

void pushValue(lua_State* luaState, const double& value);

void pushValue(lua_State* luaState, const float& value);

void pushValue(lua_State* luaState, const int& value);

void pushValue(lua_State* luaState, const size_t& value);

void pushValue(lua_State* luaState, const std::string& value);

void pushValue(lua_State* luaState, const char* value);

void pushValue(lua_State* luaState, const bool& value);

void pushValue(lua_State* luaState, const lua_CFunction& value);

void pushValue(lua_State* L, const scripting::ade_odata& value);

/**
 * @brief Convenience function for string literals
 *
 * @param luaState The lua_State to push the values to
 * @param value The value which should be pushed.
 * @return void
 */
template<size_t N>
inline void pushValue(lua_State* luaState, const char(& value)[N]) {
	pushValue(luaState, (const char*)value);
}


bool popValue(lua_State* luaState, float& target, int stackposition = -1, bool remove = true);

bool popValue(lua_State* luaState, double& target, int stackposition = -1, bool remove = true);

bool popValue(lua_State* luaState, int& target, int stackposition = -1, bool remove = true);

bool popValue(lua_State* luaState, size_t& target, int stackposition = -1, bool remove = true);

bool popValue(lua_State* luaState, std::string& target, int stackposition = -1, bool remove = true);

bool popValue(lua_State* luaState, bool& target, int stackposition = -1, bool remove = true);

bool popValue(lua_State* luaState, lua_CFunction& target, int stackposition = -1, bool remove = true);

bool popValue(lua_State* L, scripting::ade_odata& od, int stackposition = -1, bool remove = true);

}
}


#endif
