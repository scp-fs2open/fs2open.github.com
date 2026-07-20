#include "ui/dialogs/JumpNodeEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_JumpNodeEditorDialog.h"

#include <globalincs/globals.h>
#include <jumpnode/jumpnode.h>
#include <mission/util.h>
#include <ui/util/menu.h>

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

	// -1 is the "mixed selection" sentinel... shown as blank via specialValueText.
	for (auto* sb : {ui->redSpinBox, ui->greenSpinBox, ui->blueSpinBox, ui->alphaSpinBox}) {
		sb->setMinimum(-1);
		sb->setSpecialValueText(" ");
	}

	initializeUi();
	updateUi();

	connect(_model.get(), &JumpNodeEditorDialogModel::jumpNodeMarkingChanged, this, [this] {
		initializeUi();
		updateUi();
	});

	// "Select Jump Node" menu: jump the editor to any jump node in the mission.
	Editor* editor = viewport->editor;
	util::installSelectMenu(
		this,
		[]() {
			std::vector<util::SelectMenuEntry> entries;
			for (const auto& jn : Jump_nodes) {
				entries.push_back({QString::fromUtf8(jn.GetName()), jn.GetSCPObjectNumber()});
			}
			return entries;
		},
		[this, editor]() { return _model->hasMultipleSelection() ? -1 : editor->currentObject; },
		[editor](int objnum) {
			editor->unmark_all();
			editor->selectObject(objnum);
		},
		tr("&Select Jump Node"));

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

	ui->redSpinBox->setValue(_model->isColorRMixed() ? -1 : _model->getColorR());
	ui->greenSpinBox->setValue(_model->isColorGMixed() ? -1 : _model->getColorG());
	ui->blueSpinBox->setValue(_model->isColorBMixed() ? -1 : _model->getColorB());
	ui->alphaSpinBox->setValue(_model->isColorAMixed() ? -1 : _model->getColorA());

	const int hiddenState = _model->getHiddenState();
	ui->hiddenByDefaultCheckBox->setTristate(hiddenState == Qt::PartiallyChecked);
	ui->hiddenByDefaultCheckBox->setCheckState(static_cast<Qt::CheckState>(hiddenState));

	ui->layerCombo->setCurrentIndex(ui->layerCombo->findData(QString::fromStdString(_model->getLayer())));

	updateColorSwatch();
}

void JumpNodeEditorDialog::updateColorSwatch()
{
	if (_model->hasAnyColorMixed()) {
		// Mixed selection: render a neutral patterned swatch with a "?".
		ui->colorSwatch->setText("?");
		ui->colorSwatch->setAlignment(Qt::AlignCenter);
		ui->colorSwatch->setStyleSheet("background: #888; color: white;"
		                               "border: 1px solid #444; border-radius: 3px;");
		return;
	}
	ui->colorSwatch->setText("");
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

void JumpNodeEditorDialog::on_hiddenByDefaultCheckBox_clicked()
{
	// clicked() is used (not toggled()) so a click on a tri-state PartiallyChecked
	// box still routes here. Read the post-click state from the widget itself.
	_model->setHidden(ui->hiddenByDefaultCheckBox->isChecked());
}

void JumpNodeEditorDialog::on_layerCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	_model->setLayer(ui->layerCombo->itemData(index).toString().toUtf8().constData());
}

} // namespace fso::fred::dialogs
