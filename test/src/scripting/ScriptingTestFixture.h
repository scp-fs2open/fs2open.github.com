#pragma once

#include "util/FSTestFixture.h"

#include "scripting/scripting.h"

#include <memory>

namespace test {
namespace scripting {

class ScriptingTestFixture : public FSTestFixture {
 protected:
	std::unique_ptr<script_state> _state;

	void EvalTestScript();

 public:
	explicit ScriptingTestFixture(uint64_t init_flags);

	void SetUp() override;

	void TearDown() override;
};

}
}
