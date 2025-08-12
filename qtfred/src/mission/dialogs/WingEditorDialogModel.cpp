#include "WingEditorDialogModel.h"
#include <QMessageBox>
#include "FredApplication.h"
#include <unordered_set>
#include "iff_defs/iff_defs.h"
#include "mission/missionhotkey.h"
#include "mission/missionparse.h"
#include <QObject>
#include <QMessageBox>

namespace fso::fred::dialogs {
WingEditorDialogModel::WingEditorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	reloadFromCurWing();

	connect(_editor, &Editor::currentObjectChanged, this, &WingEditorDialogModel::onEditorSelectionChanged);
}

void WingEditorDialogModel::onEditorSelectionChanged(int)
{
	reloadFromCurWing();
}


void WingEditorDialogModel::reloadFromCurWing()
{
	int w = _editor->cur_wing;

	if (w == _currentWingIndex)
		return; // no change

	_currentWingIndex = w;

	if (w < 0 || Wings[w].wave_count == 0) {
		// no wing selected/valid — disable or clear fields
		modify(_currentWingIndex, -1);
		modify(_currentWingName, SCP_string());
		return;
	}

	const auto& wing = Wings[w];
	modify(_currentWingIndex, w);
	modify(_currentWingName, SCP_string(wing.name));
	// TODO: copy values from `wing` into your model fields & emit changed signals

	Q_EMIT wingChanged();
}

bool WingEditorDialogModel::wingIsValid() const
{
	return _currentWingIndex >= 0 && _currentWingIndex < MAX_WINGS && Wings[_currentWingIndex].wave_count > 0;
}

wing* WingEditorDialogModel::getCurrentWing() const
{
	if (!wingIsValid()) {
		return nullptr;
	}
	return &Wings[_currentWingIndex];
}

bool WingEditorDialogModel::isPlayerWing() const
{
	if (!wingIsValid()) {
		return false; // TODO make isSafeForWing() function
	}

	return _editor->wing_is_player_wing(_currentWingIndex);
}

bool WingEditorDialogModel::containsPlayerStart() const
{
	if (!wingIsValid()) {
		return false; // TODO make isSafeForWing() function
	}

	return _editor->wing_contains_player_start(_currentWingIndex);
}

bool WingEditorDialogModel::wingAllFighterBombers() const
{
	const auto w = getCurrentWing();

	if (!w)
		return false;

	for (int i = 0; i < w->wave_count; ++i) {
		const int si = w->ship_index[i];
		if (si < 0 || si >= MAX_SHIPS)
			return false;
		const int sclass = Ships[si].ship_info_index;
		if (!SCP_vector_inbounds(Ship_info, sclass))
			return false;
		if (!Ship_info[sclass].is_fighter_bomber())
			return false;
	}
	return true;
}

bool WingEditorDialogModel::arrivalIsDockBay() const
{
	const auto w = getCurrentWing();

	if (!w)
		return false;

	switch (w->arrival_location) {
		case ArrivalLocation::FROM_DOCK_BAY:
			return true;
		default:
			return false;
	}
}

bool WingEditorDialogModel::arrivalNeedsTarget() const
{
	const auto w = getCurrentWing();

	if (!w)
		return false;

	switch (w->arrival_location) {
		case ArrivalLocation::AT_LOCATION:
			return false;
		default:
			return true;
	}
}


bool WingEditorDialogModel::departureIsDockBay() const
{
	const auto w = getCurrentWing();

	if (!w)
		return false;

	switch (w->departure_location) {
	case DepartureLocation::TO_DOCK_BAY:
		return true;
	default:
		return false;
	}
}

bool WingEditorDialogModel::departureNeedsTarget() const
{
	const auto w = getCurrentWing();

	if (!w)
		return false;

	switch (w->departure_location) {
	case DepartureLocation::AT_LOCATION:
		return false;
	default:
		return true;
	}
}

int WingEditorDialogModel::getMaxWaveThreshold() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	if (!w)
		return 0;

	const int perWaveMax = w->wave_count - 1;
	const int poolLimit = MAX_SHIPS_PER_WING - w->wave_count;
	return std::max(0, std::min(perWaveMax, poolLimit));
}

