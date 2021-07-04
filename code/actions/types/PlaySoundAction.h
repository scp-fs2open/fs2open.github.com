#pragma once

#include "actions/Action.h"
#include "actions/expression/ActionExpression.h"
#include "gamesnd/gamesnd.h"

namespace actions {
namespace types {

class PlaySoundAction : public Action {
  public:
	using ValueType = SCP_string;

	static flagset<ProgramContextFlags> getRequiredExecutionContextFlags();

	explicit PlaySoundAction(expression::TypedActionExpression<ValueType> soundIdExpression);
	~PlaySoundAction() override;

	ActionResult execute(ProgramLocals& locals) const override;

	std::unique_ptr<Action> clone() const override;

  private:
	expression::TypedActionExpression<ValueType> m_soundIdExpression;
};

} // namespace types
} // namespace actions
