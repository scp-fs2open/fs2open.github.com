//
//

#include "SelectionDialogModel.h"

#include <parse/parselo.h>
#include <globalincs/linklist.h>
#include <iff_defs/iff_defs.h>
#include <ship/ship.h>
#include <mission/object.h>

namespace fso {
namespace fred {
namespace dialogs {

SelectionDialogModel::SelectionDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
	for(size_t i = 0; i < Iff_info.size(); i++)
		_filter_iff.push_back(true);

	initializeData();
}
void SelectionDialogModel::initializeData() {

	auto ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		ptr->flags.set(Object::Object_Flags::Temp_marked, ptr->flags[Object::Object_Flags::Marked]);
		ptr = GET_NEXT(ptr);
	}

	updateObjectList();

	_wing_list.clear();
	for (auto i = 0; i < MAX_WINGS; i++) {
		if (Wings[i].wave_count) {
			ListEntry entry;
			entry.name = Wings[i].name;
			entry.id = i;
			entry.selected = false;

			_wing_list.push_back(entry);
		}
	}

	_waypoint_list.clear();
	SCP_list<waypoint_list>::iterator ii;
	int i;
	for (i = 0, ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++i, ++ii) {
		ListEntry entry;
		entry.name = ii->get_name();
		entry.id = i;
		entry.selected = false;

		_wing_list.push_back(entry);
	}
}
void SelectionDialogModel::updateObjectList() {
	updateStatus(true);

	_obj_list.clear();

	if (_filter_starts) {
		auto ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (ptr->type == OBJ_START) {
				ListEntry entry;
				entry.name = Ships[ptr->instance].ship_name;
				entry.id = OBJ_INDEX(ptr);
				entry.selected = ptr->flags[Object::Object_Flags::Temp_marked];

				_obj_list.push_back(entry);
			}

			ptr = GET_NEXT(ptr);
		}
	}

	if (_filter_ships) {
		auto ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (ptr->type == OBJ_SHIP) {
				if (_filter_iff[Ships[ptr->instance].team]) {
					ListEntry entry;
					entry.name = Ships[ptr->instance].ship_name;
					entry.id = OBJ_INDEX(ptr);
					entry.selected = ptr->flags[Object::Object_Flags::Temp_marked];

					_obj_list.push_back(entry);
				}
			}

			ptr = GET_NEXT(ptr);
		}
	}

	if (_filter_waypoints) {
		auto ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (ptr->type == OBJ_WAYPOINT) {
				int waypoint_num;
				waypoint_list* wp_list = find_waypoint_list_with_instance(ptr->instance, &waypoint_num);
				Assert(wp_list != NULL);

				SCP_string text;
				sprintf(text, "%s:%d", wp_list->get_name(), waypoint_num + 1);

				ListEntry entry;
				entry.name = text;
				entry.id = OBJ_INDEX(ptr);
				entry.selected = ptr->flags[Object::Object_Flags::Temp_marked];

				_obj_list.push_back(entry);
			}

			ptr = GET_NEXT(ptr);
		}
	}
}
bool SelectionDialogModel::apply() {
	_editor->unmark_all();
	updateStatus(false);
	auto ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Temp_marked]) {
			_editor->markObject(OBJ_INDEX(ptr));
		}

		ptr = GET_NEXT(ptr);
	}

	return true;
}
void SelectionDialogModel::clearSelection() {
	auto ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		ptr->flags.remove(Object::Object_Flags::Temp_marked);
		ptr = GET_NEXT(ptr);
	}

	for (auto& entry : _obj_list) {
		entry.selected = false;
	}
	for (auto& entry : _wing_list) {
		entry.selected = false;
	}
	for (auto& entry : _waypoint_list) {
		entry.selected = false;
	}

	modelChanged();
}
void SelectionDialogModel::selectAll() {
	for (auto& entry : _obj_list) {
		Objects[entry.id].flags.set(Object::Object_Flags::Temp_marked);
		entry.selected = true;
	}
	for (auto& entry : _wing_list) {
		entry.selected = true;
	}
	for (auto& entry : _waypoint_list) {
		entry.selected = true;
	}

	modelChanged();
}
void SelectionDialogModel::invertSelection() {
	for (auto& entry : _obj_list) {
		entry.selected = !entry.selected;
		Objects[entry.id].flags.set(Object::Object_Flags::Temp_marked, entry.selected);
	}

	updateWingListSelection();

	modelChanged();
}
void SelectionDialogModel::updateWingListSelection() {
	for (auto& entry : _wing_list) {
		auto count = 0;
		for (auto j = 0; j < Wings[entry.id].wave_count; j++) {
			for (auto& obj_entry : _obj_list) {
				if (obj_entry.id == _editor->wing_objects[entry.id][j]) {
					if (obj_entry.selected) {
						count++;
					}

					break;
				}
			}
		}

		entry.selected = count == Wings[entry.id].wave_count;
	}

	for (auto& entry : _waypoint_list) {
		waypoint_list* wp_list = find_waypoint_list_at_index(entry.id);
		Assert(wp_list != NULL);

		auto count = 0;
		SCP_vector<waypoint>::iterator jj;
		int j;
		for (j = 0, jj = wp_list->get_waypoints().begin(); jj != wp_list->get_waypoints().end(); ++j, ++jj) {
			for (auto& obj_entry : _obj_list) {
				if ((Objects[obj_entry.id].type == OBJ_WAYPOINT)
					&& (Objects[obj_entry.id].instance == calc_waypoint_instance(entry.id, j))) {
					if (obj_entry.selected) {
						count++;
					}

					break;
				}
			}
		}

		entry.selected = (uint) count == wp_list->get_waypoints().size();
	}
}
const SCP_vector<SelectionDialogModel::ListEntry>& SelectionDialogModel::getObjectList() const {
	return _obj_list;
}
const SCP_vector<SelectionDialogModel::ListEntry>& SelectionDialogModel::getWingList() const {
	return _wing_list;
}
const SCP_vector<SelectionDialogModel::ListEntry>& SelectionDialogModel::getWaypointList() const {
	return _waypoint_list;
}

