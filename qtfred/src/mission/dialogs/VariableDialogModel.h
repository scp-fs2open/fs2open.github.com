#pragma once

#include "globalincs/pstypes.h"

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

struct variable_info {
	SCP_string name = "<unnamed>";
	bool container = false;
	bool map = false;
	bool string = true;
	
	SCP_vector<int> number_values;
	SCP_vector<string_values> string_values;
};


struct variable_or_container_info {
	SCP_string name = "<unnamed>";
	bool container = false;
	bool map = false;
	bool string = true;
	
	SCP_vector<SCP_string> keys;
	SCP_vector<int> number_values;
	SCP_vector<string_values> number_values;
};

class VariableDialogModel : public AbstractDialogModel {
public:
	VariableDialogModel(QObject* parent, EditorViewport* viewport);

	void changeSelection(SCP_string name, bool container){}
	void changeName(SCP_string oldName, SCP_string newName, bool container)

	bool apply() override;
	void reject() override;

private:
	SCP_vector<variable_or_container_info> _items;

};

} // namespace dialogs
} // namespace fred
} // namespace fso