int WingEditorDialogModel::getMinArrivalDistance() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	if (!w)
		return 0;

	switch (w->arrival_location) {
		case ArrivalLocation::AT_LOCATION:
		case ArrivalLocation::FROM_DOCK_BAY:
			return 0;
		default:
			break;
	}

	const int anchor = w->arrival_anchor;

	// If special anchor or invalid, no radius to enforce
	if (anchor < 0 || (anchor & SPECIAL_ARRIVAL_ANCHOR_FLAG))
		return 0;

	// Anchor should be a real ship
	if (anchor >= 0 && anchor < MAX_SHIPS) {
		const int objnum = Ships[anchor].objnum;
		if (objnum >= 0) {
			const object& obj = Objects[objnum];

			// Enforce at least min(500, 2.0 * target_radius)
			float min_rad = std::round(MIN_TARGET_ARRIVAL_MULTIPLIER * obj.radius);
			return std::min(MIN_TARGET_ARRIVAL_DISTANCE, min_rad);
		}
	}

	return 0;
}

SCP_vector<SCP_string> WingEditorDialogModel::getCurrentSelectableWings()
{
	// going to make an exception for this function.  Going to just send the list as a secondary thing that gets added
	// to the rest of the combobox
	SCP_vector<SCP_string> wingNames;

	for (const auto& wing : Wings) {
		// strlen is slow.... and that's all I was trying to check anyway
		if (wing.name[0] != '\0') {
			wingNames.emplace_back(wing.name);
		}
	}

	return wingNames;
}

