#pragma once

#include "globalincs/pstypes.h"

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

struct variableInfo {
	SCP_string name = "<unnamed>";
	SCP_string original_name = "";
	bool deleted = false;
	bool string = true;
	int flags = 0;
	int numberValue = 0;
	SCP_string stringValue = "";
};


struct containerInfo {
	SCP_string name = "<unnamed>";
	SCP_string original_name = "";
	bool deleted = false;
	bool list = false;
	bool string = true;
	int flags = 0;

	SCP_vector<SCP_string> keys;
	SCP_vector<int> numberValues;
	SCP_vector<SCP_string> stringValues;
};

class VariableDialogModel : public AbstractDialogModel {
public:
	VariableDialogModel(QObject* parent, EditorViewport* viewport);

	// true on string, false on number
	bool getVariableType(SCP_string name);
	bool getVariableNetworkStatus(SCP_string name);
	// 0 neither, 1 on mission complete, 2 on mission close (higher number saves more often)
	int getVariableOnMissionCloseOrCompleteFlag(SCP_string name);
	bool getVariableEternalFlag(SCP_string name);

	SCP_string getVariableStringValue(SCP_string name);
	int getVariableNumberValue(SCP_string name);

	// !! Note an innovation: when getting a request to set a value, 
	// this model will return the value that sticks and then will overwrite
	// the value in the dialog.  This means that we don't have to have the UI 
	// repopulate the whole editor on each change.

	// true on string, false on number
	bool setVariableType(SCP_string name, bool string);
	bool setVariableNetworkStatus(SCP_string name, bool network);
	int setVariableOnMissionCloseOrCompleteFlag(SCP_string name, int flags);
	bool setVariableEternalFlag(SCP_string name, bool eternal);

	SCP_string setVariableStringValue(SCP_string name, SCP_string value);
	int setVariableNumberValue(SCP_string name, int value);

	SCP_string addNewVariable();
	SCP_string changeVariableName(SCP_string oldName, SCP_string newName);
	SCP_string copyVariable(SCP_string name);
	// returns whether it succeeded
	bool removeVariable(SCP_string name);

	// Container Section

	// true on string, false on number
	bool getContainerValueType(SCP_string name);
	// true on list, false on map
	bool getContainerListOrMap(SCP_string name);
	bool getContainerNetworkStatus(SCP_string name);
	// 0 neither, 1 on mission complete, 2 on mission close (higher number saves more often)
	int getContainerOnMissionCloseOrCompleteFlag(SCP_string name);
	bool getContainerEternalFlag(SCP_string name);

	bool setContainerValueType(SCP_string name, bool type);
	bool setContainerListOrMap(SCP_string name, bool list);
	bool setContainerNetworkStatus(SCP_string name, bool network);
	int setContainerOnMissionCloseOrCompleteFlag(SCP_string name, int flags);
	bool setContainerEternalFlag(SCP_string name, bool eternal);

	SCP_string addContainer();
	SCP_string changeContainerName(SCP_string oldName, SCP_string newName);
	bool removeContainer(SCP_string name);

	SCP_string addListItem(SCP_string containerName);

	SCP_string copyListItem(SCP_string containerName, int index);
	bool removeListItem(SCP_string containerName, int index);

	SCP_string addMapItem(SCP_string ContainerName);
	SCP_string copyMapItem(SCP_string containerName, SCP_string key);
	bool removeMapItem(SCP_string containerName, SCP_string key);

	SCP_string replaceMapItemKey(SCP_string containerName, SCP_string oldKey, SCP_string newKey);
	SCP_string changeMapItemStringValue(SCP_string containerName, SCP_string key, SCP_string newValue);
	SCP_string changeMapItemNumberValue(SCP_string containerName, SCP_string key, int newValue);
	

	bool apply() override;
	void reject() override;

private:
	SCP_vector<variableInfo> _variableItems;
	SCP_vector<containerInfo> _containerItems;

	const variableInfo* lookupVariable(SCP_string name){
		for (int x = 0; x < static_cast<int>(_varaibleItems.size()); ++x){
			if (_varaibleItems[x].name == name){
				return &_varaibleItems[x];
			}
		}

		return nullptr;
	}

	const containerInfo* lookupContainer(SCP_string name){
		for (int x = 0; x < static_cast<int>(_containerItems.size()); ++x){
			if (_containerItems[x].name == name){
				return &_containerItems[x];
			}
		}

		return nullptr;
	}

	// many of the controls in this editor can lead to drastic actions, so this will be very useful.
	const bool confirmAction(SCP_string question, SCP_string informativeText)
	{
	QMessageBox msgBox;
	msgBox.setText(question.c_str());
	msgBox.setInformativeText(informativeText.C_str());
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	int ret = msgBox.exec();

	switch (ret) {
		case QMessageBox::Yes:
			return true;
			break;
		case QMessageBox::Cancel:
			return false;
			break;
		default:
			UNREACHABLE("Bad return value from confirmation message box in the Loadout dialog editor.");
			break;
		}
	}
};

} // namespace dialogs
} // namespace fred
} // namespace fso

