#pragma once

#include "actions/Action.h"
#include "actions/expression/ActionExpression.h"

namespace actions {
namespace types {

class SetDirectionAction : public Action {
  public:
	using ValueType = vec3d;

	static flagset<ProgramContextFlags> getRequiredExecutionContextFlags();

	SetDirectionAction(expression::TypedActionExpression<ValueType> newDirExpression);
	~SetDirectionAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	std::unique_ptr<Action> clone() const override;

  private:
	expression::TypedActionExpression<ValueType> m_newDirExpression;
};

} // namespace types
} // namespace actions
