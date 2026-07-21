#include "MissionCutscenesDialogModel.h"
#include "state/DialogStateHelpers.h"

#include <QDataStream>
#include <QIODevice>

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
		The_mission.cutscenes.back().formula = _sexp_tree->_model.save_tree(item.formula);
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
QByteArray MissionCutscenesDialogModel::captureWorkingState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(m_cutscenes.size());
	for (size_t i = 0; i < m_cutscenes.size(); ++i) {
		const mission_cutscene& c = m_cutscenes[i];
		ds << static_cast<qint32>(m_sig[i]);
		ds << static_cast<qint32>(c.type);
		ds << QString::fromUtf8(c.filename);
		// c.formula is a tree-widget node id; round-trip the branch through
		// Sexp_nodes to serialize its content.
		const int sexp = _sexp_tree->_model.save_tree(c.formula);
		fso::fred::state::writeSexp(ds, sexp);
		fso::fred::state::freeSexpFormula(sexp);
	}

	return data;
}

void MissionCutscenesDialogModel::restoreWorkingState(const QByteArray& state)
{
	QDataStream ds(state);

	m_cutscenes.clear();
	m_sig.clear();
	_sexp_tree->clear_tree(); // resets tree_nodes; the dialog rebuilds the visuals

	qint32 count;
	ds >> count;
	for (int i = 0; i < count; ++i) {
		mission_cutscene c;
		qint32 sig, type;
		QString filename;

		ds >> sig >> type >> filename;
		const int sexp = fso::fred::state::readSexp(ds);
		c.formula = _sexp_tree->_model.load_sub_tree(sexp, true, "true");
		fso::fred::state::freeSexpFormula(sexp);

		c.type = static_cast<int>(type);
		strcpy_s(c.filename, NAME_LENGTH, filename.toUtf8().constData());

		m_cutscenes.push_back(c);
		m_sig.push_back(static_cast<int>(sig));
	}
	_sexp_tree->_model.post_load();

	cur_cutscene = -1;
	set_modified();
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
	_modified = false;
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
void MissionCutscenesDialogModel::setTreeControl(sexp_tree_view* tree)
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

void MissionCutscenesDialogModel::setCutsceneFilenameAt(int index, const SCP_string& filename)
{
	if (!SCP_vector_inbounds(m_cutscenes, index))
		return;
	strcpy_s(m_cutscenes[index].filename, NAME_LENGTH, filename.c_str());

	set_modified();
	modelChanged();
}

} // namespace fso::fred::dialogs