std::pair<int, SCP_vector<SCP_string>> WingEditorDialogModel::getLeaderList()
{
	std::pair<int, SCP_vector<SCP_string>> out;
	if (_currentWingIndex < 0)
		return out;

	out.first = ::Wings[_currentWingIndex].special_ship;
	for (int x = 0; x < ::Wings[_currentWingIndex].wave_count; ++x) {
		int si = ::Wings[_currentWingIndex].ship_index[x];
		if (si >= 0 && si < MAX_SHIPS) {
			out.second.emplace_back(Ships[si].ship_name);
		}
	}
	return out;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getHotkeyList() const
{
	std::vector<std::pair<int, std::string>> items;
	items.emplace_back(-1, "None");

	for (int i = 0; i < MAX_KEYED_TARGETS; ++i) {
		auto key = textify_scancode(Key_sets[i]);
		SCP_string key_str = "Set " + std::to_string(i + 1) + " (" + key + ")";
		items.emplace_back(i, key_str);
	}
	
	items.emplace_back(MAX_KEYED_TARGETS, "Hidden");

	return items;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getFormationList() const
{
	std::vector<std::pair<int, std::string>> items;
	items.emplace_back(-1, "Default");

	for (int i = 0; i < static_cast<int>(Wing_formations.size()); i++) {
		items.emplace_back(i, Wing_formations[i].name);
	}

	return items;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getArrivalLocationList() const
{
	std::vector<std::pair<int, std::string>> items;
	for (int i = 0; i < MAX_ARRIVAL_NAMES; i++) {
		items.emplace_back(i, Arrival_location_names[i]);
	}
	return items;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getDepartureLocationList() const
{
	std::vector<std::pair<int, std::string>> items;
	for (int i = 0; i < MAX_DEPARTURE_NAMES; i++) {
		items.emplace_back(i, Departure_location_names[i]);
	}
	return items;
}

static bool shipHasDockBay(int ship_info_index)
{
	if (ship_info_index < 0 || ship_info_index >= (int)::Ship_info.size())
		return false;
	auto mn = ::Ship_info[ship_info_index].model_num;
	if (mn < 0)
		return false;
	auto pm = model_get(mn);
	return pm && pm->ship_bay && pm->ship_bay->num_paths > 0;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getArrivalTargetList() const
{
	std::vector<std::pair<int, std::string>> items;
	const auto* w = getCurrentWing();
	if (!w)
		return items;

	// No target needed for free-space arrival
	if (w->arrival_location == ArrivalLocation::AT_LOCATION)
		return items;

	const bool requireDockBay = (w->arrival_location == ArrivalLocation::FROM_DOCK_BAY);

	// Add "special" anchors (Any friendly/hostile/etc.), both all-ships and players-only variants
	if (!requireDockBay) {
		char buf[NAME_LENGTH + 15];
		for (int restrict_to_players = 0; restrict_to_players < 2; ++restrict_to_players) {
			for (int iff = 0; iff < (int)::Iff_info.size(); ++iff) {
				stuff_special_arrival_anchor_name(buf, iff, restrict_to_players, 0);
				items.emplace_back(get_special_anchor(buf), buf);
			}
		}
	}

	// Add ships (and player starts) that are NOT currently marked
	for (object* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if ((objp->type != OBJ_SHIP && objp->type != OBJ_START) || objp->flags[Object::Object_Flags::Marked]) {
			continue;
		}

		const int ship_idx = objp->instance;
		const int sclass = ::Ships[ship_idx].ship_info_index;

		if (requireDockBay && !shipHasDockBay(sclass))
			continue;

		items.emplace_back(ship_idx, ::Ships[ship_idx].ship_name);
	}

	return items;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getDepartureTargetList() const
{
	std::vector<std::pair<int, std::string>> items;
	const auto* w = getCurrentWing();
	if (!w)
		return items;

	// Only dock-bay departures need a specific target
	if (w->departure_location != DepartureLocation::TO_DOCK_BAY)
		return items;

	for (object* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if ((objp->type != OBJ_SHIP && objp->type != OBJ_START) || objp->flags[Object::Object_Flags::Marked]) {
			continue;
		}

		const int ship_idx = objp->instance;
		const int sclass = ::Ships[ship_idx].ship_info_index;

		if (!shipHasDockBay(sclass))
			continue;

		items.emplace_back(ship_idx, ::Ships[ship_idx].ship_name);
	}

	return items;
}

SCP_string WingEditorDialogModel::getWingName() const
{
	if (!wingIsValid())
		return "";

	return _currentWingName;
}

void WingEditorDialogModel::setWingName(const SCP_string& name)
{
	if (!wingIsValid())
		return;

	if (_editor->rename_wing(_currentWingIndex, name)) {
		modify(_currentWingName, name);
		Q_EMIT modelChanged();
	}
}

int WingEditorDialogModel::getWingLeaderIndex() const
{
	if (!wingIsValid())
		return -1;

	const auto w = getCurrentWing();
	int idx = w->special_ship;

	return (idx >= 0 && idx < w->wave_count) ? idx : -1;
}

void WingEditorDialogModel::setWingLeaderIndex(int idx)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	
	modify(w->special_ship, idx);
}

int WingEditorDialogModel::getNumberOfWaves() const
{
	if (!wingIsValid())
		return 1;

	const auto w = getCurrentWing();
	int num = w->num_waves;

	return num;
}

void WingEditorDialogModel::setNumberOfWaves(int num)
{
	if (!wingIsValid())
		return;

	// you read that right, I don't see a limit for the number of waves.
	// Original Fred had a UI limit of 99, but yolo
	if (num < 1) {
		//QMessageBox::warning(this, "Invalid Number of Waves", "The number of waves must be at least 1.");
		num = 1;
	}
	auto* w = getCurrentWing();

	modify(w->num_waves, num);
}

int WingEditorDialogModel::getWaveThreshold() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	int thr = w->threshold;

	return thr;
}

void WingEditorDialogModel::setWaveThreshold(int newThreshold)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

	modify(w->threshold, std::clamp(newThreshold, 0, getMaxWaveThreshold()));
}

int WingEditorDialogModel::getHotkey() const
{
	if (!wingIsValid())
		return -1;

	const auto w = getCurrentWing();
	int idx = w->hotkey;

	return idx;
}

void WingEditorDialogModel::setHotkey(int newHotkeyIndex)
{
	if (!wingIsValid())
		return;

	// Valid values:
	// -1  = None
	// 0..MAX_KEYED_TARGETS-1 = Sets 1..N
	// MAX_KEYED_TARGETS      = Hidden
	if (newHotkeyIndex < -1 || newHotkeyIndex > MAX_KEYED_TARGETS) {
		newHotkeyIndex = -1; // ignore bad input; treat as None
	}

	auto* w = getCurrentWing();
	modify(w->hotkey, newHotkeyIndex);
}

int WingEditorDialogModel::getFormationId() const
{
	if (!wingIsValid())
		return -1;

	const auto w = getCurrentWing();
	int id = w->formation;

	return id;
}

void WingEditorDialogModel::setFormationId(int newFormationId)
{
	if (!wingIsValid())
		return;
	auto* w = getCurrentWing();

	if (!SCP_vector_inbounds(Wing_formations, newFormationId)) {
		newFormationId = 0; // ignore bad input; treat as Default
	}

	modify(w->formation, newFormationId);
}

float WingEditorDialogModel::getFormationScale() const
{
	if (!wingIsValid())
		return 1.0f;

	const auto w = getCurrentWing();
	float scale = w->formation_scale;

	return scale;
}

void WingEditorDialogModel::setFormationScale(float newScale)
{
	if (!wingIsValid())
		return;
	auto* w = getCurrentWing();

	if (newScale < 0.0f) {
		newScale = 0.0f; // TODO Unsure if formation scale has a minimum value
	}

	modify(w->formation_scale, newScale);
}

void WingEditorDialogModel::alignWingFormation()
{
	if (!wingIsValid())
		return;
	
	auto wingp = getCurrentWing();
	auto leader_objp = &Objects[Ships[wingp->ship_index[0]].objnum];

	// TODO make all changes to the model temporary and only apply them on close/next/previous
	//auto old_formation = wingp->formation;
	//auto old_formation_scale = wingp->formation_scale;

	//wingp->formation = m_formation - 1;
	//wingp->formation_scale = (float)atof(m_formation_scale);

	for (int i = 1; i < wingp->wave_count; i++) {
		auto objp = &Objects[Ships[wingp->ship_index[i]].objnum];

		get_absolute_wing_pos(&objp->pos, leader_objp, _currentWingIndex, i, false);
		objp->orient = leader_objp->orient;
	}

	// roll back temporary formation
	//wingp->formation = old_formation;
	//wingp->formation_scale = old_formation_scale;

	_editor->updateAllViewports();
}

void WingEditorDialogModel::showWingFlagsDialog()
{
	if (!wingIsValid())
		return;
	//fred_main_wing_flags_dialog(_currentWingIndex, _viewport);
	//_editor->updateAllViewports();
}

void WingEditorDialogModel::selectPreviousWing()
{
	int begin = (_currentWingIndex >= 0 && _currentWingIndex < MAX_WINGS) ? _currentWingIndex : 0;
	int prv = -1;

	for (int step = 1; step <= MAX_WINGS; ++step) {
		// add MAX_WINGS before modulo to avoid negative
		int i = (begin - step + MAX_WINGS) % MAX_WINGS;
		if (Wings[i].wave_count > 0 && Wings[i].name[0] != '\0') {
			prv = i;
			break;
		}
	}

	if (prv < 0) {
		return; // no other wings
	}
	
	_editor->unmark_all();
	_editor->mark_wing(prv);
	reloadFromCurWing();
}

void WingEditorDialogModel::selectNextWing()
{
	int begin = (_currentWingIndex >= 0 && _currentWingIndex < MAX_WINGS) ? _currentWingIndex : -1;
	int nxt = -1;

	for (int step = 1; step <= MAX_WINGS; ++step) {
		// add MAX_WINGS before modulo to avoid negative
		int i = (begin + step + MAX_WINGS) % MAX_WINGS;
		if (Wings[i].wave_count > 0 && Wings[i].name[0] != '\0') {
			nxt = i;
			break;
		}
	}

	if (nxt < 0) {
		return; // no other wings
	}

	_editor->unmark_all();
	_editor->mark_wing(nxt);
	reloadFromCurWing();
}

void WingEditorDialogModel::deleteCurrentWing()
{
	if (!wingIsValid())
		return;

	_editor->delete_wing(_currentWingIndex);
	reloadFromCurWing();
}

void WingEditorDialogModel::disbandCurrentWing()
{
	if (!wingIsValid())
		return;

	_editor->remove_wing(_currentWingIndex);
	reloadFromCurWing();
}

SCP_string WingEditorDialogModel::getSquadronLogo() const
{
	if (!wingIsValid())
		return "";

	const auto w = getCurrentWing();
	SCP_string filename = w->wing_squad_filename;

	return filename;
}

ArrivalLocation WingEditorDialogModel::getArrivalType() const
{
	if (!wingIsValid())
		return ArrivalLocation::AT_LOCATION; // fallback to a default value

	const auto w = getCurrentWing();
	return w->arrival_location;
}

void WingEditorDialogModel::setArrivalType(ArrivalLocation newArrivalType)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	modify(w->arrival_location, newArrivalType);

	// If the new arrival type is a dock bay,clear warp in parameters
	// else, clear arrival paths
	if (newArrivalType == ArrivalLocation::FROM_DOCK_BAY) {
		// TODO clear warp in parameters
	} else {
		// TODO clear arrival paths
	}

	// If the new arrival type does not need a target, clear it
	if (newArrivalType == ArrivalLocation::AT_LOCATION) {
		modify(w->arrival_anchor, -1);
		modify(w->arrival_distance, 0);
	} else {

		// Set the target to the first available
		const auto& targets = getArrivalTargetList();

		if (targets.empty()) {
			// No targets available, set to -1
			modify(w->arrival_anchor, -1);
			modify(w->arrival_distance, 0);
			return;
		}

		const int currentAnchor = w->arrival_anchor;

		bool valid_anchor = std::find_if(targets.begin(), targets.end(), [currentAnchor](const auto& entry) {
			return entry.first == currentAnchor;
		}) != targets.end();

		if (!valid_anchor) {
			// Set to the first available target
			modify(w->arrival_anchor, targets[0].first);
		}

		// Set the distance to minimum if current is smaller
		int minDistance = getMinArrivalDistance();
		if (w->arrival_distance < minDistance) {
			setArrivalDistance(minDistance);
		}
	}
}

int WingEditorDialogModel::getArrivalDelay() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	return w->arrival_delay;
}

void WingEditorDialogModel::setArrivalDelay(int delayIn)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	if (delayIn < 0) {
		delayIn = 0;
	}
	modify(w->arrival_delay, delayIn);
}

