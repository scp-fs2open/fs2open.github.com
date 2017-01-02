
#ifndef LUA_HEADERS_H
#define LUA_HEADERS_H
#pragma once

/**
 * @file LuaHeaders.hpp
 * 
 * Contains the lua headers wrapped in an @c exteren @c "C" block.
 */

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#endif // LUA_HEADERS_H
