
#include "mod_table/mod_table.h"
#include "scripting/ade_args.h"
#include "scripting/api/objs/shipclass.h"
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

	// Generated from a validator registered with ADE_OBJ_VALIDATOR
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("shipclass"));
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("weaponclass"));
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("team"));
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("loadout_ship"));
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("HudGauge"));
	ASSERT_EQ("Detects whether handle is valid", isValidByClass["shipclass"]->description);

	// Derived classes inherit it rather than listing their own
	ASSERT_EQ(isValidByClass.end(), isValidByClass.find("ship"));
	ASSERT_EQ(isValidByClass.end(), isValidByClass.find("sound3D"));

	// An explicit declaration replaces the generated one
	ASSERT_NE(isValidByClass.end(), isValidByClass.find("sound"));
	ASSERT_NE(SCP_string::npos, isValidByClass["sound"]->description.find("sound entry"));
}

// Exercises the generated isValid() of an int-index handle from Lua
TEST_F(ScriptingBaseTest, intValidator) {
	this->EvalTestScript();
}

// A registered validator must also drive the "return nil instead of an invalid object" option in ade_set_args
TEST_F(ScriptingBaseTest, intValidatorNilReturn) {
	auto L = _state->GetLuaSession();
	const bool previous = Lua_API_returns_nil_instead_of_invalid_object;

	// Headless there are no ship classes, so index 0 is invalid
	Lua_API_returns_nil_instead_of_invalid_object = false;
	::scripting::ade_set_args(L, "o", ::scripting::api::l_Shipclass.Set(0));
	ASSERT_TRUE(lua_isuserdata(L, -1));
	lua_pop(L, 1);

	Lua_API_returns_nil_instead_of_invalid_object = true;
	::scripting::ade_set_args(L, "o", ::scripting::api::l_Shipclass.Set(0));
	ASSERT_TRUE(lua_isnil(L, -1));
	lua_pop(L, 1);

	Lua_API_returns_nil_instead_of_invalid_object = previous;
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
