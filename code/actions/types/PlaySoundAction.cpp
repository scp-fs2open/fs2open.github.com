//
//

#include "PlaySoundAction.h"

#include "ship/ship.h"

namespace actions {
namespace types {
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

	vec3d global_pos;
	vm_vec_unrotate(&global_pos, &local_pos, &locals.host.objp->orient);
	global_pos += locals.host.objp->pos;

	// Sound is not attached to the host but the object sound system currently only supports persistent sounds
	snd_play_3d(gamesnd_get_game_sound(_soundId), &global_pos, &Eye_position);

	return ActionResult::Finished;
}
void PlaySoundAction::parseValues(const flagset<ProgramContextFlags>& parse_flags)
{
	if (!parse_flags[ProgramContextFlags::HasObject]) {
		error_display(1, "The sound effect action requires a host object but in this context none is available!");
	}
	_soundId = parse_game_sound_inline();
}
std::unique_ptr<Action> PlaySoundAction::clone() const
{
	return std::unique_ptr<Action>(new PlaySoundAction(*this));
}
} // namespace types
} // namespace actions
