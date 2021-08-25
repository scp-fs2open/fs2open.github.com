
#include "scripting/scripting.h"
#include "scripting/scripting_doc.h"

#include <gtest/gtest.h>

TEST(ScriptingState, TestValidDocumentation)
{
	script_state state("Documentation test");

	state.OutputDocumentation([](const SCP_string& error) {
		FAIL() << "Failed to parse scripting documentation:\n" << error;
	});
}
