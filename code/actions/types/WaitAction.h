#pragma once

#include "actions/Action.h"
#include "actions/common.h"
#include "actions/expression/ActionExpression.h"
#include "utils/RandomRange.h"

namespace actions {
namespace types {

class WaitAction : public Action {
  public:
	using ValueType = float;

	static flagset<ProgramContextFlags> getRequiredExecutionContextFlags();

	explicit WaitAction(expression::TypedActionExpression<ValueType> waitTimeExpression);
	~WaitAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	std::unique_ptr<Action> clone() const override;

  private:
	expression::TypedActionExpression<ValueType> _waitTimeExpression;
};

} // namespace types
} // namespace actions
