#include <QtWidgets/QMessageBox>
#include "MissionGoalsDialog.h"

#include "ui/util/SignalBlockers.h"
#include "mission/commands/FredCommands.h"
#include "mission/util.h"
#include "ui_MissionGoalsDialog.h"
#include <ui/util/DialogUndo.h>
#include <QSignalBlocker>

namespace fso::fred::dialogs {

MissionGoalsDialog::MissionGoalsDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface({TreeFlags::LabeledRoot, TreeFlags::RootDeletable}),
	  ui(new Ui::MissionGoalsDialog()), _model(new MissionGoalsDialogModel(this, viewport)),
	  _viewport(viewport), _fredView(parent)
{
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Mission Goals"));

	ui->goalEventTree->initializeEditor(viewport->editor, this, viewport, parent);
	_model->setTreeControl(ui->goalEventTree);

	ui->goalName->setMaxLength(NAME_LENGTH - 1);

	ui->helpTextBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	ui->helpTextBox->setVisible(viewport->Show_sexp_help_mission_goals);

	connect(_model.get(), &MissionGoalsDialogModel::modelChanged, this, &MissionGoalsDialog::updateUi);
	connect(ui->goalEventTree, &sexp_tree_view::modified, this, &MissionGoalsDialog::onGoalTreeModified);

	_model->initializeData();

	load_tree();

	recreate_tree();

	// The before-state for the next tree edit: the tree mutates before
	// modified() fires, so a fresh capture at handler time would already
	// contain the edit. indexChanged fires on every push/undo/redo, keeping
	// the cache aligned with the current working state.
	_workingStateCache = _model->captureWorkingState();
	connect(_dialogStack, &QUndoStack::indexChanged, this, [this](int) {
		// accept() moves _model into the ApplyDialogCommand and then clears
		// the stack, which emits indexChanged with no model left.
		if (_model)
			_workingStateCache = _model->captureWorkingState();
	});
}

MissionGoalsDialog::~MissionGoalsDialog() = default;

void MissionGoalsDialog::accept()
{
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Mission Goals")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void MissionGoalsDialog::reject()
{
	if (!_model) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		return;
	}
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
	}
}

void MissionGoalsDialog::closeEvent(QCloseEvent* e)
{
	reject();
	// reject() hides the dialog when it actually closes. Let that close
	// proceed (so a dialog created with WA_DeleteOnClose is destroyed),
	// and only veto it when reject() decided to keep the dialog open (e.g.
	// the user cancelled the unsaved-changes prompt).
	if (isVisible()) {
		e->ignore();
	} else {
		e->accept();
	}
}

void MissionGoalsDialog::onGoalTreeModified()
{
	_model->setModified();
	if (_suppressTreeUndo)
		return;

	pushWorkingStateSnapshot(_workingStateCache, tr("Edit Objective Formula"));
}

void MissionGoalsDialog::pushWorkingStateSnapshot(const QByteArray& before, const QString& label)
{
	const QByteArray after = _model->captureWorkingState();
	if (after == before)
		return;

	_dialogStack->push(new DialogSnapshotCommand(before, after,
		[this](const QByteArray& blob) {
			_suppressTreeUndo = true;
			_model->restoreWorkingState(blob);
			recreate_tree();
			updateUi();
			_suppressTreeUndo = false;
		},
		label));
}

void MissionGoalsDialog::syncGoalRootLabel(int goalIndex)
{
	auto& goals = _model->getGoals();
	if (!SCP_vector_inbounds(goals, goalIndex))
		return;

	const int formula = goals[goalIndex].formula;
	for (int i = 0; i < ui->goalEventTree->topLevelItemCount(); ++i) {
		auto* item = ui->goalEventTree->topLevelItem(i);
		if (item && item->data(0, sexp_tree_view::FormulaDataRole).toInt() == formula) {
			item->setText(0, QString::fromStdString(goals[goalIndex].name));
			break;
		}
	}
}

