#include "ui/dialogs/JumpNodeEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_JumpNodeEditorDialog.h"

#include <mission/util.h>

namespace fso::fred::dialogs {

JumpNodeEditorDialog::JumpNodeEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), _viewport(viewport), ui(new Ui::JumpNodeEditorDialog()),
	  _model(new JumpNodeEditorDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	initializeUi();
	updateUi();

	connect(_model.get(), &JumpNodeEditorDialogModel::jumpNodeMarkingChanged, this, [this] {
		initializeUi();
		updateUi();
	});

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

JumpNodeEditorDialog::~JumpNodeEditorDialog() = default;

void JumpNodeEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	updateJumpNodeListComboBox();
	enableOrDisableControls();
}

void JumpNodeEditorDialog::updateJumpNodeListComboBox()
{
	ui->selectJumpNodeComboBox->clear();

	for (auto& wp : _model->getJumpNodeList()) {
		ui->selectJumpNodeComboBox->addItem(QString::fromStdString(wp.first), wp.second);
	}

	ui->selectJumpNodeComboBox->setEnabled(!_model->getJumpNodeList().empty());

	ui->selectJumpNodeComboBox->setCurrentIndex(_model->getCurrentJumpNodeIndex());
}

void JumpNodeEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	
	ui->nameLineEdit->setText(QString::fromStdString(_model->getName()));
	ui->displayNameLineEdit->setText(QString::fromStdString(_model->getDisplayName()));
	ui->modelFileLineEdit->setText(QString::fromStdString(_model->getModelFilename()));

	ui->redSpinBox->setValue(_model->getColorR());
	ui->greenSpinBox->setValue(_model->getColorG());
	ui->blueSpinBox->setValue(_model->getColorB());
	ui->alphaSpinBox->setValue(_model->getColorA());

	ui->hiddenByDefaultCheckBox->setChecked(_model->getHidden());
}

void JumpNodeEditorDialog::enableOrDisableControls()
{
	const bool enable = _model->hasValidSelection();

	ui->nameLineEdit->setEnabled(enable);
	ui->displayNameLineEdit->setEnabled(enable);
	ui->modelFileLineEdit->setEnabled(enable);
	ui->redSpinBox->setEnabled(enable);
	ui->greenSpinBox->setEnabled(enable);
	ui->blueSpinBox->setEnabled(enable);
	ui->alphaSpinBox->setEnabled(enable);
	ui->hiddenByDefaultCheckBox->setEnabled(enable);
}

void JumpNodeEditorDialog::on_selectJumpNodeComboBox_currentIndexChanged(int index)
{
	auto itemId = ui->selectJumpNodeComboBox->itemData(index).value<int>();
	_model->selectJumpNodeByListIndex(itemId);
}

void JumpNodeEditorDialog::on_nameLineEdit_editingFinished()
{
	_model->setName(ui->nameLineEdit->text().toUtf8().constData());
	updateUi(); // Update immediately in case the name change is rejected
}

void JumpNodeEditorDialog::on_displayNameLineEdit_editingFinished()
{
	_model->setDisplayName(ui->displayNameLineEdit->text().toUtf8().constData());
}

void JumpNodeEditorDialog::on_modelFileLineEdit_editingFinished()
{
	_model->setModelFilename(ui->modelFileLineEdit->text().toUtf8().constData());
	updateUi(); // Update immediately in case the name change is rejected
}

void JumpNodeEditorDialog::on_redSpinBox_valueChanged(int value)
{
	_model->setColorR(value);
}

void JumpNodeEditorDialog::on_greenSpinBox_valueChanged(int value)
{
	_model->setColorG(value);
}

void JumpNodeEditorDialog::on_blueSpinBox_valueChanged(int value)
{
	_model->setColorB(value);
}

void JumpNodeEditorDialog::on_alphaSpinBox_valueChanged(int value)
{
	_model->setColorA(value);
}

void JumpNodeEditorDialog::on_hiddenByDefaultCheckBox_toggled(bool checked)
{
	_model->setHidden(checked);
}

} // namespace fso::fred::dialogs