int WingEditorDialogModel::getMinWaveDelay() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	return w->wave_delay_min;
}

void WingEditorDialogModel::setMinWaveDelay(int newMin)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	if (newMin < 0) {
		newMin = 0;
	}
	// Ensure the minimum is not greater than the maximum
	if (newMin > w->wave_delay_max) {
		w->wave_delay_max = newMin;
	}
	modify(w->wave_delay_min, newMin);
}

int WingEditorDialogModel::getMaxWaveDelay() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	return w->wave_delay_max;
}

void WingEditorDialogModel::setMaxWaveDelay(int newMax)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	if (newMax < 0) {
		newMax = 0;
	}
	// Ensure the maximum is not less than the minimum
	if (newMax < w->wave_delay_min) {
		w->wave_delay_min = newMax;
	}
	modify(w->wave_delay_max, newMax);
}

int WingEditorDialogModel::getArrivalTarget() const
{
	if (!wingIsValid())
		return -1;

	const auto w = getCurrentWing();
	// If the arrival location is AT_LOCATION, no target is needed.
	if (w->arrival_location == ArrivalLocation::AT_LOCATION) {
		return -1;
	}
	
	return w->arrival_anchor;
}

void WingEditorDialogModel::setArrivalTarget(int targetIndex)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

	// If the arrival location is AT_LOCATION, no target is needed.
	if (w->arrival_location == ArrivalLocation::AT_LOCATION) {
		targetIndex = -1;
	}
	
	// Validate against the dynamic list which includes special anchors unless dock-bay is required
	bool ok = false;
	for (const auto& [id, /*label*/ _] : getArrivalTargetList()) {
		if (id == targetIndex) {
			ok = true;
			break;
		}
	}

	if (!ok) {
		targetIndex = -1;
	}

	modify(w->arrival_anchor, targetIndex);

	// Set the distance to minimum if current is smaller
	int minDistance = getMinArrivalDistance();
	if (minDistance < w->arrival_distance) {
		setArrivalDistance(0);
	}
}

