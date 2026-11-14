#pragma once

#include <QDialog>

#include "mission/EditorViewport.h"
#include "mission/dialogs/MissionGoalsDialogModel.h"
#include "ui/FredView.h"

#include "ui/widgets/sexp_tree_view.h"

namespace fso::fred::dialogs {

namespace Ui {
class MissionGoalsDialog;
}

class MissionGoalsDialog : public QDialog, public SexpTreeEditorInterface
{
    Q_OBJECT

public:
    explicit MissionGoalsDialog(FredView* parent, EditorViewport* viewport);
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
	void on_objectiveInvalidCheck_toggled(bool checked);
	void on_noCompletionMusicCheck_toggled(bool checked);
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

    EditorViewport* _viewport  = nullptr;
    FredView*       _fredView  = nullptr;
    QUndoStack*     _dialogStack = nullptr;
	void load_tree();
	void recreate_tree();

	// In-dialog undo helpers. Tree edits and goal-structure ops go through
	// working-state snapshots; field edits push merging FieldEditCommands.
	// _workingStateCache is the before-state for the next tree edit (the tree
	// mutates before modified() fires); it is refreshed on every stack index
	// change so undo/redo keeps it coherent. _suppressTreeUndo guards
	// programmatic tree changes (add objective, snapshot restore) so they
	// don't push their own mislabeled tree-edit commands.
	void onGoalTreeModified();
	void pushWorkingStateSnapshot(const QByteArray& before, const QString& label);
	void syncGoalRootLabel(int goalIndex);

	QByteArray _workingStateCache;
	bool       _suppressTreeUndo = false;
};

} // namespace fso::fred::dialogs
