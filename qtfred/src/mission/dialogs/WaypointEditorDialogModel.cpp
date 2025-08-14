#include <mission/object.h>
#include <globalincs/linklist.h>
#include <ship/ship.h>
#include <iff_defs/iff_defs.h>
#include "mission/dialogs/WaypointEditorDialogModel.h"

namespace fso::fred::dialogs {

WaypointEditorDialogModel::WaypointEditorDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
	connect(viewport->editor, &Editor::currentObjectChanged, this, &WaypointEditorDialogModel::onSelectedObjectChanged);
	connect(viewport->editor, &Editor::objectMarkingChanged, this, &WaypointEditorDialogModel::onSelectedObjectMarkingChanged);
	connect(viewport->editor, &Editor::missionChanged, this, &WaypointEditorDialogModel::onMissionChanged);

	initializeData();
}

bool WaypointEditorDialogModel::apply()
{
	if (!validateData()) {
		return false;
	}

	// apply name
	char old_name[255];
	strcpy_s(old_name, _editor->cur_waypoint_list->get_name());
	const char* str = _currentName.c_str();
	_editor->cur_waypoint_list->set_name(str);
	if (strcmp(old_name, str) != 0) {
		update_sexp_references(old_name, str);
		_editor->ai_update_goal_references(sexp_ref_type::WAYPOINT, old_name, str);
		_editor->update_texture_replacements(old_name, str); // ?? Uh really? Check that FRED does this also
	}

	_editor->missionChanged();
	return true;
}

void WaypointEditorDialogModel::reject()
{
	// do nothing
}

void WaypointEditorDialogModel::initializeData()
{
	_enabled = true;

	if (query_valid_object(_editor->currentObject) && Objects[_editor->currentObject].type == OBJ_WAYPOINT) {
		Assertion(_editor->cur_waypoint_list == find_waypoint_list_with_instance(Objects[_editor->currentObject].instance), "Waypoint no longer exists in the mission!");
	}

	updateWaypointPathList();

	if (_editor->cur_waypoint_list != nullptr) {
		_currentName = _editor->cur_waypoint_list->get_name();
	} else {
		_currentName = "";
		_enabled = false;
	}

	Q_EMIT waypointPathMarkingChanged();
}

void WaypointEditorDialogModel::updateWaypointPathList()
{

	_waypointPathList.clear();
	_currentWaypointPathSelected = -1;

	for (size_t i = 0; i < Waypoint_lists.size(); ++i) {
		_waypointPathList.emplace_back(Waypoint_lists[i].get_name(), static_cast<int>(i));
	}

	if (_editor->cur_waypoint_list != nullptr) {
		int index = find_index_of_waypoint_list(_editor->cur_waypoint_list);
		Assertion(index >= 0, "Could not find waypoint path in waypoint path list!");
		_currentWaypointPathSelected = index;
	}
}

bool WaypointEditorDialogModel::validateData()
{
	// Reset flag before applying
	_bypass_errors = false;

	if (query_valid_object(_editor->currentObject) && Objects[_editor->currentObject].type == OBJ_WAYPOINT) {
		Assertion(_editor->cur_waypoint_list == find_waypoint_list_with_instance(Objects[_editor->currentObject].instance), "Waypoint no longer exists in the mission!");
	}

	// wing name collision
	for (auto& wing : Wings) {
		if (!stricmp(wing.name, _currentName.c_str())) {
			showErrorDialogNoCancel("This waypoint path name is already being used by a wing");
			return false;
		}
	}

	// ship name collision
	object* ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			if (!stricmp(_currentName.c_str(), Ships[ptr->instance].ship_name)) {
				showErrorDialogNoCancel("This waypoint path name is already being used by a ship");
				return false;
			}
		}

		ptr = GET_NEXT(ptr);
	}

	// We don't need to check teams.  "Unknown" is a valid name and also an IFF.

	// target priority group name collision
	for (auto& ai : Ai_tp_list) {
		if (!stricmp(_currentName.c_str(), ai.name)) {
			showErrorDialogNoCancel("This waypoint path name is already being used by a target priority group");
			return false;
		}
	}

	// waypoint path name collision
	for (const auto& ii : Waypoint_lists) {
		if (!stricmp(ii.get_name(), _currentName.c_str()) && (&ii != _editor->cur_waypoint_list)) {
			showErrorDialogNoCancel("This waypoint path name is already being used by another waypoint path");
			return false;
		}
	}

	// jump node name collision
	if (jumpnode_get_by_name(_currentName.c_str()) != nullptr) {
		showErrorDialogNoCancel("This waypoint path name is already being used by a jump node");
		return false;
	}

	// formatting
	if (!_currentName.empty() && _currentName[0] == '<') {
		showErrorDialogNoCancel("Waypoint names not allowed to begin with '<'");
		return false;
	}

	return true;
}

void WaypointEditorDialogModel::showErrorDialogNoCancel(const SCP_string& message)
{
	if (_bypass_errors) {
		return;
	}

	_bypass_errors = true;
	_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", message, {DialogButton::Ok});
}

void WaypointEditorDialogModel::onSelectedObjectChanged(int) {
	initializeData();
}

void WaypointEditorDialogModel::onSelectedObjectMarkingChanged(int, bool) {
	initializeData();
}

void WaypointEditorDialogModel::onMissionChanged()
{
	// When the mission is changed we also need to update our data in case one of our elements changed
	initializeData();
}

const SCP_string& WaypointEditorDialogModel::getCurrentName() const {
	return _currentName;
}

void WaypointEditorDialogModel::setCurrentName(const SCP_string& name)
{
	modify(_currentName, name);
}

int WaypointEditorDialogModel::getCurrentlySelectedPath() const {
	return _currentWaypointPathSelected;
}

void WaypointEditorDialogModel::setCurrentlySelectedPath(int id)
{
	if (_currentWaypointPathSelected == id) {
		// Nothing to do here
		return;
	}

	if (id < 0 || id >= static_cast<int>(Waypoint_lists.size())) {
		return; // out of range; ignore
	}

	if (apply()) {
		_editor->unmark_all();

		// mark all waypoints belonging to the selected list
		int listIndex = id;
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (ptr->type == OBJ_WAYPOINT) {
				if (calc_waypoint_list_index(ptr->instance) == listIndex) {
					_editor->markObject(OBJ_INDEX(ptr));
				}
			}
		}

		_currentWaypointPathSelected = id;
	}
}

bool WaypointEditorDialogModel::isEnabled() const {
	return _enabled;
}

const SCP_vector<std::pair<SCP_string, int>>& WaypointEditorDialogModel::getWaypointPathList() const
{
	return _waypointPathList;
}

} // namespace fso::fred::dialogs
