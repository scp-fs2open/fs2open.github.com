#include "BackgroundEditorDialog.h"

#include "ui_BackgroundEditor.h"

namespace fso {
namespace fred {
namespace dialogs {

BackgroundEditorDialog::BackgroundEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::BackgroundEditor()),
	_viewport(viewport) {
    ui->setupUi(this);
	
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

BackgroundEditorDialog::~BackgroundEditorDialog() {
}

}
}
}
