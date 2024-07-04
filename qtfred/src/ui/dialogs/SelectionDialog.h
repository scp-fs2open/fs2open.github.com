#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QCheckBox>

#include <ui/FredView.h>
#include <mission/dialogs/SelectionDialogModel.h>
#include <QtWidgets/QListWidgetItem>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class SelectionDialog;
}

class SelectionDialog : public QDialog {
	Q_OBJECT
public:
	SelectionDialog(FredView* parent, EditorViewport* viewport);
	~SelectionDialog() override;

 private:
	void updateUI();
	void updateListSelection();

	void objectSelectionChanged();

	void wingSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);

	SCP_vector<QCheckBox*> _iffCheckBoxes;

	std::unique_ptr<Ui::SelectionDialog> ui;
	std::unique_ptr<SelectionDialogModel> _model;
};


}
}
}


