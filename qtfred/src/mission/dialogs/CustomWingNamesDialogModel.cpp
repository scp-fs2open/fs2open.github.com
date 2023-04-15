#include "CustomWingNamesDialogModel.h"

#include "ship/ship.h"

namespace fso {
namespace fred {
namespace dialogs {

CustomWingNamesDialogModel::CustomWingNamesDialogModel(QObject * parent, EditorViewport * viewport) :
	AbstractDialogModel(parent, viewport) {
	initializeData();
}

bool CustomWingNamesDialogModel::apply() {
	int i;

	for (auto wing : _m_starting) {
		Editor::strip_quotation_marks(wing);
	}

	for (auto wing : _m_squadron) {
		Editor::strip_quotation_marks(wing);
	}

	for (auto wing : _m_tvt) {
		Editor::strip_quotation_marks(wing);
	}
	
	if (strcmp(_m_starting[0].c_str(), _m_tvt[0].c_str()) != 0)
	{
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Custom Wing Error", "The first starting wing and the first team-versus-team wing must have the same wing name.",
		{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return false;
		}
	}

	if (!stricmp(_m_starting[0].c_str(), _m_starting[1].c_str()) || !stricmp(_m_starting[0].c_str(), _m_starting[2].c_str())
		|| !stricmp(_m_starting[1].c_str(), _m_starting[2].c_str()))
	{
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Custom Wing Error", "Duplicate wing names in starting wing list.",
		{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return false;
		}
	}

	if (!stricmp(_m_squadron[0].c_str(), _m_squadron[1].c_str()) || !stricmp(_m_squadron[0].c_str(), _m_squadron[2].c_str()) || !stricmp(_m_squadron[0].c_str(), _m_squadron[3].c_str()) || !stricmp(_m_squadron[0].c_str(), _m_squadron[4].c_str())
		|| !stricmp(_m_squadron[1].c_str(), _m_squadron[2].c_str()) || !stricmp(_m_squadron[1].c_str(), _m_squadron[3].c_str()) || !stricmp(_m_squadron[1].c_str(), _m_squadron[4].c_str())
		|| !stricmp(_m_squadron[2].c_str(), _m_squadron[3].c_str()) || !stricmp(_m_squadron[2].c_str(), _m_squadron[4].c_str())
		|| !stricmp(_m_squadron[3].c_str(), _m_squadron[4].c_str()))
	{
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Custom Wing Error", "Duplicate wing names in squadron wing list.",
		{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return false;
		}
	}

	if (!stricmp(_m_tvt[0].c_str(), _m_tvt[1].c_str()))
	{
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Custom Wing Error", "Duplicate wing names in team-versus-team wing list.",
		{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return false;
		}
	}


	// copy starting wings -- Cyborg: Fixing this will require us to redo the 
	// wing name selection editor (Maybe a dedicated squadron editor would be good?)
	// But we have not yet finished overhauling the loadout selection screen, which
	// would mean that allowing FRED to work with more than just the first three starting wings
	// would require SCPUI, and at least some other changes.
	for (i = 0; i < RETAIL_MAX_STARTING_WINGS; i++) {
		if (i < static_cast<int>(Starting_wing_names.size())){
			Starting_wing_names[i] = _m_starting[i];
		} else if (!_m_starting[i].empty()){
			Starting_wing_names.push_back(_m_starting[i]);
		}
	}

	// copy squadron wings
	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		if (i < static_cast<int>(Squadron_wing_names.size())){
			Squadron_wing_names[i] = _m_squadron[i];
		} else if (!_m_squadron[i].empty()) {
			Squadron_wing_names.push_back(_m_squadron[i]);
		}
	}

	// copy tvt wings
	// Cyborg - Nothing fancy for now, just allow copying to the first wing of each team.
	for (i = 0; i < MAX_TVT_TEAMS; i++) {
		TVT_wing_names[i].clear();
		if (!_m_tvt[i].empty()){
			TVT_wing_names[i].push_back(_m_tvt[i]);
		}
	}

	_viewport->editor->update_custom_wing_indexes();

