#include "ui/dialogs/WaypointPathGeneratorDialog.h"
#include "mission/commands/FredCommands.h"
#include "ui/util/SignalBlockers.h"
#include "ui_WaypointPathGeneratorDialog.h"

#include <object/waypoint.h>

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QFocusEvent>
#include <QPushButton>

namespace fso::fred::dialogs {

WaypointPathGeneratorDialog::WaypointPathGeneratorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), _viewport(viewport), _fredView(parent), ui(new Ui::WaypointPathGeneratorDialog()),
	  _model(new WaypointPathGeneratorDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);

	initializeUi();

	resize(QDialog::sizeHint());
}

void WaypointPathGeneratorDialog::accept()
{
	// Not called directly — on_buttonBox_accepted handles the full undo wiring.
	// This override exists so closeEvent can funnel through reject() cleanly.
	_dialogStack->clear();
	_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	QDialog::accept();
}

void WaypointPathGeneratorDialog::reject()
{
	if (_model) {
		_model->reject();
	}
	_dialogStack->clear();
	_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	QDialog::reject();
}

void WaypointPathGeneratorDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore();
}

void WaypointPathGeneratorDialog::focusInEvent(QFocusEvent* e)
{
	_fredView->undoGroup()->setActiveStack(_dialogStack);
	QDialog::focusInEvent(e);
}

WaypointPathGeneratorDialog::~WaypointPathGeneratorDialog() = default;

void WaypointPathGeneratorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	// Rename OK button to "Generate"
	if (auto* btn = ui->buttonBox->button(QDialogButtonBox::Ok)) {
		btn->setText("Generate");
	}

	// Axis combo
	ui->axisComboBox->clear();
	ui->axisComboBox->addItem("XZ (default)");
	ui->axisComboBox->addItem("XY");
	ui->axisComboBox->addItem("ZY");

	// Scene objects combo
	ui->centerObjectCombo->clear();
	for (auto& obj : _model->getSceneObjects()) {
		ui->centerObjectCombo->addItem(QString::fromStdString(obj.first), obj.second);
	}

	updateUi();
	enableOrDisableControls();
}

void WaypointPathGeneratorDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	ui->pathNameLineEdit->setText(QString::fromStdString(_model->getPathName()));

	bool useObj = _model->getUseObjectCenter();
	ui->manualCenterRadio->setChecked(!useObj);
	ui->objectCenterRadio->setChecked(useObj);

	ui->centerXSpinBox->setValue(static_cast<double>(_model->getCenterX()));
	ui->centerYSpinBox->setValue(static_cast<double>(_model->getCenterY()));
	ui->centerZSpinBox->setValue(static_cast<double>(_model->getCenterZ()));

	// Sync object combo selection
	int objnum = _model->getCenterObjectObjnum();
	for (int i = 0; i < ui->centerObjectCombo->count(); ++i) {
		if (ui->centerObjectCombo->itemData(i).value<int>() == objnum) {
			ui->centerObjectCombo->setCurrentIndex(i);
			break;
		}
	}

	ui->axisComboBox->setCurrentIndex(static_cast<int>(_model->getAxis()));
	ui->numPointsSpinBox->setValue(_model->getNumPoints());
	ui->loopsSpinBox->setValue(_model->getLoops());
	ui->radiusSpinBox->setValue(static_cast<double>(_model->getRadius()));
	ui->driftSpinBox->setValue(static_cast<double>(_model->getDrift()));

	ui->varianceXSpinBox->setValue(static_cast<double>(_model->getVarianceX()));
	ui->varianceYSpinBox->setValue(static_cast<double>(_model->getVarianceY()));
	ui->varianceZSpinBox->setValue(static_cast<double>(_model->getVarianceZ()));
}

void WaypointPathGeneratorDialog::enableOrDisableControls()
{
	bool useObj = _model->getUseObjectCenter();

	ui->centerXSpinBox->setEnabled(!useObj);
	ui->centerYSpinBox->setEnabled(!useObj);
	ui->centerZSpinBox->setEnabled(!useObj);

	bool hasObjects = !_model->getSceneObjects().empty();
	ui->centerObjectCombo->setEnabled(useObj && hasObjects);
	if (useObj && !hasObjects) {
		ui->objectCenterRadio->setEnabled(false);
	} else {
		ui->objectCenterRadio->setEnabled(true);
	}
}

