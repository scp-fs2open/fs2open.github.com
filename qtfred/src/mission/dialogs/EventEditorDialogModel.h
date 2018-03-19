#pragma once

#include "AbstractDialogModel.h"

#include <mission/missionmessage.h>
#include <mission/missiongoals.h>

namespace fso {
namespace fred {
namespace dialogs {

class EventEditorDialogModel: public AbstractDialogModel {
 Q_OBJECT
 public:
	EventEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;
};

}
}
}


