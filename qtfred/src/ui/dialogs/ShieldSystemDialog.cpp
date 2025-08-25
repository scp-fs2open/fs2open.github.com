#include <QCloseEvent>
#include <QKeyEvent>
#include "ShieldSystemDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_ShieldSystemDialog.h"
#include "mission/util.h"

namespace fso::fred::dialogs {

ShieldSystemDialog::ShieldSystemDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	ui(new Ui::ShieldSystemDialog()),
	_model(new ShieldSystemDialogModel(this, viewport)) {
    ui->setupUi(this);

	initializeUi();
	updateUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShieldSystemDialog::~ShieldSystemDialog() = default;

void ShieldSystemDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void ShieldSystemDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void ShieldSystemDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void ShieldSystemDialog::initializeUi() {
	util::SignalBlockers blockers(this);

	if (ui->shipTeamCombo->count() == 0) {
		for (const auto& teamName : _model->getTeamOptions()) {
			ui->shipTeamCombo->addItem(QString::fromStdString(teamName));
		}
	}

	ui->shipTeamCombo->setCurrentIndex(_model->getCurrentTeam());

	if (ui->shipTypeCombo->count() == 0) {
		for (const auto& typeName : _model->getShipTypeOptions()) {
			ui->shipTypeCombo->addItem(QString::fromStdString(typeName));
		}
	}

	ui->shipTypeCombo->setCurrentIndex(_model->getCurrentShipType());
}

void ShieldSystemDialog::updateUi() {
	util::SignalBlockers blockers(this);

	auto typeShieldSys = _model->getCurrentTypeShieldSys();
	ui->typeHasShieldRadio->setChecked(typeShieldSys == GlobalShieldStatus::HasShields);
	ui->typeNoShieldRadio->setChecked(typeShieldSys == GlobalShieldStatus::NoShields);

	auto teamShieldSys = _model->getCurrentTeamShieldSys();
	ui->teamHasShieldRadio->setChecked(teamShieldSys == GlobalShieldStatus::HasShields);
	ui->teamNoShieldRadio->setChecked(teamShieldSys == GlobalShieldStatus::NoShields);
}

void ShieldSystemDialog::on_okAndCancelButtons_accepted() {
	accept();
}

void ShieldSystemDialog::on_okAndCancelButtons_rejected() {
	reject();
}

void ShieldSystemDialog::on_shipTypeCombo_currentIndexChanged(int index) {
	if (index >= 0) {
		_model->setCurrentTeam(index);
	}
	updateUi();
}

void ShieldSystemDialog::on_shipTeamCombo_currentIndexChanged(int index) {
	if (index >= 0) {
		_model->setCurrentShipType(index);
	}
	updateUi();
}

void ShieldSystemDialog::on_typeHasShieldRadio_toggled(bool checked) {
	if (checked) {
		_model->setCurrentTypeShieldSys(checked);
	}
}

void ShieldSystemDialog::on_typeNoShieldRadio_toggled(bool checked) {
	if (checked) {
		_model->setCurrentTypeShieldSys(!checked);
	}
}

void ShieldSystemDialog::on_teamHasShieldRadio_toggled(bool checked) {
	if (checked) {
		_model->setCurrentTeamShieldSys(checked);
	}
}

void ShieldSystemDialog::on_teamNoShieldRadio_toggled(bool checked) {
	if (checked) {
		_model->setCurrentTeamShieldSys(!checked);
	}
}

} // namespace fso::fred::dialogs
