#ifndef _LUA_H
#define _LUA_H
#ifdef USE_LUA

extern "C" {
	#include "lauxlib.h"
	#include "lualib.h"
}

extern const script_lua_lib_list Lua_libraries[];

#endif //USE_LUA
#endif //_LUA_H