void MissionGoalsDialog::updateUi()
{
	// Avoid infinite recursion by blocking signal calls caused by our changes here
	util::SignalBlockers blocker(this);

	if (!_model->isCurrentGoalValid()) {
		ui->goalName->setText(QString());
		ui->goalDescription->setText(QString());
		ui->goalTypeCombo->setCurrentIndex(-1);
		ui->goalTeamCombo->setCurrentIndex(0);

		ui->goalTypeCombo->setEnabled(false);
		ui->goalName->setEnabled(false);
		ui->goalDescription->setEnabled(false);
		ui->objectiveInvalidCheck->setEnabled(false);
		ui->goalScore->setEnabled(false);
		ui->noCompletionMusicCheck->setEnabled(false);
		ui->goalTeamCombo->setEnabled(false);

		return;
	}

	auto& goal = _model->getCurrentGoal();

	ui->goalName->setText(QString::fromUtf8(goal.name.c_str()));
	ui->goalDescription->setText(QString::fromUtf8(goal.message.c_str()));
	ui->goalTypeCombo->setCurrentIndex(goal.type & GOAL_TYPE_MASK);
	ui->objectiveInvalidCheck->setChecked((goal.type & INVALID_GOAL) != 0);
	ui->noCompletionMusicCheck->setChecked((goal.flags & MGF_NO_MUSIC) != 0);
	ui->goalScore->setValue(goal.score);
	ui->goalTeamCombo->setCurrentIndex(goal.team);

	ui->goalTypeCombo->setEnabled(true);
	ui->goalName->setEnabled(true);
	ui->goalDescription->setEnabled(true);
	ui->objectiveInvalidCheck->setEnabled(true);
	ui->goalScore->setEnabled(true);
	ui->noCompletionMusicCheck->setEnabled(true);
	ui->goalTeamCombo->setEnabled((The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) != 0);
}
void MissionGoalsDialog::load_tree()
{
	ui->goalEventTree->clear_tree();
	auto& goals = _model->getGoals();
	for (auto& goal : goals) {
		goal.formula = ui->goalEventTree->_model.load_sub_tree(goal.formula, true, "true");
	}
	ui->goalEventTree->_model.post_load();
}
void MissionGoalsDialog::recreate_tree()
{
	ui->goalEventTree->clear();
	const auto& goals = _model->getGoals();
	for (const auto& goal : goals) {
		if (!_model->isGoalVisible(goal)) {
			continue;
		}

		auto h = ui->goalEventTree->insert(goal.name.c_str());
		h->setData(0, sexp_tree_view::FormulaDataRole, goal.formula);
		ui->goalEventTree->add_sub_tree(goal.formula, h);
	}

	_model->setCurrentGoal(-1);
}
void MissionGoalsDialog::createNewObjective()
{
	auto& goal = _model->createNewGoal();

	auto h = ui->goalEventTree->insert(goal.name.c_str());

	ui->goalEventTree->setCurrentItemIndex(-1);
	ui->goalEventTree->add_operator("true", h);
	auto index = goal.formula = ui->goalEventTree->getCurrentItemIndex();
	h->setData(0, sexp_tree_view::FormulaDataRole, index);

	ui->goalEventTree->setCurrentItem(h);
}
void MissionGoalsDialog::changeGoalCategory(int type)
{
	if (_model->isCurrentGoalValid()) {
		const QByteArray before = _model->captureWorkingState();
		_model->setCurrentGoalCategory(type);
		recreate_tree();
		pushWorkingStateSnapshot(before, tr("Change Objective Category"));
	}
}

void MissionGoalsDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void MissionGoalsDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void MissionGoalsDialog::on_displayTypeCombo_currentIndexChanged(int index)
{
	const int before = _model->m_display_goal_types;
	if (before == index)
		return;

	_model->setGoalDisplayType(index);
	recreate_tree();

	auto* cmd = new FieldEditCommand<int>(FieldId::Goal_DisplayFilter, nullptr, tr("Change Goal Display Type"), true);
	cmd->addEntry(before, index, [this](const int& v) {
		_model->setGoalDisplayType(v);
		{
			QSignalBlocker blocker(ui->displayTypeCombo);
			ui->displayTypeCombo->setCurrentIndex(v);
		}
		recreate_tree();
	});
	_dialogStack->push(cmd);
}

void MissionGoalsDialog::on_goalTypeCombo_currentIndexChanged(int index)
{
	changeGoalCategory(index);
}

void MissionGoalsDialog::on_goalName_textChanged(const QString& text)
{
	if (!_model->isCurrentGoalValid())
		return;

	const int index = _model->cur_goal;
	const SCP_string before = _model->getCurrentGoal().name;
	const SCP_string after  = text.toUtf8().constData();
	if (before == after)
		return;

	_model->setCurrentGoalName(after.c_str());
	syncGoalRootLabel(index);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Goal_Name + index * FieldId::Goal_FieldStride,
	    nullptr, tr("Change Objective Name"), true);
	cmd->addEntry(before, after, [this, index](const SCP_string& v) {
		_model->setGoalNameAt(index, v);
		syncGoalRootLabel(index);
	});
	_dialogStack->push(cmd);
}

