#include <QtWidgets/QMessageBox>
#include "MissionGoalsDialog.h"

#include "ui/util/SignalBlockers.h"

#include "ui_MissionGoalsDialog.h"

namespace fso {
namespace fred {
namespace dialogs {

MissionGoalsDialog::MissionGoalsDialog(QWidget* parent, EditorViewport* viewport) :
	QDialog(parent),
	SexpTreeEditorInterface({ TreeFlags::LabeledRoot, TreeFlags::RootDeletable }),
	ui(new Ui::MissionGoalsDialog()),
	_model(new MissionGoalsDialogModel(this, viewport)),
	_viewport(viewport)
{
    ui->setupUi(this);

    ui->goalEventTree->initializeEditor(viewport->editor, this);
    _model->setTreeControl(ui->goalEventTree);

	ui->goalName->setMaxLength(NAME_LENGTH - 1);

	ui->helpTextBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	connect(this, &QDialog::accepted, _model.get(), &MissionGoalsDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &MissionGoalsDialogModel::reject);

	connect(_model.get(), &MissionGoalsDialogModel::modelChanged, this, &MissionGoalsDialog::updateUI);

	connect(ui->displayTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		_model->setGoalDisplayType(index);
		recreate_tree();
	});

	connect(ui->goalEventTree, &sexp_tree::selectedRootChanged, this, [this](int forumla) {
		auto& goals = _model->getGoals();
		for (auto i = 0; i < (int)goals.size(); ++i) {
			if (goals[i].formula == forumla) {
				_model->setCurrentGoal(i);
				break;
			}
		}
	});
	connect(ui->goalEventTree, &sexp_tree::rootNodeDeleted, this, [this](int node) {
		_model->deleteGoal(node);
	});
	connect(ui->goalEventTree, &sexp_tree::rootNodeFormulaChanged, this, [this](int old, int node) {
		_model->changeFormula(old, node);
	});
	connect(ui->goalEventTree, &sexp_tree::helpChanged, this, [this](const QString& help) {
		ui->helpTextBox->setPlainText(help);
	});

	connect(ui->newObjectiveBtn, &QPushButton::clicked, this, [this](bool) {
		createNewObjective();
	});

	connect(ui->goalDescription, &QLineEdit::textChanged, this, [this](const QString& text) {
		if (_model->isCurrentGoalValid()) {
			_model->setCurrentGoalMessage(text.toUtf8().constData());
		}
	});

	connect(ui->goalScore, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
		if (_model->isCurrentGoalValid()) {
			_model->setCurrentGoalScore(value);
		}
	});

	connect(ui->goalTypeCombo,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			&MissionGoalsDialog::changeGoalCategory);

	connect(ui->goalName, &QLineEdit::textChanged, this, [this](const QString& text) {
		if (_model->isCurrentGoalValid()) {
			_model->setCurrentGoalName(text.toUtf8().constData());

			auto item = ui->goalEventTree->currentItem();
			while (item->parent() != nullptr) {
				item = item->parent();
			}

			item->setText(0, text);
		}
	});

	connect(ui->objectiveInvalidCheck, &QCheckBox::stateChanged, this, [this](int state) {
		bool checked = state == Qt::Checked;

		if (_model->isCurrentGoalValid()) {
			_model->setCurrentGoalInvalid(checked);
		}
	});
	connect(ui->noCompletionMusicCheck, &QCheckBox::stateChanged, this, [this](int state) {
		bool checked = state == Qt::Checked;

		if (_model->isCurrentGoalValid()) {
			_model->setCurrentGoalNoMusic(checked);
		}
	});

	connect(ui->goalTeamCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int team) {
		if (_model->isCurrentGoalValid()) {
			_model->setCurrentGoalTeam(team);
		}
	});

	_model->initializeData();

	load_tree();

	recreate_tree();
}
MissionGoalsDialog::~MissionGoalsDialog()
{
}
void MissionGoalsDialog::updateUI() {
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
void MissionGoalsDialog::load_tree() {
	ui->goalEventTree->clear_tree();
	auto& goals = _model->getGoals();
	for (auto &goal: goals) {
		goal.formula = ui->goalEventTree->load_sub_tree(goal.formula, true, "true");
	}
	ui->goalEventTree->post_load();
}
void MissionGoalsDialog::recreate_tree() {
	ui->goalEventTree->clear();
	const auto& goals = _model->getGoals();
	for (const auto& goal: goals) {
		if (!_model->isGoalVisible(goal)) {
			continue;
		}

		auto h = ui->goalEventTree->insert(goal.name.c_str());
		h->setData(0, sexp_tree::FormulaDataRole, goal.formula);
		ui->goalEventTree->add_sub_tree(goal.formula, h);
	}

	_model->setCurrentGoal(-1);
}
void MissionGoalsDialog::createNewObjective() {
	auto& goal = _model->createNewGoal();

	auto h = ui->goalEventTree->insert(goal.name.c_str());

	ui->goalEventTree->setCurrentItemIndex(-1);
	ui->goalEventTree->add_operator("true", h);
	auto index = goal.formula = ui->goalEventTree->getCurrentItemIndex();
	h->setData(0, sexp_tree::FormulaDataRole, index);

	ui->goalEventTree->setCurrentItem(h);
}
void MissionGoalsDialog::changeGoalCategory(int type) {
	if (_model->isCurrentGoalValid()) {
		_model->setCurrentGoalCategory(type);
		recreate_tree();
	}
}
void MissionGoalsDialog::closeEvent(QCloseEvent* event) {
	if (_model->query_modified()) {
		auto result = QMessageBox::question(this,
											"Close",
											"Do you want to keep your changes?",
											QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
											QMessageBox::Cancel);

		if (result == QMessageBox::Cancel) {
			event->ignore();
			return;
		}

		if (result == QMessageBox::Yes) {
			accept();
			event->accept();
			return;
		}
	}

	QDialog::closeEvent(event);
}

}
}
}
