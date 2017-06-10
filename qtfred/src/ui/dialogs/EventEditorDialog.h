#pragma once

#include <QDialog>

#include <mission/dialogs/EventEditorDialogModel.h>

#include <memory>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class EventEditorDialog;
}

class EventEditorDialog: public QDialog {
 Q_OBJECT
 public:
	EventEditorDialog(QWidget* parent, Editor* editor, FredRenderer* renderer);
	~EventEditorDialog();

 private:
	std::unique_ptr<Ui::EventEditorDialog> ui;

	std::unique_ptr<EventEditorDialogModel> _model;
};

}
}
}