int WingEditorDialogModel::getArrivalDistance() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	return w->arrival_distance;
}

void WingEditorDialogModel::setArrivalDistance(int newDistance)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	if (newDistance < 0) {
		newDistance = 0;
	}

	// Enforce safe min distance
	const int minD = getMinArrivalDistance();
	if (newDistance != 0 && std::abs(newDistance) < minD) {
		newDistance = minD;
	}

	modify(w->arrival_distance, newDistance);
}

int WingEditorDialogModel::getArrivalTree() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	return w->arrival_cue;
}

void WingEditorDialogModel::setArrivalTree(int /*oldTree*/, int newTree)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

	// TODO not sure what oldTree was for, ignoring for now

	modify(w->arrival_cue, newTree);
}

bool WingEditorDialogModel::getNoArrivalWarpFlag() const
{
	if (!wingIsValid())
		return false;

	const auto w = getCurrentWing();
	return w->flags[Ship::Wing_Flags::No_arrival_warp];
}

void WingEditorDialogModel::setNoArrivalWarpFlag(bool flagIn)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	if (flagIn) {
		w->flags.set(Ship::Wing_Flags::No_arrival_warp);
	} else {
		w->flags.remove(Ship::Wing_Flags::No_arrival_warp);
	}
}

bool WingEditorDialogModel::getNoArrivalWarpAdjustFlag() const
{
	if (!wingIsValid())
		return false;

	const auto w = getCurrentWing();
	return w->flags[Ship::Wing_Flags::Same_arrival_warp_when_docked];
}

