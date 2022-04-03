//
//

#include "LuaSEXP.h"

namespace scripting {
namespace api {

lua_sexp_h::lua_sexp_h(sexp::LuaSEXP* handle) : sexp_handle(handle) {
}
lua_sexp_h::lua_sexp_h() : sexp_handle(nullptr) {
}
bool lua_sexp_h::isValid() {
	return sexp_handle != nullptr;
}

ADE_OBJ(l_LuaSEXP, lua_sexp_h, "LuaSEXP", "Lua SEXP handle");

ADE_VIRTVAR(Action,
	l_LuaSEXP,
	"function(any... args) => void action",
	"The action of this SEXP",
	"function(any... args) => void action",
	"The action function or nil on error")
{
	lua_sexp_h lua_sexp;
	luacpp::LuaFunction action_arg;
	if (!ade_get_args(L, "o|u", l_LuaSEXP.Get(&lua_sexp), &action_arg)) {
		return ADE_RETURN_NIL;
	}

	if (!lua_sexp.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		Assertion(action_arg.isValid(), "Action function reference must be valid!");

		lua_sexp.sexp_handle->setAction(action_arg);
	}

	auto action = lua_sexp.sexp_handle->getAction();
	return ade_set_args(L, "u", &action);
}

}
}

