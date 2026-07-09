#include "MissionSpecDialog.h"

#include "ui_MissionSpecDialog.h"

#include <globalincs/globals.h>
#include <ui/dialogs/General/ImagePickerDialog.h>
#include <ui/dialogs/MissionSpecs/CustomDataDialog.h>
#include <ui/dialogs/MissionSpecs/CustomStringsDialog.h>
#include <ui/dialogs/MissionSpecs/CustomWingNamesDialog.h>
#include <ui/dialogs/MissionSpecs/SoundEnvironmentDialog.h>
#include <ui/dialogs/MissionSpecs/SupportRearmDialog.h>
#include <ui/util/default_dir.h>
#include <ui/util/DialogUndo.h>
#include <ui/util/SignalBlockers.h>
#include "mission/util.h"
#include <mission/commands/FredCommands.h>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include <tuple>

namespace fso::fred::dialogs {

MissionSpecDialog::MissionSpecDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::MissionSpecDialog()), _model(new MissionSpecDialogModel(this, viewport)),
	_viewport(viewport), _fredView(parent) {
    ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Mission Specs"));

	ui->missionTitle->setMaxLength(NAME_LENGTH - 1);
	ui->missionDesigner->setMaxLength(NAME_LENGTH - 1);
	ui->squadronName->setMaxLength(NAME_LENGTH - 1);
	ui->squadronLogo->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->lowResScreen->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->highResScreen->setMaxLength(MAX_FILENAME_LEN - 1);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &MissionSpecDialog::updateUi);

	initializeUi();
	updateUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

MissionSpecDialog::~MissionSpecDialog() = default;

