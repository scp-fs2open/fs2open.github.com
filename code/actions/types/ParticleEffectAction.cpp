
#include "ParticleEffectAction.h"

#include "math/vecmat.h"
#include "parse/parselo.h"
#include "ship/ship.h"

#include <utility>

namespace actions {
namespace types {

flagset<ProgramContextFlags> ParticleEffectAction::getRequiredExecutionContextFlags()
{
	return flagset<ProgramContextFlags>{ProgramContextFlags::HasObject};
}

ParticleEffectAction::ParticleEffectAction(expression::TypedActionExpression<ValueType> effectExpression)
	: m_effectExpression(std::move(effectExpression))
{
}
ParticleEffectAction::~ParticleEffectAction() = default;

ActionResult ParticleEffectAction::execute(ProgramLocals& locals) const
{
	auto effectIdx = particle::ParticleManager::get()->getEffectByName(m_effectExpression.execute(locals.variables));

	if (!effectIdx.isValid()) {
		// In case the parsing code failed
		return ActionResult::Errored;
	}

	using namespace particle;

	auto source = ParticleManager::get()->createSource(effectIdx);

	vec3d local_pos;
	matrix local_orient;
	if (locals.hostSubobject != -1) {
		auto instance = object_get_model_instance(locals.host.objp);
		Assertion(instance != -1, "Model instances are required if a host subobject is specified.");

		auto pmi = model_get_instance(instance);
		auto pm = model_get(pmi->model_num);

		model_instance_local_to_global_point_orient(&local_pos,
			&local_orient,
			&locals.localPosition,
			&locals.localOrient,
			pm,
			pmi,
			locals.hostSubobject);
	} else {
		local_pos = locals.localPosition;
		local_orient = locals.localOrient;
	}

	local_pos += locals.variables.getValue({"locals", "position"}).getVector();

	auto direction = locals.variables.getValue({"locals", "direction"}).getVector();

	source.moveToObject(locals.host.objp, &local_pos);
	source.setOrientationFromNormalizedVec(&direction, true);

	source.finish();

	return ActionResult::Finished;
}

std::unique_ptr<Action> ParticleEffectAction::clone() const
{
	return std::unique_ptr<Action>(new ParticleEffectAction(*this));
}

} // namespace types
} // namespace actions
