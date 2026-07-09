#include <localization/localize.h>
#include "MissionGoalsDialogModel.h"
#include "state/DialogStateHelpers.h"

#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

MissionGoalsDialogModel::MissionGoalsDialogModel(QObject* parent, fso::fred::EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
}
bool MissionGoalsDialogModel::apply()
{
	SCP_vector<std::pair<SCP_string, SCP_string>> names;

	auto changes_detected = query_modified();

	for (auto &goal: Mission_goals) {
		free_sexp2(goal.formula);
		goal.satisfied = 0;  // use this as a processed flag
	}

	// rename all sexp references to old goals
	for (size_t i=0; i<m_goals.size(); i++) {
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
			if (SCP_truncate(buf, NAME_LENGTH - 1))
				buf.back() = '>';

			names.emplace_back(goal.name, buf);
		}
	}

	// copy all dialog goals to the mission
	Mission_goals.clear();
	for (const auto &dialog_goal: m_goals) {
		Mission_goals.push_back(dialog_goal);
		Mission_goals.back().formula = _sexp_tree->_model.save_tree(dialog_goal.formula);
		if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS ) {
			Assertion(dialog_goal.team != -1, "Invalid goal team!");
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
QByteArray MissionGoalsDialogModel::captureWorkingState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(m_goals.size());
	for (size_t i = 0; i < m_goals.size(); ++i) {
		const mission_goal& g = m_goals[i];
		ds << static_cast<qint32>(m_sig[i]);
		ds << QString::fromStdString(g.name);
		ds << static_cast<qint32>(g.type);
		ds << static_cast<qint32>(g.satisfied);
		ds << QString::fromStdString(g.message);
		// g.formula is a tree-widget node id; round-trip the branch through
		// Sexp_nodes to serialize its content.
		const int sexp = _sexp_tree->_model.save_tree(g.formula);
		fso::fred::state::writeSexp(ds, sexp);
		fso::fred::state::freeSexpFormula(sexp);
		ds << static_cast<qint32>(g.score);
		ds << static_cast<qint32>(g.flags);
		ds << static_cast<qint32>(g.team);
	}

	return data;
}

void MissionGoalsDialogModel::restoreWorkingState(const QByteArray& state)
{
	QDataStream ds(state);

	m_goals.clear();
	m_sig.clear();
	_sexp_tree->clear_tree(); // resets tree_nodes; the dialog rebuilds the visuals

	qint32 count;
	ds >> count;
	for (int i = 0; i < count; ++i) {
		mission_goal g;
		qint32 sig, type, satisfied, score, flags, team;
		QString name, message;

		ds >> sig >> name >> type >> satisfied >> message;
		const int sexp = fso::fred::state::readSexp(ds);
		g.formula = _sexp_tree->_model.load_sub_tree(sexp, true, "true");
		fso::fred::state::freeSexpFormula(sexp);
		ds >> score >> flags >> team;

		g.name      = name.toStdString();
		g.type      = static_cast<int>(type);
		g.satisfied = static_cast<int>(satisfied);
		g.message   = message.toStdString();
		g.score     = static_cast<int>(score);
		g.flags     = static_cast<int>(flags);
		g.team      = static_cast<int>(team);

		m_goals.push_back(g);
		m_sig.push_back(static_cast<int>(sig));
	}
	_sexp_tree->_model.post_load();

	cur_goal = -1;
	set_modified();
}