void MissionGoalsDialog::on_goalDescription_textChanged(const QString& text)
{
	if (!_model->isCurrentGoalValid())
		return;

	const int index = _model->cur_goal;
	const SCP_string before = _model->getCurrentGoal().message;
	_model->setCurrentGoalMessage(text.toUtf8().constData());
	// The setter localizes the text, so read the stored value back for the command.
	const SCP_string after = _model->getCurrentGoal().message;
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Goal_Message + index * FieldId::Goal_FieldStride,
	    nullptr, tr("Change Objective Text"), true);
	cmd->addEntry(before, after, [this, index](const SCP_string& v) { _model->setGoalMessageAt(index, v); });
	_dialogStack->push(cmd);
}

void MissionGoalsDialog::on_goalScore_valueChanged(int value)
{
	if (!_model->isCurrentGoalValid())
		return;

	const int index  = _model->cur_goal;
	const int before = _model->getCurrentGoal().score;
	if (before == value)
		return;

	_model->setCurrentGoalScore(value);

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Goal_Score + index * FieldId::Goal_FieldStride,
	    nullptr, tr("Change Objective Score"), true);
	cmd->addEntry(before, value, [this, index](const int& v) { _model->setGoalScoreAt(index, v); });
	_dialogStack->push(cmd);
}

void MissionGoalsDialog::on_goalTeamCombo_currentIndexChanged(int team)
{
	if (!_model->isCurrentGoalValid())
		return;

	const int index  = _model->cur_goal;
	const int before = _model->getCurrentGoal().team;
	if (before == team)
		return;

	_model->setCurrentGoalTeam(team);

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Goal_Team + index * FieldId::Goal_FieldStride,
	    nullptr, tr("Change Objective Team"), true);
	cmd->addEntry(before, team, [this, index](const int& v) { _model->setGoalTeamAt(index, v); });
	_dialogStack->push(cmd);
}

void MissionGoalsDialog::on_objectiveInvalidCheck_toggled(bool checked)
{
	if (!_model->isCurrentGoalValid())
		return;

	const int index   = _model->cur_goal;
	const bool before = (_model->getCurrentGoal().type & INVALID_GOAL) != 0;
	if (before == checked)
		return;

	_model->setCurrentGoalInvalid(checked);

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Goal_Invalid + index * FieldId::Goal_FieldStride,
	    nullptr, tr("Toggle Objective Invalid"), true);
	cmd->addEntry(before, checked, [this, index](const bool& v) { _model->setGoalInvalidAt(index, v); });
	_dialogStack->push(cmd);
}

void MissionGoalsDialog::on_noCompletionMusicCheck_toggled(bool checked)
{
	if (!_model->isCurrentGoalValid())
		return;

	const int index   = _model->cur_goal;
	const bool before = (_model->getCurrentGoal().flags & MGF_NO_MUSIC) != 0;
	if (before == checked)
		return;

	_model->setCurrentGoalNoMusic(checked);

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Goal_NoMusic + index * FieldId::Goal_FieldStride,
	    nullptr, tr("Toggle Objective No Music"), true);
	cmd->addEntry(before, checked, [this, index](const bool& v) { _model->setGoalNoMusicAt(index, v); });
	_dialogStack->push(cmd);
}

void MissionGoalsDialog::on_newObjectiveBtn_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_suppressTreeUndo = true;
	createNewObjective();
	_suppressTreeUndo = false;
	pushWorkingStateSnapshot(before, tr("Add Objective"));
}

void MissionGoalsDialog::on_goalEventTree_selectedRootChanged(int formula)
{
	auto& goals = _model->getGoals();
	for (size_t i = 0; i < goals.size(); ++i) {
		if (goals[i].formula == formula) {
			_model->setCurrentGoal(static_cast<int>(i));
			break;
		}
	}
}

void MissionGoalsDialog::on_goalEventTree_rootNodeDeleted(int node)
{
	// The widget has not freed the branch yet (it emits before free_node2),
	// so a fresh capture still serializes the deleted goal's formula. The
	// modified() the widget emits afterwards is absorbed by the snapshot
	// equality check.
	const QByteArray before = _model->captureWorkingState();
	_model->deleteGoal(node);
	pushWorkingStateSnapshot(before, tr("Delete Objective"));
}

void MissionGoalsDialog::on_goalEventTree_rootNodeFormulaChanged(int old, int node)
{
	_model->changeFormula(old, node);
}

void MissionGoalsDialog::on_goalEventTree_helpChanged(const QString& help)
{
	ui->helpTextBox->setPlainText(help);
}

} // namespace fso::fred::dialogs
