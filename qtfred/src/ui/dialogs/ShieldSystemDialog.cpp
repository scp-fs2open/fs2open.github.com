#include <QFocusEvent>
#include <ui/util/DialogUndo.h>
#include <QRadioButton>
#include "ShieldSystemDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_ShieldSystemDialog.h"
#include "mission/commands/FredCommands.h"
#include "ui/FredView.h"
#include <ship/ship.h>
#include <object/object.h>

namespace fso::fred::dialogs {

namespace {

SCP_vector<BatchFlagCommand::ShipSnapshot> captureShipFlagSnapshots()
{
	SCP_vector<BatchFlagCommand::ShipSnapshot> snaps;
	for (const auto& ship : Ships) {
		if (ship.objnum >= 0) {
			snaps.push_back({ Objects[ship.objnum].signature,
			                  Objects[ship.objnum].flags,
			                  ship.flags });
		}
	}
	return snaps;
}

QString statusLabelText(GlobalShieldStatus status)
{
	switch (status) {
	case GlobalShieldStatus::HasShields:
		return QObject::tr("Currently: Has shields");
	case GlobalShieldStatus::NoShields:
		return QObject::tr("Currently: No shields");
	case GlobalShieldStatus::MixedShields:
	default:
		return QObject::tr("Currently: Mixed");
	}
}

// Clear the selection in a Qt exclusive button group. Without toggling
// autoExclusive off, neither radio in an exclusive pair can be unchecked
// programmatically.
void clearRadioPair(QRadioButton* a, QRadioButton* b)
{
	a->setAutoExclusive(false);
	b->setAutoExclusive(false);
	a->setChecked(false);
	b->setChecked(false);
	a->setAutoExclusive(true);
	b->setAutoExclusive(true);
}

} // namespace

ShieldSystemDialog::ShieldSystemDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_fredView(parent),
	_viewport(viewport),
	ui(new Ui::ShieldSystemDialog()),
	_model(new ShieldSystemDialogModel(this, viewport)) {

    ui->setupUi(this);
    util::installMainStackUndoShortcuts(this, _fredView->mainUndoStack());

	initializeUi();
	updateUi();

	// Keep the dialog in sync when the undo stack changes per-ship shield flags.
	connect(viewport->editor, &Editor::missionChanged, this, [this]() {
		_model->refresh();
		updateUi();
	});

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShieldSystemDialog::~ShieldSystemDialog() = default;

void ShieldSystemDialog::focusInEvent(QFocusEvent* e)
{
	_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	QDialog::focusInEvent(e);
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
	ui->typeCurrentStatusLabel->setText(statusLabelText(typeShieldSys));
	if (typeShieldSys == GlobalShieldStatus::MixedShields) {
		clearRadioPair(ui->typeHasShieldRadio, ui->typeNoShieldRadio);
		ui->applyTypeButton->setEnabled(false);
	} else {
		ui->typeHasShieldRadio->setChecked(typeShieldSys == GlobalShieldStatus::HasShields);
		ui->typeNoShieldRadio->setChecked(typeShieldSys == GlobalShieldStatus::NoShields);
		ui->applyTypeButton->setEnabled(true);
	}

	auto teamShieldSys = _model->getCurrentTeamShieldSys();
	ui->teamCurrentStatusLabel->setText(statusLabelText(teamShieldSys));
	if (teamShieldSys == GlobalShieldStatus::MixedShields) {
		clearRadioPair(ui->teamHasShieldRadio, ui->teamNoShieldRadio);
		ui->applyTeamButton->setEnabled(false);
	} else {
		ui->teamHasShieldRadio->setChecked(teamShieldSys == GlobalShieldStatus::HasShields);
		ui->teamNoShieldRadio->setChecked(teamShieldSys == GlobalShieldStatus::NoShields);
		ui->applyTeamButton->setEnabled(true);
	}
}

void ShieldSystemDialog::on_shipTypeCombo_currentIndexChanged(int index) {
	if (index >= 0) {
		_model->setCurrentShipType(index);
	}
	updateUi();
}

void ShieldSystemDialog::on_shipTeamCombo_currentIndexChanged(int index) {
	if (index >= 0) {
		_model->setCurrentTeam(index);
	}
	updateUi();
}

void ShieldSystemDialog::on_typeHasShieldRadio_toggled(bool checked) {
	if (checked) {
		ui->applyTypeButton->setEnabled(true);
	}
}

void ShieldSystemDialog::on_typeNoShieldRadio_toggled(bool checked) {
	if (checked) {
		ui->applyTypeButton->setEnabled(true);
	}
}

void ShieldSystemDialog::on_teamHasShieldRadio_toggled(bool checked) {
	if (checked) {
		ui->applyTeamButton->setEnabled(true);
	}
}

void ShieldSystemDialog::on_teamNoShieldRadio_toggled(bool checked) {
	if (checked) {
		ui->applyTeamButton->setEnabled(true);
	}
}

void ShieldSystemDialog::on_applyTypeButton_clicked() {
	auto before = captureShipFlagSnapshots();
	_model->applyType(ui->typeHasShieldRadio->isChecked());
	auto after = captureShipFlagSnapshots();
	if (before != after) {
		_fredView->mainUndoStack()->push(
			new BatchFlagCommand(std::move(before), std::move(after),
			                     _viewport->editor, tr("Apply Shield System by Class")));
	}
	updateUi();
}

void ShieldSystemDialog::on_applyTeamButton_clicked() {
	auto before = captureShipFlagSnapshots();
	_model->applyTeam(ui->teamHasShieldRadio->isChecked());
	auto after = captureShipFlagSnapshots();
	if (before != after) {
		_fredView->mainUndoStack()->push(
			new BatchFlagCommand(std::move(before), std::move(after),
			                     _viewport->editor, tr("Apply Shield System by Team")));
	}
	updateUi();
}

} // namespace fso::fred::dialogs
