#include "ShipFlagsDialog.h"

#include "ui_ShipFlagsDialog.h"

#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso {
namespace fred {
namespace dialogs {

ShipFlagsDialog::ShipFlagsDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipFlagsDialog()), _model(new ShipFlagsDialogModel(this, viewport)),
	  _viewport(viewport)
{
	ui->setupUi(this);

	connect(this, &QDialog::accepted, _model.get(), &ShipFlagsDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &ShipFlagsDialogModel::reject);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipFlagsDialog::updateUI);

	// Column One
	connect(ui->destroyBeforeMissionCheckbox,
		&QCheckBox::stateChanged,
		this,
		&ShipFlagsDialog::destroyBeforeMissionChanged);
	connect(ui->destroySecondsSpinBox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&ShipFlagsDialog::destroyBeforeMissionSecondsChanged);
	connect(ui->scanableCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::scanableChanged);
	connect(ui->cargoKnownCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::cargoChanged);
	connect(ui->toggleSubsytemScanningCheckbox,
		&QCheckBox::stateChanged,
		this,
		&ShipFlagsDialog::subsytemScanningChanged);
	connect(ui->reinforcementUnitCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::reinforcementChanged);
	connect(ui->protectShipCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::protectShipChanged);
	connect(ui->beamProtectCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::beamProtectChanged);
	connect(ui->flakProtectCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::flakProtectChanged);
	connect(ui->laserProtectCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::laserProtectChanged);
	connect(ui->missileProtectCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::missileProtectChanged);
	connect(ui->ignoreForGoalsCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::ignoreForGoalsChanged);
	connect(ui->escortShipCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::escortChanged);
	connect(ui->escortPrioritySpinBox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&ShipFlagsDialog::escortValueChanged);
	connect(ui->noArrivalMusicCheckBox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::noArrivalMusicChanged);
	connect(ui->invulnerableCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::invulnerableChanged);
	connect(ui->guardianedCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::guardianedChanged);
	connect(ui->primitiveCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::primitiveChanged);
	connect(ui->noSubspaceDriveCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::noSubspaceChanged);
	connect(ui->hiddenFromSensorsCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::hiddenChanged);
	connect(ui->stealthCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::stealthChanged);
	connect(ui->friendlyStealthCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::friendlyStealthChanged);
	connect(ui->kamikazeCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::kamikazeChanged);
	connect(ui->kamikazeDamageSpinBox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&ShipFlagsDialog::kamikazeDamageChanged);
	connect(ui->noMoveCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::immobileChanged);

