#include "IconCoordinatesDialog.h"

#include "mission/dialogs/BriefingEditorDialogModel.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace fso::fred::dialogs {

IconCoordinatesDialog::IconCoordinatesDialog(QWidget* parent, BriefingEditorDialogModel* model)
	: QDialog(parent), _model(model)
{
	setWindowTitle("Icon Coordinates");
	setupUi();

	_initialIconPos = _model->getIconPosition();
	populateFromCurrentIcon();
}

void IconCoordinatesDialog::setupUi()
{
	auto* mainLayout = new QVBoxLayout(this);

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

	auto* buttonLayout = new QHBoxLayout();
	auto* okBtn = new QPushButton("OK", this);
	auto* cancelBtn = new QPushButton("Cancel", this);
	buttonLayout->addStretch();
	buttonLayout->addWidget(okBtn);
	buttonLayout->addWidget(cancelBtn);
	mainLayout->addLayout(buttonLayout);

	connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, this, &IconCoordinatesDialog::onCancelClicked);
	connect(_posX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &IconCoordinatesDialog::onSpinBoxEditingFinished);
	connect(_posY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &IconCoordinatesDialog::onSpinBoxEditingFinished);
	connect(_posZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &IconCoordinatesDialog::onSpinBoxEditingFinished);

	setLayout(mainLayout);
	setMinimumWidth(280);
}

void IconCoordinatesDialog::populateFromCurrentIcon()
{
	_populatingUi = true;
	_posX->setValue(_initialIconPos.xyz.x);
	_posY->setValue(_initialIconPos.xyz.y);
	_posZ->setValue(_initialIconPos.xyz.z);
	_populatingUi = false;
}

void IconCoordinatesDialog::applyCurrentInputsToIcon()
{
	vec3d pos;
	pos.xyz.x = static_cast<float>(_posX->value());
	pos.xyz.y = static_cast<float>(_posY->value());
	pos.xyz.z = static_cast<float>(_posZ->value());

	// Keep this on the same path as icon dragging so Change Locally is respected.
	_model->setIconPosition(pos);
}

void IconCoordinatesDialog::onSpinBoxEditingFinished()
{
	if (_populatingUi) {
		return;
	}

	applyCurrentInputsToIcon();
}

void IconCoordinatesDialog::onCancelClicked()
{
	_model->setIconPosition(_initialIconPos);
	reject();
}

} // namespace fso::fred::dialogs
