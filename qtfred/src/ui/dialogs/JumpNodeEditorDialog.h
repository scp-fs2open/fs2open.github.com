#pragma once
#include <mission/dialogs/JumpNodeEditorDialogModel.h>
#include <ui/FredView.h>

#include <QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class JumpNodeEditorDialog;
}

class JumpNodeEditorDialog : public QDialog {
	Q_OBJECT
  public:
	JumpNodeEditorDialog(FredView* parent, EditorViewport* viewport);
	~JumpNodeEditorDialog() override;

  private slots:
	void on_selectJumpNodeComboBox_currentIndexChanged(int index);
	void on_nameLineEdit_editingFinished();
	void on_displayNameLineEdit_editingFinished();
	void on_modelFileLineEdit_editingFinished();
	void on_redSpinBox_valueChanged(int value);
	void on_greenSpinBox_valueChanged(int value);
	void on_blueSpinBox_valueChanged(int value);
	void on_alphaSpinBox_valueChanged(int value);
	void on_hiddenByDefaultCheckBox_toggled(bool checked);

  private: // NOLINT(readability-redundant-access-specifiers)
	EditorViewport* _viewport;
	std::unique_ptr<Ui::JumpNodeEditorDialog> ui;
	std::unique_ptr<JumpNodeEditorDialogModel> _model;

	void initializeUi();
	void updateJumpNodeListComboBox();
	void updateUi();
	void enableOrDisableControls();
};

} // namespace fso::fred::dialogs
