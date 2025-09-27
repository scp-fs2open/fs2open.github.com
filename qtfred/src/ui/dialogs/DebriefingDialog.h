#pragma once

#include <mission/dialogs/DebriefingDialogModel.h>
#include <ui/FredView.h>

#include <QAbstractButton>
#include <QtWidgets/QDialog>

#include "ui/widgets/sexp_tree.h"


namespace fso::fred::dialogs {

namespace Ui {
class DebriefingDialog;
}

class DebriefingDialog : public QDialog, public SexpTreeEditorInterface {
	Q_OBJECT

public:
	explicit DebriefingDialog(FredView* parent, EditorViewport* viewport);
	~DebriefingDialog() override;

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
	void on_actionChangeTeams_currentIndexChanged(int index);

	void on_debriefingTextEdit_textChanged();
	void on_recommendationTextEdit_textChanged();
	void on_voiceFileLineEdit_textChanged(const QString& string);
	void on_voiceFileBrowseButton_clicked();
	void on_voiceFilePlayButton_clicked();
	void on_formulaTreeView_nodeChanged(int newTree);

	void on_successMusicComboBox_currentIndexChanged(int index);
	void on_averageMusicComboBox_currentIndexChanged(int index);
	void on_failureMusicComboBox_currentIndexChanged(int index);

private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::DebriefingDialog> ui;
	std::unique_ptr<DebriefingDialogModel> _model;
	EditorViewport* _viewport;

	void initializeUi();
	void updateUi();
	void enableDisableControls();

};
} // namespace fso::fred::dialogs
