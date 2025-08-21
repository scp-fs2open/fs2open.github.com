#pragma once

#include "mission/dialogs/MissionEventsDialogModel.h"

#include <QDialog>
#include <QListWidget>

#include "ui/widgets/sexp_tree.h"

#include <mission/missiongoals.h>
#include <mission/missionmessage.h>

#include <memory>

class QCheckBox;

namespace fso::fred::dialogs {

namespace Ui {
class MissionEventsDialog;
}

class MissionEventsDialog: public QDialog, public SexpTreeEditorInterface {
	Q_OBJECT

  public:
	explicit MissionEventsDialog(QWidget* parent, EditorViewport* viewport);
	~MissionEventsDialog() override;

	void accept() override;
	void reject() override;

	SCP_vector<SCP_string> getMessages() override;
	bool hasDefaultMessageParamter() override;

  protected:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_btnNewEvent_clicked();
    void on_btnInsertEvent_clicked();
	void on_btnDeleteEvent_clicked();
	void on_eventUpBtn_clicked();
	void on_eventDownBtn_clicked();

	void on_repeatCountBox_valueChanged(int value);
	void on_triggerCountBox_valueChanged(int value);
	void on_intervalTimeBox_valueChanged(int value);
	void on_chainedCheckBox_stateChanged(int state);
	void on_chainedDelayBox_valueChanged(int value);
	void on_scoreBox_valueChanged(int value);
	void on_teamCombo_currentIndexChanged(int index);
	
	void on_editDirectiveText_textChanged(const QString& text);
	void on_editDirectiveKeypressText_textChanged(const QString& text);

	void on_checkLogTrue_stateChanged(int state);
	void on_checkLogFalse_stateChanged(int state);
	void on_checkLogPrevious_stateChanged(int state);
	void on_checkLogAlwaysFalse_stateChanged(int state);
	void on_checkLogFirstRepeat_stateChanged(int state);
	void on_checkLogLastRepeat_stateChanged(int state);
	void on_checkLogFirstTrigger_stateChanged(int state);
	void on_checkLogLastTrigger_stateChanged(int state);

	void on_messageList_currentRowChanged(int row);
	void on_messageList_itemDoubleClicked(QListWidgetItem* item);

	void on_btnNewMsg_clicked();
	void on_btnInsertMsg_clicked();
	void on_btnDeleteMsg_clicked();
	void on_msgUpBtn_clicked();
	void on_msgDownBtn_clicked();

	void on_messageName_textChanged(const QString& text);
	void on_messageContent_textChanged();
	void on_btnMsgNote_clicked();
	void on_aniCombo_editingFinished();
	void on_aniCombo_selectedIndexChanged(int index);
	void on_btnAniBrowse_clicked();
	void on_waveCombo_editingFinished();
	void on_waveCombo_selectedIndexChanged(int index);
	void on_btnBrowseWave_clicked();
	void on_btnWavePlay_clicked();
	void on_personaCombo_currentIndexChanged(int index);
	void on_btnUpdateStuff_clicked();
	void on_messageTeamCombo_currentIndexChanged(int index);


private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::MissionEventsDialog> ui;
	EditorViewport* _viewport;
	std::unique_ptr<IEventTreeOps> _treeOps;
	std::unique_ptr<MissionEventsDialogModel> _model;

	int m_last_message_node = -1;
	QString m_last_message_name;

	void updateEventUi();
	void updateEventMoveButtons();
	void updateMessageUi();
	void updateMessageMoveButtons();

	void initMessageList();
	void initHeadCombo();
	void initWaveFilenames();
	void initPersonas();

	void initMessageTeams();
	void initEventTeams();

	void rootNodeDeleted(int node);
	void rootNodeRenamed(int node);
	void rootNodeFormulaChanged(int old, int node);
	void rootNodeSelectedByFormula(int formula);

	void rebuildMessageList();
	void initMessageWidgets();
	void initEventWidgets();
	void updateEventBitmap();

	static SCP_vector<int> read_root_formula_order(sexp_tree* tree);
};

} // namespace fso::fred::dialogs