void MissionSpecDialog::accept()
{
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr); // detach from Qt parent before transferring ownership
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Mission Specs")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void MissionSpecDialog::reject()
{
	if (!_model) {
		// Model was already moved into the undo command (accept() succeeded);
		// just restore the active stack and close.
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		return;
	}

	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void MissionSpecDialog::closeEvent(QCloseEvent* e) {
	reject();
	// reject() hides the dialog when it actually closes. Let that close
	// proceed (so a dialog created with WA_DeleteOnClose is destroyed),
	// and only veto it when reject() decided to keep the dialog open (e.g.
	// the user cancelled the unsaved-changes prompt).
	if (isVisible()) {
		e->ignore();
	} else {
		e->accept();
	}
}


void MissionSpecDialog::initializeUi()
{
	initFlagList();
	updateUi();
}

void MissionSpecDialog::updateUi() {
	util::SignalBlockers blockers(this);

	ui->missionTitle->setText(_model->getMissionTitle().c_str());
	ui->missionDesigner->setText(_model->getDesigner().c_str());

	SCP_string created = "Created: " + _model->getCreatedTime();
	SCP_string modified = "Modified: " + _model->getModifiedTime();
	ui->createdLabel->setText(created.c_str());
	ui->modifiedLabel->setText(modified.c_str());

	updateMissionType();

	ui->maxRespawnCount->setEnabled(_model->getMissionType() & MISSION_TYPE_MULTI);
	ui->respawnDelayCount->setEnabled(_model->getMissionType() & MISSION_TYPE_MULTI);
	ui->maxRespawnCount->setValue(_model->getNumRespawns());
	ui->respawnDelayCount->setValue(_model->getMaxRespawnDelay());
	ui->playerEntryDelayDoubleSpinBox->setValue(_model->getPlayerEntryDelay());

	ui->squadronName->setText(_model->getSquadronName().c_str());
	ui->squadronLogo->setEnabled(false);
	ui->squadronLogo->setText(_model->getSquadronLogo().c_str());

	ui->lowResScreen->setText(_model->getLowResLoadingScren().c_str());
	ui->highResScreen->setText(_model->getHighResLoadingScren().c_str());

	ui->toggleTrail->setChecked(_model->getMissionFlag(Mission::Mission_Flags::Toggle_ship_trails));
	ui->toggleSpeedDisplay->setChecked(_model->getTrailThresholdFlag());
	ui->minDisplaySpeed->setEnabled(_model->getTrailThresholdFlag());
	ui->minDisplaySpeed->setValue(_model->getTrailDisplaySpeed());

	updateCmdMessage();

	updateMusic();

	updateFlags();

	updateAIProfiles();

	updateTextEditors();
}

void MissionSpecDialog::initFlagList()
{
	updateFlags();

	const auto descs = _model->getMissionFlagDescriptions();
	QVector<std::pair<QString, QString>> qtDescs;
	qtDescs.reserve(static_cast<int>(descs.size()));
	for (const auto& d : descs)
		qtDescs.append({QString::fromUtf8(d.first.c_str()), QString::fromUtf8(d.second.c_str())});
	ui->flagList->setFlagDescriptions(qtDescs);

	// per flag immediate apply to the model
	connect(ui->flagList, &fso::fred::FlagListWidget::flagToggled, this, [this](const QString& name, bool checked) {
		const SCP_string flagName = name.toUtf8().constData();

		// Locate the flag's list position (stable merge identity) and its
		// current value before applying the toggle.
		int index = -1;
		bool before = checked;
		const auto& flags = _model->getMissionFlagsList();
		for (int i = 0; i < static_cast<int>(flags.size()); ++i) {
			if (!stricmp(flags[i].first.c_str(), flagName.c_str())) {
				index = i;
				before = flags[i].second;
				break;
			}
		}

		_model->setMissionFlag(flagName, checked);
		updateLargeShipCollisionGroup();

		if (index < 0 || before == checked) {
			return;
		}

		auto* cmd = new FieldEditCommand<bool>(FieldId::Spec_MissionFlag + index, nullptr, tr("Toggle Mission Flag"), true);
		cmd->addEntry(before, checked, [this, flagName](const bool& v) {
			_model->setMissionFlag(flagName, v);
			updateUi();
		});
		_dialogStack->push(cmd);
	});
}

void MissionSpecDialog::updateFlags()
{
	const auto flags = _model->getMissionFlagsList();

	QVector<std::pair<QString, int>> toWidget;
	toWidget.reserve(static_cast<int>(flags.size()));
	for (const auto& p : flags) {
		QString name = QString::fromUtf8(p.first.c_str());
		toWidget.append({name, p.second ? Qt::Checked : Qt::Unchecked});
	}

	ui->flagList->setFlags(toWidget);
	updateLargeShipCollisionGroup();
}

void MissionSpecDialog::updateLargeShipCollisionGroup()
{
	const auto enabled = _model->getMissionFlag(Mission::Mission_Flags::Large_ships_no_collide_by_default);
	ui->largeShipCollisionGroupLabel->setVisible(enabled);
	ui->largeShipCollisionGroup->setVisible(enabled);
	ui->largeShipCollisionGroup->setValue(_model->getLargeShipNoCollideCollisionGroup());
}

void MissionSpecDialog::updateMissionType() {
	int m_type = _model->getMissionType();

	ui->m_type_SinglePlayer->setChecked(m_type & MISSION_TYPE_SINGLE);

	// for multiplayer -- be sure to assign a default type if not already assigned.
	bool multi = m_type & MISSION_TYPE_MULTI;
	ui->m_type_MultiPlayer->setChecked(multi);
	for (auto &button : ui->mt_multiGroup->buttons()) {
		button->setEnabled(multi);
		button->setCheckable(multi);
	}

	ui->m_type_Training->setChecked(m_type & MISSION_TYPE_TRAINING);

	// we need to set one of these three multiplayer modes so interface looks correct
	if (!(m_type & (MISSION_TYPE_MULTI_COOP | MISSION_TYPE_MULTI_DOGFIGHT | MISSION_TYPE_MULTI_TEAMS))) {
		m_type |= MISSION_TYPE_MULTI_COOP;
	}

	ui->m_type_Cooperative->setChecked(m_type & MISSION_TYPE_MULTI_COOP);

	ui->m_type_TeamVsTeam->setChecked(m_type & MISSION_TYPE_MULTI_TEAMS);

	ui->m_type_Dogfight->setChecked(m_type & MISSION_TYPE_MULTI_DOGFIGHT);
}

void MissionSpecDialog::updateCmdMessage() {
	int i, save_idx = 0;

	auto sender = _model->getCommandSender();
	ui->senderCombBox->clear();
	ui->senderCombBox->addItem(DEFAULT_COMMAND, QVariant(QString(DEFAULT_COMMAND)));

	for (i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			if (Ship_info[Ships[i].ship_info_index].is_huge_ship()) {
				ui->senderCombBox->addItem(Ships[i].ship_name, QVariant(QString(Ships[i].ship_name)));
			}
		}
	}

	ui->senderCombBox->setCurrentIndex(ui->senderCombBox->findText(sender.c_str()));

	save_idx = _model->getCommandPersona();
	ui->personaComboBox->clear();
	for (i = 0; i < (int)Personas.size(); i++) {
		if (Personas[i].flags & PERSONA_FLAG_COMMAND) {
			ui->personaComboBox->addItem(Personas[i].name, QVariant(i));
		}
	}
	ui->personaComboBox->setCurrentIndex(ui->personaComboBox->findData(save_idx));

	ui->toggleOverrideHashCommand->setChecked(_model->getMissionFlag(Mission::Mission_Flags::Override_hashcommand));
}

