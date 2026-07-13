//
//

#if defined(_MSC_VER) && _MSC_VER <= 1920
// work around MSVC 2015 and 2017 compiler bug
// https://bugreports.qt.io/browse/QTBUG-72073
#define QT_NO_FLOAT16_OPERATORS
#endif

#include "ShipEditorDialogModel.h"

#include "iff_defs/iff_defs.h"
#include "jumpnode/jumpnode.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "missioneditor/common.h"

#include <globalincs/linklist.h>
#include <globalincs/utility.h>
#include <hud/hudsquadmsg.h>
#include <localization/localize.h>
#include <mission/object.h>

#include <QtWidgets>

namespace fso::fred::dialogs {
ShipEditorDialogModel::ShipEditorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}


int ShipEditorDialogModel::tristate_set(int val, int cur_state)
{
	if (cur_state == Qt::PartiallyChecked)
		return Qt::PartiallyChecked;
	bool cur_bool = (cur_state == Qt::Checked);
	if (static_cast<bool>(val) != cur_bool)
		return Qt::PartiallyChecked;
	return cur_state;
}
int ShipEditorDialogModel::getSingleShip() const
{
	return _singleShip;
}
bool ShipEditorDialogModel::getIfMultipleShips() const
{
	return _multiEdit;
}
int ShipEditorDialogModel::getNumSelectedPlayers() const
{
	return _playerCount;
}
int ShipEditorDialogModel::getNumUnmarkedPlayers() const
{
	return _pshipCount;
}
bool ShipEditorDialogModel::getUIEnable() const
{
	return _enable;
}
int ShipEditorDialogModel::getNumSelectedShips() const
{
	return _shipCount;
}
int ShipEditorDialogModel::getUseCue() const
{
	return _cueInit;
}
int ShipEditorDialogModel::getNumSelectedObjects() const
{
	return _totalCount;
}
int ShipEditorDialogModel::getNumValidPlayers() const
{
	return _pvalidCount;
}
int ShipEditorDialogModel::getIfPlayerShip() const
{
	return _playerShipIndex;
}

SCP_vector<std::pair<SCP_string, int>> ShipEditorDialogModel::getPlayerOrders()
{
	SCP_vector<std::pair<SCP_string, int>> orders;

	// Build the canonical default order set from marked ships (caller guarantees all marked ships share it)
	SCP_set<size_t> default_orders;
	object* objp;
	for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags[Object::Object_Flags::Marked])) {
			const SCP_set<size_t>& these_orders =
				ship_get_default_orders_accepted(&Ship_info[Ships[objp->instance].ship_info_index]);
			if (default_orders.empty()) {
				default_orders = these_orders;
			} else {
				Assert(default_orders == these_orders);
			}
		}
	}

	for (size_t order_id : default_orders) {
		SCP_string name = Player_orders[order_id].localized_name;
		int state = -1;
		for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				(objp->flags[Object::Object_Flags::Marked])) {
				int ship_state = Ships[objp->instance].orders_accepted.contains(order_id) ? Qt::Checked : Qt::Unchecked;
				state = (state == -1) ? ship_state : tristate_set(ship_state, state);
			}
		}
		orders.emplace_back(name, (state == -1) ? Qt::Unchecked : state);
	}

	return orders;
}

void ShipEditorDialogModel::applyPlayerOrders(const SCP_vector<std::pair<SCP_string, int>>& orders)
{
	object* ptr;
	for (ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			auto i = ptr->instance;
			SCP_set<size_t> default_orders = ship_get_default_orders_accepted(&Ship_info[Ships[i].ship_info_index]);
			for (size_t order_id : default_orders) {
				for (const auto& [name, state] : orders) {
					if (name == Player_orders[order_id].localized_name) {
						if (state == Qt::Checked) {
							Ships[i].orders_accepted.insert(order_id);
						} else if (state == Qt::Unchecked) {
							Ships[i].orders_accepted.erase(order_id);
						}
						// Qt::PartiallyChecked: leave each ship's existing state unchanged
						break;
					}
				}
			}
		}
	}
	setModified();
	_editor->missionChanged();
}

void ShipEditorDialogModel::setArrivalPaths(const SCP_vector<std::pair<SCP_string, bool>>& newPaths)
{
	_arrivalPaths = newPaths;
	if (!newPaths.empty() && !_multiEdit && _singleShip >= 0) {
		int num_allowed = 0;
		int m_path_mask = 0;
		for (int i = 0; i < static_cast<int>(newPaths.size()); i++) {
			if (newPaths[i].second) {
				m_path_mask |= (1 << i);
				num_allowed++;
			}
		}
		if (num_allowed == static_cast<int>(newPaths.size())) {
			m_path_mask = 0;
		}
		Ships[_singleShip].arrival_path_mask = m_path_mask;
		setModified();
		_editor->missionChanged();
	}
}

