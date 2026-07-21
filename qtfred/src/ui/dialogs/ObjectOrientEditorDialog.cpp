//
//

#include "ObjectOrientEditorDialog.h"

#include "ui_ObjectOrientationDialog.h"

#include <ui/util/DialogUndo.h>
#include <ui/util/SignalBlockers.h>
#include "mission/util.h"
#include <mission/commands/FredCommands.h>
#include <QCloseEvent>

namespace fso::fred::dialogs {

ObjectOrientEditorDialog::ObjectOrientEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::ObjectOrientEditorDialog()), _model(new ObjectOrientEditorDialogModel(this, viewport)),
	_viewport(viewport), _fredView(parent) {
	this->setFocus();
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Object Orientation"));

	// set our internal values, update the UI
	initializeUi();
	updateUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}
ObjectOrientEditorDialog::~ObjectOrientEditorDialog() = default;

void ObjectOrientEditorDialog::accept()
{
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Orient Objects")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void ObjectOrientEditorDialog::reject()
{
	if (!_model) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		return;
	}
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
	}
}

void ObjectOrientEditorDialog::closeEvent(QCloseEvent* e)
{
	reject();
	// reject() hides the dialog when it actually closes. Let that close
	// proceed (so a dialog created with WA_DeleteOnClose is destroyed),
	// and only veto it when reject() decided to keep the dialog open (e.g.
	// the user cancelled the unsaved-changes prompt).
	if (isVisible()) {
		e->ignore();
	} else {
		e->accept();
	}
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
	updateModes();
	updatePointTo();
	updateLocation();

	enableOrDisableControls();
}

void ObjectOrientEditorDialog::updateModes()
{
	util::SignalBlockers blockers(this);

	ui->setAbsoluteRadioButton->setChecked(_model->getSetMode() == ObjectOrientEditorDialogModel::SetMode::Absolute);
	ui->setRelativeRadioButton->setChecked(_model->getSetMode() == ObjectOrientEditorDialogModel::SetMode::Relative);
	ui->transformIndependentlyRadioButton->setChecked(
		_model->getTransformMode() == ObjectOrientEditorDialogModel::TransformMode::Independent);
	ui->transformRelativelyRadioButton->setChecked(
		_model->getTransformMode() == ObjectOrientEditorDialogModel::TransformMode::Relative);
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
	ui->objectComboBox->setCurrentIndex(ui->objectComboBox->findData(_model->getPointToObjectIndex()));
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
	const float before = _model->getPosition().x;
	_model->setPositionX(static_cast<float>(value));
	pushValueCommand(FieldId::Orient_PosX, tr("Change Position"), before, _model->getPosition().x,
		[this](const float& v) { _model->setPositionX(v); });
}

void ObjectOrientEditorDialog::on_positionYSpinBox_valueChanged(double value)
{
	const float before = _model->getPosition().y;
	_model->setPositionY(static_cast<float>(value));
	pushValueCommand(FieldId::Orient_PosY, tr("Change Position"), before, _model->getPosition().y,
		[this](const float& v) { _model->setPositionY(v); });
}

void ObjectOrientEditorDialog::on_positionZSpinBox_valueChanged(double value)
{
	const float before = _model->getPosition().z;
	_model->setPositionZ(static_cast<float>(value));
	pushValueCommand(FieldId::Orient_PosZ, tr("Change Position"), before, _model->getPosition().z,
		[this](const float& v) { _model->setPositionZ(v); });
}

void ObjectOrientEditorDialog::on_orientationPSpinBox_valueChanged(double value)
{
	const float before = _model->getOrientation().p;
	_model->setOrientationP(static_cast<float>(value));
	pushValueCommand(FieldId::Orient_Pitch, tr("Change Orientation"), before, _model->getOrientation().p,
		[this](const float& v) { _model->setOrientationP(v); });
}

void ObjectOrientEditorDialog::on_orientationBSpinBox_valueChanged(double value)
{
	const float before = _model->getOrientation().b;
	_model->setOrientationB(static_cast<float>(value));
	pushValueCommand(FieldId::Orient_Bank, tr("Change Orientation"), before, _model->getOrientation().b,
		[this](const float& v) { _model->setOrientationB(v); });
}

void ObjectOrientEditorDialog::on_orientationHSpinBox_valueChanged(double value)
{
	const float before = _model->getOrientation().h;
	_model->setOrientationH(static_cast<float>(value));
	pushValueCommand(FieldId::Orient_Heading, tr("Change Orientation"), before, _model->getOrientation().h,
		[this](const float& v) { _model->setOrientationH(v); });
}

// The set-mode transform rebases the displayed values with rounding on both
// legs, so it is a snapshot rather than a field command.
void ObjectOrientEditorDialog::changeSetMode(ObjectOrientEditorDialogModel::SetMode mode)
{
	if (_model->getSetMode() == mode) {
		return;
	}

	const QByteArray before = _model->captureWorkingState();
	_model->setSetMode(mode);
	const QByteArray after = _model->captureWorkingState();
	updateUi();

	_dialogStack->push(new DialogSnapshotCommand(before, after,
		[this](const QByteArray& blob) {
			_model->restoreWorkingState(blob);
			updateUi();
		},
		tr("Change Set Mode")));
}

