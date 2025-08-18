#pragma once

#include <QDialog>

#include "mission/EditorViewport.h"
#include "mission/dialogs/MissionGoalsDialogModel.h"

#include "ui/widgets/sexp_tree.h"

namespace fso::fred::dialogs {

namespace Ui {
class MissionGoalsDialog;
}

class MissionGoalsDialog : public QDialog, public SexpTreeEditorInterface
{
    Q_OBJECT

public:
    explicit MissionGoalsDialog(QWidget *parent, EditorViewport* viewport);
    ~MissionGoalsDialog() override;

	void accept() override;
	void reject() override;

 protected:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_displayTypeCombo_currentIndexChanged(int index);
	void on_goalTypeCombo_currentIndexChanged(int index);
	void on_goalName_textChanged(const QString& text);
	void on_goalDescription_textChanged(const QString& text);
	void on_goalScore_valueChanged(int value);
	void on_goalTeamCombo_currentIndexChanged(int team);
	void on_objectiveInvalidCheck_stateChanged(bool checked);
	void on_noCompletionMusicCheck_stateChanged(bool checked);
	void on_newObjectiveBtn_clicked();

	void on_goalEventTree_selectedRootChanged(int formula);
	void on_goalEventTree_rootNodeDeleted(int node);
	void on_goalEventTree_rootNodeFormulaChanged(int old, int node);
	void on_goalEventTree_helpChanged(const QString& help);

 private: // NOLINT(readability-redundant-access-specifiers)
	void updateUi();
	void createNewObjective();
	void changeGoalCategory(int type);

    std::unique_ptr<Ui::MissionGoalsDialog> ui;
    std::unique_ptr<MissionGoalsDialogModel> _model;

    EditorViewport* _viewport = nullptr;
	void load_tree();
	void recreate_tree();
};

} // namespace fso::fred::dialogs
