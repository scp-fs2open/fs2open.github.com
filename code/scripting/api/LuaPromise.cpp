#include "LuaPromise.h"

#include "globalincs/pstypes.h"

#include "scripting/lua/LuaValue.h"

namespace scripting {
namespace api {

enum class State {
	Invalid,
	Pending,
	Resolved,
};

resolve_context::~resolve_context() = default;

class continuation_resolve_context : public resolve_context {
  public:
	continuation_resolve_context(LuaPromise::ContinuationFunction continuation) : _continuation(std::move(continuation))
	{
	}
	~continuation_resolve_context() override = default;

	void setResolver(Resolver resolver) override { _resolver = std::move(resolver); }

	void resolve(const luacpp::LuaValueList& resolveVals) { _resolver(_continuation(resolveVals)); }

  private:
	LuaPromise::ContinuationFunction _continuation;
	Resolver _resolver;
};

struct LuaPromise::internal_state : std::enable_shared_from_this<LuaPromise::internal_state> {
	State state = State::Invalid;

	luacpp::LuaValueList resolvedValue;

	std::shared_ptr<resolve_context> resolveCtx;

	SCP_vector<std::shared_ptr<continuation_resolve_context>> continuationContexts;

	void registerResolveCallback()
	{
		// Need a weak pointer here to avoid circular dependency. Lua will keep this object alive as long as needed
		std::weak_ptr<LuaPromise::internal_state> weak_self = shared_from_this();
		resolveCtx->setResolver([this, weak_self](const luacpp::LuaValueList& resolveVals) {
			if (auto _ = weak_self.lock()) {
				// Now we know that "this" is still valid
				resolveCb(resolveVals);
			}
		});
	}

	void resolveCb(const luacpp::LuaValueList& resolveVals)
	{
		resolvedValue = resolveVals;
		state         = State::Resolved;

		// Call everyone who has registered on our coroutine so that those promises also resolve
		for (const auto& cont : continuationContexts) {
			cont->resolve(resolveVals);
		}
	}
};

LuaPromise::LuaPromise() : m_state(std::make_shared<LuaPromise::internal_state>()) {}

LuaPromise::LuaPromise(luacpp::LuaValueList resolveValue) : LuaPromise()
{
	m_state->state         = State::Resolved;
	m_state->resolvedValue = std::move(resolveValue);
}

LuaPromise::LuaPromise(std::shared_ptr<resolve_context> resolveContext) : LuaPromise()
{
	m_state->state      = State::Pending;
	m_state->resolveCtx = std::move(resolveContext);

	// This executes promises eagerly since registering the callback kicks of the async operation
	m_state->registerResolveCallback();
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
		return LuaPromise(continuation(m_state->resolvedValue));
	}

	// This is the connection between us and the resulting promise. We need the reference to resolve the promise
	// and the returned promise uses it to know when to resolve
	auto continuationContext = std::make_shared<continuation_resolve_context>(std::move(continuation));
	m_state->continuationContexts.push_back(continuationContext);

	return LuaPromise(continuationContext);
}
bool LuaPromise::isValid() const { return m_state->state != State::Invalid; }
bool LuaPromise::isResolved() const { return m_state->state == State::Resolved; }

const luacpp::LuaValueList& LuaPromise::resolveValue() const
{
	Assertion(isResolved(), "Tried to get value from unresolved promise!");

	return m_state->resolvedValue;
}

} // namespace api
} // namespace scripting
