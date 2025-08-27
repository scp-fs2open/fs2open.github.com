#include "MissionSpecDialog.h"

#include "ui_MissionSpecDialog.h"

#include <ui/dialogs/General/ImagePickerDialog.h>
#include <ui/dialogs/MissionSpecs/CustomDataDialog.h>
#include <ui/dialogs/MissionSpecs/CustomStringsDialog.h>
#include <ui/dialogs/MissionSpecs/CustomWingNamesDialog.h>
#include <ui/dialogs/MissionSpecs/SoundEnvironmentDialog.h>
#include <ui/util/SignalBlockers.h>
#include "mission/util.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

namespace fso::fred::dialogs {

MissionSpecDialog::MissionSpecDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::MissionSpecDialog()), _model(new MissionSpecDialogModel(this, viewport)),
	_viewport(viewport) {
    ui->setupUi(this);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &MissionSpecDialog::updateUi);

	initializeUi();
	updateUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

MissionSpecDialog::~MissionSpecDialog() = default;

void MissionSpecDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void MissionSpecDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void MissionSpecDialog::closeEvent(QCloseEvent* e) {
	reject();
	e->ignore(); // Don't let the base class close the window
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

	ui->squadronName->setText(_model->getSquadronName().c_str());
	ui->squadronLogo->setEnabled(false);
	ui->squadronLogo->setText(_model->getSquadronLogo().c_str());

	ui->lowResScreen->setText(_model->getLowResLoadingScren().c_str());
	ui->highResScreen->setText(_model->getHighResLoadingScren().c_str());

	ui->toggleSupportShip->setChecked(_model->getDisallowSupport());
	ui->toggleHullRepair->setChecked(_model->getMissionFlag(Mission::Mission_Flags::Support_repairs_hull));
	ui->hullRepairMax->setValue(_model->getHullRepairMax());
	ui->subsysRepairMax->setValue(_model->getSubsysRepairMax());

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

	// per flag immediate apply to the model
	connect(ui->flagList, &fso::fred::FlagListWidget::flagToggled, this, [this](const QString& name, bool checked) {
		_model->setMissionFlag(name.toUtf8().constData(), checked);
	});
}

void MissionSpecDialog::updateFlags()
{
	const auto flags = _model->getMissionFlagsList();

	QVector<std::pair<QString, int>> toWidget;
	toWidget.reserve(static_cast<int>(flags.size()));
	for (const auto& p : flags) {
		QString name = QString::fromUtf8(p.first.c_str());
		toWidget.append({name, p.second});
	}

	ui->flagList->setFlags(toWidget);
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
	_model->setMissionTitle(string.toUtf8().constData());
}

void MissionSpecDialog::on_missionDesigner_textChanged(const QString & string) {
	_model->setDesigner(string.toUtf8().constData());
}

void MissionSpecDialog::on_m_type_SinglePlayer_toggled(bool checked) {
	if (checked) {
		_model->setMissionType(MISSION_TYPE_SINGLE);
	}
}

void MissionSpecDialog::on_m_type_MultiPlayer_toggled(bool checked) {
	if (checked) {
		_model->setMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_COOP);
	}
}

void MissionSpecDialog::on_m_type_Training_toggled(bool checked) {
	if (checked) {
		_model->setMissionType(MISSION_TYPE_TRAINING);
	}
}

void MissionSpecDialog::on_m_type_Cooperative_toggled(bool checked) {
	if (checked) {
		_model->setMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_COOP);
	}
}

void MissionSpecDialog::on_m_type_TeamVsTeam_toggled(bool checked) {
	if (checked) {
		_model->setMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_TEAMS);
	}
}

void MissionSpecDialog::on_m_type_Dogfight_toggled(bool checked) {
	if (checked) {
		_model->setMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_DOGFIGHT);
	}
}

void MissionSpecDialog::on_maxRespawnCount_valueChanged(int value) {
	_model->setNumRespawns(value);
}

void MissionSpecDialog::on_respawnDelayCount_valueChanged(int value) {
	_model->setMaxRespawnDelay(value);
}

void MissionSpecDialog::on_squadronName_textChanged(const QString & string) {
	_model->setSquadronName(string.toUtf8().constData());
}

void MissionSpecDialog::on_customWingNameButton_clicked()
{
	CustomWingNamesDialog dialog(this, _viewport);
	dialog.setInitialStartingWings(_model->getCustomStartingWings());
	dialog.setInitialSquadronWings(_model->getCustomSquadronWings());
	dialog.setInitialTvTWings(_model->getCustomTvTWings());

	if (dialog.exec() == QDialog::Accepted) {
		_model->setCustomStartingWings(dialog.getStartingWings());
		_model->setCustomSquadronWings(dialog.getSquadronWings());
		_model->setCustomTvTWings(dialog.getTvTWings());
	}
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

	const std::string chosen = dlg.selectedFile().toUtf8().constData();
	_model->setSquadronLogo(chosen);
}

