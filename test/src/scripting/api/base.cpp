
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

class ScriptingSerializationTest : public ScriptingBaseTest {
  public:
	ScriptingSerializationTest() = default;
};

TEST_F(ScriptingSerializationTest, serializeNil)
{
	this->EvalTestScript();
}

TEST_F(ScriptingSerializationTest, serializeBoolean)
{
	this->EvalTestScript();
}

TEST_F(ScriptingSerializationTest, serializeString)
{
	this->EvalTestScript();
}

TEST_F(ScriptingSerializationTest, serializeNumber)
{
	this->EvalTestScript();
}

TEST_F(ScriptingSerializationTest, serializeTable)
{
	this->EvalTestScript();
}
