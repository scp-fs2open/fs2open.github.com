#pragma once

#include "actions/Action.h"
#include "actions/expression/ActionExpression.h"
#include "particle/ParticleManager.h"

namespace actions {
namespace types {

class ParticleEffectAction : public Action {

  public:
	using ValueType = SCP_string;

	static flagset<ProgramContextFlags> getRequiredExecutionContextFlags();

	explicit ParticleEffectAction(expression::TypedActionExpression<ValueType> effectExpression);
	~ParticleEffectAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	std::unique_ptr<Action> clone() const override;

  private:
	expression::TypedActionExpression<ValueType> m_effectExpression;
};

} // namespace types
} // namespace actions
