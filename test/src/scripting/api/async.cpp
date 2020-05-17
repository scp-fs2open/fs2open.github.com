
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

TEST_F(ScriptingAsyncPromiseTest, resolveWithValues)
{
	this->EvalTestScript();
}

class ScriptingAsyncAsyncRunTest : public ScriptingAsyncTest {
  public:
	ScriptingAsyncAsyncRunTest() { pushModDir("run"); }
};

TEST_F(ScriptingAsyncAsyncRunTest, runWithoutAwait)
{
	this->EvalTestScript();
}

TEST_F(ScriptingAsyncAsyncRunTest, runWithResolvedAwait)
{
	this->EvalTestScript();
}
