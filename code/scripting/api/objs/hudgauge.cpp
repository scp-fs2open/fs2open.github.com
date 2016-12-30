//
//

#include "hudgauge.h"

namespace scripting {
namespace api {

ADE_OBJ(l_HudGauge, HudGauge, "HudGauge", "HUD Gauge handle");

ADE_VIRTVAR(Name, l_HudGauge, "string", "Custom HUD Gauge name", "string", "Custom HUD Gauge name, or nil if handle is invalid")
{
	HudGauge* gauge;

	if (!ade_get_args(L, "o", l_HudGauge.GetPtr(&gauge)))
		return ADE_RETURN_NIL;

	if (gauge->getObjectType() != HUD_OBJECT_CUSTOM)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "s", gauge->getCustomGaugeName());
}

ADE_VIRTVAR(Text, l_HudGauge, "string", "Custom HUD Gauge text", "string", "Custom HUD Gauge text, or nil if handle is invalid")
{
	HudGauge* gauge;
	char* text = NULL;

	if (!ade_get_args(L, "o|s", l_HudGauge.GetPtr(&gauge), &text))
		return ADE_RETURN_NIL;

	if (gauge->getObjectType() != HUD_OBJECT_CUSTOM)
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR && text != NULL)
	{
		gauge->updateCustomGaugeText(text);
	}

	return ade_set_args(L, "s", gauge->getCustomGaugeText());
}


}
}
