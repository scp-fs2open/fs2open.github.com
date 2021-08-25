#include "LuaCoroutineRunner.h"

#include "scripting/ade_args.h"
#include "scripting/api/objs/promise.h"

namespace scripting {
namespace api {

namespace {

/**
 * @brief A run context which resumes a coroutine until it is finished
 *
 * This will manage the passed coroutine and resume it until it is completed. For every yielded promise it registers
 * itself as the continuation which then resumes the coroutine.
 */
class run_resolve_context : public resolve_context, public std::enable_shared_from_this<run_resolve_context> {
  public:
	run_resolve_context(luacpp::LuaThread coroutine,
		std::shared_ptr<executor::Executor> executor,
		std::shared_ptr<executor::IExecutionContext> executionContext)
		: _coroutine(std::move(coroutine)), _executor(std::move(executor)),
		  _executionContext(std::move(executionContext))
	{
	}

	void setResolver(Resolver resolver) override
	{
		m_resolver = std::move(resolver);

		// Kick off the coroutine once we know that someone cares about its result
		scheduleResume(luacpp::LuaValueList());
	}

  private:
	void postToExecutor(executor::Executor::Callback cb)
	{
		if (_executor.get() == executor::currentExecutor()) {
			// We are already in the right executor so we can invoke the callback directly
			const auto ret = cb();
			// It is possible that we get a reschedule here if the execution state is currently suspended
			if (ret == executor::Executor::CallbackResult::Reschedule) {
				_executor->post(std::move(cb));
			}
		} else {
			_executor->post(std::move(cb));
		}
	}

	void scheduleResume(const luacpp::LuaValueList& resumeParams)
	{
		if (!_executor) {
			// If we have no executor we just execute the resume directly
			resumeCoroutine(resumeParams);
			return;
		}

		auto self = shared_from_this();
		if (_executionContext) {
			// If we have a context, wrap our resumer in that so that we only execute in our context
			postToExecutor(executor::runInContext(_executionContext,
				[this, self, resumeParams](executor::IExecutionContext::State state) {
					if (state == executor::IExecutionContext::State::Invalid) {
						// State became invalid while waiting
						auto errorMessage = luacpp::LuaValue::createValue(_coroutine.getLuaState(),
							"Coroutine context became invalid.");
						m_resolver(true, {errorMessage});
						m_resolver = nullptr;
						return executor::Executor::CallbackResult::Done;
					}

					resumeCoroutine(resumeParams);
					// We only need to run this once
					return executor::Executor::CallbackResult::Done;
				}));
			return;
		}

		postToExecutor([this, self, resumeParams]() {
			resumeCoroutine(resumeParams);
			// We only need to run this once
			return executor::Executor::CallbackResult::Done;
		});
	}

	void resumeCoroutine(const luacpp::LuaValueList& resumeParams)
	{
		const auto result = _coroutine.resume(resumeParams);

		if (result.completed) {
			// Thread is finished! We can call our resolver
			m_resolver(false, result.returnVals);

			// Clean up reference since we do not need this anymore
			m_resolver = nullptr;
			return;
		}

		// The coroutine suspended so the return value must be a promise
		Assertion(result.returnVals.size() == 1,
			"Wrong number of yielded values. Should be 1 but is " SIZE_T_ARG,
			result.returnVals.size());

		auto promiseStackStart = lua_gettop(_coroutine.getLuaState());
		result.returnVals.front().pushValue(_coroutine.getLuaState());

		internal::Ade_get_args_skip      = promiseStackStart;
		internal::Ade_get_args_lfunction = true;
		LuaPromise* promise              = nullptr;
		if (!ade_get_args(_coroutine.getLuaState(), "o", l_Promise.GetPtr(&promise))) {
			LuaError(_coroutine.getLuaState(),
				"Failed to get promise after coroutine yielded. Make sure you only use async.await in async "
				"coroutines.");
			return;
		}

		lua_settop(_coroutine.getLuaState(), promiseStackStart);

		// Register ourself to be called when the promise resolves so that we can resume our coroutine
		auto self = shared_from_this();
		promise->then([this, self](const luacpp::LuaValueList& resolveVals) {
			// Since "self" keeps "this" alive it is safe to access that here
			scheduleResume(resolveVals);

			return luacpp::LuaValueList();
		});
		promise->catchError([this, self](const luacpp::LuaValueList& resolveVals) {
			// If the awaited coroutine causes an error we stop the coroutine and propagate that error to the
			// promise
			m_resolver(true, resolveVals);
			m_resolver = nullptr;

			return luacpp::LuaValueList();
		});
	}

	Resolver m_resolver;
	luacpp::LuaThread _coroutine;
	std::shared_ptr<executor::Executor> _executor;
	std::shared_ptr<executor::IExecutionContext> _executionContext;
};

} // namespace

LuaPromise runAsyncCoroutine(luacpp::LuaThread luaThread,
	std::shared_ptr<executor::Executor> executor,
	std::shared_ptr<executor::IExecutionContext> executionContext)
{
	return LuaPromise(
		std::make_shared<run_resolve_context>(std::move(luaThread), std::move(executor), std::move(executionContext)));
}

} // namespace api
} // namespace scripting