void MissionSpecDialog::updateMusic() {
	int i, idx;

	idx = _model->getEventMusic();
	ui->defaultMusicCombo->clear();
	ui->defaultMusicCombo->addItem("None",QVariant(0));
	for (i = 0; i < (int)Soundtracks.size(); i++) {
		ui->defaultMusicCombo->addItem(Soundtracks[i].name, QVariant(i+1));
	}
	ui->defaultMusicCombo->setCurrentIndex(ui->defaultMusicCombo->findData(idx));

	auto musicPack = _model->getSubEventMusic();
	ui->musicPackCombo->clear();
	ui->musicPackCombo->addItem("None");
	for (i = 0; i < (int)Soundtracks.size(); i++) {
		ui->musicPackCombo->addItem(Soundtracks[i].name, QVariant(QString(Soundtracks[i].name)));
	}
	ui->musicPackCombo->setCurrentIndex(ui->musicPackCombo->findText(musicPack.c_str()));
}

void MissionSpecDialog::updateAIProfiles() {
	int idx = _model->getAIProfileIndex();
	ui->aiProfileCombo->clear();
	for (int i = 0; i < Num_ai_profiles; i++) {
		ui->aiProfileCombo->addItem(Ai_profiles[i].profile_name, QVariant(AI_PROFILES_INDEX(&Ai_profiles[i])));
	}
	ui->aiProfileCombo->setCurrentIndex(ui->aiProfileCombo->findData(idx));
}

void MissionSpecDialog::updateTextEditors() {
	QTextCursor textCursor;
	
	textCursor = ui->missionDescEditor->textCursor();
	ui->missionDescEditor->document()->setPlainText(_model->getMissionDescText().c_str());
	ui->missionDescEditor->setTextCursor(textCursor);

	textCursor = ui->designerNoteEditor->textCursor();
	ui->designerNoteEditor->document()->setPlainText(_model->getDesignerNoteText().c_str());
	ui->designerNoteEditor->setTextCursor(textCursor);
}

void MissionSpecDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void MissionSpecDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void MissionSpecDialog::on_missionTitle_textChanged(const QString & string) {
	const SCP_string before = _model->getMissionTitle();
	_model->setMissionTitle(string.toUtf8().constData());
	pushValueCommand(FieldId::Spec_Title, tr("Change Mission Title"), before, _model->getMissionTitle(),
		[this](const SCP_string& v) { _model->setMissionTitle(v); });
}

void MissionSpecDialog::on_missionDesigner_textChanged(const QString & string) {
	const SCP_string before = _model->getDesigner();
	_model->setDesigner(string.toUtf8().constData());
	pushValueCommand(FieldId::Spec_Designer, tr("Change Designer Name"), before, _model->getDesigner(),
		[this](const SCP_string& v) { _model->setDesigner(v); });
}

