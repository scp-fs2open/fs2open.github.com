#pragma once

#include "LuaPromise.h"

#include "executor/Executor.h"
#include "executor/IExecutionContext.h"
#include "scripting/lua/LuaThread.h"

namespace scripting {
namespace api {

/**
 * @brief Runs an asynchronous Lua thread and runs it to completion
 *
 * This takes the specified lua thread and resumes it until it is completed. Every yielded value must be a LuaPromise
 * which suspends the coroutine until that promise resolves.
 *
 * @param luaThread The lua coroutine to run
 * @param executor The executor on which the code of the coroutine should be executed. May be nullptr.
 * @param executionContext The context to use for execution. Code on the coroutine will only be executed when the
 * context is valid.
 * @return A LuaPromise that will resolve with the value returned when the coroutine completes
 */
LuaPromise runAsyncCoroutine(luacpp::LuaThread luaThread,
	std::shared_ptr<executor::Executor> executor,
	std::shared_ptr<executor::IExecutionContext> executionContext);

} // namespace api
} // namespace scripting
