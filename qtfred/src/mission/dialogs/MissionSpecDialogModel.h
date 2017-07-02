#pragma once

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class MissionSpecDialogModel : public AbstractDialogModel {
private:
	SCP_string designer_name;

public:
	MissionSpecDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	SCP_string getDesigner();
	void setDesigner(SCP_string designer_name);

	bool query_modified();
};

}
}
}