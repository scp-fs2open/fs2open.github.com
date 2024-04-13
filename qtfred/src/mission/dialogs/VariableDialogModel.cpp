#include "VariableDialogModel.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include <unordered_set>

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

void VariableDialogModel::checkValidModel()
{
    std::unordered_set<SCP_string> namesTaken;
    std::unordered_set<SCP_string> duplicates;

    for (const auto& variable : _variableItems){
        if (!namesTaken.insert(variable.name).second) {
            duplicates.insert(variable.name);
        } 
    }

    SCP_string messageOut1;
    SCP_string messageOut2;

    if (!duplicates.empty()){
        for (const auto& item : duplicates){
            if (messageOut2.empty()){
                messageOut2 = "\"" + item + "\"";
            } else {
                messageOut2 += ", "\"" + item + "\"";
            }
        }
        
        sprintf(messageOut1, "There are %zu duplicate variables:\n", duplicates.size());
        messageOut1 += messageOut2 + "\n\n";
    }

    duplicates.clear();
    unordered_set<SCP_string> namesTakenContainer;
    SCP_vector<SCP_string> duplicateKeys;

    for (const auto& container : _containerItems){
        if (!namesTakenContainer.insert(container.name).second) {
            duplicates.insert(container.name);
        }

        if (!container.list){
            unordered_set<SCP_string> keysTakenContainer;

            for (const auto& key : container.keys){
                if (!keysTakenContainer.insert(key)) {
                    SCP_string temp = key + "in map" + container.name + ", ";
                    duplicateKeys.push_back(temp);
                }
            }
        }
    }

    messageOut2.clear();
    
    if (!duplicates.empty()){
        for (const auto& item : duplicates){
            if (messageOut2.empty()){
                messageOut2 = "\"" + item + "\"";
            } else {
                messageOut2 += ", "\"" + item + "\"";
            }
        }
        
        SCP_string temp;

        sprintf(temp, "There are %zu duplicate containers:\n\n", duplicates.size());
        messageOut1 += messageOut2 + "\n";
    }

    messageOut2.clear();

    if (!duplicateKeys.empty()){
        for (const auto& key : duplicateKeys){
            messageOut2 += key;
        }

        SCP_string temp;

        sprintf(temp, "There are %zu duplicate map keys:\n\n", duplicateKeys.size());
        messageOut1 += messageOut2 + "\n";
    }

    if (messageOut1.empty()){
        apply();
    } else {
        messageOut1 = "Please correct these variable, container and key names. The editor cannot apply your changes until they are fixed:\n\n" + messageOut1;

	    QMessageBox msgBox;
        msgBox.setText(messageOut1.c_str());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }



}