	// Column Two
	connect(ui->noDynamicGoalsCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::noDynamicGoalsChanged);
	connect(ui->redAlertCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::redAlertChanged);
	connect(ui->gravityCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::gravityChanged);
	connect(ui->specialWarpinCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::warpinChanged);
	connect(ui->targetableAsBombCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::targetableAsBombChanged);
	connect(ui->disableBuiltInMessagesCheckbox,
		&QCheckBox::stateChanged,
		this,
		&ShipFlagsDialog::disableBuiltInMessagesChanged);
	connect(ui->neverScreamCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::neverScreamChanged);
	connect(ui->alwaysScreamCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::alwaysScreamChanged);
	connect(ui->vaporizeCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::vaporizeChanged);
	connect(ui->respawnPrioritySpinBox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&ShipFlagsDialog::respawnPriorityChanged);
	connect(ui->autoCarryCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::autoCarryChanged);
	connect(ui->autoLinkCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::autoLinkChanged);
	connect(ui->hideShipNameCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::hideShipNameChanged);
	connect(ui->classDynamicCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::classDynamicChanged);
	connect(ui->disableETSCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::disableETSChanged);
	connect(ui->cloakCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::cloakChanged);
	connect(ui->scrambleMessagesCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::scrambleMessagesChanged);
	connect(ui->noCollideCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::noCollideChanged);
	connect(ui->noSelfDestructCheckbox, &QCheckBox::stateChanged, this, &ShipFlagsDialog::noSelfDestructChanged);

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipFlagsDialog::~ShipFlagsDialog() {}

void ShipFlagsDialog::closeEvent(QCloseEvent* event)
{
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
			"Changes detected",
			"Do you want to keep your changes?",
			{DialogButton::Yes, DialogButton::No, DialogButton::Cancel});

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

void ShipFlagsDialog::updateUI()
{
	util::SignalBlockers blockers(this);

	// Column One
	// Destroy before mission
	auto value = _model->getDestroyed();
	ui->destroyBeforeMissionCheckbox->setCheckState(Qt::CheckState(value));
	value = _model->getDestroyedSeconds();
	ui->destroySecondsSpinBox->setValue(value);
	// Scanable
	value = _model->getScanable();
	ui->scanableCheckbox->setCheckState(Qt::CheckState(value));
	// Cargo known
	value = _model->getCargoKnown();
	ui->cargoKnownCheckbox->setCheckState(Qt::CheckState(value));
	// Toggle Subsytem Sacnning
	value = _model->getSubsystemScanning();
	ui->toggleSubsytemScanningCheckbox->setCheckState(Qt::CheckState(value));
	// Reinforcement
	value = _model->getReinforcment();
	ui->reinforcementUnitCheckbox->setCheckState(Qt::CheckState(value));
	// Protect Flags
	value = _model->getProtectShip();
	ui->protectShipCheckbox->setCheckState(Qt::CheckState(value));
	value = _model->getBeamProtect();
	ui->beamProtectCheckbox->setCheckState(Qt::CheckState(value));
	value = _model->getFlakProtect();
	ui->flakProtectCheckbox->setCheckState(Qt::CheckState(value));
	value = _model->getLaserProtect();
	ui->laserProtectCheckbox->setCheckState(Qt::CheckState(value));
	value = _model->getMissileProtect();
	ui->missileProtectCheckbox->setCheckState(Qt::CheckState(value));
	// Ignore For goals
	value = _model->getIgnoreForGoals();
	ui->ignoreForGoalsCheckbox->setCheckState(Qt::CheckState(value));
	// Escort
	value = _model->getEscort();
	ui->escortShipCheckbox->setCheckState(Qt::CheckState(value));
	value = _model->getEscortValue();
	ui->escortPrioritySpinBox->setValue(value);
	// No Arrival Music
	value = _model->getNoArrivalMusic();
	ui->noArrivalMusicCheckBox->setCheckState(Qt::CheckState(value));
	// Invulnerable
	value = _model->getInvulnerable();
	ui->invulnerableCheckbox->setCheckState(Qt::CheckState(value));
	// Guardiened
	value = _model->getGuardianed();
	ui->guardianedCheckbox->setCheckState(Qt::CheckState(value));
	// Pirmitive Sensors
	value = _model->getPrimitiveSensors();
	ui->primitiveCheckbox->setCheckState(Qt::CheckState(value));
	// No Subspace Drive
	value = _model->getNoSubspaceDrive();
	ui->noSubspaceDriveCheckbox->setCheckState(Qt::CheckState(value));
	// Hidden From Sensors
	value = _model->getHidden();
	ui->hiddenFromSensorsCheckbox->setCheckState(Qt::CheckState(value));
	// Stealth
	value = _model->getStealth();
	ui->stealthCheckbox->setCheckState(Qt::CheckState(value));
	// Freindly Stealth
	value = _model->getFriendlyStealth();
	ui->friendlyStealthCheckbox->setCheckState(Qt::CheckState(value));
	// Kamikaze
	value = _model->getKamikaze();
	ui->kamikazeCheckbox->setCheckState(Qt::CheckState(value));
	value = _model->getKamikazeDamage();
	ui->kamikazeDamageSpinBox->setValue(value);
	// Does Not move
	value = _model->getImmobile();
	ui->noMoveCheckbox->setCheckState(Qt::CheckState(value));
	// Column Two
	// No Dynamic Goals
	value = _model->getNoDynamicGoals();
	ui->noDynamicGoalsCheckbox->setCheckState(Qt::CheckState(value));
	// Red Alert Carry
	value = _model->getRedAlert();
	ui->redAlertCheckbox->setCheckState(Qt::CheckState(value));
	// Affected By Gravity
	value = _model->getGravity();
	ui->gravityCheckbox->setCheckState(Qt::CheckState(value));
	// Special Warpin
	value = _model->getWarpin();
	ui->specialWarpinCheckbox->setCheckState(Qt::CheckState(value));
	// Targetable As Bomb
	value = _model->getTargetableAsBomb();
	ui->targetableAsBombCheckbox->setCheckState(Qt::CheckState(value));
	// Disable Built-in Messages
	value = _model->getDisableBuiltInMessages();
	ui->disableBuiltInMessagesCheckbox->setCheckState(Qt::CheckState(value));
	// Never Scream On Death
	value = _model->getNeverScream();
	ui->neverScreamCheckbox->setCheckState(Qt::CheckState(value));
	// Always Scream on Death
	value = _model->getAlwaysScream();
	ui->alwaysScreamCheckbox->setCheckState(Qt::CheckState(value));
	// Vaporize on Death
	value = _model->getVaporize();
	ui->vaporizeCheckbox->setCheckState(Qt::CheckState(value));
	// Respawn
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		ui->respawnPrioritySpinBox->setEnabled(true);
	} else {
		ui->respawnPrioritySpinBox->setEnabled(false);
	}
	value = _model->getRespawnPriority();
	ui->respawnPrioritySpinBox->setValue(value);
	// AutoNav Carry Status
	value = _model->getAutoCarry();
	ui->autoCarryCheckbox->setCheckState(Qt::CheckState(value));
	// AutoNav Needs Link
	value = _model->getAutoLink();
	ui->autoLinkCheckbox->setCheckState(Qt::CheckState(value));
	// Hide Ship Name
	value = _model->getHideShipName();
	ui->hideShipNameCheckbox->setCheckState(Qt::CheckState(value));
	// Set Class Dynamically
	value = _model->getClassDynamic();
	ui->classDynamicCheckbox->setCheckState(Qt::CheckState(value));
	//Disable ETS
	value = _model->getDisableETS();
	ui->disableETSCheckbox->setCheckState(Qt::CheckState(value));
	//Cloaked
	value = _model->getCloak();
	ui->cloakCheckbox->setCheckState(Qt::CheckState(value));
	//Scramble Messages
	value = _model->getScrambleMessages();
	ui->scrambleMessagesCheckbox->setCheckState(Qt::CheckState(value));
	//No Collisions
	value = _model->getNoCollide();
	ui->noCollideCheckbox->setCheckState(Qt::CheckState(value));
	//No Disabled Self-Destruct
	value = _model->getNoSelfDestruct();
	ui->noSelfDestructCheckbox->setCheckState(Qt::CheckState(value));
}

