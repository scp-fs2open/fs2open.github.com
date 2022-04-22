//
//

#include "luaaisexp.h"

namespace scripting {
namespace api {

lua_ai_sexp_h::lua_ai_sexp_h(sexp::LuaAISEXP* handle) : sexp_handle(handle) {
}
lua_ai_sexp_h::lua_ai_sexp_h() : sexp_handle(nullptr) {
}
bool lua_ai_sexp_h::isValid() {
	return sexp_handle != nullptr;
}

ADE_OBJ(l_LuaAISEXP, lua_ai_sexp_h, "LuaAISEXP", "Lua AI SEXP handle");

ADE_VIRTVAR(ActionEnter,
	l_LuaAISEXP,
	"function(ai_helper helper, oswpt | nil arg) => boolean action",
	"The action of this AI SEXP to be executed once when the AI recieves this order. Return true if the AI goal is complete.",
	"function(ai_helper helper, oswpt | nil arg) => boolean action",
	"The action function or nil on error")
{
	lua_ai_sexp_h lua_sexp;
	luacpp::LuaFunction action_arg;
	if (!ade_get_args(L, "o|u", l_LuaAISEXP.Get(&lua_sexp), &action_arg)) {
		return ADE_RETURN_NIL;
	}

	if (!lua_sexp.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		Assertion(action_arg.isValid(), "Action function reference must be valid!");

		lua_sexp.sexp_handle->setActionEnter(action_arg);
	}

	auto action = lua_sexp.sexp_handle->getActionEnter();
	return ade_set_args(L, "u", &action);
}

ADE_VIRTVAR(ActionFrame,
	l_LuaAISEXP,
	"function(ai_helper helper, oswpt | nil arg) => boolean action",
	"The action of this AI SEXP to be executed each frame while active. Return true if the AI goal is complete.",
	"function(ai_helper helper, oswpt | nil arg) => boolean action",
	"The action function or nil on error")
{
	lua_ai_sexp_h lua_sexp;
	luacpp::LuaFunction action_arg;
	if (!ade_get_args(L, "o|u", l_LuaAISEXP.Get(&lua_sexp), &action_arg)) {
		return ADE_RETURN_NIL;
	}

	if (!lua_sexp.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		Assertion(action_arg.isValid(), "Action function reference must be valid!");

		lua_sexp.sexp_handle->setActionFrame(action_arg);
	}

	auto action = lua_sexp.sexp_handle->getActionFrame();
	return ade_set_args(L, "u", &action);
}

}
}

