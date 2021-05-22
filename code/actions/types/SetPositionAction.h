#pragma once

#include "actions/Action.h"
#include "actions/expression/ActionExpression.h"

namespace actions {
namespace types {

class SetPositionAction : public Action {

  public:
	using ValueType = vec3d;

	static flagset<ProgramContextFlags> getRequiredExecutionContextFlags();

	SetPositionAction(expression::TypedActionExpression<ValueType> newPositionExpression);
	~SetPositionAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	std::unique_ptr<Action> clone() const override;

  private:
	expression::TypedActionExpression<vec3d> m_newPosExpression;
};

} // namespace types
} // namespace actions
