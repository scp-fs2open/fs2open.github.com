#include "MissionSpecDialog.h"

#include "ui_MissionSpecDialog.h"

#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>
#include <QFileDialog>

namespace fso {
namespace fred {
namespace dialogs {

MissionSpecDialog::MissionSpecDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::MissionSpecDialog()), _model(new MissionSpecDialogModel(this, viewport)),
	_viewport(viewport) {
    ui->setupUi(this);

	connect(this, &QDialog::accepted, _model.get(), &MissionSpecDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &MissionSpecDialogModel::reject);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &MissionSpecDialog::updateUI);

	// Mission title and creator
	connect(ui->missionTitle, static_cast<void (QLineEdit::*)(const QString &)>(&QLineEdit::textChanged), this, &MissionSpecDialog::missionTitleChanged);
	connect(ui->missionDesigner, static_cast<void (QLineEdit::*)(const QString &)>(&QLineEdit::textChanged), this, &MissionSpecDialog::missionDesignerChanged);

	// Mission type
	connect(ui->m_type_SinglePlayer, &QRadioButton::toggled, this, [this](bool param) {missionTypeToggled(param, MISSION_TYPE_SINGLE); });
	connect(ui->m_type_MultiPlayer, &QRadioButton::toggled, this, [this](bool param) {missionTypeToggled(param, MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_COOP); });
	connect(ui->m_type_Training, &QRadioButton::toggled, this, [this](bool param) {missionTypeToggled(param, MISSION_TYPE_TRAINING); });
	connect(ui->m_type_Cooperative, &QRadioButton::toggled, this, [this](bool param) {missionTypeToggled(param, MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_COOP); });
	connect(ui->m_type_TeamVsTeam, &QRadioButton::toggled, this, [this](bool param) {missionTypeToggled(param, MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_TEAMS); });
	connect(ui->m_type_Dogfight, &QRadioButton::toggled, this, [this](bool param) {missionTypeToggled(param, MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_DOGFIGHT); });

	// Respawn info
	connect(ui->maxRespawnCount, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MissionSpecDialog::maxRespawnChanged);
	connect(ui->respawnDelayCount, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MissionSpecDialog::respawnDelayChanged);

	// Custom Wing Names
	// Placeholder - UI and Model need to be made

	// Squadron Reassign
	connect(ui->squadronName, &QLineEdit::textChanged, this, &MissionSpecDialog::squadronNameChanged);
	
	// Loading Screen - Nothing to connect as buttons are connected directly to slots

	// Support Ships
	connect(ui->toggleSupportShip, &QCheckBox::toggled, this, &MissionSpecDialog::disallowSupportChanged);
	connect(ui->toggleHullRepair, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Support_repairs_hull); });
	connect(ui->hullRepairMax, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MissionSpecDialog::hullRepairMaxChanged);
	connect(ui->subsysRepairMax, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MissionSpecDialog::subsysRepairMaxChanged);

	// Ship Trails
	connect(ui->toggleTrail, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Toggle_ship_trails); });
	connect(ui->toggleSpeedDisplay, &QCheckBox::toggled, this, &MissionSpecDialog::trailDisplaySpeedToggled);
	connect(ui->minDisplaySpeed, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MissionSpecDialog::minTrailDisplaySpeedChanged);

	// Built-in Command Messages
	connect(ui->senderCombBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this, &MissionSpecDialog::cmdSenderChanged);
	connect(ui->personaComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MissionSpecDialog::cmdPersonaChanged);

	// Mission Music
	
	// Sound Environment
	
	// Mission flags - Lambda functions are used to allow the passing of additional parameters to a single method
	connect(ui->toggleAllTeamsAtWar, &QCheckBox::toggled, _model.get(), &MissionSpecDialogModel::setMissionFullWar);
	connect(ui->toggleRedAlert, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Red_alert); });
	connect(ui->toggleScramble, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Scramble); });
	connect(ui->togglePromotion, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_promotion); });
	connect(ui->toggleBuiltinMsg, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_builtin_msgs); });
	connect(ui->toggleBuiltinCmdMsg, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_builtin_command); });
	connect(ui->toggleNoTraitor, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_traitor); });
	connect(ui->toggleBeamFreeDefault, &QCheckBox::toggled, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Beam_free_all_by_default); });
	connect(ui->toggleNoBriefing, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_briefing); });
	connect(ui->toggleDebriefing, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Toggle_debriefing); });
	connect(ui->toggleAutopilotCinematics, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Use_ap_cinematics); });
	connect(ui->toggleHardcodedAutopilot, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Deactivate_ap); });
	connect(ui->toggleAIControlStart, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Player_start_ai); });
	connect(ui->toggleChaseViewStart, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Player_start_chase_view); });
	connect(ui->toggle2DMission, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Mission_2d); });
	connect(ui->toggleGoalsInBriefing, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Always_show_goals); });
	connect(ui->toggleMissionEndToMainhall, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::End_to_mainhall); });

	// AI Profiles
	connect(ui->aiProfileCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MissionSpecDialog::aiProfileIndexChanged);

	// Mission Description
	connect(ui->missionDescEditor,&QPlainTextEdit::textChanged, this, &MissionSpecDialog::missionDescChanged);

	// Designer Notes
	connect(ui->designerNoteEditor, &QPlainTextEdit::textChanged, this, &MissionSpecDialog::designerNotesChanged);

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

