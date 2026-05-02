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

bool WaypointEditorDialogModel::apply() {
	return true;
}

void WaypointEditorDialogModel::reject() {}

void WaypointEditorDialogModel::initializeData() {
	_selectedWaypointPaths.clear();

	// Collect unique waypoint list indices from all marked OBJ_WAYPOINT objects
	SCP_vector<bool> seen(Waypoint_lists.size(), false);
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_WAYPOINT && ptr->flags[Object::Object_Flags::Marked]) {
			int listIdx = calc_waypoint_list_index(ptr->instance);
			if (listIdx >= 0 && listIdx < static_cast<int>(Waypoint_lists.size()) && !seen[listIdx]) {
				seen[listIdx] = true;
				_selectedWaypointPaths.push_back(listIdx);
			}
		}
	}

	// Fall back to cur_waypoint_list if nothing marked
	if (_selectedWaypointPaths.empty() && _editor->cur_waypoint_list != nullptr) {
		int idx = find_index_of_waypoint_list(_editor->cur_waypoint_list);
		if (idx >= 0) {
			_selectedWaypointPaths.push_back(idx);
		}
	}

	if (!_selectedWaypointPaths.empty()) {
		const auto& path = Waypoint_lists[_selectedWaypointPaths.front()];
		_currentName = path.get_name();
		_noDrawLines = path.get_no_draw_lines();
		_hasCustomColor = path.get_has_custom_color();
		_colorR = path.get_color_r();
		_colorG = path.get_color_g();
		_colorB = path.get_color_b();
	} else {
		_currentName.clear();
		_noDrawLines = false;
		_hasCustomColor = false;
		_colorR = _colorG = _colorB = 255;
	}

	Q_EMIT waypointPathMarkingChanged();
	_modified = false;
}

bool WaypointEditorDialogModel::hasValidSelection() const {
	return !_selectedWaypointPaths.empty();
}

bool WaypointEditorDialogModel::hasMultipleSelection() const {
	return _selectedWaypointPaths.size() > 1;
}

bool WaypointEditorDialogModel::hasAnyPathsInMission() const {
	return !Waypoint_lists.empty();
}

int WaypointEditorDialogModel::getSelectionCount() const {
	return static_cast<int>(_selectedWaypointPaths.size());
}

bool WaypointEditorDialogModel::validateName(const SCP_string& name) {
	// wing name collision
	for (auto& wing : Wings) {
		if (!stricmp(wing.name, name.c_str())) {
			showErrorDialogNoCancel("This waypoint path name is already being used by a wing");
			return false;
		}
	}

	// ship name collision
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			if (!stricmp(name.c_str(), Ships[ptr->instance].ship_name)) {
				showErrorDialogNoCancel("This waypoint path name is already being used by a ship");
				return false;
			}
		}
	}

	// target priority group name collision
	for (auto& ai : Ai_tp_list) {
		if (!stricmp(name.c_str(), ai.name)) {
			showErrorDialogNoCancel("This waypoint path name is already being used by a target priority group");
			return false;
		}
	}

	// waypoint path name collision
	const waypoint_list* current_path = _selectedWaypointPaths.empty() ? nullptr : &Waypoint_lists[_selectedWaypointPaths.front()];
	for (const auto& ii : Waypoint_lists) {
		if (!stricmp(ii.get_name(), name.c_str()) && (&ii != current_path)) {
			showErrorDialogNoCancel("This waypoint path name is already being used by another waypoint path");
			return false;
		}
	}

	// jump node name collision
	if (jumpnode_get_by_name(name.c_str()) != nullptr) {
		showErrorDialogNoCancel("This waypoint path name is already being used by a jump node");
		return false;
	}

	// formatting
	if (!name.empty() && name[0] == '<') {
		showErrorDialogNoCancel("Waypoint names not allowed to begin with '<'");
		return false;
	}

	return true;
}

void WaypointEditorDialogModel::showErrorDialogNoCancel(const SCP_string& message) {
	if (_bypass_errors) {
		return;
	}
	_bypass_errors = true;
	_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", message, {DialogButton::Ok});
}

const SCP_string& WaypointEditorDialogModel::getCurrentName() const {
	return _currentName;
}

bool WaypointEditorDialogModel::setCurrentName(const SCP_string& name) {
	if (hasMultipleSelection() || _selectedWaypointPaths.empty()) {
		return true;
	}

	_bypass_errors = false;

	SCP_string trimmed = name;
	SCP_trim(trimmed);

	if (!validateName(trimmed)) {
		return false;
	}

	auto& path = Waypoint_lists[_selectedWaypointPaths.front()];

	char old_name[NAME_LENGTH];
	strcpy_s(old_name, path.get_name());
	path.set_name(trimmed.c_str());
	const char* new_name = path.get_name();

	if (strcmp(old_name, new_name) != 0) {
		update_sexp_references(old_name, new_name);
		_editor->ai_update_goal_references(sexp_ref_type::WAYPOINT_PATH, old_name, new_name);

		for (auto& wpt : path.get_waypoints()) {
			char old_buf[NAME_LENGTH];
			char new_buf[NAME_LENGTH];
			waypoint_stuff_name(old_buf, old_name, wpt.get_index() + 1);
			waypoint_stuff_name(new_buf, new_name, wpt.get_index() + 1);
			update_sexp_references(old_buf, new_buf);
			_editor->ai_update_goal_references(sexp_ref_type::WAYPOINT, old_buf, new_buf);
		}
	}

	_currentName = new_name;
	set_modified();
	_editor->missionChanged();
	return true;
}

