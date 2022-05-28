//
//

#include "species.h"
#include "species_defs/species_defs.h"

extern int Species_initted;

namespace scripting {
namespace api {

//**********HANDLE: Species
ADE_OBJ(l_Species, int, "species", "Species handle");

ADE_VIRTVAR(Name, l_Species, "string", "Species name", "string", "Species name, or empty string if handle is invalid")
{
	if(!Species_initted)
		return ade_set_error(L, "s", "");

	const char* s = nullptr;
	int idx;
	if(!ade_get_args(L, "o|s", l_Species.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= (int)Species_info.size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != nullptr) {
		auto size = sizeof(Species_info[idx].species_name);
		strncpy_s(Species_info[idx].species_name, s, size-1);
	}

	return ade_set_args(L, "s", Species_info[idx].species_name);
}

ADE_FUNC(isValid, l_Species, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Species.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= (int)Species_info.size())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}


}
}
