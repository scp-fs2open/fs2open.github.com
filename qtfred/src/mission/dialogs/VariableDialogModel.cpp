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
    // TODO!  Look for referenced variables and containers. 
    // Need a way to clean up references.  I'm thinking making some pop ups to confirm replacements created in the editor.
    // This means we need a way to count and replace references. 

    // So, previously, I was just obliterating and overwriting, but I need to rethink this info.
    /*memset(Sexp_variables, 0, MAX_SEXP_VARIABLES * size_of(sexp_variable));

    for (int i = 0; i < static_cast<int>(_variableItems.size()); ++i){
        Sexp_variables[i].type = _variableItems[i].flags;
        strcpy_s(Sexp_variables[i].variable_name, _variableItems[i].name.c_str());
        
        if (_variableItems[i].flags & SEXP_VARIABLE_STRING){
            strcpy_s(Sexp_variables[i].text, _variableItems[i].stringValue);
        } else {
            strcpy_s(Sexp_variables[i].text, std::to_string(_variableItems[i].numberValue).c_str())
        }
    }
    */

    // TODO! containers


}

// true on string, false on number
bool VariableDialogModel::getVariableType(SCP_string name)
{
    return (auto variable = lookupVariable(name)) ? (variable->string) : true;   
}

bool VariableDialogModel::getVariableNetworkStatus(SCP_string name)
{
    return (auto variable = lookupVariable(name)) ? (variable->flags & SEXP_VARIABLE_NETWORK > 0) : false;
}



// 0 neither, 1 on mission complete, 2 on mission close (higher number saves more often)
int VariableDialogModel::getVariableOnMissionCloseOrCompleteFlag(SCP_string name)
{
    auto variable = lookupVariable(name);

    if (!variable) {
        return 0;
    }

    int returnValue = 0;

    if (variable->flags & SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE)
        returnValue = 2;
    else if (variable->flags & SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS)
        returnValue = 1;

    return returnValue;
}


bool VariableDialogModel::getVariableEternalFlag(SCP_string name)
{
    return (auto variable = lookupVariable(name)) ? (variable->flags & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE > 0) : false;
}


SCP_string VariableDialogModel::getVariableStringValue(SCP_string name)
{
    return ((auto variable = lookupVariable(name)) && variable->string) ? (variable->stringValue) : "";
}

int VariableDialogModel::getVariableNumberValue(SCP_string name)
{
    return ((auto variable = lookupVariable(name)) && !variable->string) ? (variable->numberValue) : 0;
}



// true on string, false on number
bool VariableDialogModel::setVariableType(SCP_string name, bool string)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable || variable->string == string){
        return string;
    }


    // Here we change the variable type!
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
            variable->string = string;
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

    if (!(variable->flags & SEXP_VARIABLE_NETWORK) && network){
        variable->flags |= SEXP_VARIABLE_NETWORK;
    } else {
        variable->flags &= ~SEXP_VARIABLE_NETWORK; 
    }
    return network;
}

int VariableDialogModel::setVariableOnMissionCloseOrCompleteFlag(SCP_string name, int flags)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable || flags < 0 || flags > 2){
        return 0;
    }

    if (flags == 0) {
        variable->flags &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS | SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
    } else if (flags == 1) {
        variable->flags &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
        variable->flags |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
    } else {
        variable->flags |= (SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS | SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
    }
    
    return flags;
}

bool VariableDialogModel::setVariableEternalFlag(SCP_string name, bool eternal)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable){
        return false;
    }

    if (eternal) {
        variable->flags |= SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
    } else {
        variable->flags &= ~SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
    }

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
    int count = 1;
    SCP_string name;

    do {
        name = "";
        sprintf(&name, "<unnamed_%i>", count);
        variable = lookupVariable(name);
        ++count;
    } while (variable != nullptr && count < 51);


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

    int count = 1;
    variableInfo* variableSearch;

    do {
        SCP_string newName;
        sprintf(&newName, "%s_copy%i", name, count);
        variableSearch = lookupVariable(newName);

        // open slot found!
        if (!variableSearch){
            // create the new entry in the model
            _variableItems.emplace_back();

            // and set everything as a copy from the original, except original name and deleted.
            auto& newVariable = _variableItems.back();
            newVariable.name = newName;
            newVariable.flags = variable.flags;
            newVariable.string = variable.string;

            if (newVariable.string) {
                newVariable.stringValue = variable.stringValue;
            } else {
                newVariable.numberValue = variable.numberValue;
            }

            return newName;
        }
    } while (variableSearch != nullptr && count < 51);

    return "";
}

