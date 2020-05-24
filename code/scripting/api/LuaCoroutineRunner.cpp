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
	run_resolve_context(luacpp::LuaThread coroutine) : _coroutine(std::move(coroutine)) {}

	~run_resolve_context() override = default;
	void setResolver(Resolver resolver) override
	{
		m_resolver = std::move(resolver);

		// Kick off the coroutine once we know that someone cares about its result
		resumeCoroutine(luacpp::LuaValueList());
	}

  private:
	void resumeCoroutine(const luacpp::LuaValueList& resumeParams)
	{
		const auto result = _coroutine.resume(resumeParams);

		if (result.completed) {
			// Thread is finished! We can call our resolver
			m_resolver(false, result.returnVals);
			return;
		}

		// The coroutine suspended so the return value must be a promise
		Assertion(result.returnVals.size() == 1,
			"Wrong number of yielded values. Should be 1 but is " SIZE_T_ARG,
			result.returnVals.size());

		auto promiseStackStart = lua_gettop(_coroutine.getLuaState());
		result.returnVals.front().pushValue(_coroutine.getLuaState());

		internal::Ade_get_args_skip = promiseStackStart;
		LuaPromise* promise         = nullptr;
		if (!ade_get_args(_coroutine.getLuaState(), "o", l_Promise.GetPtr(&promise))) {
			internal::Ade_get_args_skip = 0;
			LuaError(_coroutine.getLuaState(),
				"Failed to get promise after coroutine yielded. Make sure you only use async.await in async "
				"coroutines.");
			return;
		}
		internal::Ade_get_args_skip = 0;

		// Register ourself to be called when the promise resolves so that we can resume our coroutine
		auto self = shared_from_this();
		promise
			->then([this, self](const luacpp::LuaValueList& resolveVals) {
				// Since "self" keeps "this" alive it is safe to access that here
				resumeCoroutine(resolveVals);

				return luacpp::LuaValueList();
			})
			.catchError([this, self](const luacpp::LuaValueList& resolveVals) {
				// If the awaited coroutine causes an error we stop the coroutine and propagate that error to the
				// promise
				m_resolver(true, resolveVals);

				return luacpp::LuaValueList();
			});
	}

	Resolver m_resolver;
	luacpp::LuaThread _coroutine;
};

} // namespace

LuaPromise runAsyncCoroutine(luacpp::LuaThread luaThread)
{
	return LuaPromise(std::make_shared<run_resolve_context>(std::move(luaThread)));
}

} // namespace api
} // namespace scripting
