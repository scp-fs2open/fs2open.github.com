
#include "async.h"

#include "scripting/api/LuaCoroutineRunner.h"
#include "scripting/api/objs/promise.h"
#include "scripting/lua/LuaThread.h"

namespace scripting {
namespace api {

//**********LIBRARY: Async
ADE_LIB(l_Async, "Async", "async", "Support library for asynchronous operations");

/**
 * @brief A simple run context that allows a promise to be resolved by calling a lua function
 *
 * This is very similar to JavaScript Promises where a user defined function is passed a special "resolve" function that
 * should be called when the coroutine is finished.
 */
class lua_func_resolve_context : public resolve_context {
  public:
	lua_func_resolve_context(luacpp::LuaFunction callback) : _callback(std::move(callback)) {}

	void setResolver(Resolver resolver) override
	{
		const auto resolveFunc = luacpp::LuaFunction::createFromStdFunction(_callback.getLuaState(),
			[resolver](lua_State*, const luacpp::LuaValueList& resolveVals) {
				resolver(resolveVals);
				return luacpp::LuaValueList();
			});

		_callback({resolveFunc});
	}

  private:
	luacpp::LuaFunction _callback;
};

ADE_FUNC(promise,
	l_Async,
	"function(function resolve(...))",
	"Creates a promise that resolves when the resolve function of the callback is called. The function will be called "
	"on it's own.",
	"promise",
	"The promise or nil on error")
{
	luacpp::LuaFunction callback;
	if (!ade_get_args(L, "u", &callback)) {
		return ADE_RETURN_NIL;
	}

	if (!callback.isValid()) {
		LuaError(L, "Invalid function supplied!");
		return ADE_RETURN_NIL;
	}

	std::unique_ptr<resolve_context> resolveCtx(new lua_func_resolve_context(std::move(callback)));

	return ade_set_args(L, "o", l_Promise.Set(LuaPromise(std::move(resolveCtx))));
}

ADE_FUNC(resolved,
	l_Async,
	"...",
	"Creates a resolved promise with the values passed to this function.",
	"promise",
	"Resolved promise")
{
	auto nargs = lua_gettop(L);
	luacpp::LuaValueList values;
	values.reserve(nargs);

	for (int i = 1; i <= nargs; ++i) {
		luacpp::LuaValue val;
		val.setReference(luacpp::UniqueLuaReference::create(L, i));
		values.push_back(val);
	}

	return ade_set_args(L, "o", l_Promise.Set(LuaPromise(values)));
}

ADE_FUNC(run,
	l_Async,
	"function body()",
	"Runs an asynchronous function",
	"promise",
	"A promise that resolves with the return value of the body when it reaches a return statement")
{
	luacpp::LuaFunction body;
	if (!ade_get_args(L, "u", &body)) {
		return ADE_RETURN_NIL;
	}

	auto coroutine = luacpp::LuaThread::create(L, body);
	coroutine.setErrorCallback([](lua_State*, lua_State* thread) {
		LuaError(thread);
		return true;
	});

	return ade_set_args(L, "o", l_Promise.Set(runAsyncCoroutine(std::move(coroutine))));
}

ADE_FUNC(await,
	l_Async,
	"promise",
	"Suspends an asynchronous coroutine until the passed promise resolves.",
	"unknown",
	"The resolve value of the promise")
{
	LuaPromise* promise = nullptr;
	if (!ade_get_args(L, "o", l_Promise.GetPtr(&promise))) {
		return ADE_RETURN_NIL;
	}

	if (promise == nullptr || !promise->isValid()) {
		LuaError(L, "Invalid promise detected. This should not happen. Please contact a developer.");
		return ADE_RETURN_NIL;
	}

	if (promise->isResolved()) {
		// No need to suspend if the promise is already resolved. Just take the resolve values from the promise and
		// return them
		const auto& retVals = promise->resolveValue();
		for (const auto& retVal : retVals) {
			retVal.pushValue(L); // Push onto this stack to ensure it is actually returned from this function
		}
		return static_cast<int>(retVals.size());
	}

	// Return the promise via the yield so that the resumer can register themself on the promise to resume when that
	// resolves
	return lua_yield(L, 1);
}

} // namespace api
} // namespace scripting
