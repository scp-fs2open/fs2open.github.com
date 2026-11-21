#include <ship/ship.h>
#include <mission/missionparse.h>
#include <iff_defs/iff_defs.h>
#include "mission/dialogs/GlobalShipFlagsDialogModel.h"

namespace fso::fred::dialogs {

GlobalShipFlagsDialogModel::GlobalShipFlagsDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {

}

bool GlobalShipFlagsDialogModel::apply() {
	// nothing to do
	return true;
}
void GlobalShipFlagsDialogModel::reject() {
	// nothing to do
}

void GlobalShipFlagsDialogModel::setNoShieldsAll()
{
	bool changed = false;
	for (auto& ship : Ships) {
		if (ship.objnum >= 0) {
			changed |= !Objects[ship.objnum].flags[Object::Object_Flags::No_shields];
			Objects[ship.objnum].flags.set(Object::Object_Flags::No_shields);
		}
	}
	if (changed) {
		_editor->missionChanged();
	}
}

void GlobalShipFlagsDialogModel::setNoSubspaceDriveOnFightersBombers()
{
	bool changed = false;
	for (auto& ship : Ships) {
		if (ship.objnum >= 0) {
			// only for fighters and bombers
			const bool value = Ship_info[ship.ship_info_index].is_fighter_bomber();
			changed |= ship.flags[Ship::Ship_Flags::No_subspace_drive] != value;
			ship.flags.set(Ship::Ship_Flags::No_subspace_drive, value);
		}
	}
	if (changed) {
		_editor->missionChanged();
	}
}

void GlobalShipFlagsDialogModel::setPrimitiveSensorsOnFightersBombers()
{
	bool changed = false;
	for (auto& ship : Ships) {
		if (ship.objnum >= 0) {
			// only for fighters and bombers
			const bool value = Ship_info[ship.ship_info_index].is_fighter_bomber();
			changed |= ship.flags[Ship::Ship_Flags::Primitive_sensors] != value;
			ship.flags.set(Ship::Ship_Flags::Primitive_sensors, value);
		}
	}
	if (changed) {
		_editor->missionChanged();
	}
}

void GlobalShipFlagsDialogModel::setAffectedByGravityOnFightersBombers()
{
	// FRED only affects fighters and bombers.. that seems really strange for this one
	
	bool changed = false;
	for (auto& ship : Ships) {
		if (ship.objnum >= 0) {
			// only for fighters and bombers
			const bool value = Ship_info[ship.ship_info_index].is_fighter_bomber();
			changed |= ship.flags[Ship::Ship_Flags::Affected_by_gravity] != value;
			ship.flags.set(Ship::Ship_Flags::Affected_by_gravity, value);
		}
	}
	if (changed) {
		_editor->missionChanged();
	}
}

} // namespace fso::fred::dialogs