void ShipEditorDialogModel::setDeparturePaths(const SCP_vector<std::pair<SCP_string, bool>>& newPaths)
{
	_departurePaths = newPaths;
	if (!newPaths.empty() && !_multiEdit && _singleShip >= 0) {
		int num_allowed = 0;
		int m_path_mask = 0;
		for (int i = 0; i < static_cast<int>(newPaths.size()); i++) {
			if (newPaths[i].second) {
				m_path_mask |= (1 << i);
				num_allowed++;
			}
		}
		if (num_allowed == static_cast<int>(newPaths.size())) {
			m_path_mask = 0;
		}
		Ships[_singleShip].departure_path_mask = m_path_mask;
		setModified();
		_editor->missionChanged();
	}
}
void ShipEditorDialogModel::initializeData()
{
	int type, wing = -1;
	int cargo = 0, base_ship, base_player, pship = -1;
	int escort_count;
	_respawnPriority = 0;
	_texEnable = true;
	SCP_set<size_t> current_orders;
	_pshipCount = 0; // a total count of the player ships not marked
	_playerCount = 0;
	_shipCount = 0;
	_cueInit = 0;
	_totalCount = 0;
	_pvalidCount = 0;
	_playerShipIndex = 0;
	_shipOrders.clear();
	_arrivalPaths.clear();
	_departurePaths.clear();
	_altName = "";
	_callsign = "";
	object* objp;
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		_missionType = 0; // multi player mission
	} else {
		_missionType = 1; // non-multiplayer mission (implies single player mission I guess)
	}
	_shipCount = _playerCount = escort_count = _pshipCount = _pvalidCount = 0;
	base_ship = base_player = -1;
	_enable = true;
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_SHIP) && (Ships[objp->instance].flags[Ship::Ship_Flags::Escort])) {
			escort_count++; // get a total count of escort ships
		}

		if (objp->type == OBJ_START) {
			_pshipCount++; // count all player starts in mission
		}

		if (objp->flags[Object::Object_Flags::Marked]) {
			type = objp->type;
			if ((type == OBJ_START) && !_missionType) { // in multiplayer missions, starts act like ships
				type = OBJ_SHIP;
			}

			auto i = -1;
			if (type == OBJ_START) {
				_playerCount++;
				// if _playerCount is 1, base_player will be the one and only player
				i = base_player = objp->instance;

			} else if (type == OBJ_SHIP) {
				_shipCount++;
				// if _shipCount is 1, base_ship will be the one and only ship
				i = base_ship = objp->instance;
			}

			if (i >= 0) {
				if (Ship_info[Ships[i].ship_info_index].flags[Ship::Info_Flags::Player_ship]) {
					_pvalidCount++;
				}
			}
		}

		objp = GET_NEXT(objp);
	}

	_totalCount = _shipCount + _playerCount; // get total number of objects being edited.
	_multiEdit = (_totalCount > 1);

	_arrivalTreeFormula = _departureTreeFormula = -1;
	_arrivalLocation = -1;
	_arrivalDist = -1;
	_arrivalTarget = -1;
	_arrivalDelay = -1;
	_departureLocation = -1;
	_departureTarget = -1;
	_departureDelay = -1;

	_playerShipIndex = _singleShip = -1;

	_shipOrders.clear(); // assume they are all the same type
	if (_shipCount) {
		if (!_multiEdit) {
			Assert((_shipCount == 1) && (base_ship >= 0)); // NOLINT
			_shipName = Ships[base_ship].ship_name;
			_shipDisplayName = Ships[base_ship].has_display_name() ? Ships[base_ship].get_display_name() : "<none>";
		} else {
			_shipName = "";
			_shipDisplayName = "";
		}

		_updateArrival = _updateDeparture = true;

		base_player = 0;
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
				if (objp->flags[Object::Object_Flags::Marked]) {
					// do processing for both ships and players
					auto i = get_ship_from_obj(objp);
					if (base_player >= 0) {
						_shipClass = Ships[i].ship_info_index;
						_team = Ships[i].team;
						pship = (objp->type == OBJ_START) ? Qt::Checked : Qt::Unchecked;
						base_player = -1;
					} else {
						if (Ships[i].ship_info_index != _shipClass) {
							_shipClass = -1;
							_texEnable = false;
						}
						if (Ships[i].team != _team) {
							_team = -1;
						}
						pship = tristate_set(Objects[Ships[i].objnum].type == OBJ_START, pship);
					}
					// 'and' in the ship type of this ship to our running bitfield
					current_orders = ship_get_default_orders_accepted(&Ship_info[Ships[i].ship_info_index]);
					if (_shipOrders.empty()) {
						_shipOrders = current_orders;
					} else if (_shipOrders != current_orders) {
						_shipOrders = {std::numeric_limits<size_t>::max()};
					}

					if (Ships[i].flags[Ship::Ship_Flags::Escort]) {
						escort_count--; // remove marked escorts from count
					}

					if (Objects[Ships[i].objnum].type == OBJ_START) {
						_pshipCount--; // removed marked starts from count
					}
					if ((objp->type == OBJ_SHIP) || ((objp->type == OBJ_START) && !_missionType)) {
						// process this if ship not in a wing
						if (Ships[i].wingnum < 0) {
							if (!_cueInit) {
								_cueInit = 1;
								_arrivalTreeFormula = Ships[i].arrival_cue;
								_departureTreeFormula = Ships[i].departure_cue;
								_arrivalLocation = static_cast<int>(Ships[i].arrival_location);
								_arrivalDist = Ships[i].arrival_distance;
								_arrivalTarget = anchor_to_target(Ships[i].arrival_anchor);
								_arrivalDelay = Ships[i].arrival_delay;
								_departureLocation = static_cast<int>(Ships[i].departure_location);
								_departureDelay = Ships[i].departure_delay;
								_departureTarget = anchor_to_target(Ships[i].departure_anchor);

							} else {
								_cueInit++;
								if (static_cast<int>(Ships[i].arrival_location) != _arrivalLocation) {
									_arrivalLocation = -1;
								}

								if (static_cast<int>(Ships[i].departure_location) != _departureLocation) {
									_departureLocation = -1;
								}

								_arrivalDist = Ships[i].arrival_distance;
								_arrivalDelay = Ships[i].arrival_delay;
								_departureDelay = Ships[i].departure_delay;

								if (Ships[i].arrival_anchor != target_to_anchor(_arrivalTarget)) {
									_arrivalTarget = -1;
								}

								if (!cmp_sexp_chains(_arrivalTreeFormula, Ships[i].arrival_cue)) {
									_arrivalTreeFormula = -1;
									_updateArrival = false;
								}

								if (!cmp_sexp_chains(_departureTreeFormula, Ships[i].departure_cue)) {
									_departureTreeFormula = -1;
									_updateDeparture = false;
								}

								if (Ships[i].departure_anchor != target_to_anchor(_departureTarget)) {
									_departureTarget = -1;
								}
							}
						}
						// process the first ship in group, else process the rest
						if (base_ship >= 0) {
							_aiClass = Ships[i].weapons.ai_class;
							cargo = Ships[i].cargo1;
							_cargo = Cargo_names[cargo];
							_cargoTitle = Ships[i].cargo_title;
							_hotkey = Ships[i].hotkey + 1;
							_score = Ships[i].score;
							_assistScore = static_cast<int>(Ships[i].assist_score_pct * 100);

							_persona = Ships[i].persona_index;
							_altName = Fred_alt_names[base_ship];
							_callsign = Fred_callsigns[base_ship];
							if (The_mission.game_type & MISSION_TYPE_MULTI) {
								_respawnPriority = Ships[i].respawn_priority;
							}
							// we use final_death_time member of ship structure for holding the amount of time before a
							// mission to destroy this ship
							wing = Ships[i].wingnum;
							if (wing < 0) {
								_wing = "None";

							} else {
								_wing = Wings[wing].name;
								if (!_editor->query_single_wing_marked())
									_updateArrival = _updateDeparture = false;
							}

							// set routine local varaiables for ship/object flags
							_noArrivalWarp = (Ships[i].flags[Ship::Ship_Flags::No_arrival_warp]) ? 2 : 0;
							_noDepartureWarp = (Ships[i].flags[Ship::Ship_Flags::No_departure_warp]) ? 2 : 0;
							_dockWarpoutChange =
								(Ships[i].flags[Ship::Ship_Flags::Same_departure_warp_when_docked]) ? 2 : 0;
							_dockWarpinChange =
								(Ships[i].flags[Ship::Ship_Flags::Same_arrival_warp_when_docked]) ? 2 : 0;

							base_ship = -1;
							if (!_multiEdit)
								_singleShip = i;

						} else {
							if (Ships[i].weapons.ai_class != _aiClass) {
								_aiClass = -1;
							}

							if (Ships[i].cargo1 != cargo) {
								_cargo = "";
							}

							if (_cargoTitle != Ships[i].cargo_title) {
								_cargoTitle = "";
							}

							_score = Ships[i].score;
							_assistScore = static_cast<int>(Ships[i].assist_score_pct * 100);

							if (Ships[i].hotkey != _hotkey - 1) {
								_hotkey = -1;
							}

							if (Ships[i].persona_index != _persona) {
								_persona = -2;
							}

							if (Ships[i].wingnum != wing) {
								_wing = "";
							}

							_noArrivalWarp =
								tristate_set(Ships[i].flags[Ship::Ship_Flags::No_arrival_warp], _noArrivalWarp);
							_noDepartureWarp =
								tristate_set(Ships[i].flags[Ship::Ship_Flags::No_departure_warp], _noDepartureWarp);
							_dockWarpinChange =
								tristate_set(Ships[i].flags[Ship::Ship_Flags::Same_arrival_warp_when_docked],
									_dockWarpinChange);
							_dockWarpoutChange =
								tristate_set(Ships[i].flags[Ship::Ship_Flags::Same_departure_warp_when_docked], _dockWarpoutChange);
						}
					}
				}
			}

			objp = GET_NEXT(objp);
		}

		_isPlayerShip = pship;

		if (_persona > 0) {
			int persona_index = 0;
			for (int i = 0; i < _persona; i++) {
				if (Personas[i].flags & PERSONA_FLAG_WINGMAN)
					persona_index++;
			}
			_persona = persona_index;
		}
	} else {                    // no ships selected, 0 or more player ships selected
		if (_playerCount > 1) { // multiple player ships selected
			Assert(base_player >= 0);
			_shipName = "";
			_shipDisplayName = "";
			_isPlayerShip = true;
			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if ((objp->type == OBJ_START) && (objp->flags[Object::Object_Flags::Marked])) {
					auto i = objp->instance;
					if (base_player >= 0) {
						_shipClass = Ships[i].ship_info_index;
						_team = Ships[i].team;
						base_player = -1;

					} else {
						if (Ships[i].ship_info_index != _shipClass)
							_shipClass = -1;
						if (Ships[i].team != _team)
							_team = -1;
					}
				}

				objp = GET_NEXT(objp);
			}
			// only 1 player selected..
		} else if (query_valid_object(_editor->currentObject) && (Objects[_editor->currentObject].type == OBJ_START)) {
			// Assert((_playerCount == 1) && !_multiEdit);
			_playerShipIndex = Objects[_editor->currentObject].instance;
			_shipName = Ships[_playerShipIndex].ship_name;
			_shipDisplayName =
				Ships[_playerShipIndex].has_display_name() ? Ships[_playerShipIndex].get_display_name() : "<none>";
			_shipClass = Ships[_playerShipIndex].ship_info_index;
			_team = Ships[_playerShipIndex].team;
			_isPlayerShip = true;
			_altName = Fred_alt_names[_playerShipIndex];
			_callsign = Fred_callsigns[_playerShipIndex];

		} else { // no ships or players selected..
			_shipName = "";
			_shipDisplayName = "";
			_shipClass = -1;
			_team = -1;
			_persona = -1;
			_isPlayerShip = false;
		}

		_aiClass = -1;
		_cargo = "";
		_cargoTitle = "";
		_hotkey = 0;
		_score = 0; // cause control to be blank
		_assistScore = 0;
		_arrivalLocation = -1;
		_departureLocation = -1;
		_arrivalDelay = 0;
		_departureDelay = 0;
		_arrivalDist = 0;
		_arrivalTarget = -1;
		_departureTarget = -1;
		_noArrivalWarp = false;
		_noDepartureWarp = false;
		_dockWarpinChange = false;
		_dockWarpoutChange = false;
		_wing = "None";
		_enable = false;
		_arrivalPaths.clear();
		_departurePaths.clear();
	}

	// Compute common layer across all marked ships/players
	{
		_layer = "";
		bool first = true;
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) &&
				ptr->flags[Object::Object_Flags::Marked]) {
				SCP_string layer = _viewport->getObjectLayerName(OBJ_INDEX(ptr));
				if (first) {
					_layer = layer;
					first = false;
				} else if (_layer != layer) {
					_layer = "";
					break;
				}
			}
		}
	}

	modelChanged();
	_modified = false;
}

