#pragma once
#include <mission/dialogs/JumpNodeEditorDialogModel.h>
#include <mission/commands/FredCommands.h>
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

protected:
	void changeEvent(QEvent* e) override;

private slots:
	void on_prevNodeButton_clicked();
	void on_nextNodeButton_clicked();
	void on_nameLineEdit_editingFinished();
	void on_displayNameLineEdit_editingFinished();
	void on_modelFileLineEdit_editingFinished();
	void on_redSpinBox_valueChanged(int value);
	void on_greenSpinBox_valueChanged(int value);
	void on_blueSpinBox_valueChanged(int value);
	void on_alphaSpinBox_valueChanged(int value);
	void on_hiddenByDefaultCheckBox_clicked();
	void on_layerCombo_currentIndexChanged(int index);

private: // NOLINT(readability-redundant-access-specifiers)
	FredView*       _fredView;
	EditorViewport* _viewport;
	std::unique_ptr<Ui::JumpNodeEditorDialog> ui;
	std::unique_ptr<JumpNodeEditorDialogModel> _model;

	void initializeUi();
	void updateUi();
	void updateColorSwatch();
};

} // namespace fso::fred::dialogs
