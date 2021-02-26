
#include "SetPositionAction.h"

#include "parse/parselo.h"

#include <utility>

namespace actions {
namespace types {

flagset<ProgramContextFlags> SetPositionAction::getRequiredExecutionContextFlags()
{
	return flagset<ProgramContextFlags>{};
}

SetPositionAction::SetPositionAction(expression::TypedActionExpression<ValueType> newPositionExpression)
	: m_newPosExpression(std::move(newPositionExpression))
{
}
SetPositionAction::~SetPositionAction() = default;

ActionResult SetPositionAction::execute(ProgramLocals& locals) const
{
	locals.variables.setValue({"locals", "position"}, expression::Value(m_newPosExpression.execute(locals.variables)));
	return ActionResult::Finished;
}

std::unique_ptr<Action> SetPositionAction::clone() const
{
	return std::unique_ptr<Action>(new SetPositionAction(*this));
}

} // namespace types
} // namespace actions
