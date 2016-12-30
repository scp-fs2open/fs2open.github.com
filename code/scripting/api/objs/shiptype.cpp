//
//

#include "shiptype.h"
#include "ship/ship.h"

extern int Species_initted;

namespace scripting {
namespace api {

//**********HANDLE: Shiptype
ADE_OBJ(l_Shiptype, int, "shiptype", "Ship type handle");

ADE_VIRTVAR(Name, l_Shiptype, "string", "Ship type name", "string", "Ship type name, or empty string if handle is invalid")
{
	if(!Species_initted)
		return ade_set_error(L, "s", "");

	char *s = NULL;
	int idx;
	if(!ade_get_args(L, "o|s", l_Shiptype.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= (int)Ship_types.size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != NULL) {
		strncpy(Ship_types[idx].name, s, sizeof(Ship_types[idx].name)-1);
	}

	return ade_set_args(L, "s", Ship_types[idx].name);
}

ADE_FUNC(isValid, l_Shiptype, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Shiptype.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= (int)Ship_types.size())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

}
}
