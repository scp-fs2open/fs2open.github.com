#pragma once

#include <QDialog>

#include <mission/dialogs/WaypointEditorDialogModel.h>
#include <ui/FredView.h>

#include <memory>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class WaypointEditorDialog;
}

class WaypointEditorDialog : public QDialog {
	Q_OBJECT
public:
	WaypointEditorDialog(FredView* parent, EditorViewport* viewport);
	~WaypointEditorDialog();

	void reject() override;

 protected:
	bool event(QEvent* event) override;

 private:

	void pathSelectionChanged(int index);

	void updateComboBox();

	void updateUI();

	void nameTextChanged(const QString& newText);

	EditorViewport* _viewport = nullptr;
	Editor* _editor = nullptr;
	
	std::unique_ptr<Ui::WaypointEditorDialog> ui;

	std::unique_ptr<WaypointEditorDialogModel> _model;
};

}
}
}