MissionSpecDialog::~MissionSpecDialog() {
}

void MissionSpecDialog::closeEvent(QCloseEvent* event) {
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question, "Changes detected", "Do you want to keep your changes?",
		{ DialogButton::Yes, DialogButton::No, DialogButton::Cancel });

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

void MissionSpecDialog::updateUI() {
	util::SignalBlockers blockers(this);

	ui->missionTitle->setText(_model->getMissionTitle().c_str());
	ui->missionDesigner->setText(_model->getDesigner().c_str());

	ui->createdLabel->setText(_model->getCreatedTime().c_str());
	ui->modifiedLabel->setText(_model->getModifiedTime().c_str());

	updateMissionType();

	ui->maxRespawnCount->setEnabled(_model->getMissionType() & MISSION_TYPE_MULTI);
	ui->respawnDelayCount->setEnabled(_model->getMissionType() & MISSION_TYPE_MULTI);
	ui->maxRespawnCount->setValue(_model->getNumRespawns());
	ui->respawnDelayCount->setValue(_model->getMaxRespawnDelay());

	ui->squadronName->setText(_model->getSquadronName().c_str());
	ui->squadronLogo->setEnabled(false);
	ui->squadronLogo->setText(_model->getSquadronLogo().c_str());

	ui->lowResScreen->setText(_model->getLowResLoadingScren().c_str());
	ui->highResScreen->setText(_model->getHighResLoadingScren().c_str());

	ui->toggleSupportShip->setChecked(_model->getDisallowSupport());
	ui->hullRepairMax->setValue(_model->getHullRepairMax());
	ui->subsysRepairMax->setValue(_model->getSubsysRepairMax());

	ui->toggleSpeedDisplay->setChecked(_model->getTrailThresholdFlag());
	ui->minDisplaySpeed->setEnabled(_model->getTrailThresholdFlag());
	ui->minDisplaySpeed->setValue(_model->getTrailDisplaySpeed());

	updateCmdMessage();

	updateMusic();

	updateFlags();

	updateAIProfiles();

	updateTextEditors();
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
		if (Ships[i].objnum >= 0)
			if (Ship_info[Ships[i].ship_info_index].is_huge_ship())
				ui->senderCombBox->addItem(Ships[i].ship_name, QVariant(QString(Ships[i].ship_name)));
	}
	ui->senderCombBox->setCurrentIndex(ui->senderCombBox->findText(sender.c_str()));

	save_idx = _model->getCommandPersona();
	ui->personaComboBox->clear();
	for (i = 0; i < Num_personas; i++) {
		if (Personas[i].flags & PERSONA_FLAG_COMMAND) {
			ui->personaComboBox->addItem(Personas[i].name, QVariant(i));
		}
	}
	ui->personaComboBox->setCurrentIndex(ui->personaComboBox->findData(save_idx));
}

