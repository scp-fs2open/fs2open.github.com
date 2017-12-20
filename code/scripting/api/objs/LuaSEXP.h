#pragma once

#include "scripting/ade_api.h"
#include "parse/sexp/LuaSEXP.h"

namespace scripting {
namespace api {

struct lua_sexp_h {
	sexp::LuaSEXP* sexp_handle;

	lua_sexp_h(sexp::LuaSEXP* sexp_handle);
	lua_sexp_h();

	bool isValid();
};

DECLARE_ADE_OBJ(l_LuaSEXP, lua_sexp_h);

}
}

