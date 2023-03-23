#pragma once

#include "controlconfig/controlsconfig.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct control_h {
	int control;
	control_h();
	explicit control_h(int l_control);
	CCI* getControl() const;
	conflict* getConflict() const;
};

DECLARE_ADE_OBJ(l_Control, control_h);

} // namespace api
} // namespace scripting