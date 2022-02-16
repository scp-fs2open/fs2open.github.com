//
//

#include "PlaySoundAction.h"

#include "ship/ship.h"

#include <utility>

namespace actions {
namespace types {

flagset<ProgramContextFlags> PlaySoundAction::getRequiredExecutionContextFlags()
{
	return flagset<ProgramContextFlags>{ProgramContextFlags::HasObject};
}

PlaySoundAction::PlaySoundAction(expression::TypedActionExpression<ValueType> soundIdExpression)
	: m_soundIdExpression(std::move(soundIdExpression))
{
}
PlaySoundAction::~PlaySoundAction() = default;

ActionResult PlaySoundAction::execute(ProgramLocals& locals) const
{
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

	vec3d global_pos;
	vm_vec_unrotate(&global_pos, &local_pos, &locals.host.objp->orient);
	global_pos += locals.host.objp->pos;

	const auto soundId = gamesnd_get_by_name(m_soundIdExpression.execute(locals.variables).c_str());

	// Sound is not attached to the host but the object sound system currently only supports persistent sounds
	snd_play_3d(gamesnd_get_game_sound(soundId), &global_pos, &Eye_position);

	return ActionResult::Finished;
}
std::unique_ptr<Action> PlaySoundAction::clone() const
{
	return std::unique_ptr<Action>(new PlaySoundAction(*this));
}
} // namespace types
} // namespace actions
