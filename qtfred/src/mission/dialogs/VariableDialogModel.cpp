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

    for (const auto& variable : _variableItems){
        
    }

}