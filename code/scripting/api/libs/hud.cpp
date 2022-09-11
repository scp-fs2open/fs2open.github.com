//
//

#include "hud.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/hudgauge.h"
#include "scripting/api/objs/object.h"
#include "scripting/api/objs/vecmath.h"

#include "hud/hud.h"
#include "hud/hudconfig.h"
#include "hud/hudtargetbox.h"
#include "hud/hudtarget.h"

extern int Training_obj_num_display_lines;

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

ADE_VIRTVAR(HUDDefaultGaugeCount, l_HUD, "number", "Specifies the number of HUD gauges defined by FSO.  Note that for historical reasons, HUD scripting functions use a zero-based index (0 to n-1) for gauges.", "number", "The number of FSO HUD gauges")
{
	int amount = (int)default_hud_gauges.size();

	return ade_set_args(L, "i", amount);
}

static int getDefaultGaugeIndex(lua_State* L)
{
	if (lua_isnumber(L, 1))
	{
		int idx = -1;
		if (!ade_get_args(L, "i", &idx))
			return -1;

		return idx;
	}
	else
	{
		const char* name;
		if (!ade_get_args(L, "s", &name))
			return -1;

		return hud_get_default_gauge_index(name);
	}
}

ADE_FUNC(getHUDConfigShowStatus, l_HUD, "number|string gaugeNameOrIndex", "Gets the HUD configuration show status for the specified default HUD gauge.", "boolean", "Returns show status or nil if gauge invalid")
{
	int idx = getDefaultGaugeIndex(L);

	if ((idx < 0) || (idx >= (int)default_hud_gauges.size()))
		return ADE_RETURN_NIL;

	if (hud_config_show_flag_is_set(idx))
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(setHUDGaugeColor, l_HUD,
         "number|string gaugeNameOrIndex, [number red, number green, number blue, number alpha]",
         "Modifies color used to draw the gauge in the pilot config", "boolean", "If the operation was successful")
{
	int idx = getDefaultGaugeIndex(L);
	int r = 0;
	int g = 0;
	int b = 0;
	int a = 0;

	if(!ade_get_args(L, "|iiii", &r, &g, &b, &a))
		return ADE_RETURN_FALSE;

	if ((idx < 0) || (idx >= NUM_HUD_GAUGES))
		return ADE_RETURN_FALSE;

	gr_init_alphacolor(&HUD_config.clr[idx], r, g, b, a);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getHUDGaugeColor,
	l_HUD,
	"number|string gaugeNameOrIndex",
	"Color specified in the config to draw the gauge",
	"number, number, number, number",
	"Red, green, blue, and alpha of the gauge")
{
	int idx = getDefaultGaugeIndex(L);

	if ((idx < 0) || (idx >= NUM_HUD_GAUGES))
		return ADE_RETURN_NIL;

	color c = HUD_config.clr[idx];

	return ade_set_args(L, "iiii", (int) c.red, (int) c.green, (int) c.blue, (int) c.alpha);
}

ADE_FUNC(setHUDGaugeColorInMission, l_HUD,
         "number|string gaugeNameOrIndex, [number red, number green, number blue, number alpha]",
         "Set color currently used to draw the gauge", "boolean", "If the operation was successful")
{
	int idx = getDefaultGaugeIndex(L);
	int r = 0;
	int g = 0;
	int b = 0;
	int a = 255;

	if(!ade_get_args(L, "|iiii", &r, &g, &b, &a))
		return ADE_RETURN_FALSE;

	if ((idx < 0) || (idx >= (int)default_hud_gauges.size()))
		return ADE_RETURN_FALSE;

	default_hud_gauges[idx]->updateColor(r, g, b, a);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getHUDGaugeColorInMission,
	l_HUD,
	"number|string gaugeNameOrIndex",
	"Color currently used to draw the gauge",
	"number, number, number, number",
	"Red, green, blue, and alpha of the gauge")
{
	int idx = getDefaultGaugeIndex(L);

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
	HudGauge* gauge = nullptr;

	gauge = hud_get_custom_gauge(name);
	if (gauge == nullptr)
	{
		int idx = hud_get_default_gauge_index(name);
		if (idx >= 0 && idx < (int)default_hud_gauges.size())
			gauge = default_hud_gauges[idx].get();
	}

	if (gauge == nullptr)
		return ADE_RETURN_NIL;
	else
		return ade_set_args(L, "o", l_HudGauge.Set(gauge));
}

ADE_FUNC(flashTargetBox, l_HUD, "enumeration section, [number duration_in_milliseconds]", "Flashes a section of the target box with a default duration of " SCP_TOKEN_TO_STR(TBOX_FLASH_DURATION) " milliseconds", nullptr, nullptr)
{
	enum_h section;
	int num_args, duration;
	
	num_args = ade_get_args(L, "o|i", l_Enum.Get(&section), &duration);
	if (num_args == 1)
		duration = TBOX_FLASH_DURATION;
	else if (num_args != 2)
		return ADE_RETURN_NIL;

	if (duration <= 1)
		return ADE_RETURN_NIL;

	int section_index = 0;
	if (section.IsValid())
	{
		switch (section.index)
		{
			case LE_TBOX_FLASH_NAME:
				section_index = TBOX_FLASH_NAME;
				break;
			case LE_TBOX_FLASH_CARGO:
				section_index = TBOX_FLASH_CARGO;
				break;
			case LE_TBOX_FLASH_HULL:
				section_index = TBOX_FLASH_HULL;
				break;
			case LE_TBOX_FLASH_STATUS:
				section_index = TBOX_FLASH_STATUS;
				break;
			case LE_TBOX_FLASH_SUBSYS:
				section_index = TBOX_FLASH_SUBSYS;
				break;
			default:
				Warning(LOCATION, "Invalid targetbox section enumeration!");
				return ADE_RETURN_NIL;
		}
	}

	hud_targetbox_start_flash(section_index, duration);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getTargetDistance, l_HUD, "object targetee, [vector targeter_position]", "Returns the distance as displayed on the HUD, that is, the distance from a position to the bounding box of a target.  If targeter_position is nil, the function will use the player's position.", "number", "The distance, or nil if invalid")
{
	object_h *targetee_h;
	vec3d *targeter_pos = nullptr;

	if (!ade_get_args(L, "o|o", l_Object.GetPtr(&targetee_h), l_Vector.GetPtr(&targeter_pos)))
		return ADE_RETURN_NIL;

	if (targetee_h == nullptr || !targetee_h->IsValid())
		return ADE_RETURN_NIL;

	if (targeter_pos == nullptr)
	{
		if (Player_obj)
			targeter_pos = &Player_obj->pos;
		else
		{
			Warning(LOCATION, "Player_obj is NULL; cannot assign position!");
			targeter_pos = &vmd_zero_vector;
		}
	}

	auto dist = hud_find_target_distance(targetee_h->objp, targeter_pos);
	return ade_set_args(L, "f", dist);
}

ADE_FUNC(getDirectiveLines, l_HUD, nullptr, "Returns the number of lines displayed by the currently active directives", "number", "The number of lines")
{
	return ade_set_args(L, "i", Training_obj_num_display_lines);
}


}
}
