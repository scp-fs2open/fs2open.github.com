#pragma once

#include "globalincs/flagset.h"

#include "actions/Action.h"
#include "actions/expression/ActionExpression.h"

namespace actions {
namespace types {

class MoveToSubmodel : public Action {
  public:
	using ValueType = SCP_string;

	static flagset<ProgramContextFlags> getRequiredExecutionContextFlags();

	explicit MoveToSubmodel(expression::TypedActionExpression<ValueType> subObjectExpression);
	~MoveToSubmodel() override;

	ActionResult execute(ProgramLocals& locals) const override;

	std::unique_ptr<Action> clone() const override;

  private:
	expression::TypedActionExpression<ValueType> m_subObjectExpression;
};

} // namespace types
} // namespace actions