void ShipFlagsDialog::destroyBeforeMissionChanged(int value)
{
	_model->setDestroyed(value);
}

void ShipFlagsDialog::destroyBeforeMissionSecondsChanged(int value)
{
	_model->setDestroyedSeconds(value);
}

void ShipFlagsDialog::scanableChanged(int value)
{
	_model->setScanable(value);
}

void ShipFlagsDialog::cargoChanged(int value)
{
	_model->setCargoKnown(value);
}

void ShipFlagsDialog::subsytemScanningChanged(int value)
{
	_model->setSubsystemScanning(value);
}

void ShipFlagsDialog::reinforcementChanged(int value)
{
	_model->setReinforcment(value);
}

void ShipFlagsDialog::protectShipChanged(int value)
{
	_model->setProtectShip(value);
}

void ShipFlagsDialog::beamProtectChanged(int value)
{
	_model->setBeamProtect(value);
}

void ShipFlagsDialog::flakProtectChanged(int value)
{
	_model->setFlakProtect(value);
}

void ShipFlagsDialog::laserProtectChanged(int value)
{
	_model->setLaserProtect(value);
}

void ShipFlagsDialog::missileProtectChanged(int value)
{
	_model->setMissileProtect(value);
}

void ShipFlagsDialog::ignoreForGoalsChanged(int value)
{
	_model->setIgnoreForGoals(value);
}

