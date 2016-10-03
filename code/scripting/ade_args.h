//
//

#ifndef FS2_OPEN_ADE_ARGS_H
#define FS2_OPEN_ADE_ARGS_H

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

namespace scripting {
int ade_get_args(lua_State *L, const char *fmt, ...);
int ade_set_args(lua_State *L, const char *fmt, ...);

//*************************Lua hacks*************************
//WMC - Hack to allow for quick&easy return value parsing
extern int Ade_get_args_skip;
//WMC - Tell ade_get_args it is parsing scripting functions,
//which have no upvalues
extern bool Ade_get_args_lfunction;
}

#endif //FS2_OPEN_ADE_ARGS_H
