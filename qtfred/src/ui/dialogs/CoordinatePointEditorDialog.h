#pragma once
#include <QDialog>
#include <mission/dialogs/CoordinatePointEditorDialogModel.h>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class CoordinatePointEditorDialog;
}

class CoordinatePointEditorDialog : public QDialog {
	Q_OBJECT
public:
	CoordinatePointEditorDialog(FredView* parent, EditorViewport* viewport);
	~CoordinatePointEditorDialog() override;

private slots:
	void on_prevPointButton_clicked();
	void on_nextPointButton_clicked();
	void on_nameEdit_editingFinished();
	void on_categoryEdit_editingFinished();
	void on_shapeCombo_currentIndexChanged(int index);
	void on_sizeSpinBox_valueChanged(double value);
	void on_escortPrioritySpinBox_valueChanged(int value);
	void on_multiTeamCombo_currentIndexChanged(int index);
	void on_layerCombo_currentIndexChanged(int index);
	void on_visibleInMissionCheck_clicked();
	void on_colorRSpinBox_valueChanged(int value);
	void on_colorGSpinBox_valueChanged(int value);
	void on_colorBSpinBox_valueChanged(int value);
	void on_colorASpinBox_valueChanged(int value);

private:
	EditorViewport* _viewport;
	std::unique_ptr<Ui::CoordinatePointEditorDialog> ui;
	std::unique_ptr<CoordinatePointEditorDialogModel> _model;

	void initializeUi();
	void updateUi();
	void updateColorSwatch();
};

} // namespace fso::fred::dialogs
