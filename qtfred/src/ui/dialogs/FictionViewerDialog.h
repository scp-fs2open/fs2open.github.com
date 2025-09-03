#pragma once

#include <QtWidgets/QDialog>

#include <mission/dialogs/FictionViewerDialogModel.h>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class FictionViewerDialog;
}

class FictionViewerDialog : public QDialog {
	Q_OBJECT
public:
	FictionViewerDialog(FredView* parent, EditorViewport* viewport);
	~FictionViewerDialog() override;

	void accept() override;
	void reject() override;

 protected:
	void closeEvent(QCloseEvent* e) override; // funnel all Window X presses through reject()

private slots:
	// dialog controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_storyFileEdit_textChanged(const QString& text);
	void on_fontFileEdit_textChanged(const QString& text);
	void on_voiceFileEdit_textChanged(const QString& text);
	void on_musicComboBox_currentIndexChanged(int index);

 private: // NOLINT(readability-redundant-access-specifiers)

	void initializeUi();
	void updateUi();

	void updateMusicComboBox();

	

	EditorViewport* _viewport = nullptr;
	std::unique_ptr<Ui::FictionViewerDialog> ui;
	std::unique_ptr<FictionViewerDialogModel> _model;
};

} // namespace fso::fred::dialogs
