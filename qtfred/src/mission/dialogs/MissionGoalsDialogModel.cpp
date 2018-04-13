//
//

#include "MissionGoalsDialogModel.h"


namespace fso {
namespace fred {
namespace dialogs {

MissionGoalsDialogModel::MissionGoalsDialogModel(QObject* parent, fso::fred::EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
}
bool MissionGoalsDialogModel::apply() {
	char buf[256], names[2][MAX_GOALS][NAME_LENGTH];
	int i, count;

	for (i=0; i<Num_goals; i++)
		free_sexp2(Mission_goals[i].formula);

	auto changes_detected = query_modified();

	count = 0;
	for (i=0; i<Num_goals; i++)
		Mission_goals[i].satisfied = 0;  // use this as a processed flag

	// rename all sexp references to old events
	for (i=0; i<m_num_goals; i++)
		if (m_sig[i] >= 0) {
			strcpy_s(names[0][count], Mission_goals[m_sig[i]].name);
			strcpy_s(names[1][count], m_goals[i].name);
			count++;
			Mission_goals[m_sig[i]].satisfied = 1;
		}

	// invalidate all sexp references to deleted events.
	for (i=0; i<Num_goals; i++)
		if (!Mission_goals[i].satisfied) {
			sprintf(buf, "<%s>", Mission_goals[i].name);
			strcpy(buf + NAME_LENGTH - 2, ">");  // force it to be not too long
			strcpy_s(names[0][count], Mission_goals[i].name);
			strcpy_s(names[1][count], buf);
			count++;
		}

	Num_goals = m_num_goals;
	for (i=0; i<Num_goals; i++) {
		Mission_goals[i] = m_goals[i];
		Mission_goals[i].formula = _sexp_tree->save_tree(Mission_goals[i].formula);
		if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS ) {
			Assert( Mission_goals[i].team != -1 );
		}
	}

	// now update all sexp references
	while (count--)
		update_sexp_references(names[0][count], names[1][count], OPF_GOAL_NAME);

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
	Assertion(cur_goal >= 0 && cur_goal < m_num_goals, "Current goal index is not valid!");

	return m_goals[cur_goal];
}
bool MissionGoalsDialogModel::isCurrentGoalValid() const {
	return cur_goal >= 0 && cur_goal < m_num_goals;
}
void MissionGoalsDialogModel::initializeData() {
	m_num_goals = Num_goals;
	for (auto i = 0; i < Num_goals; i++) {
		m_goals[i] = Mission_goals[i];
		m_sig[i] = i;
		if (strlen(m_goals[i].name) <= 0) {
			strcpy_s(m_goals[i].name, "<unnamed>");
		}
	}
	cur_goal = -1;

	modelChanged();
}
std::array<mission_goal, MAX_GOALS>& MissionGoalsDialogModel::getGoals() {
	return m_goals;
}
void MissionGoalsDialogModel::setCurrentGoal(int index) {
	cur_goal = index;

	modelChanged();
}
bool MissionGoalsDialogModel::isGoalVisible(const mission_goal& goal) const {
	return (goal.type & GOAL_TYPE_MASK) == m_display_goal_types;
}
int MissionGoalsDialogModel::getNumGoals() const {
	return m_num_goals;
}
void MissionGoalsDialogModel::setGoalDisplayType(int type) {
	m_display_goal_types = type;
}
bool MissionGoalsDialogModel::query_modified() {
	int i;

	if (modified)
		return true;

	if (Num_goals != m_num_goals)
		return true;

	for (i=0; i<Num_goals; i++) {
		if (stricmp(Mission_goals[i].name, m_goals[i].name))
			return true;
		if (stricmp(Mission_goals[i].message, m_goals[i].message))
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
void MissionGoalsDialogModel::deleteGoal(int formula) {
	int goal;
	for (goal=0; goal<m_num_goals; goal++){
		if (m_goals[goal].formula == formula){
			break;
		}
	}

	Assert(goal < m_num_goals);
	while (goal < m_num_goals - 1) {
		m_goals[goal] = m_goals[goal + 1];
		m_sig[goal] = m_sig[goal + 1];
		goal++;
	}
	m_num_goals--;

	modelChanged();
}
void MissionGoalsDialogModel::changeFormula(int old_form, int new_form) {
	int i;

	for (i=0; i<m_num_goals; i++){
		if (m_goals[i].formula == old_form){
			break;
		}
	}

	Assert(i < m_num_goals);
	m_goals[i].formula = new_form;

	modelChanged();
}
mission_goal& MissionGoalsDialogModel::createNewGoal() {
	Assert(m_num_goals < MAX_GOALS);
	m_goals[m_num_goals].type = m_display_goal_types;			// this also marks the goal as valid since bit not set
	m_sig[m_num_goals] = -1;
	strcpy_s(m_goals[m_num_goals].name, "Goal name");
	strcpy_s(m_goals[m_num_goals].message, "Mission goal text");
	m_goals[m_num_goals].score = 0;
		// team defaults to the first team.
	m_goals[m_num_goals].team = 0;

	auto new_goal = m_num_goals++;

	return m_goals[new_goal];
}
void MissionGoalsDialogModel::setCurrentGoalMessage(const char* text) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");
	strcpy_s(getCurrentGoal().message, text);

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
	strcpy_s(getCurrentGoal().name, name);

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