SCP_vector<std::pair<SCP_string, bool>> ShipEditorDialogModel::getArrivalPaths() const
{
	SCP_vector<std::pair<SCP_string, bool>> m_path_list;
	if (_arrivalTarget < 0 || _arrivalTarget >= MAX_SHIPS)
		return m_path_list;
	int target_class = Ships[_arrivalTarget].ship_info_index;
	auto m_model = model_get(Ship_info[target_class].model_num);
	if (!m_model || !m_model->ship_bay || m_model->ship_bay->num_paths <= 0)
		return m_path_list;
	auto m_num_paths = m_model->ship_bay->num_paths;
	auto m_path_mask = Ships[_singleShip].arrival_path_mask;

	for (int i = 0; i < m_num_paths; i++) {
		int path_id = m_model->ship_bay->path_indexes[i];
		const char* raw_name = m_model->paths[path_id].name;
		SCP_string name = (raw_name && raw_name[0]) ? SCP_string{raw_name} : SCP_string{"<unnamed path>"};
		bool allowed;
		if (m_path_mask == 0) {
			allowed = true;
		} else {
			allowed = (m_path_mask & (1 << i)) != 0;
		}
		m_path_list.emplace_back(name, allowed);
	}
	return m_path_list;
}
SCP_vector<std::pair<SCP_string, bool>> ShipEditorDialogModel::getDeparturePaths() const
{
	SCP_vector<std::pair<SCP_string, bool>> m_path_list;
	if (_departureTarget < 0 || _departureTarget >= MAX_SHIPS)
		return m_path_list;
	int target_class = Ships[_departureTarget].ship_info_index;
	auto m_model = model_get(Ship_info[target_class].model_num);
	if (!m_model || !m_model->ship_bay || m_model->ship_bay->num_paths <= 0)
		return m_path_list;
	auto m_num_paths = m_model->ship_bay->num_paths;
	auto m_path_mask = Ships[_singleShip].departure_path_mask;

	for (int i = 0; i < m_num_paths; i++) {
		int path_id = m_model->ship_bay->path_indexes[i];
		const char* raw_name = m_model->paths[path_id].name;
		SCP_string name = (raw_name && raw_name[0]) ? SCP_string{raw_name} : SCP_string{"<unnamed path>"};
		bool allowed;
		if (m_path_mask == 0) {
			allowed = true;
		} else {
			allowed = (m_path_mask & (1 << i)) != 0;
		}
		m_path_list.emplace_back(name, allowed);
	}
	return m_path_list;
}
bool ShipEditorDialogModel::apply()
{
	return true;
}

