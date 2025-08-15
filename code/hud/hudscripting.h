#pragma once

#include "globalincs/pstypes.h"
#include "hud/hud.h"
#include "scripting/lua/LuaFunction.h"

class HudGaugeScripting: public HudGauge {
	luacpp::LuaFunction _renderFunction;
  public:
	HudGaugeScripting();

	void render(float frametime, bool config = false) override;

	void initName(SCP_string name);

	const luacpp::LuaFunction& getRenderFunction() const;
	void setRenderFunction(const luacpp::LuaFunction& renderFunction);
};
