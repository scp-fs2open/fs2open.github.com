#pragma once

#include "actions/Action.h"

namespace actions {
namespace types {

class SetDirectionAction : public Action {
	vec3d _newDir = vmd_zero_vector;

  public:
	~SetDirectionAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	void parseValues(const flagset<ProgramContextFlags>& parse_flags) override;

	std::unique_ptr<Action> clone() const override;
};

} // namespace types
} // namespace actions
