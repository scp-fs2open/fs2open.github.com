//
//

#include "hudgauge.h"
#include "hud/hudscripting.h"
#include "scripting/api/objs/color.h"
#include "font.h"
#include "texture.h"

namespace scripting {
namespace api {

ADE_OBJ(l_HudGauge, HudGauge*, "HudGauge", "HUD Gauge handle");

ADE_FUNC(isCustom, l_HudGauge, nullptr, "Custom HUD Gauge status", "boolean", "Returns true if this is a custom HUD gauge, or false if it is a non-custom (default) HUD gauge")
{
	HudGauge* gauge;

	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", gauge->isCustom());
}

ADE_VIRTVAR(Name, l_HudGauge, "string", "Custom HUD Gauge name", "string", "Custom HUD Gauge name, or nil if this is a default gauge or the handle is invalid")
{
	HudGauge* gauge;

	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	if (gauge->getObjectType() != HUD_OBJECT_CUSTOM)
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting the hud gauge name is not supported");

	return ade_set_args(L, "s", gauge->getCustomGaugeName());
}

ADE_VIRTVAR(Text, l_HudGauge, "string", "Custom HUD Gauge text", "string", "Custom HUD Gauge text, or nil if this is a default gauge or the handle is invalid")
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

ADE_VIRTVAR(ConfigType, l_HudGauge, "string", "The config type (such as \"LEAD_INDICATOR\") of this HUD Gauge", "string", "HUD Gauge config type, or nil if this gauge does not have a config type (custom gauges and some default gauges do not) or if the handle is invalid")
{
	HudGauge* gauge;

	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting the hud gauge config type is not supported");

	int type = gauge->getConfigType();
	if (type >= 0 && type < NUM_HUD_GAUGES)
		return ade_set_args(L, "s", Legacy_HUD_gauges[type].hud_gauge_text);

	return ADE_RETURN_NIL;
}

ADE_VIRTVAR(ObjectType, l_HudGauge, "string", "The object type (such as \"Lead indicator\") of this HUD Gauge", "string", "HUD Gauge object type, or nil if this gauge does not have an object type or if the handle is invalid")
{
	HudGauge* gauge;

	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
		LuaError(L, "Setting the hud gauge object type is not supported");

	int type = gauge->getObjectType();
	if (type == HUD_OBJECT_CUSTOM)
		return ade_set_args(L, "s", "Custom");

	for (int i = 0; i < Num_hud_gauge_types; i++)
	{
		if (type == Hud_gauge_types[i].def)
			return ade_set_args(L, "s", Hud_gauge_types[i].name);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(getBaseResolution, l_HudGauge, nullptr, "Returns the base width and base height (which may be different from the screen width and height) used by the specified HUD gauge.",
	"number, number", "Base width and height")
{
	HudGauge* gauge;
	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "ii", gauge->getBaseWidth(), gauge->getBaseHeight());
}

ADE_FUNC(getAspectQuotient, l_HudGauge, nullptr, "Returns the aspect quotient (ratio between the current aspect ratio and the HUD's native aspect ratio) used by the specified HUD gauge.",
	"number", "Aspect quotient")
{
	HudGauge* gauge;
	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "f", gauge->getAspectQuotient());
}

ADE_FUNC(getPosition, l_HudGauge, nullptr, "Returns the position of the specified HUD gauge.",
	"number, number", "X and Y coordinates")
{
	HudGauge* gauge;
	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	int x, y;
	gauge->getPosition(&x, &y);
	return ade_set_args(L, "ii", x, y);
}

ADE_FUNC(setPosition, l_HudGauge, "number, number", "Sets the position of the specified HUD gauge.", nullptr, nullptr)
{
	HudGauge* gauge;
	int x, y;
	if (!ade_get_args(L, "oii", l_HudGauge.Get(&gauge), &x, &y))
		return ADE_RETURN_NIL;

	gauge->setGaugeCoords(x, y);
	return ADE_RETURN_NIL;
}

ADE_FUNC(getFont, l_HudGauge, nullptr, "Returns the font used by the specified HUD gauge.",
	"font", "The font handle")
{
	HudGauge* gauge;
	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	int font_num = gauge->getFont();

	if (font_num < 0 || font_num >= font::FontManager::numberOfFonts())
		return ade_set_error(L, "o", l_Font.Set(font_h()));

	return ade_set_args(L, "o", l_Font.Set(font_h(font_num)));
}

ADE_FUNC(getOriginAndOffset, l_HudGauge, nullptr, "Returns the origin and offset of the specified HUD gauge as specified in the table.",
	"number, number, number, number", "Origin X, Origin Y, Offset X, Offset Y")
{
	HudGauge* gauge;
	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	float originX, originY;
	int offsetX, offsetY;
	gauge->getOriginAndOffset(&originX, &originY, &offsetX, &offsetY);
	return ade_set_args(L, "ffii", originX, originY, offsetX, offsetY);
}

ADE_FUNC(getCoords, l_HudGauge, nullptr, "Returns the coordinates of the specified HUD gauge as specified in the table.",
	"boolean, number, number", "Coordinates flag (whether coordinates are used), X, Y")
{
	HudGauge* gauge;
	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	bool useCoords;
	int x, y;
	gauge->getCoords(&useCoords, &x, &y);
	return ade_set_args(L, "bii", useCoords, x, y);
}

ADE_FUNC(isHiRes, l_HudGauge, nullptr, "Returns whether this is a hi-res HUD gauge, determined by whether the +Filename property is prefaced with \"2_\".  Not all gauges have such a filename.",
	"boolean", "Whether the HUD gauge is known to be hi-res")
{
	HudGauge* gauge;
	if (!ade_get_args(L, "o", l_HudGauge.Get(&gauge)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", gauge->isHiRes());
}

ADE_FUNC(getColor,
	l_HudGauge,
	nullptr,
	"Gets the active 2D drawing color. False to return raw rgb, true to return a color object. Defaults to false.",
	"number, number, number, number | color",
	"The current color, in red, green, blue, and alpha components from 0 to 255")
{
	HudGauge* gauge;
	bool rc = false;
	if (!ade_get_args(L, "o|b", l_HudGauge.Get(&gauge), &rc))
		return ADE_RETURN_NIL;

	auto &c = gauge->getColor();

	if (rc) {
		return ade_set_args(L, "o", l_Color.Set(c));
	}else{
		return ade_set_args(L, "iiii", static_cast<int>(c.red), static_cast<int>(c.green), static_cast<int>(c.blue), static_cast<int>(c.alpha));
	}
}

ADE_FUNC(setColor,
	l_HudGauge,
	"number|color /* red value or color object */, [number Green, number Blue, number Alpha]",
	"Sets the current color used by this HUD gauge.  Numbers must be 0-255 in red/green/blue/alpha components; alpha is optional.",
	nullptr, nullptr)
{
	HudGauge* gauge = nullptr;
	int r, g, b, a = -1;

	if (lua_isnumber(L, 2)) {
		if (!ade_get_args(L, "oiii|i", l_HudGauge.Get(&gauge) , &r, &g, &b, &a))
			return ADE_RETURN_NIL;

		if (a < 0)
			a = (HUD_color_alpha + 1) * 16; // from sexp_hud_set_color()
		else
			CLAMP(a, 0, 255);

		CLAMP(r, 0, 255);
		CLAMP(g, 0, 255);
		CLAMP(b, 0, 255);
	} else {
		color col;
		gr_init_alphacolor(&col, 0, 0, 0, 255);

		ade_get_args(L, "oo", l_HudGauge.Get(&gauge), l_Color.Get(&col));

		r = col.red;
		g = col.green;
		b = col.blue;
		a = col.alpha;
	}

	if (gauge == nullptr) {
		return ADE_RETURN_NIL;
	}

	gauge->sexpLockConfigColor(false);
	gauge->updateColor(static_cast<ubyte>(r), static_cast<ubyte>(g), static_cast<ubyte>(b), static_cast<ubyte>(a));
	gauge->sexpLockConfigColor(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(setRenderOverride,
	l_HudGauge,
	"boolean",
	"Sets a rendering override to enable or disable rendering of this gauge. This takes precedence over all other gauge toggles!",
	"boolean",
	"True to enable the override and stop the gauge from rendering, false otherwise")
{
	HudGauge* gauge;
	bool override = false;
	if (!ade_get_args(L, "o|b", l_HudGauge.Get(&gauge), &override))
		return ADE_RETURN_NIL;

	gauge->updateScriptingOverride(override);

	return ade_set_args(L, "b", gauge->getScriptingOverride());
}

ADE_VIRTVAR(RenderFunction,
            l_HudGauge,
            "function (HudGaugeDrawFunctions gauge_handle) => void",
            "For scripted HUD gauges, the function that will be called for rendering the HUD gauge",
            "function (HudGaugeDrawFunctions gauge_handle) => void",
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

ADE_FUNC(drawImage, l_HudGaugeDrawFuncs, "texture Texture, [number X=0, number Y=0]",
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
