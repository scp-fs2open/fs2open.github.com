#include "CombinedExecutionContext.h"

namespace executor {
CombinedExecutionContext::CombinedExecutionContext(SCP_vector<std::shared_ptr<IExecutionContext>> contexts)
	: m_contexts(std::move(contexts))
{
}
IExecutionContext::State CombinedExecutionContext::determineContextState() const
{
	bool suspended_context = false;
	bool invalid_context = false;
	for (const auto& context : m_contexts) {
		const auto state = context->determineContextState();

		if (state == IExecutionContext::State::Suspended) {
			suspended_context = true;
		} else if (state == IExecutionContext::State::Invalid) {
			invalid_context = true;
		}
	}

	// If there is an invalid context then the combined state is invalid
	// If there is not but there is a suspended one, then the state is suspended
	if (invalid_context) {
		return IExecutionContext::State::Invalid;
	} else if (suspended_context) {
		return IExecutionContext::State::Suspended;
	} else {
		return IExecutionContext::State::Valid;
	}
}
CombinedExecutionContext::~CombinedExecutionContext() = default;

} // namespace executor