void ShipEditorDialogModel::reject() {}
void ShipEditorDialogModel::shipAltNameClose(int ship)
{

	if (_multiEdit) {
		return;
	}
	drop_white_space(_altName);
	if (_altName.empty()) {
		return;
	}

	if (!_altName.empty() || !stricmp(_altName.c_str(), "<none>")) {
		if (*Fred_alt_names[ship]) {
			bool used = false;
			for (int i = 0; i < MAX_SHIPS; ++i) {
				if (i != ship && !strcmp(Fred_alt_names[i], Fred_alt_names[ship])) {
					used = true;
					break;
				}
			}
			if (!used) {
				mission_parse_remove_alt(Fred_alt_names[ship]);
			}
			// zero the entry
			strcpy_s(Fred_alt_names[ship], "");
		}
	}
	// otherwise see if it already exists
	if (mission_parse_lookup_alt(_altName.c_str()) >= 0) {
		strcpy_s(Fred_alt_names[ship], _altName.c_str());
		return;
	}
	// otherwise try and add it
	if (mission_parse_add_alt(_altName.c_str()) >= 0) {
		strcpy_s(Fred_alt_names[ship], _altName.c_str());
		return;
	}
	strcpy_s(Fred_alt_names[ship], "");
	_viewport->dialogProvider->showButtonDialog(DialogType::Error,
		"Alt Name Error",
		"Couldn't add new alternate type name. Already using too many!",
		{DialogButton::Ok});
}

void ShipEditorDialogModel::shipCallsignClose(int ship)
{

	if (_multiEdit) {
		return;
	}
	drop_white_space(_callsign);
	if (_callsign.empty()) {
		return;
	}

	if (!_callsign.empty() || !stricmp(_callsign.c_str(), "<none>")) {
		if (*Fred_callsigns[ship]) {
			bool used = false;
			for (int i = 0; i < MAX_SHIPS; ++i) {
				if (i != ship && !strcmp(Fred_callsigns[i], Fred_callsigns[ship])) {
					used = true;
					break;
				}
			}
			if (!used) {
				mission_parse_remove_callsign(Fred_callsigns[ship]);
			}
			// zero the entry
			strcpy_s(Fred_callsigns[ship], "");
		}
	}
	// otherwise see if it already exists
	if (mission_parse_lookup_callsign(_callsign.c_str()) >= 0) {
		strcpy_s(Fred_callsigns[ship], _callsign.c_str());
		return;
	}
	// otherwise try and add it
	if (mission_parse_add_callsign(_callsign.c_str()) >= 0) {
		strcpy_s(Fred_callsigns[ship], _callsign.c_str());
		return;
	}
	strcpy_s(Fred_callsigns[ship], "");
	_viewport->dialogProvider->showButtonDialog(DialogType::Error,
		"Alt Name Error",
		"Couldn't add new Callsign. Already using too many!",
		{DialogButton::Ok});
}

