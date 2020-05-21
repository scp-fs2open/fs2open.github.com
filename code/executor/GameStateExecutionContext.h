#pragma once

#include "IExecutionContext.h"

#include "gamesequence/gamesequence.h"
#include "utils/event.h"

namespace executor {

/**
 * @brief An execution context that captures the game state
 *
 * This will only execute if the game state is the same as at the time when captureContext() was called. If a game state
 * is pushed onto the stack then this context will only suspend. Should the game state change completely then this
 * context will be invalid. Will also become invalid if the same game state is active but with a different instance id.
 */
class GameStateExecutionContext : public IExecutionContext, public std::enable_shared_from_this<GameStateExecutionContext> {
  public:
	GameStateExecutionContext(GS_STATE contextState, int stateInstance);
	~GameStateExecutionContext() override;

	State determineContextState() const override;

	static std::shared_ptr<IExecutionContext> captureContext();

  private:
	GS_STATE m_contextState = GS_STATE_INVALID;
	int m_stateInstanceId = -1;
};

} // namespace executor
