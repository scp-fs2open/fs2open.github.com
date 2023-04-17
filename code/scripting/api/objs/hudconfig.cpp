#include "hudconfig.h"

#include "hud/hudconfig.h"
#include "ui/ui.h"
#include "scripting/api/objs/color.h"
#include "iff_defs/iff_defs.h"
#include "globalincs/alphacolors.h"

namespace scripting {
namespace api {

gauge_config_h::gauge_config_h() : gauge(-1) {}
gauge_config_h::gauge_config_h(int l_gauge) : gauge(l_gauge) {}

HC_gauge_region* gauge_config_h::getGauge() const
{
	if (!isValid()) {
		return nullptr;
	}

	return &HC_gauge_regions[GR_1024][gauge];
}

int gauge_config_h::getIndex() const
{
	return gauge;
}

const char* gauge_config_h::getName() const
{
	if (!isValid()) {
		return nullptr;
	}

	return HC_gauge_descriptions(gauge);
}

bool gauge_config_h::isValid() const
{
	return gauge >= 0 && gauge < NUM_HUD_GAUGES;
}

hud_preset_h::hud_preset_h() : preset(-1) {}
hud_preset_h::hud_preset_h(int l_preset) : preset(l_preset) {}

int hud_preset_h::getIndex() const
{
	return preset;
}

SCP_string hud_preset_h::getName() const
{
	if (!isValid()) {
		return "";
	}

	return HC_preset_filenames[preset];
}

bool hud_preset_h::isValid() const
{
	return preset >= 0 && preset < (int)HC_preset_filenames.size();
}

//**********HANDLE: hud preset
ADE_OBJ(l_HUD_Preset, hud_preset_h, "hud_preset", "Hud preset handle");

ADE_VIRTVAR(Name, l_HUD_Preset, nullptr, "The name of this preset", "string", "The name")
{
	hud_preset_h current;
	if (!ade_get_args(L, "o", l_HUD_Preset.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getName().c_str());
}

ADE_FUNC(deletePreset, l_HUD_Preset, nullptr, "Deletes the preset file", nullptr, nullptr)
{
	hud_preset_h current;
	if (!ade_get_args(L, "o", l_HUD_Preset.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	hud_config_delete_preset(current.getName());

	return ADE_RETURN_NIL;
}

ADE_FUNC(isValid, l_HUD_Preset, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	hud_preset_h current;
	if (!ade_get_args(L, "o", l_HUD_Preset.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

//**********HANDLE: gauge config
ADE_OBJ(l_Gauge_Config, gauge_config_h, "gauge_config", "Gauge config handle");

ADE_VIRTVAR(Name, l_Gauge_Config, nullptr, "The name of this gauge", "string", "The name")
{
	gauge_config_h current;
	if (!ade_get_args(L, "o", l_Gauge_Config.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getName());
}

ADE_VIRTVAR(CurrentColor,
	l_Gauge_Config,
	"color",
	"Gets the current color of the gauge. If setting the color, gauges that use IFF for color cannot be set.",
	"color",
	"The gauge color or nil if the gauge is invalid")
{
	gauge_config_h current;
	color newColor;
	if (!ade_get_args(L, "o|o", l_Gauge_Config.Get(&current), l_Color.Get(&newColor))) {
		return ADE_RETURN_NIL;
	}

	if (!current.isValid()) {
		return ade_set_error(L, "o", l_Color.Set(Color_text_normal));
	}

	if (ADE_SETTING_VAR) {
		if (!current.getGauge()->use_iff) {
			HUD_config.clr[current.getIndex()] = newColor;
		}
	}

	const color *thisColor;
	
	if (!current.getGauge()->use_iff) {
		thisColor = &HUD_config.clr[current.getIndex()];
	} else {
		if (current.getGauge()->color == 1) {
			thisColor = iff_get_color(IFF_COLOR_TAGGED, 0);
		} else {
			thisColor = &Color_bright_red;
		}
	}

	return ade_set_args(L, "o", l_Color.Set(*thisColor));
}

ADE_VIRTVAR(ShowGaugeFlag,
	l_Gauge_Config,
	"boolean Show",
	"Gets the current status of the show gauge flag.",
	"boolean",
	"True if on, false if otherwise")
{
	gauge_config_h current;
	bool show;
	if (!ade_get_args(L, "o|b", l_Gauge_Config.Get(&current), &show)) {
		return ADE_RETURN_NIL;
	}

	if (!current.isValid()) {
		return ade_set_error(L, "b", false);
	}

	if (ADE_SETTING_VAR) {
		if (show) {
			hud_config_show_flag_set(current.getIndex());
		} else {
			hud_config_show_flag_clear(current.getIndex());
		}
	}

	return ade_set_args(L, "b", (bool)hud_config_show_flag_is_set(current.getIndex()));
}

ADE_VIRTVAR(PopupGaugeFlag,
	l_Gauge_Config,
	"boolean Popup",
	"Gets the current status of the popup gauge flag.",
	"boolean",
	"True if on, false otherwise")
{
	gauge_config_h current;
	bool popup;
	if (!ade_get_args(L, "o|b", l_Gauge_Config.Get(&current), &popup)) {
		return ADE_RETURN_NIL;
	}

	if (!current.isValid()) {
		return ade_set_error(L, "b", false);
	}

	if (ADE_SETTING_VAR) {
		if (popup) {
			hud_config_popup_flag_set(current.getIndex());
		} else {
			hud_config_popup_flag_clear(current.getIndex());
		}
	}

	if (current.getGauge()->can_popup == 0) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", (bool)hud_config_popup_flag_is_set(current.getIndex()));
}

ADE_VIRTVAR(CanPopup,
	l_Gauge_Config,
	nullptr,
	"Gets whether or not the gauge can have the popup flag.",
	"boolean",
	"True if can popup, false otherwise")
{
	gauge_config_h current;
	if (!ade_get_args(L, "o", l_Gauge_Config.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (!current.isValid()) {
		return ade_set_error(L, "b", false);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getGauge()->can_popup == 0) {
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(UsesIffForColor,
	l_Gauge_Config,
	nullptr,
	"Gets whether or not the gauge uses IFF for color.",
	"boolean",
	"True if uses IFF, false otherwise")
{
	gauge_config_h current;
	if (!ade_get_args(L, "o", l_Gauge_Config.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (!current.isValid()) {
		return ade_set_error(L, "b", false);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (current.getGauge()->use_iff == 0) {
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setSelected, l_Gauge_Config, "boolean", "Sets if the gauge is the currently selected gauge for drawing as selected.", nullptr, nullptr)
{
	gauge_config_h current;
	bool select;
	if (!ade_get_args(L, "o|b", l_Gauge_Config.Get(&current), &select)) {
		return ADE_RETURN_NIL;
	}

	if (!current.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (select) {
		HC_gauge_selected = current.getIndex();
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(isValid, l_Gauge_Config, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	gauge_config_h current;
	if (!ade_get_args(L, "o", l_Gauge_Config.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

} // namespace api
} // namespace scripting