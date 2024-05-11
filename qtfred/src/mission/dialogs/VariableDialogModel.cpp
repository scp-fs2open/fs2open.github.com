#include "VariableDialogModel.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include <unordered_set>
#include <climits>

namespace fso {
namespace fred {
namespace dialogs {
	static int _textMode = 0;

VariableDialogModel::VariableDialogModel(QObject* parent, EditorViewport* viewport) 
		: AbstractDialogModel(parent, viewport)
{
	_deleteWarningCount = 0;
	initializeData();
}

void VariableDialogModel::reject() 
{
    _variableItems.clear();
    _containerItems.clear();
}

bool VariableDialogModel::checkValidModel()
{
    std::unordered_set<SCP_string> namesTaken;
    std::unordered_set<SCP_string> duplicates;

    for (const auto& variable : _variableItems){
        if (!namesTaken.insert(variable.name).second) {
            duplicates.insert(variable.name);
        } 
    }

    SCP_string messageOut;
    SCP_string messageBuffer;

    if (!duplicates.empty()){
        for (const auto& item : duplicates){
            if (messageBuffer.empty()){
                messageBuffer = "\"" + item + "\"";
            } else {
                messageBuffer += ", ""\"" + item + "\"";
            }
        }
        
        sprintf(messageOut, "There are %zu duplicate variables:\n", duplicates.size());
        messageOut += messageBuffer + "\n\n";
    }

    duplicates.clear();
    std::unordered_set<SCP_string> namesTakenContainer;
    SCP_vector<SCP_string> duplicateKeys;

    for (const auto& container : _containerItems){
        if (!namesTakenContainer.insert(container.name).second) {
            duplicates.insert(container.name);
        }

        if (!container.list){
			std::unordered_set<SCP_string> keysTakenContainer;

            for (const auto& key : container.keys){
                if (!keysTakenContainer.insert(key).second) {
                    SCP_string temp = key + "in map" + container.name + ", ";
                    duplicateKeys.push_back(temp);
                }
            }
        }
    }

    messageBuffer.clear();
    
    if (!duplicates.empty()){
        for (const auto& item : duplicates){
            if (messageBuffer.empty()){
                messageBuffer = "\"" + item + "\"";
            } else {
                messageBuffer += ", ""\"" + item + "\"";
            }
        }
        
        SCP_string temp;

        sprintf(temp, "There are %zu duplicate containers:\n\n", duplicates.size());
        messageOut += temp + messageBuffer + "\n";
    }

    messageBuffer.clear();

    if (!duplicateKeys.empty()){
        for (const auto& key : duplicateKeys){
            messageBuffer += key;
        }

        SCP_string temp;

        sprintf(temp, "There are %zu duplicate map keys:\n\n", duplicateKeys.size());
        messageOut += messageBuffer + "\n";
    }

    if (messageOut.empty()){
        return true;
    } else {
        messageOut = "Please correct these issues. The editor cannot apply your changes until they are fixed:\n\n" + messageOut;

	    QMessageBox msgBox;
        msgBox.setText(messageOut.c_str());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

		return false;
    }
}

// TODO! This function in general just needs a lot of work.
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
                if (!stricmp(Sexp_variables[i].variable_name, variable.originalName.c_str())){
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
                        Sexp_variables[i].type = variable.flags;

                        if (variable.flags & SEXP_VARIABLE_STRING){
                            strcpy_s(Sexp_variables[i].text, variable.stringValue.c_str());
                            Sexp_variables[i].type |= SEXP_VARIABLE_STRING;
                        } else {
							strcpy_s(Sexp_variables[i].text, std::to_string(variable.numberValue).c_str());
                            Sexp_variables[i].type |= SEXP_VARIABLE_NUMBER;
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
        
        if (any(container.type & ContainerType::STRING_DATA)) {
            newContainer.string = true;
        } else if (any(container.type & ContainerType::NUMBER_DATA)) {
    		newContainer.string = false;
        }

        // using the SEXP variable version of these values here makes things easier
        if (any(container.type & ContainerType::SAVE_TO_PLAYER_FILE)) { 
            newContainer.flags |= SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
        }

        if (any(container.type & ContainerType::SAVE_ON_MISSION_CLOSE)) {
            newContainer.flags |= SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE;
        }

        if (any(container.type & ContainerType::SAVE_ON_MISSION_PROGRESS)) {
            newContainer.flags |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
        }

        if (any(container.type & ContainerType::NETWORK)) {
            newContainer.flags |= SEXP_VARIABLE_NETWORK;
        }
        
        newContainer.list = container.is_list();
    }
}



// true on string, false on number
bool VariableDialogModel::getVariableType(int index)
{
	auto variable = lookupVariable(index);
    return (variable) ? (variable->string) : true;   
}

bool VariableDialogModel::getVariableNetworkStatus(int index)
{
	auto variable = lookupVariable(index);
	return (variable) ? ((variable->flags & SEXP_VARIABLE_NETWORK) != 0) : false;
}


// 0 neither, 1 on mission complete, 2 on mission close (higher number saves more often)
int VariableDialogModel::getVariableOnMissionCloseOrCompleteFlag(int index)
{
    auto variable = lookupVariable(index);

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

bool VariableDialogModel::getVariableEternalFlag(int index)
{
	auto variable = lookupVariable(index);
    return (variable) ? ((variable->flags & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE) != 0) : false;
}


SCP_string VariableDialogModel::getVariableStringValue(int index)
{
	auto variable = lookupVariable(index);
    return (variable && variable->string) ? (variable->stringValue) : "";
}

int VariableDialogModel::getVariableNumberValue(int index)
{
	auto variable = lookupVariable(index);
    return (variable && !variable->string) ? (variable->numberValue) : 0;
}



// true on string, false on number
bool VariableDialogModel::setVariableType(int index, bool string)
{
    auto variable = lookupVariable(index);

    // nothing to change, or invalid entry
    // Best way to say that it failed is to say 
    // that it is not switching to what the ui asked for. 
    if (!variable || variable->string == string){
        return !string;
    }

    // Here we change the variable type!
    // this variable is currently a string
    if (variable->string) {
        // no risk change, because no string was specified.
        if (variable->stringValue == "") {
            variable->string = string;
            return variable->string;
        } else {
            // if there was no previous number value 
            if (variable->numberValue == 0){
                try {                    
                    variable->numberValue = std::stoi(variable->stringValue);
                }
                // nothing to do here, because that just means we can't convert and we have to use the old value.
                catch (...) {}

            }

			variable->string = string;
            return variable->string;
        }
    
    // this variable is currently a number
    } else {
        // safe change because there was no number value specified
        if (variable->numberValue == 0){
            variable->string = string;
            return variable->string;
        } else {
            // if there was no previous string value 
            if (variable->stringValue == ""){
                sprintf(variable->stringValue, "%i", variable->numberValue);
            }

			variable->string = string;
            return variable->string;
        }
    }
}

bool VariableDialogModel::setVariableNetworkStatus(int index, bool network)
{
    auto variable = lookupVariable(index);

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

int VariableDialogModel::setVariableOnMissionCloseOrCompleteFlag(int index, int flags)
{
    auto variable = lookupVariable(index);

    // nothing to change, or invalid entry
    if (!variable || flags < 0 || flags > 2){
        return 0;
    }

    if (flags == 0) {
        variable->flags &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS | SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
    } else if (flags == 1) {
        variable->flags &= ~SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE;
        variable->flags |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
    } else {
        variable->flags |= (SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS | SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);
    }
    
    return flags;
}

bool VariableDialogModel::setVariableEternalFlag(int index, bool eternal)
{
    auto variable = lookupVariable(index);

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

SCP_string VariableDialogModel::setVariableStringValue(int index, SCP_string value)
{
    auto variable = lookupVariable(index);

    // nothing to change, or invalid entry
    if (!variable || !variable->string){
        return "";
    }
    
    variable->stringValue = value;
	return value;
}

int VariableDialogModel::setVariableNumberValue(int index, int value)
{
    auto variable = lookupVariable(index);

    // nothing to change, or invalid entry
    if (!variable || variable->string){
        return 0;
    }
    
    variable->numberValue = value;

	return value;
}

SCP_string VariableDialogModel::addNewVariable()
{
    variableInfo* variable = nullptr;
    int count = 1;
    SCP_string name;

    do {
        name = "";
        sprintf(name, "newVar%i", count);
        variable = lookupVariableByName(name);
        ++count;
    } while (variable != nullptr && count < 51);


    if (variable){
        return "";
    }

    _variableItems.emplace_back();
    _variableItems.back().name = name;
    return name;
}

SCP_string VariableDialogModel::addNewVariable(SCP_string nameIn){
    _variableItems.emplace_back();
    _variableItems.back().name = nameIn;
    return _variableItems.back().name;
}

SCP_string VariableDialogModel::changeVariableName(int index, SCP_string newName)
{ 
    auto variable = lookupVariable(index);

    // nothing to change, or invalid entry
    if (!variable){
        return "";
    }

    // Truncate name if needed
    if (newName.length() >= TOKEN_LENGTH){
        newName = newName.substr(0, TOKEN_LENGTH - 1);
    }

    // We cannot have two variables with the same name, but we need to check this somewhere else (like on accept attempt).
    variable->name = newName;
    return newName;
}

SCP_string VariableDialogModel::copyVariable(int index)
{
    auto variable = lookupVariable(index);

    // nothing to change, or invalid entry
    if (!variable){
        return "";
    }

    int count = 1;
    variableInfo* variableSearch;
	SCP_string newName;

    do {
        sprintf(newName, "%i_%s", count, variable->name.substr(0, TOKEN_LENGTH - 4).c_str());
        variableSearch = lookupVariableByName(newName);

        // open slot found!
        if (!variableSearch){
            // create the new entry in the model
			variableInfo newInfo;

            // and set everything as a copy from the original, except original name and deleted.
			newInfo.name = newName;
			newInfo.flags = variable->flags;
			newInfo.string = variable->string;

            if (newInfo.string) {
				newInfo.stringValue = variable->stringValue;
            } else {
				newInfo.numberValue = variable->numberValue;
            }

			_variableItems.push_back(std::move(newInfo));

            return newName;
        }

		++count;
    } while (variableSearch != nullptr && count < 100);

    return "";
}

// returns whether it succeeded
bool VariableDialogModel::removeVariable(int index, bool toDelete)
{
    auto variable = lookupVariable(index);

    // nothing to change, or invalid entry
    if (!variable){
        return false;
    }

    if (variable->deleted == toDelete){
        return variable->deleted;
    }

    if (toDelete){
        if (_deleteWarningCount < 2){

            SCP_string question = "Are you sure you want to delete this variable? Any references to it will have to be changed.";
            SCP_string info = "";

            if (!confirmAction(question, info)){
                --_deleteWarningCount;
                return variable->deleted;
            }

            // adjust to the user's actions.  If they are deleting variable after variable, allow after a while.
            ++_deleteWarningCount;
        }

        variable->deleted = toDelete;
        return variable->deleted;

    } else {
        variable->deleted = toDelete;
        return variable->deleted;
    }
}


// Container Section

// true on string, false on number
bool VariableDialogModel::getContainerValueType(int index)
{
	auto container = lookupContainer(index);
    return (container) ? container->string : true;
}

bool VariableDialogModel::getContainerKeyType(int index)
{
    auto container = lookupContainer(index);
	return (container) ? container->stringKeys : true;
}

// true on list, false on map
bool VariableDialogModel::getContainerListOrMap(int index)
{
	auto container = lookupContainer(index);
    return (container) ? container->list : true;
}

bool VariableDialogModel::getContainerNetworkStatus(int index)
{
	auto container = lookupContainer(index);
    return (container) ? ((container->flags & SEXP_VARIABLE_NETWORK) != 0) : false;    
}

// 0 neither, 1 on mission complete, 2 on mission close (higher number saves more often)
int VariableDialogModel::getContainerOnMissionCloseOrCompleteFlag(int index)
{
    auto container = lookupContainer(index);

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

bool VariableDialogModel::getContainerEternalFlag(int index)
{
	auto container = lookupContainer(index);
    return (container) ? ((container->flags & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE) != 0) : false;    
}


bool VariableDialogModel::setContainerValueType(int index, bool type)
{
    auto container = lookupContainer(index);

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

bool VariableDialogModel::setContainerKeyType(int index, bool string) 
{    
    auto container = lookupContainer(index);

    // nothing to change, or invalid entry
    if (!container){
        return false;
    }

    if (container->stringKeys == string){
        return container->stringKeys;
    }

    if (container->stringKeys) {
        // Ok, this is the complicated type.  First check if all keys can just quickly be transferred to numbers.
        bool quickConvert = true;
		int test;
        for (auto& key : container->keys) {
            try {                    
                test = std::stoi(key);
            }
            catch (...) {
                quickConvert = false;
				nprintf(("Cyborg", "This is Cyborg. Long story short, I don't need this variable, but c++ thinks I do. Last good number on conversion was: %i\n", test));
            }
        } 

        // Don't even notify the user. Switching back is exceedingly easy.
        if (quickConvert) {
            container->stringKeys = string;
            return container->stringKeys;
        }

        // If we couldn't convert easily, then we need some input from the user
                // now ask about data
        QMessageBox msgBoxListToMapRetainData;
		msgBoxListToMapRetainData.setWindowTitle("Key Type Conversion");
	    msgBoxListToMapRetainData.setText("Fred could not convert all string keys to numbers automatically.  Would you like to use default keys, filter out integers from the current keys or cancel the operation?");
	    msgBoxListToMapRetainData.setInformativeText("Current keys will be overwritten unless you cancel and cannot be restored. Filtering will keep *any* numerical digits and starting \"-\" in the string.  Filtering also does not prevent duplicate keys.");
		msgBoxListToMapRetainData.addButton("Use Default Keys", QMessageBox::ActionRole); // No, these categories don't make sense, but QT makes underlying assumptions about where each button will be
        msgBoxListToMapRetainData.addButton("Filter Current Keys ", QMessageBox::RejectRole);
		auto defaultButton = msgBoxListToMapRetainData.addButton("Cancel", QMessageBox::HelpRole);
	    msgBoxListToMapRetainData.setDefaultButton(defaultButton);
	    auto ret = msgBoxListToMapRetainData.exec();

        switch(ret){
            // just use default keys
            case QMessageBox::ActionRole:
            {
                int current = 0;
                for (auto& key : container->keys){
                    sprintf(key, "%i", current);
                    ++current;
                }

				container->stringKeys = string;
				return container->stringKeys;
            }

            // filter out current keys
            case QMessageBox::RejectRole:
                for (auto& key: container->keys){
                    key = trimIntegerString(key);
                }

				container->stringKeys = string;
				return container->stringKeys;

            // cancel the operation
            case QMessageBox::HelpRole:
                return !string;
            default:
                UNREACHABLE("Bad button value from confirmation message box in the Variable editor, please report!");
        }

    } else {
        // transferring to keys to string type. This can just change because a valid number is always a valid string.
        container->stringKeys = string;
        return container->stringKeys;
    }

    return false;
}

// This is the most complicated function, because we need to query the user on what they want to do if the had already entered data. 
bool VariableDialogModel::setContainerListOrMap(int index, bool list)
{
    auto container = lookupContainer(index);

    // nothing to change, or invalid entry
    if (!container){
        return !list;
    }

    if (container->list == list){
        return list;
    }

    if (container->list) {
        // no data to either transfer to map/purge/ignore
        if (container->string && container->stringValues.empty()){
            container->list = list;

            // still need to deal with extant keys by resizing data values.
            if (!container->keys.empty()){
				container->stringValues.resize(container->keys.size());
            }

            return list;
        } else if (!container->string && container->numberValues.empty()){
            container->list = list;

            // still need to deal with extant keys by resizing data values.
            if (!container->keys.empty()){
				container->numberValues.resize(container->keys.size());
            }

            return list;
        }

        QMessageBox msgBoxListToMapConfirm;
	    msgBoxListToMapConfirm.setText("This list already has data.  Continue conversion to map?");
	    msgBoxListToMapConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	    msgBoxListToMapConfirm.setDefaultButton(QMessageBox::Cancel);
	    int ret = msgBoxListToMapConfirm.exec();

	    switch (ret) {
            case QMessageBox::Yes:
                break;

            case QMessageBox::Cancel:
                return container->list;
                break;

            default:
                UNREACHABLE("Bad button value from confirmation message box in the Variable editor, please report!");
                return false;
                break;            
	    }

        // now ask about data
        QMessageBox msgBoxListToMapRetainData;
		msgBoxListToMapRetainData.setWindowTitle("List to Map Conversion");
	    msgBoxListToMapRetainData.setText("Would you to keep the list data as keys or values, or would you like to purge the container contents?");
		msgBoxListToMapRetainData.setInformativeText("Converting to keys will erase current keys and cannot be undone.  Purging all container data cannot be undone.");
		msgBoxListToMapRetainData.addButton("Keep as Values", QMessageBox::ActionRole); // No, these categories don't make sense, but QT makes underlying assumptions about where each button will be
        msgBoxListToMapRetainData.addButton("Convert to Keys", QMessageBox::RejectRole); // Instead of putting them in order of input to the 
        msgBoxListToMapRetainData.addButton("Purge", QMessageBox::ApplyRole);
		auto defaultButton = msgBoxListToMapRetainData.addButton("Cancel", QMessageBox::HelpRole);
	    msgBoxListToMapRetainData.setDefaultButton(defaultButton);
	    ret = msgBoxListToMapRetainData.exec();

	    switch (ret) {
            case QMessageBox::RejectRole:
                // The easy version. (I know ... I should have standardized all storage as strings internally.... Now I'm in too deep)
                if (container->string){
                    container->keys = container->stringValues;
                    container->stringValues.clear();
					container->stringValues.resize(container->keys.size(), "");
                    container->list = list;
                    return container->list;
                }

                // The hard version ...... I guess it's not that bad, actually
                container->keys.clear();

                for (auto& number : container->numberValues){
                    SCP_string temp;
                    sprintf(temp, "%i", number);
                    container->keys.push_back(temp);
                }
                
                container->numberValues.clear();
                container->list = list;
                return container->list;

            case QMessageBox::ActionRole:
			{
				auto currentSize = (container->string) ? container->stringValues.size() : container->numberValues.size();

				// Keys and data are already set to the correct sizes. Key type should persist from the last time it was a map, so no need
				// to adjust keys.
				if (currentSize == container->keys.size()) {
					container->list = list;
					return container->list;
				}

				// not enough data items.
				if (currentSize < container->keys.size()) {
					// just put the default value in them. Any string I specify for string values will
					// be inconvenient to someone.
					if (container->string) {
						SCP_string newValue = "";
						container->stringValues.resize(container->keys.size(), newValue);
					}
					else {
						// But differentiating numbers by having zero be the default is a good idea and does
						container->numberValues.resize(container->keys.size(), 0);
					}

				}
				else {
					// here currentSize must be greater than the key size, because we already dealt with equal size.
					// So let's add a few keys to make them level.
					while (currentSize > container->keys.size()) {
						int keyIndex = 0;
						SCP_string newKey;

						if (container->stringKeys) {
							sprintf(newKey, "key%i", keyIndex);
						}
						else {
							sprintf(newKey, "%i", keyIndex);
						}

						// avoid duplicates
						if (!lookupContainerKeyByName(index, newKey)) {
							container->keys.push_back(newKey);
						}

						++keyIndex;
					}
				}

				container->list = list;
				return container->list;
			}
            case QMessageBox::ApplyRole:

                container->list = list;
                container->stringValues.clear();
                container->numberValues.clear();
                container->keys.clear();
                return container->list;
                break;

            case QMessageBox::HelpRole:
            	return !list;
                break;

            default:
                UNREACHABLE("Bad button value from confirmation message box in the Variable editor, please report!");
                return false;
                break;
            
		}
	} else {
		// why yes, in this case it really is that simple.  It doesn't matter what keys are doing, and there should already be valid values.
		container->list = list;
		return container->list;
	}

	return !list;
}

bool VariableDialogModel::setContainerNetworkStatus(int index, bool network)
{
    auto container = lookupContainer(index);

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

int VariableDialogModel::setContainerOnMissionCloseOrCompleteFlag(int index, int flags)
{
    auto container = lookupContainer(index);

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

bool VariableDialogModel::setContainerEternalFlag(int index, bool eternal)
{
    auto container = lookupContainer(index);

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
        sprintf(name, "newCont%i", count);
        container = lookupContainerByName(name);
        ++count;
    } while (container != nullptr && count < 51);

    if (container){
        return "";
    }

    _containerItems.emplace_back();
    _containerItems.back().name = name;
    return _containerItems.back().name;
}

SCP_string VariableDialogModel::addContainer(SCP_string nameIn)
{
    _containerItems.emplace_back();
    _containerItems.back().name = nameIn;
    return _containerItems.back().name;
}

SCP_string VariableDialogModel::copyContainer(int index)
{
    auto container = lookupContainer(index);

    // nothing to copy, invalid entry
    if (!container){
        return "";
    }

    // K.I.S.S. We could guarantee the names be unique, but so can the user, and there will definitely be a lower number of containers
    _containerItems.push_back(*container);
    _containerItems.back().name = "copy_" + _containerItems.back().name;
    _containerItems.back().name = _containerItems.back().name.substr(0, TOKEN_LENGTH - 1);
    return _containerItems.back().name;
}

SCP_string VariableDialogModel::changeContainerName(int index, SCP_string newName)
{
    auto container = lookupContainer(index);

    // nothing to change, or invalid entry
    if (!container){
        return "";
    }

    // We cannot have two containers with the same name, but we need to check this somewhere else (like on accept attempt).
    container->name = newName;
    return newName;
}

bool VariableDialogModel::removeContainer(int index, bool toDelete)
{
    auto container = lookupContainer(index);

    if (!container){
        return false;
    }

    if (container->deleted == toDelete){
        return container->deleted;
    }

    if (toDelete){
        
        if (_deleteWarningCount < 3){
            SCP_string question = "Are you sure you want to delete this container? Any references to it will have to be changed.";
            SCP_string info = "";

            if (!confirmAction(question, info)){
                return container->deleted;
            }

            // adjust to the user's actions.  If they are deleting container after container, allow after a while.
            ++_deleteWarningCount;
        }

		container->deleted = toDelete;
        return container->deleted;

    } else {
		container->deleted = toDelete;
        return container->deleted;
    }
}

SCP_string VariableDialogModel::addListItem(int index)
{
    auto container = lookupContainer(index);

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

SCP_string VariableDialogModel::addListItem(int index, SCP_string item)
{
	auto container = lookupContainer(index);

	if (!container){
		return "";
	}

	if (container->string) {
		container->stringValues.push_back(item);
		return container->stringValues.back();
	} else {
		auto temp = trimIntegerString(item);
		
		try {
			int tempNumber = std::stoi(temp);
			container->numberValues.push_back(tempNumber);
		}
		catch(...){
			container->numberValues.push_back(0);
			return "0";
		}

		sprintf(temp, "%i", container->numberValues.back());
		return temp;
	}
}

std::pair<SCP_string, SCP_string> VariableDialogModel::addMapItem(int index)
{
    auto container = lookupContainer(index);

    std::pair <SCP_string, SCP_string> ret = {"", ""};

    // no container available
    if (!container){
        return ret;
    }

    bool conflict;
    int count = 0;
    SCP_string newKey;

    do {
        conflict = false;
        
        if (container->stringKeys){
            sprintf(newKey, "key%i", count);
        } else {
            sprintf(newKey, "%i", count);
        }

        for (int x = 0; x < static_cast<int>(container->keys.size()); ++x) {
            if (container->keys[x] == newKey){
                conflict = true;
                break;
            }
        }

        ++count;
    } while (conflict && count < 101);

    if (conflict) {
        return ret;
    }

    ret.first = newKey;
	container->keys.push_back(newKey);

    if (container->string){
        ret.second = "";
		container->stringValues.push_back("");
    } else {
        ret.second = "0";
		container->numberValues.push_back(0);
	}

    return ret;
}

std::pair<SCP_string, SCP_string> VariableDialogModel::addMapItem(int index, SCP_string key, SCP_string value)
{
	auto container = lookupContainer(index);

	std::pair <SCP_string, SCP_string> ret = { "", "" };

	// no container available
	if (!container) {
		return ret;
	}

	bool conflict;
	int count = 0;
	SCP_string newKey;

	if (key.empty()) {
		do {
			conflict = false;

			if (container->stringKeys){
				sprintf(newKey, "key%i", count);
			} else {
				sprintf(newKey, "%i", count);
			}

			for (int x = 0; x < static_cast<int>(container->keys.size()); ++x) {
				if (container->keys[x] == newKey){
					conflict = true;
					break;
				}
			}

			++count;
		} while (conflict && count < 101);
	} else {
		newKey = key;
	}

	if (conflict) {
		return ret;
	}

	ret.first = newKey;
	container->keys.push_back(ret.first);

	if (container->string) {
		ret.second = value;
	} else {
		try {
			container->numberValues.push_back(std::stoi(value));
			ret.second = value;
		}
		catch (...) {
			ret.second = "0";
			container->numberValues.push_back(0);
		}
	}

	return ret;
}

SCP_string VariableDialogModel::copyListItem(int containerIndex, int index)
{
    auto container = lookupContainer(containerIndex);

    if (!container || index < 0 || (container->string && index >= static_cast<int>(container->stringValues.size())) || (!container->string && index >= static_cast<int>(container->numberValues.size()))){
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

SCP_string VariableDialogModel::changeListItem(int containerIndex, int index, SCP_string newString)
{
	auto container = lookupContainer(containerIndex);

	if (!container){
		return "";
	}

	if (container->string){
		auto listItem = lookupContainerStringItem(containerIndex, index);

		if (!listItem){
			return "";
		}
	
		*listItem = newString;

	} else {
		auto listItem = lookupContainerNumberItem(containerIndex, index);
		
		if (!listItem){
			return "";
		}

		try{
			*listItem = std::stoi(newString);
		}
		catch(...){
			SCP_string temp;
			sprintf(temp, "%i", *listItem);
			return temp;
		}
	}

	return "";
}

bool VariableDialogModel::removeListItem(int containerIndex, int index)
{
    auto container = lookupContainer(containerIndex);

    if (!container || index < 0 || (container->string && index >= static_cast<int>(container->stringValues.size())) || (container->string && index >= static_cast<int>(container->numberValues.size()))){
        return false;
    }

    if (_deleteWarningCount < 3){
        SCP_string question = "Are you sure you want to delete this list item? This can't be undone.";
        SCP_string info = "";

        if (!confirmAction(question, info)){
            --_deleteWarningCount;
            return container->deleted;
        }

        // adjust to the user's actions.  If they are deleting variable after variable, allow after a while.
        ++_deleteWarningCount;
    }

    // Most efficient, given the situation (single deletions)
    if (container->string) {
        container->stringValues.erase(container->stringValues.begin() + index);
    } else {
        container->numberValues.erase(container->numberValues.begin() + index);
    }

    return true;
}

std::pair<SCP_string, SCP_string> VariableDialogModel::copyMapItem(int index, int mapIndex)
{
    auto container = lookupContainer(index);

    // any invalid case, early return
    if (!container) {
        return std::make_pair("", "");
    }

    auto key = lookupContainerKey(index, mapIndex);

    if (!key) {
        return std::make_pair("", "");
    }
    
    

    if (container->string){
        auto value = lookupContainerStringItem(index, mapIndex);

        // no valid value.
        if (!value){
            return std::make_pair("", "");
        }

        SCP_string copyValue = *value;
        SCP_string newKey = *key + "0";
        int count = 0;

        bool found;

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
                sprintf(newKey, "%s%i", key->c_str(), ++count);
            }

        } while (found && count < 100);
        
        // we could not generate a new key .... somehow.
        if (found){
            return std::make_pair("", "");
        }

        container->keys.push_back(newKey);
        container->stringValues.push_back(copyValue);

        return std::make_pair(newKey, copyValue);

    } else {
        auto value = lookupContainerNumberItem(index, mapIndex);

        // no valid value.
        if (!value){
            return std::make_pair("", "");
        }

        int copyValue = *value;
        SCP_string newKey = *key + "0";
        int count = 0;

        bool found;

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
                sprintf(newKey, "%s%i", key->c_str(), ++count);
            }

        } while (found && count < 100);


        // we could not generate a new key .... somehow.
        if (found){
            return std::make_pair("", "");
        }

        container->keys.push_back(newKey);
        container->numberValues.push_back(copyValue);

        SCP_string temp;
        sprintf(temp, "%i", copyValue);

        return std::make_pair(newKey, temp);
    }

    return std::make_pair("", "");
}

// requires a model reload anyway, so no return value.
void VariableDialogModel::shiftListItemUp(int containerIndex, int itemIndex)
{
    auto container = lookupContainer(containerIndex);
    
    // handle bogus cases;  < 1 is not a typo, since shifting the top item up should do nothing.
    if (!container || !container->list || itemIndex < 1) {
        return;
    }

    // handle itemIndex out of bounds
    if ( (container->string && itemIndex >= static_cast<int>(container->stringValues.size())) 
    ||   (!container->string && itemIndex >= static_cast<int>(container->numberValues.size())) ){
        return;
    }

    // now that we know it's going to work, just swap em.
    if (container->string) {
        std::swap(container->stringValues[itemIndex], container->stringValues[itemIndex - 1]);
    } else {
        std::swap(container->numberValues[itemIndex], container->numberValues[itemIndex - 1]);
    }
}

// requires a model reload anyway, so no return value.
void VariableDialogModel::shiftListItemDown(int containerIndex, int itemIndex)
{
    auto container = lookupContainer(containerIndex);
    
    // handle bogus cases
    if (!container || !container->list || itemIndex < 0) {
        return;
    }

    // handle itemIndex out of bounds.  -1 is necessary. since the bottom item is cannot be moved down.
    if ( (container->string && itemIndex >= static_cast<int>(container->stringValues.size()) - 1) 
    ||   (!container->string && itemIndex >= static_cast<int>(container->numberValues.size()) - 1) ){
        return;
    }

    // now that we know it's going to work, just swap em.
    if (container->string) {
        std::swap(container->stringValues[itemIndex], container->stringValues[itemIndex + 1]);
    } else {
        std::swap(container->numberValues[itemIndex], container->numberValues[itemIndex + 1]);
    }
}

// it's really because of this feature that we need data to only be in one or the other vector for maps.
// If we attempted to maintain data automatically and there was a deletion, deleting the data in
// both of the map's data vectors might be undesired, and not deleting takes the map immediately
// out of sync.  Also, just displaying both data sets would be misleading.
// We just need to tell the user that the data cannot be maintained. 
bool VariableDialogModel::removeMapItem(int index, int itemIndex)
{
    auto container = lookupContainer(index);

    if (!container){
        return false;
    }
    // container is valid.

    auto item = lookupContainerKey(index, itemIndex);

    if (!item){
        return false;
    }
    // key is valid

    // double check that we want to delete
    if (_deleteWarningCount < 3){
        SCP_string question = "Are you sure you want to delete this map item?  This can't be undone.";
        SCP_string info = "";

        if (!confirmAction(question, info)){
            --_deleteWarningCount;
            return container->deleted;
        }

        // adjust to the user's actions.  If they are deleting variable after variable, allow after a while.
        ++_deleteWarningCount;
    }


    // Now double check that we have a data value.
    if (container->string && lookupContainerStringItem(index, itemIndex)){
        container->stringValues.erase(container->stringValues.begin() + itemIndex);
    } else if (!container->string && lookupContainerNumberItem(index, itemIndex)){
        container->numberValues.erase(container->numberValues.begin() + itemIndex);
    } else {
        return false;
    }

    // if we get here, we've succeeded and it's time to erase the key and bug out
    container->keys.erase(container->keys.begin() + itemIndex);
    // "I'm outta here!"
    return true;
}

SCP_string VariableDialogModel::changeMapItemKey(int index, SCP_string oldKey, SCP_string newKey)
{
    auto container = lookupContainer(index);

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

SCP_string VariableDialogModel::changeMapItemStringValue(int index, int itemIndex, SCP_string newValue)
{
	auto item = lookupContainerStringItem(index, itemIndex);
    
	if (!item){
        return "";
    }

	*item = newValue;

    return *item;
}

void VariableDialogModel::swapKeyAndValues(int index)
{
    auto container = lookupContainer(index);

    // bogus cases
    if (!container || container->list){
        return;
    }

    // data type is the same as the key type
    if (container->string == container->stringKeys){
        // string-string is the easiest case
        if (container->string){
            std::swap(container->stringValues, container->keys);
        
        // Complicated
        } else {
            // All right, make a copy.
            SCP_vector<SCP_string> keysCopy = container->keys;

            // easy part 1
            for (int x = 0; x < static_cast<int>(container->numberValues.size()); ++x) {
                // Honestly, if we did our job correctly, this shouldn't happen, but just in case.
                if (x >= static_cast<int>(container->keys.size()) ){
                    // emplacing should be sufficient since we start at index 0.
                    container->keys.emplace_back();
                    keysCopy.emplace_back();
                }
            
                container->keys[x] = "";
                sprintf(container->keys[x], "%i", container->numberValues[x]);
            }

            // not as easy part 2
			for (int x = 0; x < static_cast<int>(keysCopy.size()); ++x) {
                if (keysCopy[x] == ""){
                    container->numberValues[x] = 0;
                } else {
                    try {
                        // why *yes* it did occur to me that I made a mistake when I designed this 
                        int temp = std::stoi(keysCopy[x]);
                        container->numberValues[x] = temp;
                    }
                    catch(...){
                        container->numberValues[x] = 0;
                    }
                }
            }
        }
    // not the same types
    } else {
        // Ok.  Because keys are always strings, it will be easier when keys are numbers, because they are underlied by strings.
        if (container->string){
            // Make a copy of the keys....
            SCP_vector<SCP_string> keysCopy = container->keys;
            // make the easy transfer from stringvalues to keys. Requiring that key values change type.
            container->keys = container->stringValues;
            container->stringKeys = true;

            for (int x = 0; x < static_cast<int>(keysCopy.size()); ++x){
                // This *is* likely to happen as these sizes were not in sync.
                if (x >= static_cast<int>(container->numberValues.size())){
                    container->numberValues.emplace_back();
                }

                try {
                    // why *yes* it did occur to me that I made a mistake when I designed this 
                    int temp = std::stoi(keysCopy[x]);
                    container->numberValues[x] = temp;
                }
                catch(...){
                    container->numberValues[x] = 0;
                }
            }

            container->string = false;

        // so here values are numbers and keys are strings.  This might actually be easier than I thought.
        } else {
            // Directly copy key strings to the string values
            container->stringValues = container->keys;

            // Transfer the number values to a temporary string, then place that string in the keys vector
            for (int x = 0; x < static_cast<int>(container->numberValues.size()); ++x){
                // Here, this shouldn't happen, but just in case.  The direct assignment above is where it could have been mis-aligned.
                if (x >= static_cast<int>(container->keys.size())){
                    container->keys.emplace_back();
                }

                sprintf(container->keys[x], "%i", container->numberValues[x]);
            }

            // change the types of the container keys and values.
            container->string = true;
            container->stringKeys = false;
        }

    }
}

SCP_string VariableDialogModel::changeMapItemNumberValue(int index, int itemIndex, int newValue)
{
	auto mapItem = lookupContainerNumberItem(index, itemIndex);

    if (!mapItem){
        return "";
    }

	*mapItem = newValue;
	
	SCP_string ret;
	sprintf(ret, "%i", newValue);
	return ret;
}


// These functions should only be called when the container is guaranteed to exist!
const SCP_vector<SCP_string>& VariableDialogModel::getMapKeys(int index) 
{
    auto container = lookupContainer(index);

    if (!container) {
		SCP_string temp;
		sprintf(temp, "getMapKeys() found that container %s does not exist.", container->name.c_str());
        throw std::invalid_argument(temp.c_str());
    }

    if (container->list) {
		SCP_string temp;
		sprintf(temp, "getMapKeys() found that container %s is not a map.", container->name.c_str());
		throw std::invalid_argument(temp);
    }

    return container->keys;
}

// Only call when the container is guaranteed to exist!
const SCP_vector<SCP_string>& VariableDialogModel::getStringValues(int index) 
{
    auto container = lookupContainer(index);

    if (!container) {
		SCP_string temp;
		sprintf(temp, "getStringValues() found that container %s does not exist.", container->name.c_str());
        throw std::invalid_argument(temp);
    }

    if (!container->string) {
		SCP_string temp;
		sprintf(temp, "getStringValues() found that container %s does not store strings.", container->name.c_str());
		throw std::invalid_argument(temp);
    }

    return container->stringValues;
}

// Only call when the container is guaranteed to exist!
const SCP_vector<int>& VariableDialogModel::getNumberValues(int index) 
{
    auto container = lookupContainer(index);

    if (!container) {
		SCP_string temp;
		sprintf(temp, "getNumberValues() found that container %s does not exist.", container->name.c_str());
		throw std::invalid_argument(temp);  
    }

    if (container->string) {
		SCP_string temp;
		sprintf(temp, "getNumberValues() found that container %s does not store numbers.", container->name.c_str());
		throw std::invalid_argument(temp);
    }

    return container->numberValues;
}

const SCP_vector<std::array<SCP_string, 3>> VariableDialogModel::getVariableValues()
{
    SCP_vector<std::array<SCP_string, 3>> outStrings;

    for (const auto& item : _variableItems) {
        SCP_string notes = "";

        if (item.deleted) {
            notes = "Deleted";
        } else if (item.originalName == "") {
            notes = "New";
        } else if (item.name != item.originalName){
            notes = "Renamed";
        } else if (item.string && item.stringValue == "") {
            notes = "Defaulting to empty string";
        }

		SCP_string temp;
		sprintf(temp, "%i", item.numberValue);
		outStrings.push_back(std::array<SCP_string, 3>{item.name, (item.string) ? item.stringValue : temp, notes});
    }

    return outStrings;
}

const SCP_vector<std::array<SCP_string, 3>> VariableDialogModel::getContainerNames()
{
    // This logic makes the string which we use to display the type of the container, based on the specific mode we're using. 
    SCP_string listPrefix;
    SCP_string listPostscript;

    SCP_string mapPrefix;
    SCP_string mapMidScript;
    SCP_string mapPostscript;

    switch (_textMode) {
        case 1: 
            listPrefix = "";
            listPostscript = " List";
            break;

        case 2: 
            listPrefix = "List (";
            listPostscript = ")";
            break;

        case 3: 
            listPrefix = "List <";
            listPostscript = ">";            
            break;
        
        case 4:
            listPrefix = "(";
            listPostscript = ")";            
            break;

        case 5:
            listPrefix = "<";
            listPostscript = ">";
            break;

        case 6:
            listPrefix = "";
            listPostscript = "";
            break;


        default:
            // this takes care of weird cases.  The logic should be simple enough to not have bugs, but just in case, switch back to default.
            _textMode = 0;
            listPrefix = "List of ";
            listPostscript = "s";
            break;
    }

    switch (_textMode) {
        case 1:
            mapPrefix = "";
            mapMidScript = "-keyed Map of ";
            mapPostscript = " Values";
       
            break;
        case 2:
            mapPrefix = "Map (";
            mapMidScript = ", ";
            mapPostscript = ")";

            break;
        case 3:
            mapPrefix = "Map <";
            mapMidScript = ", ";
            mapPostscript = ">";

            break;
        case 4:
            mapPrefix = "(";
            mapMidScript = ", ";
            mapPostscript = ")";

            break;
        case 5:
            mapPrefix = "<";
            mapMidScript = ", ";
            mapPostscript = ">";

            break;
        case 6:
            mapPrefix = "";
            mapMidScript = ", ";
            mapPostscript = "";

            break;

        default:
            _textMode = 0;
            mapPrefix = "Map with ";
            mapMidScript = " Keys and ";
            mapPostscript = " Values";

            break;
    }


    SCP_vector<std::array<SCP_string, 3>> outStrings;

    for (const auto& item : _containerItems) {
        SCP_string type = "";
        SCP_string notes = "";

        if (item.string) {
            type = "String";
        } else {
            type += "Number";
        }

        if (item.list){
            type = listPrefix + type + listPostscript;
            
        } else {

            type = mapPrefix;

            if (item.stringKeys){
                type += "String";
            } else {
                type += "Number";
            }

            type += mapMidScript;

            if (item.string){
                type += "String";
            } else {
                type += "Number";
            }

            type += mapPostscript;
        }


        if (item.deleted) {
            notes = "Deleted";
        } else if (item.originalName == "") {
            notes = "New";
        } else if (item.name != item.originalName){
            notes = "Renamed";
        }

		outStrings.push_back(std::array<SCP_string, 3>{item.name, type, notes});
    }

    return outStrings;   
}

void VariableDialogModel::setTextMode(int modeIn) { _textMode = modeIn;}

// This function is for cleaning up input strings that should be numbers.  We could use std::stoi,
// but this helps to not erase the entire string if user ends up mistyping just one digit.
// If we ever allowed float types in sexp variables ... *shudder* ... we would definitely need a float
// version of this cleanup. 
SCP_string VariableDialogModel::trimIntegerString(SCP_string source) 
{
	SCP_string ret;
    bool foundNonZero = false;
    // I was tempted to prevent exceeding the max length of the destination c-string here, but no integer 
    // can exceed the 31 digit limit. And we *will* have an integer at the end of this. 

	// filter out non-numeric digits
	std::copy_if(source.begin(), source.end(), std::back_inserter(ret),
		[&foundNonZero, &ret](char c) -> bool { 
			switch (c) {
                // ignore leading zeros.  If all digits are zero, this will be handled elsewhere
				case '0':
                    if (foundNonZero)
                        return true;
                    else
                        return false;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
                    foundNonZero = true;
					return true;
					break;
                // only copy the '-' char if it is the first thing to be copied.
                case '-':
                    if (ret.empty()){
                        return true;
                    } else {
                        return false;
                    }
				default:
                    return false;
					break;
			}
		}
	);

    // -0 as a string value is not a possible edge case because if we haven't found a digit, we don't copy zero.
    // "-" is possible and could be zero, however, and an empty string that should be zero is possible as well.
    if (ret.empty() || ret == "-"){
        ret = "0";
    }

	// here we deal with overflow values.
    try {
		// some OS's will deal with this properly.
        long test = std::stol(ret);

        if (test > INT_MAX) {
            return "2147483647";
        } else if (test < INT_MIN) {
            return "-2147483648";
        }

        return ret;
    }
    // Others will not, sadly.
	// So down here, we can still return the right overflow values if stol derped out.  Since we've already cleaned out non-digits, 
	// checking for length *really should* allow us to know if something overflowed
    catch (...){
		if (ret.size() > 9){
			if (ret[0] == '-'){
				return "-2147483648";
			} else {
				return "2147483647";
			}
		}
		
		// emergency return value
		return "0";
    }
}


} // dialogs
} // fred
} // fso