	return true;
}

void CustomWingNamesDialogModel::reject() {
}

void CustomWingNamesDialogModel::initializeData() {
	int i;

	// init starting wings
	for (i = 0; i < RETAIL_MAX_STARTING_WINGS; i++) {
		if (i < static_cast<int>(Starting_wing_names.size())){
			_m_starting[i] = Starting_wing_names[i];
		} else {
			_m_starting[i].clear();
		}
	}

	// init squadron wings
	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		if (i < static_cast<int>(Squadron_wing_names.size())){
			_m_squadron[i] = Squadron_wing_names[i];
		} else {
			_m_squadron[i].clear();
		}
	}

	// init tvt wings
	for (i = 0; i < MAX_TVT_TEAMS; i++) {
		if (!TVT_wing_names[i].empty()){
			_m_tvt[i] = TVT_wing_names[i][0];
		} else {
			_m_tvt[i].clear();
		}
	}
}

void CustomWingNamesDialogModel::setStartingWing(SCP_string str, int index) {
	modify(_m_starting[index], str);
}

void CustomWingNamesDialogModel::setSquadronWing(SCP_string str, int index) {
	modify(_m_squadron[index], str);
}

void CustomWingNamesDialogModel::setTvTWing(SCP_string str, int index) {
	modify(_m_tvt[index], str);
}

SCP_string CustomWingNamesDialogModel::getStartingWing(int index) {
	return _m_starting[index];
}

SCP_string CustomWingNamesDialogModel::getSquadronWing(int index) {
	return _m_squadron[index];
}

SCP_string CustomWingNamesDialogModel::getTvTWing(int index) {
	return _m_tvt[index];
}

bool CustomWingNamesDialogModel::query_modified() {

	// did the first wing change
	if (!Starting_wing_names.empty() && Starting_wing_names[0]!= _m_starting[0])
		return true;
	
	// did the first wing get added?
	if (Starting_wing_names.empty() && !_m_starting[0].empty())
		return true;

	// did the second wing change?
	if (Starting_wing_names.size() > 1 && Starting_wing_names[1] != _m_starting[1])
		return true;

	// did the second wing get added?
	if (Starting_wing_names.size() < 2 && !_m_starting[1].empty())
		return true;

	// did the third wing change?
	if (Starting_wing_names.size() > 2 && Starting_wing_names[2] != _m_starting[2])	
		return true;

	// did the third wing get added?
	if (Starting_wing_names.size() < 3 && !_m_starting[2].empty())
		return true;


	// same for squadron wings
	// wing 1
	if (!Squadron_wing_names.empty() && Squadron_wing_names[0] != _m_squadron[0])
		return true;

	if (Squadron_wing_names.empty() && !_m_squadron[0].empty())
		return true;

	// wing 2
	if (Squadron_wing_names.size() > 1 && Squadron_wing_names[1] != _m_squadron[1])
		return true;

	if (Squadron_wing_names.size() < 2 && !_m_squadron[1].empty())
		return true;

	// wing 3
	if (Squadron_wing_names.size() > 2 && Squadron_wing_names[2] != _m_squadron[2])
		return true;

	if (Squadron_wing_names.size() < 3 && !_m_squadron[2].empty())
		return true;

	// wing 4
	if (Squadron_wing_names.size() > 3 && Squadron_wing_names[3] != _m_squadron[3])
		return true;

	if (Squadron_wing_names.size() < 4 && !_m_squadron[3].empty())
		return true;

	// wing 5
	if (Squadron_wing_names.size() > 4 && Squadron_wing_names[4] != _m_squadron[4])
		return true;

	if (Squadron_wing_names.size() < 5 && !_m_squadron[4].empty())
		return true;

	// have the tvt wings changed?
	// wing 1
	if (!TVT_wing_names[0].empty() && TVT_wing_names[0][0] != _m_tvt[0])
		return true;
	
	if (TVT_wing_names[0].empty() && !_m_tvt[0].empty())
		return true;

	// wing 2
	if (!TVT_wing_names[1].empty() && TVT_wing_names[1][0] != _m_tvt[1])
		return true;

	if (TVT_wing_names[1].empty() && !_m_tvt[1].empty())
		return true;
}

}
}
}