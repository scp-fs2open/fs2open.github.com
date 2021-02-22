
#include "MoveToSubmodel.h"

#include "math/vecmat.h"
#include "parse/parselo.h"
#include "ship/ship.h"

namespace actions {
namespace types {

MoveToSubmodel::~MoveToSubmodel() = default;

ActionResult MoveToSubmodel::execute(ProgramLocals& locals) const
{
	// The calling code should ensure that this never happens
	Assertion(locals.hostSubobject >= 0, "Did not have a valid host subobject.");

	auto instance = object_get_model_instance(locals.host.objp);
	Assertion(instance != -1, "Model instances are required if a host subobject is specified.");

	auto pmi = model_get_instance(instance);
	auto pm = model_get(pmi->model_num);

	if (_goToParent) {
		// Special case when we simply want to go to our parent without explicitly specifying its name
		auto parentSubobject = pm->submodel[locals.hostSubobject].parent;

		if (parentSubobject < 0) {
			Warning(LOCATION,
				"Current program subobject %s on model %s has no parent!",
				pm->submodel[locals.hostSubobject].name,
				pm->filename);
			return ActionResult::Errored;
		}

		// We have a valid parent so this move will work properly
		locals.hostSubobject = parentSubobject;
		return ActionResult::Finished;
	}

	// We need to do a linear search for the right subobject
	for (int i = 0; i < pm->n_models; ++i) {
		const auto submodel = pm->submodel[i];

		if (subsystem_stricmp(_destination.c_str(), submodel.name) == 0) {
			// Found something!
			locals.hostSubobject = i;
			return ActionResult::Finished;
		}
	}

	Warning(LOCATION, "Could not find subobject %s in model %s!", _destination.c_str(), pm->filename);
	return ActionResult::Errored;
}

void MoveToSubmodel::parseValues(const flagset<ProgramContextFlags>& parse_flags)
{
	if (!parse_flags[ProgramContextFlags::HasObject]) {
		error_display(1, "This action requires a host object but in this context none is available!");
	}
	if (!parse_flags[ProgramContextFlags::HasSubobject]) {
		error_display(1, "This action required a host sub-object but in this context none is available!");
	}

	if (optional_string("<parent>")) {
		_goToParent = true;
	} else {
		stuff_string(_destination, F_NAME);
	}
}

std::unique_ptr<Action> MoveToSubmodel::clone() const
{
	return std::unique_ptr<Action>(new MoveToSubmodel(*this));
}

} // namespace types
} // namespace actions