void ShipEditorDialogModel::setShipName(const SCP_string& m_ship_name)
{
	if (_shipName == m_ship_name)
		return;

	// Name changes only apply to single-ship editing
	if (_multiEdit || _singleShip < 0)
		return;

	SCP_string new_name = m_ship_name;
	drop_white_space(new_name);

	if (new_name.empty()) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Ship Name Error",
			"A ship name cannot be empty.",
			{DialogButton::Ok});
		_shipName = Ships[_singleShip].ship_name;
		modelChanged();
		return;
	}

	if (new_name[0] == '<') {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Ship Name Error",
			"Ship names not allowed to begin with <.",
			{DialogButton::Ok});
		_shipName = Ships[_singleShip].ship_name;
		modelChanged();
		return;
	}

	// Check for duplicate ship names
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->instance != _singleShip)) {
			if (!stricmp(new_name.c_str(), Ships[ptr->instance].ship_name)) {
				_viewport->dialogProvider->showButtonDialog(DialogType::Error,
					"Ship Name Error",
					"This ship name is already being used by another ship.",
					{DialogButton::Ok});
				_shipName = Ships[_singleShip].ship_name;
				modelChanged();
				return;
			}
		}
	}

	// Check for conflict with wing names
	for (auto& w : Wings) {
		if (w.wave_count && !stricmp(w.name, new_name.c_str())) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Ship Name Error",
				"This ship name is already being used by a wing.",
				{DialogButton::Ok});
			_shipName = Ships[_singleShip].ship_name;
			modelChanged();
			return;
		}
	}

	// Check for conflict with AI target priority group names
	for (auto& tp : Ai_tp_list) {
		if (!stricmp(new_name.c_str(), tp.name)) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Ship Name Error",
				"This ship name is already being used by a target priority group.",
				{DialogButton::Ok});
			_shipName = Ships[_singleShip].ship_name;
			modelChanged();
			return;
		}
	}

	// Check for conflict with waypoint paths
	if (find_matching_waypoint_list(new_name.c_str()) != nullptr) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Ship Name Error",
			"This ship name is already being used by a waypoint path.",
			{DialogButton::Ok});
		_shipName = Ships[_singleShip].ship_name;
		modelChanged();
		return;
	}

	// Check for conflict with jump nodes
	if (jumpnode_get_by_name(new_name.c_str()) != nullptr) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Ship Name Error",
			"This ship name is already being used by a jump node.",
			{DialogButton::Ok});
		_shipName = Ships[_singleShip].ship_name;
		modelChanged();
		return;
	}

	// Wing member ships cannot be renamed independently
	int wing = Ships[_singleShip].wingnum;
	if (wing >= 0) {
		Assert((wing < MAX_WINGS) && Wings[wing].wave_count); // NOLINT
		char expected_name[255];
		int j;
		for (j = 0; j < Wings[wing].wave_count; j++)
			if (_editor->wing_objects[wing][j] == Ships[_singleShip].objnum)
				break;
		Assert(j < Wings[wing].wave_count);
		wing_bash_ship_name(expected_name, Wings[wing].name, static_cast<int>(j + 1));
		if (strcmp(expected_name, new_name.c_str())) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Ship Name Error",
				"This ship is part of a wing, and its name cannot be changed",
				{DialogButton::Ok});
			_shipName = Ships[_singleShip].ship_name;
			modelChanged();
			return;
		}
	}

	// All validation passed — write the new name
	char old_name[NAME_LENGTH];
	strcpy_s(old_name, Ships[_singleShip].ship_name);
	strcpy_s(Ships[_singleShip].ship_name, new_name.c_str());
	_shipName = new_name;

	if (strcmp(old_name, Ships[_singleShip].ship_name)) {
		update_sexp_references(old_name, Ships[_singleShip].ship_name);
		_editor->ai_update_goal_references(sexp_ref_type::SHIP, old_name, Ships[_singleShip].ship_name);
		_editor->update_texture_replacements(old_name, Ships[_singleShip].ship_name);
		int j = find_item_with_string(Reinforcements, &reinforcements::name, old_name);
		if (j >= 0) {
			Assert(strlen(Ships[_singleShip].ship_name) < NAME_LENGTH);
			strcpy_s(Reinforcements[j].name, Ships[_singleShip].ship_name);
		}
	}

	setModified();
	_editor->missionChanged();
	modelChanged();
}
SCP_string ShipEditorDialogModel::getShipName() const
{
	return _shipName;
}

void ShipEditorDialogModel::setShipDisplayName(const SCP_string& m_ship_display_name)
{
	if (_shipDisplayName == m_ship_display_name)
		return;
	_shipDisplayName = m_ship_display_name;

	// Display name only applies to single-ship editing
	if (!_multiEdit && _singleShip >= 0) {
		SCP_string display = m_ship_display_name;
		lcl_fred_replace_stuff(display);
		if (display == _shipName || stricmp(display.c_str(), "<none>") == 0) {
			Ships[_singleShip].display_name = "";
			Ships[_singleShip].flags.remove(Ship::Ship_Flags::Has_display_name);
		} else {
			Ships[_singleShip].display_name = display;
			Ships[_singleShip].flags.set(Ship::Ship_Flags::Has_display_name);
		}
		setModified();
		_editor->missionChanged();
	}
	modelChanged();
}
SCP_string ShipEditorDialogModel::getShipDisplayName() const
{
	return _shipDisplayName;
}

