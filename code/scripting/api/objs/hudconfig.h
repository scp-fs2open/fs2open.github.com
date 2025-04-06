#pragma once

#include "hud/hudconfig.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct gauge_config_h {
	SCP_string gauge;
	gauge_config_h() = default;
	explicit gauge_config_h(SCP_string l_gauge);
	HudGauge* getGauge() const;
	SCP_string getId() const;
	bool isValid() const;
};

struct hud_preset_h {
	int preset;
	hud_preset_h();
	explicit hud_preset_h(int l_preset);
	int getIndex() const;
	SCP_string getName() const;
	bool isValid() const;
};

struct hud_color_preset_h {
	int preset;
	hud_color_preset_h();
	explicit hud_color_preset_h(int l_preset);
	int getIndex() const;
	SCP_string getName() const;
	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Gauge_Config, gauge_config_h);
DECLARE_ADE_OBJ(l_HUD_Preset, hud_preset_h);
DECLARE_ADE_OBJ(l_HUD_Color_Preset, hud_color_preset_h);

} // namespace api
} // namespace scripting