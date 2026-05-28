#pragma once

#include <QDialog>

#include <mission/dialogs/PropEditorDialogModel.h>
#include <ui/FredView.h>

namespace Ui {
class PropEditorDialog;
}

namespace fso::fred::dialogs {

class PropEditorDialog : public QDialog {
	Q_OBJECT

 public:
	PropEditorDialog(FredView* parent, EditorViewport* viewport);
	~PropEditorDialog() override;

 protected:
	void changeEvent(QEvent* e) override;

 private slots:
	void on_propNameLineEdit_editingFinished();
	void on_nextButton_clicked();
	void on_prevButton_clicked();
	void on_layerCombo_currentIndexChanged(int index);

 private: // NOLINT(readability-redundant-access-specifiers)
	FredView*       _fredView;
	EditorViewport* _viewport;
	std::unique_ptr<::Ui::PropEditorDialog> ui;
	std::unique_ptr<PropEditorDialogModel> _model;

	void initializeUi();
	void updateUi();
};

}
