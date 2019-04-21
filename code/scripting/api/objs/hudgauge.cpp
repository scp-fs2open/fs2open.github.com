//
//

#include "hudgauge.h"
#include "hud/hudscripting.h"
#include "texture.h"

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
         "string text, number x, number y",
         "Draws a string in the context of the HUD gauge.",
         "boolean",
         "true on success, false otherwise") {
	HudGauge* gauge;
	float x;
	float y;
	const char* text;

	if (!ade_get_args(L, "osff", l_HudGaugeDrawFuncs.Get(&gauge), &text, &x, &y)) {
		return ADE_RETURN_FALSE;
	}

	int gauge_x, gauge_y;
	gauge->getPosition(&gauge_x, &gauge_y);

	gauge->renderString(fl2i(gauge_x + x), fl2i(gauge_y + y), text);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawLine, l_HudGaugeDrawFuncs, "number X1, number Y1, number X2, number Y2",
         "Draws a line in the context of the HUD gauge.", "boolean", "true on success, false otherwise")
{
	HudGauge* gauge;
	float x1;
	float y1;
	float x2;
	float y2;

	if (!ade_get_args(L, "offff", l_HudGaugeDrawFuncs.Get(&gauge), &x1, &y1, &x2, &y2)) {
		return ADE_RETURN_FALSE;
	}

	int gauge_x, gauge_y;
	gauge->getPosition(&gauge_x, &gauge_y);

	gauge->renderLine(fl2i(gauge_x + x1), fl2i(gauge_y + y1), fl2i(gauge_x + x2), fl2i(gauge_y + y2));

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawCircle, l_HudGaugeDrawFuncs, "number radius, number X, number Y, [boolean filled=true]",
         "Draws a circle in the context of the HUD gauge.", "boolean", "true on success, false otherwise")
{
	HudGauge* gauge;
	float x;
	float y;
	float radius;
	bool filled=true;

	if (!ade_get_args(L, "offf|b", l_HudGaugeDrawFuncs.Get(&gauge), &radius, &x, &y, &filled)) {
		return ADE_RETURN_FALSE;
	}

	int gauge_x, gauge_y;
	gauge->getPosition(&gauge_x, &gauge_y);

	gauge->renderCircle(fl2i(gauge_x + x), fl2i(gauge_y + y), fl2i(radius*2), filled);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawRectangle, l_HudGaugeDrawFuncs, "number X1, number Y1, number X2, number Y2, [boolean Filled=true]",
         "Draws a rectangle in the context of the HUD gauge.", "boolean", "true on success, false otherwise")
{
	HudGauge* gauge;
	float x1;
	float y1;
	float x2;
	float y2;
	bool filled = true;

	if (!ade_get_args(L, "offff|b", l_HudGaugeDrawFuncs.Get(&gauge), &x1, &y1, &x2, &y2, &filled)) {
		return ADE_RETURN_FALSE;
	}

	int gauge_x, gauge_y;
	gauge->getPosition(&gauge_x, &gauge_y);

	if (filled) {
		gauge->renderRect(fl2i(gauge_x + x1), fl2i(gauge_y + y1), fl2i(x2-x1), fl2i(y2-y1));
	} else {
		gauge->renderLine(fl2i(gauge_x + x1), fl2i(gauge_y + y1), fl2i(gauge_x + x2), fl2i(gauge_y + y1));
		gauge->renderLine(fl2i(gauge_x + x1), fl2i(gauge_y + y2), fl2i(gauge_x + x2), fl2i(gauge_y + y2));
		gauge->renderLine(fl2i(gauge_x + x1), fl2i(gauge_y + y1), fl2i(gauge_x + x1), fl2i(gauge_y + y2));
		gauge->renderLine(fl2i(gauge_x + x2), fl2i(gauge_y + y1), fl2i(gauge_x + x2), fl2i(gauge_y + y2));
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawImage, l_HudGaugeDrawFuncs, "texture handle Texture, [number X=0, Y=0]",
         "Draws an image in the context of the HUD gauge.", "boolean",
         "true on success, false otherwise")
{
	HudGauge* gauge;
	texture_h* texture = nullptr;
	float x1           = 0;
	float y1           = 0;

	if (!ade_get_args(L, "oo|ff", l_HudGaugeDrawFuncs.Get(&gauge), l_Texture.GetPtr(&texture), &x1, &y1)) {
		return ADE_RETURN_FALSE;
	}
	if (!texture->isValid()) {
		return ADE_RETURN_FALSE;
	}

	int gauge_x, gauge_y;
	gauge->getPosition(&gauge_x, &gauge_y);

	gauge->renderBitmapColor(texture->handle, fl2i(gauge_x + x1), fl2i(gauge_y + y1));

	return ADE_RETURN_TRUE;
}

}
}
