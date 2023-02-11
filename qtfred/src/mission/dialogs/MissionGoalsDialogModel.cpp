//
//

#include "MissionGoalsDialogModel.h"


namespace fso {
namespace fred {
namespace dialogs {

MissionGoalsDialogModel::MissionGoalsDialogModel(QObject* parent, fso::fred::EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
}
bool MissionGoalsDialogModel::apply()
{
	SCP_vector<std::pair<SCP_string, SCP_string>> names;
	int i;

	auto changes_detected = query_modified();

	for (auto &goal: Mission_goals) {
		free_sexp2(goal.formula);
		goal.satisfied = 0;  // use this as a processed flag
	}

	// rename all sexp references to old goals
	for (i=0; i<(int)m_goals.size(); i++) {
		if (m_sig[i] >= 0) {
			names.emplace_back(Mission_goals[m_sig[i]].name, m_goals[i].name);
			Mission_goals[m_sig[i]].satisfied = 1;
		}
	}

	// invalidate all sexp references to deleted goals.
	for (const auto &goal: Mission_goals) {
		if (!goal.satisfied) {
			SCP_string buf = "<" + goal.name + ">";

			// force it to not be too long
			if (SCP_truncate(buf, NAME_LENGTH))
				buf.back() = '>';

			names.emplace_back(goal.name, buf);
		}
	}

	// copy all dialog goals to the mission
	Mission_goals.clear();
	for (const auto &dialog_goal: m_goals) {
		Mission_goals.push_back(dialog_goal);
		Mission_goals.back().formula = _sexp_tree->save_tree(dialog_goal.formula);
		if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS ) {
			Assert( dialog_goal.team != -1 );
		}
	}

	// now update all sexp references
	for (const auto &name_pair: names)
		update_sexp_references(name_pair.first.c_str(), name_pair.second.c_str(), OPF_GOAL_NAME);

	// Only fire the signal after the changes have been applied to make sure the other parts of the code see the updated
	// state
	if (changes_detected) {
		_editor->missionChanged();
	}

	return true;
}
void MissionGoalsDialogModel::reject() {
	// Nothing to do here
}
mission_goal& MissionGoalsDialogModel::getCurrentGoal() {
	Assertion(cur_goal >= 0 && cur_goal < (int)m_goals.size(), "Current goal index is not valid!");

	return m_goals[cur_goal];
}
bool MissionGoalsDialogModel::isCurrentGoalValid() const {
	return cur_goal >= 0 && cur_goal < (int)m_goals.size();
}
void MissionGoalsDialogModel::initializeData() {
	m_goals.clear();
	m_sig.clear();
	for (int i=0; i<(int)Mission_goals.size(); i++) {
		m_goals.push_back(Mission_goals[i]);
		m_sig.push_back(i);

		if (m_goals[i].name.empty())
			m_goals[i].name = "<Unnamed>";
	}

	cur_goal = -1;
	modelChanged();
}
SCP_vector<mission_goal>& MissionGoalsDialogModel::getGoals() {
	return m_goals;
}
void MissionGoalsDialogModel::setCurrentGoal(int index) {
	cur_goal = index;

	modelChanged();
}
bool MissionGoalsDialogModel::isGoalVisible(const mission_goal& goal) const {
	return (goal.type & GOAL_TYPE_MASK) == m_display_goal_types;
}
void MissionGoalsDialogModel::setGoalDisplayType(int type) {
	m_display_goal_types = type;
}
bool MissionGoalsDialogModel::query_modified() {
	int i;

	if (modified)
		return true;

	if (Mission_goals.size() != m_goals.size())
		return true;

	for (i=0; i<(int)Mission_goals.size(); i++) {
		if (!SCP_string_lcase_equal_to()(Mission_goals[i].name, m_goals[i].name))
			return true;
		if (!SCP_string_lcase_equal_to()(Mission_goals[i].message, m_goals[i].message))
			return true;
		if (Mission_goals[i].type != m_goals[i].type)
			return true;
		if ( Mission_goals[i].score != m_goals[i].score )
			return true;
		if ( Mission_goals[i].team != m_goals[i].team )
			return true;
	}

	return false;
}
void MissionGoalsDialogModel::setTreeControl(sexp_tree* tree) {
	_sexp_tree = tree;
}
void MissionGoalsDialogModel::deleteGoal(int node) {
	int i;
	for (i=0; i<(int)m_goals.size(); i++)
	if (m_goals[i].formula == node)
		break;

	Assert(i < (int)m_goals.size());
	m_goals.erase(m_goals.begin() + i);
	m_sig.erase(m_sig.begin() + i);

	modelChanged();
}
void MissionGoalsDialogModel::changeFormula(int old_form, int new_form) {
	int i;

	for (i=0; i<(int)m_goals.size(); i++){
		if (m_goals[i].formula == old_form){
			break;
		}
	}

	Assert(i < (int)m_goals.size());
	m_goals[i].formula = new_form;

	modelChanged();
}
mission_goal& MissionGoalsDialogModel::createNewGoal() {
	m_goals.emplace_back();
	m_sig.push_back(-1);

	m_goals.back().type = m_display_goal_types;			// this also marks the goal as valid since bit not set
	m_goals.back().name = "Goal name";
	m_goals.back().message = "Mission goal text";

	return m_goals.back();
}
void MissionGoalsDialogModel::setCurrentGoalMessage(const char* text) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");
	getCurrentGoal().message = text;

	modelChanged();
}
void MissionGoalsDialogModel::setCurrentGoalCategory(int type) {
	// change the type being sure to keep the invalid bit if set
	auto otype = m_goals[cur_goal].type;
	m_goals[cur_goal].type = type;
	if ( otype & INVALID_GOAL ){
		m_goals[cur_goal].type |= INVALID_GOAL;
	}

	modelChanged();
}
void MissionGoalsDialogModel::setCurrentGoalScore(int value) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");
	getCurrentGoal().score = value;

	modelChanged();
}
void MissionGoalsDialogModel::setCurrentGoalName(const char* name) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");
	getCurrentGoal().name = name;

	modelChanged();
}
void MissionGoalsDialogModel::setCurrentGoalInvalid(bool invalid) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");

	if (invalid) {
		getCurrentGoal().type |= INVALID_GOAL;
	} else {
		getCurrentGoal().type &= ~INVALID_GOAL;
	}
}
void MissionGoalsDialogModel::setCurrentGoalNoMusic(bool noMusic) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");

	if (noMusic) {
		getCurrentGoal().type |= MGF_NO_MUSIC;
	} else {
		getCurrentGoal().type &= ~MGF_NO_MUSIC;
	}
}
void MissionGoalsDialogModel::setCurrentGoalTeam(int team) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");

	getCurrentGoal().team = team;
}

}
}
}
