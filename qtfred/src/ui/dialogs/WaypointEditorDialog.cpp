#include <QtWidgets/QTextEdit>
#include "ui/dialogs/WaypointEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_WaypointEditorDialog.h"

#include <globalincs/globals.h>
#include <mission/util.h>

namespace fso::fred::dialogs {


WaypointEditorDialog::WaypointEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	ui(new Ui::WaypointEditorDialog()),
	_model(new WaypointEditorDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	ui->nameEdit->setMaxLength(NAME_LENGTH - 1);

	initializeUi();
	updateUi();

	connect(_model.get(), &WaypointEditorDialogModel::waypointPathMarkingChanged, this, [this] {
		initializeUi();
		updateUi();
	});

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

WaypointEditorDialog::~WaypointEditorDialog() = default;

void WaypointEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	ui->layerCombo->clear();
	for (const auto& name : _viewport->getLayerNames()) {
		ui->layerCombo->addItem(QString::fromStdString(name), QString::fromStdString(name));
	}

	const bool enabled = _model->hasValidSelection();
	const bool hasAny = _model->hasAnyPathsInMission();
	const bool multiSelect = _model->hasMultipleSelection();

	ui->nameEdit->setEnabled(enabled && !multiSelect);
	ui->noDrawLinesCheck->setEnabled(enabled);
	ui->customColorCheck->setEnabled(enabled);
	ui->layerCombo->setEnabled(enabled);
	ui->prevPathButton->setEnabled(hasAny);
	ui->nextPathButton->setEnabled(hasAny);

	if (multiSelect) {
		setWindowTitle(QString("Edit %1 Waypoint Paths").arg(_model->getSelectionCount()));
	} else {
		setWindowTitle("Waypoint Path Editor");
	}
}

void WaypointEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
	ui->layerCombo->setCurrentIndex(ui->layerCombo->findData(QString::fromStdString(_model->getLayer())));

	ui->noDrawLinesCheck->setChecked(_model->getNoDrawLines());
	ui->customColorCheck->setChecked(_model->getHasCustomColor());
	ui->colorRSpinBox->setValue(_model->getColorR());
	ui->colorGSpinBox->setValue(_model->getColorG());
	ui->colorBSpinBox->setValue(_model->getColorB());

	const bool colorEnabled = _model->hasValidSelection() && _model->getHasCustomColor();
	ui->colorRSpinBox->setEnabled(colorEnabled);
	ui->colorGSpinBox->setEnabled(colorEnabled);
	ui->colorBSpinBox->setEnabled(colorEnabled);

	updateColorSwatch();
}

void WaypointEditorDialog::updateColorSwatch()
{
	ui->colorSwatch->setStyleSheet(QString("background: rgb(%1,%2,%3);"
	                                       "border: 1px solid #444; border-radius: 3px;")
	        .arg(_model->getColorR())
	        .arg(_model->getColorG())
	        .arg(_model->getColorB()));
}

void WaypointEditorDialog::on_prevPathButton_clicked()
{
	_model->selectPreviousPath();
}

void WaypointEditorDialog::on_nextPathButton_clicked()
{
	_model->selectNextPath();
}

void WaypointEditorDialog::on_nameEdit_editingFinished()
{
	if (!_model->setCurrentName(ui->nameEdit->text().toUtf8().constData())) {
		util::SignalBlockers blockers(this);
		ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
	}
}

void WaypointEditorDialog::on_noDrawLinesCheck_toggled(bool checked)
{
	_model->setNoDrawLines(checked);
}

void WaypointEditorDialog::on_customColorCheck_toggled(bool checked)
{
	_model->setHasCustomColor(checked);
	const bool colorEnabled = checked;
	ui->colorRSpinBox->setEnabled(colorEnabled);
	ui->colorGSpinBox->setEnabled(colorEnabled);
	ui->colorBSpinBox->setEnabled(colorEnabled);
	updateColorSwatch();
}

void WaypointEditorDialog::on_colorRSpinBox_valueChanged(int value)
{
	_model->setColorR(value);
	updateColorSwatch();
}

void WaypointEditorDialog::on_colorGSpinBox_valueChanged(int value)
{
	_model->setColorG(value);
	updateColorSwatch();
}

void WaypointEditorDialog::on_colorBSpinBox_valueChanged(int value)
{
	_model->setColorB(value);
	updateColorSwatch();
}

void WaypointEditorDialog::on_layerCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	_model->setLayer(ui->layerCombo->itemData(index).toString().toUtf8().constData());
}

} // namespace fso::fred::dialogs
