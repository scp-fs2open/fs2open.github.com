//
//

#include "intelentry.h"
#include "menuui/techmenu.h"
#include "model/modelrender.h"
#include "scripting/api/objs/vecmath.h"

namespace scripting {
namespace api {


//**********HANDLE: Intelentry
ADE_OBJ(l_Intelentry, int, "intel_entry", "Intel entry handle");

ADE_FUNC(__tostring, l_Intelentry, nullptr, "Intel entry name", "string", "Intel entry name, or an empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Intelentry.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= intel_info_size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Intel_info[idx].name);
}

ADE_FUNC(__eq, l_Intelentry, "intel_entry, intel_entry", "Checks if the two entries are equal", "boolean", "true if equal, false otherwise")
{
	int idx1,idx2;
	if(!ade_get_args(L, "oo", l_Intelentry.Get(&idx1), l_Intelentry.Get(&idx2)))
		return ade_set_error(L, "b", false);

	if(idx1 < 0 || idx1 >= intel_info_size())
		return ade_set_error(L, "b", false);

	if(idx2 < 0 || idx2 >= intel_info_size())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", idx1 == idx2);
}

ADE_VIRTVAR(Name, l_Intelentry, "string", "Intel entry name", "string", "Intel entry name, or an empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Intelentry.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= intel_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != nullptr) {
		auto len = sizeof(Intel_info[idx].name);
		strncpy(Intel_info[idx].name, s, len);
		Intel_info[idx].name[len - 1] = 0;
	}

	return ade_set_args(L, "s", Intel_info[idx].name);
}

ADE_VIRTVAR(Description, l_Intelentry, "string", "Intel entry description", "string", "Description, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Intelentry.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= intel_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR) {
		if(s != nullptr) {
			Intel_info[idx].desc = s;
		} else {
			Intel_info[idx].desc = "";
		}
	}

	return ade_set_args(L, "s", Intel_info[idx].desc);
}

ADE_VIRTVAR(AnimFilename, l_Intelentry, "string", "Intel entry animation filename", "string", "Filename, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Intelentry.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= intel_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR) {
		if(s != nullptr) {
			auto len = sizeof(Intel_info[idx].anim_filename);
			strncpy(Intel_info[idx].anim_filename, s, len);
			Intel_info[idx].anim_filename[len - 1] = 0;
		} else {
			strcpy_s(Intel_info[idx].anim_filename, "");
		}
	}

	return ade_set_args(L, "s", Intel_info[idx].anim_filename);
}

ADE_VIRTVAR(InTechDatabase, l_Intelentry, "boolean", "Gets or sets whether this intel entry is visible in the tech room", "boolean", "True or false")
{
	int idx;
	bool new_value;
	if (!ade_get_args(L, "o|b", l_Intelentry.Get(&idx), &new_value))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= intel_info_size())
		return ade_set_error(L, "b", false);

	if (ADE_SETTING_VAR) {
		if (new_value) {
			Intel_info[idx].flags |= IIF_IN_TECH_DATABASE;
		} else {
			Intel_info[idx].flags &= ~IIF_IN_TECH_DATABASE;
		}
	}

	return ade_set_args(L, "b", (Intel_info[idx].flags & IIF_IN_TECH_DATABASE) != 0);
}

ADE_VIRTVAR(CustomData,
	l_Intelentry,
	nullptr,
	"Gets the custom data table for this entry",
	"table",
	"The entry's custom data table")
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Intelentry.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= intel_info_size())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	auto table = luacpp::LuaTable::create(L);

	for (const auto& pair : Intel_info[idx].custom_data) {
		table.addValue(pair.first, pair.second);
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(hasCustomData,
	l_Intelentry,
	nullptr,
	"Detects whether the entry has any custom data",
	"boolean",
	"true if the entry's custom_data is not empty, false otherwise")
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Intelentry.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= intel_info_size())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	bool result = !Intel_info[idx].custom_data.empty();
	return ade_set_args(L, "b", result);
}

ADE_FUNC(isValid, l_Intelentry, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Intelentry.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= intel_info_size())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getIntelEntryIndex, l_Intelentry, nullptr, "Gets the index value of the intel entry", "number", "index value of the intel entry")
{
	int idx;
	if(!ade_get_args(L, "o", l_Intelentry.Get(&idx)))
		return ade_set_args(L, "i", -1);

	if(idx < 0 || idx >= intel_info_size())
		return ade_set_args(L, "i", -1);

	return ade_set_args(L, "i", idx + 1); // Lua is 1-based
}

ADE_FUNC(renderTechModel,
	l_Intelentry,
	"number X1, number Y1, number X2, number Y2, [number RotationPercent =40, number PitchPercent =0, number "
	"BankPercent=0, number Zoom=1.3, boolean Lighting=true]",
	"Draws the intel entry's tech model. True for regular lighting, false for flat lighting. Returns false if the intel entry has no tech model defined.",
	"boolean",
	"Whether the intel entry was rendered")
{
	int x1, y1, x2, y2;
	angles rot_angles = {0.0f, 0.0f, 40.0f};
	int idx;
	float zoom = 1.3f;
	bool lighting = true;
	if (!ade_get_args(L, "oiiii|ffffb", l_Intelentry.Get(&idx), &x1, &y1, &x2, &y2, &rot_angles.h, &rot_angles.p, &rot_angles.b, &zoom, &lighting))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= intel_info_size())
		return ade_set_args(L, "b", false);

	if (x2 < x1 || y2 < y1)
		return ade_set_args(L, "b", false);

	intel_data* iip = &Intel_info[idx];

	if (!VALID_FNAME(iip->tech_model))
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

	return ade_set_args(L, "b", render_tech_model(TECH_POF, x1, y1, x2, y2, zoom, lighting, -1, &orient, iip->tech_model, iip->closeup_zoom, &iip->closeup_pos));
}

// Nuke's alternate tech model rendering function
ADE_FUNC(renderTechModel2,
	l_Intelentry,
	"number X1, number Y1, number X2, number Y2, [orientation Orientation=nil, number Zoom=1.3]",
	"Draws the intel entry's tech model. Returns false if the intel entry has no tech model defined.",
	"boolean",
	"Whether the intel entry was rendered")
{
	int x1, y1, x2, y2;
	int idx;
	float zoom = 1.3f;
	matrix_h* mh = nullptr;
	if (!ade_get_args(L, "oiiii|of", l_Intelentry.Get(&idx), &x1, &y1, &x2, &y2, l_Matrix.GetPtr(&mh), &zoom))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= intel_info_size())
		return ade_set_args(L, "b", false);

	if (x2 < x1 || y2 < y1)
		return ade_set_args(L, "b", false);

	intel_data* iip = &Intel_info[idx];

	if (!VALID_FNAME(iip->tech_model))
		return ade_set_args(L, "b", false);

	matrix identity_orient = vmd_identity_matrix;
	matrix* orient = mh != nullptr ? mh->GetMatrix() : &identity_orient;

	return ade_set_args(L, "b", render_tech_model(TECH_POF, x1, y1, x2, y2, zoom, true, -1, orient, iip->tech_model, iip->closeup_zoom, &iip->closeup_pos));
}


}
}
