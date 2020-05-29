
#include "scripting/ScriptingTestFixture.h"

class ScriptingAsyncTest : public test::scripting::ScriptingTestFixture {
  public:
	ScriptingAsyncTest() : test::scripting::ScriptingTestFixture(INIT_CFILE) { pushModDir("async"); }
};

class ScriptingAsyncPromiseTest : public ScriptingAsyncTest {
  public:
	ScriptingAsyncPromiseTest() { pushModDir("promise"); }
};

TEST_F(ScriptingAsyncPromiseTest, callWithNonFunction)
{
	ASSERT_THROW(this->EvalTestScript(), os::dialogs::LuaErrorException);
}

TEST_F(ScriptingAsyncPromiseTest, callResolveTwice)
{
	ASSERT_THROW(this->EvalTestScript(), os::dialogs::LuaErrorException);
}

TEST_F(ScriptingAsyncPromiseTest, callRejectTwice)
{
	ASSERT_THROW(this->EvalTestScript(), os::dialogs::LuaErrorException);
}

TEST_F(ScriptingAsyncPromiseTest, callBothResolveReject)
{
	ASSERT_THROW(this->EvalTestScript(), os::dialogs::LuaErrorException);
}

TEST_F(ScriptingAsyncPromiseTest, resolveWithValues)
{
	this->EvalTestScript();
}

TEST_F(ScriptingAsyncPromiseTest, catchErrors)
{
	this->EvalTestScript();
}

TEST_F(ScriptingAsyncPromiseTest, thenCatchChains)
{
	this->EvalTestScript();
}

class ScriptingAsyncRunTest : public ScriptingAsyncTest {
  public:
	ScriptingAsyncRunTest() { pushModDir("run"); }
};

TEST_F(ScriptingAsyncRunTest, runWithoutAwait)
{
	this->EvalTestScript();
}

TEST_F(ScriptingAsyncRunTest, runWithResolvedAwait)
{
	this->EvalTestScript();
}

TEST_F(ScriptingAsyncRunTest, runWithErroredAwait)
{
	this->EvalTestScript();
}