void WingEditorDialogModel::setNoArrivalWarpAdjustFlag(bool flagIn)
{
	if (!wingIsValid())
		return;
	auto* w = getCurrentWing();
	if (flagIn) {
		w->flags.set(Ship::Wing_Flags::Same_arrival_warp_when_docked);
	} else {
		w->flags.remove(Ship::Wing_Flags::Same_arrival_warp_when_docked);
	}
}

DepartureLocation WingEditorDialogModel::getDepartureType() const
{
	if (!wingIsValid())
		return DepartureLocation::AT_LOCATION; // fallback to a default value

	const auto w = getCurrentWing();
	return w->departure_location;
}

void WingEditorDialogModel::setDepartureType(DepartureLocation newDepartureType)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	modify(w->departure_location, newDepartureType);

	// If the new departure type is a dock bay,clear warp out parameters
	// else, clear departure paths
	if (newDepartureType == DepartureLocation::TO_DOCK_BAY) {
		// TODO clear warp out parameters
	} else {
		// TODO clear departure paths
	}

	// If the new departure type does not need a target, clear it
	if (newDepartureType == DepartureLocation::AT_LOCATION) {
		modify(w->departure_anchor, -1);
	} else {

		// Set the target to the first available
		const auto& targets = getDepartureTargetList();

		if (targets.empty()) {
			// No targets available, set to -1
			modify(w->departure_anchor, -1);
			return;
		}

		const int currentAnchor = w->departure_anchor;

		bool valid_anchor = std::find_if(targets.begin(), targets.end(), [currentAnchor](const auto& entry) {
			return entry.first == currentAnchor;
		}) != targets.end();

		if (!valid_anchor) {
			// Set to the first available target
			modify(w->departure_anchor, targets[0].first);
		}
	}
}

int WingEditorDialogModel::getDepartureDelay() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	return w->departure_delay;
}

void WingEditorDialogModel::setDepartureDelay(int delayIn)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	if (delayIn < 0) {
		delayIn = 0;
	}
	modify(w->departure_delay, delayIn);
}

int WingEditorDialogModel::getDepartureTarget() const
{
	if (!wingIsValid())
		return -1;

	const auto w = getCurrentWing();
	// If the departure location is AT_LOCATION, no target is needed.
	if (w->departure_location == DepartureLocation::AT_LOCATION) {
		return -1;
	}

	return w->departure_anchor;
}

