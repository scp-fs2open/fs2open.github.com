#pragma once

#include "globalincs/pstypes.h"

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

struct variable_info {
	SCP_string name = "<unnamed>";
	bool string = true;
	int flags = 0;
	int number_value;
	SCP_string string_value;
};


struct container_info {
	SCP_string name = "<unnamed>";
	bool map = false;
	bool string = true;
	int flags = 0;
	
	SCP_vector<SCP_string> keys;
	SCP_vector<int> number_values;
	SCP_vector<SCP_string> number_values;
};

class VariableDialogModel : public AbstractDialogModel {
public:
	VariableDialogModel(QObject* parent, EditorViewport* viewport);

	void changeSelection(SCP_string name, bool container){}
	void changeName(SCP_string oldName, SCP_string newName, bool container)

	bool apply() override;
	void reject() override;

private:
	SCP_vector<variable_info> _variableItems;
	SCP_vector<container_info> _containerItems;
};

} // namespace dialogs
} // namespace fred
} // namespace fso
