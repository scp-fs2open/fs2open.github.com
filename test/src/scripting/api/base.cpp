
#include "scripting/ScriptingTestFixture.h"

class ScriptingBaseTest : public test::scripting::ScriptingTestFixture {
 public:
	ScriptingBaseTest() : test::scripting::ScriptingTestFixture(INIT_CFILE) {
		pushModDir("base");
	}
};

TEST_F(ScriptingBaseTest, print) {
	this->EvalTestScript();
}
