//
//

#include "EventEditorDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

EventEditorDialogModel::EventEditorDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
}
bool EventEditorDialogModel::apply() {
	return true;
}
void EventEditorDialogModel::reject() {
}
}
}
}

