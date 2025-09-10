#pragma once

#include "ui/widgets/sexp_tree.h"

#include <mission/dialogs/BriefingEditorDialogModel.h>
#include <ui/FredView.h>

#include <QAbstractButton>
#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class BriefingEditorDialog;
}

class BriefingEditorDialog : public QDialog, public SexpTreeEditorInterface {
	Q_OBJECT

  public:
	explicit BriefingEditorDialog(FredView* parent, EditorViewport* viewport);
	~BriefingEditorDialog() override;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent* e) override; // funnel all Window X presses through reject()

  private slots:
	// dialog controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_prevStageButton_clicked();
	void on_nextStageButton_clicked();
	void on_addStageButton_clicked();
	void on_insertStageButton_clicked();
	void on_deleteStageButton_clicked();

	void on_copyToOtherTeamsButton_clicked();
	void on_teamComboBox_currentIndexChanged(int index);

	void on_stageTextPlainTextEdit_textChanged();
	void on_voiceFileLineEdit_textChanged(const QString& string);
	void on_voiceFileBrowseButton_clicked();
	void on_voiceFilePlayButton_clicked();
	void on_formulaTreeView_nodeChanged(int newTree);

  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::BriefingEditorDialog> ui;
	std::unique_ptr<BriefingEditorDialogModel> _model;
	EditorViewport* _viewport;

	void initializeUi();
	void updateUi();
	void enableDisableControls();
};
} // namespace fso::fred::dialogs
