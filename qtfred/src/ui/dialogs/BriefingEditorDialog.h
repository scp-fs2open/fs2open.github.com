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

	void on_saveViewButton_clicked();
	void on_gotoViewButton_clicked();
	void on_copyViewButton_clicked();
	void on_pasteViewButton_clicked();

	void on_copyToOtherTeamsButton_clicked();
	void on_teamComboBox_currentIndexChanged(int index);

	void on_cameraTransitionTimeSpinBox_valueChanged(int arg1);
	void on_cutToNextStageCheckBox_toggled(bool checked);
	void on_cutToPrevStageCheckBox_toggled(bool checked);
	void on_disableGridCheckBox_toggled(bool checked);

	void on_iconIdSpinBox_valueChanged(int arg1);
	void on_iconLabelLineEdit_textChanged(const QString& string);
	void on_iconCloseupLabelLineEdit_textChanged(const QString& string);
	void on_iconImageComboBox_currentIndexChanged(int index);
	void on_iconShipTypeComboBox_currentIndexChanged(int index);
	void on_iconTeamComboBox_currentIndexChanged(int index);
	void on_iconScaleDoubleSpinBox_valueChanged(double arg1);

	void on_drawLinesCheckBox_toggled(bool checked);
	void on_changeLocallyCheckBox_toggled(bool checked);
	void on_flipIconCheckBox_toggled(bool checked);
	void on_highlightCheckBox_toggled(bool checked);
	void on_useWingCheckBox_toggled(bool checked);
	void on_useCargoCheckBox_toggled(bool checked);

	void on_makeIconButton_clicked();
	void on_deleteIconButton_clicked();
	void on_propagateIconButton_clicked();

	void on_stageTextPlainTextEdit_textChanged();
	void on_voiceFileLineEdit_textChanged(const QString& string);
	void on_voiceFileBrowseButton_clicked();
	void on_voiceFilePlayButton_clicked();
	void on_formulaTreeView_nodeChanged(int newTree);

	void on_defaultMusicComboBox_currentIndexChanged(int index);
	void on_musicPackComboBox_currentIndexChanged(int /*index*/);

  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::BriefingEditorDialog> ui;
	std::unique_ptr<BriefingEditorDialogModel> _model;
	EditorViewport* _viewport;

	void initializeUi();
	void updateUi();
	void enableDisableControls();
};
} // namespace fso::fred::dialogs