void ShipEditorDialogModel::setShipClass(int m_ship_class)
{
	if (_shipClass == m_ship_class)
		return;
	_shipClass = m_ship_class;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].ship_info_index != m_ship_class) {
				change_ship_type(ptr->instance, m_ship_class);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getShipClass() const
{
	return _shipClass;
}

void ShipEditorDialogModel::setAIClass(const int m_ai_class)
{
	if (_aiClass == m_ai_class)
		return;
	_aiClass = m_ai_class;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].weapons.ai_class = m_ai_class;
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getAIClass() const
{
	return _aiClass;
}

void ShipEditorDialogModel::setTeam(const int m_team)
{
	if (_team == m_team)
		return;
	_team = m_team;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].team = m_team;
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getTeam() const
{
	return _team;
}

SCP_string ShipEditorDialogModel::getLayer() const
{
	return _layer;
}

void ShipEditorDialogModel::setLayer(const SCP_string& layer)
{
	if (_layer == layer)
		return;
	_layer = layer;

	for (auto objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (objp->flags[Object::Object_Flags::Marked]) {
			if (objp->type == OBJ_SHIP || objp->type == OBJ_START) {
				int obj_idx = OBJ_INDEX(objp);
				_viewport->moveObjectToLayer(obj_idx, layer);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

void ShipEditorDialogModel::setCargo(const SCP_string& m_cargo)
{
	if (_cargo == m_cargo)
		return;
	_cargo = m_cargo;

	SCP_string cargo_name = m_cargo;
	lcl_fred_replace_stuff(cargo_name);
	int z = string_lookup(cargo_name.c_str(), Cargo_names, Num_cargo);
	if (z == -1) {
		if (Num_cargo < MAX_CARGO) {
			z = Num_cargo++;
			strcpy(Cargo_names[z], cargo_name.c_str());
		} else {
			SCP_string str;
			sprintf(str, "Maximum number of cargo names %d reached.\nIgnoring new name.\n", MAX_CARGO);
			_viewport->dialogProvider->showButtonDialog(DialogType::Warning, "Cargo Error", str, {DialogButton::Ok});
			z = 0;
			_cargo = Cargo_names[z];
		}
	}
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].cargo1 = (char)z;
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

SCP_string ShipEditorDialogModel::getCargo() const
{
	return _cargo;
}

void ShipEditorDialogModel::setCargoTitle(const SCP_string& title)
{
	if (_cargoTitle == title)
		return;
	_cargoTitle = title;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			strcpy_s(Ships[ptr->instance].cargo_title, title.c_str());
		}
	}
	setModified();
}

SCP_string ShipEditorDialogModel::getCargoTitle() const
{
	return _cargoTitle;
}

void ShipEditorDialogModel::setAltName(const SCP_string& m_altName)
{
	if (_altName == m_altName)
		return;
	_altName = m_altName;
	if (!_multiEdit && _singleShip >= 0) {
		shipAltNameClose(_singleShip);
		setModified();
		_editor->missionChanged();
	}
	modelChanged();
}

SCP_string ShipEditorDialogModel::getAltName() const
{
	return _altName;
}

void ShipEditorDialogModel::setCallsign(const SCP_string& m_callsign)
{
	if (_callsign == m_callsign)
		return;
	_callsign = m_callsign;
	if (!_multiEdit && _singleShip >= 0) {
		shipCallsignClose(_singleShip);
		setModified();
		_editor->missionChanged();
	}
	modelChanged();
}

SCP_string ShipEditorDialogModel::getCallsign() const
{
	return _callsign;
}

SCP_string ShipEditorDialogModel::getWing() const
{
	return _wing;
}

void ShipEditorDialogModel::setHotkey(const int m_hotkey)
{
	if (_hotkey == m_hotkey)
		return;
	_hotkey = m_hotkey;
	// hotkey stored as 1-indexed in model (0 = none), Ships[] uses 0-indexed (-1 = none, hotkey-1 otherwise)
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].hotkey = m_hotkey - 1;
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getHotkey() const
{
	return _hotkey;
}

void ShipEditorDialogModel::setPersona(const int m_persona)
{
	if (_persona == m_persona)
		return;
	_persona = m_persona;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].persona_index = m_persona;
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getPersona() const
{
	return _persona;
}

void ShipEditorDialogModel::setScore(const int m_score)
{
	if (_score == m_score)
		return;
	_score = m_score;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].score = m_score;
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getScore() const
{
	return _score;
}

void ShipEditorDialogModel::setAssist(const int m_assist_score)
{
	if (_assistScore == m_assist_score)
		return;
	_assistScore = m_assist_score;
	float pct = static_cast<float>(m_assist_score) / 100.0f;
	pct = std::clamp(pct, 0.0f, 1.0f);
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].assist_score_pct = pct;
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getAssist() const
{
	return _assistScore;
}

void ShipEditorDialogModel::setPlayer(const bool m_player)
{
	if (_isPlayerShip == m_player)
		return;
	_isPlayerShip = m_player;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (m_player) {
				if (Objects[Ships[ptr->instance].objnum].type != OBJ_START) {
					Player_starts++;
				}
				Objects[Ships[ptr->instance].objnum].type = OBJ_START;
			} else {
				if (Objects[Ships[ptr->instance].objnum].type == OBJ_START) {
					Player_starts--;
				}
				Objects[Ships[ptr->instance].objnum].type = OBJ_SHIP;
			}
		}
	}
	// fix up Player_start_shipnum if it became invalid
	ensure_valid_player_start_shipnum();
	setModified();
	_editor->missionChanged();
	modelChanged();
}

bool ShipEditorDialogModel::getPlayer() const
{
	return _isPlayerShip;
}

void ShipEditorDialogModel::setRespawn(const int value)
{
	if (_respawnPriority == value)
		return;
	_respawnPriority = value;
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				Ships[ptr->instance].respawn_priority = value;
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getRespawn() const
{
	return _respawnPriority;
}

void ShipEditorDialogModel::setArrivalLocationIndex(const int value)
{
	if (_arrivalLocation == value)
		return;
	_arrivalLocation = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].arrival_location = static_cast<ArrivalLocation>(value);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getArrivalLocationIndex() const
{
	return _arrivalLocation;
}

void ShipEditorDialogModel::setArrivalLocation(const ArrivalLocation value)
{
	setArrivalLocationIndex(static_cast<int>(value));
}

ArrivalLocation ShipEditorDialogModel::getArrivalLocation() const
{
	return static_cast<ArrivalLocation>(_arrivalLocation);
}

bool ShipEditorDialogModel::arrivalNeedsTarget() const
{
	return getArrivalLocation() != ArrivalLocation::AT_LOCATION;
}

bool ShipEditorDialogModel::arrivalNeedsDistance() const
{
	switch (getArrivalLocation()) {
		case ArrivalLocation::AT_LOCATION:
		case ArrivalLocation::FROM_DOCK_BAY:
			return false;
		default:
			return true;
	}
}

int ShipEditorDialogModel::computeArrivalMinDist() const
{
	// Validation only applies when arriving near a ship (not hyperspace or dock bay)
	if (getArrivalLocation() == ArrivalLocation::AT_LOCATION ||
	    getArrivalLocation() == ArrivalLocation::FROM_DOCK_BAY)
		return 0;

	// Validation doesn't apply to special anchors (negative value or ANCHOR_SPECIAL_ARRIVAL flag set)
	if (_arrivalTarget < 0 || (_arrivalTarget & ANCHOR_SPECIAL_ARRIVAL))
		return 0;

	// Compute the most restrictive minimum distance across all marked arriving ships
	int max_d = 0;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) &&
		    ptr->flags[Object::Object_Flags::Marked] &&
		    Ships[ptr->instance].wingnum < 0 &&
		    Ships[ptr->instance].objnum >= 0) {
			const int d = static_cast<int>(std::min(MIN_TARGET_ARRIVAL_DISTANCE,
			    MIN_TARGET_ARRIVAL_MULTIPLIER * Objects[Ships[ptr->instance].objnum].radius));
			max_d = std::max(max_d, d);
		}
	}
	return max_d;
}