// The six type radios all funnel here; the setter validates and can refuse,
// which the read-back turns into a no-op push.
void MissionSpecDialog::changeMissionType(int type) {
	const int before = _model->getMissionType();
	_model->setMissionType(type);
	pushValueCommand(FieldId::Spec_MissionType, tr("Change Mission Type"), before, _model->getMissionType(),
		[this](const int& v) { _model->setMissionType(v); });
}

void MissionSpecDialog::on_m_type_SinglePlayer_toggled(bool checked) {
	if (checked) {
		changeMissionType(MISSION_TYPE_SINGLE);
	}
}

void MissionSpecDialog::on_m_type_MultiPlayer_toggled(bool checked) {
	if (checked) {
		changeMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_COOP);
	}
}

void MissionSpecDialog::on_m_type_Training_toggled(bool checked) {
	if (checked) {
		changeMissionType(MISSION_TYPE_TRAINING);
	}
}

void MissionSpecDialog::on_m_type_Cooperative_toggled(bool checked) {
	if (checked) {
		changeMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_COOP);
	}
}

void MissionSpecDialog::on_m_type_TeamVsTeam_toggled(bool checked) {
	if (checked) {
		changeMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_TEAMS);
	}
}

void MissionSpecDialog::on_m_type_Dogfight_toggled(bool checked) {
	if (checked) {
		changeMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_DOGFIGHT);
	}
}

void MissionSpecDialog::on_maxRespawnCount_valueChanged(int value) {
	const uint before = _model->getNumRespawns();
	_model->setNumRespawns(value);
	pushValueCommand(FieldId::Spec_NumRespawns, tr("Change Respawn Count"), before, _model->getNumRespawns(),
		[this](const uint& v) { _model->setNumRespawns(v); });
}

void MissionSpecDialog::on_respawnDelayCount_valueChanged(int value) {
	const int before = _model->getMaxRespawnDelay();
	_model->setMaxRespawnDelay(value);
	pushValueCommand(FieldId::Spec_RespawnDelay, tr("Change Respawn Delay"), before, _model->getMaxRespawnDelay(),
		[this](const int& v) { _model->setMaxRespawnDelay(v); });
}

void MissionSpecDialog::on_playerEntryDelayDoubleSpinBox_valueChanged(double value) {
	const float before = _model->getPlayerEntryDelay();
	_model->setPlayerEntryDelay(static_cast<float>(value));
	pushValueCommand(FieldId::Spec_EntryDelay, tr("Change Player Entry Delay"), before, _model->getPlayerEntryDelay(),
		[this](const float& v) { _model->setPlayerEntryDelay(v); });
}

void MissionSpecDialog::on_squadronName_textChanged(const QString & string) {
	const SCP_string before = _model->getSquadronName();
	_model->setSquadronName(string.toUtf8().constData());
	pushValueCommand(FieldId::Spec_SquadName, tr("Change Squadron Name"), before, _model->getSquadronName(),
		[this](const SCP_string& v) { _model->setSquadronName(v); });
}

void MissionSpecDialog::on_customWingNameButton_clicked()
{
	CustomWingNamesDialog dialog(this, _viewport);
	dialog.setInitialStartingWings(_model->getCustomStartingWings());
	dialog.setInitialSquadronWings(_model->getCustomSquadronWings());
	dialog.setInitialTvTWings(_model->getCustomTvTWings());

	if (dialog.exec() != QDialog::Accepted) {
		return;
	}

	// One command covers all three wing name sets the subdialog edits.
	using WingNames = std::tuple<std::array<SCP_string, MAX_STARTING_WINGS>,
		std::array<SCP_string, MAX_SQUADRON_WINGS>, std::array<SCP_string, MAX_TVT_WINGS>>;

	const WingNames before{_model->getCustomStartingWings(), _model->getCustomSquadronWings(), _model->getCustomTvTWings()};

	_model->setCustomStartingWings(dialog.getStartingWings());
	_model->setCustomSquadronWings(dialog.getSquadronWings());
	_model->setCustomTvTWings(dialog.getTvTWings());

	const WingNames after{_model->getCustomStartingWings(), _model->getCustomSquadronWings(), _model->getCustomTvTWings()};

	pushValueCommand(FieldId::Spec_WingNames, tr("Edit Custom Wing Names"), before, after,
		[this](const WingNames& v) {
			_model->setCustomStartingWings(std::get<0>(v));
			_model->setCustomSquadronWings(std::get<1>(v));
			_model->setCustomTvTWings(std::get<2>(v));
		},
		true);
}

