
#include "TestUtil.h"

#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaThread.h"

#include "scripting/ScriptingTestFixture.h"

using namespace luacpp;

//LuaThreads are no longer valid without a full script_state, as cleanup isn't possible for pure lua
//hence, this is now a full scriptingtestfixture and not just a Lua Test
class LuaThreadTest : public test::scripting::ScriptingTestFixture {
public:
	LuaThreadTest() : test::scripting::ScriptingTestFixture(INIT_CFILE) {}
};

TEST_F(LuaThreadTest, CreateYieldFinish)
{
	lua_State* L = _state->GetLuaSession();
	ScopedLuaStackTest stackTest(L);

	const auto mainFunc = LuaFunction::createFromCode(L, R"(
local yield_val = coroutine.yield("Test")
return yield_val
)");

	const auto thread = LuaThread::create(L, mainFunc);

	// Initial start
	const auto initialResult = thread.resume(LuaValueList());

	ASSERT_FALSE(initialResult.completed);

	ASSERT_EQ(1, static_cast<int>(initialResult.returnVals.size()));
	ASSERT_EQ("Test", initialResult.returnVals.front().getValue<std::string>());

	const auto resumeResult = thread.resume({LuaValue::createValue(L, "TestValue")});

	ASSERT_TRUE(resumeResult.completed);

	ASSERT_EQ(1, static_cast<int>(resumeResult.returnVals.size()));
	ASSERT_EQ("TestValue", resumeResult.returnVals.front().getValue<std::string>());
}

TEST_F(LuaThreadTest, OnErrorCallbackIsCalled)
{
	lua_State* L = _state->GetLuaSession();
	ScopedLuaStackTest stackTest(L);

	const auto mainFunc = LuaFunction::createFromCode(L, R"(
local val = nil
coroutine.yield(val.test)
)");

	auto thread      = LuaThread::create(L, mainFunc);
	bool errorCalled = false;
	thread.setErrorCallback([&errorCalled](lua_State* /*mainState*/, lua_State* /*thread*/) {
		errorCalled = true;
		return true;
	});

	// Initial start
	const auto initialResult = thread.resume(LuaValueList());

	ASSERT_TRUE(initialResult.completed);

	ASSERT_EQ(0, static_cast<int>(initialResult.returnVals.size()));

	ASSERT_TRUE(errorCalled);
}
