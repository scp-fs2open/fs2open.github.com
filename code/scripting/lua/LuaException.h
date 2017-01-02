#ifndef LUA_EXCEPTION_H
#define LUA_EXCEPTION_H
#pragma once

#include <string>
#include <stdexcept>

namespace luacpp {
/**
 * @brief An exception within lua
 */
class LuaException: public std::runtime_error {
 public:
	LuaException(const std::string& __arg = "Lua Error!") : runtime_error(__arg) {}
};
}

#endif