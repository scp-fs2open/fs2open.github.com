
#include "async.h"

#include "executor/CombinedExecutionContext.h"
#include "executor/GameStateExecutionContext.h"
#include "executor/global_executors.h"
#include "scripting/api/LuaCoroutineRunner.h"
#include "scripting/api/LuaExecutionContext.h"
#include "scripting/api/objs/execution_context.h"
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
	"function(function(any resolveVal) => void resolve, function(any errorVal) => void reject) => void body",
	"Creates a promise that resolves when the resolve function of the callback is called or errors if the reject "
	"function is called. The function will be called "
	"on its own.",
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
	"any... resolveValues",
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
	"any... errorValues",
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
	"function() => any body, [executor executeOn = nil, boolean|execution_context captureContext = true /* Captures "
	"game state context by default */]",
	"Runs an asynchronous function. Inside this function you can use async.await to suspend the function until a "
	"promise resolves. Also allows to specify an executor on which the code of the coroutine should be executed. If "
	"captureContext is true then the game context (the game state) at the time of the call is captured and the "
	"coroutine is only run if that state is still active.",
	"promise",
	"A promise that resolves with the return value of the body when it reaches a return statement")
{
	luacpp::LuaFunction body;
	executor_h executor;
	std::shared_ptr<executor::IExecutionContext> context;

	if (lua_type(L, 3) == LUA_TUSERDATA) {
		execution_context_h* ctx = nullptr;
		if (!ade_get_args(L, "u|oo", &body, l_Executor.Get(&executor), l_ExecutionContext.GetPtr(&ctx))) {
			return ADE_RETURN_NIL;
		}
		if (ctx != nullptr) {
			context = ctx->getExecutionContext();
		}
	} else {
		bool captureContext = true;
		if (!ade_get_args(L, "u|ob", &body, l_Executor.Get(&executor), &captureContext)) {
			return ADE_RETURN_NIL;
		}
		if (executor.isValid() && captureContext) {
			context = executor::GameStateExecutionContext::captureContext();
		}
	}

	auto coroutine = luacpp::LuaThread::create(L, body);
	coroutine.setErrorCallback([](lua_State*, lua_State* thread) {
		LuaError(thread);
		return true;
	});

	return ade_set_args(L,
		"o",
		l_Promise.Set(runAsyncCoroutine(std::move(coroutine), executor.getExecutor(), std::move(context))));
}

ADE_FUNC(await,
	l_Async,
	"promise",
	"Suspends an asynchronous coroutine until the passed promise resolves.",
	"any",
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

ADE_FUNC(error,
	l_Async,
	"any... errorValues",
	"Causes the currently running coroutine to fail with an error with the specified values.",
	nullptr,
	"Does not return")
{
	// error cannot be used on the main thread since there is nothing that will wait for the promise
	if (lua_pushthread(L)) {
		// We are the main thread
		lua_pop(L, 1);

		LuaError(L, "Tried to cause a coroutine error on the main thread! That is not supported.");
		return ADE_RETURN_NIL;
	}
	lua_pop(L, 1);

	auto nargs = lua_gettop(L);
	luacpp::LuaValueList values;
	values.reserve(nargs);

	for (int i = 1; i <= nargs; ++i) {
		luacpp::LuaValue val;
		val.setReference(luacpp::UniqueLuaReference::create(L, i));
		values.push_back(val);
	}

	// Push an errored promise on the stack and then yield the coroutine. The awaiter will get this promise and stop
	// execution.
	ade_set_args(L, "o", l_Promise.Set(LuaPromise::errored(values)));
	return lua_yield(L, 1);
}

//**********SUBLIBRARY: async/context
ADE_LIB_DERIV(l_Async_Context, "context", nullptr, "Support library for creating execution contexts.", l_Async);

ADE_FUNC(captureGameState,
	l_Async_Context,
	nullptr,
	"Captures the current game state as an execution context",
	"execution_context",
	"The execution context or invalid handle on error")
{
	return ade_set_args(L,
		"o",
		l_ExecutionContext.Set(execution_context_h(executor::GameStateExecutionContext::captureContext())));
}

ADE_FUNC(createLuaState,
	l_Async_Context,
	"function() => enumeration",
	"Creates an execution state by storing the passed function and calling that when the state is required.",
	"execution_context",
	"The execution context or invalid handle on error")
{
	luacpp::LuaFunction body;
	if (!ade_get_args(L, "u", &body)) {
		return ade_set_args(L, "o", l_ExecutionContext.Set(execution_context_h(nullptr)));
	}

	return ade_set_args(L,
		"o",
		l_ExecutionContext.Set(execution_context_h(std::make_shared<LuaExecutionContext>(std::move(body)))));
}

ADE_FUNC(combineContexts,
	l_Async_Context,
	"execution_context... contexts",
	"Combines several execution contexts into a single one by only return a valid state if all contexts are valid.",
	"execution_context",
	"The execution context or invalid handle on error")
{
	SCP_vector<std::shared_ptr<executor::IExecutionContext>> contexts;
	for (int i = 0; i < lua_gettop(L); ++i) {
		execution_context_h* ctx;
		internal::Ade_get_args_skip = i;
		if (!ade_get_args(L, "o", l_ExecutionContext.GetPtr(&ctx))) {
			return ade_set_args(L, "o", l_ExecutionContext.Set(execution_context_h(nullptr)));
		}
		contexts.emplace_back(ctx->getExecutionContext());
	}

	return ade_set_args(L,
		"o",
		l_ExecutionContext.Set(
			execution_context_h(std::make_shared<executor::CombinedExecutionContext>(std::move(contexts)))));
}

} // namespace api
} // namespace scripting