void WingEditorDialogModel::setDepartureTarget(int targetIndex)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();
	// If the departure location is AT_LOCATION, no target is needed.
	if (w->departure_location == DepartureLocation::AT_LOCATION) {
		targetIndex = -1;
	}

	// Validate against the dynamic list that already filters for dock bays.
	bool ok = false;
	for (const auto& [id, /*label*/ _] : getDepartureTargetList()) {
		if (id == targetIndex) {
			ok = true;
			break;
		}
	}

	if (!ok) {
		targetIndex = -1; // invalid choice -> clear
	}

	modify(w->departure_anchor, targetIndex);
}

int WingEditorDialogModel::getDepartureTree() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	return w->departure_cue;
}

void WingEditorDialogModel::setDepartureTree(int /*oldTree*/, int newTree)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

	// TODO not sure what oldTree was for, ignoring for now

	modify(w->departure_cue, newTree);
}

bool WingEditorDialogModel::getNoDepartureWarpFlag() const
{
	if (!wingIsValid())
		return false;

	const auto w = getCurrentWing();
	return w->flags[Ship::Wing_Flags::No_departure_warp];
}

void WingEditorDialogModel::setNoDepartureWarpFlag(bool flagIn)
{
	if (!wingIsValid())
		return;
	auto* w = getCurrentWing();
	if (flagIn) {
		w->flags.set(Ship::Wing_Flags::No_departure_warp);
	} else {
		w->flags.remove(Ship::Wing_Flags::No_departure_warp);
	}
}

bool WingEditorDialogModel::getNoDepartureWarpAdjustFlag() const
{
	if (!wingIsValid())
		return false;

	const auto w = getCurrentWing();
	return w->flags[Ship::Wing_Flags::Same_departure_warp_when_docked];
}

void WingEditorDialogModel::setNoDepartureWarpAdjustFlag(bool flagIn)
{
	if (!wingIsValid())
		return;
	auto* w = getCurrentWing();
	if (flagIn) {
		w->flags.set(Ship::Wing_Flags::Same_departure_warp_when_docked);
	} else {
		w->flags.remove(Ship::Wing_Flags::Same_departure_warp_when_docked);
	}
}

bool WingEditorDialogModel::getReinforcementFlag()
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::Reinforcement];
}

bool WingEditorDialogModel::getCountingGoalsFlag()
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::Ignore_count];
}

bool WingEditorDialogModel::getArrivalMusicFlag()
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::No_arrival_music];
}

bool WingEditorDialogModel::getArrivalMessageFlag()
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::No_arrival_message];
}

bool WingEditorDialogModel::getFirstWaveMessageFlag()
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::No_first_wave_message];
}

bool WingEditorDialogModel::getDynamicGoalsFlag()
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::No_dynamic];
}

bool WingEditorDialogModel::setReinforcementFlag(bool flagIn)
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	// TODO: This may need to add/remove the reinforcement flags for the inidividual ships.
	if (flagIn) {
		Wings[_currentWingIndex].flags.set(Ship::Wing_Flags::Reinforcement);
	} else {
		Wings[_currentWingIndex].flags.remove(Ship::Wing_Flags::Reinforcement);
	}

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::Reinforcement];
}

bool WingEditorDialogModel::setCountingGoalsFlag(bool flagIn)
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	if (flagIn) {
		Wings[_currentWingIndex].flags.set(Ship::Wing_Flags::Ignore_count);
	} else {
		Wings[_currentWingIndex].flags.remove(Ship::Wing_Flags::Ignore_count);
	}

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::Ignore_count];
}

bool WingEditorDialogModel::setArrivalMusicFlag(bool flagIn)
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	if (flagIn) {
		Wings[_currentWingIndex].flags.set(Ship::Wing_Flags::No_arrival_music);
	} else {
		Wings[_currentWingIndex].flags.remove(Ship::Wing_Flags::No_arrival_music);
	}

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::No_arrival_music];
}

