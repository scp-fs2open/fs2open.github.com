#include "WingEditorDialogModel.h"
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
	prepareSquadLogoList();

	connect(_editor, &Editor::currentObjectChanged, this, &WingEditorDialogModel::onEditorSelectionChanged);
	connect(_editor, &Editor::missionChanged, this, &WingEditorDialogModel::onEditorMissionChanged);
}

void WingEditorDialogModel::onEditorSelectionChanged(int)
{
	reloadFromCurWing();
}

void WingEditorDialogModel::onEditorMissionChanged()
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
		// No wing selected
		modify(_currentWingIndex, -1);
		modify(_currentWingName, SCP_string());
		return;
	}

	const auto& wing = Wings[w];
	modify(_currentWingIndex, w);
	modify(_currentWingName, SCP_string(wing.name));

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

std::vector<std::pair<SCP_string, bool>> WingEditorDialogModel::getDockBayPathsForWingMask(uint32_t mask, int anchorShipnum)
{
	std::vector<std::pair<SCP_string, bool>> out;

	if (anchorShipnum < 0 || !ship_has_dock_bay(anchorShipnum))
		return out;

	const int sii = Ships[anchorShipnum].ship_info_index;
	const int model_num = Ship_info[sii].model_num;
	auto* pm = model_get(model_num);
	if (!pm || !pm->ship_bay)
		return out;

	const int num_paths = pm->ship_bay->num_paths;
	const auto* idx = pm->ship_bay->path_indexes;

	const bool all_allowed = (mask == 0);
	out.reserve(static_cast<size_t>(num_paths));

	for (int i = 0; i < num_paths; ++i) {
		const int path_id = idx[i];
		const char* name = pm->paths[path_id].name;
		const bool allowed = all_allowed ? true : ((mask & (1u << i)) != 0);
		out.emplace_back(name ? SCP_string{name} : SCP_string{"<unnamed path>"}, allowed);
	}

	return out;
}

void WingEditorDialogModel::prepareSquadLogoList()
{
	pilot_load_squad_pic_list();

	for (int i = 0; i < Num_pilot_squad_images; i++) {
		squadLogoList.emplace_back(Pilot_squad_image_names[i]);
	}
}

bool WingEditorDialogModel::isPlayerWing() const
{
	if (!wingIsValid()) {
		return false;
	}

	return _editor->wing_is_player_wing(_currentWingIndex);
}