bool SelectionDialogModel::isFilterShips() const {
	return _filter_ships;
}
void SelectionDialogModel::setFilterShips(bool filter_ships) {
	if (_filter_ships != filter_ships) {
		_filter_ships = filter_ships;

		updateObjectList();

		modelChanged();
	}
}

bool SelectionDialogModel::isFilterStarts() const {
	return _filter_starts;
}
void SelectionDialogModel::setFilterStarts(bool filter_starts) {
	if (_filter_starts != filter_starts) {
		_filter_starts = filter_starts;

		updateObjectList();

		modelChanged();
	}
}

bool SelectionDialogModel::isFilterWaypoints() const {
	return _filter_waypoints;
}
void SelectionDialogModel::setFilterWaypoints(bool filter_waypoints) {
	if (_filter_waypoints != filter_waypoints) {
		_filter_waypoints = filter_waypoints;

		updateObjectList();

		modelChanged();
	}
}

bool SelectionDialogModel::isFilterIFFTeam(int team) const {
	Assertion(team >= 0 && team < (int)Iff_info.size(), "Team index %d is invalid!", team);

	return _filter_iff[team];
}
void SelectionDialogModel::setFilterIFFTeam(int team, bool filter) {
	Assertion(team >= 0 && team < (int)Iff_info.size(), "Team index %d is invalid!", team);

	if (filter != _filter_iff[team]) {
		_filter_iff[team] = filter;

		updateObjectList();

		modelChanged();
	}
}
void SelectionDialogModel::updateStatus(bool first_time) {
	auto ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		ptr->flags.remove(Object::Object_Flags::Temp_marked);
		ptr = GET_NEXT(ptr);
	}

	for (auto& entry : _obj_list) {
		Objects[entry.id].flags.set(Object::Object_Flags::Temp_marked, entry.selected);
	}
	if (!first_time) {
		updateWingListSelection();
	}
}
void SelectionDialogModel::updateShipListSelection() {
	for (auto& entry : _wing_list) {
		for (auto j = 0; j < Wings[entry.id].wave_count; j++) {
			for (auto& obj_entry : _obj_list) {
				if (obj_entry.id == _editor->wing_objects[entry.id][j]) {
					obj_entry.selected = entry.selected;
					break;
				}
			}
		}
	}

	for (auto& entry : _waypoint_list) {
		waypoint_list* wp_list = find_waypoint_list_at_index(entry.id);
		Assert(wp_list != NULL);
		SCP_vector<waypoint>::iterator jj;
		int j;
		for (j = 0, jj = wp_list->get_waypoints().begin(); jj != wp_list->get_waypoints().end(); ++j, ++jj) {
			for (auto& obj_entry : _obj_list) {
				if ((Objects[obj_entry.id].type == OBJ_WAYPOINT)
					&& (Objects[obj_entry.id].instance == calc_waypoint_instance(entry.id, j))) {
					obj_entry.selected = entry.selected;
					break;
				}
			}
		}
	}
}
void SelectionDialogModel::updateObjectSelection(const SCP_vector<ListEntry>& newSelection) {
	Assertion(_obj_list.size() == newSelection.size(), "Current and new selection size differs!");
	// Check if there was any change at all
	bool changed = false;
	SCP_vector<ListEntry>::iterator this_iter;
	SCP_vector<ListEntry>::const_iterator new_iter;
	for (this_iter = _obj_list.begin(), new_iter = newSelection.cbegin(); this_iter != _obj_list.end(); ++this_iter, ++new_iter) {
		Assertion(this_iter->id == new_iter->id, "List entry mismatch detected!");
		if (this_iter->selected != new_iter->selected) {
			// At least one entry changed selection
			changed = true;
		}
		this_iter->selected = new_iter->selected;
	}

	if (changed) {
		updateWingListSelection();
		selectionUpdated();
	}
}
void SelectionDialogModel::selectWing(int wing_id) {
	for (auto& entry : _wing_list) {
		entry.selected = entry.id == wing_id;
	}
	for (auto& entry : _waypoint_list) {
		entry.selected = false;
	}

	updateShipListSelection();
	selectionUpdated();
}
void SelectionDialogModel::selectWaypointPath(int wp_id) {
	for (auto& entry : _wing_list) {
		entry.selected = false;
	}
	for (auto& entry : _waypoint_list) {
		entry.selected = entry.id == wp_id;
	}

	updateShipListSelection();
	selectionUpdated();
}
void SelectionDialogModel::reject() {
}

}
}
}
