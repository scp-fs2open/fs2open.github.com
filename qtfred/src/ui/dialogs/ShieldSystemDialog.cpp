#include "ShieldSystemDialog.h"

#include "ui_ShieldSystemDialog.h"


namespace fso {
namespace fred {
namespace dialogs {

ShieldSystemDialog::ShieldSystemDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::ShieldSystemDialog()),
	_viewport(viewport) {
    ui->setupUi(this);
	
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShieldSystemDialog::~ShieldSystemDialog() {
}

}
}
}
