#include <QtWidgets/QMessageBox>
#include "MissionGoalsDialog.h"

#include "ui/util/SignalBlockers.h"
#include "mission/util.h"
#include "ui_MissionGoalsDialog.h"

namespace fso::fred::dialogs {

MissionGoalsDialog::MissionGoalsDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface({TreeFlags::LabeledRoot, TreeFlags::RootDeletable}),
	  ui(new Ui::MissionGoalsDialog()), _model(new MissionGoalsDialogModel(this, viewport)), _viewport(viewport)
{
	ui->setupUi(this);

	ui->goalEventTree->initializeEditor(viewport->editor, this);
	_model->setTreeControl(ui->goalEventTree);

	ui->goalName->setMaxLength(NAME_LENGTH - 1);

	ui->helpTextBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	connect(_model.get(), &MissionGoalsDialogModel::modelChanged, this, &MissionGoalsDialog::updateUi);

	_model->initializeData();

	load_tree();

	recreate_tree();
}

MissionGoalsDialog::~MissionGoalsDialog() = default;

void MissionGoalsDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void MissionGoalsDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void MissionGoalsDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
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
	ui->noCompletionMusicCheck->setChecked((goal.type & MGF_NO_MUSIC) != 0);
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
		goal.formula = ui->goalEventTree->load_sub_tree(goal.formula, true, "true");
	}
	ui->goalEventTree->post_load();
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
		h->setData(0, sexp_tree::FormulaDataRole, goal.formula);
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
	h->setData(0, sexp_tree::FormulaDataRole, index);

	ui->goalEventTree->setCurrentItem(h);
}
void MissionGoalsDialog::changeGoalCategory(int type)
{
	if (_model->isCurrentGoalValid()) {
		_model->setCurrentGoalCategory(type);
		recreate_tree();
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
	_model->setGoalDisplayType(index);
	recreate_tree();
}

void MissionGoalsDialog::on_goalTypeCombo_currentIndexChanged(int index)
{
	changeGoalCategory(index);
}

void MissionGoalsDialog::on_goalName_textChanged(const QString& text)
{
	if (_model->isCurrentGoalValid()) {
		_model->setCurrentGoalName(text.toUtf8().constData());

		auto item = ui->goalEventTree->currentItem();
		while (item->parent() != nullptr) {
			item = item->parent();
		}

		item->setText(0, text);
	}
}

void MissionGoalsDialog::on_goalDescription_textChanged(const QString& text)
{
	_model->setCurrentGoalMessage(text.toUtf8().constData());
}

void MissionGoalsDialog::on_goalScore_valueChanged(int value)
{
	if (_model->isCurrentGoalValid()) {
		_model->setCurrentGoalScore(value);
	}
}

void MissionGoalsDialog::on_goalTeamCombo_currentIndexChanged(int team)
{
	if (_model->isCurrentGoalValid()) {
		_model->setCurrentGoalTeam(team);
	}
}

void MissionGoalsDialog::on_objectiveInvalidCheck_stateChanged(bool checked)
{
	if (_model->isCurrentGoalValid()) {
		_model->setCurrentGoalInvalid(checked);
	}
}

void MissionGoalsDialog::on_noCompletionMusicCheck_stateChanged(bool checked)
{
	if (_model->isCurrentGoalValid()) {
		_model->setCurrentGoalNoMusic(checked);
	}
}

void MissionGoalsDialog::on_newObjectiveBtn_clicked()
{
	createNewObjective();
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
	_model->deleteGoal(node);
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
