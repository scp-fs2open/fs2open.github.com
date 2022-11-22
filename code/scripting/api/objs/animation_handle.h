#pragma once

#include "scripting/ade_api.h"
#include "model/modelanimation.h"

namespace scripting {
	namespace api {

		using animation_handle_h = animation::ModelAnimationSet::AnimationList;

		DECLARE_ADE_OBJ(l_AnimationHandle, animation_handle_h);
	}
}
