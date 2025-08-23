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
	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			Objects[Ships[i].objnum].flags.set(Object::Object_Flags::No_shields);
		}
	}
}

void GlobalShipFlagsDialogModel::setNoSubspaceDriveOnFightersBombers()
{
	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			// only for fighters and bombers
			Ships[i].flags.set(Ship::Ship_Flags::No_subspace_drive,
				Ship_info[Ships[i].ship_info_index].is_fighter_bomber());
		}
	}
}

void GlobalShipFlagsDialogModel::setPrimitiveSensorsOnFightersBombers()
{
	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			// only for fighters and bombers
			Ships[i].flags.set(Ship::Ship_Flags::Primitive_sensors,
				Ship_info[Ships[i].ship_info_index].is_fighter_bomber());
		}
	}
}

void GlobalShipFlagsDialogModel::setAffectedByGravityOnFightersBombers()
{
	// FRED only affects fighters and bombers.. that seems really strange for this one
	
	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			// only for fighters and bombers
			Ships[i].flags.set(Ship::Ship_Flags::Affected_by_gravity,
				Ship_info[Ships[i].ship_info_index].is_fighter_bomber());
		}
	}
}

} // namespace fso::fred::dialogs
