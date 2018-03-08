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

	ui->eventTree->initializeEditor(viewport->editor);

	connect(this, &QDialog::accepted, _model.get(), &EventEditorDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &EventEditorDialogModel::reject);
}
EventEditorDialog::~EventEditorDialog() {
}

}
}
}

