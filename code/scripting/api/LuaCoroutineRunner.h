#pragma once

#include "LuaPromise.h"

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
 * @return A LuaPromise that will resolve with the value returned when the coroutine completes
 */
LuaPromise runAsyncCoroutine(luacpp::LuaThread luaThread);

} // namespace api
} // namespace scripting
