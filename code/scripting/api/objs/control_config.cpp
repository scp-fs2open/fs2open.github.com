#include "control_config.h"

#include "controlconfig/controlsconfig.h"
#include "io/key.h"

namespace scripting {
namespace api {

control_h::control_h() : control(-1) {}
control_h::control_h(int l_control) : control(l_control) {}

CCI* control_h::getControl() const
{
	return &Control_config[control];
}

conflict* control_h::getConflict() const
{
	return &Conflicts[control];
}

//**********HANDLE: control
ADE_OBJ(l_Control, control_h, "control", "Control handle");

ADE_VIRTVAR(Name, l_Control, nullptr, "The name of the control", "string", "The name")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", XSTR(current.getControl()->text.c_str(), current.getControl()->indexXSTR, true));
}

ADE_VIRTVAR(Bindings,
	l_Control,
	nullptr,
	"Gets a table of bindings for the control",
	"{ number => string ... }",
	"The keys table")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	auto table = luacpp::LuaTable::create(L);

	// Using a table so that if the number of bindings is ever increased the API
	// will only need minimal adjustments.
	table.addValue(1, current.getControl()->first.textify().c_str());
	table.addValue(2, current.getControl()->second.textify().c_str());

	return ade_set_args(L, "t", &table);
}

ADE_VIRTVAR(Inversions,
	l_Control,
	nullptr,
	"Gets a table of inversions for the control",
	"{ number => boolean ... }",
	"The inversions table. True if inverted, false otherwise.")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	auto table = luacpp::LuaTable::create(L);

	// Using a table so that if the number of bindings is ever increased the API
	// will only need minimal adjustments.
	table.addValue(1, current.getControl()->first.is_inverted());
	table.addValue(2, current.getControl()->second.is_inverted());

	return ade_set_args(L, "t", &table);
}

ADE_VIRTVAR(Shifted,
	l_Control,
	nullptr,
	"Gets a table of shifted for the control",
	"{ number => boolean ... }",
	"The shifted table. True if shifted, false otherwise.")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	auto table = luacpp::LuaTable::create(L);

	// Using a table so that if the number of bindings is ever increased the API
	// will only need minimal adjustments.
	table.addValue(1, (bool)(current.getControl()->first.get_btn() & KEY_SHIFTED));
	table.addValue(2, (bool)(current.getControl()->second.get_btn() & KEY_SHIFTED));

	return ade_set_args(L, "t", &table);
}

ADE_VIRTVAR(Alted,
	l_Control,
	nullptr,
	"Gets a table of alts for the control",
	"{ number => boolean ... }",
	"The alted table. True if alted, false otherwise.")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	auto table = luacpp::LuaTable::create(L);

	// Using a table so that if the number of bindings is ever increased the API
	// will only need minimal adjustments.
	table.addValue(1, (bool)(current.getControl()->first.get_btn() & KEY_ALTED));
	table.addValue(2, (bool)(current.getControl()->second.get_btn() & KEY_ALTED));

	return ade_set_args(L, "t", &table);
}

ADE_VIRTVAR(Tab,
	l_Control,
	nullptr,
	"The tab the control belongs in. 0 = Target Tab, 1 = Ship Tab, 2 = Weapon Tab, 3 = Computer Tab",
	"number",
	"The tab number")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getControl()->tab);
}

ADE_VIRTVAR(Disabled,
	l_Control,
	nullptr,
	"Whether or not the control is disabled and should be hidden.",
	"boolean",
	"True for disabled, false otherwise")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", current.getControl()->disabled);
}

ADE_VIRTVAR(IsAxis,
	l_Control,
	nullptr,
	"Whether or not the bound control is an axis control.",
	"boolean",
	"True for axis, false otherwise")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", current.getControl()->is_axis());
}

ADE_VIRTVAR(IsModifier,
	l_Control,
	nullptr,
	"Whether or not the bound control is a modifier.",
	"boolean",
	"True for modifier, false otherwise")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	bool mod = false;

	int k = current.getControl()->get_btn(CID_KEYBOARD);
	if ((k == KEY_LALT) || (k == KEY_RALT) || (k == KEY_LSHIFT) || (k == KEY_RSHIFT))
		mod = true;

	return ade_set_args(L, "b", mod);
}

ADE_VIRTVAR(Conflicted,
	l_Control,
	nullptr,
	"Whether or not the bound control has a conflict.",
	"boolean",
	"Returns the conflict string if true, nil otherwise")
{
	control_h current;
	if (!ade_get_args(L, "o", l_Control.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (!current.getControl()->is_axis() &&
		((current.getConflict()->first >= 0) || (current.getConflict()->second >= 0))) {
		int i = current.getConflict()->first;
		if (i < 0) {
			i = current.getConflict()->second;
		}

		SCP_string str = XSTR("Control conflicts with:", 209);
		str += " ";

		if (Control_config[i].indexXSTR > 1) {
			str += XSTR(Control_config[i].text.c_str(), Control_config[i].indexXSTR, true);
		} else if (Control_config[i].indexXSTR == 1) {
			str += XSTR(Control_config[i].text.c_str(), CONTROL_CONFIG_XSTR + i, true);
		} else {
			str += Control_config[i].text.c_str();
		}

		return ade_set_args(L, "s", str.c_str());
	}

	return ADE_RETURN_NIL;
}

} // namespace api
} // namespace scripting