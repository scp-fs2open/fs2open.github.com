#pragma once

#include "actions/Action.h"
#include "actions/expression/ActionExpression.h"
#include "utils/RandomRange.h"

namespace actions {
namespace types {

class WaitAction : public Action {
	expression::ActionExpression<float> _waitTimeExpression;

  public:
	~WaitAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	void parseValues(const flagset<ProgramContextFlags>& parse_flags) override;

	std::unique_ptr<Action> clone() const override;
};

} // namespace types
} // namespace actions
