#include "GameStateExecutionContext.h"

#include "events/events.h"

namespace executor {

GameStateExecutionContext::GameStateExecutionContext(GS_STATE contextState, int stateInstance)
	: m_contextState(contextState), m_stateInstanceId(stateInstance)
{
}
GameStateExecutionContext::~GameStateExecutionContext() = default;

IExecutionContext::State GameStateExecutionContext::determineContextState() const
{
	const auto depth = gameseq_get_state_idx(m_contextState);

	if (depth < 0) {
		// Our game state is not present anymore
		return IExecutionContext::State::Invalid;
	}

	if (gameseq_get_state() == m_contextState) {
		// Check if this is still the same instance
		if (gameseq_get_state_instance_id() != m_stateInstanceId) {
			return IExecutionContext::State::Invalid;
		}

		// This is still the same state and it is active
		return State::Valid;
	}

	// We are not in the correct state but it is still in the stack so we might return to it at some point
	return State::Suspended;
}

std::shared_ptr<IExecutionContext> GameStateExecutionContext::captureContext()
{
	return std::make_shared<GameStateExecutionContext>(static_cast<GS_STATE>(gameseq_get_state()),
		gameseq_get_state_instance_id());
}

} // namespace executor