void MissionSpecDialog::on_squadronLogoButton_clicked() {
	const auto files = _model->getSquadLogoList();
	if (files.empty()) {
		QMessageBox::information(this, "Select Squad Image", "No images found.");
		return;
	}

	QStringList qnames;
	qnames.reserve(static_cast<int>(files.size()));
	for (const auto& s : files)
		qnames << QString::fromStdString(s);

	ImagePickerDialog dlg(this);
	dlg.setWindowTitle("Select Squad Image");
	dlg.allowUnset(true);
	dlg.setImageFilenames(qnames);

	// Optional: preselect current
	dlg.setInitialSelection(QString::fromStdString(_model->getSquadronLogo()));

	if (dlg.exec() != QDialog::Accepted)
		return;

	const SCP_string before = _model->getSquadronLogo();
	const std::string chosen = dlg.selectedFile().toUtf8().constData();
	_model->setSquadronLogo(chosen);
	pushValueCommand(FieldId::Spec_SquadLogo, tr("Change Squadron Logo"), before, _model->getSquadronLogo(),
		[this](const SCP_string& v) { _model->setSquadronLogo(v); }, true);
}

// Shared by the loading screen line edits (previously unwired) and their
// browse buttons.
void MissionSpecDialog::changeLowResScreen(const SCP_string& name) {
	const SCP_string before = _model->getLowResLoadingScren();
	_model->setLowResLoadingScreen(name);
	pushValueCommand(FieldId::Spec_LoadScreenLow, tr("Change Low Res Loading Screen"), before,
		_model->getLowResLoadingScren(), [this](const SCP_string& v) { _model->setLowResLoadingScreen(v); });
}

void MissionSpecDialog::changeHighResScreen(const SCP_string& name) {
	const SCP_string before = _model->getHighResLoadingScren();
	_model->setHighResLoadingScreen(name);
	pushValueCommand(FieldId::Spec_LoadScreenHigh, tr("Change High Res Loading Screen"), before,
		_model->getHighResLoadingScren(), [this](const SCP_string& v) { _model->setHighResLoadingScreen(v); });
}

void MissionSpecDialog::on_lowResScreen_textChanged(const QString& string) {
	changeLowResScreen(string.toUtf8().constData());
}

void MissionSpecDialog::on_lowResScreenButton_clicked() {
	const QString lastDir = util::getLastDir("missionSpec/lowResScreen", CF_TYPE_INTERFACE);

	const QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), lastDir,
		tr("Image Files (*.dds *.pcx *.jpg *.jpeg *.tga *.png);;DDS (*.dds);;PCX (*.pcx);;JPG (*.jpg *.jpeg);;TGA (*.tga);;PNG (*.png);;All Files (*.*)"));
	if (!filename.isEmpty()) {
		util::saveLastDir("missionSpec/lowResScreen", filename);
		changeLowResScreen(QFileInfo(filename).fileName().toUtf8().constData());
	}
}

void MissionSpecDialog::on_highResScreen_textChanged(const QString& string) {
	changeHighResScreen(string.toUtf8().constData());
}

void MissionSpecDialog::on_highResScreenButton_clicked() {
	const QString lastDir = util::getLastDir("missionSpec/highResScreen", CF_TYPE_INTERFACE);

	const QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), lastDir,
		tr("Image Files (*.dds *.pcx *.jpg *.jpeg *.tga *.png);;DDS (*.dds);;PCX (*.pcx);;JPG (*.jpg *.jpeg);;TGA (*.tga);;PNG (*.png);;All Files (*.*)"));
	if (!filename.isEmpty()) {
		util::saveLastDir("missionSpec/highResScreen", filename);
		changeHighResScreen(QFileInfo(filename).fileName().toUtf8().constData());
	}
}

