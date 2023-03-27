#pragma once

#include "hud/hudconfig.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct gauge_config_h {
	int gauge;
	gauge_config_h();
	explicit gauge_config_h(int l_gauge);
	HC_gauge_region* getGauge() const;
	int getIndex() const;
	const char* getName() const;
};

DECLARE_ADE_OBJ(l_Gauge_Config, gauge_config_h);

} // namespace api
} // namespace scripting