#pragma once

#include "actions/Action.h"
#include "actions/expression/ActionExpression.h"

namespace actions {
namespace types {

class SetPositionAction : public Action {
	expression::ActionExpression<vec3d> _newPosExpression;

  public:
	~SetPositionAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	void parseValues(const flagset<ProgramContextFlags>& parse_flags) override;

	std::unique_ptr<Action> clone() const override;
};

} // namespace types
} // namespace actions