void MissionSpecDialog::on_lowResScreenButton_clicked() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.dds *.pcx *.jpg *.jpeg *.tga *.png);;DDS (*.dds);;PCX (*.pcx);;JPG (*.jpg *.jpeg);;TGA (*.tga);;PNG (*.png) ;;All Files (*.*)"));
	if (!(filename.isNull() || filename.isEmpty())) {
		_model->setLowResLoadingScreen(QFileInfo(filename).fileName().toUtf8().constData());
	}
}

void MissionSpecDialog::on_highResScreenButton_clicked() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.dds *.pcx *.jpg *.jpeg *.tga *.png);;DDS (*.dds);;PCX (*.pcx);;JPG (*.jpg *.jpeg);;TGA (*.tga);;PNG (*.png) ;;All Files (*.*)"));
	if (!(filename.isNull() || filename.isEmpty())) {
		_model->setHighResLoadingScreen(QFileInfo(filename).fileName().toUtf8().constData());
	}
}

void MissionSpecDialog::on_toggleSupportShip_toggled(bool enabled) {
	_model->setDisallowSupport(enabled);
}

void MissionSpecDialog::on_toggleHullRepair_toggled(bool enabled) {
	_model->setMissionFlagDirect(Mission::Mission_Flags::Support_repairs_hull, enabled);
}

void MissionSpecDialog::on_hullRepairMax_valueChanged(double value) {
	_model->setHullRepairMax((float)value);
}

void MissionSpecDialog::on_subsysRepairMax_valueChanged(double value) {
	_model->setSubsysRepairMax((float)value);
}

void MissionSpecDialog::on_toggleTrail_toggled(bool enabled) {
	_model->setMissionFlagDirect(Mission::Mission_Flags::Toggle_ship_trails, enabled);
}

void MissionSpecDialog::on_toggleSpeedDisplay_toggled(bool enabled) {
	_model->setTrailThresholdFlag(enabled);
}

void MissionSpecDialog::on_minDisplaySpeed_valueChanged(int value) {
	_model->setTrailDisplaySpeed(value);
}

void MissionSpecDialog::on_senderCombBox_currentIndexChanged(int index) {
	SCP_string sender = ui->senderCombBox->itemData(index).value<QString>().toUtf8().constData();
	_model->setCommandSender(sender);
}

void MissionSpecDialog::on_personaComboBox_currentIndexChanged(int index) {
	auto cmdPIndex = ui->personaComboBox->itemData(index).value<int>();
	_model->setCommandPersona(cmdPIndex);
}

void MissionSpecDialog::on_toggleOverrideHashCommand_toggled(bool checked) {
	_model->setMissionFlagDirect(Mission::Mission_Flags::Override_hashcommand, checked);
}

void MissionSpecDialog::on_defaultMusicCombo_currentIndexChanged(int index) {
	auto defMusicIdx = ui->defaultMusicCombo->itemData(index).value<int>();
	_model->setEventMusic(defMusicIdx);
}

void MissionSpecDialog::on_musicPackCombo_currentIndexChanged(int index) {
	SCP_string subMusic = ui->musicPackCombo->itemData(index).value<QString>().toUtf8().constData();
	_model->setSubEventMusic(subMusic);
}

void MissionSpecDialog::on_aiProfileCombo_currentIndexChanged(int index)
{
	auto aipIndex = ui->aiProfileCombo->itemData(index).value<int>();
	_model->setAIProfileIndex(aipIndex);
}

void MissionSpecDialog::on_soundEnvButton_clicked()
{
	SoundEnvironmentDialog dlg(this, _viewport);
	dlg.setInitial(_model->getSoundEnvironmentParams());

	if (dlg.exec() == QDialog::Accepted) {
		_model->setSoundEnvironmentParams(dlg.items());
	}
}

void MissionSpecDialog::on_customDataButton_clicked()
{
	CustomDataDialog dlg(this, _viewport);
	dlg.setInitial(_model->getCustomData());

	if (dlg.exec() == QDialog::Accepted) {
		_model->setCustomData(dlg.items());
	}
}

void MissionSpecDialog::on_customStringsButton_clicked()
{
	CustomStringsDialog dlg(this, _viewport);
	dlg.setInitial(_model->getCustomStrings());

	if (dlg.exec() == QDialog::Accepted) {
		_model->setCustomStrings(dlg.items());
	}
}

void MissionSpecDialog::on_missionDescEditor_textChanged()
{
	SCP_string desc = ui->missionDescEditor->document()->toPlainText().toUtf8().constData();
	_model->setMissionDescText(desc);
}

void MissionSpecDialog::on_designerNoteEditor_textChanged()
{
	SCP_string note = ui->designerNoteEditor->document()->toPlainText().toUtf8().constData();
	_model->setDesignerNoteText(note);
}

} // namespace fso::fred::dialogs
