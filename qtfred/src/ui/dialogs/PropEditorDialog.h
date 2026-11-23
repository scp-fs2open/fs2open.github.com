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

 private slots:
	void on_propNameLineEdit_editingFinished();
	void on_nextButton_clicked();
	void on_prevButton_clicked();
	void on_layerCombo_currentIndexChanged(int index);
	void on_spawnDelaySpinBox_valueChanged(int value);
	void on_despawnDelaySpinBox_valueChanged(int value);

 private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<::Ui::PropEditorDialog> ui;
	std::unique_ptr<PropEditorDialogModel> _model;
	EditorViewport* _viewport;

	SCP_vector<int> _loadedProps;
	int _loadedSpawnCue = -1;
	int _loadedDespawnCue = -1;

	void initializeUi();
	void updateUi();
	void loadCueTrees();
};

}
