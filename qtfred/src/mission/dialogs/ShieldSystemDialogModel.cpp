#include <ship/ship.h>
#include <mission/missionparse.h>
#include <iff_defs/iff_defs.h>
#include "mission/dialogs/ShieldSystemDialogModel.h"

namespace fso::fred::dialogs {

ShieldSystemDialogModel::ShieldSystemDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport), _teams(Iff_info.size(), GlobalShieldStatus::HasShields), _types(MAX_SHIP_CLASSES, GlobalShieldStatus::HasShields) {

	initializeData();
}

void ShieldSystemDialogModel::initializeData() {
	_currTeam = 0;
	_currType = 0;

	_editor->normalizeShieldSysData();

	_editor->exportShieldSysData(_teams, _types);

	for (const auto& info : Ship_info) {
		_shipTypeOptions.emplace_back(info.name);
	}

	for (const auto& iff : Iff_info) {
		_teamOptions.emplace_back(iff.iff_name);
	}
}

bool ShieldSystemDialogModel::apply() {
	if (query_modified()) {
		_editor->importShieldSysData(_teams, _types);
	}
	return true;
}
void ShieldSystemDialogModel::reject() {
	// nothing to do
}

bool ShieldSystemDialogModel::query_modified() const {
	return !_editor->compareShieldSysData(_teams, _types);
}

int ShieldSystemDialogModel::getCurrentTeam() const
{
	return _currTeam;
}
int ShieldSystemDialogModel::getCurrentShipType() const
{
	return _currType;
}
void ShieldSystemDialogModel::setCurrentTeam(int team)
{
	Assertion(SCP_vector_inbounds(Iff_info, team), "Team index %d out of bounds (size: %d)", team, static_cast<int>(Iff_info.size()));
	modify(_currTeam, team);
}
void ShieldSystemDialogModel::setCurrentShipType(int type)
{
	Assertion(type >= 0 && type < MAX_SHIP_CLASSES, "Ship class index %d is invalid!", type); // NOLINT(readability-simplify-boolean-expr)
	modify(_currType, type);
}

GlobalShieldStatus ShieldSystemDialogModel::getCurrentTeamShieldSys() const
{
	return _teams[_currTeam];
}
GlobalShieldStatus ShieldSystemDialogModel::getCurrentTypeShieldSys() const
{
	return _types[_currType];
}
void ShieldSystemDialogModel::setCurrentTeamShieldSys(bool value)
{
	// UI can only turn shields on or off, so just map to the appropriate enum value

	if (value) {
		modify(_teams[_currTeam], GlobalShieldStatus::HasShields);
	} else {
		modify(_teams[_currTeam], GlobalShieldStatus::NoShields);
	}
}
void ShieldSystemDialogModel::setCurrentTypeShieldSys(bool value)
{
	// UI can only turn shields on or off, so just map to the appropriate enum value

	if (value) {
		modify(_types[_currType], GlobalShieldStatus::HasShields);
	} else {
		modify(_types[_currType], GlobalShieldStatus::NoShields);
	}
}

const SCP_vector<SCP_string>& ShieldSystemDialogModel::getShipTypeOptions() const
{
	return _shipTypeOptions;
}
const SCP_vector<SCP_string>& ShieldSystemDialogModel::getTeamOptions() const
{
	return _teamOptions;
}

} // namespace fso::fred::dialogs
