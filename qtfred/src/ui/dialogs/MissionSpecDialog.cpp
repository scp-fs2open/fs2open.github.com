#include "MissionSpecDialog.h"

#include "ui_MissionSpecDialog.h"

#include "mission\missionparse.h"

#include <QCloseEvent>

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
	connect(ui->missionTitle, static_cast<void (QLineEdit::*)(const QString &)>(&QLineEdit::textChanged), this, &MissionSpecDialog::updateTitle);
	connect(ui->missionDesigner, static_cast<void (QLineEdit::*)(const QString &)>(&QLineEdit::textChanged), this, &MissionSpecDialog::updateDesigner);

	// Mission type
	connect(ui->m_type_SinglePlayer, &QRadioButton::toggled, this, &MissionSpecDialog::singleRadioToggled);
	connect(ui->m_type_MultiPlayer, &QRadioButton::toggled, this, &MissionSpecDialog::multiRadioToggled);
	connect(ui->m_type_Training, &QRadioButton::toggled, this, &MissionSpecDialog::trainingRadioToggled);
	connect(ui->m_type_Cooperative, &QRadioButton::toggled, this, &MissionSpecDialog::coopRadioToggled);
	connect(ui->m_type_TeamVsTeam, &QRadioButton::toggled, this, &MissionSpecDialog::multiTeamRadioToggled);
	connect(ui->m_type_Dogfight, &QRadioButton::toggled, this, &MissionSpecDialog::dogfightRadioToggled);

	// Mission flags - Lambda functions are used to allow the passing of additional parameters to a single method
	connect(ui->toggleAllTeamsAtWar, &QCheckBox::toggled, _model.get(), &MissionSpecDialogModel::setMissionFullWar);
	connect(ui->toggleRedAlert, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Red_alert); });
	connect(ui->toggleScramble, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Scramble); });
	connect(ui->togglePromotion, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_promotion); });
	connect(ui->toggleBuiltinMsg, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_builtin_msgs); });
	connect(ui->toggleBuiltinCmdMsg, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_builtin_command); });
	connect(ui->toggleNoTraitor, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_traitor); });
	connect(ui->toggleBeamFreeDefault, &QCheckBox::toggled, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Beam_free_all_by_default); });
	connect(ui->toggleDaisyChainedDocking, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Allow_dock_trees); });
	connect(ui->toggleNoBriefing, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::No_briefing); });
	connect(ui->toggleDebriefing, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Toggle_debriefing); });
	connect(ui->toggleAutopilotCinematics, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Use_ap_cinematics); });
	connect(ui->toggleHardcodedAutopilot, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Deactivate_ap); });
	connect(ui->toggleAIControlStart, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Player_start_ai); });
	connect(ui->toggle2DMission, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Mission_2d); });
	connect(ui->toggleGoalsInBriefing, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Always_show_goals); });
	connect(ui->toggleMissionEndToMainhall, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::End_to_mainhall); });

	// Support Ships
	connect(ui->toggleHullRepair, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Support_repairs_hull); });
	
	// Ship Trails
	connect(ui->toggleTrail, &QCheckBox::toggled, this, [this](bool param) {flagToggled(param, Mission::Mission_Flags::Toggle_ship_trails); });
	

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
	ui->missionTitle->setText(_model->getMissionTitle().c_str());
	ui->missionDesigner->setText(_model->getDesigner().c_str());

	updateMissionType();
	updateFlags();
	ui->aiProfileCombo->clear();
	for (auto &profile : _model->getAIProfiles()) {
		ui->aiProfileCombo->addItem(QString(profile.c_str()));
	}
}

void MissionSpecDialog::updateTitle() {
	_model->setMissionTitle(ui->missionTitle->text().toStdString());
}

void MissionSpecDialog::updateDesigner() {
	_model->setDesigner(ui->missionDesigner->text().toStdString());
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

void MissionSpecDialog::updateFlags() {
	auto flags = _model->getMissionFlags();
	ui->toggle2DMission->setChecked(flags[Mission::Mission_Flags::Mission_2d]);
	ui->toggleAIControlStart->setChecked(flags[Mission::Mission_Flags::Player_start_ai]);
	ui->toggleAllTeamsAtWar->setChecked(flags[Mission::Mission_Flags::All_attack]);
	ui->toggleAutopilotCinematics->setChecked(flags[Mission::Mission_Flags::Use_ap_cinematics]);
	ui->toggleBeamFreeDefault->setChecked(flags[Mission::Mission_Flags::Beam_free_all_by_default]);
	ui->toggleBuiltinCmdMsg->setChecked(flags[Mission::Mission_Flags::No_builtin_command]);
	ui->toggleBuiltinMsg->setChecked(flags[Mission::Mission_Flags::No_builtin_msgs]);
	ui->toggleDaisyChainedDocking->setChecked(flags[Mission::Mission_Flags::Allow_dock_trees]);
	ui->toggleDebriefing->setChecked(flags[Mission::Mission_Flags::Toggle_debriefing]);
	ui->toggleGoalsInBriefing->setChecked(flags[Mission::Mission_Flags::Always_show_goals]);
	ui->toggleHardcodedAutopilot->setChecked(flags[Mission::Mission_Flags::Deactivate_ap]);
	ui->toggleMissionEndToMainhall->setChecked(flags[Mission::Mission_Flags::End_to_mainhall]);
	ui->toggleNoBriefing->setChecked(flags[Mission::Mission_Flags::No_briefing]);
	ui->toggleNoTraitor->setChecked(flags[Mission::Mission_Flags::No_traitor]);
	ui->togglePromotion->setChecked(flags[Mission::Mission_Flags::No_promotion]);
	ui->toggleRedAlert->setChecked(flags[Mission::Mission_Flags::Red_alert]);
	ui->toggleScramble->setChecked(flags[Mission::Mission_Flags::Scramble]);
}

void MissionSpecDialog::singleRadioToggled(bool enabled) {
	if (enabled) {
		_model->setMissionType(MISSION_TYPE_SINGLE);
	}
}

void MissionSpecDialog::multiRadioToggled(bool enabled) {
	if (enabled) {
		_model->setMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_COOP);	// Coop is the default multiplayer mission type
	}
}

void MissionSpecDialog::trainingRadioToggled(bool enabled) {
	if (enabled) {
		_model->setMissionType(MISSION_TYPE_TRAINING);
	}
}

void MissionSpecDialog::coopRadioToggled(bool enabled) {
	if (enabled) {
		_model->setMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_COOP);
	}
}

void MissionSpecDialog::multiTeamRadioToggled(bool enabled) {
	if (enabled) {
		_model->setMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_TEAMS);
	}
}

void MissionSpecDialog::dogfightRadioToggled(bool enabled) {
	if (enabled) {
		_model->setMissionType(MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_DOGFIGHT);
	}
}

void MissionSpecDialog::flagToggled(bool enabled, Mission::Mission_Flags flag) {
	_model->setMissionFlag(flag, enabled);
}

}
}
}
