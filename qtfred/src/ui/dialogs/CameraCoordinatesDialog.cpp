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
	setAttribute(Qt::WA_DeleteOnClose);
	setupUi();

	// Connect to the map widget's camera changed signal for live updates
	connect(_mapWidget, &fso::fred::BriefingMapWidget::cameraChanged,
		this, &CameraCoordinatesDialog::onCameraChanged);

	// Initialize with current values
	onCameraChanged(brief_get_current_cam_pos(), brief_get_current_cam_orient());
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
	auto* applyBtn = new QPushButton("Apply", this);
	auto* closeBtn = new QPushButton("Close", this);
	buttonLayout->addStretch();
	buttonLayout->addWidget(applyBtn);
	buttonLayout->addWidget(closeBtn);
	mainLayout->addLayout(buttonLayout);

	connect(applyBtn, &QPushButton::clicked, this, &CameraCoordinatesDialog::onApplyClicked);
	connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);

	setLayout(mainLayout);
	setMinimumWidth(280);
}

void CameraCoordinatesDialog::onCameraChanged(vec3d pos, matrix orient)
{
	_updatingFromCamera = true;

	_posX->setValue(pos.xyz.x);
	_posY->setValue(pos.xyz.y);
	_posZ->setValue(pos.xyz.z);

	angles a;
	vm_extract_angles_matrix(&a, &orient);

	// Convert radians to degrees
	_heading->setValue(fl_degrees(a.h));
	_pitch->setValue(fl_degrees(a.p));
	_bank->setValue(fl_degrees(a.b));

	_updatingFromCamera = false;
}

void CameraCoordinatesDialog::onSpinBoxChanged()
{
	if (_updatingFromCamera)
		return;
	onApplyClicked();
}

void CameraCoordinatesDialog::onApplyClicked()
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

	// Update the model's stage camera
	_model->setCameraPosition(pos);
	_model->setCameraOrientation(orient);

	// Apply to the map widget with instant transition
	_mapWidget->setStage(_model->getCurrentStage());
}

} // namespace fso::fred::dialogs
