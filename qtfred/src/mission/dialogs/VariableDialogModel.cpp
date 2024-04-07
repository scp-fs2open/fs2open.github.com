#include "VariableDialogModel.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"

namespace fso {
namespace fred {
namespace dialogs {


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
    // TODO VALIDATE!  
    // TODO!  Look for referenced varaibles and containers. 
    // We actually can't fully trust what the model says....
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

    // TODO! containers


}

// true on string, false on number
bool VariableDialogModel::getVariableType(SCP_string name)
{
    return (auto variable = lookupVariable(name)) ? (variable->string) : false;   
}

bool VariableDialogModel::getVariableNetworkStatus(SCP_string name)
{
    return (auto variable = lookupVariable(name)) ? (variable->flags & SEXP_VARIABLE_NETWORK > 0) : false;
}



// 0 neither, 1 on mission complete, 2 on mission close (higher number saves more often)
int VariableDialogModel::getVariableOnMissionCloseOrCompleteFlag(SCP_string name)
{
    return (auto variable = lookupVariable(name)) ? (variable->flags) : 0;
}


bool VariableDialogModel::getVariableEternalFlag(SCP_string name)
{
    // TODO! figure out correct value for retrieving eternal.
    return (auto variable = lookupVariable(name)) ? (variable->flags) : false;
}


SCP_string VariableDialogModel::getVariableStringValue(SCP_string name)
{
    return ((auto variable = lookupVariable(name)) && variable->string) ? (variable->stringValue) : "";
}

int VariableDialogModel::getVariableNumberValue(SCP_string name)
{
    return ((auto variable = lookupVariable(name)) && !variable->string) ? (variable->numberValue) : 0;
}



// TODO!  Need a way to clean up references.

// true on string, false on number
bool VariableDialogModel::setVariableType(SCP_string name, bool string)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable || variable->string == string){
        return string;
    }

    //TODO!  We need a way to detect the number of references, because then we could see if we need to warn
    // about references.

    // changing the type here!
    // this variable is currently a string
    if (variable->string) {
        // no risk change, because no string was specified.
        if (variable->stringValue == "") {
            variable->string = string;
            return variable->string;
        } else {
            SCP_string question;
            sprintf(&question, "Changing variable %s to number variable type will make its string value irrelevant.  Continue?", variable->name.c_st());
            SCP_string info;
            sprintf(&info, "If the string cleanly converts to an integer and a number has not previously been set for this variable, the converted number value will be retained.")
            
            // if this was a misclick, let the user say so
            if (!confirmAction(question)) {
                return variable->string;
            }

            // if there was no previous number value 
            if (variable->numberValue == 0){
                try {                    
                    variable->numberValue = std::stoi(variable->stringValue);
                }
                // nothing to do here, because that just means we can't convert.
                catch {}
            }

            return string;
        }
    
    // this variable is currently a number
    } else {
        // safe change because there was no number value specified
        if (variable->numberValue == 0){
            varaible->string = string;
            return variable->string;
        } else {
            SCP_string question;
            sprintf(&question, "Changing variable %s to a string variable type will make the number value irrelevant.  Continue?", variable->name.c_st());
            SCP_string info;
            sprintf(&info, "If no string value has been previously set for this variable, then the number value specified will be set as the default string value.")
            
            // if this was a misclick, let the user say so
            if (!confirmAction(question)) {
                return variable->string;
            }

            // if there was no previous string value 
            if (variable->stringValue == ""){
                sprintf(&variable->stringValue, "%i", variable->numberValue);
            }

            return string;
        }
    }
}

bool VariableDialogModel::setVariableNetworkStatus(SCP_string name, bool network)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable){
        return false;
    }

    // TODO! Look up setting 
    // if (network){
    // variable->flags |= LOLFLAG;
    // } else {
    // variable->flags 
    // }
    return network;
}

int VariableDialogModel::setVariableOnMissionCloseOrCompleteFlag(SCP_string name, int flags)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable){
        return 0;
    }

    // TODO! Look up setting 
    

    return flags;
}

bool VariableDialogModel::setVariableEternalFlag(SCP_string name, bool eternal)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable){
        return false;
    }

    // TODO! Look up setting 


    return eternal;
}

SCP_string VariableDialogModel::setVariableStringValue(SCP_string name, SCP_string value)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable || !variable->string){
        return "";
    }
    
    variable->stringValue = value;
}

int VariableDialogModel::setVariableNumberValue(SCP_string name, int value)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable || variable->string){
        return 0;
    }
    
    variable->numberValue = value;
}

SCP_string VariableDialogModel::addNewVariable()
{
    variableInfo* variable = nullptr;
    int count = 0;
    SCP_string name;

    do {
        name = "";
        sprintf(&name, "<Unammed_%i>", count);
        variable = lookupVariable();
        ++count;
    } while (variable != nullptr && count < 50);


    if (variable){
        return "";
    }

    _variableItems.emplace_back();
    _variableItems.back().name = name;
    return name;
}

SCP_string VariableDialogModel::changeVariableName(SCP_string oldName, SCP_string newName)
{
    if (newName == "") {
        return "";
    }
 
    auto variable = lookupVariable(oldName);

    // nothing to change, or invalid entry
    if (!variable){
        return "";
    }

    // We cannot have two variables with the same name, but we need to check this somewhere else (like on accept attempt).
    variable->name = newName;
    return newName;
}

SCP_string VariableDialogModel::copyVariable(SCP_string name)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable){
        return "";
    }

    int count = 0;
    variableInfo* variableCopy = nullptr;

    while (variableCopy == nullptr && count < 50){
        SCP_string newName;
        sprintf(&newName, "%s%i", name, count);
        variableCopy = lookupVariable(newName);
        if (!variableCopy){
            _variableItems.emplace_back();
            _variableItems.back().name = newName;
            return newName;
        }
    }
}

// returns whether it succeeded
bool VariableDialogModel::removeVariable(SCP_string name)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable){
        return false;
    }

    variable->deleted = true;
    return true;
}


// Container Section

// true on string, false on number
bool VariableDialogModel::getContainerValueType(SCP_string name)
{
    
}

// true on list, false on map
bool VariableDialogModel::getContainerListOrMap(SCP_string name)
{
    
}

bool VariableDialogModel::getContainerNetworkStatus(SCP_string name)
{
    
}

// 0 neither, 1 on mission complete, 2 on mission close (higher number saves more often)
int VariableDialogModel::getContainerOnMissionCloseOrCompleteFlag(SCP_string name)
{
    
}

bool VariableDialogModel::getContainerEternalFlag(SCP_string name)
{
    
}


bool VariableDialogModel::setContainerValueType(SCP_string name, bool type)
{
    
}

bool VariableDialogModel::setContainerListOrMap(SCP_string name, bool list)
{
    
}

bool VariableDialogModel::setContainerNetworkStatus(SCP_string name, bool network)
{
    
}

int VariableDialogModel::setContainerOnMissionCloseOrCompleteFlag(SCP_string name, int flags)
{
    
}

bool VariableDialogModel::setContainerEternalFlag(SCP_string name, bool eternal)
{
    
}

SCP_string VariableDialogModel::addContainer()
{
    
}

SCP_string VariableDialogModel::changeContainerName(SCP_string oldName, SCP_string newName)
{
    
}

bool VariableDialogModel::removeContainer(SCP_string name)
{
    
}


} // dialogs
} // fred
} // fso
