#include "IExecutionContext.h"

namespace executor {

Executor::Callback runInContext(const std::shared_ptr<IExecutionContext>& context,
	const IExecutionContext::Callback& func)
{
	return [context, func]() {
		const auto ctxState = context->determineContextState();

		switch (ctxState) {
		case IExecutionContext::State::Valid:
			// We are in a valid state so execute our wrapped function
			return func(IExecutionContext::State::Valid);
		case IExecutionContext::State::Suspended:
			// The state is not valid but my return so keep this function alive
			return Executor::CallbackResult::Reschedule;
		case IExecutionContext::State::Invalid:
			// Call the function once more to let it know that the state became invalid
			func(IExecutionContext::State::Invalid);
			// This state will not become valid again so we do not need to be rescheduled
			return Executor::CallbackResult::Done;
		default:
			UNREACHABLE("Unhandled context state %d", static_cast<int>(ctxState));
			return Executor::CallbackResult::Done;
		}
	};
}

} // namespace executor
