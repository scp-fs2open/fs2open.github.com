//
//

#include "FormWingDialog.h"

#include "ui_FormWingDialog.h"

namespace fso {
namespace fred {
namespace dialogs {

FormWingDialog::FormWingDialog(QWidget* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::FormWingDialog), _model(new FormWingDialogModel(this, viewport)) {
	ui->setupUi(this);
	ui->nameEdit->setMaxLength(NAME_LENGTH - 4);

	connect(_model.get(), &FormWingDialogModel::modelChanged, this, &FormWingDialog::updateUI);

	connect(ui->nameEdit, &QLineEdit::textChanged, this, &FormWingDialog::nameTextChanged);

	resize(sizeHint());
}
FormWingDialog::~FormWingDialog() {

}
void FormWingDialog::updateUI() {
	ui->nameEdit->setText(QString::fromStdString(_model->getName()));
}
void FormWingDialog::nameTextChanged(const QString& newText) {
	_model->setName(newText.toStdString());
}
FormWingDialogModel* FormWingDialog::getModel() {
	return _model.get();
}

}
}
}
