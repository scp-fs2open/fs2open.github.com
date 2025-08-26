#include <QCloseEvent>
#include <QKeyEvent>
#include "GlobalShipFlagsDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_GlobalShipFlagsDialog.h"
#include "mission/util.h"

namespace fso::fred::dialogs {

GlobalShipFlagsDialog::GlobalShipFlagsDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), _viewport(viewport), ui(new Ui::GlobalShipFlagsDialog()), _model(new GlobalShipFlagsDialogModel(this, viewport)) {
    ui->setupUi(this);
}
GlobalShipFlagsDialog::~GlobalShipFlagsDialog() = default;

void GlobalShipFlagsDialog::on_noShieldsButton_clicked()
{
	auto result = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
		"Set No Shields",
		"Are you sure you want to set the No Shields flag for all ships? This is immediate and cannot be undone!",
		{DialogButton::Yes, DialogButton::No});
	if (result == DialogButton::Yes) {
		_model->setNoShieldsAll();
	}
}

void GlobalShipFlagsDialog::on_noSubspaceDriveButton_clicked()
{
	auto result = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
		"Set No Subspace Drive",
		"Are you sure you want to set the No Subspace Drive flag for all fighters and bombers? This is immediate and cannot be undone!",
		{DialogButton::Yes, DialogButton::No});
	if (result == DialogButton::Yes) {
		_model->setNoSubspaceDriveOnFightersBombers();
	}
}

void GlobalShipFlagsDialog::on_primitiveSensorsButton_clicked()
{
	auto result = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
		"Set Primitive Sensors",
		"Are you sure you want to set the Primitive Sensors flag for all fighters and bombers? This is immediate and "
		"cannot be undone!",
		{DialogButton::Yes, DialogButton::No});
	if (result == DialogButton::Yes) {
		_model->setPrimitiveSensorsOnFightersBombers();
	}
}

void GlobalShipFlagsDialog::on_affectedByGravityButton_clicked()
{
	auto result = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
		"Set Affected by Gravity",
		"Are you sure you want to set the Affected by Gravity flag for all fighters and bombers? This is immediate and "
		"cannot be undone!",
		{DialogButton::Yes, DialogButton::No});
	if (result == DialogButton::Yes) {
		_model->setAffectedByGravityOnFightersBombers();
	}
}

} // namespace fso::fred::dialogs
