#pragma once

#include "AbstractDialogModel.h"
namespace fso {
namespace fred {
namespace dialogs {

class EventEditorDialogModel: public AbstractDialogModel {
 Q_OBJECT
 public:
	EventEditorDialogModel(QObject* parent, Editor* editor, FredRenderer* renderer);

	void applyChanges();
	void discardChanges();
};

}
}
}