void ObjectOrientEditorDialog::on_setAbsoluteRadioButton_toggled(bool checked)
{
	if (checked) {
		changeSetMode(ObjectOrientEditorDialogModel::SetMode::Absolute);
	}
}

void ObjectOrientEditorDialog::on_setRelativeRadioButton_toggled(bool checked)
{
	if (checked) {
		changeSetMode(ObjectOrientEditorDialogModel::SetMode::Relative);
	}
}

void ObjectOrientEditorDialog::on_transformIndependentlyRadioButton_toggled(bool checked)
{
	if (checked) {
		const auto before = static_cast<int>(_model->getTransformMode());
		_model->setTransformMode(ObjectOrientEditorDialogModel::TransformMode::Independent);
		updateUi();
		pushValueCommand(FieldId::Orient_TransformMode, tr("Change Transform Mode"), before,
			static_cast<int>(_model->getTransformMode()), [this](const int& v) {
				_model->setTransformMode(static_cast<ObjectOrientEditorDialogModel::TransformMode>(v));
			});
	}
}

void ObjectOrientEditorDialog::on_transformRelativelyRadioButton_toggled(bool checked)
{
	if (checked) {
		const auto before = static_cast<int>(_model->getTransformMode());
		_model->setTransformMode(ObjectOrientEditorDialogModel::TransformMode::Relative);
		updateUi();
		pushValueCommand(FieldId::Orient_TransformMode, tr("Change Transform Mode"), before,
			static_cast<int>(_model->getTransformMode()), [this](const int& v) {
				_model->setTransformMode(static_cast<ObjectOrientEditorDialogModel::TransformMode>(v));
			});
	}
}

void ObjectOrientEditorDialog::on_pointToCheckBox_toggled(bool checked)
{
	const bool before = _model->getPointTo();
	_model->setPointTo(checked);
	updateUi();
	pushValueCommand(FieldId::Orient_PointTo, tr("Toggle Point To"), before, checked,
		[this](const bool& v) { _model->setPointTo(v); });
}

void ObjectOrientEditorDialog::on_objectRadioButton_toggled(bool checked)
{
	if (checked) {
		const auto before = static_cast<int>(_model->getPointMode());
		_model->setPointMode(ObjectOrientEditorDialogModel::PointToMode::Object);
		updateUi();
		pushValueCommand(FieldId::Orient_PointMode, tr("Change Point To Mode"), before,
			static_cast<int>(_model->getPointMode()), [this](const int& v) {
				_model->setPointMode(static_cast<ObjectOrientEditorDialogModel::PointToMode>(v));
			});
	}
}

void ObjectOrientEditorDialog::on_objectComboBox_currentIndexChanged(int index)
{
	auto objNum = ui->objectComboBox->itemData(index).value<int>();
	const int before = _model->getPointToObjectIndex();
	_model->setPointToObjectIndex(objNum);
	pushValueCommand(FieldId::Orient_PointObject, tr("Change Point To Object"), before,
		_model->getPointToObjectIndex(), [this](const int& v) { _model->setPointToObjectIndex(v); });
}

void ObjectOrientEditorDialog::on_locationRadioButton_toggled(bool checked)
{
	if (checked) {
		const auto before = static_cast<int>(_model->getPointMode());
		_model->setPointMode(ObjectOrientEditorDialogModel::PointToMode::Location);
		updateUi();
		pushValueCommand(FieldId::Orient_PointMode, tr("Change Point To Mode"), before,
			static_cast<int>(_model->getPointMode()), [this](const int& v) {
				_model->setPointMode(static_cast<ObjectOrientEditorDialogModel::PointToMode>(v));
			});
	}
}

void ObjectOrientEditorDialog::on_locationXSpinBox_valueChanged(double value)
{
	const float before = _model->getLocation().x;
	_model->setLocationX(static_cast<float>(value));
	pushValueCommand(FieldId::Orient_LocationX, tr("Change Point To Location"), before, _model->getLocation().x,
		[this](const float& v) { _model->setLocationX(v); });
}

void ObjectOrientEditorDialog::on_locationYSpinBox_valueChanged(double value)
{
	const float before = _model->getLocation().y;
	_model->setLocationY(static_cast<float>(value));
	pushValueCommand(FieldId::Orient_LocationY, tr("Change Point To Location"), before, _model->getLocation().y,
		[this](const float& v) { _model->setLocationY(v); });
}

void ObjectOrientEditorDialog::on_locationZSpinBox_valueChanged(double value)
{
	const float before = _model->getLocation().z;
	_model->setLocationZ(static_cast<float>(value));
	pushValueCommand(FieldId::Orient_LocationZ, tr("Change Point To Location"), before, _model->getLocation().z,
		[this](const float& v) { _model->setLocationZ(v); });
}

} // namespace fso::fred::dialogs
