
#include "SetDirectionAction.h"

#include "math/vecmat.h"
#include "parse/parselo.h"

namespace actions {
namespace types {
SetDirectionAction::~SetDirectionAction() = default;

ActionResult SetDirectionAction::execute(ProgramLocals& locals) const
{
	locals.direction = _newDir;
	return ActionResult::Finished;
}

void SetDirectionAction::parseValues(const flagset<ProgramContextFlags>& /*parse_flags*/)
{
	stuff_parenthesized_vec3d(&_newDir);

	if (vm_vec_mag(&_newDir) < 0.1f) {
		error_display(0,
			"Zero vector is not valid here! The vector will be normalized so the actual length does not matter.");
		_newDir = vmd_zero_vector;
		_newDir.xyz.z = 1.0f;
	}

	vm_vec_normalize(&_newDir);
}

std::unique_ptr<Action> SetDirectionAction::clone() const
{
	return std::unique_ptr<Action>(new SetDirectionAction(*this));
}

} // namespace types
} // namespace actions
