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
	int getIndex() const;
};

struct preset_h {
	int preset;
	preset_h();
	explicit preset_h(int l_preset);
	CC_preset* getPreset() const;
};

DECLARE_ADE_OBJ(l_Control, control_h);
DECLARE_ADE_OBJ(l_Preset, preset_h);

} // namespace api
} // namespace scripting