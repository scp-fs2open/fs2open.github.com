#pragma once

#include "scripting/ade_api.h"
#include "parse/sexp/LuaAISEXP.h"

namespace scripting {
namespace api {

struct lua_ai_sexp_h {
	sexp::LuaAISEXP* sexp_handle;

	lua_ai_sexp_h(sexp::LuaAISEXP* sexp_handle);
	lua_ai_sexp_h();

	bool isValid();
};

DECLARE_ADE_OBJ(l_LuaAISEXP, lua_ai_sexp_h);

}
}

