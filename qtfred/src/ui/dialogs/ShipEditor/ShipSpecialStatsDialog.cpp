#include "ShipSpecialStatsDialog.h"

#include "ui_ShipSpecialStatsDialog.h"

#include <mission/util.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso::fred::dialogs {

ShipSpecialStatsDialog::ShipSpecialStatsDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipSpecialStatsDialog()), _model(new ShipSpecialStatsDialogModel(this, viewport)),
	  _viewport(viewport)
{
	ui->setupUi(this);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, [this]() { updateUI(false); });

	updateUI(true);

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipSpecialStatsDialog::~ShipSpecialStatsDialog() = default;

void ShipSpecialStatsDialog::accept()
{ // If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void ShipSpecialStatsDialog::reject()
{ // Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void ShipSpecialStatsDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void ShipSpecialStatsDialog::on_buttonBox_accepted()
{
	accept();
}
void ShipSpecialStatsDialog::on_buttonBox_rejected()
{
	reject();
}
void ShipSpecialStatsDialog::on_damageSpinBox_valueChanged(int value)
{
	_model->setDamage(value);
}
void ShipSpecialStatsDialog::on_blastSpinBox_valueChanged(int value)
{
	_model->setBlast(value);
}
void ShipSpecialStatsDialog::on_iRadiusSpinBox_valueChanged(int value)
{
	_model->setInnerRadius(value);
}
void ShipSpecialStatsDialog::on_oRadiusSpinBox_valueChanged(int value)
{
	_model->setOuterRadius(value);
}
void ShipSpecialStatsDialog::on_createShockwaveCheckBox_toggled(bool value)
{
	_model->setShockwave(value);
}
void ShipSpecialStatsDialog::on_shockwaveSpeedSpinBox_valueChanged(int value)
{
	_model->setShockwaveSpeed(value);
}
void ShipSpecialStatsDialog::on_deathRollCheckBox_toggled(bool value)
{
	_model->setDeathRoll(value);
}
void ShipSpecialStatsDialog::on_rollTimeSpinBox_valueChanged(int value)
{
	_model->setRollDuration(value);
}
void ShipSpecialStatsDialog::on_specialShieldCheckBox_toggled(bool value)
{
	_model->setSpecialShield(value);
}
void ShipSpecialStatsDialog::on_shieldSpinBox_valueChanged(int value)
{
	_model->setShield(value);
}
void ShipSpecialStatsDialog::on_specialHullCheckbox_toggled(bool value)
{
	_model->setSpecialHull(value);
}
void ShipSpecialStatsDialog::on_hullSpinBox_valueChanged(int value)
{
	_model->setHull(value);
}
void ShipSpecialStatsDialog::on_explodeCheckBox_toggled(bool value)
{
	_model->setSpecialExp(value);
}
void ShipSpecialStatsDialog::updateUI(bool first)
{
	util::SignalBlockers blockers(this);
	if (first) {
		ui->explodeCheckBox->setChecked(_model->getSpecialExp());
		ui->damageSpinBox->setValue(_model->getDamage());
		ui->blastSpinBox->setValue(_model->getBlast());
		ui->iRadiusSpinBox->setValue(_model->getInnerRadius());
		ui->oRadiusSpinBox->setValue(_model->getOuterRadius());
		ui->createShockwaveCheckBox->setChecked(_model->getShockwave());
		ui->shockwaveSpeedSpinBox->setValue(_model->getShockwaveSpeed());
		ui->deathRollCheckBox->setChecked(_model->getDeathRoll());
		ui->rollTimeSpinBox->setValue(_model->getRollDuration());

		ui->specialShieldCheckBox->setChecked(_model->getSpecialShield());
		ui->shieldSpinBox->setValue(_model->getShield());
		ui->specialHullCheckbox->setChecked(_model->getSpecialHull());
		ui->hullSpinBox->setValue(_model->getHull());
	}
	// Enable/Disable
	if (_model->getSpecialExp()) {
		ui->damageSpinBox->setEnabled(true);
		ui->blastSpinBox->setEnabled(true);
		ui->iRadiusSpinBox->setEnabled(true);
		ui->oRadiusSpinBox->setEnabled(true);
		ui->createShockwaveCheckBox->setEnabled(true);
		ui->deathRollCheckBox->setEnabled(true);
		if (_model->getShockwave()) {
			ui->shockwaveSpeedSpinBox->setEnabled(true);
		} else {
			ui->shockwaveSpeedSpinBox->setEnabled(false);
		}
		if (_model->getDeathRoll()) {
			ui->rollTimeSpinBox->setEnabled(true);
		} else {
			ui->rollTimeSpinBox->setEnabled(false);
		}
	} else {
		ui->damageSpinBox->setEnabled(false);
		ui->blastSpinBox->setEnabled(false);
		ui->iRadiusSpinBox->setEnabled(false);
		ui->oRadiusSpinBox->setEnabled(false);
		ui->createShockwaveCheckBox->setEnabled(false);
		ui->deathRollCheckBox->setEnabled(false);
		ui->shockwaveSpeedSpinBox->setEnabled(false);
		ui->rollTimeSpinBox->setEnabled(false);
	}
	ui->shieldSpinBox->setEnabled(_model->getSpecialShield());
	ui->hullSpinBox->setEnabled(_model->getSpecialHull());
}
} // namespace fso::fred::dialogs