bool WingEditorDialogModel::containsPlayerStart() const
{
	if (!wingIsValid()) {
		return false;
	}

	return Editor::wing_contains_player_start(_currentWingIndex);
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

std::pair<int, SCP_vector<SCP_string>> WingEditorDialogModel::getLeaderList() const
{
	std::pair<int, SCP_vector<SCP_string>> items;
	if (!wingIsValid())
		return items;

	auto w = getCurrentWing();

	items.first = w->special_ship;
	for (int x = 0; x < w->wave_count; ++x) {
		int si = w->ship_index[x];
		if (si >= 0 && si < MAX_SHIPS) {
			items.second.emplace_back(Ships[si].ship_name);
		}
	}
	return items;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getHotkeyList()
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

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getFormationList()
{
	std::vector<std::pair<int, std::string>> items;
	items.emplace_back(-1, "Default");

	for (int i = 0; i < static_cast<int>(Wing_formations.size()); i++) {
		items.emplace_back(i, Wing_formations[i].name);
	}

	return items;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getArrivalLocationList()
{
	std::vector<std::pair<int, std::string>> items;
	items.reserve(MAX_ARRIVAL_NAMES);
	for (int i = 0; i < MAX_ARRIVAL_NAMES; i++) {
		items.emplace_back(i, Arrival_location_names[i]);
	}
	return items;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getDepartureLocationList()
{
	std::vector<std::pair<int, std::string>> items;
	items.reserve(MAX_DEPARTURE_NAMES);
	for (int i = 0; i < MAX_DEPARTURE_NAMES; i++) {
		items.emplace_back(i, Departure_location_names[i]);
	}
	return items;
}

static bool shipHasDockBay(int ship_info_index)
{
	if (ship_info_index < 0 || ship_info_index >= (int)::Ship_info.size())
		return false;
	auto mn = Ship_info[ship_info_index].model_num;
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

	// Add special anchors (Any friendly/hostile/etc); both all ships and players only variants
	if (!requireDockBay) {
		char buf[NAME_LENGTH + 15];
		for (int restrict_to_players = 0; restrict_to_players < 2; ++restrict_to_players) {
			for (int iff = 0; iff < (int)::Iff_info.size(); ++iff) {
				stuff_special_arrival_anchor_name(buf, iff, restrict_to_players, 0);
				items.emplace_back(get_special_anchor(buf), buf);
			}
		}
	}

	// Add ships and player starts that are NOT currently marked
	for (object* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if ((objp->type != OBJ_SHIP && objp->type != OBJ_START) || objp->flags[Object::Object_Flags::Marked]) {
			continue;
		}

		const int ship_idx = objp->instance;
		const int sclass = Ships[ship_idx].ship_info_index;

		if (requireDockBay && !shipHasDockBay(sclass))
			continue;

		items.emplace_back(ship_idx, Ships[ship_idx].ship_name);
	}

	return items;
}

std::vector<std::pair<int, std::string>> WingEditorDialogModel::getDepartureTargetList() const
{
	std::vector<std::pair<int, std::string>> items;
	const auto* w = getCurrentWing();
	if (!w)
		return items;

	// Only dockbay departures need a specific target
	if (w->departure_location != DepartureLocation::TO_DOCK_BAY)
		return items;

	for (object* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if ((objp->type != OBJ_SHIP && objp->type != OBJ_START) || objp->flags[Object::Object_Flags::Marked]) {
			continue;
		}

		const int ship_idx = objp->instance;
		const int sclass = Ships[ship_idx].ship_info_index;

		if (!shipHasDockBay(sclass))
			continue;

		items.emplace_back(ship_idx, Ships[ship_idx].ship_name);
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
		newScale = 0.0f; // Unsure if formation scale has a minimum value
	}

	modify(w->formation_scale, newScale);
}

void WingEditorDialogModel::alignWingFormation()
{
	if (!wingIsValid())
		return;
	
	auto wingp = getCurrentWing();
	auto leader_objp = &Objects[Ships[wingp->ship_index[0]].objnum];

	// TODO Handle this when the dialog supports temporary changes in the future
	//make all changes to the model temporary and only apply them on close/next/previous
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

SCP_string WingEditorDialogModel::getSquadLogo() const
{
	if (!wingIsValid())
		return "";

	const auto w = getCurrentWing();

	return w->wing_squad_filename;
}

void WingEditorDialogModel::setSquadLogo(const SCP_string& filename)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

	if (filename.size() >= TOKEN_LENGTH) {
		return;
	}

	strcpy_s(w->wing_squad_filename, filename.c_str());
	set_modified();
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

std::vector<std::pair<SCP_string, bool>> WingEditorDialogModel::getWingFlags() const
{
	std::vector<std::pair<SCP_string, bool>> flags;
	if (!wingIsValid())
		return flags;

	const auto* w = getCurrentWing();

	for (size_t i = 0; i < Num_parse_wing_flags; ++i) {
		auto flagDef = Parse_wing_flags[i];

		// Skip flags that have checkboxes elsewhere in the dialog
		if (flagDef.def == Ship::Wing_Flags::No_arrival_warp || flagDef.def == Ship::Wing_Flags::No_departure_warp ||
			flagDef.def == Ship::Wing_Flags::Same_arrival_warp_when_docked ||
			flagDef.def == Ship::Wing_Flags::Same_departure_warp_when_docked) {
			continue;
		}

		bool checked = w->flags[flagDef.def];
		flags.emplace_back(flagDef.name, checked);
	}

	return flags;
}

void WingEditorDialogModel::setWingFlags(const std::vector<std::pair<SCP_string, bool>>& newFlags)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

	for (const auto& [name, checked] : newFlags) {
		// Find the matching flagDef by name
		for (size_t i = 0; i < Num_parse_wing_flags; ++i) {
			if (!stricmp(name.c_str(), Parse_wing_flags[i].name)) {
				if (checked)
					w->flags.set(Parse_wing_flags[i].def);
				else
					w->flags.remove(Parse_wing_flags[i].def);
				break;
			}
		}
	}

	set_modified();
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

	// If the new arrival type is a dock bay, clear warp in parameters
	// else, clear arrival paths
	if (newArrivalType == ArrivalLocation::FROM_DOCK_BAY) {
		for (auto& ship : Ships) {
			if (ship.objnum < 0)
				continue;
			if (ship.wingnum != _currentWingIndex)
				continue;

			ship.warpin_params_index = -1;
		}
	} else {
		modify(w->arrival_path_mask, 0);
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

	if (w->arrival_anchor == targetIndex) {
		return; // no change
	}

	modify(w->arrival_anchor, targetIndex);

	// Set the distance to minimum if current is smaller
	int minDistance = getMinArrivalDistance();
	if (minDistance < w->arrival_distance) {
		setArrivalDistance(0);
	}

	modify(w->arrival_path_mask, 0);
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

std::vector<std::pair<SCP_string, bool>> WingEditorDialogModel::getArrivalPaths() const
{
	if (!wingIsValid())
		return {};

	const auto* w = getCurrentWing();
	if (w->arrival_location != ArrivalLocation::FROM_DOCK_BAY)
		return {};

	return getDockBayPathsForWingMask(w->arrival_path_mask, w->arrival_anchor);
}

void WingEditorDialogModel::setArrivalPaths(const std::vector<std::pair<SCP_string, bool>>& chosen)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

	if (w->arrival_location != ArrivalLocation::FROM_DOCK_BAY)
		return;

	const int anchor = w->arrival_anchor;
	if (anchor < 0 || !ship_has_dock_bay(anchor))
		return;

	// Rebuild mask in the same order we produced the list
	int mask = 0;
	int num_allowed = 0;

	for (size_t i = 0; i < chosen.size(); ++i) {
		if (chosen[i].second) {
			mask |= (1 << static_cast<int>(i));
			++num_allowed;
		}
	}

	// if all are allowed, store 0
	if (num_allowed == static_cast<int>(chosen.size())) {
		mask = 0;
	}

	if (mask != w->arrival_path_mask) {
		w->arrival_path_mask = mask;
		set_modified();
	}
}

int WingEditorDialogModel::getArrivalTree() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	return w->arrival_cue;
}

void WingEditorDialogModel::setArrivalTree(int newTree)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

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
	set_modified();
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
	set_modified();
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
		for (auto& ship : Ships) {
			if (ship.objnum < 0)
				continue;
			if (ship.wingnum != _currentWingIndex)
				continue;

			ship.warpout_params_index = -1;
		}
	} else {
		modify(w->departure_path_mask, 0);
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

	if (w->departure_anchor == targetIndex) {
		return; // no change
	}

	modify(w->departure_anchor, targetIndex);
	modify(w->departure_path_mask, 0);
}

std::vector<std::pair<SCP_string, bool>> WingEditorDialogModel::getDeparturePaths() const
{
	if (!wingIsValid())
		return {};

	const auto* w = getCurrentWing();
	if (w->departure_location != DepartureLocation::TO_DOCK_BAY)
		return {};

	return getDockBayPathsForWingMask(w->departure_path_mask, w->departure_anchor);
}

void WingEditorDialogModel::setDeparturePaths(const std::vector<std::pair<SCP_string, bool>>& chosen)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

	if (w->departure_location != DepartureLocation::TO_DOCK_BAY)
		return;

	const int anchor = w->departure_anchor;
	if (anchor < 0 || !ship_has_dock_bay(anchor))
		return;

	// Rebuild mask in the same order we produced the list
	int mask = 0;
	int num_allowed = 0;

	for (size_t i = 0; i < chosen.size(); ++i) {
		if (chosen[i].second) {
			mask |= (1 << static_cast<int>(i));
			++num_allowed;
		}
	}

	// if all are allowed, store 0
	if (num_allowed == static_cast<int>(chosen.size())) {
		mask = 0;
	}

	if (mask != w->departure_path_mask) {
		w->departure_path_mask = mask;
		set_modified();
	}
}

int WingEditorDialogModel::getDepartureTree() const
{
	if (!wingIsValid())
		return 0;

	const auto w = getCurrentWing();
	return w->departure_cue;
}

void WingEditorDialogModel::setDepartureTree(int newTree)
{
	if (!wingIsValid())
		return;

	auto* w = getCurrentWing();

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
	set_modified();
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
	set_modified();
}

} // namespace fso::fred::dialogs