//
//

#include "ObjectOrientEditorDialog.h"

#include "ui_ObjectOrientationDialog.h"

#include <ui/util/SignalBlockers.h>
#include "mission/util.h"
#include <QCloseEvent>

namespace fso::fred::dialogs {

ObjectOrientEditorDialog::ObjectOrientEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::ObjectOrientEditorDialog()), _model(new ObjectOrientEditorDialogModel(this, viewport)),
	_viewport(viewport) {
	this->setFocus();
	ui->setupUi(this);

	// set our internal values, update the UI
	initializeUi();
	updateUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}
ObjectOrientEditorDialog::~ObjectOrientEditorDialog() = default;

void ObjectOrientEditorDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void ObjectOrientEditorDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void ObjectOrientEditorDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void ObjectOrientEditorDialog::initializeUi()
{
	updateComboBox();

	if (_model->getPointToObjectList().empty()) {
		_model->setPointMode(ObjectOrientEditorDialogModel::PointToMode::Location);
	}
}

void ObjectOrientEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	updatePosition();
	updateOrientation();
	updatePointTo();
	updateLocation();

	enableOrDisableControls();
}

void ObjectOrientEditorDialog::enableOrDisableControls()
{
	ui->orientationGroupBox->setEnabled(!_model->getPointTo() && _model->isOrientationEnabledForType());
	ui->pointToGroupBox->setEnabled(_model->getPointTo() && _model->isOrientationEnabledForType());
	ui->transformSettingsGroupBox->setEnabled(_model->getNumObjectsMarked() > 1 && _model->isOrientationEnabledForType());

	bool enableLocation = _model->getPointTo() && _model->getPointMode() == ObjectOrientEditorDialogModel::PointToMode::Location;
	bool noEntries = _model->getPointToObjectList().empty();
	bool enableObject = _model->getPointTo() && !noEntries && _model->getPointMode() == ObjectOrientEditorDialogModel::PointToMode::Object;

	ui->objectRadioButton->setEnabled(!noEntries);
	ui->objectComboBox->setEnabled(enableObject);
	ui->locationXSpinBox->setEnabled(enableLocation);
	ui->locationYSpinBox->setEnabled(enableLocation);
	ui->locationZSpinBox->setEnabled(enableLocation);
}

void ObjectOrientEditorDialog::updatePosition()
{
	util::SignalBlockers blockers(this);

	ui->positionXSpinBox->setValue(_model->getPosition().x);
	ui->positionYSpinBox->setValue(_model->getPosition().y);
	ui->positionZSpinBox->setValue(_model->getPosition().z);
}

void ObjectOrientEditorDialog::updateOrientation()
{
	util::SignalBlockers blockers(this);

	ui->orientationPSpinBox->setValue(_model->getOrientation().p);
	ui->orientationBSpinBox->setValue(_model->getOrientation().b);
	ui->orientationHSpinBox->setValue(_model->getOrientation().h);
}

void ObjectOrientEditorDialog::updatePointTo()
{
	util::SignalBlockers blockers(this);

	ui->pointToCheckBox->setChecked(_model->getPointTo());
	ui->objectRadioButton->setChecked(_model->getPointMode() == ObjectOrientEditorDialogModel::PointToMode::Object);
	ui->locationRadioButton->setChecked(_model->getPointMode() == ObjectOrientEditorDialogModel::PointToMode::Location);
}

void ObjectOrientEditorDialog::updateComboBox()
{
	util::SignalBlockers blockers(this);

	ui->objectComboBox->clear();

	for (auto& entry : _model->getPointToObjectList()) {
		ui->objectComboBox->addItem(QString::fromStdString(entry.name), QVariant(entry.objIndex));
	}
	ui->objectComboBox->setCurrentIndex(ui->objectComboBox->findData(_model->getPointToObjectIndex()));
}

void ObjectOrientEditorDialog::updateLocation()
{
	util::SignalBlockers blockers(this);

	ui->locationXSpinBox->setValue(_model->getLocation().x);
	ui->locationYSpinBox->setValue(_model->getLocation().y);
	ui->locationZSpinBox->setValue(_model->getLocation().z);
}

void ObjectOrientEditorDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void ObjectOrientEditorDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void ObjectOrientEditorDialog::on_positionXSpinBox_valueChanged(double value)
{
	_model->setPositionX(static_cast<float>(value));
}

void ObjectOrientEditorDialog::on_positionYSpinBox_valueChanged(double value)
{
	_model->setPositionY(static_cast<float>(value));
}

void ObjectOrientEditorDialog::on_positionZSpinBox_valueChanged(double value)
{
	_model->setPositionZ(static_cast<float>(value));
}

void ObjectOrientEditorDialog::on_orientationPSpinBox_valueChanged(double value)
{
	_model->setOrientationP(static_cast<float>(value));
}

void ObjectOrientEditorDialog::on_orientationBSpinBox_valueChanged(double value)
{
	_model->setOrientationB(static_cast<float>(value));
}

void ObjectOrientEditorDialog::on_orientationHSpinBox_valueChanged(double value)
{
	_model->setOrientationH(static_cast<float>(value));
}

void ObjectOrientEditorDialog::on_setAbsoluteRadioButton_toggled(bool checked)
{
	if (checked) {
		_model->setSetMode(ObjectOrientEditorDialogModel::SetMode::Absolute);
		updateUi();
	}
}

void ObjectOrientEditorDialog::on_setRelativeRadioButton_toggled(bool checked)
{
	if (checked) {
		_model->setSetMode(ObjectOrientEditorDialogModel::SetMode::Relative);
		updateUi();
	}
}

void ObjectOrientEditorDialog::on_transformIndependentlyRadioButton_toggled(bool checked)
{
	if (checked) {
		_model->setTransformMode(ObjectOrientEditorDialogModel::TransformMode::Independent);
		updateUi();
	}
}

void ObjectOrientEditorDialog::on_transformRelativelyRadioButton_toggled(bool checked)
{
	if (checked) {
		_model->setTransformMode(ObjectOrientEditorDialogModel::TransformMode::Relative);
		updateUi();
	}
}

void ObjectOrientEditorDialog::on_pointToCheckBox_toggled(bool checked)
{
	_model->setPointTo(checked);
	updateUi();
}

void ObjectOrientEditorDialog::on_objectRadioButton_toggled(bool checked)
{
	if (checked) {
		_model->setPointMode(ObjectOrientEditorDialogModel::PointToMode::Object);
		updateUi();
	}
}

void ObjectOrientEditorDialog::on_objectComboBox_currentIndexChanged(int index)
{
	auto objNum = ui->objectComboBox->itemData(index).value<int>();
	_model->setPointToObjectIndex(objNum);
}

void ObjectOrientEditorDialog::on_locationRadioButton_toggled(bool checked)
{
	if (checked) {
		_model->setPointMode(ObjectOrientEditorDialogModel::PointToMode::Location);
		updateUi();
	}
}

void ObjectOrientEditorDialog::on_locationXSpinBox_valueChanged(double value)
{
	_model->setLocationX(static_cast<float>(value));
}

void ObjectOrientEditorDialog::on_locationYSpinBox_valueChanged(double value)
{
	_model->setLocationY(static_cast<float>(value));
}

void ObjectOrientEditorDialog::on_locationZSpinBox_valueChanged(double value)
{
	_model->setLocationZ(static_cast<float>(value));
}

} // namespace fso::fred::dialogs