bool WingEditorDialogModel::setArrivalMessageFlag(bool flagIn)
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	if (flagIn) {
		Wings[_currentWingIndex].flags.set(Ship::Wing_Flags::No_arrival_message);
	} else {
		Wings[_currentWingIndex].flags.remove(Ship::Wing_Flags::No_arrival_message);
	}

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::No_arrival_message];
}

bool WingEditorDialogModel::setFirstWaveMessageFlag(bool flagIn)
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	if (flagIn) {
		Wings[_currentWingIndex].flags.set(Ship::Wing_Flags::No_first_wave_message);
	} else {
		Wings[_currentWingIndex].flags.remove(Ship::Wing_Flags::No_first_wave_message);
	}

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::No_first_wave_message];
}

bool WingEditorDialogModel::setDynamicGoalsFlag(bool flagIn)
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	if (flagIn) {
		Wings[_currentWingIndex].flags.set(Ship::Wing_Flags::No_dynamic);
	} else {
		Wings[_currentWingIndex].flags.remove(Ship::Wing_Flags::No_dynamic);
	}

	return Wings[_currentWingIndex].flags[Ship::Wing_Flags::No_dynamic];
}

SCP_string WingEditorDialogModel::setSquadLogo(SCP_string filename)
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return "";

	if (filename.size() >= TOKEN_LENGTH) {
		SCP_string messageOut = "This file name is too long, the maximum is 31 characters.";

		QMessageBox msgBox;
		msgBox.setText(messageOut.c_str());
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.exec();

		return "";
	}

	strcpy_s(Wings[_currentWingIndex].wing_squad_filename, filename.c_str());
	return Wings[_currentWingIndex].wing_squad_filename;
}

bool WingEditorDialogModel::resetArrivalPaths()
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	Wings[_currentWingIndex].arrival_path_mask = 0;

	return true;
}

bool WingEditorDialogModel::resetDeparturePaths()
{
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS)
		return false;

	Wings[_currentWingIndex].departure_path_mask = 0;

	return true;
}

bool WingEditorDialogModel::setArrivalPath(std::pair<int, bool> pathStatusIn)
{
	// shouldn't even be here...
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS ||
		Wings[_currentWingIndex].arrival_location != ArrivalLocation::FROM_DOCK_BAY)
		return !pathStatusIn.second;

	// bad arrival target
	if (Wings[_currentWingIndex].arrival_anchor < 0 ||
		Wings[_currentWingIndex].arrival_anchor >= MAX_SHIPS ||
		Ships[Wings[_currentWingIndex].arrival_anchor].ship_name[0] == '\0') //TODO test this
		return !pathStatusIn.second;

	auto anchorShipModel = model_get(Ship_info[Ships[Wings[_currentWingIndex].arrival_anchor].ship_info_index].model_num);

	const int pathIndex = pathStatusIn.first;

	// not enough paths
	if (!anchorShipModel || !anchorShipModel->ship_bay || pathIndex < 0 || pathIndex >= anchorShipModel->ship_bay->num_paths) {
		return !pathStatusIn.second;
	}

	//
}

bool WingEditorDialogModel::setDeparturePath(std::pair<int, bool> pathStatusIn)
{
	// shouldn't even be here...
	if (_currentWingIndex < 0 || _currentWingIndex >= MAX_WINGS ||
		Wings[_currentWingIndex].departure_location != DepartureLocation::TO_DOCK_BAY)
		return !pathStatusIn.second;

	// bad arrival target
	if (Wings[_currentWingIndex].departure_anchor < 0 || Wings[_currentWingIndex].departure_anchor >= MAX_SHIPS ||
		Ships[Wings[_currentWingIndex].departure_anchor].ship_name[0] == '\0') // TODO test this
		return !pathStatusIn.second;

	auto anchorShipModel = model_get(Ship_info[Ships[Wings[_currentWingIndex].arrival_anchor].ship_info_index].model_num);

	const int pathIndex = pathStatusIn.first;

	// not enough paths
	if (!anchorShipModel || !anchorShipModel->ship_bay || pathIndex < 0 || pathIndex >= anchorShipModel->ship_bay->num_paths) {
		return !pathStatusIn.second;
	}
}

} // namespace fso::fred::dialogs