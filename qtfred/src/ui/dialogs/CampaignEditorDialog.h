#pragma once

#include <QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QListWidgetItem>
#include <QTextDocument>

#include <memory>

#include "mission/dialogs/CampaignEditorDialogModel.h"
#include "ui/widgets/sexp_tree_view.h"

namespace fso::fred::dialogs {

namespace Ui {
class CampaignEditorDialog;
}

class CampaignEditorDialog : public QMainWindow, public SexpTreeEditorInterface {
	Q_OBJECT

  public:
	explicit CampaignEditorDialog(FredView* parent, EditorViewport* viewport);
	~CampaignEditorDialog() override;

	bool requireCampaignOperators() const override { return true; }

	// Provide the campaign's missions for the OPF_MISSION_NAME data list so the
	// is-previous-event-*/is-previous-goal-* operators have a real list to pick from
	// instead of falling back to the (irrelevant) Mission_filename default.
	SCP_vector<SCP_string> getMissionNames() override;
	bool hasDefaultMissionName() override;

	// Filter the OPF_GOAL_NAME / OPF_EVENT_NAME data lists to the goals/events that
	// belong to the mission named by the operator's first argument. Lazy-loads each
	// referenced mission's goal/event list from disk on first access.
	SCP_vector<SCP_string> getMissionGoals(const SCP_string& reference_name) override;
	SCP_vector<SCP_string> getMissionEvents(const SCP_string& reference_name) override;

  protected:
	void closeEvent(QCloseEvent* e) override; // funnel all Window X presses through reject()

  private slots:
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSave_As_triggered();
	void on_actionExit_triggered();

	void on_nameLineEdit_textChanged(const QString& arg1);
	void on_typeComboBox_currentIndexChanged(int index);
	void on_resetTechAtStartCheckBox_toggled(bool checked);
	void on_campaignCustomDataButton_clicked();
	void on_descriptionPlainTextEdit_textChanged();

	void on_shipsListWidget_itemChanged(QListWidgetItem* item);
	void on_weaponsListWidget_itemChanged(QListWidgetItem* item);

	void on_errorCheckerButton_clicked();

	void on_availableMissionsFilterLineEdit_textChanged(const QString& arg1);
	void on_availableMissionsListWidget_itemSelectionChanged();

	void on_graphView_missionSelected(int missionIndex);
	void on_graphView_specialModeToggleRequested(int missionIndex);
	void on_graphView_addMissionHereRequested(QPointF sceneTopLeft);
	void on_graphView_deleteMissionRequested(int missionIndex);
	void on_graphView_addRepeatBranchRequested(int missionIndex);
	void on_graphView_createMissionAtAndConnectRequested(QPointF sceneTopLeft, int fromIndex, bool isSpecial);
	void on_graphView_setFirstMissionRequested(int missionIndex);

	void on_briefCutsceneComboBox_currentIndexChanged(const QString& arg1);
	void on_debriefingPersonaSpinBox_valueChanged(int arg1);
	void on_mainhallComboBox_currentIndexChanged(const QString& arg1);
	void on_substituteMainhallComboBox_currentIndexChanged(const QString& arg1);

	void on_moveBranchTopButton_clicked();
	void on_moveBranchUpButton_clicked();
	void on_moveBranchDownButton_clicked();
	void on_moveBranchBottomButton_clicked();

	void on_loopDescriptionPlainTextEdit_textChanged();
	void on_loopAnimLineEdit_textChanged(const QString& arg1);
	void on_loopVoiceLineEdit_textChanged(const QString& arg1);
	void on_loopAnimBrowseButton_clicked();
	void on_loopVoiceBrowseButton_clicked();
	void on_testVoiceButton_clicked();

	void on_retailFormatCheckbox_toggled(bool checked);

  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::CampaignEditorDialog> ui;
	std::unique_ptr<ICampaignEditorTreeOps> _treeOps;
	std::unique_ptr<CampaignEditorDialogModel> _model;
	EditorViewport* const _viewport;

	void initializeUi();
	void updateUi();
	void updateTechLists();
	void updateAvailableMissionsList();
	void updateMissionDetails();
	void updateLoopDetails();
	void enableDisableControls();

	bool questionSaveChanges();
};

} // namespace fso::fred::dialogs
