#include "animation_handle.h"

namespace scripting {
namespace api {

//**********HANDLE: Animation Handle
ADE_OBJ(l_AnimationHandle, animation_handle_h, "animation_handle", "A handle for animation instances");

ADE_FUNC(start, l_AnimationHandle, "[boolean forwards = true, boolean resetOnStart = false, boolean completeInstant = false, boolean pause = false]",
	"Triggers an animation. Forwards controls the direction of the animation. ResetOnStart will cause the animation to play from its initial state, "
	"as opposed to its current state. CompleteInstant will immediately complete the animation. Pause will instead stop the animation at the current state.",
	"boolean",
	"True if successful, false if no animation was started or nil on failure")
{
	animation_handle_h* anim_set;
	bool forwards = true;
	bool forced = false;
	bool instant = false;
	bool pause = false;

	if (!ade_get_args(L, "o|bbbb", l_AnimationHandle.GetPtr(&anim_set), &forwards, &forced, &instant, &pause))
		return ADE_RETURN_NIL;

	if (anim_set == nullptr)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", anim_set->start(forwards ? animation::ModelAnimationDirection::FWD : animation::ModelAnimationDirection::RWD, forced || instant, instant, pause));
}

ADE_FUNC(getTime, l_AnimationHandle, nullptr,
	"Returns the total duration of this animation, unaffected by the speed set, in seconds.",
	"number",
	"The time this animation will take to complete")
{
	animation_handle_h* anim_set;

	if (!ade_get_args(L, "o", l_AnimationHandle.GetPtr(&anim_set)))
		return ADE_RETURN_NIL;

	if (anim_set == nullptr)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "f", ((float)anim_set->getTime()) * 0.001f);
}

ADE_FUNC(stopNextLoop, l_AnimationHandle, nullptr,
	"Will stop this looping animation on its next repeat.",
	nullptr, nullptr)
{
	animation_handle_h* anim_set;

	if (!ade_get_args(L, "o", l_AnimationHandle.GetPtr(&anim_set)))
		return ADE_RETURN_NIL;

	if (anim_set == nullptr)
		return ADE_RETURN_NIL;

	anim_set->setFlag(animation::Animation_Instance_Flags::Stop_after_next_loop);

	return ADE_RETURN_NIL;
}

}
}