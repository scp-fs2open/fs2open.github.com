
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
	_state->SetHookVar("Test", 's', "Something else");

	// Check that overwriting hook vars works
	_state->SetHookVar("Test", 's', "Hello World");
	_state->SetHookVar("Value", 'i', 1234);

	this->EvalTestScript();

	// Now check that the stack mechanism works. This should result in the old "Test" value
	_state->RemHookVar("Test");

	this->EvalTestScript();

	// And then check if clearing the stacked value works
	_state->RemHookVar("Test");

	this->EvalTestScript();
}

TEST_F(HookVarsTest, removeNonExistent)
{
	// Should not cause any errors
	_state->RemHookVar("Test");
}