mission_goal& MissionGoalsDialogModel::getCurrentGoal() {
	Assertion(SCP_vector_inbounds(m_goals, cur_goal), "Current goal index is not valid!");

	return m_goals[cur_goal];
}
bool MissionGoalsDialogModel::isCurrentGoalValid() const {
	return SCP_vector_inbounds(m_goals, cur_goal);
}
void MissionGoalsDialogModel::initializeData() {
	m_goals.clear();
	m_sig.clear();
	for (size_t i=0; i<Mission_goals.size(); i++) {
		m_goals.push_back(Mission_goals[i]);
		m_sig.push_back(static_cast<int>(i));

		if (m_goals[i].name.empty())
			m_goals[i].name = "<Unnamed>";
	}

	cur_goal = -1;
	modelChanged();
	_modified = false;
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
	modify(m_display_goal_types, type);
}
void MissionGoalsDialogModel::setTreeControl(sexp_tree_view* tree) {
	_sexp_tree = tree;
}
void MissionGoalsDialogModel::deleteGoal(int node) {
	size_t i;
	for (i=0; i<m_goals.size(); i++)
	if (m_goals[i].formula == node)
		break;

	Assert(i < m_goals.size());
	m_goals.erase(m_goals.begin() + i);
	m_sig.erase(m_sig.begin() + i);

	set_modified();
	modelChanged();
}
void MissionGoalsDialogModel::changeFormula(int old_form, int new_form) {
	size_t i;

	for (i=0; i<m_goals.size(); i++){
		if (m_goals[i].formula == old_form){
			break;
		}
	}

	Assert(i < m_goals.size());
	m_goals[i].formula = new_form;

	set_modified();
	modelChanged();
}
mission_goal& MissionGoalsDialogModel::createNewGoal() {
	m_goals.emplace_back();
	m_sig.push_back(-1);

	m_goals.back().type = m_display_goal_types;			// this also marks the goal as valid since bit not set
	m_goals.back().name = "Goal name";
	m_goals.back().message = "Mission goal text";

	set_modified();
	return m_goals.back();
}
void MissionGoalsDialogModel::setCurrentGoalMessage(const char* text) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");
	getCurrentGoal().message = text;
	lcl_fred_replace_stuff(getCurrentGoal().message);

	set_modified();
	modelChanged();
}
void MissionGoalsDialogModel::setCurrentGoalCategory(int type) {
	// change the type being sure to keep the invalid bit if set
	auto otype = m_goals[cur_goal].type;
	m_goals[cur_goal].type = type;
	if ( otype & INVALID_GOAL ){
		m_goals[cur_goal].type |= INVALID_GOAL;
	}

	set_modified();
	modelChanged();
}
void MissionGoalsDialogModel::setCurrentGoalScore(int value) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");
	getCurrentGoal().score = value;

	set_modified();
	modelChanged();
}
void MissionGoalsDialogModel::setCurrentGoalName(const char* name) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");
	getCurrentGoal().name = name;

	set_modified();
	modelChanged();
}
void MissionGoalsDialogModel::setCurrentGoalInvalid(bool invalid) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");

	if (invalid) {
		getCurrentGoal().type |= INVALID_GOAL;
	} else {
		getCurrentGoal().type &= ~INVALID_GOAL;
	}

	set_modified();
}
void MissionGoalsDialogModel::setCurrentGoalNoMusic(bool noMusic) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");

	if (noMusic) {
		getCurrentGoal().flags |= MGF_NO_MUSIC;
	} else {
		getCurrentGoal().flags &= ~MGF_NO_MUSIC;
	}

	set_modified();
}
void MissionGoalsDialogModel::setCurrentGoalTeam(int team) {
	Assertion(isCurrentGoalValid(), "Current goal is not valid!");

	getCurrentGoal().team = team;

	set_modified();
}

void MissionGoalsDialogModel::setGoalNameAt(int index, const SCP_string& name) {
	if (!SCP_vector_inbounds(m_goals, index))
		return;
	m_goals[index].name = name;
	set_modified();
	modelChanged();
}

void MissionGoalsDialogModel::setGoalMessageAt(int index, const SCP_string& message) {
	if (!SCP_vector_inbounds(m_goals, index))
		return;
	m_goals[index].message = message;
	set_modified();
	modelChanged();
}

void MissionGoalsDialogModel::setGoalScoreAt(int index, int score) {
	if (!SCP_vector_inbounds(m_goals, index))
		return;
	m_goals[index].score = score;
	set_modified();
	modelChanged();
}

void MissionGoalsDialogModel::setGoalTeamAt(int index, int team) {
	if (!SCP_vector_inbounds(m_goals, index))
		return;
	m_goals[index].team = team;
	set_modified();
	modelChanged();
}

void MissionGoalsDialogModel::setGoalInvalidAt(int index, bool invalid) {
	if (!SCP_vector_inbounds(m_goals, index))
		return;
	if (invalid) {
		m_goals[index].type |= INVALID_GOAL;
	} else {
		m_goals[index].type &= ~INVALID_GOAL;
	}
	set_modified();
	modelChanged();
}

void MissionGoalsDialogModel::setGoalNoMusicAt(int index, bool noMusic) {
	if (!SCP_vector_inbounds(m_goals, index))
		return;
	if (noMusic) {
		m_goals[index].flags |= MGF_NO_MUSIC;
	} else {
		m_goals[index].flags &= ~MGF_NO_MUSIC;
	}
	set_modified();
	modelChanged();
}

} // namespace fso::fred::dialogs
