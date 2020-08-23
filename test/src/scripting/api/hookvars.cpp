
#include "scripting/ScriptingTestFixture.h"

namespace {

class HookVarsTest : public test::scripting::ScriptingTestFixture {
  public:
	HookVarsTest() : test::scripting::ScriptingTestFixture(INIT_CFILE)
	{
		pushModDir("hookvars");
	}
};

} // namespace

TEST_F(HookVarsTest, empty)
{
	this->EvalTestScript();
}

TEST_F(HookVarsTest, withHookVars)
{
	_state->SetHookVar("Test", 's', "Hello World");
	_state->SetHookVar("Value", 'i', 1234);

	this->EvalTestScript();

	_state->RemHookVar("Value");

	this->EvalTestScript();
}
