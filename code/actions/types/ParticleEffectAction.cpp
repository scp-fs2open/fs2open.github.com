
#include "ParticleEffectAction.h"

#include "math/vecmat.h"
#include "parse/parselo.h"
#include "ship/ship.h"

namespace actions {
namespace types {
ParticleEffectAction::~ParticleEffectAction() = default;

ActionResult ParticleEffectAction::execute(ProgramLocals& locals) const
{
	if (!_effectHandle.isValid()) {
		// In case the parsing code failed
		return ActionResult::Finished;
	}

	using namespace particle;

	auto source = ParticleManager::get()->createSource(_effectHandle);

	vec3d local_pos;
	matrix local_orient;
	if (locals.hostSubobject != -1) {
		auto instance = object_get_model_instance(locals.host.objp);
		Assertion(instance != -1, "Model instances are required if a host subobject is specified.");

		auto pmi = model_get_instance(instance);
		auto pm = model_get(pmi->model_num);

		find_submodel_instance_point_orient(&local_pos,
			&local_orient,
			pm,
			pmi,
			locals.hostSubobject,
			&locals.localPosition,
			&locals.localOrient);
	} else {
		local_pos = locals.localPosition;
		local_orient = locals.localOrient;
	}

	local_pos += locals.position;

	source.moveToObject(locals.host.objp, &local_pos);
	source.setOrientationFromNormalizedVec(&locals.direction, true);

	source.finish();

	return ActionResult::Finished;
}

void ParticleEffectAction::parseValues(const flagset<ProgramContextFlags>& parse_flags)
{
	if (!parse_flags[ProgramContextFlags::HasObject]) {
		error_display(1, "The particle effect action requires a host object but in this context none is available!");
	}
	_effectHandle = particle::util::parseEffect();
}

std::unique_ptr<Action> ParticleEffectAction::clone() const
{
	return std::unique_ptr<Action>(new ParticleEffectAction(*this));
}

} // namespace types
} // namespace actions
