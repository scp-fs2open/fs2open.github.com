#include <mission/object.h>
#include <globalincs/linklist.h>
#include <ship/ship.h>
#include <iff_defs/iff_defs.h>
#include <unordered_set>
#include "mission/dialogs/WaypointEditorDialogModel.h"

#include <QTimer>

namespace fso::fred::dialogs {

namespace {

// Writes one or more color channels to every selected path that already uses a custom color.
// Channels still flagged mixed retain their per-path value; non-mixed channels take the model value.
// Skips paths without custom color so a channel edit never silently turns custom-color on.
void applyChannelsToAllPaths(const SCP_vector<int>& selected,
    int r, int g, int b,
    bool rMixed, bool gMixed, bool bMixed)
{
	for (auto idx : selected) {
		auto& path = Waypoint_lists[idx];
		if (!path.get_has_custom_color()) {
			continue;
		}
		int outR = rMixed ? path.get_color_r() : r;
		int outG = gMixed ? path.get_color_g() : g;
		int outB = bMixed ? path.get_color_b() : b;
		path.set_color(outR, outG, outB);
	}
}

} // namespace

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
	_noDrawLinesMixed = false;
	_hasCustomColorMixed = false;
	_redMixed = _greenMixed = _blueMixed = false;

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

		if (hasMultipleSelection()) {
			for (size_t i = 1; i < _selectedWaypointPaths.size(); ++i) {
				const auto& other = Waypoint_lists[_selectedWaypointPaths[i]];
				if (other.get_no_draw_lines() != _noDrawLines)
					_noDrawLinesMixed = true;
				if (other.get_has_custom_color() != _hasCustomColor)
					_hasCustomColorMixed = true;
				if (other.get_color_r() != _colorR)
					_redMixed = true;
				if (other.get_color_g() != _colorG)
					_greenMixed = true;
				if (other.get_color_b() != _colorB)
					_blueMixed = true;
			}
		}
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

bool WaypointEditorDialogModel::hasAnyPathsInMission() {
	return !Waypoint_lists.empty();
}

int WaypointEditorDialogModel::getSelectionCount() const {
	return static_cast<int>(_selectedWaypointPaths.size());
}

bool WaypointEditorDialogModel::validateName(const SCP_string& name) {
	if (name.empty()) {
		showErrorDialogNoCancel("Waypoint path name cannot be empty.");
		return false;
	}

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
	const waypoint_list* current_path = &Waypoint_lists[_selectedWaypointPaths.front()];
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
	if (name[0] == '<') {
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
		// Name field should be disabled in these cases; signal failure so the dialog
		// restores the displayed text from getCurrentName().
		return false;
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
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
	return true;
}

bool WaypointEditorDialogModel::getNoDrawLines() const { return _noDrawLines; }

int WaypointEditorDialogModel::getNoDrawLinesState() const {
	if (_noDrawLinesMixed) return Qt::PartiallyChecked;
	return _noDrawLines ? Qt::Checked : Qt::Unchecked;
}

void WaypointEditorDialogModel::setNoDrawLines(bool val) {
	_noDrawLines = val;
	_noDrawLinesMixed = false;
	for (auto idx : _selectedWaypointPaths) {
		Waypoint_lists[idx].set_no_draw_lines(val);
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

bool WaypointEditorDialogModel::getHasCustomColor() const { return _hasCustomColor; }

int WaypointEditorDialogModel::getHasCustomColorState() const {
	if (_hasCustomColorMixed) return Qt::PartiallyChecked;
	return _hasCustomColor ? Qt::Checked : Qt::Unchecked;
}

void WaypointEditorDialogModel::setHasCustomColor(bool val) {
	_hasCustomColor = val;
	_hasCustomColorMixed = false;
	for (auto idx : _selectedWaypointPaths) {
		auto& path = Waypoint_lists[idx];
		if (val) {
			// Preserve per-path channel values for channels still flagged mixed; apply
			// the model's resolved channel values otherwise.
			int outR = _redMixed   ? path.get_color_r() : _colorR;
			int outG = _greenMixed ? path.get_color_g() : _colorG;
			int outB = _blueMixed  ? path.get_color_b() : _colorB;
			path.set_color(outR, outG, outB);
		} else {
			path.clear_color();
		}
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

int WaypointEditorDialogModel::getColorR() const { return _colorR; }

void WaypointEditorDialogModel::setColorR(int r) {
	// The spinbox uses -1 as its "mixed" sentinel (minimum=-1, specialValueText=" ").
	// A read-back of that sentinel must leave _redMixed untouched... don't "simplify"
	// this guard by letting the value fall through to a clear.
	if (r < 0) return;
	CLAMP(r, 0, 255);
	_colorR = r;
	_redMixed = false;
	if (_hasCustomColor && !_hasCustomColorMixed) {
		applyChannelsToAllPaths(_selectedWaypointPaths, _colorR, _colorG, _colorB,
		                        _redMixed, _greenMixed, _blueMixed);
		_suppressRefresh = true;
		set_modified();
		_editor->missionChanged();
		_suppressRefresh = false;
	}
}

int WaypointEditorDialogModel::getColorG() const { return _colorG; }

void WaypointEditorDialogModel::setColorG(int g) {
	// -1 = spinbox "mixed" sentinel; leave _greenMixed untouched. See setColorR.
	if (g < 0) return;
	CLAMP(g, 0, 255);
	_colorG = g;
	_greenMixed = false;
	if (_hasCustomColor && !_hasCustomColorMixed) {
		applyChannelsToAllPaths(_selectedWaypointPaths, _colorR, _colorG, _colorB,
		                        _redMixed, _greenMixed, _blueMixed);
		_suppressRefresh = true;
		set_modified();
		_editor->missionChanged();
		_suppressRefresh = false;
	}
}

int WaypointEditorDialogModel::getColorB() const { return _colorB; }

void WaypointEditorDialogModel::setColorB(int b) {
	// -1 = spinbox "mixed" sentinel; leave _blueMixed untouched. See setColorR.
	if (b < 0) return;
	CLAMP(b, 0, 255);
	_colorB = b;
	_blueMixed = false;
	if (_hasCustomColor && !_hasCustomColorMixed) {
		applyChannelsToAllPaths(_selectedWaypointPaths, _colorR, _colorG, _colorB,
		                        _redMixed, _greenMixed, _blueMixed);
		_suppressRefresh = true;
		set_modified();
		_editor->missionChanged();
		_suppressRefresh = false;
	}
}

bool WaypointEditorDialogModel::isColorRMixed() const { return _redMixed; }
bool WaypointEditorDialogModel::isColorGMixed() const { return _greenMixed; }
bool WaypointEditorDialogModel::isColorBMixed() const { return _blueMixed; }
bool WaypointEditorDialogModel::hasAnyColorMixed() const {
	return _redMixed || _greenMixed || _blueMixed;
}

SCP_string WaypointEditorDialogModel::getLayer() const {
	std::unordered_set<int> selected(_selectedWaypointPaths.begin(), _selectedWaypointPaths.end());
	SCP_string result;
	bool first = true;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type != OBJ_WAYPOINT) continue;
		if (selected.find(calc_waypoint_list_index(ptr->instance)) == selected.end()) continue;
		SCP_string layer = _viewport->getObjectLayerName(OBJ_INDEX(ptr));
		if (first) {
			result = layer;
			first = false;
		} else if (result != layer) {
			return "";
		}
	}
	return result;
}

void WaypointEditorDialogModel::setLayer(const SCP_string& layer) {
	// moveObjectToLayer may unmark objects (hidden target layer) and fires
	// notifyLayerStructureChanged. Both reach back into our refresh slots and would
	// otherwise rebuild the dialog once per affected waypoint object.
	_suppressRefresh = true;
	std::unordered_set<int> selected(_selectedWaypointPaths.begin(), _selectedWaypointPaths.end());
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type != OBJ_WAYPOINT) continue;
		if (selected.find(calc_waypoint_list_index(ptr->instance)) == selected.end()) continue;
		_viewport->moveObjectToLayer(OBJ_INDEX(ptr), layer);
	}
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
	// One refresh now that the dust has settled.
	initializeData();
}

void WaypointEditorDialogModel::selectWaypointPathByIndex(int idx) {
	if (idx < 0 || idx >= static_cast<int>(Waypoint_lists.size()))
		return;

	// Prevent rebuilding the dialog for each waypoint as we mark them
	_suppressRefresh = true;
	_editor->unmark_all();
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_WAYPOINT && calc_waypoint_list_index(ptr->instance) == idx) {
			_editor->markObject(OBJ_INDEX(ptr));
		}
	}
	_suppressRefresh = false;
	initializeData();
}

