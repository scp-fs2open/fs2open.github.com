//

#include "MissionStatsDialogModel.h"

#include <ship/ship.h>
#include <object/waypoint.h>
#include <object/object.h>
#include <mission/missionmessage.h>
#include <mission/missiongoals.h>
#include <parse/sexp.h>
#include <parse/sexp_container.h>
#include <iff_defs/iff_defs.h>
#include <jumpnode/jumpnode.h>
#include <prop/prop.h>
#include <globalincs/globals.h>
#include <globalincs/linklist.h>

namespace fso::fred::dialogs {

MissionStatsDialogModel::MissionStatsDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
}

bool MissionStatsDialogModel::apply()
{
	return true;
}

void MissionStatsDialogModel::reject()
{
}

int MissionStatsDialogModel::getNumObjects()
{
	return Num_objects;
}

int MissionStatsDialogModel::getMaxObjects()
{
	return MAX_OBJECTS;
}

int MissionStatsDialogModel::getShipCount()
{
	return ship_get_num_ships();
}

int MissionStatsDialogModel::getMaxShips()
{
	return MAX_SHIPS;
}

int MissionStatsDialogModel::getPropCount()
{
	int count = 0;
	for (const auto& p : Props) {
		if (p.has_value())
			count++;
	}
	return count;
}

int MissionStatsDialogModel::getWingCount()
{
	return Num_wings;
}

int MissionStatsDialogModel::getWaypointPathCount()
{
	return static_cast<int>(Waypoint_lists.size());
}

int MissionStatsDialogModel::getJumpNodeCount()
{
	return static_cast<int>(Jump_nodes.size());
}

int MissionStatsDialogModel::getMessageCount()
{
	return Num_messages;
}

int MissionStatsDialogModel::getEventCount()
{
	return static_cast<int>(Mission_events.size());
}

int MissionStatsDialogModel::getGoalCount()
{
	return static_cast<int>(Mission_goals.size());
}

int MissionStatsDialogModel::getVariableCount()
{
	return sexp_variable_count();
}

int MissionStatsDialogModel::getContainerCount()
{
	return static_cast<int>(get_all_sexp_containers().size());
}

SCP_vector<std::pair<SCP_string, MissionStatsDialogModel::IFFShipCounts>> MissionStatsDialogModel::getShipsByIFF()
{
	SCP_map<int, IFFShipCounts> iffCounts;

	for (auto* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (objp->type != OBJ_SHIP && objp->type != OBJ_START)
			continue;

		const ship* shipp = &Ships[objp->instance];
		const ship_info& sip = Ship_info[shipp->ship_info_index];
		int iff = shipp->team;

		IFFShipCounts& rc = iffCounts[iff];
		if (sip.flags[Ship::Info_Flags::Fighter]) {
			rc.fighter++;
		} else if (sip.flags[Ship::Info_Flags::Bomber]) {
			rc.bomber++;
		} else if (sip.flags[Ship::Info_Flags::Capital]  ||
				   sip.flags[Ship::Info_Flags::Supercap] ||
				   sip.flags[Ship::Info_Flags::Cruiser]  ||
				   sip.flags[Ship::Info_Flags::Corvette]) {
			rc.capital++;
		} else {
			rc.other++;
		}
	}

	SCP_vector<std::pair<SCP_string, IFFShipCounts>> result;
	for (int i = 0; i < static_cast<int>(Iff_info.size()); ++i) {
		auto it = iffCounts.find(i);
		if (it == iffCounts.end())
			continue;
		result.emplace_back(Iff_info[i].iff_name, it->second);
	}
	return result;
}

SCP_vector<MissionStatsDialogModel::EscortEntry> MissionStatsDialogModel::getEscortList()
{
	SCP_vector<EscortEntry> result;

	for (auto* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (objp->type != OBJ_SHIP && objp->type != OBJ_START)
			continue;
		const ship* shipp = &Ships[objp->instance];
		if (shipp->flags[Ship::Ship_Flags::Escort]) {
			result.push_back({ shipp->ship_name, shipp->escort_priority });
		}
	}

	return result;
}

SCP_map<int, SCP_vector<SCP_string>> MissionStatsDialogModel::getHotkeyMap()
{
	SCP_map<int, SCP_vector<SCP_string>> result;

	for (auto& wing : Wings) {
		if (wing.wave_count > 0 && wing.hotkey >= 0) {
			result[wing.hotkey].emplace_back(SCP_string(wing.name) + " (wing)");
		}
	}

	for (auto* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (objp->type != OBJ_SHIP && objp->type != OBJ_START)
			continue;
		const ship* shipp = &Ships[objp->instance];
		if (shipp->hotkey >= 0) {
			result[shipp->hotkey].emplace_back(shipp->ship_name);
		}
	}

	return result;
}

} // namespace fso::fred::dialogs
