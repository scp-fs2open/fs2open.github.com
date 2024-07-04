//
//

#include "ScriptingTestFixture.h"
#include "cfile/cfile.h"

extern "C" {
#include <lua.h>
}

namespace test {
namespace scripting {

ScriptingTestFixture::ScriptingTestFixture(uint64_t init_flags) : FSTestFixture(init_flags) {
	pushModDir("scripting");
}
void ScriptingTestFixture::SetUp() {
	FSTestFixture::SetUp();

	_state.reset(new script_state("Test state"));
	_state->CreateLuaState();
}
void ScriptingTestFixture::TearDown() {
	_state = nullptr;

	FSTestFixture::TearDown();
}
void ScriptingTestFixture::EvalTestScript() {
	auto fp = cfopen("test.lua", "rb", CFILE_NORMAL, CF_TYPE_SCRIPTS);
	if (fp == nullptr) {
		FAIL() << "Failed to open test file!";
	}

	auto length = cfilelength(fp);
	SCP_string content;
	content.resize(length);

	auto read = cfread(&content[0], 1, length, fp);
	cfclose(fp);

	if (read != length) {
		FAIL() << "Failed to read test file!";
	}

	const ::testing::TestInfo* const test_info =
		::testing::UnitTest::GetInstance()->current_test_info();

	std::stringstream ss;
	ss << test_info->test_case_name() << ": " << test_info->name();

	auto success = _state->EvalString(content.c_str(), ss.str().c_str());
	if (!success) {
		const char* error = lua_tostring(_state->GetLuaSession(), -1);
		FAIL() << error << "\n";
	}
}

}
}