bool WaypointEditorDialogModel::getNoDrawLines() const { return _noDrawLines; }

void WaypointEditorDialogModel::setNoDrawLines(bool val) {
	_noDrawLines = val;
	for (auto idx : _selectedWaypointPaths) {
		Waypoint_lists[idx].set_no_draw_lines(val);
	}
	set_modified();
	_editor->missionChanged();
}

bool WaypointEditorDialogModel::getHasCustomColor() const { return _hasCustomColor; }

void WaypointEditorDialogModel::setHasCustomColor(bool val) {
	_hasCustomColor = val;
	for (auto idx : _selectedWaypointPaths) {
		if (val) {
			Waypoint_lists[idx].set_color((ubyte)_colorR, (ubyte)_colorG, (ubyte)_colorB);
		} else {
			Waypoint_lists[idx].clear_color();
		}
	}
	set_modified();
	_editor->missionChanged();
}

int WaypointEditorDialogModel::getColorR() const { return _colorR; }

void WaypointEditorDialogModel::setColorR(int r) {
	CLAMP(r, 0, 255);
	_colorR = r;
	if (_hasCustomColor) {
		for (auto idx : _selectedWaypointPaths) {
			Waypoint_lists[idx].set_color((ubyte)_colorR, (ubyte)_colorG, (ubyte)_colorB);
		}
	}
	set_modified();
	_editor->missionChanged();
}

int WaypointEditorDialogModel::getColorG() const { return _colorG; }

void WaypointEditorDialogModel::setColorG(int g) {
	CLAMP(g, 0, 255);
	_colorG = g;
	if (_hasCustomColor) {
		for (auto idx : _selectedWaypointPaths) {
			Waypoint_lists[idx].set_color((ubyte)_colorR, (ubyte)_colorG, (ubyte)_colorB);
		}
	}
	set_modified();
	_editor->missionChanged();
}

int WaypointEditorDialogModel::getColorB() const { return _colorB; }

void WaypointEditorDialogModel::setColorB(int b) {
	CLAMP(b, 0, 255);
	_colorB = b;
	if (_hasCustomColor) {
		for (auto idx : _selectedWaypointPaths) {
			Waypoint_lists[idx].set_color((ubyte)_colorR, (ubyte)_colorG, (ubyte)_colorB);
		}
	}
	set_modified();
	_editor->missionChanged();
}

SCP_string WaypointEditorDialogModel::getLayer() const {
	SCP_string result;
	bool first = true;
	for (auto pathIdx : _selectedWaypointPaths) {
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (ptr->type == OBJ_WAYPOINT && calc_waypoint_list_index(ptr->instance) == pathIdx) {
				SCP_string layer = _viewport->getObjectLayerName(OBJ_INDEX(ptr));
				if (first) {
					result = layer;
					first = false;
				} else if (result != layer) {
					return "";
				}
			}
		}
	}
	return result;
}

void WaypointEditorDialogModel::setLayer(const SCP_string& layer) {
	for (auto pathIdx : _selectedWaypointPaths) {
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (ptr->type == OBJ_WAYPOINT && calc_waypoint_list_index(ptr->instance) == pathIdx) {
				_viewport->moveObjectToLayer(OBJ_INDEX(ptr), layer);
			}
		}
	}
	set_modified();
	_editor->missionChanged();
}

void WaypointEditorDialogModel::selectWaypointPathByIndex(int idx) {
	if (idx < 0 || idx >= static_cast<int>(Waypoint_lists.size()))
		return;

	_editor->unmark_all();
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_WAYPOINT && calc_waypoint_list_index(ptr->instance) == idx) {
			_editor->markObject(OBJ_INDEX(ptr));
		}
	}
}

void WaypointEditorDialogModel::selectNextPath() {
	if (Waypoint_lists.empty())
		return;
	int current = _selectedWaypointPaths.empty() ? -1 : _selectedWaypointPaths.front();
	int next = (current + 1) % static_cast<int>(Waypoint_lists.size());
	selectWaypointPathByIndex(next);
}

void WaypointEditorDialogModel::selectPreviousPath() {
	if (Waypoint_lists.empty())
		return;
	int current = _selectedWaypointPaths.empty() ? 0 : _selectedWaypointPaths.front();
	int prev = (current - 1 + static_cast<int>(Waypoint_lists.size())) % static_cast<int>(Waypoint_lists.size());
	selectWaypointPathByIndex(prev);
}

void WaypointEditorDialogModel::onSelectedObjectChanged(int) {
	initializeData();
}

void WaypointEditorDialogModel::onSelectedObjectMarkingChanged(int, bool) {
	initializeData();
}

void WaypointEditorDialogModel::onMissionChanged() {
	initializeData();
}

} // namespace fso::fred::dialogs
