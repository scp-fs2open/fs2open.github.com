#include "CustomWingNamesDialogModel.h"

#include "ship/ship.h"

namespace fso::fred::dialogs {

CustomWingNamesDialogModel::CustomWingNamesDialogModel(QObject * parent, EditorViewport * viewport) :
	AbstractDialogModel(parent, viewport) {
}

bool CustomWingNamesDialogModel::apply() {
	for (auto& wing : _m_starting) {
		Editor::strip_quotation_marks(wing);
	}

	for (auto& wing : _m_squadron) {
		Editor::strip_quotation_marks(wing);
	}

	for (auto& wing : _m_tvt) {
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


	// No direct application; this model is used to collect custom strings
	// and the actual application is handled by the MissionSpecDialogModel.
	return true;
}

void CustomWingNamesDialogModel::reject() {
	// No direct rejection; this model is used to collect custom strings
	// and the actual rejection is handled by the MissionSpecDialogModel.
}

void CustomWingNamesDialogModel::setInitialStartingWings(const std::array<SCP_string, MAX_STARTING_WINGS>& startingWings)
{
	_m_starting = startingWings;
}

void CustomWingNamesDialogModel::setInitialSquadronWings(const std::array<SCP_string, MAX_SQUADRON_WINGS>& squadronWings)
{
	_m_squadron = squadronWings;
}

void CustomWingNamesDialogModel::setInitialTvTWings(const std::array<SCP_string, MAX_TVT_WINGS>& tvtWings)
{
	_m_tvt = tvtWings;
}

const std::array<SCP_string, MAX_STARTING_WINGS>& CustomWingNamesDialogModel::getStartingWings() const {
	return _m_starting;
}

const std::array<SCP_string, MAX_SQUADRON_WINGS>& CustomWingNamesDialogModel::getSquadronWings() const {
	return _m_squadron;
}

const std::array<SCP_string, MAX_TVT_WINGS>& CustomWingNamesDialogModel::getTvTWings() const {
	return _m_tvt;
}

void CustomWingNamesDialogModel::setStartingWing(const SCP_string& str, int index)
{
	modify(_m_starting[index], str);
}

void CustomWingNamesDialogModel::setSquadronWing(const SCP_string& str, int index)
{
	modify(_m_squadron[index], str);
}

void CustomWingNamesDialogModel::setTvTWing(const SCP_string& str, int index)
{
	modify(_m_tvt[index], str);
}

const SCP_string& CustomWingNamesDialogModel::getStartingWing(int index)
{
	return _m_starting[index];
}

const SCP_string& CustomWingNamesDialogModel::getSquadronWing(int index)
{
	return _m_squadron[index];
}

const SCP_string& CustomWingNamesDialogModel::getTvTWing(int index)
{
	return _m_tvt[index];
}

} // namespace fso::fred::dialogs