void MissionSpecDialog::updateMusic() {
	int i, idx;

	idx = _model->getEventMusic();
	ui->defaultMusicCombo->clear();
	ui->defaultMusicCombo->addItem("None",QVariant(0));
	for (i = 0; i < Num_soundtracks; i++) {
		ui->defaultMusicCombo->addItem(Soundtracks[i].name, QVariant(i+1));
	}
	ui->defaultMusicCombo->setCurrentIndex(ui->defaultMusicCombo->findData(idx));

	auto musicPack = _model->getSubEventMusic();
	ui->musicPackCombo->clear();
	ui->musicPackCombo->addItem("None");
	for (i = 0; i < Num_soundtracks; i++) {
		ui->musicPackCombo->addItem(Soundtracks[i].name, QVariant(QString(Soundtracks[i].name)));
	}
	ui->musicPackCombo->setCurrentIndex(ui->musicPackCombo->findText(musicPack.c_str()));
}

void MissionSpecDialog::updateFlags() {
	auto flags = _model->getMissionFlags();
	ui->toggle2DMission->setChecked(flags[Mission::Mission_Flags::Mission_2d]);
	ui->toggleAIControlStart->setChecked(flags[Mission::Mission_Flags::Player_start_ai]);
	ui->toggleChaseViewStart->setChecked(flags[Mission::Mission_Flags::Player_start_chase_view]);
	ui->toggleAllTeamsAtWar->setChecked(flags[Mission::Mission_Flags::All_attack]);
	ui->toggleAutopilotCinematics->setChecked(flags[Mission::Mission_Flags::Use_ap_cinematics]);
	ui->toggleBeamFreeDefault->setChecked(flags[Mission::Mission_Flags::Beam_free_all_by_default]);
	ui->toggleBuiltinCmdMsg->setChecked(flags[Mission::Mission_Flags::No_builtin_command]);
	ui->toggleBuiltinMsg->setChecked(flags[Mission::Mission_Flags::No_builtin_msgs]);
	ui->toggleDebriefing->setChecked(flags[Mission::Mission_Flags::Toggle_debriefing]);
	ui->toggleGoalsInBriefing->setChecked(flags[Mission::Mission_Flags::Always_show_goals]);
	ui->toggleHardcodedAutopilot->setChecked(flags[Mission::Mission_Flags::Deactivate_ap]);
	ui->toggleMissionEndToMainhall->setChecked(flags[Mission::Mission_Flags::End_to_mainhall]);
	ui->toggleNoBriefing->setChecked(flags[Mission::Mission_Flags::No_briefing]);
	ui->toggleNoTraitor->setChecked(flags[Mission::Mission_Flags::No_traitor]);
	ui->togglePromotion->setChecked(flags[Mission::Mission_Flags::No_promotion]);
	ui->toggleRedAlert->setChecked(flags[Mission::Mission_Flags::Red_alert]);
	ui->toggleScramble->setChecked(flags[Mission::Mission_Flags::Scramble]);
	ui->toggleHullRepair->setChecked(flags[Mission::Mission_Flags::Support_repairs_hull]);
	ui->toggleTrail->setChecked(flags[Mission::Mission_Flags::Toggle_ship_trails]);
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

void MissionSpecDialog::missionTitleChanged(const QString & string) {
	_model->setMissionTitle(string.toStdString());
}

void MissionSpecDialog::missionDesignerChanged(const QString & string) {
	_model->setDesigner(string.toStdString());
}

void MissionSpecDialog::missionTypeToggled(bool enabled, int m_type) {
	if (enabled) {
		_model->setMissionType(m_type);
	}
}

void MissionSpecDialog::maxRespawnChanged(int value) {
	_model->setNumRespawns(value);
}

void MissionSpecDialog::respawnDelayChanged(int value) {
	_model->setMaxRespawnDelay(value);
}

void MissionSpecDialog::squadronNameChanged(const QString & string) {
	_model->setSquadronName(string.toStdString());
}

void MissionSpecDialog::on_customWingNameButton_clicked() {
	CustomWingNamesDialog* dialog = new CustomWingNamesDialog(this, _viewport);
	dialog->exec();
}

void MissionSpecDialog::on_squadronLogoButton_clicked() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.dds *.pcx);;DDS (*.dds);;PCX(*.pcx);;All Files (*.*)"));
	if (!(filename.isNull() || filename.isEmpty())) {
		_model->setSquadronLogo(QFileInfo(filename).fileName().toStdString());
	}
}

