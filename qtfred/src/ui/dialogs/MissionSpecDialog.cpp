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

	connect(ui->missionDesigner, static_cast<void (QLineEdit::*)(const QString &)>(&QLineEdit::textEdited), this, &MissionSpecDialog::updateDesigner);

	connect(ui->m_type_SinglePlayer, &QRadioButton::toggled, this, &MissionSpecDialog::singleRadioToggled);
	connect(ui->m_type_MultiPlayer, &QRadioButton::toggled, this, &MissionSpecDialog::multiRadioToggled);
	connect(ui->m_type_Training, &QRadioButton::toggled, this, &MissionSpecDialog::trainingRadioToggled);
	connect(ui->m_type_Cooperative, &QRadioButton::toggled, this, &MissionSpecDialog::coopRadioToggled);
	connect(ui->m_type_TeamVsTeam, &QRadioButton::toggled, this, &MissionSpecDialog::multiTeamRadioToggled);
	connect(ui->m_type_Dogfight, &QRadioButton::toggled, this, &MissionSpecDialog::dogfightRadioToggled);

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
	ui->aiProfileCombo->clear();
	for (auto &i : _model->getAIProfiles()) {
		ui->aiProfileCombo->addItem(QString(i.c_str()));
	}

	updateMissionType();
}

void MissionSpecDialog::updateDesigner(const QString &designerName) {
	_model->setDesigner(designerName.toStdString());
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

}
}
}
