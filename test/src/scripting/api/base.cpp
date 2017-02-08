
#include "scripting/ScriptingTestFixture.h"

class ScriptingBaseTest : public test::scripting::ScriptingTestFixture {
 public:
	ScriptingBaseTest() : test::scripting::ScriptingTestFixture(0) {
		pushModDir("base");
	}
};

TEST_F(ScriptingBaseTest, print) {
	this->EvalTestScript();
}
