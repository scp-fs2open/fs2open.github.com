//
//

#include "ObjectOrientEditorDialog.h"

#include "ui_ObjectOrientationDialog.h"

#include <QCloseEvent>

namespace fso {
namespace fred {
namespace dialogs {

ObjectOrientEditorDialog::ObjectOrientEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::ObjectOrientEditorDialog()), _model(new ObjectOrientEditorDialogModel(this, viewport)),
	_viewport(viewport) {
	ui->setupUi(this);

	connect(this, &QDialog::accepted, _model.get(), &ObjectOrientEditorDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &ObjectOrientEditorDialogModel::reject);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ObjectOrientEditorDialog::updateUI);

	connect(ui->objectComboBox,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this,
			&ObjectOrientEditorDialog::objectSelectionChanged);

	connect(ui->objectRadio, &QRadioButton::toggled, this, &ObjectOrientEditorDialog::objectRadioToggled);
	connect(ui->locationRadio, &QRadioButton::toggled, this, &ObjectOrientEditorDialog::locationRadioToggled);

	connect(ui->pointToCheck, &QCheckBox::toggled, this, &ObjectOrientEditorDialog::pointToChecked);

	connect(ui->position_x,
			static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this,
			&ObjectOrientEditorDialog::positionValueChangedX);
	connect(ui->position_y,
			static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this,
			&ObjectOrientEditorDialog::positionValueChangedY);
	connect(ui->position_z,
			static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this,
			&ObjectOrientEditorDialog::positionValueChangedZ);

	connect(ui->location_x,
			static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this,
			&ObjectOrientEditorDialog::locationValueChangedX);
	connect(ui->location_y,
			static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this,
			&ObjectOrientEditorDialog::locationValueChangedY);
	connect(ui->location_z,
			static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this,
			&ObjectOrientEditorDialog::locationValueChangedZ);

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}
ObjectOrientEditorDialog::~ObjectOrientEditorDialog() {
}

void ObjectOrientEditorDialog::closeEvent(QCloseEvent* event) {
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question, "Changes detected", "Do you want to keep your changes?",
			{ DialogButton::Yes, DialogButton::No, DialogButton::Cancel });

		if (button == DialogButton::Cancel) {
			event->ignore();
			return;
		}

		if (button == DialogButton::Yes) {
			accept();
			return;
		}
	}

	QDialog::closeEvent(event);
}

void ObjectOrientEditorDialog::updateUI() {
	ui->position_x->setValue(_model->getPosition().xyz.x);
	ui->position_y->setValue(_model->getPosition().xyz.y);
	ui->position_z->setValue(_model->getPosition().xyz.z);

	ui->location_x->setValue(_model->getLocation().xyz.x);
	ui->location_y->setValue(_model->getLocation().xyz.y);
	ui->location_z->setValue(_model->getLocation().xyz.z);

	ui->pointToCheck->setChecked(_model->isPointTo());

	ui->objectRadio->setChecked(_model->getPointMode() == ObjectOrientEditorDialogModel::PointToMode::Object);
	ui->locationRadio->setChecked(_model->getPointMode() == ObjectOrientEditorDialogModel::PointToMode::Location);

	updateComboBox();

	ui->position_x->setEnabled(_model->isEnabled());
	ui->position_y->setEnabled(_model->isEnabled());
	ui->position_z->setEnabled(_model->isEnabled());

	ui->location_x->setEnabled(_model->isEnabled());
	ui->location_y->setEnabled(_model->isEnabled());
	ui->location_z->setEnabled(_model->isEnabled());

	ui->pointToCheck->setEnabled(_model->isEnabled());

	ui->objectRadio->setEnabled(_model->isEnabled());

	ui->objectComboBox->setEnabled(_model->isEnabled());

	ui->locationRadio->setEnabled(_model->isEnabled());
	ui->location_x->setEnabled(_model->isEnabled());
	ui->location_y->setEnabled(_model->isEnabled());
	ui->location_z->setEnabled(_model->isEnabled());
}
void ObjectOrientEditorDialog::updateComboBox() {
	ui->objectComboBox->clear();

	for (auto& entry : _model->getEntries()) {
		ui->objectComboBox->addItem(QString::fromStdString(entry.name), QVariant(entry.objIndex));
	}
	ui->objectComboBox->setCurrentIndex(ui->objectComboBox->findData(_model->getObjectIndex()));
}
void ObjectOrientEditorDialog::objectSelectionChanged(int index) {
	QSignalBlocker blocker(ui->objectComboBox);

	auto objNum = ui->objectComboBox->itemData(index).value<int>();
	_model->setSelectedObjectNum(objNum);
}
void ObjectOrientEditorDialog::objectRadioToggled(bool enabled) {
	if (enabled) {
		_model->setPointMode(ObjectOrientEditorDialogModel::PointToMode::Object);
	}
}
void ObjectOrientEditorDialog::locationRadioToggled(bool enabled) {
	if (enabled) {
		_model->setPointMode(ObjectOrientEditorDialogModel::PointToMode::Location);
	}
}
void ObjectOrientEditorDialog::pointToChecked(bool checked) {
	_model->setPointTo(checked);
}
void ObjectOrientEditorDialog::positionValueChangedX(double value) {
	auto oldVal = _model->getPosition();
	oldVal.xyz.x = (float) value;
	_model->setPosition(oldVal);
}
void ObjectOrientEditorDialog::positionValueChangedY(double value) {
	auto oldVal = _model->getPosition();
	oldVal.xyz.y = (float) value;
	_model->setPosition(oldVal);
}
void ObjectOrientEditorDialog::positionValueChangedZ(double value) {
	auto oldVal = _model->getPosition();
	oldVal.xyz.z = (float) value;
	_model->setPosition(oldVal);
}

void ObjectOrientEditorDialog::locationValueChangedX(double value) {
	auto oldVal = _model->getLocation();
	oldVal.xyz.x = (float) value;
	_model->setLocation(oldVal);
}
void ObjectOrientEditorDialog::locationValueChangedY(double value) {
	auto oldVal = _model->getLocation();
	oldVal.xyz.y = (float) value;
	_model->setLocation(oldVal);
}
void ObjectOrientEditorDialog::locationValueChangedZ(double value) {
	auto oldVal = _model->getLocation();
	oldVal.xyz.z = (float) value;
	_model->setLocation(oldVal);
}

}
}
}
