//
//

#include "EventEditorDialog.h"
#include "ui_EventEditorDialog.h"

namespace fso {
namespace fred {
namespace dialogs {

EventEditorDialog::EventEditorDialog(QWidget* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::EventEditorDialog()), _model(new EventEditorDialogModel(this, viewport)) {
	ui->setupUi(this);

	connect(this, &QDialog::accepted, _model.get(), &EventEditorDialogModel::applyChanges);
	connect(this, &QDialog::rejected, _model.get(), &EventEditorDialogModel::discardChanges);
}
EventEditorDialog::~EventEditorDialog() {
}

}
}
}

