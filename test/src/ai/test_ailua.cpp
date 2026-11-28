#include "ai/ai.h"
#include "ai/ailua.h"
#include "scripting/lua/LuaValue.h"
#include "scripting/scripting.h"
#include "ship/ship.h"

#include "util/FSTestFixture.h"

namespace {

// The goals of Lua AI modes keep Lua values. The values must be freed before the Lua state closes.
class AiLuaGoalTest : public test::FSTestFixture {
  public:
	AiLuaGoalTest() : test::FSTestFixture(INIT_CFILE) {}

  protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();

		// ai_lua_init() adds a handler to the global Script_system, and nothing removes it. Add it only one
		// time, so that more tests or --gtest_repeat do not add more handlers.
		static bool lua_ai_initialized = false;
		if (!lua_ai_initialized) {
			ai_lua_init();
			lua_ai_initialized = true;
		}
		Script_system.CreateLuaState();
	}

	void TearDown() override {
		Script_system.Clear();

		test::FSTestFixture::TearDown();
	}
};

} // namespace

TEST_F(AiLuaGoalTest, clear_frees_goal_arguments) {
	lua_State* L = Script_system.GetLuaSession();
	ASSERT_NE(L, nullptr);

	Ai_info[0].lua_ai_target.arguments.push_back(luacpp::LuaValue::createValue(L, 1));
	Ai_info[0].goals[0].lua_ai_target.arguments.push_back(luacpp::LuaValue::createValue(L, "goal"));
	Wings[0].ai_goals[0].lua_ai_target.arguments.push_back(luacpp::LuaValue::createValue(L, 2.0));

	Script_system.Clear();

	EXPECT_TRUE(Ai_info[0].lua_ai_target.arguments.empty());
	EXPECT_TRUE(Ai_info[0].goals[0].lua_ai_target.arguments.empty());
	EXPECT_TRUE(Wings[0].ai_goals[0].lua_ai_target.arguments.empty());
}