// returns whether it succeeded
bool VariableDialogModel::removeVariable(SCP_string name)
{
    auto variable = lookupVariable(name);

    // nothing to change, or invalid entry
    if (!variable){
        return false;
    }

    SCP_string question = "Are you sure you want to delete this variable? Any references to it will have to be replaced."
    SCP_string info = "";
    if (!confirmAction(question, info)){
        return false;
    }

    variable->deleted = true;
    return true;
}


// Container Section

// true on string, false on number
bool VariableDialogModel::getContainerValueType(SCP_string name)
{
    return (auto container = lookupContainer(name)) ? container->string : true;
}

// true on list, false on map
bool VariableDialogModel::getContainerListOrMap(SCP_string name)
{
    return (auto contaner = lookupContainer(name)) ? container->list : true;
}

bool VariableDialogModel::getContainerNetworkStatus(SCP_string name)
{
    return (auto container = lookupContainer(name)) ? (container->flags & SEXP_VARIABLE_NETWORK > 0) : false;    
}

// 0 neither, 1 on mission complete, 2 on mission close (higher number saves more often)
int VariableDialogModel::getContainerOnMissionCloseOrCompleteFlag(SCP_string name)
{
    auto container = lookupContainer(name);

    if (!container) {
        return 0;
    }

    if (container->flags & SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE)
        return 2;
    else if (container->flags & SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS)
        return 1;
    else 
        return 0;
}

bool VariableDialogModel::getContainerEternalFlag(SCP_string name)
{
    return (auto container = lookupContainer(name)) ? (container->flags & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE > 0) : false;    
}


bool VariableDialogModel::setContainerValueType(SCP_string name, bool type)
{
    auto container = lookupContainer(name);

    if (!container){
        return true;
    }

    if (container->string == type){
        return container->string;
    }

    if ((container->string && container->stringValues.empty()) || (!container->string && container->numberValues.empty())){
        container->string = type;
        return container->type;
    }

    // if the other list is not empty, then just convert.  No need to confirm.  
    // The values will be there if they decide to switch back.
    if (container->string && !container->numberValues.empty()){

        container->string = type;
        return container->string;

    } else if (!container->string && !container->stringValues.empty()){
        
        container->string = type;
        return container->string;
    }

    // so when the other list *is* empty, then we can attempt to copy values.
    if (container->string && container->numberValues.empty()){
        
        SCP_string question = "Do you want to attempt conversion of these string values to number values?";
        SCP_string info = "Your string values will still be there if you convert this container back to string type.";

        if (confirmAction(question, info)) {

            bool transferable = true;
            SCP_vector<int> numbers;

            for (const auto& item : container->stringValues){
                try {
                    numbers.push_back(stoi(item));
                }
                catch {
                    transferable = false;
                    break;
                }
            }

            if (transferable){
                container->numberValues = std::move(numbers);
            }
        }

        // now that we've handled value conversion, convert the container
        container->string = type;
        return container->string;
    } else if (!container->string && container->stringValues.empty()){

        SCP_string question = "Do you want to convert these number values to string values?";
        SCP_string info = "Your number values will still be there if you convert this container back to number type.";

        if (confirmAction(question, info)) {
            for (const auto& item : container->numberValues){
                container->stringValues.emplace_back(item);
            }
        }

        // now that we've handled value conversion, convert the container
        container->string = type;
        return container->string;
    }

    // we shouldn't get here, but if we do return the current value because that's what the model thinks, anyway.
    return container->string;
}

// This is the most complicated function, because we need to query the user on what they want to do if the had already entered data. 
bool VariableDialogModel::setContainerListOrMap(SCP_string name, bool list)
{
    
}

bool VariableDialogModel::setContainerNetworkStatus(SCP_string name, bool network)
{
    auto container = lookupContainer(name);

    // nothing to change, or invalid entry
    if (!container){
        return false;
    }

    if (network) {
        container->flags |= SEXP_VARIABLE_NETWORK;
    } else {
        container->flags &= ~SEXP_VARIABLE_NETWORK;
    }

    return network;
}

