#include "propclass.h"
#include "model.h"
#include "vecmath.h"
#include "prop/prop.h"

namespace scripting {
namespace api {

//**********HANDLE: Propclass
ADE_OBJ(l_Propclass, int, "propclass", "Prop class handle");

ADE_FUNC(__tostring,
	l_Propclass,
	NULL,
	"Prop class name",
	"string",
	"Prop class name, or an empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Propclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= prop_info_size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Prop_info[idx].name);
}

ADE_FUNC(__eq,
	l_Propclass,
	"propclass, propclass",
	"Checks if the two classes are equal",
	"boolean",
	"true if equal, false otherwise")
{
	int idx1, idx2;
	if (!ade_get_args(L, "oo", l_Propclass.Get(&idx1), l_Propclass.Get(&idx2)))
		return ade_set_error(L, "b", false);

	if (idx1 < 0 || idx1 >= prop_info_size())
		return ade_set_error(L, "b", false);

	if (idx2 < 0 || idx2 >= prop_info_size())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", idx1 == idx2);
}

ADE_VIRTVAR(Name,
	l_Propclass,
	"string",
	"Prop class name",
	"string",
	"Prop class name, or an empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Propclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= prop_info_size())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting prop class name is not supported");
	}

	return ade_set_args(L, "s", Prop_info[idx].name);
}

