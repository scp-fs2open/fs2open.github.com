#pragma once

#include <QDialog>
#include <QListWidget>

#include "ui/widgets/sexp_tree.h"

#include <mission/missiongoals.h>
#include <mission/missionmessage.h>

#include <memory>

class QCheckBox;

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class EventEditorDialog;
}

const int MAX_SEARCH_MESSAGE_DEPTH = 5;		// maximum search number of event nodes with message text

class EventEditorDialog: public QDialog, public SexpTreeEditorInterface {
	std::unique_ptr<Ui::EventEditorDialog> ui;

	Editor* _editor = nullptr;

	SCP_vector<int> m_sig;
	SCP_vector<mission_event> m_events;
	int cur_event = -1;
	void set_current_event(int evt);

	SCP_vector<MMessage> m_messages;
	int m_cur_msg = -1;
	void set_current_message(int msg);

	int m_wave_id = -1;

	// Message data
	int m_last_message_node = -1;

	void connectCheckBox(QCheckBox* box, bool* var);

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

	void rebuildMessageList();

	void initMessageWidgets();

	void initEventWidgets();

	void updateEventBitmap();

	void connectLogState(QCheckBox* box, uint32_t flag);

	void newEventHandler();
	void insertEventHandler();
	void deleteEventHandler();
	QTreeWidgetItem* get_event_handle(int num);
	void reset_event(int num, QTreeWidgetItem* after);

	bool query_modified();
 protected:
	void keyPressEvent(QKeyEvent* event) override;
 Q_OBJECT
 protected:
	void closeEvent(QCloseEvent* event) override;
 public:
	EventEditorDialog(QWidget* parent, EditorViewport* viewport);
	~EventEditorDialog() override;

	void rootNodeDeleted(int node);
	void rootNodeRenamed(int node);
	void rootNodeFormulaChanged(int old, int node);

	bool hasDefaultMessageParamter() override;
	SCP_vector<SCP_string> getMessages() override;
	int getRootReturnType() const override;
};

}
}
}

