#pragma once

#include <QtWidgets/QDialog>

#include <ui/FredView.h>

#include <mission/commands/FredCommands.h>
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

	void changeSetMode(ObjectOrientEditorDialogModel::SetMode mode);

	// Every remaining control is a flat single value: after the live setter
	// ran, this pushes one merging command whose apply re-runs the setter and
	// refreshes the whole (signal-blocked) UI.
	template <typename T, typename Setter>
	void pushValueCommand(int fieldId, const QString& label, const T& before, const T& after, Setter&& setter)
	{
		if (before == after) {
			return;
		}

		auto* cmd = new FieldEditCommand<T>(fieldId, nullptr, label, true);
		cmd->addEntry(before, after, [this, setter](const T& v) {
			setter(v);
			updateUi();
		});
		_dialogStack->push(cmd);
	}

	// Boilerplate
	std::unique_ptr<Ui::ObjectOrientEditorDialog> ui;
	std::unique_ptr<ObjectOrientEditorDialogModel> _model;
	EditorViewport* _viewport;
	FredView*       _fredView    = nullptr;
	QUndoStack*     _dialogStack = nullptr;

	// Group updates
	void updatePosition();
	void updateOrientation();
	void updateModes();
	void updatePointTo();
	void updateComboBox();
	void updateLocation();
};


} // namespace fso::fred::dialogs
