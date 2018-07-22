#pragma once

#include "actions/Action.h"

namespace actions {
namespace types {

class SetPositionAction : public Action {
	vec3d _newPos = vmd_zero_vector;

  public:
	~SetPositionAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	void parseValues(const flagset<ProgramContextFlags>& parse_flags) override;

	std::unique_ptr<Action> clone() const override;
};

} // namespace types
} // namespace actions
