#pragma once

#include "object/object.h"
#include "scripting/lua/LuaFunction.h"

namespace scripting {
namespace api {
namespace util {

void convert_arg(lua_State* L, luacpp::LuaValueList& out, object* objp);
}

template <typename Ret, typename... Args>
class LuaEventCallback {
	luacpp::LuaFunction _func;

	void convert_args(lua_State* /*L*/, luacpp::LuaValueList& /*out*/) {}

	template <typename T, typename... A>
	void convert_args(lua_State* L, luacpp::LuaValueList& out, T t, A... args)
	{
		util::convert_arg(L, out, t);

		convert_args(L, out, args...);
	}

  public:
	explicit LuaEventCallback(const luacpp::LuaFunction& func) : _func(func) {}

	void operator()(Args... args)
	{
		using namespace luacpp;

		LuaValueList lua_args;
		convert_args(_func.getLuaState(), lua_args, args...);

		_func(lua_args);
	}
};

template <typename Ret, typename... Args>
std::function<Ret(Args...)> make_lua_callback(const luacpp::LuaFunction& func)
{
	return std::function<Ret(Args...)>(LuaEventCallback<Ret, Args...>(func));
}

} // namespace api
} // namespace scripting
