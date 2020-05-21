
#include "async.h"

#include "executor/GameStateExecutionContext.h"
#include "executor/global_executors.h"
#include "scripting/api/LuaCoroutineRunner.h"
#include "scripting/api/objs/executor.h"
#include "scripting/api/objs/promise.h"
#include "scripting/lua/LuaThread.h"

namespace scripting {
namespace api {

//**********LIBRARY: Async
ADE_LIB(l_Async, "Async", "async", "Support library for asynchronous operations");

int executorGetter(lua_State* L, const std::shared_ptr<executor::Executor>& executor)
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only!");
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_Executor.Set(executor_h(executor)));
}

ADE_VIRTVAR(OnFrameExecutor,
	l_Async,
	nullptr,
	"An executor that executes operations at the end of rendering a frame.",
	"executor",
	"The executor handle")
{
	return executorGetter(L, executor::OnFrameExecutor);
}

ADE_VIRTVAR(OnSimulationExecutor,
	l_Async,
	nullptr,
	"An executor that executes operations after all object simulation has been done but before rendering starts. This "
	"is the place to do physics manipulations.",
	"executor",
	"The executor handle")
{
	return executorGetter(L, executor::OnSimulationExecutor);
}

/**
 * @brief A simple run context that allows a promise to be resolved by calling a lua function
 *
 * This is very similar to JavaScript Promises where a user defined function is passed a special "resolve" function that
 * should be called when the coroutine is finished.
 */
class lua_func_resolve_context : public resolve_context {
  public:
	lua_func_resolve_context(lua_State* L, luacpp::LuaFunction callback) : _luaState(L), _callback(std::move(callback))
	{
	}

	void setResolver(Resolver resolver) override
	{
		struct shared_state {
			Resolver resolver;
			bool alreadyCalled = false;
		};

		auto state      = std::make_shared<shared_state>();
		state->resolver = std::move(resolver);

		const auto resolveFunc = luacpp::LuaFunction::createFromStdFunction(_callback.getLuaState(),
			[state](lua_State* L, const luacpp::LuaValueList& resolveVals) {
				if (state->alreadyCalled) {
					LuaError(L, "Promise has already been resolved or rejected before!");
					return luacpp::LuaValueList();
				}

				state->resolver(false, resolveVals);
				state->alreadyCalled = true;

				state->resolver = nullptr;
				return luacpp::LuaValueList();
			});

		const auto rejectFunc = luacpp::LuaFunction::createFromStdFunction(_callback.getLuaState(),
			[state](lua_State* L, const luacpp::LuaValueList& resolveVals) {
				if (state->alreadyCalled) {
					LuaError(L, "Promise has already been resolved or rejected before!");
					return luacpp::LuaValueList();
				}

				state->resolver(true, resolveVals);
				state->alreadyCalled = true;

				state->resolver = nullptr;
				return luacpp::LuaValueList();
			});

		_callback(_luaState, {resolveFunc, rejectFunc});
	}

  private:
	lua_State* _luaState = nullptr;
	luacpp::LuaFunction _callback;
};

ADE_FUNC(promise,
	l_Async,
	"function(function resolve(...), function reject(...))",
	"Creates a promise that resolves when the resolve function of the callback is called or errors if the reject "
	"function is called. The function will be called "
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

	std::unique_ptr<resolve_context> resolveCtx(new lua_func_resolve_context(L, std::move(callback)));

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

	return ade_set_args(L, "o", l_Promise.Set(LuaPromise::resolved(values)));
}

ADE_FUNC(errored,
	l_Async,
	"...",
	"Creates an errored promise with the values passed to this function.",
	"promise",
	"Errored promise")
{
	auto nargs = lua_gettop(L);
	luacpp::LuaValueList values;
	values.reserve(nargs);

	for (int i = 1; i <= nargs; ++i) {
		luacpp::LuaValue val;
		val.setReference(luacpp::UniqueLuaReference::create(L, i));
		values.push_back(val);
	}

	return ade_set_args(L, "o", l_Promise.Set(LuaPromise::errored(values)));
}

ADE_FUNC(run,
	l_Async,
	"function body()",
	"Runs an asynchronous function. Inside this function you can use async.await to suspend the function until a "
	"promise resolves.",
	"promise",
	"A promise that resolves with the return value of the body when it reaches a return statement")
{
	luacpp::LuaFunction body;
	executor_h executor;
	bool captureContext = true;
	if (!ade_get_args(L, "u|ob", &body, l_Executor.Get(&executor), &captureContext)) {
		return ADE_RETURN_NIL;
	}

	auto coroutine = luacpp::LuaThread::create(L, body);
	coroutine.setErrorCallback([](lua_State*, lua_State* thread) {
		LuaError(thread);
		return true;
	});

	std::shared_ptr<executor::IExecutionContext> exeContext;
	if (executor.isValid() && captureContext) {
		exeContext = executor::GameStateExecutionContext::captureContext();
	}

	return ade_set_args(L,
		"o",
		l_Promise.Set(runAsyncCoroutine(std::move(coroutine), executor.getExecutor(), std::move(exeContext))));
}

ADE_FUNC(await,
	l_Async,
	"promise",
	"Suspends an asynchronous coroutine until the passed promise resolves.",
	"unknown",
	"The resolve value of the promise")
{
	// await cannot be used on the main thread since there is nothing that will wait for the promise
	if (lua_pushthread(L)) {
		// We are the main thread
		lua_pop(L, 1);

		LuaError(L, "Tried to await something on the main thread! That is not supported.");
		return ADE_RETURN_NIL;
	}
	lua_pop(L, 1);

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

ADE_FUNC(yield,
	l_Async,
	nullptr,
	"Returns a promise that will resolve on the next execution of the current executor. Effectively allows to "
	"asynchronously wait until the next frame.",
	"promise",
	"The promise")
{
	if (executor::currentExecutor() == nullptr) {
		LuaError(L,
			"There is no running executor at the moment. This function needs to be called from a executor callback.");
		return ADE_RETURN_NIL;
	}

	class yield_resolve_context : public resolve_context {
	  public:
		explicit yield_resolve_context(executor::Executor* executor) : m_exec(executor) {}
		void setResolver(Resolver resolver) override
		{
			m_exec->post([resolver]() {
				resolver(false, luacpp::LuaValueList());
				return executor::Executor::CallbackResult::Done;
			});
		}

	  private:
		executor::Executor* m_exec = nullptr;
	};
	return ade_set_args(L,
		"o",
		l_Promise.Set(LuaPromise(std::make_shared<yield_resolve_context>(executor::currentExecutor()))));
}

} // namespace api
} // namespace scripting
