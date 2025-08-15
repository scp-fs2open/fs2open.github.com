#pragma once
#include <QDialog>
#include <mission/dialogs/WaypointEditorDialogModel.h>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class WaypointEditorDialog;
}

class WaypointEditorDialog : public QDialog {
	Q_OBJECT
public:
	WaypointEditorDialog(FredView* parent, EditorViewport* viewport);
	~WaypointEditorDialog() override;

private slots:
	void on_pathSelection_currentIndexChanged(int index);
	void on_nameEdit_editingFinished();

 private: // NOLINT(readability-redundant-access-specifiers)
	EditorViewport* _viewport;
	std::unique_ptr<Ui::WaypointEditorDialog> ui;
	std::unique_ptr<WaypointEditorDialogModel> _model;

	void initializeUi();
	void updateWaypointListComboBox();
    void updateUi();
};

} // namespace fso::fred::dialogs

