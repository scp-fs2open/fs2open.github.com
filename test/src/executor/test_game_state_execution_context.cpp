#include "executor/GameStateExecutionContext.h"
#include "gamesequence/gamesequence.h"

#include <gtest/gtest.h>

// The unit tests, like FRED and qtFRED, never call gameseq_init(), so there is no game state stack here.
TEST(GameStateExecutionContextTest, valid_without_state_stack)
{
	ASSERT_FALSE(GameState_Stack_Valid());

	const auto context = executor::GameStateExecutionContext::captureContext();

	EXPECT_EQ(executor::IExecutionContext::State::Valid, context->determineContextState());
}

TEST(GameStateExecutionContextTest, real_state_invalid_without_state_stack)
{
	ASSERT_FALSE(GameState_Stack_Valid());

	const executor::GameStateExecutionContext context(GS_STATE_MAIN_MENU, 1);

	EXPECT_EQ(executor::IExecutionContext::State::Invalid, context.determineContextState());
}

TEST(GameStateExecutionContextTest, instance_id_without_state_stack)
{
	ASSERT_FALSE(GameState_Stack_Valid());

	EXPECT_EQ(0, gameseq_get_state_instance_id());
}
