#include "VariableDialogModel.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"


VariableDialogModel::VariableDialogModel(QObject* parent, EditorViewport* viewport) 
		: AbstractDialogModel(parent, viewport)
{
		initializeData();
}

void VariableDialogModel::reject() 
{
    _variableItems.clear();
    _containerItems.clear();
}

void VariableDialogModel::apply() 
{
    memset(Sexp_variables, 0, MAX_SEXP_VARIABLES * size_of(sexp_variable));

    for (int i = 0; i < static_cast<int>(_variableItems.size()); ++i){
        Sexp_variables[i].type = _variableItems[i].flags;
        strcpy_s(Sexp_variables[i].variable_name, _variableItems[i].name.c_str());
        
        if (_variableItems[i].flags & SEXP_VARIABLE_STRING){
            strcpy_s(Sexp_variables[i].text, _variableItems[i].stringValue);
        } else {
            strcpy_s(Sexp_variables[i].text, std::to_string(_variableItems[i].numberValue).c_str())
        }
    }

    

}