#include "CameraCoordinatesDialog.h"

#include "mission/dialogs/BriefingEditorDialogModel.h"
#include "ui/widgets/BriefingMapWidget.h"

#include "math/vecmat.h"
#include "mission/missionbriefcommon.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>

namespace fso::fred::dialogs {

CameraCoordinatesDialog::CameraCoordinatesDialog(QWidget* parent,
	BriefingEditorDialogModel* model,
	fso::fred::BriefingMapWidget* mapWidget)
	: QDialog(parent), _model(model), _mapWidget(mapWidget)
{
	setWindowTitle("Camera Coordinates");
	setupUi();

	// Initialize with current camera values once.
	_initialCameraPos = brief_get_current_cam_pos();
	_initialCameraOrient = brief_get_current_cam_orient();
	populateFromCurrentCamera();
}

void CameraCoordinatesDialog::setupUi()
{
	auto* mainLayout = new QVBoxLayout(this);

	// Position group
	auto* posGroup = new QGroupBox("Position", this);
	auto* posLayout = new QFormLayout(posGroup);

	auto makeSpinBox = [this](double min, double max, int decimals) {
		auto* sb = new QDoubleSpinBox(this);
		sb->setRange(min, max);
		sb->setDecimals(decimals);
		sb->setSingleStep(10.0);
		sb->setKeyboardTracking(false);
		return sb;
	};

	_posX = makeSpinBox(-999999.0, 999999.0, 1);
	_posY = makeSpinBox(-999999.0, 999999.0, 1);
	_posZ = makeSpinBox(-999999.0, 999999.0, 1);
	posLayout->addRow("X:", _posX);
	posLayout->addRow("Y:", _posY);
	posLayout->addRow("Z:", _posZ);
	mainLayout->addWidget(posGroup);

	// Orientation group
	auto* orientGroup = new QGroupBox("Orientation (degrees)", this);
	auto* orientLayout = new QFormLayout(orientGroup);

	_heading = makeSpinBox(-360.0, 360.0, 2);
	_pitch = makeSpinBox(-360.0, 360.0, 2);
	_bank = makeSpinBox(-360.0, 360.0, 2);
	orientLayout->addRow("Heading:", _heading);
	orientLayout->addRow("Pitch:", _pitch);
	orientLayout->addRow("Bank:", _bank);
	mainLayout->addWidget(orientGroup);

	// Apply button
	auto* buttonLayout = new QHBoxLayout();
	auto* okBtn = new QPushButton("OK", this);
	auto* cancelBtn = new QPushButton("Cancel", this);
	buttonLayout->addStretch();
	buttonLayout->addWidget(okBtn);
	buttonLayout->addWidget(cancelBtn);
	mainLayout->addLayout(buttonLayout);

	connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, this, &CameraCoordinatesDialog::onCancelClicked);
	connect(_posX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraCoordinatesDialog::onSpinBoxEditingFinished);
	connect(_posY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraCoordinatesDialog::onSpinBoxEditingFinished);
	connect(_posZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraCoordinatesDialog::onSpinBoxEditingFinished);
	connect(_heading, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraCoordinatesDialog::onSpinBoxEditingFinished);
	connect(_pitch, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraCoordinatesDialog::onSpinBoxEditingFinished);
	connect(_bank, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CameraCoordinatesDialog::onSpinBoxEditingFinished);

	setLayout(mainLayout);
	setMinimumWidth(280);
}

void CameraCoordinatesDialog::populateFromCurrentCamera()
{
	_populatingUi = true;
	const auto pos = _initialCameraPos;
	const auto orient = _initialCameraOrient;

	_posX->setValue(pos.xyz.x);
	_posY->setValue(pos.xyz.y);
	_posZ->setValue(pos.xyz.z);

	angles a;
	vm_extract_angles_matrix(&a, &orient);

	// Convert radians to degrees
	_heading->setValue(fl_degrees(a.h));
	_pitch->setValue(fl_degrees(a.p));
	_bank->setValue(fl_degrees(a.b));
	_populatingUi = false;
}

void CameraCoordinatesDialog::applyCurrentInputsToCamera()
{
	vec3d pos;
	pos.xyz.x = static_cast<float>(_posX->value());
	pos.xyz.y = static_cast<float>(_posY->value());
	pos.xyz.z = static_cast<float>(_posZ->value());

	angles a;
	a.h = fl_radians(static_cast<float>(_heading->value()));
	a.p = fl_radians(static_cast<float>(_pitch->value()));
	a.b = fl_radians(static_cast<float>(_bank->value()));

	matrix orient;
	vm_angles_2_matrix(&orient, &a);

	// Apply through the same path used by keyboard camera controls.
	_mapWidget->applyCameraToCurrentStage(pos, orient);
}

void CameraCoordinatesDialog::onSpinBoxEditingFinished()
{
	if (_populatingUi) {
		return;
	}

	applyCurrentInputsToCamera();
}

void CameraCoordinatesDialog::onCancelClicked()
{
	_mapWidget->applyCameraToCurrentStage(_initialCameraPos, _initialCameraOrient);
	reject();
}

} // namespace fso::fred::dialogs
