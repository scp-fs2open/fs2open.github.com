#pragma once

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class EventEditorDialogModel: public AbstractDialogModel {
 Q_OBJECT
 public:
	EventEditorDialogModel(QObject* parent, EditorViewport* viewport);

	void applyChanges();
	void discardChanges();
};

}
}
}


