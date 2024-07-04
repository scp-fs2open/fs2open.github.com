#include "LuaPromise.h"

#include "globalincs/pstypes.h"

#include "scripting/lua/LuaValue.h"

namespace scripting {
namespace api {

enum class State {
	Invalid,
	Pending,
	Resolved,
	Errored,
};

resolve_context::~resolve_context() = default;

class continuation_resolve_context : public resolve_context {
  public:
	continuation_resolve_context(bool wantErrors, LuaPromise::ContinuationFunction continuation)
		: _wantErrors(wantErrors), _continuation(std::move(continuation))
	{
	}
	~continuation_resolve_context() override = default;

	void setResolver(Resolver resolver) override { _resolver = std::move(resolver); }

	void resolve(bool error, const luacpp::LuaValueList& resolveVals)
	{
		Assertion(_resolver, "Promise resolved without a resolver! Probably called twice...");

		// If the error value is what we want then we need to call our continuation. Otherwise the value just passes
		// through this instance without the continuation being called
		if (error == _wantErrors) {
			// The next value is never an error since either we are in the "then" case or we are "catching" the
			// exception and so the return value is no longer an error.
			_resolver(false, _continuation(resolveVals));
		} else {
			_resolver(error, resolveVals);
		}

		// Not needed anymore, might as well clean up references
		_resolver     = nullptr;
		_continuation = nullptr;
	}

  private:
	bool _wantErrors = false;
	LuaPromise::ContinuationFunction _continuation;
	Resolver _resolver;
};

struct LuaPromise::internal_state : std::enable_shared_from_this<LuaPromise::internal_state> {
	State state = State::Invalid;

	luacpp::LuaValueList resolvedValue;

	SCP_vector<std::shared_ptr<continuation_resolve_context>> continuationContexts;

	void registerResolveCallback(const std::shared_ptr<resolve_context>& resolveCtx)
	{
		// Weak pointer here since if our pointer is cleaned up that means no one is interested in our promise anymore.
		auto self = shared_from_this();
		resolveCtx->setResolver([this, self](bool error, const luacpp::LuaValueList& resolveVals) mutable {
			Assertion(self != nullptr, "Resolver called multiple times!");

			resolveCb(error, resolveVals);

			// Remove the reference to ourself in case the resolver does not dispose of this function object
			self = nullptr;
		});
	}

	void resolveCb(bool error, const luacpp::LuaValueList& resolveVals)
	{
		resolvedValue = resolveVals;
		state         = error ? State::Errored : State::Resolved;

		// Call everyone who has registered on our coroutine so that those promises also resolve
		for (const auto& cont : continuationContexts) {
			cont->resolve(error, resolveVals);
		}
		// This will only be needed once so we can clear out the references now
		continuationContexts.clear();
	}
};

LuaPromise::LuaPromise() : m_state(std::make_shared<LuaPromise::internal_state>()) {}

LuaPromise::LuaPromise(const std::shared_ptr<resolve_context>& resolveContext) : LuaPromise()
{
	m_state->state = State::Pending;

	// This executes promises eagerly since registering the callback kicks off the async operation
	m_state->registerResolveCallback(resolveContext);
}

LuaPromise::LuaPromise(const LuaPromise&) = default;
LuaPromise& LuaPromise::operator=(const LuaPromise&) = default;

LuaPromise::LuaPromise(LuaPromise&&) noexcept = default;
LuaPromise& LuaPromise::operator=(LuaPromise&&) noexcept = default;

LuaPromise LuaPromise::then(LuaPromise::ContinuationFunction continuation)
{
	// NOT THREAD SAFE!
	if (m_state->state == State::Invalid) {
		return LuaPromise();
	}

	// The easy way
	if (m_state->state == State::Resolved) {
		return LuaPromise::resolved(continuation(m_state->resolvedValue));
	}

	// If the promise is already in an error state then the continuation doesn't matter
	if (m_state->state == State::Errored) {
		return LuaPromise::errored(m_state->resolvedValue);
	}

	return registerContinuation(false, std::move(continuation));
}

LuaPromise LuaPromise::catchError(LuaPromise::ContinuationFunction continuation)
{
	// NOT THREAD SAFE!
	if (m_state->state == State::Invalid) {
		return LuaPromise();
	}

	// If we want to catch errors then the value from this resolved promise just passes through
	if (m_state->state == State::Resolved) {
		return LuaPromise::resolved(m_state->resolvedValue);
	}

	// We actually want this value
	if (m_state->state == State::Errored) {
		return LuaPromise::errored(continuation(m_state->resolvedValue));
	}

	return registerContinuation(true, std::move(continuation));
}

bool LuaPromise::isValid() const { return m_state->state != State::Invalid; }
bool LuaPromise::isResolved() const { return m_state->state == State::Resolved; }
bool LuaPromise::isErrored() const { return m_state->state == State::Errored; }

const luacpp::LuaValueList& LuaPromise::resolveValue() const
{
	Assertion(isResolved(), "Tried to get value from unresolved promise!");

	return m_state->resolvedValue;
}
const luacpp::LuaValueList& LuaPromise::errorValue() const
{
	Assertion(isErrored(), "Tried to get error value from unresolved promise!");

	return m_state->resolvedValue;
}
LuaPromise LuaPromise::resolved(luacpp::LuaValueList resolveValue)
{
	LuaPromise p;
	p.m_state->state         = State::Resolved;
	p.m_state->resolvedValue = std::move(resolveValue);
	return p;
}

LuaPromise LuaPromise::errored(luacpp::LuaValueList resolveValue)
{
	LuaPromise p;
	p.m_state->state         = State::Errored;
	p.m_state->resolvedValue = std::move(resolveValue);
	return p;
}
LuaPromise LuaPromise::registerContinuation(bool catchErrors, LuaPromise::ContinuationFunction continuation)
{
	// This is the connection between us and the resulting promise. We need the reference to resolve the promise
	// and the returned promise uses it to know when to resolve
	auto continuationContext = std::make_shared<continuation_resolve_context>(catchErrors, std::move(continuation));
	m_state->continuationContexts.push_back(continuationContext);

	return LuaPromise(continuationContext);
}
LuaPromise::~LuaPromise() = default;

} // namespace api
} // namespace scripting
