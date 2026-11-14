#pragma once

#include "AbstractDialogModel.h"

#include <mission/missiongoals.h>

#include "ui/widgets/sexp_tree_view.h"

namespace fso::fred::dialogs {

class MissionGoalsDialogModel: public AbstractDialogModel {
 public:
	MissionGoalsDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	QByteArray captureState() const override;
	void restoreState(const QByteArray& state) override;

	// Serialize/restore the dialog's WORKING state (m_goals + m_sig, with each
	// goal's formula branch serialized from the tree widget) for the in-dialog
	// undo stack. Selection and the display-type filter are view state and are
	// not captured. The dialog rebuilds the visual tree (recreate_tree) after
	// a restore.
	QByteArray captureWorkingState() const;
	void restoreWorkingState(const QByteArray& state);

	mission_goal& getCurrentGoal();

	bool isCurrentGoalValid() const;

	void setCurrentGoal(int index);

	void initializeData();

	SCP_vector<mission_goal>& getGoals();

	bool isGoalVisible(const mission_goal& goal) const;

	void setGoalDisplayType(int type);

	void deleteGoal(int formula);

	void changeFormula(int old_form, int new_form);

	mission_goal& createNewGoal();

	void setModified() { set_modified(); }

	void setCurrentGoalMessage(const char* text);
	void setCurrentGoalScore(int value);
	void setCurrentGoalCategory(int type);
	void setCurrentGoalName(const char* name);
	void setCurrentGoalInvalid(bool invalid);
	void setCurrentGoalNoMusic(bool noMusic);
	void setCurrentGoalTeam(int team);

	// Indexed setters for undo commands: a command captured on one goal must
	// still restore that goal after the user selects another.
	void setGoalNameAt(int index, const SCP_string& name);
	void setGoalMessageAt(int index, const SCP_string& message);
	void setGoalScoreAt(int index, int score);
	void setGoalTeamAt(int index, int team);
	void setGoalInvalidAt(int index, bool invalid);
	void setGoalNoMusicAt(int index, bool noMusic);

	// TODO HACK: This does not belong here since it is a UI specific control. Once the model based SEXP tree is implemented
	// this should be replaced
	void setTreeControl(sexp_tree_view* tree);
 public:
	int cur_goal = -1;
	SCP_vector<int> m_sig;
	SCP_vector<mission_goal> m_goals;

	int m_display_goal_types = 0;

	sexp_tree_view* _sexp_tree = nullptr;
};

} // namespace fso::fred::dialogs
