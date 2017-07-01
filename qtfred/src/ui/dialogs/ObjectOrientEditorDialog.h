#pragma once

#include <QtWidgets/QDialog>

#include <ui/FredView.h>

#include <mission/dialogs/ObjectOrientEditorDialogModel.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ObjectOrientEditorDialog;
}

class ObjectOrientEditorDialog : public QDialog {
 public:
	ObjectOrientEditorDialog(FredView* parent, EditorViewport* viewport);
	~ObjectOrientEditorDialog();


protected:
	void closeEvent(QCloseEvent*) override;

private:
	std::unique_ptr<Ui::ObjectOrientEditorDialog> ui;
	std::unique_ptr<ObjectOrientEditorDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();
	void updateComboBox();

	void objectSelectionChanged(int index);

	void objectRadioToggled(bool enabled);
	void locationRadioToggled(bool enabled);

	void pointToChecked(bool checked);

	void positionValueChangedX(double value);
	void positionValueChangedY(double value);
	void positionValueChangedZ(double value);

	void locationValueChangedX(double value);
	void locationValueChangedY(double value);
	void locationValueChangedZ(double value);
};


}
}
}
