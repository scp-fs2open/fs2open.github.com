#pragma once

#include "nebula/volumetrics.h"
#include "scripting/ade_api.h"

namespace scripting::api {

struct volumetric_h {
	volumetric_h() = default;
	explicit volumetric_h(int idx);

	int index = -1;

	bool isValid() const;
	volumetric_nebula* get();
	const volumetric_nebula* get() const;
};

DECLARE_ADE_OBJ(l_Volumetric, volumetric_h);

} // namespace scripting::api