#ifndef _LUA_EXTENSIONS_H
#define _LUA_EXTENSIONS_H

#include "lua.h"


// These are libraries that we want to add to the core Lua code.  We can't integrate them into the
// base Lua library because the library may be loaded differently on different platforms.

#define LUA_BITLIBNAME "luabit"
LUALIB_API int luaopen_bit(lua_State* L);


// open all extension libraries
LUALIB_API void luaL_openlibs_ext(lua_State* L);

#endif
