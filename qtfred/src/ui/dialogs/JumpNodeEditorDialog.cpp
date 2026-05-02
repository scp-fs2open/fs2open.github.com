#include "ui/dialogs/JumpNodeEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_JumpNodeEditorDialog.h"

#include <globalincs/globals.h>
#include <mission/util.h>

namespace fso::fred::dialogs {

JumpNodeEditorDialog::JumpNodeEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), _viewport(viewport), ui(new Ui::JumpNodeEditorDialog()),
	  _model(new JumpNodeEditorDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	ui->nameLineEdit->setMaxLength(NAME_LENGTH - 1);
	ui->displayNameLineEdit->setMaxLength(NAME_LENGTH - 1);
	ui->modelFileLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);

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

	ui->layerCombo->clear();
	for (const auto& name : _viewport->getLayerNames()) {
		ui->layerCombo->addItem(QString::fromStdString(name), QString::fromStdString(name));
	}

	const bool enabled = _model->hasValidSelection();
	const bool hasAny = _model->hasAnyNodesInMission();
	const bool multiSelect = _model->hasMultipleSelection();

	ui->nameLineEdit->setEnabled(enabled && !multiSelect);
	ui->displayNameLineEdit->setEnabled(enabled);
	ui->modelFileLineEdit->setEnabled(enabled);
	ui->redSpinBox->setEnabled(enabled);
	ui->greenSpinBox->setEnabled(enabled);
	ui->blueSpinBox->setEnabled(enabled);
	ui->alphaSpinBox->setEnabled(enabled);
	ui->hiddenByDefaultCheckBox->setEnabled(enabled);
	ui->layerCombo->setEnabled(enabled);
	ui->prevNodeButton->setEnabled(hasAny);
	ui->nextNodeButton->setEnabled(hasAny);

	if (multiSelect) {
		setWindowTitle(QString("Edit %1 Jump Nodes").arg(_model->getSelectionCount()));
	} else {
		setWindowTitle("Jump Node Editor");
	}
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

	ui->layerCombo->setCurrentIndex(ui->layerCombo->findData(QString::fromStdString(_model->getLayer())));

	updateColorSwatch();
}

void JumpNodeEditorDialog::updateColorSwatch()
{
	ui->colorSwatch->setStyleSheet(QString("background: rgba(%1,%2,%3,%4);"
	                                       "border: 1px solid #444; border-radius: 3px;")
	        .arg(_model->getColorR())
	        .arg(_model->getColorG())
	        .arg(_model->getColorB())
	        .arg(_model->getColorA()));
}

void JumpNodeEditorDialog::on_prevNodeButton_clicked()
{
	_model->selectPreviousNode();
}

void JumpNodeEditorDialog::on_nextNodeButton_clicked()
{
	_model->selectNextNode();
}

void JumpNodeEditorDialog::on_nameLineEdit_editingFinished()
{
	if (!_model->setName(ui->nameLineEdit->text().toUtf8().constData())) {
		util::SignalBlockers blockers(this);
		ui->nameLineEdit->setText(QString::fromStdString(_model->getName()));
	}
}

void JumpNodeEditorDialog::on_displayNameLineEdit_editingFinished()
{
	if (!_model->setDisplayName(ui->displayNameLineEdit->text().toUtf8().constData())) {
		util::SignalBlockers blockers(this);
		ui->displayNameLineEdit->setText(QString::fromStdString(_model->getDisplayName()));
	}
}

void JumpNodeEditorDialog::on_modelFileLineEdit_editingFinished()
{
	if (!_model->setModelFilename(ui->modelFileLineEdit->text().toUtf8().constData())) {
		util::SignalBlockers blockers(this);
		ui->modelFileLineEdit->setText(QString::fromStdString(_model->getModelFilename()));
	}
}

void JumpNodeEditorDialog::on_redSpinBox_valueChanged(int value)
{
	_model->setColorR(value);
	updateColorSwatch();
}

void JumpNodeEditorDialog::on_greenSpinBox_valueChanged(int value)
{
	_model->setColorG(value);
	updateColorSwatch();
}

void JumpNodeEditorDialog::on_blueSpinBox_valueChanged(int value)
{
	_model->setColorB(value);
	updateColorSwatch();
}

void JumpNodeEditorDialog::on_alphaSpinBox_valueChanged(int value)
{
	_model->setColorA(value);
	updateColorSwatch();
}

void JumpNodeEditorDialog::on_hiddenByDefaultCheckBox_toggled(bool checked)
{
	_model->setHidden(checked);
}

void JumpNodeEditorDialog::on_layerCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	_model->setLayer(ui->layerCombo->itemData(index).toString().toUtf8().constData());
}

} // namespace fso::fred::dialogs
