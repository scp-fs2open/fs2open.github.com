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

bool ade_odata_helper(lua_State* L, int stackposition, size_t idx);
}

bool ade_odata_is_userdata_type(lua_State* L, int stackposition, size_t typeIdx, bool cleanup = true);

template<typename T>
bool ade_odata_is_userdata_type(lua_State* L, int stackposition, const T& obj_type) {
	return ade_odata_is_userdata_type(L, stackposition, obj_type.GetIdx());
}

void pushValue(lua_State* luaState, const double& value);

void pushValue(lua_State* luaState, const float& value);

void pushValue(lua_State* luaState, const int& value);

void pushValue(lua_State* luaState, const size_t& value);

void pushValue(lua_State* luaState, const std::string& value);

void pushValue(lua_State* luaState, const char* value);

void pushValue(lua_State* luaState, const bool& value);

void pushValue(lua_State* luaState, const lua_CFunction& value);

template<typename T>
void pushValue(lua_State* L, scripting::ade_odata_setter<T>&& value) {
	using namespace scripting;

	//WMC - char must be 1 byte, foo.
	static_assert(sizeof(char) == 1, "char must be 1 byte!");
	//WMC - step by step

	//Create new LUA object and get handle
	auto newod = (char*)lua_newuserdata(L, sizeof(T));
	//Create or get object metatable
	luaL_getmetatable(L, ::scripting::internal::getTableEntry(value.idx).Name);
	//Set the metatable for the object
	lua_setmetatable(L, -2);

	//Copy the actual object data to the Lua object
	new(newod) T(std::move(value.value));
}

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

template <typename T>
bool popValue(lua_State* L, scripting::ade_odata_getter<T>&& od, int stackposition = -1, bool remove = true)
{
	// Use the helper to reduce the amount of code here
	if (!internal::ade_odata_helper(L, stackposition, od.idx)) {
		return false;
	}
	auto lua_ptr = lua_touserdata(L, stackposition);

	// Copy the value over by using the standard copy constructor
	*od.value_ptr = *reinterpret_cast<T*>(lua_ptr);

	if (remove) {
		lua_remove(L, stackposition);
	}

	return true;
}

template <typename T>
bool popValue(lua_State* L, scripting::ade_odata_ptr_getter<T>&& od, int stackposition = -1, bool remove = true)
{
	// Use the helper to reduce the amount of code here
	if (!internal::ade_odata_helper(L, stackposition, od.idx)) {
		return false;
	}
	auto lua_ptr = lua_touserdata(L, stackposition);

	// Only write the pointer value to the output pointer
	*od.value_ptr = reinterpret_cast<T*>(lua_ptr);

	if (remove) {
		lua_remove(L, stackposition);
	}

	return true;
}

}
}


#endif
