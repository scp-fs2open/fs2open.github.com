#pragma once

#include <QDialog>
#include <QListWidget>

#include "mission/dialogs/EventEditorDialogModel.h"
#include "ui/widgets/sexp_tree.h"

#include <mission/missiongoals.h>
#include <mission/missionmessage.h>

#include <memory>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class EventEditorDialog;
}

const int MAX_SEARCH_MESSAGE_DEPTH = 5;		// maximum search number of event nodes with message text

class EventEditorDialog: public QDialog, public SexpTreeEditorInterface {
	std::unique_ptr<Ui::EventEditorDialog> ui;

	std::unique_ptr<EventEditorDialogModel> _model;

	int m_num_events = 0;
	int m_sig[MAX_MISSION_EVENTS];
	mission_event m_events[MAX_MISSION_EVENTS];
	int cur_event = -1;
	void set_current_event(int evt);

	int m_num_messages = 0;
	SCP_vector<MMessage> m_messages;
	int m_cur_msg = -1;
	void set_current_message(int msg);

	int m_wave_id = -1;

	// Event data
	uint32_t m_repeat_count = 0;
	uint32_t m_trigger_count = 0;
	uint32_t m_interval = 0;
	int m_event_score = 0;
	int m_chain_delay = 0;
	int m_team = -1;
	bool m_chained = false;
	QString m_obj_text = "";
	QString m_obj_key_text = "";

	// Message data
	int m_last_message_node = -1;

	// Event log data
	bool m_log_true = false;
	bool m_log_false = false;
	bool m_log_always_false = false;
	bool m_log_1st_repeat = false;
	bool m_log_last_repeat = false;
	bool m_log_1st_trigger = false;
	bool m_log_last_trigger = false;
	bool m_log_state_change = false;


	bool modified = false;

	void initEventTree();
	void load_tree();
	void create_tree();

	void initMessageList();
	void initHeadCombo();
	void initWaveFilenames();
	void initPersonas();

	void applyChanges();
	void rejectChanges();

	void messageDoubleClicked(QListWidgetItem* item);

	void createNewMessage();
	void deleteMessage();

	void browseAni();
	void browseWave();

	void playWave();
	void updateStuff();

	void updatePersona();

 Q_OBJECT
 public:
	EventEditorDialog(QWidget* parent, EditorViewport* viewport);
	~EventEditorDialog() override;

	void rootNodeDeleted(int node) override;
	void rootNodeRenamed(int node) override;
	void rootNodeFormulaChanged(int old, int node) override;
	bool hasDefaultMessageParamter() override;
	SCP_vector<SCP_string> getMessages() override;
	int getRootReturnType() const override;

	void rebuildMessageList();
};

}
}
}