void MissionSpecDialog::on_lowResScreenButton_clicked() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.dds *.pcx *.jpg *.jpeg *.tga *.png);;DDS (*.dds);;PCX (*.pcx);;JPG (*.jpg *.jpeg);;TGA (*.tga);;PNG (*.png) ;;All Files (*.*)"));
	if (!(filename.isNull() || filename.isEmpty())) {
		_model->setLowResLoadingScreen(QFileInfo(filename).fileName().toStdString());
	}
}

void MissionSpecDialog::on_highResScreenButton_clicked() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.dds *.pcx *.jpg *.jpeg *.tga *.png);;DDS (*.dds);;PCX (*.pcx);;JPG (*.jpg *.jpeg);;TGA (*.tga);;PNG (*.png) ;;All Files (*.*)"));
	if (!(filename.isNull() || filename.isEmpty())) {
		_model->setHighResLoadingScreen(QFileInfo(filename).fileName().toStdString());
	}
}

void MissionSpecDialog::disallowSupportChanged(bool enabled) {
	_model->setDisallowSupport(enabled);
}

void MissionSpecDialog::hullRepairMaxChanged(double value) {
	_model->setHullRepairMax((float)value);
}

void MissionSpecDialog::subsysRepairMaxChanged(double value) {
	_model->setSubsysRepairMax((float)value);
}

void MissionSpecDialog::trailDisplaySpeedToggled(bool enabled) {
	_model->setTrailThresholdFlag(enabled);
}

void MissionSpecDialog::minTrailDisplaySpeedChanged(int value) {
	_model->setTrailDisplaySpeed(value);
}

void MissionSpecDialog::cmdSenderChanged(int index) {
	auto sender = ui->senderCombBox->itemData(index).value<QString>().toStdString();
	_model->setCommandSender(sender);
}

void MissionSpecDialog::cmdPersonaChanged(int index) {
	auto cmdPIndex = ui->personaComboBox->itemData(index).value<int>();
	_model->setCommandPersona(cmdPIndex);
}

void MissionSpecDialog::eventMusicChanged(int index) {
	auto defMusicIdx = ui->defaultMusicCombo->itemData(index).value<int>();
	_model->setEventMusic(defMusicIdx);
}

void MissionSpecDialog::subEventMusicChanged(int index) {
	auto subMusic = ui->musicPackCombo->itemData(index).value<QString>().toStdString();
	_model->setSubEventMusic(subMusic);
}

void MissionSpecDialog::flagToggled(bool enabled, Mission::Mission_Flags flag) {
	_model->setMissionFlag(flag, enabled);
}

void MissionSpecDialog::missionDescChanged() {
	_model->setMissionDescText(ui->missionDescEditor->document()->toPlainText().toStdString());
}

void MissionSpecDialog::designerNotesChanged() {
	_model->setDesignerNoteText(ui->designerNoteEditor->document()->toPlainText().toStdString());
}

void MissionSpecDialog::aiProfileIndexChanged(int index) {
	auto aipIndex = ui->aiProfileCombo->itemData(index).value<int>();
	_model->setAIProfileIndex(aipIndex);
}

}
}
}
