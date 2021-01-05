
#include "SetDirectionAction.h"

#include "math/vecmat.h"
#include "parse/parselo.h"

namespace actions {
namespace types {
SetDirectionAction::~SetDirectionAction() = default;

ActionResult SetDirectionAction::execute(ProgramLocals& locals) const
{
	auto dir = _newDirExpression.execute();

	vm_vec_normalize_safe(&dir);

	locals.direction = dir;
	return ActionResult::Finished;
}

void SetDirectionAction::parseValues(const flagset<ProgramContextFlags>& /*parse_flags*/)
{
	_newDirExpression = expression::ActionExpression<vec3d>::parseFromTable();
}

std::unique_ptr<Action> SetDirectionAction::clone() const
{
	return std::unique_ptr<Action>(new SetDirectionAction(*this));
}

} // namespace types
} // namespace actions
