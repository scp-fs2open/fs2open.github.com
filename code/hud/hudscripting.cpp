//
//

#include "hud/hudscripting.h"

#include "parse/parselo.h"
#include "scripting/api/objs/hudgauge.h"
#include "scripting/scripting.h"

HudGaugeScripting::HudGaugeScripting() :
	HudGauge(HUD_OBJECT_SCRIPTING,
	         HUD_CENTER_RETICLE,
	         true,
	         false,
	         (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_TOPDOWN | VM_OTHER_SHIP),
	         255,
	         255,
	         255) {
}

void HudGaugeScripting::render(float /*frametime*/, bool config) {
	using namespace scripting::api;

	// Not yet supported in config
	if (config) {
		return;
	}

	if (!_renderFunction.isValid()) {
		return;
	}

	_renderFunction.call(Script_system.GetLuaSession(),
		{luacpp::LuaValue::createValue(_renderFunction.getLuaState(), l_HudGaugeDrawFuncs.Set(this))});
}

void HudGaugeScripting::initName(SCP_string name) {
	if (name.size() > NAME_LENGTH - 1) {
		error_display(0,
		              "Name \"%s\" is too long. May not be longer than %d! Name will be truncated",
		              name.c_str(),
		              NAME_LENGTH - 1);
		name.resize(NAME_LENGTH - 1);
	}
	strcpy(custom_name, name.c_str());
}
const luacpp::LuaFunction& HudGaugeScripting::getRenderFunction() const {
	return _renderFunction;
}
void HudGaugeScripting::setRenderFunction(const luacpp::LuaFunction& renderFunction) {
	_renderFunction = renderFunction;
}
