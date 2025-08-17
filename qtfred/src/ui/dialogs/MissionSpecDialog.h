#include <QtWidgets/QDialog>
#include <QAbstractButton>

#include <ui/FredView.h>

#include <mission/dialogs/MissionSpecDialogModel.h>

namespace fso::fred::dialogs {

namespace Ui {
class MissionSpecDialog;
}

class MissionSpecDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MissionSpecDialog(FredView* parent, EditorViewport* viewport);
    ~MissionSpecDialog() override;

	void accept() override;
	void reject() override;

protected:
	void closeEvent(QCloseEvent*) override;

private slots:
	// Dialog controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	// Left column
	void on_missionTitle_textChanged(const QString& string);
	void on_missionDesigner_textChanged(const QString& string);
	void on_m_type_SinglePlayer_toggled(bool checked);
	void on_m_type_MultiPlayer_toggled(bool checked);
	void on_m_type_Training_toggled(bool checked);
	void on_m_type_Cooperative_toggled(bool checked);
	void on_m_type_TeamVsTeam_toggled(bool checked);
	void on_m_type_Dogfight_toggled(bool checked);
	void on_maxRespawnCount_valueChanged(int value);
	void on_respawnDelayCount_valueChanged(int value);
	void on_customWingNameButton_clicked();
	void on_squadronName_textChanged(const QString& string);
	void on_squadronLogoButton_clicked();
	void on_lowResScreenButton_clicked();
	void on_highResScreenButton_clicked();

	// Middle column
	void on_toggleSupportShip_toggled(bool checked);
	void on_toggleHullRepair_toggled(bool checked);
	void on_hullRepairMax_valueChanged(double value);
	void on_subsysRepairMax_valueChanged(double value);
	void on_toggleTrail_toggled(bool checked);
	void on_toggleSpeedDisplay_toggled(bool checked);
	void on_minDisplaySpeed_valueChanged(int value);
	void on_senderCombBox_currentIndexChanged(int index);
	void on_personaComboBox_currentIndexChanged(int index);
	void on_toggleOverrideHashCommand_toggled(bool checked);
	void on_defaultMusicCombo_currentIndexChanged(int index);
	void on_musicPackCombo_currentIndexChanged(int index);

	// Right column
	// flags are dynamically generated and connected
	void on_aiProfileCombo_currentIndexChanged(int index);

	// General
	void on_soundEnvButton_clicked();
	void on_customDataButton_clicked();
	void on_customStringsButton_clicked();
	void on_missionDescEditor_textChanged();
	void on_designerNoteEditor_textChanged();


private: // NOLINT(readability-redundant-access-specifiers)
    std::unique_ptr<Ui::MissionSpecDialog> ui;
	std::unique_ptr<MissionSpecDialogModel> _model;
	EditorViewport* _viewport;

	void initializeUi();
	void updateUi();

	void initFlagList();
	void updateFlags();

	void updateMissionType();
	void updateCmdMessage();
	void updateMusic();
	void updateAIProfiles();
	void updateTextEditors();

	void missionTitleChanged(const QString &);
	void missionDesignerChanged(const QString &);

	void missionTypeToggled(bool, int);

	void maxRespawnChanged(int);
	void respawnDelayChanged(int);

	void squadronNameChanged(const QString &);

	void disallowSupportChanged(bool);
	void hullRepairMaxChanged(double);
	void subsysRepairMaxChanged(double);

	void trailDisplaySpeedToggled(bool);
	void minTrailDisplaySpeedChanged(int);

	void cmdSenderChanged(int);
	void cmdPersonaChanged(int);

	void eventMusicChanged(int);
	void subEventMusicChanged(int);

	void flagToggled(bool enabled, Mission::Mission_Flags flag);

	void missionDescChanged();
	void designerNotesChanged();

	void aiProfileIndexChanged(int index);
	void lightProfileIndexChanged(int index);
	
};

} // namespace fso::fred::dialogs
