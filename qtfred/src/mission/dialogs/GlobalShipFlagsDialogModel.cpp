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
	for (auto& ship : Ships) {
		if (ship.objnum >= 0) {
			Objects[ship.objnum].flags.set(Object::Object_Flags::No_shields);
		}
	}
}

void GlobalShipFlagsDialogModel::setNoSubspaceDriveOnFightersBombers()
{
	for (auto& ship : Ships) {
		if (ship.objnum >= 0) {
			// only for fighters and bombers
			ship.flags.set(Ship::Ship_Flags::No_subspace_drive,
				Ship_info[ship.ship_info_index].is_fighter_bomber());
		}
	}
}

void GlobalShipFlagsDialogModel::setPrimitiveSensorsOnFightersBombers()
{
	for (auto& ship : Ships) {
		if (ship.objnum >= 0) {
			// only for fighters and bombers
			ship.flags.set(Ship::Ship_Flags::Primitive_sensors,
				Ship_info[ship.ship_info_index].is_fighter_bomber());
		}
	}
}

void GlobalShipFlagsDialogModel::setAffectedByGravityOnFightersBombers()
{
	// FRED only affects fighters and bombers.. that seems really strange for this one
	
	for (auto& ship : Ships) {
		if (ship.objnum >= 0) {
			// only for fighters and bombers
			ship.flags.set(Ship::Ship_Flags::Affected_by_gravity,
				Ship_info[ship.ship_info_index].is_fighter_bomber());
		}
	}
}

} // namespace fso::fred::dialogs