void MissionSpecDialog::on_supportRearmOptionsButton_clicked()
{
	SupportRearmDialog dlg(this, _viewport);
	dlg.setInitial(_model->getSupportRearmSettings());
	if (dlg.exec() != QDialog::Accepted) {
		return;
	}

	const SupportRearmSettings before = _model->getSupportRearmSettings();
	_model->setSupportRearmSettings(dlg.settings());
	pushValueCommand(FieldId::Spec_SupportRearm, tr("Edit Support Ship Settings"), before,
		_model->getSupportRearmSettings(),
		[this](const SupportRearmSettings& v) { _model->setSupportRearmSettings(v); }, true);
}

void MissionSpecDialog::on_toggleTrail_toggled(bool enabled) {
	const bool before = _model->getMissionFlag(Mission::Mission_Flags::Toggle_ship_trails);
	_model->setMissionFlagDirect(Mission::Mission_Flags::Toggle_ship_trails, enabled);
	pushValueCommand(FieldId::Spec_ToggleTrail, tr("Toggle Ship Trails"), before, enabled, [this](const bool& v) {
		_model->setMissionFlagDirect(Mission::Mission_Flags::Toggle_ship_trails, v);
	});
}

void MissionSpecDialog::on_toggleSpeedDisplay_toggled(bool enabled) {
	const bool before = _model->getTrailThresholdFlag();
	_model->setTrailThresholdFlag(enabled);
	pushValueCommand(FieldId::Spec_SpeedDisplay, tr("Toggle Trail Speed Threshold"), before, enabled,
		[this](const bool& v) { _model->setTrailThresholdFlag(v); });
}

void MissionSpecDialog::on_minDisplaySpeed_valueChanged(int value) {
	const int before = _model->getTrailDisplaySpeed();
	_model->setTrailDisplaySpeed(value);
	pushValueCommand(FieldId::Spec_MinDisplaySpeed, tr("Change Trail Display Speed"), before,
		_model->getTrailDisplaySpeed(), [this](const int& v) { _model->setTrailDisplaySpeed(v); });
}

void MissionSpecDialog::on_senderCombBox_currentIndexChanged(int index) {
	SCP_string sender = ui->senderCombBox->itemData(index).value<QString>().toUtf8().constData();
	const SCP_string before = _model->getCommandSender();
	_model->setCommandSender(sender);
	pushValueCommand(FieldId::Spec_CommandSender, tr("Change Command Sender"), before, _model->getCommandSender(),
		[this](const SCP_string& v) { _model->setCommandSender(v); });
}

void MissionSpecDialog::on_personaComboBox_currentIndexChanged(int index) {
	auto cmdPIndex = ui->personaComboBox->itemData(index).value<int>();
	const int before = _model->getCommandPersona();
	_model->setCommandPersona(cmdPIndex);
	pushValueCommand(FieldId::Spec_CommandPersona, tr("Change Command Persona"), before, _model->getCommandPersona(),
		[this](const int& v) { _model->setCommandPersona(v); });
}

void MissionSpecDialog::on_toggleOverrideHashCommand_toggled(bool checked) {
	const bool before = _model->getMissionFlag(Mission::Mission_Flags::Override_hashcommand);
	_model->setMissionFlagDirect(Mission::Mission_Flags::Override_hashcommand, checked);
	pushValueCommand(FieldId::Spec_OverrideHash, tr("Toggle Command Override"), before, checked, [this](const bool& v) {
		_model->setMissionFlagDirect(Mission::Mission_Flags::Override_hashcommand, v);
	});
}

void MissionSpecDialog::on_defaultMusicCombo_currentIndexChanged(int index) {
	auto defMusicIdx = ui->defaultMusicCombo->itemData(index).value<int>();
	const int before = _model->getEventMusic();
	_model->setEventMusic(defMusicIdx);
	pushValueCommand(FieldId::Spec_EventMusic, tr("Change Event Music"), before, _model->getEventMusic(),
		[this](const int& v) { _model->setEventMusic(v); });
}

void MissionSpecDialog::on_musicPackCombo_currentIndexChanged(int index) {
	SCP_string subMusic = ui->musicPackCombo->itemData(index).value<QString>().toUtf8().constData();
	const SCP_string before = _model->getSubEventMusic();
	_model->setSubEventMusic(subMusic);
	pushValueCommand(FieldId::Spec_SubEventMusic, tr("Change Substitute Music"), before, _model->getSubEventMusic(),
		[this](const SCP_string& v) { _model->setSubEventMusic(v); });
}

