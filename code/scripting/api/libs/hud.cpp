//
//

#include "hud.h"
#include "scripting/api/objs/hudgauge.h"

#include "hud/hud.h"
#include "hud/hudconfig.h"

namespace scripting {
namespace api {


//**********LIBRARY: HUD library
ADE_LIB(l_HUD, "HUD", "hu", "HUD library");

ADE_VIRTVAR(HUDDrawn, l_HUD, "boolean", "Current HUD draw status", "boolean", "If the HUD is drawn or not")
{
	bool to_draw = false;

	if(!ade_get_args(L, "*|b", &to_draw))
		return ADE_RETURN_NIL;

	if(ADE_SETTING_VAR)
	{
		if (to_draw)
			HUD_draw = 1;
		else
			HUD_draw = 0;
	}

	if (HUD_draw)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(HUDDisabledExceptMessages, l_HUD, "boolean", "Specifies if only the messages gauges of the hud are drawn", "boolean", "true if only the message gauges are drawn, false otherwise")
{
	bool to_draw = false;

	if (!ade_get_args(L, "*|b", &to_draw))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
	{
		hud_disable_except_messages(to_draw);
	}

	if (hud_disabled_except_messages())
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(HUDDefaultGaugeCount, l_HUD, "number", "Specifies the amount of HUD gauges defined by FSO", "number", "The number of FSO HUD gauges")
{
	int amount = (int)default_hud_gauges.size();

	return ade_set_args(L, "i", amount);
}

ADE_FUNC(setHUDGaugeColor, l_HUD,
         "number gaugeIndex, [number red, number green, number blue, number alpha]",
         "Modifies color used to draw the gauge in the pilot config", "boolean", "If the operation was successful")
{
	int idx = -1;
	int r = 0;
	int g = 0;
	int b = 0;
	int a = 0;

	if(!ade_get_args(L, "i|iiii", &idx, &r, &g, &b, &a))
		return ADE_RETURN_FALSE;

	if ((idx < 0) || (idx >= NUM_HUD_GAUGES))
		return ADE_RETURN_FALSE;

	gr_init_alphacolor(&HUD_config.clr[idx], r, g, b, a);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getHUDGaugeColor,
	l_HUD,
	"number gaugeIndex",
	"Color specified in the config to draw the gauge",
	"number, number, number, number",
	"Red, green, blue, and alpha of the gauge")
{
	int idx = -1;

	if(!ade_get_args(L, "i", &idx))
		return ADE_RETURN_NIL;

	if ((idx < 0) || (idx >= NUM_HUD_GAUGES))
		return ADE_RETURN_NIL;

	color c = HUD_config.clr[idx];

	return ade_set_args(L, "iiii", (int) c.red, (int) c.green, (int) c.blue, (int) c.alpha);
}

ADE_FUNC(setHUDGaugeColorInMission, l_HUD,
         "number gaugeIndex, [number red, number green, number blue, number alpha]",
         "Set color currently used to draw the gauge", "boolean", "If the operation was successful")
{
	int idx = -1;
	int r = 0;
	int g = 0;
	int b = 0;
	int a = 255;

	if(!ade_get_args(L, "i|iiii", &idx, &r, &g, &b, &a))
		return ADE_RETURN_FALSE;

	if ((idx < 0) || (idx >= (int)default_hud_gauges.size()))
		return ADE_RETURN_FALSE;

	default_hud_gauges[idx]->updateColor(r, g, b, a);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getHUDGaugeColorInMission,
	l_HUD,
	"number gaugeIndex",
	"Color currently used to draw the gauge",
	"number, number, number, number",
	"Red, green, blue, and alpha of the gauge")
{
	int idx = -1;

	if(!ade_get_args(L, "i", &idx))
		return ADE_RETURN_NIL;

	if ((idx < 0) || (idx >= (int)default_hud_gauges.size()))
		return ADE_RETURN_NIL;

	color c = default_hud_gauges[idx]->getColor();

	return ade_set_args(L, "iiii", (int) c.red, (int) c.green, (int) c.blue, (int) c.alpha);
}

ADE_FUNC(getHUDGaugeHandle, l_HUD, "string Name", "Returns a handle to a specified HUD gauge", "HudGauge", "HUD Gauge handle, or nil if invalid")
{
	const char* name;
	if (!ade_get_args(L, "s", &name))
		return ADE_RETURN_NIL;
	HudGauge* gauge = NULL;

	gauge = hud_get_gauge(name);

	if (gauge == NULL)
		return ADE_RETURN_NIL;
	else
		return ade_set_args(L, "o", l_HudGauge.Set(gauge));
}


}
}

