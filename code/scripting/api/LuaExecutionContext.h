#pragma once

#include "executor/IExecutionContext.h"
#include "gamesequence/gamesequence.h"
#include "scripting/lua/LuaFunction.h"
#include "utils/event.h"

namespace scripting {
namespace api {

class LuaExecutionContext : public executor::IExecutionContext {
  public:
	LuaExecutionContext(luacpp::LuaFunction contextFunction);
	~LuaExecutionContext() override;

	State determineContextState() const override;

  private:
	luacpp::LuaFunction m_contextFunction;
};

} // namespace api
} // namespace scripting