void MissionSpecDialog::on_largeShipCollisionGroup_valueChanged(int value)
{
	const int before = _model->getLargeShipNoCollideCollisionGroup();
	_model->setLargeShipNoCollideCollisionGroup(value);
	pushValueCommand(FieldId::Spec_LargeShipGroup, tr("Change Collision Group"), before,
		_model->getLargeShipNoCollideCollisionGroup(),
		[this](const int& v) { _model->setLargeShipNoCollideCollisionGroup(v); });
}

void MissionSpecDialog::on_aiProfileCombo_currentIndexChanged(int index)
{
	auto aipIndex = ui->aiProfileCombo->itemData(index).value<int>();
	const int before = _model->getAIProfileIndex();
	_model->setAIProfileIndex(aipIndex);
	pushValueCommand(FieldId::Spec_AIProfile, tr("Change AI Profile"), before, _model->getAIProfileIndex(),
		[this](const int& v) { _model->setAIProfileIndex(v); });
}

void MissionSpecDialog::on_soundEnvButton_clicked()
{
	SoundEnvironmentDialog dlg(this, _viewport);
	dlg.setInitial(_model->getSoundEnvironmentParams());

	if (dlg.exec() != QDialog::Accepted) {
		return;
	}

	const sound_env before = _model->getSoundEnvironmentParams();
	_model->setSoundEnvironmentParams(dlg.items());
	pushValueCommand(FieldId::Spec_SoundEnv, tr("Edit Sound Environment"), before,
		_model->getSoundEnvironmentParams(), [this](const sound_env& v) { _model->setSoundEnvironmentParams(v); },
		true);
}

void MissionSpecDialog::on_customDataButton_clicked()
{
	CustomDataDialog dlg(this, _viewport);
	dlg.setInitial(_model->getCustomData());

	if (dlg.exec() != QDialog::Accepted) {
		return;
	}

	const SCP_map<SCP_string, SCP_string> before = _model->getCustomData();
	_model->setCustomData(dlg.items());
	pushValueCommand(FieldId::Spec_CustomData, tr("Edit Custom Data"), before, _model->getCustomData(),
		[this](const SCP_map<SCP_string, SCP_string>& v) { _model->setCustomData(v); }, true);
}

void MissionSpecDialog::on_customStringsButton_clicked()
{
	CustomStringsDialog dlg(this, _viewport);
	dlg.setInitial(_model->getCustomStrings());

	if (dlg.exec() != QDialog::Accepted) {
		return;
	}

	const SCP_vector<custom_string> before = _model->getCustomStrings();
	_model->setCustomStrings(dlg.items());
	pushValueCommand(FieldId::Spec_CustomStrings, tr("Edit Custom Strings"), before, _model->getCustomStrings(),
		[this](const SCP_vector<custom_string>& v) { _model->setCustomStrings(v); }, true);
}

void MissionSpecDialog::on_missionDescEditor_textChanged()
{
	SCP_string desc = ui->missionDescEditor->document()->toPlainText().toUtf8().constData();
	const SCP_string before = _model->getMissionDescText();
	_model->setMissionDescText(desc);
	// The setter truncates, so read the stored value back.
	pushValueCommand(FieldId::Spec_MissionDesc, tr("Change Mission Description"), before, _model->getMissionDescText(),
		[this](const SCP_string& v) { _model->setMissionDescText(v); });
}

void MissionSpecDialog::on_designerNoteEditor_textChanged()
{
	SCP_string note = ui->designerNoteEditor->document()->toPlainText().toUtf8().constData();
	const SCP_string before = _model->getDesignerNoteText();
	_model->setDesignerNoteText(note);
	pushValueCommand(FieldId::Spec_DesignerNotes, tr("Change Designer Notes"), before, _model->getDesignerNoteText(),
		[this](const SCP_string& v) { _model->setDesignerNoteText(v); });
}

} // namespace fso::fred::dialogs
