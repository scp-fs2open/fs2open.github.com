#pragma once

#include "globalincs/pstypes.h"
#include "AbstractDialogModel.h"
#include "parse/sexp_container.h"

namespace fso::fred::dialogs {

struct VariableInfo {
	SCP_string name = "<unnamed>";
	SCP_string originalName;
	bool is_string = true;
	int flags = 0;
	int numberValue = 0;
	SCP_string stringValue;
};

struct ContainerInfo {
	SCP_string name = "<unnamed>";
	SCP_string originalName;
	bool is_list = true;
	bool values_are_strings = true;
	bool keys_are_strings = true;
	int flags = 0;

	SCP_vector<SCP_string> keys;
	SCP_vector<int> numberValues;
	SCP_vector<SCP_string> stringValues;
};

class VariableDialogModel : public AbstractDialogModel {
  public:
	VariableDialogModel(QObject* parent, EditorViewport* viewport);

	// Main lifecycle methods
	bool apply() override;
	void reject() override;

	// High Level Getters
	const SCP_vector<VariableInfo>& getVariables() const;
	const SCP_vector<ContainerInfo>& getContainers() const;

	// Variables
	bool variableListHasSpace() const;
	void addNewVariable();
	void copyVariable(int index);
	void markVariableForDeletion(int index);
	bool isVariableNameUnique(const SCP_string& name, int variableIndex) const;
	SCP_string getVariableName(int index) const;
	void setVariableName(int index, const SCP_string& newName);
	bool getVariableType(int index) const;
	void setVariableType(int index, bool is_string);
	SCP_string getVariableValue(int index) const;
	void setVariableValue(int index, const SCP_string& value);
	bool getVariableNetwork(int index) const;
	void setVariableNetwork(int index, bool enabled);
	bool getVariableEternal(int index) const;
	void setVariableEternal(int index, bool enabled);
	int getVariablePersistenceType(int index) const;
	void setVariablePersistenceType(int index, int type);

	// Containers
	void addNewContainer();
	void copyContainer(int index);
	void markContainerForDeletion(int index);
	bool isContainerNameUnique(const SCP_string& name, int containerIndex) const;
	bool isContainerEmpty(int index) const;
	SCP_string getContainerName(int index) const;
	void setContainerName(int index, const SCP_string& newName);
	bool getContainerType(int index) const;
	void setContainerType(int index, bool is_list);
	bool getContainerValueType(int index) const;
	void setContainerValueType(int index, bool values_are_strings);
	bool getContainerKeyType(int index) const;
	void setContainerKeyType(int index, bool keys_are_strings);
	bool getContainerNetwork(int index) const;
	void setContainerNetwork(int index, bool enabled);
	bool getContainerEternal(int index) const;
	void setContainerEternal(int index, bool enabled);
	int getContainerPersistenceType(int index) const;
	void setContainerPersistenceType(int index, int type);

	// Container Lists
	void addListItem(int containerIndex);
	void removeListItem(int containerIndex, int itemIndex);
	SCP_string getListItemValue(int containerIndex, int itemIndex) const;
	void setListItemValue(int containerIndex, int itemIndex, const SCP_string& value);
	void moveListItem(int containerIndex, int itemIndex, bool up);
	int copyListItem(int containerIndex, int itemIndex);

	// Container Maps
	void addMapItem(int containerIndex);
	void removeMapItem(int containerIndex, int itemIndex);
	SCP_string getMapItemKey(int containerIndex, int itemIndex) const;
	void setMapItemKey(int containerIndex, int itemIndex, const SCP_string& key);
	SCP_string getMapItemValue(int containerIndex, int itemIndex) const;
	void setMapItemValue(int containerIndex, int itemIndex, const SCP_string& value);
	void swapMapKeysAndValues(int containerIndex);

  private:
	SCP_vector<VariableInfo> m_variables;
	SCP_vector<ContainerInfo> m_containers;

	SCP_vector<SCP_string> m_deleted_variables;

	void initializeData();

	static sexp_container createSexpContainerFromInfo(const ContainerInfo& info);

	// Helper for converting user string input to a valid integer string
	static SCP_string trimIntegerString(const SCP_string& source);

	void sortMap(int containerIndex);

	bool checkValidity();
};

} // namespace fso::fred::dialogs