ADE_VIRTVAR(Model,
	l_Propclass,
	"model",
	"Model",
	"model",
	"Prop class model, or invalid model handle if propclass handle is invalid")
{
	int idx = -1;
	model_h* mdl = NULL;
	if (!ade_get_args(L, "o|o", l_Propclass.Get(&idx), l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	if (idx < 0 || idx >= prop_info_size())
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	prop_info* pip = &Prop_info[idx];

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting prop class model is not supported");
	}

	return ade_set_args(L, "o", l_Model.Set(model_h(pip->model_num)));
}

ADE_VIRTVAR(CustomData,
	l_Propclass,
	nullptr,
	"Gets the custom data table for this prop class",
	"table",
	"The prop class's custom data table")
{
	int idx;
	if (!ade_get_args(L, "o", l_Propclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= prop_info_size())
		return ADE_RETURN_NIL;

	auto table = luacpp::LuaTable::create(L);

	prop_info* pip = &Prop_info[idx];

	for (const auto& pair : pip->custom_data) {
		table.addValue(pair.first, pair.second);
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(hasCustomData,
	l_Propclass,
	nullptr,
	"Detects whether the prop class has any custom data",
	"boolean",
	"true if the propclass's custom_data is not empty, false otherwise")
{
	int idx;
	if (!ade_get_args(L, "o", l_Propclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= prop_info_size())
		return ADE_RETURN_NIL;

	prop_info* pip = &Prop_info[idx];

	bool result = !pip->custom_data.empty();
	return ade_set_args(L, "b", result);
}

ADE_VIRTVAR(CustomStrings,
	l_Propclass,
	nullptr,
	"Gets the indexed custom string table for this prop. Each item in the table is a table with the following values: "
	"Name - the name of the custom string, Value - the value associated with the custom string, String - the custom "
	"string itself.",
	"table",
	"The prop's custom data table")
{
	int idx;
	if (!ade_get_args(L, "o", l_Propclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= prop_info_size())
		return ADE_RETURN_NIL;

	prop_info* pip = &Prop_info[idx];

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting Custom Data is not supported");
	}

	auto table = luacpp::LuaTable::create(L);

	int cnt = 0;

	for (const auto& cs : pip->custom_strings) {
		cnt++;
		auto item = luacpp::LuaTable::create(L);

		item.addValue("Name", luacpp::LuaValue::createValue(Script_system.GetLuaSession(), cs.name));
		item.addValue("Value", luacpp::LuaValue::createValue(Script_system.GetLuaSession(), cs.value));
		item.addValue("String", luacpp::LuaValue::createValue(Script_system.GetLuaSession(), cs.text));

		table.addValue(cnt, item);
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(hasCustomStrings,
	l_Propclass,
	nullptr,
	"Detects whether the prop has any custom strings",
	"boolean",
	"true if the prop's custom_strings is not empty, false otherwise")
{
	int idx;
	if (!ade_get_args(L, "o", l_Propclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= prop_info_size())
		return ADE_RETURN_NIL;

	prop_info* pip = &Prop_info[idx];

	bool result = !pip->custom_strings.empty();
	return ade_set_args(L, "b", result);
}

ADE_FUNC(isValid,
	l_Propclass,
	NULL,
	"Detects whether handle is valid",
	"boolean",
	"true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if (!ade_get_args(L, "o", l_Propclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= prop_info_size())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(renderTechModel,
	l_Propclass,
	"number X1, number Y1, number X2, number Y2, [number RotationPercent =0, number PitchPercent =0, number "
	"BankPercent=40, number Zoom=1.3, boolean Lighting=true]",
	"Draws prop model as if in techroom. True for regular lighting, false for flat lighting.",
	"boolean",
	"Whether prop was rendered")
{
	int x1, y1, x2, y2;
	angles rot_angles = {0.0f, 0.0f, 40.0f};
	int idx;
	float zoom = 1.3f;
	bool lighting = true;
	if (!ade_get_args(L,
			"oiiii|ffffb",
			l_Propclass.Get(&idx),
			&x1,
			&y1,
			&x2,
			&y2,
			&rot_angles.h,
			&rot_angles.p,
			&rot_angles.b,
			&zoom,
			&lighting))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= ship_info_size())
		return ade_set_args(L, "b", false);

	if (x2 < x1 || y2 < y1)
		return ade_set_args(L, "b", false);

	CLAMP(rot_angles.p, 0.0f, 100.0f);
	CLAMP(rot_angles.b, 0.0f, 100.0f);
	CLAMP(rot_angles.h, 0.0f, 100.0f);

	// Handle angles
	matrix orient = vmd_identity_matrix;
	angles view_angles = {-0.6f, 0.0f, 0.0f};
	vm_angles_2_matrix(&orient, &view_angles);

	rot_angles.p = (rot_angles.p * 0.01f) * PI2;
	rot_angles.b = (rot_angles.b * 0.01f) * PI2;
	rot_angles.h = (rot_angles.h * 0.01f) * PI2;
	vm_rotate_matrix_by_angles(&orient, &rot_angles);

	return ade_set_args(L, "b", render_tech_model(TECH_PROP, x1, y1, x2, y2, zoom, lighting, idx, &orient));
}

// Nuke's alternate tech model rendering function
ADE_FUNC(renderTechModel2,
	l_Propclass,
	"number X1, number Y1, number X2, number Y2, [orientation Orientation=nil, number Zoom=1.3]",
	"Draws prop model as if in techroom",
	"boolean",
	"Whether prop was rendered")
{
	int x1, y1, x2, y2;
	int idx;
	float zoom = 1.3f;
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "oiiiio|f", l_Propclass.Get(&idx), &x1, &y1, &x2, &y2, l_Matrix.GetPtr(&mh), &zoom))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= ship_info_size())
		return ade_set_args(L, "b", false);

	if (x2 < x1 || y2 < y1)
		return ade_set_args(L, "b", false);

	// Handle angles
	matrix* orient = mh->GetMatrix();

	return ade_set_args(L, "b", render_tech_model(TECH_PROP, x1, y1, x2, y2, zoom, true, idx, orient));
}

ADE_FUNC(isModelLoaded,
	l_Propclass,
	"[boolean Load = false]",
	"Checks if the model used for this propclass is loaded or not and optionally loads the model, which might be a "
	"slow operation.",
	"boolean",
	"If the model is loaded or not")
{
	int idx;
	bool load_check = false;
	if (!ade_get_args(L, "o|b", l_Propclass.Get(&idx), &load_check))
		return ADE_RETURN_FALSE;

	prop_info* pip = &Prop_info[idx];

	if (pip == nullptr)
		return ADE_RETURN_FALSE;

	if (load_check) {
		pip->model_num = model_load(pip->pof_file);
	}

	if (pip->model_num > -1)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(getPropClassIndex,
	l_Propclass,
	nullptr,
	"Gets the index value of the prop class",
	"number",
	"index value of the prop class")
{
	int idx;
	if (!ade_get_args(L, "o", l_Propclass.Get(&idx)))
		return ade_set_args(L, "i", -1);

	if (idx < 0 || idx >= prop_info_size())
		return ade_set_args(L, "i", -1);

	return ade_set_args(L, "i", idx + 1); // Lua is 1-based
}

} // namespace api
} // namespace scripting
