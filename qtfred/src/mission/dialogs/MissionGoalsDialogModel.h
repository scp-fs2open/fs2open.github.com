#pragma once

#include "AbstractDialogModel.h"

#include <mission/missiongoals.h>

#include "ui/widgets/sexp_tree.h"

namespace fso {
namespace fred {
namespace dialogs {

class MissionGoalsDialogModel: public AbstractDialogModel {
 public:
	MissionGoalsDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

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

	bool query_modified();

	void setCurrentGoalMessage(const char* text);
	void setCurrentGoalScore(int value);
	void setCurrentGoalCategory(int type);
	void setCurrentGoalName(const char* name);
	void setCurrentGoalInvalid(bool invalid);
	void setCurrentGoalNoMusic(bool noMusic);
	void setCurrentGoalTeam(int team);

	// HACK: This does not belong here since it is a UI specific control. Once the model based SEXP tree is implemented
	// this should be replaced
	void setTreeControl(sexp_tree* tree);
 public:
	int cur_goal = -1;
	SCP_vector<int> m_sig;
	SCP_vector<mission_goal> m_goals;
	bool modified = false;

	int m_display_goal_types;

	sexp_tree* _sexp_tree = nullptr;
};

}
}
}
