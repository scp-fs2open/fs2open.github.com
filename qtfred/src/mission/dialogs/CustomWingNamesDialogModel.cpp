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
	
	if (_m_starting[0] != _m_tvt[0])
	{
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Custom Wing Error", "The first starting wing and the first team-versus-team wing must have the same wing name.",
		{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return false;
		}
	}

	if (lcase_equal(_m_starting[0], _m_starting[1]) || lcase_equal(_m_starting[0], _m_starting[2])
		|| lcase_equal(_m_starting[1], _m_starting[2]))
	{
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Custom Wing Error", "Duplicate wing names in starting wing list.",
		{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return false;
		}
	}

	if (lcase_equal(_m_squadron[0], _m_squadron[1]) || lcase_equal(_m_squadron[0], _m_squadron[2]) || lcase_equal(_m_squadron[0], _m_squadron[3]) || lcase_equal(_m_squadron[0], _m_squadron[4])
		|| lcase_equal(_m_squadron[1], _m_squadron[2]) || lcase_equal(_m_squadron[1], _m_squadron[3]) || lcase_equal(_m_squadron[1], _m_squadron[4])
		|| lcase_equal(_m_squadron[2], _m_squadron[3]) || lcase_equal(_m_squadron[2], _m_squadron[4])
		|| lcase_equal(_m_squadron[3], _m_squadron[4]))
	{
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Custom Wing Error", "Duplicate wing names in squadron wing list.",
		{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return false;
		}
	}

	if (lcase_equal(_m_tvt[0], _m_tvt[1]))
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
	return Starting_wing_names[0] != _m_starting[0] || Starting_wing_names[1] != _m_starting[1] || Starting_wing_names[2] != _m_starting[2]
		|| Squadron_wing_names[0] != _m_squadron[0] || Squadron_wing_names[1] != _m_squadron[1] || Squadron_wing_names[2] != _m_squadron[2] || Squadron_wing_names[3] != _m_squadron[3] || Squadron_wing_names[4] != _m_squadron[4]
		|| TVT_wing_names[0] != _m_tvt[0] || TVT_wing_names[1] != _m_tvt[1];
}

}
}
}