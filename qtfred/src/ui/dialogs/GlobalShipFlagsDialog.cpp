#include <QCloseEvent>
#include <ui/util/DialogUndo.h>
#include <QFocusEvent>
#include <QKeyEvent>
#include "GlobalShipFlagsDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_GlobalShipFlagsDialog.h"
#include "mission/util.h"
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
} // anonymous namespace

GlobalShipFlagsDialog::GlobalShipFlagsDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), _fredView(parent), _viewport(viewport),
	ui(new Ui::GlobalShipFlagsDialog()), _model(new GlobalShipFlagsDialogModel(this, viewport)) {

    ui->setupUi(this);
    util::installMainStackUndoShortcuts(this, _fredView->mainUndoStack());
}
GlobalShipFlagsDialog::~GlobalShipFlagsDialog() = default;

void GlobalShipFlagsDialog::focusInEvent(QFocusEvent* e)
{
	_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	QDialog::focusInEvent(e);
}

void GlobalShipFlagsDialog::pushBatchFlagCommand(SCP_vector<BatchFlagCommand::ShipSnapshot> before, const QString& text)
{
	auto after = captureShipFlagSnapshots();
	if (before == after) {
		return; // every ship already had the flag — nothing to undo
	}
	_fredView->mainUndoStack()->push(
		new BatchFlagCommand(std::move(before), std::move(after), _viewport->editor, text));
}

void GlobalShipFlagsDialog::on_noShieldsButton_clicked()
{
	auto result = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
		"Set No Shields",
		"Are you sure you want to set the No Shields flag for all ships?",
		{DialogButton::Yes, DialogButton::No});
	if (result != DialogButton::Yes)
		return;

	auto before = captureShipFlagSnapshots();
	_model->setNoShieldsAll();
	pushBatchFlagCommand(std::move(before), tr("Set No Shields (All Ships)"));
}

void GlobalShipFlagsDialog::on_noSubspaceDriveButton_clicked()
{
	auto result = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
		"Set No Subspace Drive",
		"Are you sure you want to set the No Subspace Drive flag for all fighters and bombers?",
		{DialogButton::Yes, DialogButton::No});
	if (result != DialogButton::Yes)
		return;

	auto before = captureShipFlagSnapshots();
	_model->setNoSubspaceDriveOnFightersBombers();
	pushBatchFlagCommand(std::move(before), tr("Set No Subspace Drive (Fighters/Bombers)"));
}

void GlobalShipFlagsDialog::on_primitiveSensorsButton_clicked()
{
	auto result = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
		"Set Primitive Sensors",
		"Are you sure you want to set the Primitive Sensors flag for all fighters and bombers?",
		{DialogButton::Yes, DialogButton::No});
	if (result != DialogButton::Yes)
		return;

	auto before = captureShipFlagSnapshots();
	_model->setPrimitiveSensorsOnFightersBombers();
	pushBatchFlagCommand(std::move(before), tr("Set Primitive Sensors (Fighters/Bombers)"));
}

void GlobalShipFlagsDialog::on_affectedByGravityButton_clicked()
{
	auto result = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
		"Set Affected by Gravity",
		"Are you sure you want to set the Affected by Gravity flag for all fighters and bombers?",
		{DialogButton::Yes, DialogButton::No});
	if (result != DialogButton::Yes)
		return;

	auto before = captureShipFlagSnapshots();
	_model->setAffectedByGravityOnFightersBombers();
	pushBatchFlagCommand(std::move(before), tr("Set Affected by Gravity (Fighters/Bombers)"));
}

} // namespace fso::fred::dialogs
