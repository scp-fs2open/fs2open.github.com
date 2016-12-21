//
//

#ifndef FS2_OPEN_SEXPVAR_H
#define FS2_OPEN_SEXPVAR_H

#include "globalincs/pstypes.h"
#include "scripting/ade.h"
#include "scripting/ade_api.h"
#include <parse/sexp.h>

namespace scripting {
namespace api {

//**********HANDLE: sexpvariable
struct sexpvar_h
{
	int idx;
	char variable_name[TOKEN_LENGTH];

	sexpvar_h(){idx=-1;variable_name[0]='\0';}
	sexpvar_h(int n_idx){idx = n_idx; strcpy_s(variable_name, Sexp_variables[n_idx].variable_name);}
	bool IsValid(){
		return (idx > -1
			&& idx < MAX_SEXP_VARIABLES
			&& (Sexp_variables[idx].type & SEXP_VARIABLE_SET)
			&& !strcmp(Sexp_variables[idx].variable_name, variable_name));}
};

DECLARE_ADE_OBJ(l_SEXPVariable, sexpvar_h);

}
}

#endif //FS2_OPEN_SEXPVAR_H