void ShipEditorDialogModel::setArrivalTarget(const int value)
{
	if (_arrivalTarget == value)
		return;
	_arrivalTarget = value;

	// Re-validate the existing arrival distance now that the target has changed.
	// A target change from a special anchor to a real ship can make a previously
	// acceptable distance too close.
	const int min_dist = computeArrivalMinDist();
	if (min_dist > 0 && _arrivalDist > -min_dist && _arrivalDist < min_dist) {
		const int clamped = (_arrivalDist < 0) ? -min_dist : min_dist;
		QMessageBox::warning(nullptr,
		    tr("Arrival Distance"),
		    tr("Ship must arrive at least %1 meters away from target.\n"
		       "Value has been reset to this.  Use with caution!")
		        .arg(min_dist));
		_arrivalDist = clamped;
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) &&
			    ptr->flags[Object::Object_Flags::Marked] &&
			    Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].arrival_distance = clamped;
			}
		}
	}

	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0 && value >= 0) {
				Ships[ptr->instance].arrival_anchor = target_to_anchor(value);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getArrivalTarget() const
{
	return _arrivalTarget;
}

void ShipEditorDialogModel::setArrivalDistance(const int value)
{
	if (_arrivalDist == value)
		return;

	const int min_dist = computeArrivalMinDist();
	int effective_value = value;
	if (min_dist > 0 && value > -min_dist && value < min_dist) {
		effective_value = (value < 0) ? -min_dist : min_dist;
		QMessageBox::warning(nullptr,
		    tr("Arrival Distance"),
		    tr("Ship must arrive at least %1 meters away from target.\n"
		       "Value has been reset to this.  Use with caution!")
		        .arg(min_dist));
	}

	_arrivalDist = effective_value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].arrival_distance = effective_value;
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getArrivalDistance() const
{
	return _arrivalDist;
}

void ShipEditorDialogModel::setArrivalDelay(const int value)
{
	if (_arrivalDelay == value)
		return;
	_arrivalDelay = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].arrival_delay = value;
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getArrivalDelay() const
{
	return _arrivalDelay;
}

void ShipEditorDialogModel::setArrivalCue(const bool value)
{
	modify(_updateArrival, value);
}

bool ShipEditorDialogModel::getArrivalCue() const
{
	return _updateArrival;
}

void ShipEditorDialogModel::setArrivalTreeDirty(int formula)
{
	if (_multiEdit && !_updateArrival)
		return;

	_arrivalTreeFormula = formula;
	_updateArrival = true;

	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && ptr->flags[Object::Object_Flags::Marked]) {
			auto i = ptr->instance;
			if (Ships[i].wingnum >= 0)
				continue;
			if (Ships[i].arrival_cue >= 0 && Ships[i].arrival_cue != formula)
				free_sexp2(Ships[i].arrival_cue);
			Ships[i].arrival_cue = formula;
		}
	}

	setModified();
	_editor->missionChanged();
}

int ShipEditorDialogModel::getArrivalFormula() const
{
	return _arrivalTreeFormula;
}