int WaypointEditorDialogModel::getSelectedPathIndex() const {
	return _selectedWaypointPaths.empty() ? -1 : _selectedWaypointPaths.front();
}

void WaypointEditorDialogModel::selectNextPath() {
	if (Waypoint_lists.empty())
		return;
	if (_selectedWaypointPaths.empty()) {
		selectWaypointPathByIndex(0);
		return;
	}
	int next = (_selectedWaypointPaths.front() + 1) % static_cast<int>(Waypoint_lists.size());
	selectWaypointPathByIndex(next);
}

void WaypointEditorDialogModel::selectPreviousPath() {
	if (Waypoint_lists.empty())
		return;
	if (_selectedWaypointPaths.empty()) {
		selectWaypointPathByIndex(static_cast<int>(Waypoint_lists.size()) - 1);
		return;
	}
	int prev = (_selectedWaypointPaths.front() - 1 + static_cast<int>(Waypoint_lists.size()))
	           % static_cast<int>(Waypoint_lists.size());
	selectWaypointPathByIndex(prev);
}

void WaypointEditorDialogModel::scheduleInitializeData() {
	// Bulk selection changes fire one signal per object, so coalesce
	// the burst into a single refresh once the event loop settles.tles.
	if (_initPending) {
		return;
	}
	_initPending = true;
	QTimer::singleShot(0, this, [this] {
		_initPending = false;
		initializeData();
	});
}

void WaypointEditorDialogModel::onSelectedObjectChanged(int) {
	if (_suppressRefresh) return;
	scheduleInitializeData();
}

void WaypointEditorDialogModel::onSelectedObjectMarkingChanged(int, bool) {
	if (_suppressRefresh) return;
	scheduleInitializeData();
}

void WaypointEditorDialogModel::onMissionChanged() {
	if (_suppressRefresh) return;
	scheduleInitializeData();
}

} // namespace fso::fred::dialogs
