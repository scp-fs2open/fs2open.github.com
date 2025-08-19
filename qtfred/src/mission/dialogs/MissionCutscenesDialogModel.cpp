#include "MissionCutscenesDialogModel.h"


namespace fso::fred::dialogs {

MissionCutscenesDialogModel::MissionCutscenesDialogModel(QObject* parent, fso::fred::EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
}
bool MissionCutscenesDialogModel::apply()
{
	SCP_vector<std::pair<SCP_string, SCP_string>> names;

	auto changes_detected = query_modified();

	 for (auto& cs : The_mission.cutscenes) {
		free_sexp2(cs.formula);
	}

	The_mission.cutscenes.clear();
	The_mission.cutscenes.reserve(m_cutscenes.size());
	for (const auto& item : m_cutscenes) {
		The_mission.cutscenes.push_back(item);
		The_mission.cutscenes.back().formula = _sexp_tree->save_tree(item.formula);
	}

	// Only fire the signal after the changes have been applied to make sure the other parts of the code see the updated state
	if (changes_detected) {
		_editor->missionChanged();
	}

	return true;
}
void MissionCutscenesDialogModel::reject()
{
	// Nothing to do here
}
mission_cutscene& MissionCutscenesDialogModel::getCurrentCutscene()
{
	Assertion(SCP_vector_inbounds(m_cutscenes, cur_cutscene), "Current cutscene index is not valid!");
	return m_cutscenes[cur_cutscene];
}
bool MissionCutscenesDialogModel::isCurrentCutsceneValid() const
{
	return SCP_vector_inbounds(m_cutscenes, cur_cutscene);
}
void MissionCutscenesDialogModel::initializeData()
{
	m_cutscenes.clear();
	m_sig.clear();
	for (int i = 0; i < static_cast<int>(The_mission.cutscenes.size()); i++) {
		m_cutscenes.push_back(The_mission.cutscenes[i]);
		m_sig.push_back(i);

		if (m_cutscenes[i].filename[0] == '\0')
			strcpy_s(m_cutscenes[i].filename, "<Unnamed>");
	}

	cur_cutscene = -1;
	modelChanged();
}
SCP_vector<mission_cutscene>& MissionCutscenesDialogModel::getCutscenes()
{
	return m_cutscenes;
}
void MissionCutscenesDialogModel::setCurrentCutscene(int index)
{
	cur_cutscene = index;

	modelChanged();
}

int MissionCutscenesDialogModel::getSelectedCutsceneType() const
{
	return m_display_cutscene_types;
}

bool MissionCutscenesDialogModel::isCutsceneVisible(const mission_cutscene& cutscene) const
{
	return (cutscene.type == m_display_cutscene_types);
}
void MissionCutscenesDialogModel::setCutsceneType(int type)
{
	modify(m_display_cutscene_types, type);
}
int MissionCutscenesDialogModel::getCutsceneType() const
{
	return m_display_cutscene_types;
}
bool MissionCutscenesDialogModel::query_modified()
{
	if (modified)
		return true;

	if (The_mission.cutscenes.size() != m_cutscenes.size())
		return true;

	for (size_t i = 0; i < The_mission.cutscenes.size(); i++) {
		if (!lcase_equal(The_mission.cutscenes[i].filename, m_cutscenes[i].filename))
			return true;
		if (The_mission.cutscenes[i].type != m_cutscenes[i].type)
			return true;
	}

	return false;
}
void MissionCutscenesDialogModel::setTreeControl(sexp_tree* tree)
{
	_sexp_tree = tree;
}
void MissionCutscenesDialogModel::deleteCutscene(int node)
{
	size_t i;
	for (i = 0; i < m_cutscenes.size(); i++) {
		if (m_cutscenes[i].formula == node) {
			break;
		}
	}

	Assertion(i < m_cutscenes.size(), "Invalid cutscene index!");
	m_cutscenes.erase(m_cutscenes.begin() + i);
	m_sig.erase(m_sig.begin() + i);

	set_modified();
	modelChanged();
}
void MissionCutscenesDialogModel::changeFormula(int old_form, int new_form)
{
	size_t i;
	for (i=0; i<m_cutscenes.size(); i++){
		if (m_cutscenes[i].formula == old_form){
			break;
		}
	}

	Assertion(i < m_cutscenes.size(), "Invalid cutscene index!");
	m_cutscenes[i].formula = new_form;

	set_modified();
	modelChanged();
}
mission_cutscene& MissionCutscenesDialogModel::createNewCutscene()
{
	m_cutscenes.emplace_back();
	m_sig.push_back(-1);

	m_cutscenes.back().type = m_display_cutscene_types;
	m_cutscenes.back().filename[0] = '\0';

	set_modified();
	return m_cutscenes.back();
}
void MissionCutscenesDialogModel::setCurrentCutsceneType(int type)
{
	modify(m_cutscenes[cur_cutscene].type, type);

	modelChanged();
}
void MissionCutscenesDialogModel::setCurrentCutsceneFilename(const char* filename)
{
	Assertion(isCurrentCutsceneValid(), "Current cutscene is not valid!");
	strcpy_s(getCurrentCutscene().filename, NAME_LENGTH, filename);

	set_modified();
	modelChanged();
}

} // namespace fso::fred::dialogs
