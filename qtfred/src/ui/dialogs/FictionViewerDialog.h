#pragma once

#include <QDialog>

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

	void reject() override;

	void musicSelectionChanged(int index);
	void storyFileTextChanged();
	void fontFileTextChanged();
	void voiceFileTextChanged();

 protected:
	bool event(QEvent* event) override;

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
