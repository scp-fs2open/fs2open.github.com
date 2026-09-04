
#include "scripting/scripting_doc.h"

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

TEST_F(ScriptingBaseTest, autoIsValid) {
	this->EvalTestScript();
}

// Verifies that automatically generated isValid() functions and explicitly declared ones coexist correctly in the
// documentation: no class lists isValid more than once, and an explicit declaration replaces the generated one.
TEST_F(ScriptingBaseTest, isValidDocumentation) {
	const auto doc = _state->OutputDocumentation([](const SCP_string& error) {
		ADD_FAILURE() << "Documentation error: " << error;
	});

	SCP_unordered_map<SCP_string, const ::scripting::DocumentationElement*> isValidByClass;

	for (const auto& element : doc.elements) {
		if (element->type != ::scripting::ElementType::Class)
			continue;

		int count = 0;
		for (const auto& child : element->children) {
			if (child->name == "isValid") {
				count++;
				isValidByClass[element->name] = child.get();
			}
		}
		ASSERT_LE(count, 1) << "Class '" << element->name << "' lists isValid more than once";
	}

	// Generated from the isValid() member of the stored type
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("object"));
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("gamestate"));
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("parse_subsystem"));
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("soundfile"));
	ASSERT_EQ("Detects whether handle is valid", isValidByClass["object"]->description);

	// Derived classes inherit it rather than listing their own
	ASSERT_EQ(isValidByClass.end(), isValidByClass.find("ship"));
	ASSERT_EQ(isValidByClass.end(), isValidByClass.find("sound3D"));

	// An explicit declaration replaces the generated one
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("sound"));
	ASSERT_NE(SCP_string::npos, isValidByClass["sound"]->description.find("sound entry"));
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
