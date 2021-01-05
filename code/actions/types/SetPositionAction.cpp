
#include "SetPositionAction.h"

#include "parse/parselo.h"

namespace actions {
namespace types {
SetPositionAction::~SetPositionAction() = default;

ActionResult SetPositionAction::execute(ProgramLocals& locals) const
{
	locals.position = _newPosExpression.execute();
	return ActionResult::Finished;
}

void SetPositionAction::parseValues(const flagset<ProgramContextFlags>& /*parse_flags*/)
{
	_newPosExpression = expression::ActionExpression<vec3d>::parseFromTable();
}

std::unique_ptr<Action> SetPositionAction::clone() const
{
	return std::unique_ptr<Action>(new SetPositionAction(*this));
}

} // namespace types
} // namespace actions
