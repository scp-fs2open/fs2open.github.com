
#include "SetDirectionAction.h"

#include "math/vecmat.h"
#include "parse/parselo.h"

#include <utility>

namespace actions {
namespace types {

flagset<ProgramContextFlags> SetDirectionAction::getRequiredExecutionContextFlags()
{
	return flagset<ProgramContextFlags>{};
}

SetDirectionAction::SetDirectionAction(expression::TypedActionExpression<vec3d> newDirExpression)
	: m_newDirExpression(std::move(newDirExpression))
{
}
SetDirectionAction::~SetDirectionAction() = default;

ActionResult SetDirectionAction::execute(ProgramLocals& locals) const
{
	auto dir = m_newDirExpression.execute(locals.variables);

	vm_vec_normalize_safe(&dir);

	locals.variables.setValue({"locals", "direction"}, expression::Value(dir));
	return ActionResult::Finished;
}

std::unique_ptr<Action> SetDirectionAction::clone() const
{
	return std::unique_ptr<Action>(new SetDirectionAction(*this));
}

} // namespace types
} // namespace actions
