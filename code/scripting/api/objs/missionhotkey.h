#pragma once

#include "mission/missionhotkey.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct hotkey_h {
	int line;
	hotkey_h();
	explicit hotkey_h(int l_line);
	hotkey_line* getLine() const;
	int getIndex() const;
};

DECLARE_ADE_OBJ(l_Hotkey, hotkey_h);

} // namespace api
} // namespace scripting