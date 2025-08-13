#pragma once

#include <QtWidgets/QDialog>

#include <ui/FredView.h>

#include <mission/dialogs/ObjectOrientEditorDialogModel.h>

namespace fso::fred::dialogs {

namespace Ui {
class ObjectOrientEditorDialog;
}

class ObjectOrientEditorDialog : public QDialog {
	Q_OBJECT
 public:
	ObjectOrientEditorDialog(FredView* parent, EditorViewport* viewport);
	~ObjectOrientEditorDialog() override;

	void accept() override;
	void reject() override;

protected:
	void closeEvent(QCloseEvent* e) override; // funnel all Window X presses through reject()

private slots:
	// dialog controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();
	// Position
	void on_positionXSpinBox_valueChanged(double value);
	void on_positionYSpinBox_valueChanged(double value);
	void on_positionZSpinBox_valueChanged(double value);
	// Orientation
	void on_orientationPSpinBox_valueChanged(double value);
	void on_orientationBSpinBox_valueChanged(double value);
	void on_orientationHSpinBox_valueChanged(double value);
	// Settings
	void on_setAbsoluteRadioButton_toggled(bool checked);
	void on_setRelativeRadioButton_toggled(bool checked);
	void on_transformIndependentlyRadioButton_toggled(bool checked);
	void on_transformRelativelyRadioButton_toggled(bool checked);
	// Point to
	void on_pointToCheckBox_toggled(bool checked);
	void on_objectRadioButton_toggled(bool checked);
	void on_objectComboBox_currentIndexChanged(int index);
	void on_locationRadioButton_toggled(bool checked);
	void on_locationXSpinBox_valueChanged(double value);
	void on_locationYSpinBox_valueChanged(double value);
	void on_locationZSpinBox_valueChanged(double value);


private: // NOLINT(readability-redundant-access-specifiers)
	void initializeUi();
	void updateUi();
	void enableOrDisableControls();

	// Boilerplate
	std::unique_ptr<Ui::ObjectOrientEditorDialog> ui;
	std::unique_ptr<ObjectOrientEditorDialogModel> _model;
	EditorViewport* _viewport;

	// Group updates
	void updatePosition();
	void updateOrientation();
	void updatePointTo();
	void updateComboBox();
	void updateLocation();
};


} // namespace fso::fred::dialogs