void ShipEditorDialogModel::setNoArrivalWarp(const int value)
{
	if (_noArrivalWarp == value)
		return;
	_noArrivalWarp = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (value) {
				Ships[ptr->instance].flags.set(Ship::Ship_Flags::No_arrival_warp);
			} else {
				Ships[ptr->instance].flags.remove(Ship::Ship_Flags::No_arrival_warp);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getNoArrivalWarp() const
{
	return _noArrivalWarp;
}

void ShipEditorDialogModel::setDockWarpinChange(const int state)
{
	if (_dockWarpinChange == state)
		return;
	_dockWarpinChange = state;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (state) {
				Ships[ptr->instance].flags.set(Ship::Ship_Flags::Same_arrival_warp_when_docked);
			} else {
				Ships[ptr->instance].flags.remove(Ship::Ship_Flags::Same_arrival_warp_when_docked);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getDockWarpinChange() const
{
	return _dockWarpinChange;
}

void ShipEditorDialogModel::setDepartureLocationIndex(const int value)
{
	if (_departureLocation == value)
		return;
	_departureLocation = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].departure_location = static_cast<DepartureLocation>(value);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getDepartureLocationIndex() const
{
	return _departureLocation;
}

void ShipEditorDialogModel::setDepartureLocation(const DepartureLocation value)
{
	setDepartureLocationIndex(static_cast<int>(value));
}

DepartureLocation ShipEditorDialogModel::getDepartureLocation() const
{
	return static_cast<DepartureLocation>(_departureLocation);
}

void ShipEditorDialogModel::setDepartureTarget(const int value)
{
	if (_departureTarget == value)
		return;
	_departureTarget = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0 && value >= 0) {
				Ships[ptr->instance].departure_anchor = target_to_anchor(value);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getDepartureTarget() const
{
	return _departureTarget;
}

void ShipEditorDialogModel::setDepartureDelay(const int value)
{
	if (_departureDelay == value)
		return;
	_departureDelay = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].departure_delay = value;
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getDepartureDelay() const
{
	return _departureDelay;
}

void ShipEditorDialogModel::setDepartureCue(const bool value)
{
	modify(_updateDeparture, value);
}

bool ShipEditorDialogModel::getDepartureCue() const
{
	return _updateDeparture;
}

void ShipEditorDialogModel::setDepartureTreeDirty(int formula)
{
	if (_multiEdit && !_updateDeparture)
		return;

	_departureTreeFormula = formula;
	_updateDeparture = true;

	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && ptr->flags[Object::Object_Flags::Marked]) {
			auto i = ptr->instance;
			if (Ships[i].wingnum >= 0)
				continue;
			if (Ships[i].departure_cue >= 0 && Ships[i].departure_cue != formula)
				free_sexp2(Ships[i].departure_cue);
			Ships[i].departure_cue = formula;
		}
	}

	setModified();
	_editor->missionChanged();
}

int ShipEditorDialogModel::getDepartureFormula() const
{
	return _departureTreeFormula;
}

void ShipEditorDialogModel::setNoDepartureWarp(const int value)
{
	if (_noDepartureWarp == value)
		return;
	_noDepartureWarp = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (value) {
				Ships[ptr->instance].flags.set(Ship::Ship_Flags::No_departure_warp);
			} else {
				Ships[ptr->instance].flags.remove(Ship::Ship_Flags::No_departure_warp);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getNoDepartureWarp() const
{
	return _noDepartureWarp;
}

void ShipEditorDialogModel::setDockWarpoutChange(const int state) {
	if (_dockWarpoutChange == state)
		return;
	_dockWarpoutChange = state;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (state) {
				Ships[ptr->instance].flags.set(Ship::Ship_Flags::Same_departure_warp_when_docked);
			} else {
				Ships[ptr->instance].flags.remove(Ship::Ship_Flags::Same_departure_warp_when_docked);
			}
		}
	}
	setModified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getDockWarpoutChange() const
{
	return _dockWarpoutChange;
}

void ShipEditorDialogModel::onPrevious()
{
	int i, n, arr[MAX_SHIPS];
	n = makeShipList(arr);
	if (!n) {
		return;
	}
	if (_editor->cur_ship < 0) {
		i = n - 1;
	} else {
		for (i = 0; i < n; i++) {
			if (Ships[_editor->cur_ship].objnum == arr[i]) {
				break;
			}
		}

		Assert(i < n);
		i--;
		if (i < 0) {
			i = n - 1;
		}
	}

	_editor->unmark_all();
	_editor->selectObject(arr[i]);
}

void ShipEditorDialogModel::onNext()
{
	int i, n, arr[MAX_SHIPS];
	n = makeShipList(arr);
	if (!n)
		return;

	if (_editor->cur_ship < 0)
		i = 0;
	else {
		for (i = 0; i < n; i++)
			if (Ships[_editor->cur_ship].objnum == arr[i])
				break;

		Assert(i < n);
		i++;
		if (i == n)
			i = 0;
	}

	_editor->unmark_all();
	_editor->selectObject(arr[i]);
}

void ShipEditorDialogModel::onDeleteShip()
{
	_editor->delete_marked();
	_editor->unmark_all();
}

void ShipEditorDialogModel::onShipReset()
{
	int i, j, index, ship;
	object* objp;
	ship_info* sip;
	ship_subsys* ptr;
	ship_weapon* wp;
	model_subsystem* sp;

	setCargo("Nothing");
	setAIClass(AI_DEFAULT_CLASS);
	if (_shipClass >= 0) {
		setTeam(Species_info[Ship_info[_shipClass].species].default_iff);
	}

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (((objp->type == OBJ_SHIP) || ((objp->type == OBJ_START) && !_missionType)) &&
			(objp->flags[Object::Object_Flags::Marked])) {
			ship = objp->instance;

			// reset ship goals
			for (i = 0; i < MAX_AI_GOALS; i++) {
				Ai_info[Ships[ship].ai_index].goals[i].ai_mode = AI_GOAL_NONE;
			}

			objp->phys_info.speed = 0.0f;
			objp->shield_quadrant[0] = 100.0f;
			objp->hull_strength = 100.0f;

			sip = &Ship_info[Ships[ship].ship_info_index];
			for (i = 0; i < sip->num_primary_banks; i++)
				Ships[ship].weapons.primary_bank_weapons[i] = sip->primary_bank_weapons[i];

			for (i = 0; i < sip->num_secondary_banks; i++) {
				Ships[ship].weapons.secondary_bank_weapons[i] = sip->secondary_bank_weapons[i];
				Ships[ship].weapons.secondary_bank_capacity[i] = sip->secondary_bank_ammo_capacity[i];
			}

			index = 0;
			ptr = GET_FIRST(&Ships[ship].subsys_list);
			while (ptr != END_OF_LIST(&Ships[ship].subsys_list)) {
				ptr->current_hits = 0.0f;
				if (ptr->system_info->type == SUBSYSTEM_TURRET) {
					wp = &ptr->weapons;
					sp = &Ship_info[Ships[ship].ship_info_index].subsystems[index];

					j = 0;
					for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
						if (sp->primary_banks[i] != -1) {
							wp->primary_bank_weapons[j++] = sp->primary_banks[i];
						}
					}

					wp->num_primary_banks = j;
					j = 0;
					for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
						if (sp->secondary_banks[i] != -1) {
							wp->secondary_bank_weapons[j] = sp->secondary_banks[i];
							wp->secondary_bank_capacity[j++] = sp->secondary_bank_capacity[i];
						}
					}

					wp->num_secondary_banks = j;
					for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
						wp->secondary_bank_ammo[i] = 100;
					}
				}

				index++;
				ptr = GET_NEXT(ptr);
			}
		}

		objp = GET_NEXT(objp);
	}
	modelChanged();
	_editor->missionChanged();
	if (_multiEdit) {

		_viewport->dialogProvider->showButtonDialog(DialogType::Information,
			"Reset",
			"Ships reset to ship class defaults",
			{DialogButton::Ok});
	} else {

		_viewport->dialogProvider->showButtonDialog(DialogType::Information,
			"Reset",
			"Ship reset to ship class defaults",
			{DialogButton::Ok});
	}
}

bool ShipEditorDialogModel::wingIsPlayerWing(int wingNum) const
{
	return _editor->wing_is_player_wing(wingNum);
}

const SCP_set<size_t>& ShipEditorDialogModel::getShipOrders() const
{
	return _shipOrders;
}

bool ShipEditorDialogModel::getTexEditEnable() const
{
	return _texEnable;
}

int ShipEditorDialogModel::makeShipList(int* arr)
{
	int n = 0;
	object* ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			arr[n++] = OBJ_INDEX(ptr);
		}

		ptr = GET_NEXT(ptr);
	}

	return n;
}

void ShipEditorDialogModel::setModified()
{
	if (!_modified) {
		_modified = true;
	}
}
} // namespace fso::fred::dialogs