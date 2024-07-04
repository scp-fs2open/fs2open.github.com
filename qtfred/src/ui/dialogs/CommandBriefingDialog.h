#ifndef COMMANDBRIEFEDITORDIALOG_H
#define COMMANDBRIEFEDITORDIALOG_H

#include <QtWidgets/QDialog>
#include <mission/dialogs/CommandBriefingDialogModel.h>
#include <ui/FredView.h>

#include <QAbstractButton>
#include <QtWidgets/QDialog>


namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class CommandBriefingDialog;
}

class CommandBriefingDialog : public QDialog {

	Q_OBJECT

public:

	explicit CommandBriefingDialog(FredView* parent, EditorViewport* viewport);
	~CommandBriefingDialog() override; // NOLINT

protected:
	void closeEvent(QCloseEvent*) override;

private slots: // where the buttons go
	void on_actionPrevStage_clicked();
	void on_actionNextStage_clicked();
	void on_actionAddStage_clicked();
	void on_actionInsertStage_clicked();
	void on_actionDeleteStage_clicked();
	void on_actionChangeTeams_clicked();
	void on_actionCopyToOtherTeams_clicked();
	void on_actionBrowseAnimation_clicked();
	void on_actionBrowseSpeechFile_clicked();
	void on_actionTestSpeechFileButton_clicked();
	void on_actionLowResolutionBrowse_clicked();
	void on_actionHighResolutionBrowse_clicked();

private:
	std::unique_ptr<Ui::CommandBriefingDialog> ui;
	std::unique_ptr<CommandBriefingDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();
	void disableTeams();
	void enableDisableControls();

	// when fields get updated
	void briefingTextChanged();
	void animationFilenameChanged(const QString&);
	void speechFilenameChanged(const QString&);
	void lowResolutionFilenameChanged(const QString&);
	void highResolutionFilenameChanged(const QString&);

	static bool browseFile(QString* stringIn);

};
} // namespace dialogs
} // namespace fred
} // namespace fso

#endif