bool VariableDialogModel::apply() 
{

    // what did we delete from the original list?  We need to check these references and clean them.
    std::unordered_set<SCP_string> deletedVariables;
    SCP_vector<std::pair<int, SCP_string>> nameChangedVariables;
    bool found;

    // first we have to edit known variables.
    for (const auto& variable : _variableItems){
        found = false;

        // set of instructions for updating variables
        if (!variable.originalName.empty()) {
            for (int i = 0; i < MAX_SEXP_VARIABLES; ++i) {
                if (!stricmp(Sexp_variables[i].variable_name, variable.originalName)){
                    if (variable.deleted) {
                        memset(Sexp_variables[i].variable_name, 0, NAME_LENGTH);
                        memset(Sexp_variables[i].text, 0, NAME_LENGTH);
                        Sexp_variables[i].type = 0;

                        deletedVariables.insert(variable.originalName);
                    } else {
                        if (variable.name != variable.originalName) {
                            nameChangedVariables.emplace_back(i, variable.originalName);   
                        }

                        strcpy_s(Sexp_variables[i].variable_name, variable.name.c_str());
                        Sexp_variables[i].flags = variable.flags;

                        if (variable.flags & SEXP_VARIABLE_STRING){
                            strcpy_s(Sexp_variables[i].text, variable.stringValue);
                            Sexp_variables[i].flags |= SEXP_VARIABLE_STRING;
                        } else {
                            strcpy_s(Sexp_variables[i].text, std::to_string(variable.numberValue).c_str())
                            Sexp_variables[i].flags |= SEXP_VARIABLE_NUMBER;
                        }
                    }

                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            // TODO! Lookup how FRED adds new variables.  (look for an empty slot maybe?)
        }
    }
    

    // TODO! containers
    std::unordered_set<SCP_string> deletedContainers;

    // TODO!  Look for referenced variables and containers. 
    // Need a way to clean up references.  I'm thinking making some pop ups to confirm replacements created in the editor.

	return false;
}

void VariableDialogModel::initializeData()
{
    _variableItems.clear();
    _containerItems.clear();

    for (int i = 0; i < static_cast<int>(_variableItems.size()); ++i){ 
        if (strlen(Sexp_variables[i].text)) {
            _variableItems.emplace_back();
            auto& item = _variableItems.back();
            item.name = Sexp_variables[i].variable_name;
            item.originalName = item.name;

            if (Sexp_variables[i].type & SEXP_VARIABLE_STRING) {
                item.string = true;
                item.stringValue = Sexp_variables[i].text;
                item.numberValue = 0;
            } else {
                item.string = false;
                
                Sexp_variables[i].text;
                try {
					item.numberValue = std::stoi(Sexp_variables[i].text);
                }                
                catch (...) {
                    item.numberValue = 0;
                }

                item.stringValue = "";
            }
        }
    }

    const auto& containers = get_all_sexp_containers();

    for (const auto& container : containers) {
        _containerItems.emplace_back();
        auto& newContainer = _containerItems.back();
        
        newContainer.name = container.container_name;
        newContainer.originalName = newContainer.name;
        newContainer.deleted = false;
        
        if (container.type & ContainerType::STRING_DATA) {
            newContainer.string = true;
        } else if (container.type & ContainerType::NUMBER_DATA) {
    		newContainer.string = false;
        }

        // using the SEXP variable version of these values here makes things easier
        if (container.type & ContainerType::SAVE_TO_PLAYER_FILE) { 
            newContainer.flags |= SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
        }

        if (container.type & ContainerType::SAVE_ON_MISSION_CLOSE) {
            newContainer.flags |= SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE;
        }

        if (container.type & ContainerType::SAVE_ON_MISSION_PROGRESS) {
            newContainer.flags |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
        }

        if (container.type & ContainerType::NETWORK) {
            newContainer.flags =| SEXP_VARIABLE_NETWORK;
        }
        
        newContainer.list = container.is_list();
    }
}



// true on string, false on number
bool VariableDialogModel::getVariableType(SCP_string name)
{
	auto variable = lookupVariable(name);
    return (variable) ? (variable->string) : true;   
}

bool VariableDialogModel::getVariableNetworkStatus(SCP_string name)
{
	auto variable = lookupVariable(name);
	return (variable) ? ((variable->flags & SEXP_VARIABLE_NETWORK) > 0) : false;
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
	auto variable = lookupVariable(name);
    return (variable) ? ((variable->flags & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE) > 0) : false;
}


SCP_string VariableDialogModel::getVariableStringValue(SCP_string name)
{
	auto variable = lookupVariable(name);
    return (variable && variable->string) ? (variable->stringValue) : "";
}

int VariableDialogModel::getVariableNumberValue(SCP_string name)
{
	auto variable = lookupVariable(name);
    return (variable && !variable->string) ? (variable->numberValue) : 0;
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
            sprintf(question, "Changing variable %s to number variable type will make its string value irrelevant.  Continue?", variable->name.c_str());
            SCP_string info;
			sprintf(info, "If the string cleanly converts to an integer and a number has not previously been set for this variable, the converted number value will be retained.");
            
            // if this was a misclick, let the user say so
            if (!confirmAction(question, info)) {
                return variable->string;
            }

            // if there was no previous number value 
            if (variable->numberValue == 0){
                try {                    
                    variable->numberValue = std::stoi(variable->stringValue);
                }
                // nothing to do here, because that just means we can't convert.
                catch (...) {}
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
            sprintf(question, "Changing variable %s to a string variable type will make the number value irrelevant.  Continue?", variable->name.c_str());
            SCP_string info;
			sprintf(info, "If no string value has been previously set for this variable, then the number value specified will be set as the default string value.");
            
            // if this was a misclick, let the user say so
            if (!confirmAction(question, info)) {
                return variable->string;
            }

            // if there was no previous string value 
            if (variable->stringValue == ""){
                sprintf(variable->stringValue, "%i", variable->numberValue);
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
        sprintf(name, "<unnamed_%i>", count);
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
        sprintf(newName, "%s_copy%i", name, count);
        variableSearch = lookupVariable(newName);

        // open slot found!
        if (!variableSearch){
            // create the new entry in the model
            _variableItems.emplace_back();

            // and set everything as a copy from the original, except original name and deleted.
            auto& newVariable = _variableItems.back();
            newVariable.name = newName;
            newVariable.flags = variableSearch->flags;
            newVariable.string = variableSearch->string;

            if (newVariable.string) {
                newVariable.stringValue = variableSearch->stringValue;
            } else {
                newVariable.numberValue = variableSearch->numberValue;
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

	SCP_string question = "Are you sure you want to delete this variable? Any references to it will have to be replaced.";
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
	auto container = lookupContainer(name);
    return (container) ? container->string : true;
}

// true on list, false on map
bool VariableDialogModel::getContainerListOrMap(SCP_string name)
{
	auto container = lookupContainer(name);
    return (container) ? container->list : true;
}

bool VariableDialogModel::getContainerNetworkStatus(SCP_string name)
{
	auto container = lookupContainer(name);
    return (container) ? ((container->flags & SEXP_VARIABLE_NETWORK) > 0) : false;    
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
	auto container = lookupContainer(name);
    return (container) ? ((container->flags & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE) > 0) : false;    
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
        return container->string;
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
                catch(...) {
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
                container->stringValues.emplace_back(std::to_string(item));
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
	return false;
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
        sprintf(name, "<unnamed_%i>", count);
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
    auto container = lookupContainer(name);

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

    if (!container || index < 0 || (container->string && index >= static_cast<int>(container->stringValues.size())) || (container->string && index >= static_cast<int>(container->numberValues.size()))){
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

    if (!container || index < 0 || (container->string && index >= static_cast<int>(container->stringValues.size())) || (container->string && index >= static_cast<int>(container->numberValues.size()))){
        return false;
    }

    // Most efficient, given the situation (single deletions)
    if (container->string) {
        container->stringValues.erase(container->stringValues.begin() + index);
    } else {
        container->numberValues.erase(container->numberValues.begin() + index);
    }

}

std::pair<SCP_string, SCP_string> VariableDialogModel::copyMapItem(SCP_string containerName, SCP_string keyIn)
{
    auto container = lookupContainer(containerName);

    if (!container) {
        return std::make_pair("", "");
    }

    for (int x = 0; x < static_cast<int>(container->keys.size()); ++x) {
        if (container->keys[x] == keyIn){
            if (container->string){
                if (x < static_cast<int>(container->stringValues.size())){
                    SCP_string copyValue = container->stringValues[x];
                    SCP_string newKey;
                    int size = static_cast<int>(container->keys.size());
                    sprintf(newKey, "key%i", size);
                    
                    bool found = false;

                    do {
                        found = false;
                        for (int y = 0; y < static_cast<int>(container->keys.size()); ++y){
                            if (container->keys[y] == newKey) {
                                found = true;
                                break;
                            }
                        }

                        // attempt did not work, try next number
                        if (found) {
                            ++size;
                            newKey = "";
                            sprintf(newKey, "key%i", size);
                        }

					} while (found && size < static_cast<int>(container->keys.size()) + 100);
                    
                    // we could not generate a new key .... somehow.
                    if (found){
                        return std::make_pair("", "");
                    }

                    container->keys.push_back(newKey);
                    container->stringValues.push_back(copyValue);

                    return std::make_pair(newKey, copyValue);

                } else {
                    return std::make_pair("", "");
                }
            } else {
                if (x < static_cast<int>(container->numberValues.size())){
                    int copyValue = container->numberValues[x];
                    SCP_string newKey;
                    int size = static_cast<int>(container->keys.size());
                    sprintf(newKey, "key%i", size);
                    
                    bool found = false;

                    do {
                        found = false;
                        for (int y = 0; y < static_cast<int>(container->keys.size()); ++y){
                            if (container->keys[y] == newKey) {
                                found = true;
                                break;
                            }
                        }

                        // attempt did not work, try next number
                        if (found) {
                            ++size;
                            newKey = "";
                            sprintf(newKey, "key%i", size);
                        }

					} while (found && size < static_cast<int>(container->keys.size()) + 100);
                    
                    // we could not generate a new key .... somehow.
                    if (found){
                        return std::make_pair("", "");
                    }

                    container->keys.push_back(newKey);
                    container->numberValues.push_back(copyValue);

					SCP_string temp;
					sprintf(temp, "%i", copyValue);

                    return std::make_pair(newKey, temp);

                } else {
                    return std::make_pair("", "");
                }
            }
        }
    }
}

// it's really because of this feature that we need data to only be in one or the other vector for maps.
// If we attempted to maintain data automatically and there was a deletion, deleting the data in
// both of the map's data vectors might be undesired, and not deleting takes the map immediately
// out of sync.  Also, just displaying both data sets would be misleading.
// We just need to tell the user that the data cannot be maintained. 
bool VariableDialogModel::removeMapItem(SCP_string containerName, SCP_string key)
{
    auto container = lookupContainer(containerName);

    if (!container){
        return false;
    }

    for (int x = 0; x < static_cast<int>(container->keys.size()); ++x) {
        if (container->keys[x] == key) {
            if (container->string && x < static_cast<int>(container->stringValues.size())) {
                container->stringValues.erase(container->stringValues.begin() + x);
            } else if (!container->string && x < static_cast<int>(container->numberValues.size())){
                container->numberValues.erase(container->numberValues.begin() + x);
            } else {
                return false;
            }

            // if we get here, we've succeeded and it's time to bug out
            container->keys.erase(container->keys.begin() + x);
            // "I'm outta here!"
            return true;
        }
    }

    // NO SPRINGS!!! HEHEHEHEHE
    return false;
}

SCP_string VariableDialogModel::replaceMapItemKey(SCP_string containerName, SCP_string oldKey, SCP_string newKey)
{
    auto container = lookupContainer(containerName);

    if (!container){
        return "";
    }

    for (auto& key : container->keys){
        if (key == oldKey) {
            key = newKey;
            return newKey;
        }
    }

    // Failure
    return oldKey;
}

SCP_string VariableDialogModel::changeMapItemStringValue(SCP_string containerName, SCP_string key, SCP_string newValue)
{
    auto container = lookupContainer(containerName);

    if (!container || !container->string){
        return "";
    }

    for (int x = 0; x < static_cast<int>(container->keys.size()); ++x){
        if (container->keys[x] == key) {
            if (x < static_cast<int>(container->stringValues.size())){
                container->stringValues[x] = newValue;
                return newValue;
            } else {
                return "";
            }
        }
    }

    // Failure
    return "";
}

SCP_string VariableDialogModel::changeMapItemNumberValue(SCP_string containerName, SCP_string key, int newValue)
{
    auto container = lookupContainer(containerName);

    if (!container || !container->string){
        return "";
    }

    for (int x = 0; x < static_cast<int>(container->keys.size()); ++x){
        if (container->keys[x] == key) {
            if (x < static_cast<int>(container->numberValues.size())){
                container->numberValues[x] = newValue;
                SCP_string returnValue;
				sprintf(returnValue, "%i", newValue);
                return returnValue;
            } else {
                return "";
            }
        }
    }

    // Failure
    return "";
}

// These functions should only be called when the container is guaranteed to exist!
const SCP_vector<SCP_string>& VariableDialogModel::getMapKeys(SCP_string containerName) 
{
    auto container = lookupContainer(containerName);

    if (!container) {
		SCP_string temp;
		sprintf("getMapKeys() found that container %s does not exist.", containerName.c_str());
        throw std::invalid_argument(temp.c_str());
    }

    if (container->list) {
		SCP_string temp;
		sprintf("getMapKeys() found that container %s is not a map.", containerName.c_str());
		throw std::invalid_argument(temp);
    }

    return container->keys;
}

// Only call when the container is guaranteed to exist!
const SCP_vector<SCP_string>& VariableDialogModel::getStringValues(SCP_string containerName) 
{
    auto container = lookupContainer(containerName);

    if (!container) {
		SCP_string temp;
		sprintf("getStringValues() found that container %s does not exist.", containerName.c_str());
        throw std::invalid_argument(temp);
    }

    if (!container->string) {
		SCP_string temp;
		sprintf("getStringValues() found that container %s does not store strings.", containerName.c_str());
		throw std::invalid_argument(temp);
    }

    return container->stringValues;
}

// Only call when the container is guaranteed to exist!
const SCP_vector<int>& VariableDialogModel::getNumberValues(SCP_string containerName) 
{
    auto container = lookupContainer(containerName);

    if (!container) {
		SCP_string temp;
		sprintf("getNumberValues() found that container %s does not exist.", containerName.c_str());
		throw std::invalid_argument(temp);  
    }

    if (container->string) {
		SCP_string temp;
		sprintf("getNumberValues() found that container %s does not store numbers.", containerName.c_str());
		throw std::invalid_argument(temp);
    }

    return container->numberValues;
}

const SCP_vector<std::tuple<SCP_string, SCP_string, SCP_string>> VariableDialogModel::getVariableValues()
{
    SCP_vector<std::tuple<SCP_string, SCP_string, SCP_string>> outStrings;

    for (const auto& item : _variableItems) {
        SCP_string notes = "";

        if (item.deleted) {
            notes = "Marked for Deletion";
        } else if (item.originalName == "") {
            notes = "New";
        } else if (item.name != item.originalName){
            notes = "Renamed";
        } else if (item.string && item.stringValue == "") {
            notes = "Defaulting to empty string";
        }

		SCP_string temp;
		sprintf(temp, "%i", item.numberValue);
        outStrings.emplace_back(item.name, (item.string) ? item.stringValue : temp, notes);
    }

    return outStrings;
}


const SCP_vector<std::pair<SCP_string, SCP_string>> VariableDialogModel::getContainerNames()
{
    SCP_vector<std::pair<SCP_string, SCP_string>> outStrings;

    for (const auto& item : _containerItems) {
        SCP_string notes = "";

        if (item.deleted) {
            notes = "Marked for Deletion";
        } else if (item.originalName == "") {
            notes = "New";
        } else if (item.name != item.originalName){
            notes = "Renamed";
        }

        outStrings.emplace_back(item.name, notes);
    }

    return outStrings;   
}


} // dialogs
} // fred
} // fso
