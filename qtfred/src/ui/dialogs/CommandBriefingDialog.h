#pragma once

#include <QtWidgets/QDialog>
#include <mission/dialogs/CommandBriefingDialogModel.h>
#include <ui/FredView.h>

#include <QAbstractButton>
#include <QtWidgets/QDialog>


namespace fso::fred::dialogs {

namespace Ui {
class CommandBriefingDialog;
}

class CommandBriefingDialog : public QDialog {
	Q_OBJECT

public:
	explicit CommandBriefingDialog(FredView* parent, EditorViewport* viewport);
	~CommandBriefingDialog() override;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent* e) override; // funnel all Window X presses through reject()

private slots:
	// dialog controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_actionPrevStage_clicked();
	void on_actionNextStage_clicked();
	void on_actionAddStage_clicked();
	void on_actionInsertStage_clicked();
	void on_actionDeleteStage_clicked();
	void on_actionCopyToOtherTeams_clicked();
	void on_actionBrowseAnimation_clicked();
	void on_actionBrowseSpeechFile_clicked();
	void on_actionTestSpeechFileButton_clicked();
	void on_actionLowResolutionBrowse_clicked();
	void on_actionHighResolutionBrowse_clicked();

	void on_actionChangeTeams_currentIndexChanged(int index);
	void on_actionBriefingTextEditor_textChanged();
	void on_animationFilename_textChanged(const QString& string);
	void on_speechFilename_textChanged(const QString& string);
	void on_actionLowResolutionFilenameEdit_textChanged(const QString& string);
	void on_actionHighResolutionFilenameEdit_textChanged(const QString& string);

private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::CommandBriefingDialog> ui;
	std::unique_ptr<CommandBriefingDialogModel> _model;
	EditorViewport* _viewport;

	void initializeUi();
	void updateUi();
	void enableDisableControls();

	static bool browseFile(QString* stringIn);

};
} // namespace fso::fred::dialogs