void WaypointPathGeneratorDialog::on_pathNameLineEdit_editingFinished()
{
	_model->setPathName(ui->pathNameLineEdit->text().toUtf8().constData());
}

void WaypointPathGeneratorDialog::on_manualCenterRadio_toggled(bool checked)
{
	if (checked) {
		_model->setUseObjectCenter(false);
		enableOrDisableControls();
	}
}

void WaypointPathGeneratorDialog::on_objectCenterRadio_toggled(bool checked)
{
	if (checked) {
		_model->setUseObjectCenter(true);
		// If there are scene objects and none is selected yet, select the first
		if (!_model->getSceneObjects().empty() && _model->getCenterObjectObjnum() < 0) {
			auto objnum = ui->centerObjectCombo->itemData(0).value<int>();
			_model->setCenterObjectObjnum(objnum);
			util::SignalBlockers blockers(this);
			ui->centerObjectCombo->setCurrentIndex(0);
		}
		enableOrDisableControls();
	}
}

void WaypointPathGeneratorDialog::on_centerObjectCombo_currentIndexChanged(int index)
{
	if (index >= 0) {
		auto objnum = ui->centerObjectCombo->itemData(index).value<int>();
		_model->setCenterObjectObjnum(objnum);
	}
}

void WaypointPathGeneratorDialog::on_centerXSpinBox_valueChanged(double value)
{
	_model->setCenterX(static_cast<float>(value));
}

void WaypointPathGeneratorDialog::on_centerYSpinBox_valueChanged(double value)
{
	_model->setCenterY(static_cast<float>(value));
}

void WaypointPathGeneratorDialog::on_centerZSpinBox_valueChanged(double value)
{
	_model->setCenterZ(static_cast<float>(value));
}

void WaypointPathGeneratorDialog::on_axisComboBox_currentIndexChanged(int index)
{
	_model->setAxis(static_cast<GeneratorAxis>(index));
}

void WaypointPathGeneratorDialog::on_numPointsSpinBox_valueChanged(int value)
{
	_model->setNumPoints(value);
}

void WaypointPathGeneratorDialog::on_loopsSpinBox_valueChanged(int value)
{
	_model->setLoops(value);
}

void WaypointPathGeneratorDialog::on_radiusSpinBox_valueChanged(double value)
{
	_model->setRadius(static_cast<float>(value));
}

void WaypointPathGeneratorDialog::on_driftSpinBox_valueChanged(double value)
{
	_model->setDrift(static_cast<float>(value));
}

void WaypointPathGeneratorDialog::on_varianceXSpinBox_valueChanged(double value)
{
	_model->setVarianceX(static_cast<float>(value));
}

void WaypointPathGeneratorDialog::on_varianceYSpinBox_valueChanged(double value)
{
	_model->setVarianceY(static_cast<float>(value));
}

void WaypointPathGeneratorDialog::on_varianceZSpinBox_valueChanged(double value)
{
	_model->setVarianceZ(static_cast<float>(value));
}

void WaypointPathGeneratorDialog::on_buttonBox_accepted()
{
	if (_model->apply()) {
		// Collect exact positions from the just-created path (always appended last).
		SCP_string pathName = _model->getPathName();
		SCP_vector<vec3d> positions;
		if (!Waypoint_lists.empty()) {
			for (const auto& wp : Waypoint_lists.back().get_waypoints())
				positions.push_back(*wp.get_pos());
		}
		_fredView->mainUndoStack()->push(
			new GenerateWaypointPathCommand(pathName, std::move(positions),
			                               _viewport->editor,
			                               tr("Generate Waypoint Path")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void WaypointPathGeneratorDialog::on_buttonBox_rejected()
{
	reject();
}

} // namespace fso::fred::dialogs
