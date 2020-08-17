//
//

#include "intelentry.h"
#include "menuui/techmenu.h"

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


}
}
