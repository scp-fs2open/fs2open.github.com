#pragma once

#include <QtWidgets/QDialog>

#include <mission/dialogs/FictionViewerDialogModel.h>
#include <ui/FredView.h>

#include <memory>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class FictionViewerDialog;
}

class FictionViewerDialog : public QDialog {
	Q_OBJECT
public:
	FictionViewerDialog(FredView* parent, EditorViewport* viewport);
	~FictionViewerDialog();

	void musicSelectionChanged(int index);
	void storyFileTextChanged();
	void fontFileTextChanged();
	void voiceFileTextChanged();

 protected:
	void keyPressEvent(QKeyEvent* event) override;
	void closeEvent(QCloseEvent*) override;
 private:

	void updateMusicComboBox();

	void updateUI();

	

	EditorViewport* _viewport = nullptr;
	Editor* _editor = nullptr;
	
	std::unique_ptr<Ui::FictionViewerDialog> ui;
	std::unique_ptr<FictionViewerDialogModel> _model;
};

}
}
}
