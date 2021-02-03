//
//

#include "sexpvar.h"
#include "enums.h"

namespace scripting {
namespace api {

ADE_OBJ(l_SEXPVariable, sexpvar_h, "sexpvariable", "SEXP Variable handle");

ADE_VIRTVAR(Name, l_SEXPVariable, "string", "SEXP Variable name.", "string", "SEXP Variable name, or empty string if handle is invalid")
{
	sexpvar_h *svh = nullptr;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_SEXPVariable.GetPtr(&svh), &s))
		return ade_set_error(L, "s", "");

	if (!svh->IsValid())
		return ade_set_error(L, "s", "");

	sexp_variable *sv = &Sexp_variables[svh->idx];

	if (ADE_SETTING_VAR && s != nullptr) {
		LuaError(L, "Reassigning SEXP variable names is not supported.  (Tried to rename %s to %s.)", sv->variable_name, s);
	}

	return ade_set_args(L, "s", sv->variable_name);
}

ADE_VIRTVAR(Persistence, l_SEXPVariable, "enumeration", "SEXP Variable persistance, uses SEXPVAR_*_PERSISTENT enumerations", "enumeration", "SEXPVAR_*_PERSISTENT enumeration, or invalid numeration if handle is invalid")
{
	sexpvar_h *svh = NULL;
	enum_h *type = NULL;
	if(!ade_get_args(L, "o|o", l_SEXPVariable.GetPtr(&svh), l_Enum.GetPtr(&type)))
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	if(!svh->IsValid())
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	sexp_variable *sv = &Sexp_variables[svh->idx];

	if(ADE_SETTING_VAR && type != NULL)
	{
		if(type->index == LE_SEXPVAR_PLAYER_PERSISTENT)
		{
			sv->type &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS);
			sv->type |= SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE;
		}
		else if(type->index == LE_SEXPVAR_CAMPAIGN_PERSISTENT)
		{
			sv->type |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
			sv->type &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
		}
		else if(type->index == LE_SEXPVAR_NOT_PERSISTENT)
		{
			sv->type &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS);
			sv->type &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
		}
	}

	enum_h ren;
	if(sv->type & SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE)
		ren.index = LE_SEXPVAR_PLAYER_PERSISTENT;
	else if(sv->type & SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS)
		ren.index = LE_SEXPVAR_CAMPAIGN_PERSISTENT;
	else
		ren.index = LE_SEXPVAR_NOT_PERSISTENT;

	return ade_set_args(L, "o", l_Enum.Set(ren));
}

ADE_VIRTVAR(Type, l_SEXPVariable, "enumeration", "SEXP Variable type, uses SEXPVAR_TYPE_* enumerations", "enumeration", "SEXPVAR_TYPE_* enumeration, or invalid numeration if handle is invalid")
{
	sexpvar_h *svh = NULL;
	enum_h *type = NULL;
	if(!ade_get_args(L, "o|o", l_SEXPVariable.GetPtr(&svh), l_Enum.GetPtr(&type)))
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	if(!svh->IsValid())
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	sexp_variable *sv = &Sexp_variables[svh->idx];

	if(ADE_SETTING_VAR && type != NULL)
	{
		if(type->index == LE_SEXPVAR_TYPE_NUMBER)
		{
			sv->type &= ~(SEXP_VARIABLE_NUMBER);
			sv->type |= SEXP_VARIABLE_STRING;
		}
		else if(type->index == LE_SEXPVAR_TYPE_STRING)
		{
			sv->type |= SEXP_VARIABLE_NUMBER;
			sv->type &= ~(SEXP_VARIABLE_STRING);
		}
	}

	enum_h ren;
	if(sv->type & SEXP_VARIABLE_NUMBER)
		ren.index = LE_SEXPVAR_TYPE_NUMBER;
	else if(sv->type & SEXP_VARIABLE_STRING)
		ren.index = LE_SEXPVAR_TYPE_STRING;

	return ade_set_args(L, "o", l_Enum.Set(ren));
}

ADE_VIRTVAR(Value, l_SEXPVariable, "number/string", "SEXP variable value", "string", "SEXP variable contents, or nil if the variable is of an invalid type or the handle is invalid")
{
	sexpvar_h *svh = NULL;
	const char* newvalue = nullptr;
	char number_as_str[TOKEN_LENGTH];

	if(lua_type(L, 2) == LUA_TNUMBER)
	{
		int newnumber = 0;
		if(!ade_get_args(L, "o|i", l_SEXPVariable.GetPtr(&svh), &newnumber))
			return ADE_RETURN_NIL;

		sprintf(number_as_str, "%d", newnumber);
		newvalue = number_as_str;
	}
	else
	{
		if(!ade_get_args(L, "o|s", l_SEXPVariable.GetPtr(&svh), &newvalue))
			return ADE_RETURN_NIL;
	}

	if(!svh->IsValid())
		return ADE_RETURN_NIL;

	sexp_variable *sv = &Sexp_variables[svh->idx];

	if(ADE_SETTING_VAR && newvalue)
	{
		sexp_modify_variable(newvalue, svh->idx, false);
	}

	if(sv->type & SEXP_VARIABLE_NUMBER)
		return ade_set_args(L, "i", atoi(sv->text));
	else if(sv->type & SEXP_VARIABLE_STRING)
		return ade_set_args(L, "s", sv->text);
	else
		return ADE_RETURN_NIL;
}

ADE_FUNC(__tostring, l_SEXPVariable, NULL, "Returns SEXP name", "string", "SEXP name, or empty string if handle is invalid")
{
	sexpvar_h *svh = NULL;
	if(!ade_get_args(L, "o", l_SEXPVariable.GetPtr(&svh)))
		return ade_set_error(L, "s", "");

	if(!svh->IsValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Sexp_variables[svh->idx].variable_name);
}

ADE_FUNC(isValid, l_SEXPVariable, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	sexpvar_h *svh = NULL;
	if(!ade_get_args(L, "o", l_SEXPVariable.GetPtr(&svh)))
		return ADE_RETURN_NIL;

	if(!svh->IsValid())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(delete, l_SEXPVariable, NULL, "Deletes a SEXP Variable", "boolean", "True if successful, false if the handle is invalid")
{
	sexpvar_h *svh = NULL;
	if(!ade_get_args(L, "o", l_SEXPVariable.GetPtr(&svh)))
		return ade_set_error(L, "b", false);

	if(!svh->IsValid())
		return ade_set_error(L, "b", false);

	sexp_variable_delete(svh->idx);

	return ADE_RETURN_TRUE;
}

}
}

