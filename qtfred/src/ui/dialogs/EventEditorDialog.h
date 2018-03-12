#pragma once

#include <QDialog>

#include "mission/dialogs/EventEditorDialogModel.h"

#include <mission/missiongoals.h>

#include <memory>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class EventEditorDialog;
}

class EventEditorDialog: public QDialog {
	int m_num_events;
	int m_sig[MAX_MISSION_EVENTS];
	mission_event m_events[MAX_MISSION_EVENTS];

 Q_OBJECT
 public:
	EventEditorDialog(QWidget* parent, EditorViewport* viewport);
	~EventEditorDialog();

 private:
	std::unique_ptr<Ui::EventEditorDialog> ui;

	std::unique_ptr<EventEditorDialogModel> _model;
	void initEventTree();
	void load_tree();
	void create_tree();
};

}
}
}

