#include <QtWidgets/QDialog>
#include <QAbstractButton>

#include <ui/FredView.h>

#include <mission/commands/FredCommands.h>
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
	void on_playerEntryDelayDoubleSpinBox_valueChanged(double value);
	void on_customWingNameButton_clicked();
	void on_squadronName_textChanged(const QString& string);
	void on_squadronLogoButton_clicked();
	void on_lowResScreen_textChanged(const QString& string);
	void on_lowResScreenButton_clicked();
	void on_highResScreen_textChanged(const QString& string);
	void on_highResScreenButton_clicked();
	void on_supportRearmOptionsButton_clicked();

	// Middle column
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
	void on_largeShipCollisionGroup_valueChanged(int value);
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
	FredView*       _fredView    = nullptr;
	QUndoStack*     _dialogStack = nullptr;

	void initializeUi();
	void updateUi();

	void initFlagList();
	void updateFlags();
	void updateLargeShipCollisionGroup();

	void updateMissionType();
	void updateCmdMessage();
	void updateMusic();
	void updateAIProfiles();
	void updateTextEditors();

	// Shared by the type radios (each maps to one mission-type value) and by
	// the loading screen line edits and their browse buttons.
	void changeMissionType(int type);
	void changeLowResScreen(const SCP_string& name);
	void changeHighResScreen(const SCP_string& name);

	// Every control here is a flat single value: after the live setter ran,
	// this pushes one merging command whose apply re-runs the setter and
	// refreshes the whole (signal-blocked) UI.
	template <typename T, typename Setter>
	void pushValueCommand(int fieldId, const QString& label, const T& before, const T& after, Setter&& setter,
		bool noMerge = false)
	{
		if (before == after) {
			return;
		}

		auto* cmd = new FieldEditCommand<T>(fieldId, nullptr, label, true);
		if (noMerge) {
			cmd->setNoMerge(); // subdialog visits are discrete actions
		}
		cmd->addEntry(before, after, [this, setter](const T& v) {
			setter(v);
			updateUi();
		});
		_dialogStack->push(cmd);
	}
};

} // namespace fso::fred::dialogs
