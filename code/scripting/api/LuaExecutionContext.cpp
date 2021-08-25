#include "LuaExecutionContext.h"

#include "scripting/ade_args.h"
#include "scripting/api/objs/enums.h"

namespace scripting {
namespace api {

LuaExecutionContext::LuaExecutionContext(luacpp::LuaFunction contextFunction)
	: m_contextFunction(std::move(contextFunction))
{
}
LuaExecutionContext::~LuaExecutionContext() = default;

executor::IExecutionContext::State LuaExecutionContext::determineContextState() const
{
	auto L = m_contextFunction.getLuaState();
	const auto returnVal = m_contextFunction.call(L);
	if (returnVal.size() != 1) {
		LuaError(L, "Need exactly one return value for Lua execution context function!");
		return executor::IExecutionContext::State::Invalid;
	}

	const auto currentTop = lua_gettop(L);
	returnVal[0].pushValue(L);

	enum_h enumValue;
	internal::Ade_get_args_lfunction = true;
	internal::Ade_get_args_skip = currentTop;
	if (!ade_get_args(L, "o", l_Enum.Get(&enumValue))) {
		lua_settop(L, currentTop);
		return executor::IExecutionContext::State::Invalid;
	}
	lua_settop(L, currentTop);

	switch (enumValue.index) {
	case LE_CONTEXT_INVALID:
		return executor::IExecutionContext::State::Invalid;
	case LE_CONTEXT_SUSPENDED:
		return executor::IExecutionContext::State::Suspended;
	case LE_CONTEXT_VALID:
		return executor::IExecutionContext::State::Valid;
	default:
		LuaError(L, "Got invalid enum value %s from Lua context function.", enumValue.getName().c_str());
		return executor::IExecutionContext::State::Invalid;
	}
}

} // namespace api
} // namespace scripting
