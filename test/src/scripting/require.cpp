
#include "scripting/ScriptingTestFixture.h"

class RequireTest : public test::scripting::ScriptingTestFixture {
 public:
	RequireTest() : test::scripting::ScriptingTestFixture(INIT_CFILE | INIT_MOD_TABLE) {
		pushModDir("require");
	}
};

TEST_F(RequireTest, defaultRequire) {
	this->EvalTestScript();
}

TEST_F(RequireTest, overrideRequire) {
	this->EvalTestScript();
}