int VariableDialogModel::setContainerOnMissionCloseOrCompleteFlag(SCP_string name, int flags)
{
    auto container = lookupContainer(name);

    // nothing to change, or invalid entry
    if (!container || flags < 0 || flags > 2){
        return 0;
    }

    if (flags == 0) {
        container->flags &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS | SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
    } else if (flags == 1) {
        container->flags &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
        container->flags |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
    } else {
        container->flags |= (SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS | SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
    }
    
    return flags;
}

bool VariableDialogModel::setContainerEternalFlag(SCP_string name, bool eternal)
{
    auto container = lookupContainer(name);

    // nothing to change, or invalid entry
    if (!container){
        return false;
    }

    if (eternal) {
        container->flags |= SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
    } else {
        container->flags &= ~SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
    }

    return eternal;
}

SCP_string VariableDialogModel::addContainer()
{
    containerInfo* container = nullptr;
    int count = 1;
    SCP_string name;

    do {
        name = "";
        sprintf(&name, "<unnamed_%i>", count);
        container = lookupContainer(name);
        ++count;
    } while (container != nullptr && count < 51);

    if (container){
        return "";
    }

    _containerItems.emplace_back();
    _containerItems.back().name = name;
    return name;
}

SCP_string VariableDialogModel::changeContainerName(SCP_string oldName, SCP_string newName)
{
    if (newName == "") {
        return "";
    }
 
    auto container = lookupContainer(oldName);

    // nothing to change, or invalid entry
    if (!container){
        return "";
    }

    // We cannot have two containers with the same name, but we need to check this somewhere else (like on accept attempt).
    container->name = newName;
    return newName;
}

bool VariableDialogModel::removeContainer(SCP_string name)
{
    auto container = lookupContainer(oldName);

    if (!container){
        return false;
    }

    container->deleted = true;
}

SCP_string VariableDialogModel::addListItem(SCP_string containerName)
{
    auto container = lookupContainer(containerName);

    if (!container){
        return "";
    }

    if (container->string) {
        container->stringValues.emplace_back("New_Item");
        return container->stringValues.back();
    } else {
        container->numberValues.push_back(0);
        return "0";
    }
}

std::pair<SCP_string, SCP_string> VariableDialogModel::addMapItem(SCP_string ContainerName)
{
    
}

SCP_string VariableDialogModel::copyListItem(SCP_string containerName, int index)
{
    auto container = lookupContainer(containerName);

    if (!container || index < 0 || (cotainer->string && index >= static_cast<int>(contaner->stringValues.size())) || (container->string && index >= static_cast<int>(container->numberValues.size()))){
        return "";
    }

    if (container->string) {
        container->stringValues.push_back(container->stringValues[index]);
        return container->stringValues.back();
    } else {
        container->numberValues.push_back(container->numberValues[index]);
        return "0";
    }

}

bool VariableDialogModel::removeListItem(SCP_string containerName, int index)
{
    auto container = lookupContainer(containerName);

    if (!container || index < 0 || (cotainer->string && index >= static_cast<int>(contaner->stringValues.size())) || (container->string && index >= static_cast<int>(container->numberValues.size()))){
        return false;
    }

    // Most efficient, given the situation (single deletions)
    if (container->string) {
        container->stringValues.erase(container->stringValues.begin() + index);
    } else {
        container->numberValues.erase(container->numberValues.begin() + index);
    }

}

SCP_string VariableDialogModel::copyMapItem(SCP_string containerName, SCP_string key)
{
    for (const auto& )
}

bool VariableDialogModel::removeMapItem(SCP_string containerName, SCP_string key)
{

}

SCP_string VariableDialogModel::replaceMapItemKey(SCP_string containerName, SCP_string oldKey, SCP_string newKey)
{

}

SCP_string VariableDialogModel::changeMapItemStringValue(SCP_string containerName, SCP_string key, SCP_string newValue)
{

}

SCP_string VariableDialogModel::changeMapItemNumberValue(SCP_string containerName, SCP_string key, int newValue)
{

}

} // dialogs
} // fred
} // fso
