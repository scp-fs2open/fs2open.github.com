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

protected:
	void changeEvent(QEvent* e) override;

private slots:
	void on_prevPathButton_clicked();
	void on_nextPathButton_clicked();
	void on_nameEdit_editingFinished();
	void on_noDrawLinesCheck_clicked();
	void on_customColorCheck_clicked();
	void on_colorRSpinBox_valueChanged(int value);
	void on_colorGSpinBox_valueChanged(int value);
	void on_colorBSpinBox_valueChanged(int value);
	void on_layerCombo_currentIndexChanged(int index);

 private: // NOLINT(readability-redundant-access-specifiers)
	FredView*       _fredView;
	EditorViewport* _viewport;
	std::unique_ptr<Ui::WaypointEditorDialog> ui;
	std::unique_ptr<WaypointEditorDialogModel> _model;

	void initializeUi();
	void updateUi();
	void updateColorSwatch();
};

} // namespace fso::fred::dialogs