void ShipFlagsDialog::escortChanged(int value)
{
	_model->setEscort(value);
}

void ShipFlagsDialog::escortValueChanged(int value)
{
	_model->setEscortValue(value);
}

void ShipFlagsDialog::noArrivalMusicChanged(int value)
{
	_model->setNoArrivalMusic(value);
}

void ShipFlagsDialog::invulnerableChanged(int value)
{
	_model->setInvulnerable(value);
}

void ShipFlagsDialog::guardianedChanged(int value)
{
	_model->setGuardianed(value);
}

void ShipFlagsDialog::primitiveChanged(int value)
{
	_model->setPrimitiveSensors(value);
}

void ShipFlagsDialog::noSubspaceChanged(int value)
{
	_model->setNoSubspaceDrive(value);
}

void ShipFlagsDialog::hiddenChanged(int value)
{
	_model->setHidden(value);
}

void ShipFlagsDialog::stealthChanged(int value)
{
	_model->setStealth(value);
}

void ShipFlagsDialog::friendlyStealthChanged(int value)
{
	_model->setFriendlyStealth(value);
}

void ShipFlagsDialog::kamikazeChanged(int value)
{
	_model->setKamikaze(value);
}

void ShipFlagsDialog::kamikazeDamageChanged(int value)
{
	_model->setKamikazeDamage(value);
}

void ShipFlagsDialog::immobileChanged(int value)
{
	_model->setImmobile(value);
}

void ShipFlagsDialog::noDynamicGoalsChanged(int value)
{
	_model->setNoDynamicGoals(value);
}

void ShipFlagsDialog::redAlertChanged(int value)
{
	_model->setRedAlert(value);
}

void ShipFlagsDialog::gravityChanged(int value)
{
	_model->setGravity(value);
}

void ShipFlagsDialog::warpinChanged(int value)
{
	_model->setWarpin(value);
}

void ShipFlagsDialog::targetableAsBombChanged(int value)
{
	_model->setTargetableAsBomb(value);
}

void ShipFlagsDialog::disableBuiltInMessagesChanged(int value)
{
	_model->setDisableBuiltInMessages(value);
}

void ShipFlagsDialog::neverScreamChanged(int value)
{
	_model->setNeverScream(value);
}

void ShipFlagsDialog::alwaysScreamChanged(int value)
{
	_model->setAlwaysScream(value);
}

void ShipFlagsDialog::vaporizeChanged(int value)
{
	_model->setVaporize(value);
}

void ShipFlagsDialog::respawnPriorityChanged(int value)
{
	_model->setRespawnPriority(value);
}

void ShipFlagsDialog::autoCarryChanged(int value)
{
	_model->setAutoCarry(value);
}

void ShipFlagsDialog::autoLinkChanged(int value)
{
	_model->setAutoLink(value);
}

void ShipFlagsDialog::hideShipNameChanged(int value)
{
	_model->setHideShipName(value);
}

void ShipFlagsDialog::classDynamicChanged(int value)
{
	_model->setClassDynamic(value);
}

void ShipFlagsDialog::disableETSChanged(int value)
{
	_model->setDisableETS(value);
}

void ShipFlagsDialog::cloakChanged(int value)
{
	_model->setCloak(value);
}

void ShipFlagsDialog::scrambleMessagesChanged(int value)
{
	_model->setScrambleMessages(value);
}

void ShipFlagsDialog::noCollideChanged(int value)
{
	_model->setNoCollide(value);
}

void ShipFlagsDialog::noSelfDestructChanged(int value)
{
	_model->setNoSelfDestruct(value);
}

} // namespace dialogs
} // namespace fred
} // namespace fso