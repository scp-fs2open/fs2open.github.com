//
//

#include "hudgauge.h"
#include "hud/hudscripting.h"

namespace scripting {
namespace api {

ADE_OBJ(l_HudGauge, HudGauge*, "HudGauge", "HUD Gauge handle");

ADE_VIRTVAR(Name, l_HudGauge, "string", "Custom HUD Gauge name", "string", "Custom HUD Gauge name, or nil if handle is invalid")
{
	HudGauge* gauge;

	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	if (gauge->getObjectType() != HUD_OBJECT_CUSTOM)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "s", gauge->getCustomGaugeName());
}

ADE_VIRTVAR(Text, l_HudGauge, "string", "Custom HUD Gauge text", "string", "Custom HUD Gauge text, or nil if handle is invalid")
{
	HudGauge* gauge;
	const char* text = nullptr;

	if (!ade_get_args(L, "o|s", l_HudGauge.Get(&gauge), &text))
		return ADE_RETURN_NIL;

	if (gauge->getObjectType() != HUD_OBJECT_CUSTOM)
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR && text != NULL)
	{
		gauge->updateCustomGaugeText(text);
	}

	return ade_set_args(L, "s", gauge->getCustomGaugeText());
}

ADE_VIRTVAR(RenderFunction,
            l_HudGauge,
            "function (HudGaugeDrawFunctions gauge_handle)",
            "For scripted HUD gauges, the function that will be called for rendering the HUD gauge",
            "function (HudGaugeDrawFunctions gauge_handle)",
            "Render function or nil if no action is set or handle is invalid") {
	HudGauge* gauge;
	luacpp::LuaFunction func;

	if (!ade_get_args(L, "o|u", l_HudGauge.Get(&gauge), &func)) {
		return ADE_RETURN_NIL;
	}

	if (gauge->getObjectType() != HUD_OBJECT_SCRIPTING) {
		return ADE_RETURN_NIL;
	}

	auto scriptedGauge = static_cast<HudGaugeScripting*>(gauge);

	if (ADE_SETTING_VAR && func.isValid()) {
		scriptedGauge->setRenderFunction(func);
	}

	return ade_set_args(L, "u", scriptedGauge->getRenderFunction());
}

ADE_OBJ(l_HudGaugeDrawFuncs,
        HudGauge*,
        "HudGaugeDrawFunctions",
        "Handle to the rendering functions used for HUD gauges. Do not keep a reference to this since these are only useful inside the rendering callback of a HUD gauge.");

ADE_FUNC(drawString,
         l_HudGaugeDrawFuncs,
         "number x, number y, string text",
         "Draws a string in the context of the HUD gauge.",
         "boolean",
         "true on success, false otherwise") {
	HudGauge* gauge;
	float x;
	float y;
	const char* text;

	if (!ade_get_args(L, "offs", l_HudGaugeDrawFuncs.Get(&gauge), &x, &y, &text)) {
		return ADE_RETURN_FALSE;
	}

	int gauge_x, gauge_y;
	gauge->getPosition(&gauge_x, &gauge_y);

	gauge->renderString(fl2i(gauge_x + x), fl2i(gauge_y), text);

	return ADE_RETURN_TRUE;
}

}
}
