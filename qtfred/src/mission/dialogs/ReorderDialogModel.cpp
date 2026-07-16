#include "mission/dialogs/ReorderDialogModel.h"

#include "missioneditor/common.h"
#include "jumpnode/jumpnode.h"
#include "object/waypoint.h"
#include "prop/prop.h"
#include "ship/ship.h"

// FRED-side parallel arrays that reassign_ship_slot keeps in sync (see
// missioneditor/common.h).  Reached the same way the ship editor and
// object-duplication code do.
extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];
extern char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];

namespace fso::fred::dialogs {

ReorderDialogModel::ReorderDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport)
{
}

bool ReorderDialogModel::apply()
{
	// Direct-edit dialog: moves are applied as they are made, so there is nothing
	// to commit here.
	return true;
}

void ReorderDialogModel::reject()
{
	// Direct-edit dialog: nothing to roll back.
}

SCP_vector<int> ReorderDialogModel::getSlots(Type type) const
{
	SCP_vector<int> slotList;
	switch (type) {
	case Type::Ships:
		for (int i = 0; i < MAX_SHIPS; ++i)
			if (Ships[i].objnum >= 0)
				slotList.push_back(i);
		break;
	case Type::Wings:
		for (int i = 0; i < MAX_WINGS; ++i)
			if (Wings[i].wave_count > 0)
				slotList.push_back(i);
		break;
	case Type::Props:
		for (int i = 0; i < static_cast<int>(Props.size()); ++i)
			if (Props[i].has_value())
				slotList.push_back(i);
		break;
	case Type::WaypointLists:
		for (int i = 0; i < static_cast<int>(Waypoint_lists.size()); ++i)
			slotList.push_back(i);
		break;
	case Type::JumpNodes:
		for (int i = 0; i < static_cast<int>(Jump_nodes.size()); ++i)
			if (Jump_nodes[i].GetSCPObjectNumber() >= 0)
				slotList.push_back(i);
		break;
	}
	return slotList;
}

SCP_vector<SCP_string> ReorderDialogModel::getItemNames(Type type) const
{
	SCP_vector<SCP_string> names;
	for (int slot : getSlots(type)) {
		switch (type) {
		case Type::Ships:
			names.emplace_back(Ships[slot].ship_name);
			break;
		case Type::Wings:
			names.emplace_back(Wings[slot].name);
			break;
		case Type::Props:
			names.emplace_back(Props[slot]->prop_name);
			break;
		case Type::WaypointLists:
			names.emplace_back(Waypoint_lists[slot].get_name());
			break;
		case Type::JumpNodes:
			names.emplace_back(Jump_nodes[slot].GetName());
			break;
		}
	}
	return names;
}

void ReorderDialogModel::moveItem(Type type, int from_pos, int to_pos)
{
	if (from_pos == to_pos)
		return;

	SCP_vector<int> slotList = getSlots(type);
	int count = static_cast<int>(slotList.size());
	if (from_pos < 0 || from_pos >= count || to_pos < 0 || to_pos >= count)
		return;

	switch (type) {
	case Type::Ships: {
		FredShipSlotConfig cfg;
		cfg.fred_alt_names = Fred_alt_names;
		cfg.fred_callsigns = Fred_callsigns;
		cfg.cur_ship = &_editor->cur_ship;
		rotate_ship_slots(slotList, from_pos, to_pos, cfg);
		break;
	}
	case Type::Wings: {
		FredWingSlotConfig cfg;
		cfg.wing_objects = _editor->wing_objects;
		cfg.cur_wing = &_editor->cur_wing;
		rotate_wing_slots(slotList, from_pos, to_pos, cfg);
		break;
	}
	case Type::Props:
		rotate_prop_slots(slotList, from_pos, to_pos);
		break;
	case Type::WaypointLists:
		rotate_waypoint_lists(from_pos, to_pos);
		break;
	case Type::JumpNodes:
		rotate_jump_nodes(slotList, from_pos, to_pos);
		break;
	}

	set_modified();
	_editor->missionChanged();
}

} // namespace fso::fred::dialogs
