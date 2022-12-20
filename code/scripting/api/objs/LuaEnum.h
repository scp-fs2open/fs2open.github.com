#pragma once

#include "parse/sexp/LuaSEXP.h"
#include "parse/sexp.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct lua_enum_h {
	int lua_enum;
	lua_enum_h();
	explicit lua_enum_h(int idx);
	dynamic_sexp_enum_list* getEnum() const;
};

DECLARE_ADE_OBJ(l_LuaEnum, lua_enum_h);

} // namespace api
} // namespace scripting
