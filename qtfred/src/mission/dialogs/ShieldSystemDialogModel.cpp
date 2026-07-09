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

	for (const auto& info : Ship_info) {
		_shipTypeOptions.emplace_back(info.name);
	}

	for (const auto& iff : Iff_info) {
		_teamOptions.emplace_back(iff.iff_name);
	}

	refresh();
}

void ShieldSystemDialogModel::refresh() {
	_editor->normalizeShieldSysData();
	_editor->exportShieldSysData(_teams, _types);
	_modified = false;
}

bool ShieldSystemDialogModel::apply() {
	// Each Apply button commits immediately; nothing to do at dialog close.
	return true;
}

void ShieldSystemDialogModel::reject() {
	// nothing to do
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
	_currTeam = team;
}
void ShieldSystemDialogModel::setCurrentShipType(int type)
{
	Assertion(type >= 0 && type < MAX_SHIP_CLASSES, "Ship class index %d is invalid!", type); // NOLINT(readability-simplify-boolean-expr)
	_currType = type;
}

GlobalShieldStatus ShieldSystemDialogModel::getCurrentTeamShieldSys() const
{
	return _teams[_currTeam];
}
GlobalShieldStatus ShieldSystemDialogModel::getCurrentTypeShieldSys() const
{
	return _types[_currType];
}

void ShieldSystemDialogModel::applyTeam(bool hasShields)
{
	// Mutate only the currently selected team entry. Other entries already
	// reflect current per-ship state (refresh() ran on open / after last
	// apply), so importShieldSysData is a no-op for them. Mixed entries
	// stay Mixed, Has/No entries already match each ship's flag.
	_teams[_currTeam] = hasShields ? GlobalShieldStatus::HasShields : GlobalShieldStatus::NoShields;
	_editor->importShieldSysData(_teams, _types);
	refresh();
}

void ShieldSystemDialogModel::applyType(bool hasShields)
{
	_types[_currType] = hasShields ? GlobalShieldStatus::HasShields : GlobalShieldStatus::NoShields;
	_editor->importShieldSysData(_teams, _types);
	refresh();
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
