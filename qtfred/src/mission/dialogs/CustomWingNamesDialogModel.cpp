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
	if (strcmp(_m_starting[0].c_str(), _m_tvt[0].c_str()))
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


	// copy starting wings
	for (i = 0; i < MAX_STARTING_WINGS; i++) {
		strcpy_s(Starting_wing_names[i], _m_starting[i].c_str());
	}

	// copy squadron wings
	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		strcpy_s(Squadron_wing_names[i], _m_squadron[i].c_str());
	}

	// copy tvt wings
	for (i = 0; i < MAX_TVT_WINGS; i++) {
		strcpy_s(TVT_wing_names[i], _m_tvt[i].c_str());
	}

	_viewport->editor->update_custom_wing_indexes();

	return true;
}

void CustomWingNamesDialogModel::reject() {
}

void CustomWingNamesDialogModel::initializeData() {
	int i;

	// init starting wings
	for (i = 0; i < MAX_STARTING_WINGS; i++) {
		_m_starting[i] = Starting_wing_names[i];
	}

	// init squadron wings
	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		_m_squadron[i] = Squadron_wing_names[i];
	}

	// init tvt wings
	for (i = 0; i < MAX_TVT_WINGS; i++) {
		_m_tvt[i] = TVT_wing_names[i];
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
	return strcmp(Starting_wing_names[0], _m_starting[0].c_str()) || strcmp(Starting_wing_names[1], _m_starting[1].c_str()) || strcmp(Starting_wing_names[2], _m_starting[2].c_str())
		|| strcmp(Squadron_wing_names[0], _m_squadron[0].c_str()) || strcmp(Squadron_wing_names[1], _m_squadron[1].c_str()) || strcmp(Squadron_wing_names[2], _m_squadron[2].c_str()) || strcmp(Squadron_wing_names[3], _m_squadron[3].c_str()) || strcmp(Squadron_wing_names[4], _m_squadron[4].c_str())
		|| strcmp(TVT_wing_names[0], _m_tvt[0].c_str()) || strcmp(TVT_wing_names[1], _m_tvt[1].c_str());;
}

}
}
}