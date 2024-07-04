
#include "TestUtil.h"

#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaThread.h"

using namespace luacpp;

class LuaThreadTest : public LuaStateTest {
};

TEST_F(LuaThreadTest, CreateYieldFinish)
{
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
