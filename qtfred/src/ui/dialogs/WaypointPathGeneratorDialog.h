#pragma once

#include <mission/dialogs/WaypointPathGeneratorDialogModel.h>
#include <ui/FredView.h>

#include <QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class WaypointPathGeneratorDialog;
}

class WaypointPathGeneratorDialog : public QDialog {
	Q_OBJECT
  public:
	WaypointPathGeneratorDialog(FredView* parent, EditorViewport* viewport);
	~WaypointPathGeneratorDialog() override;

  private slots:
	void on_pathNameLineEdit_editingFinished();

	void on_manualCenterRadio_toggled(bool checked);
	void on_objectCenterRadio_toggled(bool checked);
	void on_centerObjectCombo_currentIndexChanged(int index);
	void on_centerXSpinBox_valueChanged(double value);
	void on_centerYSpinBox_valueChanged(double value);
	void on_centerZSpinBox_valueChanged(double value);

	void on_axisComboBox_currentIndexChanged(int index);
	void on_numPointsSpinBox_valueChanged(int value);
	void on_loopsSpinBox_valueChanged(int value);
	void on_radiusSpinBox_valueChanged(double value);
	void on_driftSpinBox_valueChanged(double value);

	void on_varianceXSpinBox_valueChanged(double value);
	void on_varianceYSpinBox_valueChanged(double value);
	void on_varianceZSpinBox_valueChanged(double value);

	void on_buttonBox_accepted();
	void on_buttonBox_rejected();

  private: // NOLINT(readability-redundant-access-specifiers)
	EditorViewport* _viewport;
	std::unique_ptr<Ui::WaypointPathGeneratorDialog> ui;
	std::unique_ptr<WaypointPathGeneratorDialogModel> _model;

	void initializeUi();
	void updateUi();
	void enableOrDisableControls();
};

} // namespace fso::fred::dialogs
